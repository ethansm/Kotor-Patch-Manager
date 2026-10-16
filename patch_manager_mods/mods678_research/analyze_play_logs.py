#!/usr/bin/env python3
"""Play-log analyzer for the KOTOR II engine-hook mods companion-ai, ranged-ai and inventory-sort-filter.

Usage:
    python3 analyze_play_logs.py PLAYLOGS_DIR [--hooks-toml [ID=]PATH ...] [--window HH:MM-HH:MM]
                                 [--anchor "YYYY-MM-DD HH:MM:SS=TICK_MS"] [--out PATH] [--strict]

Reads (each optional) from PLAYLOGS_DIR: companion-ai_log.txt (KpLog core, companion-ai 0.2.1) or ai_trace.txt (legacy / fallback shape),
ranged_ai_log.txt or ranged-ai_log.txt (KpLog core, ranged-ai 0.4.0), inventory_sort_filter_log.txt,
steam-208580.log (the Proton/Wine log). Writes REPORT.md into PLAYLOGS_DIR (or --out).
Python 3.12, stdlib only. Never reads the live game folder: it only reads the directory it is given.

Every line format below was taken from the C++ sources (file:line refs in the comments next to each regex).
"""
from __future__ import annotations

import argparse
import bisect
import datetime as dt
import os
import re
import sys
import tomllib
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "scripts"))
import kplog_parse  # noqa: E402

HERE = Path(__file__).resolve().parent
MODS_DIR = HERE.parent                       # patch_manager_mods/

# ----------------------------------------------------------------------------------------------------------------------
# Tables (edit here, nowhere else)
# ----------------------------------------------------------------------------------------------------------------------
# Default hook tomls: (patch id, path). The id is read from the sibling manifest.toml when present.
DEFAULT_TOMLS = [
    MODS_DIR / "companion_ai_src" / "kotor2-steam-aspyr.hooks.toml",
    MODS_DIR / "ranged_ai_src" / "kotor2-steam-aspyr.hooks.toml",
    MODS_DIR / "inventory_sort_filter_src" / "kotor2-steam-aspyr.hooks.toml",
]
EXPECTED_HOOK_COUNTS = {"companion-ai": 31, "ranged-ai": 7}   # task spec; inventory-sort-filter: no fixed count

# Script trace markers (text inside the P line after "| "). key, regex, where the text comes from.
#   P-line text = "GENERIC DEBUG *************** <marker text>" (party_ai_fixes.nss, kite_routine.nss, party_ai_patches.py)
MARKERS = [
    ("Party E1",          r"Party E1:",                          "party_ai_fixes.nss:45/55/61/64 (all E1 lines)"),
    ("Party E1 skipped",  r"Party E1: skipped",                  "party_ai_fixes.nss:45"),
    ("Party E1 exit",     r"Party E1: no enemy seen, stance (\d+) combat exit", "party_ai_fixes.nss:64"),
    ("Party H1",          r"Party H1:",                          "party_ai_patches.py:131"),
    ("Party MEDPAC",      r"Party MEDPAC:",                      "party_ai_patches.py:263"),
    ("Party M1",          r"Party M1:",                          "party_ai_patches.py:216"),
    ("Party M4",          r"Party M4:",                          "party_ai_patches.py:317"),
    ("Party M10",         r"Party M10:",                         "party_ai_patches.py:103"),
    ("Party E3",          r"Party E3:",                          "party_ai_fixes.nss:78/86"),
    ("Party E8",          r"Party E8:",                          "party_ai_patches.py:88"),
    ("Party E11",         r"Party E11:",                         "party_ai_patches.py:412"),
    ("Kite step",         r"Kite step (\d+)(?: streak (\d+))?",       "kite_routine.nss:223 (step 1) / :252 (step 3)"),
    ("Kite K2 cap",       r"Kite K2 cap",                        "kite_routine.nss:233"),
    ("Kite K1",           r"Kite K1 drop target",                "kite_routine.nss:208"),
    ("Kite E12",          r"Kite E12 reset",                     "kite_routine.nss:184"),
]
MARKER_RES = [(k, re.compile(rx), src) for k, rx, src in MARKERS]

# Vanilla trace lines used by the heuristics (design_ai_script_fixes.md section 8 test matrix, around lines 850-880).
CLEAR_RE = re.compile(r"(?<!Shout )\bClear (\d{4})\b")        # round-final action line; "Shout Clear 18xx/19xx" excluded
HEAL_CODES = ("2300", "2400")                                  # heal / cure rounds (the recast pattern)
SPELL_ID_RE = re.compile(r"Spells\.2DA ID = (\d+)")
POISON_START_RE = re.compile(r"Starting Poison Check")
POISON_INVALID_RE = re.compile(r"Returning Invalid Poison Object")
ROUND_START_RE = re.compile(r"START DETERMINE COMBAT ROUND")
SHOUT_1800_RE = re.compile(r"Shout Clear 1800")
KNOCKED_OUT_RE = re.compile(r"Someone is knocked out\.")
AOE_RE = re.compile(r"Jedi Support AI: AOE Target")
MEDPACK_RE = re.compile(r"I have a Med Pack")
# "attack line" for the E1-skipped-too-long check (heuristic: the vanilla attack/cast action lines; --attack-regex overrides)
DEFAULT_ATTACK_RE = r"(?i)\battack|Spells\.2DA ID|Clear 1450|Clear 1[01]00"
RECAST_MAX_ROUNDS = 3
E1_SKIP_MAX_MS = 30_000

# checklist ids (PLAY_CHECKLIST_2026-10-02.md)
CHECKLIST = [
    ("M6-1", "Backpack: new first, then usable, then grouped by kind"),
    ("M6-2", "Red tint on unusable gear, changes per party member"),
    ("M6-3", "Equipment slot list after closing inventory: equips, no wrongly red rows (H6d)"),
    ("M6-4", "'All' re-click cycles New -> Usable -> Upgradeable -> All"),
    ("M8-1", "Ranged (Kiting) chosen: message + HUD icon"),
    ("M8-2", "Keeps distance from melee, moves for a clear shot"),
    ("M8-3", "Kites at most twice in a row, then shoots"),
    ("M8-4", "Far target / melee weapon: walks in (kite_approach)"),
    ("M8-5", "Kites on its very first combat round"),
    ("CAI-1", "A1 freeze fix: out-of-range Ranged companion not frozen"),
    ("CAI-2", "H2 healing: heals any party member below 50%, no medpacs"),
    ("CAI-3", "Poison: Heal/Master Heal companion cures within ~15 m"),
    ("CAI-4", "Area powers never centred on an ally"),
    ("CAI-5", "H1 Revitalize: Jedi Support without it keeps fighting"),
    ("CAI-6", "E1: Ranged/Kiting follow after the last enemy dies"),
    ("CAI-7", "FM1: bumped companion steps aside and keeps following"),
    ("CAI-8", "FM4: companions follow after save and load"),
    ("E3", "Stance-11 H1 lines with los=0 (log answer)"),
    # Phase-4 probe checks (companion-ai 0.2.0-probe, PROBE_DESIGN.md section 2); PASS = data agrees with the atlas, FAIL = contradicts it / shows a bug's reach
    ("PR-FM2B", "Probe FM2C: failed path search is partial (2) then fail (3); kill FM2b when no line"),
    ("PR-FM2", "Probe FM2R/FM2F: re-approach loop pairs (n >= 3) come with FM2C/FM2F lines"),
    ("PR-FM3", "Probe FM3: scheduler sentinel LOST rate"),
    ("PR-SLOT6", "Probe RS: slot-6 site mix for party members (S6A/S6B/S6C)"),
    ("PR-FM5", "Probe RS: OnBlocked runs per second (< 5 kills the FM5 throttle)"),
    ("PR-E9", "Probe RS/SHX: allied shouts (pattern 15) delivered to party listeners"),
    ("PR-P2", "Probe P2: companion perception cadence ~4 s; controlled-companion runs"),
    ("PR-PF", "Probe PF: party-flag writers (saveA == saveB, no unknown callers)"),
    ("PR-RAR", "Probe RAR: action histogram, MOVESTATE / REQ"),
    ("PR-SHOUT", "Probe SHX: shout delivery without ABORT exits"),
    ("PR-SS", "Probe SS1/RS: companion SetState followed by an S6C run within 60 s"),
    ("PR-E4", "Probe GCA: GetCurrentAction 3/39 after engine approach actions"),
    ("PR-PARTY", "Probe PARTY: every member and the controlled creature has the party flag"),
    ("PR-A1X", "Probe A1X: every A1 end followed up (slot-6 run), no ORPHAN"),
    ("PR-RAH2", "ranged-ai H2 retarget=1 never followed by H1 end52c!=0 inRange=0 within 2 s"),
]

BENIGN_EXC = {"40010006": "DBG_PRINTEXCEPTION_C (OutputDebugString)", "406d1388": "thread rename",
              "6ba": "RPC_S_SERVER_UNAVAILABLE (wine startup)", "6be": "RPC (wine startup)"}

FILES = {"trace": "ai_trace.txt", "ranged": "ranged_ai_log.txt", "inv": "inventory_sort_filter_log.txt"}
# companion-ai 0.2.1-probe logs through KpLog: core file companion-ai_log.txt ("T=... f=- companion-ai/<ch> <text>" + "# kplog session" header),
# or ai_trace.txt in the fallback shape ("<ms> companion-ai/<ch> <text>"); 0.2.0-probe and older wrote raw lines to ai_trace.txt.
# find_files picks the first of these that exists (core name first).
TRACE_NAMES = ("companion-ai_log.txt", "ai_trace.txt")
# ranged-ai 0.4.0 logs through KpLog: core file ranged-ai_log.txt (T=... prefix + "# kplog session" header), or ranged_ai_log.txt in the
# fallback shape ("<ms> ranged-ai/main ..."); 0.3.0 and older wrote raw lines to ranged_ai_log.txt. find_files picks one (see there).
RANGED_NAMES = ("ranged_ai_log.txt", "ranged-ai_log.txt")
QUOTE_MAX = 220

# ----------------------------------------------------------------------------------------------------------------------
# Regexes for the log formats (source refs: patch_manager_mods/...)
# ----------------------------------------------------------------------------------------------------------------------
# companion_ai_src/CompanionAI.cpp:474 (P) and :490 (A):  "t=%lu P %08x %s %s ev=%d d=%d | %s"
PA_RE = re.compile(r"^t=(\d+) ([PA]) ([0-9a-fA-F]{8}) (.+?) (\S+) ev=(-?\d+) d=(-?\d+) \| (.*)$")
# :617  "t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f max=%.1f los=%d ranged=%d area=%s tgt=%08x -> %s new=%08x (wouldSpin=%u)"
A1_RE = re.compile(r"^t=(\d+) A1 cr=([0-9a-fA-F]{8}) party=(\d+) raw=(-?\d+) eff=(-?\d+) reason=(\S+) .*? -> (.+?) new=([0-9a-fA-F]+) \(wouldSpin=(\d+)\)")
# :544  "t=%lu E2 FALLBACK cr=%08x %s party=%d combat=%d round=%d script=%s a1spin=%d lastA1=%ldms"
E2_RE = re.compile(r"^t=(\d+) E2 FALLBACK cr=([0-9a-fA-F]{8}) (\S+) party=(\d+) combat=(-?\d+) round=(-?\d+) script=(\S+) a1spin=(\d+) lastA1=(-?\d+)ms")
# :375  "t=%lu A1 summary: cr=%08x would-spin passes=%u/%ds ends=%u retargets=%u"
A1SUM_RE = re.compile(r"^t=(\d+) A1 summary: cr=([0-9a-fA-F]{8}) would-spin passes=(\d+)/(\d+)s ends=(\d+) retargets=(\d+)(?: strict=(\d+)/(\d+))?")
# :420  "t=%lu E1 SPIN cr=%08x %s party=%d stance=%d entries/s=%u ..."
E1SPIN_RE = re.compile(r"^t=(\d+) E1 SPIN cr=([0-9a-fA-F]{8}) (\S+) party=(\d+) stance=(-?\d+)")
E1END_RE = re.compile(r"^t=(\d+) E1 END cr=([0-9a-fA-F]{8}) (\S+) spin lasted ([\d.]+) s")
# :695  "t=%lu FM1 bumpee=%08x %s party=%d mover=%08x %s combat=%d tgt=%08x round=%d queue=[%s]%s -> %s%s"
FM1_RE = re.compile(r"^t=(\d+) FM1 bumpee=([0-9a-fA-F]{8}) (\S+) party=(\d+) .* -> (keep|clear)\b")
# :732  "t=%lu FM4 cr=%08x %s tid=%lu fi=%08x live: ... -> restored (mode 1) | reset kept (fm4=0) | no FollowInfo after save"
FM4_RE = re.compile(r"^t=(\d+) FM4 cr=([0-9a-fA-F]{8}) (\S+) tid=(\d+) fi=([0-9a-fA-F]+) .* -> (.*)$")
# :457  "# T1 first ShipBuild flip: tid=%lu (load tid=%lu) script=%s"   (no t= on this line)
T1_RE = re.compile(r"^# T1 first ShipBuild flip: tid=(\d+) \(load tid=(\d+)\) script=(\S*)")
# :352  "t=%lu E3 P1 calls/s=%.0f creatures=%d%s maxPerCreature/s=%.1f elapsed>=4000:%d%% (p1=%d interval=%d)"
E3_RE = re.compile(r"^t=(\d+) E3 P1 calls/s=([\d.]+) creatures=(\d+)(\+?) maxPerCreature/s=([\d.]+) elapsed>=4000:(\d+)% \(p1=(\d+) interval=(\d+)\)")
# :761 / :746  "t=%lu A4 ..."
A4_RE = re.compile(r"^t=(\d+) A4 (.*)$")
A3_RE = re.compile(r"^t=(\d+) A3 (.*)$")
# :365  "t=%lu S lines=%llu dropped=%llu ... | flips=%u ... a1{would=%u end=%u retarget=%u} ... fm4{snap=%u restore=%u} ..."
S_RE = re.compile(r"^t=(\d+) S lines=")
# :114 (cap marker), :120 (drop marker), :113 (repeat line)
CAP_RE = re.compile(r"^-- ai_trace: file cap (\d+) MB reached at t=(\d+)")
DROP_RE = re.compile(r"^-- ai_trace: (\d+) lines dropped by the per-second cap \((\d+)/s\) at t=(\d+)")
REPEAT_RE = re.compile(r"^t=(\d+) R previous line repeated (\d+)x")
# :821  "# companion-ai %s session %s t=%lu pid=%lu tid=%lu cwd=%s ini=%s"
HEADER_RE = re.compile(r"^# companion-ai (\S+) session (\d{4}-\d\d-\d\d \d\d:\d\d:\d\d) t=(\d+) pid=(\d+) tid=(\d+)")
# :173/:179  "# config (why) ..." / "# config fixes a1=%d a1_stances=%x ..."; :232 unparseable
CONFIG_RE = re.compile(r"^# config")
CONFIG_BAD_RE = re.compile(r"^# config: unparseable value for: (.*?) \(flags")
KV_RE = re.compile(r"([A-Za-z_][\w/]*)=(\S+)")

# ranged_ai_src/RangedAI.cpp:206 H1, :229 H2, :244 S1, :263 S2, :275/:283 S3, :319 H3, :330 S4 (all carry t=<tick ms>)
R_H1_RE = re.compile(r"^H1 t=(\d+) cr=([0-9a-fA-F]{8}) party=(\d+) pc=(\d+) stance=(-?\d+) ranged=(\d+) los=(-?\d+) .*?inRange=(\d+) .*?"
                     r"me=\(([-\d.]+),([-\d.]+),([-\d.]+)\) .*?lastBlocked=\(([-\d.]+),([-\d.]+),([-\d.]+)\) -> hold=(\d+)")
R_H2_RE = re.compile(r"^H2 t=(\d+) cr=([0-9a-fA-F]{8}) party=(\d+) pc=(\d+) stance=(-?\d+) ranged=(\d+) los=(-?\d+) "
                     r"me=\(([-\d.]+),([-\d.]+),([-\d.]+)\) lastBlocked=\(([-\d.]+),([-\d.]+),([-\d.]+)\) -> retarget=(\d+)")
R_H3_RE = re.compile(r"^H3 t=(\d+) cr=([0-9a-fA-F]{8}) stance=16 ranged=(\d+) inRange=(\d+) kite_approach=(\d+) -> (\d+)(.*)$")
R_S_RE = re.compile(r"^(S[1-4])\b(?: t=(\d+))?(.*)$")
R_S3_RE = re.compile(r"^S3 t=(\d+) stance click: id=0x([0-9a-fA-F]+) stance (-?\d+) -> (-?\d+) msg strref=(-?\d+)")
R_CFG_RE = re.compile(r"^config: hold=")
R_LOADED_RE = re.compile(r"^ranged-ai (\S+) loaded")

# inventory_sort_filter_src/InventorySortFilter.cpp:128 config, :264 populate, :276/:284 tab click, :365 flush, :357 flush-no-rows
INV_CFG_RE = re.compile(r"^config: sort=")
INV_POP_RE = re.compile(r"^populate: panel=([0-9a-fA-F]+) mode=(-?\d+) extra=(-?\d+) creature=([0-9a-fA-F]+)")
INV_FLUSH_RE = re.compile(r"^flush: panel=([0-9a-fA-F]+) creature=([0-9a-fA-F]+) rows=(\d+) seen=(\d+) dropped=(\d+) direct=(\d+) "
                          r"usable=(\d+) tint-candidates=(\d+) sort=(\d+) tint=(\d+) extra=(-?\d+) selected=(\d+) counter=(-?\d+)")
INV_FLUSH_EMPTY_RE = re.compile(r"^flush: no backpack rows this populate \(mode=(-?\d+) extra=(-?\d+)\)")
INV_CYCLE_RE = re.compile(r"^tab click: cycle -> state (\d+) \(engine mode (\d+), extra=(-?\d+)\)")
INV_ERR_RES = [("bad frame", re.compile(r"bad frame")), ("counter", re.compile(r"counter -?\d+ -> -?\d+")),
               ("caption bad", re.compile(r"^caption: text object bad")), ("overflow", re.compile(r"overflow at")),
               ("stale buffer", re.compile(r"stale buffer")), ("tint skipped", re.compile(r"tint skipped")),
               ("log cap", re.compile(r"log cap reached"))]
