#!/usr/bin/env python3
"""KOTOR II log dispatcher: agents run this instead of choosing a parser or typing the game path. Python 3, stdlib only.

Usage:
    kotor2_logs.py status [--game-dir G] [--since-hours H]     one line per LIVE log (see rule below); H default 24
    kotor2_logs.py check NAME [extra args...] [--game-dir G]   route NAME to its parser with compact defaults, pass stdout + exit code through
    kotor2_logs.py routes                                      print the ROUTES table

status line:  <name>  <KB>KB  <age e.g. 3h12m>  <lines>L  <parser>     sorted by name; the age is the ONLY time-dependent field.
  Live log = name ends in `_log.txt` or `_metrics.txt`, or is `kpatch.log`, and carries no archive marker (`.1`, `.runN`, `.prevN`,
  `.buildN`, `.crashN`, `_pre*`, `_prev`, `_baseline`, `_sessionN`, `.old`). Rotated/archived copies are only counted ("skipped: N").
  Size-0 files are omitted; logs older than --since-hours collapse into one "stale: N logs" line.
check NAME: bare filename (resolved in the game dir) or a path; missing -> "missing: NAME", exit 2. Extra args go to the routed tool.
  Default game dir: $KOTOR2_ROOT or ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II (read-only here).
  The play-log class (ai_trace / ranged_ai / inventory_sort_filter) chains archive_play_logs.py -> analyze_play_logs.py
  inside a temp dir (removed afterwards; --no-write would print nothing) and prints only the analyzer's stdout summary lines.
"""
from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
DEFAULT_GAME_DIR = os.path.expanduser(
    os.environ.get("KOTOR2_ROOT", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"))

ENGFIX = "patch_manager_mods/engine_bug_research/tools/analyze_engfix_logs.py"
LUO = "scripts/luo_log_check.py"
RUNS = "patch_manager_mods/perf_lab_src/analyze_runs.py"
PLAY = "patch_manager_mods/mods678_research/analyze_play_logs.py"
ARCHIVE = "scripts/archive_play_logs.py"
TRIAGE = "scripts/log_triage.py"
SELFHEAL = "scripts/selfheal_log_check.py"
QUEUEHUD = "scripts/queue_hud_log_check.py"

# name -> (parser label, route kind). kinds: engfix, luo, runs, playlogs, selfheal, queuehud. Anything else -> log_triage.
ROUTES = {
    "companion-selfheal_log.txt": ("selfheal_log_check", "selfheal"),
    "queue-hud_log.txt": ("queue_hud_log_check", "queuehud"),
    "k2-dialogue-letterbox-fix_log.txt": ("analyze_engfix_logs", "engfix"),
    "levelup_override_log.txt": ("luo_log_check", "luo"),
    "perflab_log.txt": ("analyze_runs", "runs"),
    "perflab_metrics.txt": ("analyze_runs", "runs"),
    "robustnessfixes_log.txt": ("analyze_runs", "runs"),
    "texmem_log.txt": ("analyze_runs", "runs"),
    "activationfix_log.txt": ("analyze_runs", "runs"),
    "equipiconfix_log.txt": ("analyze_runs", "runs"),
    "k2-texture-memory-release_log.txt": ("analyze_runs", "runs"),
    "k2-activation-fix_log.txt": ("analyze_runs", "runs"),
    "k2-equip-icon-fix_log.txt": ("analyze_runs", "runs"),
    "perf-lab_log.txt": ("analyze_runs", "runs"),
    "k2-robustness-fixes_log.txt": ("analyze_runs", "runs"),
    "ai_trace.txt": ("analyze_play_logs", "playlogs"),
    "ranged_ai_log.txt": ("analyze_play_logs", "playlogs"),
    "ranged-ai_log.txt": ("analyze_play_logs", "playlogs"),
    "inventory_sort_filter_log.txt": ("analyze_play_logs", "playlogs"),
}
KPLOG_HEADER = "# kplog session"
KPLOG_HINT = "hint: scripts/kplog_parse.py <file> --stats for structured kplog fields"

_ARCHIVE_MARK = re.compile(r"(\.\d+\.|\.run\d|\.prev\d|\.build|\.crash|\.old\b|_pre|_prev|_baseline|session\d)", re.I)


def is_live_log(name: str) -> bool:
    if name == "kpatch.log":
        return True
    if not (name.endswith("_log.txt") or name.endswith("_metrics.txt")):
        return False
    return not _ARCHIVE_MARK.search(name)


def is_candidate(name: str) -> bool:
    """Matches the archive_play_logs globs (*log*.txt, kpatch.log, *metrics*.txt) - live or rotated."""
    low = name.lower()
    return name == "kpatch.log" or (low.endswith(".txt") and ("log" in low or "metrics" in low))


def first_line(path: str) -> str:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            return f.readline().rstrip("\n")
    except OSError:
        return ""


def kind_of(path: str) -> str:
    """Route kind for a file: ROUTES by basename, else kplog header -> 'kplog', else 'triage'."""
    base = os.path.basename(path)
    if base in ROUTES:
        return ROUTES[base][1]
    if first_line(path).startswith(KPLOG_HEADER):
        return "kplog"
    return "triage"


def parser_label(path: str) -> str:
    base = os.path.basename(path)
    if base in ROUTES:
        return ROUTES[base][0]
    return "log_triage+kplog" if first_line(path).startswith(KPLOG_HEADER) else "log_triage"


def _py(script: str) -> str:
    return str(REPO / script)


def route(name: str, game_dir: str) -> list[str]:
    """argv for the routed tool (not executed). `name` is a bare filename (resolved in game_dir) or a path."""
    path = name if os.sep in name else os.path.join(game_dir, name)
    kind = kind_of(path)
    py = sys.executable
    if kind == "engfix":
        return [py, _py(ENGFIX), path]
    if kind == "luo":
        return [py, _py(LUO), "--log", path]
    if kind == "runs":
        return [py, _py(RUNS), "--game-dir", os.path.dirname(os.path.abspath(path)) if os.sep in name else game_dir,
                "--summary", "--latest", "1"]
    if kind == "selfheal":
        return [py, _py(SELFHEAL), path]
    if kind == "queuehud":
        return [py, _py(QUEUEHUD), path]
    if kind == "playlogs":
        return [py, str(HERE / "kotor2_logs.py"), "_playlogs", os.path.dirname(os.path.abspath(path)) if os.sep in name else game_dir]
    return [py, _py(TRIAGE), path]


def _fmt_age(sec: float) -> str:
    m = max(0, int(sec // 60))
    return "%dh%02dm" % (m // 60, m % 60) if m >= 60 else "%dm" % m


def count_lines(path: str) -> int:
    n, last = 0, b"\n"
    with open(path, "rb") as f:
        while True:
            b = f.read(1 << 20)
            if not b:
                break
            n += b.count(b"\n")
            last = b[-1:]
    return n + (0 if last == b"\n" else 1)


def status(game_dir: str, since_hours: float, now: float | None = None, out=None) -> int:
    out = out or sys.stdout
    now = time.time() if now is None else now
    try:
        names = sorted(os.listdir(game_dir))
    except OSError:
        print("missing: %s" % game_dir, file=out)
        return 2
    rows, skipped, stale = [], 0, 0
    for n in names:
        p = os.path.join(game_dir, n)
        if not is_candidate(n) or not os.path.isfile(p):
            continue
        if not is_live_log(n):
            skipped += 1
            continue
        st = os.stat(p)
        if st.st_size == 0:
            continue
        age = now - st.st_mtime
        if age > since_hours * 3600:
            stale += 1
            continue
        rows.append("%s  %dKB  %s  %dL  %s" % (n, (st.st_size + 1023) // 1024, _fmt_age(age), count_lines(p), parser_label(p)))
    for r in rows:
        print(r, file=out)
    if stale:
        print("stale: %d logs" % stale, file=out)
    print("skipped: %d rotated/archived" % skipped, file=out)
    return 0


def playlogs(game_dir: str) -> int:
    """Chain archive_play_logs -> analyze_play_logs --no-write inside a temp dir; print only the analyzer's stdout."""
    tmp = tempfile.mkdtemp(prefix="kotor2_logs_")
    try:
        a = subprocess.run([sys.executable, _py(ARCHIVE), "--game-dir", game_dir, "--dest", tmp, "--tag", "check"],
                           capture_output=True, text=True)
        if a.returncode != 0:
            sys.stderr.write(a.stderr)
            return a.returncode
        subdirs = sorted(d for d in os.listdir(tmp) if os.path.isdir(os.path.join(tmp, d)))
        if not subdirs:
            print("missing: archive produced no directory")
            return 2
        # --no-write makes the analyzer print nothing, so let it write REPORT.md into the temp archive copy (never the game dir)
        # and print only its stdout summary lines (drop the "wrote <tmp path>" line: temp path is noise).
        r = subprocess.run([sys.executable, _py(PLAY), os.path.join(tmp, subdirs[-1])], capture_output=True, text=True)
        sys.stdout.write("".join(l + "\n" for l in r.stdout.splitlines() if not l.startswith("wrote ")))
        if r.returncode != 0:
            sys.stderr.write(r.stderr)
        return r.returncode
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def print_routes(out=None) -> None:
    out = out or sys.stdout
    for n in sorted(ROUTES):
        print("%s -> %s" % (n, ROUTES[n][0]), file=out)
    print("# kplog session header -> log_triage (+ kplog_parse hint)", file=out)
    print("everything else -> log_triage", file=out)


ALIASES = {
    "selfheal": "companion-selfheal_log.txt",
    "companion-selfheal": "companion-selfheal_log.txt",
    "queue-hud": "queue-hud_log.txt",
    "queuehud": "queue-hud_log.txt",
}


def check(name: str, extra: list[str], game_dir: str) -> int:
    resolved = ALIASES.get(name, name)
    path = resolved if os.sep in resolved else os.path.join(game_dir, resolved)
    if not os.path.isfile(path):
        print("missing: %s" % name)
        return 2
    argv = route(resolved, game_dir)
    kind = kind_of(path)
    if kind not in ("playlogs",):
        argv = argv + list(extra)
    rc = subprocess.run(argv).returncode
    if kind == "kplog":
        print(KPLOG_HINT)
    return rc


CHECK_USAGE = """usage: kotor2_logs.py check NAME [extra args...] [--game-dir G]
  extra args pass through to the routed tool; for the log_triage route:
  --new              only lines added since the last --new, then advance the bookmark
  --no-advance       with --new: do not move the bookmark
  --all-launches     every launch instead of the last one
  --launch N         one launch by index
  other routes: run `kotor2_logs.py routes` and that tool's --help"""


def main(argv=None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if not argv or argv[0] in ("-h", "--help"):
        print(__doc__)
        return 0
    cmd, rest = argv[0], argv[1:]
    if cmd == "routes":
        print_routes()
        return 0
    if cmd == "_playlogs":
        return playlogs(rest[0])
    if cmd == "status":
        ap = argparse.ArgumentParser(prog="kotor2_logs.py status")
        ap.add_argument("--game-dir", default=DEFAULT_GAME_DIR)
        ap.add_argument("--since-hours", type=float, default=24)
        a = ap.parse_args(rest)
        return status(a.game_dir, a.since_hours)
    if cmd == "check":
        if not rest:
            print(CHECK_USAGE, file=sys.stderr)
            return 2
        if rest[0] in ("-h", "--help"):
            print(CHECK_USAGE)
            return 0
        game_dir, name, extra, i = DEFAULT_GAME_DIR, rest[0], [], 1
        while i < len(rest):
            if rest[i] == "--game-dir" and i + 1 < len(rest):
                game_dir = rest[i + 1]
                i += 2
                continue
            extra.append(rest[i])
            i += 1
        return check(name, extra, game_dir)
    print("unknown command: %s (status|check|routes)" % cmd, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
