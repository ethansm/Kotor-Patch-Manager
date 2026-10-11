#!/usr/bin/env python3
"""F0 intake (deterministic, no LLM): verified_findings.json -> fix_queue.json + human_list.json + refuted_skim.json.

Usage:
  fix_intake.py                 write $FIX_WORK/{fix_queue,human_list,refuted_skim}.json, print the breakdown
  fix_intake.py --stats         print the breakdown only (writes nothing)
  fix_intake.py --self-test
Env: FIX_FINDINGS (input), FIX_DIR / FIX_WORK (outputs), KOTOR2_REVIEW_HOME (sanitized tree for line checks).

Rules (plan section 3; contract INTERFACES.md "Fix stage"):
  * Fixable tiers only: CONFIRMED, CONFIRMED-BY-SONNET-ONLY. CONTESTED-NEEDS-HUMAN -> human_list. REFUTED cpp-logic /
    build-config -> refuted_skim (human skim, never fixed). Decoys/controls never reach the tiers (verify_merge).
  * Dedupe: union-find over findings with the same primary file and line ranges overlapping (+-SLACK lines), or
    the same set of cited (file, line) evidence. One task per cluster (so no two tasks edit the same lines).
    Inside a cluster a finding counts as a duplicate when its claim shares >= 35% identifiers with an earlier
    member (same root cause), else as co-located (distinct defect, same lines; the fixer must fix each).
  * Class per finding, task class = most restrictive member:
      human-only   file excluded / missing / NEVER_EDIT (deploy, install, ledger, address DB, agent policy docs) / *.md
      needs-design hook-asm, build-config or gate-integrity specialty; *.toml / *.def; _shared_src; evidence in >1
                   file; cited span > 40 lines; severity high; sandbox_tests_only (live-dir / ledger writers)
      needs-review tier CONFIRMED-BY-SONNET-ONLY (Haiku disagreed)
      auto-fixable everything else
  * Flags: needs_live_check (hook-asm, hooks.toml, exports.def), gate_semantics (gate-integrity),
    sandbox_tests_only (LEDGER_UNSAFE scripts, or the claim/file writes the live game dir),
    relocate (an evidence quote is not within +-3 lines of its cited line; hint gives the found line).
  * Order: _shared_src, then mods, then scripts, then tests, then the rest; wave = 1 + max wave of earlier tasks that
    share an allowed file, so no two fixers in one wave touch the same file.
"""
from __future__ import annotations

import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402

SLACK = 3
CLASS_RANK = {c: i for i, c in enumerate(["auto-fixable", "needs-review", "needs-design", "human-only"])}
LIVE_RE = re.compile(r"(live_override|steamapps|game dir|live game|Override/|KOTOR2_ROOT|_diversity_premerge_backup)", re.I)


def norm(s: str) -> str:
    return " ".join(str(s or "").split())


def locate(q: dict) -> dict:
    """Check one evidence quote against the sanitized tree. -> {file, cited, found, status}."""
    rel = fc.safe_rel(q.get("file", ""))
    out = {"file": q.get("file"), "cited": q.get("line"), "found": None, "status": "unsafe"}
    if rel is None:
        return out
    lines = fc.read_lines(rel)
    if lines is None:
        out["status"] = "missing"
        return out
    text = norm(q.get("text"))
    if len(text) < 6:
        out["status"] = "trivial"
        return out
    ln = int(q.get("line") or 0)
    nq = max(1, str(q.get("text") or "").count("\n") + 1)
    win = norm(" ".join(lines[max(0, ln - 4): ln + 3 + nq]))
    if text in win:
        out.update(status="ok", found=ln)
        return out
    for i in range(len(lines)):
        if text in norm(" ".join(lines[i:i + nq])):
            out.update(status="moved", found=i + 1)
            return out
    out["status"] = "not-found"
    return out


