#!/usr/bin/env python3
"""perf-lab / robustness / texmem / activationfix run analyzer (stdlib only, read-only).

Usage:
  analyze_runs.py [--game-dir DIR] [--from-offsets FILE] [--out OUT.md] [--arms texmem|hwmip] [--latest N] [--summary]
  --latest N: summary table + launch details for the last N launches only (overall gate/hang lines still cover ALL, labelled).
  --summary: only the summary table + overall lines. Every output (stdout and --out) ends with `VERDICT: PASS|FAIL launches=K hangs=H gateB=PASS|FAIL`.

Inputs (all optional; a missing file is reported as absent, never a crash):
  perflab_metrics.txt, perflab_alloc_snaps.txt, perflab_log.txt, robustnessfixes_log.txt, texmem_log.txt,
  activationfix_log.txt, equipiconfix_log.txt (+ the config files perflab_debug.txt, texmem.txt, shown as-is).

Offsets: each log is read from a byte offset (default table below = doc 10 run A; `--from-offsets FILE` has lines
`name bytes [ignored...]`, `#` comments).  The file is opened, seeked to the offset, and if the byte before the offset
is not a newline the partial first line is discarded, so a mid-line offset never produces a garbage record.

0.5.0 log content (all append-only, absent in 0.3.0/0.4.0 logs, parsed generically as key=value and tolerated when missing):
  `module=<name|->` header key; `EVT kind=.. tick= slot= name=` + `EVT counts ...` (S3 measured, SNDRI verdict); `TPCTOP` /
  `RESTOP` (top-N tables at LOAD_DOWN/UNLOAD); `VA` (address-space: min/last largest_free_mb, WARN < 256 MB); `HWV` + `HWVFB`
  (`--arms hwmip`); FRAME `gms_calls/gms_ms`, KT `txi_via_tpc_kb`, BUILD `requeue_tpc_retained_kb`, RT `redemB`.

Launch splitting and matching rule (what each log actually prints):
  * perflab_metrics.txt: a new launch starts at every `#METRICS 1 <event> tick=<GetTickCount>` header (the block counter
    is per process).  The header carries NO pid, version or wall-clock; `tick` is GetTickCount() in ms.
  * perflab_log.txt: `[<tick> t<frames>] perf-lab <ver> attach ... key=value ...` is the launch start (version, config
    flags, hwmip=ON/OFF/0/1 when 0.4.0+).  Its tick is the launch START tick.  An attach line with no metrics block
    (died before the first 30 s flush) becomes a stub launch.
  * robustness / texmem / activationfix / equipiconfix: `[HH:MM:SS.mmm <tick>] ... loaded: ... pid=<pid>` starts a block;
    every following line up to the next `loaded` line belongs to that block (COUNTERS / FIRED / REFUSED / IGNORED / PASSED
    / STATS ...).  Lines before the first `loaded` line (offset landed mid-launch) are ignored and counted.
  * Matching: pid is NOT usable as the key (the Steam/Proton pid repeats: 312, 316, 312, ...) and perf-lab prints none,
    so a block is joined to a launch by GetTickCount: the launch with the latest start tick <= block tick + 2000 ms whose
    window (start .. last metrics tick + 5000 ms) contains it.  Start tick = attach tick, else previous launch end + 1,
    else first block tick - 120000.  A block that fits no launch is an "orphan" (a launch outside the perf-lab data)
    and is only counted.  The pid printed by the matched loaded line is shown for the launch; the launch wall-clock
    time is that line's HH:MM:SS minus (its tick - launch start tick).  perflab_alloc_snaps.txt (`#SNAP n EV tick=`)
    is joined the same way by the snapshot tick.  GetTickCount wrap (49.7 days) is not handled (all logs of one run
    are far from it); durations use mod-2^32 differences.

Output is markdown; large files are streamed line by line (NAME/BRRING/HOOKCOST_BY lines are skipped cheaply).
"""
import argparse
import os
import re
import statistics
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "scripts"))
import kplog_parse  # noqa: E402

DEFAULT_GAME_DIR = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
DEFAULT_OFFSETS = {
    "perflab_metrics.txt": 10828745,
    "perflab_alloc_snaps.txt": 746320,
    "perflab_log.txt": 53860,
    "activationfix_log.txt": 3221,
    "equipiconfix_log.txt": 31015,
    "robustnessfixes_log.txt": 0,
    "texmem_log.txt": 0,
}
LOG_FILES = ("robustnessfixes_log.txt", "texmem_log.txt", "activationfix_log.txt", "equipiconfix_log.txt")
# KpLog core file (kplog.ini enabled) for each text-log key; the key stays the legacy/fallback file name.
LOG_ALIASES = {"texmem_log.txt": "k2-texture-memory-release_log.txt",
               "activationfix_log.txt": "k2-activation-fix_log.txt",
               "equipiconfix_log.txt": "k2-equip-icon-fix_log.txt"}
ALL_FILES = ("perflab_metrics.txt", "perflab_alloc_snaps.txt", "perflab_log.txt") + LOG_FILES
CONFIG_FILES = ("perflab_debug.txt", "texmem.txt")
MATCH_SLACK_MS = 2000
TAIL_SLACK_MS = 5000
FRAME_EDGES = ["<1", "1", "2-3", "4-7", "8-15", "16-31", "32-63", "64-127", "128-255", ">=256"]  # ms, PerfLab.cpp lbucket
TPC_RESTYPE = 3007
MB = 1024.0 * 1024.0
VA_WARN_MB = 256.0   # WARN when a launch's smallest largest_free_mb drops below this


# ----------------------------------------------------------------------------------------------------------------
# reading
# ----------------------------------------------------------------------------------------------------------------
class ReadInfo:
    def __init__(self, name, path, offset):
        self.name, self.path, self.offset = name, path, offset
        self.present = os.path.isfile(path)
        self.size = os.path.getsize(path) if self.present else 0
        self.lines = 0
        self.partial_skipped = False
        self.beyond_eof = False
        self.notes = []

    def status(self):
        if not self.present:
            return "ABSENT"
        s = "read %d lines" % self.lines
        if self.partial_skipped:
            s += ", partial first line skipped"
        if self.beyond_eof:
            s += ", OFFSET BEYOND EOF (file rotated/truncated?)"
        return s


def read_lines(info):
    """Yield decoded lines of info.path starting at info.offset (see module docstring)."""
    if not info.present:
        return
    if info.offset > info.size:
        info.beyond_eof = True
        return
    with open(info.path, "rb") as f:
        if info.offset > 0:
            f.seek(info.offset - 1)
            if f.read(1) != b"\n":
                f.readline()
                info.partial_skipped = True
        for raw in f:
            info.lines += 1
            yield raw.decode("latin1").rstrip("\r\n")


def load_offsets(path):
    offs = {}
    with open(path) as f:
        for ln in f:
            ln = ln.split("#", 1)[0].strip()
            if not ln:
                continue
            p = ln.split()
            if len(p) < 2:
                continue
            offs[p[0]] = int(p[1])
    return offs


# ----------------------------------------------------------------------------------------------------------------
# generic helpers
# ----------------------------------------------------------------------------------------------------------------
def num(s):
    try:
        return int(s)
    except ValueError:
        pass
    try:
        return float(s)
    except ValueError:
        return s


def is_num(s):
    return isinstance(s, (int, float)) and not isinstance(s, bool)


def parse_kv(tokens):
    """key=value tokens -> dict (numbers converted); `word n n n ...` (bare word followed by numbers) -> list."""
    d = {}
    i = 0
    while i < len(tokens):
        t = tokens[i]
        if "=" in t and not t.startswith("="):
            k, v = t.split("=", 1)
            d[k] = num(v)
        elif t and not is_num(num(t)) and i + 1 < len(tokens) and is_num(num(tokens[i + 1])):
            j = i + 1
            vals = []
            while j < len(tokens) and is_num(num(tokens[j])):
                vals.append(num(tokens[j]))
                j += 1
            d[t] = vals
            i = j
            continue
        i += 1
    return d


def tdiff(a, b):
    return (a - b) & 0xFFFFFFFF


def med(vals):
    v = [x for x in vals if x is not None]
    return statistics.median(v) if v else None


