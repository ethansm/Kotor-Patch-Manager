# Audit source: n_commf02

Audit image: .claude/research/texture_diversity/p1/audit/n_commf02.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commf02
- base: cream tunic (A1-D1, dark diagonal seams at A1/B1), olive-khaki tan panel (C2-D2), khaki-olive trousers (A4-B4), brown gloves (A2-A3), rust-brown corset (B2-B3), brown boot (B3-C4)
- paint_zones: C1-D1 cream band (plain, above the seam near y 105); A1 lower-left between the seams; C2-D2 olive-tan panel (flat); A4 trouser front (plain khaki); D3 cream lower-right panel (x 430-500, y 330-450)
- avoid: dark diagonal seams A1-B1, B2-B3 corset and navy strap, A2-A3 gloves and pouch strip, C3 ochre pouch patch (x 335-440, y 330-375), D2 gold strip at right edge, boot B3-C4, D4 pouch
- skin_arms: no (cream reads skin-toned; inventory skin=0.177 is cloth; no bare skin island)
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
- P4-f02: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; decal cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'B4', 'C3', 'C4', 'D2', 'D4']; band cells=['C1', 'D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'B4', 'C3', 'C4', 'D2', 'D4']
- P5-f02: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; band cells=['A4', 'C2'] islands=['torso', 'legs'] avoid=['A1', 'B1', 'B2', 'B3', 'A2', 'A3', 'B4', 'C3', 'C4', 'D2', 'D4']; band cells=['C1', 'D1'] islands=['torso', 'legs'] avoid=['A1', 'B1', 'B2', 'B3', 'A2', 'A3', 'B4', 'C3', 'C4', 'D2', 'D4']
- P8-f02: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; wear cells=['A4', 'C2'] islands=['torso', 'legs'] avoid=['A1', 'B1', 'B2', 'B3', 'A2', 'A3', 'B4', 'C3', 'C4', 'D2', 'D4']

## Audit stats (json)
{
 "atlas": "n_commf02",
 "size": [
  512,
  512
 ],
 "model": "n_commf",
 "looks": [
  "P4-f02",
  "P5-f02",
  "P8-f02"
 ],
 "content_fraction": 0.963,
 "painted_of_content": 0.446,
 "unpainted_nonskin_of_content": 0.333,
 "unpainted_skin_like_of_content": 0.22,
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
   "content": 1.0,
   "painted": 0.819,
   "skin_like": 0.118
  },
  "B1": {
   "content": 0.979,
   "painted": 0.796,
   "skin_like": 0.096
  },
  "C1": {
   "content": 0.94,
   "painted": 0.739,
   "skin_like": 0.265
  },
  "D1": {
   "content": 0.946,
   "painted": 0.425,
   "skin_like": 0.114
  },
  "A2": {
   "content": 0.744,
   "painted": 0.01,
   "skin_like": 0.152
  },
  "B2": {
   "content": 0.922,
   "painted": 0.012,
   "skin_like": 0.306
  },
  "C2": {
   "content": 0.987,
   "painted": 0.328,
   "skin_like": 0.327
  },
  "D2": {
   "content": 0.998,
   "painted": 0.224,
   "skin_like": 0.312
  },
  "A3": {
   "content": 0.894,
   "painted": 0.008,
   "skin_like": 0.292
  },
  "B3": {
   "content": 0.997,
   "painted": 0.016,
   "skin_like": 0.402
  },
  "C3": {
   "content": 1.0,
   "painted": 0.272,
   "skin_like": 0.696
  },
  "D3": {
   "content": 0.999,
   "painted": 0.228,
   "skin_like": 0.514
  },
  "A4": {
   "content": 0.997,
   "painted": 0.687,
   "skin_like": 0.153
  },
  "B4": {
   "content": 1.0,
   "painted": 0.855,
   "skin_like": 0.143
  },
  "C4": {
   "content": 1.0,
   "painted": 0.832,
   "skin_like": 0.095
  },
  "D4": {
   "content": 1.0,
   "painted": 0.716,
   "skin_like": 0.14
  }
 }
}
