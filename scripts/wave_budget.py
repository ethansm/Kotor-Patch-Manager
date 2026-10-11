#!/usr/bin/env python3
"""wave_budget.py - token-budget check and window splitter for subagent file briefs. Python 3, stdlib only.

Usage:
  wave_budget.py check <entry>... [--brief-bytes N]
  wave_budget.py windows <path>

entry     `path` or `path:lo-hi` (1-based inclusive line range; hi clamps to EOF, lo past EOF or lo>hi -> 0).
          Estimate = ceil(bytes/4) for .md/.txt, ceil(bytes/3.5) otherwise. Bytes are raw (CRLF counts).
check     prints `total=<n> per_entry=[...] verdict=OK|OVER (<reasons>)`.
          total = overhead + ceil(brief_bytes/4) + sum(entries). Budget: total <= 60000, each entry <= 12000
          tokens, at most 3 entries. overhead defaults to 10000 and is read from WAVE_OVERHEAD_TOKENS at call time.
          Exit 0 ok, 1 over budget, 2 usage or missing file.
windows   prints contiguous `path:lo-hi` windows covering the whole file, each <= 12000 tokens (a single
          line over the cap cannot be split and gets its own window). Exit 0, 2 on a missing file.

Read-only. Reads only the files named on the command line.
"""
import argparse
import io
import math
import os
import re
import sys
from fractions import Fraction
from pathlib import Path

TOTAL_CAP = 60000
PER_ENTRY_CAP = 12000
MAX_ENTRIES = 3
DEFAULT_OVERHEAD = 10000
PROSE_SUFFIXES = (".md", ".txt")
_ENTRY_RE = re.compile(r"(.*):(\d+)-(\d+)", re.DOTALL)


def _ceil_div(a, b):
    return -((-a) // b)


def _is_prose(path):
    return os.path.splitext(path)[1].lower() in PROSE_SUFFIXES


def _tokens(nbytes, path):
    if _is_prose(path):
        return _ceil_div(nbytes, 4)
    return _ceil_div(2 * nbytes, 7)  # ceil(nbytes / 3.5)


def _lines(data):
    # Split on b"\n" only, matching sed -n / wc -l line numbering. Lines keep their raw bytes.
    return io.BytesIO(data).readlines()


def _parse(entry):
    m = _ENTRY_RE.fullmatch(entry)
    if not m:
        return entry, None, None
    lo, hi = int(m.group(2)), int(m.group(3))
    if lo < 1:
        raise ValueError(f"line range must start at 1 or later: {entry!r}")
    return m.group(1), lo, hi


def estimate_entry(entry):
    """Token estimate for `path` or `path:lo-hi`. Missing file raises FileNotFoundError."""
    path, lo, hi = _parse(entry)
    data = Path(path).read_bytes()
    if lo is None:
        return _tokens(len(data), path)
    lines = _lines(data)
    hi = min(hi, len(lines))
    if lo > hi:
        return 0
    return _tokens(sum(len(line) for line in lines[lo - 1:hi]), path)


def check(entries, brief_bytes=0, overhead=None):
    """Budget verdict for a list of entries. overhead=None reads WAVE_OVERHEAD_TOKENS at call time."""
    entries = list(entries)
    if overhead is None:
        overhead = int(os.environ.get("WAVE_OVERHEAD_TOKENS") or DEFAULT_OVERHEAD)
    per_entry = [estimate_entry(e) for e in entries]
    over_cap = [e for e, n in zip(entries, per_entry) if n > PER_ENTRY_CAP]
    total = overhead + _ceil_div(brief_bytes, 4) + sum(per_entry)
    too_many = len(entries) > MAX_ENTRIES
    return {
        "total": total,
        "per_entry": per_entry,
        "over_cap": over_cap,
        "too_many": too_many,
        "ok": total <= TOTAL_CAP and not over_cap and not too_many,
    }


def suggest_windows(path, cap=PER_ENTRY_CAP):
    """Contiguous `path:lo-hi` windows covering the file, each estimated <= cap tokens."""
    if cap <= 0:
        raise ValueError("cap must be positive")
    p = os.fspath(path)
    lines = _lines(Path(p).read_bytes())
    n = len(lines)
    if n == 0:
        return []
    prefix = [0]
    for line in lines:
        prefix.append(prefix[-1] + len(line))
    total = prefix[-1]
    if _tokens(total, p) <= cap:
        return [f"{p}:1-{n}"]
    bytes_per_token = Fraction(4) if _is_prose(p) else Fraction(7, 2)
    # window lines = floor(0.9 * cap * bytes_per_token / avg_bytes_per_line), exact rational arithmetic
    per_window = max(1, math.floor(Fraction(9, 10) * cap * bytes_per_token * n / total))
    windows = []
    start = 0
    while start < n:
        end = min(n, start + per_window)
        # Dense long lines can push a window over the cap; shrink it until it fits (or is one line).
        while end - start > 1 and _tokens(prefix[end] - prefix[start], p) > cap:
            end -= 1
        windows.append(f"{p}:{start + 1}-{end}")
        start = end
    return windows


def main(argv=None):
    parser = argparse.ArgumentParser(description="Token budget check and window splitter for subagent briefs.")
    sub = parser.add_subparsers(dest="cmd", required=True)
    p_check = sub.add_parser("check", help="check entries against the wave budget")
    p_check.add_argument("entries", nargs="+", metavar="entry")
    p_check.add_argument("--brief-bytes", type=int, default=0, metavar="N")
    p_win = sub.add_parser("windows", help="split a file into <=12000-token windows")
    p_win.add_argument("path")
    args = parser.parse_args(argv)

    if args.cmd == "windows":
        try:
            windows = suggest_windows(args.path)
        except FileNotFoundError as exc:
            print(f"wave_budget: missing file: {exc.filename}", file=sys.stderr)
            return 2
        except (OSError, ValueError) as exc:
            print(f"wave_budget: {exc}", file=sys.stderr)
            return 2
        for window in windows:
            print(window)
        return 0

    if args.brief_bytes < 0:
        parser.error("--brief-bytes must be >= 0")
    try:
        res = check(args.entries, brief_bytes=args.brief_bytes)
    except FileNotFoundError as exc:
        print(f"wave_budget: missing file: {exc.filename}", file=sys.stderr)
        return 2
    except (OSError, ValueError) as exc:
        print(f"wave_budget: {exc}", file=sys.stderr)
        return 2

    reasons = []
    if res["total"] > TOTAL_CAP:
        reasons.append(f"total {res['total']} > {TOTAL_CAP}")
    reasons += [f"{e} > {PER_ENTRY_CAP}" for e in res["over_cap"]]
    if res["too_many"]:
        reasons.append(f"{len(args.entries)} entries > {MAX_ENTRIES}")
    verdict = "OK" if res["ok"] else "OVER (" + "; ".join(reasons) + ")"
    print(f"total={res['total']} per_entry={res['per_entry']} verdict={verdict}")
    return 0 if res["ok"] else 1


if __name__ == "__main__":
    sys.exit(main())