INV_STATE_NAMES = {0: "All", 1: "New", 2: "Usable", 3: "Upgradeable"}   # SortLogic.h:34

# steam log (Proton/Wine): the patcher prints through OutputDebugStringA, wine logs it as warn:debugstr:
APPLIED_RE = re.compile(r"\[KotorPatcher\] Applied (\w+) hook at 0x([0-9A-Fa-f]+)(?: -> (\S+))?")
LOADED_RE = re.compile(r"\[Config\] Loaded (\w+) hook: ([\w\-]+)(?: -> (\S+))? @ 0x([0-9A-Fa-f]+)")
CONSUMED_RE = re.compile(r"\[Config\] Parsed consumed_exit_address = 0x([0-9A-Fa-f]+) for hook at 0x([0-9A-Fa-f]+)")
PATCH_DLL_RE = re.compile(r'loaddll:build_module Loaded L"[^"]*\\\\patches\\\\([\w\-]+)\.dll"')
STEAM_TS_RE = re.compile(r"^(\d+\.\d+):")
PATCHER_ERR_RE = re.compile(r"(?i)\b(error|fail(?:ed|ure)?|cannot|could not|unable|invalid|mismatch|not found|abort|exception)\b")
EXC_CODE_RE = re.compile(r"dispatch_exception code=([0-9a-fA-F]+) ?(?:\((\w+)\))?")
UNHANDLED_RE = re.compile(r"(?i)unhandled exception|wine: unhandled|unhandled page fault")
PROBE_RE = re.compile(r"IsBad(?:Read|Write|Code)Ptr")
MSG_RE = re.compile(r'OutputDebugStringA "(.*?)(?:\\n)?"\s*$')
SEH_MSG_RE = re.compile(r'dispatch_exception "(\[.*?)(?:\\n)?"\s*$')


# ----------------------------------------------------------------------------------------------------------------------
# helpers
# ----------------------------------------------------------------------------------------------------------------------
def q(ln: int, text: str, name: str = "") -> str:
    text = text.rstrip("\r\n")
    if len(text) > QUOTE_MAX:
        text = text[:QUOTE_MAX] + " ..."
    return f"{name}:{ln}: {text}" if name else f"{ln}: {text}"


def quotes_block(items: list[str], cap: int = 4) -> list[str]:
    if not items:
        return []
    out = ["", "```"] + items[:cap]
    if len(items) > cap:
        out.append(f"... (+{len(items) - cap} more)")
    out.append("```")
    return out


def read_lines(path: Path):
    with open(path, "r", encoding="utf-8", errors="replace", newline=None) as f:
        for i, line in enumerate(f, 1):
            yield i, line.rstrip("\r\n")


def _read_trace_pieces(path: Path):
    """read_ranged_lines for the companion-ai trace (companion-ai_log.txt core, ai_trace.txt fallback or legacy raw). Yields (ln, kind, msg):
    'line' (msg = the old companion-ai text, KpLog prefix removed), 'header' (the '# kplog session' line), 'drop' (KpLog DROPPED;
    msg = 'DROPPED n (ring r, rate s)', RE_DROPPED matches it), 'mark' (KpLog MARK).
    Two differences from read_ranged_lines, both needed for the legacy raw '#' comments to keep working:
    (1) kplog_parse reads every '# ' line as a KpLog header and strips the '# ', which would hide '# config ...' and the
        '# companion-ai ... session' line (HEADER_RE). So a '#' line keeps its raw text as kind 'line', except '# kplog session'.
    (2) a KpLog MARK is a user label (KP_MARK / F9), not a lost line, so it is kind 'mark' and the readers ignore it."""
    for ln, line in read_lines(path):
        if line.startswith("#") and not kplog_parse.RE_SESSION.match(line):
            yield ln, "line", line
            continue
        d = kplog_parse.parse_line(line) if line else None
        if d is None:
            yield ln, "line", line
        elif d["kind"] == "header":
            yield ln, "header", d["msg"]
        elif d["kind"] == "mark":
            yield ln, "mark", d["msg"]
        elif d["kind"] == "dropped":
            yield ln, "drop", f"DROPPED {d['dropped']} (ring {d['ring']}, rate {d['rate']})"
        else:
            yield ln, "line", d["msg"]


TRACE_CONT_RE = re.compile(r"^t=\d+ (\+\+?) (.*)$")


def read_trace_lines(path: Path):
    """_read_trace_pieces with the continuation pieces re-joined: the core cuts a message at 232 bytes, so companion-ai 0.2.1 splits a longer
    line (S summaries, PRS, '# config') into pieces; every piece after the first reads `t=<ms> + <rest>` (joined with a space) or
    `t=<ms> ++ <rest>` (cut mid-word, joined directly). The joined line is reported at the first piece's line number."""
    pend = None   # [ln, kind, msg]
    for ln, kind, msg in _read_trace_pieces(path):
        m = TRACE_CONT_RE.match(msg) if kind == "line" else None
        if m and pend is not None and pend[1] == "line":
            pend[2] += ("" if m.group(1) == "++" else " ") + m.group(2)
            continue
        if pend is not None:
            yield tuple(pend)
        pend = [ln, kind, msg]
    if pend is not None:
        yield tuple(pend)


def read_ranged_lines(path: Path):
    """Yield (ln, kind, msg) for a ranged-ai log in any shape (legacy raw / KpLog fallback / KpLog core).
    kind: 'line' (msg = the 0.3.0 text, KpLog prefix and level tag removed), 'header' (a KpLog session header: a new session starts),
    'drop' (KpLog MARK / DROPPED line). Legacy raw lines and unparseable lines come back as kind 'line' with the text unchanged."""
    for ln, line in read_lines(path):
        d = kplog_parse.parse_line(line) if line else None
        if d is None:
            yield ln, "line", line
        elif d["kind"] == "header":
            yield ln, "header", d["msg"]
        elif d["kind"] in ("mark", "dropped"):
            yield ln, "drop", d["msg"]
        else:
            yield ln, "line", d["msg"]


def fmt_addr(a: int) -> str:
    return f"0x{a:08X}"


def parse_bitmask(tok: str) -> set[int]:
    try:
        v = int(tok, 16)
    except ValueError:
        return {11, 12, 16}
    return {i for i in range(32) if v >> i & 1}


def kv(line: str) -> dict[str, str]:
    return {m.group(1): m.group(2) for m in KV_RE.finditer(line)}


class Anoms:
    def __init__(self):
        self.items: list[tuple[str, str, str]] = []

    def add(self, sev: str, src: str, msg: str):
        self.items.append((sev, src, msg))


class Clock:
    """tick (GetTickCount ms) -> wall clock. Anchors: ai_trace header, else the steam log's mtime (approximate)."""

    def __init__(self):
        self.anchor_dt: dt.datetime | None = None
        self.anchor_tick: int | None = None
        self.source = "none"

    def set(self, when: dt.datetime, tick: int, source: str, override: bool = False):
        if self.anchor_dt is None or override:
            self.anchor_dt, self.anchor_tick, self.source = when, tick, source

    def wall(self, tick_ms: int) -> dt.datetime | None:
        if self.anchor_dt is None:
            return None
        return self.anchor_dt + dt.timedelta(milliseconds=tick_ms - self.anchor_tick)


class Window:
    def __init__(self, spec: str, clock: Clock):
        m = re.fullmatch(r"(\d{1,2}):(\d\d)-(\d{1,2}):(\d\d)", spec.strip())
        if not m:
            raise SystemExit(f"--window must be HH:MM-HH:MM, got {spec!r}")
        self.start = int(m.group(1)) * 60 + int(m.group(2))
        self.end = int(m.group(3)) * 60 + int(m.group(4))      # inclusive minute
        self.clock = clock
        self.spec = spec
        self.applied_any = False

    def active(self) -> bool:
        return self.clock.anchor_dt is not None

    def contains(self, tick_ms: int) -> bool:
        w = self.clock.wall(tick_ms)
        if w is None:
            return True
        m = w.hour * 60 + w.minute
        self.applied_any = True
        if self.start <= self.end:
            return self.start <= m <= self.end
        return m >= self.start or m <= self.end


def tick_str(clock: Clock, t: int) -> str:
    w = clock.wall(t)
    return f"t={t}" + (f" ({w:%H:%M:%S})" if w else "")


# ----------------------------------------------------------------------------------------------------------------------
# hook tomls
# ----------------------------------------------------------------------------------------------------------------------
def load_hooks(specs: list[str] | None) -> tuple[list[dict], list[str]]:
    """Returns (hooks, notes). Each hook: patch, address, type, function, consumed (int|None), toml."""
    notes: list[str] = []
    paths: list[tuple[str | None, Path]] = []
    if specs:
        for s in specs:
            if "=" in s and not os.path.exists(s):
                pid, p = s.split("=", 1)
                paths.append((pid, Path(p)))
            else:
                paths.append((None, Path(s)))
    else:
        paths = [(None, p) for p in DEFAULT_TOMLS]
    hooks: list[dict] = []
    for pid, p in paths:
        if not p.exists():
            notes.append(f"hooks toml not found: {p}")
            continue
        with open(p, "rb") as f:
            data = tomllib.load(f)
        if pid is None:
            man = p.parent / "manifest.toml"
            pid = p.parent.name
            if man.exists():
                try:
                    with open(man, "rb") as f:
                        pid = tomllib.load(f).get("patch", {}).get("id", pid)
                except tomllib.TOMLDecodeError:
                    pass
        n = 0
        for h in data.get("hooks", []):
            hooks.append({"patch": pid, "address": int(h["address"]), "type": str(h.get("type", "")).upper(),
                          "function": h.get("function", ""), "consumed": h.get("consumed_exit_address"), "toml": str(p)})
            n += 1
        exp = EXPECTED_HOOK_COUNTS.get(pid)
        if exp is not None and exp != n and not specs:
            notes.append(f"{pid}: toml has {n} hooks, task spec expects {exp}")
    return hooks, notes


# ----------------------------------------------------------------------------------------------------------------------
# steam log
# ----------------------------------------------------------------------------------------------------------------------
def analyze_steam(path: Path | None, hooks: list[dict], an: Anoms, clock: Clock) -> dict:
    R: dict = {"present": path is not None}
    if path is None:
        return R
    name = path.name
    applied: dict[int, list[tuple[int, str, str]]] = defaultdict(list)    # address -> [(line, type, function)]
    loaded: dict[int, tuple[int, str, str]] = {}                          # address -> (line, patch id, type)
    consumed: dict[int, tuple[int, int]] = {}                             # hook address -> (line, consumed addr)
    dlls: dict[str, int] = {}
    patcher_err: list[str] = []
    other_err = Counter()
    err_lines: list[str] = []
    n_err = 0
    exc_codes: Counter = Counter()
    exc_first: dict[str, str] = {}
    av_probe = 0
    av_other: list[str] = []
    pending_av: list[tuple[int, str]] = []         # (line, text) of AVs awaiting an IsBad*Ptr within 80 lines
    unhandled: list[str] = []
    ts_first = ts_last = None
    n_lines = 0
    detach = None
    last_msg = None
    for ln, line in read_lines(path):
        n_lines += 1
        if ln < 3 or line[:1].isdigit():
            m = STEAM_TS_RE.match(line)
            if m:
                ts = float(m.group(1))
                if ts_first is None:
                    ts_first = ts
                ts_last = ts
        if pending_av:
            if PROBE_RE.search(line):
                av_probe += len(pending_av)
                pending_av.clear()
            else:
                while pending_av and ln - pending_av[0][0] > 80:
                    av_other.append(q(pending_av[0][0], pending_av[0][1], name))
                    pending_av.pop(0)
        if "err:" in line and ":err:" in line:
            n_err += 1
            if len(err_lines) < 5:
                err_lines.append(q(ln, line, name))
        if "dispatch_exception code=" in line:
            m = EXC_CODE_RE.search(line)
            if m:
                code = m.group(1).lower().lstrip("0") or "0"
                if code not in BENIGN_EXC:
                    exc_codes[code] += 1
                    exc_first.setdefault(code, q(ln, line, name))
                    if code == "c0000005":
                        pending_av.append((ln, line))
        if "nhandled" in line and UNHANDLED_RE.search(line) and len(unhandled) < 5:
            unhandled.append(q(ln, line, name))
        if "loaddll:build_module" in line and "patches" in line:
            m = PATCH_DLL_RE.search(line)
            if m:
                dlls.setdefault(m.group(1), ln)
        if "[" not in line or ("OutputDebugStringA" not in line and "dispatch_exception \"[" not in line):
            continue
        m = MSG_RE.search(line)
        if not m:
            m = SEH_MSG_RE.search(line)
            if m and m.group(1) == last_msg:      # wine echoes each debug string through dispatch_exception: skip the twin
                continue
        if not m:
            continue
        text = m.group(1)
        last_msg = text
        if "DLL_PROCESS_DETACH" in text:
            detach = ln
        a = APPLIED_RE.search(text)
        if a:
            applied[int(a.group(2), 16)].append((ln, a.group(1).upper(), a.group(3) or ""))
            continue
        a = LOADED_RE.search(text)
        if a:
            loaded[int(a.group(4), 16)] = (ln, a.group(2), a.group(1).upper())
            continue
        a = CONSUMED_RE.search(text)
        if a:
            consumed[int(a.group(2), 16)] = (ln, int(a.group(1), 16))
            continue
        tag = re.match(r"\[(\w+)\]", text)
        if tag:
            if tag.group(1) in ("KotorPatcher", "Config", "Patcher") and PATCHER_ERR_RE.search(text):
                patcher_err.append(q(ln, text, name))
            elif re.search(r"(?i)\b(error|warning)\b", text):
                other_err[re.sub(r"[0-9a-fA-F]{6,}", "#", text)[:90]] += 1
    for l, t in pending_av:
        av_other.append(q(l, t, name))
    R.update(name=name, n_lines=n_lines, ts_first=ts_first, ts_last=ts_last, detach=detach)
    if ts_last is not None and path.exists():
        mt = dt.datetime.fromtimestamp(path.stat().st_mtime)
        clock.set(mt, int(ts_last * 1000), "steam log mtime (approximate)")

    # --- DETOUR table
    rows = []
    for h in hooks:
        ap = applied.get(h["address"], [])
        ld = loaded.get(h["address"])
        cs = consumed.get(h["address"])
        row = dict(h)
        row["applied"] = ap
        row["loaded"] = ld
        row["consumed_logged"] = cs
        row["status"] = "OK" if ap else ("LOADED-NOT-APPLIED" if ld else "MISSING")
        if ap and ap[0][1] != h["type"] and h["type"]:
            row["status"] = f"TYPE-MISMATCH ({ap[0][1]} vs toml {h['type']})"
        if ap and len(ap) > 1:
            row["status"] += f" (applied {len(ap)}x)"
        rows.append(row)
    R["rows"] = rows
    R["dlls"] = dlls
    patches = sorted({h["patch"] for h in hooks})
    R["patches"] = patches
    for p in patches:
        want = [r for r in rows if r["patch"] == p]
        miss = [r for r in want if not r["applied"]]
        if miss:
            sev = "FAIL"
            dll = f"{p}.dll {'loaded' if p in dlls else 'NOT loaded'}"
            an.add(sev, name, f"{p}: {len(miss)}/{len(want)} hooks have no 'Applied' line ({dll}): "
                              + ", ".join(fmt_addr(r['address']) for r in miss))
    exp_cons = [h for h in hooks if h["consumed"] is not None]
    got_cons = [(h, consumed.get(h["address"])) for h in exp_cons]
    R["consumed_expected"] = len(exp_cons)
    R["consumed_parsed"] = sum(1 for _, c in got_cons if c)
    R["consumed_wrong"] = [(h, c) for h, c in got_cons if c and c[1] != h["consumed"]]
    R["consumed_missing"] = [h for h, c in got_cons if not c]
    hook_addrs = {h["address"] for h in hooks}
    R["consumed_extra"] = [a for a in consumed if a not in hook_addrs and loaded.get(a, (0, ""))[1] in patches]
    if R["consumed_missing"] and any(applied.get(h["address"]) or loaded.get(h["address"]) for h in R["consumed_missing"]):
        an.add("FAIL", name, "consumed_exit_address not parsed for: " + ", ".join(
            f"{h['patch']}@{fmt_addr(h['address'])}" for h in R["consumed_missing"]))
    for h, c in R["consumed_wrong"]:
        an.add("FAIL", name, f"consumed_exit_address mismatch at {fmt_addr(h['address'])}: log {fmt_addr(c[1])}, toml {fmt_addr(h['consumed'])}")
    R["patcher_err"] = patcher_err
    R["other_err"] = other_err
    R["n_err"] = n_err
    R["err_lines"] = err_lines
    R["exc_codes"] = exc_codes
    R["exc_first"] = exc_first
    R["av_probe"] = av_probe
    R["av_other"] = av_other
    R["unhandled"] = unhandled
    if patcher_err:
        an.add("FAIL", name, f"{len(patcher_err)} KotorPatcher/Config error lines, first: {patcher_err[0]}")
    if unhandled:
        an.add("FAIL", name, f"{len(unhandled)}+ 'unhandled exception' lines, first: {unhandled[0]}")
    if av_other:
        an.add("WARN", name, f"{len(av_other)} access violation(s) not followed by an IsBad*Ptr probe (possible real fault), first: {av_other[0]}")
    for code, n in exc_codes.items():
        if code != "c0000005":
            an.add("WARN", name, f"exception code {code} x{n}, first: {exc_first[code]}")
    if detach is None and n_lines:
        an.add("INFO", name, "no DLL_PROCESS_DETACH line: the game may still be running or crashed (log copied live?)")
    return R


