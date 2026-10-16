#!/usr/bin/env python3
"""Holster probe log checker (holster route). Python 3, stdlib only; read-only (--new writes only its bookmark).

Usage: holster_log_check.py [LOG | --log LOG] [--new [--no-advance] [--state PATH]]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/holster_log.txt
  Report (at most 40 lines): header, status (loaded / cap reached / BONETEST), COMB edges per creature (plus unreadable=N for `old=? edge=?` lines), ATTACH tags and
  (body, handleR, handleL) tuples per creature, SWAP total, ANIMRES and ANIMSEND id histograms, GOB samples per gob
  (the 8 with the most samples: parent18, pos min/max/mean per axis, quat min/max per component, mean-quat angle),
  PLACE (only when PLACE lines exist: count, gobvt histogram, quat1 unit-norm check, last pos0/quat0 -> pos1/quat1 for the
  4 gobs with the most samples; the GOB view then drops to 4 gobs so the report stays within 40 lines).
  --new: only lines after the bookmark shared with log_triage (same state file, keyed by absolute path), then advance it.
  Exit 0 = analyzed, 2 = log missing ("no log: <path>").

Grammar: patch_manager_mods/holster_src/Holster.cpp. Each keyword is found with a regex search, so a KpLog stamp in front
is fine. A keyword line whose fields do not parse is counted in skipped (one example line is shown).
GOB quat is logged from Gob+0xB0 in engine order (w, x, y, z; 01_inventory_3d_viewport.md). The mean angle sign-aligns each
sample to the first one, averages, normalizes, and reports 2*acos(|w|). Min/max use the logged values as-is; sign_flips=k
counts samples whose sign disagrees with the first one (a flipped sample is the same rotation).
"""
from __future__ import annotations

import argparse
import math
import os
import re
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import log_triage as LT  # noqa: E402  (--new bookmark: same state file and helpers; log_triage is not modified)

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/holster_log.txt"
CAPS = {"comb": 4, "attach": 3, "tuples": 3, "tags": 5, "ids": 10, "gobs": 8,
        "gobs_with_place": 4, "place_gobs": 4}   # GOB view and PLACE view both shrink so the report stays <= 40 lines
UNIT_TOL = 2e-3   # PLACE quat1 |norm-1| limit: the DLL enforces 1e-3 (poseSane); %.3f printing adds up to ~1e-3 rounding
HEX = r"[0-9a-fA-F]+"
INT = r"-?\d+"
NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)"


def _rx(template):
    """Compile a pattern whose <H> <I> <N> tokens stand for hex, signed int and decimal float."""
    return re.compile(template.replace("<H>", HEX).replace("<I>", INT).replace("<N>", NUM))


