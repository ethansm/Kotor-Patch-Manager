#!/usr/bin/env python3
"""Fix stage F4, final step: merge fixer / check / reviewer / integrator records into the fix report (no LLM).

Usage:
  fix_merge.py [--out-dir DIR]        write DIR/fix_report.{json,md}   (default $KOTOR2_FIX_REPORT = .claude/reviews)
  fix_merge.py --self-test
Stdlib only, no timestamps: identical inputs give byte-identical outputs. Exit 1 when any queue task is not terminal
(or a calibration item has no review yet); the re-queue lines for run_fix.sh / run_fix_review.sh / run_integrate.sh
are printed and stored in the report.

Terminal statuses per queue task (INTERFACES "Task status"), judged on the LATEST attempt:
  fixed+tested            kept/modified in a series whose verify-series was ok, no live check needed
  fixed+needs-live-check  same, but flags.needs_live_check / fixer needs_live_check / hook-asm specialty
  rejected                reviewer reject (or still `revise` after the last revision round, $FIX_REVISE_ROUNDS)
  needs-wider-scope       fixer asked for files outside allowed_files / a contract change (scope_request); a human
                          decides, scope is never expanded automatically
  human-only              class human-only; gate failures after the one retry; integrator dropped the patch
  not-fixable             fixer: every finding not-fixable / not-reproducible / already-fixed
  failed                  fixer invalid/failed after retry; verify-series failed
Per finding: the task status, except fixer-level not-fixable findings of a fixed task (not-fixable) and findings
the reviewer marked defect_fixed=no (human-only: contested). Contested findings from the verify stage
(work/fix/human_list.json) are listed as human-only, task_id null.
The calibration answer key (work/fix/calib_key.json) is read ONLY here, for the catch / false-reject statistics.
"""
from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402
import build_integration_batches as bib  # noqa: E402

TERMINAL = ["fixed+tested", "fixed+needs-live-check", "rejected", "human-only", "not-fixable", "needs-wider-scope", "failed"]
NON_FIXED = ("not-fixable", "not-reproducible", "already-fixed", "needs-wider-scope")
CODE_REL = ".claude/review/fix"


def rel(p) -> str:
    p = Path(p)
    try:
        return p.resolve().relative_to(fc.REPO).as_posix()
    except ValueError:
        return str(p)


def revise_rounds() -> int:
    """Mirror of record_fix.revise_rounds ($FIX_REVISE_ROUNDS, default 1)."""
    try:
        return max(0, int(os.environ.get("FIX_REVISE_ROUNDS", "1")))
    except ValueError:
        return 1


def is_test_path(p: str) -> bool:
    low = p.lower()
    name = low.rsplit("/", 1)[-1]
    return low.startswith("tests/") or "/tests/" in low or name.startswith("test_") or name == "test.toml" \
        or any(name.endswith(x) for x in ("_test.cpp", "_test.py", "_test.h", ".test.py", ".test.toml"))


def scope_flags(path: str) -> list[str]:
    rel_p = fc.safe_rel(path)
    flags = []
    if rel_p is None:
        return ["unsafe-or-excluded"]
    if fc.never_edit(rel_p) or fc.is_excluded(rel_p):
        flags.append("never-edit")
    if is_test_path(rel_p):
        flags.append("test-file (tests-weakening risk)")
    return flags


def short(s, n=160) -> str:
    s = " ".join(str(s or "").split())
    return s if len(s) <= n else s[:n - 3] + "..."


# ---------------------------------------------------------------- batches / series lookup
def batch_index() -> dict:
    """tid -> {batch, attempt}"""
    idx = {}
    for b in bib.batch_ids():
        d = fc.rj(fc.BATCHES / f"{b}.json") or {}
        for t in d.get("tasks") or []:
            idx[t["task_id"]] = {"batch": b, "attempt": t.get("attempt")}
    return idx


def series_record(b: str):
    return fc.rj(fc.OUT / b / "series.json")


def gate_map(g) -> dict:
    out = {}
    if isinstance(g, dict):
        for k in sorted(g):
            v = g[k]
            out[k] = v.get("status") if isinstance(v, dict) else v
    return out