# ----------------------------------------------------------------------------------------------------------------------
# ai_trace.txt
# ----------------------------------------------------------------------------------------------------------------------
def analyze_trace(path: Path | None, win: Window | None, clock: Clock, an: Anoms, opts) -> dict:
    R: dict = {"present": path is not None}
    if path is None:
        return R
    name = path.name
    attack_re = re.compile(opts.attack_regex)
    R["name"] = name
    header = None
    cfg_lines: list[tuple[int, str]] = []
    cfg_bad: list[tuple[int, str]] = []
    cfg_notes: list[tuple[int, str]] = []
    cur_cfg: dict = {"a1": None, "a1_stances": {11, 12, 16}, "a1_party_only": 1}
    t1 = None
    fm4: list[dict] = []
    a1: list[dict] = []
    e2: dict[str, list[tuple[int, int]]] = defaultdict(list)           # cr -> [(t, line)]
    e2_total = Counter()
    a1sum: dict[str, Counter] = defaultdict(Counter)
    e1spin: list[dict] = []
    e1end = 0
    fm1 = Counter()
    fm1_q: list[str] = []
    a4: list[str] = []
    a3: list[str] = []
    e3: list[dict] = []
    caps: list[str] = []
    drops: list[tuple[int, int, str]] = []
    repeats = 0
    repeat_lines = 0
    last_s: dict | None = None
    last_s_line = ""
    pa_count: dict[str, Counter] = defaultdict(Counter)                 # cr label -> Counter(("P"/"A", script, ev))
    cr_tag: dict[str, str] = {}
    n_pa = 0
    marker_cr: dict[str, Counter] = defaultdict(Counter)                # marker key -> Counter(cr label)
    marker_total = Counter()
    marker_ex: dict[str, list[str]] = defaultdict(list)
    seq: dict[str, list[tuple]] = defaultdict(list)                     # cr label -> interesting events
    kite_events: list[dict] = []
    shout_t: dict[str, list[int]] = defaultdict(list)                   # cr -> ticks of 'Shout Clear 1800' lines
    poison_bad_t: dict[str, list[int]] = defaultdict(list)              # cr -> ticks of 'Returning Invalid Poison Object'
    other_counts = Counter()
    t_min = t_max = None
    skipped_by_window = 0
    n_lines = 0

    def label(oid: str, tag: str) -> str:
        oid = oid.lower()
        cr_tag.setdefault(oid, tag)
        return oid

    for ln, kind, line in read_trace_lines(path):
        if kind in ("header", "mark"):          # KpLog session line (the companion '# companion-ai ... session' line after it is the HEADER_RE one) / KpLog label: not companion lines
            continue
        n_lines += 1
        if kind == "drop":                      # KpLog DROPPED n: same meaning as a DROP_RE hit, count from the line
            drops.append((int(kplog_parse.RE_DROPPED.match(line).group(1)), ln, line))
            continue
        if not line:
            continue
        c0 = line[0]
        if c0 == "#":
            m = HEADER_RE.match(line)
            if m:
                header = dict(ver=m.group(1), when=m.group(2), tick=int(m.group(3)), pid=m.group(4), tid=int(m.group(5)), ln=ln, text=line)
                try:
                    clock.set(dt.datetime.strptime(m.group(2), "%Y-%m-%d %H:%M:%S"), int(m.group(3)), f"{name} session header", override=(clock.source != "--anchor"))
                except ValueError:
                    pass
                continue
            m = T1_RE.match(line)
            if m:
                t1 = dict(tid=int(m.group(1)), load_tid=int(m.group(2)), script=m.group(3), ln=ln, text=line)
                continue
            m = CONFIG_BAD_RE.match(line)
            if m:
                cfg_bad.append((ln, line))
                continue
            if CONFIG_RE.match(line):
                if line.startswith("# config fixes"):
                    d = kv(line)
                    cur_cfg["a1"] = int(d["a1"]) if d.get("a1", "").isdigit() else cur_cfg["a1"]
                    cur_cfg["a1_stances"] = parse_bitmask(d["a1_stances"]) if "a1_stances" in d else cur_cfg["a1_stances"]
                    cur_cfg["a1_party_only"] = int(d["a1_party_only"]) if d.get("a1_party_only", "").isdigit() else 1
                    cur_cfg["ln"] = ln
                    cfg_lines.append((ln, line))
                elif line.startswith("# config:"):
                    cfg_notes.append((ln, line))
                else:
                    cfg_lines.append((ln, line))
                continue
            other_counts["# other comment"] += 1
            continue
        if c0 == "-":
            m = CAP_RE.match(line)
            if m:
                caps.append(q(ln, line, name))
                continue
            m = DROP_RE.match(line)
            if m:
                drops.append((int(m.group(1)), ln, line))
                continue
            other_counts["-- other marker"] += 1
            continue
        if c0 != "t":
            other_counts["unrecognised"] += 1
            continue
        m = re.match(r"t=(\d+) (\S+)", line)
        if not m:
            other_counts["unrecognised"] += 1
            continue
        t = int(m.group(1))
        kind = m.group(2)
        if win and win.active() and not win.contains(t):
            skipped_by_window += 1
            continue
        t_min = t if t_min is None else min(t_min, t)
        t_max = t if t_max is None else max(t_max, t)

        if kind in ("P", "A"):
            m = PA_RE.match(line)
            if not m:
                other_counts["P/A unparsed"] += 1
                continue
            n_pa += 1
            cr = label(m.group(3), m.group(4))
            script, ev, text = m.group(5), m.group(6), m.group(8)
            pa_count[cr][(kind, script, ev)] += 1
            if kind == "P":
                for key, rx, _src in MARKER_RES:
                    mm = rx.search(text)
                    if mm:
                        marker_cr[key][cr] += 1
                        marker_total[key] += 1
                        if len(marker_ex[key]) < 3:
                            marker_ex[key].append(q(ln, line, name))
                        if key == "Kite step":
                            kite_events.append(dict(t=t, ln=ln, cr=cr, step=int(mm.group(1)), streak=int(mm.group(2) or 0), script=script, line=line))
            interesting = (CLEAR_RE.search(text) or SPELL_ID_RE.search(text) or POISON_START_RE.search(text) or POISON_INVALID_RE.search(text)
                           or ROUND_START_RE.search(text) or attack_re.search(text) or "Party E1:" in text or SHOUT_1800_RE.search(text)
                           or KNOCKED_OUT_RE.search(text) or AOE_RE.search(text) or MEDPACK_RE.search(text) or "Party H1:" in text
                           or "Party MEDPAC:" in text)
            if interesting:
                seq[cr].append((t, ln, kind, script, text))
                if SHOUT_1800_RE.search(text):
                    shout_t[cr].append(t)
                if POISON_INVALID_RE.search(text):
                    poison_bad_t[cr].append(t)
            continue
        if kind == "R":
            m = REPEAT_RE.match(line)
            if m:
                repeats += int(m.group(2))
                repeat_lines += 1
            continue
        if kind == "A1":
            m = A1_RE.match(line)
            if m:
                outcome = m.group(7).strip()
                a1.append(dict(t=t, ln=ln, cr=m.group(2).lower(), party=int(m.group(3)), raw=int(m.group(4)), eff=int(m.group(5)),
                               reason=m.group(6), outcome=("end" if outcome.startswith("end") else "retarget" if outcome.startswith("retarget")
                                                           else "spin"), outcome_text=outcome, new=m.group(8), line=line))
                continue
            m = A1SUM_RE.match(line)
            if m:
                c = a1sum[m.group(2).lower()]
                c["passes"] += int(m.group(3)); c["ends"] += int(m.group(5)); c["retargets"] += int(m.group(6)); c["lines"] += 1
                if "sample" not in c:
                    c["sample"] = (ln, line)
                continue
            other_counts["A1 unparsed"] += 1
            continue
        if kind == "E2":
            m = E2_RE.match(line)
            if m:
                cr = m.group(2).lower()
                cr_tag.setdefault(cr, m.group(3))
                e2[cr].append((t, ln))
                e2_total[cr] += 1
            continue
        if kind == "E1":
            m = E1SPIN_RE.match(line)
            if m:
                cr = label(m.group(2), m.group(3))
                e1spin.append(dict(t=t, ln=ln, cr=cr, party=int(m.group(4)), stance=int(m.group(5)), cfg=dict(cur_cfg), line=line))
                continue
            if E1END_RE.match(line):
                e1end += 1
            continue
        if kind == "FM1":
            m = FM1_RE.match(line)
            if m:
                fm1[m.group(5)] += 1
                if len(fm1_q) < 4:
                    fm1_q.append(q(ln, line, name))
            continue
        if kind == "FM4":
            m = FM4_RE.match(line)
            if m:
                fm4.append(dict(t=t, ln=ln, cr=m.group(2).lower(), tag=m.group(3), tid=int(m.group(4)), outcome=m.group(6), line=line))
            continue
        if kind == "A4":
            a4.append(q(ln, line, name))
            continue
        if kind == "A3":
            a3.append(q(ln, line, name))
            continue
        if kind == "E3":
            m = E3_RE.match(line)
            if m:
                e3.append(dict(t=t, ln=ln, calls=float(m.group(2)), creatures=int(m.group(3)), overflow=bool(m.group(4)), maxper=float(m.group(5)),
                               share=int(m.group(6)), p1=int(m.group(7)), interval=int(m.group(8)), line=line))
            continue
        if kind == "S" and S_RE.match(line):
            d = {k: int(v) for k, v in re.findall(r"(\w+)=(\d+)", line)}
            for grp, body in re.findall(r"(\w+)\{([^}]*)\}", line):
                for k, v in re.findall(r"(\w+)=(\d+)", body):
                    d[f"{grp}.{k}"] = int(v)
            last_s, last_s_line = d, q(ln, line, name)
            last_s["_ln"] = ln
            continue
        if kind in ("PR", "PRS"):   # Phase-4 probe lines: parsed by analyze_probes
            continue
        other_counts[f"t= {kind}"] += 1

    R.update(n_lines=n_lines, header=header, t1=t1, cfg_lines=cfg_lines, cfg_bad=cfg_bad, cfg_notes=cfg_notes,
             t_min=t_min, t_max=t_max, skipped_by_window=skipped_by_window, other_counts=other_counts)

    # ---- config anomalies
    for ln, line in cfg_bad:
        an.add("FAIL", name, f"{q(ln, line)}")
    for ln, line in cfg_notes:
        an.add("WARN", name, q(ln, line))
    if not cfg_lines and n_lines:
        an.add("WARN", name, "no '# config' lines found: the file is not an ai_trace.txt of this version, or is truncated")

    # ---- T1 vs FM4 tid
    R["fm4"] = fm4
    tid_bad = []
    if t1 and fm4:
        tid_bad = [f for f in fm4 if f["tid"] != t1["tid"]]
        if tid_bad:
            an.add("FAIL", name, f"FM4 tid mismatch: T1 tid={t1['tid']} but {len(tid_bad)}/{len(fm4)} FM4 lines differ "
                                 f"(tids {sorted({f['tid'] for f in tid_bad})}), first: {q(tid_bad[0]['ln'], tid_bad[0]['line'], name)}")
    R["tid_bad"] = tid_bad

    # ---- FM4 pairing
    R["last_s"] = last_s
    R["last_s_line"] = last_s_line
    fm4_cfg = None
    for ln, line in cfg_lines:
        if "fm4=" in line:
            mm = re.search(r"\bfm4=(\d+)", line)
            if mm:
                fm4_cfg = int(mm.group(1))
    R["fm4_cfg"] = fm4_cfg
    R["fm4_unpaired"] = None
    if last_s is not None and "fm4.snap" in last_s:
        snap, rest = last_s["fm4.snap"], last_s.get("fm4.restore", 0)
        if fm4_cfg == 1 and snap != rest:
            R["fm4_unpaired"] = snap - rest
            an.add("FAIL", name, f"FM4 snapshot/restore unpaired with fm4=1: S line says snap={snap} restore={rest} ({last_s_line})")
        elif fm4_cfg is None and snap != rest:
            an.add("WARN", name, f"FM4 snap={snap} restore={rest} in the last S line (fm4 config unknown)")
    nofi = [f for f in fm4 if "no FollowInfo" in f["outcome"]]
    if nofi:
        an.add("WARN", name, f"{len(nofi)} FM4 line(s) 'no FollowInfo after save' (restore skipped), first: {q(nofi[0]['ln'], nofi[0]['line'], name)}")
    if fm4_cfg == 1:
        notrest = [f for f in fm4 if "restored" not in f["outcome"] and "no FollowInfo" not in f["outcome"]]
        if notrest:
            an.add("WARN", name, f"{len(notrest)} FM4 line(s) not restored while fm4=1, first: {q(notrest[0]['ln'], notrest[0]['line'], name)}")

    # ---- A1 effectiveness
    win_ms = opts.e2_window_ms
    a1rows = []
    for r in a1:
        ts = [x[0] for x in e2.get(r["cr"], [])]
        i = bisect.bisect_left(ts, r["t"])
        hit = None
        if i < len(ts) and ts[i] - r["t"] <= win_ms:
            hit = e2[r["cr"]][i]
        budget_gone = e2_total.get(r["cr"], 0) > 20
        a1rows.append(dict(r, e2=hit, e2_budget_gone=budget_gone, delta=(hit[0] - r["t"]) if hit else None))
    R["a1"] = a1rows
    need = [r for r in a1rows if (r["outcome"] == "end" or opts.a1_strict)]
    R["a1_need"] = need
    R["a1_missing"] = [r for r in need if not r["e2"]]
    R["a1_missing_reliable"] = [r for r in R["a1_missing"] if not r["e2_budget_gone"]]
    R["a1sum"] = a1sum
    if R["a1_missing"]:
        rel = R["a1_missing_reliable"]
        an.add("FAIL" if rel else "WARN", name,
               f"{len(R['a1_missing'])}/{len(need)} A1 'end' line(s) have no E2 FALLBACK within {win_ms} ms (8 s combat-timeout risk)"
               + ("" if rel else " [E2 log budget >20 for those creatures: E2 lines are throttled, inconclusive]")
               + f", first: {q(R['a1_missing'][0]['ln'], R['a1_missing'][0]['line'], name)}")

    # ---- E1 SPIN with a1=1
    bad_spin = []
    for s in e1spin:
        c = s["cfg"]
        if c.get("a1") == 1 and s["party"] == 1 and s["stance"] in c.get("a1_stances", set()):
            bad_spin.append(s)
    R["e1spin"] = e1spin
    R["e1end"] = e1end
    R["e1spin_bad"] = bad_spin
    if bad_spin:
        an.add("FAIL", name, f"{len(bad_spin)} E1 SPIN line(s) for party stance in a1_stances while a1=1 (A1 should have ended the spin), "
                             f"first: {q(bad_spin[0]['ln'], bad_spin[0]['line'], name)}")
    elif any(s for s in e1spin if s["cfg"].get("a1") == 0):
        an.add("INFO", name, "E1 SPIN lines present while a1=0 (expected: fix disabled)")

    # ---- FM1/A4/P1/caps
    R["fm1"] = fm1
    R["fm1_q"] = fm1_q
    R["a4"] = a4
    R["a3"] = a3
    R["e3"] = e3
    R["caps"] = caps
    R["drops"] = drops
    R["repeats"] = (repeats, repeat_lines)
    if caps:
        an.add("WARN", name, f"file cap reached: {caps[0]} (later events are missing)")
    if drops:
        tot = sum(d[0] for d in drops)
        an.add("INFO", name, f"{tot} lines dropped by the per-second cap in {len(drops)} marker(s): P/A line counts are lower bounds")
    if a4:
        an.add("WARN", name, f"{len(a4)} A4 line(s) (NULL spell row guard fired), first: {a4[0]}")

    # ---- script lines
    R["pa_count"] = pa_count
    R["cr_tag"] = cr_tag
    R["n_pa"] = n_pa
    R["marker_cr"] = marker_cr
    R["marker_total"] = marker_total
    R["marker_ex"] = marker_ex
    R["kite_events"] = kite_events

    # recast anomaly
    recast = []
    for cr, ev in seq.items():
        clears = [i for i, e in enumerate(ev) if CLEAR_RE.search(e[4])]
        sigs = []
        for k, i in enumerate(clears):
            t, ln, _kind, script, text = ev[i]
            code = CLEAR_RE.search(text).group(1)
            nxt = clears[k + 1] if k + 1 < len(clears) else len(ev)
            prv = clears[k - 1] + 1 if k > 0 else 0
            spell = next((SPELL_ID_RE.search(e[4]).group(1) for e in ev[i + 1:nxt] if SPELL_ID_RE.search(e[4])), None)
            poison = any(POISON_START_RE.search(e[4]) for e in ev[prv:i]) and not any(POISON_INVALID_RE.search(e[4]) for e in ev[prv:i])
            sigs.append((code, spell, poison, t, ln))
        run: list[tuple] = []
        def flush_run():
            if len(run) > RECAST_MAX_ROUNDS and run[0][0] in HEAL_CODES:
                recast.append(dict(cr=cr, n=len(run), code=run[0][0], spell=run[0][1], poison=run[0][2], t0=run[0][3], ln0=run[0][4],
                                   t1=run[-1][3], ln1=run[-1][4]))
        for s in sigs:
            if run and s[:3] == run[0][:3]:
                run.append(s)
            else:
                flush_run()
                run = [s]
        flush_run()
    R["recast"] = recast
    for r in recast:
        who = f"{cr_tag.get(r['cr'], '-')}({r['cr']})"
        kindtxt = "poison-cure" if r["poison"] else "heal/cure"
        an.add("FAIL", name, f"recast pattern: {who} repeated Clear {r['code']} ({kindtxt}{', spell ' + r['spell'] if r['spell'] else ''}) "
                             f"{r['n']} rounds in a row, lines {r['ln0']}..{r['ln1']}")

    # E1 skipped too long
    skip_bad = []
    attack_re2 = attack_re
    for cr, ev in seq.items():
        last_attack = None
        run_start = None
        reported = False
        for (t, ln, kind, script, text) in ev:
            if "Party E1: skipped" in text:
                if run_start is None:
                    run_start = (t, ln)
                base = last_attack if last_attack else run_start
                gap = t - base[0]
                if gap > E1_SKIP_MAX_MS and not reported:
                    skip_bad.append(dict(cr=cr, gap=gap, ln=ln, t=t, base=base, had_attack=last_attack is not None))
                    reported = True
                continue
            if "Party E1:" in text:
                run_start = None; reported = False
                continue
            if attack_re2.search(text):
                last_attack = (t, ln); run_start = None; reported = False
    R["e1_skip_bad"] = skip_bad
    for s in skip_bad:
        an.add("FAIL", name, f"Party E1: skipped for {s['gap'] / 1000:.1f} s (> {E1_SKIP_MAX_MS // 1000} s) after "
                             f"{'the last attack line' if s['had_attack'] else 'the first skipped line (no attack line seen)'} "
                             f"for {cr_tag.get(s['cr'], '-')}({s['cr']}), line {s['ln']}")
    R["seq"] = seq
    for lst in list(shout_t.values()) + list(poison_bad_t.values()):
        lst.sort()
    R["shout_t"] = shout_t
    R["poison_bad_t"] = poison_bad_t
    R["knocked_out"] = sum(1 for ev in seq.values() for e in ev if KNOCKED_OUT_RE.search(e[4]))
    R["aoe"] = sum(1 for ev in seq.values() for e in ev if AOE_RE.search(e[4]))
    R["medpack"] = sum(1 for ev in seq.values() for e in ev if MEDPACK_RE.search(e[4]))
    return R