KIND_RE = re.compile(r"\b(COMB|ATTACH|SWAP|ANIMRES|ANIMSEND|GOB|PLACE) ")
# Holster.cpp prints old=? edge=? when the COMB old-state read failed: counted as unreadable, never skipped.
COMB_UNREAD = _rx(r"\bCOMB creature=<H> old=\? new=<I> edge=\? tick=\d+")
FULL_RE = {
    "COMB": _rx(r"\bCOMB creature=(?P<creature><H>) old=(?P<old><I>) new=(?P<new><I>) edge=(?P<edge><I>) tick=\d+"),
    "ATTACH": _rx(r"\bATTACH obj=<H> tag='(?P<tag>[^']*)' creature=(?P<creature><H>) body=(?P<body><H>) "
                  r"handleR=(?P<hr><H>) handleL=(?P<hl><H>) tick=\d+"),
    "SWAP": _rx(r"\bSWAP exec=<H>" + "".join(" d%d=<H>" % i for i in range(8)) + r" tick=\d+"),
    "ANIMRES": _rx(r"\bANIMRES self=<H> id=(?P<id><H>) style=\d+ gate=\d+ cc=\d+ d4=\d+ e8=\d+ tick=\d+"),
    "ANIMSEND": _rx(r"\bANIMSEND creature=<H> id=(?P<id><H>) tick=\d+"),
    "GOB": _rx(r"\bGOB n=(?P<n><I>) gob=(?P<gob><H>) parent18=(?P<parent><H>) "
               r"pos=\((?P<px><N>),(?P<py><N>),(?P<pz><N>)\) quat=\((?P<qw><N>),(?P<qx><N>),(?P<qy><N>),(?P<qz><N>)\)"),
    "PLACE": _rx(r"\bPLACE n=(?P<n><I>) gob=(?P<gob><H>) node=(?P<node><H>) beh=(?P<beh><H>) gobvt=(?P<gobvt><H>) "
                 r"pos0=\((?P<p0x><N>),(?P<p0y><N>),(?P<p0z><N>)\) quat0=\((?P<q0w><N>),(?P<q0x><N>),(?P<q0y><N>),(?P<q0z><N>)\) "
                 r"pos1=\((?P<p1x><N>),(?P<p1y><N>),(?P<p1z><N>)\) quat1=\((?P<q1w><N>),(?P<q1x><N>),(?P<q1y><N>),(?P<q1z><N>)\)"),
}
MAIN_RES = {
    "loaded": re.compile(r"\bholster (\S+) loaded\b"),
    "cap": re.compile(r"\bcap reached max_lines=(\d+)"),
    "bonetest": re.compile(r"\bBONETEST (patched|refused)\b"),
    "tid": re.compile(r"\bTID (gobupdate|place)=(\d+)"),
    "reload": re.compile(r"\bRELOAD (place=\d .*)$"),
}


def _hex(s):
    """Lowercase hex without leading zeros, so 0x001F and 1f are one key."""
    return "%x" % int(s, 16)


def _record(kind, n, m):
    g = m.groupdict()
    rec = {"kind": kind, "line": n}
    if kind == "COMB":
        rec.update(creature=_hex(g["creature"]), old=int(g["old"]), new=int(g["new"]), edge=int(g["edge"]))
    elif kind == "ATTACH":
        rec.update(tag=g["tag"], creature=_hex(g["creature"]), body=_hex(g["body"]), hr=_hex(g["hr"]), hl=_hex(g["hl"]))
    elif kind in ("ANIMRES", "ANIMSEND"):
        rec.update(id=_hex(g["id"]))
    elif kind == "GOB":
        rec.update(gob=_hex(g["gob"]), parent=_hex(g["parent"]),
                   pos=[float(g[k]) for k in ("px", "py", "pz")], quat=[float(g[k]) for k in ("qw", "qx", "qy", "qz")])
    elif kind == "PLACE":
        rec.update(seq=int(g["n"]), gob=_hex(g["gob"]), node=_hex(g["node"]), beh=_hex(g["beh"]), gobvt=_hex(g["gobvt"]),
                   pos0=[float(g[k]) for k in ("p0x", "p0y", "p0z")],
                   quat0=[float(g[k]) for k in ("q0w", "q0x", "q0y", "q0z")],
                   pos1=[float(g[k]) for k in ("p1x", "p1y", "p1z")],
                   quat1=[float(g[k]) for k in ("q1w", "q1x", "q1y", "q1z")])
    return rec