# ---------------------------------------------------------------- per-task evaluation
def evaluate(t: dict, bidx: dict) -> dict:
    tid = t["task_id"]
    ev = {"task_id": tid, "class": t.get("class"), "specialty": t.get("specialty"), "order": t.get("order"),
          "primary_file": t.get("primary_file"), "mod": t.get("mod"), "status": None, "stage": None,
          "reason": "", "attempt": 0, "batch": None, "patches": [], "fixer_status": None,
          "reviewer_verdict": None, "integration": None, "needs_live_check": False, "evidence": {},
          "finding_override": {}, "scope_request": None}

    def pend(stage, reason):
        ev["status"], ev["stage"], ev["reason"] = "pending", stage, reason
        return ev

    def done(status, reason):
        ev["status"], ev["reason"] = status, reason
        return ev

    if t.get("class") == "human-only":
        return done("human-only", "class human-only: " + "; ".join(t.get("class_reasons") or []))
    st = bib.task_state(tid)
    n = st["attempt"]
    ev["attempt"] = n
    res = st["result"]
    if not n or not isinstance(res, dict):
        return pend("fix", "no fixer result")
    rstat = res.get("status")
    if rstat != "ok" or not isinstance(res.get("output"), dict):
        if rstat in ("failed", "invalid") and n >= 2:
            return done("failed", f"fixer {rstat} after retry")
        return pend("fix", f"fixer {rstat}")
    out = res["output"]
    fstats = {f.get("finding_id"): f.get("status") for f in out.get("findings") or []}
    ev["finding_override"] = {k: v for k, v in fstats.items() if v in NON_FIXED}
    if not any(v == "fixed" for v in fstats.values()):
        if any(v == "needs-wider-scope" for v in fstats.values()):
            sr = out.get("scope_request") if isinstance(out.get("scope_request"), dict) else {}
            files = [str(x) for x in sr.get("files") or []]
            ev["fixer_status"] = "needs-wider-scope"
            ev["scope_request"] = {"files": [{"path": x, "flags": scope_flags(x)} for x in files],
                                   "reason": short(sr.get("reason") or out.get("not_fixable_reason"), 400)}
            return done("needs-wider-scope", "scope request: " + (", ".join(files) or "(no files given)"))
        why = out.get("not_fixable_reason") or ", ".join(sorted({str(v) for v in fstats.values()}))
        ev["fixer_status"] = "not-fixable"
        return done("not-fixable", short(why))
    ev["fixer_status"] = "fixed"
    ev["needs_live_check"] = bool((t.get("flags") or {}).get("needs_live_check") or out.get("needs_live_check")
                                  or t.get("specialty") == "hook-asm")
    ck = st["check"]
    if st["patch"] is None or not isinstance(ck, dict):
        return pend("fix", "no patch/check for the latest attempt")
    ev["evidence"]["check"] = {"ok": ck.get("ok"), "gates": gate_map(ck.get("gates")),
                               "new_failures": ck.get("new_failures") or [],
                               "ledger_unchanged": ck.get("ledger_unchanged")}
    if ck.get("ok") is not True:
        bad = [k for k, v in gate_map(ck.get("gates")).items() if v == "fail"]
        if n >= 2:
            return done("human-only", "gate failures after retry: " + (", ".join(bad) or "check not ok"))
        return pend("fix", "check failed on attempt 1 (retry with the error)")
    rv = st["review"]
    verdict = bib.review_verdict(rv)
    ev["reviewer_verdict"] = verdict
    if verdict is None:
        return pend("review", "no ok review")
    if verdict == "reject":
        return done("rejected", "reviewer reject: " + short("; ".join(
            f"[{i.get('severity')}] {i.get('description')}" for i in rv["output"].get("issues") or []), 200))
    if verdict == "revise":
        rounds = max(0, n - 2)
        if n >= (2 if revise_rounds() <= 1 else 2 + revise_rounds()):
            return done("rejected", f"still revise after {rounds} revision round(s) (last round a{n})")
        return pend("fix", "reviewer asked for a revision")
    for pf in rv["output"].get("per_finding") or []:
        if pf.get("defect_fixed") in ("no", "partial"):
            ev["finding_override"].setdefault(pf.get("finding_id"), "contested-" + pf["defect_fixed"])
    bi = bidx.get(tid)
    if bi is None or bi.get("attempt") != n:
        return pend("batch", "accepted but not in an integration batch")
    b = ev["batch"] = bi["batch"]
    if not bib.run_ok(b):
        return pend("integrate", f"{b}: no ok integrator run")
    ver = bib.verify_rec(b)
    if not isinstance(ver, dict):
        return pend("integrate", f"{b}: verify-series not run")
    ev["evidence"]["verify"] = {"ok": ver.get("ok"), "gates": gate_map(ver.get("gates")),
                                "summary": short(ver.get("summary") or ver.get("detail") or "", 200)}
    if ver.get("ok") is not True:
        return done("failed", f"{b}: verify-series failed")
    sr = series_record(b)
    if not isinstance(sr, dict):
        return pend("integrate", f"{b}: verified series not published")
    ev["evidence"]["integration_gates"] = [
        f"{g.get('cmd')} -> {g.get('exit_code')}: {short(g.get('summary'), 80)}" for g in sr.get("gates_run") or []]
    row = next((x for x in sr.get("series") or [] if x.get("task_id") == tid), None)
    if row is None:
        dr = next((x for x in sr.get("dropped") or [] if x.get("task_id") == tid), {})
        ev["integration"] = "drop"
        return done("human-only", f"dropped by integration ({b}): " + short(dr.get("reason"), 200))
    ev["integration"] = row.get("action")
    ev["patches"] = [rel(fc.OUT / b / row["file"])]
    return done("fixed+needs-live-check" if ev["needs_live_check"] else "fixed+tested",
                "integration " + str(row.get("action")) + (" (needs live check)" if ev["needs_live_check"] else ""))


