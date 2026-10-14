# Haiku check b: n_commf contact sheets

Sheets: S1 = n_commf_side.png (side view), S2 = n_commf_back.png (back view).
Facing: S1 character faces the right edge of each tile (barrels that stick out to the right are forward).
S2 back view: forward is toward or away from the camera, so forward-pointing parts appear end-on (specks).

Cell key: VISIBLE = clearly seen on or outside silhouette. PARTIAL = only a small tip or edge shows. HIDDEN = nothing seen.

## S1 (side view)

| Weapon \ Bone | pelvis_g | lthigh_g | rthigh_g | torso_g | torsoUpr_g | rhand |
|---|---|---|---|---|---|---|
| 1 w_lghtsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | VISIBLE: at hand, horizontal, points forward |
| 2 w_dblsbr_001 | PARTIAL: sliver above head, vertical, up | HIDDEN | HIDDEN | PARTIAL: sliver above head, vertical, up | PARTIAL: sliver above head, vertical, up | VISIBLE: at hand, horizontal, extends forward far |
| 3 w_shortsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | VISIBLE: at hand, horizontal, short, forward |
| 4 w_blstrpstl_001 | VISIBLE: right of torso, chest height, points forward | VISIBLE: right of torso edge, horizontal, forward | VISIBLE: right of torso edge, horizontal, forward | VISIBLE: right of torso, upper chest, forward | VISIBLE: right of upper torso, horizontal, forward | VISIBLE: at hand, vertical, points down |
| 5 w_blstrcrbn_001 | VISIBLE: right of hip, horizontal barrel, forward | VISIBLE: right of waist, horizontal barrel, forward | VISIBLE: right of waist, long barrel, forward | VISIBLE: right of torso, horizontal barrel, forward | VISIBLE: right of upper torso, horizontal barrel, forward | VISIBLE: at hand, vertical, points down |
| 6 w_hvyblstr_001 | VISIBLE: right of chest, horizontal, forward | VISIBLE: right of waist, horizontal, forward | VISIBLE: right of waist, horizontal, forward | VISIBLE: right of torso, horizontal, forward | VISIBLE: right of upper torso, horizontal, forward | VISIBLE: at hand, vertical, points down |

## S2 (back view)

| Weapon \ Bone | pelvis_g | lthigh_g | rthigh_g | torso_g | torsoUpr_g | rhand |
|---|---|---|---|---|---|---|
| 1 w_lghtsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | PARTIAL: speck beside hand, end-on |
| 2 w_dblsbr_001 | PARTIAL: speck on top of head, up | HIDDEN | HIDDEN | VISIBLE: grey shaft above head, vertical, up | VISIBLE: grey shaft above head, vertical, up | PARTIAL: two specks beside hand, end-on |
| 3 w_shortsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | PARTIAL: speck beside hand, end-on |
| 4 w_blstrpstl_001 | HIDDEN | HIDDEN | HIDDEN | PARTIAL: dark speck mid-back, end-on | PARTIAL: dark speck upper back, end-on | VISIBLE: at hand, diagonal, down-right |
| 5 w_blstrcrbn_001 | HIDDEN | PARTIAL: speck at waist, left of centre | PARTIAL: speck at waist, right edge | HIDDEN | HIDDEN | VISIBLE: at hand, diagonal, down-right |
| 6 w_hvyblstr_001 | HIDDEN | HIDDEN | HIDDEN | PARTIAL: pale speck mid-back, end-on | PARTIAL: grey speck at back, waist | VISIBLE: at hand, diagonal, down-right |

## Questions

Q1. Rows 4-6, cols pelvis_g..torsoUpr_g: barrels horizontal and forward?
- S1: YES. Barrels stick out horizontally to the right of the body in all 15 cells (e.g. S1 row4 col1 pistol, S1 row5 col4 carbine, S1 row6 col2 heavy blaster).
- S2: UNSURE. Back view shows forward barrels end-on; only specks appear (S2 row4 col4, row5 col2-3, row6 col4-5). Consistent with forward, not proof.

Q2. Rows 1-3, cols pelvis_g, lthigh_g, rthigh_g, torso_g: HIDDEN or PARTIAL?
- S1: YES. All 12 cells are HIDDEN or PARTIAL. Only S1 row2 col1 and row2 col4 show tips above the head.
- S2: NO. S2 row2 col4 (torso_g) shows a grey double-saber shaft clearly above the head, which is VISIBLE, not PARTIAL. The other 11 cells are HIDDEN or PARTIAL.

Q3. Rhand column: all 6 weapons clearly next to the hand?
- S1: YES. All 6 are visible at the hand: lightsaber, double saber and short saber horizontal; pistol, carbine and heavy blaster vertical, pointing down.
- S2: NO. Rows 1-3 show only specks beside the hand (end-on). Rows 4-6 are clear at the right hand.

## Borderline cells (judgement calls)

- S2 row2 col4 and row2 col5: classed VISIBLE. If a grader counts them as tips only, S2 Q2 flips to YES.
- S1 row2 col1, col4, col5: classed PARTIAL (sliver at the top edge of the tile).
- S2 row5 col2, col3 and S2 row4/row6 specks: classed PARTIAL because they are tiny; they may be end-on barrels or stocks.

## Verdict

PARTIAL. S1 answers YES on all three questions. S2 has NO on Q2 and Q3 and UNSURE on Q1.
