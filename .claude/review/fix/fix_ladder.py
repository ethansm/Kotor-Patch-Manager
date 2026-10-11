#!/usr/bin/env python3
"""Haiku-first ladder helper for the Fix stage (no LLM calls). A VARIANT is a derived copy of a real
needs-design task (new id, `style` field) that lives only in $WORK/ladder_ws/fix_queue.json, so the existing
runners can fix / check / review variants unchanged with FIX_WORK=$WORK/ladder_ws.

  fix_ladder.py sample --class needs-design --n 9 [--exclude ID,ID] [--seed ladder1]
                                          deterministic pick, round-robin across specialties; prints ids comma-joined
  fix_ladder.py variants --only TID,TID [--styles minimal,defensive,test-first]
                                          write $WORK/ladder/variants.json + $WORK/ladder_ws/fix_queue.json
                                          (idempotent: re-running accumulates the union of variants)
  fix_ladder.py status [--only PARENT,..] [--json]
                                          per parent -> per variant: latest attempt, outcome, review verdict
  fix_ladder.py promote PARENT            copy the judge's pick (latest attempt) to PARENT as attempt 1
                                          (needs $FIX_DIR/judge/<parent>.json pick_vtid + accept review)
  fix_ladder.py judge-queue | judge-prompt | judge-copy | record-judge | judge-calib build|list|score | judge-spent
                                          blind comparative judge of check-ok variants (see the `# ---- judge` section;
                                          driven by run_fix_judge.sh)
  fix_ladder.py --self-test

Variant id: vtid(parent, style) = "T" + sha256("variant:<parent>:<style>")[:10] (matches fc.TASK_ID_RE).
A variant's `parent` field is never rendered into any prompt (fix_prompts.py self-test checks that).
"""
from __future__ import annotations

import copy
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402
import record_fix as rf  # noqa: E402

REAL_WORK = fc.WORK.parent if fc.WORK.name == "ladder_ws" else fc.WORK   # same ladder state from either FIX_WORK
LADDER = REAL_WORK / "ladder"
LADDER_WS = REAL_WORK / "ladder_ws"           # FIX_WORK for variant runs: fix_queue.json = real tasks + variants
VARIANTS = LADDER / "variants.json"
JUDGE = fc.FIX_DIR / "judge"
STYLES = ["minimal", "defensive", "test-first"]
STYLE_RE = re.compile(r"^[a-z][a-z0-9-]{0,30}$")


def vtid(tid: str, style: str) -> str:
    return "T" + hashlib.sha256(f"variant:{tid}:{style}".encode()).hexdigest()[:10]


def _csv(v: str | None) -> list[str]:
    return [x.strip() for x in (v or "").split(",") if x.strip()]


def _is_variant(t: dict) -> bool:
    return bool(t.get("style") or t.get("parent"))


def load_variants() -> list[dict]:
    d = fc.rj(VARIANTS)
    out = []
    for v in (d or {}).get("variants") or []:
        if isinstance(v, dict) and fc.TASK_ID_RE.match(str(v.get("task_id"))) and v.get("parent") and v.get("style"):
            out.append({"task_id": v["task_id"], "parent": v["parent"], "style": v["style"]})
    return out


# ---------------------------------------------------------------------------------------------------------
def sample(cls: str, n: int, exclude: set[str], seed: str) -> list[str]:
    by_spec: dict[str, list[str]] = {}
    for t in fc.load_queue()["tasks"]:
        if t.get("class") != cls or _is_variant(t) or t["task_id"] in exclude:
            continue
        by_spec.setdefault(str(t.get("specialty")), []).append(t["task_id"])
    for spec in by_spec:
        by_spec[spec].sort(key=lambda i: (fc.sha_hex(seed + i), i))
    out: list[str] = []
    specs = sorted(by_spec)
    i = 0
    while len(out) < n and any(by_spec[s] for s in specs):
        for s in specs:
            if len(out) >= n:
                break
            if len(by_spec[s]) > i:
                out.append(by_spec[s][i])
        i += 1
    return out


def make_variant_task(parent: dict, style: str) -> dict:
    v = copy.deepcopy(parent)
    v["task_id"] = vtid(parent["task_id"], style)
    v["parent"] = parent["task_id"]
    v["style"] = style
    if parent.get("class") != "needs-design":
        v["class"] = "needs-design"
        v["line_cap"] = fc.LINE_CAP["needs-design"]
    return v


def write_ladder_ws(variants: list[dict]) -> Path:
    """Real queue + variant tasks (same merge pattern as fix_calibrate.write_review_ws)."""
    real = {t["task_id"]: t for t in fc.load_queue()["tasks"] if not _is_variant(t)}
    extra = [make_variant_task(real[v["parent"]], v["style"]) for v in variants]
    q = dict(fc.load_queue())
    q["tasks"] = [t for t in q["tasks"] if not _is_variant(t)] + extra
    fc.write_json(LADDER_WS / "fix_queue.json", q)
    return LADDER_WS


def variants_cmd(only: list[str], styles: list[str]) -> int:
    real = {t["task_id"]: t for t in fc.load_queue()["tasks"]}
    if not only:
        print("fix_ladder: --only <tids> required", file=sys.stderr)
        return 2
    for s in styles:
        if not STYLE_RE.match(s) or not (fc.CODE_DIR / "specialty_notes" / "style" / f"{s}.md").is_file():
            print(f"fix_ladder: unknown style {s!r} (no specialty_notes/style/{s}.md)", file=sys.stderr)
            return 2
    for tid in only:
        if tid not in real:
            print(f"fix_ladder: unknown parent {tid}", file=sys.stderr)
            return 2
        if _is_variant(real[tid]):
            print(f"fix_ladder: {tid} is itself a variant", file=sys.stderr)
            return 2
    have = load_variants()
    ids = {v["task_id"] for v in have}
    for p in only:
        for s in styles:
            if vtid(p, s) not in ids:
                have.append({"task_id": vtid(p, s), "parent": p, "style": s})
                ids.add(vtid(p, s))
    have.sort(key=lambda v: (v["parent"], v["style"]))
    fc.write_json(VARIANTS, {"variants": have})
    write_ladder_ws(have)
    print(f"fix_ladder: {len(have)} variant(s) over {len({v['parent'] for v in have})} parent(s); queue {LADDER_WS / 'fix_queue.json'}")
    return 0


# ---------------------------------------------------------------------------------------------------------
def latest_attempt(tid: str) -> int:
    n = 0
    for a in (1, 2, 3, 4, 5):
        if (fc.RESULTS / f"{tid}.a{a}.json").exists():
            n = a
    return n


def status(only: list[str]) -> list[dict]:
    vs = load_variants()
    parents = sorted({v["parent"] for v in vs})
    if only:
        parents = [p for p in parents if p in only]
    rows = []
    for p in parents:
        row = {"parent": p, "variants": []}
        for v in [x for x in vs if x["parent"] == p]:
            n = latest_attempt(v["task_id"])
            rv = rf.review_ok(v["task_id"], n) if n else None
            row["variants"].append({"task_id": v["task_id"], "style": v["style"], "attempt": n,
                                    "outcome": rf.outcome(v["task_id"], n) if n else "none",
                                    "verdict": ((rv or {}).get("output") or {}).get("verdict")})
        pick = (fc.rj(JUDGE / f"{p}.json") or {}).get("pick_vtid")
        row["pick_vtid"] = pick
        rows.append(row)
    return rows


