#!/usr/bin/env python3
"""Holster probe log checker (holster route). Python 3, stdlib only; read-only (--new writes only its bookmark).

Usage: holster_log_check.py [LOG | --log LOG] [--new [--no-advance] [--state PATH]] [--mounts [--json]] [--row SM-L11..SM-L16]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/holster_log.txt
  Report (at most 51 lines: the 47-line base report plus at most 3 HW38 lines, SLIDE dly / REACH HW38 / REACH min/median/max, each only when its
  field appears in the window, plus the HW41 `STOWHOLD` summary line when any STOWHOLD line exists): header, status (loaded / cap reached / BONETEST), COMB edges per creature (plus unreadable=N for `old=? edge=?` lines), ATTACH tags and
  (body, handleR, handleL) tuples per creature, SWAP total, GOBREUSE total, FLOURISH open/close, SLIDE start/end (draw/stow split, frames min/max, maxdt max), PARTYHOLD on/off, MOVEHOLD total, DELAY cuts (combat/flourish), ANIMRES and ANIMSEND id histograms, GOB samples per gob
  (the 8 with the most samples: parent18, pos min/max/mean per axis, quat min/max per component, mean-quat angle),
  PLACE (only when PLACE lines exist: count, gobvt histogram, quat1 unit-norm check, last pos0/quat0 -> pos1/quat1 for the
  4 gobs with the most samples; the GOB view then drops to 4 gobs so the report stays within 47 lines). REACH (HW19 probe): found, steps, persist1, wrote_both, refusals, err range.
  HW40: a REACH step the KPM 232-byte cut split into three lines is joined back first; `partial=N` on the REACH line counts steps the window cut (shown only when N > 0).
  MOUNT (HW35, only when `MOUNT gob=` lines exist; GOB view drops to 2 gobs): per (cat, bone) at most 3 rows: count (nofacing included), nofacing,
  component-wise median of rel (sign-aligned, normalized; nofacing excluded), spread = max angle of any rel from that median.
  --mounts: print only the MOUNT section. --mounts --json: the same per (cat, bone) groups as a JSON list on stdout, nothing else (--json needs --mounts).
  HW38 (0.19/0.20 fields, each line only when its field appears in the window): `SLIDE dly` (dly= on SLIDE start, count and max per Gob),
  `REACH HW38` (vo= max and vo>0 outside mid-carry, pb= max and pb>0 at f>0.30), `REACH min/median/max` (wr, f, dzA, fy, fx).
  HW41 (holster 0.20.3-dev, log only): `STOWHOLD p= pathF= carryW= wd= dm= lf= hold=` lines (first ~8 stows, log_verbose/reach_test) are a window
  fact, not a record: one summary line `STOWHOLD n= hold=<n>/<n> | wd max while pathF>lf | wd max overall`; a line that does not parse is skipped.
  --row SM-L11..SM-L16: pass-signal clauses of one play-test row (PLAY_TEST_QUEUE.md), `SM-Lnn <clause>: PASS|FAIL|NO-DATA <reason>`, then
  `SM-Lnn verdict`. SM-L16 (STOWHOLD, hold=1 lines only): hold on; the weapon stays on the hand (wd <= 0.005 m) while pathF > lf + 0.01; lands on
  the mount (|wd - dm| <= 0.02 m) at pathF <= 0.02. Exit 0 PASS, 1 FAIL, 3 NO-DATA; unknown row = exit 2.
  --new: only lines after the bookmark shared with log_triage (same state file, keyed by absolute path), then advance it.
  Exit 0 = analyzed, 2 = log missing ("no log: <path>") or a bad flag combination.

Grammar: patch_manager_mods/holster_src/Holster.cpp. Each keyword is found with a regex search, so a KpLog stamp in front
is fine. A keyword line whose fields do not parse is counted in skipped (one example line is shown).
GOB quat is logged from Gob+0xB0 in engine order (w, x, y, z; 01_inventory_3d_viewport.md). The mean angle sign-aligns each
sample to the first one, averages, normalizes, and reports 2*acos(|w|). Min/max use the logged values as-is; sign_flips=k
counts samples whose sign disagrees with the first one (a flipped sample is the same rotation).
"""
from __future__ import annotations

import argparse
import json
import math
import os
import re
import statistics
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import log_triage as LT  # noqa: E402  (--new bookmark: same state file and helpers; log_triage is not modified)

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/holster_log.txt"
CAPS = {"comb": 4, "attach": 3, "tuples": 3, "tags": 5, "ids": 10, "gobs": 8,
        "gobs_with_place": 4, "place_gobs": 4, "gobs_with_mount": 2, "mounts": 3}   # GOB view and PLACE view both shrink so the report stays <= 47 lines
UNIT_TOL = 2e-3   # PLACE quat1 |norm-1| limit: the DLL enforces 1e-3 (poseSane); %.3f printing adds up to ~1e-3 rounding
HEX = r"[0-9a-fA-F]+"
INT = r"-?\d+"
NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)"


def _rx(template):
    """Compile a pattern whose <H> <I> <N> tokens stand for hex, signed int and decimal float."""
    return re.compile(template.replace("<H>", HEX).replace("<I>", INT).replace("<N>", NUM))