# ----------------------------------------------------------------------------------------------------------------------
# ranged_ai_log.txt
# ----------------------------------------------------------------------------------------------------------------------
def analyze_ranged(path: Path | None, win: Window | None, an: Anoms, cr_tag: dict) -> dict:
    R: dict = {"present": path is not None}
    if path is None:
        return R
    name = path.name
    s_counts = Counter()
    s_ex: dict[str, list[str]] = defaultdict(list)
    s3_to16 = []
    s_err: list[str] = []
    h1 = defaultdict(lambda: Counter())          # stance -> Counter(n, los0, hold, ...)
    h1_los0: dict[int, list[tuple[int, str]]] = defaultdict(list)
    h1_eq = Counter()                            # lastBlocked == me
    h2 = []
    h3 = Counter()
    h3_rows = []
    cfgs: list[tuple[int, str]] = []
    loaded: list[tuple[int, str]] = []
    cap = None
    skipped = 0
    n_lines = 0
    t_min = t_max = None
    drops: list[str] = []
    for ln, kind, line in read_ranged_lines(path):
        n_lines += 1
        if not line:
            continue
        if kind == "header":
            continue
        if kind == "drop":
            drops.append(q(ln, line, name))
            continue
        if line.startswith("-- log cap reached"):
            cap = q(ln, line, name)
            continue
        if R_CFG_RE.match(line):
            cfgs.append((ln, line))
            continue
        if R_LOADED_RE.match(line):
            loaded.append((ln, line))
            continue
        mt = re.search(r"\bt=(\d+)", line)
        t = int(mt.group(1)) if mt else None
        if t is not None and win and win.active() and not win.contains(t):
            skipped += 1
            continue
        if t is not None:
            t_min = t if t_min is None else min(t_min, t)
            t_max = t if t_max is None else max(t_max, t)
        m = R_H1_RE.match(line)
        if m:
            stance = int(m.group(5))
            los = int(m.group(7))
            c = h1[stance]
            c["n"] += 1
            c["los0"] += los == 0
            c["hold"] += int(m.group(15)) == 1
            c["los0_hold"] += (los == 0 and int(m.group(15)) == 1)
            c["ranged"] += int(m.group(6)) == 1
            c["inRange"] += int(m.group(8)) == 1
            if los == 0:
                h1_los0[stance].append((ln, line))
            if m.group(9, 10, 11) == m.group(12, 13, 14):
                h1_eq[(stance, los == 0)] += 1
            continue
        m = R_H2_RE.match(line)
        if m:
            h2.append(dict(ln=ln, t=int(m.group(1)), cr=m.group(2).lower(), stance=int(m.group(5)), los=int(m.group(7)),
                           eq=m.group(8, 9, 10) == m.group(11, 12, 13), retarget=int(m.group(14)), line=line))
            continue
        m = R_H3_RE.match(line)
        if m:
            key = (int(m.group(3)), int(m.group(4)), int(m.group(5)), int(m.group(6)))   # ranged, inRange, kite_approach, eff
            h3[key] += 1
            h3_rows.append(dict(ln=ln, t=int(m.group(1)), cr=m.group(2).lower(), ranged=key[0], inrange=key[1], ka=key[2], eff=key[3], line=line))
            continue
        m = R_S3_RE.match(line)
        if m:
            s_counts["S3"] += 1
            if len(s_ex["S3"]) < 3:
                s_ex["S3"].append(q(ln, line, name))
            if int(m.group(4)) == 16 and m.group(2).lower() == "415":
                s3_to16.append((ln, line))
            continue
        m = R_S_RE.match(line)
        if m:
            k = m.group(1)
            s_counts[k] += 1
            if len(s_ex[k]) < 3:
                s_ex[k].append(q(ln, line, name))
            if "bad frame" in line or "not writable" in line:
                s_err.append(q(ln, line, name))
            continue
    R.update(name=name, n_lines=n_lines, s_counts=s_counts, s_ex=s_ex, s3_to16=s3_to16, s_err=s_err, h1=h1, h1_los0=h1_los0, h1_eq=h1_eq, h2=h2,
             h3=h3, h3_rows=h3_rows, cfgs=cfgs, loaded=loaded, cap=cap, drops=drops, skipped=skipped, t_min=t_min, t_max=t_max)
    if len(loaded) > 1:
        an.add("INFO", name, f"{len(loaded)} 'ranged-ai loaded' lines: the file holds several sessions (the fallback file is opened in append mode); counts are totals")
    if drops:
        an.add("INFO", name, f"OBSERVE: {len(drops)} KpLog MARK/DROPPED line(s) (rate limit max_lines_per_sec; the 0.3.0 file had none), "
                             f"counts are lower bounds, first: {drops[0]}")
    if s_err:
        an.add("FAIL", name, f"stance menu errors: {s_err[0]}")
    if cap:
        an.add("WARN", name, f"log cap reached ({cap}); later events are missing")
    bad_eq = [h for h in h2 if not h["eq"]]
    if bad_eq:
        an.add("WARN", name, f"{len(bad_eq)}/{len(h2)} H2 line(s) with lastBlocked != me (H2 is reached on the SECOND blocked check at the same "
                             f"position, so equality is expected), first: {q(bad_eq[0]['ln'], bad_eq[0]['line'], name)}")
    return R


# ----------------------------------------------------------------------------------------------------------------------
# inventory_sort_filter_log.txt
# ----------------------------------------------------------------------------------------------------------------------
def analyze_inv(path: Path | None, an: Anoms) -> dict:
    R: dict = {"present": path is not None}
    if path is None:
        return R
    name = path.name
    cfg = []
    errs: dict[str, list[str]] = defaultdict(list)
    flushes = []
    empty_flushes = []
    cycles = []
    clicks = 0
    cur_pop = None
    n_lines = 0
    loaded = []
    for ln, line in read_lines(path):
        n_lines += 1
        s = line.strip()
        if not s:
            continue
        if INV_CFG_RE.match(s):
            cfg.append((ln, s)); continue
        if s.startswith("inventory-sort-filter") and "loaded" in s:
            loaded.append((ln, s)); continue
        m = INV_POP_RE.match(s)
        if m:
            cur_pop = dict(ln=ln, mode=int(m.group(2)), extra=int(m.group(3)), creature=m.group(4).lower())
            continue
        m = INV_FLUSH_RE.match(s)
        if m:
            mode = cur_pop["mode"] if cur_pop else None
            extra = int(m.group(11))
            state = None
            if mode == 0:
                state = {1: 2, 2: 3}.get(extra, 0)
            elif mode == 1:
                state = 1
            flushes.append(dict(ln=ln, creature=m.group(2).lower(), rows=int(m.group(3)), seen=int(m.group(4)), dropped=int(m.group(5)),
                                direct=int(m.group(6)), usable=int(m.group(7)), tint=int(m.group(8)), sort=int(m.group(9)), tint_on=int(m.group(10)),
                                extra=extra, state=state, mode=mode, line=s))
            continue
        m = INV_FLUSH_EMPTY_RE.match(s)
        if m:
            empty_flushes.append((ln, s)); continue
        if s.startswith("tab click:"):
            clicks += 1
            m = INV_CYCLE_RE.match(s)
            if m:
                cycles.append(dict(ln=ln, state=int(m.group(1)), line=s))
            continue
        for key, rx in INV_ERR_RES:
            if rx.search(s):
                errs[key].append(q(ln, s, name))
                break
    R.update(name=name, n_lines=n_lines, cfg=cfg, errs=errs, flushes=flushes, empty_flushes=empty_flushes, cycles=cycles, clicks=clicks, loaded=loaded)
    for key in ("bad frame", "counter", "caption bad", "overflow"):
        if errs.get(key):
            an.add("FAIL", name, f"{len(errs[key])} '{key}' error line(s), first: {errs[key][0]}")
    for key in ("stale buffer", "log cap"):
        if errs.get(key):
            an.add("WARN", name, f"{len(errs[key])} '{key}' line(s), first: {errs[key][0]}")
    if cfg and not flushes and not empty_flushes:
        an.add("INFO", name, "inventory log has a config line but no flush lines: the inventory screen was not opened")
    return R


# ----------------------------------------------------------------------------------------------------------------------
# Phase-4 probes (companion-ai 0.2.0-probe; engine_ai_atlas/PROBE_DESIGN.md section 2): PR / PRS lines of ai_trace.txt
# ----------------------------------------------------------------------------------------------------------------------
PR_CHECK_IDS = ["PR-FM2B", "PR-FM2", "PR-FM3", "PR-SLOT6", "PR-FM5", "PR-E9", "PR-P2", "PR-PF", "PR-RAR", "PR-SHOUT", "PR-SS", "PR-E4",
                "PR-PARTY", "PR-A1X", "PR-RAH2"]
_H8 = r"([0-9a-fA-F]{8})"
# companion_ai_src/CompanionAI.cpp (probe handlers); every regex is checked against the printf literal by test_analyze_play_logs.py
PR_FM2C_RE = re.compile(rf"^t=(\d+) PR FM2C cr={_H8} tgt={_H8} res=(-?\d+) code=(-?\d+) cached=(-?\d+) cp=(-?\d+) calls=(-?\d+) n2=(\d+) n3=(\d+)")
PR_FM2F_RE = re.compile(rf"^t=(\d+) PR FM2F cr={_H8} tgt={_H8} grp=(\d+) n=(\d+) span=(\d+)ms ctrl=(\d+)")
PR_FM2R_RE = re.compile(rf"^t=(\d+) PR FM2R cr={_H8} tgt={_H8} other=(-?\d+) inr=(-?\d+) range=(-?[\d.]+) d=(-?[\d.]+) prog=(-?[\d.]+) n=(\d+)")
PR_FM3L_RE = re.compile(rf"^t=(\d+) PR FM3L level=(-?\d+) sentinel={_H8} now=(-?\d+) removed=(\d+) done=(-?\d+) elapsed=(\d+) slice=(\d+)")
PR_FM3X_RE = re.compile(rf"^t=(\d+) PR FM3X id={_H8} level=(-?\d+) idx=(-?\d+) cursor=(-?\d+) sentinel=(\d+)")
PRS_FM3_RE = re.compile(r"^t=(\d+) PRS FM3 passes=(\d+) lost=(\d+) removes=(\d+) beforeCursor=(\d+)")
PR_RS_RE = re.compile(rf"^t=(\d+) PR RS site=(\w+) cr={_H8} (\S+) party=(\d+) empty=(\d+)(?: spk={_H8} spkParty=(-?\d+) pat=(-?\d+)(?: spkTag=(\S+))?)?")
PRS_E9_RE = re.compile(r"^t=(\d+) PRS E9 deliveries=(\d+)")
PRS_RS_RE = re.compile(rf"^t=(\d+) PRS RS cr={_H8} s6a=(\d+) s6b=(\d+) s6c=(\d+) s7=(\d+) s12=(\d+) s1=(\d+) max12ps=(\d+)")
PR_P2_RE = re.compile(rf"^t=(\d+) PR P2 cr={_H8} ctrl=(\d+) elapsed=(\d+) combat=(-?\d+)")
PRS_P2_RE = re.compile(rf"^t=(\d+) PRS P2 cr={_H8} runs=(\d+) avg=(\d+) max=(\d+) ctrlRuns=(\d+)")
PR_PF_RE = re.compile(rf"^t=(\d+) PR PF cr={_H8} (\S+) old=(-?\d+) new=(-?\d+) apply=(-?\d+) site=(\S+)")
PRS_PF_RE = re.compile(r"^t=(\d+) PRS PF saveA=(\d+) saveB=(\d+) xferA=(\d+) xferB=(\d+) other=(\d+)")
PRS_RAR_RE = re.compile(rf"^t=(\d+) PRS RAR cr={_H8} ispc=(-?\d+) n=(\d+)(?: (.*))?$")
PR_RAR_MOVE_RE = re.compile(rf"^t=(\d+) PR RAR MOVESTATE cr={_H8} type=([0-9a-fA-F]+) res=(-?\d+) state=(-?\d+)")
PR_RAR_REQ_RE = re.compile(rf"^t=(\d+) PR RAR REQ cr={_H8} type=([0-9a-fA-F]+) n=(\d+)")
PR_SHX_RE = re.compile(rf"^t=(\d+) PR SHX (\S+) spk={_H8} type=(-?\d+) (\S+) party=(-?\d+) r=(-?[\d.]+) seen=(-?\d+) idx=(-?\d+) count=(-?\d+) last={_H8} text=(.*)$")
PRS_SHX_RE = re.compile(r"^t=(\d+) PRS SHX shouts=(\d+) r35=(\d+) partySpk=(\d+) end=(\d+) xbreak=(\d+) noarea=(\d+) abortStartCr=(\d+) abortStartOther=(\d+) abortMid=(\d+)")
PR_SS1_RE = re.compile(rf"^t=(\d+) PR SS1 cr={_H8} (\S+) state (-?\d+)->(-?\d+) combat=(-?\d+)")
PR_GCA_RE = re.compile(rf"^t=(\d+) PR GCA cr={_H8} res=(-?\d+) lastRun=([0-9a-fA-F]+) moving=(-?\d+)")
PRS_GCA_RE = re.compile(rf"^t=(\d+) PRS GCA cr={_H8} (.*)$")
PR_PARTY_RE = re.compile(rf"^t=(\d+) PR PARTY why=(\w+) ctrl={_H8} f=(-?\d+)/(-?\d+) pobj={_H8} f=(-?\d+)/(-?\d+) pc={_H8} tbl=(-?\d+) \[(.*)\]$")
PR_A1X_STRICT_RE = re.compile(rf"^t=(\d+) PR A1X STRICT cr={_H8} new={_H8} d2=(-?[\d.]+) lim2=(-?[\d.]+) -> (ok|would-reject)")
PR_A1X_FOLLOW_RE = re.compile(rf"^t=(\d+) PR A1X FOLLOWUP cr={_H8} site=(\w+) after=(-?\d+)ms")
PR_A1X_ORPHAN_RE = re.compile(rf"^t=(\d+) PR A1X ORPHAN cr={_H8} since=(-?\d+)ms")
A1X_SUFFIX_RE = re.compile(r"\(wouldSpin=\d+\) c524=(-?\d+) f570=(-?\d+) combat=(-?\d+)")
CFG_RETHINK_RE = re.compile(r"\ba1_rethink_ms=(\d+)")
CFG_SUMMARY_RE = re.compile(r"\bsummary_sec=(\d+)")
DROP_SPAN_MS = 2000   # a per-second-cap drop marker is written at the next admitted line: the dropped lines lie within the 2 s before it
BUDGET_RE = re.compile(r"^-- probe (\w+) line budget reached --")
R_H1_END52C_RE = re.compile(r"^H1 t=(\d+) cr=([0-9a-fA-F]{8}) .*?inRange=(\d+) .*?end52c=([0-9a-fA-F]+)")
SS_SET_STATES = {1} | set(range(3, 11)) | set(range(0xE, 0x14))     # PR-SS: SetState targets that remove the slot-6 script (S6C)
E4_RUNS = {0xC, 0x11, 0x13, 0x3F}                                   # PR-E4: engine-driven approach actions
PR_SS_WINDOW_MS = 60000
PR_P2_PASS = (3900, 4500)
PR_P2_FAIL_BELOW = 3500


def parse_rar_hist(s: str) -> dict[int, tuple[int, int, int, int, int]]:
    """'c:1/2/3/4' or 'c:1/2/3/4+o5' tokens -> {type: (r1, r2, r3, r4, other)} (type in hex; 0 = the out-of-range bucket)."""
    out: dict[int, tuple[int, int, int, int, int]] = {}
    for m in re.finditer(r"([0-9a-fA-F]+):(\d+)/(\d+)/(\d+)/(\d+)(?:\+o(\d+))?", s or ""):
        out[int(m.group(1), 16)] = tuple(int(m.group(i)) for i in range(2, 6)) + (int(m.group(6) or 0),)
    return out


