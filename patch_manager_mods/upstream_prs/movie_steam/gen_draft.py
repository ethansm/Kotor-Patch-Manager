#!/usr/bin/env python3
"""Generate kotor2-steam-aspyr.hooks.toml.draft from upstream kotor2_hooks.toml (read-only): change only target_versions,
the two addresses and the IAT operand in hook 0's original_bytes; verify all other fields equal and bytes == Steam exe. Needs K2_STEAM_EXE."""
import tomllib
from pathlib import Path
from hookcheck import steam_read
SRC = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-movie/Patches/Movie Patch/kotor2_hooks.toml")
OUT = Path(__file__).with_name("kotor2-steam-aspyr.hooks.toml.draft")
GOG_SHA = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"
STEAM_SHA = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"
MAP = {0x00920AF1: 0x0040D901, 0x0091FD10: 0x0040CB20}
txt = SRC.read_text()
assert txt.count(GOG_SHA) == 1
txt = txt.replace(GOG_SHA, STEAM_SHA)
for g, s in MAP.items():
    pat = f"address = 0x{g:08X}"; assert txt.count(pat) == 1
    txt = txt.replace(pat, f"address = 0x{s:08X}")
old_iat = "    0xFF, 0x15, 0xBC, 0x54, 0x98, 0x00\n"   # call [0x9854BC] BinkBufferBlit (GOG)
new_iat = "    0xFF, 0x15, 0xA0, 0x64, 0x98, 0x00\n"   # call [0x9864A0] BinkBufferBlit (Steam)
assert txt.count(old_iat) == 1
txt = txt.replace(old_iat, new_iat)
OUT.write_text(txt)
a, b = tomllib.loads(SRC.read_text()), tomllib.loads(txt)
for ha, hb in zip(a["hooks"], b["hooks"]):
    assert MAP[ha["address"]] == hb["address"]
    ka = {k: v for k, v in ha.items() if k not in ("address", "original_bytes")}
    kb = {k: v for k, v in hb.items() if k not in ("address", "original_bytes")}
    assert ka == kb
    diff = [i for i, (x, y) in enumerate(zip(ha["original_bytes"], hb["original_bytes"])) if x != y]
    print(hex(hb["address"]), "len", len(hb["original_bytes"]), "differing byte indexes vs GOG:", diff)
    assert steam_read(hb["address"], len(hb["original_bytes"])) == bytes(hb["original_bytes"])
print("OK: all other fields identical; original_bytes == Steam exe")
