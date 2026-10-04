#!/usr/bin/env python3
"""Generic KOTOR II mod-log summarizer. Python 3, stdlib only; reads logs, writes only the --new bookmark file.

For mod logs that have no dedicated parser (hotbar, buffhud, chareffects, equip3d, inv3d, shieldprobe, equipiconfix,
hangprobe, levelup, perflab, ...). Agents run this instead of tail/grep on raw logs. Output is deterministic (sorted,
stable ordering, no wall-clock, no mtime) and size-capped (--max-bytes), and the LAST line is always the VERDICT.

Usage:
    log_triage.py FILE... [--all-launches] [--launch N] [--top N] [--max-bytes N]
                          [--new] [--no-advance] [--state PATH] [--json]

  --all-launches  report every launch (default: only the last).      --launch N  report launch N (1-based).
  --top N         top-N signatures (default 12).                     --max-bytes N  per-file text cap (default 1800).
  --new           analyze only lines after this file's bookmark, then advance it (see below).
  --no-advance    with --new: do not move the bookmark.   --state PATH  bookmark file (default ~/.cache/kotor2_logs/bookmarks.json).
  --json          one JSON object per file, one per line, sorted keys (top/alerts capped, no byte trimming).

Text report (one block per file, blank line between files):
    FILE name | 646724 B | 6000 lines           size and total line count; no mtime
    launches: K (showing #K, lines A-B)         new: lines A-B (M new) / rotated: reading from line 1  (with --new)
    <banner line, raw, <=160 chars>
    patterns: U unique of N lines
    <count>  <first_line_no>  <signature>      top-N by count desc, then first line asc
    alerts:                                     deduplicated by signature, count/first line/text, sorted by first line, cap 10
    cap: likely                                 launch ends at EOF on a multiple of 1000 lines (>=1000), or last line says cap/limit reached
    tail:                                       last 5 non-blank raw lines (<=160 chars)
    VERDICT: CLEAN|ALERTS(n)|EMPTY  launches=K lines=N     ALWAYS the final line; n = distinct alert signatures

Launch split: a new launch starts at a line whose message (leading timestamp/tick prefix removed) is a banner,
  "<name> [v]<version> loaded" with an optional leading "== ", where <name> has a lowercase letter and no colon
  (also "<name> <version> attach", e.g. perflab; "force-power-hotbar 1.0.0 loaded", "buffhud loaded", "inv3d v2 loaded", "== LevelUpOverride 0.1.0-probe loaded";
  NOT "cfg: loaded x", "img: loaded x", "INI loaded: ...", "CONFIG loaded ..."). If the file has "# kplog session" headers
  ONLY those split (a KpLog patch also prints its own "... loaded" line right after the header). No banner anywhere = the whole
  file is one launch. Lines before the first banner form their own (earlier) launch.
Prefixes stripped before analysis: "[123 f0] ", "[t=123 f=0] ", "[123] ", "HH:MM:SS.mmm ", "[HH:MM:SS.mmm 123] ", "T=0.304 f=- ".
Signature: hex (0x.., 8+ hex digits containing a digit, runs of 3+ byte pairs) and decimal numbers (floats, negatives) -> "#",
  whitespace collapsed, cut at 100 chars; every hexdump continuation line ("    +10: 00 ff ...") is the single signature "+# hexdump".
Alerts: the banner line itself never alerts; a message matches (case-insensitive) \\b(warn|warning|err|error|fail|failed|failure|assert|exception|crash|abort|timeout|hang|
  null|bad|invalid|overflow|drop|dropped)\\b, except when every match is a zero counter ("dropped=0", "errors: 0", "0 failed probes").
Trimming (--max-bytes): tail lines are cut to 80 chars, then top-N is reduced to 3, alerts to 3, tail to 2, then top, alerts and
  tail down to 0; "(trimmed)" is appended to the launches line; VERDICT is never dropped.
--new: bookmark per file {inode,size,line} keyed by absolute path. Only lines after the bookmark are analyzed (launch split still
  applies inside that region); line numbers stay absolute. Inode change, shrink, or fewer lines than bookmarked prints
  "rotated: reading from line 1" and reads from the top. A trailing line without newline counts as a line.
Exit: 0, or 2 if any FILE is missing (prints "missing: PATH" and keeps going with the others).
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

DEFAULT_STATE = "~/.cache/kotor2_logs/bookmarks.json"
SIG_MAX = 100
RAW_MAX = 160

_TS = r"\d{2}:\d{2}:\d{2}\.\d{3}"
RE_PREFIX = re.compile(
    r"^(?:\[t=\d+ f=\d+\] ?"
    r"|\[" + _TS + r" \d+(?: [ft]\d+)?\] ?"
    r"|\[\d+(?: [ft]\d+)?\] ?"
    r"|" + _TS + r" "
    r"|T=\d+\.\d+ f=(?:\d+|-) )")
RE_SESSION = re.compile(r"^# kplog session\b")
RE_BANNER = re.compile(r"^(?:==\s*)?(?=[\w.\-]*[a-z])[A-Za-z][\w.\-]*"
                       r"(?:(?:\s+v?\d[\w.\-+]*)?\s+loaded\b|\s+v?\d[\w.\-+]*\s+attach\b)")
RE_HEXDUMP = re.compile(r"^\s+\+[0-9a-fA-F]+:")
RE_ALERT = re.compile(r"\b(warn|warning|err|error|fail(?:ed|ure)?|assert|exception|crash|abort|timeout|hang|null|bad|invalid"
                      r"|overflow|drop(?:ped)?)\b", re.I)
RE_ZERO_AFTER = re.compile(r"\s*[=:]\s*0(?:\.0+)?\b")
RE_ZERO_BEFORE = re.compile(r"\b0\s+$")
RE_CAP = re.compile(r"\bcap\b|limit reached", re.I)
RE_CTRL = re.compile(r"[\x00-\x08\x0b-\x1f\x7f]")
RE_HEX0X = re.compile(r"0[xX][0-9a-fA-F]+")
RE_HEXLONG = re.compile(r"\b(?=[0-9a-fA-F]*\d)[0-9a-fA-F]{8,}\b")
RE_HEXRUN = re.compile(r"\b[0-9a-fA-F]{2}(?: [0-9a-fA-F]{2}\b){2,}")
RE_NUM = re.compile(r"-?\d+(?:\.\d+)?")
RE_WS = re.compile(r"\s+")
HEXDUMP_SIG = "+# hexdump"


def strip_prefix(line):
    return RE_PREFIX.sub("", line, count=1)


def clean(s):
    return RE_CTRL.sub(" ", s)


def cut(s, n):
    s = clean(s)
    return s if len(s) <= n else s[:n - 3] + "..."


def signature(line):
    """Normalized signature of one raw log line (see module docstring)."""
    if RE_HEXDUMP.match(line):
        return HEXDUMP_SIG
    m = strip_prefix(line)
    m = RE_HEX0X.sub("#", m)
    m = RE_HEXLONG.sub("#", m)
    m = RE_HEXRUN.sub("#", m)
    m = RE_NUM.sub("#", m)
    m = RE_WS.sub(" ", m).strip()
    return m[:SIG_MAX] if m else "<blank>"


def is_banner(line):
    return bool(RE_BANNER.match(strip_prefix(line)))


def is_alert(line):
    if RE_HEXDUMP.match(line):
        return False
    msg = strip_prefix(line)
    found = False
    for m in RE_ALERT.finditer(msg):
        if not RE_ZERO_AFTER.match(msg, m.end()) and not RE_ZERO_BEFORE.search(msg[:m.start()]):
            found = True
            break
    return found


def split_launches(lines, first_no):
    """Return [(start_idx, end_idx_exclusive)] over `lines`. Empty list for no lines."""
    if not lines:
        return []
    sessions = [i for i, ln in enumerate(lines) if RE_SESSION.match(ln)]
    starts = sessions if sessions else [i for i, ln in enumerate(lines) if is_banner(ln)]
    if not starts or starts[0] != 0:
        starts = [0] + starts
    ends = starts[1:] + [len(lines)]
    return list(zip(starts, ends))


def analyze_launch(lines, first_no, total_lines_in_file, top, alert_cap):
    """Analyze a launch's lines (first_no = absolute 1-based number of lines[0])."""
    count = {}
    first = {}
    alerts = {}
    afirst = {}
    amsg = {}
    for i, ln in enumerate(lines):
        no = first_no + i
        sig = signature(ln)
        count[sig] = count.get(sig, 0) + 1
        first.setdefault(sig, no)
        if not (i == 0 and is_banner(ln)) and is_alert(ln):
            alerts[sig] = alerts.get(sig, 0) + 1
            if sig not in afirst:
                afirst[sig] = no
                amsg[sig] = cut(strip_prefix(ln).strip(), SIG_MAX)
    ranked = sorted(count, key=lambda s: (-count[s], first[s], s))
    top_rows = [dict(count=count[s], line=first[s], sig=s) for s in ranked]
    alert_rows = [dict(count=alerts[s], line=afirst[s], text=amsg[s]) for s in sorted(alerts, key=lambda s: (afirst[s], s))]
    end_no = first_no + len(lines) - 1
    nonblank = [ln for ln in lines if ln.strip()]
    last = nonblank[-1] if nonblank else ""
    cap = bool(lines) and ((end_no == total_lines_in_file and end_no >= 1000 and end_no % 1000 == 0)
                           or bool(RE_CAP.search(last)))
    return dict(start=first_no, end=end_no, lines=len(lines), banner=cut(lines[0], RAW_MAX) if lines else "",
                unique=len(count), top=top_rows, alerts=alert_rows, cap=cap,
                tail=[cut(ln, RAW_MAX) for ln in nonblank[-5:]])


