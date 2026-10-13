SUMMARY: n_commm08 painted 33.6% of content; P7-m08 paints torso and legs only, so the avoid cells hide robe cloth and arms/larm/rarm stay at 0%.
GAP | C2,C3,D2,D3 (BR red x800-1000 y690-900) | dark green trouser/robe cloth, flat, unpainted | arms, larm, rarm (TR green rectangles; painted 0.0 each, not in P7 islands) | action: add island arms/larm/rarm after label check | confidence 2
GAP | A3,B3 lower halves (below sash, panel y330-384) | dark green robe cloth under the striped sash | torso (TR purple band) | action: widen cells, drop A3,B3 from P7 avoid; keep A2,B2 and A3 strap excluded | confidence 3
GAP | D4 | dark green robe lower, flat, red in BR | torso (TR purple, painted 0.0) | action: widen cells, drop D4 from P7 avoid | confidence 3
GAP | C4 | dark green robe folds, no boot shape visible, red in BR | torso (TR purple, painted 0.009) | action: fix annotation (boot claim) and widen cells, drop C4 from avoid | confidence 2
MISID | D4 (annotation says rust pouch) | rust pouch is at D1 top-right corner (TL x445-500 y5-60); D4 is plain cloth | D1 corner, label not legible | action: fix annotation, move pouch to D1, keep excluded | confidence 4
MISID | A3-B4 (annotation says brown stitched vest, A4 brown vest) | brown stitched leather vest is A1-B1 top row; A3-B4 is dark green robe | torso | action: fix annotation | confidence 4
MISID | C3,D3 (annotation says brown belt band) | no brown belt visible; C3-D3 is dark green cloth | torso | action: fix annotation | confidence 3
GAP | A2 (x30-135 y130-260) | grey gauntlet, red hand shape in BR | green hand island at TR A2, label not legible | action: keep excluded (glove) | confidence 4
GAP | B2,C2 (sash reaches upper B3) | orange/grey striped sash, red and yellow stripes in BR | torso_arm (TR orange stripe), likely | action: keep excluded (belt-like band, palette clash) | confidence 2
GAP | A2 left strip (x0-30), A3 left strap (x0-40 y285-325) | brown bracer strip and brown strap with metal clasp; BR marks skin-like | none, outside P7 islands | action: keep excluded (bracer, hardware) | confidence 3
GAP | A4 claw/grey triangle (x40-120 y395-505) | grey claw or hand shape, skin-like yellow in BR | green claw island at TR A4 | action: keep excluded (hand/skin) | confidence 2
OVERPAINT | D1 top-right corner | BL shows green on pouch edge (x455-510 y515-575) | D1 corner, label not legible | action: keep excluded, restrict D1 mask | confidence 1
GAP | C2/D2 top (x330-400 y130-205) | brown pouch or strap-like object, painted state unclear | torso_arm, likely | action: check BL; keep excluded if pouch | confidence 1
GAP | D2/D3 right edge (x460-512 y190-330) | pale lilac strip, sleeve or arm | larm or rarm (TR blue-violet strip) | action: add island larm or rarm if cloth | confidence 1
GAP | all cyan regions (30.2% of content) | seam padding and bleed outside every island | none | action: keep excluded | confidence 4
OVERPAINT | A1,B1,C1,B4,A4 lower | green covers cloth and leather torso only; no glove, boot, belt in green except D1 edge (line 12) | torso, legs | action: none, keep as is | confidence 4
