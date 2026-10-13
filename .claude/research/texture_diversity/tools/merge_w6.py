#!/usr/bin/env python3
"""Merge wave-6 species fits (w6/out/fit_N.tsv) into catalog_final.json (status KEEP-SPEC; others stay SPECIES-LOCKED with a w6 note).
Run from the texture_diversity dir: python3 tools/merge_w6.py"""
import csv, json
FIX = {'A11-08': 'Ubese body is n_sithassn01 (annotated, hard plate, conf 2, 59 spawns); tier C so left locked'}
rows = {}
for n in (1, 2, 3, 4):
    for r in csv.DictReader(open(f'w6/out/fit_{n}.tsv'), delimiter='\t'):
        if r['id'].startswith(('#', '-')) or not r.get('verdict'):
            continue
        rows[r['id']] = r
cards = json.load(open('catalog_final.json'))
k = d = 0
for c in cards:
    r = rows.get(c['id'])
    if not r or c['status'] != 'SPECIES-LOCKED':
        continue
    if r['verdict'] == 'KEEP':
        c['status'] = 'KEEP-SPEC'; c['status_note'] = 'w6 fit: species atlas found'
        c['w6'] = {x: r[x] for x in ('verdict', 'atlases', 'cells', 'stage', 'scale', 'blend', 'score', 'caveat')}; k += 1
    else:
        c['status_note'] = 'w6: ' + FIX.get(c['id'], r['verdict']); c['w6'] = {'verdict': r['verdict']}; d += 1
json.dump(cards, open('catalog_final.json', 'w'), indent=1, ensure_ascii=False)
with open('w6/fit_merged.tsv', 'w') as f:
    f.write('id\tstatus\tatlases\tcells\tstage\tscale\tblend\tscore\tcaveat\n')
    for c in cards:
        w = c.get('w6')
        if w and c['status'] == 'KEEP-SPEC':
            f.write('\t'.join([c['id'], c['status']] + [w[x] for x in ('atlases', 'cells', 'stage', 'scale', 'blend', 'score', 'caveat')]) + '\n')
tot = {}
for c in cards: tot[c['status']] = tot.get(c['status'], 0) + 1
print('KEEP-SPEC', k, 'still locked', d, tot)
