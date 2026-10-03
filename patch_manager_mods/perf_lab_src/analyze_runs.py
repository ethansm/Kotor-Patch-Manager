#!/usr/bin/env python3
"""perf-lab / robustness / texmem / activationfix run analyzer (stdlib only, read-only).

Usage:
  analyze_runs.py [--game-dir DIR] [--from-offsets FILE] [--out OUT.md] [--arms texmem|hwmip]

Inputs (all optional; a missing file is reported as absent, never a crash):
  perflab_metrics.txt, perflab_alloc_snaps.txt, perflab_log.txt, robustnessfixes_log.txt, texmem_log.txt,
  activationfix_log.txt, equipiconfix_log.txt (+ the config files perflab_debug.txt, texmem.txt, shown as-is).

Offsets: each log is read from a byte offset (default table below = doc 10 run A; `--from-offsets FILE` has lines
`name bytes [ignored...]`, `#` comments).  The file is opened, seeked to the offset, and if the byte before the offset
is not a newline the partial first line is discarded, so a mid-line offset never produces a garbage record.

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
ALL_FILES = ("perflab_metrics.txt", "perflab_alloc_snaps.txt", "perflab_log.txt") + LOG_FILES
CONFIG_FILES = ("perflab_debug.txt", "texmem.txt")
MATCH_SLACK_MS = 2000
TAIL_SLACK_MS = 5000
FRAME_EDGES = ["<1", "1", "2-3", "4-7", "8-15", "16-31", "32-63", "64-127", "128-255", ">=256"]  # ms, PerfLab.cpp lbucket
TPC_RESTYPE = 3007
MB = 1024.0 * 1024.0


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
    __slots__ = ("n", "ev", "tick", "frames", "live", "allocs", "hdr", "kv", "rt")

    def __init__(self, n, ev, hdr):
        self.n, self.ev = n, ev
        self.hdr = hdr
        self.tick = hdr.get("tick", 0)
        self.frames = hdr.get("frames(hook)")
        self.live = hdr.get("live_bytes")
        self.allocs = hdr.get("allocs")
        self.kv = {}   # TAG -> dict
        self.rt = {}   # restype -> resident bytes

    def g(self, tag, key, default=None):
        return self.kv.get(tag, {}).get(key, default)


_SKIP_PREFIX = ("NAME ", "NAMES ", "BRRING ", "HOOKCOST_BY ", "VQTOP ", "FREECALLER ", "REQUEUECALLER ",
                "GETIMGCALLER ", "VMCMD", "# ", "G ", "RES ", "VQ ")


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
            if len(p) >= 13:
                try:
                    blk.rt[int(p[1])] = int(p[12])
                except ValueError:
                    pass
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


def parse_textlog(info):
    """-> (blocks, headless_line_count)"""
    blocks, headless = [], 0
    for ln in read_lines(info):
        m = _LG.match(ln)
        if not m:
            if blocks and ln.strip():
                blocks[-1].lines.append((blocks[-1].tick, ln.strip()))   # continuation (e.g. "  kt REFUSED: ...")
            continue
        hh, mm, ss, ms, tick, msg = m.groups()
        wall = int(hh) * 3600 + int(mm) * 60 + int(ss) + int(ms) / 1000.0
        if " loaded" in msg and "loaded:" in msg:
            blocks.append(LogBlock(info.name, int(tick), wall, msg))
        elif blocks:
            blocks[-1].lines.append((int(tick), msg))
        else:
            headless += 1
    return blocks, headless


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
                if k not in ("tick", "frames(hook)", "live_bytes", "allocs"):
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
            for b in (down, up):
                for key in ("module", "mod", "area"):
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
            if x.wall_s is None:
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
    if nu >= 2:
        out["S2"] = ("covered", "%d loads with a module UNLOAD (out and back possible; module names are not logged, round trip unproven)" % nu)
    elif nu == 1:
        out["S2"] = ("unknown", "1 load with a module UNLOAD; return leg not provable (no module names)")
    else:
        out["S2"] = ("not covered", "no load with a module UNLOAD")
    cl = classify_loads(loads)
    if cl["quicksave_pairs"] >= 10 and cl["quickloads"] >= 1:
        out["S3"] = ("covered", "%d short save-like load pairs in a chain + %d quickload-like (heuristic)" % (cl["quicksave_pairs"], cl["quickloads"]))
    elif cl["quicksave_pairs"] >= 10:
        out["S3"] = ("unknown", "%d short save-like pairs but no following UNLOAD load (quickload not seen)" % cl["quicksave_pairs"])
    elif cl["quicksave_pairs"] == 0 and cl["quickloads"] == 0:
        out["S3"] = ("not covered", "no short save-like load pairs")
    else:
        out["S3"] = ("not covered", "longest short-pair chain %d (<10), quickloads %d" % (cl["quicksave_pairs"], cl["quickloads"]))
    rb = x.logs.get("robustnessfixes_log.txt")
    if rb is None:
        out["SNDRI"] = ("unknown", "no robustness log block")
    else:
        c = rb.counters()
        fired = [m for m in rb.where("FIRED") if "sndri" in m]
        v = c.get("sndri_restart")
        if fired or (is_num(v) and v > 0):
            out["SNDRI"] = ("covered", "sndri_restart=%s%s" % (v, ", FIRED line" if fired else ""))
        else:
            out["SNDRI"] = ("unknown", "sndri_restart=%s (0 = options/sound apply not observed, or no restart needed)" % fmt(v))
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
    return a


# ----------------------------------------------------------------------------------------------------------------
# rendering
# ----------------------------------------------------------------------------------------------------------------
def mbs(v):
    return None if v is None else v / MB


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
            rows.append([i, L["ms"], L["module"] or "n/a", "yes" if L["unload"] else "no",
                         fmt(g.get("mip_ms")), fmt(g.get("mipmaps")), fmt(g.get("glfinish_ms"))] +
                        [fmt(g.get(k)) for k in hwk] +
                        [" ".join("%s=%s" % (k, fmt(g[k])) for k in other if k in g and g[k] != 0) or "-",
                         fmt(hu), fmt(hd), fmt(dd), fmt(dn.rt.get(TPC_RESTYPE) / MB if TPC_RESTYPE in dn.rt else None),
                         fmt(dn.g("LIVE", "textures"))])
        o += [""] + table(hdr, rows) + [""]
        o.append("(GL columns are UP->DOWN deltas of the cumulative GL counters, `*max*` keys show the DOWN value; the module name is "
                 "not logged by perf-lab 0.3.0, shown only if a `module=` key exists; `unload` = a perf-lab UNLOAD block within the load window.)")
    # counters
    o.append("")
    o.append("- GETIMG (end of launch): other_tpc_retained=%s ctrl_tpc_retained=%s; BUILD requeue_tpc_retained=%s; **run B gate (other_tpc_retained == 0): %s**" % (
        fmt(a["other_ret"]), fmt(a["ctrl_ret"]), fmt(a["requeue_ret"]), a["gate"]))
    o.append("- RT: TPC (restype 3007) resident at last block %s MB (at first block %s MB)" % (
        fmt(a["tpc_mb"]), fmt(x.tpc_resident_mb("first"))))
    unl = [b for b in x.blocks if b.ev == "UNLOAD" and "KT" in b.kv]
    if unl:
        o.append("- KT per UNLOAD (cumulative still_in_demand/uaf_write): " + ", ".join(
            "#%d %s/%s" % (b.n, fmt(b.g("KT", "still_in_demand")), fmt(b.g("KT", "uaf_write"))) for b in unl) +
                 "; end of launch %s/%s. Note: since robustness 1.1.0, uaf_write counts writes into the KT dummy = orphan released, not a real UAF." % (
                     fmt(a["kt_sid"]), fmt(a["kt_uaf"])))
    else:
        o.append("- KT: no UNLOAD blocks with KT data; end of launch still_in_demand=%s uaf_write=%s (since robustness 1.1.0 uaf_write = writes into the KT dummy, not a real UAF)" % (
            fmt(a["kt_sid"]), fmt(a["kt_uaf"])))
    o.append("- NET max_len=%s; VM exec_bad_id=%s instr_limit=%s; AB 0x74CFE0 actionbar_calls=%s, max per frame=%s" % (
        fmt(a["netmax"]), fmt(a["vm_bad"]), fmt(a["vm_lim"]), fmt(a["ab_calls"]), fmt(a["ab_max"])))
    fs = a["frame"]
    if fs and fs.get("n"):
        o.append("- frame-time histogram (ms buckets, cumulative): n=%d, p50 bucket %s ms, p95 bucket %s ms, >=32 ms: %d (%.2f%%), max %s ms; counts %s" % (
            fs["n"], fs["p50"], fs["p95"], fs["slow32"], fs["slow32pct"], fmt(fs["max_ms"]), " ".join(str(v) for v in fs["hist"])))
    else:
        o.append("- frame-time histogram: none")
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
    o.append("- save/load events: no explicit save/load text exists in any log; heuristic from load pairs: quicksave-like pairs (longest chain) %d, quickload-like %d" % (
        cl["quicksave_pairs"], cl["quickloads"]))
    o.append("- scenarios: " + "; ".join("**%s %s** (%s)" % (k, v[0], v[1]) for k, v in sc.items() if not k.startswith("_")))
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
                     sc["S1"][0], sc["S2"][0], sc["S3"][0], sc["SNDRI"][0]])
    return rows


SUMMARY_HDR = ["#", "start", "pid", "ver", "hwmip", "texmem", "loads", "first load ms", "hang", "other_tpc_ret", "ctrl_tpc_ret", "run B gate",
               "KT still_in_demand", "KT uaf_write", "TPC MB", "heap UP MB (1st)", "heap DOWN MB (1st)", "robust FIRED", "act IGN/PASS",
               "S1", "S2", "S3", "SNDRI"]


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
    return o


def render(game_dir, infos, offsets_used, L, A, orphans, snap_orph, headless, arms, cfg_text):
    o = ["# perf-lab run analysis", "", "Game dir: `%s`" % game_dir, ""]
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
    o.append("## Summary (all launches)")
    o.append("")
    if L:
        o += table(SUMMARY_HDR, summary_rows(L, A))
    else:
        o.append("(no launches)")
    o.append("")
    gates = [A[x.idx]["gate"] for x in L]
    real = [g for g in gates if g != "n/a"]
    if not real:
        o.append("**Overall run B gate (other_tpc_retained == 0 in every launch): N/A (no GETIMG data)**")
    elif all(g == "PASS" for g in real):
        o.append("**Overall run B gate (other_tpc_retained == 0 in every launch): PASS (%d launches)**" % len(real))
    else:
        bad = [str(x.idx) for x in L if A[x.idx]["gate"] == "FAIL"]
        o.append("**Overall run B gate (other_tpc_retained == 0 in every launch): FAIL (launch %s)**" % ", ".join(bad))
    hung = [str(x.idx) for x in L if A[x.idx]["hang"]]
    o.append("**Hangs (UP without DOWN): %s**" % (("launch " + ", ".join(hung)) if hung else "none"))
    o.append("")
    if arms:
        o += render_arms(arms, L, A)
    o.append("## Launch details")
    o.append("")
    for x in L:
        o += render_launch(x, A[x.idx])
        o.append("")
    return "\n".join(o) + "\n"


# ----------------------------------------------------------------------------------------------------------------
def run(game_dir, offsets, arms=None):
    infos = {n: ReadInfo(n, os.path.join(game_dir, n), offsets.get(n, 0)) for n in ALL_FILES}
    mlaunches = parse_metrics(infos["perflab_metrics.txt"])
    snaps = parse_snaps(infos["perflab_alloc_snaps.txt"])
    attaches, _ = parse_perflab_log(infos["perflab_log.txt"])
    text, headless = {}, {}
    for n in LOG_FILES:
        text[n], headless[n] = parse_textlog(infos[n])
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
    md = render(game_dir, infos, offsets, L, A, orphans, snap_orph, headless, arms, cfg)
    return md, L, A


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--game-dir", default=DEFAULT_GAME_DIR)
    ap.add_argument("--from-offsets", metavar="FILE")
    ap.add_argument("--out", metavar="OUT.md")
    ap.add_argument("--arms", choices=("texmem", "hwmip"))
    args = ap.parse_args(argv)
    offsets = dict(DEFAULT_OFFSETS)
    if args.from_offsets:
        offsets = load_offsets(args.from_offsets)   # an explicit file replaces the defaults; unlisted files start at 0
    md, _, _ = run(args.game_dir, offsets, args.arms)
    if args.out:
        with open(args.out, "w") as f:
            f.write(md)
    else:
        sys.stdout.write(md)
    return 0


if __name__ == "__main__":
    sys.exit(main())
