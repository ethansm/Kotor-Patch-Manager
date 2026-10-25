#!/usr/bin/env python3
"""Command-queue probe log checker (cmdq-probe route). Python 3, stdlib only; read-only.

Usage: cmdq_probe_log_check.py [LOG | --log LOG] [--json] [--all]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/cmdq-probe_log.txt
  Default view (at most 58 lines by construction): header, CHANNELS (parsed lines per keyword), FLAGS (per-flag counts),
  up to 3 MALFORMED and 3 UNKNOWN samples, then up to 7 hits per flag.
  --json: one JSON object with the same counts; hits and samples capped like the text view.
  --all: no hit or sample cap.
  Exit 0 = analyzed (flags are reported, not failures), 2 = log missing.

Line shape: "<ms> <patch>/<chan> [ERR|WARN|INFO] <payload>", or the KpLog stamp "T=<ms> f=<frame|-> <patch>/<chan> <payload>",
or a bare payload (untimed). Blank lines are ignored. The first word of the payload selects the grammar:
  PAUSE mask=<n> val=<n>
  CMD kind=<spell|item> actor=<hex> id=<hex> tgt=<hex> real=<0|1>
  FIRE kind=<spell|item> actor=<hex>
  ROUND ev=<start|end> round=<hex> owner=<hex>
  CLR victim=<hex> caller=<hex> q=<hex,..>
  REEQ actor=<hex>
  MED site=<entry|canuse> this=<hex> arg=<hex> base=<hex> flag=<0|1>
  QS at=<cmd0|cmd1|fire|rend> actor=<hex> n=<n> q=<hex:hex,..> rn=<hex:hex,..> cur=<hex:hex|-> fx=<hex,..>   (0.2.0)
Hex values take an optional 0x prefix; <n> is decimal. Every listed key of a keyword is required. A keyword line with a
missing, duplicate or invalid field is MALFORMED; a first word that is no keyword is UNKNOWN. Both are counted, never fatal.

Flags (timing uses the <ms> stamp, so untimed lines never trigger M1, M2 or C1):
  M1 medpack-refused: MED site=canuse flag=0 after a MED site=entry within 2000 ms (entry base equal, or 0 for a handle arg).
  M2 item-never-fired: MED site=entry with no later FIRE kind=item within 3000 ms.
  C1 combat-start-wipe: a CLR within 100 ms before a ROUND ev=start (the CLR caller is shown).
  S1 stale-build: CMD kind=spell real=1 whose id differs from the latest real=0 CMD of the same actor within 500 ms
     before it (the engine built another cast than the one just queued; .claude/research/combat_ai_recast/README.md).
  Q1 deep-queue: a QS line with n >= 6 queue entries (attack flood, WI-139).
Timed flags assume the log's stamps rise monotonically, which holds for one game process.
"""
from __future__ import annotations

import argparse
import bisect
import json
import re
import sys
from collections import Counter
from pathlib import Path

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/cmdq-probe_log.txt"
HIT_CAP = 7               # hits shown per flag (--all lifts it); 3 + 2 x 5 + 5 x (7 + 2) = 58 lines worst case
BAD_CAP = 3               # MALFORMED / UNKNOWN samples shown (--all lifts it)
M1_WINDOW_MS = 2000
M2_WINDOW_MS = 3000
C1_WINDOW_MS = 100
S1_WINDOW_MS = 500
Q1_DEPTH = 6
CHANNELS = ("PAUSE", "CMD", "FIRE", "ROUND", "CLR", "REEQ", "MED", "QS")
FLAG_ORDER = ("M1", "M2", "C1", "S1", "Q1")
FLAG_TITLE = {"M1": "M1 medpack-refused", "M2": "M2 item-never-fired", "C1": "C1 combat-start-wipe",
              "S1": "S1 stale-build", "Q1": "Q1 deep-queue"}