def fmt(v, nd=1):
    if v is None:
        return "-"
    if isinstance(v, bool):
        return "yes" if v else "no"
    if isinstance(v, float):
        if abs(v - round(v)) < 1e-9 and abs(v) < 1e15:
            return str(int(round(v)))
        r = ("%." + str(nd) + "f") % v
        return r[1:] if r.startswith("-") and float(r) == 0 else r
    return str(v)


def table(headers, rows):
    out = ["| " + " | ".join(headers) + " |", "|" + "|".join("---" for _ in headers) + "|"]
    for r in rows:
        out.append("| " + " | ".join(str(c).replace("|", "/") for c in r) + " |")
    return out


def norm_flag(v):
    """hwmip=ON/OFF/1/0 -> 1/0 (None if not a flag)."""
    if isinstance(v, str):
        u = v.upper()
        if u in ("ON", "TRUE", "YES"):
            return 1
        if u in ("OFF", "FALSE", "NO"):
            return 0
        return None
    if is_num(v):
        return 1 if v else 0
    return None


# ----------------------------------------------------------------------------------------------------------------
# perflab_metrics.txt
# ----------------------------------------------------------------------------------------------------------------
class Block:
    __slots__ = ("n", "ev", "tick", "frames", "live", "allocs", "hdr", "kv", "rt", "rt_redem", "evts", "tpctop", "restop", "hwvfb")

    def __init__(self, n, ev, hdr):
        self.n, self.ev = n, ev
        self.hdr = hdr
        self.tick = hdr.get("tick", 0)
        self.frames = hdr.get("frames(hook)")
        self.live = hdr.get("live_bytes")
        self.allocs = hdr.get("allocs")
        self.kv = {}   # TAG -> dict
        self.rt = {}   # restype -> resident bytes
        self.rt_redem = {}   # restype -> redemB (0.5.0 RT column / `redemB=` token; absent in older logs)
        self.evts = []   # 0.5.0 `EVT kind=...` lines of this block (dicts)
        self.tpctop = None   # 0.5.0 {"hdr": {n,total_mb,overflow}, "items": [(rank, kb, name)]}
        self.restop = None   # 0.5.0 {"items": [(rank, type, kb, name)]}
        self.hwvfb = []   # 0.5.0 `HWVFB <i> reason= target= w= h= fmt= type=` dicts

    @property
    def module(self):
        """0.5.0 header `module=<name|->`; None if absent, "-" if menu / no module."""
        m = self.hdr.get("module")
        return None if m is None else str(m)

    def g(self, tag, key, default=None):
        return self.kv.get(tag, {}).get(key, default)


_SKIP_PREFIX = ("NAME ", "NAMES ", "BRRING ", "HOOKCOST_BY ", "VQTOP ", "FREECALLER ", "REQUEUECALLER ",
                "GETIMGCALLER ", "VMCMD", "# ", "G ", "RES ", "VQ ")


def _parse_top(blk, tag, p):
    """`TPCTOP n= total_mb= [overflow=]` / `TPCTOP <rank> kb= name=` and `RESTOP <rank> type= kb= name=` (0.5.0)."""
    if len(p) > 1 and "=" in p[1]:                     # header line
        if tag == "TPCTOP":
            blk.tpctop = {"hdr": parse_kv(p[1:]), "items": []}
        return
    if len(p) < 2:
        return
    d = parse_kv(p[2:])
    rank = num(p[1])
    if tag == "TPCTOP":
        if blk.tpctop is None:
            blk.tpctop = {"hdr": {}, "items": []}
        blk.tpctop["items"].append((rank, d.get("kb"), d.get("name", "?")))
    else:
        if blk.restop is None:
            blk.restop = {"items": []}
        blk.restop["items"].append((rank, d.get("type", "?"), d.get("kb"), d.get("name", "?")))


def parse_metrics(info):
    """-> list of launches, each a list of Blocks (split at `#METRICS 1`)."""
    launches, cur, blk = [], None, None
    for ln in read_lines(info):
        if not ln:
            continue
        c = ln[0]
        if c == "#":
            if ln.startswith("#METRICS "):
                p = ln.split()
                try:
                    n = int(p[1])
                except (IndexError, ValueError):
                    continue
                hdr = parse_kv(p[3:])
                blk = Block(n, p[2] if len(p) > 2 else "?", hdr)
                if n == 1 or cur is None:
                    cur = []
                    launches.append(cur)
                cur.append(blk)
            elif ln.startswith("#END"):
                blk = None
            continue
        if blk is None or ln.startswith(_SKIP_PREFIX):
            continue
        p = ln.split()
        tag = p[0]
        if tag == "RT":
            # `RT <restype> <11 counters> <resident> [<redemB> | redemB=<n>]` (0.5.0 appends redemB); `RT stat redemB=..` tolerated
            ints = None
            if len(p) >= 2:
                try:
                    ints = int(p[1])
                except ValueError:
                    ints = None
            if ints is not None and len(p) >= 13 and "=" not in p[12]:
                try:
                    blk.rt[ints] = int(p[12])
                except ValueError:
                    pass
            red = None
            for t in p[13:]:
                if t.startswith("redemB="):
                    red = num(t.split("=", 1)[1])
                elif red is None and is_num(num(t)):
                    red = num(t)
            if red is None:
                for t in p[1:]:
                    if t.startswith("redemB="):
                        red = num(t.split("=", 1)[1])
            if is_num(red):
                if ints is not None:
                    blk.rt_redem[ints] = red
                else:
                    blk.rt_redem["stat"] = red
            continue
        if tag == "EVT":
            d = parse_kv(p[1:])
            if len(p) > 1 and p[1] == "counts":
                blk.kv.setdefault("EVTC", {}).update(d)
            elif "kind" in d:
                blk.evts.append(d)
            continue
        if tag in ("TPCTOP", "RESTOP"):
            _parse_top(blk, tag, p)
            continue
        if tag == "HWVFB":
            d = parse_kv(p[2:] if len(p) > 1 and "=" not in p[1] else p[1:])
            if len(p) > 1 and "=" not in p[1]:
                d["i"] = num(p[1])
            blk.hwvfb.append(d)
            continue
        d = parse_kv(p[1:])
        if d:
            blk.kv.setdefault(tag, {}).update(d)
    return launches


def parse_snaps(info):
    """-> list of dicts (#SNAP headers only; the S lines are skipped)."""
    out = []
    for ln in read_lines(info):
        if ln.startswith("#SNAP "):
            p = ln.split()
            if len(p) < 4:
                continue
            d = parse_kv(p[3:])
            d["n"], d["ev"] = num(p[1]), p[2]
            out.append(d)
    return out


# ----------------------------------------------------------------------------------------------------------------
# perflab_log.txt
# ----------------------------------------------------------------------------------------------------------------
_PL = re.compile(r"^\[(\d+) t(-?\d+)\] (.*)$")


def parse_perflab_log(info):
    attaches, lives = [], []
    for ln in read_lines(info):
        m = _PL.match(ln)
        if not m:
            continue
        tick, msg = int(m.group(1)), m.group(3)
        if " attach " in msg and msg.startswith("perf-lab"):
            ver = msg.split()[1]
            kv = parse_kv(msg.split())
            attaches.append({"tick": tick, "version": ver, "kv": kv})
        elif msg.startswith("live="):
            lives.append(tick)
    return attaches, lives


# ----------------------------------------------------------------------------------------------------------------
# text logs (robustness / texmem / activationfix / equipiconfix)
# ----------------------------------------------------------------------------------------------------------------
_LG = re.compile(r"^\[(\d\d):(\d\d):(\d\d)\.(\d\d\d) (\d+)\] (.*)$")


class LogBlock:
    def __init__(self, name, tick, wall_s, msg):
        self.name, self.tick, self.wall_s = name, tick, wall_s
        self.loaded = msg
        self.pid = None
        m = re.search(r"\bpid=(\d+)", msg)
        if m:
            self.pid = int(m.group(1))
        self.lines = []   # (tick, msg)

    def counters(self, tag="COUNTERS"):
        last = {}
        for _, m in self.lines:
            if m.startswith(tag + " "):
                last = parse_kv(m.split()[1:])
        return last

    def count(self, prefix):
        return sum(1 for _, m in self.lines if m.startswith(prefix))

    def where(self, prefix):
        return [m for _, m in self.lines if m.startswith(prefix)]


