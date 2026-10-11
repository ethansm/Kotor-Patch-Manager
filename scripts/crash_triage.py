#!/usr/bin/env python3
"""crash_triage.py - KOTOR II crash and hang triage CLI (Proton appid 208580). Python 3, stdlib only.

Usage:
    crash_triage.py proc [--game-dir DIR]             check if game process is running, hung, or stopped
    crash_triage.py log [--log PATH] [--limit N]      bounded signal/exception/backtrace scan from crash log
    crash_triage.py kpm [--log PATH]                  check KPM detour count and patcher initialization banner
    crash_triage.py kill [--timeout SEC]              gracefully terminate hung game process and check teardown

Every subcommand ends with a single unambiguous `VERDICT:` line.
"""
from __future__ import annotations

import argparse
import os
import re
import signal
import subprocess
import sys
import time
from pathlib import Path

DEFAULT_PROTON_LOG = os.path.expanduser("~/steam-208580.log")
DEFAULT_GAME_DIR = os.path.expanduser(
    os.environ.get("KOTOR2_ROOT", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
)

SIGNAL_REGEX = re.compile(
    r"(unhandled exception|backtrace: --- Exception|EXCEPTION_|err:module|err:seh|Backtrace:)",
    re.IGNORECASE
)
OFFSET_REGEX = re.compile(r"swkotor2\s*\(\+(0x[0-9a-fA-F]+)\)|0x00[4-8][0-9a-fA-F]{5}")


def get_game_pids() -> list[tuple[int, str]]:
    """Return [(pid, cmdline)] matching the real swkotor2.exe process."""
    try:
        out = subprocess.check_output(["pgrep", "-af", r"^S:.*swkotor2\.exe"], text=True)
        results = []
        for line in out.strip().splitlines():
            parts = line.strip().split(None, 1)
            if parts and parts[0].isdigit():
                results.append((int(parts[0]), parts[1] if len(parts) > 1 else ""))
        return results
    except subprocess.CalledProcessError:
        return []
    except FileNotFoundError:
        return []


def get_proc_stats(pid: int) -> dict[str, str]:
    """Return process stat, pcpu, etime via ps."""
    try:
        out = subprocess.check_output(["ps", "-o", "stat,pcpu,etime", "-p", str(pid)], text=True)
        lines = out.strip().splitlines()
        if len(lines) >= 2:
            cols = lines[1].split()
            return {
                "stat": cols[0] if len(cols) > 0 else "?",
                "pcpu": cols[1] if len(cols) > 1 else "?",
                "etime": cols[2] if len(cols) > 2 else "?"
            }
    except Exception:
        pass
    return {"stat": "?", "pcpu": "?", "etime": "?"}


def cmd_proc(args: argparse.Namespace) -> int:
    procs = get_game_pids()
    if not procs:
        print("swkotor2.exe is NOT running.")
        print("VERDICT: STOPPED")
        return 0

    pid, cmdline = procs[0]
    stats = get_proc_stats(pid)
    stat = stats["stat"]
    pcpu_str = stats["pcpu"]
    try:
        pcpu = float(pcpu_str)
    except ValueError:
        pcpu = 0.0

    print(f"swkotor2.exe is running (pid={pid}, stat={stat}, cpu={pcpu_str}%, etime={stats['etime']})")
    print(f"  cmdline: {cmdline}")

    # R or RN at high CPU indicates spinning hang
    if ("R" in stat) and pcpu >= 75.0:
        print(f"  State is {stat} at {pcpu_str}% CPU -> spinning load or black-screen hang.")
        print("VERDICT: HANG")
    else:
        print("VERDICT: RUNNING")
    return 0


def cmd_log(args: argparse.Namespace) -> int:
    log_path = Path(args.log)
    if not log_path.exists():
        print(f"Proton log not found: {log_path}")
        print("VERDICT: NO_LOG")
        return 2

    st = log_path.stat()
    mtime_str = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(st.st_mtime))
    size_mb = st.st_size / (1024 * 1024)
    print(f"Log: {log_path} ({size_mb:.2f} MB, modified {mtime_str})")

    matches = []
    offsets = []
    with log_path.open("r", encoding="utf-8", errors="replace") as f:
        for line_no, line in enumerate(f, 1):
            if SIGNAL_REGEX.search(line):
                matches.append((line_no, line.rstrip("\n")))
                for m in OFFSET_REGEX.finditer(line):
                    val = m.group(1) or m.group(0)
                    if val not in offsets:
                        offsets.append(val)
                if len(matches) >= args.limit:
                    break

    if not matches:
        print("No exception, fault, or signal patterns found in log.")
        print("VERDICT: CLEAN")
        return 0

    print(f"Found {len(matches)} exception/fault signatures (capped at {args.limit}):")
    for lno, text in matches[:15]:
        print(f"  L{lno:06d}: {text[:120]}")
    if len(matches) > 15:
        print(f"  ... and {len(matches) - 15} more lines.")

    if offsets:
        print(f"Candidate crash addresses/offsets: {', '.join(offsets[:5])}")
        print(f"  Resolve with: python3 scripts/query_ghidra.py address {offsets[0]}")
        print(f"VERDICT: FAULT({offsets[0]})")
    else:
        print("VERDICT: EXCEPTION")
    return 0