_HEX = r"(?:0[xX])?[0-9a-fA-F]+"
# Value grammar per field kind, matched with fullmatch.
VALUE_RE = {k: re.compile(v) for k, v in {
    "hex": _HEX,
    "hexlist": r"(?:" + _HEX + r"(?:," + _HEX + r")*)?",
    "num": r"-?\d+",
    "bit": r"[01]",
    "kind": r"spell|item",
    "ev": r"start|end",
    "site": r"entry|canuse",
    "at": r"cmd0|cmd1|fire|rend",
    "pairlist": r"(?:" + _HEX + ":" + _HEX + r"(?:," + _HEX + ":" + _HEX + r")*)?",
    "pairopt": r"-|" + _HEX + ":" + _HEX,
}.items()}
# Field table: keyword -> {key: kind}. Every listed key is REQUIRED on that keyword; unlisted keys are ignored.
FIELDS = {
    "PAUSE": {"mask": "num", "val": "num"},
    "CMD": {"kind": "kind", "actor": "hex", "id": "hex", "tgt": "hex", "real": "bit"},
    "FIRE": {"kind": "kind", "actor": "hex"},
    "ROUND": {"ev": "ev", "round": "hex", "owner": "hex"},
    "CLR": {"victim": "hex", "caller": "hex", "q": "hexlist"},
    "REEQ": {"actor": "hex"},
    "MED": {"site": "site", "this": "hex", "arg": "hex", "base": "hex", "flag": "bit"},
    "QS": {"at": "at", "actor": "hex", "n": "num", "q": "pairlist", "rn": "pairlist", "cur": "pairopt", "fx": "hexlist"},
}
REQUIRED = {kw: tuple(spec) for kw, spec in FIELDS.items()}
PREFIX_RE = re.compile(
    r"^(?:T=)?(?P<t>\d+(?:\.\d+)?)\s+(?:f=\S+\s+)?[\w.-]+/\w+\s+(?:(?:ERR|WARN|INFO)\s+)?(?P<body>.*)$")


class Bad(Exception):
    def __init__(self, label, reason):
        super().__init__(reason)
        self.label, self.reason = label, reason


def _convert(kind, val):
    if kind == "hex":
        return int(val, 16)
    if kind == "hexlist":
        return [int(x, 16) for x in val.split(",")] if val else []
    if kind == "pairlist":
        return [tuple(int(h, 16) for h in x.split(":")) for x in val.split(",")] if val else []
    if kind == "pairopt":
        return None if val == "-" else tuple(int(h, 16) for h in val.split(":"))
    if kind == "num":
        return int(val, 10)
    if kind == "bit":
        return int(val)
    return val  # enum kinds stay strings


def parse_body(body):
    """-> (keyword, {key: value}), or None when the first word is no keyword (UNKNOWN). Raises Bad (MALFORMED)."""
    words = body.split()
    if not words or words[0] not in FIELDS:
        return None
    kw = words[0]
    raw = {}
    for tok in words[1:]:
        key, sep, val = tok.partition("=")
        if not sep or not key:
            raise Bad(kw, "token %r has no key=value" % tok)
        if key in raw:
            raise Bad(kw, "duplicate %s" % key)
        raw[key] = val
    missing = [k for k in REQUIRED[kw] if k not in raw]
    if missing:
        raise Bad(kw, "missing " + ",".join(missing))
    out = {}
    for key, kind in FIELDS[kw].items():
        val = raw[key]
        if not VALUE_RE[kind].fullmatch(val):
            raise Bad(kw, "bad %s=%s" % (key, val))
        out[key] = _convert(kind, val)
    return kw, out


def parse_lines(lines):
    """lines: [(line_no, raw)] -> (records in log order, unknown [(line_no, word)], malformed [(line_no, kw, reason)]).
    A record is {"line", "t" (ms as float, None when untimed), "kw", "f" (typed fields)}. Blank lines are ignored."""
    recs, unknown, malformed = [], [], []
    for n, raw in lines:
        m = PREFIX_RE.match(raw)
        body = (m.group("body") if m else raw).strip()
        if not body:
            continue
        t = float(m.group("t")) if m else None
        try:
            got = parse_body(body)
        except Bad as e:
            malformed.append((n, e.label, e.reason))
            continue
        if got is None:
            unknown.append((n, body.split()[0][:24]))
            continue
        kw, fields = got
        recs.append({"line": n, "t": t, "kw": kw, "f": fields})
    return recs, unknown, malformed


def _ms_text(t):
    if t is None:
        return "-"
    return "%d" % t if float(t).is_integer() else "%.3f" % t


def _hex(v):
    return "0x%x" % v


def _hit(flag, rec, ref, detail):
    return {"flag": flag, "line": rec["line"], "t": rec["t"], "ref": ref, "detail": detail}