KIND_RE = re.compile(r"\b(COMB|ATTACH|SWAP|ANIMRES|ANIMSEND|GOB|PLACE|GOBREUSE|FLOURISH|SLIDE) ")
# HW35 MOUNT lines start with `gob=`; the older chPlace line `MOUNT <what> parent=...` is not a record (MAIN_RES["mount"] counts it).
MOUNT_HEAD_RE = re.compile(r"\bMOUNT gob=")
MOUNT_NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"
# HW38: SLIDE cut lines carry owner and gob (the start/end records do not all of them: end has no gob); REACH vo=/pb=/f= are read off the whole line.
SLIDE_CUT_EV_RE = _rx(r"\bSLIDE cut reason=\w+ owner=(?P<owner><H>) gob=(?P<gob><H>) left=(?P<left><I>)")
REACH_LINE_RE = re.compile(r"\bREACH owner=\w+ w=")
REACH_VO_RE = re.compile(r"\bREACH owner=\w+ w=.*? vo=(%s)" % NUM)
REACH_PB_RE = re.compile(r"\bREACH owner=\w+ w=.*? pb=(%s)" % NUM)
REACH_F_RE = re.compile(r"\bREACH owner=\w+ w=.*? f=(%s)" % NUM)
# HW40 (holster 0.20.2-dev): the KPM core cuts a log message at 232 bytes, so one REACH step is logged as three lines: piece 1 ends in ` #<seq>`,
# pieces 2 and 3 start `REACH+ #<seq> owner=<hex> `. _join_reach() rebuilds the old single-line REACH text from them.
REACH_HEAD_RE = re.compile(r"\bREACH owner=\w+ (?P<body>.*?) #(?P<seq>\d+)$")
REACH_CONT_RE = re.compile(r"\bREACH\+ #(?P<seq>\d+) owner=\w+ (?P<body>.*)$")
# HW38 edges of the carry: the printed f is 2 decimals, so 0.00 and 1.00 are reach/release (the via bump is 0 there); anything between is mid-carry.
CARRY_EDGE_F = 0.005
# SM-L15: vo comes from the via clock (viaProgress), not from the printed f= clock, so a tiny vo at a printed edge is not a FAIL:
# vo below this (m) counts as 0 for "vo=0 at reach/release".
VO_EDGE_TOL = 0.005
# HW38 SM-L14: the stagger window for dly= on the left Gob (ms), and the via/pole limits of SM-L11..SM-L15
DLY_LO_MS, DLY_HI_MS = 50, 70
PB_F_MAX = 0.30          # HW29b kPoleBiasEndF: pb > 0 only at f <= this
EX_EXTRACT = 0.05        # the existing "extracting" threshold on ex= (same as the summary)
# HW41 (holster 0.20.3-dev): STOWHOLD p=<slide progress 0..1> pathF=<hand path fraction, 1 hand end .. 0 mount> carryW=<carry weight>
# wd=<m, shown weapon to carried hand> dm=<m, hand to mount> lf=<stow landing fraction> hold=<1 path-driven hold, 0 old time window>.
# Trailing fields are tolerated (no end anchor). SM-L16 limits: margins are inclusive, EPS absorbs float noise on the %.2f/%.3f prints.
STOWHOLD_HEAD_RE = re.compile(r"\bSTOWHOLD ")
STOWHOLD_RE = _rx(r"\bSTOWHOLD p=(?P<p><N>) pathF=(?P<pathF><N>) carryW=(?P<carryW><N>) wd=(?P<wd><N>) dm=(?P<dm><N>) lf=(?P<lf><N>) hold=(?P<hold>\d+)")
STOW_JUDGE_MARGIN = 0.01  # SM-L16 (b): a line is judged only when pathF > lf + this
STOW_WD_MAX = 0.005       # SM-L16 (b): m, wd on a judged line must be <= this
STOW_MOUNT_F = 0.02       # SM-L16 (c): pathF <= this is at the mount
STOW_LAND_TOL = 0.02      # SM-L16 (c): m, |wd - dm| on a line at the mount must be <= this
STOW_EPS = 1e-9


def _mount_quat(name):
    """(w,x,y,z) as named groups name0..name3, or the literal nan (groups stay None)."""
    return r"(?:nan|\(" + ",".join(r"(?P<%s%d>%s)" % (name, i, MOUNT_NUM) for i in range(4)) + r"\))"


