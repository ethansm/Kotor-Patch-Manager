#!/usr/bin/env python3
"""Read-only: for each hook (gog_va, steam_va, original_bytes) check Steam exe bytes, GOG bytes, .text uniqueness with 8 context bytes."""
import struct, sys, os, csv
from pathlib import Path
STEAM = Path(os.environ.get("K2_STEAM_EXE"))
GB = Path(os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/k2_win_gog_aspyr_swkotor2.exe/memblocks"))
data = STEAM.read_bytes()
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe+6)[0]; opt = struct.unpack_from("<H", data, pe+20)[0]
base = struct.unpack_from("<I", data, pe+24+28)[0]
secs = []
for i in range(nsec):
    o = pe+24+opt+40*i
    name = data[o:o+8].rstrip(b"\0").decode()
    vsz, rva, rsz, raw = struct.unpack_from("<IIII", data, o+8)
    secs.append((name, base+rva, vsz, rsz, raw))
def steam_read(va, n):
    for name, a, vsz, rsz, raw in secs:
        if a <= va < a+max(vsz, rsz):
            off = raw+va-a; return data[off:off+n]
    raise SystemExit(f"{va:X} not in Steam")
text = [s for s in secs if s[0] == ".text"][0]
TEXT = data[text[4]:text[4]+text[3]]; TBASE = text[1]
def gog_read(va, n):
    for r in csv.DictReader(open(GB/"blocks.tsv"), delimiter="\t"):
        s, sz = int(r["start"],16), int(r["size"])
        if s <= va < s+sz:
            with open(GB/r["file"],"rb") as f:
                f.seek(va-s); return f.read(n)
    raise SystemExit(f"{va:X} not in GOG")
def count(pat):
    c = 0; i = TEXT.find(pat)
    while i != -1:
        c += 1; i = TEXT.find(pat, i+1)
    return c
def check(name, gog, steam, orig):
    orig = bytes(orig)
    g = gog_read(gog, len(orig)); s = steam_read(steam, len(orig))
    ctx = steam_read(steam, len(orig)+8)
    print(f"{name}: GOG {gog:#x} bytes_match={g==orig}  Steam {steam:#x} bytes_match={s==orig} "
          f"bytes_only_hits={count(orig)} +8ctx_hits={count(ctx)} ctx={ctx[len(orig):].hex(' ')}")
if __name__ == "__main__":
    pass