def parse_lines(window):
    """window: [(line_no, raw)] -> (records in log order, skipped [(line_no, kind, text)], main facts)."""
    recs, skipped = [], []
    main = {"loaded": [], "cap": [], "bonetest": Counter(), "unreadable": 0, "tid": {}, "reload": []}
    for n, raw in window:
        text = raw.strip()
        m = MAIN_RES["loaded"].search(text)
        if m:
            main["loaded"].append(m.group(1))
        m = MAIN_RES["cap"].search(text)
        if m:
            main["cap"].append(m.group(1))
        m = MAIN_RES["bonetest"].search(text)
        if m:
            main["bonetest"][m.group(1)] += 1
        m = MAIN_RES["tid"].search(text)
        if m:
            main["tid"].setdefault(m.group(1), m.group(2))
        m = MAIN_RES["reload"].search(text)
        if m:
            main["reload"].append(m.group(1))
        k = KIND_RE.search(text)
        if k is None:
            continue
        kind = k.group(1)
        if kind == "COMB" and COMB_UNREAD.search(text):
            main["unreadable"] += 1
            continue
        f = FULL_RE[kind].search(text)
        if f is None:
            skipped.append((n, kind, text))
        else:
            recs.append(_record(kind, n, f))
    return recs, skipped, main


def _status(f):
    if f["loaded"]:
        loaded = "yes %s" % f["loaded"][-1] + (" x%d" % len(f["loaded"]) if len(f["loaded"]) > 1 else "")
    else:
        loaded = "no"
    cap = ("yes max_lines=%s" % f["cap"][-1]) if f["cap"] else "no"
    bone = ", ".join("%s x%d" % kv for kv in f["bonetest"].most_common()) or "absent"
    tid = f.get("tid", {})
    tids = "gobupdate=%s place=%s" % (tid.get("gobupdate", "absent"), tid.get("place", "absent"))
    if "gobupdate" in tid and "place" in tid:
        tids += " (same thread)" if tid["gobupdate"] == tid["place"] else " (DIFFERENT threads)"
    rl = f.get("reload", [])
    reload = ("%d last: %s" % (len(rl), rl[-1][:70])) if rl else "0"
    return "status: loaded=%s | cap reached=%s | BONETEST=%s | TID %s | RELOAD %s" % (loaded, cap, bone, tids, reload)


def _comb(recs):
    per = {}
    for r in recs:
        if r["kind"] == "COMB":
            per.setdefault(r["creature"], []).append(r)
    total = sum(len(v) for v in per.values())
    rows = sorted(per.items(), key=lambda kv: -len(kv[1]))   # stable: ties keep first-seen order
    shown = rows[:CAPS["comb"]]
    head = "COMB edges=%d creatures=%d" % (total, len(rows))
    if len(rows) > len(shown):
        head += " (top %d shown)" % len(shown)
    out = [head]
    for cid, evs in shown:
        out.append("  creature=0x%s edges=%d last new=%d" % (cid, len(evs), evs[-1]["new"]))
    return out


def _attach(recs):
    rows = [r for r in recs if r["kind"] == "ATTACH"]
    tags = Counter(r["tag"] for r in rows)
    per = {}
    for r in rows:
        per.setdefault(r["creature"], Counter())[(r["body"], r["hr"], r["hl"])] += 1
    by_creature = sorted(per.items(), key=lambda kv: -sum(kv[1].values()))
    tag_txt = " ".join("'%s'=%d" % kv for kv in tags.most_common(CAPS["tags"])) or "none"
    shown = by_creature[:CAPS["attach"]]
    head = "ATTACH n=%d creatures=%d tags: %s" % (len(rows), len(per), tag_txt)
    if len(per) > len(shown):
        head += " (first %d creatures shown)" % len(shown)
    out = [head]
    for cid, tuples in shown:
        best = sorted(tuples.items(), key=lambda kv: -kv[1])
        items = ["%s/%s/%s x%d" % (b, hr, hl, c) for (b, hr, hl), c in best[:CAPS["tuples"]]]
        more = len(best) - CAPS["tuples"]
        suffix = " ; +%d more" % more if more > 0 else ""
        out.append("  creature=0x%s distinct=%d (body/handleR/handleL): %s%s" % (cid, len(best), "; ".join(items), suffix))
    return out


