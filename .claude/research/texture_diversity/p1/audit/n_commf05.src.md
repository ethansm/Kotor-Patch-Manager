# Audit source: n_commf05

Audit image: .claude/research/texture_diversity/p1/audit/n_commf05.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commf05
- base: rust-brown wrap top (A1-D1, C2-D2 back), ochre-gold corset (B2-B3), dark brown gloves (A2-A3), grey-blue vest/trousers (A4-B4), rust-brown lower panels (C3-D3), brown pouch (D4 corner)
- paint_zones: A1 chest panel (rust wrap, plain left half); C1-D1 upper band (plain rust); D2 back panel (rust, flat); A4 lower-left grey panel (plain); D3 rust lower-right panel
- avoid: B2-B3 ochre corset and studs, A2-A3 gloves and pouch strip, B1 ochre V-neck lining, C3 ochre patch (x 335-440, y 330-375), D4 bag corner (x 445-512, y 440-500), A1 top-left skin/strap sliver
- skin_arms: no (skin 0.106 is rust cloth; no bare skin)
- fit_roles: farmer, refugee, merchant
- palette_clash: r

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
- P2-f05: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; decal cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'C3', 'D4']; wear cells=['C1', 'D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'C3', 'D4']
- P4-f05: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; decal cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'C3', 'D4']; band cells=['C1', 'D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'C3', 'D4']

## Audit stats (json)
{
 "atlas": "n_commf05",
 "size": [
  512,
  512
 ],
 "model": "n_commf",
 "looks": [
  "P2-f05",
  "P4-f05"
 ],
 "content_fraction": 0.94,
 "painted_of_content": 0.441,
 "unpainted_nonskin_of_content": 0.512,
 "unpainted_skin_like_of_content": 0.047,
 "content_outside_every_island": 0.282,
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
   "content": 0.837,
   "painted": 0.802,
   "skin_like": 0.0
  },
  "B1": {
   "content": 0.906,
   "painted": 0.772,
   "skin_like": 0.004
  },
  "C1": {
   "content": 0.953,
   "painted": 0.744,
   "skin_like": 0.016
  },
  "D1": {
   "content": 0.964,
   "painted": 0.428,
   "skin_like": 0.097
  },
  "A2": {
   "content": 0.719,
   "painted": 0.01,
   "skin_like": 0.034
  },
  "B2": {
   "content": 0.961,
   "painted": 0.011,
   "skin_like": 0.05
  },
  "C2": {
   "content": 0.97,
   "painted": 0.338,
   "skin_like": 0.05
  },
  "D2": {
   "content": 0.965,
   "painted": 0.232,
   "skin_like": 0.133
  },
  "A3": {
   "content": 0.847,
   "painted": 0.008,
   "skin_like": 0.112
  },
  "B3": {
   "content": 0.984,
   "painted": 0.014,
   "skin_like": 0.04
  },
  "C3": {
   "content": 0.973,
   "painted": 0.255,
   "skin_like": 0.009
  },
  "D3": {
   "content": 0.983,
   "painted": 0.232,
   "skin_like": 0.129
  },
  "A4": {
   "content": 0.993,
   "painted": 0.686,
   "skin_like": 0.15
  },
  "B4": {
   "content": 0.986,
   "painted": 0.853,
   "skin_like": 0.413
  },
  "C4": {
   "content": 0.995,
   "painted": 0.832,
   "skin_like": 0.342
  },
  "D4": {
   "content": 0.997,
   "painted": 0.716,
   "skin_like": 0.315
  }
 }
}