MOUNT_RX = (r"\bMOUNT gob=(?P<gob><H>) stem=(?P<stem>\S+) bone=(?P<bone>\S+) bq=" + _mount_quat("bq") + " mq=" + _mount_quat("mq")
            + r" (?:oq=nan rel=nan|oq=" + _mount_quat("oq") + " rel=" + _mount_quat("rel") + r") cat=(?P<cat><I>)")
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
    "MOUNT": _rx(MOUNT_RX),
    "GOBREUSE": _rx(r"\bGOBREUSE gob=(?P<gob><H>) node=(?P<node><H>) oldm=(?P<oldm>\S+) newm=(?P<newm>\S+)"),
    "FLOURISH": _rx(r"\bFLOURISH (?:(?P<open>open) slot=(?P<oslot><I>) cli=<I> srv=<I> p524=<I> p528=<I> b11E8=<I>"
                  r"|(?P<close>close) slot=(?P<cslot><I>) p520=<I> p524=<I>)"),
    # HW38: start may end with dly=<ms> (dual stagger, left Gob); owner/gob/left are named for the SM-L14 open/close matching
    "SLIDE": _rx(r"\bSLIDE (?:(?P<start>start) dir=(?P<sdir>\w+) owner=(?P<sowner><H>) gob=(?P<sgob><H>) left=(?P<sleft><I>) from=<N> ms=<I>(?: dly=(?P<dly><I>))?"
                 r"|(?P<end>end) dir=(?P<edir>\w+) owner=(?P<eowner><H>) ms=<I> frames=(?P<frames><I>) maxdt=(?P<maxdt><I>))"),
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
    "reachex": re.compile(r"\bREACH owner=\w+ w=[\d.]+ .*? sw=-?[\d.]+ ex=(-?[\d.]+) of=(-?[\d.]+)"),   # HW31 (0.18.0-dev): extract bump (extractK) and orientation progress (oriF) of the frame; optional
    "reachhold": re.compile(r"\bREACH owner=\w+ hold=(\d+) why=(\w+)"),   # HW25 (0.14.0-dev): a frame where a transient refusal re-applied the last arm
    "reachrefuse": re.compile(r"\bREACH owner=\w+ refuse=(\w+)"),
    "reachtopo": re.compile(r"\bREACH owner=\w+ w=[\d.]+ .*? topo=(\w+) bk=(\d)"),   # HW27 (0.15.0-dev): which reach the mount height chose (hinge / over / hip / thigh), bk=1 = a back bone
    "slidecfg": re.compile(r"\bCONFIG .*?\bslide=(\S+) resid_ms=(\d+) wrist_max=(\d+)"),   # HW28 (0.15.0-dev): resolved <cat>:<draw ms>/<stow ms>/<grab %>, residual ms, wrist cap
    "node": re.compile(r"\bPLACENODE node=\w+ gob=\w+ t=(\d+) step=(-?\d+) name=(\S+) target=(\d)(?: bvt=(\w+))?(?: gname=(\S+) mname=(\S+))?(?: cat=(\w+))?"),
}


def _hex(s):
    """Lowercase hex without leading zeros, so 0x001F and 1f are one key."""
    return "%x" % int(s, 16)


def _quat_of(g, name):
    """The four components, or None for a literal `nan` (the DLL never prints one for bq/mq, but a hand-made line may)."""
    if g["%s0" % name] is None:
        return None
    return [float(g["%s%d" % (name, i)]) for i in range(4)]


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
    elif kind == "MOUNT":   # HW35: oq/rel are None when the owner facing is unknown (`oq=nan rel=nan`): nofacing, not in the median
        nofacing = g["oq0"] is None
        rec.update(gob=_hex(g["gob"]), stem=g["stem"], bone=g["bone"], cat=int(g["cat"]), nofacing=nofacing,
                   bq=_quat_of(g, "bq"), mq=_quat_of(g, "mq"),
                   oq=None if nofacing else _quat_of(g, "oq"), rel=None if nofacing else _quat_of(g, "rel"))
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


def _join_reach(window):
    """HW40: [(line_no, raw)] -> (window with each split REACH step joined into one line, partial).
    A step is piece 1 (`REACH owner=.. ... #<seq>`) plus its two continuations (`REACH+ #<seq> owner=.. <rest>`), matched by seq, the
    continuations after piece 1 in the same window (other lines may sit between). The joined line is piece 1 minus ` #<seq>` plus ONE space
    and each continuation's rest, i.e. the old single-line REACH text. A piece 1 with fewer than two continuations stays as it was (it fails
    the full REACH regexes, as a cut line always did) and its continuations are dropped. A continuation with no earlier piece 1 is dropped.
    partial = the number of seqs cut this way (0 for old single-line logs, which get the same window back unchanged)."""
    out = []                # [line_no, raw]; a piece 1 is rewritten below once its continuations are known
    heads = {}              # seq -> [index in out, [continuation bodies], piece 1 text up to its body]
    cut = set()             # seqs with a continuation whose piece 1 is not earlier in the window
    for n, raw in window:
        text = raw.strip()
        mc = REACH_CONT_RE.search(text)
        if mc:
            if mc.group("seq") in heads:
                heads[mc.group("seq")][1].append(mc.group("body"))
            else:
                cut.add(mc.group("seq"))
            continue
        mh = REACH_HEAD_RE.search(text)
        if mh:
            heads[mh.group("seq")] = [len(out), [], text[:mh.end("body")]]
        out.append([n, raw])
    partial = set(cut)
    for seq, (idx, conts, head) in heads.items():
        if len(conts) >= 2:
            out[idx][1] = head + "".join(" " + c for c in conts)
        else:
            partial.add(seq)
    return [(n, raw) for n, raw in out], len(partial)


