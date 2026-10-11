#!/usr/bin/env python3
"""Turn one raw `claude -p` output into findings/<id>.json + runs/<id>.json (no LLM calls).

Usage: record_run.py <packet_id> <raw_stdout> <raw_stderr> <exit_code> <attempts> <rate_limit_hits>
                     <started_iso> <duration_s> <prompt_file>
Accepts --output-format json (one object) or stream-json (JSONL; the last `result` event wins, and the
first assistant message's usage gives first_turn_prompt_tokens for `prep.py resize`).
Also: `record_run.py --validate <schema.json> <instance.json>` (exit 0 iff valid).
"""
from __future__ import annotations

import json
import os
import sys
from datetime import datetime, timezone
from pathlib import Path

REVIEW = Path(__file__).resolve().parent.parent
SCHEMA = json.loads((REVIEW / "worker_schema.json").read_text())


def validate(inst, sch, path="$") -> list[str]:
    """Minimal draft-2020-12 subset: type, enum, required, properties, additionalProperties, items,
    minLength, pattern."""
    import re
    errs = []
    t = sch.get("type")
    types = t if isinstance(t, list) else [t] if t else []
    pymap = {"object": dict, "array": list, "string": str, "boolean": bool, "null": type(None),
             "integer": int, "number": (int, float)}
    if types and not any(isinstance(inst, pymap[x]) and not (x in ("integer", "number") and isinstance(inst, bool))
                         for x in types):
        return [f"{path}: expected {types}, got {type(inst).__name__}"]
    if "enum" in sch and inst not in sch["enum"]:
        errs.append(f"{path}: {inst!r} not in enum")
    if isinstance(inst, str):
        if len(inst) < sch.get("minLength", 0):
            errs.append(f"{path}: shorter than minLength")
        if "pattern" in sch and not re.search(sch["pattern"], inst):
            errs.append(f"{path}: {inst!r} does not match {sch['pattern']}")
    if isinstance(inst, dict):
        for k in sch.get("required", []):
            if k not in inst:
                errs.append(f"{path}: missing {k}")
        props = sch.get("properties", {})
        for k, v in inst.items():
            if k in props:
                errs += validate(v, props[k], f"{path}.{k}")
            elif sch.get("additionalProperties") is False:
                errs.append(f"{path}: unexpected key {k}")
    if isinstance(inst, list) and "items" in sch:
        for i, v in enumerate(inst):
            errs += validate(v, sch["items"], f"{path}[{i}]")
    return errs


USAGE_KEYS = ("input_tokens", "cache_creation_input_tokens", "cache_read_input_tokens", "output_tokens")


def parse_raw(text: str) -> tuple[dict | None, list[dict]]:
    """-> (result event, per-request usage list). Stream events of one API message share an id."""
    text = text.strip()
    if not text:
        return None, []
    try:
        obj = json.loads(text)
        if isinstance(obj, dict):
            return obj, []
    except json.JSONDecodeError:
        pass
    result, turns, seen = None, [], {}
    for line in text.splitlines():
        try:
            ev = json.loads(line)
        except json.JSONDecodeError:
            continue
        if ev.get("type") == "assistant":
            msg = ev.get("message") or {}
            u = msg.get("usage") or {}
            if u:
                rec = {k: int(u.get(k) or 0) for k in USAGE_KEYS}
                mid = msg.get("id") or f"_{len(turns)}"
                if mid in seen:
                    turns[seen[mid]] = rec          # later event of the same message: keep the latest usage
                else:
                    seen[mid] = len(turns)
                    turns.append(rec)
        if ev.get("type") == "result":
            result = ev
    return result, turns


def structured(result: dict):
    if isinstance(result.get("structured_output"), dict):
        return result["structured_output"]
    r = result.get("result")
    if isinstance(r, str):
        r = r.strip()
        if r.startswith("```"):
            r = r.strip("`").split("\n", 1)[-1]
        try:
            return json.loads(r)
        except json.JSONDecodeError:
            return None
    return r if isinstance(r, dict) else None


def main() -> None:
    if sys.argv[1] == "--validate":
        sch = json.loads(Path(sys.argv[2]).read_text())
        errs = validate(json.loads(Path(sys.argv[3]).read_text()), sch)
        print("\n".join(errs) or "valid")
        sys.exit(1 if errs else 0)
    pid, raw_out, raw_err, rc, attempts, rl_hits, started, dur, prompt_file = sys.argv[1:10]
    packet = json.loads((REVIEW / "packets" / f"{pid}.json").read_text())
    result, turns = parse_raw(Path(raw_out).read_text(errors="replace") if Path(raw_out).exists() else "")
    out = structured(result) if result else None
    errs = validate(out, SCHEMA) if out is not None else ["no structured output"]
    if out is not None and out.get("packet_id") != pid:
        errs.append(f"packet_id mismatch: {out.get('packet_id')!r}")
    subtype = (result or {}).get("subtype", "")
    is_error = bool((result or {}).get("is_error")) or int(rc) != 0
    budget_hit = "budget" in subtype
    if out is None and not result:
        status = "rate_limited" if int(rl_hits) and int(rc) != 0 else "failed"
    elif is_error and out is None:
        status = "failed"
    elif errs:
        status = "invalid"
    else:
        status = "ok"
    truncated = bool(out.get("truncated")) if isinstance(out, dict) else True
    if budget_hit:
        truncated = True
    usage = (result or {}).get("usage") or {}
    run = {"packet_id": pid, "files": packet["files"], "exit_code": int(rc), "attempts": int(attempts),
           "rate_limit_hits": int(rl_hits), "schema_valid": not errs, "schema_errors": errs[:20],
           "truncated": truncated, "status": status, "result_subtype": subtype, "usage": usage,
           "cost_usd": (result or {}).get("total_cost_usd"), "num_turns": (result or {}).get("num_turns"),
           "duration_s": float(dur), "started": started,
           "ended": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
           "prompt_bytes": Path(prompt_file).stat().st_size if Path(prompt_file).exists() else None,
           "turn_usage": turns,
           "turn_prompt_tokens": [t["input_tokens"] + t["cache_creation_input_tokens"] + t["cache_read_input_tokens"]
                                  for t in turns],
           "first_turn_prompt_tokens": (turns[0]["input_tokens"] + turns[0]["cache_creation_input_tokens"]
                                        + turns[0]["cache_read_input_tokens"]) if turns else None, "raw": os.path.relpath(Path(raw_out).resolve(), REVIEW)}
    (REVIEW / "runs").mkdir(exist_ok=True)
    (REVIEW / "runs" / f"{pid}.json").write_text(json.dumps(run, indent=1, sort_keys=True) + "\n")
    if isinstance(out, dict):
        out["_usage"] = {"usage": usage, "total_cost_usd": run["cost_usd"], "status": status}
        (REVIEW / "findings").mkdir(exist_ok=True)
        (REVIEW / "findings" / f"{pid}.json").write_text(json.dumps(out, indent=1, sort_keys=True) + "\n")
    run["turns_over_100k"] = sum(1 for t in run["turn_prompt_tokens"] if t > 100_000)
    (REVIEW / "runs" / f"{pid}.json").write_text(json.dumps(run, indent=1, sort_keys=True) + "\n")
    print(f"{pid}: {status} truncated={truncated} cost={run['cost_usd']} errs={len(errs)} "
          f"turns={len(turns)} turns>100K={run['turns_over_100k']}")


if __name__ == "__main__":
    main()