def _session_wall(sess):
    """'YYYY-MM-DD HH:MM:SS' -> seconds since midnight (same unit as the legacy wall_s)."""
    try:
        hh, mm, ss = sess.split()[1].split(":")
        return int(hh) * 3600 + int(mm) * 60 + int(ss)
    except (IndexError, ValueError):
        return None


def parse_textlog(info):
    """-> (blocks, headless_line_count). Accepts legacy `[hh:mm:ss.mmm tick] msg`, KpLog fallback
    `<tick> id/chan [LVL ]msg` (wall_s None) and KpLog core `T=<s> f=<n> id/chan [LVL ]msg` (tick rebuilt from the
    load line's `tick=` plus the T delta; wall_s = `# kplog session` time + T)."""
    blocks, headless = [], 0
    sess_wall = None
    t_load = None     # core T (seconds) of the current block's load line, None for legacy/fallback blocks
    for ln in read_lines(info):
        m = _LG.match(ln)
        if m:
            hh, mm, ss, ms, tick, msg = m.groups()
            wall = int(hh) * 3600 + int(mm) * 60 + int(ss) + int(ms) / 1000.0
            tick = int(tick)
            core_t = None
        else:
            p = kplog_parse.parse_line(ln)
            if p is None or p["kind"] not in ("line", "fallback", "header"):
                if blocks and ln.strip():
                    blocks[-1].lines.append((blocks[-1].tick, ln.strip()))   # continuation (e.g. "  kt REFUSED: ...")
                continue
            if p["kind"] == "header":
                if p.get("session"):
                    sess_wall = _session_wall(p["session"])
                continue
            msg = p["msg"]
            if p["kind"] == "fallback":
                tick, wall, core_t = int(p["t_ms"]), None, None
            else:
                core_t = p["t_ms"]   # core lines: t_ms is actually seconds (T=<s>.<ms>); fallback lines: milliseconds
                wall = (sess_wall + core_t) % 86400 if sess_wall is not None else None
                tick = None
        is_load = " loaded" in msg and "loaded:" in msg
        if core_t is not None:
            if is_load:
                tm = re.search(r"\btick=(\d+)", msg)
                t_load = core_t
                tick = int(tm.group(1)) if tm else int(core_t * 1000)
                blocks.append(LogBlock(info.name, tick, wall, msg))
                continue
            if not blocks:
                headless += 1
                continue
            tick = blocks[-1].tick + int(round((core_t - t_load) * 1000)) if t_load is not None else blocks[-1].tick
            blocks[-1].lines.append((tick, msg))
            continue
        if is_load:
            blocks.append(LogBlock(info.name, tick, wall, msg))
        elif blocks:
            blocks[-1].lines.append((tick, msg))
        else:
            headless += 1
    return blocks, headless


def parse_textlog_pair(old_info, new_info):
    """Per mod there may be two files (legacy/fallback name and KpLog core name). Choose the one whose newest
    `loaded:` block has the larger tick (core ticks are rebuilt from the same GetTickCount base, so they compare);
    ties and empty files fall back to the legacy name. Blocks are returned under the legacy key."""
    ob, oh = parse_textlog(old_info)
    if new_info is None:
        return ob, oh
    nb, nh = parse_textlog(new_info)
    for blk in nb:
        blk.name = old_info.name
    if nb and (not ob or tdiff(nb[-1].tick, ob[-1].tick) < 0x80000000 and nb[-1].tick != ob[-1].tick):
        return nb, nh
    return ob, oh


# ----------------------------------------------------------------------------------------------------------------
# launches
# ----------------------------------------------------------------------------------------------------------------
class Launch:
    def __init__(self, idx):
        self.idx = idx
        self.blocks = []
        self.attach = None
        self.logs = {}        # file name -> LogBlock
        self.snaps = []
        self.start = None
        self.lo = None
        self.hi = None
        self.wall_s = None
        self.pid = None
        self.pid_conflict = False
        self.stub = False

    # ---- header-derived
    @property
    def version(self):
        return self.attach["version"] if self.attach else "?"

    def flags(self):
        d = {}
        if self.attach:
            d.update({k: v for k, v in self.attach["kv"].items() if k not in ("exe", "scan")})
            if "scan" in self.attach["kv"]:
                d["scan"] = self.attach["kv"]["scan"]
        for b in self.blocks[:1]:
            for k, v in b.hdr.items():
                if k not in ("tick", "frames(hook)", "live_bytes", "allocs", "module"):
                    d.setdefault(k, v)
        return d

    @property
    def hwmip(self):
        for k, v in self.flags().items():
            if k == "hwmip":
                f = norm_flag(v)
                if f is not None:
                    return f
        return 0

    def texmem_state(self):
        b = self.logs.get("texmem_log.txt")
        if not b:
            return "absent"
        t = b.loaded
        if "DISABLED" in t:
            return "DISABLED"
        if "REFUSED" in t:
            return "REFUSED"
        if "patched" in t:
            return "patched"
        return "unknown"

    # ---- loads
    def loads(self):
        """-> (list of load dicts, list of anomalies). A load is an UP->DOWN pair; unmatched UP at end = hang."""
        out, notes = [], []
        up = None
        unloads = [b.tick for b in self.blocks if b.ev == "UNLOAD"]
        for b in self.blocks:
            if b.ev == "LOAD_UP":
                if up is not None:
                    out.append(self._mk(up, None, unloads))
                    notes.append("UP at tick %d superseded by another UP (no DOWN)" % up.tick)
                up = b
            elif b.ev == "LOAD_DOWN":
                if up is None:
                    notes.append("DOWN at tick %d without a preceding UP (UP before the read offset?)" % b.tick)
                    continue
                out.append(self._mk(up, b, unloads))
                up = None
        if up is not None:
            out.append(self._mk(up, None, unloads))
        return out, notes

    def _mk(self, up, down, unloads):
        L = {"up": up, "down": down, "ms": None, "hang": down is None, "unload": False, "gl": {}, "module": None}
        if down is not None:
            L["ms"] = tdiff(down.tick, up.tick)
            hi = down.tick + 1000
            L["unload"] = any(tdiff(u, up.tick - 1000) <= tdiff(hi, up.tick - 1000) for u in unloads)
            gu, gd = up.kv.get("GL", {}), down.kv.get("GL", {})
            for k, v in gd.items():
                if is_num(v):
                    u = gu.get(k, 0)
                    u = u if is_num(u) else 0
                    L["gl"][k] = v if "max" in k else v - u
        L["module_up"] = up.module
        L["module_down"] = down.module if down is not None else None
        for key in ("module_down", "module_up"):
            if L[key] not in (None, "-"):
                L["module"] = L[key]
                break
        else:
            if "-" in (L["module_down"], L["module_up"]):
                L["module"] = "-"
            if L["module"] is None:
                for b in (down, up):
                    if b is None:
                        continue
                    for key in ("mod", "area"):
                        if key in b.hdr:
                            L["module"] = b.hdr[key]
                            break
        return L

    def last(self):
        return self.blocks[-1] if self.blocks else None

    def cum(self, tag, key):
        """cumulative counter at the last block that has it."""
        for b in reversed(self.blocks):
            if tag in b.kv and key in b.kv[tag]:
                return b.kv[tag][key]
        return None

    def cum_max(self, tag, key):
        vals = [b.kv[tag][key] for b in self.blocks if tag in b.kv and is_num(b.kv[tag].get(key))]
        return max(vals) if vals else None

    # ---- 0.5.0 helpers
    def module_seq(self):
        """Distinct consecutive module names (header `module=`, menu `-` skipped) in block order."""
        seq = []
        for b in self.blocks:
            m = b.module
            if m is None or m == "-":
                continue
            if not seq or seq[-1] != m:
                seq.append(m)
        return seq

    def round_trip(self):
        """-> (bool, sequence): True if some module name is seen again after a different one."""
        seq = self.module_seq()
        return len(seq) != len(set(seq)), seq

    def has_evt(self):
        return any(b.evts or "EVTC" in b.kv for b in self.blocks)

    def evt_counts(self):
        """Cumulative EVT counters (last `EVT counts` line); falls back to tallying `EVT kind=` lines (deduped across blocks)."""
        for b in reversed(self.blocks):
            if "EVTC" in b.kv:
                return {k: v for k, v in b.kv["EVTC"].items() if is_num(v)}
        seen, c = set(), {}
        for b in self.blocks:
            for e in b.evts:
                key = (e.get("kind"), e.get("tick"), e.get("slot"), e.get("name"))
                if key in seen:
                    continue
                seen.add(key)
                c[e["kind"]] = c.get(e["kind"], 0) + 1
        return c

    def evt_list(self):
        seen, out = set(), []
        for b in self.blocks:
            for e in b.evts:
                key = (e.get("kind"), e.get("tick"), e.get("slot"), e.get("name"))
                if key not in seen:
                    seen.add(key)
                    out.append(e)
        return out

    def va_series(self):
        """-> list of (tick, largest_free_mb, free_mb) for blocks that carry a `VA` line."""
        out = []
        for b in self.blocks:
            v = b.kv.get("VA")
            if v and is_num(v.get("largest_free_mb")):
                out.append((b.tick, v["largest_free_mb"], v.get("free_mb")))
        return out

    def tpctops(self):
        """-> blocks that carry a TPCTOP capture (LOAD_DOWN / UNLOAD)."""
        return [b for b in self.blocks if b.tpctop is not None]

    def hwv_last(self):
        for b in reversed(self.blocks):
            if "HWV" in b.kv:
                return b.kv["HWV"]
        return None

    def hwv_first_bad(self):
        for b in self.blocks:
            v = b.kv.get("HWV", {}).get("lvl1_first_bad")
            if v not in (None, "-", 0, "none", "0"):
                return v
        return None

    def hwvfb_all(self):
        seen, out = set(), []
        for b in self.blocks:
            for d in b.hwvfb:
                key = tuple(sorted((k, str(v)) for k, v in d.items()))
                if key not in seen:
                    seen.add(key)
                    out.append(d)
        return out

    def tpc_resident_mb(self, which="last"):
        bl = [b for b in self.blocks if TPC_RESTYPE in b.rt]
        if not bl:
            return None
        return bl[-1 if which == "last" else 0].rt[TPC_RESTYPE] / MB