def _hist(kind, recs):
    ids = Counter(r["id"] for r in recs if r["kind"] == kind)
    top = sorted(ids.items(), key=lambda kv: (-kv[1], kv[0]))[:CAPS["ids"]]
    more = len(ids) - len(top)
    txt = ", ".join("0x%s x%d" % kv for kv in top) or "none"
    suffix = " (+%d more ids)" % more if more > 0 else ""
    return "%s n=%d distinct=%d top%d: %s%s" % (kind, sum(ids.values()), len(ids), CAPS["ids"], txt, suffix)


def _quat_angle_deg(quats):
    """Degrees of the rotation of the mean quaternion (w, x, y, z), each sample sign-aligned to the first; None if degenerate."""
    ref = quats[0]
    acc = [0.0, 0.0, 0.0, 0.0]
    for q in quats:
        s = -1.0 if sum(a * b for a, b in zip(q, ref)) < 0 else 1.0
        for i in range(4):
            acc[i] += s * q[i]
    norm = math.sqrt(sum(c * c for c in acc))
    if norm < 1e-9:
        return None
    return math.degrees(2.0 * math.acos(min(1.0, abs(acc[0]) / norm)))


def _span(vals):
    return "%.3f..%.3f" % (min(vals), max(vals))


def _gobs(recs, cap):
    rows = [r for r in recs if r["kind"] == "GOB"]
    per = {}
    for r in rows:
        per.setdefault(r["gob"], []).append(r)
    ordered = sorted(per.items(), key=lambda kv: -len(kv[1]))   # stable: ties keep first-seen order
    shown = ordered[:cap]
    head = "GOB lines=%d gobs=%d" % (len(rows), len(per))
    if len(per) > len(shown):
        head += " (top %d by samples shown)" % len(shown)
    out = [head]
    for gid, samples in shown:
        quats = [s["quat"] for s in samples]
        flips = sum(1 for q in quats if sum(a * b for a, b in zip(q, quats[0])) < 0)
        parents = Counter(s["parent"] for s in samples)
        parent = parents.most_common(1)[0][0]
        extra = " (+%d other)" % (len(parents) - 1) if len(parents) > 1 else ""
        angle = _quat_angle_deg(quats)
        ang = "n/a" if angle is None else "%.2f" % angle
        out.append("  gob=0x%s samples=%d parent18=0x%s%s sign_flips=%d rot_mean=%s deg"
                   % (gid, len(samples), parent, extra, flips, ang))
        axes = []
        for i, name in enumerate("xyz"):
            vals = [s["pos"][i] for s in samples]
            axes.append("%s %s mean %.3f" % (name, _span(vals), sum(vals) / len(vals)))
        out.append("    pos " + " | ".join(axes))
        out.append("    quat " + " | ".join("%s %s" % (name, _span([q[i] for q in quats]))
                                            for i, name in enumerate("wxyz")))
    return out


def _norm(q):
    return math.sqrt(sum(c * c for c in q))


def _vec(vals):
    return ",".join("%.3f" % v for v in vals)


