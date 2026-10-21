#!/usr/bin/env python3
"""Holster probe log checker (holster route). Python 3, stdlib only; read-only (--new writes only its bookmark).

Usage: holster_log_check.py [LOG | --log LOG] [--new [--no-advance] [--state PATH]]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/holster_log.txt
  Report (at most 47 lines): header, status (loaded / cap reached / BONETEST), COMB edges per creature (plus unreadable=N for `old=? edge=?` lines), ATTACH tags and
  (body, handleR, handleL) tuples per creature, SWAP total, GOBREUSE total, FLOURISH open/close, SLIDE start/end (draw/stow split, frames min/max, maxdt max), PARTYHOLD on/off, MOVEHOLD total, DELAY cuts (combat/flourish), ANIMRES and ANIMSEND id histograms, GOB samples per gob
  (the 8 with the most samples: parent18, pos min/max/mean per axis, quat min/max per component, mean-quat angle),
  PLACE (only when PLACE lines exist: count, gobvt histogram, quat1 unit-norm check, last pos0/quat0 -> pos1/quat1 for the
  4 gobs with the most samples; the GOB view then drops to 4 gobs so the report stays within 47 lines). REACH (HW19 probe): found, steps, persist1, wrote_both, refusals, err range.
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
        "gobs_with_place": 4, "place_gobs": 4}   # GOB view and PLACE view both shrink so the report stays <= 47 lines
UNIT_TOL = 2e-3   # PLACE quat1 |norm-1| limit: the DLL enforces 1e-3 (poseSane); %.3f printing adds up to ~1e-3 rounding
HEX = r"[0-9a-fA-F]+"
INT = r"-?\d+"
NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)"


def _rx(template):
    """Compile a pattern whose <H> <I> <N> tokens stand for hex, signed int and decimal float."""
    return re.compile(template.replace("<H>", HEX).replace("<I>", INT).replace("<N>", NUM))


KIND_RE = re.compile(r"\b(COMB|ATTACH|SWAP|ANIMRES|ANIMSEND|GOB|PLACE|GOBREUSE|FLOURISH|SLIDE) ")
# Holster.cpp prints old=? edge=? when the COMB old-state read failed: counted as unreadable, never skipped.
COMB_UNREAD = _rx(r"\bCOMB creature=<H> old=\? new=<I> edge=\? tick=\d+")
FULL_RE = {
    "COMB": _rx(r"\bCOMB creature=(?P<creature><H>) old=(?P<old><I>) new=(?P<new><I>) edge=(?P<edge><I>) tick=\d+"),
    "ATTACH": _rx(r"\bATTACH obj=<H> tag='(?P<tag>[^']*)' creature=(?P<creature><H>) body=(?P<body><H>) "
                  r"handleR=(?P<hr><H>) handleL=(?P<hl><H>) tick=\d+"),
    "SWAP": _rx(r"\bSWAP exec=<H>" + "".join(" d%d=<H>" % i for i in range(8)) + r" tick=\d+"),
    "ANIMRES": _rx(r"\bANIMRES self=(?P<self><H>) id=(?P<id><H>) style=(?P<style>\d+) gate=(?P<gate>\d+) cc=\d+ d4=\d+ e8=\d+(?: hol=(?P<hol>\d) comb=-?\d+)? tick=\d+(?: own=(?P<own><H>))?"),
    "ANIMSEND": _rx(r"\bANIMSEND creature=<H> id=(?P<id><H>) tick=\d+"),
    "GOB": _rx(r"\bGOB n=(?P<n><I>) gob=(?P<gob><H>) parent18=(?P<parent><H>) "
               r"pos=\((?P<px><N>),(?P<py><N>),(?P<pz><N>)\) quat=\((?P<qw><N>),(?P<qx><N>),(?P<qy><N>),(?P<qz><N>)\)"),
    "PLACE": _rx(r"\bPLACE n=(?P<n><I>) gob=(?P<gob><H>) node=(?P<node><H>) beh=(?P<beh><H>) gobvt=(?P<gobvt><H>) "
                 r"pos0=\((?P<p0x><N>),(?P<p0y><N>),(?P<p0z><N>)\) quat0=\((?P<q0w><N>),(?P<q0x><N>),(?P<q0y><N>),(?P<q0z><N>)\) "
                 r"pos1=\((?P<p1x><N>),(?P<p1y><N>),(?P<p1z><N>)\) quat1=\((?P<q1w><N>),(?P<q1x><N>),(?P<q1y><N>),(?P<q1z><N>)\)(?: cat=(?P<cat>\w+))?"),
    "GOBREUSE": _rx(r"\bGOBREUSE gob=(?P<gob><H>) node=(?P<node><H>) oldm=(?P<oldm>\S+) newm=(?P<newm>\S+)"),
    "FLOURISH": _rx(r"\bFLOURISH (?:(?P<open>open) slot=(?P<oslot><I>) cli=<I> srv=<I> p524=<I> p528=<I> b11E8=<I>"
                  r"|(?P<close>close) slot=(?P<cslot><I>) p520=<I> p524=<I>)"),
    "SLIDE": _rx(r"\bSLIDE (?:(?P<start>start) dir=(?P<sdir>\w+) owner=<H> gob=<H> left=<I> from=<N> ms=<I>"
                 r"|(?P<end>end) dir=(?P<edir>\w+) owner=<H> ms=<I> frames=(?P<frames><I>) maxdt=(?P<maxdt><I>))"),
}
MAIN_RES = {
    "loaded": re.compile(r"\bholster (\S+) loaded\b"),
    "cap": re.compile(r"\bcap reached max_lines=(\d+)"),
    "bonetest": re.compile(r"\bBONETEST (patched|refused)\b"),
    "tid": re.compile(r"\bTID (gobupdate|place)=(\d+)"),
    "reload": re.compile(r"\bRELOAD (place=\d .*)$"),
    "tune": re.compile(r"\bTUNE boot (place=\d .*)$"),
    "refuse": re.compile(r"\bPLACEREFUSE reason=(\w+) n=(\d+)"),
    "mount": re.compile(r"\bMOUNT (\S+) parent=\w+ bone=(\S+) node=(\w+)"),
    "owner": re.compile(r"\bPLACENODE .*\btarget=1 .*\bowner=(\w+) body=(\S+)"),
    "preview": re.compile(r"\bPREVIEWNODE gob=\w+ node=\w+ name=(\w+) parent=(\w+) owner=(\w+) body=(\S+) mname=(\S+) cat=(\w+) gobvt=(\w+) tid=(\d+)"),
    "camnode": re.compile(r"\bCAMNODE parent=(\w+) pmodel=(\S+) pvt=\w+ pparent=(\w+) ppmodel=(\S+) ppvt=\w+ camhook_p=(\w+) camhook_pp=(\w+)"),
    # toggle-channel events (Holster.cpp); counted from the raw line, never skipped: a malformed line simply does not count
    "partyhold": re.compile(r"\bPARTYHOLD (on|off) n=\d+"),
    "movehold": re.compile(r"\bMOVEHOLD slot=-?\d+ skip cut \(moving\)"),
    "animstyle": re.compile(r"\bANIMSTYLE owner=(\w+) id=(\w+) style=(\d+)->(\d+)"),   # HW16 (0.13.0-dev): the unarmed override fired
    "delay": re.compile(r"\bDELAY slot=-?\d+ cut before=\d+ elapsed=\d+ kind=(combat|flourish) wrote=1\b"),
    "reach": re.compile(r"\bREACH owner=\w+ w=([\d.]+) reach=([\d.]+) .*?err=(-?[\d.]+) persist=(\d) wrote=(\d)"
                       r"(?: wr=(-?[\d.]+) hg=(\d) f=(-?[\d.]+))?(?: dzA=(-?[\d.]+) fy=(-?[\d.]+) fx=(-?[\d.]+) g=(-?[\d.]+) cap=(-?[\d.]+))?(?: hold=(\d+))?"),
    "reachtw": re.compile(r"\bREACH owner=\w+ w=[\d.]+ .*? bk=\d tw=(-?[\d.]+) sw=(-?[\d.]+)"),   # HW29 (0.17.0-dev): forearm twist and wrist swing written this frame, degrees
    "reachhold": re.compile(r"\bREACH owner=\w+ hold=(\d+) why=(\w+)"),   # HW25 (0.14.0-dev): a frame where a transient refusal re-applied the last arm
    "reachrefuse": re.compile(r"\bREACH owner=\w+ refuse=(\w+)"),
    "reachtopo": re.compile(r"\bREACH owner=\w+ w=[\d.]+ .*? topo=(\w+) bk=(\d)"),   # HW27 (0.15.0-dev): which reach the mount height chose (hinge / over / hip / thigh), bk=1 = a back bone
    "slidecfg": re.compile(r"\bCONFIG .*?\bslide=(\S+) resid_ms=(\d+) wrist_max=(\d+)"),   # HW28 (0.15.0-dev): resolved <cat>:<draw ms>/<stow ms>/<grab %>, residual ms, wrist cap
    "node": re.compile(r"\bPLACENODE node=\w+ gob=\w+ t=(\d+) step=(-?\d+) name=(\S+) target=(\d)(?: bvt=(\w+))?(?: gname=(\S+) mname=(\S+))?(?: cat=(\w+))?"),
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
        if kind == "ANIMRES":
            rec.update(self=_hex(g["self"]), style=int(g["style"]), gate=int(g["gate"]), hol=int(g["hol"]) if g["hol"] else -1,
                       own=_hex(g["own"]) if g["own"] else "")
    elif kind == "GOB":
        rec.update(gob=_hex(g["gob"]), parent=_hex(g["parent"]),
                   pos=[float(g[k]) for k in ("px", "py", "pz")], quat=[float(g[k]) for k in ("qw", "qx", "qy", "qz")])
    elif kind == "PLACE":
        rec.update(seq=int(g["n"]), gob=_hex(g["gob"]), node=_hex(g["node"]), beh=_hex(g["beh"]), gobvt=_hex(g["gobvt"]),
                   pos0=[float(g[k]) for k in ("p0x", "p0y", "p0z")],
                   quat0=[float(g[k]) for k in ("q0w", "q0x", "q0y", "q0z")],
                   pos1=[float(g[k]) for k in ("p1x", "p1y", "p1z")],
                   quat1=[float(g[k]) for k in ("q1w", "q1x", "q1y", "q1z")], cat=g["cat"] or "")
    elif kind == "GOBREUSE":
        rec.update(gob=_hex(g["gob"]), node=_hex(g["node"]), oldm=g["oldm"], newm=g["newm"])
    elif kind == "FLOURISH":
        rec.update(ev="open" if g["open"] else "close")
    elif kind == "SLIDE":
        if g["start"]:
            rec.update(ev="start", dir=g["sdir"])
        else:
            rec.update(ev="end", dir=g["edir"], frames=int(g["frames"]), maxdt=int(g["maxdt"]))
    return rec


def parse_lines(window):
    """window: [(line_no, raw)] -> (records in log order, skipped [(line_no, kind, text)], main facts)."""
    recs, skipped = [], []
    main = {"loaded": [], "cap": [], "bonetest": Counter(), "unreadable": 0, "tid": {}, "reload": [], "tune": [], "refuse": {}, "node": Counter(), "mount": Counter(),
            "owner": Counter(), "combc": set(), "partyhold": Counter(), "movehold": 0, "delay": Counter(),
            "reach": [], "slidecut": Counter(), "resid": [], "reachfound": 0, "reachrefuse": Counter(), "reachhold": Counter(), "reachtw": [],
            "reachtopo": Counter(), "slidecfg": [],
            "preview": [], "camnode": [], "wmname": set()}
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
        m = MAIN_RES["tune"].search(text)
        if m:
            main["tune"].append(m.group(1))
        m = MAIN_RES["refuse"].search(text)
        if m:
            main["refuse"][m.group(1)] = max(main["refuse"].get(m.group(1), 0), int(m.group(2)))
        m = MAIN_RES["node"].search(text)
        if m:
            main["node"]["t=%s step=%s name=%s target=%s" % m.groups()[:4] + (" bvt=%s" % m.group(5) if m.group(5) else "")
                   + (" gname=%s mname=%s" % m.group(6, 7) if m.group(6) else "")
                   + (" cat=%s" % m.group(8) if m.group(8) else "")] += 1
        if m and m.group(4) == "1" and m.group(7):
            main["wmname"].add(m.group(7).lower())
        m = MAIN_RES["partyhold"].search(text)
        if m:
            main["partyhold"][m.group(1)] += 1
        if MAIN_RES["movehold"].search(text):
            main["movehold"] += 1
        m = MAIN_RES["delay"].search(text)
        if m:
            main["delay"][m.group(1)] += 1
        m = MAIN_RES["animstyle"].search(text)
        if m:
            main.setdefault("animstyle", []).append((_hex(m.group(1)), _hex(m.group(2)), int(m.group(3)), int(m.group(4))))
        if text.find(" SLIDE cut ") >= 0 or text.startswith("SLIDE cut "):   # HW24: a hard reset that hit a running slide
            mc = re.search(r"\bSLIDE cut reason=(\w+)", text)
            if mc:
                main["slidecut"][mc.group(1)] += 1
        if " RESID " in text or text.startswith("RESID "):   # HW24: carried-hand vs mount residual at the carryW step
            mr = re.search(r"\bRESID .*?\bpos_cm=(-?[\d.]+) ang=(-?[\d.]+) dq=(-?[\d.]+)", text)
            if mr:
                main["resid"].append(tuple(float(v) for v in mr.groups()))
        m = MAIN_RES["slidecfg"].search(text)
        if m:
            main["slidecfg"].append((m.group(1), int(m.group(2)), int(m.group(3))))
        m = MAIN_RES["reachtopo"].search(text)
        if m:
            main["reachtopo"][(m.group(1), int(m.group(2)))] += 1
        m = MAIN_RES["reachtw"].search(text)
        if m:
            main["reachtw"].append((float(m.group(1)), float(m.group(2))))
        m = MAIN_RES["reach"].search(text)
        if m:
            main["reach"].append((float(m.group(1)), float(m.group(2)), float(m.group(3)), int(m.group(4)), int(m.group(5)))
                                 + tuple(None if v is None else float(v) for v in m.groups()[5:]))
        elif " REACH found " in text or text.startswith("REACH found "):
            main["reachfound"] += 1
        else:
            mh = MAIN_RES["reachhold"].search(text)
            if mh:
                main["reachhold"][mh.group(2)] += 1
            m = MAIN_RES["reachrefuse"].search(text)
            if m:
                main["reachrefuse"][m.group(1)] += 1
        m = MAIN_RES["preview"].search(text)
        if m:
            main["preview"].append(m.groups())
        m = MAIN_RES["camnode"].search(text)
        if m:
            main["camnode"].append(m.groups())
        m = MAIN_RES["mount"].search(text)
        if m:
            main["mount"][(m.group(1), m.group(2))] += 1
        m = MAIN_RES["owner"].search(text)
        if m:
            main["owner"][(_hex(m.group(1)), m.group(2))] += 1
        k = KIND_RE.search(text)
        if k is None:
            continue
        kind = k.group(1)
        if kind == "SLIDE" and k.end() < len(text) and text.startswith("cut ", k.end()):   # HW24 SLIDE cut: counted above, not a start/end record
            continue
        if kind == "COMB" and COMB_UNREAD.search(text):
            main["unreadable"] += 1
            continue
        f = FULL_RE[kind].search(text)
        if f is None:
            skipped.append((n, kind, text))
        else:
            recs.append(_record(kind, n, f))
            if kind == "COMB":
                main["combc"].add(recs[-1]["creature"])
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


def _animstyle(f, recs):
    """HW16: ANIMSTYLE lines (the unarmed override fired; the DLL logs only the first 40, so n is a floor) and the hol=1 ANIMRES lines of locomotion ids
    (a holstered creature still resolving an armed row shows up as style 2/3/4/7/9 there). Appended to the ANIMRES line: no extra report line."""
    ev = f.get("animstyle", [])
    hol = sum(1 for r in recs if r["kind"] == "ANIMRES" and r.get("hol") == 1)
    if not ev and not hol:
        return ""
    txt = " | ANIMSTYLE n=%d owners=%d" % (len(ev), len({e[0] for e in ev}))
    if ev:
        txt += " styles " + ",".join("%d->%d" % kv for kv in sorted({(e[2], e[3]) for e in ev}))
    return txt + " hol=1 ANIMRES=%d" % hol


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
        out.append("  gob=0x%s%s samples=%d last pos0=(%s) quat0=(%s)"
                   % (gid, " cat=%s" % last["cat"] if last.get("cat") else "", len(samples), _vec(last["pos0"]), _vec(last["quat0"])))
        out.append("    -> pos1=(%s) quat1=(%s)" % (_vec(last["pos1"]), _vec(last["quat1"])))
    return out


def _preview_diag(f):
    """HE0 (doc 42 HW-EDIT): PREVIEWNODE = weapon Gobs still on the engine's rhand/lhand node (equip-screen preview, or in-world without
    bone_test); CAMNODE = one line per distinct parent with FindPart("camerahook") results (0 = not found, 1 = FindPart slot mismatch)."""
    out = []
    pv = f.get("preview") or []
    if pv:
        by_parent = {}
        for name, parent, owner, body, mname, cat, vt, tid in pv:
            e = by_parent.setdefault(_hex(parent), {"body": body, "owner": _hex(owner), "m": [], "tid": set()})
            e["m"].append("%s:%s/%s" % (name, mname, cat))
            e["tid"].add(tid)
        out.append("PREVIEWNODE n=%d parents=%d tids=%s" % (len(pv), len(by_parent), ",".join(sorted({p[7] for p in pv}))))
        for parent, e in list(by_parent.items())[:6]:
            out.append("  parent=0x%s body=%s owner=0x%s weapons=%s" % (parent, e["body"], e["owner"], " ".join(e["m"][:4])))
        wm = f.get("wmname") or set()
        if wm:
            pm = {p[4].lower() for p in pv}
            out.append("PREVIEW mname vs in-world target=1 mname: shared=%s preview-only=%s" % (
                ",".join(sorted(pm & wm)) or "-", ",".join(sorted(pm - wm)) or "-"))
        bodies = {k[1] for k in (f.get("owner") or {})}
        if bodies:
            pb = {p[3] for p in pv}
            out.append("PREVIEW body vs in-world body: shared=%s preview-only=%s" % (",".join(sorted(pb & bodies)) or "-", ",".join(sorted(pb - bodies)) or "-"))
    for parent, pm, pp, ppm, cp, cpp in (f.get("camnode") or [])[:8]:
        out.append("CAMNODE parent=0x%s %s camhook=%s | pparent=0x%s %s camhook=%s" % (_hex(parent), pm, _hex(cp), _hex(pp), ppm, _hex(cpp)))
    return out


def _place_diag(f):
    """TUNE boot, PLACEREFUSE (highest n seen per reason; with log_verbose=1 the DLL logs the first 5 then every 500th, in quiet mode (default) the first hit of each reason, each new (reason, gob) pair and every 10000th hit) and PLACENODE verdicts. Empty for old logs."""
    out = []
    if f.get("tune"):
        out.append("TUNE boot %s" % f["tune"][-1][:90])
    if f.get("refuse"):
        out.append("PLACEREFUSE (n>=): " + ", ".join("%s=%d" % kv for kv in sorted(f["refuse"].items(), key=lambda kv: -kv[1])))
    if f.get("node"):
        top = sorted(f["node"].items(), key=lambda kv: (" target=1" not in kv[0], -kv[1]))[:5]   # weapons (target=1) first
        out.append("PLACENODE distinct=%d: %s" % (len(f["node"]), "; ".join("%s x%d" % kv for kv in top)))
    if f.get("owner"):   # C7f: [parent+0x1D8] of weapon nodes vs the COMB creatures; body = per-body offset key
        comb = f.get("combc", set())
        out.append("OWNER (weapon parent+0x1D8 / body): " + ", ".join(
            "0x%s %s comb=%s x%d" % (k[0], k[1], ("yes" if k[0] in comb else "no") if comb else "n/a", v) for k, v in f["owner"].most_common(5)))
    out += _preview_diag(f)
    if f.get("mount"):
        out.append("MOUNT " + ", ".join("%s bone=%s x%d" % (k[0], k[1], v) for k, v in f["mount"].most_common(5)))
    return out


def _events(recs):
    """GOBREUSE / FLOURISH / SLIDE one-liners (always printed, so a 0 is a scanned zero, not a missing section)."""
    reuse = sum(1 for r in recs if r["kind"] == "GOBREUSE")
    fl = Counter(r["ev"] for r in recs if r["kind"] == "FLOURISH")
    sl = [r for r in recs if r["kind"] == "SLIDE"]
    starts = Counter(r["dir"] for r in sl if r["ev"] == "start")
    ends = [r for r in sl if r["ev"] == "end"]
    line = "SLIDE start=%d end=%d draw=%d stow=%d" % (sum(starts.values()), len(ends), starts["draw"], starts["stow"])
    if ends:
        line += " frames min=%d max=%d maxdt max=%d" % (min(r["frames"] for r in ends), max(r["frames"] for r in ends),
                                                        max(r["maxdt"] for r in ends))
    return ["GOBREUSE total=%d" % reuse, "FLOURISH open=%d close=%d" % (fl["open"], fl["close"]), line]


def _toggle_events(f):
    """PARTYHOLD / MOVEHOLD / DELAY one-liners (always printed, so a 0 is a scanned zero)."""
    ph, dl = f.get("partyhold", Counter()), f.get("delay", Counter())
    return ["PARTYHOLD on=%d off=%d" % (ph["on"], ph["off"]), "MOVEHOLD total=%d" % f.get("movehold", 0),
            "DELAY cuts=%d combat=%d flourish=%d" % (sum(dl.values()), dl["combat"], dl["flourish"])]


def _reach_events(f):
    """REACH one-liner (HW19 probe): sampled steps, persist (engine did not resample) / wrote counts, err range, refusals."""
    r = f.get("reach", [])
    ref = f.get("reachrefuse", Counter())
    line = "REACH found=%d steps=%d persist1=%d wrote_both=%d refuse=%s" % (
        f.get("reachfound", 0), len(r), sum(1 for x in r if x[3]), sum(1 for x in r if (x[4] & 3) == 3),
        ",".join("%s:%d" % kv for kv in sorted(ref.items())) or "none")
    errs = [x[2] for x in r if x[2] >= 0]
    if errs:
        line += " err min=%.3f med=%.3f max=%.3f reach<1=%d" % (min(errs), sorted(errs)[len(errs) // 2], max(errs), sum(1 for x in r if x[1] < 0.999))
    # HW24 (0.12.0-dev): wrist bit (wrote & 4), hinge, g (mount vs post-IK hand), cap, residual, hard-reset cuts, all on the one line
    line += " wrist=%d hg=%d" % (sum(1 for x in r if x[4] & 4), sum(1 for x in r if len(x) > 6 and x[6] == 1.0))
    gs = [x[11] for x in r if len(x) > 12 and x[11] is not None and x[11] >= 0]
    if gs:
        line += " g max=%.0f over_cap=%d" % (max(gs), sum(1 for x in r if len(x) > 12 and x[11] is not None and x[12] is not None and x[11] - x[12] > 20))
    rs = f.get("resid", [])
    if rs:
        line += " resid max=%.1fcm/%.0fdeg" % (max(x[0] for x in rs), max(x[1] for x in rs))
    cuts = f.get("slidecut", Counter())
    if cuts:
        line += " cut=%s" % ",".join("%s:%d" % kv for kv in sorted(cuts.items()))
    # HW25 (0.14.0-dev): held frames by refusal kind, and the solves that followed a hold episode (hold=N on the full REACH line)
    held = f.get("reachhold", Counter())
    after = [int(x[13]) for x in r if len(x) > 13 and x[13] is not None and x[13] > 0]
    if held or after:
        line += " hold=%s after_hold=%d max=%d" % (",".join("%s:%d" % kv for kv in sorted(held.items())) or "none", len(after), max(after) if after else 0)
    # HW27 (0.15.0-dev): topo=<hinge|over|hip|thigh>[+bk]:N (+bk = a back bone, e.g. torso_g now resolves to hip); HW28: the last CONFIG slide= timing
    topo = f.get("reachtopo", Counter())
    if topo:
        line += " topo=%s" % ",".join("%s%s:%d" % (k[0], "+bk" if k[1] else "", n) for k, n in sorted(topo.items()))
    tws = f.get("reachtw", [])
    if tws:   # HW29 (0.17.0-dev): tw= forearm twist (<= reach_twist_deg), sw= wrist swing (<= reach_wrist_max_deg); twisted = frames with a twist above 1 deg
        line += " tw max=%.0f sw max=%.0f twisted=%d" % (max(x[0] for x in tws), max(x[1] for x in tws), sum(1 for x in tws if x[0] > 1.0))
    cfg = f.get("slidecfg", [])
    if cfg:
        line += " slide=%s resid_ms=%d wrist_max=%d" % cfg[-1]
    return [line]


def render(recs, skipped, main_facts, nlines, meta):
    has_place = any(r["kind"] == "PLACE" for r in recs) or any(k == "PLACE" for _, k, _ in skipped)
    out = ["HOLSTER lines=%d parsed=%d skipped=%d%s" % (nlines, len(recs), len(skipped), meta), _status(main_facts)]
    out += _place_diag(main_facts)
    comb = _comb(recs)
    un = main_facts.get("unreadable", 0)
    if un:
        comb[0] += " unreadable=%d" % un
    out += comb
    out += _attach(recs)
    out.append("SWAP total=%d" % sum(1 for r in recs if r["kind"] == "SWAP"))
    out += _events(recs)
    out += _toggle_events(main_facts)
    out += _reach_events(main_facts)
    out.append(_hist("ANIMRES", recs))
    styles = Counter((r["self"], r["style"], r["gate"]) for r in recs if r["kind"] == "ANIMRES")
    if styles:   # C6: +0xC5 = wield style byte, +0xC6 = table gate; one entry per creature and value
        out[-1] += " | style per creature (self style/gate): " + ", ".join("%s %d/%d x%d" % (k[0], k[1], k[2], v)
                                                                            for k, v in styles.most_common(8))
    out[-1] += _animstyle(main_facts, recs)
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
