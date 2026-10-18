#!/usr/bin/env python3
"""enemy-effects-strip log checker (enemystrip route). Python 3, stdlib only; read-only (--new writes only its bookmark).

Usage: enemy_strip_log_check.py [LOG | --log LOG] [--new [--no-advance] [--state PATH]]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/enemy-effects-strip_log.txt
  Analyses every line of the file (all sessions). --new: only lines after the log_triage bookmark (same state file, keyed by
  absolute path), then advances it; --no-advance keeps the bookmark.
  Output: one block of about 30 lines. Every list shows at most 5 items then "...". Raw log lines are never echoed.
  Exit 0 = analyzed, 2 = log missing.

Grammar: the KP_LOGV strings of patch_manager_mods/enemy_effects_strip_src/EnemyStrip.cpp (other lines are not summarised):
  BUILD: slots=N/M pill=P                          INFO when built; any other BUILD: line is a failure
  strip: target=HEX tier=T slots=N [resref,...]    one line per strip change (the strip cap line is not counted)
  effects CHANGED sig=HEX count=N
  eff[i] type=T sub=S spell=SP dur=D/F creator=HEX creatorIsLeader=B ratio=R buff=I debuff=I state=ST
  leader skills base=HEX raw=[8 ints] awareness(idx3)=N (UNPROVEN index)   |   leader skills: awareness=? (...)
  WARN/ERR lines: counted by kind (quoted names, hex words and numbers masked)
The KpLog prefix and the fallback sink shape are parsed by scripts/kplog_parse.py parse_line; a bare body is accepted as-is.

ES-P0b facts: spell ids per (effect type, state), -1 (unreadable) excluded. Fear = type 8 state 8 (SetState Horrified, the
ip_fear icon). Creator = the u32 at the effect's creator offset, printed %08x (0 also when unreadable). Buckets, no magnitude
guess: none (0), invalid (0x7F000000 = OBJECT_INVALID), client-oid (bit 31 set), server-oid-or-ptr (bit 31 clear, any other
value; an object id or a low pointer, UNPROVEN which). leaderMatch = the count of creatorIsLeader=1, which the mod computes as
creator == the server-leader POINTER (EnemyStrip.cpp logEffects), so it cannot match an object-id creator.
Awareness: raw idx3 values (UNPROVEN index).
"""
from __future__ import annotations

import argparse
import os
import re
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kplog_parse as KP  # noqa: E402  (parse_line only; the prefix is not re-implemented here)
import log_triage as LT  # noqa: E402  (--new bookmark: same state file and helpers; log_triage is not modified)

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/enemy-effects-strip_log.txt"
PATCH = "enemy-effects-strip"
LIST_MAX = 5              # items shown per list, then "..."
STRIP_TAIL = 5            # last strip transitions shown
KIND_CUT = 80             # characters of a WARN kind shown
OBJECT_INVALID = 0x7F000000
CLIENT_BIT = 0x80000000   # client object ids carry bit 31 (server oid | 0x80000000)
FEAR_TYPE = 8             # EffType_SetState (EnemyStrip.cpp)
FEAR_STATE = 8            # Horrified = ip_fear icon (EnemyStripLogic.h)

RE_LOADED = re.compile(r"^enemy-effects-strip (\S+) loaded")
RE_BUILD = re.compile(r"^BUILD: slots=(\d+)/(\d+) pill=([01])$")
RE_STRIP = re.compile(r"^strip: target=([0-9a-fA-F]+) tier=(-?\d+) slots=(-?\d+) \[(.*)\]$")
RE_STRIP_CAP = re.compile(r"^strip: log cap reached")
RE_CHANGED = re.compile(r"^effects CHANGED sig=([0-9a-fA-F]+) count=(-?\d+)$")
RE_EFF = re.compile(r"^eff\[(\d+)\] type=(-?\d+) sub=(-?\d+) spell=(-?\d+) dur=(\S+) creator=([0-9a-fA-F]+) "
                    r"creatorIsLeader=([01]) ratio=(\S+) buff=(\S+) debuff=(\S+) state=(\S+)$")
