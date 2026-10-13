# Audit source: n_commf03

Audit image: .claude/research/texture_diversity/p1/audit/n_commf03.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commf03
- base: mid-grey ribbed tunic (A1-D2, C3-D3), brown corset with brass studs (B2-B3), dark brown gloves (A2-A3), khaki-olive trousers (A4-B4), grey leg panel (C4-D4), brown boot with orange piping (B4-C4)
- paint_zones: A1 chest panel (grey ribbed, clean); D1 upper-right band (plain grey); C2-D2 back-body panel (grey, flat); D3 lower-right grey panel (x 384-512, y 256-384); A4 trouser front (plain khaki)
- avoid: B2-B3 studded corset, A2-A3 gloves and buckled pouch strip, B4-C4 boot and orange piping (x 230-260), C3 ochre pouch (x 335-440, y 330-375), D4 pouch corner, skin sliver at A1 top-left
- skin_arms: no (skin 0.052 is the A1 pink sliver only)
- fit_roles: mechanic, guard-

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
- P1-f03: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; decal cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']; wear cells=['A4', 'D3'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']
- P4-f03: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; decal cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']; band cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']
- P6-f03: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; band cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']
- P8-f03: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; wear cells=['A4', 'C2', 'D2'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']

## Audit stats (json)
{
 "atlas": "n_commf03",
 "size": [
  512,
  512
 ],
 "model": "n_commf",
 "looks": [
  "P1-f03",
  "P4-f03",
  "P6-f03",
  "P8-f03"
 ],
 "content_fraction": 0.968,
 "painted_of_content": 0.446,
 "unpainted_nonskin_of_content": 0.5,
 "unpainted_skin_like_of_content": 0.054,
 "content_outside_every_island": 0.285,
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
   "skin_like": 0.053
  },
  "B1": {
   "content": 1.0,
   "painted": 0.788,
   "skin_like": 0.099
  },
  "C1": {
   "content": 1.0,
   "painted": 0.724,
   "skin_like": 0.098
  },
  "D1": {
   "content": 0.948,
   "painted": 0.421,
   "skin_like": 0.013
  },
  "A2": {
   "content": 0.714,
   "painted": 0.011,
   "skin_like": 0.102
  },
  "B2": {
   "content": 0.982,
   "painted": 0.011,
   "skin_like": 0.273
  },
  "C2": {
   "content": 0.999,
   "painted": 0.328,
   "skin_like": 0.074
  },
  "D2": {
   "content": 0.998,
   "painted": 0.222,
   "skin_like": 0.067
  },
  "A3": {
   "content": 0.867,
   "painted": 0.008,
   "skin_like": 0.157
  },
  "B3": {
   "content": 0.995,
   "painted": 0.016,
   "skin_like": 0.139
  },
  "C3": {
   "content": 1.0,
   "painted": 0.273,
   "skin_like": 0.016
  },
  "D3": {
   "content": 0.993,
   "painted": 0.223,
   "skin_like": 0.014
  },
  "A4": {
   "content": 0.996,
   "painted": 0.687,
   "skin_like": 0.088
  },
  "B4": {
   "content": 1.0,
   "painted": 0.855,
   "skin_like": 0.0
  },
  "C4": {
   "content": 1.0,
   "painted": 0.832,
   "skin_like": 0.0
  },
  "D4": {
   "content": 1.0,
   "painted": 0.716,
   "skin_like": 0.001
  }
 }
}
