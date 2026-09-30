#!/usr/bin/env python3
"""Resolve PerfLab site addresses to Steam function names (steam_functions.tsv + name_map in the knowledge DB).

  resolve_sites.py report perflab_alloc.txt            # top-site tables with function names
  resolve_sites.py snaps  perflab_alloc_snaps.txt      # list snapshots
  resolve_sites.py diff   perflab_alloc_snaps.txt A B  # per-site live-byte delta between snapshots A and B (leaks)
  resolve_sites.py metrics perflab_metrics.txt         # 0.2.0: list metric snapshots + per-load (LOAD_UP -> LOAD_DOWN) summary
  resolve_sites.py metrics perflab_metrics.txt A B     # 0.2.0: counter deltas between snapshots A and B (by restype, waits, GL, frames, VQ)
  resolve_sites.py metrics-ab runA/perflab_metrics.txt runB/perflab_metrics.txt   # per-load averages + last snapshot, arm A vs arm B
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

# Standard Aurora restype ids (R1: the exe has no extension table in strings.tsv; ids verified for TGA/TPC/MDL/MDX/TXI/VIS/2DA/NCS/LYT/BWM via CResHelper vtables).
RT = {1: 'BMP', 3: 'TGA', 4: 'WAV', 6: 'PLT', 7: 'INI', 10: 'TXT', 2002: 'MDL', 2009: 'NSS', 2010: 'NCS', 2012: 'ARE', 2013: 'SET', 2014: 'IFO',
      2015: 'BIC', 2016: 'WOK', 2017: '2DA', 2018: 'TLK', 2022: 'TXI', 2023: 'GIT', 2024: 'BTI', 2025: 'UTI', 2026: 'BTC', 2027: 'UTC', 2029: 'DLG',
      2030: 'ITP', 2032: 'UTT', 2033: 'DDS', 2035: 'UTS', 2036: 'LTR', 2037: 'GFF', 2038: 'FAC', 2040: 'UTE', 2042: 'UTD', 2044: 'UTP', 2045: 'DFT',
      2046: 'GIC', 2047: 'GUI', 2051: 'UTM', 2052: 'DWK', 2053: 'PWK', 2056: 'JRL', 2058: 'UTW', 2059: '4PC', 2060: 'SSF', 3000: 'LYT', 3001: 'VIS',
      3002: 'RIM', 3003: 'PTH', 3004: 'LIP', 3007: 'TPC', 3008: 'MDX', -1: '?'}
RTF = ['allocN', 'allocB', 'evictN', 'evictB', 'freeN', 'freeB', 'relN', 'relZero', 'demN', 'redem60', 'resident']

def kv(line):
    out = {}
    for tok in line.split():
        if '=' in tok:
            k, v = tok.split('=', 1); v = v.split('(')[0].rstrip('MB').split('MB@')[0]
            try: out[k] = float(v)
            except ValueError: out[k] = v
    return out

def mblocks(path):
    out, cur = [], None
    for l in open(path, errors='replace'):
        l = l.rstrip('\n')
        if l.startswith('#METRICS '):
            p = l.split(); cur = {'n': int(p[1]), 'ev': p[2], 'hdr': kv(l), 'rt': {}, 'lines': {}, 'vq': [], 'raw': [l]}; out.append(cur)
        elif cur is None: continue
        elif l.startswith('#END'): cur = None
        else:
            cur['raw'].append(l)
            if l.startswith('RT '):
                v = [int(x) for x in l.split()[1:]]; cur['rt'][v[0]] = dict(zip(RTF, v[1:]))
            elif l.startswith('VQ') : cur['vq'].append(l)
            elif l[:1] != '#': cur['lines'].setdefault(l.split()[0], kv(l))
    return out

def rtname(t): return RT.get(t, str(t))
def d(B, A, sec, k): return B['lines'].get(sec, {}).get(k, 0) - A['lines'].get(sec, {}).get(k, 0)

def mdiff(A, B, show=True):
    r = {'ms': B['hdr'].get('tick', 0) - A['hdr'].get('tick', 0), 'live': B['hdr'].get('live_bytes', 0) - A['hdr'].get('live_bytes', 0),
         'wait_ms': d(B, A, 'WAIT', 'total_ms'), 'waits': d(B, A, 'WAIT', 'end'), 'sleeps': d(B, A, 'WAIT', 'sleeps'),
         'glfinish_ms': d(B, A, 'GL', 'glfinish_ms'), 'glfinish': d(B, A, 'GL', 'glfinish'), 'glimage': d(B, A, 'GL', 'glimage'),
         'mip_ms': d(B, A, 'GL', 'mip_ms'), 'mipmaps': d(B, A, 'GL', 'mipmaps'), 'frames': d(B, A, 'FRAME', 'frames'),
         'redem': d(B, A, 'RES', 'redemand_60s'), 'getimg_tpc_after_upload': d(B, A, 'GETIMG', 'tpc_after_upload'),
         'resident': {t: B['rt'][t]['resident'] - A['rt'].get(t, {}).get('resident', 0) for t in B['rt']}}
    if show:
        print(f"#{A['n']} {A['ev']} -> #{B['n']} {B['ev']}: {r['ms']/1000:.1f} s, live heap {r['live']/2**20:+.1f} MB, frames {r['frames']:.0f}")
        print(f"  C2 wait {r['wait_ms']:.0f} ms in {r['waits']:.0f} waits ({r['sleeps']:.0f} Sleep(1)); G1 glFinish {r['glfinish_ms']:.0f} ms / {r['glfinish']:.0f} of {r['glimage']:.0f} uploads;"
              f" M1 mipmaps {r['mip_ms']:.0f} ms / {r['mipmaps']:.0f}; J1 re-demands {r['redem']:.0f}; M3 TPC GetImage after upload {r['getimg_tpc_after_upload']:.0f}")
        if r['ms'] > 0:
            print(f"  share of elapsed: C2 {100*r['wait_ms']/r['ms']:.1f}%  G1 {100*r['glfinish_ms']/r['ms']:.1f}%  M1 {100*r['mip_ms']/r['ms']:.1f}%")
        print('  restype      dAllocN      dAllocMB   dEvictN   dEvictMB  dRelZero  dDemand  dRedem   residentMB(B)  dResidentMB')
        rows = []
        for t, b in B['rt'].items():
            a = A['rt'].get(t, dict.fromkeys(RTF, 0)); dd = {k: b[k] - a[k] for k in RTF}
            if any(dd[k] for k in RTF) or b['resident']: rows.append((b['resident'], t, dd, b))
        for res, t, dd, b in sorted(rows, key=lambda x: -x[0])[:30]:
            print(f"  {rtname(t):>6} {t:>5} {dd['allocN']:>9} {dd['allocB']/2**20:>11.1f} {dd['evictN']:>9} {dd['evictB']/2**20:>9.1f} {dd['relZero']:>9} {dd['demN']:>8} {dd['redem60']:>6} {b['resident']/2**20:>14.1f} {dd['resident']/2**20:>12.1f}")
        for l in B['vq'][:5]: print('  ' + l)
    return r

def loads(bs):
    out, up = [], None
    for b in bs:
        if b['ev'] == 'LOAD_UP': up = b
        elif b['ev'] == 'LOAD_DOWN' and up: out.append((up, b)); up = None
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
elif mode == 'metrics':
    bs = mblocks(path)
    if len(sys.argv) > 4:
        A, B = [next(b for b in bs if b['n'] == int(x)) for x in sys.argv[3:5]]; mdiff(A, B)
    else:
        for b in bs: print(b['n'], b['ev'], ' '.join(f"{k}={v:g}" if isinstance(v, float) else f"{k}={v}" for k, v in b['hdr'].items()))
        for i, (u, dn) in enumerate(loads(bs)): print(f'\n== load {i + 1}'); mdiff(u, dn)
elif mode == 'metrics-ab':
    for tag, p in (('A', sys.argv[2]), ('B', sys.argv[3])):
        bs = mblocks(p); ls = [mdiff(u, dn, False) for u, dn in loads(bs)]
        print(f'== arm {tag}: {p}: {len(ls)} loads, experiments {bs[-1]["hdr"].get("tx")}/{bs[-1]["hdr"].get("g1skip")} (tx/g1skip: 1 on, 0 off)')
        for k in ('ms', 'wait_ms', 'glfinish_ms', 'mip_ms', 'redem', 'getimg_tpc_after_upload'):
            v = [x[k] for x in ls]; print(f'  {k:>24}: ' + (' '.join(f'{x:.0f}' for x in v)) + (f'  mean {sum(v)/len(v):.0f}' if v else ''))
        last = bs[-1]; tpc = last['rt'].get(3007, {}).get('resident', 0)
        print(f"  last snapshot #{last['n']} {last['ev']}: live heap {last['hdr'].get('live_bytes', 0)/2**20:.1f} MB, TPC resident {tpc/2**20:.1f} MB; " + ' | '.join(last['vq'][:1]))