def parse_lines(window):
    """window: [(line_no, raw)] -> (records in log order, skipped [(line_no, kind, text)], main facts)."""
    window, reach_partial = _join_reach(window)
    recs, skipped = [], []
    main = {"loaded": [], "cap": [], "bonetest": Counter(), "unreadable": 0, "tid": {}, "reload": [], "tune": [], "refuse": {}, "node": Counter(), "mount": Counter(),
            "owner": Counter(), "combc": set(), "partyhold": Counter(), "movehold": 0, "delay": Counter(),
            "reach": [], "slidecut": Counter(), "resid": [], "reachfound": 0, "reachrefuse": Counter(), "reachhold": Counter(), "reachtw": [], "reachex": [],
            "reachtopo": Counter(), "slidecfg": [], "reach_partial": reach_partial,   # HW40: REACH steps cut by the window (partial=N on the REACH line when N > 0)
            "preview": [], "camnode": [], "wmname": set(),
            "slide_seq": [], "vo": [], "pb": [],   # HW38: SLIDE start/end/cut events in log order; (f or None, vo) and (f or None, pb) per REACH line
            "stowhold": []}   # HW41: one dict per parsed STOWHOLD line, in log order
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
            mce = SLIDE_CUT_EV_RE.search(text)   # HW38: the cut closes the running slide of that Gob (SM-L14)
            if mce:
                main["slide_seq"].append({"ev": "cut", "owner": _hex(mce.group("owner")), "gob": _hex(mce.group("gob")), "left": int(mce.group("left")) == 1})
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
        m = MAIN_RES["reachex"].search(text)
        if m:
            main["reachex"].append((float(m.group(1)), float(m.group(2))))
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
        if REACH_LINE_RE.search(text):   # HW38: vo= / pb= (0.20.0-dev) with the f= of the same line; a missing f is None (not classified)
            mf = REACH_F_RE.search(text)
            fval = float(mf.group(1)) if mf else None
            mv = REACH_VO_RE.search(text)
            if mv:
                main["vo"].append((fval, float(mv.group(1))))
            mp = REACH_PB_RE.search(text)
            if mp:
                main["pb"].append((fval, float(mp.group(1))))
        if STOWHOLD_HEAD_RE.search(text):   # HW41: a window fact, not a record (so never in recs); a line whose fields do not parse is skipped
            fs = STOWHOLD_RE.search(text)
            if fs is None:
                skipped.append((n, "STOWHOLD", text))
            else:
                main["stowhold"].append({k: (int(v) if k == "hold" else float(v)) for k, v in fs.groupdict().items()})
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
        if MOUNT_HEAD_RE.search(text):   # HW35: a bad quat or a zero quat (cannot be normalized) is skipped, never raises
            f = FULL_RE["MOUNT"].search(text)
            rec = _record("MOUNT", n, f) if f else None
            if rec is not None and not all(q is not None and _norm(q) > 1e-9 for q in _mount_quats(rec)):
                rec = None
            if rec is None:
                skipped.append((n, "MOUNT", text))
            else:
                recs.append(rec)
            continue
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
            if kind == "SLIDE":   # HW38: start (with its optional dly=) or end (owner only: the end line names no Gob)
                if f.group("start"):
                    main["slide_seq"].append({"ev": "start", "owner": _hex(f.group("sowner")), "gob": _hex(f.group("sgob")), "left": int(f.group("sleft")) == 1,
                                              "dir": f.group("sdir"), "dly": int(f.group("dly")) if f.group("dly") else None})
                else:
                    main["slide_seq"].append({"ev": "end", "owner": _hex(f.group("eowner")), "dir": f.group("edir")})
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


def _mount_quats(rec):
    return [rec["bq"], rec["mq"]] + ([] if rec["nofacing"] else [rec["oq"], rec["rel"]])


def _unit(q):
    n = _norm(q)
    return [c / n for c in q]


def _mount_stats(samples):
    """(median rel, spread deg) over the facing samples: each rel sign-aligned to the first, normalized, component-wise median,
    normalized; spread = max angle between any rel and that median. (None, None) when there is no usable facing sample."""
    facing = [_unit(s["rel"]) for s in samples if not s["nofacing"]]
    if not facing:
        return None, None
    ref = facing[0]
    aligned = [[(-1.0 if sum(a * b for a, b in zip(q, ref)) < 0 else 1.0) * c for c in q] for q in facing]
    raw = [statistics.median(q[i] for q in aligned) for i in range(4)]
    if _norm(raw) < 1e-9:
        return None, None
    med = _unit(raw)
    spread = max(2.0 * math.degrees(math.acos(min(1.0, abs(sum(a * b for a, b in zip(q, med)))))) for q in facing)
    return med, spread


def _mounts(recs, skipped):
    """MOUNT section (HW35): one row per (cat, bone), most samples first, at most CAPS["mounts"] rows."""
    rows = [r for r in recs if r["kind"] == "MOUNT"]
    per = {}
    for r in rows:
        per.setdefault((r["cat"], r["bone"]), []).append(r)
    shown = sorted(per.items(), key=lambda kv: -len(kv[1]))[:CAPS["mounts"]]   # stable: ties keep first-seen order
    bad = sum(1 for _, kind, _ in skipped if kind == "MOUNT")
    head = "MOUNT lines=%d nofacing=%d skipped=%d groups=%d" % (len(rows), sum(1 for r in rows if r["nofacing"]), bad, len(per))
    if len(per) > len(shown):
        head += " (top %d by samples shown)" % len(shown)
    out = [head]
    for (cat, bone), samples in shown:
        med, spread = _mount_stats(samples)
        out.append("  cat=%d bone=%s count=%d nofacing=%d median rel=%s spread=%s" % (
            cat, bone, len(samples), sum(1 for s in samples if s["nofacing"]),
            "n/a" if med is None else "(%s)" % _vec(med),
            "n/a" if spread is None else "%.2f deg" % spread))
    return out


def _jnum(x):
    """A finite float for JSON, else None (inf/nan from an absurd exponent never reaches the JSON)."""
    return float(x) if x is not None and math.isfinite(x) else None


def _mounts_json(recs):
    """HW38 --mounts --json: one entry per (cat, bone), sorted by cat then bone, no cap. rel = median (w, x, y, z) or null when no facing
    sample; spread = deg (unrounded, the same value SM-L13 judges) or null."""
    per = {}
    for r in recs:
        if r["kind"] == "MOUNT":
            per.setdefault((r["cat"], r["bone"]), []).append(r)
    out = []
    for (cat, bone), samples in sorted(per.items(), key=lambda kv: (kv[0][0], kv[0][1])):
        med, spread = _mount_stats(samples)
        out.append({"cat": cat, "bone": bone, "count": len(samples), "nofacing": sum(1 for s in samples if s["nofacing"]),
                    "rel": None if med is None else [_jnum(v) for v in med], "spread": _jnum(spread)})
    return out


