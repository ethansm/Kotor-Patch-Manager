#!/usr/bin/env python3
"""L0 effort smoke parser. Usage: python3 -I l0_parse.py <SCR>

Reads $SCR/out/*.jsonl (stream-json), takes the last `result` event per arm,
prints a markdown table, then the --effort verdict and the recorded env value.
"""
import glob
import json
import os
import sys

ORDER = ["none", "low", "medium", "xhigh", "env"]
COLS = ["arm", "output_tokens", "cost", "duration_ms", "turns", "subtype", "model"]


def last_result(path):
    res = None
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if not line.startswith("{"):
                continue
            try:
                obj = json.loads(line)
            except ValueError:
                continue
            if obj.get("type") == "result":
                res = obj
    return res


def cell(value):
    return "-" if value is None else str(value).replace("|", "\\|")


def arm_key(arm):
    return (ORDER.index(arm) if arm in ORDER else len(ORDER), arm)


def main():
    if len(sys.argv) != 2:
        print("usage: l0_parse.py <SCR>", file=sys.stderr)
        return 2
    scr = sys.argv[1]
    paths = glob.glob(os.path.join(scr, "out", "*.jsonl"))
    rows = []
    for path in paths:
        arm = os.path.basename(path)[: -len(".jsonl")]
        res = last_result(path)
        if res is None:
            rows.append({"arm": arm, "res": None})
            continue
        usage = res.get("usage") or {}
        models = sorted((res.get("modelUsage") or {}).keys())
        rows.append({
            "arm": arm,
            "res": res,
            "output_tokens": usage.get("output_tokens"),
            "cost": res.get("total_cost_usd"),
            "duration_ms": res.get("duration_ms"),
            "turns": res.get("num_turns"),
            "subtype": res.get("subtype"),
            "model": ", ".join(models) if models else None,
        })
    rows.sort(key=lambda r: arm_key(r["arm"]))

    print("| " + " | ".join(COLS) + " |")
    print("|" + "|".join(["---"] * len(COLS)) + "|")
    for r in rows:
        if r["res"] is None:
            vals = [r["arm"], "-", "-", "-", "-", "NO RESULT EVENT", "-"]
        else:
            vals = [r["arm"], r["output_tokens"], r["cost"], r["duration_ms"],
                    r["turns"], r["subtype"], r["model"]]
        print("| " + " | ".join(cell(v) for v in vals) + " |")
    print()

    with_result = [r for r in rows if r["res"] is not None]
    tokens = [(r["arm"], r["output_tokens"]) for r in with_result if r["output_tokens"] is not None]
    durs = [r["duration_ms"] for r in with_result if r["duration_ms"] is not None]
    if len(with_result) < 2 or (not tokens and not durs):
        verdict, hint = "unclear", "fewer than 2 arms with result events"
    else:
        differ = len({t for _, t in tokens}) > 1 or len(set(durs)) > 1
        verdict = "yes" if differ else "no"
        seq = [t for a, t in sorted(tokens, key=lambda x: arm_key(x[0])) if a in ORDER[:4]]
        mono = all(a <= b for a, b in zip(seq, seq[1:]))
        hint = "monotonic hint: output_tokens non-decreasing none<low<medium<xhigh: %s" % (
            "yes" if mono else "no")
    print("--effort honoured? %s (%s)" % (verdict, hint))

    env_path = os.path.join(scr, "env_effort.txt")
    try:
        with open(env_path, encoding="utf-8") as fh:
            env_line = fh.read().strip()
    except OSError:
        env_line = "missing (env_effort.txt not found)"
    print("recorded env: %s" % env_line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
