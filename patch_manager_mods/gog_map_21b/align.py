#!/usr/bin/env python3
"""Parse ExportRangeDisasm dumps and align Steam/GOG function pairs instruction by instruction.
Per pair: identical-instruction count, address-operand diffs (steam->gog value pairs, with insn offset), other diffs.
usage: align.py pairs.tsv steam_dump.txt gog_dump.txt [--json out.json] [--verbose key]"""
import re, sys, json, collections
INS = re.compile(r'^([0-9a-f]{8})  ((?:[0-9a-f]{2} )+) *(.*)$')
FLOW = re.compile(r'  ->([0-9a-f]{8})\(')

def parse(path):
    secs = collections.OrderedDict(); cur = None
    for line in open(path):
        line = line.rstrip('\n')
        if line.startswith('=== '): cur = line[4:]; secs[cur] = {'hdr': None, 'ins': [], 'raw': []}; continue
        if cur is None: continue
        if line.startswith('FUNC '): secs[cur]['hdr'] = line; continue
        m = INS.match(line)
        if m:
            addr = int(m.group(1), 16); b = bytes.fromhex(m.group(2).replace(' ', ''))
            rest = m.group(3); k = rest.find('  ->'); txt = (rest if k < 0 else rest[:k]).strip(); fl = FLOW.findall(rest[k:]) if k >= 0 else []
            secs[cur]['ins'].append((addr, b, txt, [int(x, 16) for x in fl]))
        else: secs[cur]['raw'].append(line)
    return secs

def fn_for(secs, addr):
    for k, v in secs.items():
        if k.startswith('FN ') and v['ins'] and v['ins'][0][0] == addr: return v
    return None

def align(sf, gf):
    """returns dict(n_s,n_g,ident,addr_pairs[(off,kind,s,g)],other[(off,s,g)],map{steam_addr:gog_addr}) """
    S, G = sf['ins'], gf['ins']
    res = {'n_s': len(S), 'n_g': len(G), 'ident': 0, 'addr_pairs': [], 'other': [], 'map': {}, 'len_s': sum(len(i[1]) for i in S), 'len_g': sum(len(i[1]) for i in G)}
    if len(S) != len(G): res['mismatch_count'] = True
    for i in range(min(len(S), len(G))):
        sa, sb, st, sfl = S[i]; ga, gb, gt, gfl = G[i]
        res['map'][sa] = ga
        if sb == gb:
            res['ident'] += 1
            if sb[0] == 0xE8 and len(sb) == 5 and sfl and gfl:     # identical rel32 CALL: same delta => the callee pair is (S target, G target)
                res['addr_pairs'].append((sa - S[0][0], 'relsame', sfl[0], gfl[0]))
            continue
        if len(sb) != len(gb) or sb[:1] != gb[:1] and st.split(' ')[0] != gt.split(' ')[0]:
            res['other'].append((sa - S[0][0], st, gt)); continue
        diff = [k for k in range(len(sb)) if sb[k] != gb[k]]
        # rel32 branch/call: opcode E8/E9/0F8x
        rel = (sb[0] in (0xE8, 0xE9) and len(sb) == 5) or (sb[0] == 0x0F and 0x80 <= sb[1] <= 0x8F and len(sb) == 6)
        if rel:
            if sfl and gfl: res['addr_pairs'].append((sa - S[0][0], 'rel', sfl[0], gfl[0]))
            else: res['other'].append((sa - S[0][0], st, gt))
            continue
        # abs imm32 / disp32: differing bytes must lie in one aligned-ish 4-byte window
        lo, hi = min(diff), max(diff)
        found = False
        for p in range(max(0, hi - 3), min(lo, len(sb) - 4) + 1):
            sv = int.from_bytes(sb[p:p+4], 'little'); gv = int.from_bytes(gb[p:p+4], 'little')
            rest_equal = sb[:p] == gb[:p] and sb[p+4:] == gb[p+4:]
            if rest_equal and 0x400000 <= sv < 0xE00000 and 0x400000 <= gv < 0xE00000:
                res['addr_pairs'].append((sa - S[0][0], 'abs', sv, gv)); found = True; break
        if not found: res['other'].append((sa - S[0][0], st, gt))
    return res

def load_pairs(p):
    out = []
    for l in open(p):
        if l.startswith('#') or not l.strip(): continue
        f = l.rstrip('\n').split('\t'); out.append((f[0], int(f[1], 16), int(f[2], 16)))
    return out

if __name__ == '__main__':
    pairs = []
    for p in sys.argv[1].split(','): pairs += load_pairs(p)
    ss = {}; gs = {}
    for p in sys.argv[2].split(','): ss.update(parse(p))
    for p in sys.argv[3].split(','): gs.update(parse(p))
    verbose = sys.argv[sys.argv.index('--verbose') + 1] if '--verbose' in sys.argv else None
    allres = {}
    for key, s, g in pairs:
        sf, gf = fn_for(ss, s), fn_for(gs, g)
        if not sf or not gf: print('%-18s %08x %08x  MISSING DUMP (%s,%s)' % (key, s, g, bool(sf), bool(gf))); continue
        r = align(sf, gf); allres[key] = r
        print('%-18s %08x -> %08x  insn %d/%d  bytes %d/%d  identical %d  addr-diff %d  other %d' % (key, s, g, r['n_s'], r['n_g'], r['len_s'], r['len_g'], r['ident'], len(r['addr_pairs']), len(r['other'])))
        if key == verbose:
            for a in r['addr_pairs']: print('   +%04x %s %08x -> %08x' % (a[0], a[1], a[2], a[3]))
            for o in r['other']: print('   OTHER +%04x  S: %s  | G: %s' % o)
    if '--json' in sys.argv:
        json.dump({k: {'addr_pairs': v['addr_pairs'], 'other': v['other'], 'ident': v['ident'], 'n_s': v['n_s'], 'n_g': v['n_g']} for k, v in allres.items()}, open(sys.argv[sys.argv.index('--json') + 1], 'w'), indent=1)
