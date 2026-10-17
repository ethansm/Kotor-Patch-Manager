#!/usr/bin/env python3
"""Cross-mod merged combat timeline for KOTOR II KpLog logs. Python 3, stdlib only; reads files, never writes.

Usage:
    combat_timeline.py [--dir GAME_DIR | LOG...] [--session N|last] [--combat N] [--max-lines 40] [--json]

Built on scripts/kplog_parse.py (line parsing, session grouping, T clock); mods are the KpLog patch ids. Default input is the
game dir (kotor2_logs.DEFAULT_GAME_DIR): every live *_log.txt with a timeline. Default session: last (1 = oldest). T is the
raw kplog T (ms since service start, 3 decimals); a session is one merged clock.

Evidence lines (message after the KpLog prefix):
  queue-hud  MODE m=<k> combat=<0|1|?> ... why=<w>   (m=0 combat is a state source "<mod>/MODE"; every MODE line is listed)
  queue-hud  EDGE what=combat old=<a> new=<b> ...      (rising/falling edge, source "<mod>/EDGE"; listed, not a DISAGREE source)
  any mod    CSTATE combat=<a>><b> paused=.. menu=..   (state source "<mod>"; "CSTATE cap reached" is ignored)
  any mod    HANDOFF* / ROUND* / BANNER* / E1 / E2 (companion-ai "t=<ms> E1 ..." has its t= stamp dropped)
Combat window: starts at the first rising edge from any source, ends at the next falling edge after which every MODE/CSTATE source
that reports 0|1 says 0 ('?' never votes; EDGE only counts when no MODE/CSTATE source exists), or at the last line of the session (marked "open").
DISAGREE: an interval > 250 ms where one MODE/CSTATE source says combat=1 and another says 0, inside a window.
Output per combat: COMBAT #n T=<start>..<end> dur=<s>; up to --max-lines "T  <mod>  <compact line>" rows, "(+N more)";
DISAGREE rows; final "SUMMARY combats=N lines=N disagreements=N". Exit 0, or 2 when the input is missing.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
from datetime import datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kplog_parse as KP  # noqa: E402
from kotor2_logs import DEFAULT_GAME_DIR, is_live_log  # noqa: E402

DISAGREE_MS = 250.0
TEXT_CAP = 110
RE_KV = re.compile(r"(\w+)=(\S*)")
RE_CSTATE = re.compile(r"^CSTATE combat=([01?])>([01?])\b")
RE_E12 = re.compile(r"^(?:t=\d+ )?(E[12]) (.*)$")
LISTED_PREFIXES = ("HANDOFF", "ROUND", "BANNER")


def _cap(s):
    return s if len(s) <= TEXT_CAP else s[:TEXT_CAP - 3] + "..."


def classify(patch, msg):
    """None, or dict(kind, text, votes[(src, name, new, old|None)]). votes = combat-state observations from this line."""
    m = RE_CSTATE.match(msg)
    if m:
        return dict(kind="CSTATE", text=_cap(msg), votes=[((patch, "CSTATE"), patch, m.group(2), m.group(1))])
    if msg.startswith("CSTATE"):
        return None  # "CSTATE cap reached" etc.
    m = RE_E12.match(msg)
    if m:
        return dict(kind=m.group(1), text=_cap("%s %s" % (m.group(1), m.group(2))), votes=[])
    first = msg.split(" ", 1)[0]
    kv = dict(RE_KV.findall(msg))
    if first == "MODE":
        votes = []
        comb = kv.get("combat")
        if kv.get("m") == "0" and comb in ("0", "1", "?"):
            votes = [((patch, "MODE"), patch + "/MODE", comb, None)]
        text = "MODE m=%s combat=%s soft=%s menu=%s paused=%s why=%s" % tuple(
            kv.get(k, "?") for k in ("m", "combat", "soft", "menu", "paused", "why"))
        return dict(kind="MODE", text=text, votes=votes)
    if first == "EDGE":
        votes = []
        if kv.get("what") == "combat" and kv.get("new") in ("0", "1"):
            votes = [((patch, "EDGE"), patch + "/EDGE", kv["new"], kv.get("old"))]
        text = "EDGE what=%s old=%s new=%s src=%s" % tuple(kv.get(k, "?") for k in ("what", "old", "new", "src"))
        return dict(kind="EDGE", text=text, votes=votes)
    if first.startswith(LISTED_PREFIXES):
        return dict(kind=first, text=_cap(msg), votes=[])
    return None


def session_rows(members):
    """Merged, de-duplicated (t, patch, msg) rows of one session, sorted by T (stable)."""
    rows, seen = [], set()
    for sg in members:
        for _, p in sg["entries"]:
            if p["t_ms"] is None or p["kind"] not in ("line", "fallback") or p["patch"] is None:
                continue
            key = (p["t_ms"], p["patch"], p["channel"], p["msg"])
            if key in seen:
                continue
            seen.add(key)
            rows.append((p["t_ms"], p["patch"], p["msg"]))
    rows.sort(key=lambda r: r[0])
    return rows


def find_combats(rows):
    """[dict(start, end, open, lines[(t, mod, text)], snaps[(t, {src: val})])] for one session's rows."""
    state = {}   # (patch, type) -> '0'|'1'|'?'
    names = {}   # (patch, type) -> display name
    combats, cur = [], None
    last_t = rows[-1][0] if rows else 0.0
    for t, patch, msg in rows:
        ev = classify(patch, msg)
        if ev is None:
            continue
        rising = falling = changed = False
        for src, name, new, old in ev["votes"]:
            prev = old if old in ("0", "1", "?") else state.get(src)
            if new == "1" and prev != "1":
                rising = True
            if new == "0" and prev == "1":
                falling = True
            if state.get(src) != new:
                changed = True
            state[src] = new
            names[src] = name
        if cur is None:
            if not rising:
                continue
            cur = dict(start=t, end=None, open=True, lines=[], snaps=[])
            combats.append(cur)
        cur["lines"].append((t, patch, ev["text"]))
        if changed or not cur["snaps"]:
            cur["snaps"].append((t, {names[s]: v for s, v in state.items()}))
        if falling:
            votes = [v for k, v in state.items() if k[1] != "EDGE" and v in ("0", "1")]
            if not votes:   # EDGE-only logs: the falling edge itself closes the window
                votes = [v for v in state.values() if v in ("0", "1")]
            if votes and all(v == "0" for v in votes):
                cur["end"], cur["open"] = t, False
                cur = None
    for c in combats:
        if c["end"] is None:
            c["end"] = max(last_t, c["lines"][-1][0])
    return combats


