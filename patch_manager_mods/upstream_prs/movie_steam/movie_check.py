from hookcheck import *
import csv
G0 = bytes.fromhex("50 8B 8D 20 FF FF FF 8B 51 48 83 C2 34 52 8B 85 20 FF FF FF 8B 48 4C 51 FF 15 BC 54 98 00".replace(" ",""))
S0 = G0[:-4] + bytes.fromhex("A0649800")
G1 = bytes.fromhex("558BEC81EC84000000")
check("blitMovieMitchellNetravaliKotor2 (GOG bytes at Steam: expected False)", 0x920AF1, 0x40D901, G0)
check("blitMovieMitchellNetravaliKotor2 (Steam bytes)", 0x920AF1, 0x40D901, S0)
check("releaseMovieFilterKotor2", 0x91FD10, 0x40CB20, G1)
def find(f, va):
    for r in csv.DictReader(open(f), delimiter="\t"):
        a=int(r["addr"],16); s=int(r["size"])
        if a<=va<a+s: return r["addr"],s,r["retn"],r["n_insn"],r["callconv"],r["name"],r["namespace"]
D="/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/"
for va_g, va_s in [(0x920AF1,0x40D901),(0x91FD10,0x40CB20)]:
    print(hex(va_g), find(D+"k2_win_gog_aspyr_swkotor2.exe/fp.tsv", va_g)); print(hex(va_s), find(D+"k2_win_steam_aspyr_swkotor2.exe/fp.tsv", va_s))