def status_cmd(only: list[str], as_json: bool) -> int:
    rows = status(only)
    if as_json:
        print(fc.dumps({"parents": rows}), end="")
        return 0
    print(f"{'parent':<12} {'variant':<12} {'style':<10} {'att':>3} {'outcome':<13} verdict")
    for r in rows:
        for v in r["variants"]:
            mark = " *" if r.get("pick_vtid") == v["task_id"] else ""
            print(f"{r['parent']:<12} {v['task_id']:<12} {v['style']:<10} {v['attempt']:>3} {v['outcome']:<13} {v['verdict'] or '-'}{mark}")
    return 0


# ---------------------------------------------------------------------------------------------------------
def _rewrite(o, subs: list[tuple[str, str]]):
    if isinstance(o, dict):
        return {_rewrite(k, subs): _rewrite(v, subs) for k, v in o.items()}
    if isinstance(o, list):
        return [_rewrite(x, subs) for x in o]
    if isinstance(o, str):
        for a, b in subs:
            o = o.replace(a, b)
    return o


def has_attempt(tid: str) -> bool:
    for d in (fc.RESULTS, fc.PATCHES, fc.CHECKS, fc.REVIEWS):
        if d.is_dir() and any(d.glob(f"{tid}.a*")):
            return True
    return False


def _rel(p: Path) -> str:
    try:
        return str(p.relative_to(fc.FIX_DIR))
    except ValueError:
        return str(p)


def promote(parent: str) -> int:
    real = {t["task_id"] for t in fc.load_queue()["tasks"]}
    if not fc.TASK_ID_RE.match(parent) or parent not in real:
        print(f"fix_ladder: unknown parent {parent}", file=sys.stderr)
        return 2
    if has_attempt(parent):
        print(f"fix_ladder: refuse: {parent} already has an attempt", file=sys.stderr)
        return 1
    pick = (fc.rj(JUDGE / f"{parent}.json") or {}).get("pick_vtid")
    mine = {v["task_id"]: v for v in load_variants() if v["parent"] == parent}
    if not pick or pick not in mine:
        print(f"fix_ladder: refuse: no valid pick_vtid in {JUDGE / (parent + '.json')}", file=sys.stderr)
        return 1
    n = latest_attempt(pick)
    if not n:
        print(f"fix_ladder: refuse: {pick} has no attempt", file=sys.stderr)
        return 1
    rv = rf.review_ok(pick, n)
    verdict = ((rv or {}).get("output") or {}).get("verdict")
    if verdict != "accept":
        print(f"fix_ladder: refuse: review verdict of {pick}.a{n} is {verdict!r}, need accept", file=sys.stderr)
        return 1
    subs = [(f"{pick}.a{n}", f"{parent}.a1"), (pick, parent)]
    copied = []
    for d in (fc.RESULTS, fc.PATCHES, fc.CHECKS, fc.REVIEWS):
        for src in sorted(d.glob(f"{pick}.a{n}.*")) if d.is_dir() else []:
            dst = d / (f"{parent}.a1" + src.name[len(f"{pick}.a{n}"):])
            if src.suffix == ".json":
                obj = fc.rj(src)
                if not isinstance(obj, dict):
                    print(f"fix_ladder: refuse: unreadable {src}", file=sys.stderr)
                    return 1
                obj = _rewrite(obj, subs)
                obj["task_id"] = parent
                if "attempt" in obj:
                    obj["attempt"] = 1
                obj["promoted_from"] = pick
                if "cost_usd" in obj:
                    # fcommon.spent() sums cost_usd over RESULTS/REVIEWS; keep the figure but zero it so spend is not double counted
                    obj["promoted_cost_usd"] = obj["cost_usd"]
                    obj["cost_usd"] = 0
                fc.write_json(dst, obj)
            else:
                dst.write_bytes(src.read_bytes())
            copied.append(_rel(dst))
    # fix_workspace.py reads the attempt's pre manifest from TASKS for the parent
    pre_src = fc.TASKS / f"{pick}.a{n}.pre.json"
    if pre_src.is_file():
        pre_dst = fc.TASKS / f"{parent}.a1.pre.json"
        pre = fc.rj(pre_src)
        if not isinstance(pre, dict):
            print(f"fix_ladder: refuse: unreadable {pre_src}", file=sys.stderr)
            return 1
        pre = _rewrite(pre, subs)
        if "task_id" in pre:
            pre["task_id"] = parent
        if "attempt" in pre:
            pre["attempt"] = 1
        fc.write_json(pre_dst, pre)
        copied.append(_rel(pre_dst))
    print(f"fix_ladder: promoted {pick}.a{n} -> {parent}.a1 ({len(copied)} file(s))")
    return 0


# ---- judge: blind comparative judging of ladder variants (no LLM calls here; run_fix_judge.sh drives `claude`)
#   fix_ladder.py judge-queue [--only P,..] [--json] [--all] [--no-write]
#   fix_ladder.py judge-prompt <parent> [--calib] [--system]   user prompt (or the system prompt) on stdout
#   fix_ladder.py judge-copy <parent> [--calib]                 pre-image context copy; prints copy path, settings path
#   fix_ladder.py record-judge <parent> <raw_out> [--calib] [--rc N] [--dur S]
#   fix_ladder.py judge-calib build [--force] | list [--all] | score
#   fix_ladder.py judge-spent
# Labels A,B,C are assigned by a shuffle seeded with the parent id; the label->vtid answer key lives in
# <real work>/ladder/judge_key/<parent>.json and is never rendered into any prompt. Variants whose patches are
# identical (after whitespace normalisation) count as ONE candidate (the earliest style).
import shutil  # noqa: E402
import fix_prompts as fp  # noqa: E402

JUDGE_SCHEMA = fc.CODE_DIR / "judge_schema.json"
JUDGE_SYSTEM = fc.CODE_DIR / "judge_prompt.md"
JUDGE_LABELS = "ABC"
MAX_CANDS = len(JUDGE_LABELS)
MAX_CAND_PATCH = 16 * 1024
CALIB_BAD_PREF = ["no-op-fix", "off-by-one", "swallow-error", "assert-delete", "unrelated-file"]


def _real_work() -> Path:
    """FIX_WORK may be <work>/ladder_ws (variant runners); ladder/ and calib_good/ always live in <work>."""
    return fc.WORK.parent if fc.WORK.name == "ladder_ws" else fc.WORK


_LROOT = _real_work() / "ladder"
JUDGE_KEYS = _LROOT / "judge_key"
CALIB_ITEMS = _LROOT / "judge_calib"
CALIB_KEY_J = _LROOT / "judge_calib_key.json"
CALIB_JUDGE = JUDGE / "calib"
CALIB_GOOD_DIR = _real_work() / "calib_good"


def _variants_any() -> list[dict]:
    d = fc.rj(_LROOT / "variants.json")
    out = []
    for v in (d or {}).get("variants") or []:
        if isinstance(v, dict) and fc.TASK_ID_RE.match(str(v.get("task_id"))) and v.get("parent") and v.get("style"):
            out.append({"task_id": v["task_id"], "parent": v["parent"], "style": v["style"]})
    return out


def blind_order(seed: str, names: list[str]) -> list[str]:
    return sorted(names, key=lambda n: (fc.sha_hex(f"judge-order:{seed}:{n}"), n))


def _patch_sha(text: str) -> str:
    return fc.sha_hex("\n".join(x.rstrip() for x in text.replace("\r\n", "\n").strip().splitlines()))


