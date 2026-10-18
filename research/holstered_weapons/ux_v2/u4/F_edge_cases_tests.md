# U4 Question F: edge cases and test matrix (viewport markers)

Recon only, 10-18. Sources: .claude/plans/holster_editor_ux_v2.md:105-113, 124-191, 207; .claude/handoffs/holster_editor_ux_v2.handoff.md:44-51.
"new" = pure hse:: function that does not exist yet (rg finds no hover, nearest-axis or marker code). Names are proposals.
Host tests: hse_view_test.cpp uses KT_CHECK / KT_CHECK_EQ (HCHECK/HEQ are only in common_contract_test.cpp).

## Blockers (read first)
1. Projection model is refuted. HseView.h:161-197 (quaternion, node -Z forward) fails HUX-P0 (mounts at sx 4579 / -1443). HUX-C0 fit is pending (handoff :46, :51). The 8 vectors in hse_view_test.cpp:164-246 (yaw case :212-218 included) pin the refuted model: re-derive them, do not keep them.
2. Camera is not static. S1 Q1 says "camera fixed" (research/holstered_weapons/ux_v2/S1.md:4-7). HUX-P0 shows q turns about 1 deg per yaw degree and camera z follows panZ (handoff :44). Marker code must re-read camerahook every tick (P12, P13).
3. Item budget: kPageItemsBase 26 + kInspItems 12 = 38 of kPageItemCap 40 (HseView.h:731-734; test at hse_view_test.cpp:975-980). A 5-item marker pool gives 43: AddX returns -1 (B1).

