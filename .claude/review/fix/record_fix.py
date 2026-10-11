#!/usr/bin/env python3
"""Fix-stage recorder + task-state engine (no LLM calls). Contract: ../INTERFACES.md "Fix stage".

Recording (reuses tools/record_run.py validate/parse_raw/structured and record_verify.count_tool_calls):
  record_fix.py fix       <tid>.a<N> <raw_out> <raw_err> <rc> <attempts> <rl_hits> <started> <dur> <prompt>  -> results/<tid>.aN.json
  record_fix.py review    <tid>.a<N> ...same...                                       -> reviews/<tid>.aN.review.json
  record_fix.py integrate <batch>    ...same...                                       -> integ/<batch>.run.json
  A review's quotes are checked against the review copy tasks/<tid>.aN.review (whitespace-normalized text within
  +-3 lines, quotes < 6 chars are trivial); any fabricated quote makes the review status "invalid" (re-queue).

State (used by run_fix.sh / run_fix_review.sh / fix_calibrate.py):
  outcome <tid> <n>        none|error|notfixed|needs-diff|empty|scope|needs-check|check-ok|check-failed
  next <tid> [--revise]    done | fix <n> plain|error|revise <base> | finish <n>
                           (revise rounds: a3..a(2+R), R = $FIX_REVISE_ROUNDS, default 1; round k revises k-1 or the base)
  error-file <tid> <n>     path of check.json (or scope.json) to feed back to the fixer
  revise-text <tid> <base> reviewer instructions for a revise attempt (stdout)
  review-attempt <tid>     latest attempt with check ok + non-empty patch (empty output if none)
  pending --stage fix|review [--class A,B] [--wave N] [--only ids] [--revise] [--exclude-file F] [--chunk P] [--queue-file F]
  info <tid>               class<TAB>specialty<TAB>comma-separated allowed files<TAB>style (empty unless a ladder variant)
  rl-hits --since EPOCH <tid>...   rate-limit hits of results written since EPOCH
  summary fix|review       counts
  --self-test
"""
from __future__ import annotations

import json
import os
import sys
import re
import shutil
import time
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402
import fix_prompts as fp  # noqa: E402
import record_run as rr  # noqa: E402  (fcommon appended tools/ to sys.path)

try:
    from record_verify import count_tool_calls  # noqa: E402
except Exception:                                  # pragma: no cover - fallback keeps this script standalone
    def count_tool_calls(text: str) -> int:
        seen = set()
        for line in text.splitlines():
            try:
                ev = json.loads(line)
            except json.JSONDecodeError:
                continue
            if ev.get("type") == "assistant":
                for blk in (ev.get("message") or {}).get("content") or []:
                    if isinstance(blk, dict) and blk.get("type") == "tool_use":
                        seen.add(blk.get("id") or f"_{len(seen)}")
        return len(seen)

SCHEMAS = {"fix": "fix_schema.json", "review": "review_schema.json", "integrate": "integrate_schema.json"}
MIN_QUOTE = 6
WINDOW = 3


# ---------------------------------------------------------------------------------------------------------
# quote check (reviewer quotes vs review copy)
def _norm(s) -> str:
    return " ".join(str(s or "").split())


def check_quotes(quotes: list, root: Path) -> dict:
    cache: dict = {}
    res = {"total": 0, "ok": 0, "fabricated": [], "trivial": []}
    for q in quotes or []:
        res["total"] += 1
        if not isinstance(q, dict):
            res["fabricated"].append({"reason": "not an object"})
            continue
        f, line, text = q.get("file"), q.get("line"), q.get("text")
        rec = {"file": f, "line": line, "text": str(text)[:160]}
        rel = fc.safe_rel(f) if isinstance(f, str) else None
        if rel is None or isinstance(line, bool) or not isinstance(line, int) or not isinstance(text, str):
            res["fabricated"].append(dict(rec, reason="malformed quote or unsafe/excluded path"))
            continue
        if rel not in cache:
            cache[rel] = fc.read_lines(rel, root)
        lines = cache[rel]
        if lines is None:
            res["fabricated"].append(dict(rec, reason="file not found in review copy"))
            continue
        want = _norm(text)
        if len(want) < MIN_QUOTE:
            res["trivial"].append(dict(rec, reason="quote shorter than 6 characters"))
            continue
        if line < 1 or line > len(lines):
            res["fabricated"].append(dict(rec, reason=f"line {line} outside file (1-{len(lines)})"))
            continue
        extra = max(0, len(text.strip().splitlines()) - 1)
        lo, hi = max(1, line - WINDOW), min(len(lines), line + WINDOW + extra)
        if want in _norm("\n".join(lines[lo - 1:hi])):
            res["ok"] += 1
        else:
            res["fabricated"].append(dict(rec, reason=f"text not found within lines {lo}-{hi}"))
    return res


