#!/usr/bin/env python3
"""Prompt renderers for the Fix pipeline (stdlib only, no LLM calls). Contract: ../INTERFACES.md "Fix stage".

Usage (prompt on stdout):
  fix_prompts.py fixer <tid> --attempt N [--error-file F] [--revise-file F] [--root DIR]
  fix_prompts.py fixer-system <tid>
  fix_prompts.py reviewer <tid> --attempt N [--root DIR]
  fix_prompts.py reviewer-system <tid>
  fix_prompts.py integrator <batch>
  fix_prompts.py integrator-system
  fix_prompts.py sizes <tid> --attempt N          prompt byte sizes as JSON (fixer + reviewer when renderable)
  fix_prompts.py --self-test

Isolation rules (self-tested):
  * the reviewer prompt holds the findings, the harness-made patch and +-15 lines of post-image; never the
    fixer's rationale / test_plan / notes / status (results/<tid>.aN.json is never opened here);
  * work/fix/calib_key.json is never opened by this module (calibration ids look like real ids; the task
    entries of calibration items live in work/fix/calib_tasks.json and are merged by task_any());
  * no excluded path (secret patterns) is ever rendered.
Excerpt source for the fixer: the task copy $FIX_HOME/tasks/<tid>.aN (falls back to base/, then the pinned tree,
so --dry-run works before any copy exists). Reviewer: $FIX_HOME/tasks/<tid>.aN.review.
"""
from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402

CALIB_TASKS = fc.WORK / "calib_tasks.json"      # task entries of calibration items (NOT the answer key)
EXCERPT_FIXER = 20
EXCERPT_REVIEW = 15
MAX_ERR_BYTES = 3072
MAX_PATCH_BYTES = 24 * 1024
MAX_EXCERPT_LINES = 140                          # per merged interval, after widening

FLAG_TEXT = {
    "relocate": "relocate: the cited line numbers of at least one finding may be wrong. The claim text names the "
                "true place. Grep for its identifiers, confirm the real lines, and report them in actual_lines.",
    "needs_live_check": "needs_live_check: this touches code that only the running game can confirm. Set "
                        "needs_live_check true in your output.",
    "gate_semantics": "gate_semantics: the code decides what a checker or gate accepts. Do not change what it "
                      "accepts or rejects beyond what the claim spells out; otherwise answer not-fixable.",
    "sandbox_tests_only": "sandbox_tests_only: the tests of this file run only inside the harness sandbox; you "
                          "cannot run anything yourself.",
}


# ---------------------------------------------------------------------------------------------------------
def task_any(tid: str) -> dict:
    """Task from fix_queue.json, else from work/fix/calib_tasks.json (calibration items). Env-free."""
    q = fc.rj(fc.QUEUE)
    for t in (q or {}).get("tasks") or []:
        if t.get("task_id") == tid:
            return t
    c = fc.rj(CALIB_TASKS)
    for t in (c or {}).get("tasks") or []:
        if t.get("task_id") == tid:
            return t
    sys.exit(f"fix_prompts: unknown task {tid}")


def all_task_ids() -> list[str]:
    ids = [t["task_id"] for t in ((fc.rj(fc.QUEUE) or {}).get("tasks") or [])]
    ids += [t["task_id"] for t in ((fc.rj(CALIB_TASKS) or {}).get("tasks") or [])]
    return ids


def _root_for(tid: str, attempt: int, kind: str, override: str | None) -> Path:
    if override:
        return Path(override)
    cands = [fc.TASKS / (f"{tid}.a{attempt}" + (".review" if kind == "review" else "")), fc.BASE, fc.TREE]
    for c in cands:
        if c.is_dir():
            return c
    return cands[0]


def _num(lines: list[str], lo: int, hi: int) -> str:
    return "\n".join(f"{n:>6} | {lines[n - 1]}" for n in range(lo, hi + 1))


