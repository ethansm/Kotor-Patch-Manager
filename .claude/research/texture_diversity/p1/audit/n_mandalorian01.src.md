# Audit source: n_mandalorian01

Audit image: .claude/research/texture_diversity/p1/audit/n_mandalorian01.png (Read it with the Read tool; it is an image)
Audit stats: appended at the end of this file

## Annotation (eyeballed earlier; may be WRONG, that is what you check)
### n_mandalorian01
- what: Mandalorian armour, blue colourway (navy cloth, steel-blue plate).
- layout: A1 shoulder plate; B1 helmet visor and dark hood top; B2 chest plate with centre bars; C1 dark hood/cape; C2 right chest plate; D2 side plates; A3-C3 navy trouser cloth; D3 buckled hip plate; A4 shin plate, B4 knee plate, C4 shin plate with silver clasp; D4 dark, low detail. Brown belt strap runs the top of row 4.
- base: navy cloth, steel-blue and pale grey-blue plate, dark brown belt.
- paint_zones: B2 chest centre (EMBLEM); C2 right chest plate (EMBLEM second spot); A1 shoulder plate edge (TRIM); A3 navy thigh cloth (WEAR or STAIN); D3 hip plate (HARDWARE).
- avoid: B1 visor and lens; row 4 belt and knee/shin plates; silver clasps at the bottom of B4-C4; D4 (unclear use); A2 grey ornament at left edge.
- skin: no
- surface: mixed (about half plate)
- fit_types: EMBLEM > TRIM > COLOR (plate only) > WEAR (cloth only) > HARDWARE
- palette_clash: navy or black paint vanishes on navy cloth; pale silver paint washes out on steel-blue plate.
- confidence: 3

(this atlas has its own layout, see its annotation)

## Looks that paint this atlas (recipe layers)
- MD-crimson: recolor_hsv cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'torso_arm', 'lshldrpad', 'rshldrpad', 'legs', 'head', 'arms'] avoid=[]; decal cells=['B2'] islands=['torso'] avoid=['B1', 'B4']; tint cells=['D3'] islands=['torso', 'legs'] avoid=['D4']
- MD-green: recolor_hsv cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'torso_arm', 'lshldrpad', 'rshldrpad', 'legs', 'head', 'arms'] avoid=[]; wear cells=['A3', 'B3'] islands=['legs'] avoid=['A4', 'B4', 'C4', 'D4']
- MD-amber: recolor_hsv cells=['A1', 'B1', 'C1', 'D1', 'A2', 'B2', 'C2', 'D2', 'A3', 'B3', 'C3', 'D3', 'A4', 'B4', 'C4', 'D4'] islands=['torso', 'torso_arm', 'lshldrpad', 'rshldrpad', 'legs', 'head', 'arms'] avoid=[]; decal cells=['C2'] islands=['torso'] avoid=['B1', 'C4']; wear cells=['B2', 'C2'] islands=['torso'] avoid=['B1']

## Audit stats (json)
{
 "atlas": "n_mandalorian01",
 "size": [
  512,
  512
 ],
 "model": "n_mandalorian",
 "looks": [
  "MD-crimson",
  "MD-green",
  "MD-amber"
 ],
 "content_fraction": 0.8,
 "painted_of_content": 0.83,
 "unpainted_nonskin_of_content": 0.168,
 "unpainted_skin_like_of_content": 0.001,
 "content_outside_every_island": 0.206,
 "islands": {
  "arms": {
   "texels_of_atlas": 0.098,
   "painted_by_looks": 1.0
  },
  "head": {
   "texels_of_atlas": 0.077,
   "painted_by_looks": 1.0
  },
  "larm": {
   "texels_of_atlas": 0.085,
   "painted_by_looks": 1.0
  },
  "legs": {
   "texels_of_atlas": 0.125,
   "painted_by_looks": 1.0
  },
  "lshldrpad": {
   "texels_of_atlas": 0.027,
   "painted_by_looks": 1.0
  },
  "rarm": {
   "texels_of_atlas": 0.08,
   "painted_by_looks": 1.0
  },
  "rshldrpad": {
   "texels_of_atlas": 0.02,
   "painted_by_looks": 1.0
  },
  "torso": {
   "texels_of_atlas": 0.526,
   "painted_by_looks": 1.0
  },
  "torso_arm": {
   "texels_of_atlas": 0.099,
   "painted_by_looks": 1.0
  }
 },
 "cells": {
  "A1": {
   "content": 0.609,
   "painted": 0.914,
   "skin_like": 0.009
  },
  "B1": {
   "content": 0.676,
   "painted": 0.908,
   "skin_like": 0.005
  },
  "C1": {
   "content": 0.665,
   "painted": 0.838,
   "skin_like": 0.009
  },
  "D1": {
   "content": 0.534,
   "painted": 0.833,
   "skin_like": 0.013
  },
  "A2": {
   "content": 0.705,
   "painted": 0.861,
   "skin_like": 0.002
  },
  "B2": {
   "content": 0.783,
   "painted": 0.818,
   "skin_like": 0.012
  },
  "C2": {
   "content": 0.842,
   "painted": 0.933,
   "skin_like": 0.003
  },
  "D2": {
   "content": 0.982,
   "painted": 0.752,
   "skin_like": 0.017
  },
  "A3": {
   "content": 0.909,
   "painted": 0.811,
   "skin_like": 0.001
  },
  "B3": {
   "content": 0.965,
   "painted": 0.952,
   "skin_like": 0.019
  },
  "C3": {
   "content": 0.841,
   "painted": 0.857,
   "skin_like": 0.004
  },
  "D3": {
   "content": 0.899,
   "painted": 0.781,
   "skin_like": 0.005
  },
  "A4": {
   "content": 0.952,
   "painted": 0.777,
   "skin_like": 0.015
  },
  "B4": {
   "content": 0.789,
   "painted": 0.764,
   "skin_like": 0.066
  },
  "C4": {
   "content": 0.963,
   "painted": 0.778,
   "skin_like": 0.005
  },
  "D4": {
   "content": 0.682,
   "painted": 0.75,
   "skin_like": 0.001
  }
 }
}