def build_launches(mlaunches, attaches, snaps, text_logs):
    """Create Launch objects from metrics launches + attach-only stubs, then join the text logs / snaps by tick."""
    L = []
    for blocks in mlaunches:
        x = Launch(0)
        x.blocks = blocks
        L.append(x)
    # attach -> launch: latest attach with tick <= first block tick, newer than the previous launch's last block
    used = set()
    prev_end = None
    for x in L:
        first = x.blocks[0].tick
        cand = [i for i, a in enumerate(attaches)
                if i not in used and tdiff(first, a["tick"]) < 0x80000000 and (prev_end is None or tdiff(a["tick"], prev_end) < 0x80000000 and a["tick"] > prev_end)]
        if cand:
            i = max(cand, key=lambda j: attaches[j]["tick"])
            x.attach = attaches[i]
            used.add(i)
        prev_end = x.blocks[-1].tick
    for i, a in enumerate(attaches):
        if i not in used:
            s = Launch(0)
            s.attach, s.stub = a, True
            L.append(s)
    # order by start tick
    def start_of(x):
        return x.attach["tick"] if x.attach else x.blocks[0].tick - 120000
    L.sort(key=start_of)
    prev = None
    for k, x in enumerate(L, 1):
        x.idx = k
        end = x.blocks[-1].tick if x.blocks else (x.attach["tick"] + 60000)
        if x.attach:
            x.start = x.attach["tick"]
        elif prev is not None:
            x.start = prev.hi + 1
        else:
            x.start = x.blocks[0].tick - 120000
        x.lo, x.hi = x.start, end
        prev = x
    orphans = {}

    def find(tick):
        best = None
        for x in L:
            if x.lo <= tick + MATCH_SLACK_MS and tick <= x.hi + TAIL_SLACK_MS:
                if best is None or x.lo > best.lo:
                    best = x
        return best

    for name, blocks in text_logs.items():
        orphans[name] = 0
        for b in blocks:
            x = find(b.tick)
            if x is None:
                orphans[name] += 1
                continue
            if name in x.logs:        # a second loaded line in one launch (should not happen): keep the first
                orphans[name] += 1
                continue
            x.logs[name] = b
            if b.pid is not None:
                if x.pid is None:
                    x.pid = b.pid
                elif x.pid != b.pid:
                    x.pid_conflict = True
            if x.wall_s is None and b.wall_s is not None:   # fallback-shape blocks carry no wall time
                x.wall_s = (b.wall_s - tdiff(b.tick, x.start) / 1000.0) % 86400
    snap_orph = 0
    for s in snaps:
        t = s.get("tick")
        x = find(t) if is_num(t) else None
        if x is None:
            snap_orph += 1
        else:
            x.snaps.append(s)
    return L, orphans, snap_orph