def _slide_dly_lines(f):
    """HW38: dly= on SLIDE start lines (dual stagger; the left Gob): count and max per Gob. [] when no start carries dly=."""
    starts = [e for e in f.get("slide_seq", []) if e["ev"] == "start" and e["dly"] is not None]
    if not starts:
        return []
    per = {}
    for e in starts:
        per.setdefault(e["gob"], []).append(e)
    ordered = sorted(per.items(), key=lambda kv: -len(kv[1]))   # stable: ties keep first-seen order
    parts = ["gob=0x%s %s n=%d max=%d" % (gid, "L" if v[0]["left"] else "R", len(v), max(e["dly"] for e in v)) for gid, v in ordered[:4]]
    if len(ordered) > 4:
        parts.append("+%d more gobs" % (len(ordered) - 4))
    return ["SLIDE dly n=%d max=%d per Gob: %s" % (len(starts), max(e["dly"] for e in starts), "; ".join(parts))]


def _mid_carry(fv):
    """HW38: f strictly inside the carry. The printed f has 2 decimals, so 0.00 and 1.00 are the reach/release edges (CARRY_EDGE_F)."""
    return CARRY_EDGE_F < fv < 1.0 - CARRY_EDGE_F


def _reach_hw38_lines(f):
    """HW38 REACH summaries: vo= (max, vo>0 outside mid-carry), pb= (max, pb>0 at f>0.30), wr/f/dzA/fy/fx min/median/max.
    Each line is printed only when its field appears in the window."""
    out = []
    vo, pb = f.get("vo", []), f.get("pb", [])
    parts = []
    if vo:
        outside = sum(1 for fv, v in vo if fv is not None and not _mid_carry(fv) and v > 0)
        parts.append("vo max=%.3f vo>0 outside mid-carry=%d (%.1f%% of %d lines)" % (max(v for _, v in vo), outside, 100.0 * outside / len(vo), len(vo)))
    if pb:
        late = sum(1 for fv, v in pb if fv is not None and fv > PB_F_MAX and v > 0)
        parts.append("pb max=%.1f pb>0 at f>%.2f=%d (of %d lines)" % (max(v for _, v in pb), PB_F_MAX, late, len(pb)))
    if parts:
        out.append("REACH HW38 " + " | ".join(parts))
    stats = []
    for name, idx, fmt in (("wr", 5, "%.0f"), ("f", 7, "%.2f"), ("dzA", 8, "%.2f"), ("fy", 9, "%.2f"), ("fx", 10, "%.2f")):
        vals = [x[idx] for x in f.get("reach", []) if len(x) > idx and x[idx] is not None]
        if vals:
            stats.append("%s %s/%s/%s" % (name, fmt % min(vals), fmt % statistics.median(vals), fmt % max(vals)))
    if stats:
        out.append("REACH min/median/max: " + " | ".join(stats))
    return out


def _stowhold_lines(f):
    """HW41: one STOWHOLD summary line, only when the window has a STOWHOLD line (else []). wd in cm; `n/a` when no line has pathF > lf."""
    rows = f.get("stowhold", [])
    if not rows:
        return []
    held = sum(1 for r in rows if r["hold"] == 1)
    past = [r["wd"] for r in rows if r["pathF"] > r["lf"]]
    past_txt = ("%.1f cm" % (100.0 * max(past))) if past else "n/a"
    return ["STOWHOLD n=%d hold=%d/%d | wd max while pathF>lf: %s | wd max overall: %.1f cm"
            % (len(rows), held, len(rows), past_txt, 100.0 * max(r["wd"] for r in rows))]


# ---- --row SM-Lnn: pass-signal clauses of the play-test rows (PLAY_TEST_QUEUE.md). Each clause is (label, PASS|FAIL|NO-DATA, reason).
ROWS = ("SM-L11", "SM-L12", "SM-L13", "SM-L14", "SM-L15", "SM-L16")
ROW_EXIT = {"PASS": 0, "FAIL": 1, "NO-DATA": 3}


def _max_clause(label, vals, limit, inclusive, fmt):
    """PASS when max(vals) <= limit (inclusive) or < limit; FAIL otherwise; NO-DATA when no line carries the field."""
    if not vals:
        return (label, "NO-DATA", "no line carries the field")
    worst = max(vals)
    ok = worst <= limit if inclusive else worst < limit
    return (label, "PASS" if ok else "FAIL", ("max " + fmt + " over %d line(s)") % (worst, len(vals)))


def _sm_l11(main):
    """Forearm twist and wrist swing caps (the REACH tw=/sw= tail, HW29) and the g-cap margin (the REACH g=/cap= fields, HW24)."""
    tws = main.get("reachtw", [])
    gc = [x[11] - x[12] for x in main.get("reach", []) if len(x) > 12 and x[11] is not None and x[12] is not None]
    return [_max_clause("tw<=80", [t for t, _ in tws], 80.0, True, "%.0f"),
            _max_clause("sw<=60", [s for _, s in tws], 60.0, True, "%.0f"),
            _max_clause("g-cap<20", gc, 20.0, False, "%.0f")]