# ---------------------------------------------------------------------------------------------------------
def dest_for(kind: str, ident: str) -> tuple[Path, str | None, int | None]:
    if kind == "integrate":
        return fc.INTEG / f"{ident}.run.json", None, None
    tid, _, a = ident.rpartition(".a")
    if not fc.TASK_ID_RE.match(tid) or not a.isdigit():
        sys.exit(f"record_fix: id must be <tid>.a<N>, got {ident!r}")
    if kind == "fix":
        return fc.RESULTS / f"{tid}.a{a}.json", tid, int(a)
    return fc.REVIEWS / f"{tid}.a{a}.review.json", tid, int(a)


def record(kind: str, ident: str, raw_out: str, raw_err: str, rc: str, attempts: str, rl_hits: str,
           started: str, dur: str, prompt_file: str) -> dict:
    dest, tid, att = dest_for(kind, ident)
    schema = json.loads((fc.CODE_DIR / SCHEMAS[kind]).read_text())
    text = Path(raw_out).read_text(errors="replace") if Path(raw_out).exists() else ""
    result, turns = rr.parse_raw(text)
    out = rr.structured(result) if result else None
    errs = rr.validate(out, schema) if out is not None else ["no structured output"]
    warns: list[str] = []
    quote_check = None
    if isinstance(out, dict):
        if kind in ("fix", "review"):
            if out.get("task_id") != tid:
                errs.append(f"task_id mismatch: {out.get('task_id')!r}")
            try:
                want = {f["id"] for f in fp.task_any(tid).get("findings") or []}
            except SystemExit:
                want = set()
            rows = out.get("findings") if kind == "fix" else out.get("per_finding")
            got = {r.get("finding_id") for r in rows or [] if isinstance(r, dict)}
            if want and got != want:
                warns.append(f"finding ids differ: missing {sorted(want - got)} unexpected {sorted(got - want)}")
        if kind == "fix":
            for p in out.get("files_changed") or []:
                if fc.safe_rel(p) is None:
                    warns.append(f"files_changed has an unsafe path: {str(p)[:80]}")
            wide = any(isinstance(f, dict) and f.get("status") == "needs-wider-scope" for f in out.get("findings") or [])
            sr = out.get("scope_request")
            if wide and not (isinstance(sr, dict) and sr.get("files")):
                warns.append("needs-wider-scope without a scope_request listing files")
            for p in (sr or {}).get("files") or [] if isinstance(sr, dict) else []:
                if fc.safe_rel(p) is None:
                    warns.append(f"scope_request has an unsafe path: {str(p)[:80]}")
        if kind == "review":
            root = fc.TASKS / f"{tid}.a{att}.review"
            quote_check = check_quotes(out.get("quotes") or [], root)
            if out.get("verdict") == "accept" and quote_check["ok"] == 0:
                warns.append("accept without any verified quote")
        if kind == "integrate":
            manifest = fc.rj(fc.BATCHES / f"{ident}.json")
            want = [t.get("task_id") for t in (manifest or {}).get("tasks") or []] if isinstance(manifest, dict) else []
            got = [s.get("task_id") for s in out.get("series") or [] if isinstance(s, dict)]
            if out.get("batch") != ident:
                errs.append(f"batch mismatch: {out.get('batch')!r}")
            if not want:
                errs.append(f"batch manifest {fc.BATCHES / (ident + '.json')} missing or empty")
            else:
                if sorted(set(want) - set(got)):
                    errs.append(f"missing series entries: {sorted(set(want) - set(got))}")
                if sorted(set(got) - set(want)):
                    errs.append(f"unexpected series entries: {sorted(set(got) - set(want))}")
                if len(got) != len(set(got)):
                    errs.append("duplicate series entries")
    schema_ok = not errs
    if quote_check and quote_check["fabricated"]:
        errs.append(f"quote check: {len(quote_check['fabricated'])} fabricated quote(s)")
    subtype = (result or {}).get("subtype", "")
    is_error = bool((result or {}).get("is_error")) or int(rc) != 0
    if out is None and not result:
        status = "rate_limited" if int(rl_hits) and int(rc) != 0 else "failed"
    elif is_error and out is None:
        status = "failed"
    elif errs:
        status = "invalid"
    else:
        status = "ok"
    usage = (result or {}).get("usage") or {}
    ptoks = [t["input_tokens"] + t["cache_creation_input_tokens"] + t["cache_read_input_tokens"] for t in turns]
    rec = {"kind": kind, "status": status, "schema_valid": schema_ok, "schema_errors": errs[:20], "warnings": warns,
           "exit_code": int(rc), "attempts": int(attempts), "rate_limit_hits": int(rl_hits),
           "result_subtype": subtype, "budget_hit": "budget" in subtype, "usage": usage,
           "cost_usd": (result or {}).get("total_cost_usd"), "num_turns": (result or {}).get("num_turns"),
           "tool_calls": count_tool_calls(text), "duration_s": float(dur), "started": started,
           "ended": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
           "prompt_bytes": Path(prompt_file).stat().st_size if prompt_file and Path(prompt_file).exists() else None,
           "turn_usage": turns, "turn_prompt_tokens": ptoks,
           "model": next(iter((result or {}).get("modelUsage") or {}), None),
           "raw": os.path.relpath(Path(raw_out).resolve(), fc.FIX_DIR) if Path(raw_out).exists() else None}
    if tid:
        rec["task_id"], rec["attempt"] = tid, att
    else:
        rec["batch"] = ident
    if quote_check is not None:
        rec["quote_check"] = quote_check
    if isinstance(out, dict):
        rec["output"] = out
    fc.write_json(dest, rec)
    return rec


