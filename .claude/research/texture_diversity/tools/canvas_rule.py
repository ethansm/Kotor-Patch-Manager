#!/usr/bin/env python3
"""Deterministic zone->cell compatibility of every KEEP card vs the 35 canvases (cell-level estimate, confirm with a mask)."""
import re, json, collections, statistics
from pathlib import Path
R = Path(__file__).resolve().parent.parent
t = (R/'w4/canvas_summary.md').read_text()
cv = {}
for blk in re.split(r'\n(?=### )', t)[1:]:
    m = re.match(r'### (?:n_)?(comm[mf]\d+)', blk)
    if not m: continue
    name = 'n_' + m.group(1)
    def cells(key):
        mm = re.search(r'^- %s:\s*(.*)$' % key, blk, re.M); s = mm.group(1) if mm else ''
        out = set()
        for a, b in re.findall(r'\b([A-D][1-4])\s*[-\u2013]\s*([A-D][1-4])\b', s):
            for c in range(ord(a[0]), ord(b[0]) + 1):
                for r in range(min(int(a[1]), int(b[1])), max(int(a[1]), int(b[1])) + 1): out.add(f"{chr(c)}{r}")
        return out | set(re.findall(r'\b[A-D][1-4]\b', s))
    sk = re.search(r'^- skin_arms:\s*(\w+)', blk, re.M); cf = re.search(r'^- confidence:\s*(\d)', blk, re.M)
    cv[name] = {'paint': sorted(cells('paint_zones')), 'avoid': sorted(cells('avoid')), 'skin': sk.group(1) if sk else '?', 'conf': int(cf.group(1)) if cf else 0}
json.dump(cv, open(R/'w4/canvas_cells.json', 'w'), indent=1)
ZONE = {'chest_left': ['A1', 'B1'], 'chest_center': ['A1', 'B1'], 'upper_back': ['C1', 'D1', 'C2', 'D2'], 'shoulder_l': ['C1', 'D1'], 'shoulder_r': ['C1', 'D1'],
        'upper_arm': ['C3', 'D3'], 'forearm': ['C3', 'D3'], 'cuff': ['C3', 'D3'], 'collar': ['B1'], 'belt': ['B2', 'B3'], 'hip_pocket': ['A4', 'B4'], 'thigh': ['A4', 'B4', 'C4', 'D4'],
        'knee': ['A4', 'B4', 'C4', 'D4'], 'shin': ['C4', 'D4'], 'hem': ['A4', 'B4', 'C4', 'D4'], 'boot_top': ['B4', 'C4'], 'whole_garment': ['A1', 'B1', 'C1', 'D1', 'A4', 'B4']}
final = json.load(open(R/'catalog_final.json'))
for c in final:
    cells = set(x for z in c['zones'] for x in ZONE.get(z.lower().replace(' ', '_'), []))
    ok = []
    for n, v in cv.items():
        if c['type'] == 'TATTOO' and v['skin'] not in ('yes', 'partial'): continue
        if c['type'] == 'COLOR' or ((cells & set(v['paint'])) - set(v['avoid'])): ok.append(n)
    c['canvas_rule'] = ok; c['rule_cells'] = sorted(cells)
json.dump(final, open(R/'catalog_final.json', 'w'), indent=1)
k = [c for c in final if c['status'] in ('KEEP', 'KEEP-GENERIC')]
print(len(cv), 'canvases parsed; keep cards', len(k), 'median canvases/card', statistics.median(len(c['canvas_rule']) for c in k),
      'min', min(len(c['canvas_rule']) for c in k), 'zero:', [c['cid'] for c in k if not c['canvas_rule']])
