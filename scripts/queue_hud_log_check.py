#!/usr/bin/env python3
"""Queue-HUD trace log checker (queue-hud route). Python 3.12, stdlib only; read-only (--new writes only its bookmark).

Usage: queue_hud_log_check.py [LOG | --log LOG] [--cmd N] [--member K] [--new [--no-advance] [--state PATH]]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/queue-hud_log.txt
  Default view (under 60 lines): per-command lifecycle, decline histogram, DROPs, EDGE timeline, suspicious flags.
  --cmd N: one command's lines only (max 30 lines). --member K: only lines with m=K (EDGE has no m, so it is dropped).
  --new: only lines after the bookmark shared with log_triage (same state file, keyed by absolute path), then advance it.
  Exit 0 = analyzed (flags are reported, not failures), 2 = log missing.

Grammar: the INFO text after the KpLog "T=<ms> f=<frame|-> <patch>/<channel> " stamp (a bare line parses too). Golden
strings: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp (testLogFormatGolden). Lines whose first word is none of
these keywords are ignored; a keyword line with a missing or malformed field is counted in SKIPPED, never a crash.
  CMD#<n> m=<k|?> cr=<hex> kind=item|spell|? id=<hex|-> tgt=<hex|-> dec=<dec> fb=<0|1> engq=<n|unreadable> buf=<n> paused=<0|1> combat=<0|1>
  WAIT cmd=<n> m=<k> why=<reason> buf=<n> engNe=<n> age=<ms>
  RESOLVE cmd=<n> m=<k|?> seen=<0|1> reinj=<n> age=<ms>
  DROP cmd=<n> m=<k|?> why=<reason> n=<count>
  EDGE what=<combat|paused|menu|party_table|gather_m<k>|creature|journal_cap> old=<0|1> new=<0|1> src=<hud|unreadable> buf=<n,n,...|empty>
    (journal_cap is a marker: its old/new carry no meaning and the timeline shows it as a marker)
  QJ m=<k> ne=<n> types=[<hex,...,+N>] round=<n> cur=<hex|-> aa8=<n> ab8=<n> paused=<0|1> combat=<0|1> ours=<tag>@<age>ms|external
  REINJECT|DISPATCH|CANCEL cmd=<n> [m=<k>] [why=<reason>] [attempt=<n>]   (REINJECT/DISPATCH: a why other than ok is a
    decline, except REINJECT why=eaten_paused, which is the re-inject reason and shows as REINJECT#<attempt>)
  REINJECT_NO cmd=<n> m=<k> why=<ok|inactive|pending|cancel|max_reinject|in_queue|no_leader>   (a why other than ok is a decline)
  VANISH cmd=<n> m=<k> kind=<executed|unseen|eaten_paused|eaten_wait> paused=<0|1> exec=<0|1> ne=<n> round=<n>
    cur=<hex|-> aa8=<n> ab8=<n> age=<ms>
  HANDOFF m=<k> active=<n|-> buffered=<n> cmds=<n,n,...|->
  HANDOFF_ONE cmd=<n> m=<k> seq=<i>/<n>
  STALE_CUR m=<k> sub=<n> since=<n> aa8=<n> ab8=<n> paused=<0|1>
  IDENT_SWAP from=<n> to=<n>   (no m=: dropped by --member)

Flags:
  A follower-wiped (QJ): 0x3D in one member's shown types=, absent from that member's next QJ (only when that QJ is not
    +N truncated), and that next QJ's ours is not cancel. Types compare by hex value (3d == 3D).
  B orphan-round (QJ): the first QJ after an ours=cancel QJ has round >= that cancel QJ's round (cancel round > 0).
  C unseen-reinject (RESOLVE): seen=0 with reinj>0.
  D eaten-loop (VANISH): one command (lifecycle) has >= 4 VANISH kind=eaten_paused. Anchored at the 4th.
  E stale-wait (WAIT why=cur_busy): age >= 3000 for a member with no STALE_CUR line in the window.
  F handoff-short (HANDOFF): buffered=n > 0 but fewer than n HANDOFF_ONE for that member before its next HANDOFF.
  G double-exec (VANISH kind=executed or RESOLVE seen=1): one command (lifecycle) has two of either.
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
import log_triage as LT  # noqa: E402  (--new bookmark: same state file and helpers; log_triage is not modified)

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/queue-hud_log.txt"
FOLLOWER_TYPE = 0x3D
CAPS = {"cmd": 18, "drop": 5, "edge": 8, "handoff": 2, "stale": 2, "flag": 8}
# default view worst case = 60 lines: header 1 + SKIPPED 1 + DECLINES 1 + COMMANDS 20 + DROPS 7 + EDGES 10
#   + HANDOFFS 4 + VANISHES 2 + STALE_CUR 4 + FLAGS 10
CMD_VIEW_MAX = 30
CMD_KWS = ("CMD", "WAIT", "RESOLVE", "DROP", "REINJECT", "REINJECT_NO", "DISPATCH", "CANCEL", "VANISH")
VANISH_KINDS = ("executed", "unseen", "eaten_paused", "eaten_wait")
EATEN_PAUSED = "eaten_paused"   # REINJECT why= that is the re-inject reason (not a decline)
PREFIX_RE = re.compile(r"^T=\S+\s+f=\S+\s+[\w.-]+/\w+\s+(?:(?:ERR|WARN)\s+)?(.*)$")
HEXV = r"[0-9a-fA-F]+"
# Field value grammar, matched with fullmatch. Keys not listed here are not validated.
FIELD_RE = {
    "m": r"-?\d+|\?", "cmd": r"\d+", "cr": HEXV, "kind": r"item|spell|\?", "id": r"-|" + HEXV, "tgt": r"-|" + HEXV,
    "dec": r"\w+", "fb": r"[01]", "engq": r"-?\d+|unreadable", "buf": r"-?\d+", "paused": r"[01]", "combat": r"[01]",
    "why": r"\w+", "engNe": r"-?\d+", "age": r"\d+", "seen": r"[01]", "reinj": r"-?\d+", "n": r"-?\d+",
    "what": r"combat|paused|menu|party_table|gather_m\d+|creature|journal_cap", "old": r"[01]", "new": r"[01]",
    "src": r"\w+", "buflist": r"(?:-?\d+(?:,-?\d+)*)?",
    "ne": r"-?\d+", "types": r"\[(?:" + HEXV + r"(?:," + HEXV + r")*(?:,\+\d+)?|\+\d+)?\]",
    "round": r"-?\d+", "cur": r"-|" + HEXV, "aa8": r"-?\d+", "ab8": r"-?\d+",
    "ours": r"external|[^@\s]+(?:@\d+ms)?",
    "exec": r"[01]", "attempt": r"\d+", "vanish_kind": "|".join(VANISH_KINDS),
    "active": r"-|\d+", "buffered": r"\d+", "cmds": r"-|\d+(?:,\d+)*", "seq": r"\d+/\d+",
    "sub": r"-?\d+", "since": r"-?\d+", "from": r"-?\d+", "to": r"-?\d+",
}
OVERRIDE = {("EDGE", "buf"): "buflist", ("VANISH", "kind"): "vanish_kind"}
REQUIRED = {
    "CMD": ("m", "cr", "kind", "id", "tgt", "dec", "fb", "engq", "buf", "paused", "combat"),
    "WAIT": ("cmd", "m", "why", "buf", "engNe", "age"),
    "RESOLVE": ("cmd", "m", "seen", "reinj", "age"),
    "DROP": ("cmd", "m", "why", "n"),
    "EDGE": ("what", "old", "new", "src", "buf"),
    "QJ": ("m", "ne", "types", "round", "cur", "aa8", "ab8", "paused", "combat", "ours"),
    "REINJECT": ("cmd",), "DISPATCH": ("cmd",), "CANCEL": ("cmd",),
    "REINJECT_NO": ("cmd", "m", "why"),
    "VANISH": ("cmd", "m", "kind", "paused", "exec", "ne", "round", "cur", "aa8", "ab8", "age"),
    "HANDOFF": ("m", "active", "buffered", "cmds"),
    "HANDOFF_ONE": ("cmd", "m", "seq"),
    "STALE_CUR": ("m", "sub", "since", "aa8", "ab8", "paused"),
    "IDENT_SWAP": ("from", "to"),
}


class Bad(Exception):
    def __init__(self, label, reason):
        super().__init__(reason)
        self.label, self.reason = label, reason


def _num(s):
    return int(s) if s is not None and re.fullmatch(r"-?\d+", s) else None


def _ms(m):
    return "?" if m is None else str(m)


# QueueHud.cpp keeps its existing free-form re-inject / dispatch lines and appends cmd=<n>. Rewrite them to the grammar keyword.
_REINJECT_OLD = re.compile(r"re-injecting eaten \S+ action for member (\d+) .*\bcmd=(\d+)(?: why=(\w+))?$")
_DISPATCH_OLD = re.compile(r"dispatching buffered \S+ for member (\d+) .*\bcmd=(\d+)$")


def normalize_old(body):
    m = _REINJECT_OLD.match(body)
    if m:
        out = "REINJECT cmd=%s m=%s" % (m.group(2), m.group(1))
        if m.group(3):   # optional trailing why=eaten_paused (newer build)
            out += " why=%s" % m.group(3)
        att = re.search(r"\battempt=(\d+)", body)   # the free-form text carries attempt=<n> inside its parentheses
        return out + (" attempt=%s" % att.group(1) if att else "")
    m = _DISPATCH_OLD.match(body)
    if m:
        return "DISPATCH cmd=%s m=%s" % (m.group(2), m.group(1))
    return body


def parse_body(body):
    """(kw, cmd, fields) for a queue-hud line; None when the first word is no queue-hud keyword; Bad when malformed."""
    toks = normalize_old(body).split()
    if not toks:
        return None
    head, cmd = toks[0], None
    if head.startswith("CMD#"):
        if not re.fullmatch(r"CMD#\d+", head):
            raise Bad(head, "bad command number")
        kw, cmd = "CMD", int(head[4:])
    elif head in REQUIRED and head != "CMD":
        kw = head
    else:
        return None
    fields = {}
    for t in toks[1:]:
        k, eq, v = t.partition("=")
        if not eq or not k:
            raise Bad(head, "bare token %r" % t)
        if k in fields:
            raise Bad(head, "duplicate %s" % k)
        fields[k] = v
    for k in REQUIRED[kw]:
        if k not in fields:
            raise Bad(head, "missing %s" % k)
    for k, v in fields.items():
        pat = FIELD_RE.get(OVERRIDE.get((kw, k), k))
        if pat is not None and not re.fullmatch(pat, v):
            raise Bad(head, "malformed %s=%s" % (k, v))
    return kw, cmd, fields


def parse_lines(window):
    """window: [(line_no, raw)] -> (records in log order, skipped [(line_no, label, reason)])."""
    recs, skipped = [], []
    for n, raw in window:
        m = PREFIX_RE.match(raw)
        body = (m.group(1) if m else raw).strip()
        try:
            got = parse_body(body)
        except Bad as e:
            skipped.append((n, e.label, e.reason))
            continue
        if got is not None:
            kw, cmd, fields = got
            recs.append({"line": n, "body": body, "kw": kw, "cmd": cmd if cmd is not None else _num(fields.get("cmd")),
                         "m": _num(fields.get("m")), "f": fields})
    return recs, skipped


def qj_types(rec):
    """(shown type values as ints, +N tail count or None). Hex is parsed, so comparisons are by value."""
    raw = rec["f"]["types"][1:-1]
    shown, tail = [], None
    for item in raw.split(",") if raw else []:
        if item.startswith("+"):
            tail = int(item[1:])
        else:
            shown.append(int(item, 16))
    return shown, tail


def qj_ours(rec):
    """(tag, age_ms or None) from ours=<tag>@<age>ms | external."""
    v = rec["f"]["ours"]
    if v == "external":
        return "external", None
    tag, _, age = v.partition("@")
    return tag, (int(age[:-2]) if age.endswith("ms") and age[:-2].isdigit() else None)


def qj_flags(recs):
    """[(line, text)] for flags A and B (QJ, per member m=) and C (RESOLVE)."""
    out, last, pending = [], {}, {}
    for r in recs:
        f = r["f"]
        if r["kw"] == "RESOLVE":
            if _num(f["seen"]) == 0 and (_num(f["reinj"]) or 0) > 0:
                out.append((r["line"], "C L%d cmd=%s m=%s unseen-reinject seen=0 reinj=%s" % (
                    r["line"], r["cmd"], _ms(r["m"]), f["reinj"])))
            continue
        if r["kw"] != "QJ":
            continue
        shown, tail = qj_types(r)
        tag, _age = qj_ours(r)
        prev = last.get(r["m"])
        if (prev is not None and FOLLOWER_TYPE in qj_types(prev)[0] and FOLLOWER_TYPE not in shown
                and tail is None and tag != "cancel"):
            out.append((r["line"], "A L%d m=%s follower 0x3d wiped (was L%d, ours=%s)" % (
                r["line"], _ms(r["m"]), prev["line"], f["ours"])))
        cancel = pending.pop(r["m"], None)
        if cancel is not None:
            c_round, round_ = _num(cancel["f"]["round"]), _num(f["round"])
            if c_round > 0 and round_ >= c_round:
                out.append((r["line"], "B L%d m=%s orphan-round round %d -> %d after ours=cancel (L%d)" % (
                    r["line"], _ms(r["m"]), c_round, round_, cancel["line"])))
        if tag == "cancel":
            pending[r["m"]] = r
        last[r["m"]] = r
    return sorted(out)


def collapse(events):
    out = []
    for e in events:
        if out and out[-1][0] == e:
            out[-1][1] += 1
        else:
            out.append([e, 1])
    return [e if c == 1 else "%s x%d" % (e, c) for e, c in out]


def _reinject_reason(r):
    """REINJECT why=eaten_paused: the reason for a re-inject, not a decline."""
    return r["kw"] == "REINJECT" and r["f"].get("why") == EATEN_PAUSED


def build_cmds(recs):
    """Per-command lifecycles in log order. A new CMD# line for a number starts a new lifecycle (counter restarts).
    Each lifecycle also keeps what flags D and G need: vanish [(line, kind)] and executed [(line, label)]."""
    insts, open_ = [], {}
    for r in recs:
        if r["kw"] not in CMD_KWS or r["cmd"] is None:
            continue
        f, kw, why = r["f"], r["kw"], r["f"].get("why")
        if kw == "CMD" or r["cmd"] not in open_:
            open_[r["cmd"]] = {"cmd": r["cmd"], "m": None, "head": None, "events": [], "outcome": None,
                               "reinjects": 0, "vanish": [], "executed": []}
            insts.append(open_[r["cmd"]])
        inst = open_[r["cmd"]]
        if inst["m"] is None:
            inst["m"] = r["m"]
        if kw == "CMD":
            inst["head"] = "%s dec=%s" % (f["kind"], f["dec"])
        elif kw == "WAIT":
            inst["events"].append("wait:%s" % why)
        elif kw == "VANISH":
            inst["events"].append("VANISH:%s" % f["kind"])
            inst["vanish"].append((r["line"], f["kind"]))
            if f["kind"] == "executed":
                inst["executed"].append((r["line"], "VANISH kind=executed"))
        elif kw in ("REINJECT", "DISPATCH"):
            if kw == "REINJECT":
                inst["reinjects"] += 1
            if _reinject_reason(r):
                n = _num(f.get("attempt"))
                inst["events"].append("REINJECT#%d" % (inst["reinjects"] if n is None else n))
            elif why in (None, "ok"):
                inst["events"].append(kw.lower())
            else:
                inst["events"].append("%s-declined:%s" % (kw.lower(), why))
        elif kw == "REINJECT_NO":
            inst["events"].append("reinject-no:ok" if why == "ok" else "reinject-declined:%s" % why)
        elif kw == "CANCEL":
            inst["events"].append("cancel")
        elif kw == "RESOLVE":
            inst["outcome"] = "RESOLVE seen=%s reinj=%s age=%s" % (f["seen"], f["reinj"], f["age"])
            if f["seen"] == "1":
                inst["executed"].append((r["line"], "RESOLVE seen=1"))
        elif kw == "DROP":
            inst["outcome"] = "DROP why=%s n=%s" % (why, f["n"])
    return insts


def life_flags(insts):
    """[(line, text)] for flags D (eaten loop) and G (double exec), per lifecycle."""
    out = []
    for i in insts:
        m, cmd = _ms(i["m"]), i["cmd"]
        ep = [line for line, kind in i["vanish"] if kind == EATEN_PAUSED]
        if len(ep) >= 4:
            out.append((ep[3], "D L%d cmd=%s m=%s VANISH kind=eaten_paused x%d" % (ep[3], cmd, m, len(ep))))
        n_v = sum(1 for _, label in i["executed"] if label.startswith("VANISH"))
        n_r = len(i["executed"]) - n_v
        if n_v >= 2 or n_r >= 2:
            second = sorted(i["executed"])[1][0]
            out.append((second, "G L%d cmd=%s m=%s executed twice (VANISH kind=executed x%d, RESOLVE seen=1 x%d)" % (
                second, cmd, m, n_v, n_r)))
    return out


def handoff_windows(recs):
    """[(HANDOFF rec, [HANDOFF_ONE recs])]. A HANDOFF_ONE belongs to the latest HANDOFF of the same m (until the next one)."""
    out, open_ = [], {}
    for r in recs:
        if r["kw"] == "HANDOFF":
            entry = (r, [])
            out.append(entry)
            open_[r["m"]] = entry
        elif r["kw"] == "HANDOFF_ONE" and r["m"] in open_:
            open_[r["m"]][1].append(r)
    return out


def handoff_flags(recs):
    """[(line, text)] for flag F: HANDOFF buffered=n > 0 with fewer than n HANDOFF_ONE in its window."""
    out = []
    for h, ones in handoff_windows(recs):
        n = _num(h["f"]["buffered"])
        if n > 0 and len(ones) < n:
            out.append((h["line"], "F L%d m=%s HANDOFF buffered=%d but %d HANDOFF_ONE" % (h["line"], _ms(h["m"]), n, len(ones))))
    return out


def stale_flags(recs):
    """[(line, text)] for flag E: WAIT why=cur_busy with age >= 3000 for a member that has no STALE_CUR in the window."""
    stale_members = {r["m"] for r in recs if r["kw"] == "STALE_CUR"}
    out = []
    for r in recs:
        if r["kw"] != "WAIT" or r["f"]["why"] != "cur_busy" or r["m"] in stale_members:
            continue
        age = _num(r["f"]["age"])
        if age is not None and age >= 3000:
            out.append((r["line"], "E L%d m=%s WAIT why=cur_busy age=%d with no STALE_CUR for this member" % (
                r["line"], _ms(r["m"]), age)))
    return out


def all_flags(recs):
    """Every flag as sorted [(line, text)]: A, B, C (qj_flags) plus D, E, F, G."""
    return sorted(qj_flags(recs) + life_flags(build_cmds(recs)) + handoff_flags(recs) + stale_flags(recs))


def life_line(i):
    parts = ["cmd %s m=%s %s" % (i["cmd"], _ms(i["m"]), i["head"] or "(no CMD# line)")]
    parts += collapse(i["events"]) + [i["outcome"] or "no outcome"]
    return " -> ".join(parts)


def _edge_text(r):
    f = r["f"]
    if f["what"] == "journal_cap":
        return "L%d journal_cap marker src=%s buf=%s" % (r["line"], f["src"], f["buf"])
    return "L%d %s %s->%s src=%s buf=%s" % (r["line"], f["what"], f["old"], f["new"], f["src"], f["buf"])


def _section(title, items, cap, count=None):
    out = ["%s %d" % (title, len(items) if count is None else count)]
    if not items:
        return out + ["  (none)"]
    out += ["  " + s for s in items[:cap]]
    if len(items) > cap:
        out.append("  ... +%d more" % (len(items) - cap))
    return out


def _handoff_text(h, ones):
    f = h["f"]
    return "L%d m=%s active=%s buffered=%s cmds=%s -> HANDOFF_ONE x%d" % (
        h["line"], _ms(h["m"]), f["active"], f["buffered"], f["cmds"], len(ones))


def _stale_items(recs):
    """One item per member: 'm=<k> x<count> first L<line>', members in first-seen order."""
    groups = {}
    for r in recs:
        if r["kw"] == "STALE_CUR":
            groups.setdefault(r["m"], []).append(r["line"])
    return ["m=%s x%d first L%d" % (_ms(m), len(ls), ls[0]) for m, ls in groups.items()]


def render_default(recs, skipped, nlines, meta):
    insts = build_cmds(recs)
    declines = Counter()
    for r in recs:
        why = r["f"].get("why")
        if ((r["kw"] == "WAIT" and why != "ok") or (r["kw"] in ("REINJECT", "DISPATCH") and why not in (None, "ok")
                                                    and not _reinject_reason(r))
                or (r["kw"] == "REINJECT_NO" and why != "ok")):
            declines[why] += 1
    drops = [r for r in recs if r["kw"] == "DROP"]
    edges = [r for r in recs if r["kw"] == "EDGE"]
    vanishes = [r for r in recs if r["kw"] == "VANISH"]
    stale = [r for r in recs if r["kw"] == "STALE_CUR"]
    n_swap = sum(1 for r in recs if r["kw"] == "IDENT_SWAP")
    flags = all_flags(recs)
    n_qj = sum(1 for r in recs if r["kw"] == "QJ")
    out = ["QUEUE-HUD lines=%d parsed=%d skipped=%d cmds=%d qj=%d%s" % (nlines, len(recs), len(skipped), len(insts), n_qj, meta)]
    if skipped:
        shown = "; ".join("L%d %s %s" % s for s in skipped[:3])
        more = " ; ... +%d more" % (len(skipped) - 3) if len(skipped) > 3 else ""
        out.append("SKIPPED %d: %s%s" % (len(skipped), shown, more))
    top = " ".join("%s=%d" % kv for kv in declines.most_common(5)) or "none"
    out.append("DECLINES %d: %s" % (sum(declines.values()), top))
    out += _section("COMMANDS", [life_line(i) for i in insts], CAPS["cmd"])
    out += _section("DROPS", ["L%d cmd=%s m=%s %s n=%s" % (r["line"], r["cmd"], _ms(r["m"]), r["f"]["why"], r["f"]["n"])
                              for r in drops], CAPS["drop"])
    out += _section("EDGES", [_edge_text(r) for r in edges], CAPS["edge"])
    hand = handoff_windows(recs)
    hsec = _section("HANDOFFS", [_handoff_text(h, ones) for h, ones in hand], CAPS["handoff"])
    if n_swap:
        hsec[0] += " (IDENT_SWAP x%d)" % n_swap
    out += hsec
    hist = Counter(r["f"]["kind"] for r in vanishes)
    out += _section("VANISHES", [" ".join("%s=%d" % (k, hist[k]) for k in VANISH_KINDS)] if vanishes else [], 1,
                    count=len(vanishes))
    out += _section("STALE_CUR", _stale_items(stale), CAPS["stale"], count=len(stale))
    out += _section("FLAGS", [text for _, text in flags], CAPS["flag"])
    return out


def render_cmd(recs, n):
    insts = [i for i in build_cmds(recs) if i["cmd"] == n]
    rows = [r for r in recs if r["cmd"] == n and r["kw"] in CMD_KWS]
    lines = [life_line(i) for i in insts] + ["L%d %s" % (r["line"], r["body"]) for r in rows]
    if not lines:
        return ["cmd %d: no lines" % n]
    if len(lines) > CMD_VIEW_MAX:
        lines = lines[:CMD_VIEW_MAX - 1] + ["... +%d more" % (len(lines) - CMD_VIEW_MAX + 1)]
    return lines


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
    ap.add_argument("--cmd", type=int, help="one command's lines only (max 30 lines)")
    ap.add_argument("--member", type=int, help="only lines with m=K (EDGE has no m)")
    ap.add_argument("--new", action="store_true", help="only lines after the log_triage bookmark, then advance it")
    ap.add_argument("--no-advance", action="store_true", help="with --new: do not move the bookmark")
    ap.add_argument("--state", default=LT.DEFAULT_STATE, help="bookmark file for --new")
    a = ap.parse_args(argv)
    path = Path(a.log or a.log_pos or DEFAULT_LOG)
    if not path.is_file():
        print("no log: %s" % path)
        return 2
    window, meta = read_window(path, a)
    recs, skipped = parse_lines(window)
    if a.member is not None:
        recs = [r for r in recs if r["m"] == a.member]
        meta += " member=%d" % a.member
    lines = render_cmd(recs, a.cmd) if a.cmd is not None else render_default(recs, skipped, len(window), meta)
    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
