#!/usr/bin/env python3
"""Verify-stage variant of record_run.py: turn one raw `claude -p` output into a result file (no LLM calls).

Usage:
  record_verify.py verify     <finding_id> <raw_stdout> <raw_stderr> <exit_code> <attempts> <rate_limit_hits> <started_iso> <duration_s> <prompt_file>
  record_verify.py adjudicate <batch_id>   (same arguments)
  record_verify.py --self-test
verify     -> $VERIFY_DIR/results/<id>.json        validated against verify/verify_schema.json
adjudicate -> $VERIFY_DIR/adjudications/<batch>.json  validated against verify/adjudicate_schema.json and
              against work/adjudicate/<batch>.json (every batch item answered exactly once)
Reuses record_run.py (minimal JSON-schema subset validator, stream-json parser, structured-output
extraction). Status: ok | invalid | failed | rate_limited. A result is COMPLETE only when status == ok.
"""
from __future__ import annotations

import json
import os
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
VERIFY_CODE = HERE.parent / "verify"
sys.path.insert(0, str(HERE))
sys.path.append(str(VERIFY_CODE))       # append: verify/ holds select.py (stdlib name)
import record_run as rr  # noqa: E402
import vcommon as vc  # noqa: E402

TOOL_BUDGET = 14


def count_tool_calls(text: str) -> int:
    seen = set()
    for line in text.splitlines():
        try:
            ev = json.loads(line)
        except json.JSONDecodeError:
            continue
        if ev.get("type") != "assistant":
            continue
        for blk in (ev.get("message") or {}).get("content") or []:
            if isinstance(blk, dict) and blk.get("type") == "tool_use":
                seen.add(blk.get("id") or f"_{len(seen)}")
    return len(seen)


def record(kind: str, ident: str, raw_out: str, raw_err: str, rc: str, attempts: str, rl_hits: str,
           started: str, dur: str, prompt_file: str, schema_path: Path | None = None) -> dict:
    schema = json.loads((schema_path or VERIFY_CODE / ("verify_schema.json" if kind == "verify" else "adjudicate_schema.json")).read_text())
    text = Path(raw_out).read_text(errors="replace") if Path(raw_out).exists() else ""
    result, turns = rr.parse_raw(text)
    out = rr.structured(result) if result else None
    errs = rr.validate(out, schema) if out is not None else ["no structured output"]
    warns = []
    if isinstance(out, dict):
        if kind == "verify":
            if out.get("finding_id") != ident:
                errs.append(f"finding_id mismatch: {out.get('finding_id')!r}")
            if len(out.get("tool_trace") or []) > TOOL_BUDGET:
                warns.append(f"tool_trace has {len(out['tool_trace'])} entries (> {TOOL_BUDGET})")
        else:
            if out.get("batch") != ident:
                errs.append(f"batch mismatch: {out.get('batch')!r}")
            manifest = vc.rj(vc.BATCHES / f"{ident}.json")
            want = list((manifest or {}).get("items") or []) if isinstance(manifest, dict) else []
            got = [i.get("finding_id") for i in out.get("items") or [] if isinstance(i, dict)]
            if not want:
                errs.append(f"batch manifest {vc.BATCHES / (ident + '.json')} missing or empty")
            else:
                miss = sorted(set(want) - set(got))
                extra = sorted(set(got) - set(want))
                dup = sorted({g for g in got if got.count(g) > 1})
                if miss:
                    errs.append(f"missing items: {miss}")
                if extra:
                    errs.append(f"unexpected items: {extra}")
                if dup:
                    errs.append(f"duplicate items: {dup}")
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
    prompt_bytes = Path(prompt_file).stat().st_size if prompt_file and Path(prompt_file).exists() else None
    rec = {"kind": kind, "status": status, "schema_valid": not errs, "schema_errors": errs[:20], "warnings": warns,
           "exit_code": int(rc), "attempts": int(attempts), "rate_limit_hits": int(rl_hits),
           "result_subtype": subtype, "budget_hit": "budget" in subtype, "usage": usage,
           "cost_usd": (result or {}).get("total_cost_usd"), "num_turns": (result or {}).get("num_turns"),
           "tool_calls": count_tool_calls(text), "duration_s": float(dur), "started": started,
           "ended": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"), "prompt_bytes": prompt_bytes,
           "turn_usage": turns, "turn_prompt_tokens": ptoks, "model": next(iter((result or {}).get("modelUsage") or {}), None),
           "raw": os.path.relpath(Path(raw_out).resolve(), vc.VERIFY_DIR) if Path(raw_out).exists() else None}
    if isinstance(out, dict):
        rec["output"] = out
    if kind == "verify":
        rec["finding_id"] = ident
        dest_dir = vc.RESULTS
    else:
        rec["batch"] = ident
        dest_dir = vc.ADJ
    dest_dir.mkdir(parents=True, exist_ok=True)
    (dest_dir / f"{ident}.json").write_text(vc.dumps(rec), encoding="utf-8")
    return rec


def main(argv: list[str]) -> int:
    if argv and argv[0] == "--self-test":
        return self_test()
    if len(argv) != 10 or argv[0] not in ("verify", "adjudicate"):
        print(__doc__)
        return 2
    kind, ident = argv[0], argv[1]
    rec = record(kind, ident, *argv[2:10])
    print(f"{ident}: {rec['status']} cost={rec['cost_usd']} errs={len(rec['schema_errors'])} "
          f"turns={rec['num_turns']} tool_calls={rec['tool_calls']}")
    return 0


