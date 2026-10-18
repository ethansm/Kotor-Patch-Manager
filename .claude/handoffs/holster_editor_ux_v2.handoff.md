# Session Handoff & Plan State: .claude/plans/holster_editor_ux_v2.md

## 0. Next-Phase Direction
No phases parsed from .claude/plans/holster_editor_ux_v2.md; check its header levels (## Phase N / table with Id + Status columns).

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/holster_editor_ux_v2.md`
- **Branch:** `KOTOR-II` (8b74740 docs(play-queue): HUX-P0 + HUX-U1 rows (HW-UX2 U0/U1 prepared, GATE PASS))
- **Generated:** 2026-10-18 00:59:43 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
8b74740 docs(play-queue): HUX-P0 + HUX-U1 rows (HW-UX2 U0/U1 prepared, GATE PASS)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 2 ++
 1 file changed, 2 insertions(+)
```
- **Dirty Files (168):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M CLAUDE.md`
  - ... +158 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/holster_editor_ux_v2.md`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## 10-18 state (U0 + U1 built)
- Commits: plan 57ad185; mod-config 0.1.1-dev 1f7b7cb; holster 0.1.4-dev fd6135c; holster-editor 0.2.6-dev 5662013; play rows 8b74740. KPM: 536fa3c, edd9f5b, 8059084 + GameAddr commit.
- Prepared: `kpm_hotswap.py --mod holster --mod holster-editor --mod mod-config --prepare` -> SWAP x3; GATE PASS head 566201349ea5.
- NEXT: user GO (game closed) -> `python3 scripts/kpm_hotswap.py --mod holster --mod holster-editor --mod mod-config --apply`; user plays HUX-P0 + HUX-U1 (+ HW-LH1 ini edit); Haiku log readers via kotor2_logs.py; then U2 (needs P0 camera q confirmation + ITEMNAME result).
- ITEMNAME: identified=0 default; identified=1 (probe_names=2) goes through the strref->achievement switch 0x4010E0 and an unchecked base-row deref.
- 10-18 01:17 HW-UX2 U0+U1 LIVE x3; rollback batch_swap_20261018_011740.

## 10-18 HUX-P0 log read
- ITEMNAME works with identified=0 (name0 `{06}Vibrosword`, `{05}Vibroblade`, `{02}Long Sword`); strip the `{NN}` prefix; identified=1 not needed.
- PROJ refuted S1's camera axes: hook pos (0.024,0.001,1.593), q(w,x,y,z)=(0.7071,0,0,0.7071), fovy 10.33@zoom2.2 / 21.6@1.05, ext (1345,357,576,896). Mounts at world y ~ +-0.44 project to sx 4579 / -1443 (view x range 1345..1921). Camera z follows panZ; q turns ~1 deg per yaw degree.
- NEXT U0b: show candidate-projection labels A..F in the view (six forward/up axis conventions) and ask which sits on the weapon; then fix worldToScreen. U2 mouse/mode bar and U3 rows do not depend on it; U4 markers do.
- 10-18 01:41 U0b LIVE: holster-editor 0.2.7-dev (091a1661), rollback holster-editor_swap_20261018_014104. Play row HUX-C0 (6 calibration clicks x 2-3 rounds); then run fit_camera.py on the CALIB lines and fix worldToScreen.

## 10-18 U2+U3+left mirror prepared
- Commits: mod-config 0.1.2-dev 094f019b (Esc key edge); holster 0.1.5-dev 7fc7ca65+30ad0bba (left default 1, effLeft=left, left_extra_rot -8, src bit 8); holster-editor 0.3.0-dev 7881dc3e (U2) + 0.3.1-dev 670add3c (U3). KPM: b10c245, 904be67, d460820. GATE PASS head 670add3c.
- Prepared swap x3; awaiting GO. Rows HUX-U2, HUX-U3, HW-LH2, ESC2, HUX-C0 (zoom out first).
- HUX-C0 camera fit still pending (fit_camera.py) -> then U4 markers. Known: U3 inspector squeezes strip rows to ~19 px (check readability); body-scope tilt warns 'no change'.
- 10-18 02:48 U2+U3+left mirror+Esc fix LIVE x3: holster f2f57f91, holster-editor 63398f9a, mod-config f12da44c; rollback batch_swap_20261018_024840.
- 10-18 U0c LIVE: holster-editor 0.3.2-dev (649975bb, KPM 220b98d), CAMENG probe; play row CAM1 (no clicks). Rollback: see kpm_hotswap --rollback holster-editor_swap_<latest>. Next: read CAMENG, fix worldToScreen (use engine camera pose, or call MapToScreen), then U4.

## 10-18 HW-UX3 (plan .claude/plans/holster_editor_layout_v3.md, APPROVED "approve HW-UX3")
- User feedback that drove it: default zoom shows hip only (want whole character, fit zoom ~0.6, kZoomMin 0.8->0.4); bigger viewport; description box too tall; show actual values; sliders need a marker with text OUTSIDE (vanilla Sound Options look: tapered teal wedge in rounded frame, thin white double-arrow marker, title above); reset buttons need an arrow icon; no cycling `< >` inputs; page too busy. Crossed left/right swords PASS in play (HW-LH2 row PASS, f49bc1b3). Nice-to-have: flourish out of combat should draw, flourish, re-holster. NEW bug report: weapons stay at the HIP in combat too (HW7 "draw in combat" was never built; the global rhand/lhand literal repoint pins weapons to the hip always).
- CAM1 play result: engine camera view+0x78 pos=(4.8725,0,0.4358) q(wxyz)=(0.4786,0.5210,0.5210,0.4775) fovy=22.73/zoom deg, viewport ints +0x218..+0x224 = 0 (MapToScreen call skipped by design). Render DOES follow a resized control rect (A1/A2: 0x420F50 -> 0x4184F0 -> 0x479460 glViewport from the rect; aspect w/h). Full-window-viewport hypothesis REFUTED (F). Unexplained: screen shows ~1.0 m body at zoom 1.17 while pose+rect maths predicts ~1.8 m. U4 markers stay blocked; CAM2 probe needed (log V+0x64..0x70 pushed rect, glGetIntegerv GL_VIEWPORT, untruncated CALIB node lists).
- Research R1/G (research/holstered_weapons/ux_v3/*.txt): mod-config button borders need new helper (done: guikit::setButtonFill, layout +0x70 border inferred, needs live check); desc split SetDescSplit(page,<=850) items in MUI_PANE_PREVIEW stay clickable; kMaxItems 80 ok (engine list growth unverified, 24->40 worked); page has 38 items today, 7 stances in code; no absolute bone setter (loop CycleBone). Flourish: SetCombatState mode 2 server-side (0x79F621, guard 0x57A926) so client flag +0x138 never changes; hip pin is global load-time imm32 (code_patches.toml:7) with H7 write at 0x504EF5 as the per-creature gate point; vanilla Sound Options slider texture names NOT in repo (only SLD_ALIGN uibit_forcebar_p): draw our own wedge/marker textures on plain labels via setFill.
- Built + committed 0be7747b (all host-tested, NOT installed): HseLayoutV3.h (pageLayoutV3, 69 items, 3 screen sizes), ResetIcon.h + ResetIconTex.inc + scripts/gen_reset_icon.py (hse_reset/hse_reset_hi 32x32), GuiKit.h button fill helpers, HolsterLogRate.h + holster 0.1.6-dev quiet logging (`log_verbose=0` default; =1 old behaviour), mod-config kMaxItems 80 (in 6d854c0c). Pending: KPM Common sync (check_shared_headers --sync) before holster/holster-editor builds; editor HseView.h kPageItemCap=40 must go to 80; editor log noise (PROJ per tick, CAMRAW, CHORD IDLE->DEAD, INSP refresh) to be cut in the page rebuild.
- Live now unchanged: holster 0.1.5-dev, holster-editor 0.3.2-dev, mod-config 0.1.2-dev (repo has newer, uninstalled).
- Opus independent review of ALL findings+code launched (output research/holstered_weapons/ux_v3/OPUS_review.txt). User wants: fixes applied, context persisted, then PLAN MODE with a detailed plan that states the strategy: parallel Haiku (<80k ctx, cap ~15 tool calls each) for research/investigation/validation/testing; Sonnet only when Haikus disagree or flag complexity; Opus review at the end. User will "clear context and apply" from plan mode, so the plan must be self-contained.

## 10-18 later: Opus review DONE, fixes committed e93904c8 (holster 0.1.7-dev, still NOT installed)
- Review file research/holstered_weapons/ux_v3/OPUS_review.txt (APPROVE-WITH-FIXES). Fixed: COMB quiet-mode dropped ~90% of edges (now edges always log, ring eviction, creatureKey), HseLayoutV3 view/preview overlap guard + 18 px hints, setButtonFillStyle probes both borders, resetIconApply (button then label fallback), plan §4b corrections.
- Corrections to remember: camera ctor 0x47ECD0 zeroes +0x218..0x224 (A1/F wrong); render uses the same +0xA8/+0xB4 pose; predicted visible span ~1.7 m (z 0.02..1.69) vs eyeballed 1.0 m, cause unknown, CAM2 must log draw-time GL_VIEWPORT, S rect vs control rect, parent stack DAT_00a309c0/c8/ca/ce, DAT_00a32a54, GUI-to-pixel scale. Flourish/combat draw design (HW7): native rhand/lhand attach (bone_test=0), per-frame H7 (0x504EF5) hip write only while owner [parent+0x1D8] not drawn; drawn = [+0x138]!=0 OR flourish window from 0x7408A0/0x66B8A0 (or 0x57A560 param3==2). Un-repoint-around-flourish is WRONG.
- Remaining L1 scope gaps (must be in the plan): editor consumer cap 40 -> 80 (HseView.h:793-794 static_assert, hse_view_test.cpp:980, common_contract_test.cpp:226-227) and install mod-config first (no cap query in ABI); refresh g_ed.viewRect (HolsterEditor.cpp ~1768-1771, users ~1192/1422/1510) and the wheel hit rect (~1780) after the view resize; reset icon label-clone fallback; per-frame SetRect re-apply cap kMaxReapply=8 in mod-config (drifting pages); check hint font height; vanilla/mod-config controls under the widened view band (play check).
- KPM Common sync (check_shared_headers --sync) still pending before holster/holster-editor builds. Live versions unchanged.

## 10-18 HW-UX3 build round (plan .claude/plans/holster_hw_ux3_build.md; waves A, B, C1 DONE, nothing installed)
- Commits: wave A (CAM2 `logCam2` probe in HolsterEditor.cpp, slider art gen_slider_art.py/SliderArtTex.inc/SliderArt.h) -> allowlist 5 CAM2 engine globals in tests/static/raw_addresses.toml -> C1 holster 0.2.0-dev d3e97cd6 (`place_mode=drawgate`: native rhand/lhand attach, hip pose written only while [owner+0x138]==0; HolsterDrawGate.h) -> C1 review fixes 545307ac (B1 weapon-only "w_" filter, B2 hand from node name) -> wave B holster-editor 0.4.0-dev bb7747d7 -> round review fixes + play rows 8f680952.
- Reviews: research/holstered_weapons/ux_v3/OPUS_review_C1.txt (FIX-FIRST, fixed) and OPUS_review_round.txt (0 blocking; F1 bone buttons no longer use the unsaved-edit guard because a bone change keeps the values; F2 art textures created independently; F3 zero-width wedge parked; F4 bone loop checks edit open). APPROVE-FOR-INSTALL.
- Validation findings: A0c reapply counter is zeroed by every SetRect (per-tick SetRect safe; "move only if >1 px" can strand a rect); mod-config 0.1.3-dev (6d854c0c) carries SetDescSplit + the 80 cap together, so hasDescSplit()==true implies cap 80; page is 75 items (69 + 6 wedge labels).
- Owner decisions open: N1 "Reset Scope" button was dropped (its delete-overrides function now unreachable from the UI); N2 slider thumb moves at scrub rate, not track scale, while dragging. C2 (flourish window) needs per-creature window (g_flourishOpen is one global bool), new hooks for 0x7408A0 (7B `558bec51894dfc`), 0x66B8A0 (6B `558bec83ec48`, invoker unknown), 0x57A560 return 0x79F626/0x66BAC7; full reinstall + GO.
- Play rows added: UX3, UX3b, CAM2, LOG1, ICON1, HW7a, HW7c (PLAY_TEST_QUEUE.md). HW7b waits for C2. CAM1 row status in the queue is still stale.
- NEXT (Wave D, needs user GO + game closed): `check_shared_headers.py --sync` -> `kpm_status.py` -> `kpm_hotswap.py --mod mod-config holster holster-editor --prepare` (expect SWAP x3) -> fresh `pre_install_gate.sh --mod mod_config_src --mod holster_src --mod holster_editor_src` GATE PASS -> GO -> `--apply`. Live versions still holster 0.1.5-dev, holster-editor 0.3.2-dev, mod-config 0.1.2-dev. Wave E (U4 markers) blocked on CAM2 log.