def m1_hits(recs):
    """Medpack refused: canuse flag=0 paired with the latest earlier entry (same base, or base 0 = handle arg, no
    readable item pointer) within M1_WINDOW_MS."""
    entries = []  # MED site=entry records, log order
    hits = []
    for r in recs:
        if r["kw"] != "MED":
            continue
        f = r["f"]
        if f["site"] == "entry":
            entries.append(r)
        elif f["flag"] == 0 and r["t"] is not None:
            for e in reversed(entries):
                if e["t"] is None or e["t"] > r["t"]:
                    continue
                if e["f"]["base"] not in (0, f["base"]):
                    continue
                if r["t"] - e["t"] <= M1_WINDOW_MS:
                    hits.append(_hit("M1", r, e["line"], "L%d t=%s canuse base=%s flag=0 refused; entry L%d t=%s (dt=%s ms)"
                                     % (r["line"], _ms_text(r["t"]), _hex(f["base"]), e["line"], _ms_text(e["t"]),
                                        _ms_text(r["t"] - e["t"]))))
                break  # the latest compatible entry decides
    return hits


def m2_hits(recs):
    """Pending item never completed: MED site=entry with no FIRE kind=item within M2_WINDOW_MS after it."""
    fires = [r for r in recs if r["kw"] == "FIRE" and r["f"]["kind"] == "item" and r["t"] is not None]
    fire_lines = [r["line"] for r in fires]
    hits = []
    for r in recs:
        if r["kw"] != "MED" or r["f"]["site"] != "entry" or r["t"] is None:
            continue
        done = False
        for j in range(bisect.bisect_right(fire_lines, r["line"]), len(fires)):
            dt = fires[j]["t"] - r["t"]
            if dt > M2_WINDOW_MS:
                break
            if dt >= 0:
                done = True
                break
        if not done:
            f = r["f"]
            hits.append(_hit("M2", r, None, "L%d t=%s entry this=%s arg=%s base=%s: no FIRE kind=item within %d ms"
                             % (r["line"], _ms_text(r["t"]), _hex(f["this"]), _hex(f["arg"]), _hex(f["base"]),
                                M2_WINDOW_MS)))
    return hits


def c1_hits(recs):
    """Combat-start wipe: a CLR within C1_WINDOW_MS before a ROUND ev=start."""
    clrs = [r for r in recs if r["kw"] == "CLR" and r["t"] is not None]
    clr_lines = [r["line"] for r in clrs]
    hits = []
    for r in recs:
        if r["kw"] != "ROUND" or r["f"]["ev"] != "start" or r["t"] is None:
            continue
        for j in range(bisect.bisect_left(clr_lines, r["line"]) - 1, -1, -1):
            c = clrs[j]
            dt = r["t"] - c["t"]
            if dt > C1_WINDOW_MS:
                break
            if dt >= 0:
                f = r["f"]
                hits.append(_hit("C1", r, c["line"], "L%d t=%s ROUND ev=start round=%s owner=%s: CLR L%d t=%s caller=%s "
                                 "victim=%s (dt=%s ms)" % (r["line"], _ms_text(r["t"]), _hex(f["round"]), _hex(f["owner"]),
                                                          c["line"], _ms_text(c["t"]), _hex(c["f"]["caller"]),
                                                          _hex(c["f"]["victim"]), _ms_text(dt))))
                break
    return hits


def s1_hits(recs):
    """Stale build: a spell CMD real=1 whose id is not the id the same actor queued (real=0) within S1_WINDOW_MS before."""
    last_q = {}  # actor -> latest real=0 spell CMD record
    hits = []
    for r in recs:
        if r["kw"] != "CMD" or r["f"]["kind"] != "spell":
            continue
        f = r["f"]
        if f["real"] == 0:
            last_q[f["actor"]] = r
            continue
        q = last_q.get(f["actor"])
        if q is None or r["t"] is None or q["t"] is None or not 0 <= r["t"] - q["t"] <= S1_WINDOW_MS:
            continue
        if q["f"]["id"] != f["id"]:
            hits.append(_hit("S1", r, q["line"], "L%d t=%s actor=%s built id=%s; queued id=%s at L%d (dt=%s ms)"
                             % (r["line"], _ms_text(r["t"]), _hex(f["actor"]), _hex(f["id"]), _hex(q["f"]["id"]),
                                q["line"], _ms_text(r["t"] - q["t"]))))
    return hits


