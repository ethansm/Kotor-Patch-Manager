# Audit source: n_commm06

Audit image: .claude/research/texture_diversity/p1/audit/n_commm06.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_commm06
- base: charcoal jacket (A1-C2 front, C1-D1 back), charcoal trouser legs (C2-D3), pink-salmon ruched sash (B2), navy striped band (B3), tan-khaki vest (A4-B4), navy trouser legs (C3-C4)
- paint_zones: A4+B4 tan vest (flat, wrinkled; best with dark or navy ink); D1 charcoal back panel upper right (flat dark, light ink reads); D2 charcoal trouser panel (flat, light trim or patch); A1 charcoal front below the V trim (small)
- avoid: B2 pink ruched sash; B3 navy striped band; A2-A3 black pouch and bracer strap; V-neck trim (red-maroon and navy) at B1-C1; C3/D3 light-blue pouch; C3-D4 red-brown patch and red-brown pouch; right-edge blue-grey bracer strip
- skin_arms: no
- fit_roles: m

Shared layout (male)
## LAYOUT (male; ref_M.png tiles M01-M11 and the 5 canvases agree)
- Row 1: A1 tunic front + dark hand blob | B1 tunic V-neck | C1 front edge + shirt back | D1 shirt back/sleeve panel (flat)
- Row 2: A2 glove + bracer (avoid) | B2 waist band (belt, sash or ribbed band, varies) | C2 trouser leg top | D2 trouser leg top
- Row 3: A3 glove/bracer lower (avoid) | B3 waist band lower | C3 trouser leg (pouch at C3/D3 seam) | D3 trouser leg, side panel
- Row 4: A4 lower vest/tunic (flat, large) | B4 lower vest (flat, large) | C4 boot/shin (avoid) | D4 pouch and gear (avoid)
- Mostly black/empty cells: none. Black gaps only at the D4 corner and between the boots; never paint gaps.

## Looks that paint this atlas (recipe layers)
- P3-m06: colorize cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B2', 'B3', 'C4', 'D4']; decal cells=['B4'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B1', 'B2', 'B3', 'C1', 'C3', 'C4', 'D3', 'D4']; band cells=['D1', 'D2'] islands=['torso', 'legs'] avoid=['A2', 'A3', 'B1', 'B2', 'B3', 'C1', 'C3', 'C4', 'D3', 'D4']

## Audit stats (json)
{
 "atlas": "n_commm06",
 "size": [
  512,
  512
 ],
 "model": "n_commm",
 "looks": [
  "P3-m06"
 ],
 "content_fraction": 0.937,
 "painted_of_content": 0.341,
 "unpainted_nonskin_of_content": 0.596,
 "unpainted_skin_like_of_content": 0.063,
 "content_outside_every_island": 0.297,
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
   "content": 0.994,
   "painted": 0.837,
   "skin_like": 0.0
  },
  "B1": {
   "content": 0.993,
   "painted": 0.785,
   "skin_like": 0.0
  },
  "C1": {
   "content": 0.922,
   "painted": 0.711,
   "skin_like": 0.0
  },
  "D1": {
   "content": 0.948,
   "painted": 0.444,
   "skin_like": 0.036
  },
  "A2": {
   "content": 0.945,
   "painted": 0.008,
   "skin_like": 0.062
  },
  "B2": {
   "content": 0.919,
   "painted": 0.01,
   "skin_like": 0.056
  },
  "C2": {
   "content": 0.912,
   "painted": 0.436,
   "skin_like": 0.035
  },
  "D2": {
   "content": 0.925,
   "painted": 0.337,
   "skin_like": 0.046
  },
  "A3": {
   "content": 0.903,
   "painted": 0.007,
   "skin_like": 0.216
  },
  "B3": {
   "content": 0.932,
   "painted": 0.014,
   "skin_like": 0.513
  },
  "C3": {
   "content": 0.943,
   "painted": 0.259,
   "skin_like": 0.162
  },
  "D3": {
   "content": 0.955,
   "painted": 0.221,
   "skin_like": 0.033
  },
  "A4": {
   "content": 0.984,
   "painted": 0.546,
   "skin_like": 0.01
  },
  "B4": {
   "content": 0.868,
   "painted": 0.782,
   "skin_like": 0.043
  },
  "C4": {
   "content": 0.91,
   "painted": 0.01,
   "skin_like": 0.0
  },
  "D4": {
   "content": 0.929,
   "painted": 0.0,
   "skin_like": 0.008
  }
 }
}
