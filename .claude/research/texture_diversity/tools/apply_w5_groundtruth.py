#!/usr/bin/env python3
"""Apply spawn-table ground truth (w5/atlas_users.md, faction_probe.tsv) to the wave-5 fit assignments.
Idempotent: rewrites each id's w5 atlases/cells/stage/caveat from GT. Run from texture_diversity dir after merge_w5.py and map_atlases.py."""
import csv, json
GT = {  # id: (atlases, cells, stage, caveat)
 'A1-03': ('n_tsfcmdr01', 'n_tsfcmdr01:D1', 'S1D', 'Grenn uses tsfcmdr01 (4 spawns); cuff cell unverified; tier C'),
 'A8-02': ('n_sithcomm01,n_sithcomm02,n_sithcomm03,n_sithcomf01,n_sithcomf02,n_sithcomf03', 'n_sithcomm0x:A2+D2;n_sithcomf0x:B2+C2', 'S1D', 'Sith Captain atlases (36 spawns); cream vanishes on grey; sash cells by H5/H6'),
 'A8-03': ('n_sithcomm01,n_sithcomm02,n_sithcomm03,n_sithcomf01,n_sithcomf02,n_sithcomf03', 'n_sithcomm0x:D2;n_sithcomf0x:D1+B2', 'S1D', 'Sith Captain atlases (36 spawns); avoid pouches and zip'),
 'A7-09': ('n_sithassn01', 'n_sithassn01:B2+D3', 'S1D', 'Ubese Bounty Hunter model (57 spawns); hard plate, charcoal needs light paint; conf 2'),
 'SITHJ1-3': ('n_sithcom01,n_sithcom03', 'n_sithcom0x:A1,B2,D3,A4 plates', 'S1H', 'Sith Commando body (182 spawns); sithcom02 already red; copper seams via S1D; avoid cowl C1-D2'),
 'A8-01': ('n_sithsoldier03', 'n_sithsoldier03:A1,B1,C1,D1', 'S1D', 'Sith Soldier/Elite Trooper body (66 spawns); commando atlas dropped; avoid D1 arch outline'),
 'A6-05': ('n_ondsoldm1,n_ond_off', 'n_ondsoldm1:A4,C4,D1;n_ond_off:C3,C4,D2,D3,A4', 'S1H', 'Vaklu spawns use Onderon soldier/officer models (14); needs its own appearance row or texture variant'),
 'REPUB-4': ('n_repsold01,n_repsold_f01', 'n_repsold01:D1,B2,D3', 'S1D', 'Republic Soldier body (31 spawns) is already red; add yellow accents only; cells unverified'),
 'STRAT1-5': ('n_repsold01,n_repoff01', 'n_repsold01:D1;n_repoff01:B1', 'S1H', 'Player atlases dropped (0 NPC spawns); jacket cells only'),
 'A12-07': ('n_czerkaoff01,n_tsfoff01,n_tsfoff_f01', 'n_czerkaoff01:D2;n_tsfoff01:C1;n_tsfoff_f01:D1', 'S1D', 'Player atlases dropped; collar not covered; silver weak on grey'),
 'SITHJ1-5': (None, None, None, 'Covenant has 0 spawns in registry; lilac skirt stand-in only; low value'),
}
FEW = 'few NPC users (jedicoun 4 spawns, jedimem/jedirobem 0): needs new spawns'
ROBE = ('SITHJ1-4', 'SITHJ1-6', 'SITHJ1-8', 'SYM1-1', 'SYM1-5')
cards = json.load(open('catalog_final.json'))
for c in cards:
    w = c.get('w5')
    if not w or c['status'] != 'KEEP-UNI':
        continue
    if c['id'] in GT:
        a, ce, st, cav = GT[c['id']]
        if a: w['atlases'], w['cells'], w['stage'] = a, ce, st
        w['caveat'] = cav; w['groundtruth'] = 'registry x appearance.2da'
    elif c['id'] in ROBE:
        w['caveat'] = (w['caveat'] + '; ' + FEW)[:200]; w['groundtruth'] = 'few users'
json.dump(cards, open('catalog_final.json', 'w'), indent=1, ensure_ascii=False)
with open('w5/fit_merged.tsv', 'w') as f:
    f.write('id\tstatus\tatlases\tcells\tstage\tscale\tblend\tscore\tcaveat\n')
    for c in cards:
        w = c.get('w5')
        if w: f.write('\t'.join([c['id'], c['status'], w['atlases'], w['cells'], w['stage'], w['scale'], w['blend'], w['score'], w['caveat']]) + '\n')
# rewrite CATALOG 12.2 table
names = {c['id']: c['name'] for c in cards}
keep = [c for c in cards if c['status'] == 'KEEP-UNI']
tbl = ['### 12.2 KEEP-UNI cards (30, targets corrected with spawn ground truth, see 12.4)', '', '| id | card | atlases | stage | score | caveat |', '|---|---|---|---|---|---|']
for c in sorted(keep, key=lambda c: -int(c['w5']['score'] or 0)):
    w = c['w5']; tbl.append(f"| {c['id']} | {names[c['id']][:44]} | {w['atlases'].replace(';', ', ')} | {w['stage']} | {w['score']} | {w['caveat']} |")
s = open('CATALOG.md').read()
i, j = s.index('### 12.2 KEEP-UNI'), s.index('Merged: A7-01')
s = s[:i] + '\n'.join(tbl) + '\n\n' + s[j:]
open('CATALOG.md', 'w').write(s)
print(len(keep))