def _merge(rngs: list[tuple[int, int]]) -> list[tuple[int, int]]:
    out: list[list[int]] = []
    for lo, hi in sorted(rngs):
        if out and lo <= out[-1][1] + 1:
            out[-1][1] = max(out[-1][1], hi)
        else:
            out.append([lo, hi])
    return [(a, b) for a, b in out]


def excerpts(ranges: dict[str, list[tuple[int, int]]], root: Path, pad: int, title: str) -> str:
    """ranges: {rel: [(lo, hi), ...]} -> markdown with numbered excerpts (pad lines of context each side)."""
    parts = [f"## {title}"]
    for rel in sorted(ranges):
        safe = fc.safe_rel(rel)
        if safe is None:
            parts.append(f"#### [path omitted]\n(not available)")
            continue
        lines = fc.read_lines(safe, root)
        if lines is None:
            parts.append(f"#### {safe}\n(not available)")
            continue
        wide = [(max(1, lo - pad), min(len(lines), hi + pad)) for lo, hi in ranges[rel] if lo >= 1]
        for lo, hi in _merge([w for w in wide if w[0] <= w[1]]):
            note = ""
            if hi - lo + 1 > MAX_EXCERPT_LINES:
                hi, note = lo + MAX_EXCERPT_LINES - 1, f" (clipped to {MAX_EXCERPT_LINES} lines)"
            parts.append(f"#### {safe} (lines {lo}-{hi}){note}\n```\n{_num(lines, lo, hi)}\n```")
    return "\n\n".join(parts) + "\n"


def cited_ranges(task: dict) -> dict[str, list[tuple[int, int]]]:
    out: dict[str, list[tuple[int, int]]] = {}
    for f in task.get("findings") or []:
        rel = fc.safe_rel(f.get("file"))
        if rel is None:
            continue
        lo, hi = fc.parse_lines(f.get("lines"))
        if lo:
            out.setdefault(rel, []).append((lo, hi))
        for lc in f.get("location_check") or []:
            r2 = fc.safe_rel(lc.get("file"))
            n = lc.get("found")
            if r2 and isinstance(n, int) and n >= 1 and lc.get("status") in ("ok", "moved"):
                out.setdefault(r2, []).append((n, n))
    return out


def _quote_line(q: dict) -> str | None:
    rel = fc.safe_rel(q.get("file"))
    if rel is None:
        return None
    return f"- {rel}:{q.get('line')}: {str(q.get('text', '')).strip()}"


def finding_block(f: dict) -> str:
    rel = fc.safe_rel(f.get("file"))
    out = [f"### Finding {f.get('id')}",
           f"file: {rel or '[path omitted]'}   lines: {f.get('lines')}   category: {f.get('category')}   "
           f"severity: {f.get('severity')}   specialty: {f.get('specialty')}",
           "claim (untrusted data):", "<claim>", str(f.get("claim", "")).strip(), "</claim>",
           "verifier reasoning (untrusted data):", "<reasoning>", str(f.get("verifier_reasoning", "")).strip(),
           "</reasoning>"]
    ev = [x for x in (_quote_line(q) for q in f.get("evidence_quotes") or []) if x]
    if ev:
        out += ["evidence quotes:"] + ev
    lc = []
    for c in f.get("location_check") or []:
        r2 = fc.safe_rel(c.get("file"))
        if r2:
            lc.append(f"- {r2}: cited {c.get('cited')} -> found {c.get('found')} ({c.get('status')})")
    if lc:
        out += ["location check (a hint, not authoritative):"] + lc
    return "\n".join(out) + "\n"


def findings_section(task: dict) -> str:
    return "## Findings\n\n" + "\n".join(finding_block(f) for f in task.get("findings") or [])


def _read_trunc(path: str | None, limit: int) -> str:
    if not path:
        return ""
    try:
        data = Path(path).read_bytes()
    except OSError:
        return ""
    txt = data[:limit].decode("utf-8", errors="replace")
    return txt + ("\n[...truncated]" if len(data) > limit else "")