# ---------------------------------------------------------------------------------------------------------
# task state
def _p(tid: str, n: int) -> dict[str, Path]:
    return {"res": fc.RESULTS / f"{tid}.a{n}.json", "patch": fc.PATCHES / f"{tid}.a{n}.patch",
            "scope": fc.PATCHES / f"{tid}.a{n}.scope.json", "check": fc.CHECKS / f"{tid}.a{n}.check.json",
            "review": fc.REVIEWS / f"{tid}.a{n}.review.json"}


def outcome(tid: str, n: int) -> str:
    p = _p(tid, n)
    r = fc.rj(p["res"])
    if not isinstance(r, dict):
        return "none"
    if r.get("status") != "ok" or not isinstance(r.get("output"), dict):
        return "error"
    if not any(f.get("status") == "fixed" for f in r["output"].get("findings") or [] if isinstance(f, dict)):
        return "notfixed"
    sc = fc.rj(p["scope"])
    if not isinstance(sc, dict):
        return "needs-diff"
    if sc.get("empty"):
        return "empty"
    if not sc.get("ok"):
        return "scope"
    ck = fc.rj(p["check"])
    if not isinstance(ck, dict):
        return "needs-check"
    return "check-ok" if ck.get("ok") else "check-failed"


def review_ok(tid: str, n: int) -> dict | None:
    r = fc.rj(_p(tid, n)["review"])
    return r if isinstance(r, dict) and r.get("status") == "ok" and isinstance(r.get("output"), dict) else None


def revise_rounds() -> int:
    """Max revise rounds after the first reviewed attempt ($FIX_REVISE_ROUNDS, default 1; invalid -> 1, <0 -> 0)."""
    try:
        return max(0, int(os.environ.get("FIX_REVISE_ROUNDS", "1")))
    except ValueError:
        return 1


_ATT_RE = re.compile(r"\.a(\d+)\.")


def attempt_numbers(tid: str) -> list[int]:
    """Attempt numbers with any record on disk, ascending; always includes 1..3 so legacy lookups are unchanged."""
    nums = {1, 2, 3}
    for d in (fc.RESULTS, fc.PATCHES, fc.CHECKS, fc.REVIEWS):
        if Path(d).is_dir():
            for f in Path(d).glob(f"{tid}.a*"):
                m = _ATT_RE.search(f.name)
                if m and f.name.startswith(tid + ".a"):
                    nums.add(int(m.group(1)))
    return sorted(nums)


def review_attempt(tid: str) -> int | None:
    """Latest attempt whose check passed and whose patch is non-empty (calibration items qualify too)."""
    for n in reversed(attempt_numbers(tid)):
        p = _p(tid, n)
        sc, ck = fc.rj(p["scope"]), fc.rj(p["check"])
        if isinstance(sc, dict) and isinstance(ck, dict) and sc.get("ok") and not sc.get("empty") and ck.get("ok") \
                and p["patch"].exists() and p["patch"].stat().st_size > 0:
            return n
    return None


def next_action(tid: str, revise: bool = False) -> str:
    o1 = outcome(tid, 1)
    if o1 in ("none", "error"):
        return "fix 1 plain 0" if not revise else "done"
    if o1 in ("needs-diff", "needs-check"):
        return "finish 1" if not revise else "done"
    latest = 1
    if o1 in ("check-failed", "scope"):
        o2 = outcome(tid, 2)
        if o2 in ("none", "error"):
            return "fix 2 error 1" if not revise else "done"
        if o2 in ("needs-diff", "needs-check"):
            return "finish 2" if not revise else "done"
        latest = 2
    if not revise:
        return "done"
    base = latest
    for k in range(3, 3 + revise_rounds()):
        # round k revises `base` (the latest check-ok attempt whose review says revise; later rounds chain from k-1)
        if outcome(tid, base) != "check-ok":
            return "done"
        rv = review_ok(tid, base)
        if not rv or rv["output"].get("verdict") != "revise":
            return "done"
        ok = outcome(tid, k)
        if ok in ("none", "error"):
            return f"fix {k} revise {base}"
        if ok in ("needs-diff", "needs-check"):
            return f"finish {k}"
        base = k
    return "done"


