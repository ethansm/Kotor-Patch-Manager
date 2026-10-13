# Audit source: n_commf09

Audit image: .claude/research/texture_diversity/p1/audit/n_commf09.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commf09
- base: blue-violet tunic (A1-C1 with light piping; C2-D2 side); dark brown leather corset (B2-B3) and gloves (A2-A3); blue trouser legs (A4, D3); dark brown boots (B4-C4); dark brown belt flap (C3)
- paint_zones: A1 blue chest left (view x10-150 y60-150, below collar); C1 upper-right blue panel (view x310-500 y30-110); C2 blue tunic side (view x300-460 y140-230); A4 blue trouser front (view x10-100 y380-500, seam at x100)
- avoid: corset B2-B3; gloves A2-A3; boots C3-C4; belt flap C3 (view x330-440 y330-375); collar piping (view x150-230 y0-60) and hem band (view y150-190); leather strap D2 (view x470-512 y140-300); pouch D4; tear at A1 (view x80-110 y20-75)
- skin_arms: no
-

Shared layout (female)
## LAYOUT (female; shared by ref_F and all n_commf*)
- A1+B1+C1: tunic front wrap, V-neck at B1 (A1 top-left skin/strap sliver: avoid)
- C1+D1, C2+D2: tunic back and shoulder/back-body panels (widest plain cloth)
- A2+A3: dark leather gloves (centre) and ammo pouch strip (left edge): avoid
- B2+B3: corset/belt with lacing and studs, navy strap under it at B3: avoid
- A4+B4: trousers/skirt front (large plain field); C3+D3: sleeve/arm with belt pouch strip at y 330-375: avoid
- C4+D4: leg panels; boot across B4-C4 and pouch in D4 corner: avoid
- Unused UV: no cell is mostly black. D3/D4 hold the only real gaps (41-46% black, commf01 only); commf02/03 are fully padded (0% black)

## Looks that paint this atlas (recipe layers)
- P6-f09: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; band cells=['C1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'C3', 'C4', 'D2', 'D4']; band cells=['A4'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'C3', 'C4', 'D2', 'D4']

## Audit stats (json)
{
 "atlas": "n_commf09",
 "size": [
  512,
  512
 ],
 "model": "n_commf",
 "looks": [
  "P6-f09"
 ],
 "content_fraction": 0.908,
 "painted_of_content": 0.447,
 "unpainted_nonskin_of_content": 0.55,
 "unpainted_skin_like_of_content": 0.003,
 "content_outside_every_island": 0.28,
 "islands": {
  "arms": {
   "texels_of_atlas": 0.147,
   "painted_by_looks": 0.0
  },
  "larm": {
   "texels_of_atlas": 0.147,
   "painted_by_looks": 0.0
  },
  "legs": {
   "texels_of_atlas": 0.224,
   "painted_by_looks": 0.711
  },
  "rarm": {
   "texels_of_atlas": 0.147,
   "painted_by_looks": 0.0
  },
  "torso": {
   "texels_of_atlas": 0.564,
   "painted_by_looks": 0.734
  },
  "torso_arm": {
   "texels_of_atlas": 0.006,
   "painted_by_looks": 1.0
  }
 },
 "cells": {
  "A1": {
   "content": 0.978,
   "painted": 0.821,
   "skin_like": 0.0
  },
  "B1": {
   "content": 0.898,
   "painted": 0.809,
   "skin_like": 0.0
  },
  "C1": {
   "content": 0.808,
   "painted": 0.769,
   "skin_like": 0.0
  },
  "D1": {
   "content": 0.839,
   "painted": 0.434,
   "skin_like": 0.005
  },
  "A2": {
   "content": 0.936,
   "painted": 0.008,
   "skin_like": 0.019
  },
  "B2": {
   "content": 0.621,
   "painted": 0.012,
   "skin_like": 0.014
  },
  "C2": {
   "content": 0.862,
   "painted": 0.269,
   "skin_like": 0.001
  },
  "D2": {
   "content": 0.978,
   "painted": 0.221,
   "skin_like": 0.002
  },
  "A3": {
   "content": 0.905,
   "painted": 0.008,
   "skin_like": 0.006
  },
  "B3": {
   "content": 0.812,
   "painted": 0.016,
   "skin_like": 0.014
  },
  "C3": {
   "content": 0.929,
   "painted": 0.225,
   "skin_like": 0.001
  },
  "D3": {
   "content": 0.994,
   "painted": 0.224,
   "skin_like": 0.0
  },
  "A4": {
   "content": 1.0,
   "painted": 0.684,
   "skin_like": 0.0
  },
  "B4": {
   "content": 0.981,
   "painted": 0.852,
   "skin_like": 0.0
  },
  "C4": {
   "content": 0.999,
   "painted": 0.832,
   "skin_like": 0.0
  },
  "D4": {
   "content": 0.995,
   "painted": 0.715,
   "skin_like": 0.0
  }
 }
}