def read_lines(path):
    data = Path(path).read_bytes()
    parts = data.decode("utf-8", errors="replace").split("\n")
    if parts and parts[-1] == "":
        parts.pop()
    return [p[:-1] if p.endswith("\r") else p for p in parts]


# ---------------------------------------------------------------- state

def load_state(path):
    try:
        with open(path, encoding="utf-8") as f:
            d = json.load(f)
        return d if isinstance(d, dict) else {}
    except (OSError, ValueError):
        return {}


def save_state(path, state):
    p = Path(path)
    p.parent.mkdir(parents=True, exist_ok=True)
    tmp = p.with_name(p.name + ".tmp")
    tmp.write_text(json.dumps(state, sort_keys=True, indent=1) + "\n", encoding="utf-8")
    os.replace(tmp, p)


# ---------------------------------------------------------------- render

def render_launch(L, top_n, alert_n, tail_n, tail_w, heading):
    out = []
    if heading:
        out.append(heading)
    out.append(L["banner"])
    out.append("patterns: %d unique of %d lines" % (L["unique"], L["lines"]))
    for r in L["top"][:top_n]:
        out.append("%d  %d  %s" % (r["count"], r["line"], r["sig"]))
    if L["alerts"]:
        extra = len(L["alerts"]) - min(alert_n, len(L["alerts"]))
        out.append("alerts:" + (" (+%d more)" % extra if extra > 0 and alert_n > 0 else ""))
        for r in L["alerts"][:alert_n]:
            out.append("%d  %d  %s" % (r["count"], r["line"], r["text"]))
    if L["cap"]:
        out.append("cap: likely")
    if tail_n and L["tail"]:
        out.append("tail:")
        out.extend(cut(t, tail_w) for t in L["tail"][-tail_n:])
    return out