## Matrix (39 rows; ok = worldToScreen.ok; NDC/px in UI units; ext = {x,y,w,h})
| id | scenario | expected | host pin (fn + concrete vector) |
|---|---|---|---|
| P1 | point behind camera | hidden | existing :200; cam 0, identity, P=(0,0,3) gives ok=0 |
| P2 | depth exactly 0.05 | hidden (test is `<=`) | existing :204; P=(0,0,-0.05) gives ok=0 |
| P3 | depth 0.0502 | shown, depth 0.0502 | existing :206; P=(0,0,-0.0502) |
| P4 | depth 0.06, far off-screen | ok=1, sx≈1643.4; rect test hides it (no clamp) | new; P=(1,0,-0.06), fovy 60, ext {100,50,200,100} |
| P5 | NaN/inf in P, cam, q, fovy (raw 22.726/0 = inf) | ok=0 | existing :228-238; add fovy=NaN and fovy=inf |
| P6 | zero quaternion; q={3,0,0,0} | zero: ok=0; {3,0,0,0}: ok=1 | existing :236, :246 |
| P7 | ext w<=0 or h<=0; negative ext.x | w/h<=0: ok=0; ext {-300,50,200,100}, P ahead: sx=-200, ok=1 | existing :239-241; new negative-x vector |
| P8 | ultrawide: same P, ext w 200 to 420 (h 100) | sx 305 to 415; sy unchanged | new; P=(2.4249,0,-2), fovy 60, cam 0, identity |
| P9 | zoom 2.2 vs 1.05 (fovy 10.33 vs 21.64) | P=(0,0.1,-1): sy 44.7 vs 73.8 | new; fovy = 22.726/zoom (HUX-P0 log) |
| P10 | foreshortened axis (tip on view ray) | length 0, axis hidden | new; O=(0,0,-3), T=(0,0,-3.5) both sx=200, sy=100 |
| P11 | axis tip behind camera, origin visible | axis hidden (no segment clipping) | new; O=(0,0,-1), T=(0,0,1): T.ok=0 |
| P12 | panZ moves camera z mid-drag | re-project each tick; no cached pose | new; cam z 1.593 vs 1.411, same P: sx/sy differ; GetNodeWorld called once per tick |
| P13 | yaw turns camera q | markers move; S1 "fixed camera" not assumed | new; q rotated 10 deg about Z, P=(0,0.1,-1): sx moves >1 px |
| P14 | left vs right mount order | sign(sx_r - sx_l) = sign(camRight·(r-l)) | new; rhand (0.03,0.44,0.85), lhand (0.03,-0.45,0.87) (HUX-P0) |
| G1 | no edit open | all markers parked at -9999; 0 SetRect after park | new markerGate(editOpen=0): all hidden; cf. HolsterEditor.cpp:1103 |
| G2 | edit open, weapon not registered | weapon-relative markers hidden; status says why | new markerGate(editOpen=1, weapon=0): 0 visible |
| G3 | first frame, camera not read yet (HolsterEditor.cpp:2042) | all hidden; one WARN (warnOk latch) | new markerGate(camOk=0): hidden |
| G4 | provider gone mid-edit (RigGob null, GetNodeWorld false) | park all; no write to stale ctl; resumes on camOk | new; camOk 1,0,1 gives visible,hidden,visible; 0 writes on step 2 |
| G5 | mode switch while hovering (Tilt to Move) | hover bracket and active axis cleared | new; active=X in Tilt, mode=Move gives active=none |
| G6 | member or scope switch while hovering | hover reset; markers re-placed from new mount | new; enterEdit/member/scope reset (plan :130-131) |
| G7 | press starts on a marker | no orbit, no weapon gesture | new chord input inView=0 with L-edge gives action none (cf. testChordPressOutsideNeverStarts) |
| G8 | RMB down on hovered axis | active axis set in Tilt/Roll-Turn; ignored in Move/Depth | new; hover X, RMB in Tilt gives active=X; in Move unchanged |
| G9 | cursor leaves axis during an active drag | hover frozen to the pressed axis until release | new; dragActive=1 ignores cursor move |
| H1 | axis projected length < 10 px | hidden; 10.0 shown | new axisVisible(O=(200,100), T=(209.99,100)) gives 0; T=(210,100) gives 1 |
| H2 | cursor 10 px from x tip | hover x | new hoverAxis; O=(100,100), X=(110,100), cursor (120,100) gives x |
| H3 | cursor exactly 20 px away | none (strict <20) | new; cursor (130,100) gives none; (129,100) gives x |
| H4 | equal distance (5 px) to x and y segments | lower index (x) | new; O=(100,100), X=(110,100), Y=(100,110), cursor (105,105) gives x |
| H5 | nearest, 1 px to x and 5 px to y | x | new; cursor (105,101) gives x |
| H6 | hidden (foreshortened) axis under cursor | excluded; next visible axis or none | new; x hidden, cursor on x gives y or none |
| H7 | two tips on one pixel; anchor and mount on one pixel | both drawn, fixed draw order; anchor and mount never hover | new; X=Y=(110,100), cursor (110,100) gives x |
| R1 | point outside live view rect | park, no clamp | new inRect(ext {1345,357,576,896}): (1344,400) gives 0; (1345,400) gives 1; (1921,400) gives 0 |
| R2 | point inside, label rect over edge | hide marker | new labelInside((1915,400,24,16)) gives 0 (1939 > 1921) |
| R3 | view rect moves on layout | re-place from PaneRect each tick; pure translation | new; ext.x 1345 to 1395: centre sx 1633 to 1683 |
| R4 | 1 px churn threshold | SetRect only if rounded rect moves >=1 px | new needsSetRect; ({100,50,20,20} vs same) gives 0; x 101 gives 1; 100.4 rounds to 100 gives 0 |
| R5 | gaveUp latch (ModConfig.cpp:67, :271; reset :701) | after 8 re-applies: stop writing, hide, log once | new markerChurn counter: 9th mismatch gives 0 writes, state=GAVE_UP |
| R6 | own writes fight engine re-layout | 1 SetRect per changed marker per tick | new; mock: 7 markers changed gives 7 calls, unchanged gives 0 |
| B1 | 26+12 base, 5 marker items | 43 > 40: needs one of the fixes in OPEN | new static check 38+5 <= 40 FAILS (BLOCKER) |
| B2 | any marker AddX returns -1 | markers disabled as a set, WARN, no crash | new markerEnabled(all AddX>=0), as modeBarOk (HolsterEditor.cpp:477-478) |
| B3 | S1 Q2 pool of 7 (mid-dots added) | budget 45: reject | new markerBudget(7) gives 45 > 40 |

## Live-play checks (add to patch_manager_mods/PLAY_TEST_QUEUE.md as pending rows; not run here)
- HUX-U4a: after the C0 fit, the diamond and plus sit on the drawn bone and mount, within 10 % of the view.
- HUX-U4b: the x, y, z tips point along the axes Tilt, Roll and Turn drag; the hovered tip shows brackets.
- HUX-U4c: a press on a marker does not orbit; a press off markers orbits.
- HUX-U4d: zoom, panZ and yaw mid-drag keep markers on the weapon with no -9999 jump while the weapon shows.
- HUX-U4e: no edit open shows no markers; left and right edits show the right hand's markers.
- HUX-U4f: 60 s session log has no `RELAYOUT gave up` and no `MUI refused` (kotor2_logs.py check).

## Gates
`python3 scripts/run_tests.py hse_view holster common_contract menu_ui mod_config`; `python3 scripts/build_check.py`; `python3 scripts/compat_check.py` on any touched shared header (MenuUiLogic.h, ModConfigApi.h if kMaxItems changes).

## OPEN decision (one question)
Budget fix for U4: raise mui kMaxItems above 40 (shared lib, version bump), drop the 6 inspector reset buttons, or cut the marker pool to 2 items?
