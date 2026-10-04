import sys, struct
from hookcheck import *
def gog_text():
    p = GB/"blocks.tsv"
    for r in csv.DictReader(open(p), delimiter="\t"):
        if r["start"].lower().endswith("401000"):
            return open(GB/r["file"],"rb").read(), 0x401000
GT, GBASE = gog_text()
def callers(buf, base, target):
    out = []
    i = buf.find(b"\xe8")
    while i != -1:
        if i+5 <= len(buf):
            rel = struct.unpack_from("<i", buf, i+1)[0]
            if base+i+5+rel == target: out.append(base+i)
        i = buf.find(b"\xe8", i+1)
    return out
if __name__ == "__main__":
    which = sys.argv[1]; t = int(sys.argv[2],16)
    buf, base = (GT, GBASE) if which == "gog" else (TEXT, TBASE)
    print(" ".join(f"{a:X}" for a in callers(buf, base, t)))
