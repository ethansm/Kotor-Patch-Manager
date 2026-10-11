#!/usr/bin/env python3
"""Fix stage F3 input: group reviewer-accepted, check-clean patches into integration batches (no LLM).

Usage:
  build_integration_batches.py [--dry-run] [--force] [--no-compat-list]   build work/fix/batches/batch_NN.{json,md}
  build_integration_batches.py pending [--all]     batch ids without an ok integrator run, or with an ok run not yet verified
  build_integration_batches.py publish <batch>     copy the verified final series to $KOTOR2_FIX_OUT/<batch>/
                                                   NNNN-<tid>.patch + series.json (exit 1 unless verify ok)
  build_integration_batches.py --self-test

Input per queue task: the LATEST attempt (highest N over results/patches/checks/reviews) whose review is
reviews/<tid>.aN.review.json status ok with output.verdict == accept AND checks/<tid>.aN.check.json ok.
Calibration task ids (work/fix/calib_key.json) are never batched. Only the ids are read from the key.

Batches: _shared_src first, then one batch per mod (alphabetical), then scripts (grouped by file, small files
share a batch), then tests, then others; <= 8 patches per batch, same-file patches kept together, queue `order`
kept. gates = {build: [mods], tests: [run_tests.py filters], compat: [_shared_src files]}.
Refuses to renumber when integ outputs exist and the plan differs, unless --force.
Env: FIX_COMPAT_LIST_CMD (stand-in for `compat_check.py <files> --list`; prints the BUILD line), standard FIX_* dirs.
"""
from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402

MAX_PER_BATCH = 8
ATT_RE = re.compile(r"\.a(\d+)\.")
CLAIM_MAX = 900


# ---------------------------------------------------------------- per-task state (shared with fix_merge.py)
def attempt_numbers(tid: str) -> list[int]:
    nums = set()
    for d in (fc.RESULTS, fc.PATCHES, fc.CHECKS, fc.REVIEWS):
        if Path(d).is_dir():
            for p in Path(d).glob(f"{tid}.a*"):
                m = ATT_RE.search(p.name)
                if m and p.name.startswith(tid + ".a"):
                    nums.add(int(m.group(1)))
    return sorted(nums)


def task_state(tid: str, attempt: int | None = None) -> dict:
    nums = attempt_numbers(tid)
    n = attempt if attempt is not None else (nums[-1] if nums else 0)
    st = {"task_id": tid, "attempt": n, "attempts": nums, "result": None, "scope": None, "check": None,
          "review": None, "patch": None}
    if not n:
        return st
    st["result"] = fc.rj(fc.RESULTS / f"{tid}.a{n}.json")
    st["scope"] = fc.rj(fc.PATCHES / f"{tid}.a{n}.scope.json")
    st["check"] = fc.rj(fc.CHECKS / f"{tid}.a{n}.check.json")
    st["review"] = fc.rj(fc.REVIEWS / f"{tid}.a{n}.review.json")
    pp = fc.PATCHES / f"{tid}.a{n}.patch"
    st["patch"] = pp if pp.is_file() else None
    return st


def review_verdict(rv) -> str | None:
    """accept|revise|reject from an OK review record, else None."""
    if isinstance(rv, dict) and rv.get("status") == "ok" and rv.get("schema_valid") is not False \
            and isinstance(rv.get("output"), dict):
        v = rv["output"].get("verdict")
        return v if v in ("accept", "revise", "reject") else None
    return None


def eligible(st: dict) -> bool:
    if not st["attempt"] or st["patch"] is None or not isinstance(st["check"], dict):
        return False
    if st["check"].get("ok") is not True or review_verdict(st["review"]) != "accept":
        return False
    sc = st["scope"]
    if isinstance(sc, dict) and (sc.get("ok") is False or sc.get("empty") is True):
        return False
    return True


def calib_ids() -> set[str]:
    k = fc.rj(fc.CALIB_KEY)
    return set((k or {}).get("items", {}).keys()) if isinstance(k, dict) else set()


# ---------------------------------------------------------------- grouping
def patch_files(text: str) -> list[str]:
    out = []
    for m in re.finditer(r"^\+\+\+ b/(\S+)", text, re.M):
        p = m.group(1)
        if p not in out:
            out.append(p)
    return out