# ---------------------------------------------------------------- statistics
def calibration(key: dict) -> tuple[dict, list[str]]:
    items = (key or {}).get("items") or {}
    modes: dict[str, dict] = {}
    unreviewed: list[str] = []
    for tid in sorted(items):
        k = items[tid]
        st = bib.task_state(tid)
        verdict = bib.review_verdict(st["review"])
        ckok = st["check"].get("ok") if isinstance(st["check"], dict) else None
        if verdict is None:
            unreviewed.append(tid)
            continue
        kind, mode = k.get("kind"), k.get("mode")
        m = modes.setdefault(f"{kind}/{mode}", {"kind": kind, "mode": mode, "n": 0, "caught": 0, "by_check": 0,
                                                  "by_review": 0, "accepted": 0, "rejected": 0, "revised": 0,
                                                  "check_failed": 0, "false_rejects": 0})
        m["n"] += 1
        m["rejected"] += verdict == "reject"
        m["revised"] += verdict == "revise"
        m["accepted"] += verdict == "accept"
        m["check_failed"] += ckok is False
        m["by_check"] += ckok is False
        m["by_review"] += verdict != "accept"
        m["caught"] += (ckok is False) or verdict != "accept"
        m["false_rejects"] += (ckok is False) or verdict == "reject"      # only meaningful for known-good
    seeded = [m for m in modes.values() if m["kind"] == "seeded-bad"]
    good = [m for m in modes.values() if m["kind"] == "known-good"]
    sn, sc = sum(m["n"] for m in seeded), sum(m["caught"] for m in seeded)
    gn = sum(m["n"] for m in good)
    for m in modes.values():
        m["catch_rate"] = round(m["caught"] / m["n"], 4) if m["n"] else None
        m["false_reject_rate"] = round(m["false_rejects"] / m["n"], 4) if m["n"] and m["kind"] == "known-good" else None
    return ({"modes": dict(sorted(modes.items())),
             "seeded_bad": {"n": sn, "caught": sc, "catch_rate": round(sc / sn, 4) if sn else None},
             "known_good": {"n": gn, "false_rejects": sum(m["false_rejects"] for m in good),
                            "false_reject_rate": round(sum(m["false_rejects"] for m in good) / gn, 4) if gn else None},
             "unreviewed": unreviewed}, unreviewed)


def stage_costs() -> dict:
    def calls(d, suffix):
        n = 0
        if Path(d).is_dir():
            for f in Path(d).glob(f"*{suffix}"):
                if isinstance(fc.rj(f), dict):
                    n += 1
        return n
    c = {"fix": {"cost_usd": fc.spent("fix"), "calls": calls(fc.RESULTS, ".json")},
         "review": {"cost_usd": fc.spent("review"), "calls": calls(fc.REVIEWS, ".review.json")},
         "integrate": {"cost_usd": fc.spent("integrate"), "calls": calls(fc.INTEG, ".run.json")}}
    c["total_usd"] = round(sum(v["cost_usd"] for v in c.values() if isinstance(v, dict)), 6)
    return c


