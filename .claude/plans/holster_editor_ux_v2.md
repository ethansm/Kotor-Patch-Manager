# Plan HW-UX2: holster editor hybrid UX (Gemini 4 Argon research, verified)

Status: APPROVED 2026-10-18 (user "approve HW-UX2"); implementing U0+U1. Source: `/home/ethansm/Documents/holster-3d-edit.pdf` (11 pages, Gemini webapp).
Verification: 11 Haiku recon + 3 Sonnet arbiters, reports in `research/holstered_weapons/ux_v2/` (H1-H11, S1-S3,
`s3_sim.cpp` gimbal repro). Cost $3.46. Parent plan: `.claude/plans/holster_editor_options.md` (HW-EDIT v2).
User framing: mockups are brainstorm only. The skeletal bone-node buttons shown on every body are DEFERRED to a later
toggle mode; the other elements (RMB mode bar, inspector rows, markers, stance bar) are wanted.

## 0. Constraints (unchanged)
- `git commit -- <paths>` only. Installs: the user's GO with the game closed. DLL-only = hot-swap after GATE PASS.
- No `0x00[4-B]xxxxx` literals in sources. Versions 0.x.y. Live-only checks go to `PLAY_TEST_QUEUE.md`.
- Holster owns data and pose maths; the editor owns UI; mod-config owns controls.