def category(task: dict, files: list[str]) -> tuple[int, str]:
    pf = task.get("primary_file") or (files[0] if files else "")
    mod = fc.mod_of(pf)
    if mod == "_shared_src":
        return 0, "_shared_src"
    if mod:
        return 1, mod
    if pf.startswith("scripts/"):
        return 2, "scripts"
    if pf.startswith("tests/"):
        return 3, "tests"
    return 4, "other"


def pack(tasks: list[dict]) -> list[list[dict]]:
    """tasks sorted by queue order -> file groups kept together -> chunks of <= MAX_PER_BATCH."""
    groups: dict[str, list[dict]] = {}
    for t in tasks:
        groups.setdefault(t["primary"], []).append(t)
    ordered = sorted(groups.values(), key=lambda g: (min(x["order"] for x in g), g[0]["primary"]))
    chunks: list[list[dict]] = []
    cur: list[dict] = []
    for g in ordered:
        g = sorted(g, key=lambda x: (x["order"], x["task_id"]))
        while len(g) > MAX_PER_BATCH:                    # one file with > 8 patches: split regardless
            if cur:
                chunks.append(cur)
                cur = []
            chunks.append(g[:MAX_PER_BATCH])
            g = g[MAX_PER_BATCH:]
        if len(cur) + len(g) > MAX_PER_BATCH:
            chunks.append(cur)
            cur = []
        cur.extend(g)
    if cur:
        chunks.append(cur)
    return chunks


def compat_mods(files: list[str], use_cmd: bool) -> list[str]:
    """Mods that must be rebuilt for _shared_src files: parse the BUILD line of compat_check --list."""
    if not files or not use_cmd:
        return []
    env_cmd = os.environ.get("FIX_COMPAT_LIST_CMD")
    if env_cmd is not None:
        cmd = env_cmd.split() + files if env_cmd.strip() else None
        cwd = None
    else:
        root = fc.BASE if (fc.BASE / "scripts" / "compat_check.py").is_file() else fc.REPO
        script = root / "scripts" / "compat_check.py"
        cmd = [sys.executable, "-I", str(script)] + files + ["--list"] if script.is_file() else None
        cwd = str(root)
    if not cmd:
        return []
    try:
        r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=180)
    except (OSError, subprocess.SubprocessError):
        return []
    mods: list[str] = []
    for line in r.stdout.splitlines():
        if line.startswith("BUILD"):
            for m in re.findall(r"--mod\s+(\S+)", line):
                if "/" not in m and m not in mods:
                    mods.append(m)
    return sorted(mods)


def test_filters(files: list[str]) -> list[str]:
    out: list[str] = []

    def add(x):
        if x and x not in out:
            out.append(x)

    for f in files:
        base = Path(f).name
        mod = fc.mod_of(f)
        if mod:
            add(mod)
        elif f.startswith("scripts/") and f.endswith(".py"):
            if base.startswith("test_"):
                add(f)
            else:
                cand = f"scripts/test_{base}"
                if (fc.BASE / cand).is_file():
                    add(cand)
        elif f.startswith("tests/"):
            if base.startswith("test_") and base.endswith(".py"):
                add(f)
            elif base.endswith("_test.cpp"):
                add(base[:-len(".cpp")])
            else:
                parts = f.split("/")
                add("/".join(parts[:2]) if len(parts) > 2 else "")
    return sorted(out)


def _short(s: str, n: int) -> str:
    s = " ".join(str(s or "").split())
    return s if len(s) <= n else s[:n - 3] + "..."


def load_eligible(queue: dict) -> tuple[list[dict], dict]:
    skip = calib_ids()
    items, why = [], {"calibration": 0, "not_accepted": 0}
    for t in queue["tasks"]:
        tid = t["task_id"]
        if tid in skip:
            why["calibration"] += 1
            continue
        st = task_state(tid)
        if not eligible(st):
            why["not_accepted"] += 1
            continue
        text = st["patch"].read_text(encoding="utf-8", errors="replace")
        files = patch_files(text) or list(t.get("allowed_files") or [])
        sc = st["scope"] or {}
        for f in sc.get("changed_files") or []:
            if f not in files:
                files.append(f)
        files = sorted(files)
        rank, key = category(t, files)
        items.append({"task_id": tid, "attempt": st["attempt"], "patch": str(st["patch"]), "text": text,
                      "files": files, "primary": t.get("primary_file") or (files[0] if files else ""),
                      "order": t.get("order") if isinstance(t.get("order"), (int, float)) else 10 ** 9,
                      "rank": rank, "key": key, "task": t, "check": st["check"], "review": st["review"],
                      "result": st["result"]})
    return items, why