def cmd_kpm(args: argparse.Namespace) -> int:
    log_path = Path(args.log)
    if not log_path.exists():
        print(f"Log not found: {log_path}")
        print("VERDICT: NO_LOG")
        return 2

    detour_count = 0
    init_success = False
    with log_path.open("r", encoding="utf-8", errors="replace") as f:
        for line in f:
            if "Applied DETOUR" in line:
                detour_count += 1
            elif "SUCCESS: Patcher initialized" in line:
                init_success = True

    print(f"KPM Detours applied: {detour_count}")
    print(f"Patcher initialized: {'YES' if init_success else 'NO'}")

    if init_success and detour_count > 0:
        print(f"VERDICT: PASS(detours={detour_count})")
        return 0
    elif not init_success:
        print("VERDICT: ABORTED")
        return 1
    else:
        print("VERDICT: NO_DETOURS")
        return 0


def cmd_kill(args: argparse.Namespace) -> int:
    procs = get_game_pids()
    if not procs:
        print("No swkotor2.exe process found.")
        print("VERDICT: NOT_RUNNING")
        return 0

    pid = procs[0][0]
    print(f"Sending SIGTERM to swkotor2.exe (PID {pid})...")
    try:
        os.kill(pid, signal.SIGTERM)
    except OSError as e:
        print(f"Error terminating process: {e}")

    # Wait gracefully
    deadline = time.time() + args.timeout
    alive = True
    while time.time() < deadline:
        if not get_game_pids():
            alive = False
            break
        time.sleep(0.5)

    if alive:
        print(f"Process still alive after {args.timeout}s; escalating to SIGKILL...")
        try:
            os.kill(pid, signal.SIGKILL)
            time.sleep(1.0)
        except OSError:
            pass

    if not get_game_pids():
        print(f"Process {pid} terminated successfully.")
        print("VERDICT: KILLED")
        return 0
    else:
        print(f"Process {pid} could not be terminated.")
        print("VERDICT: KILL_FAILED")
        return 1


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)

    p_proc = sub.add_parser("proc", help="Check process state (running, hung, stopped)")
    p_proc.add_argument("--game-dir", default=DEFAULT_GAME_DIR, help="Game directory")

    p_log = sub.add_parser("log", help="Bounded exception/signal scan in crash log")
    p_log.add_argument("--log", default=DEFAULT_PROTON_LOG, help="Proton log path (default ~/steam-208580.log)")
    p_log.add_argument("--limit", type=int, default=40, help="Max lines to match (default 40)")

    p_kpm = sub.add_parser("kpm", help="Check KPM detours and patcher initialization")
    p_kpm.add_argument("--log", default=DEFAULT_PROTON_LOG, help="Proton log path")

    p_kill = sub.add_parser("kill", help="Gracefully terminate hung swkotor2.exe")
    p_kill.add_argument("--timeout", type=float, default=5.0, help="Seconds to wait before SIGKILL (default 5.0)")

    args = p.parse_args(argv)
    dispatch = {
        "proc": cmd_proc,
        "log": cmd_log,
        "kpm": cmd_kpm,
        "kill": cmd_kill
    }
    return dispatch[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