def disagreements(c):
    """Intervals > DISAGREE_MS inside a window where a MODE/CSTATE source says 1 and another says 0."""
    out, snaps = [], c["snaps"]
    for i, (t0, st) in enumerate(snaps):
        t1 = snaps[i + 1][0] if i + 1 < len(snaps) else c["end"]
        if round(t1 - t0, 3) <= 0:
            continue
        ones = sorted(n for n, v in st.items() if v == "1" and not n.endswith("/EDGE"))
        zeros = sorted(n for n, v in st.items() if v == "0" and not n.endswith("/EDGE"))
        if not (ones and zeros):
            continue
        if out and out[-1]["ones"] == ones and out[-1]["zeros"] == zeros and out[-1]["t1"] == t0:
            out[-1]["t1"] = t1
        else:
            out.append(dict(t0=t0, t1=t1, ones=ones, zeros=zeros))
    res = []
    for d in out:
        d["dur_ms"] = round(d["t1"] - d["t0"], 3)
        if d["dur_ms"] > DISAGREE_MS:
            res.append(d)
    return res


def gather_files(dirpath):
    out = []
    for name in sorted(os.listdir(dirpath)):
        p = os.path.join(dirpath, name)
        if name != "kpatch.log" and is_live_log(name) and os.path.isfile(p) and os.path.getsize(p) > 0:
            out.append(p)
    return out


def fmt_t(t):
    return "%.3f" % t