def revise_text(tid: str, base: int) -> str:
    rv = review_ok(tid, base)
    o = (rv or {}).get("output") or {}
    lines = [str(o.get("revise_instructions", "")).strip()]
    for i in o.get("issues") or []:
        if isinstance(i, dict) and i.get("severity") in ("blocker", "major"):
            lines.append(f"- {i.get('severity')}: {str(i.get('description', '')).strip()}")
    return "\n".join(x for x in lines if x) + "\n"


def error_file(tid: str, n: int) -> str:
    p = _p(tid, n)
    o = outcome(tid, n)
    return str(p["scope"] if o == "scope" else p["check"])


# ---------------------------------------------------------------------------------------------------------
def _queue_tasks() -> list[dict]:
    q = fc.load_queue()
    return sorted(q["tasks"], key=lambda t: (t.get("wave", 0), t.get("order", 0), t["task_id"]))


def pending(stage: str, classes: list[str] | None, wave: int | None, only: list[str] | None, revise: bool,
            exclude: set[str], chunk: int | None, queue_file: str | None) -> list[str]:
    ids: list[str]
    tasks = {t["task_id"]: t for t in _queue_tasks()}
    if stage == "review":
        known = dict(tasks)
        for t in (fc.rj(fp.CALIB_TASKS) or {}).get("tasks") or []:
            known[t["task_id"]] = t
        if only:
            ids = list(only)
        else:
            qf = fc.rj(queue_file) if queue_file else None
            ids = list(qf["tasks"]) if isinstance(qf, dict) and isinstance(qf.get("tasks"), list) else list(tasks)
        out = []
        for i in ids:
            t = known.get(i)
            if t is None:
                print(f"record_fix: unknown task {i}", file=sys.stderr)
                continue
            if t.get("class") == "human-only" or i in exclude:
                continue
            n = review_attempt(i)
            if n is not None and not review_ok(i, n):
                out.append(i)
        return out[:chunk] if chunk else out
    classes = classes or ["auto-fixable", "needs-review"]
    if only:
        sel = []
        for i in only:
            if i not in tasks:
                print(f"record_fix: unknown task {i}", file=sys.stderr)
            elif tasks[i]["class"] == "human-only":
                print(f"record_fix: {i} is human-only; skipped", file=sys.stderr)
            else:
                sel.append(tasks[i])
    else:
        sel = [t for t in tasks.values() if t["class"] in classes and t["class"] != "human-only"
               and (wave is None or t.get("wave") == wave)]
        sel.sort(key=lambda t: (t.get("wave", 0), t.get("order", 0), t["task_id"]))
    sel = [t for t in sel if t["task_id"] not in exclude and next_action(t["task_id"], revise) != "done"]
    if not chunk:
        return [t["task_id"] for t in sel]
    if not sel:
        return []
    w0 = sel[0].get("wave")
    used: set[str] = set()
    picked: list[str] = []
    for t in sel:
        if t.get("wave") != w0 and not only:
            continue
        files = set(t.get("allowed_files") or [])
        if files & used:
            continue
        picked.append(t["task_id"])
        used |= files
        if len(picked) >= chunk:
            break
    return picked


def rl_hits(since: float, ids: list[str]) -> int:
    tot = 0
    for tid in ids:
        for n in attempt_numbers(tid):
            for f in (_p(tid, n)["res"], _p(tid, n)["review"]):
                if f.exists() and f.stat().st_mtime >= since:
                    r = fc.rj(f) or {}
                    tot += int(r.get("rate_limit_hits") or 0)
    return tot


