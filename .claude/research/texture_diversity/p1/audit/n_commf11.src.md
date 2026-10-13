# Audit source: n_commf11

Audit image: .claude/research/texture_diversity/p1/audit/n_commf11.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commf11
- base: A1-D1 slate-grey wrap top (cell means 0x29-0x36, dark); B1 brown V-neck undershirt; A2-B3 dark brown leather gloves, bracer and corset; A4-B4 khaki-brown trouser; C2-D2 grey back vest; C3 grey-blue arm
- paint_zones: C2+D2 grey vest (largest plain panel); D1 top-back strip; A4+B4 khaki trouser leg; A1 upper-left wrap panel away from the tear (x85-115 y25-75)
- avoid: gloves and bracer A2-A3; corset lace B2-B3; belt buckle B3 (x225-250 y335-370); boot shaft B3-B4; black gaps C3-D4; tear in A1; pouch D4
- skin_arms: no (skin=0.0)
- fit_roles: mechanic, guard-civilian, refugee
- palette_clash: black, charcoal, navy and dark brown add-ons vanish on the dark base; olive or k

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
- P6-f11: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3']; band cells=['D1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D3', 'D4']; band cells=['D2'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'B4', 'C3', 'C4', 'D3', 'D4']

## Audit stats (json)
{
 "atlas": "n_commf11",
 "size": [
  512,
  512
 ],
 "model": "n_commf",
 "looks": [
  "P6-f11"
 ],
 "content_fraction": 0.677,
 "painted_of_content": 0.554,
 "unpainted_nonskin_of_content": 0.445,
 "unpainted_skin_like_of_content": 0.001,
 "content_outside_every_island": 0.087,
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
   "content": 0.829,
   "painted": 0.979,
   "skin_like": 0.0
  },
  "B1": {
   "content": 0.658,
   "painted": 0.958,
   "skin_like": 0.0
  },
  "C1": {
   "content": 0.432,
   "painted": 0.931,
   "skin_like": 0.0
  },
  "D1": {
   "content": 0.466,
   "painted": 0.804,
   "skin_like": 0.001
  },
  "A2": {
   "content": 0.64,
   "painted": 0.012,
   "skin_like": 0.0
  },
  "B2": {
   "content": 0.526,
   "painted": 0.013,
   "skin_like": 0.004
  },
  "C2": {
   "content": 0.52,
   "painted": 0.306,
   "skin_like": 0.0
  },
  "D2": {
   "content": 0.445,
   "painted": 0.254,
   "skin_like": 0.0
  },
  "A3": {
   "content": 0.583,
   "painted": 0.012,
   "skin_like": 0.0
  },
  "B3": {
   "content": 0.832,
   "painted": 0.017,
   "skin_like": 0.005
  },
  "C3": {
   "content": 0.926,
   "painted": 0.272,
   "skin_like": 0.002
  },
  "D3": {
   "content": 0.798,
   "painted": 0.213,
   "skin_like": 0.0
  },
  "A4": {
   "content": 0.748,
   "painted": 0.915,
   "skin_like": 0.0
  },
  "B4": {
   "content": 0.86,
   "painted": 0.985,
   "skin_like": 0.0
  },
  "C4": {
   "content": 0.846,
   "painted": 0.967,
   "skin_like": 0.0
  },
  "D4": {
   "content": 0.72,
   "painted": 0.972,
   "skin_like": 0.0
  }
 }
}
