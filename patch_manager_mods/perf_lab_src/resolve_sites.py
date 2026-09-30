#!/usr/bin/env python3
"""Resolve PerfLab site addresses to Steam function names (steam_functions.tsv + name_map in the knowledge DB).

  resolve_sites.py report perflab_alloc.txt            # top-site tables with function names
  resolve_sites.py snaps  perflab_alloc_snaps.txt      # list snapshots
  resolve_sites.py diff   perflab_alloc_snaps.txt A B  # per-site live-byte delta between snapshots A and B (leaks)
"""
import bisect, os, sqlite3, sys
HERE = os.path.dirname(os.path.abspath(__file__))
KN = os.path.join(HERE, '..', 'ghidra_knowledge')
starts, info = [], {}
for l in open(os.path.join(KN, 'steam_functions.tsv')).read().split('\n')[1:]:
    f = l.split('\t')
    if len(f) >= 4:
        a = int(f[0], 16); starts.append(a); info[a] = (int(f[1]), f[3])
starts.sort()
names = {}
try:
    c = sqlite3.connect(os.path.join(KN, 'kotor2_ghidra_knowledge.db'))
    for a, n in c.execute("select steam_addr,name from name_map where confidence in ('high','medium')"): names[int(a, 16)] = n
    for a, n in c.execute("select address,symbol from functions where symbol is not null"):
        names.setdefault(int(a, 16), n)
except Exception: pass

def fn(addr):
    if addr <= 1: return '-'
    i = bisect.bisect_right(starts, addr) - 1
    if i < 0: return '?'
    s = starts[i]; nm = names.get(s) or info[s][1]
    return f'{nm}+0x{addr - s:x}'

def snaps(path):
    out, cur = [], None
    for l in open(path):
        if l.startswith('#SNAP'): cur = {'hdr': l.strip(), 'sites': {}}; out.append(cur)
        elif l.startswith('S ') and cur:
            _, a1, a2, live, n = l.split(); cur['sites'][(int(a1, 16), int(a2, 16))] = (int(live), int(n))
    return out

mode, path = sys.argv[1], sys.argv[2]
if mode == 'report':
    for l in open(path):
        p = l.split()
        if len(p) > 3 and len(p[0]) == 8 and p[2] == '|':
            print(l.rstrip()[:70], '\n     ', fn(int(p[0], 16)), '<-', fn(int(p[1], 16)))
        else: print(l.rstrip())
elif mode == 'snaps':
    for i, s in enumerate(snaps(path)): print(i + 1, s['hdr'][:150])
elif mode == 'diff':
    ss = snaps(path); A, B = ss[int(sys.argv[3]) - 1], ss[int(sys.argv[4]) - 1]
    rows = []
    for k in set(A['sites']) | set(B['sites']):
        d = B['sites'].get(k, (0, 0))[0] - A['sites'].get(k, (0, 0))[0]
        if d: rows.append((d, k, B['sites'].get(k, (0, 0))[1] - A['sites'].get(k, (0, 0))[1]))
    rows.sort(reverse=True)
    print('A:', A['hdr'][:120]); print('B:', B['hdr'][:120]); print('net delta live bytes: %d' % sum(r[0] for r in rows))
    for d, k, dn in rows[:40]: print(f'{d:>12} B {dn:>8} blk  {k[0]:08x} {k[1]:08x}  {fn(k[0])} <- {fn(k[1])}')
