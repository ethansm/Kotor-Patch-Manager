#!/usr/bin/env python3
"""Companion-selfheal probe log checker (doc 27, CS3). Python 3.12, stdlib only; read-only.

Usage: selfheal_log_check.py [LOG | --log LOG] [--from LINE] [--to LINE]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/companion-selfheal_log.txt
  1-based line numbers; verdicts use only lines in range. The whole file is one session (the probe logs no banner).
  Exit 0 = PASS/INCOMPLETE, 1 = FAIL, 2 = log missing/unreadable.

Line formats come from patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp; KpLog.h prefixes each line with
"<ms> companion-selfheal/<chan> [ERR |WARN ]" (prefix optional here so bare lines parse too).
Checks (0.4.0-dev adds): BUILD n=N/D (FAIL if N<D without why=), ICON fallback and TEX giving up are WARN.
0.6.0-dev (CS12): REFUSE already queued, KICK member=N before=[..] after=[..] k=K route=rotate, KICK skipped member=N (gate=v),
KICK starved member=N, DONE ... queued=Q, TIMEOUT unqueued member=N. `KICK starved` and `TIMEOUT unqueued` are WARN (never FAIL): a timeout while
queued cannot happen by design, so an unqueued timeout means the use never entered the queue or was dropped.
Checks: P1 (>=1 `P1 hud built ... layoutOpen=1`, ctlCount 1..4096), P3 (every `P3 slot[i]=` has k=0|1|2),
DUMP blocks (hp cur<=max, heal-row cand {64,65,66,84,127,128} has heal>0 and only those have heal>0, a pick line, routeA idOk=1 when a pick exists).
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
BUILD_RE = re.compile(r"^BUILD n=(\S+)/(\d+)(?:\s+why=(.*))?$")   # 0.3.0: /10, 0.4.0-dev: /12 (+2 ring labels)
ICON_RE = re.compile(r"^ICON fallback member=(\S+)")
TEX_RE = re.compile(r"^TEX (\S+) (queued|refilled|giving up)\b")
USE_RE = re.compile(r"^USE member=(\S+) item=(\S+) \"(.*)\" heal=(\S+) hp=(\S+?)/(\S+) src=(\S+)\s*$")
REFUSE_RE = re.compile(r"^REFUSE\s*(.*?)\s*$")
DONE_RE = re.compile(r"^DONE member=(\S+) hp(\S+) stack(\S+) timeout=(\S+)\s*$")
AUTO_RE = re.compile(r"^AUTO member=(\S+) hp=(\S+?)/(\S+) pick=\"(.*)\" keep=(\S+)\s*$")
CFGSAVE_RE = re.compile(r"^CFG save tag=(\S*) auto=(\S+) hp=(\S+) keep=(\S+) ok=(\S+)\s*$")
CFGRELOAD_RE = re.compile(r"^CFG reload n=(\S+)\s*$")
DONE2_RE = re.compile(r"^DONE member=(\S+) hp(\S+) stack(\S+) queued=(\S+)\s*$")
KICK_RE = re.compile(r"^KICK member=(\S+) before=\[(.*?)\] after=\[(.*?)\] k=(\S+) route=rotate\s*$")
KICKSKIP_RE = re.compile(r"^KICK skipped member=(\S+) \((\w+)=(\S+?)\)\s*$")
KICKSTARVED_RE = re.compile(r"^KICK starved member=(\S+)\s*$")
TIMEOUT_RE = re.compile(r"^TIMEOUT unqueued member=(\S+)\s*$")
CLICK_RE = re.compile(r"^CLICK member=(\S+) noitem\s*$")
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


def v030_line(v, n, t):
    """Parse one companion-selfheal 0.3.0 line into v; True if it was one."""
    m = BUILD_RE.match(t)
    if m:
        k, den, why = to_int(m.group(1)), to_int(m.group(2)), (m.group(3) or "").strip()
        v["build"].append((n, k, why))
        if k is None or (k < den and not why):
            v["fail"].append((n, "BUILD n=%s/%s without why" % (m.group(1), m.group(2))))
    elif ICON_RE.match(t):
        v["warn"].append((n, "ICON fallback member=%s (cross texture not usable, \"+\" text shown)" % ICON_RE.match(t).group(1)))
    elif TEX_RE.match(t):
        g = TEX_RE.match(t).groups()
        v["tex"][g[1]] = v["tex"].get(g[1], 0) + 1
        if g[1] == "giving up":
            v["warn"].append((n, "TEX %s giving up (art missing, labels still work)" % g[0]))
    elif USE_RE.match(t):
        g = USE_RE.match(t).groups()
        v["use"].append({"line": n, "member": g[0], "item": g[1], "name": g[2], "heal": to_int(g[3]), "src": g[6]})
    elif DONE_RE.match(t):
        g = DONE_RE.match(t).groups()
        v["done"].append({"line": n, "member": g[0], "hp": to_int(g[1]), "stack": to_int(g[2]), "timeout": to_int(g[3])})
    elif DONE2_RE.match(t):
        g = DONE2_RE.match(t).groups()
        v["done"].append({"line": n, "member": g[0], "hp": to_int(g[1]), "stack": to_int(g[2]), "timeout": 0, "queued": to_int(g[3]), "new": True})
    elif KICK_RE.match(t):
        g = KICK_RE.match(t).groups()
        v["kick"].append({"line": n, "member": g[0], "before": g[1], "after": g[2], "k": to_int(g[3])})
        if to_int(g[3]) is None or to_int(g[3]) < 1 or not g[2].lower().startswith("3f"):
            v["warn"].append((n, "KICK member=%s did not bring 0x3F to the head (before=[%s] after=[%s] k=%s)" % (g[0], g[1], g[2], g[3])))
    elif KICKSKIP_RE.match(t):
        g = KICKSKIP_RE.match(t).groups()
        key = "%s=%s" % (g[1], g[2])
        v["kick_skip"][key] = v["kick_skip"].get(key, 0) + 1
    elif KICKSTARVED_RE.match(t):
        v["kick_starved"] += 1
        v["warn"].append((n, "KICK starved member=%s (queue stayed starved >5 s unpaused; kicking stopped for that pending)" % KICKSTARVED_RE.match(t).group(1)))
    elif TIMEOUT_RE.match(t):
        ms = TIMEOUT_RE.match(t).group(1)
        v["timeout_unq"] += 1
        for d in reversed(v["done"]):
            if d["member"] == ms and d.get("new"):
                d["timeout"] = 1
                break
        v["warn"].append((n, "TIMEOUT unqueued member=%s (pending ended with no progress and the use was not in the queue)" % ms))
    elif AUTO_RE.match(t):
        g = AUTO_RE.match(t).groups()
        v["auto"].append({"line": n, "member": g[0], "pick": g[3]})
    elif CFGSAVE_RE.match(t):
        g = CFGSAVE_RE.match(t).groups()
        v["cfg_save"].append((n, g[0], to_int(g[4])))
        if to_int(g[4]) != 1:
            v["fail"].append((n, "CFG save tag=%s ok=%s" % (g[0], g[4])))
    elif CFGRELOAD_RE.match(t):
        v["cfg_reload"].append((n, to_int(CFGRELOAD_RE.match(t).group(1))))
    elif CLICK_RE.match(t):
        v["click_noitem"] += 1
    elif REFUSE_RE.match(t):
        why = REFUSE_RE.match(t).group(1) or "(none)"
        if why.startswith("already queued"):
            v["refuse_queued"] += 1
        v["refuse"][why] = v["refuse"].get(why, 0) + 1
    else:
        return False
    v["seen"] = True
    return True


def check(lines, lo=1, hi=None):
    r = {"p1": "MISSING", "p1_fail": [], "p1_ok": 0, "p3": "PASS", "p3_fail": [], "p3_n": 0,
         "blocks": [], "dump_fail": [], "table": [],
         "v": {"build": [], "use": [], "done": [], "auto": [], "refuse": {}, "cfg_save": [], "cfg_reload": [], "click_noitem": 0,
               "fail": [], "warn": [], "tex": {}, "seen": False,
               "kick": [], "kick_skip": {}, "kick_starved": 0, "timeout_unq": 0, "refuse_queued": 0}}
    v = r["v"]
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
        if v030_line(v, n, t):
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
            # The HUD item group also lists non-heal items (grenades, stims: row=-1 or another spell row, heal=-1); they are skipped.
            # A heal row must estimate heal > 0, and only a heal row may estimate any heal.
            if row is None or heal is None:
                r["dump_fail"].append((n, "cand \"%s\": malformed row=%s heal=%s" % (m.group(1), m.group(2), m.group(3))))
            elif row in ROWS and heal <= 0:
                r["dump_fail"].append((n, "cand \"%s\": heal row %d but heal=%s not > 0" % (m.group(1), row, m.group(3))))
            elif row not in ROWS and heal > 0:
                r["dump_fail"].append((n, "cand \"%s\": row=%s not in %s but heal=%d" % (m.group(1), m.group(2), sorted(ROWS), heal)))
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
    if v["use"] and v["done"] and all(d["timeout"] == 1 and not d.get("new") for d in v["done"]):
        v["fail"].append((v["use"][0]["line"], "route A never lands: %d USE, every DONE has timeout=1" % len(v["use"])))
    bad = (bool(v["fail"]) or r["p1"] == "FAIL" or r["p3"] == "FAIL" or bool(r["dump_fail"])
           or (r["p1"] == "MISSING" and (r["blocks"] or r["p3_n"])))
    r["verdict"] = "FAIL" if bad else ("PASS" if (r["blocks"] or v["seen"]) else "INCOMPLETE")
    return r


def fmt(r):
    o = []
    if r["table"]:
        o.append("idx  hp        family            pick                         heal count (block@line)")
        for m in r["table"]:
            fams = sorted({FAMILY.get(c[2], "?") for c in m["cands"] if c[2] in ROWS})
            hp = "%s/%s" % (m["cur"], m["max"])
            p = m["pick"]
            o.append("%-4s %-9s %-17s %-28s %-4s %-5s (@%d)" % (
                m["idx"], hp, ",".join(f.split("(")[0] for f in fams) or "-",
                ('"%s" row=%s' % (p[0], p[1])) if p else "none", p[2] if p else "-", p[3] if p else "-", m["block"]))
    v = r["v"]
    if v["seen"]:
        o.append("member USE->DONE: hp-delta stack-delta timeout")
        pend = {}
        for ev in sorted(v["use"] + v["done"], key=lambda e: e["line"]):
            if "src" in ev:
                pend.setdefault(ev["member"], []).append(ev)
            else:
                u = pend.get(ev["member"], [None]).pop(0) if pend.get(ev["member"]) else None
                o.append("  m%s %s hp%+d stack%+d timeout=%s%s" % (ev["member"], ('"%s" src=%s' % (u["name"], u["src"])) if u else "(no USE)",
                         ev["hp"] or 0, ev["stack"] or 0, ev["timeout"], (" queued=%s" % ev["queued"]) if "queued" in ev else ""))
        for ms, us in pend.items():
            for u in us:
                o.append("  m%s \"%s\" src=%s no DONE" % (ms, u["name"], u["src"]))
        if v["refuse"]:
            o.append("REFUSE: " + ", ".join("%s x%d" % kv for kv in sorted(v["refuse"].items())))
        if v["kick"] or v["kick_skip"] or v["kick_starved"] or v["timeout_unq"] or v["refuse_queued"]:
            o.append("CS12: KICK=%d skipped=%d starved=%d REFUSE-queued=%d TIMEOUT-unqueued=%d" % (
                len(v["kick"]), sum(v["kick_skip"].values()), v["kick_starved"], v["refuse_queued"], v["timeout_unq"]))
            if v["kick_skip"]:
                o.append("KICK skipped: " + ", ".join("%s x%d" % kv for kv in sorted(v["kick_skip"].items())))
        o.append("AUTO=%d CLICK-noitem=%d CFG save=%d reload=%d" % (len(v["auto"]), v["click_noitem"], len(v["cfg_save"]), len(v["cfg_reload"])))
        if v["tex"]:
            o.append("TEX " + ", ".join("%s x%d" % kv for kv in sorted(v["tex"].items())))
        for n, msg in v["warn"]:
            o.append("WARN line %d: %s" % (n, msg))
        for n, msg in v["fail"]:
            o.append("FAIL 0.3.0 line %d: %s" % (n, msg))
    for tag, key in (("P1", "p1_fail"), ("P3", "p3_fail"), ("DUMP", "dump_fail")):
        for n, msg in r[key]:
            o.append("FAIL %s line %d: %s" % (tag, n, msg))
    extra = ""
    if v["seen"]:
        extra = " USE=%d DONE=%d TIMEOUT=%d" % (len(v["use"]), len(v["done"]), sum(1 for d in v["done"] if d["timeout"] == 1))
    o.append("SELFHEAL P1=%s P3=%s DUMP=%d blocks%s VERDICT=%s" % (r["p1"], r["p3"], len(r["blocks"]), extra, r["verdict"]))
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
