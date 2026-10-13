# Audit source: n_commf07

Audit image: .claude/research/texture_diversity/p1/audit/n_commf07.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commf07
- base: taupe-grey wrap top (A1-C1) and tunic panel (C2-D2); dark brown laced corset (B2-B3); dark leather gloves (A2-A3) and bracer; taupe trouser legs (A3-B4); dark brown boot (C3-C4); near-black belt pouch (C3-D4)
- paint_zones: A1 plain taupe chest left (view x10-80 y20-170); C1 upper-right back (view x300-500 y10-110); C2 tunic side (view x300-460 y140-230, seam at x355); A4 trouser front left (view x10-110 y380-500)
- avoid: corset B2-B3 (view x140-290 y185-340); gloves A2-A3; bracer edge A2; pouch C3 (view x335-440 y330-375); boots C3-C4; tear at A1 (view x90-115 y25-75); pouch D4; gap B3-C4
- skin_arms: no
- fit_roles: refugee, mechanic, thug
- palette_clash: near-black

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
- P2-f07: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; decal cells=['C2'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']; wear cells=['C1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']
- P5-f07: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; band cells=['A4', 'C2'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']; band cells=['C1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D4']

## Audit stats (json)
{
 "atlas": "n_commf07",
 "size": [
  512,
  512
 ],
 "model": "n_commf",
 "looks": [
  "P2-f07",
  "P5-f07"
 ],
 "content_fraction": 0.667,
 "painted_of_content": 0.554,
 "unpainted_nonskin_of_content": 0.444,
 "unpainted_skin_like_of_content": 0.002,
 "content_outside_every_island": 0.088,
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
   "content": 0.828,
   "painted": 0.979,
   "skin_like": 0.004
  },
  "B1": {
   "content": 0.656,
   "painted": 0.958,
   "skin_like": 0.0
  },
  "C1": {
   "content": 0.398,
   "painted": 0.923,
   "skin_like": 0.021
  },
  "D1": {
   "content": 0.461,
   "painted": 0.803,
   "skin_like": 0.008
  },
  "A2": {
   "content": 0.58,
   "painted": 0.013,
   "skin_like": 0.001
  },
  "B2": {
   "content": 0.474,
   "painted": 0.014,
   "skin_like": 0.005
  },
  "C2": {
   "content": 0.509,
   "painted": 0.279,
   "skin_like": 0.006
  },
  "D2": {
   "content": 0.447,
   "painted": 0.245,
   "skin_like": 0.001
  },
  "A3": {
   "content": 0.608,
   "painted": 0.012,
   "skin_like": 0.0
  },
  "B3": {
   "content": 0.823,
   "painted": 0.017,
   "skin_like": 0.007
  },
  "C3": {
   "content": 0.924,
   "painted": 0.268,
   "skin_like": 0.006
  },
  "D3": {
   "content": 0.772,
   "painted": 0.186,
   "skin_like": 0.002
  },
  "A4": {
   "content": 0.751,
   "painted": 0.912,
   "skin_like": 0.011
  },
  "B4": {
   "content": 0.863,
   "painted": 0.985,
   "skin_like": 0.189
  },
  "C4": {
   "content": 0.849,
   "painted": 0.966,
   "skin_like": 0.172
  },
  "D4": {
   "content": 0.731,
   "painted": 0.968,
   "skin_like": 0.15
  }
 }
}
