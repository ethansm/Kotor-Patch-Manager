#!/usr/bin/env python3
"""autopair.py ADDR... : candidate GOG functions for Steam functions by (size, retn) + nearest known delta (from dumps/r1_align.json call pairs + pairs_r1.tsv)."""
import sys, json, bisect, glob
from seq import S, G
sd = {x[0]: x for x in S}
known = {}
import align
for f in glob.glob('dumps/*_align.json'):
    for k, r in json.load(open(f)).items():
        for off, kind, s, g in r['addr_pairs']:
            if s in sd and kind == 'rel': known[s] = g
for k, s, g in align.load_pairs('pairs_r1.tsv'): known[s] = g
ks = sorted(known)
bysz = {}
for g in G: bysz.setdefault((g[1], g[2]), []).append(g[0])
def cands(s, tol=0x1800):
    a = sd[s]; i = bisect.bisect_left(ks, s)
    near = [ks[j] for j in (i - 2, i - 1, i, i + 1) if 0 <= j < len(ks)]
    ds = set(known[n] - n for n in near)
    out = []
    for g in bysz.get((a[1], a[2]), []):
        if any(abs((g - s) - d) <= tol for d in ds): out.append(g)
    return out, sorted(ds)
if __name__ == '__main__':
    for t in sys.argv[1:]:
        s = int(t, 16)
        if s not in sd: print('%08x not a steam function start' % s); continue
        c, ds = cands(s)
        print('%08x size %d r%d -> %s   (deltas tried %s)' % (s, sd[s][1], sd[s][2], ['%08x %s' % (x, [g for g in G if g[0] == x][0][3][:36]) for x in c], ['%+x' % d for d in ds]))