def plan_batches(items: list[dict], use_compat: bool = True) -> list[dict]:
    pools: dict[tuple[int, str], list[dict]] = {}
    for it in items:
        k = (it["rank"], it["key"] if it["rank"] in (0, 1) else it["key"])
        pools.setdefault(k, []).append(it)
    batches: list[dict] = []
    for (rank, key) in sorted(pools):
        for chunk in pack(pools[(rank, key)]):
            batches.append({"category": key, "rank": rank, "items": chunk})
    plan = []
    for i, b in enumerate(batches, 1):
        files = sorted({f for it in b["items"] for f in it["files"]})
        shared = [f for f in files if fc.mod_of(f) == "_shared_src"]
        mods = sorted({fc.mod_of(f) for f in files if fc.mod_of(f)})
        build = sorted({m for m in mods if m != "_shared_src" and m.endswith("_src")}
                       | set(compat_mods(shared, use_compat)))
        base_fail: dict[str, dict] = {}
        for it in b["items"]:
            bl = it["check"].get("baseline")
            if isinstance(bl, dict):
                for flt, v in bl.items():
                    fl = (v or {}).get("failing") if isinstance(v, dict) else None
                    cur = base_fail.setdefault(flt, {"failing": []})
                    for x in fl or []:
                        if x not in cur["failing"]:
                            cur["failing"].append(x)
        for v in base_fail.values():
            v["failing"].sort()
        bid = f"batch_{i:02d}"
        plan.append({"batch": bid, "category": b["category"], "mods": mods, "files": files,
                     "tasks": [{"task_id": it["task_id"], "attempt": it["attempt"], "patch": it["patch"]}
                               for it in b["items"]],
                     "gates": {"build": build, "tests": test_filters(files), "compat": shared},
                     "baseline": dict(sorted(base_fail.items())),
                     "wrapper": str(fc.FIX_HOME / "bin" / f"fixgate_{bid}.sh"),
                     "_items": b["items"]})
    return plan


# ---------------------------------------------------------------- dossier
def _fence(text: str) -> str:
    return "````diff\n" + text.rstrip("\n") + "\n````\n"


def dossier(b: dict) -> str:
    L = [f"# Integration batch {b['batch']} ({b['category']})", "",
         f"FIXGATE = {b['wrapper']}",
         "Run it as `" + b["wrapper"] + " <subcommand> [args]` (the only Bash command you may run).",
         "Patch files are placed in your worktree before you start: `_patches/<task_id>.patch` "
         "(use `FIXGATE apply _patches/<task_id>.patch`). Save resolved diffs as `_resolved/<task_id>.patch`.", "",
         "## Gates for this batch",
         "- build (`FIXGATE build <mod_src>`): " + (", ".join(b["gates"]["build"]) or "(none)"),
         "- test (`FIXGATE test <filter>`): " + (", ".join(b["gates"]["tests"]) or "(none)"),
         "- compat (`FIXGATE compat <file>...`): " + (", ".join(b["gates"]["compat"]) or "(none)"),
         "- then `FIXGATE diffcheck`", "",
         "## Baseline failures (pre-existing on the clean snapshot; not caused by this batch)"]
    if b["baseline"]:
        for flt, v in b["baseline"].items():
            L.append(f"- filter `{flt}`: " + (", ".join(v["failing"]) or "(none failing)"))
    else:
        L.append("- (none recorded)")
    L += ["", f"## Patches in apply order ({len(b['tasks'])})", ""]
    for n, it in enumerate(b["_items"], 1):
        t, rv, ck = it["task"], it["review"], it["check"]
        out = rv["output"]
        L += [f"### {n}. {it['task_id']} (attempt {it['attempt']}; {t.get('specialty')}; class {t.get('class')})",
              "files: " + ", ".join(it["files"]),
              "patch file: `_patches/" + it["task_id"] + ".patch`", "", "Findings addressed:"]
        for f in t.get("findings") or []:
            L.append(f"- `{f.get('id')}` [{f.get('severity')}] {f.get('file')}:{f.get('lines')} - "
                     + _short(f.get("claim"), CLAIM_MAX))
        flags = [k for k, v in sorted((t.get("flags") or {}).items()) if v]
        ro = (it["result"] or {}).get("output") or {}
        if ro.get("needs_live_check"):
            flags.append("fixer:needs_live_check")
        L += ["", "Reviewer: " + out.get("verdict", "?") + (f" (flags: {', '.join(flags)})" if flags else "")]
        for iss in out.get("issues") or []:
            L.append(f"- [{iss.get('severity')}] {_short(iss.get('description'), 500)}")
        for pf in out.get("per_finding") or []:
            L.append(f"- finding {pf.get('finding_id')}: defect_fixed={pf.get('defect_fixed')} - "
                     + _short(pf.get("why"), 300))
        gates = ck.get("gates") or {}
        L += ["", "Checks: " + ("ok" if ck.get("ok") else "FAILED")
              + (f"; new_failures={ck.get('new_failures')}" if ck.get("new_failures") else "")]
        for g in sorted(gates):
            v = gates[g] if isinstance(gates[g], dict) else {}
            L.append(f"- {g}: {v.get('status')}" + (f" - {_short(v.get('detail'), 200)}" if v.get("detail") else ""))
        L += ["", "Patch:", _fence(it["text"])]
    return "\n".join(L).rstrip("\n") + "\n"


