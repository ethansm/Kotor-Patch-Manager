#!/usr/bin/env python3
"""LevelUpOverride live-log checker (Phase 6b M4). Python 3.12, stdlib only; reads the log, never writes anything.

Usage:
    luo_log_check.py [--log PATH] [--from LINE] [--to LINE] [--scenario ID] [--scenarios-file PATH]
                     [--expect-crc HEX|DLLPATH] [--expect-version X.Y.Z] [--table PATH] [--json]

  --log        default ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/levelup_override_log.txt
               (C++ LogName = "levelup_override_log.txt", LevelUpOverride.cpp:120; the previous log rotates to
               levelup_override_log.old.txt). 1-based line numbers; --from default 1, --to default end of file.
               The whole file is parsed (so CONFIG state set before --from is known); verdicts use only lines in range.
  --scenario   id in tests/luo_scenarios/saves.toml (default --scenarios-file; the file is optional). Format (owned by
               that file's header): [scenario.<id>] title/status/v_items, [scenario.<id>.expect] events = [regex...] must each
               match some line in range, absent = [regex...] must match none. Regexes are re.search()ed against the raw line
               with the "[tick] " prefix removed. v_items = V items the scenario exercises: each must not be FAIL (their status is
               shown; NOT-EXERCISED/PARTIAL there is reported but not failing, since e.g. S5 only proves the unconfigured half).
               `decisions` is M3-only and ignored.
  Exit 0 when nothing is FAIL, 1 otherwise.

Every log line format below comes from patch_manager_mods/level_up_override_src (file:line next to each regex).

V checklist (doc 08 "Phase 6 RESUME HERE") - rule table. Status: PASS / FAIL / PARTIAL (some evidence, not the full rule) / NOT-EXERCISED.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import os
import re
import sys
import zlib
from collections import Counter, defaultdict
from pathlib import Path

try:
    import tomllib
except ImportError:  # pragma: no cover
    tomllib = None

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/levelup_override_log.txt"
DEFAULT_SCENARIOS = REPO / "tests/luo_scenarios/saves.toml"
DEFAULT_TABLE = REPO / "research/companion_levelup/runtime/levelup_paths.ini"
SIM_PATH = REPO / "research/companion_levelup/simulate_rewrite.py"

# ----------------------------------------------------------------------------------------------------------------------
# V checklist rule table (edit here). PASS rule / FAIL rule / NOT-EXERCISED rule.
# ----------------------------------------------------------------------------------------------------------------------
V_RULES = {
    1: ("Configure page look (LevelUpUi.h:122-204)",
        "PASS: a `UI ctor ... created=N/N` with equal N, back0..back2 lines all `ok=1 alpha ok=1`, `UI create` appended==n, "
        ">=1 `UI page open` (a `UI colour readback` line, logged before 1.0.0, must be ok=1 when present). FAIL: ctor created<N, any back/alpha ok=0, appended!=n, readback ok=0, "
        "`UI setText failed`, `UI no usable scale`, `UI tick: no record`. PARTIAL: ctor/create fine but no page opened. "
        "NOT-EXERCISED: no UI ctor line. Note lists page tags seen (doc 08: kreia/g0t0 rows still to be confirmed by eye)."),
    2: ("Leader instant level-up (LevelUpOverride.cpp:1512-1545)",
        "PASS: >=1 `INSTANT-LEADER`. PARTIAL: only `INSTANT` (non-leader route). FAIL: `ENGINE 0x73F450 bytes differ`. "
        "NOT-EXERCISED: neither."),
    3: ("Hold / status / GUARD / BANKED (LevelUpOverride.cpp:1382,1473,1585)",
        "PASS: a real `HOLD tag=` AND a `GUARD` (or `GUARD autook`) AND a `BANKED ... shown=1`. PARTIAL: some but not all. "
        "FAIL: `WOULD HOLD` while mode=rewrite, `BANKED engine function bytes differ`, `ENGINE gate function bytes differ`, "
        "or the same BANKED (tag,n) logged twice with no HOLD/UNHOLD between. NOT-EXERCISED: none of HOLD/WOULD HOLD/UNHOLD/GUARD/BANKED."),
    4: ("R1 manual picks kept (LevelUpOverride.cpp:1287-1299)",
        "A manual call = header flow not starting with `auto`. PASS: >=1 manual call (or `SKIP manual` line) and no APPLIED in any "
        "manual call. FAIL: APPLIED inside a manual-flow call. NOT-EXERCISED: no manual-flow call with a DECIDE and no SKIP manual."),
    5: ("Unconfigured tag + PC vanilla (LevelUpOverride.cpp:1274)",
        "PASS: >=1 level-up call header whose tag has no DECIDE (unconfigured, e.g. pc) and that call has no APPLIED/DRY/SKIP/HOLD. "
        "FAIL: such a call carries APPLIED/DRY/WOULD REWRITE, or GUARD/HOLD fires for a tag never configured. PARTIAL: only the "
        "`GUID load g00000000` + `CONFIG source=dev` (GUID-less save) evidence. NOT-EXERCISED: neither."),
    6: ("Restart -> load -> GUID (LevelUpOverride.cpp:906,1146,1050,1153)",
        "Per banner (first launch line): PASS: the first `GUID load` after it is a non-zero guid, found=32/32 result=1, followed by "
        "`CONFIG source=playthrough` of the same guid (and equal to any earlier GUID save/new in range). PARTIAL: first load is "
        "g00000000 (new game / GUID-less save -> unconfigured, `CONFIG source=dev`) or no banner in range. FAIL: non-zero guid "
        "with found!=32 or result!=1, a different guid than the last saved one, no source=playthrough after it, `GUID ENGINE "
        "mismatch`, or `GUID new:` refusal. NOT-EXERCISED: no GUID line."),
    7: ("No hang / leak (anomaly scan)",
        "PASS: >=1 line in range and none of: `== log line cap reached ==`, `TABLE ERROR`, any `ENGINE ... differ`, `APPLY ABORTED`, "
        "`PLAYTHROUGH ... LOSSY/write refused`, `UI setText failed`, one normalized line repeated >200x, >300 GUARD-PROBE, "
        "tick going backwards inside one launch. FAIL: any of those. NOT-EXERCISED: empty range. (A true hang is only visible as the "
        "log going silent; this checks what the log can show.)"),
}

# ----------------------------------------------------------------------------------------------------------------------
# Line grammar: (kind, regex, source). First match wins. Timestamped lines are `[tick] text` (logLine, LevelUpOverride.cpp:138);
# indented lines come from logf() inside one level-up call dump.
# ----------------------------------------------------------------------------------------------------------------------
TS_RE = re.compile(r"^\[(\d+)\] (.*)$")
HDR_RE = re.compile(r"^\[(\d+)\] #(\d+) tid=(\d+) flow=(\S+) .*? stats=([0-9a-f]+) pending=([0-9a-f]+) addToList=(-?\d+) tag=(\S+) "
                    r"classes=(.*?) total=(-?\d+) key=(-?\d+) xp=(\d+) unspent170=(\d+)\s*$")       # cpp:1224
GRAMMAR = [
    ("BANNER", r"== LevelUpOverride (\S+) loaded; first hook call \(build apply=(\d+)(?: crc32=([0-9a-f]{8}))?\) ==", "cpp:906 (crc32= since 1.0.0)"),
    ("DEV_ENABLED", r"DEV keys enabled \(dev=1\)", "cpp:285 (1.0.0)"),
    ("GUID_NEW_SERVER", r"GUID new-server \(cache cleared, was g([0-9a-f]+)\)", "cpp:1153"),
    ("GUID_NEW", r"GUID new g([0-9a-f]+) live-write ok=(\d+)", "cpp:1109"),
    ("GUID_NEW_REFUSED", r"GUID new: ", "cpp:1099/1101"),
    ("GUID_LOAD", r"GUID load g([0-9a-f]+) found=(\d+)/32 result=(-?\d+) table=([0-9a-f]+)", "cpp:1146"),
    ("GUID_SAVE", r"GUID save g([0-9a-f]+) table=([0-9a-f]+) ok=(\d+)", "cpp:1138"),
    ("GUID_SAVE_NONE", r"GUID save none", "cpp:1135"),
    ("GUID_ENGINE_MISMATCH", r"GUID ENGINE mismatch", "cpp:962"),
    ("CONFIG_PT", r"CONFIG source=playthrough g([0-9a-f]+) tags=(.*)", "cpp:1050"),
    ("CONFIG_DEV", r"CONFIG source=dev\s*$", "cpp:1046"),
    ("CONFIG", r"CONFIG (loaded|missing \(defaults\)) mode=(\w+) apply=(\d+) verbose=(\d+) table=(\S+) tags=(.*)", "cpp:285"),
    ("PLAYTHROUGH_LOSSY", r"PLAYTHROUGH (?:write refused|.*LOSSY)", "cpp:1071/1077"),
    ("PLAYTHROUGH", r"PLAYTHROUGH (loaded|missing|write g([0-9a-f]+) tag=(\S+) ok=(\d+))", "cpp:1071/1114"),
    ("TABLE_ERROR", r"TABLE ERROR", "cpp:522"),
    ("TABLE", r"TABLE (loaded|warning)", "cpp:519"),
    ("UI_CTOR", r"UI ctor panel=([0-9a-f]+) created=(\d+)/(\d+) fallbackScale=(\d+)", "ui.h:122"),
    ("UI_PAGE_BACK", r"UI page back(\d+) fill=(\S+) ok=(\d+) alpha ok=(\d+)", "ui.h:138"),
    ("UI_CREATE", r"UI create panel=([0-9a-f]+) n=(\d+) appended=(\d+) textobj=([0-9a-f]+)", "ui.h:146"),
    ("UI_SETTEXT_FAIL", r"UI setText failed", "ui.h:194"),
    ("UI_COLOUR", r"UI colour readback ok=(\d+)", "ui.h:204"),
    ("UI_PAGE_OPEN", r"UI page open tag=(\S+) start=(\S+) \((\w+)\)", "ui.h:296"),
    ("UI_CLICK", r"UI click (\S+) tag=(\S+) row=(\S+)", "ui.h:372"),
    ("UI_APPLY", r"UI apply tag=(\S+) preset=(\S+) instant=(\d+) hold=(\d+)(?: pause=(\d+))? ok=(\d+)", "ui.h:387"),
    ("UI_VANILLA", r"UI vanilla tag=(\S+) ok=(\d+)", "ui.h:393"),
    ("UI_PROBLEM", r"UI (?:tick: no record|no usable scale)", "ui.h:407/420"),
    ("GUARD_PROBE", r"GUARD-PROBE (\S+) site=(0x[0-9A-Fa-f]+) leader=([0-9a-f]+) stats=([0-9a-f]+) tagCreature=(\S+) tagStats=(\S+) decision=(\S+)", "cpp:1581"),
    ("GUARD_AUTOOK", r"GUARD autook tag=(\S+) key=(-?\d+) site=(0x[0-9A-Fa-f]+)", "cpp:1597"),
    ("GUARD", r"GUARD (\S+) tag=(\S+) key=(-?\d+) site=(0x[0-9A-Fa-f]+)", "cpp:1585"),
    ("WOULD_HOLD", r"WOULD HOLD tag=(\S+) key=(-?\d+) site=(0x[0-9A-Fa-f]+)", "cpp:1382"),
    ("HOLD", r"HOLD tag=(\S+) key=(-?\d+) site=(0x[0-9A-Fa-f]+)", "cpp:1382"),
    ("UNHOLD", r"UNHOLD tag=(\S+)", "cpp:1383"),
    ("BANKED_DISABLED", r"BANKED engine function bytes differ", "cpp:1435"),
    ("BANKED", r"BANKED tag=(\S+) n=(\d+) guid=([0-9a-f]+) shown=(\d+)", "cpp:1473"),
    ("INSTANT_LEADER", r"INSTANT-LEADER tag=(\S+) key=(-?\d+) controlledId=([0-9a-f]+) creature=([0-9a-f]+) site=(0x[0-9A-Fa-f]+)", "cpp:1545"),
    ("INSTANT", r"INSTANT tag=(\S+) key=(-?\d+) site=(0x[0-9A-Fa-f]+)", "cpp:1512"),
    ("ENGINE_MISMATCH", r"ENGINE .*differ", "cpp:1330/1532"),
    ("LOG_CAP", r"== log line cap reached ==", "cpp:135"),
    # indented (inside a call dump)
    ("DECIDE", r"\s+DECIDE tag=(\S+) preset=(\S+) key=(-?\d+) decision=(\S+) rec=(\S+)", "cpp:1274"),
    ("DRY", r"\s+DRY tag=(\S+) key=(-?\d+) status=(\S+)", "cpp:1278/formatDry"),
    ("SKIP_MANUAL", r"\s+SKIP manual (\S+) key=(-?\d+) flow=(\S+)", "cpp:1287"),
    ("SKIP", r"\s+SKIP (.*)", "cpp:1279/1291"),
    ("ALREADY", r"\s+ALREADY \(replay\) (\S+) key=(-?\d+)", "cpp:1289"),
    ("WOULD_REWRITE", r"\s+WOULD REWRITE (\S+) key=(-?\d+)", "cpp:1293"),
    ("APPLIED", r"\s+APPLIED tag=(\S+) key=(-?\d+) ok=(\d+)", "cpp:1299 (tag= is the PRESET id)"),
    ("APPLY_ABORTED", r"\s+APPLY ABORTED (.*?) - vanilla", "cpp:841"),
    ("PRESET_MISSING", r"\s+tag (\S+) configured but preset", "cpp:1305"),
    ("PENDING_UNREADABLE", r"\s+pending unreadable", "cpp:1228"),
    ("DETAIL", r"\s+(?:pend|after)[.:]|\s+stats\.", "cpp:722-760,1243-1259 (verbose dumps)"),
]
GRAMMAR_RE = [(k, re.compile(r"^(?:" + rx + r")"), src) for k, rx, src in GRAMMAR]
INDENTED = {"DECIDE", "DRY", "SKIP_MANUAL", "SKIP", "ALREADY", "WOULD_REWRITE", "APPLIED", "APPLY_ABORTED", "PRESET_MISSING",
            "PENDING_UNREADABLE", "DETAIL"}
ALL_KINDS = [k for k, _, _ in GRAMMAR] + ["CALL"]
CRC_RE = re.compile(r"crc32=([0-9a-f]{8})")
CLASS_RE = re.compile(r"(\d+):(\d+)(?:\(neg\)\d*)?")


class Ev:
    __slots__ = ("line", "raw", "text", "tick", "kind", "g", "call", "cfg")

    def __init__(self, line, raw, text, tick, kind, g, call):
        self.line, self.raw, self.text, self.tick, self.kind, self.g, self.call = line, raw, text, tick, kind, g, call
        self.cfg = None

    def d(self):
        return {"line": self.line, "kind": self.kind, "raw": self.raw}


def parse_config_tags(s):
    """'atton:preset/i0/h1/p3,kreia:-/i0/h0' -> {tag: (preset, instant, hold, pause)}; old 0.3.0 lacks /p (pause=0)."""
    out = {}
    for ent in (s or "").split(","):
        ent = ent.strip()
        if not ent or ":" not in ent:
            continue
        tag, rest = ent.split(":", 1)
        parts = rest.split("/")
        preset = parts[0]
        flags = {p[0]: int(p[1:]) for p in parts[1:] if len(p) > 1 and p[1:].lstrip("-").isdigit()}
        out[tag] = (None if preset == "-" else preset, flags.get("i", 0), flags.get("h", 0), flags.get("p", 0))
    return out


def parse_log(lines):
    """Return (events, unknown_count). Tracks the CONFIG state in force at every line (Ev.cfg) and the owning call header."""
    evs, unknown = [], 0
    call = None
    state = {"src": None, "pt": {}, "dev": {}, "mode": None, "guid": None}
    for n, raw in enumerate(lines, 1):
        raw = raw.rstrip("\r\n")
        if not raw.strip():
            continue
        m = TS_RE.match(raw)
        tick = int(m.group(1)) if m else None
        text = m.group(2) if m else raw
        kind, g = None, ()
        mh = HDR_RE.match(raw)
        if mh:
            kind, g = "CALL", mh.groups()
            call = len(evs)
        else:
            for k, rx, _ in GRAMMAR_RE:
                mm = rx.match(text)
                if mm:
                    kind, g = k, mm.groups()
                    break
        if kind is None:
            unknown += 1
            evs.append(Ev(n, raw, text, tick, "UNKNOWN", (), call))
            continue
        if kind == "BANNER":
            state.update(src=None, pt={}, guid=None)
        elif kind == "CONFIG":
            state["dev"] = parse_config_tags(g[5])
            state["mode"] = g[1]
        elif kind == "CONFIG_DEV":
            state["src"] = "dev"
        elif kind == "CONFIG_PT":
            state["src"], state["pt"], state["guid"] = "playthrough", parse_config_tags(g[1]), g[0]
        e = Ev(n, raw, text, tick, kind, g, call)
        e.cfg = (state["src"], dict(state["pt"]), dict(state["dev"]), state["mode"])
        evs.append(e)
    return evs, unknown


def active_cfg(ev, tag):
    src, pt, dev, _ = ev.cfg
    d = pt if src == "playthrough" else dev
    return d.get(tag) or d.get(tag.lower())


# ----------------------------------------------------------------------------------------------------------------------
# V item evaluators. Each returns (status, [supporting line numbers], note)
# ----------------------------------------------------------------------------------------------------------------------
def by(evs, *kinds):
    return [e for e in evs if e.kind in kinds]


def ln(es):
    return [e.line for e in es]


def v1(evs):
    ctor, back, create, col = by(evs, "UI_CTOR"), by(evs, "UI_PAGE_BACK"), by(evs, "UI_CREATE"), by(evs, "UI_COLOUR")
    bad = [e for e in ctor if e.g[1] != e.g[2]] + [e for e in back if e.g[2] != "1" or e.g[3] != "1"] + \
          [e for e in create if e.g[1] != e.g[2]] + [e for e in col if e.g[0] != "1"] + by(evs, "UI_SETTEXT_FAIL", "UI_PROBLEM")
    if bad:
        return "FAIL", ln(bad), "UI creation problem"
    if not ctor:
        return "NOT-EXERCISED", [], "no UI ctor line"
    opens = by(evs, "UI_PAGE_OPEN")
    tags = sorted({e.g[0] for e in opens})
    sup = ln(ctor[:1] + back[:3] + create[:1] + col[:1] + opens[:2])
    if not opens:
        return "PARTIAL", sup, "ctor/back/create ok, no page opened"
    return "PASS", sup, "ctor+backs+create ok; page tags seen: %s; kreia/g0t0 rows not in log: %s" % (
        ",".join(tags), ",".join(t for t in ("kreia", "g0t0") if t not in tags) or "none")


def v2(evs):
    bad = [e for e in by(evs, "ENGINE_MISMATCH") if "0x73F450" in e.text]
    if bad:
        return "FAIL", ln(bad), "leader instant disabled (engine bytes differ)"
    il, i = by(evs, "INSTANT_LEADER"), by(evs, "INSTANT")
    if il:
        return "PASS", ln(il), "%d INSTANT-LEADER" % len(il)
    if i:
        return "PARTIAL", ln(i), "INSTANT seen but no INSTANT-LEADER"
    return "NOT-EXERCISED", [], ""


def v3(evs):
    hold, would, unhold = by(evs, "HOLD"), by(evs, "WOULD_HOLD"), by(evs, "UNHOLD")
    guard, probe = by(evs, "GUARD", "GUARD_AUTOOK"), by(evs, "GUARD_PROBE")
    banked = by(evs, "BANKED")
    fails = [e for e in would if e.cfg[3] == "rewrite"] + by(evs, "BANKED_DISABLED") + \
            [e for e in by(evs, "ENGINE_MISMATCH") if "gate function" in e.text]
    seen = set()
    for e in evs:   # same (tag,n) twice with no HOLD/UNHOLD between
        if e.kind in ("HOLD", "UNHOLD"):
            seen.clear()
        elif e.kind == "BANKED":
            k = (e.g[0], e.g[1])
            if k in seen:
                fails.append(e)
            seen.add(k)
    if fails:
        return "FAIL", ln(fails), "inconsistent hold/banked state"
    allc = hold + would + unhold + guard + banked
    if not allc:
        return "NOT-EXERCISED", [], "no HOLD/GUARD/BANKED lines (%d GUARD-PROBE)" % len(probe)
    sup = ln(hold[:2] + guard[:2] + banked[:2] + would[:1] + unhold[:1])
    missing = [n for n, l in (("HOLD", hold), ("GUARD", guard), ("BANKED", banked)) if not l]
    if missing:
        return "PARTIAL", sup, "missing: " + ",".join(missing)
    return "PASS", sup, "HOLD x%d, GUARD x%d, BANKED x%d" % (len(hold), len(guard), len(banked))


def call_flow(evs, e):
    if e.call is None:
        return None
    return evs[e.call].g[3]


def v4(evs_all, rng):
    evs = rng
    applied_bad, ok_calls = [], []
    for e in evs:
        if e.kind == "APPLIED":
            f = call_flow(evs_all, e)
            if f is not None and not f.startswith("auto"):
                applied_bad.append(e)
    if applied_bad:
        return "FAIL", ln(applied_bad), "APPLIED in a manual-flow call"
    ok_calls = by(evs, "SKIP_MANUAL")
    for e in evs:
        if e.kind == "DECIDE":
            f = call_flow(evs_all, e)
            if f is not None and not f.startswith("auto"):
                ok_calls.append(e)
    if not ok_calls:
        return "NOT-EXERCISED", [], "no manual-flow call with a DECIDE"
    return "PASS", ln(ok_calls[:4]), "%d manual call(s)/SKIP manual, none APPLIED" % len(ok_calls)


def v5(evs_all, evs):
    calls = by(evs, "CALL")
    configured_tags = set()
    for e in by(evs, "CONFIG_PT", "CONFIG"):
        configured_tags |= {t.lower() for t, v in parse_config_tags(e.g[1] if e.kind == "CONFIG_PT" else e.g[5]).items() if v[0]}
    decided = {e.call for e in evs if e.kind == "DECIDE" and e.call is not None}
    idx = {id(e): i for i, e in enumerate(evs_all)}
    vanilla_calls, bad = [], []
    for c in calls:
        ci = idx[id(c)]
        if ci in decided:
            continue
        own = [e for e in evs if e.call == ci]
        if any(e.kind in ("APPLIED", "DRY", "WOULD_REWRITE", "HOLD") for e in own):
            bad.append(c)
        else:
            vanilla_calls.append(c)
    for e in by(evs, "GUARD", "GUARD_AUTOOK", "HOLD"):
        tag = e.g[1] if e.kind == "GUARD" else e.g[0]
        if tag.lower() not in configured_tags and not active_cfg(e, tag):
            bad.append(e)
    if bad:
        return "FAIL", ln(bad), "rewrite/hold activity for an unconfigured tag"
    if vanilla_calls:
        return "PASS", ln(vanilla_calls[:4]), "unconfigured call(s) for tag(s) %s left vanilla" % ",".join(sorted({c.g[7] for c in vanilla_calls}))
    dev = [e for e in by(evs, "GUID_LOAD") if e.g[0] == "0" * 8]
    devc = by(evs, "CONFIG_DEV")
    if dev and devc:
        return "PARTIAL", ln(dev[:1] + devc[:1]), "GUID-less load -> source=dev only; no unconfigured level-up call seen"
    return "NOT-EXERCISED", [], ""


def v6(evs):
    gl = by(evs, "GUID_LOAD")
    bad = [e for e in by(evs, "GUID_ENGINE_MISMATCH", "GUID_NEW_REFUSED")]
    for e in gl:
        if e.g[0] != "0" * 8 and (e.g[1] != "32" or e.g[2] != "1"):
            bad.append(e)
    saved = None
    banners = by(evs, "BANNER")
    # walk in order, remember last saved/new guid; check every first-load-after-banner
    first_after = {}
    cur_banner = None
    for e in evs:
        if e.kind == "BANNER":
            cur_banner = e
        elif e.kind in ("GUID_SAVE", "GUID_NEW") and e.g[-1] == "1":
            saved = e.g[0]
        elif e.kind == "GUID_LOAD" and cur_banner is not None and id(cur_banner) not in first_after:
            first_after[id(cur_banner)] = (cur_banner, e)
    passed, partial = [], []
    for b, l in first_after.values():
        if l.g[0] == "0" * 8:
            partial.append(l)
            continue
        i = evs.index(l)
        pt = [x for x in evs[i + 1:i + 6] if x.kind == "CONFIG_PT" and x.g[0] == l.g[0]]
        if not pt:
            bad.append(l)
        else:
            passed.append((b, l, pt[0]))
    if saved:
        for b, l, _ in passed:
            if l.g[0] != saved and evs.index(b) > 0 and any(x.kind in ("GUID_SAVE", "GUID_NEW") and x.line < b.line for x in evs):
                prev = [x for x in evs if x.kind in ("GUID_SAVE", "GUID_NEW") and x.line < b.line]
                if prev and prev[-1].g[0] != l.g[0]:
                    bad.append(l)
    if bad:
        return "FAIL", ln(bad), "bad GUID load / engine mismatch / refusal / guid differs from the saved one"
    if not gl and not by(evs, "GUID_NEW_SERVER"):
        return "NOT-EXERCISED", [], ""
    if passed:
        b, l, p = passed[0]
        return "PASS", [b.line, l.line, p.line], "banner -> GUID load g%s found=32/32 -> source=playthrough" % l.g[0]
    if partial:
        return "PARTIAL", ln(partial[:2]), "first load after launch is g00000000 (unconfigured); no restart->non-zero-load proof"
    return "PARTIAL", ln(gl[:2]), "GUID loads seen but no banner in range (restart not proven)"


def v7(evs):
    if not evs:
        return "NOT-EXERCISED", [], "empty range"
    bad = by(evs, "LOG_CAP", "TABLE_ERROR", "ENGINE_MISMATCH", "APPLY_ABORTED", "PLAYTHROUGH_LOSSY", "UI_SETTEXT_FAIL", "BANKED_DISABLED")
    notes = []
    norm = Counter()
    for e in evs:
        if e.kind in ("DETAIL", "CALL", "UNKNOWN"):
            continue
        norm[re.sub(r"[0-9a-f]{5,}|\d+", "#", e.text)] += 1
    spam = [k for k, c in norm.items() if c > 200]
    if spam:
        notes.append("repeated >200x: " + spam[0][:60])
    probes = by(evs, "GUARD_PROBE")
    if len(probes) > 300:
        notes.append("%d GUARD-PROBE" % len(probes))
    last = None
    back = []
    for e in evs:
        if e.kind == "BANNER":
            last = None
        if e.tick is not None:
            if last is not None and e.tick < last and last - e.tick < 2 ** 31:
                back.append(e)
            last = e.tick
    if back:
        notes.append("tick went backwards")
    if bad or spam or len(probes) > 300 or back:
        return "FAIL", ln(bad + back), "; ".join(notes) or "anomaly lines present"
    return "PASS", [], "%d events, no anomalies" % len(evs)


# ----------------------------------------------------------------------------------------------------------------------
# Replay of DECIDE through simulate_rewrite.level_now
# ----------------------------------------------------------------------------------------------------------------------
def load_sim():
    spec = importlib.util.spec_from_file_location("simulate_rewrite", SIM_PATH)
    mod = importlib.util.module_from_spec(spec)
    sys.modules.setdefault("simulate_rewrite", mod)
    spec.loader.exec_module(mod)
    return mod


def replay(evs_all, rng, table_path):
    """Returns dict(status, rows, note). Needs per DECIDE: classes+key (CALL header + DECIDE), hold+pause (CONFIG tags=),
    preset records + hold_class (levelup_paths.ini). Missing pieces are listed."""
    decides = by(rng, "DECIDE")
    if not decides:
        return {"status": "NOT-EXERCISED", "rows": [], "note": "no DECIDE lines in range"}
    try:
        sim = load_sim()
        _cs, presets, recs = sim.parse_table_v2(Path(table_path).read_text(errors="replace"))
    except Exception as ex:  # noqa: BLE001
        return {"status": "NOT-POSSIBLE", "rows": [], "note": "replay not possible: cannot load simulate_rewrite/table (%s)" % ex}
    rows, missing, diverge = [], Counter(), []
    for d in decides:
        tag, pid, key, dec, rec = d.g[0], d.g[1], int(d.g[2]), d.g[3], d.g[4]
        hdr = evs_all[d.call] if d.call is not None else None
        if hdr is None:
            missing["classes (CALL header absent in file)"] += 1
            continue
        classes = [(int(a), int(b)) for a, b in CLASS_RE.findall(hdr.g[8])]
        if not classes:
            missing["classes (header classes= unparsable: %r)" % hdr.g[8]] += 1
            continue
        c = active_cfg(d, tag)
        if c is None:
            missing["hold/pause for tag %s (no CONFIG tags= entry in force; 0.3.0 CONFIG lacks /p)" % tag] += 1
            continue
        if pid not in recs or pid not in presets:
            missing["preset %s not in %s" % (pid, Path(table_path).name)] += 1
            continue
        hold_class = int(presets[pid].hold_class)
        off, cl, _r = sim.level_now(classes, recs[pid], key, bool(c[2]), hold_class, c[3])
        live = "HOLD" if dec == "HOLD_MIDLOOP" else dec
        live_rec = None if rec == "-" else int(rec)
        # live HOLD_MIDLOOP is HOLD with the record wiped; PATH/REBASE carry the char level
        ok = (off == live) and (cl == live_rec or (live in ("HOLD", "VANILLA") and cl is None))
        row = {"line": d.line, "tag": tag, "preset": pid, "classes": classes, "key": key, "hold": c[2], "pause": c[3],
               "live": live, "live_rec": live_rec, "offline": off, "offline_rec": cl, "match": ok}
        rows.append(row)
        if not ok:
            diverge.append(row)
    note = ""
    if missing:
        note = "replay incomplete: missing fields: " + "; ".join("%s x%d" % kv for kv in missing.items())
    status = "FAIL" if diverge else ("PASS" if rows else "NOT-POSSIBLE")
    if status == "NOT-POSSIBLE":
        note = "replay not possible: " + (note or "no usable DECIDE")
    return {"status": status, "rows": rows, "note": note, "diverged": [r["line"] for r in diverge]}


# ----------------------------------------------------------------------------------------------------------------------
# Scenario expectations
# ----------------------------------------------------------------------------------------------------------------------
def check_scenario(sid, path, rng, vres):
    if tomllib is None or not Path(path).exists():
        return {"status": "SKIPPED", "note": "scenarios file missing (%s)" % path, "checks": []}
    doc = tomllib.loads(Path(path).read_text())
    sc = doc.get("scenario", {}).get(sid)
    if sc is None:
        return {"status": "FAIL", "note": "scenario %r not in %s" % (sid, path), "checks": []}
    exp = sc.get("expect", {})
    checks = []

    def texts():
        for e in rng:
            yield e, (e.text if e.tick is not None else e.raw).lstrip()  # '^' anchors work on indented call lines

    for pat in exp.get("events", []):
        rx = re.compile(pat)
        hit = [e.line for e, t in texts() if rx.search(t)]
        checks.append({"type": "event", "pattern": pat, "ok": bool(hit), "lines": hit[:5]})
    for pat in exp.get("absent", []):
        rx = re.compile(pat)
        hit = [e.line for e, t in texts() if rx.search(t)]
        checks.append({"type": "absent", "pattern": pat, "ok": not hit, "lines": hit[:5]})
    for item in sc.get("v_items", []):
        st = vres[item]["status"]
        checks.append({"type": "v_item", "pattern": "V%d" % item, "ok": st != "FAIL", "lines": [], "status": st})
    ok = all(c["ok"] for c in checks)
    return {"status": "PASS" if ok else "FAIL", "title": sc.get("title", ""), "scenario_status": sc.get("status", ""), "checks": checks, "note": ""}


# ----------------------------------------------------------------------------------------------------------------------
def run(lines, lo=1, hi=None, scenario=None, scenarios_file=DEFAULT_SCENARIOS, expect_crc=None, expect_version=None, table=DEFAULT_TABLE):
    evs_all, unknown = parse_log(lines)
    hi = hi if hi is not None else len(lines)
    rng = [e for e in evs_all if lo <= e.line <= hi]
    vres = {}
    for i, fn in ((1, v1), (2, v2), (3, v3), (5, None), (6, v6), (7, v7)):
        if i == 5:
            st, sup, note = v5(evs_all, rng)
        else:
            st, sup, note = fn(rng)
        vres[i] = {"status": st, "lines": sup, "note": note}
    st, sup, note = v4(evs_all, rng)
    vres[4] = {"status": st, "lines": sup, "note": note}
    for i in vres:
        vres[i]["title"] = V_RULES[i][0]
        vres[i]["rule"] = V_RULES[i][1]
    banners = by(rng, "BANNER")
    versions = [(e.line, e.g[0], e.g[1]) for e in banners]
    crcs = []
    for e in banners:
        mm = CRC_RE.search(e.raw)
        if mm:
            crcs.append((e.line, mm.group(1)))
    ident = {"banners": versions, "crc32": crcs, "checks": []}
    if expect_version:
        okv = bool(versions) and all(v[1] == expect_version for v in versions)
        ident["checks"].append({"what": "version==%s" % expect_version, "ok": okv if versions else None})
    if expect_crc:
        want = expect_crc.lower()
        if os.path.isfile(expect_crc):   # a DLL path: crc32 of the file (same value the DLL logs)
            want = "%08x" % (zlib.crc32(Path(expect_crc).read_bytes()) & 0xFFFFFFFF)
        if not crcs:
            ident["checks"].append({"what": "crc32==%s" % want, "ok": None,
                                    "note": "UNVERIFIABLE: no banner with crc32= in range (DLLs before 1.0.0 log the version only)"})
        else:
            ident["checks"].append({"what": "crc32==%s" % want, "ok": all(c[1] == want for c in crcs)})
    rep = replay(evs_all, rng, table)
    sc = check_scenario(scenario, scenarios_file, rng, vres) if scenario else None
    kinds = Counter(e.kind for e in rng)
    fails = [("V%d" % i) for i, r in vres.items() if r["status"] == "FAIL"]
    if rep["status"] == "FAIL":
        fails.append("replay")
    if sc and sc["status"] == "FAIL":
        fails.append("scenario")
    fails += [c["what"] for c in ident["checks"] if c["ok"] is False]
    return {"range": [lo, hi], "events": len(rng), "unknown_lines": unknown, "kinds": dict(kinds), "v": vres, "ident": ident,
            "replay": rep, "scenario": sc, "fails": fails, "exit": 1 if fails else 0}


def format_report(r):
    o = ["luo_log_check: lines %d-%d, %d events, %d unknown lines" % (r["range"][0], r["range"][1], r["events"], r["unknown_lines"])]
    o.append("kinds: " + ", ".join("%s=%d" % kv for kv in sorted(r["kinds"].items())))
    o.append("")
    for i in sorted(r["v"]):
        v = r["v"][i]
        o.append("V%d %-14s %s%s%s" % (i, v["status"], v["title"], ("  [lines %s]" % ",".join(map(str, v["lines"][:8]))) if v["lines"] else "",
                                       ("  - " + v["note"]) if v["note"] else ""))
    o.append("")
    idn = r["ident"]
    o.append("banner: " + ("; ".join("line %d version %s apply=%s" % b for b in idn["banners"]) or "none in range"))
    o.append("dll crc32 logged: " + ("; ".join("line %d %s" % m for m in idn["crc32"]) or "no (DLLs before 1.0.0 do not log it)"))
    for c in idn["checks"]:
        o.append("  %s: %s %s" % (c["what"], {True: "OK", False: "FAIL", None: "n/a"}[c["ok"]], c.get("note", "")))
    rp = r["replay"]
    o.append("replay (simulate_rewrite.level_now): %s %s" % (rp["status"], rp.get("note", "")))
    for row in rp["rows"]:
        o.append("  line %d %s key=%d classes=%s hold=%d pause=%d live=%s/%s offline=%s/%s %s" % (
            row["line"], row["tag"], row["key"], row["classes"], row["hold"], row["pause"], row["live"], row["live_rec"],
            row["offline"], row["offline_rec"], "OK" if row["match"] else "DIVERGES"))
    sc = r["scenario"]
    if sc:
        o.append("")
        o.append("scenario: %s %s %s" % (sc["status"], sc.get("title", ""), sc.get("note", "")))
        for c in sc["checks"]:
            o.append("  [%s] %s %s%s%s" % ("ok" if c["ok"] else "FAIL", c["type"], c["pattern"],
                                           (" lines " + ",".join(map(str, c["lines"]))) if c["lines"] else "",
                                           (" (" + c["status"] + ")") if "status" in c else ""))
    o.append("")
    o.append("RESULT: " + ("FAIL (%s)" % ", ".join(r["fails"]) if r["fails"] else "no FAIL"))
    return "\n".join(o)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--log", default=str(DEFAULT_LOG))
    ap.add_argument("--from", dest="lo", type=int, default=1)
    ap.add_argument("--to", dest="hi", type=int, default=None)
    ap.add_argument("--scenario")
    ap.add_argument("--scenarios-file", default=str(DEFAULT_SCENARIOS))
    ap.add_argument("--expect-crc", help="8-hex crc32 of the installed DLL, or the DLL path (crc computed here); compared with the banner crc32=")
    ap.add_argument("--expect-version")
    ap.add_argument("--table", default=str(DEFAULT_TABLE), help="levelup_paths.ini for the replay (default: repo runtime table)")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args(argv)
    try:
        lines = Path(a.log).read_text(errors="replace").splitlines()
    except OSError as ex:
        print("cannot read log: %s" % ex, file=sys.stderr)
        return 2
    r = run(lines, a.lo, a.hi, a.scenario, a.scenarios_file, a.expect_crc, a.expect_version, a.table)
    print(json.dumps(r, indent=1, default=str) if a.json else format_report(r))
    return r["exit"]


if __name__ == "__main__":
    sys.exit(main())
