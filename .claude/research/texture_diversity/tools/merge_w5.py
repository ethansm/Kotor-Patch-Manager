#!/usr/bin/env python3
"""Merge wave-5 fit results (w5/out/fit_N.tsv) into catalog_final.json and write w5/fit_merged.tsv.

Orchestrator overrides fix faction mismatches the fit agents flagged (A8-02/03 Sith officer, A1-03/A1-04 Republic/TSF,
A3-02 Czerka). Run from the texture_diversity dir: python3 tools/merge_w5.py
"""
import csv, json

OVERRIDE = {
    # id: (atlases, cells, caveat) -- faction fix; cells from H4/H5 paint_zones
    'A8-02': ('n_sithcomf01,n_sithcomf02,n_sithcomf03,n_sithoff_low_m', 'n_sithcomf0x:B2+C2;n_sithoff_low_m:A2',
              'Sith officer: grey cloth; cream vanishes; sithoff_low conf 2'),
    'A8-03': ('n_sithcomf01,n_sithcomf02,n_sithcomf03,n_sithoff_low_m', 'n_sithcomf0x:D1+B2;n_sithoff_low_m:B2',
              'Sith officer tabs; sithcomf conf 4, sithoff_low conf 2'),
    'A1-04': ('n_tsfoff01,n_tsfoff_f01,n_repoff01', 'n_tsfoff_f01:C3;n_tsfoff01:C1;n_repoff01:B1',
              'Republic Navy: not Czerka; collar cells unverified'),
    'A1-03': ('n_tsfoff01,n_tsfoff_f01', 'cuff cell unverified', 'TSF Lieutenant: officer atlas, not commander; tier C'),
    'A3-02': ('n_czerkaoff01,n_czerkaoffc01', 'n_czerkaoffc01:B2;n_czerkaoff01:B2', 'Czerka only; upper-arm cells unmapped'),
}
rows = {}
for n in (1, 2, 3, 4):
    for r in csv.DictReader(open(f'w5/out/fit_{n}.tsv'), delimiter='\t'):
        if r['id'].startswith('##') or not r['verdict']:
            continue
        rows[r['id']] = r
cards = json.load(open('catalog_final.json'))
cnt = {}
for c in cards:
    r = rows.get(c['id'])
    if not r or c['status'] != 'DEFER':
        continue
    v = r['verdict']
    st = 'KEEP-UNI' if v == 'KEEP' else ('MERGE' if v.startswith('MERGE') else 'DROP')
    c['w5'] = {k: r[k] for k in ('verdict', 'atlases', 'cells', 'stage', 'scale', 'blend', 'score', 'caveat')}
    if c['id'] in OVERRIDE and st == 'KEEP-UNI':
        a, ce, cav = OVERRIDE[c['id']]
        c['w5'].update(atlases=a, cells=ce, caveat=cav, override='orchestrator faction fix')
    c['status'] = st
    c['status_note'] = 'w5 fit: ' + v
    cnt[st] = cnt.get(st, 0) + 1
json.dump(cards, open('catalog_final.json', 'w'), indent=1, ensure_ascii=False)
with open('w5/fit_merged.tsv', 'w') as f:
    f.write('id\tstatus\tatlases\tcells\tstage\tscale\tblend\tscore\tcaveat\n')
    for c in cards:
        w = c.get('w5')
        if w:
            f.write('\t'.join([c['id'], c['status'], w['atlases'], w['cells'], w['stage'], w['scale'], w['blend'], w['score'], w['caveat']]) + '\n')
tot = {}
for c in cards:
    tot[c['status']] = tot.get(c['status'], 0) + 1
print('merged', cnt, 'totals', tot)