# ---------------------------------------------------------------- report
def build_report() -> tuple[dict, list[str]]:
    q = fc.load_queue()
    tasks = sorted(q["tasks"], key=lambda t: (t.get("order") if isinstance(t.get("order"), (int, float)) else 10 ** 9, t["task_id"]))
    skip = bib.calib_ids()
    bidx = batch_index()
    evs = [evaluate(t, bidx) for t in tasks if t["task_id"] not in skip]
    byid = {e["task_id"]: e for e in evs}

    findings = []
    for t in tasks:
        e = byid.get(t["task_id"])
        if e is None:
            continue
        for f in t.get("findings") or []:
            fid = f.get("id")
            status, note = e["status"], ""
            ov = e["finding_override"].get(fid)
            if e["status"].startswith("fixed") or e["status"] == "pending":
                if ov == "needs-wider-scope":
                    status, note = "needs-wider-scope", "fixer: needs-wider-scope"
                elif ov in NON_FIXED:
                    status, note = "not-fixable", f"fixer: {ov}"
                elif ov == "contested-no":
                    status, note = "human-only", "reviewer: defect_fixed=no (contested)"
                elif ov == "contested-partial":
                    note = "reviewer: defect_fixed=partial"
            findings.append({"id": fid, "task_id": t["task_id"], "file": f.get("file"), "lines": f.get("lines"),
                             "severity": f.get("severity"), "specialty": f.get("specialty"), "status": status,
                             "note": note or e["reason"],
                             "patches": e["patches"] if status.startswith("fixed") else [],
                             "evidence": e["evidence"] if status.startswith("fixed") else {}})
    hl = fc.rj(fc.HUMAN) or {}
    contested = hl.get("contested") or []
    for c in contested:
        findings.append({"id": c.get("id"), "task_id": None, "file": c.get("file"), "lines": c.get("lines"),
                         "severity": None, "specialty": c.get("specialty"), "status": "human-only",
                         "note": "contested at the verify stage", "patches": [], "evidence": {}})
    findings.sort(key=lambda f: (str(f["task_id"] or "~"), str(f["id"])))

    counts = {s: 0 for s in TERMINAL + ["pending"]}
    for e in evs:
        counts[e["status"]] += 1
    fcounts = {s: 0 for s in TERMINAL + ["pending"]}
    for f in findings:
        fcounts[f["status"]] = fcounts.get(f["status"], 0) + 1

    calib, unrev = calibration(fc.rj(fc.CALIB_KEY) or {})

    fixed_cand = [e for e in evs if e["fixer_status"] == "fixed" and e["reviewer_verdict"]]
    acc = sum(1 for e in fixed_cand if e["reviewer_verdict"] == "accept")
    agreement = {"fixer_fixed_and_reviewed": len(fixed_cand), "reviewer_accept": acc,
                 "reviewer_revise": sum(1 for e in fixed_cand if e["reviewer_verdict"] == "revise"),
                 "reviewer_reject": sum(1 for e in fixed_cand if e["reviewer_verdict"] == "reject"),
                 "agreement_rate": round(acc / len(fixed_cand), 4) if fixed_cand else None}
    integ = [e for e in evs if e["integration"]]
    keep = sum(1 for e in integ if e["integration"] == "keep")
    mod = sum(1 for e in integ if e["integration"] == "modified")
    drop = sum(1 for e in integ if e["integration"] == "drop")
    override = {"accepted_and_integrated": len(integ), "keep": keep, "modified": mod, "drop": drop,
                "override_rate": round((mod + drop) / len(integ), 4) if integ else None}

    skim2 = fc.rj(fc.WORK / "refuted_skim_pass2.json") or {}
    skim_hits = [{"id": i.get("id"), "file": i.get("file"), "lines": i.get("lines"), "verdict": i.get("verdict"),
                  "human_action": short(i.get("human_action"), 300)}
                 for i in skim2.get("items") or [] if i.get("verdict") in ("real", "design-risk")]
    human = {
        "contested": [{"id": c.get("id"), "file": c.get("file"), "lines": c.get("lines"),
                       "claim": short(c.get("claim"), 240), "human_check_steps": c.get("human_check_steps") or []}
                      for c in contested],
        "human_only_tasks": [{"task_id": e["task_id"], "file": e["primary_file"], "reason": short(e["reason"], 240)}
                             for e in evs if e["status"] == "human-only"],
        "needs_wider_scope": [{"task_id": e["task_id"], "file": e["primary_file"], "files": e["scope_request"]["files"],
                               "reason": e["scope_request"]["reason"]}
                              for e in evs if e["status"] == "needs-wider-scope"],
        "refuted_skim_pass2": sorted(skim_hits, key=lambda x: (str(x["verdict"]), str(x["id"]))),
    }

    req = {"fix": [], "review": [], "integrate": [], "batch": []}
    for e in evs:
        if e["status"] == "pending":
            key = e["batch"] if e["stage"] == "integrate" else e["task_id"]
            if e["stage"] == "batch":
                key = e["task_id"]
            if key not in req[e["stage"]]:
                req[e["stage"]].append(key)
    req["review"] += [t for t in unrev if t not in req["review"]]
    lines = []
    if req["fix"]:
        lines.append(f"bash {CODE_REL}/run_fix.sh --only {','.join(req['fix'])}")
    if req["review"]:
        lines.append(f"bash {CODE_REL}/run_fix_review.sh --only {','.join(req['review'])}")
    if req["batch"]:
        lines.append(f"python3 -I {CODE_REL}/build_integration_batches.py   # {len(req['batch'])} accepted task(s) not batched")
    if req["integrate"]:
        lines.append(f"bash {CODE_REL}/run_integrate.sh --only {','.join(sorted(req['integrate']))}")

    rep = {"generated_by": "fix/fix_merge.py", "pinned_head": q.get("pinned_head"),
           "queue_sha256": q.get("source_sha256"),
           "counts": {"tasks": len(evs), "task_status": counts, "findings": len(findings), "finding_status": fcounts},
           "coverage": {"terminal": len(evs) - counts["pending"], "pending": counts["pending"],
                        "calibration_unreviewed": len(unrev), "complete": counts["pending"] == 0 and not unrev,
                        "requeue": lines},
           "calibration": calib, "fixer_reviewer_agreement": agreement, "sonnet_override": override,
           "cost": stage_costs(), "human_lists": human,
           "tasks": [{k: v for k, v in e.items() if k != "finding_override"} for e in evs],
           "findings": findings}
    return rep, lines


def pct(x):
    return "n/a" if x is None else f"{x * 100:.1f}%"


