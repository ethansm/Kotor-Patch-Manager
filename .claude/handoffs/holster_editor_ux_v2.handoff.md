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
