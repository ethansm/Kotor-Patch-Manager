# Haiku check a: holstered weapon contact sheets

Sheets judged from the rendered PNGs only (no zoom). Camera is the same for every tile.
Cell key: VISIBLE / PARTIAL / HIDDEN, plus position and direction for VISIBLE and PARTIAL.
Small hilts are the least certain reads.

## S1 = n_commm_back.png

| weapon \ bone | pelvis_g | lthigh_g | rthigh_g | torso_g | torsoUpr_g | rhand |
|---|---|---|---|---|---|---|
| w_lghtsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | PARTIAL: tiny hilt at right hand, direction unclear |
| w_dblsbr_001 | HIDDEN | HIDDEN | HIDDEN | PARTIAL: tiny tip at top of head, up | VISIBLE: sticks up above neck, vertical | VISIBLE: double hilt in right hand, diagonal down-right |
| w_shortsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | PARTIAL: tiny hilt at right hand, direction unclear |
| w_blstrpstl_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | VISIBLE: pistol in right hand, diagonal down-right |
| w_blstrcrbn_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | VISIBLE: carbine in right hand, diagonal down-right |
| w_hvyblstr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | VISIBLE: heavy blaster in right hand, diagonal down-right |

## S2 = n_commf_front.png

| weapon \ bone | pelvis_g | lthigh_g | rthigh_g | torso_g | torsoUpr_g | rhand |
|---|---|---|---|---|---|---|
| w_lghtsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | PARTIAL: small hilt at right hand, diagonal |
| w_dblsbr_001 | PARTIAL: small tip above head, up | HIDDEN | HIDDEN | VISIBLE: sticks up above neck, vertical | VISIBLE: sticks up above neck, vertical | VISIBLE: small double hilt at right hand, diagonal down-left |
| w_shortsbr_001 | HIDDEN | HIDDEN | HIDDEN | HIDDEN | HIDDEN | PARTIAL: small hilt at right hand, diagonal |
| w_blstrpstl_001 | VISIBLE: pistol at belt, down | VISIBLE: pistol at left hip, down | VISIBLE: pistol at right hip, down | VISIBLE: pistol on chest, vertical | VISIBLE: pistol on upper chest, vertical | VISIBLE: pistol in right hand, diagonal down-left |
| w_blstrcrbn_001 | VISIBLE: carbine at belt, down | VISIBLE: carbine at left hip, diagonal | VISIBLE: carbine at right hip, diagonal | VISIBLE: carbine on chest, vertical | VISIBLE: carbine on upper chest, vertical | VISIBLE: carbine in right hand, diagonal down-left |
| w_hvyblstr_001 | PARTIAL: thin tip at belt centre, vertical | VISIBLE: blaster at left hip, down | VISIBLE: blaster at right hip, down | VISIBLE: blaster on chest, vertical | VISIBLE: blaster on upper chest, vertical | VISIBLE: heavy blaster in right hand, diagonal down-left |

## Questions

### S1 (n_commm_back.png)
- Q1: YES. Rows 1-3 in pelvis_g, lthigh_g, rthigh_g, torso_g are HIDDEN, except row 2 torso_g which is PARTIAL (tiny tip at top of head).
- Q2: YES. Row 2 torsoUpr_g: hilt clearly sticks up above the neck.
- Q3: NO. Rows 1 and 3 at rhand are PARTIAL (tiny hilts). Rows 2, 4, 5, 6 are VISIBLE.

### S2 (n_commf_front.png)
- Q1: NO. Row 2 torso_g is VISIBLE (hilt above the neck), so the double saber is not hidden there. Row 2 pelvis_g is PARTIAL.
- Q2: YES. Row 2 torsoUpr_g: hilt sticks up above the neck.
- Q3: NO. Rows 1 and 3 at rhand are PARTIAL (small hilts). Row 2 is a small double hilt.

## Verdict

PARTIAL. Q2 is YES on both sheets. Q1 is YES on S1 and NO on S2. Q3 is NO on both sheets. The swing read is S2 row 2 torso_g.
