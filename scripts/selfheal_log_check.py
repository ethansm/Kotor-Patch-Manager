#!/usr/bin/env python3
"""Companion-selfheal probe log checker (doc 27, CS3). Python 3.12, stdlib only; read-only.

Usage: selfheal_log_check.py [LOG | --log LOG] [--from LINE] [--to LINE]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/companion-selfheal_log.txt
  1-based line numbers; verdicts use only lines in range. The whole file is one session (the probe logs no banner).
  Exit 0 = PASS/INCOMPLETE, 1 = FAIL, 2 = log missing/unreadable.

Line formats come from patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp; KpLog.h prefixes each line with
"<ms> companion-selfheal/<chan> [ERR |WARN ]" or "T=<secs> f=<frame|-> companion-selfheal/<chan> [ERR |WARN ]"
(prefix optional here so bare lines parse too).
Checks (0.4.0-dev adds): BUILD n=N/D (FAIL if N<D without why=), ICON fallback and TEX giving up are WARN.
0.6.0-dev (CS12): REFUSE already queued, KICK member=N before=[..] after=[..] k=K route=rotate, KICK skipped member=N (gate=v),
KICK starved member=N, DONE ... queued=Q, TIMEOUT unqueued member=N. `KICK starved` and `TIMEOUT unqueued` are WARN (never FAIL): a timeout while
queued cannot happen by design, so an unqueued timeout means the use never entered the queue or was dropped.
0.7.0-dev (popalign10+disc2): TEX takes both the legacy `TEX <res> queued|refilled|giving up` and the EmbedTex.h lines
`tex <res>: <why>, WxH RGBA queued for upload (...)`; `giving up`, `slot table full`, `allocation failed` are WARN. SETFAIL is WARN (counted).
P4 popover rows (7 text items auto,hpm,hpp,keepm,keepp,hpval,keepval: tobj=1, rend!=0, extOk=1, rectSet=1, vis=1, ext w>0 h>0; bg exempt),
INFO `rows h=<min> screen=<WxH>`. P5 discs (TEXSET disc and dischi != 0 else FAIL, ensure=1 else WARN; every DISC img!=0 and setfill=1),
INFO `discs slots=N/M DISC=n hover=h`. 0.8.3-dev: `TEXLATE slot=i resref=r img=h` (once per slot TEXSET logged as null that a later build resolved) clears an earlier null TEXSET disc/dischi and the DISC img=0 lines before it (INFO `late=...`). P4/P5 are NOT-SEEN (not FAIL) when their lines are absent. STATE lookA and BUILD proj/cleared are optional.
Checks: P1 (>=1 `P1 hud built ... layoutOpen=1`, ctlCount 1..4096), P3 (every `P3 slot[i]=` has k=0|1|2),
DUMP blocks (hp cur<=max, heal-row cand {64,65,66,84,127,128} has heal>0 and only those have heal>0, a pick line, routeA idOk=1 when a pick exists).
0.8.0-dev (popsize+togglepill+leaderclick): POPSIZE m= unit= panelW= widest= line= screen=WxH, POPHOVER m= row= (1 auto, 2 hp-, 3 hp+, 4 keep-, 5 keep+), POPCLICK m= row=, CLICK member=0 ok|noneed|noitem (leader click).
P6 popover fit: each POPSIZE needs panelW >= widest+2*unit and line <= 1.5*unit, else FAIL; NOT-SEEN without POPSIZE. P7 interaction: INFO counts of POPHOVER, POPCLICK and CLICK member=0 ok/noneed/noitem; WARN (never FAIL) when a CLICK member=0 ok has no later USE member=0 or DONE member=0. P6/P7 NOT-SEEN when their lines are absent.
0.8.2-dev (vanillaarrows+popocclude): ARROW neg=/pos= (once), ARROW retry=N neg= pos= and ARROW fill m= item= ok= img=; POPOCCLUDE m= btn=.
INFO `arrows neg=<h> pos=<h> fills ok=<n>/<m> retries=<r>` (final handles = last retry line's, else the first ARROW neg/pos line).
WARN (never FAIL) when the final neg or pos handle is 00000000 or any `ARROW fill ... ok=0` exists (text fallback in use). ARROW is
NOT-SEEN when no ARROW line is in range. P7 INFO gains POPOCCLUDE=<n> only when POPOCCLUDE lines are present (count only, P7 status unchanged).
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/companion-selfheal_log.txt"

# KpLog.h writes "%u %s/%s %s" then the text: exactly one space after the channel, so DUMP sub-line indentation survives.
PREFIX_RE = re.compile(r"^(?:T=\d+(?:\.\d+)? f=(?:\d+|-)|\d+)\s+[\w.-]+/\w+ (?:(?:ERR|WARN) )?(.*)$")
P1_RE = re.compile(r"^P1 hud built .*?\blayoutOpen=(\S+) .*?\bctlCount=(\S+)\s*$")
P3_RE = re.compile(r"^P3 slot\[(\d+)\]=\S+ .*?\bk=(\S+)\s*$")
MEMBER_RE = re.compile(r"^member (\d+) .*?\bhp=(\S+?)/(\S+?) ")
CAND_RE = re.compile(r"^\s+cand \"(.*)\" .*?\brow=(\S+) heal=(\S+) count=(\S+)")
PICK_RE = re.compile(r"^\s+pick missing=(\S+) -> (?:\"(.*)\" row=(\S+) heal=(\S+) count=(\S+)|none)")
ROUTE_RE = re.compile(r"^\s+routeA \(not called\):.*\bidOk=(\S+)")
BUILD_RE = re.compile(r"^BUILD n=(\S+)/(\d+)(?:\s+why=(.*?))?(?:\s+proj=(\S+) cleared=(\S+))?\s*$")   # 0.3.0: /10, 0.4.0-dev: /12 (+2 ring labels), 0.7.0-dev: proj= cleared=
ICON_RE = re.compile(r"^ICON fallback member=(\S+)")
TEX_OLD_RE = re.compile(r"^TEX (\S+) (queued|refilled|giving up)\b")   # 0.4.0..0.6.0
TEX_RE = re.compile(r"^tex (\S+): (.*?)\s*$")   # 0.7.0-dev, EmbedTex.h: `tex <res>: <why>, WxH RGBA queued for upload (basic p, pending n)`
ALLOC_FAIL_RE = re.compile(r"allocation failed|malloc\(\S*\) failed")
VERSION_RE = re.compile(r"^(\d+\.\d+\.\d+(?:-\w+)?)(?: ui=(\S+))?: probe call=")   # `0.5.0-dev: probe call=` and `0.7.0-dev ui=popalign10+disc2: probe call=`
POP_RE = re.compile(r"^POP open m=(\S+) screen=(\S+?)x(\S+) unit=(\S+) btn=\[([^\]]*)\] align=(\S+)\s*$")
POPNULL_RE = re.compile(r"^POPROW m=(\S+) item=(\S+) null\s*$")
POPROW_RE = re.compile(r'^POPROW m=(\S+) item=(\S+) ext=\[([^\]]*)\] extOk=(\S+) vis=(\S+) txt="(.*?)" tobj=(\S+) rend=(\S+) rgb=\S+ rgbOk=(\S+) rectSet=(\S+)\s*$')
TEXSET_RE = re.compile(r"^TEXSET ensure=(\S+) supported=(\S+) ready=(\S+) cross=(\S+) disc=(\S+) dischi=(\S+)(?: tog=\S+)? ring0=(\S+) slots=(\S+?)/(\S+)\s*$")   # tog=a,b,c,d (CompanionSelfHeal.cpp:517) is optional and non-capturing: legacy lines have none, groups stay 1-9
TEXLATE_RE = re.compile(r"^TEXLATE slot=(\S+) resref=(\S+) img=(\S+)\s*$")   # CompanionSelfHeal.cpp ensureTextures: once per slot that TEXSET logged as null and a later build resolved
DISC_RE = re.compile(r"^DISC m=(\S+) hover=(\S+) img=(\S+) setfill=(\S+)\s*$")
SETFAIL_RE = re.compile(r"^SETFAIL op=(\S+)(?: .*)?$")
STATE_RE = re.compile(r"^STATE member=(\S+) (\S+?)->(\S+) hp=(\S+)(?: lookA=(\S+))?\s*$")
USE_RE = re.compile(r"^USE member=(\S+) item=(\S+) \"(.*)\" heal=(\S+) hp=(\S+?)/(\S+) src=(\S+)\s*$")
REFUSE_RE = re.compile(r"^REFUSE\s*(.*?)\s*$")
DONE_RE = re.compile(r"^DONE member=(\S+) hp(\S+) stack(\S+) timeout=(\S+)\s*$")
AUTO_RE = re.compile(r"^AUTO member=(\S+) hp=(\S+?)/(\S+) pick=\"(.*)\" keep=(\S+)\s*$")
CFGSAVE_RE = re.compile(r"^CFG save tag=(\S*) auto=(\S+) hp=(\S+) keep=(\S+) ok=(\S+)\s*$")
CFGRELOAD_RE = re.compile(r"^CFG reload n=(\S+)\s*$")
DONE2_RE = re.compile(r"^DONE member=(\S+) hp(\S+) stack(\S+) queued=(\S+)(?: reason=(\S+))?\s*$")
KICK_RE = re.compile(r"^KICK member=(\S+) before=\[(.*?)\] after=\[(.*?)\] k=(\S+) route=rotate\s*$")
KICKSKIP_RE = re.compile(r"^KICK skipped member=(\S+) \((\w+)=(\S+?)\)\s*$")
KICKSTARVED_RE = re.compile(r"^KICK starved member=(\S+)\s*$")
TIMEOUT_RE = re.compile(r"^TIMEOUT unqueued member=(\S+)\s*$")
CLICK_RE = re.compile(r"^CLICK member=(\S+) noitem\s*$")
DIAGPOP_RE = re.compile(r"^DIAG pop item=(\S+) .*?rgbOk=(\d+) alphaOk=(\d+)")
# 0.8.0-dev. CLICK_RE (0.3.0) also matches `CLICK member=0 noitem`; v080_line sees that line first and keeps its legacy count.
POPSIZE_RE = re.compile(r"^POPSIZE m=(\S+) unit=(\S+) panelW=(\S+) widest=(\S+) line=(\S+) screen=(\S+)\s*$")
POPHOVER_RE = re.compile(r"^POPHOVER m=(\S+) row=(\S+)\s*$")
POPCLICK_RE = re.compile(r"^POPCLICK m=(\S+) row=(\S+)\s*$")
CLICK0_RE = re.compile(r"^CLICK member=0 (ok|noneed|noitem)\s*$")
# 0.8.2-dev: ARROW neg/pos (once), ARROW retry=N neg= pos= (per retry), ARROW fill m= item= ok= img= (per arrow), POPOCCLUDE m= btn=.
ARROW_HDR_RE = re.compile(r"^ARROW neg=(\S+) pos=(\S+)\s*$")
ARROW_RETRY_RE = re.compile(r"^ARROW retry=(\S+) neg=(\S+) pos=(\S+)\s*$")
ARROW_FILL_RE = re.compile(r"^ARROW fill m=(\S+) item=(\S+) ok=(\S+) img=(\S+)\s*$")
POPOCCLUDE_RE = re.compile(r"^POPOCCLUDE m=(\S+) btn=(\S+)\s*$")
# POPROW items that carry text; "bg" has txt="" and tobj=0 by design, so P4 skips it.
TEXT_ITEMS = ("auto", "hpm", "hpp", "keepm", "keepp", "hpval", "keepval")
TEX_WARN = {
    "giving up": "TEX %s giving up (art missing, labels still work)",
    "slot full": "TEX %s slot table full (texture not uploaded)",
    "alloc failed": "TEX %s allocation failed (texture not uploaded)",
}
ROWS = {64, 65, 66, 84, 127, 128}
FAMILY = {64: "organic medpac(10+1*TI)", 65: "organic medpac(20+2*TI)", 66: "organic medpac(30+3*TI)",
          84: "droid repair(15+1*Rep)", 127: "droid repair(25+2*Rep)", 128: "droid repair(35+3*Rep)"}


def to_int(s):
    try:
        return int(s)
    except (TypeError, ValueError):
        return None


def hexval(s):
    try:
        return int(s, 16)
    except (TypeError, ValueError):
        return None


def strip(raw):
    m = PREFIX_RE.match(raw)
    return m.group(1) if m else raw


def tex_event(t):
    """(res, kind) for a texture line in either format, else None.
    kind: queued | refilled | giving up | slot full | alloc failed | other (other = informational, never WARN)."""
    m = TEX_OLD_RE.match(t)
    if m:
        return m.group(1), m.group(2)
    m = TEX_RE.match(t)
    if not m:
        return None
    res, why = m.group(1), m.group(2)
    if "giving up" in why:
        kind = "giving up"
    elif "slot table full" in why:
        kind = "slot full"
    elif ALLOC_FAIL_RE.search(why):
        kind = "alloc failed"
    elif "queued for upload" in why:
        kind = "refilled" if why.startswith("refilled") else "queued"
    else:
        kind = "other"
    return res, kind


def v030_line(v, n, t):
    """Parse one companion-selfheal 0.3.0 line into v; True if it was one."""
    m = BUILD_RE.match(t)
    if m:
        k, den, why = to_int(m.group(1)), to_int(m.group(2)), (m.group(3) or "").strip()
        v["build"].append((n, k, why, to_int(m.group(4)), to_int(m.group(5))))
        if k is None or (k < den and not why):
            v["fail"].append((n, "BUILD n=%s/%s without why" % (m.group(1), m.group(2))))
    elif ICON_RE.match(t):
        v["warn"].append((n, "ICON fallback member=%s (cross texture not usable, \"+\" text shown)" % ICON_RE.match(t).group(1)))
    elif (tx := tex_event(t)):
        v["tex"][tx[1]] = v["tex"].get(tx[1], 0) + 1
        if tx[1] in TEX_WARN:
            v["warn"].append((n, TEX_WARN[tx[1]] % tx[0]))
    elif USE_RE.match(t):
        g = USE_RE.match(t).groups()
        v["use"].append({"line": n, "member": g[0], "item": g[1], "name": g[2], "heal": to_int(g[3]), "src": g[6]})
    elif DONE_RE.match(t):
        g = DONE_RE.match(t).groups()
        v["done"].append({"line": n, "member": g[0], "hp": to_int(g[1]), "stack": to_int(g[2]), "timeout": to_int(g[3])})
    elif DONE2_RE.match(t):
        g = DONE2_RE.match(t).groups()
        v["done"].append({"line": n, "member": g[0], "hp": to_int(g[1]), "stack": to_int(g[2]), "timeout": 0, "queued": to_int(g[3]), "reason": g[4], "new": True})
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
    elif DIAGPOP_RE.match(t):
        m = DIAGPOP_RE.match(t)
        rgb_ok, alpha_ok = to_int(m.group(2)), to_int(m.group(3))
        v["diag_pop"].append((n, to_int(m.group(1)), rgb_ok, alpha_ok))
        if rgb_ok != 1 or alpha_ok != 1:
            v["fail"].append((n, "DIAG pop item=%s color/alpha not ok (rgbOk=%s alphaOk=%s)" % (m.group(1), m.group(2), m.group(3))))
    elif REFUSE_RE.match(t):
        why = REFUSE_RE.match(t).group(1) or "(none)"
        if why.startswith(("already queued", "cast queued")):
            v["refuse_queued"] += 1
        v["refuse"][why] = v["refuse"].get(why, 0) + 1
    else:
        return False
    v["seen"] = True
    return True


def v070_line(v, n, t):
    """Parse one 0.7.0-dev line (version, popover rows, TEXSET/DISC, SETFAIL, STATE) into v; True if it was one."""
    m = VERSION_RE.match(t)
    if m:
        v["version"] = (m.group(1), m.group(2))
        return True
    if (m := POP_RE.match(t)):
        v["pop_open"].append({"line": n, "m": m.group(1), "screen": (m.group(2), m.group(3))})
    elif (m := POPNULL_RE.match(t)):
        v["pop_rows"].append({"line": n, "m": m.group(1), "item": m.group(2), "null": True})
    elif (m := POPROW_RE.match(t)):
        g = m.groups()
        v["pop_rows"].append({"line": n, "m": g[0], "item": g[1], "null": False, "ext": g[2], "extOk": g[3], "vis": g[4],
                              "txt": g[5], "tobj": g[6], "rend": g[7], "rectSet": g[9]})
    elif (m := TEXSET_RE.match(t)):
        g = m.groups()
        v["texset"].append({"line": n, "ensure": g[0], "supported": g[1], "ready": g[2], "cross": g[3], "disc": g[4],
                            "dischi": g[5], "ring0": g[6], "slots": g[7], "max": g[8]})
    elif (m := TEXLATE_RE.match(t)):
        v["texlate"].append({"line": n, "slot": m.group(1), "resref": m.group(2), "img": m.group(3)})
    elif (m := DISC_RE.match(t)):
        v["disc"].append({"line": n, "m": m.group(1), "hover": m.group(2), "img": m.group(3), "setfill": m.group(4)})
    elif (m := SETFAIL_RE.match(t)):
        v["setfail"].append((n, m.group(1)))
    elif (m := STATE_RE.match(t)):
        # STATE does not count as "seen": legacy logs with only STATE lines keep their old verdict.
        v["state"].append({"line": n, "member": m.group(1), "old": m.group(2), "new": m.group(3), "hp": m.group(4), "lookA": m.group(5)})
        return True
    else:
        return False
    v["seen"] = True
    return True


def v080_line(v, n, t):
    """Parse one 0.8.0-dev line (POPSIZE, POPHOVER, POPCLICK, leader CLICK member=0) into v; True if it was one."""
    if (m := POPSIZE_RE.match(t)):
        g = m.groups()
        # "lh" is the POPSIZE line= value (row height); "line" stays the log line number.
        v["popsize"].append({"line": n, "m": g[0], "unit": to_int(g[1]), "panelW": to_int(g[2]), "widest": to_int(g[3]),
                             "lh": to_int(g[4]), "raw": g[1:5]})
    elif (m := POPHOVER_RE.match(t)):
        v["pop_hover"].append({"line": n, "m": m.group(1), "row": to_int(m.group(2))})
    elif (m := POPCLICK_RE.match(t)):
        v["pop_click"].append({"line": n, "m": m.group(1), "row": to_int(m.group(2))})
    elif (m := CLICK0_RE.match(t)):
        v["click0"].append({"line": n, "result": m.group(1)})
        if m.group(1) == "noitem":
            v["click_noitem"] += 1   # same legacy counter v030_line bumps for `CLICK member=N noitem`
    else:
        return False
    v["seen"] = True
    return True


def v082_line(v, n, t):
    """Parse one 0.8.2-dev line (ARROW neg/retry/fill, POPOCCLUDE) into v; True if it was one."""
    if (m := ARROW_HDR_RE.match(t)):
        v["arrow_hdr"].append({"line": n, "neg": m.group(1), "pos": m.group(2)})
    elif (m := ARROW_RETRY_RE.match(t)):
        v["arrow_retry"].append({"line": n, "tries": to_int(m.group(1)), "neg": m.group(2), "pos": m.group(3)})
    elif (m := ARROW_FILL_RE.match(t)):
        v["arrow_fill"].append({"line": n, "m": m.group(1), "item": m.group(2), "ok": m.group(3), "img": m.group(4)})
    elif (m := POPOCCLUDE_RE.match(t)):
        v["popocclude"].append({"line": n, "m": m.group(1), "btn": m.group(2)})
    else:
        return False
    v["seen"] = True
    return True


def pop_check(v):
    """P4 popover rows -> (status, [(line, msg)], info). NOT-SEEN when no POP/POPROW line is in range."""
    rows, opens = v["pop_rows"], v["pop_open"]
    if not rows and not opens:
        return "NOT-SEEN", [], None
    fails, heights = [], []
    for row in rows:
        if row["item"] not in TEXT_ITEMS:
            continue
        if row["null"]:
            fails.append((row["line"], "POPROW m=%s item=%s null (text item has no row)" % (row["m"], row["item"])))
            continue
        bad = []
        if to_int(row["tobj"]) != 1:
            bad.append("tobj=%s" % row["tobj"])
        if not hexval(row["rend"]):
            bad.append("rend=%s" % row["rend"])
        if to_int(row["extOk"]) != 1:
            bad.append("extOk=%s" % row["extOk"])
        if to_int(row["rectSet"]) != 1:
            bad.append("rectSet=%s" % row["rectSet"])
        if to_int(row["vis"]) != 1:
            bad.append("vis=%s" % row["vis"])
        e = [to_int(x) for x in row["ext"].split(",")]
        if len(e) != 4 or None in e or e[2] <= 0 or e[3] <= 0:
            bad.append("ext=[%s] (need w>0 h>0)" % row["ext"])
        else:
            heights.append(e[3])
        if bad:
            fails.append((row["line"], "POPROW m=%s item=%s: %s" % (row["m"], row["item"], ", ".join(bad))))
    screen = ("%sx%s" % opens[0]["screen"]) if opens else "unknown"
    info = "rows h=%s screen=%s" % (min(heights) if heights else "none", screen)
    return ("FAIL" if fails else "PASS"), fails, info


def disc_check(v):
    """P5 discs/TEXSET -> (status, [(line, msg)] fails, [(line, msg)] warns, info). NOT-SEEN when no TEXSET/DISC line is in range."""
    texset, discs = v["texset"], v["disc"]
    if not texset and not discs:
        return "NOT-SEEN", [], [], None
    fails, warns = [], []
    # TEXLATE (a slot TEXSET logged as null resolved on a later build) clears the earlier null TEXSET disc/dischi and the DISC img=0 lines before it.
    late = {}
    for L in v["texlate"]:
        if hexval(L["img"]):
            late.setdefault(L["resref"], L["line"])
    recovered = set()
    for t in texset:
        for key, res in (("disc", "csh_disc"), ("dischi", "csh_disc_hi")):
            if hexval(t[key]) == 0 and res in late and late[res] > t["line"]:
                recovered.add(key)
            elif not hexval(t[key]):
                fails.append((t["line"], "TEXSET %s=%s (disc texture missing)" % (key, t[key])))
        if to_int(t["ensure"]) != 1:
            warns.append((t["line"], "P5 TEXSET ensure=%s (disc textures not ensured)" % t["ensure"]))
    for d in discs:
        if hexval(d["img"]) == 0 and "csh_disc" in late and late["csh_disc"] > d["line"]:   # img=0 only comes from the normal disc (hover falls back to it)
            recovered.add("DISC")
            continue
        if not hexval(d["img"]):
            fails.append((d["line"], "DISC m=%s img=%s (no disc image)" % (d["m"], d["img"])))
        if to_int(d["setfill"]) != 1:
            fails.append((d["line"], "DISC m=%s setfill=%s (fill not applied)" % (d["m"], d["setfill"])))
    hover = sum(1 for d in discs if to_int(d["hover"]) == 1)
    slots = ("%s/%s" % (texset[-1]["slots"], texset[-1]["max"])) if texset else "none"
    info = "discs slots=%s DISC=%d hover=%d" % (slots, len(discs), hover)
    if recovered:
        info += " late=" + ",".join(sorted(recovered))
    return ("FAIL" if fails else "PASS"), fails, warns, info


def popsize_check(v):
    """P6 popover fit -> (status, [(line, msg)] fails, info). NOT-SEEN when no POPSIZE line is in range.
    Each POPSIZE: panelW >= widest + 2*unit (side slack) and line <= 1.5*unit (row height against the unit)."""
    sizes = v["popsize"]
    if not sizes:
        return "NOT-SEEN", [], None
    fails, slack = [], []
    for s in sizes:
        u, pw, wd, lh = s["unit"], s["panelW"], s["widest"], s["lh"]
        if None in (u, pw, wd, lh):
            fails.append((s["line"], "POPSIZE m=%s malformed (unit=%s panelW=%s widest=%s line=%s)" % ((s["m"],) + tuple(s["raw"]))))
            continue
        need = wd + 2 * u
        slack.append(pw - need)
        if pw < need:
            fails.append((s["line"], "POPSIZE m=%s panelW=%d < widest+2*unit (%d+2*%d=%d): popover too narrow" % (s["m"], pw, wd, u, need)))
        if lh > 1.5 * u:
            fails.append((s["line"], "POPSIZE m=%s line=%d > 1.5*unit (1.5*%d=%g): rows too tall" % (s["m"], lh, u, 1.5 * u)))
    info = "popsize n=%d min_slack=%s" % (len(sizes), min(slack) if slack else "none")
    return ("FAIL" if fails else "PASS"), fails, info


def interact_check(v):
    """P7 popover/leader-click interaction -> (status, [(line, msg)] warns, info). NOT-SEEN when no POPHOVER, POPCLICK or
    CLICK member=0 line is in range. Never FAIL: WARN when a `CLICK member=0 ok` has no later USE member=0 or DONE member=0."""
    hover, clicks, c0 = v["pop_hover"], v["pop_click"], v["click0"]
    if not (hover or clicks or c0):
        return "NOT-SEEN", [], None
    landed = [e["line"] for e in v["use"] + v["done"] if e["member"] == "0"]
    lost = [c["line"] for c in c0 if c["result"] == "ok" and not any(x > c["line"] for x in landed)]
    warns = []
    if lost:
        warns.append((lost[0], "P7 CLICK member=0 ok x%d with no later USE member=0 or DONE member=0 (leader click did nothing)" % len(lost)))
    cnt = {k: sum(1 for c in c0 if c["result"] == k) for k in ("ok", "noneed", "noitem")}
    occ = v["popocclude"]   # count only: POPOCCLUDE never changes P7 status, and is omitted from the line when absent
    info = "interact POPHOVER=%d POPCLICK=%d CLICK0 ok=%d noneed=%d noitem=%d%s" % (
        len(hover), len(clicks), cnt["ok"], cnt["noneed"], cnt["noitem"], (" POPOCCLUDE=%d" % len(occ)) if occ else "")
    return ("WARN" if warns else "PASS"), warns, info


def arrow_check(v):
    """0.8.2-dev arrows -> (status, [(line, msg)] warns, info). NOT-SEEN when no ARROW line is in range.
    Final handles = the last `ARROW retry=` line's, else the first `ARROW neg=/pos=` line's. Never FAIL: WARN when the final
    neg or pos handle is 00000000 or any `ARROW fill ... ok=0` exists (text fallback in use)."""
    hdr, retry, fills = v["arrow_hdr"], v["arrow_retry"], v["arrow_fill"]
    if not (hdr or retry or fills):
        return "NOT-SEEN", [], None
    last = retry[-1] if retry else (hdr[0] if hdr else None)
    neg, pos = (last["neg"], last["pos"]) if last else ("none", "none")
    warns = []
    if last:
        for name, h in (("neg", neg), ("pos", pos)):
            if hexval(h) == 0:
                warns.append((last["line"], "ARROW %s=%s null handle (text fallback in use)" % (name, h)))
    bad = [f for f in fills if to_int(f["ok"]) == 0]
    if bad:
        warns.append((bad[0]["line"], "ARROW fill ok=0 x%d (text fallback in use)" % len(bad)))
    ok = sum(1 for f in fills if to_int(f["ok"]) == 1)
    info = "arrows neg=%s pos=%s fills ok=%d/%d retries=%d" % (neg, pos, ok, len(fills), len(retry))
    return ("WARN" if warns else "PASS"), warns, info


def check(lines, lo=1, hi=None):
    r = {"p1": "MISSING", "p1_fail": [], "p1_ok": 0, "p3": "PASS", "p3_fail": [], "p3_n": 0,
         "blocks": [], "dump_fail": [], "table": [],
         "p4": "NOT-SEEN", "p4_fail": [], "p4_info": None, "p5": "NOT-SEEN", "p5_fail": [], "p5_info": None,
         "p6": "NOT-SEEN", "p6_fail": [], "p6_info": None, "p7": "NOT-SEEN", "p7_info": None,
         "arrow": "NOT-SEEN", "arrow_info": None,
         "v": {"build": [], "use": [], "done": [], "auto": [], "refuse": {}, "cfg_save": [], "cfg_reload": [], "click_noitem": 0,
               "fail": [], "warn": [], "tex": {}, "seen": False, "diag_pop": [],
               "kick": [], "kick_skip": {}, "kick_starved": 0, "timeout_unq": 0, "refuse_queued": 0,
               "version": None, "pop_open": [], "pop_rows": [], "texset": [], "texlate": [], "disc": [], "setfail": [], "state": [],
               "popsize": [], "pop_hover": [], "pop_click": [], "click0": [],
               "arrow_hdr": [], "arrow_retry": [], "arrow_fill": [], "popocclude": []}}
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
        if v082_line(v, n, t) or v080_line(v, n, t) or v030_line(v, n, t) or v070_line(v, n, t):
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
    r["p4"], r["p4_fail"], r["p4_info"] = pop_check(v)
    r["p5"], r["p5_fail"], p5_warn, r["p5_info"] = disc_check(v)
    v["warn"].extend(p5_warn)
    r["p6"], r["p6_fail"], r["p6_info"] = popsize_check(v)
    r["p7"], p7_warn, r["p7_info"] = interact_check(v)
    v["warn"].extend(p7_warn)
    r["arrow"], arrow_warn, r["arrow_info"] = arrow_check(v)
    v["warn"].extend(arrow_warn)
    if v["setfail"]:
        ops = {}
        for _, op in v["setfail"]:
            ops[op] = ops.get(op, 0) + 1
        v["warn"].append((v["setfail"][0][0], "SETFAIL x%d: %s (a widget property did not apply)" % (
            len(v["setfail"]), ", ".join("op=%s x%d" % kv for kv in sorted(ops.items())))))
    bad = (bool(v["fail"]) or r["p1"] == "FAIL" or r["p3"] == "FAIL" or bool(r["dump_fail"])
           or r["p4"] == "FAIL" or r["p5"] == "FAIL" or r["p6"] == "FAIL"
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
        if v["use"] or v["done"]:
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
    for tag, key in (("P1", "p1_fail"), ("P3", "p3_fail"), ("P4", "p4_fail"), ("P5", "p5_fail"), ("P6", "p6_fail"), ("DUMP", "dump_fail")):
        for n, msg in r[key]:
            o.append("FAIL %s line %d: %s" % (tag, n, msg))
    # 0.7.0-dev block: printed only when its lines are present, so legacy logs keep their exact output.
    new70 = bool(v["pop_open"] or v["pop_rows"] or v["texset"] or v["disc"] or (v["version"] and v["version"][1]))
    if new70:
        if v["version"]:
            ver, ui = v["version"]
            o.append("version %s%s" % (ver, (" ui=%s" % ui) if ui else ""))
        if r["p4_info"]:
            o.append(r["p4_info"])
        if r["p5_info"]:
            o.append(r["p5_info"])
    # 0.8.0-dev block: printed only when its lines are present, so older logs keep their exact output.
    new80 = bool(v["popsize"] or v["pop_hover"] or v["pop_click"] or v["click0"])
    if new80:
        if r["p6_info"]:
            o.append(r["p6_info"])
        if r["p7_info"]:
            o.append(r["p7_info"])
    # 0.8.2-dev block: printed only when its ARROW lines are present (r["arrow_info"] is set exactly then).
    new82 = bool(v["arrow_hdr"] or v["arrow_retry"] or v["arrow_fill"])
    if new82 and r["arrow_info"]:
        o.append(r["arrow_info"])
    extra = ""
    if v["seen"]:
        extra = " USE=%d DONE=%d TIMEOUT=%d" % (len(v["use"]), len(v["done"]), sum(1 for d in v["done"] if d["timeout"] == 1))
    if new70:
        extra += " P4=%s P5=%s" % (r["p4"], r["p5"])
    if new80:
        extra += " P6=%s P7=%s" % (r["p6"], r["p7"])
    if new82:
        extra += " ARROW=%s" % r["arrow"]
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