def render_md(rep: dict) -> str:
    c = rep["counts"]
    L = ["# Fix report", "",
         f"Pinned head `{rep['pinned_head']}`. {c['tasks']} queue task(s), {c['findings']} finding(s).", "",
         "## Status", "", "| status | tasks | findings |", "|---|---|---|"]
    for s in TERMINAL + ["pending"]:
        L.append(f"| {s} | {c['task_status'].get(s, 0)} | {c['finding_status'].get(s, 0)} |")
    cv = rep["coverage"]
    L += ["", "## Coverage", "",
          f"- terminal tasks: {cv['terminal']} of {c['tasks']}; pending: {cv['pending']}; "
          f"calibration items unreviewed: {cv['calibration_unreviewed']}",
          f"- complete: {'yes' if cv['complete'] else 'NO'}"]
    for ln in cv["requeue"]:
        L.append(f"- re-queue: `{ln}`")
    ca = rep["calibration"]
    L += ["", "## Calibration", "",
          f"- seeded-bad: {ca['seeded_bad']['caught']}/{ca['seeded_bad']['n']} caught (catch rate {pct(ca['seeded_bad']['catch_rate'])})",
          f"- known-good: {ca['known_good']['false_rejects']}/{ca['known_good']['n']} false-rejected "
          f"(rate {pct(ca['known_good']['false_reject_rate'])})", "",
          "| kind/mode | n | caught | by check | by review | accepted | rejected | revised |", "|---|---|---|---|---|---|---|---|"]
    for k, m in ca["modes"].items():
        L.append(f"| {k} | {m['n']} | {m['caught']} | {m['by_check']} | {m['by_review']} | {m['accepted']} | {m['rejected']} | {m['revised']} |")
    ag, ov = rep["fixer_reviewer_agreement"], rep["sonnet_override"]
    L += ["", "## Agreement and overrides", "",
          f"- fixer said fixed, reviewer accepted: {ag['reviewer_accept']}/{ag['fixer_fixed_and_reviewed']} ({pct(ag['agreement_rate'])}); "
          f"revise {ag['reviewer_revise']}, reject {ag['reviewer_reject']}",
          f"- Sonnet override of accepted patches: {ov['modified'] + ov['drop']}/{ov['accepted_and_integrated']} "
          f"({pct(ov['override_rate'])}); keep {ov['keep']}, modified {ov['modified']}, dropped {ov['drop']}"]
    co = rep["cost"]
    L += ["", "## Cost (CLI-reported)", "", "| stage | calls | USD |", "|---|---|---|"]
    for s in ("fix", "review", "integrate"):
        L.append(f"| {s} | {co[s]['calls']} | {co[s]['cost_usd']:.4f} |")
    L.append(f"| total | | {co['total_usd']:.4f} |")
    hl = rep["human_lists"]
    L += ["", f"## Human lists", "", f"### Contested at verify ({len(hl['contested'])})", ""]
    for x in hl["contested"]:
        L.append(f"- `{x['id']}` {x['file']}:{x['lines']} - {x['claim']}")
        for s in x["human_check_steps"]:
            L.append(f"  - check: {short(s, 200)}")
    L += ["", f"### Human-only tasks ({len(hl['human_only_tasks'])})", ""]
    for x in hl["human_only_tasks"]:
        L.append(f"- `{x['task_id']}` {x['file']} - {x['reason']}")
    L += ["", f"### Needs wider scope ({len(hl['needs_wider_scope'])}) - never auto-expanded; a human decides", ""]
    for x in hl["needs_wider_scope"]:
        L.append(f"- `{x['task_id']}` {x['file']} - {x['reason']}")
        for fl in x["files"]:
            L.append(f"  - requested: `{fl['path']}`" + (f" **FLAG: {', '.join(fl['flags'])}**" if fl["flags"] else ""))
    L += ["", f"### Refuted-skim pass 2: real / design-risk ({len(hl['refuted_skim_pass2'])})", ""]
    for x in hl["refuted_skim_pass2"]:
        L.append(f"- `{x['id']}` [{x['verdict']}] {x['file']}:{x['lines']} - {x['human_action']}")
    L += ["", "## Tasks", "", "| task | class | specialty | status | patch | reason |", "|---|---|---|---|---|---|"]
    for e in rep["tasks"]:
        L.append(f"| {e['task_id']} | {e['class']} | {e['specialty']} | {e['status']} | "
                 f"{', '.join(e['patches']) or '-'} | {short(e['reason'], 110).replace('|', '/')} |")
    L += ["", "## Findings", "", "| finding | task | file:lines | status | note |", "|---|---|---|---|---|"]
    for f in rep["findings"]:
        L.append(f"| {f['id']} | {f['task_id'] or '-'} | {f['file']}:{f['lines']} | {f['status']} | "
                 f"{short(f['note'], 100).replace('|', '/')} |")
    return "\n".join(L).rstrip("\n") + "\n"