def summary(stage: str) -> dict:
    from collections import Counter
    c: Counter = Counter()
    for t in _queue_tasks():
        tid = t["task_id"]
        if stage == "fix":
            c[next((o for o in (outcome(tid, n) for n in reversed(attempt_numbers(tid))) if o != "none"), "none")] += 1
        else:
            n = review_attempt(tid)
            rv = review_ok(tid, n) if n else None
            c["no-patch" if n is None else ("pending" if not rv else rv["output"].get("verdict", "?"))] += 1
    return dict(sorted(c.items()))


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
    if cmd in SCHEMAS:
        if len(rest) != 9:
            print(__doc__)
            return 2
        rec = record(cmd, *rest)
        extra = ""
        if "quote_check" in rec:
            qc = rec["quote_check"]
            extra = f" quotes={qc['ok']}/{qc['total']} fabricated={len(qc['fabricated'])}"
        print(f"{rest[0]}: {rec['status']} cost={rec['cost_usd']} errs={len(rec['schema_errors'])} "
              f"turns={rec['num_turns']} tool_calls={rec['tool_calls']}{extra}")
        return 0
    if cmd == "outcome" and len(rest) == 2:
        print(outcome(rest[0], int(rest[1])))
    elif cmd == "next" and rest:
        print(next_action(rest[0], "--revise" in rest[1:]))
    elif cmd == "error-file" and len(rest) == 2:
        print(error_file(rest[0], int(rest[1])))
    elif cmd == "revise-text" and len(rest) == 2:
        sys.stdout.write(revise_text(rest[0], int(rest[1])))
    elif cmd == "review-attempt" and len(rest) == 1:
        n = review_attempt(rest[0])
        print("" if n is None else n)
    elif cmd == "info" and len(rest) == 1:
        t = fp.task_any(rest[0])
        print("\t".join([str(t.get("class")), str(t.get("specialty")), ",".join(t.get("allowed_files") or []),
                         str(t.get("style") or "")]))
    elif cmd == "pending":
        opt = {"--stage": "fix", "--class": None, "--wave": None, "--only": None, "--exclude-file": None,
               "--chunk": None, "--queue-file": None}
        revise, i = False, 0
        while i < len(rest):
            if rest[i] == "--revise":
                revise = True
                i += 1
            elif rest[i] in opt and i + 1 < len(rest):
                opt[rest[i]] = rest[i + 1]
                i += 2
            else:
                sys.exit(f"record_fix: bad pending arg {rest[i]!r}")
        ex = set()
        if opt["--exclude-file"] and Path(opt["--exclude-file"]).exists():
            ex = {x.strip() for x in Path(opt["--exclude-file"]).read_text().split() if x.strip()}
        ids = pending(opt["--stage"], opt["--class"].split(",") if opt["--class"] else None,
                      int(opt["--wave"]) if opt["--wave"] else None,
                      [x for x in opt["--only"].split(",") if x] if opt["--only"] else None, revise, ex,
                      int(opt["--chunk"]) if opt["--chunk"] else None, opt["--queue-file"])
        for x in ids:
            print(x)
    elif cmd == "rl-hits" and len(rest) >= 2 and rest[0] == "--since":
        print(rl_hits(float(rest[1]), rest[2:]))
    elif cmd == "summary" and len(rest) == 1:
        sys.stdout.write(fc.dumps(summary(rest[0])))
    else:
        print(__doc__)
        return 2
    return 0


# ---------------------------------------------------------------------------------------------------------
def self_test() -> int:
    import subprocess
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        env = dict(os.environ, FIX_DIR=str(Path(td) / "data"), FIX_WORK=str(Path(td) / "work"),
                   KOTOR2_REVIEW_HOME=str(Path(td) / "home"), FIX_SELFTEST_TMP=td)
        return subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), "--self-test-run"], env=env).returncode


def _stream(payload: dict | None, cost=0.01, subtype="success", is_error=False, tools=1) -> str:
    ev = [{"type": "system", "subtype": "init"}]
    for k in range(tools):
        ev.append({"type": "assistant", "message": {"id": f"m{k}", "usage": {"input_tokens": 2, "cache_creation_input_tokens": 100,
                   "cache_read_input_tokens": 50, "output_tokens": 20}, "content": [{"type": "tool_use", "id": f"t{k}", "name": "Read"}]}})
    ev.append({"type": "result", "subtype": subtype, "is_error": is_error, "num_turns": 2, "total_cost_usd": cost,
               "usage": {"input_tokens": 5, "output_tokens": 60}, "modelUsage": {"claude-haiku-5-5": {}},
               **({"result": json.dumps(payload), "structured_output": payload} if payload is not None else {"result": ""})})
    return "\n".join(json.dumps(e) for e in ev) + "\n"