def build(sessions, sel, combat_n, max_lines):
    """-> (report dict). `sessions` = group_sessions result; `sel` = 1-based index of the session shown."""
    epoch, members = sessions[sel - 1]
    combats = find_combats(session_rows(members))
    shown = []
    for n, c in enumerate(combats, 1):
        if combat_n is not None and n != combat_n:
            continue
        dis = disagreements(c)
        shown.append(dict(n=n, start=c["start"], end=c["end"], open=c["open"], dur_s=round((c["end"] - c["start"]) / 1000.0, 3),
                          total_lines=len(c["lines"]),
                          lines=[dict(t=t, mod=m, text=x) for t, m, x in c["lines"][:max_lines]],
                          more=max(0, len(c["lines"]) - max_lines), disagree=dis))
    when = datetime.fromtimestamp(epoch).strftime("%Y-%m-%d %H:%M:%S") if epoch is not None else "unknown"
    return dict(session=sel, sessions=len(sessions), when=when,
                files=sorted({Path(sg["file"]).name for sg in members}), combats=shown,
                summary=dict(combats=len(shown), lines=sum(c["total_lines"] for c in shown),
                             disagreements=sum(len(c["disagree"]) for c in shown)))


def render(rep, out):
    print("SESSION %d/%d %s (%s)" % (rep["session"], rep["sessions"], rep["when"], ", ".join(rep["files"])), file=out)
    for c in rep["combats"]:
        print("COMBAT #%d T=%s..%s dur=%.3fs%s" % (c["n"], fmt_t(c["start"]), fmt_t(c["end"]), c["dur_s"],
                                                   " (open)" if c["open"] else ""), file=out)
        for ln in c["lines"]:
            print("%s  %s  %s" % (fmt_t(ln["t"]), ln["mod"], ln["text"]), file=out)
        if c["more"]:
            print("(+%d more)" % c["more"], file=out)
        for d in c["disagree"]:
            print("DISAGREE T=%s..%s dur=%gms combat=1:[%s] combat=0:[%s]" % (
                fmt_t(d["t0"]), fmt_t(d["t1"]), d["dur_ms"], ",".join(d["ones"]), ",".join(d["zeros"])), file=out)
    s = rep["summary"]
    print("SUMMARY combats=%d lines=%d disagreements=%d" % (s["combats"], s["lines"], s["disagreements"]), file=out)


def main(argv=None, out=None):
    out = out or sys.stdout
    ap = argparse.ArgumentParser(description="Merged cross-mod combat timeline from KpLog logs.")
    ap.add_argument("files", nargs="*", metavar="LOG")
    ap.add_argument("--dir", help="game dir (default: $KOTOR2_ROOT or the Steam install)")
    ap.add_argument("--session", default="last", help="N (1 = oldest) or last (default)")
    ap.add_argument("--combat", type=int, help="show only combat #N of the session")
    ap.add_argument("--max-lines", type=int, default=40)
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args(argv)

    files = list(a.files)
    if not files:
        d = a.dir or DEFAULT_GAME_DIR
        if not os.path.isdir(d):
            print("missing: %s" % d, file=out)
            return 2
        files = gather_files(d)
        if not files:
            print("missing: no *_log.txt files in %s" % d, file=out)
            return 2
    segs = []
    for f in files:
        if not os.path.isfile(f):
            print("missing: %s" % f, file=out)
            return 2
        segs += KP.read_segments(f)
    sessions = KP.group_sessions(segs)
    if not sessions:
        print("missing: no timed kplog lines in input", file=out)
        return 2
    if a.session == "last":
        sel = len(sessions)
    else:
        try:
            sel = int(a.session)
        except ValueError:
            sel = 0
        if not 1 <= sel <= len(sessions):
            print("missing: session %s (have %d)" % (a.session, len(sessions)), file=out)
            return 2
    rep = build(sessions, sel, a.combat, max(0, a.max_lines))
    if a.json:
        print(json.dumps(rep, indent=1), file=out)
    else:
        render(rep, out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