def analyze_probes(trace_path: Path | None, ranged_path: Path | None, win: Window | None, an: Anoms) -> dict:
    """The PR-* checks. Each result: id -> (status PASS|FAIL|OBSERVE, summary, evidence). PASS = the data agrees with the atlas (no action),
    FAIL = it contradicts the atlas or shows a bug's reach (act on it), OBSERVE = the case did not occur."""
    R: dict = {"present": trace_path is not None, "checks": {}, "prs_last": {}, "n_pr": 0, "n_prs": 0, "budget": []}
    budget_ln: dict[str, int] = {}        # probe -> line number of its "line budget reached" marker (later PR lines of that probe are missing)
    budget_t: dict[str, int] = {}         # probe -> t of the last t= line before the marker (a lower bound of the moment the budget ran out)
    drop_t: list[int] = []                # t of every "-- ai_trace: N lines dropped" marker (lines were lost in the 2 s before it)
    summary_ms = 10000
    a1_strict = [0, 0]                    # A1 summary strict=N/rejected totals (budget-free)
    prev_t = 0
    name = trace_path.name if trace_path else FILES["trace"]
    L = defaultdict(list)                 # kind -> [dict]
    cfg_rethink = 3000
    t_min = t_max = None
    last_prs: dict[tuple[str, str], tuple[int, str]] = {}
    if trace_path:
        for ln, kind, line in read_trace_lines(trace_path):
            if kind in ("header", "mark"):
                continue
            if kind == "drop":                  # KpLog DROPPED has no companion t= of its own: the last t= line before it stands in
                drop_t.append(prev_t)
                continue
            if not line:
                continue
            if line[0] == "#":
                if line.startswith("# config fixes"):
                    mm = CFG_RETHINK_RE.search(line)
                    if mm:
                        cfg_rethink = int(mm.group(1))
                elif line.startswith("# config (") or line.startswith("# config trace"):
                    mm = CFG_SUMMARY_RE.search(line)
                    if mm:
                        summary_ms = int(mm.group(1)) * 1000
                continue
            if line[0] == "-":
                mm = BUDGET_RE.match(line)
                if mm:
                    R["budget"].append(mm.group(1))
                    budget_ln.setdefault(mm.group(1), ln)
                    budget_t.setdefault(mm.group(1), prev_t)
                else:
                    mm = DROP_RE.match(line)
                    if mm:
                        drop_t.append(int(mm.group(3)))
                continue
            mt = re.match(r"t=(\d+) (\S+)", line)
            if not mt:
                continue
            t = int(mt.group(1))
            prev_t = t
            if win and win.active() and not win.contains(t):
                continue
            kind = mt.group(2)
            t_min = t if t_min is None else min(t_min, t)
            t_max = t if t_max is None else max(t_max, t)
            if kind == "A1":
                m = A1_RE.match(line)
                if m:
                    outcome = m.group(7).strip()
                    L["A1"].append(dict(t=t, ln=ln, cr=m.group(2).lower(), outcome=("end" if outcome.startswith("end") else
                                                                                        "retarget" if outcome.startswith("retarget") else "spin"),
                                        probe=bool(A1X_SUFFIX_RE.search(line)), line=line))
                else:
                    m = A1SUM_RE.match(line)
                    if m and m.group(7):
                        a1_strict[0] += int(m.group(7)); a1_strict[1] += int(m.group(8))
                continue
            if kind not in ("PR", "PRS"):
                continue
            R["n_pr" if kind == "PR" else "n_prs"] += 1
            for key, rx in _PR_PARSERS:
                m = rx.match(line)
                if m:
                    L[key].append(dict(t=t, ln=ln, m=m, line=line))
                    if key.startswith("PRS_"):
                        last_prs[(key, (m.group(2).lower() if key in ("PRS_RS", "PRS_P2", "PRS_RAR", "PRS_GCA") else ""))] = (ln, line)
                    break
            else:
                R.setdefault("unparsed", []).append(q(ln, line, name))
    R["prs_last"] = last_prs
    ev = lambda d: f"{name}:{d['ln']}"                                   # noqa: E731
    C: dict[str, tuple[str, str, str]] = {}

    def judgeable(ln: int, t_from: int, t_to: int, probes: tuple[str, ...]) -> bool:
        """Can the ABSENCE of a line of `probes` in [t_from, t_to] (event at line `ln`) be trusted? No once one of the probes' line budgets
        ran out before the end of the window (marker line / the last t= before it), and no if a per-second-cap drop marker overlaps it."""
        for pr in probes:
            if pr in budget_ln and (ln >= budget_ln[pr] or t_to >= budget_t[pr]):
                return False
        return not any(dt >= t_from and dt - DROP_SPAN_MS <= t_to for dt in drop_t)

    # ---- PR-FM2B: FM2C partial (2) vs failed (3)
    rows = L["FM2C"]
    if not rows:
        C["PR-FM2B"] = ("OBSERVE", "no FM2C lines (no failed path search: the grid closest result succeeds, kill FM2b)", "")
    else:
        bad = [r for r in rows if int(r["m"].group(4)) not in (2, 3) or int(r["m"].group(2), 16) == 0]
        n2 = sum(1 for r in rows if int(r["m"].group(4)) == 2)
        n3 = sum(1 for r in rows if int(r["m"].group(4)) == 3)
        cached = sum(1 for r in rows if int(r["m"].group(6)) == -1)
        txt = f"{len(rows)} FM2C line(s): res=2 x{n2}, res=3 x{n3}, cached=-1 x{cached}"
        if bad:
            C["PR-FM2B"] = ("FAIL", f"{txt}; {len(bad)} with res outside {{2,3}} or cr 00000000, first: {q(bad[0]['ln'], bad[0]['line'], name)}", ev(bad[0]))
        else:
            C["PR-FM2B"] = ("PASS", txt + (" (only res=2 lines for a looping pair would mean a creeping-partial loop: see PR-FM2)" if n2 and not n3 else ""), ev(rows[0]))

    # ---- PR-FM2: re-approach pairs with n >= 3 (FM2R) need an FM2C/FM2F line for that creature in the same 30 s
    pair_n: dict[tuple[str, str], dict] = {}
    for r in L["FM2R"]:
        key = (r["m"].group(2).lower(), r["m"].group(3).lower())
        n = int(r["m"].group(9))
        if key not in pair_n or n > pair_n[key]["n"]:
            pair_n[key] = dict(n=n, t=r["t"], ln=r["ln"], prog=float(r["m"].group(8)), line=r["line"])
    loops = {k: v for k, v in pair_n.items() if v["n"] >= 3}
    if not loops:
        C["PR-FM2"] = ("OBSERVE", f"no (creature, target) pair with n >= 3 in the FM2R lines ({len(pair_n)} pair(s) seen)", "")
    else:
        miss, unjudged = [], 0
        for (cr, tgt), v in loops.items():
            near = [r for k in ("FM2C", "FM2F") for r in L[k] if r["m"].group(2).lower() == cr and abs(r["t"] - v["t"]) <= 30000]
            if near:
                continue
            if judgeable(v["ln"], v["t"] - 30000, v["t"] + 30000, ("FM2C", "FM2F")):
                miss.append((cr, tgt, v))
            else:
                unjudged += 1   # the FM2C/FM2F line budget ran out or lines were dropped: the absence proves nothing
        txt = f"{len(loops)} pair(s) with n >= 3 (max n {max(v['n'] for v in loops.values())})"
        if miss:
            cr, tgt, v = miss[0]
            C["PR-FM2"] = ("FAIL", f"{txt}; {len(miss)} without any FM2C/FM2F line for that creature within 30 s (contradicts gate 1.1), first: "
                                  f"{q(v['ln'], v['line'], name)}", f"{name}:{v['ln']}")
        elif unjudged:
            C["PR-FM2"] = ("OBSERVE", f"{txt}; {unjudged} without an FM2C/FM2F line but not judged (probe line budget reached or lines dropped by the "
                                      "per-second cap in that window)", "")
        else:
            first = next(iter(loops.values()))
            C["PR-FM2"] = ("PASS", txt + ", each with FM2C or FM2F lines for the creature in the same 30 s", f"{name}:{first['ln']}")

    # ---- PR-FM3
    bad_l = [r for r in L["FM3L"] if int(r["m"].group(4)) == int(r["m"].group(2)) and int(r["m"].group(5)) == 0]
    prs = L["PRS_FM3"]
    if bad_l:
        C["PR-FM3"] = ("FAIL", f"{len(bad_l)} FM3L line(s) with now == level and removed=0 (not a lost sentinel): {q(bad_l[0]['ln'], bad_l[0]['line'], name)}", ev(bad_l[0]))
    elif prs and int(prs[-1]["m"].group(2)) > 0:
        m = prs[-1]["m"]
        span_min = ((t_max - t_min) / 60000.0) if t_min is not None and t_max is not None and t_max > t_min else 0.0
        rate = f", {int(m.group(3)) / span_min:.2f} lost/min over {span_min:.1f} min" if span_min else ""
        C["PR-FM3"] = ("PASS", f"passes={m.group(2)} lost={m.group(3)} removes={m.group(4)} beforeCursor={m.group(5)}{rate} "
                              "(kill if < 1 LOST/min in crowded fights; >= 1/min goes to the user)", ev(prs[-1]))
    else:
        C["PR-FM3"] = ("OBSERVE", "no PRS FM3 line with passes > 0", "")

    # ---- PR-SLOT6 / PR-FM5 / PR-E9 (RS)
    rs = L["RS"]
    s6 = [r for r in rs if r["m"].group(2) in ("S6A", "S6B") and r["m"].group(5) == "1"]
    if s6:
        mix = Counter(r["m"].group(2) for r in rs if r["m"].group(5) == "1" and r["m"].group(2) in ("S6A", "S6B", "S6C"))
        C["PR-SLOT6"] = ("PASS", "slot-6 site mix for party members: " + ", ".join(f"{k} x{v}" for k, v in sorted(mix.items())), ev(s6[0]))
    else:
        C["PR-SLOT6"] = ("OBSERVE", "no S6A/S6B RS line for a party member", "")
    last_by_cr: dict[str, dict] = {}
    for r in L["PRS_RS"]:
        last_by_cr[r["m"].group(2).lower()] = r
    with12 = [r for r in last_by_cr.values() if int(r["m"].group(7)) > 0]
    if not with12:
        C["PR-FM5"] = ("OBSERVE", "no OnBlocked (S12) runs in any PRS RS line", "")
    else:
        top = max(with12, key=lambda r: int(r["m"].group(9)))
        mx = int(top["m"].group(9))
        C["PR-FM5"] = ("PASS" if mx < 5 else "FAIL",
                       f"max OnBlocked runs/s = {mx} for {top['m'].group(2).lower()} over {len(with12)} creature(s) with S12 runs "
                       + ("(< 5/s: kill the FM5 throttle)" if mx < 5 else "(>= 5/s: promote the FM5 throttle)"), ev(top))
    # PR-E9: PRS E9 = the budget-free count of pat-15 OnShout deliveries from non-party creatures to party listeners (printed at the first summary
    # even when 0, then on change); the S7 lines only name the speakers. Allied or hostile is not knowable from the log: the tags are reported.
    e9_prs = L["PRS_E9"]
    e9_n = int(e9_prs[-1]["m"].group(2)) if e9_prs else None
    s7 = [r for r in rs if r["m"].group(2) == "S7" and r["m"].group(7) is not None and r["m"].group(9) == "15" and r["m"].group(8) == "0" and r["m"].group(5) == "1"]
    tags = Counter((r["m"].group(10) or "?") for r in s7)
    nonparty35 = [r for r in L["SHX"] if r["m"].group(4) == "5" and r["m"].group(6) == "0" and abs(float(r["m"].group(7)) - 35.0) < 0.5]
    if e9_n:
        C["PR-E9"] = ("PASS", f"{e9_n} pat-15 shout delivery(ies) from non-party creatures to party listeners (PRS E9); S7 speaker tag(s): "
                              + (", ".join(f"{k} x{v}" for k, v in sorted(tags.items())) if tags else "none logged")
                              + " (mark which are allies, enemy shouts count here too)", ev(e9_prs[-1]))
    elif e9_n == 0 and nonparty35:
        C["PR-E9"] = ("FAIL", f"SHX shows {len(nonparty35)} r=35 non-party creature shout(s) but PRS E9 deliveries=0", ev(nonparty35[0]))
    elif e9_n == 0:
        C["PR-E9"] = ("OBSERVE", "PRS E9 deliveries=0 and no non-party r=35 creature shout seen: the allied case did not occur", "")
    elif nonparty35:
        C["PR-E9"] = ("OBSERVE", f"{len(nonparty35)} non-party r=35 creature shout(s) but no PRS E9 line (probe_rs off or the session ended before the first summary)", "")
    else:
        C["PR-E9"] = ("OBSERVE", "no allied (non-party, r=35) shouts seen", "")

    # ---- PR-P2
    last_p2: dict[str, dict] = {}
    for r in L["PRS_P2"]:
        last_p2[r["m"].group(2).lower()] = r
    sized = {cr: r for cr, r in last_p2.items() if int(r["m"].group(3)) >= 3 and int(r["m"].group(4)) > 0}
    ctrl_runs = sum(int(r["m"].group(6)) for r in last_p2.values())
    if not sized:
        C["PR-P2"] = ("OBSERVE", f"no creature with >= 3 P2 runs (ctrlRuns total {ctrl_runs})" if last_p2 else "no PRS P2 lines", "")
    else:
        low = [r for r in sized.values() if int(r["m"].group(4)) < PR_P2_FAIL_BELOW]
        avgs = ", ".join(f"{cr}={r['m'].group(4)}" for cr, r in sorted(sized.items()))
        if low:
            C["PR-P2"] = ("FAIL", f"avg gap below {PR_P2_FAIL_BELOW} ms (companion perception faster than 4 s): {avgs}; ctrlRuns {ctrl_runs}", ev(low[0]))
        elif all(PR_P2_PASS[0] <= int(r["m"].group(4)) <= PR_P2_PASS[1] for r in sized.values()):
            C["PR-P2"] = ("PASS", f"every companion avg in [{PR_P2_PASS[0]},{PR_P2_PASS[1]}] ms: {avgs}; ctrlRuns {ctrl_runs} "
                                  "(> 0 supports keying p2 on the controlled id)", ev(next(iter(sized.values()))))
        else:
            C["PR-P2"] = ("OBSERVE", f"avg gaps outside [{PR_P2_PASS[0]},{PR_P2_PASS[1]}] but not below {PR_P2_FAIL_BELOW}: {avgs}; ctrlRuns {ctrl_runs}", "")

    # ---- PR-PF
    pfl, pfs = L["PF"], L["PRS_PF"]
    if not pfl and not pfs:
        C["PR-PF"] = ("OBSERVE", "no PF lines", "")
    else:
        unk = [r for r in pfl if r["m"].group(7) == "?"]
        m = pfs[-1]["m"] if pfs else None
        unbalanced = m is not None and int(m.group(2)) != int(m.group(3))
        txt = (f"saveA={m.group(2)} saveB={m.group(3)} xferA={m.group(4)} xferB={m.group(5)} other={m.group(6)}" if m else "no PRS PF line") + f", {len(pfl)} PR PF line(s)"
        if unk or unbalanced:
            why = (f"{len(unk)} line(s) with site=? (caller not in the 15-site table)" if unk else "saveA != saveB")
            C["PR-PF"] = ("FAIL", f"{txt}; {why}" + (f", first: {q(unk[0]['ln'], unk[0]['line'], name)}" if unk else ""), ev(unk[0]) if unk else ev(pfs[-1]))
        else:
            C["PR-PF"] = ("PASS", txt, ev(pfs[-1]) if pfs else ev(pfl[0]))

    # ---- PR-RAR
    rar = L["PRS_RAR"]
    if not rar:
        C["PR-RAR"] = ("OBSERVE", "no PRS RAR lines", "")
    else:
        last_rar: dict[str, dict] = {}
        for r in rar:
            last_rar[r["m"].group(2).lower()] = r
        badr = []
        for cr, r in last_rar.items():
            h = parse_rar_hist(r["m"].group(5))
            if 0 in h and (h[0][0] or h[0][1] or h[0][3] or h[0][4]):
                badr.append(r)
        mv, rq = len(L["RAR_MOVE"]), len(L["RAR_REQ"])
        txt = f"{len(last_rar)} creature histogram(s); MOVESTATE x{mv}, REQ x{rq}"
        if badr:
            C["PR-RAR"] = ("FAIL", f"{txt}; type 0 / > 0x47 with a result other than 3: {q(badr[0]['ln'], badr[0]['line'], name)}", ev(badr[0]))
        else:
            C["PR-RAR"] = ("PASS", txt, ev(rar[-1]))

    # ---- PR-SHOUT: the bug's reach = a creature speaker's ABORT-START (stale id in the area list) or any ABORT-MID. A non-creature speaker
    # outside the list (abortStartOther) is the vanilla no-listener case: INFO only.
    shl, shs = L["SHX"], L["PRS_SHX"]
    aborts = [r for r in shl if r["m"].group(2) == "ABORT-MID" or (r["m"].group(2) == "ABORT-START" and r["m"].group(4) == "5")]
    other_lines = sum(1 for r in shl if r["m"].group(2) == "ABORT-START" and r["m"].group(4) != "5")
    m = shs[-1]["m"] if shs else None
    ab_counts = (int(m.group(8)) + int(m.group(10))) if m else 0
    other = int(m.group(9)) if m else other_lines
    if other:
        an.add("INFO", name, f"SHX: {other} ABORT-START with a non-creature speaker outside the area list (vanilla no-listener case, not the AI-SHOUTABORT bug)")
    if aborts or ab_counts:
        txt = (f"abortStartCr={m.group(8)} abortMid={m.group(10)} (abortStartOther={m.group(9)}) of {m.group(2)} shouts" if m else f"{len(aborts)} creature ABORT line(s)")
        C["PR-SHOUT"] = ("FAIL", f"{txt} (AI-SHOUTABORT reach: promote the 0x547203 fix)" + (f", first: {q(aborts[0]['ln'], aborts[0]['line'], name)}" if aborts else ""),
                         ev(aborts[0]) if aborts else ev(shs[-1]))
    elif m and int(m.group(2)) > 0:
        C["PR-SHOUT"] = ("PASS", f"shouts={m.group(2)} r35={m.group(3)} partySpk={m.group(4)} end={m.group(5)} xbreak={m.group(6)} noarea={m.group(7)}, "
                                 f"no creature ABORT (abortStartOther={m.group(9)}: INFO)", ev(shs[-1]))
    else:
        C["PR-SHOUT"] = ("OBSERVE", "no shouts seen", "")

    # ---- PR-SS: a party SetState to a slot-6-removing state needs an S6C run for that creature within 60 s. Judged from the budget-free
    # PRS RS s6c counters when the log has any (a rise in a PRS line within the window); the PR S6C lines are evidence, and the only source
    # (judged under the budget / drop rules) when no PRS RS line exists.
    ss = [r for r in L["SS1"] if int(r["m"].group(5)) in SS_SET_STATES]
    if not ss:
        C["PR-SS"] = ("OBSERVE", f"no SS1 line to a state in {{1,3..10,0xE..0x13}} ({len(L['SS1'])} SS1 line(s))", "")
    else:
        s6c: dict[str, list[int]] = defaultdict(list)
        for r in rs:
            if r["m"].group(2) == "S6C":
                s6c[r["m"].group(3).lower()].append(r["t"])
        rises: dict[str, list[int]] = defaultdict(list)       # cr -> t of every PRS RS line whose s6c count rose (or restarted after an eviction)
        prev_s6c: dict[str, int] = {}
        for r in L["PRS_RS"]:
            cr, n = r["m"].group(2).lower(), int(r["m"].group(5))
            if n > prev_s6c.get(cr, 0) or (n < prev_s6c.get(cr, 0) and n > 0):
                rises[cr].append(r["t"])
            prev_s6c[cr] = n
        prs_mode = bool(L["PRS_RS"])
        missing, judged, unjudged = [], 0, 0
        for r in ss:
            cr, t0 = r["m"].group(2).lower(), r["t"]
            end_prs = t0 + PR_SS_WINDOW_MS + summary_ms
            if not prs_mode and len(s6c.get(cr, ())) >= 20:       # no PRS to count with and the per-creature S6C line cap (20) is used up
                unjudged += 1
                continue
            if any(t0 <= x <= t0 + PR_SS_WINDOW_MS for x in s6c.get(cr, ())) or any(t0 < x <= end_prs for x in rises.get(cr, ())):
                judged += 1                                       # an S6C run is on record (PR line or PRS counter)
                continue
            if t_max is not None and (end_prs if prs_mode else t0 + PR_SS_WINDOW_MS) > t_max:
                unjudged += 1                                     # the log ends inside the window
            elif prs_mode or judgeable(r["ln"], t0, t0 + PR_SS_WINDOW_MS, ("RS",)):
                judged += 1; missing.append(r)
            else:
                unjudged += 1                                     # RS line budget reached / lines dropped: the missing PR S6C line proves nothing
        if missing:
            C["PR-SS"] = ("FAIL", f"{len(missing)}/{judged} judged SS1 line(s) without an S6C run for that creature within 60 s, first: "
                                  f"{q(missing[0]['ln'], missing[0]['line'], name)}", ev(missing[0]))
        elif judged:
            C["PR-SS"] = ("PASS", f"all {judged} judged SS1 line(s) were followed by an S6C run within 60 s" + (f" ({unjudged} not judged)" if unjudged else ""), ev(ss[0]))
        else:
            C["PR-SS"] = ("OBSERVE", f"{len(ss)} SS1 line(s) but none judgeable (RS line budget reached, lines dropped, or the log ends inside the 60 s window)", "")

    # ---- PR-E4: GetCurrentAction vs the last RunActions type
    gca = L["GCA"]
    e4_fail = [r for r in gca if int(r["m"].group(3)) == 0 and int(r["m"].group(4), 16) in E4_RUNS]
    e4_pass = [r for r in gca if int(r["m"].group(3)) in (3, 39) and int(r["m"].group(4), 16) in E4_RUNS]
    if e4_fail:
        C["PR-E4"] = ("FAIL", f"{len(e4_fail)} GCA line(s) with res 0 right after an engine approach action (contradicts lesson 325), first: "
                              f"{q(e4_fail[0]['ln'], e4_fail[0]['line'], name)}", ev(e4_fail[0]))
    elif e4_pass:
        C["PR-E4"] = ("PASS", f"{len(e4_pass)} GCA line(s) with res 3/39 after RunActions type in {{0xC,0x11,0x13,0x3F}}", ev(e4_pass[0]))
    else:
        C["PR-E4"] = ("OBSERVE", f"no GCA line with res 3/39 after an engine approach action ({len(gca)} GCA line(s))", "")

    # ---- PR-PARTY (why=pf lines come from the ADDMEMBER/SWITCHPC/SELECT/CMD sites, whose member flags are in transition: not judged)
    pl = L["PARTY"]
    if not pl:
        C["PR-PARTY"] = ("OBSERVE", "no PARTY lines", "")
    else:
        badp, judged, pobj_diff = [], 0, 0
        for r in pl:
            g = r["m"]
            if g.group(2) == "leader" and g.group(3).lower() != g.group(6).lower():
                pobj_diff += 1
            if g.group(2) == "pf":
                continue
            judged += 1
            flags = [int(g.group(4))] + [int(x.split(":")[2]) for x in g.group(11).split(",") if x.count(":") == 3]
            if any(f == 0 for f in flags):
                badp.append(r)
        txt = f"{len(pl)} PARTY line(s) ({judged} judged); pobj != ctrl after a leader switch x{pobj_diff}"
        if badp:
            C["PR-PARTY"] = ("FAIL", f"{txt}; {len(badp)} with a member or the controlled creature at f11ac=0, first: {q(badp[0]['ln'], badp[0]['line'], name)}", ev(badp[0]))
        else:
            C["PR-PARTY"] = ("PASS", txt + ", every member and ctrl has f11ac=1", ev(pl[0]))

    # ---- PR-A1X: follow-up after every A1 end (retarget ORPHANs are reported, not failed: a successful retarget needs no slot-6 run)
    a1 = [r for r in L["A1"] if r["probe"] and r["outcome"] in ("end", "retarget")]
    if not a1:
        C["PR-A1X"] = ("OBSERVE", "no A1 end/retarget line with the A1X suffix", "")
    else:
        win_ms = cfg_rethink + 1000
        follow = defaultdict(list)
        for r in L["A1X_FOLLOW"]:
            follow[r["m"].group(2).lower()].append(r["t"])
        orphans = [r for r in L["A1X_ORPHAN"]]
        a1_by_cr = defaultdict(list)
        for r in a1:
            a1_by_cr[r["cr"]].append(r)
        bad_orph, retarget_orph = [], 0
        for o in orphans:
            cr = o["m"].group(2).lower()
            prev = [r for r in a1_by_cr.get(cr, ()) if r["t"] <= o["t"]]
            if prev and prev[-1]["outcome"] == "retarget":
                retarget_orph += 1
            else:
                bad_orph.append(o)
        ends_all = [r for r in a1 if r["outcome"] == "end" and not (t_max is not None and r["t"] + win_ms > t_max)]
        ends = [r for r in ends_all if judgeable(r["ln"], r["t"], r["t"] + win_ms, ("A1XF",))]   # not once the FOLLOWUP/ORPHAN budget ran out or lines were dropped
        no_follow = [r for r in ends if not any(r["t"] <= x <= r["t"] + win_ms for x in follow.get(r["cr"], ()))]
        wr = sum(1 for r in L["A1X_STRICT"] if r["m"].group(6) == "would-reject")
        strict_txt = (f"STRICT would-reject {a1_strict[1]}/{a1_strict[0]} (A1 summaries)" if a1_strict[0]
                      else f"STRICT would-reject {wr}/{len(L['A1X_STRICT'])}")
        skipped = len(ends_all) - len(ends)
        txt = (f"{len(a1)} A1 end/retarget line(s), {len(ends)} judged end(s)" + (f" ({skipped} not judged: line budget reached / lines dropped)" if skipped else "")
               + f", FOLLOWUP x{len(L['A1X_FOLLOW'])}, ORPHAN x{len(orphans)} ({retarget_orph} after a retarget), {strict_txt}")
        if bad_orph or no_follow:
            first = bad_orph[0] if bad_orph else no_follow[0]
            C["PR-A1X"] = ("FAIL", f"{txt}; {len(bad_orph)} ORPHAN after an end, {len(no_follow)} end(s) without a FOLLOWUP within {win_ms} ms, "
                                   f"first: {q(first['ln'], first['line'], name)}", ev(first))
        else:
            C["PR-A1X"] = ("PASS", txt, ev(a1[0]))

    # ---- PR-RAH2 (ranged_ai_log.txt): H2 retarget=1, then H1 end52c != 0 inRange=0 for the same creature within 2 s
    if not ranged_path:
        C["PR-RAH2"] = ("OBSERVE", "ranged_ai_log.txt not present", "")
    else:
        h2, h1, h1_any = [], [], 0
        for ln, kind, line in read_ranged_lines(ranged_path):
            if kind != "line":
                continue
            m = R_H2_RE.match(line)
            if m and int(m.group(14)) == 1:
                h2.append(dict(t=int(m.group(1)), cr=m.group(2).lower(), ln=ln, line=line))
                continue
            m = R_H1_END52C_RE.match(line)
            if m:
                h1_any += 1
                if int(m.group(3)) == 0 and int(m.group(4), 16) != 0:
                    h1.append(dict(t=int(m.group(1)), cr=m.group(2).lower(), ln=ln, line=line))
        rn = ranged_path.name
        hits = [(a, b) for a in h2 for b in h1 if a["cr"] == b["cr"] and 0 <= b["t"] - a["t"] <= 2000]
        if not h2 or not h1_any:
            C["PR-RAH2"] = ("OBSERVE", "no H2 retarget=1 line" if not h2 else "H2 retarget=1 seen but no H1 line carries end52c (older ranged-ai log)", "")
        elif hits:
            a, b = hits[0]
            C["PR-RAH2"] = ("FAIL", f"{len(hits)} H2 retarget=1 followed by H1 end52c!=0 inRange=0 within 2 s for the same creature (the 3D-minus-radii retarget "
                                    f"ends silently), first: {rn}:{a['ln']} -> {rn}:{b['ln']}", f"{rn}:{a['ln']}")
        else:
            C["PR-RAH2"] = ("PASS", f"{len(h2)} H2 retarget=1 line(s), none followed by H1 end52c!=0 inRange=0 within 2 s", f"{rn}:{h2[0]['ln']}")

    R["checks"] = C
    R["counts"] = {k: len(v) for k, v in L.items()}
    if R["budget"]:
        an.add("INFO", name, "probe line budget reached for: " + ", ".join(sorted(set(R["budget"]))) + " (later PR lines of those probes are missing; PRS summaries continue)")
    if R.get("unparsed"):
        an.add("WARN", name, f"{len(R['unparsed'])} PR/PRS line(s) not recognised, first: {R['unparsed'][0]}")
    return R