def public(b: dict) -> dict:
    return {k: v for k, v in b.items() if not k.startswith("_")}


def integ_outputs_exist() -> bool:
    return fc.INTEG.is_dir() and any(fc.INTEG.glob("batch_*"))


def existing_plan() -> dict:
    out = {}
    for p in sorted(fc.BATCHES.glob("batch_*.json")) if fc.BATCHES.is_dir() else []:
        d = fc.rj(p)
        if isinstance(d, dict):
            out[p.stem] = [(t["task_id"], t["attempt"]) for t in d.get("tasks") or []]
    return out


def cmd_build(argv: list[str]) -> int:
    dry = "--dry-run" in argv
    force = "--force" in argv
    use_compat = "--no-compat-list" not in argv
    queue = fc.load_queue()
    items, why = load_eligible(queue)
    plan = plan_batches(items, use_compat)
    new = {b["batch"]: [(t["task_id"], t["attempt"]) for t in b["tasks"]] for b in plan}
    print(f"build_integration_batches: {len(items)} accepted+clean patch(es) of {len(queue['tasks'])} task(s) "
          f"({why['not_accepted']} not accepted/clean, {why['calibration']} calibration excluded) -> {len(plan)} batch(es)")
    for b in plan:
        g = b["gates"]
        print(f"  {b['batch']}: {b['category']:<28} {len(b['tasks'])} patch(es) build={','.join(g['build']) or '-'} "
              f"tests={','.join(g['tests']) or '-'} compat={len(g['compat'])} file(s)")
    if dry:
        return 0
    old = existing_plan()
    if old and old != new and integ_outputs_exist() and not force:
        print("refusing: integration outputs exist and the batch plan would renumber/change them "
              f"(existing {len(old)} batch(es)); re-run with --force to overwrite.", file=sys.stderr)
        return 3
    if fc.BATCHES.is_dir():
        for p in list(fc.BATCHES.glob("batch_*.json")) + list(fc.BATCHES.glob("batch_*.md")):
            if p.stem not in new:
                p.unlink()
    fc.BATCHES.mkdir(parents=True, exist_ok=True)
    for b in plan:
        fc.write_json(fc.BATCHES / f"{b['batch']}.json", public(b))
        (fc.BATCHES / f"{b['batch']}.md").write_text(dossier(b), encoding="utf-8")
    print(f"wrote {len(plan)} batch(es) to {fc.BATCHES}")
    return 0


# ---------------------------------------------------------------- pending / publish
def batch_ids() -> list[str]:
    return [p.stem for p in sorted(fc.BATCHES.glob("batch_*.json"))] if fc.BATCHES.is_dir() else []


def run_ok(b: str) -> bool:
    r = fc.rj(fc.INTEG / f"{b}.run.json")
    return isinstance(r, dict) and r.get("status") == "ok" and r.get("schema_valid") is not False \
        and isinstance(r.get("output"), dict)


def verify_rec(b: str):
    return fc.rj(fc.INTEG / f"{b}.verify.json")


def cmd_pending(argv: list[str]) -> int:
    """pending: no ok run (full run). 'verify-only' marker for ok run without a verify record."""
    for b in batch_ids():
        if "--all" in argv:
            print(b)
        elif not run_ok(b) or verify_rec(b) is None:      # no ok run, or ok run still to be verified
            print(b)
    return 0


