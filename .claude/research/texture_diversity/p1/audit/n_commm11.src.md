# Audit source: n_commm11

Audit image: .claude/research/texture_diversity/p1/audit/n_commm11.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commm11
- base: worn beige wrap tunic (A1-D1, V-wrap centre fold), grey-tan trousers C2-D2 with frayed hems, tan leather corset B2-C2 with eyelets, plain grey-beige vest A3-B4, dark brown belt pouch C3 and boot C4
- paint_zones: D1 back band (flat beige, largest field), A4-B4 grey-beige vest (most uniform plain block, emblem zone), D2 trouser back (worn grey-tan), A1-B1 chest left of wrap fold
- avoid: V-wrap fold B1-C1, tan corset B2-C2 with eyelets, brown gloves A2-B3, bracer strip A2-A3, frayed hems C2-D2 bottom, C3 pouch, C4 boot, D4 small pouch
- skin_arms: no
- fit_roles: refugee, farmer, merchant
- palette_clash: beige, tan, khaki and cream vanish on the tunic and vest; grey pai

Shared layout (male)
## LAYOUT (male; ref_M.png tiles M01-M11 and the 5 canvases agree)
- Row 1: A1 tunic front + dark hand blob | B1 tunic V-neck | C1 front edge + shirt back | D1 shirt back/sleeve panel (flat)
- Row 2: A2 glove + bracer (avoid) | B2 waist band (belt, sash or ribbed band, varies) | C2 trouser leg top | D2 trouser leg top
- Row 3: A3 glove/bracer lower (avoid) | B3 waist band lower | C3 trouser leg (pouch at C3/D3 seam) | D3 trouser leg, side panel
- Row 4: A4 lower vest/tunic (flat, large) | B4 lower vest (flat, large) | C4 boot/shin (avoid) | D4 pouch and gear (avoid)
- Mostly black/empty cells: none. Black gaps only at the D4 corner and between the boots; never paint gaps.

## Looks that paint this atlas (recipe layers)
- P1-m11: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C4', 'D4']; decal cells=['A1'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B1', 'B2', 'B3', 'C1', 'C2', 'C3', 'C4', 'D2', 'D4']; wear cells=['A4', 'B4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B1', 'B2', 'B3', 'C1', 'C2', 'C3', 'C4', 'D2', 'D4']
- P3-m11: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C4', 'D4']; decal cells=['B4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B1', 'B2', 'B3', 'C1', 'C2', 'C3', 'C4', 'D2', 'D4']; band cells=['D1'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B1', 'B2', 'B3', 'C1', 'C2', 'C3', 'C4', 'D2', 'D4']

## Audit stats (json)
{
 "atlas": "n_commm11",
 "size": [
  512,
  512
 ],
 "model": "n_commm",
 "looks": [
  "P1-m11",
  "P3-m11"
 ],
 "content_fraction": 0.711,
 "painted_of_content": 0.419,
 "unpainted_nonskin_of_content": 0.498,
 "unpainted_skin_like_of_content": 0.083,
 "content_outside_every_island": 0.097,
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
   "content": 0.845,
   "painted": 0.981,
   "skin_like": 0.0
  },
  "B1": {
   "content": 0.695,
   "painted": 0.971,
   "skin_like": 0.0
  },
  "C1": {
   "content": 0.587,
   "painted": 0.943,
   "skin_like": 0.084
  },
  "D1": {
   "content": 0.453,
   "painted": 0.863,
   "skin_like": 0.148
  },
  "A2": {
   "content": 0.63,
   "painted": 0.012,
   "skin_like": 0.0
  },
  "B2": {
   "content": 0.616,
   "painted": 0.014,
   "skin_like": 0.024
  },
  "C2": {
   "content": 0.614,
   "painted": 0.499,
   "skin_like": 0.082
  },
  "D2": {
   "content": 0.571,
   "painted": 0.401,
   "skin_like": 0.061
  },
  "A3": {
   "content": 0.587,
   "painted": 0.012,
   "skin_like": 0.006
  },
  "B3": {
   "content": 0.891,
   "painted": 0.017,
   "skin_like": 0.08
  },
  "C3": {
   "content": 0.954,
   "painted": 0.278,
   "skin_like": 0.225
  },
  "D3": {
   "content": 0.827,
   "painted": 0.208,
   "skin_like": 0.148
  },
  "A4": {
   "content": 0.739,
   "painted": 0.738,
   "skin_like": 0.049
  },
  "B4": {
   "content": 0.769,
   "painted": 0.982,
   "skin_like": 0.249
  },
  "C4": {
   "content": 0.87,
   "painted": 0.011,
   "skin_like": 0.32
  },
  "D4": {
   "content": 0.725,
   "painted": 0.0,
   "skin_like": 0.267
  }
 }
}