def self_test_run() -> int:
    td = Path(os.environ["FIX_SELFTEST_TMP"])
    bad: list[str] = []

    def check(c, m):
        if not c:
            bad.append(m)

    TID = "T0123456789"
    fview = {"id": "f00000000001", "file": "scripts/foo.py", "lines": "6", "category": "logic", "specialty": "python-tooling",
             "severity": "low", "claim": "c", "verifier_reasoning": "r", "evidence_quotes": [], "location_check": []}
    tasks = [{"task_id": TID, "class": "auto-fixable", "specialty": "python-tooling", "flags": {}, "allowed_files": ["scripts/foo.py"],
              "context_globs": [], "line_cap": 40, "findings": [fview], "wave": 1, "order": 1},
             {"task_id": "T0123456790", "class": "auto-fixable", "specialty": "python-tooling", "flags": {}, "allowed_files": ["scripts/foo.py"],
              "context_globs": [], "line_cap": 40, "findings": [dict(fview, id="f00000000002")], "wave": 1, "order": 2},
             {"task_id": "T0123456791", "class": "needs-design", "specialty": "cpp-logic", "flags": {}, "allowed_files": ["a.cpp"],
              "context_globs": [], "line_cap": 120, "findings": [dict(fview, id="f00000000003")], "wave": 2, "order": 1},
             {"task_id": "T0123456792", "class": "human-only", "specialty": "cpp-logic", "flags": {}, "allowed_files": ["b.cpp"],
              "context_globs": [], "line_cap": 0, "findings": [dict(fview, id="f00000000004")], "wave": 1, "order": 3}]
    fc.write_json(fc.QUEUE, {"tasks": tasks})
    (td / "p.txt").write_text("prompt")
    raw = td / "raw.out"
    good = {"task_id": TID, "findings": [{"finding_id": "f00000000001", "status": "fixed", "actual_lines": "6", "note": "n"}],
            "files_changed": ["scripts/foo.py"], "rationale": "rationale text long enough", "test_plan": "t", "risk": "low",
            "needs_live_check": False, "location_quotes": [{"file": "scripts/foo.py", "line": 6, "text": "    if x < 3:"}]}

    def rec(kind, ident, payload, **kw):
        raw.write_text(_stream(payload, **kw))
        return record(kind, ident, str(raw), str(td / "e"), kw.get("rc", "0") if False else "0", "1", "0", "t0", "3.5", str(td / "p.txt"))

    r = rec("fix", f"{TID}.a1", good)
    check(r["status"] == "ok" and r["schema_valid"] and r["tool_calls"] == 1 and r["cost_usd"] == 0.01, f"fix ok {r['schema_errors']}")
    check((fc.RESULTS / f"{TID}.a1.json").exists() and r["task_id"] == TID and r["attempt"] == 1, "fix file")
    check(rec("fix", f"{TID}.a1", dict(good, task_id="T0000000000"))["status"] == "invalid", "task_id mismatch invalid")
    check(rec("fix", f"{TID}.a1", dict(good, risk="huge"))["status"] == "invalid", "enum violation")
    check(rec("fix", f"{TID}.a1", dict(good, bonus=1))["status"] == "invalid", "extra key")
    check(rec("fix", f"{TID}.a1", None, subtype="error_max_structured_output_retries", is_error=True)["status"] == "failed", "retry exhausted -> failed")
    r = rec("fix", f"{TID}.a1", None, subtype="error_max_budget_usd", is_error=True)
    check(r["status"] == "failed" and r["budget_hit"], "budget hit")
    raw.write_text("")
    check(record("fix", f"{TID}.a1", str(raw), "", "1", "1", "2", "t", "1", "")["status"] == "rate_limited", "empty + rl -> rate_limited")
    check(record("fix", f"{TID}.a1", str(raw), "", "1", "1", "0", "t", "1", "")["status"] == "failed", "empty -> failed")

    # ---- state machine -------------------------------------------------------------------------------
    def put(path, obj):
        fc.write_json(path, obj)

    check(next_action(TID) == "fix 1 plain 0" and outcome(TID, 1) == "error", "failed result -> re-run a1")
    rec("fix", f"{TID}.a1", good)
    check(outcome(TID, 1) == "needs-diff" and next_action(TID) == "finish 1", "needs-diff")
    put(fc.PATCHES / f"{TID}.a1.scope.json", {"ok": True, "empty": False})
    check(outcome(TID, 1) == "needs-check", "needs-check")
    put(fc.CHECKS / f"{TID}.a1.check.json", {"ok": False, "gates": {"build": {"status": "fail", "detail": "boom"}}})
    check(outcome(TID, 1) == "check-failed" and next_action(TID) == "fix 2 error 1", "check-failed -> retry a2")
    check(error_file(TID, 1).endswith(f"{TID}.a1.check.json"), "error file is the check json")
    rec("fix", f"{TID}.a2", good)
    put(fc.PATCHES / f"{TID}.a2.scope.json", {"ok": True, "empty": False})
    put(fc.CHECKS / f"{TID}.a2.check.json", {"ok": True, "gates": {}})
    (fc.PATCHES / f"{TID}.a2.patch").write_text("--- a/x\n+++ b/x\n@@ -1 +1 @@\n-a\n+b\n")
    check(next_action(TID) == "done" and next_action(TID, True) == "done", "a2 ok, no review yet -> nothing to do")
    check(review_attempt(TID) == 2, "review attempt = latest checked")
    # review: quotes against the review copy
    rc = fc.TASKS / f"{TID}.a2.review" / "scripts"
    rc.mkdir(parents=True)
    (rc / "foo.py").write_text("a\nb\nc\nd\ne\n    if x <= 3:\n        return x\n")
    rv = {"task_id": TID, "verdict": "revise", "per_finding": [{"finding_id": "f00000000001", "defect_fixed": "partial", "why": "w"}],
          "checks": {k: False for k in ("regression_risk", "scope_creep", "weakened_tests", "hook_abi_risk", "unchecked_returns", "boundary_change")},
          "issues": [{"severity": "major", "description": "keep it strict"}], "quotes": [{"file": "scripts/foo.py", "line": 7, "text": "if x <=   3:"}],
          "revise_instructions": "Use < not <=."}
    r = rec("review", f"{TID}.a2", rv)
    check(r["status"] == "ok" and r["quote_check"]["ok"] == 1 and not r["quote_check"]["fabricated"], f"review ok {r['schema_errors']} {r.get('quote_check')}")
    r = rec("review", f"{TID}.a2", dict(rv, quotes=[{"file": "scripts/foo.py", "line": 6, "text": "THIS TEXT DOES NOT EXIST ANYWHERE"}]))
    check(r["status"] == "invalid" and len(r["quote_check"]["fabricated"]) == 1 and r["schema_valid"], "fabricated quote -> invalid")
    r = rec("review", f"{TID}.a2", dict(rv, quotes=[{"file": "../etc/passwd", "line": 1, "text": "root:x:0:0:root"},
                                                    {"file": ".env", "line": 1, "text": "SECRET=abcdef"}, {"file": "scripts/foo.py", "line": 6, "text": "if"}]))
    qc = r["quote_check"]
    check(r["status"] == "invalid" and len(qc["fabricated"]) == 2 and len(qc["trivial"]) == 1, f"unsafe/excluded paths fabricated, short quote trivial {qc}")
    check(rec("review", f"{TID}.a2", dict(rv, task_id="T1111111111"))["status"] == "invalid", "review task mismatch")
    rec("review", f"{TID}.a2", rv)
    check(next_action(TID, True) == f"fix 3 revise 2" and review_attempt(TID) == 2, "revise verdict -> a3 with base 2")
    check("Use < not <=." in revise_text(TID, 2) and "keep it strict" in revise_text(TID, 2), "revise text")
    check(not pending("review", None, None, None, False, set(), None, None), "reviewed -> not pending for review")
    check(pending("fix", None, None, None, True, set(), None, None) == [TID], "--revise selects the task")
    rec("fix", f"{TID}.a3", good)
    put(fc.PATCHES / f"{TID}.a3.scope.json", {"ok": True, "empty": False})
    put(fc.CHECKS / f"{TID}.a3.check.json", {"ok": True, "gates": {}})
    (fc.PATCHES / f"{TID}.a3.patch").write_text("--- a/x\n+++ b/x\n@@ -1 +1 @@\n-a\n+c\n")
    check(next_action(TID, True) == "done" and review_attempt(TID) == 3, "a3 done; a3 now needs a review")
    check(pending("review", None, None, None, False, set(), None, None) == [TID], "a3 pending review")

    # ---- revise rounds (FIX_REVISE_ROUNDS) ---------------------------------------------------------------
    def att(n, ok=True):
        rec("fix", f"{TID}.a{n}", good)
        put(fc.PATCHES / f"{TID}.a{n}.scope.json", {"ok": True, "empty": False})
        put(fc.CHECKS / f"{TID}.a{n}.check.json", {"ok": ok, "gates": {}})
        (fc.PATCHES / f"{TID}.a{n}.patch").write_text("--- a/x\n+++ b/x\n@@ -1 +1 @@\n-a\n+d\n")

    def verdict(n, v):
        if not (fc.TASKS / f"{TID}.a{n}.review").exists():
            shutil.copytree(fc.TASKS / f"{TID}.a2.review", fc.TASKS / f"{TID}.a{n}.review")
        rec("review", f"{TID}.a{n}", dict(rv, verdict=v))

    check(revise_rounds() == 1, "default rounds = 1")
    verdict(3, "revise")
    check(next_action(TID, True) == "done" and next_action(TID) == "done", "R=1: a3 revise -> no a4")
    os.environ["FIX_REVISE_ROUNDS"] = "3"
    check(revise_rounds() == 3, "R=3 parsed")
    check(next_action(TID, True) == "fix 4 revise 3", f"R=3: a3 revise -> a4 on base 3: {next_action(TID, True)}")
    check(next_action(TID) == "done", "R=3 without --revise: nothing")
    rec("fix", f"{TID}.a4", good)
    check(next_action(TID, True) == "finish 4", "R=3: a4 recorded, no diff -> finish 4")
    att(4)
    check(next_action(TID, True) == "done" and review_attempt(TID) == 4, "R=3: a4 awaiting review -> done, review a4")
    verdict(4, "revise")
    check(next_action(TID, True) == "fix 5 revise 4", "R=3: a4 revise -> a5 on base 4")
    att(5)
    verdict(5, "revise")
    check(next_action(TID, True) == "done" and review_attempt(TID) == 5, "R=3: rounds exhausted after a5")
    verdict(5, "accept")
    check(next_action(TID, True) == "done", "R=3: accept stops")
    verdict(4, "accept")
    (fc.REVIEWS / f"{TID}.a5.review.json").unlink()
    check(next_action(TID, True) == "done", "R=3: a4 accept -> no a5")
    put(fc.CHECKS / f"{TID}.a4.check.json", {"ok": False, "gates": {}})
    verdict(4, "revise")
    check(next_action(TID, True) == "done", "R=3: a4 check failed -> chain stops")
    check(rl_hits(0, [TID]) == 0 and summary("fix"), "rl-hits/summary over attempts > 3")
    os.environ["FIX_REVISE_ROUNDS"] = "1"
    for n in (4, 5):
        for k in ("res", "patch", "scope", "check", "review"):
            _p(TID, n)[k].unlink(missing_ok=True)
    del os.environ["FIX_REVISE_ROUNDS"]
    # ---- needs-wider-scope ------------------------------------------------------------------------------
    TW = "T0123456795"
    wide = dict(good, task_id=TW, files_changed=[], findings=[{"finding_id": "f00000000001", "status": "needs-wider-scope", "actual_lines": "6", "note": "n"}],
                scope_request={"files": ["scripts/bar.py", "tests/test_bar.py"], "reason": "contract lives in bar.py"})
    r = rec("fix", f"{TW}.a1", wide)
    check(r["status"] == "ok" and r["schema_valid"] and not r["warnings"], f"needs-wider-scope records: {r['schema_errors']} {r['warnings']}")
    check(outcome(TW, 1) == "notfixed" and next_action(TW) == "done" and next_action(TW, True) == "done", "needs-wider-scope -> done, no retry")
    mixed = dict(wide, findings=wide["findings"] + [{"finding_id": "f00000000002", "status": "not-fixable", "actual_lines": "6", "note": "n"}])
    check(rec("fix", f"{TW}.a1", mixed)["status"] == "ok" and next_action(TW) == "done", "mixed needs-wider-scope + not-fixable -> done")
    check(rec("fix", f"{TW}.a1", dict(wide, scope_request={"files": ["a"]}))["status"] == "invalid", "scope_request requires reason")
    check(rec("fix", f"{TW}.a1", dict(wide, scope_request={"files": ["a"], "reason": "r", "x": 1}))["status"] == "invalid", "scope_request extra key")
    r = rec("fix", f"{TW}.a1", {k: v for k, v in wide.items() if k != "scope_request"})
    check(r["status"] == "ok" and any("scope_request" in w for w in r["warnings"]), "missing scope_request warns")

    # ---- pending/chunk ----------------------------------------------------------------------------------
    p = pending("fix", None, None, None, False, set(), None, None)
    check(p == ["T0123456790"], f"default classes, human-only skipped, TID done: {p}")
    check(pending("fix", ["needs-design"], None, None, False, set(), None, None) == ["T0123456791"], "class filter")
    check(pending("fix", None, 2, None, False, set(), None, None) == [], "wave filter (wave 2 has no default-class task)")
    check(pending("fix", None, None, ["T0123456790"], False, {"T0123456790"}, None, None) == [], "exclude")
    check(pending("fix", None, None, ["T0123456792"], False, set(), None, None) == [], "human-only never")
    tasks[0]["task_id"] = "T0123456793"
    fc.write_json(fc.QUEUE, {"tasks": tasks + [dict(tasks[1], task_id="T0123456794", allowed_files=["scripts/other.py"], order=9)]})
    ch = pending("fix", None, None, None, False, set(), 4, None)
    check(ch == ["T0123456793", "T0123456794"], f"chunk drops tasks sharing an allowed file: {ch}")
    ch = pending("fix", None, None, None, False, set(), 1, None)
    check(ch == ["T0123456793"], f"chunk size: {ch}")
    check(rl_hits(0, [TID]) == 0, "rl-hits")
    # ---- integrate -------------------------------------------------------------------------------------
    fc.BATCHES.mkdir(parents=True, exist_ok=True)
    fc.write_json(fc.BATCHES / "batch_01.json", {"batch": "batch_01", "tasks": [{"task_id": TID, "attempt": 3, "patch": "p"}]})
    integ = {"batch": "batch_01", "series": [{"task_id": TID, "attempt": 3, "action": "keep", "reason": "ok"}], "gates_run": [], "notes": ""}
    r = rec("integrate", "batch_01", integ)
    check(r["status"] == "ok" and (fc.INTEG / "batch_01.run.json").exists(), f"integrate ok {r['schema_errors']}")
    r = rec("integrate", "batch_01", dict(integ, series=[]))
    check(r["status"] == "invalid", "integrate missing series entry")
    print("record_fix self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