# ----------------------------------------------------------------------------------------------------------------
# analysis
# ----------------------------------------------------------------------------------------------------------------
def wall_str(x):
    if x.wall_s is None:
        return "tick %d" % x.start
    s = int(x.wall_s)
    return "%02d:%02d:%02d" % (s // 3600, (s // 60) % 60, s % 60)


def frame_summary(x):
    hist = x.cum("FRAME", "hist_ms")
    if not hist:
        return None
    tot = sum(hist)
    if tot == 0:
        return {"n": 0}

    def pct(p):
        c = 0
        for i, v in enumerate(hist):
            c += v
            if c >= p * tot:
                return FRAME_EDGES[i] if i < len(FRAME_EDGES) else str(i)
        return FRAME_EDGES[-1]
    slow = sum(hist[6:]) if len(hist) >= 7 else 0
    return {"n": tot, "p50": pct(0.5), "p95": pct(0.95), "slow32": slow, "slow32pct": 100.0 * slow / tot,
            "max_ms": x.cum("FRAME", "max_ms"), "hist": hist}


def classify_loads(loads):
    """Heuristic quicksave/quickload detection from load-screen pairs (no save/load text is logged anywhere).
    quicksave candidate = completed pair <= 1500 ms with no UNLOAD in its window, chained (gap <= 10 s) with the
    previous candidate; quickload candidate = completed pair with an UNLOAD directly following (<= 10 s) such a chain."""
    chains, cur, qloads, last_end = [], [], 0, None
    for L in loads:
        if L["down"] is None:
            continue
        chained = last_end is not None and tdiff(L["up"].tick, last_end) <= 10000
        if L["ms"] <= 1500 and not L["unload"]:
            if cur and not chained:
                chains.append(cur)
                cur = []
            cur.append(L)
        else:
            if L["unload"] and cur and chained:
                qloads += 1
            if cur:
                chains.append(cur)
                cur = []
        last_end = L["down"].tick
    if cur:
        chains.append(cur)
    return {"quicksave_pairs": max((len(c) for c in chains), default=0), "chains": [len(c) for c in chains], "quickloads": qloads}


def scenarios(x):
    loads, _ = x.loads()
    done = [L for L in loads if L["down"] is not None]
    out = {}
    # S1: longest in-module span (DOWN -> next UP, or last DOWN -> last block)
    spans = []
    for i, L in enumerate(loads):
        if L["down"] is None:
            continue
        end = loads[i + 1]["up"].tick if i + 1 < len(loads) else (x.blocks[-1].tick if x.blocks else L["down"].tick)
        spans.append(tdiff(end, L["down"].tick))
    mx = max(spans) if spans else 0
    if not done:
        out["S1"] = ("not covered", "no completed load, no in-module span")
    elif mx >= 120000:
        out["S1"] = ("covered", "longest in-module span %.0f s" % (mx / 1000.0))
    else:
        out["S1"] = ("not covered", "longest in-module span %.0f s < 120 s" % (mx / 1000.0))
    nu = sum(1 for L in done if L["unload"])
    rt, seq = x.round_trip()
    if seq:
        if rt:
            out["S2"] = ("covered", "PASS round trip by module name: %s" % " -> ".join(seq))
        else:
            out["S2"] = ("not covered", "round trip: - (no module name seen again after a different one); modules seen: %s" % " -> ".join(seq))
    elif nu >= 2:
        out["S2"] = ("covered", "%d loads with a module UNLOAD (out and back possible; no module names in this log, round trip unproven) (heuristic)" % nu)
    elif nu == 1:
        out["S2"] = ("unknown", "1 load with a module UNLOAD; return leg not provable (no module names) (heuristic)")
    else:
        out["S2"] = ("not covered", "no load with a module UNLOAD (heuristic)")
    cl = classify_loads(loads)
    if x.has_evt():
        ec = x.evt_counts()
        saves = sum(ec.get(k, 0) for k in ("save", "qsave", "autosave"))
        lds = sum(ec.get(k, 0) for k in ("load", "qload"))
        msg = "%d save-type events (save+qsave+autosave), %d load/qload (measured: EVT)" % (saves, lds)
        if saves >= 10 and lds >= 1:
            out["S3"] = ("covered", msg)
        elif saves >= 10:
            out["S3"] = ("unknown", msg + "; no load/qload seen")
        else:
            out["S3"] = ("not covered", msg + "; need >= 10 save-type and >= 1 load")
    elif cl["quicksave_pairs"] >= 10 and cl["quickloads"] >= 1:
        out["S3"] = ("covered", "%d short save-like load pairs in a chain + %d quickload-like (heuristic)" % (cl["quicksave_pairs"], cl["quickloads"]))
    elif cl["quicksave_pairs"] >= 10:
        out["S3"] = ("unknown", "%d short save-like pairs but no following UNLOAD load (quickload not seen) (heuristic)" % cl["quicksave_pairs"])
    elif cl["quicksave_pairs"] == 0 and cl["quickloads"] == 0:
        out["S3"] = ("not covered", "no short save-like load pairs (heuristic)")
    else:
        out["S3"] = ("not covered", "longest short-pair chain %d (<10), quickloads %d (heuristic)" % (cl["quicksave_pairs"], cl["quickloads"]))
    rb = x.logs.get("robustnessfixes_log.txt")
    ec = x.evt_counts() if x.has_evt() else None
    snd = ec.get("sndapply", 0) if ec is not None else None
    opt = ec.get("optsave", 0) if ec is not None else None
    if rb is None:
        out["SNDRI"] = ("unknown", "no robustness log block" + ("" if ec is None else "; EVT sndapply=%s optsave=%s" % (snd, opt)))
    else:
        c = rb.counters()
        fired = [m for m in rb.where("FIRED") if "sndri" in m]
        v = c.get("sndri_restart")
        if fired or (is_num(v) and v > 0):
            out["SNDRI"] = ("covered", "fired: sndri_restart=%s%s%s" % (v, ", FIRED line" if fired else "",
                                                                    "" if ec is None else "; EVT sndapply=%s optsave=%s" % (snd, opt)))
        elif ec is None:
            out["SNDRI"] = ("unknown", "sndri_restart=%s (0 = options/sound apply not observed, or no restart needed) (no EVT counts)" % fmt(v))
        elif snd > 0:
            out["SNDRI"] = ("toggled, no fire", "sndapply=%s optsave=%s, sndri_restart=%s, no FIRED sndri" % (snd, opt, fmt(v)))
        else:
            out["SNDRI"] = ("not toggled", "sndapply=0 optsave=%s: sound options were never applied, no verdict" % opt)
    out["_cl"] = cl
    return out


def analyze(x):
    a = {}
    loads, lnotes = x.loads()
    a["loads"], a["load_notes"] = loads, lnotes
    done = [L for L in loads if L["down"] is not None]
    a["first"] = done[0] if done else None
    a["hang"] = any(L["hang"] for L in loads)
    a["hang_n"] = sum(1 for L in loads if L["hang"])
    a["other_ret"] = x.cum("GETIMG", "other_tpc_retained")
    a["ctrl_ret"] = x.cum("GETIMG", "ctrl_tpc_retained")
    if a["other_ret"] is None:
        a["gate"] = "n/a"
    else:
        a["gate"] = "PASS" if a["other_ret"] == 0 else "FAIL"
    a["kt_sid"] = x.cum("KT", "still_in_demand")
    a["kt_uaf"] = x.cum("KT", "uaf_write")
    a["tpc_mb"] = x.tpc_resident_mb("last")
    a["netmax"] = x.cum_max("NET", "max_len")
    a["vm_bad"] = x.cum("VM", "exec_bad_id")
    a["vm_lim"] = x.cum("VM", "instr_limit")
    a["ab_calls"] = x.cum("FRAME", "actionbar_calls")
    a["ab_max"] = x.cum_max("FRAME", "max_actionbar_per_frame")
    a["frame"] = frame_summary(x)
    a["scen"] = scenarios(x)
    a["requeue_ret"] = x.cum("BUILD", "requeue_tpc_retained")
    hw = {}
    for b in x.blocks:
        for tag, d in b.kv.items():
            for k, v in d.items():
                if k.startswith("hwmip") and is_num(v):
                    hw[k] = v
    a["hw"] = hw
    # 0.5.0
    a["requeue_kb"] = x.cum("BUILD", "requeue_tpc_retained_kb")
    a["txi_kb"] = x.cum("KT", "txi_via_tpc_kb")
    a["gms_calls"] = x.cum("FRAME", "gms_calls")
    a["gms_ms"] = x.cum("FRAME", "gms_ms")
    reds = [b.rt_redem[TPC_RESTYPE] for b in x.blocks if TPC_RESTYPE in b.rt_redem]
    a["redemB"] = reds[-1] if reds else None
    a["round_trip"], a["modules"] = x.round_trip()
    va = x.va_series()
    if va:
        lf = [v[1] for v in va]
        mn = min(lf)
        first, last = lf[0], lf[-1]
        thr = max(32.0, 0.10 * first)
        trend = "falling" if last < first - thr else "rising" if last > first + thr else "flat"
        a["va"] = {"n": len(va), "min": mn, "last": last, "first": first, "trend": trend, "warn": mn < VA_WARN_MB,
                   "min_free": min((v[2] for v in va if is_num(v[2])), default=None)}
    else:
        a["va"] = None
    tops = x.tpctops()
    a["tpc_first"] = tops[0] if tops else None
    a["tpc_largest"] = max(tops, key=lambda b: b.tpctop["hdr"].get("total_mb", 0) if is_num(b.tpctop["hdr"].get("total_mb")) else 0) if tops else None
    hwv = x.hwv_last()
    if hwv is not None:
        bad = hwv.get("lvl1_bad")
        a["hwv"] = {"v": hwv, "bad": bad, "ok": hwv.get("lvl1_ok"), "first_bad": x.hwv_first_bad(),
                    "verdict": "FAIL" if (is_num(bad) and bad > 0) else "PASS" if is_num(bad) else "n/a", "fb": x.hwvfb_all()}
    else:
        a["hwv"] = None
    return a


# ----------------------------------------------------------------------------------------------------------------
# rendering
# ----------------------------------------------------------------------------------------------------------------
def mbs(v):
    return None if v is None else v / MB


def mod_label(L):
    """Module column text: n/a (no module= header, perf-lab < 0.5.0), menu, name, or `up -> down` when they differ."""
    u, d = L.get("module_up"), L.get("module_down")
    nm = lambda v: "menu" if v == "-" else v
    if u is None and d is None:
        return L["module"] if L["module"] not in (None, "-") else ("menu" if L["module"] == "-" else "n/a")
    if d is None:
        return "%s -> ?" % nm(u)
    if u is None or u == d:
        return nm(d)
    return "%s -> %s" % (nm(u), nm(d))


def render_top(title, b, restop=False):
    """TPCTOP (top 20) table of one capture block, optionally followed by its RESTOP (top 10)."""
    h = b.tpctop["hdr"]
    o = ["", "%s: block #%d %s tick %d, module %s; n=%s total_mb=%s%s" % (
        title, b.n, b.ev, b.tick, mod_label({"module": b.module, "module_up": b.module, "module_down": b.module}),
        fmt(h.get("n")), fmt(h.get("total_mb"), 2), (" overflow=%s" % fmt(h["overflow"])) if h.get("overflow") else "")]
    o += [""] + table(["rank", "kb", "name"], [[r, fmt(kb), nm] for r, kb, nm in b.tpctop["items"][:20]])
    if restop and b.restop:
        o += ["", "RESTOP top %d of the same block:" % min(10, len(b.restop["items"]))]
        o += [""] + table(["rank", "type", "kb", "name"], [[r, t, fmt(kb), nm] for r, t, kb, nm in b.restop["items"][:10]])
    return o


def render_launch_050(x, a):
    """0.5.0 sections: modules / EVT / VA / TPCTOP / HWV. Empty list for 0.3.0/0.4.0 logs."""
    o = []
    if a["modules"]:
        o.append("- modules seen (header `module=`): %s; **round trip (same module before and after a different one): %s**" % (
            " -> ".join(a["modules"]),
            ("PASS (%s)" % " -> ".join(a["modules"])) if a["round_trip"] else "- (none)"))
    if x.has_evt():
        ec = x.evt_counts()
        ev = x.evt_list()
        o.append("- EVT counts (cumulative since attach): " + " ".join("%s=%s" % (k, fmt(ec[k])) for k in
                                                                       ("save", "load", "qsave", "qload", "autosave", "optsave", "sndapply") if k in ec) +
                 ("; extra: " + " ".join("%s=%s" % kv for kv in sorted(ec.items()) if kv[0] not in
                                         ("save", "load", "qsave", "qload", "autosave", "optsave", "sndapply")) if
                  any(k not in ("save", "load", "qsave", "qload", "autosave", "optsave", "sndapply") for k in ec) else ""))
        if ev:
            o.append("- EVT lines (%d, last 5): %s" % (len(ev), "; ".join(
                "%s@%s slot=%s name=%s" % (e.get("kind"), e.get("tick"), fmt(e.get("slot")), e.get("name")) for e in ev[-5:])))
    va = a["va"]
    if va:
        o.append("- VA (address space): min largest_free_mb %s, last %s, first %s, trend %s (%d samples)%s%s" % (
            fmt(va["min"]), fmt(va["last"]), fmt(va["first"]), va["trend"], va["n"],
            (", min free_mb %s" % fmt(va["min_free"])) if va["min_free"] is not None else "",
            ("; **WARN: min largest_free_mb %s < %d MB**" % (fmt(va["min"]), VA_WARN_MB)) if va["warn"] else ""))
    first, big = a["tpc_first"], a["tpc_largest"]
    if first is not None:
        if big is first:
            o += render_top("TPCTOP top 20 (first capture = largest total_mb)", first, restop=True)
        else:
            o += render_top("TPCTOP top 20, first capture", first)
            o += render_top("TPCTOP top 20, largest total_mb", big, restop=True)
        o.append("")
    hw = a["hwv"]
    if hw:
        v = hw["v"]
        o.append("- HWV (hwmip formats, last): %s; lvl1 ok=%s bad=%s -> **%s**%s; HWVFB entries %d" % (
            " ".join("%s=%s" % (k, fmt(v[k])) for k in ("fmt_rgba", "fmt_rgb", "fmt_la", "fmt_l", "fmt_other") if k in v) or "no fmt_ counters",
            fmt(hw["ok"]), fmt(hw["bad"]), hw["verdict"], (" (first bad %s)" % hw["first_bad"]) if hw["first_bad"] else "", len(hw["fb"])))
    return o


def render_launch(x, a):
    o = []
    o.append("### Launch %d: %s, pid %s, perf-lab %s%s" % (x.idx, wall_str(x), x.pid if x.pid is not None else "?", x.version,
                                                          "  (STUB: attach line only, no metrics block)" if x.stub else ""))
    fl = x.flags()
    o.append("- start tick %d; blocks %d (last event %s, tick %s); duration %s s" % (
        x.start, len(x.blocks), x.last().ev if x.blocks else "-", x.last().tick if x.blocks else "-",
        fmt(tdiff(x.hi, x.start) / 1000.0) if x.blocks else "-"))
    o.append("- config/header flags: " + (" ".join("%s=%s" % (k, v) for k, v in fl.items()) or "(no attach/header flags)") +
             " | hwmip=%d | texmem=%s" % (x.hwmip, x.texmem_state()))
    if x.pid_conflict:
        o.append("- WARNING: matched logs disagree on pid")
    if a["hang"]:
        o.append("- **HANG: %d load(s) with LOAD_UP and no matching LOAD_DOWN (UP tick %s)**" % (
            a["hang_n"], ", ".join(str(L["up"].tick) for L in a["loads"] if L["hang"])))
    for n in a["load_notes"]:
        o.append("- note: " + n)
    f = a["first"]
    o.append("- first load UP->DOWN: %s" % ("%d ms" % f["ms"] if f else "none completed"))
    # loads table
    if a["loads"]:
        glkeys = []
        for L in a["loads"]:
            for k in L["gl"]:
                if k not in glkeys:
                    glkeys.append(k)
        main = ["mip_ms", "mipmaps", "glfinish_ms"]
        hwk = [k for k in glkeys if k.startswith("hwmip")]
        other = [k for k in glkeys if k not in main and k not in hwk]
        hdr = ["#", "ms", "module", "unload", "mip_ms", "mipmaps", "glfinish_ms"] + hwk + ["other GL", "heap up MB", "heap down MB",
                                                                                       "d heap vs prev down", "TPC MB @down", "textures @down"]
        rows = []
        prev_down_heap = None
        for i, L in enumerate(a["loads"], 1):
            up, dn = L["up"], L["down"]
            hu = mbs(up.live)
            hd = mbs(dn.live) if dn else None
            dd = (hd - prev_down_heap) if (hd is not None and prev_down_heap is not None) else None
            if hd is not None:
                prev_down_heap = hd
            if dn is None:
                rows.append([i, "**HANG**", "-", "-", "-", "-", "-"] + ["-"] * len(hwk) + ["-", fmt(hu), "-", "-", "-", "-"])
                continue
            g = L["gl"]
            rows.append([i, L["ms"], mod_label(L), "yes" if L["unload"] else "no",
                         fmt(g.get("mip_ms")), fmt(g.get("mipmaps")), fmt(g.get("glfinish_ms"))] +
                        [fmt(g.get(k)) for k in hwk] +
                        [" ".join("%s=%s" % (k, fmt(g[k])) for k in other if k in g and g[k] != 0) or "-",
                         fmt(hu), fmt(hd), fmt(dd), fmt(dn.rt.get(TPC_RESTYPE) / MB if TPC_RESTYPE in dn.rt else None),
                         fmt(dn.g("LIVE", "textures"))])
        o += [""] + table(hdr, rows) + [""]
        o.append("(GL columns are UP->DOWN deltas of the cumulative GL counters, `*max*` keys show the DOWN value; `module` is the "
                 "`module=` header key of perf-lab 0.5.0+ (`a -> b` when the UP and DOWN blocks differ, `menu` = `-`, `n/a` on 0.3.0/0.4.0 logs); "
                 "`unload` = a perf-lab UNLOAD block within the load window.)")
    # counters
    o.append("")
    o.append("- GETIMG (end of launch): other_tpc_retained=%s ctrl_tpc_retained=%s; BUILD requeue_tpc_retained=%s%s; **run B gate (other_tpc_retained == 0): %s**" % (
        fmt(a["other_ret"]), fmt(a["ctrl_ret"]), fmt(a["requeue_ret"]),
        (" (%s kb)" % fmt(a["requeue_kb"])) if a["requeue_kb"] is not None else "", a["gate"]))
    o.append("- RT: TPC (restype 3007) resident at last block %s MB (at first block %s MB)%s" % (
        fmt(a["tpc_mb"]), fmt(x.tpc_resident_mb("first")),
        ("; redemB (last) %s B = %s MB" % (fmt(a["redemB"]), fmt(a["redemB"] / MB, 2))) if a["redemB"] is not None else ""))
    kt_txi = ("; txi_via_tpc=%s (%s kb)" % (fmt(x.cum("KT", "txi_via_tpc")), fmt(a["txi_kb"]))) if a["txi_kb"] is not None else ""
    unl = [b for b in x.blocks if b.ev == "UNLOAD" and "KT" in b.kv]
    if unl:
        o.append("- KT per UNLOAD (cumulative still_in_demand/uaf_write): " + ", ".join(
            "#%d %s/%s" % (b.n, fmt(b.g("KT", "still_in_demand")), fmt(b.g("KT", "uaf_write"))) for b in unl) +
                 "; end of launch %s/%s%s. Note: since robustness 1.1.0, uaf_write counts writes into the KT dummy = orphan released, not a real UAF." % (
                     fmt(a["kt_sid"]), fmt(a["kt_uaf"]), kt_txi))
    else:
        o.append("- KT: no UNLOAD blocks with KT data; end of launch still_in_demand=%s uaf_write=%s (since robustness 1.1.0 uaf_write = writes into the KT dummy, not a real UAF)%s" % (
            fmt(a["kt_sid"]), fmt(a["kt_uaf"]), kt_txi))
    o.append("- NET max_len=%s; VM exec_bad_id=%s instr_limit=%s; AB 0x74CFE0 actionbar_calls=%s, max per frame=%s" % (
        fmt(a["netmax"]), fmt(a["vm_bad"]), fmt(a["vm_lim"]), fmt(a["ab_calls"]), fmt(a["ab_max"])))
    fs = a["frame"]
    if fs and fs.get("n"):
        o.append("- frame-time histogram (ms buckets, cumulative): n=%d, p50 bucket %s ms, p95 bucket %s ms, >=32 ms: %d (%.2f%%), max %s ms; counts %s" % (
            fs["n"], fs["p50"], fs["p95"], fs["slow32"], fs["slow32pct"], fmt(fs["max_ms"]), " ".join(str(v) for v in fs["hist"])))
    else:
        o.append("- frame-time histogram: none")
    if a["gms_calls"] is not None or a["gms_ms"] is not None:
        o.append("- FRAME gms (GetMessage-style wait, cumulative): gms_calls=%s gms_ms=%s" % (fmt(a["gms_calls"]), fmt(a["gms_ms"])))
    if a["hw"]:
        o.append("- hwmip counters (last): " + " ".join("%s=%s" % (k, fmt(v)) for k, v in sorted(a["hw"].items())))
    if x.snaps:
        evs = {}
        for s in x.snaps:
            evs[s["ev"]] = evs.get(s["ev"], 0) + 1
        o.append("- alloc snapshots: %d (%s); max live_bytes %s MB" % (len(x.snaps), ", ".join("%s %d" % kv for kv in sorted(evs.items())),
                                                                       fmt(max((mbs(s.get("live_bytes")) or 0) for s in x.snaps))))
    # scenarios
    sc = a["scen"]
    cl = sc["_cl"]
    o.append("- save/load events: %s heuristic from load pairs gives quicksave-like pairs (longest chain) %d, quickload-like %d%s" % (
        "measured by EVT (see below; S3 uses it); for comparison only, the" if x.has_evt() else "no EVT lines in this launch (pre-0.5.0 log), so",
        cl["quicksave_pairs"], cl["quickloads"], " (heuristic)"))
    o.append("- scenarios: " + "; ".join("**%s %s** (%s)" % (k, v[0], v[1]) for k, v in sc.items() if not k.startswith("_")))
    o += render_launch_050(x, a)
    # logs
    rb = x.logs.get("robustnessfixes_log.txt")
    if rb:
        c = rb.counters()
        fired = rb.where("FIRED")
        ref = [m for m in rb.where("REFUSED") + [m for _, m in rb.lines if "REFUSED" in m and not m.startswith("COUNTERS")]]
        ref = sorted(set(ref)) if "REFUSED" in rb.loaded or ref else []
        mm = re.search(r"loaded: (\d+)/(\d+) groups patched", rb.loaded)
        o.append("- robustness: %s; groups patched %s; REFUSED %s; FIRED %s; last COUNTERS %s" % (
            rb.loaded.split(" loaded")[0], "%s/%s" % mm.groups() if mm else "?", (" / ".join(ref) if ref else "none"),
            (" / ".join(fired) if fired else "none"),
            " ".join("%s=%s" % (k, v) for k, v in c.items()) or "none"))
    else:
        o.append("- robustness: no log block matched")
    tm = x.logs.get("texmem_log.txt")
    if tm:
        c = tm.counters()
        o.append("- texmem: %s; counters %s; perf-lab requeue_tpc_retained=%s" % (
            x.texmem_state(), " ".join("%s=%s" % (k, v) for k, v in c.items()) or "none logged", fmt(a["requeue_ret"])))
    else:
        o.append("- texmem: no log block matched")
    af = x.logs.get("activationfix_log.txt")
    if af:
        o.append("- activationfix: %s; IGNORED %d, PASSED %d" % (
            "DISABLED" if "DISABLED" in af.loaded else "REFUSED" if "REFUSED" in af.loaded else "patched", af.count("IGNORED"), af.count("PASSED")))
    else:
        o.append("- activationfix: no log block matched")
    ei = x.logs.get("equipiconfix_log.txt")
    if ei:
        st = ei.counters("STATS")
        o.append("- equipiconfix: loaded; last STATS %s; CANCEL lines %d" % (" ".join("%s=%s" % (k, v) for k, v in st.items()) or "none", ei.count("CANCEL")))
    return o


def summary_rows(L, A):
    rows = []
    for x in L:
        a = A[x.idx]
        f = a["first"]
        rb = x.logs.get("robustnessfixes_log.txt")
        af = x.logs.get("activationfix_log.txt")
        sc = a["scen"]
        rows.append([x.idx, wall_str(x), x.pid if x.pid is not None else "?", x.version, x.hwmip, x.texmem_state(),
                     len([l for l in a["loads"] if not l["hang"]]),
                     ("%d" % f["ms"]) if f else "-", "**HANG**" if a["hang"] else "no",
                     fmt(a["other_ret"]), fmt(a["ctrl_ret"]), a["gate"], fmt(a["kt_sid"]), fmt(a["kt_uaf"]),
                     fmt(a["tpc_mb"]), fmt(mbs(f["up"].live) if f else None), fmt(mbs(f["down"].live) if f and f["down"] else None),
                     len(rb.where("FIRED")) if rb else "-", ("%d/%d" % (af.count("IGNORED"), af.count("PASSED"))) if af else "-",
                     sc["S1"][0], sc["S2"][0], sc["S3"][0], sc["SNDRI"][0],
                     ("%s%s" % (fmt(a["va"]["min"]), " WARN" if a["va"]["warn"] else "")) if a["va"] else "-"])
    return rows


SUMMARY_HDR = ["#", "start", "pid", "ver", "hwmip", "texmem", "loads", "first load ms", "hang", "other_tpc_ret", "ctrl_tpc_ret", "run B gate",
               "KT still_in_demand", "KT uaf_write", "TPC MB", "heap UP MB (1st)", "heap DOWN MB (1st)", "robust FIRED", "act IGN/PASS",
               "S1", "S2", "S3", "SNDRI", "VA min largest MB"]


def render_arms(mode, L, A):
    o = []
    if mode == "texmem":
        key = lambda x: x.texmem_state()
        title = "Arms: texmem state"
    else:
        key = lambda x: "hwmip=%d" % x.hwmip
        title = "Arms: hwmip state"
    arms = {}
    for x in L:
        arms.setdefault(key(x), []).append(x)
    o.append("## " + title)
    o.append("")
    if mode == "texmem":
        hdr = ["arm", "n", "first-load ms (median)", "TPC resident MB", "heap UP MB", "heap DOWN MB", "tpc_freed", "tpc_kept_controller", "requeue_tpc_retained"]
    else:
        hdr = ["arm", "n", "first-load ms (median)", "first-load mip_ms (median)", "mip_ms all loads (median)"]
        hwkeys = sorted({k for x in L for k in A[x.idx]["hw"]})
        hdr += ["median " + k for k in hwkeys]
    rows = []
    for name in sorted(arms):
        xs = arms[name]
        fl = [A[x.idx]["first"] for x in xs]
        first_ms = med([f["ms"] for f in fl if f])
        if mode == "texmem":
            def cnt(x, k):
                b = x.logs.get("texmem_log.txt")
                return b.counters().get(k) if b else None
            rows.append([name, len(xs), fmt(first_ms), fmt(med([A[x.idx]["tpc_mb"] for x in xs])),
                         fmt(med([mbs(f["up"].live) for f in fl if f])), fmt(med([mbs(f["down"].live) for f in fl if f and f["down"]])),
                         fmt(med([cnt(x, "tpc_freed") for x in xs])), fmt(med([cnt(x, "tpc_kept_controller") for x in xs])),
                         fmt(med([A[x.idx]["requeue_ret"] for x in xs]))])
        else:
            allmip = [l["gl"].get("mip_ms") for x in xs for l in A[x.idx]["loads"] if l["down"] is not None]
            rows.append([name, len(xs), fmt(first_ms), fmt(med([f["gl"].get("mip_ms") for f in fl if f])), fmt(med(allmip))] +
                        [fmt(med([A[x.idx]["hw"].get(k) for x in xs])) for k in hwkeys])
    o += table(hdr, rows)
    o.append("")
    o.append("Launches per arm: " + "; ".join("%s = %s" % (n, ",".join(str(x.idx) for x in xs)) for n, xs in sorted(arms.items())) +
             ". Medians skip launches without the value (a hung launch has no first load).")
    o.append("")
    if mode == "hwmip":
        o += render_hwv(L, A)
    return o


def render_hwv(L, A):
    """`--arms hwmip` (0.5.0): HWV per-format counts, lvl1 ok/bad (FAIL if any bad) and the HWVFB fallback list per launch."""
    o = ["### HWV (hwmip upload formats, perf-lab 0.5.0+)", ""]
    xs = [x for x in L if A[x.idx]["hwv"]]
    if not xs:
        return o + ["(no HWV lines in these launches: perf-lab < 0.5.0 or hwmip arm off)", ""]
    rows = []
    for x in xs:
        h = A[x.idx]["hwv"]
        v = h["v"]
        rows.append([x.idx, x.hwmip] + [fmt(v.get(k)) for k in ("fmt_rgba", "fmt_rgb", "fmt_la", "fmt_l", "fmt_other")] +
                    [fmt(h["ok"]), fmt(h["bad"]), "**%s**" % h["verdict"], h["first_bad"] or "-", len(h["fb"])])
    o += table(["launch", "hwmip", "rgba", "rgb", "la", "l", "other", "lvl1 ok", "lvl1 bad", "verdict", "first bad (target/w/got)", "HWVFB"], rows)
    o.append("")
    bad = [str(x.idx) for x in xs if A[x.idx]["hwv"]["verdict"] == "FAIL"]
    o.append("**HWV lvl1 verdict: %s**" % (("FAIL (launch %s)" % ", ".join(bad)) if bad else "PASS (no lvl1_bad in %d launches)" % len(xs)))
    for x in xs:
        fb = A[x.idx]["hwv"]["fb"]
        if fb:
            o += ["", "HWVFB (hwmip fallbacks) launch %d:" % x.idx, ""]
            o += table(["i", "reason", "target", "w", "h", "fmt", "type"],
                       [[fmt(d.get("i")), d.get("reason", "-"), d.get("target", "-"), fmt(d.get("w")), fmt(d.get("h")), d.get("fmt", "-"), d.get("type", "-")] for d in fb])
    o.append("")
    return o


def verdict_line(L, A):
    """Final line of every output. gateB is FAIL only when some launch's run B gate is FAIL (N/A counts as not failed)."""
    hung = [x for x in L if A[x.idx]["hang"]]
    gate_ok = not any(A[x.idx]["gate"] == "FAIL" for x in L)
    return "VERDICT: %s launches=%d hangs=%d gateB=%s" % ("PASS" if gate_ok and not hung else "FAIL", len(L), len(hung), "PASS" if gate_ok else "FAIL")


def render(game_dir, infos, offsets_used, L, A, orphans, snap_orph, headless, arms, cfg_text, latest=None, summary=False):
    o = ["# perf-lab run analysis", "", "Game dir: `%s`" % game_dir, ""]
    if not summary:
        o.append("## Sources")
        rows = [[n, offsets_used.get(n, 0), infos[n].size if infos[n].present else "-", infos[n].status()] for n in ALL_FILES]
        o += table(["file", "start offset", "size", "status"], rows)
        for n, t in cfg_text.items():
            o.append("")
            o.append("`%s` (current, not per launch): %s" % (n, t))
        o.append("")
    o.append("Launches found: %d (%d with metrics, %d attach-only stubs)." % (len(L), sum(1 for x in L if not x.stub), sum(1 for x in L if x.stub)))
    extra = []
    for n in LOG_FILES:
        if orphans.get(n) or headless.get(n):
            extra.append("%s: %d block(s) outside any launch, %d line(s) before the first loaded line" % (n, orphans.get(n, 0), headless.get(n, 0)))
    if snap_orph:
        extra.append("perflab_alloc_snaps.txt: %d snapshot(s) outside any launch" % snap_orph)
    if extra:
        o.append("Unmatched: " + "; ".join(extra) + ".")
    o.append("")
    shown = L[-latest:] if latest is not None and latest > 0 else L
    cut = len(shown) < len(L)
    o.append("## Summary (last %d of %d launches)" % (len(shown), len(L)) if cut else "## Summary (all launches)")
    o.append("")
    if shown:
        o += table(SUMMARY_HDR, summary_rows(shown, A))
    else:
        o.append("(no launches)")
    o.append("")
    tag = (" [all %d launches]" % len(L)) if cut else ""
    gates = [A[x.idx]["gate"] for x in L]
    real = [g for g in gates if g != "n/a"]
    if not real:
        o.append("**Overall run B gate (other_tpc_retained == 0 in every launch)%s: N/A (no GETIMG data)**" % tag)
    elif all(g == "PASS" for g in real):
        o.append("**Overall run B gate (other_tpc_retained == 0 in every launch)%s: PASS (%d launches)**" % (tag, len(real)))
    else:
        bad = [str(x.idx) for x in L if A[x.idx]["gate"] == "FAIL"]
        o.append("**Overall run B gate (other_tpc_retained == 0 in every launch)%s: FAIL (launch %s)**" % (tag, ", ".join(bad)))
    hung = [str(x.idx) for x in L if A[x.idx]["hang"]]
    o.append("**Hangs (UP without DOWN)%s: %s**" % (tag, ("launch " + ", ".join(hung)) if hung else "none"))
    o.append("")
    if not summary:
        if arms:
            o += render_arms(arms, shown, A)
        o.append("## Launch details")
        o.append("")
        for x in shown:
            o += render_launch(x, A[x.idx])
            o.append("")
    o.append(verdict_line(L, A))
    return "\n".join(o) + "\n"


# ----------------------------------------------------------------------------------------------------------------
def run(game_dir, offsets, arms=None, latest=None, summary=False):
    infos = {n: ReadInfo(n, os.path.join(game_dir, n), offsets.get(n, 0)) for n in ALL_FILES}
    mlaunches = parse_metrics(infos["perflab_metrics.txt"])
    snaps = parse_snaps(infos["perflab_alloc_snaps.txt"])
    attaches, _ = parse_perflab_log(infos["perflab_log.txt"])
    text, headless = {}, {}
    for n in LOG_FILES:
        alias = LOG_ALIASES.get(n)
        ainfo = ReadInfo(alias, os.path.join(game_dir, alias), offsets.get(alias, 0)) if alias else None
        if ainfo is not None and not ainfo.present:
            ainfo = None
        text[n], headless[n] = parse_textlog_pair(infos[n], ainfo)
    L, orphans, snap_orph = build_launches(mlaunches, attaches, snaps, text)
    A = {x.idx: analyze(x) for x in L}
    cfg = {}
    for n in CONFIG_FILES:
        p = os.path.join(game_dir, n)
        if os.path.isfile(p):
            try:
                with open(p, errors="replace") as f:
                    cfg[n] = "`" + " ".join(f.read().split()) + "`"
            except OSError:
                cfg[n] = "(unreadable)"
        else:
            cfg[n] = "absent"
    md = render(game_dir, infos, offsets, L, A, orphans, snap_orph, headless, arms, cfg, latest, summary)
    return md, L, A


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--game-dir", default=DEFAULT_GAME_DIR)
    ap.add_argument("--from-offsets", metavar="FILE")
    ap.add_argument("--out", metavar="OUT.md")
    ap.add_argument("--arms", choices=("texmem", "hwmip"))
    ap.add_argument("--latest", type=int, metavar="N", help="summary table and launch details for the last N launches only (overall gate/hang lines still cover all)")
    ap.add_argument("--summary", action="store_true", help="only the summary table and overall lines (no Sources, no Launch details)")
    args = ap.parse_args(argv)
    offsets = dict(DEFAULT_OFFSETS)
    if args.from_offsets:
        offsets = load_offsets(args.from_offsets)   # an explicit file replaces the defaults; unlisted files start at 0
    md, _, _ = run(args.game_dir, offsets, args.arms, args.latest, args.summary)
    if args.out:
        with open(args.out, "w") as f:
            f.write(md)
    else:
        sys.stdout.write(md)
    return 0


if __name__ == "__main__":
    sys.exit(main())