def _sm_l12(main):
    """Extraction (ex=) and orientation (of=, HW31): of stays 0 on every extracting frame (ex > 0.05), and of reaches 1 somewhere."""
    ex = main.get("reachex", [])
    if not ex:
        return [("of=0 through lift", "NO-DATA", "no REACH line with ex="), ("of reaches 1", "NO-DATA", "no REACH line with ex=")]
    lift = [(e, o) for e, o in ex if e > EX_EXTRACT]
    if not lift:
        c1 = ("of=0 through lift", "NO-DATA", "no extracting frame (ex<=%.2f on all %d lines): lift not seen" % (EX_EXTRACT, len(ex)))
    else:
        bad = [o for _, o in lift if o > EX_EXTRACT]
        if bad:
            c1 = ("of=0 through lift", "FAIL", "of>%.2f on %d of %d extracting frames (max of=%.2f)" % (EX_EXTRACT, len(bad), len(lift), max(bad)))
        else:
            c1 = ("of=0 through lift", "PASS", "of<=%.2f on all %d extracting frames (max ex=%.2f)" % (EX_EXTRACT, len(lift), max(e for e, _ in lift)))
    top = max(o for _, o in ex)
    if top >= 0.99:
        c2 = ("of reaches 1", "PASS", "max of=%.2f" % top)
    else:
        c2 = ("of reaches 1", "NO-DATA", "max of=%.2f: no full orientation (a reversed draw stops short)" % top)
    return [c1, c2]


def _spread_clause(spreads):
    """spreads: [(spread deg, cat, bone)] over the groups with a facing sample. FAIL when any spread >= 5.0 (exactly 5.0 fails);
    NO-DATA when there is none."""
    if not spreads:
        return ("spread<5 deg", "NO-DATA", "no facing sample in any MOUNT group")
    worst = max(spreads)
    over = [g for g in spreads if g[0] >= 5.0]
    if over:
        return ("spread<5 deg", "FAIL", "%d group(s) at or above 5 deg; worst cat=%d bone=%s spread=%.2f" % (len(over), worst[1], worst[2], worst[0]))
    return ("spread<5 deg", "PASS", "%d group(s), max spread=%.2f deg (cat=%d bone=%s)" % (len(spreads), worst[0], worst[1], worst[2]))


def _sm_l13(recs):
    """Rest-pose mount capture (HW35): spread < 5 deg per (cat, bone) (exactly 5.0 fails) and nofacing == 0. Old `MOUNT <what> parent=` lines are not records."""
    per = {}
    for r in recs:
        if r["kind"] == "MOUNT":
            per.setdefault((r["cat"], r["bone"]), []).append(r)
    if not per:
        return [("spread<5 deg", "NO-DATA", "no MOUNT gob= lines"), ("nofacing==0", "NO-DATA", "no MOUNT gob= lines")]
    groups = sorted(per.items())
    spreads = []
    for (cat, bone), samples in groups:
        _, sp = _mount_stats(samples)
        if sp is not None:
            spreads.append((sp, cat, bone))
    c1 = _spread_clause(spreads)
    nof = [(cat, bone, sum(1 for s in samples if s["nofacing"])) for (cat, bone), samples in groups]
    bad = [g for g in nof if g[2] > 0]
    if bad:
        c2 = ("nofacing==0", "FAIL", "nofacing>0 in %d group(s); first cat=%d bone=%s nofacing=%d" % (len(bad), bad[0][0], bad[0][1], bad[0][2]))
    else:
        c2 = ("nofacing==0", "PASS", "nofacing=0 in all %d group(s)" % len(nof))
    return [c1, c2]


def _sm_l14(main):
    """Dual stagger (HW33a): dly= on the left Gob in 50..70 ms; no dly= on a right Gob; every SLIDE start ends or is cut (a start with no end = stuck).
    No dly= field at all = NO-DATA for the whole row. A reversal logs a second start for the same Gob: that supersedes the first (not stuck)."""
    labels = ("dly 50..70 ms on left", "no dly on right", "every start ends or is cut")
    seq = main.get("slide_seq", [])
    starts = [e for e in seq if e["ev"] == "start"]
    dly = [e for e in starts if e["dly"] is not None]
    if not dly:
        return [(lb, "NO-DATA", "no dly= field in window") for lb in labels]
    left = [e["dly"] for e in dly if e["left"]]
    if not left:
        c1 = (labels[0], "NO-DATA", "no dly= on a left Gob")
    else:
        off = [v for v in left if not DLY_LO_MS <= v <= DLY_HI_MS]
        if off:
            c1 = (labels[0], "FAIL", "left dly=%d outside %d..%d ms (%d of %d left starts)" % (off[0], DLY_LO_MS, DLY_HI_MS, len(off), len(left)))
        else:
            c1 = (labels[0], "PASS", "left dly=%d..%d ms over %d left start(s)" % (min(left), max(left), len(left)))
    right = [e for e in dly if not e["left"]]
    if right:
        c2 = (labels[1], "FAIL", "dly=%d on right Gob gob=0x%s (%d right starts)" % (right[0]["dly"], right[0]["gob"], len(right)))
    else:
        c2 = (labels[1], "PASS", "no dly= on a right Gob (%d left starts)" % len(left))
    running, ended, superseded = [], 0, 0
    for e in seq:
        if e["ev"] == "start":
            prev = [o for o in running if o["owner"] == e["owner"] and o["gob"] == e["gob"]]
            if prev:
                running.remove(prev[0])
                superseded += 1
            running.append(e)
        elif e["ev"] == "cut":   # the cut names the Gob
            hit = [o for o in running if o["owner"] == e["owner"] and o["gob"] == e["gob"]]
            if hit:
                running.remove(hit[0])
                ended += 1
        else:                    # the end names only the owner: the oldest running start of that owner
            hit = [o for o in running if o["owner"] == e["owner"]]
            if hit:
                running.remove(hit[0])
                ended += 1
    if running:
        c3 = (labels[2], "FAIL", "%d start(s) never ended (stuck): first gob=0x%s dir=%s" % (len(running), running[0]["gob"], running[0]["dir"]))
    else:
        c3 = (labels[2], "PASS", "%d start(s): %d ended or cut, %d superseded by a reversal" % (len(starts), ended, superseded))
    return [c1, c2, c3]