def final_series(b: str) -> tuple[list[dict], dict, list[str]]:
    """(kept entries in apply order, run output, errors). Entry = {task_id, attempt, action, patch, reason}."""
    errs: list[str] = []
    bj = fc.rj(fc.BATCHES / f"{b}.json") or {}
    byid = {t["task_id"]: t for t in bj.get("tasks") or []}
    run = fc.rj(fc.INTEG / f"{b}.run.json") or {}
    out = run.get("output") if isinstance(run.get("output"), dict) else {}
    ver = verify_rec(b) or {}
    ser = out.get("series") or []
    if isinstance(ver.get("series"), list) and ver["series"] and all(isinstance(x, dict) and "task_id" in x for x in ver["series"]):
        vs = {x["task_id"]: x for x in ver["series"]}
        merged = []
        for x in ser:
            y = dict(x)
            if x["task_id"] in vs and vs[x["task_id"]].get("action") in ("keep", "drop", "modified"):
                y["action"] = vs[x["task_id"]]["action"]
            merged.append(y)
        ser = merged
    kept = []
    home = (fc.INTEG_HOME / b).resolve()
    for x in ser:
        tid = x.get("task_id")
        if tid not in byid:
            errs.append(f"{tid}: not in batch")
            continue
        if x.get("action") == "drop":
            continue
        if x.get("action") == "modified":
            rp = str(x.get("resolved_patch") or "")
            cand = (home / rp).resolve() if rp and not rp.startswith("/") else Path(rp or "/nonexistent").resolve()
            if home not in cand.parents or ".." in rp.split("/") or not cand.is_file():
                errs.append(f"{tid}: modified without a readable resolved patch inside the worktree ({rp!r})")
                continue
            src = cand
        else:
            src = Path(byid[tid]["patch"])
        kept.append({"task_id": tid, "attempt": x.get("attempt", byid[tid]["attempt"]), "action": x.get("action"),
                     "reason": x.get("reason", ""), "src": str(src)})
    return kept, out, errs


def cmd_publish(argv: list[str]) -> int:
    if len(argv) != 1 or not re.fullmatch(r"batch_[0-9]{2}", argv[0]):
        print("usage: publish <batch_NN>", file=sys.stderr)
        return 2
    b = argv[0]
    ver = verify_rec(b)
    if not isinstance(ver, dict) or ver.get("ok") is not True:
        print(f"publish {b}: verify-series not ok; nothing published", file=sys.stderr)
        return 1
    kept, out, errs = final_series(b)
    if errs:
        print(f"publish {b}: " + "; ".join(errs), file=sys.stderr)
        return 1
    dest = fc.OUT / b
    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)
    rows = []
    for i, k in enumerate(kept, 1):
        name = f"{i:04d}-{k['task_id']}.patch"
        data = Path(k["src"]).read_bytes()
        (dest / name).write_bytes(data)
        rows.append({"index": i, "task_id": k["task_id"], "attempt": k["attempt"], "action": k["action"],
                     "file": name, "reason": k["reason"], "sha256": fc.sha_hex(data)})
    dropped = [{"task_id": x.get("task_id"), "reason": x.get("reason", "")}
               for x in (out.get("series") or []) if x.get("action") == "drop"]
    fc.write_json(dest / "series.json", {"batch": b, "series": rows, "dropped": dropped,
                                         "gates_run": out.get("gates_run") or [], "notes": out.get("notes", ""),
                                         "verify_ok": True})
    print(f"publish {b}: {len(rows)} patch(es) -> {dest}")
    return 0


# ---------------------------------------------------------------- self-test
def _mk_task(i, primary, order, cls="auto-fixable", spec="python-tooling"):
    tid = "T%010x" % (0xabc000 + i)
    return {"task_id": tid, "class": cls, "specialty": spec, "primary_file": primary, "allowed_files": [primary],
            "order": order, "mod": fc.mod_of(primary) or "", "flags": {},
            "findings": [{"id": "f%011x" % i, "file": primary, "lines": "1-2", "severity": "low",
                          "claim": f"claim {i}"}]}