def classify(f: dict) -> tuple[str, list[str], dict]:
    rel = fc.safe_rel(f["file"])
    reasons, flags = [], {}
    spec = f.get("specialty")
    files = {q.get("file") for q in f.get("evidence_quotes") or []} | {f["file"]}
    a, b = fc.parse_lines(f.get("lines"))
    if rel is None:
        return "human-only", ["excluded or unsafe path"], flags
    if fc.read_lines(rel) is None:
        reasons.append("file missing/binary in snapshot")
    if fc.never_edit(rel):
        reasons.append("never-edit path (deploy/install/ledger/address-DB/agent policy)")
    if rel.endswith(".md"):
        reasons.append("documentation/policy text")
    if any(fc.safe_rel(x) is None or fc.never_edit(fc.safe_rel(x) or "") for x in files if x):
        reasons.append("evidence cites a never-edit or excluded path")
    if spec == "hook-asm" or rel.endswith((".hooks.toml", "exports.def")):
        flags["needs_live_check"] = True
    if spec == "gate-integrity":
        flags["gate_semantics"] = True
    if any(fnm in rel for fnm in fc.LEDGER_UNSAFE) or LIVE_RE.search(f.get("claim", "")):
        flags["sandbox_tests_only"] = True
    if reasons:
        return "human-only", reasons, flags
    design = []
    if spec in ("hook-asm", "build-config", "gate-integrity"):
        design.append(f"specialty {spec}")
    if rel.endswith((".toml", ".def")):
        design.append("config/manifest file")
    if rel.startswith("patch_manager_mods/_shared_src/"):
        design.append("_shared_src (every mod)")
    if len({x for x in files if x}) > 1:
        design.append("evidence spans files")
    if b - a + 1 > 40:
        design.append("span > 40 lines")
    if f.get("final_severity") == "high":
        design.append("severity high")
    if flags.get("sandbox_tests_only"):
        design.append("writes the live game dir or the ledger (deploy-adjacent)")
    if design:
        return "needs-design", design, flags
    if f.get("tier") == "CONFIRMED-BY-SONNET-ONLY":
        return "needs-review", ["Haiku disagreed; Sonnet-only confirmation"], flags
    return "auto-fixable", ["small, local"], flags


def clusters(items: list[dict]) -> list[list[dict]]:
    parent = list(range(len(items)))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    def union(i, j):
        parent[find(i)] = find(j)

    ev = [frozenset((q.get("file"), q.get("line")) for q in it.get("evidence_quotes") or []) for it in items]
    for i in range(len(items)):
        ai, bi = fc.parse_lines(items[i]["lines"])
        for j in range(i + 1, len(items)):
            if items[i]["file"] == items[j]["file"]:
                aj, bj = fc.parse_lines(items[j]["lines"])
                if ai - SLACK <= bj and aj - SLACK <= bi:
                    union(i, j)
                    continue
            if ev[i] and ev[i] == ev[j]:
                union(i, j)
    groups = defaultdict(list)
    for i, it in enumerate(items):
        groups[find(i)].append(it)
    return list(groups.values())


TOK_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{3,}")


def same_cause(x: dict, y: dict) -> bool:
    """Duplicate (same root cause), not just co-located: identifier overlap of the claims >= 0.35."""
    tx, ty = set(TOK_RE.findall(x.get("claim", ""))), set(TOK_RE.findall(y.get("claim", "")))
    return bool(tx and ty) and len(tx & ty) / len(tx | ty) >= 0.35


def order_key(rel: str) -> tuple:
    name = rel.rsplit("/", 1)[-1]
    is_test = name.startswith("test_") or name.endswith(("_test.cpp", "_test.py")) or rel.startswith("tests/")
    if rel.startswith("patch_manager_mods/_shared_src/"):
        grp = 0
    elif is_test:
        grp = 3
    elif rel.startswith("patch_manager_mods/"):
        grp = 1
    elif rel.startswith("scripts/"):
        grp = 2
    else:
        grp = 4
    return (grp, rel)


def context_for(rel: str) -> list[str]:
    """Read-only context globs copied into the task copy (fix_workspace.py resolves them in base/)."""
    mod = fc.mod_of(rel)
    if mod:
        return [f"patch_manager_mods/{mod}/**", "patch_manager_mods/_shared_src/**", "tests/include/**"]
    if rel.startswith("scripts/"):
        stem = rel.rsplit("/", 1)[-1][:-3] if rel.endswith(".py") else ""
        ctx = ["scripts/*.py", "scripts/*.toml"]
        if stem:
            ctx.append(f"scripts/testdata/{stem}/**")
        return ctx
    d = rel.rsplit("/", 1)[0] if "/" in rel else ""
    return [f"{d}/*" if d else "*"]