def run(out_dir: Path) -> int:
    rep, lines = build_report()
    out_dir.mkdir(parents=True, exist_ok=True)
    fc.write_json(out_dir / "fix_report.json", rep)
    (out_dir / "fix_report.md").write_text(render_md(rep), encoding="utf-8")
    c = rep["counts"]
    st = ", ".join(f"{k}={v}" for k, v in c["task_status"].items() if v)
    print(f"fix_merge: {c['tasks']} task(s): {st}; wrote {out_dir}/fix_report.{{json,md}}")
    if not rep["coverage"]["complete"]:
        print(f"fix_merge: INCOMPLETE ({rep['coverage']['pending']} pending task(s), "
              f"{rep['coverage']['calibration_unreviewed']} unreviewed calibration item(s)); re-queue with:")
        for ln in lines:
            print("  " + ln)
        return 1
    return 0


# ---------------------------------------------------------------- self-test
def self_test() -> int:
    bad: list[str] = []
    tmp = Path(tempfile.mkdtemp(prefix="fixmerge_selftest_"))
    try:
        fd = tmp / "fd"
        work = fd / "work"
        for d in ("results", "patches", "checks", "reviews", "integ"):
            (fd / d).mkdir(parents=True)
        (work / "batches").mkdir(parents=True)
        specs = {}                                 # name -> (tid, file, extra task kwargs)
        names = ["fixed", "live", "modified", "rejected", "classhuman", "gatefail", "notfix", "failed",
                 "dropped", "partial", "pendfix", "revise1", "wide", "revise3"]
        tasks = []
        for i, nm in enumerate(names):
            f = f"scripts/m{i}.py"
            t = bib._mk_task(i, f, i + 1, cls="human-only" if nm == "classhuman" else "auto-fixable",
                             spec="hook-asm" if nm == "live" else "python-tooling")
            t["findings"].append({"id": "g%011x" % i, "file": f, "lines": "5", "severity": "low", "claim": "second"})
            if nm == "live":
                t["flags"] = {"needs_live_check": True}
            tasks.append(t)
            specs[nm] = t["task_id"]
        (work / "fix_queue.json").write_text(json.dumps({"tasks": tasks, "pinned_head": "abc", "source_sha256": "s"}))

        def W(path, obj):
            Path(path).write_text(json.dumps(obj))

        def fixer(tid, n, statuses, status="ok", cost=0.1):
            W(fd / "results" / f"{tid}.a{n}.json", {"status": status, "cost_usd": cost, "output": None if status != "ok" else {
                "findings": [{"finding_id": fid, "status": s} for fid, s in statuses], "needs_live_check": False,
                "not_fixable_reason": "cannot repro"}})

        def fids(nm):
            t = [x for x in tasks if x["task_id"] == specs[nm]][0]
            return [f["id"] for f in t["findings"]]

        def chk(tid, n, ok=True):
            W(fd / "patches" / f"{tid}.a{n}.patch", "x")
            (fd / "patches" / f"{tid}.a{n}.patch").write_text("--- a/x\n+++ b/x\n@@ -1 +1 @@\n-a\n+b\n")
            W(fd / "checks" / f"{tid}.a{n}.check.json", {"ok": ok, "gates": {"build": {"status": "pass" if ok else "fail"}}})

        def rev(tid, n, verdict, per=None, cost=0.2):
            W(fd / "reviews" / f"{tid}.a{n}.review.json", {"status": "ok", "schema_valid": True, "cost_usd": cost, "output": {
                "verdict": verdict, "issues": [{"severity": "major", "description": "bad thing"}] if verdict != "accept" else [],
                "per_finding": per or []}})

        for nm in ("fixed", "live", "modified", "rejected", "dropped", "partial", "pendfix", "revise1"):
            tid = specs[nm]
            fixer(tid, 1, [(f, "fixed") for f in fids(nm)])
            chk(tid, 1)
        rev(specs["fixed"], 1, "accept")
        rev(specs["live"], 1, "accept")
        rev(specs["modified"], 1, "accept")
        rev(specs["rejected"], 1, "reject")
        rev(specs["dropped"], 1, "accept")
        pf = fids("partial")
        rev(specs["partial"], 1, "accept", per=[{"finding_id": pf[0], "defect_fixed": "yes", "why": "w"},
                                                 {"finding_id": pf[1], "defect_fixed": "no", "why": "w"}])
        rev(specs["revise1"], 1, "revise")
        # revise3: a1 revise -> a3 (revision round 1) still revise
        fixer(specs["revise3"], 1, [(f, "fixed") for f in fids("revise3")])
        chk(specs["revise3"], 1)
        rev(specs["revise3"], 1, "revise")
        fixer(specs["revise3"], 3, [(f, "fixed") for f in fids("revise3")])
        chk(specs["revise3"], 3)
        rev(specs["revise3"], 3, "revise")
        # wide: fixer asks for files outside the allowed set
        wf = fids("wide")
        W(fd / "results" / f"{specs['wide']}.a1.json", {"status": "ok", "cost_usd": 0.1, "output": {
            "findings": [{"finding_id": wf[0], "status": "needs-wider-scope"}, {"finding_id": wf[1], "status": "not-fixable"}],
            "needs_live_check": False, "not_fixable_reason": "nr",
            "scope_request": {"files": ["scripts/other.py", "tests/test_other.py", "scripts/kpm_hotswap.py"], "reason": "contract in other.py"}}})
        # pendfix: fixer ok + check ok but no review yet -> pending review
        (fd / "reviews" / f"{specs['pendfix']}.a1.review.json").unlink(missing_ok=True)
        # gatefail: attempt 1 and 2 both failed the check
        for n in (1, 2):
            fixer(specs["gatefail"], n, [(f, "fixed") for f in fids("gatefail")])
            chk(specs["gatefail"], n, ok=False)
        fixer(specs["notfix"], 1, [(f, "not-fixable") for f in fids("notfix")])
        fixer(specs["failed"], 1, [], status="invalid")
        fixer(specs["failed"], 2, [], status="invalid")
        # batches: one batch with fixed, live, modified, dropped, partial
        members = ["fixed", "live", "modified", "dropped", "partial"]
        W(work / "batches" / "batch_01.json", {"batch": "batch_01", "tasks": [
            {"task_id": specs[m], "attempt": 1, "patch": "p"} for m in members]})
        W(fd / "integ" / "batch_01.run.json", {"status": "ok", "schema_valid": True, "cost_usd": 0.3, "output": {"series": []}})
        W(fd / "integ" / "batch_01.verify.json", {"ok": True, "gates": {"build": {"status": "pass"}}})
        out = tmp / "out" / "batch_01"
        out.mkdir(parents=True)
        W(out / "series.json", {"batch": "batch_01", "gates_run": [{"cmd": "diffcheck", "exit_code": 0, "summary": "ok"}],
                                "dropped": [{"task_id": specs["dropped"], "reason": "failed build"}],
                                "series": [{"task_id": specs["fixed"], "action": "keep", "file": "0001-a.patch"},
                                           {"task_id": specs["live"], "action": "keep", "file": "0002-b.patch"},
                                           {"task_id": specs["modified"], "action": "modified", "file": "0003-c.patch"},
                                           {"task_id": specs["partial"], "action": "keep", "file": "0004-d.patch"}]})
        # calibration
        calib = {"C000000001": ("seeded-bad", "assert-delete", "reject", "check-fail"),
                 "C000000002": ("seeded-bad", "no-op-fix", "reject", "accept-miss"),
                 "C000000003": ("seeded-bad", "no-op-fix", "reject", "reject"),
                 "C000000004": ("known-good", "good", "accept", "accept"),
                 "C000000005": ("known-good", "good", "accept", "reject")}
        W(work / "calib_key.json", {"items": {k: {"kind": v[0], "mode": v[1], "expected": v[2]} for k, v in calib.items()}})
        for k, v in calib.items():
            chk(k, 1, ok=v[3] != "check-fail")
            rev(k, 1, "accept" if v[3] in ("accept", "accept-miss") else "reject")
        W(work / "human_list.json", {"contested": [{"id": "cc1", "file": "scripts/z.py", "lines": "1", "claim": "c",
                                                    "human_check_steps": ["look"], "specialty": "hook-asm"}],
                                     "human_only_tasks": []})
        W(work / "refuted_skim_pass2.json", {"items": [{"id": "s1", "verdict": "real", "file": "a", "lines": "1", "human_action": "x"},
                                                        {"id": "s2", "verdict": "refute-holds", "file": "a", "lines": "1"}]})
        env = dict(os.environ, FIX_DIR=str(fd), FIX_WORK=str(work), KOTOR2_REVIEW_HOME=str(tmp / "rh"),
                   KOTOR2_FIX_OUT=str(tmp / "out"), KOTOR2_FIX_REPORT=str(tmp / "rep"))
        me = [sys.executable, "-I", str(Path(__file__).resolve()), "--out-dir", str(tmp / "rep")]
        r = subprocess.run(me, env=env, capture_output=True, text=True)
        if r.returncode != 1 or "run_fix_review.sh --only " + specs["pendfix"] not in r.stdout \
                or "run_fix.sh --only " + specs["revise1"] not in r.stdout:
            bad.append("expected exit 1 with re-queue lines: " + r.stdout + r.stderr)
        rep = json.loads((tmp / "rep" / "fix_report.json").read_text())
        st = {e["task_id"]: e["status"] for e in rep["tasks"]}
        want = {"fixed": "fixed+tested", "live": "fixed+needs-live-check", "modified": "fixed+tested",
                "rejected": "rejected", "classhuman": "human-only", "gatefail": "human-only", "notfix": "not-fixable",
                "failed": "failed", "dropped": "human-only", "partial": "fixed+tested", "pendfix": "pending",
                "revise1": "pending", "wide": "needs-wider-scope", "revise3": "rejected"}
        for nm, w in want.items():
            if st.get(specs[nm]) != w:
                bad.append(f"{nm}: {st.get(specs[nm])} != {w}")
        fst = {f["id"]: f["status"] for f in rep["findings"]}
        if fst.get(fids("partial")[1]) != "human-only" or fst.get(fids("partial")[0]) != "fixed+tested":
            bad.append(f"per-finding contested: {fst}")
        if fst.get("cc1") != "human-only":
            bad.append("verify-contested finding missing")
        ca = rep["calibration"]
        if ca["seeded_bad"] != {"n": 3, "caught": 2, "catch_rate": 0.6667} or ca["known_good"]["false_rejects"] != 1:
            bad.append(f"calibration {ca['seeded_bad']} {ca['known_good']}")
        if rep["sonnet_override"]["modified"] != 1 or rep["sonnet_override"]["drop"] != 1 \
                or rep["sonnet_override"]["accepted_and_integrated"] != 5:
            bad.append(f"override {rep['sonnet_override']}")
        if rep["cost"]["fix"]["calls"] < 8 or abs(rep["cost"]["total_usd"] - rep["cost"]["review"]["cost_usd"]
                                                  - rep["cost"]["fix"]["cost_usd"] - rep["cost"]["integrate"]["cost_usd"]) > 1e-9:
            bad.append(f"cost {rep['cost']}")
        if [x["id"] for x in rep["human_lists"]["refuted_skim_pass2"]] != ["s1"]:
            bad.append("skim pass2 list")
        nw = rep["human_lists"]["needs_wider_scope"]
        flags = {x["path"]: x["flags"] for x in nw[0]["files"]} if len(nw) == 1 else {}
        if flags.get("scripts/other.py") != [] or flags.get("scripts/kpm_hotswap.py") != ["never-edit"] \
                or flags.get("tests/test_other.py") != ["test-file (tests-weakening risk)"] or nw[0]["reason"] != "contract in other.py":
            bad.append(f"needs_wider_scope list: {nw}")
        md = (tmp / "rep" / "fix_report.md").read_text()
        if "requested: `scripts/other.py`" not in md or "FLAG: never-edit" not in md:
            bad.append("needs-wider-scope md section")
        rv3 = [e for e in rep["tasks"] if e["task_id"] == specs["revise3"]][0]
        if rv3["reason"] != "still revise after 1 revision round(s) (last round a3)":
            bad.append(f"revise3 reason: {rv3['reason']}")
        if {f["status"] for f in rep["findings"] if f["task_id"] == specs["wide"]} != {"needs-wider-scope"}:
            bad.append("wide finding statuses")
        env3 = dict(env, FIX_REVISE_ROUNDS="3", KOTOR2_FIX_REPORT=str(tmp / "rep3"))
        subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), "--out-dir", str(tmp / "rep3")], env=env3,
                       capture_output=True, text=True)
        rep3 = json.loads((tmp / "rep3" / "fix_report.json").read_text())
        e3 = [e for e in rep3["tasks"] if e["task_id"] == specs["revise3"]][0]
        if e3["status"] != "pending" or e3["stage"] != "fix":
            bad.append(f"R=3: a3 revise should stay pending: {e3['status']} {e3['reason']}")
        first = {p.name: p.read_bytes() for p in (tmp / "rep").iterdir()}
        subprocess.run(me, env=env, capture_output=True, text=True)
        if first != {p.name: p.read_bytes() for p in (tmp / "rep").iterdir()}:
            bad.append("not deterministic")
        # complete the pending ones -> exit 0
        rev(specs["pendfix"], 1, "reject")
        fixer(specs["revise1"], 2, [(f, "fixed") for f in fids("revise1")])
        chk(specs["revise1"], 2)
        rev(specs["revise1"], 2, "reject")
        r = subprocess.run(me, env=env, capture_output=True, text=True)
        if r.returncode != 0:
            bad.append("complete data should exit 0: " + r.stdout)
        # an unreviewed calibration item is a gap
        (fd / "reviews" / "C000000004.a1.review.json").unlink()
        r = subprocess.run(me, env=env, capture_output=True, text=True)
        if r.returncode != 1 or "C000000004" not in r.stdout:
            bad.append("unreviewed calibration must exit 1")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    print("fix_merge self-test:", "PASS" if not bad else "FAIL " + " | ".join(bad))
    return 1 if bad else 0


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return self_test()
    if argv[:1] in (["-h"], ["--help"]):
        print(__doc__)
        return 0
    out = fc.REPORT_OUT
    if argv[:1] == ["--out-dir"] and len(argv) == 2:
        out = Path(argv[1]).resolve()
    elif argv:
        print("usage: fix_merge.py [--out-dir DIR] | --self-test", file=sys.stderr)
        return 2
    return run(out)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
