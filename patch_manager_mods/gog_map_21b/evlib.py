#!/usr/bin/env python3
"""Evidence helpers: merged dumps r1..r5, aligned pairs, quote builders. Every quote is read from the dumps (nothing hand-typed)."""
import re, glob, collections, align, seq
from seq import S as SFN, G as GFN
SD = {x[0]: x for x in SFN}; GD = {x[0]: x for x in GFN}
ss = {}; gs = {}
for r in range(1, 6):
    for side, d in (('steam', ss), ('gog', gs)):
        try: d.update(align.parse('dumps/%s_r%d.txt' % (side, r)))
        except FileNotFoundError: pass
PAIRS = collections.OrderedDict()
for p in ('pairs_r1.tsv', 'pairs_r2.tsv', 'pairs_r3.tsv', 'pairs_r4.tsv', 'pairs_r5.tsv', 'extra_pairs.tsv'):
    try:
        for k, s, g in align.load_pairs(p):
            if k in ('TextColorAlt', 'AppRef2'): continue
            PAIRS[k] = (s, g)
    except FileNotFoundError: pass
AL = {}
for k, (s, g) in PAIRS.items():
    sf = align.fn_for(ss, s); gf = align.fn_for(gs, g)
    if sf and gf: AL[k] = (sf, gf, align.align(sf, gf))
BYSIZE = collections.defaultdict(list)
for g in GFN: BYSIZE[(g[1], g[2])].append(g[0])

def ins_at(key, saddr):
    sf, gf, r = AL[key]; S, G = sf['ins'], gf['ins']
    for i, x in enumerate(S):
        if x[0] == saddr: return S[i], G[i]
    raise KeyError('no steam insn %08x in %s' % (saddr, key))
def q(i):
    return '%08x [%s] %s' % (i[0], i[1].hex(' '), i[2])
def site(key, saddr):
    s, g = ins_at(key, saddr)
    same = s[1] == g[1]
    return 'S %s | G %s%s' % (q(s), ('%08x (bytes identical)' % g[0]) if same else q(g), '')
def site_same(key, saddr):
    s, g = ins_at(key, saddr); return s[1] == g[1]
def callsite(parent, target):
    sf, gf, r = AL[parent]; S, G = sf['ins'], gf['ins']
    for i, x in enumerate(S):
        if x[3] and x[3][0] == target and x[1][0] == 0xE8: return 'S %s | G %s' % (q(x), q(G[i])), G[i][3][0]
    raise KeyError('no call to %08x in %s' % (target, parent))
def absop(parent, sval):
    sf, gf, r = AL[parent]; S, G = sf['ins'], gf['ins']
    for off, kind, s, g in r['addr_pairs']:
        if s == sval and kind == 'abs':
            i = next(j for j, x in enumerate(S) if x[0] - S[0][0] == off)
            return 'S %s | G %s' % (q(S[i]), q(G[i])), g
    raise KeyError('no abs %08x in %s' % (sval, parent))
def fnline(key):
    sf, gf, r = AL[key]
    n_addr = sum(1 for a in r['addr_pairs'] if a[1] in ('abs', 'rel'))
    return '%s: %08x->%08x, %d/%d insn, %d byte-identical, %d differ only in address operands, %d other, size %d/%d B' % (
        key, PAIRS[key][0], PAIRS[key][1], r['n_s'], r['n_g'], r['ident'], len([1 for a in r['addr_pairs'] if a[1] in ('abs', 'rel')]), len(r['other']), r['len_s'], r['len_g'])
def decoys(key):
    s, g = PAIRS[key]; a = SD[s]
    return len(BYSIZE[(a[1], a[2])])

# REFS counts (r5 + r1/r4) : section "REFS <hex>" -> (n refs, n distinct fns)
def refs_count(side, addr):
    d = ss if side == 's' else gs
    k = 'REFS %x' % addr
    for kk, v in d.items():
        if kk == k or kk == k.upper() or kk.lower() == k.lower():
            return v
    return None