def render_text(res, args):
    """Return the text block for one file result, trimmed to args.max_bytes."""
    verdict = res["verdict"]
    shown = res["shown"]

    def build(top_n, alert_n, tail_n, tail_w, trimmed):
        out = ["FILE %s | %d B | %d lines" % (res["name"], res["size"], res["total_lines"])]
        nw = res.get("new")
        if nw:
            if nw["rotated"]:
                out.append("rotated: reading from line 1")
            if nw["count"]:
                out.append("new: lines %d-%d (%d new)" % (nw["from"], nw["to"], nw["count"]))
        if shown:
            if len(shown) == 1 and not args.all_launches:
                L = shown[0]
                n = res["launches"]
                which = "#%d" % L["n"]
                out.append("launches: %d (showing %s, lines %d-%d)%s" % (n, which, L["start"], L["end"],
                                                                           " (trimmed)" if trimmed else ""))
                out.extend(render_launch(L, top_n, alert_n, tail_n, tail_w, None))
            else:
                out.append("launches: %d (showing all)%s" % (res["launches"], " (trimmed)" if trimmed else ""))
                for L in shown:
                    out.extend(render_launch(L, top_n, alert_n, tail_n, tail_w,
                                             "launch #%d lines %d-%d" % (L["n"], L["start"], L["end"])))
        out.append(verdict)
        return "\n".join(out)

    if res.get("note"):
        # ranges that matched nothing (e.g. bad --launch): keep it minimal
        txt = build(0, 0, 0, 0, False)
        return txt.replace("\n" + verdict, "\n" + res["note"] + "\n" + verdict)

    steps = [(args.top, 10, 5, RAW_MAX), (args.top, 10, 5, 80)]
    for t in (args.top, 3):
        for a in (10, 3):
            for tl in (5, 2):
                steps.append((min(t, args.top), a, tl, 80))
    steps += [(min(3, args.top), 3, 2, 80), (min(3, args.top), 3, 0, 80), (0, 3, 0, 80), (0, 0, 0, 80)]
    seen = set()
    best = None
    for st in steps:
        if st in seen:
            continue
        seen.add(st)
        best = build(*st, trimmed=(st != steps[0]))
        if len(best.encode("utf-8")) <= args.max_bytes:
            return best
    # last resort: hard-trim to header + verdict (+ launches line when it fits)
    lines = best.split("\n")
    keep = [lines[0], lines[-1]]
    for extra in lines[1:-1]:
        if extra.startswith("launches:"):
            keep.insert(1, extra if "(trimmed)" in extra else extra + " (trimmed)")
            break
    return "\n".join(keep)


