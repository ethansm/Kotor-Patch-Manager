#!/usr/bin/env python3
"""R3 triage, step 3: hard-coded C++ addresses -> GOG name -> Steam address -> branch-DB presence.
Usage: addr_map.py UPSTREAM_PATCHES_DIR BRANCH_STEAM_DB GOG_DB OUT_TSV"""
import re, sys, sqlite3, glob, os
P, SDB, GDB, OUT = sys.argv[1:5]
D = '/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/'
H = os.path.dirname(os.path.abspath(__file__)) + '/'
gog = {}
for l in open(D+'inputs/gog_functions.tsv').read().splitlines()[1:]:
    p = l.split('\t'); gog[int(p[0], 16)] = (p[1], p[2], p[4])
m = {}   # gog addr -> (steam addr, kind)
for l in open(H+'gog_to_steam_fn_map.tsv'):
    p = l.rstrip('\n').split('\t')
    if len(p) >= 4: m[int(p[1], 16)] = (int(p[0], 16), p[3])
nm = {}  # steam addr -> (name, confidence)
for l in open(D+'inputs/name_map.tsv').read().splitlines()[1:]:
    p = l.split('\t'); nm[int(p[0], 16)] = (p[1], p[3])
gname2addr = {}
for ga_, gv in gog.items(): gname2addr.setdefault(gv[0] + '::' + gv[1], []).append(ga_)
# name_map (verified, high/medium) overrides mapper votes; matched on GOG Class::Function name
for sa, (n_, c_) in nm.items():
    nmn = re.sub(r'\(.*', '', n_).strip()
    if c_ in ('high', 'medium') and len(gname2addr.get(nmn, [])) == 1: m[gname2addr[nmn][0]] = (sa, 'name_map:' + c_)
m[0x64c2f0] = (0x51c8d0, 'manual:body returns this->[+4]+0x1f0b4 (party table accessor); GOG label CTwoDimArrays::GetSwoopUpgradeRowProvider is a suspect label, name_map marks it wrong')
m[0x6207e0] = (0x71ad40, 'manual:retn20+callee CExoString default ctor 0x733540 (same delta as Destructor_2)')
m[0x921347] = (0x919723, 'manual:CONTRACT.md/alloc_sites.tsv operator new')
sf = {}
for l in open(D+'inputs/steam_functions.tsv').read().splitlines()[1:]:
    p = l.split('\t'); sf[int(p[0], 16)] = int(p[2])
def sdb():
    c = sqlite3.connect('file:'+SDB+'?mode=ro', uri=True); return {r[0]: (r[1], r[2]) for r in c.execute("select address,class_name,function_name from functions")}, {r[0] for r in c.execute("select address from global_pointers")}
steam_db, steam_glob = sdb()
gc = sqlite3.connect('file:'+GDB+'?mode=ro', uri=True)
gog_db = {r[0]: (r[1], r[2]) for r in gc.execute("select address,class_name,function_name from functions")}
rows = []
seen = {}
for patch, files in [('PartyRosterLimit32K2', ['PartyRosterLimit32K2/PartyRosterLimit32.cpp']), ('K2ClassMenuClothes', ['K2ClassMenuClothes/ClassMenuModels.cpp']),
                     ('Movie Patch', ['Movie Patch/better_movie_playback.h', 'Movie Patch/export.cpp', 'Movie Patch/better_movie_playback.cpp'])]:
    for f in files:
        for ln, line in enumerate(open(os.path.join(P, f), encoding='utf-8', errors='replace'), 1):
            if line.lstrip().startswith('//') and '0x' not in line: continue
            for h in re.findall(r'\b0x([0-9A-Fa-f]{6,8})[uU]?\b', line):
                v = int(h, 16)
                if 0x400000 <= v < 0xB00000 and (patch, v) not in seen:
                    seen[(patch, v)] = (f, ln, line.strip()[:90])
for (patch, v), (f, ln, txt) in sorted(seen.items()):
    g = gog.get(v)
    kind = 'func' if g else ('data/other' if v >= 0x960000 else 'code-no-name')
    gname = f'{g[0]}::{g[1]}' if g else (gog_db.get(v) and '::'.join(gog_db[v]) or '')
    s = m.get(v)
    saddr = f'{s[0]:#x}' if s else ''
    sname = nm.get(s[0], ('', ''))[0] if s else ''
    indb = ''
    if s: indb = 'Y:' + '::'.join(steam_db[s[0]]) if s[0] in steam_db else 'N'
    ing = 'Y' if v in gog_db else 'N'
    chk = ''
    if s:
        chk = ('align16' if s[0] % 16 == 0 else 'MISALIGNED') + (',retn=%d' % sf[s[0]] if s[0] in sf else ',retn=?') + (',gogparam=%s' % g[2] if g else '')
    rows.append([patch, f'{v:#010x}', kind, gname, saddr, s[1] if s else '', sname, 'gogDB=' + ing, 'steamDB=' + indb if s else '', chk, f'{os.path.basename(f)}:{ln}', txt])
with open(OUT, 'w') as o:
    o.write('\t'.join(['patch', 'gog_addr', 'kind', 'gog_name', 'steam_addr', 'map_kind', 'steam_name(name_map)', 'in_gog_db', 'in_branch_steam_db', 'sanity', 'src', 'line'])+'\n')
    for r in rows: o.write('\t'.join(r)+'\n')
print(len(rows))
