import sys, struct
sys.path.insert(0,"../cmc_steam")
from xref import GT, GBASE
from hookcheck import TEXT, TBASE
import csv
def refs(buf, base, addr):
    p = struct.pack("<I", addr); out=[]; i = buf.find(p)
    while i != -1: out.append(base+i); i = buf.find(p, i+1)
    return out
def fnof(f, va):
    for r in csv.DictReader(open(f), delimiter="\t"):
        a=int(r["addr"],16); s=int(r["size"])
        if a<=va<a+s: return r["addr"], r["name"], r["size"], r["retn"], r["n_insn"]
D="/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/"
for lab, buf, base, addrs, fp in [("GOG",GT,GBASE,(0x9F22A4,0x9F22A8),D+"k2_win_gog_aspyr_swkotor2.exe/fp.tsv"),("STEAM",TEXT,TBASE,(0x9F42A4,0x9F42A8),D+"k2_win_steam_aspyr_swkotor2.exe/fp.tsv")]:
    for a in addrs:
        r = refs(buf, base, a)
        print(lab, hex(a), len(r), [(hex(x), fnof(fp,x)) for x in r][:8])
