# Audit source: n_commf01

Audit image: .claude/research/texture_diversity/p1/audit/n_commf01.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commf01
- base: tan wrap tunic (A1-C1), tan back panels (C1-D2), tan trousers (A4-B4), tan leg panels (C3-D4), brown gloves (A2-A3), brown corset (B2-B3), brown boots (B4-C4)
- paint_zones: A1 left chest panel (plain tan wrap); C1-D1 upper back band (flat, widest); C2 back body panel; A4 trouser front (large plain tan); keep off D3-D4 gaps
- avoid: B2-B3 corset and buckles, A2-A3 gloves and pouch strip, B4-C4 boot, D4 pouch, B1 pink V-neck lining, black gaps at D3-D4 and A1 corners
- skin_arms: no (tan cloth is skin-toned; only a small pink sliver at B1 neckline)
- fit_roles: refugee, farmer, merchant
- palette_clash: tan, khaki, ochre and light brown vanish; pink or rose trim clashes

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
- P2-f01: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; decal cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'B4', 'C4', 'D3', 'D4']; wear cells=['A4'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'B4', 'C4', 'D3', 'D4']
- P8-f01: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; wear cells=['A4', 'C2'] islands=['torso', 'legs'] avoid=['A1', 'B1', 'B2', 'B3', 'A2', 'A3', 'B4', 'C4', 'D3', 'D4']

## Audit stats (json)
{
 "atlas": "n_commf01",
 "size": [
  512,
  512
 ],
 "model": "n_commf",
 "looks": [
  "P2-f01",
  "P8-f01"
 ],
 "content_fraction": 0.78,
 "painted_of_content": 0.545,
 "unpainted_nonskin_of_content": 0.181,
 "unpainted_skin_like_of_content": 0.274,
 "content_outside_every_island": 0.107,
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
   "content": 0.844,
   "painted": 0.971,
   "skin_like": 0.961
  },
  "B1": {
   "content": 0.817,
   "painted": 0.945,
   "skin_like": 0.775
  },
  "C1": {
   "content": 0.759,
   "painted": 0.919,
   "skin_like": 0.523
  },
  "D1": {
   "content": 0.479,
   "painted": 0.784,
   "skin_like": 0.803
  },
  "A2": {
   "content": 0.839,
   "painted": 0.009,
   "skin_like": 0.283
  },
  "B2": {
   "content": 0.705,
   "painted": 0.014,
   "skin_like": 0.21
  },
  "C2": {
   "content": 0.693,
   "painted": 0.437,
   "skin_like": 0.63
  },
  "D2": {
   "content": 0.575,
   "painted": 0.387,
   "skin_like": 0.723
  },
  "A3": {
   "content": 0.743,
   "painted": 0.01,
   "skin_like": 0.622
  },
  "B3": {
   "content": 0.926,
   "painted": 0.016,
   "skin_like": 0.488
  },
  "C3": {
   "content": 0.953,
   "painted": 0.283,
   "skin_like": 0.832
  },
  "D3": {
   "content": 0.86,
   "painted": 0.258,
   "skin_like": 0.704
  },
  "A4": {
   "content": 0.798,
   "painted": 0.858,
   "skin_like": 0.926
  },
  "B4": {
   "content": 0.87,
   "painted": 0.983,
   "skin_like": 0.989
  },
  "C4": {
   "content": 0.869,
   "painted": 0.958,
   "skin_like": 0.971
  },
  "D4": {
   "content": 0.753,
   "painted": 0.951,
   "skin_like": 0.954
  }
 }
}
