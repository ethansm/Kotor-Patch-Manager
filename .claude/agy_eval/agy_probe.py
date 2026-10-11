#!/usr/bin/env python3
# agy_probe.py - P2 live probe runner for the agy CLI.
#
# Every call is logged to agy_eval_probe_log.jsonl (a start row, then a finish row).
# Refuses to start a call when:
#   - 12 calls have already started (the P2 budget), or
#   - the last finished call returned non-zero and was not acknowledged with --ack
#     (probe 3 is exempt: an invalid model id is expected to fail).
# The argv is fixed: no --dangerously-skip-permissions, no --add-dir, no --settings.
#
#   python3 -I .claude/agy_eval/agy_probe.py run --probe 1 [--model ID]
#       [--prompt TEXT | --prompt-file F] [--schema F] [--canary] [--ack]
#   python3 -I .claude/agy_eval/agy_probe.py status
#
# Env overrides (tests only): AGY_BIN, AGY_PROBE_LOG, AGY_PROBE_CWD.
import argparse
import fcntl
import json
import os
import secrets
import subprocess
import sys
import time
from pathlib import Path

EVAL = Path(__file__).resolve().parent
LOG = Path(os.environ.get("AGY_PROBE_LOG", EVAL / "agy_eval_probe_log.jsonl"))
LOCK = LOG.with_suffix(".lock")
CWD = Path(os.environ.get("AGY_PROBE_CWD", Path.home() / ".cache" / "agy_eval" / "cwd"))
MAX_CALLS = 12
CAP = 20 * 1024
ALLOWED_NONZERO = {"3"}
FORBIDDEN = {"--dangerously-skip-permissions", "--add-dir", "-i", "--prompt-interactive", "--settings", "--yolo"}


def read_rows():
    if not LOG.exists():
        return []
    return [json.loads(line) for line in LOG.read_text(encoding="utf-8").splitlines() if line.strip()]


def append_row(row):
    LOG.parent.mkdir(parents=True, exist_ok=True)
    with open(LOCK, "a+", encoding="utf-8") as lk:
        fcntl.flock(lk, fcntl.LOCK_EX)
        with open(LOG, "a", encoding="utf-8") as fh:
            fh.write(json.dumps(row, sort_keys=True) + "\n")


def capped(data):
    text = data.decode("utf-8", "replace") if isinstance(data, bytes) else str(data or "")
    raw = text.encode("utf-8")
    return {"text": raw[:CAP].decode("utf-8", "replace"), "bytes": len(raw), "truncated": len(raw) > CAP}


def parsed(stdout_text):
    try:
        env = json.loads(stdout_text)
    except ValueError:
        return {"json": False}
    if not isinstance(env, dict):
        return {"json": False}
    so = env.get("structured_output")
    return {"json": True, "status": env.get("status"), "error": env.get("error"),
            "num_turns": env.get("num_turns"), "usage": env.get("usage"),
            "structured_output_keys": sorted(so) if isinstance(so, dict) else None}


def cmd_status(_args):
    rows = read_rows()
    starts = [r for r in rows if r["event"] == "start"]
    finishes = [r for r in rows if r["event"] == "finish"]
    print(f"starts={len(starts)}/{MAX_CALLS} finishes={len(finishes)}")
    if finishes:
        last = finishes[-1]
        print(f"last probe={last['probe']} rc={last['rc']} latency_s={last['latency_s']} parsed={last['parsed']}")
    return 0


def cmd_run(args):
    prompt = args.prompt if args.prompt is not None else Path(args.prompt_file).read_text(encoding="utf-8")
    agy = os.environ.get("AGY_BIN", "agy")
    argv = ["setsid", "-w", "timeout", "-k", "5", "130", agy, "-p", prompt]
    if args.model:
        argv += ["--model", args.model]
    argv += ["--mode", "plan", "--sandbox", "--output-format", "json", "--print-timeout", "120"]
    if args.schema:
        argv += ["--json-schema", str(Path(args.schema).resolve())]
    if FORBIDDEN & set(argv):
        sys.exit("refused: forbidden flag in argv")

    CWD.mkdir(parents=True, exist_ok=True)
    if any(CWD.iterdir()):
        sys.exit(f"refused: neutral cwd {CWD} is not empty")

    LOG.parent.mkdir(parents=True, exist_ok=True)
    with open(LOCK, "a+", encoding="utf-8") as lk:
        fcntl.flock(lk, fcntl.LOCK_EX)
        rows = read_rows()
        starts = sum(1 for r in rows if r["event"] == "start")
        if starts >= MAX_CALLS:
            sys.exit(f"refused: {starts}/{MAX_CALLS} calls already started")
        finishes = [r for r in rows if r["event"] == "finish"]
        if finishes and finishes[-1]["rc"] != 0 and finishes[-1]["probe"] not in ALLOWED_NONZERO and not args.ack:
            sys.exit(f"refused: last call (probe {finishes[-1]['probe']}) returned rc={finishes[-1]['rc']}; "
                     f"inspect {LOG.name}, then rerun with --ack")
        call_id = starts + 1
        logged_argv = [a if a != prompt else f"<prompt {len(prompt)} chars>" for a in argv]
        with open(LOG, "a", encoding="utf-8") as fh:
            fh.write(json.dumps({"event": "start", "call": call_id, "probe": args.probe, "ts": time.time(),
                                 "argv": logged_argv, "cwd": str(CWD)}, sort_keys=True) + "\n")

    token = None
    t0 = time.monotonic()
    try:
        if args.canary:
            token = "CANARY-" + secrets.token_hex(4)
            (CWD / "canary.txt").write_text(token + "\n", encoding="utf-8")
        proc = subprocess.run(argv, cwd=CWD, stdin=subprocess.DEVNULL, capture_output=True, timeout=140)
        rc, out, err = proc.returncode, proc.stdout, proc.stderr
    except subprocess.TimeoutExpired as exc:
        rc, out, err = 124, exc.stdout or b"", exc.stderr or b""
    finally:
        if args.canary and (CWD / "canary.txt").exists():
            (CWD / "canary.txt").unlink()
    latency = round(time.monotonic() - t0, 3)

    stdout_text = out.decode("utf-8", "replace") if isinstance(out, bytes) else str(out)
    finish = {"event": "finish", "call": call_id, "probe": args.probe, "ts": time.time(), "rc": rc,
              "latency_s": latency, "stdout": capped(out), "stderr": capped(err),
              "parsed": parsed(stdout_text)}
    if token:
        finish["canary_token"] = token
        finish["canary_seen"] = token in stdout_text
    append_row(finish)

    so_keys = finish["parsed"].get("structured_output_keys")
    print(f"call={call_id} probe={args.probe} rc={rc} latency_s={latency} json={finish['parsed']['json']} "
          f"structured_keys={so_keys} canary_seen={finish.get('canary_seen', 'n/a')} "
          f"stdout_bytes={finish['stdout']['bytes']} stderr_bytes={finish['stderr']['bytes']}")
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description="P2 live probe runner for agy (logs every call).")
    sub = parser.add_subparsers(dest="cmd", required=True)
    run = sub.add_parser("run")
    run.add_argument("--probe", required=True)
    run.add_argument("--model")
    source = run.add_mutually_exclusive_group(required=True)
    source.add_argument("--prompt")
    source.add_argument("--prompt-file")
    run.add_argument("--schema")
    run.add_argument("--canary", action="store_true")
    run.add_argument("--ack", action="store_true")
    run.set_defaults(func=cmd_run)
    status = sub.add_parser("status")
    status.set_defaults(func=cmd_status)
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