def self_test() -> int:
    bad: list[str] = []
    tmp = Path(tempfile.mkdtemp(prefix="bib_selftest_"))
    try:
        base = tmp / "rh" / "fix" / "base"
        (base / "scripts").mkdir(parents=True)
        (base / "scripts" / "test_foo.py").write_text("x")
        fd = tmp / "fd"
        specs = [("patch_manager_mods/_shared_src/GuiKit.h", 1), ("patch_manager_mods/_shared_src/GuiKit.h", 2),
                 ("patch_manager_mods/zeta_src/Z.cpp", 3)]
        specs += [("patch_manager_mods/alpha_src/A.cpp", 10 + k) for k in range(10)]        # >8: split
        specs += [("scripts/foo.py", 30), ("scripts/bar.py", 31), ("tests/test_q.py", 40), ("README.md", 50),
                  ("scripts/rejected.py", 60), ("scripts/calib.py", 61), ("scripts/failcheck.py", 62)]
        tasks = [_mk_task(i, f, o) for i, (f, o) in enumerate(specs)]
        tasks.append(tasks.pop(0))                           # shuffle queue file order; `order` decides
        (fd / "work").mkdir(parents=True)
        (fd / "work" / "fix_queue.json").write_text(json.dumps({"tasks": tasks}))
        for d in ("results", "patches", "checks", "reviews", "integ"):
            (fd / d).mkdir()
        calib_tid = [t for t in tasks if t["primary_file"] == "scripts/calib.py"][0]["task_id"]
        (fd / "work" / "calib_key.json").write_text(json.dumps({"items": {calib_tid: {"kind": "seeded-bad"}}}))
        for t in tasks:
            tid, f = t["task_id"], t["primary_file"]
            patch = f"--- a/{f}\n+++ b/{f}\n@@ -1 +1 @@\n-a\n+b\n"
            (fd / "patches" / f"{tid}.a1.patch").write_text(patch)
            (fd / "patches" / f"{tid}.a1.scope.json").write_text(json.dumps({"ok": True, "empty": False, "changed_files": [f]}))
            ok = f != "scripts/failcheck.py"
            (fd / "checks" / f"{tid}.a1.check.json").write_text(json.dumps(
                {"ok": ok, "gates": {"apply": {"status": "pass", "detail": ""}},
                 "baseline": {"scripts/test_foo.py": {"failing": ["t_a"]}}}))
            verdict = "reject" if f == "scripts/rejected.py" else "accept"
            (fd / "reviews" / f"{tid}.a1.review.json").write_text(json.dumps(
                {"status": "ok", "schema_valid": True, "output": {"verdict": verdict, "issues": [], "per_finding": []}}))
        env = dict(os.environ, FIX_DIR=str(fd), FIX_WORK=str(fd / "work"), KOTOR2_REVIEW_HOME=str(tmp / "rh"),
                   KOTOR2_FIX_OUT=str(tmp / "out"), KOTOR2_FIX_REPORT=str(tmp / "rep"),
                   FIX_COMPAT_LIST_CMD="echo BUILD build_check --mod beta_src --mod gamma_src --mod x/y/z")
        me = [sys.executable, "-I", str(Path(__file__).resolve())]
        r = subprocess.run(me + ["--dry-run"], env=env, capture_output=True, text=True)
        if r.returncode != 0 or "batch_01" not in r.stdout:
            bad.append("dry-run: " + r.stdout + r.stderr)
        if (fd / "work" / "batches").exists():
            bad.append("dry-run wrote files")
        r = subprocess.run(me, env=env, capture_output=True, text=True)
        if r.returncode != 0:
            bad.append("build: " + r.stdout + r.stderr)
        bd = fd / "work" / "batches"
        bs = [json.loads(p.read_text()) for p in sorted(bd.glob("batch_*.json"))]
        cats = [b["category"] for b in bs]
        if cats != ["_shared_src", "alpha_src", "alpha_src", "zeta_src", "scripts", "tests", "other"]:
            bad.append(f"category order: {cats}")
        alltids = [t["task_id"] for b in bs for t in b["tasks"]]
        if calib_tid in alltids:
            bad.append("calibration task batched")
        if any(t["primary_file"] in ("scripts/rejected.py", "scripts/failcheck.py")
               and t["task_id"] in alltids for t in tasks):
            bad.append("rejected / failed-check task batched")
        if any(len(b["tasks"]) > MAX_PER_BATCH for b in bs):
            bad.append("batch > 8")
        if bs and bs[0]["gates"] != {"build": ["beta_src", "gamma_src"], "tests": ["_shared_src"],
                                     "compat": ["patch_manager_mods/_shared_src/GuiKit.h"]}:
            bad.append(f"shared gates {bs[0]['gates']}")
        sc = [b for b in bs if b["category"] == "scripts"]
        if not sc or sc[0]["gates"]["tests"] != ["scripts/test_foo.py"] or "scripts/test_foo.py" not in sc[0]["baseline"]:
            bad.append(f"scripts gates {sc and sc[0]['gates']}")
        md = (bd / "batch_01.md").read_text()
        if "fixgate_batch_01.sh" not in md or "Baseline failures" not in md or "claim " not in md:
            bad.append("dossier content")
        first = {p.name: p.read_bytes() for p in bd.iterdir()}
        subprocess.run(me, env=env, capture_output=True, text=True)
        if first != {p.name: p.read_bytes() for p in bd.iterdir()}:
            bad.append("not deterministic")
        # renumber refusal
        (fd / "integ" / "batch_01.run.json").write_text(json.dumps({"status": "ok", "output": {}}))
        # remove one accepted review -> plan changes
        victim = [t for t in tasks if t["primary_file"] == "scripts/bar.py"][0]["task_id"]
        (fd / "reviews" / f"{victim}.a1.review.json").unlink()
        r = subprocess.run(me, env=env, capture_output=True, text=True)
        if r.returncode != 3:
            bad.append(f"renumber not refused (rc {r.returncode})")
        r = subprocess.run(me + ["--force"], env=env, capture_output=True, text=True)
        if r.returncode != 0:
            bad.append("--force failed")
        r = subprocess.run(me + ["pending"], env=env, capture_output=True, text=True)
        if "batch_01" not in r.stdout.split():
            bad.append("ok run without verify must stay pending")
        (fd / "integ" / "batch_01.verify.json").write_text(json.dumps({"ok": False}))
        r = subprocess.run(me + ["pending"], env=env, capture_output=True, text=True)
        n_pending = len(r.stdout.split())
        if n_pending != len(list(bd.glob("batch_*.json"))) - 1:
            bad.append(f"pending count {n_pending}")
        # publish: needs verify ok
        r = subprocess.run(me + ["publish", "batch_02"], env=env, capture_output=True, text=True)
        if r.returncode != 1:
            bad.append("publish without verify must fail")
        b2 = json.loads((bd / "batch_02.json").read_text())
        t0, t1 = b2["tasks"][0]["task_id"], b2["tasks"][1]["task_id"]
        t2 = b2["tasks"][2]["task_id"]
        res_dir = tmp / "rh" / "fix" / "integ" / "batch_02" / "_resolved"
        res_dir.mkdir(parents=True)
        (res_dir / f"{t1}.patch").write_text("--- a/x\n+++ b/x\n@@ -1 +1 @@\n-a\n+c\n")
        (fd / "integ" / "batch_02.run.json").write_text(json.dumps({"status": "ok", "schema_valid": True, "output": {
            "batch": "batch_02", "gates_run": [], "notes": "",
            "series": [{"task_id": t1, "attempt": 1, "action": "modified", "reason": "r", "resolved_patch": f"_resolved/{t1}.patch"},
                       {"task_id": t0, "attempt": 1, "action": "keep", "reason": "k"},
                       {"task_id": t2, "attempt": 1, "action": "drop", "reason": "d"}]}}))
        (fd / "integ" / "batch_02.verify.json").write_text(json.dumps({"ok": True}))
        r = subprocess.run(me + ["publish", "batch_02"], env=env, capture_output=True, text=True)
        sj = tmp / "out" / "batch_02" / "series.json"
        if r.returncode != 0 or not sj.is_file():
            bad.append("publish failed: " + r.stdout + r.stderr)
        else:
            s = json.loads(sj.read_text())
            if [x["file"] for x in s["series"]] != [f"0001-{t1}.patch", f"0002-{t0}.patch"] or len(s["dropped"]) != 1:
                bad.append(f"series.json {s}")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    print("build_integration_batches self-test:", "PASS" if not bad else "FAIL " + " | ".join(bad))
    return 1 if bad else 0


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return self_test()
    if argv[:1] == ["pending"]:
        return cmd_pending(argv[1:])
    if argv[:1] == ["publish"]:
        return cmd_publish(argv[1:])
    if argv[:1] in (["-h"], ["--help"]):
        print(__doc__)
        return 0
    bad = [a for a in argv if a not in ("--dry-run", "--force", "--no-compat-list")]
    if bad:
        print(f"unknown arg {bad[0]}", file=sys.stderr)
        return 2
    return cmd_build(argv)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