def _place(recs):
    """PLACE section: parsed count, gobvt histogram, quat1 unit-norm problem line, last pos0/quat0 -> pos1/quat1 per gob."""
    rows = [r for r in recs if r["kind"] == "PLACE"]
    per = {}
    for r in rows:
        per.setdefault(r["gob"], []).append(r)
    shown = sorted(per.items(), key=lambda kv: -len(kv[1]))[:CAPS["place_gobs"]]   # stable: ties keep first-seen order
    gobvt = Counter(r["gobvt"] for r in rows)
    top = gobvt.most_common(CAPS["ids"])
    head = "PLACE lines=%d gobs=%d" % (len(rows), len(per))
    if len(per) > len(shown):
        head += " (top %d by samples shown)" % len(shown)
    head += " gobvt distinct=%d: %s" % (len(gobvt), ", ".join("0x%s x%d" % kv for kv in top) or "none")
    if len(gobvt) > len(top):
        head += " (+%d more)" % (len(gobvt) - len(top))
    out = [head]
    bad = [r for r in rows if abs(_norm(r["quat1"]) - 1.0) > UNIT_TOL]
    if bad:
        worst = max(abs(_norm(r["quat1"]) - 1.0) for r in bad)
        out.append("PROBLEM PLACE quat1 |norm-1|>%g: count=%d worst=%.4f e.g. L%d gob=0x%s"
                   % (UNIT_TOL, len(bad), worst, bad[0]["line"], bad[0]["gob"]))
    elif rows:
        out.append("PLACE quat1 unit-norm ok: %d lines within %g" % (len(rows), UNIT_TOL))
    else:
        out.append("PLACE quat1 unit-norm: n/a (no parsed PLACE lines)")
    for gid, samples in shown:
        last = samples[-1]
        out.append("  gob=0x%s samples=%d last pos0=(%s) quat0=(%s)"
                   % (gid, len(samples), _vec(last["pos0"]), _vec(last["quat0"])))
        out.append("    -> pos1=(%s) quat1=(%s)" % (_vec(last["pos1"]), _vec(last["quat1"])))
    return out


def render(recs, skipped, main_facts, nlines, meta):
    has_place = any(r["kind"] == "PLACE" for r in recs) or any(k == "PLACE" for _, k, _ in skipped)
    out = ["HOLSTER lines=%d parsed=%d skipped=%d%s" % (nlines, len(recs), len(skipped), meta), _status(main_facts)]
    comb = _comb(recs)
    un = main_facts.get("unreadable", 0)
    if un:
        comb[0] += " unreadable=%d" % un
    out += comb
    out += _attach(recs)
    out.append("SWAP total=%d" % sum(1 for r in recs if r["kind"] == "SWAP"))
    out.append(_hist("ANIMRES", recs))
    out.append(_hist("ANIMSEND", recs))
    out += _gobs(recs, CAPS["gobs_with_place"] if has_place else CAPS["gobs"])
    if has_place:
        out += _place(recs)
    if skipped:
        n, kind, text = skipped[0]
        out.append("skipped e.g. L%d %s: %s" % (n, kind, text[:120]))
    return out


def read_window(path, a):
    """([(line_no, raw)], header note). --new starts after the log_triage bookmark (line 1 when rotated) and advances it
    unless --no-advance. Line numbers stay absolute."""
    all_lines = LT.read_lines(path)
    total = len(all_lines)
    start, note = 0, ""
    if a.new:
        st = path.stat()
        key = str(path.resolve())
        state_path = os.path.expanduser(a.state)
        state = LT.load_state(state_path)
        bm = state.get(key)
        rotated = bool(bm) and (bm.get("inode") != st.st_ino or st.st_size < bm.get("size", 0) or bm.get("line", 0) > total)
        if bm and not rotated:
            start = bm.get("line", 0)
        note = (" new=L%d-L%d" % (start + 1, total)) if start < total else " new=none"
        if rotated:
            note += " rotated"
        if not a.no_advance:
            state[key] = dict(inode=st.st_ino, size=st.st_size, line=total)
            LT.save_state(state_path, state)
    return [(i + 1, s) for i, s in enumerate(all_lines) if i >= start], note


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("log_pos", nargs="?")
    ap.add_argument("--log")
    ap.add_argument("--new", action="store_true", help="only lines after the log_triage bookmark, then advance it")
    ap.add_argument("--no-advance", action="store_true", help="with --new: do not move the bookmark")
    ap.add_argument("--state", default=LT.DEFAULT_STATE, help="bookmark file for --new")
    a = ap.parse_args(argv)
    path = Path(a.log or a.log_pos or DEFAULT_LOG)
    if not path.is_file():
        print("no log: %s" % path)
        return 2
    window, meta = read_window(path, a)
    recs, skipped, main_facts = parse_lines(window)
    print("\n".join(render(recs, skipped, main_facts, len(window), meta)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
