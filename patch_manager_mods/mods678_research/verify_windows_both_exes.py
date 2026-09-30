#!/usr/bin/env python3
"""Compare every window's original_bytes (from check_windows_output_*.txt) against BOTH game executables:
swkotor2.exe (LAA build, sha 4AB72FC1...) and swkotor2.exe.unpatched_vanilla (base build, sha 6A522E71...).
2026-09-30: all 11 windows identical in both. Re-run after any game/exe change."""
import struct, re, sys, os
L = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
def load(p):
    b = open(p, 'rb').read(); pe = struct.unpack_from("<I", b, 0x3C)[0]; n = struct.unpack_from("<H", b, pe+6)[0]
    opt = struct.unpack_from("<H", b, pe+20)[0]; base = struct.unpack_from("<I", b, pe+24+28)[0]; secs = []
    for i in range(n):
        o = pe+24+opt+i*40; vsz, va, rsz, raw = struct.unpack_from("<IIII", b, o+8); secs.append((va, vsz, raw))
    return b, base, secs
def rd(t, va, n):
    b, base, secs = t; r = va-base
    for sva, vsz, raw in secs:
        if sva <= r < sva+vsz: return b[raw+r-sva:raw+r-sva+n]
exes = {k: load(f"{L}/{k}") for k in ("swkotor2.exe", "swkotor2.exe.unpatched_vanilla")}
here = os.path.dirname(os.path.abspath(__file__))
import glob
src = sys.argv[1] if len(sys.argv) > 1 else max(glob.glob(os.path.join(here, "check_windows_output_*.txt")), key=os.path.getmtime)
print("windows from", os.path.basename(src))
txt = open(src).read()
ok = True
for m in re.finditer(r"\] (.+?): 0x([0-9a-f]+) len (\d+).*?bytes = \[(.*?)\]", txt):
    name, addr, ln, bs = m.groups(); want = bytes(int(x, 16) for x in bs.split(", "))
    res = {k: rd(t, int(addr, 16), int(ln)) == want for k, t in exes.items()}
    ok &= all(res.values())
    print(f"{name:32} {addr} {ln:>3}  LAA:{res['swkotor2.exe']}  base:{res['swkotor2.exe.unpatched_vanilla']}")
print("ALL MATCH" if ok else "MISMATCH")