def candidates(parent: str) -> list[dict]:
    """Distinct check-ok candidates of a parent (a variant's LATEST attempt must be check-ok), in style order."""
    vs = sorted((v for v in _variants_any() if v["parent"] == parent),
                key=lambda v: (STYLES.index(v["style"]) if v["style"] in STYLES else len(STYLES), v["style"]))
    out, seen = [], set()
    for v in vs:
        tid = v["task_id"]
        n = latest_attempt(tid)
        if not n or rf.outcome(tid, n) != "check-ok":
            continue
        try:
            patch = (fc.PATCHES / f"{tid}.a{n}.patch").read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        if not patch.strip():
            continue
        h = _patch_sha(patch)
        if h in seen:
            continue
        seen.add(h)
        out.append({"vtid": tid, "style": v["style"], "attempt": n, "patch": patch, "sha": h,
                    "check": fc.rj(fc.CHECKS / f"{tid}.a{n}.check.json") or {},
                    "scope": fc.rj(fc.PATCHES / f"{tid}.a{n}.scope.json") or {}})
    return out


def _judge_done(parent: str, cls: str, cands: list[dict]) -> bool:
    rec = fc.rj(JUDGE / f"{parent}.json")
    if not isinstance(rec, dict):
        return False
    if cls == "auto":
        return bool(rec.get("pick_vtid"))
    if cls == "judge":
        return rec.get("status") == "ok" and not rec.get("auto") and \
            set(rec.get("candidates") or []) == {c["vtid"] for c in cands}
    return False


def judge_classify(only: list[str]) -> list[dict]:
    parents = sorted({v["parent"] for v in _variants_any()})
    if only:
        parents = [p for p in parents if p in only]
    rows = []
    for p in parents:
        cs = candidates(p)
        cls = "judge" if len(cs) >= 2 else ("auto" if len(cs) == 1 else "residue")
        rows.append({"parent": p, "class": cls, "candidates": [c["vtid"] for c in cs], "done": _judge_done(p, cls, cs)})
    return rows


def judge_queue_cmd(only: list[str], as_json: bool, show_all: bool, write: bool) -> int:
    rows = judge_classify(only)
    for r in rows if write else []:
        f = JUDGE / f"{r['parent']}.json"
        rec = fc.rj(f)
        auto_rec = isinstance(rec, dict) and bool(rec.get("auto"))
        if r["class"] == "auto" and (rec is None or auto_rec):
            if not (isinstance(rec, dict) and rec.get("pick_vtid") == r["candidates"][0]):
                fc.write_json(f, {"parent": r["parent"], "pick_vtid": r["candidates"][0], "auto": True})
            r["done"] = True
        elif r["class"] == "residue" and auto_rec:
            f.unlink()                                  # the auto pick no longer has a check-ok patch behind it
    if as_json:
        print(fc.dumps({"parents": rows}), end="")
        return 0
    ids = [r["parent"] for r in rows if r["class"] == "judge" and (show_all or not r["done"])]
    cnt = {k: sum(1 for r in rows if r["class"] == k) for k in ("judge", "auto", "residue")}
    print(f"fix_ladder: judge-queue judge={cnt['judge']} auto={cnt['auto']} residue={cnt['residue']}", file=sys.stderr)
    print(",".join(ids))
    return 0


# --- prompt rendering -------------------------------------------------------------------------------------
_GIT_HDR = re.compile(r"^(diff --git |index [0-9a-f]+\.\.[0-9a-f]+|new file mode|deleted file mode|old mode|new mode|similarity index)")


def _norm_patch(patch: str) -> str:
    """Drop git extended headers so hand-made, harness-made and seeded diffs look alike; cap the size."""
    txt = "\n".join(ln for ln in patch.splitlines() if not _GIT_HDR.match(ln)) + "\n"
    raw = txt.encode("utf-8")
    if len(raw) > MAX_CAND_PATCH:
        txt = raw[:MAX_CAND_PATCH].decode("utf-8", "replace") + "\n[...patch truncated]\n"
    return txt


def _patch_paths(patch: str) -> list[str]:
    return re.findall(r"^\+\+\+ b/(\S+)", patch, re.M)


def _gate_line(check: dict | None, scope: dict | None) -> str:
    sc, ck = scope or {}, check or {}
    size = f"+{sc.get('lines_added', '?')} -{sc.get('lines_removed', '?')} lines"
    files = ", ".join(x for x in (fc.safe_rel(f) for f in sc.get("changed_files") or []) if x)
    g = ck.get("gates") if isinstance(ck.get("gates"), dict) else {}
    gates = " ".join(f"{k}={(v or {}).get('status', '?') if isinstance(v, dict) else v}" for k, v in sorted(g.items())) or "ok"
    nf = ck.get("new_failures") or []
    return (f"size: {size}" + (f"; files: {files}" if files else "") + "\n"
            f"deterministic gates (every candidate passed them): {gates}; new test failures: {len(nf)}")


def _judge_root(item: str, calib: bool) -> Path:
    for c in (fc.TASKS / (f"judge_calib_{item}" if calib else f"judge_{item}"), fc.BASE, fc.TREE):
        if c.is_dir():
            return c
    return fc.BASE


def render_judge_prompt(task: dict, cands: list[tuple[str, str, str]], root: Path, scrub: list[str]) -> str:
    """cands = [(label, patch, gate_line)]. No ids, styles or key material may reach the text (scrubbed)."""
    allowed = [a for a in (fc.safe_rel(x) for x in task.get("allowed_files") or []) if a]
    head = ["# Patch judging", f"specialty: {task.get('specialty')}",
            f"line cap: at most {task.get('line_cap')} changed lines", "Files the patches may touch: " + ", ".join(allowed), "",
            f"{len(cands)} candidate patches for the same finding(s) follow. Your working directory holds the ORIGINAL "
            "(unpatched) files; each candidate is shown as a unified diff.", ""]
    parts = ["\n".join(head), fp.findings_section(task),
             fp.excerpts(fp.cited_ranges(task), root, fp.EXCERPT_REVIEW,
                         "Original source excerpts (your working directory; line numbers are real)")]
    for label, patch, gate in cands:
        parts.append(f"## Candidate {label}\n{gate}\n```diff\n{_norm_patch(patch).rstrip(chr(10))}\n```\n")
    parts.append("Return the structured output now. `pick` is " + ", ".join(l for l, _, _ in cands) + " or none.\n")
    text = "\n".join(parts)
    for s in sorted({x for x in scrub if x}, key=len, reverse=True):
        text = text.replace(s, "[id]")
    return text


def judge_prompt(item: str, calib: bool = False) -> str:
    if calib:
        d = fc.rj(CALIB_ITEMS / f"{item}.json")
        key = calib_items().get(item)
        if not isinstance(d, dict) or not isinstance(key, dict):
            sys.exit(f"fix_ladder: unknown calibration item {item} (run judge-calib build)")
        by = {c["name"]: c for c in d["candidates"]}
        gates = {"gates": {g: {"status": "pass"} for g in ("apply", "scope", "excluded", "binary", "assertion_delta",
                                                           "skip_scan", "build", "tests", "compat")}}

        def gate_for(patch):
            ls = patch.splitlines()
            return _gate_line(gates, {"lines_added": sum(1 for x in ls if x.startswith("+") and not x.startswith("+++")),
                                      "lines_removed": sum(1 for x in ls if x.startswith("-") and not x.startswith("---")),
                                      "changed_files": _patch_paths(patch)})
        cands = [(l, by[n]["patch"], gate_for(by[n]["patch"])) for l, n in sorted(key["labels"].items())]
        return render_judge_prompt(fp.task_any(d["source_task"]), cands, _judge_root(item, True), [item, d["source_task"]])
    task = fp.task_any(item)
    cs = candidates(item)
    if len(cs) < 2:
        sys.exit(f"fix_ladder: {item} has {len(cs)} check-ok candidate(s); nothing to judge")
    by = {c["vtid"]: c for c in cs}
    order = blind_order(item, list(by))[:MAX_CANDS]
    labels = {JUDGE_LABELS[i]: v for i, v in enumerate(order)}
    fc.write_json(JUDGE_KEYS / f"{item}.json", {"parent": item, "labels": labels,
                                                "patch_sha": {l: by[v]["sha"] for l, v in labels.items()},
                                                "attempts": {l: by[v]["attempt"] for l, v in labels.items()}})
    cands = [(l, by[v]["patch"], _gate_line(by[v]["check"], by[v]["scope"])) for l, v in sorted(labels.items())]
    return render_judge_prompt(task, cands, _judge_root(item, False), [item] + [c["vtid"] for c in cs])