_PR_PARSERS = [("FM2C", PR_FM2C_RE), ("FM2F", PR_FM2F_RE), ("FM2R", PR_FM2R_RE), ("FM3L", PR_FM3L_RE), ("FM3X", PR_FM3X_RE), ("PRS_FM3", PRS_FM3_RE),
               ("RS", PR_RS_RE), ("PRS_RS", PRS_RS_RE), ("PRS_E9", PRS_E9_RE), ("P2", PR_P2_RE), ("PRS_P2", PRS_P2_RE), ("PF", PR_PF_RE), ("PRS_PF", PRS_PF_RE),
               ("PRS_RAR", PRS_RAR_RE), ("RAR_MOVE", PR_RAR_MOVE_RE), ("RAR_REQ", PR_RAR_REQ_RE), ("SHX", PR_SHX_RE), ("PRS_SHX", PRS_SHX_RE),
               ("SS1", PR_SS1_RE), ("GCA", PR_GCA_RE), ("PRS_GCA", PRS_GCA_RE), ("PARTY", PR_PARTY_RE), ("A1X_STRICT", PR_A1X_STRICT_RE),
               ("A1X_FOLLOW", PR_A1X_FOLLOW_RE), ("A1X_ORPHAN", PR_A1X_ORPHAN_RE)]


# ----------------------------------------------------------------------------------------------------------------------
# checklist
# ----------------------------------------------------------------------------------------------------------------------
def build_checklist(S: dict, T: dict, RG: dict, I: dict, win: Window | None, P: dict | None = None) -> list[tuple[str, str, str, str]]:
    """(id, status PASS|FAIL|OBSERVE, summary, evidence refs). OBSERVE = the logs cannot decide (visual) or not exercised."""
    out: list[tuple[str, str, str, str]] = []

    def add(i, st, summ, ev=""):
        out.append((i, st, summ, ev))

    def ref(n, ln):
        return f"{n}:{ln}"

    # ---- inventory
    if not I.get("present"):
        for i in ("M6-1", "M6-2", "M6-3", "M6-4"):
            add(i, "OBSERVE", "inventory_sort_filter_log.txt not present")
    else:
        n = I["name"]
        fl = I["flushes"]
        err = {k: v for k, v in I["errs"].items() if k in ("bad frame", "counter", "caption bad", "overflow")}
        errtxt = "; ".join(f"{k} x{len(v)}" for k, v in err.items())
        sortf = [f for f in fl if f["sort"] == 1 and f["rows"] > 0]
        # M6-1
        if err:
            add("M6-1", "FAIL", f"inventory errors: {errtxt}", ref(n, next(iter(err.values()))[0].split(":")[1]))
        elif sortf:
            add("M6-1", "OBSERVE", f"sort ran in {len(sortf)} flush(es) with no errors; order is visual", ", ".join(ref(n, f["ln"]) for f in sortf[:3]))
        else:
            add("M6-1", "OBSERVE", "no backpack flush with sort=1 and rows>0 (inventory not opened?)")
        # M6-2
        tint = [f for f in fl if f["tint_on"] == 1 and f["tint"] > 0]
        creatures = {f["creature"] for f in tint}
        if err:
            add("M6-2", "FAIL", f"inventory errors: {errtxt}")
        elif len(creatures) >= 2:
            add("M6-2", "PASS", f"tint candidates seen for {len(creatures)} different creatures ({sum(f['tint'] for f in tint)} rows total)",
                ", ".join(ref(n, f["ln"]) for f in tint[:3]))
        elif tint:
            add("M6-2", "OBSERVE", f"tint candidates only for 1 creature ({tint[0]['creature']}); switch members to confirm", ref(n, tint[0]["ln"]))
        else:
            add("M6-2", "OBSERVE", "no flush with tint candidates")
        # M6-3: an equipment-slot populate shows as a flush with no backpack rows ("flush: no backpack rows"), or a flush for the same panel after
        ef = I["empty_flushes"]
        if err:
            add("M6-3", "FAIL", f"inventory errors: {errtxt}")
        elif ef and sortf:
            add("M6-3", "PASS", f"{len(ef)} no-backpack populate(s) (equipment lists) with no counter/bad-frame errors", ", ".join(ref(n, l) for l, _ in ef[:3]))
        else:
            add("M6-3", "OBSERVE", "no equipment-list populate seen (flush: no backpack rows) or no backpack flush; red rows are visual")
        # M6-4
        cyc = I["cycles"]
        states = [c["state"] for c in cyc]
        full = any(states[i:i + 4] == [1, 2, 3, 0] for i in range(len(states)))
        if full:
            add("M6-4", "PASS", "cycle New -> Usable -> Upgradeable -> All seen in tab-click lines (caption fit is visual)",
                ", ".join(ref(n, c["ln"]) for c in cyc[:4]))
        elif cyc:
            add("M6-4", "OBSERVE", f"partial cycle: states {states}", ", ".join(ref(n, c["ln"]) for c in cyc[:4]))
        else:
            add("M6-4", "OBSERVE", "no 'tab click: cycle' lines")

    # ---- ranged-ai
    if not RG.get("present"):
        add("M8-1", "OBSERVE", "ranged_ai_log.txt not present")
    else:
        n = RG["name"]
        sc = RG["s_counts"]
        if RG["s_err"]:
            add("M8-1", "FAIL", RG["s_err"][0])
        elif RG["s3_to16"] and sc["S2"]:
            add("M8-1", "PASS", f"S2 menu row x{sc['S2']}, S3 click to stance 16 x{len(RG['s3_to16'])}, S4 HUD marker x{sc['S4']} "
                                "(the message text and icon are visual)", ref(n, RG["s3_to16"][0][0]))
        elif sc["S2"]:
            add("M8-1", "OBSERVE", f"stance row built (S2 x{sc['S2']}) but no click to stance 16 logged")
        else:
            add("M8-1", "OBSERVE", "no S2/S3 lines: the stance menu was not opened with the mod loaded")
    # ---- script/engine evidence (needs ai_trace)
    tn = T.get("name", "ai_trace.txt")
    tp = T.get("present")
    kite = T.get("kite_events", []) if tp else []
    h1_16 = RG.get("h1", {}).get(16) if RG.get("present") else None
    h2_16 = [h for h in RG.get("h2", []) if h["stance"] == 16] if RG.get("present") else []
    if not tp and not RG.get("present"):
        add("M8-2", "OBSERVE", "no ai_trace.txt and no ranged_ai_log.txt")
    else:
        parts = []
        if kite:
            parts.append(f"Kite step lines x{len(kite)}")
        if h1_16 and h1_16["n"]:
            parts.append(f"stance-16 H1 x{h1_16['n']} (hold={h1_16['hold']})")
        if h2_16:
            parts.append(f"stance-16 H2 retarget x{len(h2_16)}")
        if kite and (h1_16 or h2_16):
            add("M8-2", "PASS", "; ".join(parts) + " (distance and line-of-fire behaviour itself is visual)",
                ref(tn, kite[0]["ln"]) if kite else "")
        elif parts:
            add("M8-2", "OBSERVE", "; ".join(parts))
        else:
            add("M8-2", "OBSERVE", "no Kite step / stance-16 H1/H2 lines" + ("" if tp else " (ai_trace.txt not present)"))
    # M8-3
    if not tp:
        add("M8-3", "OBSERVE", "ai_trace.txt not present")
    elif not kite:
        add("M8-3", "OBSERVE", "no 'Kite step' lines")
    else:
        mx = max((k["streak"] for k in kite if k["step"] == 1), default=0)
        caps_n = T["marker_total"].get("Kite K2 cap", 0)
        if mx > 2:
            bad = next(k for k in kite if k["step"] == 1 and k["streak"] > 2)
            add("M8-3", "FAIL", f"streak {mx} seen (cap is 2)", ref(tn, bad["ln"]))
        elif mx == 2 and caps_n:
            add("M8-3", "PASS", f"max streak 2, K2 cap fired x{caps_n}", ref(tn, T["marker_ex"]["Kite K2 cap"][0].split(":")[1]))
        else:
            add("M8-3", "OBSERVE", f"max streak {mx}, K2 cap x{caps_n}: the cap was not reached in this session")
    # M8-4
    if not RG.get("present"):
        add("M8-4", "OBSERVE", "ranged_ai_log.txt not present")
    else:
        h3 = RG["h3_rows"]
        ka = [r for r in h3 if r["ka"] == 1 and (r["ranged"] == 0 or r["inrange"] == 0)]
        if not h3:
            add("M8-4", "OBSERVE", "no H3 lines (stance 16 never in combat, or ranged-ai < 0.3.0)")
        elif any(r["eff"] != 16 for r in ka):
            bad = next(r for r in ka if r["eff"] != 16)
            add("M8-4", "FAIL", "kite_approach=1 with a melee weapon or out-of-range target, yet the decision was not 16 (approach)", ref(RG["name"], bad["ln"]))
        elif ka:
            add("M8-4", "PASS", f"{len(ka)} H3 decision(s) with ranged=0 or inRange=0 and kite_approach=1 -> 16 (approach)", ref(RG["name"], ka[0]["ln"]))
        else:
            add("M8-4", "OBSERVE", f"{len(h3)} H3 line(s), none with a far target or melee weapon and kite_approach=1")
    # M8-5
    if not tp:
        add("M8-5", "OBSERVE", "ai_trace.txt not present")
    else:
        first = []
        for k in kite:
            if k["step"] == 1 and k["streak"] == 1:
                ts = T["shout_t"].get(k["cr"], [])
                i = bisect.bisect_left(ts, k["t"] - 1000)
                if i < len(ts) and ts[i] <= k["t"] and k["script"] == "k_ai_master":
                    first.append(k)
        if first:
            add("M8-5", "PASS", f"{len(first)} 'Kite step 1 streak 1' from k_ai_master within 1 s of a 'Shout Clear 1800' (shout-driven first round)",
                ref(tn, first[0]["ln"]))
        elif kite:
            add("M8-5", "OBSERVE", "Kite step lines exist but none paired with a Shout Clear 1800 from k_ai_master (first-round timing is visual)")
        else:
            add("M8-5", "OBSERVE", "no Kite step lines")

    # ---- companion AI
    if not tp:
        for i in ("CAI-1", "CAI-2", "CAI-3", "CAI-4", "CAI-5", "CAI-6", "CAI-7", "CAI-8"):
            add(i, "OBSERVE", "ai_trace.txt not present")
    else:
        # CAI-1
        a1 = T["a1"]
        fired = [r for r in a1 if r["outcome"] in ("end", "retarget")]
        if T["a1_missing_reliable"] or T["e1spin_bad"]:
            bad = (T["a1_missing_reliable"] or T["e1spin_bad"])[0]
            add("CAI-1", "FAIL", f"A1 without E2: {len(T['a1_missing_reliable'])}; E1 SPIN with a1=1: {len(T['e1spin_bad'])}", ref(tn, bad["ln"]))
        elif fired:
            add("CAI-1", "PASS", f"A1 fired x{len(fired)} ({sum(1 for r in fired if r['outcome'] == 'end')} end, "
                                 f"{sum(1 for r in fired if r['outcome'] == 'retarget')} retarget), every 'end' followed by E2 within the window",
                ref(tn, fired[0]["ln"]))
        elif T["a1sum"]:
            add("CAI-1", "OBSERVE", f"only A1 summary lines (would-spin) for {len(T['a1sum'])} creature(s); the fix itself did not fire")
        else:
            add("CAI-1", "OBSERVE", "no A1 lines: the stuck-attack case did not occur")
        # CAI-2
        seq = T["seq"]
        heals = [(cr, e) for cr, ev in seq.items() for e in ev if CLEAR_RE.search(e[4]) and CLEAR_RE.search(e[4]).group(1) == "2300"]
        recast = [r for r in T["recast"]]
        medpac = T["marker_total"].get("Party MEDPAC", 0)
        if recast:
            add("CAI-2", "FAIL", f"recast pattern x{len(recast)} (see anomalies)", ref(tn, recast[0]["ln0"]))
        elif heals:
            add("CAI-2", "PASS", f"Clear 2300 x{len(heals)} from {len({c for c, _ in heals})} creature(s), no recast loop; Party MEDPAC x{medpac}, "
                                 f"'I have a Med Pack' x{T['medpack']} (healing a non-leader below 50% is visual)",
                ref(tn, heals[0][1][1]))
        else:
            add("CAI-2", "OBSERVE", "no 'Clear 2300' lines (no heal cast)")
        # CAI-3
        pois = [(cr, e) for cr, ev in seq.items() for e in ev if POISON_START_RE.search(e[4])]
        def _near_invalid(cr, t0):
            ts = T["poison_bad_t"].get(cr, [])
            i = bisect.bisect_left(ts, t0 - 999)
            return i < len(ts) and ts[i] < t0 + 1000
        pois_ok = [(cr, e) for cr, e in pois if not _near_invalid(cr, e[0])]
        poison_recast = [r for r in recast if r["poison"]]
        if poison_recast:
            add("CAI-3", "FAIL", "poison-cure recast pattern", ref(tn, poison_recast[0]["ln0"]))
        elif pois_ok:
            add("CAI-3", "PASS", f"'Starting Poison Check' without 'Returning Invalid Poison Object' x{len(pois_ok)}, no recast loop",
                ref(tn, pois_ok[0][1][1]))
        elif pois:
            add("CAI-3", "OBSERVE", f"poison checks x{len(pois)} but all returned an invalid object (no cure target)")
        else:
            add("CAI-3", "OBSERVE", "no poison check lines")
        # CAI-4
        if T["aoe"]:
            add("CAI-4", "OBSERVE", f"AOE Target lines x{T['aoe']}; whether the centre was an enemy is visual (target ring)")
        else:
            add("CAI-4", "OBSERVE", "no 'Jedi Support AI: AOE Target' lines")
        # CAI-5
        h1m = T["marker_total"].get("Party H1", 0)
        if h1m:
            add("CAI-5", "PASS", f"Party H1 'no usable Revitalize' x{h1m}; 'Someone is knocked out.' x{T['knocked_out']}",
                T["marker_ex"]["Party H1"][0].split(": ")[0])
        elif T["knocked_out"]:
            add("CAI-5", "OBSERVE", f"'Someone is knocked out.' x{T['knocked_out']} but no Party H1 line (the Jedi may have Revitalize)")
        else:
            add("CAI-5", "OBSERVE", "no knocked-out / H1 lines")
        # CAI-6
        ex = T["marker_total"].get("Party E1 exit", 0)
        exits = [e for ev in seq.values() for e in ev if re.search(r"Party E1: no enemy seen, stance (\d+) combat exit", e[4])]
        stances = Counter(re.search(r"stance (\d+)", e[4]).group(1) for e in exits)
        if T["e1_skip_bad"]:
            add("CAI-6", "FAIL", f"Party E1 'skipped' for > {E1_SKIP_MAX_MS // 1000} s after the last attack line", ref(tn, T["e1_skip_bad"][0]["ln"]))
        elif stances.get("11") or stances.get("16"):
            add("CAI-6", "PASS", f"combat-exit lines by stance {dict(stances)}; skipped x{T['marker_total'].get('Party E1 skipped', 0)} (none too long)",
                ref(tn, exits[0][1]))
        elif ex:
            add("CAI-6", "OBSERVE", f"combat exits only for stances {dict(stances)}, none for Ranged (11) or Kite (16)")
        else:
            add("CAI-6", "OBSERVE", "no 'Party E1: no enemy seen' lines")
        # CAI-7
        f1 = T["fm1"]
        if f1.get("keep"):
            add("CAI-7", "PASS", f"FM1 keep x{f1['keep']}, clear x{f1.get('clear', 0)} (stepping aside visually)", T["fm1_q"][0].split(": ")[0] if T["fm1_q"] else "")
        elif f1:
            add("CAI-7", "OBSERVE", f"FM1 clear x{f1.get('clear', 0)} only (no follow-only queue was bumped)")
        else:
            add("CAI-7", "OBSERVE", "no FM1 lines (nobody bumped)")
        # CAI-8
        f4 = T["fm4"]
        if T["tid_bad"] or T["fm4_unpaired"]:
            add("CAI-8", "FAIL", f"tid mismatches {len(T['tid_bad'])}; unpaired snapshots {T['fm4_unpaired']}", ref(tn, (T["tid_bad"] or f4)[0]["ln"]))
        elif f4 and any("restored" in f["outcome"] for f in f4):
            same = "T1 tid equals every FM4 tid" if T["t1"] else "no T1 line to compare tids"
            add("CAI-8", "PASS", f"FM4 restored x{sum('restored' in f['outcome'] for f in f4)}/{len(f4)}; {same} (follow behaviour after load is visual)",
                ref(tn, f4[0]["ln"]))
        elif f4:
            add("CAI-8", "OBSERVE", f"FM4 lines x{len(f4)} but none restored: {f4[0]['outcome']}")
        else:
            add("CAI-8", "OBSERVE", "no FM4 lines (no save made)")

    # ---- E3
    if not RG.get("present"):
        add("E3", "OBSERVE", "ranged_ai_log.txt not present")
    else:
        lines = RG["h1_los0"].get(11, [])
        n11 = RG["h1"].get(11, Counter())
        wtxt = f" in window {win.spec}" if win else ""
        if lines:
            holds = sum(1 for _, l in lines if re.search(r"-> hold=1\s*$", l))
            add("E3", "PASS", f"answered: {len(lines)} stance-11 H1 line(s) with los=0{wtxt} (hold=1 x{holds}, hold=0 x{len(lines) - holds}); "
                              f"stance-11 H1 total {n11.get('n', 0)}", f"{RG['name']}:{lines[0][0]}" + (f"..{lines[-1][0]}" if len(lines) > 1 else ""))
        elif n11.get("n"):
            add("E3", "OBSERVE", f"stance-11 H1 lines exist x{n11['n']}{wtxt} but none with los=0 (the ally was not on the line of fire)")
        else:
            add("E3", "OBSERVE", f"no stance-11 H1 lines{wtxt}")
    # ---- Phase-4 probe checks (analyze_probes)
    pr = (P or {}).get("checks", {})
    for i in PR_CHECK_IDS:
        if i in pr:
            add(i, *pr[i])
        else:
            add(i, "OBSERVE", "ai_trace.txt not present" if not tp else "no data")
    order = {i: n for n, (i, _) in enumerate(CHECKLIST)}
    out.sort(key=lambda r: order.get(r[0], 99))
    return out