RE_AW = re.compile(r"^leader skills base=([0-9a-fA-F]+) raw=\[([^\]]*)\] awareness\(idx3\)=(-?\d+)")
RE_AW_UNK = re.compile(r"^leader skills: awareness=\?")
_MASKS = (
    (re.compile(r"'[^']*'"), "'_'"),
    (re.compile(r"\b[0-9a-fA-F]{8}\b"), "HEX"),
    (re.compile(r"-?\b\d+\b"), "#"),
)


def _cap(items, sep=", "):
    """First LIST_MAX items joined by sep, then '...' when there are more."""
    items = list(items)
    return sep.join(str(x) for x in items[:LIST_MAX]) + (sep + "..." if len(items) > LIST_MAX else "")


def _ms(t):
    return "-" if t is None else "%d" % t


def warn_kind(msg):
    k = msg
    for rx, rep in _MASKS:
        k = rx.sub(rep, k)
    return k if len(k) <= KIND_CUT else k[:KIND_CUT - 3] + "..."


def classify(raw):
    """-> dict(kind, msg, level, t, truncated). kind: 'session' (kplog session header), 'header', 'body' or 'skip'.
    A bare line (no KpLog or fallback prefix) is a body with level INFO."""
    p = KP.parse_line(raw)
    if p is None:
        s = raw.strip()
        return dict(kind="body" if s else "skip", msg=s, level="INFO", t=None, truncated=s.endswith("[...]"))
    if p["kind"] == "header":
        return dict(kind="session" if p.get("session") else "header", msg=p["msg"], level=None, t=None, truncated=False)
    if p["kind"] in ("line", "fallback"):
        return dict(kind="body", msg=p["msg"].strip(), level=p["level"], t=p["t_ms"], truncated=p["truncated"])
    return dict(kind="skip", msg="", level=None, t=None, truncated=False)


def creator_bucket(creator):
    """Summary key for a nonzero creator value (0 = none is not bucketed)."""
    if creator == OBJECT_INVALID:
        return "creator_invalid"
    if creator & CLIENT_BIT:
        return "creator_client"
    return "creator_server"


def analyze(lines):
    """lines: [(line_no, raw)] in log order -> summary dict. Pure; render_text only formats it."""
    s = {"lines": 0, "sessions": 0, "version": None, "truncated": 0, "builds": [], "strips": [], "strip_cap": None,
         "changed": 0, "eff_lines": 0, "spells": {}, "creator_nonzero": 0, "creator_invalid": 0, "creator_client": 0, "creator_server": 0,
         "creator_leader": 0, "aw_vals": Counter(), "aw_unknown": 0, "warn_kinds": Counter(), "warn_total": 0}
    for n, raw in lines:
        s["lines"] += 1
        c = classify(raw)
        if c["kind"] == "skip":
            continue
        if c["kind"] == "session":
            s["sessions"] += 1
            continue
        if c["kind"] == "header":
            continue
        msg, level = c["msg"], c["level"]
        s["truncated"] += c["truncated"]
        if msg.startswith("BUILD:"):
            s["builds"].append((n, level, msg))
        if level in ("WARN", "ERR"):
            s["warn_total"] += 1
            s["warn_kinds"][warn_kind(msg)] += 1
            continue
        m = RE_LOADED.match(msg)
        if m:
            s["version"] = s["version"] or m.group(1)
            continue
        m = RE_STRIP.match(msg)
        if m:
            resrefs = [x for x in m.group(4).split(",") if x]
            s["strips"].append(dict(line=n, t=c["t"], tier=int(m.group(2)), slots=int(m.group(3)), resrefs=resrefs))
            continue
        if RE_STRIP_CAP.match(msg):
            s["strip_cap"] = s["strip_cap"] or n
            continue
        if RE_CHANGED.match(msg):
            s["changed"] += 1
            continue
        m = RE_EFF.match(msg)
        if m:
            s["eff_lines"] += 1
            spell = int(m.group(4))
            if spell != -1:
                s["spells"].setdefault((int(m.group(2)), m.group(11)), set()).add(spell)
            creator = int(m.group(6), 16)
            if creator:
                s["creator_nonzero"] += 1
                s[creator_bucket(creator)] += 1
            if m.group(7) == "1":
                s["creator_leader"] += 1
            continue
        m = RE_AW.match(msg)
        if m:
            s["aw_vals"][int(m.group(3))] += 1
        elif RE_AW_UNK.match(msg):
            s["aw_unknown"] += 1
    return s