def judge_system() -> str:
    return JUDGE_SYSTEM.read_text(encoding="utf-8")


# --- read-only context copy (ORIGINAL files only: no candidate's post-image is visible) -----------------------
def judge_copy(item: str, calib: bool = False) -> int:
    import fix_workspace as fw  # type: ignore
    import fix_settings as fs  # type: ignore
    fw.guard_home()
    rels: set[str] = set()
    if calib:
        d = fc.rj(CALIB_ITEMS / f"{item}.json")
        if not isinstance(d, dict):
            sys.exit(f"fix_ladder: unknown calibration item {item}")
        task = fp.task_any(d["source_task"])
        patches = [c["patch"] for c in d["candidates"]]
        rels |= set(task.get("allowed_files") or [])
    else:
        task = fp.task_any(item)
        patches = []
        for c in candidates(item):
            patches.append(c["patch"])
            pre = fc.rj(fc.TASKS / f"{c['vtid']}.a{c['attempt']}.pre.json")
            rels |= set(pre) if isinstance(pre, dict) else set(task.get("allowed_files") or [])
    for p in patches:
        rels |= set(_patch_paths(p))
    dest = fc.TASKS / (f"judge_calib_{item}" if calib else f"judge_{item}")
    fw.rmtree(dest)
    dest.mkdir(parents=True)
    n = 0
    for rel in sorted(rels):
        r = fc.safe_rel(rel)
        src = fc.BASE / r if r else None
        if not r or fc.is_excluded(r) or src is None or not src.is_file() or fw.is_binary(src):
            continue
        (dest / r).parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dest / r)
        n += 1
    if not n:
        sys.exit(f"fix_ladder: judge copy for {item} is empty (base {fc.BASE} lacks the files?)")
    fw.set_readonly(dest, set(), fw.exec_in_base)
    settings = fs.write_settings(f"judge_{'calib_' if calib else ''}{item}", [f"Read({fs.abs_rule(dest)}/**)"])
    print(dest)
    print(settings)
    return 0


# --- recording ----------------------------------------------------------------------------------------------
def record_judge(item: str, raw_out: str, calib: bool = False, rc: int = 0, dur: float = 0.0) -> dict:
    import record_run as rr  # type: ignore  (fcommon appended tools/ to sys.path)
    key = calib_items().get(item) if calib else fc.rj(JUDGE_KEYS / f"{item}.json")
    if not isinstance(key, dict) or not isinstance(key.get("labels"), dict):
        sys.exit(f"fix_ladder: no answer key for {item} (judge-prompt{' --calib' if calib else ''} first)")
    labels: dict = key["labels"]
    schema = json.loads(JUDGE_SCHEMA.read_text(encoding="utf-8"))
    text = Path(raw_out).read_text(errors="replace") if Path(raw_out).exists() else ""
    result, _turns = rr.parse_raw(text)
    out = rr.structured(result) if result else None
    errs = rr.validate(out, schema) if out is not None else ["no structured output"]
    pick, rank, per = None, [], []
    if isinstance(out, dict):
        pick = out.get("pick") if isinstance(out.get("pick"), str) else None
        if pick != "none" and pick not in labels:
            errs.append(f"pick {pick!r} is not a candidate label {sorted(labels)}")
            pick = None if pick not in labels else pick
        rank = [x for x in out.get("ranking") or [] if isinstance(x, str)]
        if any(x not in labels for x in rank) or len(set(rank)) != len(rank):
            errs.append("ranking has an unknown or repeated label")
        per = [x for x in out.get("per_candidate") or [] if isinstance(x, dict)]
        if any(x.get("label") not in labels for x in per):
            errs.append("per_candidate has an unknown label")
    is_error = bool((result or {}).get("is_error")) or rc != 0
    if out is None and (not result or is_error):
        status = "failed"
    elif errs:
        status = "invalid"
    else:
        status = "ok"
    rec = {"parent": item, "kind": "judge", "status": status, "auto": False, "pick": pick, "pick_vtid": None,
           "ranking_vtids": [], "per_candidate": [], "improve_notes": "",
           "candidates": [] if calib else sorted(v for v in labels.values()),
           "schema_errors": errs[:20], "exit_code": rc, "duration_s": float(dur),
           "cost_usd": (result or {}).get("total_cost_usd"), "num_turns": (result or {}).get("num_turns"),
           "tool_calls": rf.count_tool_calls(text),
           "model": next(iter((result or {}).get("modelUsage") or {}), None),
           "raw": os.path.relpath(Path(raw_out).resolve(), fc.FIX_DIR) if Path(raw_out).exists() else None}
    if isinstance(out, dict):
        rec["ranking_vtids"] = [labels[x] for x in rank if x in labels]
        rec["per_candidate"] = [{"label": x["label"], "vtid": labels[x["label"]], "defect_fixed": x.get("defect_fixed"),
                                 "risks": x.get("risks")} for x in per if x.get("label") in labels]
        rec["improve_notes"] = str(out.get("improve_notes", ""))
        rec["pick_vtid"] = labels[pick] if pick in labels else None
    if calib:
        rec["item"] = item
        rec["pick_name"] = labels.get(pick) if pick in labels else ("none" if pick == "none" else None)
        rec["pick_vtid"] = None
        rec["ranking_vtids"] = []
        rec["ranking_names"] = [labels[x] for x in rank if x in labels]
        for x in rec["per_candidate"]:
            x["name"], x["vtid"] = x["vtid"], None
        fc.write_json(CALIB_JUDGE / f"{item}.json", rec)
    else:
        fc.write_json(JUDGE / f"{item}.json", rec)
    return rec


def judge_spent() -> float:
    return round(fc.cost_sum(JUDGE, ".json") + fc.cost_sum(CALIB_JUDGE, ".json"), 6)


# --- judge calibration: {good, 2 seeded-bad} per known-good task + 1 all-bad set (expect `none`) ---------------
def _seeded_bad(task: dict, tree: Path, want: int) -> list[dict]:
    import fix_calibrate as fcal  # type: ignore
    out = []
    for mode in CALIB_BAD_PREF:
        if len(out) >= want:
            break
        try:                                             # mutators only READ `tree` and return new line lists
            mut = fcal.MUTATORS[mode](task, tree)
            patch = fcal.make_patch(mut) if mut else ""
            what = mut["what"] if mut else ""
        except Exception:                                # a mutator that trips on this source just does not apply
            continue
        if patch.strip():
            out.append({"name": mode, "kind": "bad", "patch": patch, "what": what})
    return out


def calib_items() -> dict:
    return (fc.rj(CALIB_KEY_J) or {}).get("items") or {}