def fixer_view(f: dict, hints: list[dict]) -> dict:
    """The ONLY finding fields a fixer or reviewer sees (no tier, no Haiku/Sonnet bookkeeping)."""
    return {"id": f["id"], "file": f["file"], "lines": f["lines"], "category": f.get("category"),
            "specialty": f.get("specialty"), "severity": f.get("final_severity"),
            "claim": f.get("claim", ""), "verifier_reasoning": f.get("reasoning", ""),
            "evidence_quotes": [{"file": q.get("file"), "line": q.get("line"), "text": q.get("text")}
                                for q in f.get("evidence_quotes") or [] if fc.safe_rel(q.get("file", ""))],
            "location_check": hints}


def build(data: dict) -> tuple[dict, dict, dict]:
    tiers = data.get("tiers") or {}
    items = [dict(f, tier=t) for t in fc.FIX_TIERS for f in tiers.get(t, [])]
    per = {}
    for f in items:
        cls, why, flags = classify(f)
        hints = [locate(q) for q in f.get("evidence_quotes") or []]
        if any(h["status"] in ("moved", "not-found") for h in hints):
            flags["relocate"] = True
        a, b = fc.parse_lines(f.get("lines"))
        prim = [h for h in hints if h["file"] == f["file"] and h["found"]]
        if prim and not any(a - SLACK <= h["found"] <= b + SLACK for h in prim):
            flags["relocate"] = True           # cited range disagrees with where its own quotes are
            hints.append({"file": f["file"], "cited": f.get("lines"), "found": sorted(h["found"] for h in prim),
                          "status": "range-mismatch"})
        per[f["id"]] = (cls, why, flags, hints)
    tasks = []
    for grp in clusters(items):
        grp.sort(key=lambda x: (fc.parse_lines(x["lines"]), x["id"]))
        cls = max((per[f["id"]][0] for f in grp), key=CLASS_RANK.get)
        flags, why = {}, []
        for f in grp:
            flags.update(per[f["id"]][2])
            why += [w for w in per[f["id"]][1] if w not in why]
        primary = grp[0]["file"]
        allowed = [primary]
        if cls != "auto-fixable":
            for f in grp:
                for q in f.get("evidence_quotes") or []:
                    r = fc.safe_rel(q.get("file", ""))
                    if r and r not in allowed and not fc.never_edit(r) and fc.read_lines(r) is not None:
                        allowed.append(r)
        specs = Counter(f.get("specialty") for f in grp)
        spec = sorted(specs, key=lambda s: (-specs[s], fc.SPECIALTIES.index(s) if s in fc.SPECIALTIES else 9))[0]
        ids = sorted(f["id"] for f in grp)
        tid = "T" + fc.sha_hex("|".join(ids))[:10]
        a = min(fc.parse_lines(f["lines"])[0] for f in grp)
        b = max(fc.parse_lines(f["lines"])[1] for f in grp)
        tasks.append({
            "task_id": tid, "class": cls, "class_reasons": why, "specialty": spec, "flags": flags,
            "finding_ids": ids,
            "duplicates": sum(1 for k, f in enumerate(grp) if any(same_cause(f, g) for g in grp[:k])),
            "colocated": len(ids) - 1 - sum(1 for k, f in enumerate(grp) if any(same_cause(f, g) for g in grp[:k])), "primary_file": primary, "lines": f"{a}-{b}",
            "mod": fc.mod_of(primary), "allowed_files": sorted(allowed, key=order_key),
            "context_globs": sorted({g for r in allowed for g in context_for(r)}),
            "line_cap": fc.LINE_CAP.get(cls, 0),
            "severity": max((f.get("final_severity") or "low" for f in grp), key=["low", "medium", "high"].index),
            "tiers": sorted({f["tier"] for f in grp}),
            "findings": [fixer_view(f, per[f["id"]][3]) for f in grp]})
    tasks.sort(key=lambda t: (order_key(t["primary_file"]), fc.parse_lines(t["lines"])))
    last_wave: dict[str, int] = {}
    for i, t in enumerate(tasks):
        t["order"] = i + 1
        if t["class"] == "human-only":
            t["wave"] = 0
            continue
        w = 1 + max((last_wave.get(r, 0) for r in t["allowed_files"]), default=0)
        t["wave"] = w
        for r in t["allowed_files"]:
            last_wave[r] = w
    counts = {"findings_in": len(items), "tasks": len(tasks),
              "by_class": dict(Counter(t["class"] for t in tasks)),
              "findings_by_class": dict(Counter(t["class"] for t in tasks for _ in t["finding_ids"])),
              "by_specialty": dict(Counter(t["specialty"] for t in tasks if t["class"] != "human-only")),
              "duplicates_merged": sum(t["duplicates"] for t in tasks),
              "colocated_merged": sum(t["colocated"] for t in tasks),
              "relocate": sum(1 for t in tasks if t["flags"].get("relocate")),
              "needs_live_check": sum(1 for t in tasks if t["flags"].get("needs_live_check")),
              "gate_semantics": sum(1 for t in tasks if t["flags"].get("gate_semantics")),
              "sandbox_tests_only": sum(1 for t in tasks if t["flags"].get("sandbox_tests_only")),
              "waves": max((t["wave"] for t in tasks), default=0),
              "files": len({r for t in tasks for r in t["allowed_files"]})}
    queue = {"generated_by": "fix/fix_intake.py", "source_sha256": fc.file_sha(fc.FINDINGS),
             "pinned_head": fc.PINNED_HEAD, "counts": counts, "tasks": tasks}
    human = {"generated_by": "fix/fix_intake.py",
             "contested": [{"id": f["id"], "file": f["file"], "lines": f["lines"], "specialty": f.get("specialty"),
                            "claim": f.get("claim", "")[:400], "human_check_steps": f.get("human_check_steps")}
                           for f in tiers.get("CONTESTED-NEEDS-HUMAN", [])],
             "human_only_tasks": [{"task_id": t["task_id"], "finding_ids": t["finding_ids"], "file": t["primary_file"],
                                   "lines": t["lines"], "reasons": t["class_reasons"]}
                                  for t in tasks if t["class"] == "human-only"]}
    skim = {"generated_by": "fix/fix_intake.py",
            "note": "REFUTED cpp-logic/build-config: verifiers over-refute design-level risks (K2, K4). Skim only; never fixed.",
            "items": [{"id": f["id"], "file": f["file"], "lines": f["lines"], "specialty": f.get("specialty"),
                       "claim": f.get("claim", "")[:300], "why_refuted": (f.get("reasoning") or "")[:300]}
                      for f in tiers.get("REFUTED", []) if f.get("specialty") in ("cpp-logic", "build-config")]}
    return queue, human, skim