def _stream(payload: dict, cost=0.01, subtype="success") -> str:
    ev = [
        {"type": "system", "subtype": "init"},
        {"type": "assistant", "message": {"id": "m1", "usage": {"input_tokens": 2, "cache_creation_input_tokens": 100,
         "cache_read_input_tokens": 50, "output_tokens": 20}, "content": [{"type": "tool_use", "id": "t1", "name": "Read"}]}},
        {"type": "assistant", "message": {"id": "m2", "usage": {"input_tokens": 3, "cache_creation_input_tokens": 0,
         "cache_read_input_tokens": 150, "output_tokens": 40}, "content": [{"type": "text", "text": "x"}]}},
        {"type": "result", "subtype": subtype, "is_error": False, "num_turns": 2, "total_cost_usd": cost,
         "usage": {"input_tokens": 5, "output_tokens": 60}, "modelUsage": {"claude-haiku-5-5": {}},
         "result": json.dumps(payload), "structured_output": payload},
    ]
    return "\n".join(json.dumps(e) for e in ev) + "\n"


def self_test() -> int:
    ok = True

    def check(cond, msg):
        nonlocal ok
        if not cond:
            ok = False
            print("FAIL:", msg)

    good = {"finding_id": "abc", "verdict": "refuted", "claim_restated": "The claim says X happens.",
            "tool_trace": [{"tool": "Read", "target": "a.cpp", "purpose": "cited lines"}],
            "supporting_quotes": [], "counter_evidence_searched": [{"pattern": "x", "scope": "a", "hits": 0, "note": ""}],
            "refutation_quotes": [{"file": "a.cpp", "line": 3, "text": "int x = 0;"}], "impact_if_real": "n/a",
            "confidence": "high", "not_verifiable_without_running": "none", "residual_questions": []}
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        os.environ["VERIFY_DIR"] = str(td / "vd")
        import importlib
        importlib.reload(vc)
        globals()["vc"] = vc
        out = td / "o.out"
        out.write_text(_stream(good))
        (td / "p.txt").write_text("prompt")
        rec = record("verify", "abc", str(out), str(td / "e"), "0", "1", "0", "t0", "3.5", str(td / "p.txt"))
        check(rec["status"] == "ok" and rec["schema_valid"], f"ok result: {rec['schema_errors']}")
        check(rec["tool_calls"] == 1 and len(rec["turn_usage"]) == 2 and rec["cost_usd"] == 0.01, "usage/tool count")
        check((vc.RESULTS / "abc.json").exists() and vc.result_ok("abc"), "file written and complete")
        bad = dict(good, verdict="maybe")
        out.write_text(_stream(bad))
        rec = record("verify", "abc", str(out), str(td / "e"), "0", "1", "0", "t0", "1", str(td / "p.txt"))
        check(rec["status"] == "invalid", "enum violation invalid")
        out.write_text(_stream(dict(good, finding_id="zzz")))
        rec = record("verify", "abc", str(out), str(td / "e"), "0", "1", "0", "t0", "1", str(td / "p.txt"))
        check(rec["status"] == "invalid" and any("mismatch" in e for e in rec["schema_errors"]), "id mismatch invalid")
        extra = dict(good, bonus=1)
        out.write_text(_stream(extra))
        check(record("verify", "abc", str(out), "", "0", "1", "0", "t", "1", "")["status"] == "invalid", "extra key invalid")
        out.write_text("")
        rec = record("verify", "abc", str(out), "", "1", "1", "2", "t", "1", "")
        check(rec["status"] == "rate_limited", "empty output + rl hits -> rate_limited")
        rec = record("verify", "abc", str(out), "", "1", "1", "0", "t", "1", "")
        check(rec["status"] == "failed", "empty output -> failed")
        # adjudication
        vc.BATCHES.mkdir(parents=True, exist_ok=True)
        (vc.BATCHES / "batch_01.json").write_text(json.dumps({"batch": "batch_01", "items": ["a", "b"]}))
        item = lambda i: {"finding_id": i, "final_verdict": "confirmed", "final_severity": "high", "agrees_with_haiku": True,
                          "reasoning": "reasoning long enough.", "human_check_steps": ["read x"], "evidence_quotes": []}
        adj = {"batch": "batch_01", "haiku_quality": {"rating": "ok", "reason": "fine"}, "items": [item("a"), item("b")], "notes": ""}
        out.write_text(_stream(adj))
        rec = record("adjudicate", "batch_01", str(out), "", "0", "1", "0", "t", "1", "")
        check(rec["status"] == "ok", f"adjudication ok {rec['schema_errors']}")
        adj["items"] = [item("a")]
        out.write_text(_stream(adj))
        rec = record("adjudicate", "batch_01", str(out), "", "0", "1", "0", "t", "1", "")
        check(rec["status"] == "invalid" and any("missing items" in e for e in rec["schema_errors"]), "missing item invalid")
        adj["items"] = [item("a"), item("a"), item("b")]
        out.write_text(_stream(adj))
        rec = record("adjudicate", "batch_01", str(out), "", "0", "1", "0", "t", "1", "")
        check(rec["status"] == "invalid" and any("duplicate" in e for e in rec["schema_errors"]), "duplicate invalid")
    print("record_verify self-test:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
