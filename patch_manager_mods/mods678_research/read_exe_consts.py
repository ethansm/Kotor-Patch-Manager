#!/usr/bin/env python3
"""Read float/double constants from swkotor2.exe by VA, using the real PE section table
(never flat VA-0x400000). Ghidra's `_DAT_xxx` in a float expression may be a float or a
double: print both and pick the one that makes sense.

Usage: read_exe_consts.py 0x987df0 0x9919f0 ...
Values decoded 2026-09-30 (Mod 8): 0x987df0 d=0.1, 0x987f08 d=0.5, 0x9919f0 f=30,
0x98bd6c f=20, 0x9951dc f=1.6, 0x9951e0 f=1.4, 0x98b5c0 f=1.5, 0x989f10 d=1.5 (eye height),
0x987538 d=0.01, 0x98d898 d=0.2, 0x98b968 d=3, 0x98c1d8 d=0.25, 0x986b38 = 0.
"""
import struct, sys

EXE = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe"
b = open(EXE, "rb").read()
pe = struct.unpack_from("<I", b, 0x3C)[0]
n = struct.unpack_from("<H", b, pe + 6)[0]
opt = struct.unpack_from("<H", b, pe + 20)[0]
base = struct.unpack_from("<I", b, pe + 24 + 28)[0]
secs = []
for i in range(n):
    o = pe + 24 + opt + i * 40
    vsz, va, rsz, raw = struct.unpack_from("<IIII", b, o + 8)
    secs.append((va, vsz, raw))


def rd(va, fmt):
    r = va - base
    for sva, vsz, raw in secs:
        if sva <= r < sva + vsz:
            return struct.unpack_from(fmt, b, raw + r - sva)[0]


for a in sys.argv[1:]:
    va = int(a, 16)
    print(hex(va), "float=%g" % rd(va, "<f"), "double=%g" % rd(va, "<d"), "int=%d" % rd(va, "<i"))
