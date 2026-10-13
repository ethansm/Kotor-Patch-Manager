# Audit source: n_commm05

Audit image: .claude/research/texture_diversity/p1/audit/n_commm05.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commm05
- base: red robe front (A1-B2, heavy folds), red back panel (C1-D1), dark green-grey ribbed band (B2-B3), red trouser legs (C2-D3), beige-cream lower vest (A4-B4), ochre-olive cloth (C4)
- paint_zones: A4+B4 beige lower vest (flat, wrinkled; best with dark ink); C1+D1 red back panel (plain, mild folds); C2 red trouser leg (flat inner area); A1 lower-left red robe front (between hand blob and V)
- avoid: B2-B3 ribbed band; A2-A3 brown bracer and black glove/pouch; C3/D3 seam gold oval pouch; C4 ochre cloth; D4 cream shin shape and red pouch; D2-D3 ochre edge strip
- skin_arms: no
- fit_roles: scholar, merchant, entertainer
- palette_clash: crimson and dark red vanish on the robe;

Shared layout (male)
## LAYOUT (male; ref_M.png tiles M01-M11 and the 5 canvases agree)
- Row 1: A1 tunic front + dark hand blob | B1 tunic V-neck | C1 front edge + shirt back | D1 shirt back/sleeve panel (flat)
- Row 2: A2 glove + bracer (avoid) | B2 waist band (belt, sash or ribbed band, varies) | C2 trouser leg top | D2 trouser leg top
- Row 3: A3 glove/bracer lower (avoid) | B3 waist band lower | C3 trouser leg (pouch at C3/D3 seam) | D3 trouser leg, side panel
- Row 4: A4 lower vest/tunic (flat, large) | B4 lower vest (flat, large) | C4 boot/shin (avoid) | D4 pouch and gear (avoid)
- Mostly black/empty cells: none. Black gaps only at the D4 corner and between the boots; never paint gaps.

## Looks that paint this atlas (recipe layers)
- P3-m05: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C4', 'D4']; decal cells=['B4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C3', 'C4', 'D2', 'D3', 'D4']; band cells=['C1', 'D1'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C3', 'C4', 'D2', 'D3', 'D4']

## Audit stats (json)
{
 "atlas": "n_commm05",
 "size": [
  512,
  512
 ],
 "model": "n_commm",
 "looks": [
  "P3-m05"
 ],
 "content_fraction": 0.947,
 "painted_of_content": 0.34,
 "unpainted_nonskin_of_content": 0.597,
 "unpainted_skin_like_of_content": 0.063,
 "content_outside_every_island": 0.302,
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
   "content": 0.996,
   "painted": 0.837,
   "skin_like": 0.317
  },
  "B1": {
   "content": 0.998,
   "painted": 0.786,
   "skin_like": 0.155
  },
  "C1": {
   "content": 0.939,
   "painted": 0.715,
   "skin_like": 0.01
  },
  "D1": {
   "content": 0.972,
   "painted": 0.436,
   "skin_like": 0.235
  },
  "A2": {
   "content": 1.0,
   "painted": 0.008,
   "skin_like": 0.258
  },
  "B2": {
   "content": 0.915,
   "painted": 0.01,
   "skin_like": 0.112
  },
  "C2": {
   "content": 0.907,
   "painted": 0.441,
   "skin_like": 0.035
  },
  "D2": {
   "content": 0.93,
   "painted": 0.345,
   "skin_like": 0.148
  },
  "A3": {
   "content": 0.948,
   "painted": 0.007,
   "skin_like": 0.264
  },
  "B3": {
   "content": 0.83,
   "painted": 0.013,
   "skin_like": 0.02
  },
  "C3": {
   "content": 0.915,
   "painted": 0.228,
   "skin_like": 0.001
  },
  "D3": {
   "content": 0.972,
   "painted": 0.229,
   "skin_like": 0.152
  },
  "A4": {
   "content": 0.992,
   "painted": 0.545,
   "skin_like": 0.014
  },
  "B4": {
   "content": 0.944,
   "painted": 0.747,
   "skin_like": 0.039
  },
  "C4": {
   "content": 0.939,
   "painted": 0.008,
   "skin_like": 0.012
  },
  "D4": {
   "content": 0.949,
   "painted": 0.0,
   "skin_like": 0.012
  }
 }
}
