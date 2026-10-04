#!/usr/bin/env python3
"""Intersect candidate rects (x,y,w,h) with all live character_p.gui controls (S1_raw_character_p_controls.txt).
usage: check_rect.py X Y W H [X Y W H ...]"""
import re, sys
from pathlib import Path
SRC = Path(__file__).resolve().parents[2] / "phase4_evidence/S1_raw_character_p_controls.txt"
ctl = []
for l in SRC.read_text().splitlines():
    m = re.match(r"id=(\d+)\s+type=(\d+)\s+tag=(\S+)\s+ext=\((\d+), (\d+), (\d+), (\d+)\)", l)
    if m: ctl.append((m[3], int(m[2]), *map(int, m.groups()[3:7])))
a = list(map(int, sys.argv[1:]))
for i in range(0, len(a), 4):
    x, y, w, h = a[i:i+4]
    print(f"rect [{x},{y},{w},{h}]")
    for tag, t, cx, cy, cw, ch in ctl:
        if x < cx+cw and cx < x+w and y < cy+ch and cy < y+h:
            full = cx <= x and cy <= y and cx+cw >= x+w and cy+ch >= y+h
            print(f"  {'CONTAINS' if full else 'PARTIAL '} {tag} type{t} [{cx},{cy},{cw},{ch}]")
