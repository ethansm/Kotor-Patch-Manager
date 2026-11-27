#!/usr/bin/env python3
"""Generate the Shield Bar's seam-free bar textures from the vanilla vitality bar art.

uibit_bar_vp_p.tga (128x256 white+alpha) is a curved-left capsule: opaque x 57..98, outline
~5px, interior alpha ~87. Two variants let the shield bar and the health bar read as ONE bar:
  uibit_bar_vp_sq.tga  health bar with a squared, outline-free LEFT edge (used while a shield is up)
  uibit_bar_sc_p.tga   corner filler on the health rect (shield edge continues to the health bar's curved corner)
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
# (the open-right body now runs on to the texture edge; the DLL clips the draw at the seam, so the visible right edge is pixel-exact)
# The shield bar is drawn at ShieldWidthFraction (0.55) of the health bar's width, which also squashed its left outline to 55% of the
# health bar's stroke. Re-sample the visible span 57..98 non-uniformly (same total width, so the drawing constants stay valid): the
# outline columns get 1/0.55 weight (they render at native thickness), the interior is compressed to make up the width.
S_FRAC, U0, U1, RAMP0, RAMP1 = 0.55, 57, 98, 66.0, 78.0
def weights(w_end):
    u = np.arange(U0, U1, 0.05) + 0.025
    w = np.where(u < RAMP0, 1 / S_FRAC, np.where(u < RAMP1, 1 / S_FRAC + (w_end - 1 / S_FRAC) * (u - RAMP0) / (RAMP1 - RAMP0), w_end))
    return u, w
lo, hi = 0.05, 1 / S_FRAC
for _ in range(60):                                # bisect w_end so the total weight == the span (41 columns)
    mid = (lo + hi) / 2
    u, w = weights(mid)
    if np.sum(w) * 0.05 > U1 - U0: hi = mid
    else: lo = mid
u, w = weights((lo + hi) / 2)
cum = np.cumsum(w) * 0.05                          # new-column position of every old position
src = sh.astype(np.float32)
dst = sh.copy()
for j in range(U1 - U0):
    uj = np.interp(j + 0.5, cum, u)                # old column that lands on new column j
    i0 = int(np.floor(uj - 0.5)); f = uj - 0.5 - i0
    i0 = max(i0, U0); i1 = min(i0 + 1, U1 - 1)
    dst[:, U0 + j] = np.round(src[:, i0] * (1 - f) + src[:, i1] * f).astype(np.uint8)
sh = dst
# uibit_bar_sc_p: corner filler drawn on the HEALTH rect. For every row it fills the strip between x=57 (the shield's right edge /
# the health bar's outer-left edge in the middle rows) and the first opaque pixel of the health art, with the straight-interior
# column of that row. Result: the shield's top/bottom edge continues right up to the health bar's curved corner (no pixel overlap).
sc = np.zeros_like(a)
for r in range(a.shape[0]):
    xs = np.nonzero(a[r, 57:, 3] > 8)[0]
    hx = 57 + int(xs[0]) if len(xs) else 57
    for x in range(57, hx):
        sc[r, x] = a[r, LO + (x - 57) % (HI - LO)]
for name, arr in (("uibit_bar_vp_sq", sq), ("uibit_bar_sh_p", sh), ("uibit_bar_sc_p", sc)):
    Image.fromarray(arr).save(out / f"{name}.tga")
    (out / f"{name}.txi").write_text("mipmap 0\n")
print("wrote", out)