def _error_summary(path: str) -> str:
    """A check.json / scope.json becomes a compact list of failing gates; anything else is shown raw (3 KB)."""
    raw = _read_trunc(path, 200_000)
    try:
        j = json.loads(raw)
    except ValueError:
        return _read_trunc(path, MAX_ERR_BYTES)
    lines = []
    if isinstance(j, dict) and isinstance(j.get("gates"), dict):
        for g, v in sorted(j["gates"].items()):
            if isinstance(v, dict) and v.get("status") == "fail":
                lines.append(f"- gate {g}: FAIL: {str(v.get('detail', '')).strip()}")
        if j.get("new_failures"):
            lines.append("- new test failures: " + json.dumps(j["new_failures"], sort_keys=True))
    if isinstance(j, dict) and j.get("violations"):
        for v in j["violations"]:
            lines.append(f"- scope violation: {v.get('path')} ({v.get('reason')})")
    if isinstance(j, dict) and j.get("over_cap"):
        lines.append(f"- patch too large: {j.get('lines_changed')} changed lines, cap {j.get('line_cap')}")
    text = "\n".join(lines) if lines else json.dumps(j, sort_keys=True, indent=1)
    return text[:MAX_ERR_BYTES] + ("\n[...truncated]" if len(text) > MAX_ERR_BYTES else "")


# ---------------------------------------------------------------------------------------------------------
def fixer_prompt(tid: str, attempt: int, error_file: str | None = None, revise_file: str | None = None,
                 root: str | None = None) -> str:
    t = task_any(tid)
    allowed = [a for a in (fc.safe_rel(x) for x in t.get("allowed_files") or []) if a]
    ctx = [g for g in t.get("context_globs") or [] if fc.safe_rel(g.replace("*", "x")) is not None]
    flags = t.get("flags") or {}
    head = [f"# Fix task", f"task_id: {tid}", f"class: {t.get('class')}", f"specialty: {t.get('specialty')}",
            f"line cap: at most {t.get('line_cap')} changed lines (added + removed) over all files",
            "", "Allowed files (editable, relative to your working directory):"]
    if t.get("style"):                           # ladder variant: style only; `parent` is never rendered
        head.insert(4, f"style: {t['style']}")
    head += [f"- {a}   (editable)" for a in allowed]
    head += ["", "Read-only context (do not edit): " + (", ".join(ctx) or "(none)")]
    on = [FLAG_TEXT[k] for k in sorted(flags) if flags.get(k) and k in FLAG_TEXT]
    if on:
        head += ["", "Flags:"] + [f"- {x}" for x in on]
    head += ["", f"This task covers {len(t.get('findings') or [])} finding(s); answer for each finding id.", ""]
    parts = ["\n".join(head), findings_section(t),
             excerpts(cited_ranges(t), _root_for(tid, attempt, "fix", root), EXCERPT_FIXER,
                      "Source excerpts (your working copy; line numbers are real)")]
    if error_file:
        parts.append("## Previous attempt failed checks:\n"
                     "Your previous patch was rejected by the deterministic gates. Fix the cause and make the "
                     "smallest correct patch again.\n\n```\n" + _error_summary(error_file) + "\n```\n")
    if revise_file:
        parts.append("## Reviewer asked for changes:\n"
                     "Your working copy ALREADY contains the previous patch. Apply the requested change on top of "
                     "it; do not undo the rest.\n\n<review>\n" + _read_trunc(revise_file, MAX_ERR_BYTES)
                     + "\n</review>\n")
    parts.append("Return the structured output now (task_id " + tid + ").\n")
    return "\n".join(parts)


def _note(kind: str, specialty: str) -> str:
    p = fc.CODE_DIR / "specialty_notes" / kind / f"{specialty}.md"
    try:
        return p.read_text(encoding="utf-8")
    except OSError:
        return ""


