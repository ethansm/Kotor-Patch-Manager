# Audit source: n_commm08

Audit image: .claude/research/texture_diversity/p1/audit/n_commm08.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commm08
- base: dark green robe (A1-D1 front and back, C2-D2 trousers), orange/grey striped sash B2-C2, brown stitched leather vest A3-B4, brown-olive C3-C4
- paint_zones: C1 back wrap (largest flat green panel), D2 trouser leg (flat green), A1-B1 left chest fold (green, avoid V-collar), A4 brown vest (stitched leather, trim lines only)
- avoid: B2-C2 orange sash, A2-B3 grey gauntlets, A2-A3 bracer strip, C3-D3 brown belt band and boot shaft, D4 rust pouch
- skin_arms: no
- fit_roles: scholar, merchant, farmer
- palette_clash: dark green, black and navy vanish; moss-olive and brown-olive blend into the robe; orange and rust trims clash with the orange sash
- confidence: 4

Shared layout (male)
## LAYOUT (male; ref_M.png tiles M01-M11 and the 5 canvases agree)
- Row 1: A1 tunic front + dark hand blob | B1 tunic V-neck | C1 front edge + shirt back | D1 shirt back/sleeve panel (flat)
- Row 2: A2 glove + bracer (avoid) | B2 waist band (belt, sash or ribbed band, varies) | C2 trouser leg top | D2 trouser leg top
- Row 3: A3 glove/bracer lower (avoid) | B3 waist band lower | C3 trouser leg (pouch at C3/D3 seam) | D3 trouser leg, side panel
- Row 4: A4 lower vest/tunic (flat, large) | B4 lower vest (flat, large) | C4 boot/shin (avoid) | D4 pouch and gear (avoid)
- Mostly black/empty cells: none. Black gaps only at the D4 corner and between the boots; never paint gaps.

## Looks that paint this atlas (recipe layers)
- P7-m08: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C4', 'D4']; decal cells=['A1'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C2', 'C3', 'D3', 'D4']; band cells=['A4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C2', 'C3', 'D3', 'D4']

## Audit stats (json)
{
 "atlas": "n_commm08",
 "size": [
  512,
  512
 ],
 "model": "n_commm",
 "looks": [
  "P7-m08"
 ],
 "content_fraction": 0.943,
 "painted_of_content": 0.336,
 "unpainted_nonskin_of_content": 0.59,
 "unpainted_skin_like_of_content": 0.075,
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
   "content": 0.93,
   "painted": 0.827,
   "skin_like": 0.432
  },
  "B1": {
   "content": 0.955,
   "painted": 0.778,
   "skin_like": 0.274
  },
  "C1": {
   "content": 0.935,
   "painted": 0.715,
   "skin_like": 0.021
  },
  "D1": {
   "content": 0.969,
   "painted": 0.437,
   "skin_like": 0.198
  },
  "A2": {
   "content": 0.998,
   "painted": 0.008,
   "skin_like": 0.305
  },
  "B2": {
   "content": 0.977,
   "painted": 0.01,
   "skin_like": 0.273
  },
  "C2": {
   "content": 0.917,
   "painted": 0.45,
   "skin_like": 0.05
  },
  "D2": {
   "content": 0.923,
   "painted": 0.347,
   "skin_like": 0.046
  },
  "A3": {
   "content": 0.938,
   "painted": 0.006,
   "skin_like": 0.265
  },
  "B3": {
   "content": 0.86,
   "painted": 0.013,
   "skin_like": 0.102
  },
  "C3": {
   "content": 0.932,
   "painted": 0.25,
   "skin_like": 0.034
  },
  "D3": {
   "content": 0.971,
   "painted": 0.229,
   "skin_like": 0.001
  },
  "A4": {
   "content": 0.986,
   "painted": 0.543,
   "skin_like": 0.003
  },
  "B4": {
   "content": 0.93,
   "painted": 0.747,
   "skin_like": 0.0
  },
  "C4": {
   "content": 0.932,
   "painted": 0.009,
   "skin_like": 0.0
  },
  "D4": {
   "content": 0.934,
   "painted": 0.0,
   "skin_like": 0.0
  }
 }
}