def calib_build(force: bool) -> int:
    import fix_calibrate as fcal  # type: ignore
    if CALIB_KEY_J.exists() and not force:
        print(f"fix_ladder: judge calibration already built ({CALIB_KEY_J.name}); use --force to rebuild")
        return 0
    tree = fc.BASE
    if not tree.is_dir():
        print(f"fix_ladder: pinned base {tree} missing (fix_workspace.py build-base)", file=sys.stderr)
        return 1
    by_id = {t["task_id"]: t for t in fc.load_queue()["tasks"]}
    goods = [g for g in sorted(CALIB_GOOD_DIR.glob("*.patch")) if g.stem in by_id][:3] if CALIB_GOOD_DIR.is_dir() else []
    if not goods:
        print(f"fix_ladder: no known-good patches for queue tasks in {CALIB_GOOD_DIR}", file=sys.stderr)
        return 1
    items, keys = {}, {}

    def add(item, kind, src, cands, expected_name):
        order = blind_order(item, [c["name"] for c in cands])[:MAX_CANDS]
        labels = {JUDGE_LABELS[i]: n for i, n in enumerate(order)}
        items[item] = {"item": item, "kind": kind, "source_task": src, "candidates": [c for c in cands if c["name"] in order]}
        exp = next((l for l, n in labels.items() if n == expected_name), "none")
        keys[item] = {"kind": kind, "source_task": src, "labels": labels, "expected_label": exp, "expected_name": expected_name}

    used = set()
    for i, g in enumerate(goods, 1):
        t = by_id[g.stem]
        used.add(t["task_id"])
        bads = _seeded_bad(t, tree, 2)
        if not bads:
            print(f"fix_ladder: WARNING no seeded-bad applies to {t['task_id']}; skipped", file=sys.stderr)
            continue
        add(f"good{i}", "good-set", t["task_id"],
            [{"name": "good", "kind": "good", "patch": g.read_text(encoding="utf-8"), "what": "hand-written"}] + bads, "good")
    pool = [t for t in fcal.eligible_sources(tree) if t["task_id"] not in used] + [by_id[g.stem] for g in goods[::-1]]
    for t in pool:
        bads = _seeded_bad(t, tree, MAX_CANDS)
        if len(bads) >= 2:
            add("allbad", "all-bad", t["task_id"], bads, "none")
            break
    else:
        print("fix_ladder: WARNING no all-bad set could be built", file=sys.stderr)
    if not items:
        print("fix_ladder: no calibration items built", file=sys.stderr)
        return 1
    for item, d in items.items():
        fc.write_json(CALIB_ITEMS / f"{item}.json", d)
    fc.write_json(CALIB_KEY_J, {"items": keys})
    try:
        os.chmod(CALIB_KEY_J, 0o600)
    except OSError:
        pass
    print(f"fix_ladder: built {len(items)} judge calibration item(s): {','.join(sorted(items))}")
    return 0


def calib_list(show_all: bool) -> int:
    ids = []
    for item in sorted(calib_items()):
        rec = fc.rj(CALIB_JUDGE / f"{item}.json")
        if show_all or not (isinstance(rec, dict) and rec.get("status") == "ok"):
            ids.append(item)
    print(",".join(ids))
    return 0


def calib_score() -> dict:
    res, unjudged = {}, []
    c_ok = c_tot = n_ok = n_tot = 0
    for item, k in sorted(calib_items().items()):
        rec = fc.rj(CALIB_JUDGE / f"{item}.json")
        if not isinstance(rec, dict) or rec.get("status") != "ok":
            unjudged.append(item)
            continue
        ok = rec.get("pick") == k["expected_label"]
        res[item] = {"kind": k["kind"], "expected": k["expected_label"], "picked": rec.get("pick"),
                     "picked_name": rec.get("pick_name"), "correct": ok}
        if k["kind"] == "good-set":
            c_tot += 1
            c_ok += ok
        else:
            n_tot += 1
            n_ok += ok
    return {"pick_accuracy": f"{c_ok}/{c_tot}", "none_detection": f"{n_ok}/{n_tot}", "items": res, "unjudged": unjudged,
            "pass": bool(calib_items()) and not unjudged and c_ok == c_tot and n_ok == n_tot and c_tot > 0 and n_tot > 0}


def judge_main(cmd: str, rest: list[str], opt) -> int | None:
    """Dispatch for the judge subcommands; None when `cmd` is not one of them."""
    pos = [x for i, x in enumerate(rest) if not x.startswith("--") and (i == 0 or rest[i - 1] not in ("--only", "--rc", "--dur"))]
    if cmd == "judge-queue":
        return judge_queue_cmd(_csv(opt("--only")), "--json" in rest, "--all" in rest, "--no-write" not in rest)
    if cmd == "judge-prompt" and pos:
        sys.stdout.write(judge_system() if "--system" in rest else judge_prompt(pos[0], "--calib" in rest))
        return 0
    if cmd == "judge-copy" and pos:
        return judge_copy(pos[0], "--calib" in rest)
    if cmd == "record-judge" and len(pos) >= 2:
        rec = record_judge(pos[0], pos[1], "--calib" in rest, int(opt("--rc", "0")), float(opt("--dur", "0")))
        print(f"{pos[0]}: {rec['status']} pick={rec['pick']} cost={rec['cost_usd']} errs={len(rec['schema_errors'])}")
        return 0
    if cmd == "judge-spent":
        print(judge_spent())
        return 0
    if cmd == "judge-calib" and rest[:1] == ["build"]:
        return calib_build("--force" in rest)
    if cmd == "judge-calib" and rest[:1] == ["list"]:
        return calib_list("--all" in rest)
    if cmd == "judge-calib" and rest[:1] == ["score"]:
        r = calib_score()
        print(fc.dumps(r), end="")
        return 0 if r["pass"] else 1
    return None
# ---- end judge


# ---------------------------------------------------------------------------------------------------------
def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return self_test()
    if argv[:1] == ["--self-test-run"]:
        return self_test_run()
    if not argv:
        print(__doc__)
        return 2
    cmd, rest = argv[0], argv[1:]

    def opt(name, default=None):
        return rest[rest.index(name) + 1] if name in rest and rest.index(name) + 1 < len(rest) else default
    if cmd == "sample":
        ids = sample(opt("--class", "needs-design"), int(opt("--n", "9")), set(_csv(opt("--exclude"))), opt("--seed", "ladder1"))
        print(",".join(ids))
        return 0
    if cmd == "variants":
        return variants_cmd(_csv(opt("--only")), _csv(opt("--styles")) or STYLES)
    if cmd == "status":
        return status_cmd(_csv(opt("--only")), "--json" in rest)
    if cmd == "promote" and rest:
        return promote(rest[0])
    rc = judge_main(cmd, rest, opt)
    if rc is not None:
        return rc
    print(__doc__)
    return 2


def self_test() -> int:
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        env = dict(os.environ, FIX_DIR=str(Path(td) / "data"), FIX_WORK=str(Path(td) / "work"),
                   KOTOR2_REVIEW_HOME=str(Path(td) / "home"), FIX_SELFTEST_TMP=td)
        return subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), "--self-test-run"], env=env).returncode


