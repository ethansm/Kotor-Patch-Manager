#!/usr/bin/env python3
"""Rank atlases for the AI-overlay build: vanilla spawns x applicable add-on cards (deterministic). Writes plan/priority.tsv.
Run from texture_diversity dir: python3 tools/priority.py"""
import csv, json, re, collections
cards = json.load(open('catalog_final.json'))
use = collections.defaultdict(list)  # atlas -> [(id, score, stage)]
for c in cards:
    for key in ('w5', 'w6'):
        w = c.get(key)
        if w and c['status'] in ('KEEP-UNI', 'KEEP-SPEC') and w.get('atlases'):
            for a in re.split(r'[,;]', w['atlases']):
                a = a.strip()
                if a and a != '-':
                    use[a].append((c['id'], int(w['score'] or 0), w['stage']))
spawns = collections.Counter()
for r in csv.DictReader(open('w5/utc_variants.tsv'), delimiter='\t'):
    spawns[r['texture']] += int(r['spawns'])
# model-default atlases have no texa column, so the UTC lookup misses them; family-level counts from w5/atlas_users.md
spawns.update({'n_ondsoldm1': 19, 'n_ond_off': 11, 'n_sithassn01': 59})
# commoner pool: 81 KEEP cards apply to all 70 commoner atlases (the diversity patch assigns them at spawn time)
com = sum(n for k, n in spawns.items() if k.startswith(('n_commm', 'n_commf')))
rows = []
for a, lst in use.items():
    ids = {i for i, _, _ in lst}
    rows.append((a, spawns.get(a, 0), len(ids), sum(s for _, s, _ in lst) // max(1, len(lst)), sum(1 for i in ids if any(x[2] == 'S2A' for x in lst if x[0] == i))))
rows.sort(key=lambda r: -(r[1] * r[2]))
with open('plan/priority.tsv', 'w') as f:
    f.write('atlas\tvanilla_spawns\tcards\tavg_score\ts2a_cards\treach(spawns*cards)\n')
    f.write(f'COMMONER_POOL(70)\t{com}\t81\t3\t9\t{com*81}\n')
    for a, s, n, av, k in rows:
        f.write(f'{a}\t{s}\t{n}\t{av}\t{k}\t{s*n}\n')
print(open('plan/priority.tsv').read()[:3200])
