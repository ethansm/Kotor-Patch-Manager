SUMMARY: n_commf02 painted 44.6% of content; the olive sleeve panel (C3-D3) and cream cloth under the corset (A3-B3 lower) are unpainted, while a D1 gadget and the B1-C1 strap get painted.
GAP | C3,D3 (C2,D2 lower) | olive-khaki textured sleeve/arm cloth; yellow gap is a skin-like false positive | unlabelled green top-right island (legend illegible; candidates arms/larm/rarm, all 0.0 painted) | action: add island (arm label) to colorize islands; keep ochre pouch C3 (y 330-375) excluded | confidence 2
GAP | A3,B3 (lower half, y ~330-384) | cream cloth band under the corset | torso (purple) | action: widen cells (drop A3,B3 from colorize avoid; pixel-mask glove, pouch, corset only) | confidence 3
MISID | A1,B1 | dark diagonal seams; annotation says avoid, image shows tunic shading | torso | action: fix annotation | confidence 2
MISID | A4 (x 85-115, y 425-490) | dark brown boot or pouch tip; annotation places boot at B3-C4 | torso/legs (unverified) | action: fix annotation (already unpainted) | confidence 2
OVERPAINT | D1 top-right corner (x 460-500, y 10-60) | dark grey object with metal trim, pouch or hardware | torso | action: keep excluded | confidence 3
OVERPAINT | B1,C1 top (x 215-300, y 0-60) | dark brown leather diagonal, strap or pouch | torso_arm (orange, 100% painted) | action: keep excluded; check torso_arm island | confidence 2
OVERPAINT | D2,D3 right edge (x 480-512, y 215-335) | ochre-gold strap or belt edge | pink right-edge strip (legend rarm; stats say rarm 0.0 painted, conflict) | action: keep excluded; verify island label | confidence 2
OVERPAINT | C2,D2 top (x 360-440, y 135-170) | ochre patch, possibly pouch | torso_arm (orange) | action: keep excluded if pouch | confidence 1
