#!/usr/bin/env python3
"""Companion-selfheal probe log checker (doc 27, CS3). Python 3.12, stdlib only; read-only.

Usage: selfheal_log_check.py [LOG | --log LOG] [--from LINE] [--to LINE]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/companion-selfheal_log.txt
  1-based line numbers; verdicts use only lines in range. The whole file is one session (the probe logs no banner).
  Exit 0 = PASS/INCOMPLETE, 1 = FAIL, 2 = log missing/unreadable.

Line formats come from patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp; KpLog.h prefixes each line with
"<ms> companion-selfheal/<chan> [ERR |WARN ]" (prefix optional here so bare lines parse too).
Checks: P1 (>=1 `P1 hud built ... layoutOpen=1`, ctlCount 1..4096), P3 (every `P3 slot[i]=` has k=0|1|2),
DUMP blocks (hp cur<=max, cand row in {64,65,66,84,127,128} and heal>0, a pick line, routeA idOk=1 when a pick exists).
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/companion-selfheal_log.txt"

PREFIX_RE = re.compile(r"^\d+ [\w.-]+/\w+ (?:(?:ERR|WARN) )?(.*)$")
P1_RE = re.compile(r"^P1 hud built .*?\blayoutOpen=(\S+) .*?\bctlCount=(\S+)\s*$")
P3_RE = re.compile(r"^P3 slot\[(\d+)\]=\S+ .*?\bk=(\S+)\s*$")
MEMBER_RE = re.compile(r"^member (\d+) .*?\bhp=(\S+?)/(\S+?) ")
CAND_RE = re.compile(r"^\s+cand \"(.*)\" .*?\brow=(\S+) heal=(\S+) count=(\S+)")
PICK_RE = re.compile(r"^\s+pick missing=(\S+) -> (?:\"(.*)\" row=(\S+) heal=(\S+) count=(\S+)|none)")
ROUTE_RE = re.compile(r"^\s+routeA \(not called\):.*\bidOk=(\S+)")
ROWS = {64, 65, 66, 84, 127, 128}
FAMILY = {64: "organic medpac(10+1*TI)", 65: "organic medpac(20+2*TI)", 66: "organic medpac(30+3*TI)",
          84: "droid repair(15+1*Rep)", 127: "droid repair(25+2*Rep)", 128: "droid repair(35+3*Rep)"}


def to_int(s):
    try:
        return int(s)
    except (TypeError, ValueError):
        return None


def strip(raw):
    m = PREFIX_RE.match(raw)
    return m.group(1) if m else raw


def check(lines, lo=1, hi=None):
    r = {"p1": "MISSING", "p1_fail": [], "p1_ok": 0, "p3": "PASS", "p3_fail": [], "p3_n": 0,
         "blocks": [], "dump_fail": [], "table": []}
    cur = None  # current DUMP block
    mem = None  # current member dict in the block

    def close_member():
        nonlocal mem
        if mem is None:
            return
        if mem["cands"] and not mem["pick_line"]:
            r["dump_fail"].append((mem["line"], "member %s: candidates but no pick line" % mem["idx"]))
        if mem["pick"] and mem["idOk"] != 1:
            r["dump_fail"].append((mem["pickline"], "member %s: pick but routeA idOk=%s" % (mem["idx"], mem["idOk"])))
        r["table"].append(mem)
        mem = None

    for n, raw in enumerate(lines, 1):
        if n < lo or (hi is not None and n > hi):
            continue
        t = strip(raw.rstrip("\r\n"))
        m = P1_RE.match(t)
        if m:
            ctl = to_int(m.group(2))
            if r["p1"] == "MISSING":
                r["p1"] = "FAIL"
            if ctl is None or not (1 <= ctl <= 4096):
                r["p1_fail"].append((n, "ctlCount=%s not in 1..4096" % m.group(2)))
            elif m.group(1) == "1":
                r["p1_ok"] += 1
            else:
                r["p1_fail"].append((n, "layoutOpen=%s" % m.group(1)))
            continue
        m = P3_RE.match(t)
        if m:
            r["p3_n"] += 1
            if m.group(2) not in ("0", "1", "2"):
                r["p3_fail"].append((n, "slot[%s] k=%s" % (m.group(1), m.group(2))))
            continue
        if t.startswith("DUMP begin"):
            if cur is not None:
                close_member()
                r["dump_fail"].append((cur["line"], "DUMP begin without DUMP end"))
            cur = {"line": n, "ended": False}
            r["blocks"].append(cur)
            continue
        if t.startswith("DUMP end"):
            close_member()
            if cur is not None:
                cur["ended"] = True
            cur = None
            continue
        if cur is None:
            continue
        m = MEMBER_RE.match(t)
        if m:
            close_member()
            c, mx = to_int(m.group(2)), to_int(m.group(3))
            mem = {"block": cur["line"], "idx": m.group(1), "line": n, "cur": c, "max": mx, "cands": [],
                   "pick_line": False, "pick": None, "pickline": None, "idOk": None}
            if c is None or mx is None:
                r["dump_fail"].append((n, "member %s: malformed hp=%s/%s" % (m.group(1), m.group(2), m.group(3))))
            elif c > mx:
                r["dump_fail"].append((n, "member %s: hp cur %d > max %d" % (m.group(1), c, mx)))
            continue
        if mem is None:
            continue
        m = CAND_RE.match(t)
        if m:
            row, heal, cnt = to_int(m.group(2)), to_int(m.group(3)), to_int(m.group(4))
            mem["cands"].append((n, m.group(1), row, heal, cnt))
            if row not in ROWS:
                r["dump_fail"].append((n, "cand \"%s\": row=%s not in %s" % (m.group(1), m.group(2), sorted(ROWS))))
            if heal is None or heal <= 0:
                r["dump_fail"].append((n, "cand \"%s\": heal=%s not > 0" % (m.group(1), m.group(3))))
            continue
        m = PICK_RE.match(t)
        if m:
            mem["pick_line"] = True
            if m.group(2) is not None:
                mem["pick"] = (m.group(2), to_int(m.group(3)), to_int(m.group(4)), to_int(m.group(5)))
                mem["pickline"] = n
            continue
        m = ROUTE_RE.match(t)
        if m:
            mem["idOk"] = to_int(m.group(1))
    close_member()
    if cur is not None:
        r["dump_fail"].append((cur["line"], "DUMP begin without DUMP end"))
    if r["p1_ok"] and not r["p1_fail"]:
        r["p1"] = "PASS"
    if r["p1_fail"] and r["p1_ok"] == 0:
        r["p1"] = "FAIL"
    elif r["p1_fail"]:
        r["p1"] = "FAIL"
    if r["p3_fail"]:
        r["p3"] = "FAIL"
    bad = (r["p1"] == "FAIL" or r["p3"] == "FAIL" or bool(r["dump_fail"])
           or (r["p1"] == "MISSING" and (r["blocks"] or r["p3_n"])))
    r["verdict"] = "FAIL" if bad else ("PASS" if r["blocks"] else "INCOMPLETE")
    return r


def fmt(r):
    o = []
    if r["table"]:
        o.append("idx  hp        family            pick                         heal count (block@line)")
        for m in r["table"]:
            fams = sorted({FAMILY.get(c[2], "?") for c in m["cands"]})
            hp = "%s/%s" % (m["cur"], m["max"])
            p = m["pick"]
            o.append("%-4s %-9s %-17s %-28s %-4s %-5s (@%d)" % (
                m["idx"], hp, ",".join(f.split("(")[0] for f in fams) or "-",
                ('"%s" row=%s' % (p[0], p[1])) if p else "none", p[2] if p else "-", p[3] if p else "-", m["block"]))
    for tag, key in (("P1", "p1_fail"), ("P3", "p3_fail"), ("DUMP", "dump_fail")):
        for n, msg in r[key]:
            o.append("FAIL %s line %d: %s" % (tag, n, msg))
    o.append("SELFHEAL P1=%s P3=%s DUMP=%d blocks VERDICT=%s" % (r["p1"], r["p3"], len(r["blocks"]), r["verdict"]))
    return "\n".join(o)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("log_pos", nargs="?")
    ap.add_argument("--log")
    ap.add_argument("--from", dest="lo", type=int, default=1)
    ap.add_argument("--to", dest="hi", type=int, default=None)
    a = ap.parse_args(argv)
    path = Path(a.log or a.log_pos or DEFAULT_LOG)
    try:
        lines = path.read_text(errors="replace").splitlines()
    except OSError:
        print("no log: %s" % path)
        return 2
    r = check(lines, a.lo, a.hi)
    print(fmt(r))
    return 1 if r["verdict"] == "FAIL" else 0


if __name__ == "__main__":
    sys.exit(main())