# ---------------------------------------------------------------- main

def triage_file(path, args, state, st_new):
    p = Path(path)
    st = p.stat()
    all_lines = read_lines(p)
    total = len(all_lines)
    res = dict(name=p.name, path=str(p.resolve()), size=st.st_size, total_lines=total)
    start = 0
    new = None
    if args.new:
        key = str(p.resolve())
        bm = state.get(key)
        rotated = False
        if bm:
            if bm.get("inode") != st.st_ino or st.st_size < bm.get("size", 0) or bm.get("line", 0) > total:
                rotated = True
            else:
                start = bm.get("line", 0)
        new = dict(rotated=rotated, count=total - start, **{"from": start + 1, "to": total})
        res["new"] = new
        st_new[key] = dict(inode=st.st_ino, size=st.st_size, line=total)
    region = all_lines[start:]
    first_no = start + 1
    spans = split_launches(region, first_no)
    K = len(spans)
    res["launches"] = K
    note = None
    if not spans:
        pick = []
    elif args.all_launches:
        pick = list(range(K))
    elif args.launch:
        if 1 <= args.launch <= K:
            pick = [args.launch - 1]
        else:
            pick = []
            note = "no such launch: %d (1..%d)" % (args.launch, K)
    else:
        pick = [K - 1]
    shown = []
    for i in pick:
        s, e = spans[i]
        L = analyze_launch(region[s:e], first_no + s, total, args.top, 10)
        L["n"] = i + 1
        shown.append(L)
    res["shown"] = shown
    nl = sum(L["lines"] for L in shown)
    nalert = sum(len(L["alerts"]) for L in shown)
    if nl == 0:
        word = "EMPTY"
    elif nalert:
        word = "ALERTS(%d)" % nalert
    else:
        word = "CLEAN"
    res["verdict"] = "VERDICT: %s  launches=%d lines=%d" % (word, K, nl)
    res["verdict_word"] = word
    if note:
        res["note"] = note
    return res


def json_obj(res, args):
    shown = []
    for L in res["shown"]:
        d = dict(L)
        d["top"] = L["top"][:args.top]
        d["alerts"] = L["alerts"][:10]
        d["alerts_total"] = len(L["alerts"])
        shown.append(d)
    d = dict(file=res["name"], path=res["path"], size=res["size"], total_lines=res["total_lines"],
             launches=res["launches"], shown=shown, verdict=res["verdict"], verdict_word=res["verdict_word"])
    if res.get("new"):
        d["new"] = res["new"]
    if res.get("note"):
        d["note"] = res["note"]
    return d


def main(argv=None, out=sys.stdout):
    ap = argparse.ArgumentParser(prog="log_triage.py", description="Deterministic KOTOR II mod-log summarizer.")
    ap.add_argument("files", nargs="+", metavar="FILE")
    ap.add_argument("--all-launches", action="store_true")
    ap.add_argument("--launch", type=int, default=0)
    ap.add_argument("--top", type=int, default=12)
    ap.add_argument("--max-bytes", type=int, default=1800)
    ap.add_argument("--new", action="store_true")
    ap.add_argument("--no-advance", action="store_true")
    ap.add_argument("--state", default=DEFAULT_STATE)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args(argv)
    state_path = os.path.expanduser(args.state)
    state = load_state(state_path) if args.new else {}
    st_new = {}
    blocks = []
    rc = 0
    for f in args.files:
        if not os.path.isfile(f):
            blocks.append("missing: %s" % f)
            rc = 2
            continue
        res = triage_file(f, args, state, st_new)
        blocks.append(json.dumps(json_obj(res, args), sort_keys=True) if args.json else render_text(res, args))
    if args.new and not args.no_advance and st_new:
        state.update(st_new)
        save_state(state_path, state)
    if args.json:
        out.write("\n".join(blocks) + "\n")
    else:
        out.write("\n\n".join(blocks) + "\n")
    return rc


if __name__ == "__main__":
    sys.exit(main())