def _sm_l15(main):
    """Via on the residual path (HW31b, vo=) and pole bias (HW29b, pb=): vo = 0 at reach/release and peaks mid-carry; pb > 0 only at f <= 0.30.
    Lines without vo=/pb= (older logs) are NO-DATA, never FAIL."""
    vo, pb = main.get("vo", []), main.get("pb", [])
    if not vo:
        out = [("vo=0 at reach/release", "NO-DATA", "no vo= field (older log)"), ("vo peaks mid-carry", "NO-DATA", "no vo= field (older log)")]
    else:
        known = [(fv, v) for fv, v in vo if fv is not None]
        if not known:
            out = [("vo=0 at reach/release", "NO-DATA", "no f= on the vo= lines"), ("vo peaks mid-carry", "NO-DATA", "no f= on the vo= lines")]
        else:
            edge = [v for fv, v in known if not _mid_carry(fv) and v >= VO_EDGE_TOL]
            if edge:
                c1 = ("vo=0 at reach/release", "FAIL", "vo>=%.3f m at reach/release on %d of %d lines (max vo=%.3f; vo<%.3f m counts as 0)"
                      % (VO_EDGE_TOL, len(edge), len(known), max(edge), VO_EDGE_TOL))
            else:
                c1 = ("vo=0 at reach/release", "PASS", "vo=0 at reach/release on all %d lines (vo<%.3f m counts as 0)" % (len(known), VO_EDGE_TOL))
            mid = [v for fv, v in known if _mid_carry(fv)]
            if not mid:
                c2 = ("vo peaks mid-carry", "NO-DATA", "no mid-carry frame in window")
            elif max(mid) > 0:
                c2 = ("vo peaks mid-carry", "PASS", "vo max=%.3f mid-carry over %d line(s)" % (max(mid), len(mid)))
            else:
                c2 = ("vo peaks mid-carry", "NO-DATA", "vo=0 mid-carry: no rifle or quarterstaff carry in window")
            out = [c1, c2]
    if not pb:
        out.append(("pb>0 only at f<=0.30", "NO-DATA", "no pb= field (older log)"))
    else:
        known = [(fv, v) for fv, v in pb if fv is not None]
        late = [v for fv, v in known if fv > PB_F_MAX and v > 0]
        if late:
            out.append(("pb>0 only at f<=0.30", "FAIL", "pb>0 at f>%.2f on %d line(s) (max pb=%.1f)" % (PB_F_MAX, len(late), max(late))))
        elif not known:
            out.append(("pb>0 only at f<=0.30", "NO-DATA", "no f= on the pb= lines"))
        elif max(v for _, v in pb) > 0:
            out.append(("pb>0 only at f<=0.30", "PASS", "pb>0 only at f<=%.2f (max pb=%.1f over %d line(s))" % (PB_F_MAX, max(v for _, v in pb), len(pb))))
        else:
            out.append(("pb>0 only at f<=0.30", "PASS", "pb=0 on all %d line(s): pole pass not run, clause holds vacuously" % len(pb)))
    return out


def _sm_l16(main):
    """HW41 stow hold (holster 0.20.3-dev, STOWHOLD lines): (a) the path-driven stow hold is on; (b) the shown weapon stays on the carried hand
    (wd <= 0.005 m) while pathF > lf + 0.01; (c) at the mount (pathF <= 0.02) wd is within 0.02 m of dm. (b) and (c) judge hold=1 lines only,
    so a hold=0 window (the old time-window stow) is NO-DATA for them, never FAIL."""
    rows = main.get("stowhold", [])
    held = [r for r in rows if r["hold"] == 1]
    if not rows:
        a = ("hold on", "NO-DATA", "no STOWHOLD line (old build, or no stow in window)")
    elif not held:
        a = ("hold on", "NO-DATA", "hold=0 on all %d line(s): hold off or old build" % len(rows))
    else:
        a = ("hold on", "PASS", "hold=1 on %d of %d line(s)" % (len(held), len(rows)))
    label_b = "weapon stays on the hand until the hand is near the mount"
    judged = [r for r in held if r["pathF"] > r["lf"] + STOW_JUDGE_MARGIN + STOW_EPS]
    if not judged:
        b = (label_b, "NO-DATA", "no hold=1 line with pathF > lf+%.2f" % STOW_JUDGE_MARGIN)
    else:
        worst = max(judged, key=lambda r: r["wd"])
        over = sum(1 for r in judged if r["wd"] > STOW_WD_MAX + STOW_EPS)
        if over:
            b = (label_b, "FAIL", "wd max %.3f m at pathF %.3f (%d of %d judged line(s) above %.3f m)"
                 % (worst["wd"], worst["pathF"], over, len(judged), STOW_WD_MAX))
        else:
            b = (label_b, "PASS", "wd <= %.3f m on all %d judged line(s) (max wd %.3f m at pathF %.3f)"
                 % (STOW_WD_MAX, len(judged), worst["wd"], worst["pathF"]))
    label_c = "lands on the mount"
    near = [r for r in held if r["pathF"] <= STOW_MOUNT_F + STOW_EPS]
    if not near:
        c = (label_c, "NO-DATA", "no hold=1 line with pathF <= %.2f" % STOW_MOUNT_F)
    else:
        worst = max(near, key=lambda r: abs(r["wd"] - r["dm"]))
        off = sum(1 for r in near if abs(r["wd"] - r["dm"]) > STOW_LAND_TOL + STOW_EPS)
        if off:
            c = (label_c, "FAIL", "|wd-dm| max %.3f m at pathF %.3f (wd %.3f dm %.3f; %d of %d line(s) above %.3f m)"
                 % (abs(worst["wd"] - worst["dm"]), worst["pathF"], worst["wd"], worst["dm"], off, len(near), STOW_LAND_TOL))
        else:
            c = (label_c, "PASS", "|wd-dm| <= %.3f m on all %d line(s) with pathF <= %.2f (max %.3f m)"
                 % (STOW_LAND_TOL, len(near), STOW_MOUNT_F, abs(worst["wd"] - worst["dm"])))
    return [a, b, c]


