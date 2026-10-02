#!/usr/bin/env python3
"""Supplementary checks for the atlas windows (read-only; complements check_atlas_windows.py, which only scans the containing function):
  1. whole-dump inbound scan: any direct J*/CALL/JMP target in ANY function that lands strictly inside an AT window
  2. jump-table scan: every `JMP [reg*4+0xTABLE]` of the containing function, entries read from the vanilla exe (until an entry
     leaves the function), must not point strictly inside an AT window
  3. nearest live hook (snapshot TSV + live patch_config.toml) distance in bytes between window edges
Usage: check_atlas_supp.py   (prints one line per window with a problem, then a summary; exit 0 always)"""
import os, re, struct, glob, bisect, sys
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
import check_atlas_windows as ck
VAN = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe")
def load_pe(p):
    b = open(p, "rb").read(); pe = struct.unpack_from("<I", b, 0x3C)[0]; n = struct.unpack_from("<H", b, pe + 6)[0]
    opt = struct.unpack_from("<H", b, pe + 20)[0]; base = struct.unpack_from("<I", b, pe + 24 + 28)[0]; secs = []
    for i in range(n):
        o = pe + 24 + opt + i * 40; vsz, va, rsz, raw = struct.unpack_from("<IIII", b, o + 8); secs.append((va, vsz, raw))
    return b, base, secs
def rd(t, va, n):
    b, base, secs = t; r = va - base
    for sva, vsz, raw in secs:
        if sva <= r < sva + vsz: return b[raw + r - sva: raw + r - sva + n]
pe = load_pe(VAN)
starts = sorted(int(os.path.basename(p)[:8], 16) for p in glob.glob(ck.DUMP + "/*/*.asm"))
def fn_end(e):
    i = bisect.bisect_right(starts, e); return starts[i] if i < len(starts) else e + 0x10000
# 1. all direct branch targets in the dump: target -> [(from, text)]
tg = {}
for p in glob.glob(ck.DUMP + "/*/*.asm"):
    for l in open(p).read().splitlines():
        m = re.match(r"([0-9a-f]{8})\s+[0-9a-f]+\s+((?:J\w+|CALL|LOOP\w*)\s+0x([0-9a-f]+))$", l)
        if m: tg.setdefault(int(m.group(3), 16), []).append((int(m.group(1), 16), m.group(2)))
live = ck.live_windows() + ck.live_config_windows()[0]
nprob = 0; nwin = 0
for name, start, length, entry, skip, cexit, eret, shipped in ck.WINDOWS:
    if not name.startswith("AT "): continue
    nwin += 1
    ins = ck.load_fn(entry); end = start + sum(len(b) for a, b, t in ins if start <= a < start + length)
    probs = []
    for t in range(start + 1, end):
        for frm, txt in tg.get(t, []):
            if not (start <= frm < end): probs.append(f"inbound {frm:#x} {txt} lands inside")
    fe = fn_end(entry)
    for a, b, t in ins:
        m = re.match(r"JMP\s+dword ptr \[\w+\*0x4 \+ 0x([0-9a-f]+)\]$", t) or re.match(r"JMP\s+dword ptr \[\w+ \* 0x4 \+ 0x([0-9a-f]+)\]$", t) or re.match(r"JMP\s+dword ptr \[.*\*0x4 \+ 0x([0-9a-f]+)\]", t)
        if not m: continue
        tbl = int(m.group(1), 16); k = 0
        while k < 256:
            raw = rd(pe, tbl + 4 * k, 4)
            if not raw: break
            v = struct.unpack("<I", raw)[0]
            if not (entry <= v < fe): break
            if start < v < end: probs.append(f"jump table {tbl:#x}[{k}] (JMP at {a:#x}) -> {v:#x} lands inside")
            k += 1
    near = min(((max(la - end, start - (la + ln)), f"{lname}") for la, ln, pid, lname in live), key=lambda x: x[0])
    if probs: nprob += 1; print(f"[PROBLEM] {name} {start:#x}+{length}: " + "; ".join(probs))
    if near[0] < 12: print(f"[NEAR<12] {name} {start:#x}+{length}: nearest live hook {near[0]} B ({near[1]})")
print(f"supplementary: {nwin} AT windows, {nprob} with an inbound/jump-table problem")