def fixer_system(tid: str) -> str:
    t = task_any(tid)
    base = (fc.CODE_DIR / "fixer_prompt.md").read_text(encoding="utf-8")
    note = _note("fixer", str(t.get("specialty")))
    out = base.rstrip() + ("\n\n## Specialty notes: " + str(t.get("specialty")) + "\n" + note.strip() + "\n" if note else "\n")
    if t.get("style"):
        snote = _note("style", str(t["style"]))
        if snote.strip():
            out = out.rstrip() + "\n\n## Approach style: " + str(t["style"]) + "\n" + snote.strip() + "\n"
    return out


def reviewer_system(tid: str) -> str:
    t = task_any(tid)
    base = (fc.CODE_DIR / "reviewer_prompt.md").read_text(encoding="utf-8")
    note = _note("reviewer", str(t.get("specialty")))
    return base.rstrip() + ("\n\n## Specialty notes: " + str(t.get("specialty")) + "\n" + note.strip() + "\n" if note else "\n")


def patch_new_ranges(patch: str) -> dict[str, list[tuple[int, int]]]:
    """Post-image line ranges of every hunk, keyed by the b/ path."""
    out: dict[str, list[tuple[int, int]]] = {}
    cur = None
    for ln in patch.splitlines():
        if ln.startswith("+++ "):
            p = ln[4:].split("\t")[0].strip()
            cur = p[2:] if p.startswith("b/") else (None if p == "/dev/null" else p)
        elif ln.startswith("@@") and cur:
            m = re.match(r"@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@", ln)
            if m:
                start, cnt = int(m.group(1)), int(m.group(2) or 1)
                out.setdefault(cur, []).append((start, max(start, start + cnt - 1)))
    return out


def reviewer_prompt(tid: str, attempt: int, root: str | None = None) -> str:
    t = task_any(tid)
    pf = fc.PATCHES / f"{tid}.a{attempt}.patch"
    try:
        patch = pf.read_text(encoding="utf-8", errors="replace")
    except OSError:
        sys.exit(f"fix_prompts: patch missing: {pf}")
    sc = fc.rj(fc.PATCHES / f"{tid}.a{attempt}.scope.json") or {}
    allowed = [a for a in (fc.safe_rel(x) for x in t.get("allowed_files") or []) if a]
    head = ["# Patch review", f"task_id: {tid}", f"class: {t.get('class')}", f"specialty: {t.get('specialty')}",
            f"line cap: {t.get('line_cap')}; patch size: +{sc.get('lines_added', '?')} -{sc.get('lines_removed', '?')}",
            "Files the patch may touch: " + ", ".join(allowed), ""]
    shown = patch if len(patch.encode()) <= MAX_PATCH_BYTES else patch.encode()[:MAX_PATCH_BYTES].decode("utf-8", "replace") + "\n[...patch truncated]\n"
    ranges = {k: v for k, v in patch_new_ranges(patch).items() if fc.safe_rel(k)}
    parts = ["\n".join(head), findings_section(t),
             "## Patch (unified diff; pre-image is under _pre/<path> in your working directory)\n```diff\n"
             + shown.rstrip("\n") + "\n```\n",
             excerpts(ranges, _root_for(tid, attempt, "review", root), EXCERPT_REVIEW,
                      "Post-image excerpts around every changed hunk (your working copy)"),
             f"Return the structured output now (task_id {tid}). Try to refute the patch.\n"]
    return "\n".join(parts)


def integrator_prompt(batch: str) -> str:
    if not re.fullmatch(r"batch_[0-9]{2}", batch):
        sys.exit("fix_prompts: bad batch id")
    dossier = (fc.BATCHES / f"{batch}.md")
    try:
        body = dossier.read_text(encoding="utf-8")
    except OSError:
        sys.exit(f"fix_prompts: dossier missing: {dossier}")
    gate = fc.FIX_HOME / "bin" / f"fixgate_{batch}.sh"
    return (f"# Integrate {batch}\nFIXGATE = {gate}\n(Use exactly this absolute path as the only Bash command, "
            f"followed by one subcommand.)\n\n{body}\n\nReturn the structured output now (batch {batch}).\n")


