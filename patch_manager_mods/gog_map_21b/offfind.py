#!/usr/bin/env python3
"""offfind.py 'steam_dump,gog_dump' 'pairs.tsv,pairs2.tsv' off1 off2 ... : first uses of each displacement in aligned Steam/GOG function pairs."""
import sys, re, align
dumps = [tuple(x.split(':')) for x in sys.argv[1].split(',')]
ss = {}; gs = {}
for s, g in dumps:
    ss.update(align.parse(s)); gs.update(align.parse(g))
pairs = []
for p in sys.argv[2].split(','): pairs += align.load_pairs(p)
maxper = 2
for off in sys.argv[3:]:
    pat = re.compile(r'(?<![0-9a-f])0x%s\b' % off.lower().replace('0x', ''), re.I)
    n = 0
    print('## %s' % off)
    for k, s, g in pairs:
        if k == 'TextColorAlt': continue
        sf = align.fn_for(ss, s); gf = align.fn_for(gs, g)
        if not sf or not gf: continue
        S, G = sf['ins'], gf['ins']; c = 0
        for i, x in enumerate(S):
            if pat.search(x[2]) and i < len(G):
                print('  %-16s S %08x %-18s %-44s | G %08x %-18s same=%s' % (k, x[0], x[1].hex(' '), x[2][:44], G[i][0], G[i][1].hex(' '), x[1] == G[i][1]))
                c += 1; n += 1
                if c >= maxper: break
        if n >= 6: break