def report(q: dict, h: dict, s: dict) -> None:
    c = q["counts"]
    print(f"intake: {c['findings_in']} fixable findings -> {c['tasks']} tasks ({c['duplicates_merged']} duplicates merged), "
          f"{c['colocated_merged']} co-located distinct findings grouped, {c['files']} files, {c['waves']} waves")
    print("  tasks by class   :", c["by_class"])
    print("  findings by class:", c["findings_by_class"])
    print("  fixer tasks by specialty:", c["by_specialty"])
    print(f"  flags: relocate {c['relocate']}, needs_live_check {c['needs_live_check']}, gate_semantics "
          f"{c['gate_semantics']}, sandbox_tests_only {c['sandbox_tests_only']}")
    print(f"  human list: {len(h['contested'])} contested + {len(h['human_only_tasks'])} human-only tasks; "
          f"refuted skim: {len(s['items'])}")


def self_test() -> int:
    import json
    import os
    import tempfile
    bad = []
    with tempfile.TemporaryDirectory() as td:
        tree = Path(td) / "worktree"
        (tree / "scripts").mkdir(parents=True)
        (tree / "scripts" / "a.py").write_text("\n".join(f"line {i}" for i in range(1, 60)) + "\n")
        fc.TREE = tree
        q = lambda f, l, t: {"file": f, "line": l, "text": t}  # noqa: E731
        base = {"category": "logic", "claim": "x", "final_severity": "low", "specialty": "python-tooling"}
        data = {"tiers": {
            "CONFIRMED": [dict(base, id="aaa", file="scripts/a.py", lines="10-12", evidence_quotes=[q("scripts/a.py", 10, "line 10")]),
                          dict(base, id="bbb", file="scripts/a.py", lines="12-14", evidence_quotes=[q("scripts/a.py", 30, "line 13")]),
                          dict(base, id="ccc", file="scripts/a.py", lines="40-41", evidence_quotes=[q("scripts/a.py", 40, "line 40")]),
                          dict(base, id="ddd", file="scripts/kpm_hotswap.py", lines="1", evidence_quotes=[]),
                          dict(base, id="eee", file=".env", lines="1", evidence_quotes=[])],
            "CONFIRMED-BY-SONNET-ONLY": [dict(base, id="fff", file="scripts/a.py", lines="50", evidence_quotes=[q("scripts/a.py", 50, "line 50")])],
            "CONTESTED-NEEDS-HUMAN": [dict(base, id="ggg", file="scripts/a.py", lines="1")],
            "REFUTED": [dict(base, id="hhh", file="x.cpp", lines="1", specialty="cpp-logic"),
                        dict(base, id="iii", file="y.py", lines="1")]}}
        qq, hh, ss = build(data)
        byf = {tuple(t["finding_ids"]): t for t in qq["tasks"]}
        if ("aaa", "bbb") not in byf:
            bad.append(f"dedupe overlap: {list(byf)}")
        elif not byf[("aaa", "bbb")]["flags"].get("relocate"):
            bad.append("relocate flag (quote at 30 found at 13)")
        if byf.get(("ddd",), {}).get("class") != "human-only" or byf.get(("eee",), {}).get("class") != "human-only":
            bad.append("human-only rules")
        if byf.get(("fff",), {}).get("class") != "needs-review":
            bad.append("sonnet-only -> needs-review")
        if byf.get(("ccc",), {}).get("class") != "auto-fixable":
            bad.append("auto-fixable")
        w = {t["finding_ids"][0]: t["wave"] for t in qq["tasks"]}
        if len({w["aaa"], w["ccc"], w["fff"]}) != 3:
            bad.append(f"waves must separate same-file tasks: {w}")
        if [x["id"] for x in hh["contested"]] != ["ggg"] or [x["id"] for x in ss["items"]] != ["hhh"]:
            bad.append("human/skim lists")
        blob = json.dumps(qq)
        for k in ("haiku_verdict", "agrees_with_haiku", "verify_key", "decoy"):
            if k in blob:
                bad.append(f"leak {k}")
        if any(fc.is_excluded(r) for t in qq["tasks"] if t["class"] != "human-only" for r in t["allowed_files"]):
            bad.append("excluded path in allowed_files")
        del os
    print("fix_intake self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad))
    return 1 if bad else 0


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return self_test()
    if argv and argv[:1] != ["--stats"]:   # --help or a typo must never rewrite the real queue (bug 17)
        print(__doc__.strip().splitlines()[0] + "\nusage: fix_intake.py [--stats | --self-test]", file=sys.stderr)
        return 2
    data = fc.rj(fc.FINDINGS)
    if not isinstance(data, dict) or "tiers" not in data:
        sys.exit(f"fix_intake: cannot read {fc.FINDINGS}")
    q, h, s = build(data)
    blob = fc.dumps(q)
    for t in q["tasks"]:
        for r in t["allowed_files"] + [x.get("file") or "" for f in t["findings"] for x in f["evidence_quotes"]]:
            if fc.is_excluded(r):
                sys.exit(f"fix_intake: excluded path leaked into task {t['task_id']}: refusing")
    if "verify_key" in blob or "withheld" in blob:
        sys.exit("fix_intake: answer-key reference in queue; refusing")
    report(q, h, s)
    if argv[:1] == ["--stats"]:
        return 0
    fc.write_json(fc.QUEUE, q)
    fc.write_json(fc.HUMAN, h)
    fc.write_json(fc.SKIM, s)
    print(f"wrote {fc.QUEUE}, {fc.HUMAN}, {fc.SKIM}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