def integrator_system() -> str:
    return (fc.CODE_DIR / "integrator_prompt.md").read_text(encoding="utf-8")


# ---------------------------------------------------------------------------------------------------------
def _args(argv: list[str]) -> tuple[list[str], dict]:
    pos, opt, i = [], {}, 0
    while i < len(argv):
        a = argv[i]
        if a in ("--attempt", "--error-file", "--revise-file", "--root"):
            if i + 1 >= len(argv):
                sys.exit(f"fix_prompts: {a} needs a value")
            opt[a[2:].replace("-", "_")] = argv[i + 1]
            i += 2
        else:
            pos.append(a)
            i += 1
    return pos, opt


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return self_test()
    if argv[:1] == ["--self-test-run"]:
        return self_test_run()
    if len(argv) < 2:
        print(__doc__)
        return 2
    cmd = argv[0]
    pos, opt = _args(argv[1:])
    if not pos:
        print(__doc__)
        return 2
    ident = pos[0]
    if cmd in ("fixer", "reviewer", "sizes"):
        if not fc.TASK_ID_RE.match(ident):
            sys.exit("fix_prompts: bad task id")
        if "attempt" not in opt or not opt["attempt"].isdigit():
            sys.exit("fix_prompts: --attempt N required")
    if cmd == "fixer":
        sys.stdout.write(fixer_prompt(ident, int(opt["attempt"]), opt.get("error_file"), opt.get("revise_file"), opt.get("root")))
    elif cmd == "fixer-system":
        sys.stdout.write(fixer_system(ident))
    elif cmd == "reviewer":
        sys.stdout.write(reviewer_prompt(ident, int(opt["attempt"]), opt.get("root")))
    elif cmd == "reviewer-system":
        sys.stdout.write(reviewer_system(ident))
    elif cmd == "integrator":
        sys.stdout.write(integrator_prompt(ident))
    elif cmd == "integrator-system":
        sys.stdout.write(integrator_system())
    elif cmd == "sizes":
        n = int(opt["attempt"])
        res = {"fixer_user": len(fixer_prompt(ident, n, root=opt.get("root")).encode()),
               "fixer_system": len(fixer_system(ident).encode())}
        if (fc.PATCHES / f"{ident}.a{n}.patch").exists():
            res |= {"reviewer_user": len(reviewer_prompt(ident, n, opt.get("root")).encode()),
                    "reviewer_system": len(reviewer_system(ident).encode())}
        sys.stdout.write(fc.dumps(res))
    else:
        print(__doc__)
        return 2
    return 0


# ---------------------------------------------------------------------------------------------------------
def self_test() -> int:
    """Re-exec in a subprocess with temp FIX_*/KOTOR2_REVIEW_HOME so nothing real is read or written."""
    import subprocess
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        env = dict(os.environ, FIX_DIR=str(Path(td) / "data"), FIX_WORK=str(Path(td) / "work"),
                   KOTOR2_REVIEW_HOME=str(Path(td) / "home"), FIX_SELFTEST_TMP=td)
        r = subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), "--self-test-run"], env=env)
        return r.returncode


