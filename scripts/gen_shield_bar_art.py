#!/usr/bin/env python3
"""Generate the Shield Bar's seam-free bar textures from the vanilla vitality bar art.

uibit_bar_vp_p.tga (128x256 white+alpha) is a curved-left capsule: opaque x 57..98, outline
~5px, interior alpha ~87. Two variants let the shield bar and the health bar read as ONE bar:
  uibit_bar_vp_sq.tga  health bar with a squared, outline-free LEFT edge (used while a shield is up)
  uibit_bar_sh_p.tga   shield bar: curved outer-left, outline-free RIGHT edge (butts into the health bar)
Open edges are filled by cycling straight interior columns 76..89 (keeps top/bottom outline + hatch).
Usage: gen_shield_bar_art.py <src uibit_bar_vp_p.tga> <out dir>
"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image

src, out = Path(sys.argv[1]), Path(sys.argv[2])
a = np.array(Image.open(src))
LO, HI = 76, 90                                    # straight interior columns [76, 90)
def col(x0, x1):                                   # cycled source column for each target column
    return [LO + (x - x0) % (HI - LO) for x in range(x0, x1)]
sq = a.copy()
sq[:, 57:LO] = a[:, [LO + (x - 57) % (HI - LO) for x in range(57, LO)]]   # square, open left (x<57 stays transparent)
sh = a.copy()
sh[:, HI:128] = a[:, [LO + (x - HI) % (HI - LO) for x in range(HI, 128)]]  # open right (to the edge)
sh[:, 99:128] = 0                                  # keep the original transparent right margin
for name, arr in (("uibit_bar_vp_sq", sq), ("uibit_bar_sh_p", sh)):
    Image.fromarray(arr).save(out / f"{name}.tga")
    (out / f"{name}.txi").write_text("mipmap 0\n")
print("wrote", out)
