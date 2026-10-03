#!/usr/bin/env python3
"""R3 triage, step 2: function-relative check of every hook.
GOG hook address -> containing GOG function (inputs/gog_functions.tsv) -> Steam function
(gog_to_steam_fn_map.tsv = gog_cluster_delta_map.py output, columns: steam_addr gog_addr name kind retn)
-> expected Steam VA = steam_fn + (hook - gog_fn).  Then check original_bytes at the expected VA
(exact / masked, masks taken from byte_check.tsv 'mask_pattern') and within +-0x80 of it (drift).
Usage: fn_delta_check.py STEAM_EXE OUT_TSV
"""
import csv, bisect, struct, sys, re
EXE, OUT = sys.argv[1:3]
D = '/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/'
H = '/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/upstream_prs/gog_only_triage/'
data = open(EXE, 'rb').read()
pe = struct.unpack_from('<I', data, 0x3C)[0]; nsec = struct.unpack_from('<H', data, pe+6)[0]; optsz = struct.unpack_from('<H', data, pe+20)[0]
base = struct.unpack_from('<I', data, pe+24+28)[0]; secs = []
for i in range(nsec):
    o = pe+24+optsz+40*i; vsz, va, rsz, raw = struct.unpack_from('<IIII', data, o+8); secs.append((base+va, max(vsz, rsz), raw))
def rd(va, n):
    for sva, sz, raw in secs:
        if sva <= va < sva+sz: return data[raw+va-sva: raw+va-sva+n]
    return b''
g = sorted((int(l.split('\t')[0], 16), l.split('\t')[1], l.split('\t')[2]) for l in open(D+'inputs/gog_functions.tsv').read().splitlines()[1:])
ga = [x[0] for x in g]
g_all = {x[0]: (x[1], x[2]) for x in g}
m = {}
for l in open(H+'gog_to_steam_fn_map.tsv'):
    p = l.rstrip('\n').split('\t')
    if len(p) >= 4: m[int(p[1], 16)] = (int(p[0], 16), p[3])
def rx(orig, pat):
    if not pat: return re.escape(orig)
    b = bytes.fromhex(pat.replace('xx', '..')) if False else None
    out = b''; i = 0
    while i < len(pat):
        if pat[i:i+2] == 'xx': out += b'.'; i += 2
        else: out += re.escape(bytes([int(pat[i:i+2], 16)])); i += 2
    return out
# Manual resolutions for hooks whose containing function the delta mapper does not cover / that drifted.
# (steam_fn, note) -- expected VA is then re-derived from hook-relative position or an explicit VA.
MANUAL = {
 ('PartyRosterLimit32K2', 142): (0x661fe2, 'HandlePlayerToServerLevelUpMessage Steam 0x661c30 (name_map); asm 0x661fdf CMP [ebp-0x34],0xc'),
 ('Movie Patch', 0): (0x40d901, 'RunMovieFrameLoop = Steam Movie::PlayFrames 0x40d6f0 (name_map) + 0x211; IAT operand differs (masked)'),
}
# name_map.tsv (verified Steam names, confidence high/medium) overrides the mapper's votes; matched on the
# GOG 'Class::Function' name (NOT on 'GOG 0x..' text in the evidence column: that text often cites a neighbour).
gname2addr = {}
for ga_, gv in g_all.items():
    gname2addr.setdefault(gv[0] + '::' + gv[1], []).append(ga_)
for l in open(D+'inputs/name_map.tsv').read().splitlines()[1:]:
    p = l.split('\t')
    if len(p) > 4 and p[3] in ('high', 'medium'):
        nmn = re.sub(r'\(.*', '', p[1]).strip()
        if len(gname2addr.get(nmn, [])) == 1:
            m[gname2addr[nmn][0]] = (int(p[0], 16), 'name_map:' + p[3])
rows = list(csv.DictReader(open(H+'byte_check.tsv'), delimiter='\t'))
hdr = ['patch', 'idx', 'gog_addr', 'type', 'function', 'len', 'gog_fn', 'map_kind', 'steam_fn', 'expected_va', 'match_at_expected', 'drift_hits(+-0x80)', 'informative_bytes', 'verdict']
res = []
for r in rows:
    a = int(r['gog_addr'], 16); orig = bytes.fromhex(r['original_bytes'].replace(' ', ''))
    i = bisect.bisect_right(ga, a) - 1; f = g[i]; name = f'{f[1]}::{f[2]}'
    mp = m.get(f[0])
    mk = (r['patch'], int(r['idx']))
    if mk in MANUAL:
        va, note = MANUAL[mk]
        inf = len(orig) - r['mask_pattern'].count('xx')
        pat = rx(orig, r['mask_pattern']) if len(orig) >= 5 else re.escape(orig)
        ok = bool(re.fullmatch(b'(?s:' + pat + b')', rd(va, len(orig))))
        res.append([r['patch'], r['idx'], r['gog_addr'], r['type'], r['function'], len(orig), name, 'manual', '', f'{va:#x}', int(ok), '', inf, ('MANUAL-MATCH: ' if ok else 'MANUAL-NOMATCH: ') + note]); continue
    if not mp:
        res.append([r['patch'], r['idx'], r['gog_addr'], r['type'], r['function'], len(orig), name, '', '', '', '', '', '', 'unmapped']); continue
    exp = mp[0] + (a - f[0])
    pat = rx(orig, r['mask_pattern']) if len(orig) >= 5 else re.escape(orig)
    w = rd(exp-0x80, 0x80+len(orig)+0x80)
    hits = [exp-0x80+mm.start() for mm in re.finditer(b'(?=(' + b'(?s:' + pat + b')))', w)]
    at = rd(exp, len(orig))
    ok = bool(re.fullmatch(b'(?s:' + pat + b')', at)) if at else False
    if ok: v = 'MATCH-at-expected'
    elif len(hits) == 1: v = f'DRIFT {hits[0]-exp:+#x}'
    elif hits: v = f'ambiguous-drift({len(hits)})'
    else: v = 'no-match'
    res.append([r['patch'], r['idx'], r['gog_addr'], r['type'], r['function'], len(orig), name, mp[1], f'{mp[0]:#x}', f'{exp:#x}', int(ok), len(hits), len(orig) - (r['mask_pattern'].count('xx') * 1 if len(orig) >= 5 else 0) , v])
with open(OUT, 'w') as o:
    o.write('\t'.join(hdr)+'\n')
    for x in res: o.write('\t'.join(map(str, x))+'\n')
import collections
for p in dict.fromkeys(x[0] for x in res):
    print(p, collections.Counter(x[13].split(':')[0].split(' ')[0].split('(')[0] for x in res if x[0] == p))