def _build_text(builds):
    if not builds:
        return "BUILD not seen"
    n, level, msg = builds[-1]
    m = RE_BUILD.match(msg)
    if m and level != "WARN" and m.group(1) == m.group(2) and m.group(3) == "1":
        return "BUILD ok slots=%s/%s pill=1 (L%d, builds=%d)" % (m.group(1), m.group(2), n, len(builds))
    if msg.startswith("BUILD: disabled"):
        return "BUILD off (L%d: %s)" % (n, msg[:80])
    return "BUILD FAIL (L%d: %s, builds=%d)" % (n, msg[:80], len(builds))


def _strip_text(st):
    return "L%d t=%s tier=%d slots=%d [%s]" % (st["line"], _ms(st["t"]), st["tier"], st["slots"], _cap(st["resrefs"], ","))


def render_text(s, note=""):
    strips = s["strips"]
    out = ["ENEMY-STRIP lines=%d sessions=%d version=%s truncated=%d%s"
           % (s["lines"], s["sessions"], s["version"] or "-", s["truncated"], note)]
    out.append(_build_text(s["builds"]))
    out.append("STRIP transitions=%d cap=%s" % (len(strips), "no" if s["strip_cap"] is None else "hit at L%d" % s["strip_cap"]))
    out += ["  " + _strip_text(st) for st in strips[-STRIP_TAIL:]]
    tiers = sorted({st["tier"] for st in strips})
    out.append("tiers seen: " + (_cap(tiers) if tiers else "none"))
    resrefs = sorted({r for st in strips for r in st["resrefs"]})
    out.append("resrefs seen (%d): %s" % (len(resrefs), _cap(resrefs) if resrefs else "none"))
    out.append("CHANGED %d, eff lines %d" % (s["changed"], s["eff_lines"]))
    out.append("WARN %d" % s["warn_total"])
    kinds = s["warn_kinds"].most_common()
    out += ["  %dx %s" % (c, k) for k, c in kinds[:LIST_MAX]]
    if len(kinds) > LIST_MAX:
        out.append("  ...")
    out.append("ES-P0b facts")
    fear = s["spells"].get((FEAR_TYPE, str(FEAR_STATE)))
    out.append("  FEAR (type %d state %d): %s" % (FEAR_TYPE, FEAR_STATE,
                                                 "spells=[%s]" % _cap(sorted(fear)) if fear else "not seen"))
    others = sorted(k for k in s["spells"] if k != (FEAR_TYPE, str(FEAR_STATE)))
    pairs = ["type=%d state=%s spells=[%s]" % (t, st, _cap(sorted(s["spells"][(t, st)]))) for t, st in others]
    out += ["  " + p for p in pairs[:LIST_MAX]]
    if len(pairs) > LIST_MAX:
        out.append("  ...")
    out.append("  creator: nonzero=%d/%d invalid=%d client-oid=%d server-oid-or-ptr=%d leaderMatch=%d"
               % (s["creator_nonzero"], s["eff_lines"], s["creator_invalid"], s["creator_client"], s["creator_server"],
                  s["creator_leader"]))
    vals = sorted(s["aw_vals"])
    out.append("  awareness idx3 (UNPROVEN index): values=[%s] reads=%d unreadable=%d"
               % (_cap(vals) if vals else "none", sum(s["aw_vals"].values()), s["aw_unknown"]))
    return out


def read_window(path, a):
    """([(line_no, raw)], note). --new starts after the log_triage bookmark (line 1 when rotated) and advances it unless
    --no-advance. Line numbers stay absolute. Same bookmark rules as npc_div_log_check.read_window."""
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
    window, note = read_window(path, a)
    if a.new and not window:
        print("ENEMY-STRIP new=none (no lines since last check)")
        return 0
    print("\n".join(render_text(analyze(window), note)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