def self_test_run() -> int:
    td = Path(os.environ["FIX_SELFTEST_TMP"])
    bad: list[str] = []

    def check(c, m):
        if not c:
            bad.append(m)

    assert fc.FIX_DIR == (td / "data").resolve() and fc.WORK == (td / "work").resolve(), "env override not applied"
    base = fc.FIX_HOME / "base"
    src = ["# header", "def a():", "    return 1", "", "def b(x):", "    if x < 3:", "        return x", "    return 0"] \
        + [f"line{i}" for i in range(9, 80)]
    (base / "scripts").mkdir(parents=True)
    (base / "scripts" / "foo.py").write_text("\n".join(src) + "\n")
    # review copy of attempt 1 for T0123456789 (post-image has `x <= 3`)
    post = list(src)
    post[5] = "    if x <= 3:"
    rc = fc.TASKS / "T0123456789.a1.review"
    (rc / "scripts").mkdir(parents=True)
    (rc / "scripts" / "foo.py").write_text("\n".join(post) + "\n")
    finding = {"id": "f00000000001", "file": "scripts/foo.py", "lines": "6-7", "category": "logic", "specialty": "python-tooling",
               "severity": "medium", "claim": "Boundary is wrong: x < 3 should include 3.", "verifier_reasoning": "Verified by reading line 6.",
               "evidence_quotes": [{"file": "scripts/foo.py", "line": 6, "text": "    if x < 3:"},
                                   {"file": "tests/static/raw_addresses.toml", "line": 1, "text": "SECRETLEAK"}],
               "location_check": [{"file": "scripts/foo.py", "cited": 6, "found": 6, "status": "ok"}]}
    task = {"task_id": "T0123456789", "class": "auto-fixable", "specialty": "python-tooling", "flags": {"relocate": True},
            "allowed_files": ["scripts/foo.py"], "context_globs": ["scripts/**"], "line_cap": 40, "findings": [finding]}
    fc.write_json(fc.QUEUE, {"tasks": [task]})
    calib_task = dict(task, task_id="T0a0b0c0d0e", findings=[dict(finding, id="f00000000002")])
    fc.write_json(CALIB_TASKS, {"tasks": [calib_task]})
    key = {"salt": "SALT-ZZ-9f8e7d6c5b4a-unique", "items": {"T0a0b0c0d0e": {"kind": "seeded-bad", "mode": "off-by-one-zz",
           "source_task": "T0123456789", "expected": "reject-zz"}}}
    fc.write_json(fc.CALIB_KEY, key)
    patch = ("--- a/scripts/foo.py\n+++ b/scripts/foo.py\n@@ -3,6 +3,6 @@\n     return 1\n \n def b(x):\n-    if x < 3:\n+    if x <= 3:\n"
             "         return x\n     return 0\n \n")
    fc.PATCHES.mkdir(parents=True)
    for t in ("T0123456789", "T0a0b0c0d0e"):
        (fc.PATCHES / f"{t}.a1.patch").write_text(patch)
        fc.write_json(fc.PATCHES / f"{t}.a1.scope.json", {"lines_added": 1, "lines_removed": 1})
    # results with a rationale that must never leak into the reviewer prompt
    fc.RESULTS.mkdir(parents=True)
    RAT = "RATIONALE-LEAK-MARKER-31337 test plan NOTES-LEAK"
    fc.write_json(fc.RESULTS / "T0123456789.a1.json", {"output": {"rationale": RAT, "test_plan": "TESTPLAN-LEAK-777", "status": "fixed"}})
    errf = td / "err.json"
    errf.write_text(json.dumps({"gates": {"build": {"status": "fail", "detail": "E" * 5000}, "apply": {"status": "pass", "detail": ""}}}))
    revf = td / "rev.txt"
    revf.write_text("verdict revise: keep the strict comparison")

    fp = fixer_prompt("T0123456789", 1, str(errf), str(revf), str(base))
    rp = reviewer_prompt("T0123456789", 1)
    rp2 = reviewer_prompt("T0a0b0c0d0e", 1)
    fp2 = fixer_prompt("T0a0b0c0d0e", 1, root=str(base))
    sysf, sysr = fixer_system("T0123456789"), reviewer_system("T0123456789")
    vtask = dict(task, task_id="T0b1b1b1b1b", style="minimal", parent="T0123456789")
    fc.write_json(fc.QUEUE, {"tasks": [task, vtask]})
    vfp, vsys = fixer_prompt("T0b1b1b1b1b", 1, root=str(base)), fixer_system("T0b1b1b1b1b")
    check("## Approach style: minimal" in vsys and "smallest diff" in vsys, "style note in styled fixer system prompt")
    check("## Approach style" not in sysf and "style:" not in fp, "no style note / line for unstyled task")
    check("style: minimal" in vfp, "style line in styled fixer prompt")
    check("T0123456789" not in vfp.replace("T0b1b1b1b1b", "") and "T0123456789" not in vsys and "judge_key" not in vfp + vsys,
          "variant prompts leak neither parent id nor judge_key")
    check("Allowed files" in fp and "scripts/foo.py   (editable)" in fp and "line cap" in fp, "fixer header")
    check("relocate" in fp and "     6 |     if x < 3:" in fp, "flags and numbered excerpt")
    check("Previous attempt failed checks:" in fp and "gate build: FAIL" in fp and len(fp.split("Previous attempt failed checks:")[1]) < 4200,
          "error section present and truncated")
    check("Reviewer asked for changes:" in fp and "keep the strict comparison" in fp, "revise section")
    check("[...truncated]" in fp, "error truncation marker")
    check(fp.count("### Finding f00000000001") == 1, "finding block")
    check("def a():" in fp and "line9" in fp, "+-20 context")
    check("SECRETLEAK" not in fp + rp and "raw_addresses" not in fp + rp + rp2 + fp2, "excluded path never rendered")
    check("+    if x <= 3:" in rp and "def b(x):" in rp, "reviewer holds the patch")
    check("    if x <= 3:" in rp.split("Post-image excerpts")[1], "reviewer post-image excerpt (review copy)")
    check("line20" in rp and "line9" in rp and "line30" not in rp, "reviewer +-15 window")
    for name, p in (("reviewer", rp), ("calib reviewer", rp2), ("reviewer system", sysr)):
        for leak in (RAT, "TESTPLAN-LEAK", "NOTES-LEAK", "rationale", "not-fixable"):
            check(leak not in p, f"{name} leaks {leak!r}")
    check("test_plan" not in rp and "needs_live_check" not in rp, "no fixer output fields in reviewer prompt")
    # nothing from calib_key.json in any rendered text
    static = ""
    for p in list((fc.CODE_DIR).glob("*.md")) + list((fc.CODE_DIR / "specialty_notes").rglob("*.md")) + [fc.CODE_DIR / "fix_schema.json", fc.CODE_DIR / "review_schema.json"]:
        static += p.read_text(encoding="utf-8", errors="replace")
    strings = []

    def walk(o):
        if isinstance(o, dict):
            for k, v in o.items():
                walk(k)
                walk(v)
        elif isinstance(o, str):
            strings.append(o)
    walk(key)
    # a task's own id is its identity in its own prompt (calibration ids look like real ids); drop it before grepping
    rendered = "\n".join([fp, fp2, rp, rp2, sysf, sysr, fixer_system("T0a0b0c0d0e"), reviewer_system("T0a0b0c0d0e")])
    rendered = rendered.replace("T0a0b0c0d0e", "").replace("T0123456789", "")
    for s in strings:
        if s in ("T0123456789", "T0a0b0c0d0e"):
            continue
        if s not in static:
            check(s not in rendered, f"calib key string leaked: {s!r}")
    check(rp2.count("T0a0b0c0d0e") >= 1, "calibration task rendered like a real one")
    # integrator
    fc.BATCHES.mkdir(parents=True)
    (fc.BATCHES / "batch_01.md").write_text("DOSSIER BODY\n")
    ip = integrator_prompt("batch_01")
    check("DOSSIER BODY" in ip and str(fc.FIX_HOME / "bin" / "fixgate_batch_01.sh") in ip, "integrator prompt")
    sizes = {"fixer_user": len(fp.encode()), "fixer_system": len(sysf.encode()), "reviewer_user": len(rp.encode()),
             "reviewer_system": len(sysr.encode()), "integrator": len(ip.encode())}
    print("fix_prompts self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad), json.dumps(sizes, sort_keys=True))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