def q1_hits(recs):
    """Deep queue: a QS snapshot with at least Q1_DEPTH queue entries (types listed)."""
    hits = []
    for r in recs:
        if r["kw"] == "QS" and r["f"]["n"] >= Q1_DEPTH:
            f = r["f"]
            hits.append(_hit("Q1", r, None, "L%d t=%s at=%s actor=%s n=%d types=%s"
                             % (r["line"], _ms_text(r["t"]), f["at"], _hex(f["actor"]), f["n"],
                                ",".join("%x" % t for t, _ in f["q"]))))
    return hits


def summarize(lines):
    """Raw text lines -> summary dict (counts and hits in log order). Pure; main only renders it."""
    recs, unknown, malformed = parse_lines(list(enumerate(lines, 1)))
    hits = {"M1": m1_hits(recs), "M2": m2_hits(recs), "C1": c1_hits(recs), "S1": s1_hits(recs), "Q1": q1_hits(recs)}
    counts = Counter(r["kw"] for r in recs)
    return {
        "lines": len(lines), "parsed": len(recs), "unknown_list": unknown, "malformed_list": malformed,
        "channels": {k: counts.get(k, 0) for k in CHANNELS},
        "hits": hits, "flags": {k: len(hits[k]) for k in FLAG_ORDER},
    }


def _take(items, cap):
    return items if cap is None else items[:cap]


def _section(title, items, cap, count):
    out = ["%s %d" % (title, count)]
    if count == 0:
        return out
    out += ["  " + s for s in _take(items, cap)]
    if cap is not None and count > cap:
        out.append("  ... +%d more" % (count - cap))
    return out


def render_text(s, hit_cap, bad_cap):
    out = ["CMDQ-PROBE lines=%d parsed=%d unknown=%d malformed=%d"
           % (s["lines"], s["parsed"], len(s["unknown_list"]), len(s["malformed_list"]))]
    out.append("CHANNELS " + " ".join("%s=%d" % (k, s["channels"][k]) for k in CHANNELS))
    out.append("FLAGS " + " ".join("%s=%d" % (k, s["flags"][k]) for k in FLAG_ORDER))
    out += _section("MALFORMED", ["L%d %s: %s" % m for m in s["malformed_list"]], bad_cap, len(s["malformed_list"]))
    out += _section("UNKNOWN", ["L%d %s" % u for u in s["unknown_list"]], bad_cap, len(s["unknown_list"]))
    for k in FLAG_ORDER:
        hits = s["hits"][k]
        out += _section(FLAG_TITLE[k], [h["detail"] for h in hits], hit_cap, len(hits))
    return out


def to_json(s, hit_cap, bad_cap):
    return {
        "lines": s["lines"], "parsed": s["parsed"],
        "unknown": len(s["unknown_list"]), "malformed": len(s["malformed_list"]),
        "channels": s["channels"], "flags": s["flags"],
        "hits": {k: _take(s["hits"][k], hit_cap) for k in FLAG_ORDER},
        "malformed_samples": [{"line": n, "kw": kw, "reason": why} for n, kw, why in _take(s["malformed_list"], bad_cap)],
        "unknown_samples": [{"line": n, "word": w} for n, w in _take(s["unknown_list"], bad_cap)],
    }


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("log_pos", nargs="?")
    ap.add_argument("--log")
    ap.add_argument("--json", action="store_true", help="one JSON object instead of the text view")
    ap.add_argument("--all", action="store_true", help="no hit or sample cap (text view may exceed 60 lines)")
    a = ap.parse_args(argv)
    path = Path(a.log or a.log_pos or DEFAULT_LOG)
    if not path.is_file():
        print("no log: %s" % path)
        return 2
    s = summarize(path.read_text(encoding="utf-8", errors="replace").splitlines())
    hit_cap, bad_cap = (None, None) if a.all else (HIT_CAP, BAD_CAP)
    if a.json:
        doc = to_json(s, hit_cap, bad_cap)
        doc["log"] = str(path)
        print(json.dumps(doc, indent=2))
    else:
        print("\n".join(render_text(s, hit_cap, bad_cap)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