def self_test_run() -> int:
    td = Path(os.environ["FIX_SELFTEST_TMP"])
    bad: list[str] = []

    def check(c, m):
        if not c:
            bad.append(m)

    assert fc.FIX_DIR == (td / "data").resolve() and fc.WORK == (td / "work").resolve(), "env override not applied"
    here = str(Path(__file__).resolve())

    def cli(*args):
        r = subprocess.run([sys.executable, "-I", here, *args], capture_output=True, text=True, env=dict(os.environ))
        return r.returncode, r.stdout, r.stderr

    check(fc.TASK_ID_RE.match(vtid("T0123456789", "minimal")) is not None, "vtid regex")
    check(vtid("T0123456789", "minimal") != vtid("T0123456789", "defensive") and
          vtid("T0123456789", "minimal") == vtid("T0123456789", "minimal"), "vtid distinct/stable")
    check(vtid("T0123456789", "minimal") == "T" + hashlib.sha256(b"variant:T0123456789:minimal").hexdigest()[:10], "vtid formula")

    specs = ["cpp-logic", "python-tooling", "gate-integrity"]
    tasks = []
    for i in range(12):
        tasks.append({"task_id": "T%010x" % (0x1000 + i), "class": "needs-design", "specialty": specs[i % 3],
                      "allowed_files": [f"scripts/f{i}.py"], "line_cap": 120, "findings": [{"id": "f%011x" % i}]})
    tasks.append({"task_id": "T%010x" % 0x2000, "class": "auto-fixable", "specialty": "cpp-logic", "line_cap": 40, "findings": []})
    fc.write_json(fc.QUEUE, {"tasks": tasks, "counts": {}})
    excl = tasks[0]["task_id"]
    rc, o1, _ = cli("sample", "--class", "needs-design", "--n", "6", "--exclude", excl)
    rc2, o2, _ = cli("sample", "--class", "needs-design", "--n", "6", "--exclude", excl)
    ids = o1.strip().split(",")
    check(rc == 0 and o1 == o2 and len(ids) == 6 and len(set(ids)) == 6, f"sample deterministic: {o1!r}")
    check(excl not in ids and "T%010x" % 0x2000 not in ids, "sample excludes id/class")
    by = {t["task_id"]: t["specialty"] for t in tasks}
    check(sorted(by[i] for i in ids[:3]) == sorted(specs), "sample round-robin over specialties")
    check(cli("sample", "--n", "6", "--seed", "other")[0] == 0, "seed accepted")

    P1, P2 = tasks[1]["task_id"], tasks[2]["task_id"]
    rc, _, _ = cli("variants", "--only", P1)
    check(rc == 0, "variants rc")
    vs = fc.rj(VARIANTS)["variants"]
    check(len(vs) == 3 and {v["style"] for v in vs} == set(STYLES) and all(v["parent"] == P1 for v in vs), f"variants.json {vs}")
    wq = fc.rj(LADDER_WS / "fix_queue.json")
    check(len(wq["tasks"]) == len(tasks) + 3, "ws queue merged")
    vt = [t for t in wq["tasks"] if t.get("parent") == P1]
    check(len(vt) == 3 and all(t["class"] == "needs-design" and t["allowed_files"] == tasks[1]["allowed_files"]
                               and t["task_id"] == vtid(P1, t["style"]) for t in vt), "variant task shape")
    check(len(fc.load_queue()["tasks"]) == len(tasks), "real queue untouched")
    cli("variants", "--only", P1)
    check(len(fc.rj(VARIANTS)["variants"]) == 3, "variants idempotent")
    cli("variants", "--only", P2, "--styles", "minimal")
    check(len(fc.rj(VARIANTS)["variants"]) == 4 and len(fc.rj(LADDER_WS / "fix_queue.json")["tasks"]) == len(tasks) + 4, "variants accumulate")
    check(cli("variants", "--only", "T0000000000")[0] != 0, "unknown parent refused")
    check(cli("variants", "--only", vt[0]["task_id"])[0] != 0, "variant-as-parent refused")
    check(cli("variants", "--only", P1, "--styles", "bogus")[0] != 0, "unknown style refused")

    # status on fake artifacts
    vm, vd = vtid(P1, "minimal"), vtid(P1, "defensive")
    for d in (fc.RESULTS, fc.PATCHES, fc.CHECKS, fc.REVIEWS):
        d.mkdir(parents=True, exist_ok=True)
    out = {"findings": [{"finding_id": "f0", "status": "fixed"}], "task_id": vm}
    fc.write_json(fc.RESULTS / f"{vm}.a1.json", {"status": "ok", "task_id": vm, "attempt": 1, "output": out, "cost_usd": 0.5})
    (fc.PATCHES / f"{vm}.a1.patch").write_text(f"--- a/x\n+++ b/x\n# {vm}\n")
    fc.write_json(fc.PATCHES / f"{vm}.a1.scope.json", {"ok": True, "empty": False, "task_id": vm})
    fc.write_json(fc.CHECKS / f"{vm}.a1.check.json", {"ok": True, "task_id": vm})
    fc.write_json(fc.RESULTS / f"{vd}.a1.json", {"status": "failed"})
    rc, so, _ = cli("status", "--json")
    st = json.loads(so)["parents"]
    byv = {v["task_id"]: v for r in st for v in r["variants"]}
    check(byv[vm]["attempt"] == 1 and byv[vm]["outcome"] == "check-ok" and byv[vm]["verdict"] is None, f"status minimal {byv[vm]}")
    check(byv[vd]["outcome"] == "error" and byv[vtid(P1, "test-first")]["attempt"] == 0, "status error/none")
    check("check-ok" in cli("status", "--only", P1)[1], "status table")

    # promote refusals and success
    check(cli("promote", P1)[0] != 0, "promote refuses without judge pick")
    fc.write_json(JUDGE / f"{P1}.json", {"pick_vtid": vm})
    check(cli("promote", P1)[0] != 0, "promote refuses without review")
    fc.write_json(fc.REVIEWS / f"{vm}.a1.review.json", {"status": "ok", "task_id": vm, "output": {"verdict": "revise"}})
    check(cli("promote", P1)[0] != 0, "promote refuses non-accept")
    fc.write_json(fc.REVIEWS / f"{vm}.a1.review.json", {"status": "ok", "task_id": vm, "attempt": 1, "output": {"verdict": "accept", "task_id": vm}, "cost_usd": 0.25})
    fc.write_json(fc.REVIEWS / f"{vm}.a1.quote.json", {"ref": f"{vm}.a1"})
    fc.write_json(fc.TASKS / f"{vm}.a1.pre.json", {"task_id": vm, "attempt": 1, "files": {"scripts/f0.py": "x"}})
    spent_fix0, spent_rev0 = fc.spent("fix"), fc.spent("review")
    rc, po, pe = cli("promote", P1)
    check(rc == 0, f"promote success: {pe}")
    check(fc.spent("fix") == spent_fix0 and fc.spent("review") == spent_rev0, "promote does not change spend")
    pre1 = fc.rj(fc.TASKS / f"{P1}.a1.pre.json")
    check(pre1 is not None and pre1.get("task_id") == P1 and pre1.get("attempt") == 1, f"pre manifest copied: {pre1}")
    r1c = fc.rj(fc.RESULTS / f"{P1}.a1.json") or {}
    check(r1c.get("cost_usd") == 0 and r1c.get("promoted_cost_usd") == 0.5, "promoted result cost zeroed")
    check(rf.outcome(P1, 1) == "check-ok" and (rf.review_ok(P1, 1) or {}).get("output", {}).get("verdict") == "accept", "promoted outcome/review")
    r1 = fc.rj(fc.RESULTS / f"{P1}.a1.json")
    check(r1["task_id"] == P1 and r1["output"]["task_id"] == P1 and vm not in json.dumps({k: v for k, v in fc.rj(fc.REVIEWS / f"{P1}.a1.review.json").items() if k != "promoted_from"}), "task_id rewritten")
    check((fc.PATCHES / f"{P1}.a1.patch").read_text() == (fc.PATCHES / f"{vm}.a1.patch").read_text(), "patch copied")
    check(fc.rj(fc.REVIEWS / f"{P1}.a1.quote.json").get("ref") == f"{P1}.a1", "quote file rewritten")
    check(cli("promote", P1)[0] != 0, "promote refuses when parent has an attempt")
    # ---- judge: queue classification, blind prompt, record, calibration (stub claude only for a raw-output sample)
    import fix_calibrate as fcal
    (fc.BASE / "scripts").mkdir(parents=True, exist_ok=True)
    for i in range(2, 8):
        (fc.BASE / f"scripts/f{i}.py").write_text(fcal.PY_FOO)
    (fc.BASE / "scripts/foo.py").write_text(fcal.PY_FOO)

    def jtask(i, rel, lines, claim):
        return {"task_id": "T%010x" % i, "class": "auto-fixable", "specialty": "python-tooling", "flags": {}, "primary_file": rel,
                "lines": lines, "allowed_files": [rel], "context_globs": [], "line_cap": 40, "wave": 1, "order": i,
                "findings": [{"id": "f%011x" % i, "file": rel, "lines": lines, "category": "logic", "specialty": "python-tooling",
                              "severity": "low", "claim": claim, "verifier_reasoning": "reasoning text", "evidence_quotes": [],
                              "location_check": []}]}
    jt = {i: jtask(0x1000 + i, f"scripts/f{i}.py", "8-9", f"CLAIM-MARKER-{i} loop bound is wrong") for i in range(2, 8)}
    cg = [jtask(0x3001, "scripts/foo.py", "5-6", "g1"), jtask(0x3002, "scripts/foo.py", "8-9", "g2"),
          jtask(0x3003, "scripts/foo.py", "5-5", "g3"), jtask(0x3004, "scripts/foo.py", "6-9", "g4")]
    q2 = [jt.get(int(t["task_id"][1:], 16) - 0x1000, t) if 0x1000 <= int(t["task_id"][1:], 16) < 0x1100 else t for t in tasks]
    fc.write_json(fc.QUEUE, {"tasks": q2 + cg, "counts": {}})
    P3, P4, P5, P6 = (tasks[i]["task_id"] for i in (3, 4, 5, 6))

    def mk_patch(path, new):
        return (f"diff --git a/{path} b/{path}\nindex 1111111..2222222 100644\n--- a/{path}\n+++ b/{path}\n"
                f"@@ -8,1 +8,1 @@\n-        if i < limit:\n+        {new}\n")

    def fake_ok(vt, path, new):
        fc.write_json(fc.RESULTS / f"{vt}.a1.json", {"status": "ok", "task_id": vt, "attempt": 1, "output": {
            "findings": [{"finding_id": "f", "status": "fixed"}], "task_id": vt}})
        (fc.PATCHES / f"{vt}.a1.patch").write_text(mk_patch(path, new))
        fc.write_json(fc.PATCHES / f"{vt}.a1.scope.json", {"ok": True, "empty": False, "changed_files": [path],
                                                            "lines_added": 1, "lines_removed": 1})
        fc.write_json(fc.CHECKS / f"{vt}.a1.check.json", {"ok": True, "new_failures": [], "gates": {
            g: {"status": "pass", "detail": f"/x/tasks/{vt}.a1/log"} for g in ("apply", "build", "tests")}})
    cli("variants", "--only", ",".join([P2, P3, P4, P5, P6]))
    for st, new in (("minimal", "if i < limit - 1:"), ("defensive", "if i <= limit:"), ("test-first", "if limit and i < limit:")):
        fake_ok(vtid(P2, st), "scripts/f2.py", new)
    fake_ok(vtid(P3, "defensive"), "scripts/f3.py", "if i < limit and limit > 0:")
    fc.write_json(fc.RESULTS / f"{vtid(P4, 'minimal')}.a1.json", {"status": "failed"})
    for st in ("minimal", "defensive"):
        fake_ok(vtid(P5, st), "scripts/f5.py", "if i < max(limit, 0):")      # identical patches -> one candidate
    fake_ok(vtid(P6, "minimal"), "scripts/f6.py", "if i < limit - 2:")
    fake_ok(vtid(P6, "test-first"), "scripts/f6.py", "if i < (limit or 0):")
    allp = [P2, P3, P4, P5, P6]
    rc, jq, _ = cli("judge-queue", "--only", ",".join(allp))
    check(rc == 0 and sorted(jq.strip().split(",")) == sorted([P2, P6]), f"judge-queue list {jq!r}")
    rows = {r["parent"]: r for r in json.loads(cli("judge-queue", "--only", ",".join(allp), "--json")[1])["parents"]}
    check({p: rows[p]["class"] for p in allp} == {P2: "judge", P3: "auto", P4: "residue", P5: "auto", P6: "judge"}, f"classes {rows}")
    check(len(rows[P2]["candidates"]) == 3 and rows[P5]["candidates"] == [vtid(P5, "minimal")], "candidates / dedupe")
    check(fc.rj(JUDGE / f"{P3}.json") == {"parent": P3, "pick_vtid": vtid(P3, "defensive"), "auto": True}, "auto pick written")
    check(fc.rj(JUDGE / f"{P5}.json", ) is not None and fc.rj(JUDGE / f"{P5}.json")["pick_vtid"] == vtid(P5, "minimal"), "dedupe auto pick")
    check(not (JUDGE / f"{P4}.json").exists() and not (JUDGE / f"{P2}.json").exists(), "no record for residue / judge")
    check(cli("judge-queue", "--only", P3)[1].strip() == "", "auto parent not in judge list")
    # prompt: blind
    rc, pr, pe = cli("judge-prompt", P2)
    check(rc == 0 and all(f"## Candidate {l}" in pr for l in "ABC") and "## Candidate D" not in pr, f"prompt headers {pe[-200:]}")
    low = pr.lower()
    needles = [vtid(P2, s) for s in STYLES] + [P2, "judge_key", "verify_key", "withheld", "parent", "minimal", "defensive", "test-first",
                                              "diff --git", "a1.log"]
    check(not [n for n in needles if n.lower() in low], f"prompt leaks {[n for n in needles if n.lower() in low]}")
    check("CLAIM-MARKER-2" in pr and "if i <= limit:" in pr and "scripts/f2.py" in pr, "prompt has finding + diffs")
    key = fc.rj(JUDGE_KEYS / f"{P2}.json")
    check(sorted(key["labels"]) == ["A", "B", "C"] and sorted(key["labels"].values()) == sorted(vtid(P2, s) for s in STYLES), f"key {key}")
    check(cli("judge-prompt", P2)[1] == pr and fc.rj(JUDGE_KEYS / f"{P2}.json") == key, "prompt/key deterministic")
    check(cli("judge-prompt", P5)[0] != 0 and cli("judge-prompt", "T0000000000")[0] != 0, "judge-prompt refuses <2 candidates / unknown")
    rc, sysp, _ = cli("judge-prompt", P2, "--system")
    check(rc == 0 and "Patch judge" in sysp and "none" in sysp, "system prompt")
    rc, co, ce = cli("judge-copy", P2)
    cp, sp = (co.strip().splitlines() + ["", ""])[:2]
    check(rc == 0 and (Path(cp) / "scripts/f2.py").read_text() == fcal.PY_FOO and Path(cp).name == f"judge_{P2}" and not (Path(cp) / "_pre").exists(), f"judge copy {ce[-200:]}")
    check(f"Read(/{cp}/**)" in json.dumps(fc.rj(Path(sp))["permissions"]["allow"]) or cp in json.dumps(fc.rj(Path(sp))), "judge settings allow the copy")

    def fake_raw(path, so, cost=0.01):
        ev = {"type": "result", "subtype": "success", "is_error": False, "num_turns": 2, "total_cost_usd": cost, "usage": {},
              "modelUsage": {"m": {}}, "structured_output": so, "result": json.dumps(so)}
        Path(path).write_text(json.dumps(ev) + "\n")

    def per(labels, fixed):
        return [{"label": l, "defect_fixed": "yes" if l == fixed else "no", "risks": []} for l in labels]
    # stub claude raw output (judge-pick-a): picks A
    pf = td / "judge_prompt.txt"
    pf.write_text(pr)
    sr = subprocess.run([sys.executable, str(fc.REVIEW / "tools" / "stub_claude" / "claude"), "-p", "--output-format", "stream-json",
                         "--model", "m", "--max-budget-usd", "0.08"], input=pr, capture_output=True, text=True, cwd=td,
                        env=dict(os.environ, STUB_MODE="judge-pick-a"))
    (td / "raw_stub.out").write_text(sr.stdout)
    rc, ro, re_ = cli("record-judge", P2, str(td / "raw_stub.out"))
    rec = fc.rj(JUDGE / f"{P2}.json") or {}
    check(rc == 0 and rec.get("status") == "ok" and rec.get("pick") == "A" and rec.get("pick_vtid") == key["labels"]["A"], f"record-judge {ro} {re_[-200:]} {rec}")
    check(rec.get("ranking_vtids") == [key["labels"][l] for l in "ABC"] and [x["vtid"] for x in rec["per_candidate"]] == [key["labels"][l] for l in "ABC"], "label->vtid mapping")
    check(abs(rec.get("cost_usd", 0) - 0.0123) < 1e-9 and judge_spent() > 0 and cli("judge-spent")[1].strip() != "0.0", "cost recorded")
    check(cli("judge-queue", "--only", ",".join(allp))[1].strip() == P6, "judged parent leaves the queue")
    check(sorted(cli("judge-queue", "--only", ",".join(allp), "--all")[1].strip().split(",")) == sorted([P2, P6]), "--all lists done too")
    # invalid label (C with only two candidates), none, garbage
    cli("judge-prompt", P6)
    k6 = fc.rj(JUDGE_KEYS / f"{P6}.json")
    fake_raw(td / "r_inv.out", {"pick": "C", "ranking": ["A", "B"], "per_candidate": per("AB", "A"), "improve_notes": ""})
    cli("record-judge", P6, str(td / "r_inv.out"))
    r6 = fc.rj(JUDGE / f"{P6}.json")
    check(r6["status"] == "invalid" and r6["pick_vtid"] is None and any("not a candidate label" in e for e in r6["schema_errors"]), f"invalid label {r6}")
    check(cli("judge-queue", "--only", P6)[1].strip() == P6, "invalid record keeps the parent queued")
    fake_raw(td / "r_none.out", {"pick": "none", "ranking": ["B", "A"], "per_candidate": per("AB", "x"), "improve_notes": "n"})
    cli("record-judge", P6, str(td / "r_none.out"))
    r6 = fc.rj(JUDGE / f"{P6}.json")
    check(r6["status"] == "ok" and r6["pick"] == "none" and r6["pick_vtid"] is None and r6["ranking_vtids"] == [k6["labels"]["B"], k6["labels"]["A"]], f"none pick {r6}")
    (td / "r_bad.out").write_text("garbage\n")
    cli("record-judge", P6, str(td / "r_bad.out"))
    check(fc.rj(JUDGE / f"{P6}.json")["status"] == "failed", "garbage -> failed")
    fake_raw(td / "r_schema.out", {"pick": "A", "ranking": ["A"], "per_candidate": [], "improve_notes": "", "extra": 1})
    cli("record-judge", P6, str(td / "r_schema.out"))
    check(fc.rj(JUDGE / f"{P6}.json")["status"] == "invalid", "extra property -> invalid")
    # FIX_WORK=<work>/ladder_ws sees the same ladder state
    r = subprocess.run([sys.executable, "-I", here, "judge-queue", "--only", ",".join(allp), "--json", "--no-write"],
                       capture_output=True, text=True, env=dict(os.environ, FIX_WORK=str(LADDER_WS)))
    check(r.returncode == 0 and {x["parent"]: x["class"] for x in json.loads(r.stdout)["parents"]} == {p: rows[p]["class"] for p in allp}, f"ladder_ws env {r.stderr[-200:]}")
    # judge calibration
    gd = td / "work" / "calib_good"
    gd.mkdir(parents=True, exist_ok=True)
    for t, new in (("T%010x" % 0x3001, "if i < limit + 1:"), ("T%010x" % 0x3002, "if i < max(limit, 0):"), ("T%010x" % 0x3003, "if i < int(limit):")):
        (gd / f"{t}.patch").write_text(mk_patch("scripts/foo.py", new))
    rc, bo, be = cli("judge-calib", "build")
    ck = fc.rj(CALIB_KEY_J) or {}
    its = ck.get("items") or {}
    check(rc == 0 and sorted(its) == ["allbad", "good1", "good2", "good3"], f"calib build {bo} {be[-300:]}")
    check(all(its[i]["expected_label"] in "ABC" and its[i]["labels"][its[i]["expected_label"]] == "good" for i in its if i != "allbad")
          and its["allbad"]["expected_label"] == "none" and "good" not in its["allbad"]["labels"].values(), "calib key")
    check(all(len(its[i]["labels"]) >= 2 for i in its) and len(its["good1"]["labels"]) == 3, f"calib candidate counts {[len(v['labels']) for v in its.values()]}")
    check(sorted(cli("judge-calib", "list")[1].strip().split(",")) == sorted(its), "calib list")
    for item in sorted(its):
        rc, cp_, ce_ = cli("judge-prompt", item, "--calib")
        lowc = cp_.lower()
        check(rc == 0 and "## Candidate A" in cp_ and "## Candidate B" in cp_, f"calib prompt {item} {ce_[-200:]}")
        check(not [n for n in ("good", "no-op-fix", "off-by-one", "swallow-error", item, its[item]["source_task"].lower(), "diff --git", "judge_key", "withheld", "verify_key") if n in lowc.replace("good", "") or (n == "good" and False)], f"calib prompt leaks {item}")
    check(cli("judge-copy", "good1", "--calib")[0] == 0 and (fc.TASKS / "judge_calib_good1" / "scripts/foo.py").is_file(), "calib copy")
    check(cli("judge-calib", "score")[0] == 1, "score fails while unjudged")
    for item, k in its.items():
        labs = sorted(k["labels"])
        fake_raw(td / f"c_{item}.out", {"pick": k["expected_label"], "ranking": labs, "per_candidate": per(labs, k["expected_label"]), "improve_notes": ""})
        rc, co2, _ = cli("record-judge", item, str(td / f"c_{item}.out"), "--calib")
        check(rc == 0 and fc.rj(CALIB_JUDGE / f"{item}.json")["status"] == "ok", f"calib record {item}")
    rc, so2, _ = cli("judge-calib", "score")
    sc2 = json.loads(so2)
    check(rc == 0 and sc2["pick_accuracy"] == "3/3" and sc2["none_detection"] == "1/1" and sc2["pass"] and not sc2["unjudged"], f"calib score {so2}")
    check(cli("judge-calib", "list")[1].strip() == "", "calib list empty when all judged")
    wrong = next(l for l in its["good2"]["labels"] if l != its["good2"]["expected_label"])
    fake_raw(td / "c_wrong.out", {"pick": wrong, "ranking": [wrong], "per_candidate": per(wrong, wrong), "improve_notes": ""})
    cli("record-judge", "good2", str(td / "c_wrong.out"), "--calib")
    fake_raw(td / "c_nn.out", {"pick": "A", "ranking": ["A"], "per_candidate": per("A", "A"), "improve_notes": ""})
    cli("record-judge", "allbad", str(td / "c_nn.out"), "--calib")
    rc, so3, _ = cli("judge-calib", "score")
    sc3 = json.loads(so3)
    check(rc == 1 and sc3["pick_accuracy"] == "2/3" and sc3["none_detection"] == "0/1" and not sc3["pass"], f"calib score wrong {so3}")
    check(cli("judge-calib", "build")[1].count("already built") == 1, "calib build idempotent")
    print("fix_ladder self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
