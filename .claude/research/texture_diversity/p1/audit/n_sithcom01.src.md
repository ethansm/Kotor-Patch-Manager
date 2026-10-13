# Audit source: n_sithcom01

Audit image: .claude/research/texture_diversity/p1/audit/n_sithcom01.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_sithcom01
- what: Sith command plate armour (name-based), light grey and white plates with a black cowl.
- layout: A1 white shoulder plate; B1 white plate; C1-D1 black cowl (grey clasp at D1); A2 grey shoulder plate with black strap; B2 chest plate with emblem; C2 cowl edge and grey plate; D2 black glove or cowl; A3 grey hip plate; B3 belt clip and buckle; C3 grey round joint disc; D3 forearm plate with black tab; A4 white greave; B4-C4 dark grey boot; D4 white greave with black buckle. Empty cells: none.
- base: white and light grey plates; black cowl; dark grey boots.
- paint_zones: A1 shoulder plate; B2 chest plate below the emblem; D3 forearm plate; A4 greave.
- avoid: B2 emblem; B3 belt clip and buckle; C3 joint disc and hinge; D4 buckle; D1 clasp; black cowl C1-D2.
- skin: no
- surface: hard_plate
- fit_types: EMBLEM, TRIM, PATTERN, HARDWARE
- palette_clash: white, cream and pale grey vanish; black on the cowl vanishes; dark red and navy read.
- confidence: 3

(this atlas has its own layout, see its annotation)

## Looks that paint this atlas (recipe layers)
- SC-copper: colorize cells=['A1', 'B2', 'D3', 'A4', 'B4'] islands=['body', 'legs', 'torso_arm'] avoid=['B3', 'C3', 'D4', 'D1', 'C1', 'C2', 'D2']; wear cells=['A4', 'D3'] islands=['legs', 'torso_arm'] avoid=['D4']
- SC-crimson: colorize cells=['A1', 'B2', 'D3', 'A4', 'B4'] islands=['body', 'legs', 'torso_arm'] avoid=['B3', 'C3', 'D4', 'D1', 'C1', 'C2', 'D2']; wear cells=['A1', 'B2'] islands=['body'] avoid=['B3', 'C3']
- SC-navy: colorize cells=['A1', 'B2', 'D3', 'A4', 'B4'] islands=['body', 'legs', 'torso_arm'] avoid=['B3', 'C3', 'D4', 'D1', 'C1', 'C2', 'D2']

## Audit stats (json)
{
 "atlas": "n_sithcom01",
 "size": [
  256,
  256
 ],
 "model": "n_sithcommando",
 "looks": [
  "SC-copper",
  "SC-crimson",
  "SC-navy"
 ],
 "content_fraction": 0.819,
 "painted_of_content": 0.201,
 "unpainted_nonskin_of_content": 0.799,
 "unpainted_skin_like_of_content": 0.0,
 "content_outside_every_island": 0.263,
 "islands": {
  "arms": {
   "texels_of_atlas": 0.105,
   "painted_by_looks": 0.0
  },
  "body": {
   "texels_of_atlas": 0.433,
   "painted_by_looks": 0.362
  },
  "head": {
   "texels_of_atlas": 0.086,
   "painted_by_looks": 0.005
  },
  "larm": {
   "texels_of_atlas": 0.105,
   "painted_by_looks": 0.0
  },
  "legs": {
   "texels_of_atlas": 0.385,
   "painted_by_looks": 0.363
  },
  "rarm": {
   "texels_of_atlas": 0.105,
   "painted_by_looks": 0.0
  },
  "torso_arm": {
   "texels_of_atlas": 0.053,
   "painted_by_looks": 0.399
  }
 },
 "cells": {
  "A1": {
   "content": 0.995,
   "painted": 0.924,
   "skin_like": 0.0
  },
  "B1": {
   "content": 0.897,
   "painted": 0.017,
   "skin_like": 0.0
  },
  "C1": {
   "content": 0.839,
   "painted": 0.0,
   "skin_like": 0.0
  },
  "D1": {
   "content": 0.923,
   "painted": 0.0,
   "skin_like": 0.0
  },
  "A2": {
   "content": 0.802,
   "painted": 0.02,
   "skin_like": 0.0
  },
  "B2": {
   "content": 0.858,
   "painted": 0.64,
   "skin_like": 0.0
  },
  "C2": {
   "content": 0.829,
   "painted": 0.0,
   "skin_like": 0.0
  },
  "D2": {
   "content": 0.681,
   "painted": 0.009,
   "skin_like": 0.0
  },
  "A3": {
   "content": 0.889,
   "painted": 0.009,
   "skin_like": 0.0
  },
  "B3": {
   "content": 0.834,
   "painted": 0.013,
   "skin_like": 0.0
  },
  "C3": {
   "content": 0.64,
   "painted": 0.008,
   "skin_like": 0.0
  },
  "D3": {
   "content": 0.693,
   "painted": 0.938,
   "skin_like": 0.0
  },
  "A4": {
   "content": 0.796,
   "painted": 0.402,
   "skin_like": 0.0
  },
  "B4": {
   "content": 0.852,
   "painted": 0.144,
   "skin_like": 0.0
  },
  "C4": {
   "content": 0.758,
   "painted": 0.0,
   "skin_like": 0.0
  },
  "D4": {
   "content": 0.814,
   "painted": 0.016,
   "skin_like": 0.0
  }
 }
}