# ----------------------------------------------------------------------------------------------------------------------
# report
# ----------------------------------------------------------------------------------------------------------------------
def md_table(headers: list[str], rows: list[list]) -> list[str]:
    out = ["| " + " | ".join(headers) + " |", "|" + "|".join("---" for _ in headers) + "|"]
    for r in rows:
        out.append("| " + " | ".join(str(c).replace("|", "\\|") for c in r) + " |")
    return out


def render(dirp: Path, found: dict, S, T, RG, I, hooks, hook_notes, checklist, an: Anoms, clock: Clock, win: Window | None, opts, P: dict | None = None) -> str:
    L: list[str] = []
    A = L.append
    A("# Play-log report")
    A("")
    A(f"- Directory: `{dirp}`")
    A(f"- Generated by `analyze_play_logs.py` ({dt.datetime.now():%Y-%m-%d %H:%M:%S})")
    A("")
    A("## 1. Summary")
    A("")
    rows = []
    for k, fname in [("trace", FILES["trace"]), ("ranged", FILES["ranged"]), ("inv", FILES["inv"])]:
        p = found.get(k)
        rows.append([p.name if p else fname, "found" if p else "**missing**", f"{p.stat().st_size} B" if p else "-"])
    p = found.get("steam")
    rows.append([p.name if p else "steam-208580.log", "found" if p else "**missing**", f"{p.stat().st_size} B" if p else "-"])
    L.extend(md_table(["file", "status", "size"], rows))
    A("")
    # time span
    spans = []
    if S.get("present") and S.get("ts_first") is not None:
        a, b = S["ts_first"] * 1000, S["ts_last"] * 1000
        spans.append(f"steam log: tick {a / 1000:.1f}..{b / 1000:.1f} s ({(b - a) / 60000:.1f} min)" + (f", {clock.wall(int(a)):%H:%M:%S}..{clock.wall(int(b)):%H:%M:%S}" if clock.anchor_dt else ""))
    if T.get("present") and T.get("t_min") is not None:
        spans.append(f"{T.get('name', FILES['trace'])}: t={T['t_min']}..{T['t_max']} ({(T['t_max'] - T['t_min']) / 60000:.1f} min)" + (f", {clock.wall(T['t_min']):%H:%M:%S}..{clock.wall(T['t_max']):%H:%M:%S}" if clock.anchor_dt else ""))
    if RG.get("present") and RG.get("t_min") is not None:
        spans.append(f"ranged_ai_log: t={RG['t_min']}..{RG['t_max']}" + (f", {clock.wall(RG['t_min']):%H:%M:%S}..{clock.wall(RG['t_max']):%H:%M:%S}" if clock.anchor_dt else ""))
    A("- Session time span: " + ("; ".join(spans) if spans else "unknown (no timestamps found)"))
    A(f"- Clock anchor (tick to wall clock): {clock.source}" + (f" = {clock.anchor_dt:%Y-%m-%d %H:%M:%S} at tick {clock.anchor_tick}" if clock.anchor_dt else ""))
    A("  (The steam log timestamps are seconds of the same GetTickCount clock as `t=` in the other logs.)")
    if win:
        A(f"- Window: {win.spec} " + ("(applied to ai_trace and ranged_ai_log events)" if win.active() else "**ignored: no clock anchor available**"))
    if T.get("present") and T.get("header"):
        A(f"- {T.get('name', FILES['trace'])} header: `{T['header']['text'][:200]}`")
    cnt = Counter(s for s, _, _ in an.items)
    A(f"- Anomalies: {cnt['FAIL']} FAIL, {cnt['WARN']} WARN, {cnt['INFO']} INFO")
    cl = Counter(c[1] for c in checklist)
    A(f"- Checklist: {cl['PASS']} PASS, {cl['FAIL']} FAIL, {cl['OBSERVE']} needs observation")
    A("")

    # 2. checklist table
    A("## 2. Checklist table")
    A("")
    L.extend(md_table(["id", "status", "what the logs show", "evidence (file:line)"],
                      [[i, st, summ, ev or "-"] for i, st, summ, ev in checklist]))
    A("")
    A("PASS = the log shows the fix ran and nothing contradicts it. OBSERVE = the logs cannot decide (visual behaviour) or the case did not occur. "
      "A PASS never replaces the visual check named in the checklist.")
    A("")

    # 3. anomalies
    A("## 3. Anomalies")
    A("")
    if not an.items:
        A("None.")
    else:
        for sev in ("FAIL", "WARN", "INFO"):
            sel = [(src, msg) for s, src, msg in an.items if s == sev]
            for src, msg in sel[:40]:
                A(f"- **{sev}** [{src}] {msg}")
            if len(sel) > 40:
                A(f"- **{sev}** ... +{len(sel) - 40} more {sev} anomalies not listed")
    A("")

    # 4. steam
    A("## 4. Steam log")
    A("")
    if not S.get("present"):
        A("steam log: not present.")
    else:
        A(f"`{S['name']}`: {S['n_lines']} lines.  Patch DLLs loaded (patches/*.dll): "
          + ", ".join(f"{p}={'yes' if p in S['dlls'] else '**NO**'}" for p in S["patches"]))
        A("")
        A("### DETOUR table (addresses from the hook tomls)")
        A("")
        rows = []
        for r in S["rows"]:
            ap = r["applied"][0][2] if r["applied"] else ""
            cons = "-"
            if r["consumed"] is not None:
                c = r["consumed_logged"]
                cons = f"{fmt_addr(r['consumed'])} " + ("parsed" if c and c[1] == r["consumed"] else "**NOT parsed**" if not c else "**MISMATCH**")
            rows.append([r["patch"], fmt_addr(r["address"]), r["type"], r["function"], r["status"], cons])
        L.extend(md_table(["patch", "address", "type", "function", "Applied line", "consumed_exit_address"], rows))
        A("")
        by_patch = defaultdict(lambda: [0, 0])
        for r in S["rows"]:
            by_patch[r["patch"]][0] += 1
            by_patch[r["patch"]][1] += 1 if r["applied"] else 0
        for p, (tot, ok) in by_patch.items():
            A(f"- {p}: {ok}/{tot} hooks applied" + ("" if ok == tot else f"; missing {tot - ok}"))
        A(f"- `Parsed consumed_exit_address` for toml hooks: {S['consumed_parsed']}/{S['consumed_expected']} "
          f"(toml hooks with consumed_exit_address: {S['consumed_expected']})"
          + (f"; wrong value x{len(S['consumed_wrong'])}" if S["consumed_wrong"] else "")
          + (f"; log has consumed lines for addresses not in the tomls: {[fmt_addr(a) for a in S['consumed_extra']]}" if S["consumed_extra"] else ""))
        for n in hook_notes:
            A(f"- note: {n}")
        A("")
        A("### Errors and exceptions")
        A("")
        A(f"- KotorPatcher/Config error lines: {len(S['patcher_err'])}")
        L.extend(quotes_block(S["patcher_err"]))
        if S["other_err"]:
            A(f"- other `[Tag] ERROR/WARNING` debug strings: {sum(S['other_err'].values())} ({len(S['other_err'])} distinct): "
              + "; ".join(f"{k} x{v}" for k, v in S["other_err"].most_common(4)))
        A(f"- `err:` lines: {S['n_err']}")
        L.extend(quotes_block(S["err_lines"], 3))
        ec = S["exc_codes"]
        A(f"- exception dispatches other than the benign debug-string/thread-rename/RPC ones: {sum(ec.values())} "
          + (", ".join(f"{c} x{n}" for c, n in ec.items()) if ec else ""))
        if "c0000005" in ec:
            A(f"  - access violations: {ec['c0000005']}, of which {S['av_probe']} are followed by an IsBad*Ptr probe (handled by SafeRead/probeRead), "
              f"{len(S['av_other'])} are not")
        L.extend(quotes_block(list(S["exc_first"].values()), 3))
        A(f"- 'unhandled exception' lines: {len(S['unhandled'])}")
        L.extend(quotes_block(S["unhandled"], 3))
        A(f"- clean shutdown (DLL_PROCESS_DETACH): {'line ' + str(S['detach']) if S['detach'] else 'not seen'}")
    A("")

    # 5. ai_trace
    A("## 5. ai_trace.txt")
    A("")
    if not T.get("present"):
        A("ai_trace.txt: not present.")
    else:
        A(f"{T['n_lines']} lines; script P/A lines parsed: {T['n_pa']}; repeat markers: {T['repeats'][1]} (hiding {T['repeats'][0]} lines)."
          + (f" Lines outside the window skipped: {T['skipped_by_window']}." if win else ""))
        if T["other_counts"]:
            A("Other/unrecognised: " + ", ".join(f"{k} x{v}" for k, v in T["other_counts"].items()))
        A("")
        A("### Config")
        A("")
        A(f"- `# config: unparseable value`: {len(T['cfg_bad'])}" + (" **FLAGGED**" if T["cfg_bad"] else ""))
        cfgq = [q(ln, ln_text, T["name"]) for ln, ln_text in (T["cfg_bad"] + T["cfg_notes"])]
        cfgq += [q(ln, ln_text, T["name"]) for ln, ln_text in T["cfg_lines"][-2:]]
        L.extend(quotes_block(cfgq, 5))
        A("")
        A("### T1 thread id vs FM4 tid")
        A("")
        if T["t1"]:
            A(f"- T1: tid={T['t1']['tid']} (load tid {T['t1']['load_tid']}), line {T['t1']['ln']}")
        else:
            A("- T1 line: not present (no traced ShipBuild flip happened)")
        A(f"- FM4 lines: {len(T['fm4'])}; tids seen: {sorted({f['tid'] for f in T['fm4']})}; mismatches vs T1: {len(T['tid_bad'])}"
          + (" **MISMATCH**" if T["tid_bad"] else ""))
        A(f"- FM4 outcomes: {dict(Counter(f['outcome'] for f in T['fm4']))}; config fm4={T['fm4_cfg']}")
        ls = T["last_s"]
        if ls:
            A(f"- FM4 pairing (last S line, cumulative): snap={ls.get('fm4.snap')} restore={ls.get('fm4.restore')}"
              + (f" **UNPAIRED by {T['fm4_unpaired']}**" if T["fm4_unpaired"] else ""))
        else:
            A("- no S summary line: FM4 pairing could not be checked from counters")
        L.extend(quotes_block([q(f["ln"], f["line"], T["name"]) for f in T["fm4"]], 3))
        A("")
        A("### Script lines per creature (P = PrintString, A = AurPostString)")
        A("")
        rows = []
        for cr, c in sorted(T["pa_count"].items(), key=lambda kv: -sum(kv[1].values()))[:15]:
            by_script = Counter(); by_ev = Counter()
            for (kind, script, ev), n in c.items():
                by_script[script] += n; by_ev[ev] += n
            rows.append([f"{T['cr_tag'].get(cr, '-')} ({cr})", sum(n for (k, _, _), n in c.items() if k == "P"),
                         sum(n for (k, _, _), n in c.items() if k == "A"),
                         ", ".join(f"{s}:{n}" for s, n in by_script.most_common(5)), ", ".join(f"{e}:{n}" for e, n in by_ev.most_common(5))])
        if rows:
            L.extend(md_table(["creature", "P", "A", "by script", "by event (ev=)"], rows))
        else:
            A("No P/A script lines.")
        A("")
        A("### A1 effectiveness (E2 FALLBACK within %d ms)" % opts.e2_window_ms)
        A("")
        a1 = T["a1"]
        oc = Counter(r["outcome"] for r in a1)
        A(f"- A1 lines: {len(a1)} ({dict(oc)}); required to have an E2: {len(T['a1_need'])} "
          f"({'all outcomes' if opts.a1_strict else 'outcome=end; retarget continues the attack, a not-applied spin is a1=0/unlisted'})")
        A(f"- with E2 in time: {len(T['a1_need']) - len(T['a1_missing'])}; **without: {len(T['a1_missing'])}** "
          f"({len(T['a1_missing_reliable'])} reliable; E2 lines are throttled after 20 per creature)")
        L.extend(quotes_block([q(r["ln"], r["line"], T["name"]) for r in T["a1_missing"]], 4))
        deltas = [r["delta"] for r in T["a1_need"] if r["delta"] is not None]
        if deltas:
            A(f"- E2 delay after A1 'end': min {min(deltas)} ms, median {sorted(deltas)[len(deltas) // 2]} ms, max {max(deltas)} ms")
        sums = T["a1sum"]
        if sums:
            A(f"- `A1 summary would-spin` lines: {sum(c['lines'] for c in sums.values())} for {len(sums)} creature(s); "
              f"passes={sum(c['passes'] for c in sums.values())} ends={sum(c['ends'] for c in sums.values())} retargets={sum(c['retargets'] for c in sums.values())}")
            L.extend(quotes_block([q(c['sample'][0], c['sample'][1], T['name']) for c in sums.values() if 'sample' in c], 3))
        else:
            A("- `A1 summary would-spin` lines: none")
        A("")
        A("### E1 SPIN")
        A("")
        A(f"- E1 SPIN lines: {len(T['e1spin'])} (E1 END {T['e1end']}); while a1=1 for a party stance in a1_stances: **{len(T['e1spin_bad'])}**")
        L.extend(quotes_block([q(s["ln"], s["line"], T["name"]) for s in (T["e1spin_bad"] or T["e1spin"])], 3))
        A("")
        A("### FM1 / A4 / A3")
        A("")
        A(f"- FM1 keep={T['fm1'].get('keep', 0)} clear={T['fm1'].get('clear', 0)}")
        L.extend(quotes_block(T["fm1_q"], 2))
        A(f"- A4 lines: {len(T['a4'])}; A3 lines: {len(T['a3'])}")
        L.extend(quotes_block(T["a4"] + T["a3"], 3))
        A("")
        A("### P1 cost baseline (E3 P1 calls/s)")
        A("")
        e3 = T["e3"]
        if e3:
            byp = defaultdict(list)
            for r in e3:
                byp[(r["p1"], r["interval"])].append(r["calls"])
            rows = []
            for (p1, iv), vals in sorted(byp.items()):
                vs = sorted(vals)
                rows.append([f"p1={p1} interval={iv}", len(vs), f"{sum(vs) / len(vs):.0f}", f"{vs[len(vs) // 2]:.0f}", f"{vs[0]:.0f}", f"{vs[-1]:.0f}"])
            L.extend(md_table(["state", "E3 lines", "mean calls/s", "median", "min", "max"], rows))
            L.extend(quotes_block([q(r["ln"], r["line"], T["name"]) for r in e3], 2))
        else:
            A("No `E3 P1 calls/s` lines.")
        A("")
        A("### Cap and drop markers")
        A("")
        A(f"- file cap markers: {len(T['caps'])}; per-second-cap drop markers: {len(T['drops'])} (lines dropped: {sum(d[0] for d in T['drops'])})")
        L.extend(quotes_block(T["caps"] + [q(l, t, T['name']) for _, l, t in T["drops"]], 3))
        if T["last_s"]:
            A(f"- last S line: {T['last_s_line']}")
        A("")
        A("### Script-fix markers (per creature)")
        A("")
        rows = []
        crs = sorted({cr for c in T["marker_cr"].values() for cr in c}, key=lambda c: -sum(T["marker_cr"][k][c] for k in T["marker_cr"]))
        keys = [k for k, _, _ in MARKERS]
        for cr in crs[:15]:
            rows.append([f"{T['cr_tag'].get(cr, '-')} ({cr})"] + [T["marker_cr"][k].get(cr, 0) or "" for k in keys])
        rows.append(["**total**"] + [T["marker_total"].get(k, 0) for k in keys])
        L.extend(md_table(["creature"] + keys, rows))
        A("")
        A("Marker texts and sources: " + "; ".join(f"`{k}` = {src}" for k, _, src in MARKERS[:4]) + " ... (table `MARKERS` at the top of the tool)")
        for k in keys:
            if T["marker_ex"].get(k):
                A(f"- {k}: `{T['marker_ex'][k][0][:200]}`")
        A("")
        A("### Recast and E1-skipped anomalies")
        A("")
        if T["recast"]:
            for r in T["recast"]:
                A(f"- recast: {T['cr_tag'].get(r['cr'], '-')} ({r['cr']}) Clear {r['code']} x{r['n']} rounds in a row"
                  f"{' (poison-cure)' if r['poison'] else ''}, lines {r['ln0']}..{r['ln1']}")
        else:
            A(f"- recast (same Clear 2300/2400 or poison-cure > {RECAST_MAX_ROUNDS} rounds in a row): none")
        if T["e1_skip_bad"]:
            for s in T["e1_skip_bad"]:
                A(f"- `Party E1: skipped` {s['gap'] / 1000:.1f} s after {'last attack line' if s['had_attack'] else 'first skipped line (no attack line)'}: "
                  f"{T['cr_tag'].get(s['cr'], '-')} ({s['cr']}) line {s['ln']}")
        else:
            A(f"- `Party E1: skipped` more than {E1_SKIP_MAX_MS // 1000} s after the last attack line: none")
    A("")

    # 6. ranged
    A("## 6. ranged_ai_log.txt")
    A("")
    if not RG.get("present"):
        A("ranged_ai_log.txt: not present.")
    else:
        A(f"{RG['n_lines']} lines" + (f"; lines outside the window skipped: {RG['skipped']}" if win else "")
          + f"; sessions (loaded lines): {len(RG['loaded'])}")
        L.extend(quotes_block([q(ln, t, RG["name"]) for ln, t in RG["cfgs"][-2:]], 2))
        A("")
        sc = RG["s_counts"]
        A(f"- S1 x{sc['S1']}, S2 x{sc['S2']}, S3 x{sc['S3']} (to stance 16: {len(RG['s3_to16'])}), S4 x{sc['S4']}")
        L.extend(quotes_block([x for k in ("S1", "S2", "S3", "S4") for x in RG["s_ex"].get(k, [])[:1]], 4))
        A("")
        A("### H1 per stance")
        A("")
        rows = []
        for st in sorted(RG["h1"]):
            c = RG["h1"][st]
            rows.append([st, c["n"], c["los0"], c["hold"], c["los0_hold"], c["ranged"], c["inRange"]])
        if rows:
            L.extend(md_table(["stance", "H1 lines", "los=0", "hold=1", "los=0 and hold=1", "ranged=1", "inRange=1"], rows))
        else:
            A("No H1 lines.")
        A("")
        l11 = RG["h1_los0"].get(11, [])
        A(f"**E3 answer** (stance 11, los=0" + (f", window {win.spec}" if win else "") + f"): {len(l11)} line(s)")
        L.extend(quotes_block([q(ln, t, RG["name"]) for ln, t in l11], 5))
        A("")
        A(f"### H2 retargets: {len(RG['h2'])}")
        A("")
        h2 = RG["h2"]
        if h2:
            A(f"- retarget=1: {sum(h['retarget'] for h in h2)}, lastBlocked == me: {sum(h['eq'] for h in h2)}/{len(h2)} "
              "(H2 is reached on the second blocked check at the same position, so equality is the expected case)")
            A(f"- by stance: {dict(Counter(h['stance'] for h in h2))}")
            L.extend(quotes_block([q(h["ln"], h["line"], RG["name"]) for h in h2], 3))
        eq = RG["h1_eq"]
        if eq:
            A(f"- H1 with lastBlocked == me: " + ", ".join(f"stance {s} {'los=0' if b else 'los!=0'}: {n}" for (s, b), n in sorted(eq.items())))
        A("")
        A(f"### H3 decisions (stance 16): {sum(RG['h3'].values())}")
        A("")
        if RG["h3"]:
            rows = [[r, i, ka, eff, "approach" if eff == 16 else "no approach", n] for (r, i, ka, eff), n in sorted(RG["h3"].items())]
            L.extend(md_table(["ranged", "inRange", "kite_approach", "-> stance", "meaning", "lines"], rows))
        if RG["cap"]:
            A(f"- log cap: {RG['cap']}")
    A("")

    # 7. inventory
    A("## 7. inventory_sort_filter_log.txt")
    A("")
    if not I.get("present"):
        A("inventory_sort_filter_log.txt: not present.")
    else:
        A(f"{I['n_lines']} lines; tab-click lines: {I['clicks']}; flush lines: {len(I['flushes'])} (+{len(I['empty_flushes'])} without backpack rows)")
        L.extend(quotes_block([q(ln, t, I["name"]) for ln, t in I["cfg"][-1:]], 1))
        A("")
        by = defaultdict(lambda: Counter())
        for f in I["flushes"]:
            nm = INV_STATE_NAMES.get(f["state"], "?")
            c = by[nm]
            c["flushes"] += 1; c["rows"] += f["rows"]; c["usable"] += f["usable"]; c["tint-candidates"] += f["tint"]; c["dropped"] += f["dropped"]
        crs = defaultdict(set)
        for f in I["flushes"]:
            crs[INV_STATE_NAMES.get(f["state"], "?")].add(f["creature"])
        rows = [[nm, c["flushes"], len(crs[nm]), c["rows"], c["usable"], c["tint-candidates"], c["dropped"]] for nm, c in by.items()]
        if rows:
            L.extend(md_table(["cycle state", "flushes", "creatures", "rows", "usable", "tint-candidates", "dropped"], rows))
        A("")
        A(f"- cycle transitions: {[INV_STATE_NAMES.get(c['state'], c['state']) for c in I['cycles']]}")
        L.extend(quotes_block([q(c["ln"], c["line"], I["name"]) for c in I["cycles"]], 3))
        errs = I["errs"]
        A("- errors: " + (", ".join(f"{k} x{len(v)}" for k, v in errs.items()) if errs else "none"))
        for k, v in errs.items():
            L.extend(quotes_block(v, 2))
        L.extend(quotes_block([q(f["ln"], f["line"], I["name"]) for f in I["flushes"]], 3))
    A("")

    # 8. Phase-4 probes
    A("## 8. Phase-4 probe summaries (PR / PRS lines)")
    A("")
    if not P or not P.get("present"):
        A("No ai_trace.txt: no probe lines.")
    else:
        cnt = P.get("counts", {})
        A(f"{P['n_pr']} PR line(s), {P['n_prs']} PRS summary line(s)" + (f"; line budget reached for: {', '.join(sorted(set(P['budget'])))}" if P["budget"] else "") + ".")
        A("")
        rows = [[k, v] for k, v in sorted(cnt.items())]
        if rows:
            L.extend(md_table(["line kind", "count"], rows))
            A("")
        A("Last PRS line per probe (and creature):")
        items = [(key, ln_line) for key, ln_line in sorted(P["prs_last"].items())]
        L.extend(quotes_block([q(ln, line, T.get("name", FILES["trace"])) for _k, (ln, line) in items], 40))
    A("")
    return "\n".join(L) + "\n"


