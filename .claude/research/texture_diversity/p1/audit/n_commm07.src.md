# Audit source: n_commm07

Audit image: .claude/research/texture_diversity/p1/audit/n_commm07.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commm07
- base: grey-brown wrap shirt (A1-C2 front, C1-D1 back panel), grey trousers (C2-D3), slate-grey lower vest (A4-B4), dark grey-brown belt (B2-B3), dark trouser shadows at the C3/D3 seam
- paint_zones: A4+B4 slate-grey lower vest (flat, smooth, largest clean area); D1 shirt back panel (flat grey-brown); B1 wrap-front V (TRIM along the V-neck edge); D2 grey trouser top right (STAIN or wear)
- avoid: A1 dark hand blob; A2-A3 glove and bracer; B2-B3 buckled corset belt; C4 boot; D4 pouch; C3/D3 dark shadow strip
- skin_arms: no (inventory skin=0.0)
- fit_roles: refugee, farmer, mechanic
- palette_clash: grey, silver and charcoal add-ons vanish on the grey-brown shirt and grey trouser

Shared layout (male)
## LAYOUT (male; ref_M.png tiles M01-M11 and the 5 canvases agree)
- Row 1: A1 tunic front + dark hand blob | B1 tunic V-neck | C1 front edge + shirt back | D1 shirt back/sleeve panel (flat)
- Row 2: A2 glove + bracer (avoid) | B2 waist band (belt, sash or ribbed band, varies) | C2 trouser leg top | D2 trouser leg top
- Row 3: A3 glove/bracer lower (avoid) | B3 waist band lower | C3 trouser leg (pouch at C3/D3 seam) | D3 trouser leg, side panel
- Row 4: A4 lower vest/tunic (flat, large) | B4 lower vest (flat, large) | C4 boot/shin (avoid) | D4 pouch and gear (avoid)
- Mostly black/empty cells: none. Black gaps only at the D4 corner and between the boots; never paint gaps.

## Looks that paint this atlas (recipe layers)
- P5-m07: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C4', 'D4']; band cells=['A4'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'C3', 'C4', 'D3', 'D4']; band cells=['B1'] islands=['torso', 'legs'] avoid=['A1', 'A2', 'A3', 'B2', 'B3', 'C3', 'C4', 'D3', 'D4']

## Audit stats (json)
{
 "atlas": "n_commm07",
 "size": [
  512,
  512
 ],
 "model": "n_commm",
 "looks": [
  "P5-m07"
 ],
 "content_fraction": 0.638,
 "painted_of_content": 0.431,
 "unpainted_nonskin_of_content": 0.569,
 "unpainted_skin_like_of_content": 0.0,
 "content_outside_every_island": 0.081,
 "islands": {
  "arms": {
   "texels_of_atlas": 0.152,
   "painted_by_looks": 0.0
  },
  "larm": {
   "texels_of_atlas": 0.152,
   "painted_by_looks": 0.0
  },
  "legs": {
   "texels_of_atlas": 0.136,
   "painted_by_looks": 0.764
  },
  "rarm": {
   "texels_of_atlas": 0.152,
   "painted_by_looks": 0.0
  },
  "torso": {
   "texels_of_atlas": 0.545,
   "painted_by_looks": 0.579
  },
  "torso_arm": {
   "texels_of_atlas": 0.084,
   "painted_by_looks": 0.628
  }
 },
 "cells": {
  "A1": {
   "content": 0.798,
   "painted": 0.991,
   "skin_like": 0.0
  },
  "B1": {
   "content": 0.641,
   "painted": 0.979,
   "skin_like": 0.0
  },
  "C1": {
   "content": 0.455,
   "painted": 0.944,
   "skin_like": 0.0
  },
  "D1": {
   "content": 0.434,
   "painted": 0.881,
   "skin_like": 0.0
  },
  "A2": {
   "content": 0.467,
   "painted": 0.016,
   "skin_like": 0.001
  },
  "B2": {
   "content": 0.41,
   "painted": 0.013,
   "skin_like": 0.0
  },
  "C2": {
   "content": 0.532,
   "painted": 0.446,
   "skin_like": 0.0
  },
  "D2": {
   "content": 0.531,
   "painted": 0.377,
   "skin_like": 0.0
  },
  "A3": {
   "content": 0.5,
   "painted": 0.014,
   "skin_like": 0.0
  },
  "B3": {
   "content": 0.799,
   "painted": 0.018,
   "skin_like": 0.0
  },
  "C3": {
   "content": 0.928,
   "painted": 0.262,
   "skin_like": 0.0
  },
  "D3": {
   "content": 0.787,
   "painted": 0.179,
   "skin_like": 0.0
  },
  "A4": {
   "content": 0.599,
   "painted": 0.911,
   "skin_like": 0.0
  },
  "B4": {
   "content": 0.767,
   "painted": 0.983,
   "skin_like": 0.0
  },
  "C4": {
   "content": 0.854,
   "painted": 0.011,
   "skin_like": 0.0
  },
  "D4": {
   "content": 0.699,
   "painted": 0.0,
   "skin_like": 0.0
  }
 }
}
