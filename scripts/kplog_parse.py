#!/usr/bin/env python3
"""kpatch.log reader (Phase 2 chunk P). Python 3, stdlib only; reads files, never writes anything.

Usage:
    kplog_parse.py FILE... [--patch P] [--channel C] [--level L] [--since MS]
                           [--merge] [--stats] [--bare] [--headers] [--tolerance S]

Line shapes (mirrored from KotorPatcher src/core/log_service.cpp and Patches/Common/KpLog.h):
    # kplog session 2026-10-03 16:58:01 | kpatch.log v1 | patcher ABI 1     header ("# " lines: session,
    # KOTOR_VERSION_SHA=...  /  # interfaces:  /  #   kpatch.log v1 by patcher    version SHA, interfaces, "# session info:")
    T=0.304 f=- logger-fixture/main WARN hello         core line: T = ms since service start (3-digit
    T=2000.001 f=12 mypatch/draw drew 12 quads         fraction), f = frame or "-", level token only for WARN/ERR
    T=2100.000 f=1 MARK before the cutscene            MARK (no patch/channel; the code files it at WARN level)
    T=5.000 f=- mypatch/kplog DROPPED 7 (ring 4, rate 3)   drop report, channel "kplog"
    57843211 mypatch/main WARN text                    fallback sink (old core, no service): <ms32> <patch>/<ch> [WARN |ERR ]text
  A line ending in "[...]" was cut by the core (232-byte text cap).
  The core only writes a level token for WARN and ERR, so every other line is reported as level "INFO";
  DEBUG/TRACE lines cannot be told apart from INFO in the file.

  --level L    max verbosity shown: ERR < WARN < INFO < DEBUG < TRACE (default: everything). Untokened lines count as INFO,
               so --level INFO/DEBUG/TRACE show them all and --level WARN keeps only WARN/ERR (and MARKs).
  --patch/--channel   exact match; MARK lines (global) always pass these two filters.
  --since MS   keep lines with t_ms >= MS.
  --bare       print strip_prefix(line) instead of the raw line (for tools that anchor on the message with ^).
  --headers    also print "# " header lines (hidden by default).
  --merge      one timeline across per-patch files sorted by T. Files are cut into segments at every "# kplog session"
               header; a segment's session start is (header wall clock - T of its first line), so a patch file opened late or
               rotated mid-run still lands in the right session. Segments whose starts are within --tolerance seconds
               (default 5) form one session and are merged. SEVERAL SESSIONS: each is merged separately, oldest first,
               each preceded by a "# --- session <start> (<files>) ---" separator. Segments without a header (fallback
               files) cannot be placed, so each is its own group (its ms32 clock is not comparable to T). Identical lines
               present in both a per-patch file and kp_log.txt (sink=both) are printed once.
  --stats      counts per patch/channel/level, DROPPED totals per patch, truncated, MARK, malformed.
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import Counter, defaultdict
from datetime import datetime
from pathlib import Path

LEVELS = ["ERR", "WARN", "INFO", "DEBUG", "TRACE"]
RANK = {n: i for i, n in enumerate(LEVELS)}
TRUNC = "[...]"

_ID = r"[A-Za-z0-9_.-]+"
_HEAD = r"^T=(\d+)\.(\d{3}) f=(\d+|-) "
RE_MARK = re.compile(_HEAD + r"MARK (.*)$")
RE_LINE = re.compile(_HEAD + "(" + _ID + ")/(" + _ID + r") (?:(WARN|ERR) )?(.*)$")
RE_DROPPED = re.compile(r"^DROPPED (\d+) \(ring (\d+), rate (\d+)\)$")
RE_FALLBACK = re.compile(r"^(\d+) (" + _ID + ")/(" + _ID + r") (?:(WARN|ERR) )?(.*)$")
RE_SESSION = re.compile(r"^# kplog session (\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)\b")
RE_PREFIX = re.compile(r"^(?:T=\d+\.\d{3} f=(?:\d+|-) (?:MARK |" + _ID + "/" + _ID + r" (?:(?:WARN|ERR) )?)"
                       r"|\d+ " + _ID + "/" + _ID + r" (?:(?:WARN|ERR) )?)")


def parse_line(s):
    """Parse one log line. Returns a dict, or None for blank/unrecognised lines.

    Keys: kind ('line'|'header'|'mark'|'dropped'|'fallback'), t_ms (float|None), frame (int|None), patch, channel,
    level, msg, truncated (bool). header adds `session` (wall-clock string, session line only); dropped adds
    dropped/ring/rate ints."""
    s = s.rstrip("\r\n")
    if s.startswith("# ") or s == "#":
        d = dict(kind="header", t_ms=None, frame=None, patch=None, channel=None, level=None,
                 msg=s[2:], truncated=False)
        m = RE_SESSION.match(s)
        if m:
            d["session"] = m.group(1)
        return d
    m = RE_MARK.match(s)
    if m:
        return dict(kind="mark", t_ms=_t(m), frame=_f(m), patch=None, channel=None, level="WARN",
                    msg=m.group(4), truncated=m.group(4).endswith(TRUNC))
    m = RE_LINE.match(s)
    if m:
        msg = m.group(7)
        d = dict(kind="line", t_ms=_t(m), frame=_f(m), patch=m.group(4), channel=m.group(5),
                 level=m.group(6) or "INFO", msg=msg, truncated=msg.endswith(TRUNC))
        dm = RE_DROPPED.match(msg) if d["channel"] == "kplog" and d["level"] == "INFO" else None
        if dm:
            d.update(kind="dropped", dropped=int(dm.group(1)), ring=int(dm.group(2)), rate=int(dm.group(3)))
        return d
    m = RE_FALLBACK.match(s)
    if m:
        msg = m.group(5)
        return dict(kind="fallback", t_ms=float(int(m.group(1))), frame=None, patch=m.group(2), channel=m.group(3),
                    level=m.group(4) or "INFO", msg=msg, truncated=msg.endswith(TRUNC))
    return None


def _t(m):
    return int(m.group(1)) + int(m.group(2)) / 1000.0


def _f(m):
    return None if m.group(3) == "-" else int(m.group(3))


def strip_prefix(s):
    """Bare message of a core or fallback line (a MARK gives its label); anything else is returned unchanged."""
    s2 = s.rstrip("\r\n")
    m = RE_PREFIX.match(s2)
    return s2[m.end():] if m else s


# ---------------------------------------------------------------- reading / filtering

def read_segments(path):
    """Split a file into segments at each '# kplog session' header. Returns a list of dicts:
    {file, session (wall string|None), entries [(raw, parsed)], malformed}."""
    segs = []
    cur = None
    text = Path(path).read_text(encoding="utf-8", errors="replace")
    for raw in text.splitlines():
        if not raw.strip():
            continue
        p = parse_line(raw)
        if p and p.get("session"):
            cur = dict(file=str(path), session=p["session"], entries=[], malformed=0)
            segs.append(cur)
        if cur is None:
            cur = dict(file=str(path), session=None, entries=[], malformed=0)
            segs.append(cur)
        if p is None:
            cur["malformed"] += 1
        else:
            cur["entries"].append((raw, p))
    return segs


def make_filter(patch=None, channel=None, level=None, since=None):
    maxrank = RANK[level] if level else None

    def keep(p):
        if p["kind"] == "header":
            return False
        if patch is not None and p["patch"] is not None and p["patch"] != patch:
            return False
        if channel is not None and p["channel"] is not None and p["channel"] != channel:
            return False
        if maxrank is not None and RANK[p["level"]] > maxrank:
            return False
        if since is not None and p["t_ms"] < since:
            return False
        return True
    return keep


def _epoch(seg):
    """Session start (unix seconds): header wall clock minus T of the first timed line. None without a header/lines."""
    if seg["session"] is None:
        return None
    wall = datetime.strptime(seg["session"], "%Y-%m-%d %H:%M:%S").timestamp()
    for _, p in seg["entries"]:
        if p["t_ms"] is not None:
            return wall - p["t_ms"] / 1000.0
    return None


def group_sessions(segs, tolerance=5.0):
    """Cluster segments into sessions. Returns [(epoch|None, [segments])], oldest first, unplaceable ones last."""
    placed, loose = [], []
    for sg in segs:
        e = _epoch(sg)
        (placed if e is not None else loose).append((e, sg))
    placed.sort(key=lambda x: x[0])
    groups = []
    for e, sg in placed:
        if groups and e - groups[-1][1] <= tolerance:
            groups[-1][2].append(sg)
            groups[-1][1] = e
        else:
            groups.append([e, e, [sg]])
    out = [(g[0], g[2]) for g in groups]
    out += [(None, [sg]) for _, sg in loose if any(p["t_ms"] is not None for _, p in sg["entries"])]
    return out


def merged_lines(segs, keep, tolerance=5.0):
    """Yield output lines: one timeline per session (see module doc)."""
    groups = group_sessions(segs, tolerance)
    multi = len(groups) > 1
    for epoch, members in groups:
        rows, seen = [], set()
        for sg in members:
            for raw, p in sg["entries"]:
                if p["t_ms"] is None or not keep(p):
                    continue
                key = (p["t_ms"], raw)
                if key in seen:
                    continue
                seen.add(key)
                rows.append((p["t_ms"], raw))
        rows.sort(key=lambda r: r[0])  # stable: ties keep file order
        if multi:
            when = datetime.fromtimestamp(epoch).strftime("%Y-%m-%d %H:%M:%S") if epoch is not None else "unknown"
            files = ", ".join(sorted({Path(sg["file"]).name for sg in members}))
            yield "# --- session %s (%s) ---" % (when, files)
        for _, raw in rows:
            yield raw


def stats(segs, keep):
    n = Counter()
    drops = defaultdict(lambda: [0, 0, 0])  # patch -> total, ring, rate
    trunc = marks = malformed = 0
    for sg in segs:
        malformed += sg["malformed"]
        for _, p in sg["entries"]:
            if not keep(p):
                continue
            if p["kind"] == "mark":
                marks += 1
            else:
                n[(p["patch"], p["channel"], p["level"])] += 1
            if p["kind"] == "dropped":
                d = drops[p["patch"]]
                d[0] += p["dropped"]
                d[1] += p["ring"]
                d[2] += p["rate"]
            trunc += p["truncated"]
    out = ["patch/channel level count"]
    for (pa, ch, lv), c in sorted(n.items()):
        out.append("%s/%s %s %d" % (pa, ch, lv, c))
    for pa, (t, r, q) in sorted(drops.items()):
        out.append("DROPPED %s total %d (ring %d, rate %d)" % (pa, t, r, q))
    out.append("truncated %d" % trunc)
    out.append("marks %d" % marks)
    out.append("malformed %d" % malformed)
    return out


def main(argv=None, out=None):
    out = out or sys.stdout
    ap = argparse.ArgumentParser(description="Read kpatch.log files (per-patch, kp_log.txt or fallback).")
    ap.add_argument("files", nargs="+", metavar="FILE")
    ap.add_argument("--patch")
    ap.add_argument("--channel")
    ap.add_argument("--level", type=str.upper, choices=LEVELS)
    ap.add_argument("--since", type=float, metavar="MS")
    ap.add_argument("--merge", action="store_true")
    ap.add_argument("--stats", action="store_true")
    ap.add_argument("--bare", action="store_true")
    ap.add_argument("--headers", action="store_true")
    ap.add_argument("--tolerance", type=float, default=5.0, metavar="S")
    a = ap.parse_args(argv)

    segs = []
    for f in a.files:
        try:
            segs += read_segments(f)
        except OSError as e:
            print("kplog_parse: %s" % e, file=sys.stderr)
            return 2
    keep = make_filter(a.patch, a.channel, a.level, a.since)
    if a.stats:
        for line in stats(segs, keep):
            print(line, file=out)
        return 0
    if a.merge:
        lines = merged_lines(segs, keep, a.tolerance)
    else:
        lines = (raw for sg in segs for raw, p in sg["entries"]
                 if keep(p) or (a.headers and p["kind"] == "header"
                                and a.patch is None and a.channel is None and a.level is None and a.since is None))
    for line in lines:
        print(strip_prefix(line) if a.bare else line, file=out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