# ----------------------------------------------------------------------------------------------------------------------
# main
# ----------------------------------------------------------------------------------------------------------------------
def find_files(d: Path) -> dict:
    found = {}
    for k, fn in FILES.items():
        p = d / fn
        found[k] = p if p.is_file() else None
    # companion-ai: KpLog core name first, then the legacy/fallback ai_trace.txt (no mtime rule: the core file wins when both exist)
    found["trace"] = next((d / n for n in TRACE_NAMES if (d / n).is_file()), None)
    # ranged-ai: legacy/fallback name vs KpLog core name; the newer file (mtime) wins, a tie goes to the legacy name
    cands = [d / n for n in RANGED_NAMES if (d / n).is_file()]
    if cands:
        found["ranged"] = max(cands, key=lambda c: (c.stat().st_mtime_ns, c.name == RANGED_NAMES[0]))
    steam = d / "steam-208580.log"
    if not steam.is_file():
        cands = sorted(d.glob("steam-*.log"))
        steam = cands[0] if cands else None
    found["steam"] = steam
    return found


def run_full(argv: list[str] | None = None) -> dict:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("playlogs_dir")
    ap.add_argument("--hooks-toml", action="append", help="[ID=]PATH of a kpatch hooks toml (repeatable; replaces the three defaults)")
    ap.add_argument("--window", help="HH:MM-HH:MM wall-clock window for ai_trace and ranged_ai_log events")
    ap.add_argument("--anchor", help='"YYYY-MM-DD HH:MM:SS=TICK_MS" clock anchor when no ai_trace header exists')
    ap.add_argument("--out", help="report path (default PLAYLOGS_DIR/REPORT.md)")
    ap.add_argument("--e2-window-ms", type=int, default=3500)
    ap.add_argument("--a1-strict", action="store_true", help="require an E2 after every A1 line, not only 'end'")
    ap.add_argument("--attack-regex", default=DEFAULT_ATTACK_RE)
    ap.add_argument("--strict", action="store_true", help="exit 2 when any FAIL anomaly or checklist FAIL exists")
    ap.add_argument("--no-write", action="store_true")
    opts = ap.parse_args(argv)

    d = Path(opts.playlogs_dir)
    if not d.is_dir():
        print(f"not a directory: {d}", file=sys.stderr)
        return {"rc": 1, "report": "", "out": None}
    found = find_files(d)
    hooks, hook_notes = load_hooks(opts.hooks_toml)
    an = Anoms()
    clock = Clock()
    if opts.anchor:
        m = re.fullmatch(r"(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d)=(\d+)", opts.anchor.strip())
        if not m:
            raise SystemExit("--anchor must be 'YYYY-MM-DD HH:MM:SS=TICK_MS'")
        clock.set(dt.datetime.strptime(m.group(1), "%Y-%m-%d %H:%M:%S"), int(m.group(2)), "--anchor", override=True)
    for n in hook_notes:
        an.add("WARN", "hooks toml", n)
    if not hooks:
        an.add("WARN", "hooks toml", "no hooks loaded: the DETOUR check is empty")
    # trace first: its header is the preferred clock anchor (steam mtime is the fallback and does not override)
    win = Window(opts.window, clock) if opts.window else None
    S = analyze_steam(found["steam"], hooks, an, clock) if found["steam"] else {"present": False}
    T = analyze_trace(found["trace"], win, clock, an, opts) if found["trace"] else {"present": False}
    RG = analyze_ranged(found["ranged"], win, an, T.get("cr_tag", {})) if found["ranged"] else {"present": False}
    I = analyze_inv(found["inv"], an) if found["inv"] else {"present": False}
    P = analyze_probes(found["trace"], found["ranged"], win, an) if (found["trace"] or found["ranged"]) else None
    if win and not win.active():
        an.add("WARN", "window", f"--window {win.spec} ignored: no clock anchor (no ai_trace header and no steam log); pass --anchor")
    for k, label in (("trace", "ai_trace.txt"), ("ranged", "ranged_ai_log.txt"), ("inv", "inventory_sort_filter_log.txt")):
        if not found[k]:
            an.add("INFO", label, "not present")
    if not found["steam"]:
        an.add("INFO", "steam log", "not present")
    checklist = build_checklist(S, T, RG, I, win, P)
    for i, st, summ, ev in checklist:
        if st == "FAIL":
            an.add("FAIL", f"checklist {i}", summ)
    # dedupe repeated anomalies from the checklist (same text)
    seen = set(); items = []
    for it in an.items:
        key = (it[0], it[2])
        if key not in seen:
            seen.add(key); items.append(it)
    an.items = items
    report = render(d, found, S, T, RG, I, hooks, hook_notes, checklist, an, clock, win, opts, P)
    out = None
    if not opts.no_write:
        out = Path(opts.out) if opts.out else d / "REPORT.md"
        out.write_text(report, encoding="utf-8")
    nfail = sum(1 for s, _, _ in an.items if s == "FAIL")
    rc = 2 if (opts.strict and nfail) else 0
    return dict(rc=rc, report=report, out=out, S=S, T=T, RG=RG, I=I, P=P, checklist=checklist, an=an, hooks=hooks, clock=clock, found=found)


def run(argv: list[str] | None = None) -> tuple[int, str, Path | None]:
    r = run_full(argv)
    return r["rc"], r["report"], r["out"]


def main(argv: list[str] | None = None) -> int:
    rc, report, out = run(argv)
    if out:
        print(f"wrote {out}")
        head = [l for l in report.splitlines() if l.startswith("- Anomalies") or l.startswith("- Checklist")]
        for h in head:
            print(h)
    return rc


if __name__ == "__main__":
    sys.exit(main())