## 1. Verdict on each Gemini idea
| # | Idea | Verdict | Plan |
|---|---|---|---|
| G1 | LMB = character, RMB held + LMB drag = weapon (user's own idea) | CONFIRMED feasible (H5) | U2: chord state machine |
| G2 | RMB mode hot-buttons [Move][Depth][Tilt][Roll] above the view | Feasible: 4 buttons, needs a higher item cap (S2 Q5) | U2 |
| G3 | RMB + wheel = depth; Shift fine, Ctrl snap | Feasible. Wheel+Shift is depth today (H5) | U2: wheel while RMB held = depth |
| G4 | Inspector rows: slider + scrubbable number + reset | No slider or editbox in mod-config (H6). Scrub = button + tick drag, HIGH feasibility | U3: scrub rows, no slider, no typing in v1 |
| G5 | Clamps: ±15 cm, depth −5..+10 cm, ±45° | REFUTED: they clip live values (heavy z 0.25 m, melee rx −150°) (H6) | U3: window ±0.30 m / ±180° around the open value, parser limits 3 m / 720° |
| G6 | Bone-relative labels (Height, Depth, Slide, Tilt, Cant, Roll) | Good. Gemini's "Depth = Z" is wrong: in the bone frame y = depth, z = height (H6) | U3: Slide x, Depth y, Height z; Tilt rx, Roll ry, Turn rz |
| G7 | Stance bar [IDLE][WALK][RUN][SPRINT][CROUCH][DRAW][HOLSTER] | Partial: Sprint, Crouch and Holster have no K2 anim (H7) | Keep the stance cycle; drop those 3; DRAW (`g<N>w1`) only after a play check |
| G8 | Slow-mo scrubber | REFUTED: the speed arg is an int loop flag (H7) | Dropped |
| G9 | Freeze animation while RMB held | UNSURE: flag 0x400 lead, unverified (H7) | Deferred probe; fallback = switch to `pause1` while RMB held (opt-in) |
| G10 | Six bone buttons on the body | Five bones exist (torsoUpr_g, torso_g, pelvis_g, rthigh_g, lthigh_g); no CHEST or BACK_LOWER (H4) | Deferred: user wants it later as a toggle mode |
| G11 | WorldToScreen projection for markers | CONFIRMED without a GL hook: the camera is fixed (yaw turns only the creature), vertical fovy, aspect = extent (S1) | U0 probe, then U4 markers |
| G12 | GL overlay lines, rings and grid (glMultMatrixf, depth off) | Feasible but NOT "40 lines, no risk": mid-function hook 0x480385, full reinstall, GL state leaks, no precedent (H1, S1) | Deferred to U6, only if U4 markers are not enough |
| G13 | Native .mdl gizmo models | REFUTED as a plan: no hide flag, depth test stays on, a sibling misses the holster offset, authoring path unproven (H3, S1) | Rejected |
| G14 | Hover-highlight the nearest axis; snapshot the axis on RMB down | Hover feasible via raw cursor (no hover API). Snapshot unneeded: worldDeltaToLocal is exact per event (H8) | U4 (hover), no snapshot |
| G15 | Pivot at the bounding-box midpoint | No extents code (H8) | Deferred |
| G16 | Quaternion canonical, Euler only for display | PARTIAL: placeQ exists but render and save use the Euler fit. **Real bug**: in the band 80 ≤ abs(ry) < 90, eulerNear freezes rx. Render error up to 18.1°, display jump up to 66.9° (S3 repro on real headers) | U1: fix eulerNear |
| G17 | Mirror to opposite side | Mirror already exists: resolveFull reflects (rx,−ry,−rz)/(−x,y,z) for inherited left (H4, S2). No button needed | U1: show "mirrored" status; U3: left extra tilt |
| G18 | Saber blade hilt-only / extended toggle | No blade state anywhere; doc 42 Q7 open (H9) | Play row HW-SBR1 first |
| G19 | Armour bulk, mannequin or body cycler, modelscale | Preview armour unproven; no cycler; modelscale unhandled (H9) | Play rows HW-PV2, HW-BD1; deferred |
| G20 | Prior art (IED, Bannerlord, Warframe, PSO2, Gunman, ImGui modifiers) | 0 CONFIRMED: Bannerlord, Gunman and PSO2 not found; Warframe holsters are cosmetic; ImGui is Shift ×10, Alt ×0.01 (Gemini quoted Blender) (H10) | Keep our Blender-style Shift = fine, Ctrl = snap |

## 2. Decisions (defaults; the user can override)
1. **Left side (user request):**
   - Left weapons mirror the right automatically. The cause of today's overlap is the editor-preview gate `effLeft = left && g_boneLeft` (Holster.cpp:631). In world with `bone_test_left=0` the left weapon stays in the hand, not holstered (S2).
   - The 10-17 FW-H1 decision C4 is reversed.
   - Order: HW-LH1 (the user sets `bone_test_left=1`), then flip the code default to 1 and drop the gate.
2. **Criss-cross on the upper back:**
   - The mirror alone crosses rifle and melee as an X but the barrels touch at x=0, and heavy overlaps exactly (H4).
   - Add `left_extra_rot` (degrees about bone X, applied as a quaternion after the mirror; inherited mirror levels only, never on explicit `l_` keys). Default −8° (rifle separation 2.9 cm; −12° gives 4.3 cm).
   - The sign is tuned live: the user wants the blade "pointed up a little more" and sitting on top of the right one.
3. **Mouse in weapon mode:**
   - LMB drag = orbit the character (as in nav).
   - RMB held + LMB drag = act on the weapon, per the mode bar: Move (screen plane), Depth (bone y), Tilt (rx), Roll (ry), Turn (rz).
   - RMB + wheel = depth in any mode. RMB alone = nothing (the trackball is removed; Tilt, Roll and Turn replace it).
   - MMB stays spin. Shift = ×0.2, Ctrl = snap (1 cm or 5°, accumulated).
4. **Per-row reset:** reverts that one value to what it was when the edit opened. Editor-side, no API change. "Use inherited" for a group comes later via a new holster slot.
5. **Names:** English item names through a verified path only (S3: the client item +0x2C0 is unproven). The U0 probe settles it; fallback is the resref.
6. **Scope label:** "For: this weapon / all rifles / this body"; the tooltip (SetDesc) explains it. Category is hidden when the type is `unknown`.

## 3. Waves (each small; Sonnet workers with disjoint write scopes; Haiku reviewers)

### U0 probe (holster-editor DLL-only hot-swap; no new hook)
- CAMHOOK line on the first LIVE tick:
  - camerahook pos plus q (w,x,y,z: fix today's `{0,0,0,1}` w-last init), fovy, extent;
  - `PROJ` of the mount world pos (GetMountWorld) and of mount + 0.3 m world +Z, using the S1 formula.
- Item name probe: per hand, log the id, the client item ptr, the resref, and the result of CSWCItem::GetName (Steam address via `static_facts.py` plus an address-DB row; never a raw literal).
  - The server-item route stays an option if GetName is unverifiable.
  - If no Steam address is confirmed, ship only the camera probe.
- Play row HUX-P0. Pass:
  - the PROJ mount point lands on the drawn weapon within 10 % of the view;
  - the +Z point is above it;
  - a readable name is logged.

### U1 correctness (holster 0.1.4 + holster-editor 0.2.6 + mod-config; DLL-only)
- **Holster `eulerNear`** (HolsterEdit.h:85-105):
  - drop the frozen-rx branch;
  - choose the primary or alternate exact decomposition by continuity;
  - use prev rx only at exactly ±90.
  - Optional: the render path uses placeQ while an edit is open.
- **Holster:** `effLeft` returns `left` once the default flips (after HW-LH1). Update the tests that pin the old gate.
- **Editor input defects:**
  - split lastMouse per button (H5);
  - reset pressed and dragging flags in enterEdit and exitEdit (S3: a stale pressedRmb eats one press);
  - RMB Ctrl snap uses `snapAccum` with a 5° step.
- **mod-config `kMaxItems` 24 → 40** (MenuUiLogic.h:36): safe, internal, no ABI (S2 Q5). Static grows ~42 KB. Ship it together with the pending Esc fix c59d2d4.

### U2 controls (holster-editor 0.3.0)
- Pure `hse::Chord` step in HseView.h:
  - inputs: L, R, M, edges, inView, fg, shift, ctrl, dx, dy, wheel;
  - output: action {none, orbit, weapon(mode), depth, spin, zoom}.
  - Uses the state table in H5 §5: IDLE, ORBIT, WEAPON, DEAD. Read R before L.
- Mode bar: 4 buttons [Move][Depth][Tilt][Roll/Turn] at the top of the view rect, live only in WEAPON mode and parked at −9999 in nav. The active mode is highlighted with a text marker (`>Move<`).
- Mappings:
  - Move: NudgeWorld dp = (0, dx, −dy)·k (camera fixed, so this stays valid once U0 confirms the camerahook q).
  - Depth: NudgeLocal(1, 0, −dy·k).
  - Tilt, Roll, Turn: NudgeLocal(axis, ±px·0.5°, 0), with the sign from the U4 projected axis when available, else fixed.
- Help line in the status pane: "LMB turn · RMB+LMB move weapon · RMB+wheel depth · Shift fine · Ctrl snap".
- Names (from U0) and Scope labels (§2.5, §2.6), text cut with `textfit::fitText` to the half-strip width.

### U3 inspector rows (holster-editor 0.3.x; needs U1 kMaxItems)
- 6 rows: Slide, Depth, Height (off x, y, z, in cm) and Tilt, Roll, Turn (rot x, y, z, in deg).
  - Each row = one scrub button `Depth  -11.0 cm` plus one reset button (↺ or `R`). That is 12 items; the page totals ~38.
  - Scrub: LMB press on the row, horizontal drag 1 px = 1 mm or 0.5°; Shift ×0.2; Ctrl snap 1 cm or 5°; applied by SetValues with absolute values.
  - Window: open value ±0.30 m or ±180°; hard limits 3 m and 720°.
- The rows are visible only in WEAPON mode. In nav mode the space holds the stance and emote rows (no item-cap churn: items are parked, not freed).
- Left hand: the status shows `Left: mirrored from right` while src = mirror. The first edit writes explicit `l_` keys without baking in `left_extra_rot` (S2 Q4).
- `left_extra_rot`: global ini key plus an optional per-category override in resolveFull (HolsterTune.h:318), mirrored levels only.

### U4 viewport markers (holster-editor; pure maths in HseView.h)
- `worldToScreen(camPos, camQ, fovy, ext, p) → {ok, sx, sy}`: reject depth ≤ 0.05; top-left UI units (S1 Q2).
- Marker pool, fixed at build:
  - bone anchor `◇`, mount point `+`, and 3 axis tips `x y z` (colour-coded by text), placed with SetRect each frame;
  - parked at −9999 when hidden;
  - an axis is hidden when its projected length is under 10 px.
- Hover: the nearest axis segment within 20 px is shown in brackets; on RMB down it becomes the active axis (replaces the mode bar for Tilt, Roll and Turn when used).
- View drags ignore presses that start on a marker.
- Watch the gaveUp latch (ModConfig.cpp:271, 8 re-layouts): markers must not fight engine layout.

### U5 left criss-cross tune (play-driven)
- After HW-LH1 and U3: tune `left_extra_rot` for rifle, melee and heavy in the editor; save per-category `l_` keys if needed.

### Deferred (separate future plans)
- Bone-node toggle mode (G10).
- GL overlay (G12).
- Animation freeze (G9).
- DRAW stance, hilt-only toggle, armour or body cycler, modelscale, bbox pivot, typed numeric input (CSWGuiEditbox RE).

## 4. Edge cases (each maps to a test or a play step)
1. **Chord order** (unit tests on hse::Chord):
   - R before L (weapon) vs L before R (orbit latched, R ignored until L up);
   - same-tick edges (weapon wins);
   - R released mid-weapon drag (DEAD until L up, no orbit jump);
   - L released first (end).
2. **Focus:**
   - alt-tab mid-drag means release; a held button on return needs a fresh edge;
   - a press that began outside the view or on a button never drags;
   - a press that began on a marker never orbits.
3. **Mode and modifiers:**
   - mode change mid-drag applies on the next press only;
   - Shift or Ctrl toggled mid-drag changes only the speed or snap, no jump;
   - the snap accumulator is zeroed on begin and on end.
4. **Lifecycle:**
   - enterEdit, exitEdit, member switch, scope change, Done, Back, Esc, tick gap: all reset chord state and the scrub capture;
   - the discard guard returns early, so a pending scrub is cancelled.
5. **Scrub:**
   - press on a row then drag outside it: the capture holds until release;
   - values pinned at the window edge and at the parser limits;
   - the per-row reset after a scope switch uses the new open value;
   - a left mirrored row shows the mirrored value; the first scrub converts it to explicit `l_` without the extra tilt.
6. **Euler:**
   - ry 79.9 / 80.0 / 85 / 89.9 / 90 / 90.1 with a mismatched prev;
   - a drag through the band: render error < 0.01° and display step < 10°;
   - SetValues(450) is shown as 90 (document it).
7. **Projection:**
   - depth ≤ 0 means hidden;
   - a foreshortened axis is hidden;
   - zoom changes fovy;
   - pane resize or ultrawide means re-read PaneRect(VIEW) every frame;
   - yaw 180 needs no sign change (the camera is fixed).
8. **Names:**
   - an empty hand, `i_null`, an item with no name, a 40+ character name, a non-English client language: fallback to the resref; text fitted with "...".
9. **Mirror:**
   - heavy (x=0 exact overlap) needs the extra tilt;
   - an explicit `l_` key overrides the mirror per field;
   - the extra tilt is never applied twice (load → save → load round trip);
   - with no global `l_` key, global stays mirror-only.
10. **Cap:**
    - 40 items; vertical fit on vanilla 800×600 and on ultrawide;
    - AddX returns −1 when full, and the editor degrades (no inspector) instead of crashing.

## 5. Test coverage (host, colocated)
- `holster_src/holster_edit_test.cpp`:
  - the H8 vectors (90.1 input, 85 with mismatched prev, 79.9 vs 80.0, the 40-step drag path);
  - the worldDeltaToLocal non-identity bone case;
  - the wrap180 ±180 case.
- `holster_src/holster_tune_test.cpp`:
  - the mirror vectors for rifle, melee and heavy (H4 table);
  - `left_extra_rot` −8° → euler −59.57, −11.99, −93.06 (±0.05°);
  - explicit `l_` overrides only its field;
  - the save round trip does not double the extra.
- `holster_editor_src/hse_view_test.cpp`:
  - `Chord` table (≥ 14 cases from §4.1-4.4);
  - `worldToScreen` (centre, edge, behind, foreshortened, aspect 21:9);
  - nearest axis;
  - scrub delta, window and limits;
  - label fit and fallback;
  - scope label per type;
  - stripSlot row 8 bottom (H7: untested today).
- `mod_config_src/menu_ui_logic_test.cpp`: kMaxItems 40, full-page refusal at 40.
- `common_contract_test.cpp`: pin `kMaxItems`-independent behaviour; HolsterApi slots unchanged in U1 to U4.
- Gates per wave:
  - `run_tests.py hse_view holster common_contract menu_ui mod_config`;
  - `build_check.py`;
  - `check_raw_addresses.py` (U0 name probe);
  - `compat_check.py` on touched shared headers.

## 6. Play rows (add when each wave ships)
- **HUX-P0:** camera and name probe (U0).
- **HW-LH1:** already pending; the user edits the ini first.
- **HUX-U1:** a drag through the 80-90° tilt has no visible snap; Esc then Esc lands on the main Options page.
- **HUX-U2:** chord gestures, the mode bar, RMB+wheel depth, names fit, the scope tooltip.
- **HUX-U3:** scrub rows, reset, mirrored left status, the criss-cross.
- **HW-SBR1, HW-PV2, HW-BD1:** saber blade out of combat, armour bulk in the preview, the N_CommF body delta (H9).

## 7. Risks
| Risk | Mitigation |
|---|---|
| The charrec_light camerahook axes differ from inv3d | U0 probe before U4; U2 Move uses fixed axes (same as today) |
| The client-item name path is wrong | U0 probe; resref fallback; no shipping on an unverified offset |
| 40 items do not fit vertically on vanilla | Park the inspector in nav; measure in HUX-U2 |
| SetRect per frame trips the gaveUp latch | Markers move only when the projected point moves > 1 px |
| eulerNear change alters saved values | Saved values differ only inside the band, where they were wrong; round-trip tests |