def _row_report(row, recs, main):
    """(exit code, output lines) for one --row: one line per clause, then the verdict (FAIL > NO-DATA > PASS)."""
    if row == "SM-L11":
        clauses = _sm_l11(main)
    elif row == "SM-L12":
        clauses = _sm_l12(main)
    elif row == "SM-L13":
        clauses = _sm_l13(recs)
    elif row == "SM-L14":
        clauses = _sm_l14(main)
    elif row == "SM-L15":
        clauses = _sm_l15(main)
    else:
        clauses = _sm_l16(main)
    lines = ["%s %s: %s %s" % (row, label, status, reason) for label, status, reason in clauses]
    statuses = {status for _, status, _ in clauses}
    verdict = "FAIL" if "FAIL" in statuses else "NO-DATA" if "NO-DATA" in statuses else "PASS"
    lines.append("%s verdict: %s" % (row, verdict))
    return ROW_EXIT[verdict], lines


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
    exs = f.get("reachex", [])
    if exs:   # HW31 (0.18.0-dev): ex= extract bump (0..1, 0 = no lift), of= orientation progress; extracting = frames with a bump above 0.05
        line += " ex max=%.2f extracting=%d of max=%.2f" % (max(x[0] for x in exs), sum(1 for x in exs if x[0] > 0.05), max(x[1] for x in exs))
    cfg = f.get("slidecfg", [])
    if cfg:
        line += " slide=%s resid_ms=%d wrist_max=%d" % cfg[-1]
    if f.get("reach_partial", 0):   # HW40: REACH steps the window cut (a piece missing, or a continuation whose piece 1 is before the window); shown only when > 0
        line += " partial=%d" % f["reach_partial"]
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
    out += _slide_dly_lines(main_facts)   # HW38: each of these is [] unless its field is in the window
    out += _toggle_events(main_facts)
    out += _reach_events(main_facts)
    out += _reach_hw38_lines(main_facts)
    out += _stowhold_lines(main_facts)   # HW41: [] unless the window has a STOWHOLD line
    out.append(_hist("ANIMRES", recs))
    styles = Counter((r["self"], r["style"], r["gate"]) for r in recs if r["kind"] == "ANIMRES")
    if styles:   # C6: +0xC5 = wield style byte, +0xC6 = table gate; one entry per creature and value
        out[-1] += " | style per creature (self style/gate): " + ", ".join("%s %d/%d x%d" % (k[0], k[1], k[2], v)
                                                                            for k, v in styles.most_common(8))
    out[-1] += _animstyle(main_facts, recs)
    out.append(_hist("ANIMSEND", recs))
    has_mount = any(r["kind"] == "MOUNT" for r in recs)
    gob_cap = CAPS["gobs_with_mount"] if has_mount else CAPS["gobs_with_place"] if has_place else CAPS["gobs"]
    out += _gobs(recs, gob_cap)
    if has_place:
        out += _place(recs)
    if has_mount:
        out += _mounts(recs, skipped)
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
    ap.add_argument("--mounts", action="store_true", help="print only the MOUNT (cat, bone) summary section")
    ap.add_argument("--json", action="store_true", help="with --mounts: the MOUNT groups as a JSON list, nothing else on stdout")
    ap.add_argument("--row", metavar="SM-Lnn", help="pass-signal verdict of one play-test row (SM-L11..SM-L16); exit 0 PASS, 1 FAIL, 3 NO-DATA")
    a = ap.parse_args(argv)
    if a.json and not a.mounts:
        print("--json needs --mounts", file=sys.stderr)
        return 2
    if a.row is not None:
        a.row = a.row.upper()
        if a.row not in ROWS:
            print("unknown row %s: expected one of %s" % (a.row, ", ".join(ROWS)), file=sys.stderr)
            return 2
        if a.mounts:
            print("--row cannot be combined with --mounts", file=sys.stderr)
            return 2
    path = Path(a.log or a.log_pos or DEFAULT_LOG)
    if not path.is_file():
        print("no log: %s" % path, file=sys.stderr if a.json else sys.stdout)   # --json: stdout stays pure JSON (or empty)
        return 2
    window, meta = read_window(path, a)
    recs, skipped, main_facts = parse_lines(window)
    if a.row is not None:
        rc, lines = _row_report(a.row, recs, main_facts)
        print("\n".join(lines))
        return rc
    if a.mounts and a.json:
        print(json.dumps(_mounts_json(recs)))
    elif a.mounts:
        print("\n".join(_mounts(recs, skipped)))
    else:
        print("\n".join(render(recs, skipped, main_facts, len(window), meta)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
