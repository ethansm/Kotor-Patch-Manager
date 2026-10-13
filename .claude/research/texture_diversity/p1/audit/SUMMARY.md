# P1 coverage audit (10-13): findings from 14 Haiku recon units, gated by Opus

Method: `k1_port_tex_audit.py` renders a 2x2 panel per atlas (grid, UV islands, painted union, gap map) plus stats; one Haiku recon unit per atlas read the image (findings in `<atlas>.findings.md`, confidence 1-5). Opus checked claims against the mask data before acting.

## Confirmed and fixed (recipes in `patch_manager_mods/k1_port/looks/`)
1. **Sleeve and back-leg cloth (C2-D3) sat in the `arms` island** (arms covers 24-65% of C2/D2/C3/D3 in n_commm and n_commf; computed from the masks, not only from the images). Base layers used torso+legs only, so those panels were never painted. Fix: include `arms` and `torso_arm`; the hand shapes in A2/A4 are excluded by rect, skin guard and `min_luma`. (14/14 audits.)
2. **Whole-cell avoids hid real cloth**: the trouser band under the corset (A3/B3 lower), the wrap hem (A2/B2 top), male C4/D4 (cloth, not boot or pouch). Fix: sub-cell `avoid_rects` replace avoid cells (new layer key, atlas fractions).
3. **Hardware that was being painted**: D1 top-right corner pouch (all commoner atlases), ochre pouch at C2/D2 top, right-edge strap D2-D3, A4 pouch, dark strap B1/C1. Now excluded by rect.
4. **Sith Commando**: only 20% painted because cells were limited to A1/B2/D3/A4/B4. Plates in B1, A2, A3, B3, C4, D1, D2 were missed. Now all plate islands (body, legs, head, torso_arm, arms) with `min_luma 0.3`, B3 emblem and C2 joint disc excluded by rect. 20% -> 77% of content.
5. **Mandalorian helmet/visor/arm plates** (user report, same cause): `head` and `arms` islands added, no avoid cells.
6. **n_commf01**: tan cloth is skin-toned, so the skin guard blocked the recolour (looks changed 9%); `keep_skin` off there. **P6-f09**: blue parent, strength 0.85.

## Annotation corrections (the eyeballed w2/w4/w5 notes are wrong here; recipes no longer rely on them)
- n_commm05: A1-B1 beige tunic, A4-B4 red cloth (annotation had them swapped); C4 red cloth, not ochre; dark glove is A2-B2, not A1; gold oval pouch at C2/D2 top, not C3/D3.
- n_commm06: tan vest is A1-B1 (not A4-B4); pink sash is B3, navy band B2; D1 corner grey box is hardware.
- n_commm07: A1 is flat grey-blue tunic cloth (not a hand blob); C3/D3 are cloth leg panels (not a shadow strip); D4 is a cloth hem (not pouch/gear).
- n_commm08: brown stitched vest is A1-B1; rust pouch is D1 corner (not D4); C3/D3 have no brown belt; A2/A4 hand shapes are real (keep excluded).
- n_commm11: only A2 is a glove, B2-B3 a tan corset, A3 a belt; D4 is a large cloth panel; skin-like flag fires on worn tan cloth.
- n_commf01: D3/D4 are tan cloth, not black gaps; B4-C4 is leg cloth. n_commf02: A4 dark brown tip is a pouch/boot; n_commf03: boot is at A4 (not B4-C4), leather reads as skin-like; n_commf05: pouch is in A4, D4 is plain cloth, A4-B4 rust; n_commf09: belt flap at C2/D2 (not C3), B4-C4 blue trouser cloth; n_commf11: D4 plain cloth, A4-B4 grey-blue.
- n_sithcom01: emblem is in B3 (not B2), joint disc in C2 (not C3), D1/D2 are light plates (not cowl/glove), C4 white greave, D4 dark leather with buckle.
- n_mandalorian01: brown belt strap runs along the BOTTOM edge of row 4 (not the top); helmet shell = `head` island at C3-D4 plus visor strip at A2-A3.
- General: the legend on the audit image is small; island labels were inferred from colour. Treat confidence 1-2 lines as leads, not facts. Yellow (skin-like) flags fire on tan leather and cloth.

## Not changed / open
- Brown belt strap on the Mandalorian bottom edge is still recoloured (low-saturation leather barely shifts; confidence 2).
- Cyan "outside every island" content (9-30% per atlas) is unused UV padding and is deliberately unpainted.
- `k1_port_tex_audit.py` needs a human look at `audit2/<atlas>.png` after any recipe change; Haiku units should not be the final judge.
