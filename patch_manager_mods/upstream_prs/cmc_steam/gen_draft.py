#!/usr/bin/env python3
"""Generate kotor2-steam-aspyr.hooks.toml.draft from the upstream GOG hooks.toml (read-only), changing only
[metadata].target_versions and each hook's address; then verify every other field is equal and original_bytes
equals the Steam exe at the new address. Needs K2_STEAM_EXE."""
import re, tomllib
from pathlib import Path
from hookcheck import steam_read
SRC = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-cmc/Patches/K2ClassMenuClothes/hooks.toml")
OUT = Path(__file__).with_name("kotor2-steam-aspyr.hooks.toml.draft")
GOG_SHA = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"
STEAM_SHA = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"
MAP = {0x005DE18A: 0x008F9DCA, 0x0048E574: 0x0085E4E4, 0x0048E691: 0x0085E601, 0x005DE199: 0x008F9DD9}
txt = SRC.read_text()
assert txt.count(GOG_SHA) == 1
txt = txt.replace(GOG_SHA, STEAM_SHA)
for g, s in MAP.items():
    pat = f"address = 0x{g:08X}"
    assert txt.count(pat) == 1, pat
    txt = txt.replace(pat, f"address = 0x{s:08X}")
OUT.write_text(txt)
a, b = tomllib.loads(SRC.read_text()), tomllib.loads(txt)
assert a["metadata"]["target_versions"] == [GOG_SHA] and b["metadata"]["target_versions"] == [STEAM_SHA]
assert len(a["hooks"]) == len(b["hooks"]) == 4
for ha, hb in zip(a["hooks"], b["hooks"]):
    assert MAP[ha["address"]] == hb["address"]
    assert {k: v for k, v in ha.items() if k != "address"} == {k: v for k, v in hb.items() if k != "address"}
    assert steam_read(hb["address"], len(hb["original_bytes"])) == bytes(hb["original_bytes"]), hex(hb["address"])
print("OK: 4 hooks; all non-address fields identical; original_bytes == Steam exe")
