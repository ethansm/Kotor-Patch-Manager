# Plan HW-UX3 build: holster editor layout v3 + combat draw (HW7) + camera probe (CAM2)

## 0. Read this first (self-contained)
- Repo: `/home/ethansm/orca/workspaces/local-setup/KOTOR-II` (worktree, branch KOTOR-II; other sessions share it: commit ONLY explicit paths with `git commit -- <paths>`, never `git add -A`, never bare `git stash`).
- Layout design doc: `.claude/plans/holster_editor_layout_v3.md` (APPROVED "approve HW-UX3"; §3 page sketch, §4b research, §5 camera). Handoff: `.claude/handoffs/holster_editor_ux_v2.handoff.md` (last section "10-18 later"). Research: `research/holstered_weapons/ux_v3/*.txt`. **`OPUS_review.txt` is the independent review; trust it over A1/F/G1/E1** (camera ctor 0x47ECD0 DOES zero +0x218..0x224; flourish needs its own window, not +0x138; "un-repoint around the flourish" is WRONG).
- Progress is tracked by the `Status:` lines in §6 (plan_manager.py does not parse these headings).
- Live NOW: holster 0.1.5-dev, holster-editor 0.3.2-dev, mod-config 0.1.2-dev. Repo is ahead, NOT installed: holster 0.1.7-dev, mod-config 0.1.3-dev (kMaxItems 80 + SetDescSplit), holster-editor 0.3.2-dev sources + L0 helpers.
- Last commits: e93904c8 (review fixes), 041aab12 (handoff). L0 helpers built and host-tested: `holster_editor_src/{HseLayoutV3.h,ResetIcon.h,hse_layout_v3_test.cpp,reset_icon_test.cpp}`, `_shared_src/{ResetIconTex.inc,GuiKit.h (setButtonFill/setButtonFillStyle/buttonFillOk)}`, `scripts/gen_reset_icon.py`, `holster_src/HolsterLogRate.h`.

## 1. Context (why)
User play feedback 10-18: editor page far too busy; default zoom shows only the hip; view too small; description box too tall; values not shown; sliders need a marker with text OUTSIDE (vanilla Sound Options look: tapered teal wedge in a rounded frame, thin white double-arrow marker at the value, title above, nothing inside); reset buttons have no icon; no cycling `< >` inputs. Crossed swords already PASS (HW-LH2). NEW bug: weapons stay on the hip IN combat, and Flourish out of combat spins empty hands then leaves a combat stance. Cause: `bone_test=1` repoints the engine's rhand/lhand node name to the hip globally (imm32 at 0x763AF9/0x763E2B via `code_patches.toml`); "draw in combat" (master plan HW7, `patch_manager_mods/42_holstered_weapons_master_plan.md`) was never built; `hs::Fsm` (HolsterLogic.h:174) is unwired. The camera model behind viewport markers (U4) is unexplained.

## 2. Execution strategy (user's rule; every wave)
1. Parallel Haiku (`model: haiku`) swarms for research, investigation, validation, test running. Briefs cap work (~15 tool calls, `rg -n` + ranges <= 120 lines, one question per agent, exact file:line anchors); "stay under 80k" in prose does NOT hold. Recon contract = `python3 scripts/agent_contract.py print recon`, output under `research/holstered_weapons/ux_v3/`.
2. Sonnet (`model: sonnet`) workers write code (Contract: worker, disjoint Write Scope, `agent_contract.py snapshot` before launch, `check --scope` after). Never `--sync`, `--prepare`, gate or install.
3. Escalate to Sonnet when two Haikus disagree or one flags something complex, before Opus steps in.
4. Orchestrator briefs, runs gates, commits, installs (after user GO), reviews diffs only on FAIL/ESCALATE or a gate failing twice.
5. Independent Opus reviewer at the end of each build wave (before any install): fresh context, given claims and code not conclusions; output `OPUS_review_<wave>.txt`; fix blocking issues, then install.
6. After each wave: update handoff, memory `kotor2-holstered-weapons.md` (+ MEMORY.md), `PLAY_TEST_QUEUE.md` rows, then the 4-section end-of-step output and exact resume command.

## 3. Waves

### Wave A: CAM2 probe + page-rebuild prerequisites (parallel; no behaviour change)
Haiku validation swarm first (3 read-only), then Sonnet workers:
- **A0 (Haiku x3):** (a) read `HseLayoutV3.h`, check contract vs `HolsterEditor.cpp` pane rects (`PaneRect` MUI_PANE_COLUMN/VIEW/PREVIEW at ~1759-1786); (b) check mod-config SetDescSplit path (`mod_config_src/ModConfig.cpp`, `ModConfigApi.h:117-120`: permille clamp 600..850; `MenuClient::hasDescSplit()`); (c) check kMaxReapply=8 against per-frame SetRect of 69 items. Disagreements go to a Sonnet.
- **A1 (Sonnet) CAM2 probe** in `HolsterEditor.cpp` (`logCamEng` 797-987), log-only: at draw time/first settled tick log `glGetIntegerv(GL_VIEWPORT)` (guarded, render thread/tick only), scene sub-object rect S+4..S+0x10 vs control rect, parent-stack depth/entry used by 0x479460 (DAT_00a309c0, DAT_00a309c8/ca, DAT_00a309ce), `DAT_00a32a54`, untruncated CALIB node lists. Goal: explain 1.0 m seen vs ~1.7 m predicted (pose pos=(4.8725,0,0.4358), q(wxyz)=(0.4786,0.5210,0.5210,0.4775), fovy=22.73/zoom deg vertical, aspect w/h). Addresses via `python3 scripts/query_ghidra.py address <hex>` first.
- **A2 (Sonnet) slider art:** `scripts/gen_slider_art.py` -> `_shared_src/SliderArtTex.inc` + `holster_editor_src/SliderArt.h` (+ `slider_art_test.cpp`, register in `holster_editor_src/test.toml`): `hse_sld_frame` (rounded teal frame), `hse_sld_wedge` (tapered teal wedge; HudKit kAccent {10,96,80} and teal of `GearIconTex.inc`), `hse_sld_mark` (thin white double-arrow). Drawn fresh; skin plain labels with `guikit::setFill`.
- A3 log noise folded into Wave B.
- Gate: `python3 scripts/run_tests.py hse_layout_v3 reset_icon slider_art guikit hse_view` and `python3 scripts/build_check.py --mod holster_editor_src`.

### Wave B: page rebuild (holster-editor 0.4.0-dev; ONE Sonnet owns HolsterEditor.cpp)
Spec = `holster_editor_layout_v3.md` §3 + review gaps. Anchors as of HEAD; re-`rg` first.
1. Always-on edit, no NAV mode: page opens editing the right weapon (`enterEdit` ~1699). Remove MODE_NAV branch of `cbTick` (2049-2102) and Done button; Back leaves. Keep `guardTransition` (1815) for hand, bone, scope, member prev/next.
2. Direct buttons: hand (Right/Left), 5 bones (Upper back=torsoUpr_g, Back=torso_g, Pelvis=pelvis_g, R hip=rthigh_g, L hip=lthigh_g; no absolute bone setter in HolsterApi, so click loops `CycleBone` (~2119) until `GetEdit` reports that bone, max 5), 3 scopes, 5 stances (Idle pause1, Walk walk, Run run, Alert pause2, Combat pause3; drop rifle and emote cycler/Play), member `◀ ▶` stays. Active button lit via text colour + `guikit::setBorderTint` / `setButtonFillStyle`.
3. Layout: `hse::pageLayoutV3(column, view, preview, hint, thumbFrac)` (`kPageItemsV3 = 69`) replaces `applyLayout` (1381-1470); `SetDescSplit(page, 850)` (check `hasDescSplit()`); inspector in `MUI_PANE_PREVIEW`. View widens left over old column band (~930 px at 3440x1440), right to `preview.x - 4`.
4. Inspector sliders: name label, track = `hse_sld_frame` + `hse_sld_wedge`, marker `hse_sld_mark` via SetRect from `thumbFrac`, value label OUTSIDE right ("-11.7 cm", amber when non-default), reset icon button (`ResetIcon.h`: `guikit::setButtonFill` then `guikit::setFill` label fallback; `resetIconApply`). Reuse scrub maths (`HseView.h` Scrub 921, `scrubTick` 1653, `inspReset` 1673, `refreshInspector` 1518). Move marker only if shifted >1 px.
5. Fit zoom: default `view_zoom` 0.6 (guess; tune in play), `kZoomMin` 0.8 -> 0.4 (`HseView.h:17`), `clampZoom`/tests updated; Reset view returns to it; ini keeps `view_zoom`.
6. Consumer caps: `HseView.h:793-794` `kPageItemCap` 40 -> 80 + static_assert, `hse_view_test.cpp:980` (`KT_CHECK_EQ(kPageItemCap, 40)`), `common_contract_test.cpp:226-227`. Editor refuses cleanly (log + "needs newer mod-config") if `AddX` returns -1 / provider cap < `kPageItemsV3`.
7. Stale caches: `viewRect`, `strip`, `statusRect`, wheel hit rect (`guikit::g_wheel.setRect` ~1780) captured only in `cbEnter` (1759-1786), read at 1192, 1422, 1510, 651; refresh after view resize.
8. Log noise: keep per-edit lines; remove `CAMRAW` (`logCamEng` 859-870); downgrade `CHORD` transitions (1895-1919), `INSP refresh` (1522), repeated `PROJ` to debug behind ini key `log_verbose=0`; keep WARN/ERR, REG, ENTER, LEAVE, BUILD, DISCARD, CAM2. Cap rules stay.
9. Press exclusion: a press starting on any button must not start a view drag; thumb/track presses go to scrubbing.
10. Versions: holster-editor `0.4.0-dev` in `manifest.toml`, banner, status text, ini header. Update `PLAY_TEST_QUEUE.md`.
- Haiku test swarm after worker: (a) `run_tests.py hse_view hse_layout_v3 reset_icon slider_art guikit common_contract` + `build_check.py --mod holster_editor_src`; (b) static checker: every `Add*` has an `Editor` field and every field placed in `pageLayoutV3` (count == `kPageItemsV3`, <= 80); (c) log checker via `scripts/kotor2_logs.py check`. Failures -> Sonnet fixer -> Opus reviewer.

### Wave C: HW7 combat draw + flourish (holster)
Keep native rhand/lhand attach (`bone_test=0` equivalent) and move the hip pose into the per-frame placement write:
- **C1 (DLL-only, holster 0.2.0-dev):** new ini mode (`place_mode=drawgate`) in which the rhand/lhand repoint is NOT applied (add a key so `g_boneTest`-gated code at Holster.cpp:781 still runs). In `HolsterPlace` (Holster.cpp:774, hook H7 0x504EF5): accept node when name is rhand/lhand (today `nodeIsTarget` at 476 matches only `g_boneName`); `owner = [parent+0x1D8]` (~837); if `[owner+0x138] != 0` or flourish window open, return early (weapon stays in hand pose); else write hip pose = `mountPose` of configured bone. Always compute `mountPose` in this mode (today skipped when `mountBone == g_boneName`, ~864). Existing place_* tuning valid only if `mountPose(pelvis_g)` equals old pelvis-node base: assert in host test, verify in play. Failure mode must be "weapon in hand" (vanilla), never stuck on hip. Add owner + drawn state to cache entry/key (`Ent` at 791) so a state flip invalidates (`ent->seq = -1`). Per-creature drawn tracker; reuse `hs::Fsm` only if creature-keyed. Combat exit is an 8 s AI timer; drawn -> hip is an instant snap (v1).
- **C2 (hooks.toml change = full reinstall + user GO, holster 0.2.1-dev):** flourish window. Timed "drawn" window (~flourish clip length, closed on `0x57A560` param3 != 2 or timeout) from detour of `0x7408A0` (player flourish, single caller of `0x79F430`) and/or `0x66B8A0` (script path), or `0x57A560` filtered on return address `0x79F621`/`0x66BAC2` with param3 == 2. New `[[hooks]]` rows need exact `original_bytes`, an `extern "C"` handler, `exports.def` line; read `BUILD_AND_INSTALL.md` §4; `kpm_reinstall.py`, never hot-swap. C2 only after C1 passes in play.
- Haiku research before C1 (parallel x4): (a) exact original bytes and caller of 0x7408A0/0x66B8A0/0x57A560; (b) `[parent+0x1D8]` is the creature across PC/party/NPC from COMB/PLACENODE logs; (c) left-hand mirror (`effLeft`, `left_extra_rot`) vs hand-node base; (d) every `g_boneTest`/`g_boneName` use in Holster.cpp the new mode must honour. Sonnet escalation on disagreement. Extract the drawn-gate decision into a pure function with host tests (nothing covers `HolsterPlace`/`nodeIsTarget`/`mountPose` today).
- Play rows: HW7a (combat draw PC and party, back to hip ~8 s after), HW7b (flourish: draw, flourish, re-holster; no stuck stance), HW7c (left X, save/load, area change, swap weapons).

### Wave D: install and play (needs user GO; game closed)
`python3 scripts/check_shared_headers.py --sync` -> `python3 scripts/kpm_status.py` -> commit sources -> `python3 scripts/kpm_hotswap.py --mod mod-config holster holster-editor --prepare` (expect SWAP x3; REINSTALL = stop) -> fresh `scripts/pre_install_gate.sh --mod mod_config_src --mod holster_src --mod holster_editor_src` (`GATE PASS`; no `--skip-default`) -> ask GO -> `kpm_hotswap.py --mod mod-config holster holster-editor --apply` -> `kpm_status.py` LIVE. Rollback: `kpm_hotswap.py --rollback latest`. Commit only own paths. A/B ship together (C1 same batch if its Opus review passes; C2 separate reinstall).
- Play rows (`| id | source | mod@version | install | steps | pass signal | status | added |`, append): **UX3**, **UX3b**, **CAM2**, **LOG1**, **ICON1**. Pending still: HUX-U1, HUX-U2, HUX-U3, HUX-C0, ESC2; CAM1 row status stale (played).

### Wave E: viewport markers U4 (after CAM2 explains projection)
Fix `worldToScreen` (HseView.h), marker pool per `research/holstered_weapons/ux_v2/u4/F_edge_cases_tests.md` and HW-UX2 plan §3 U4. Deferred: bone-node toggle, GL overlay, animation freeze, typed numeric input.

## 4. Edge cases and tests
- Layout: 1280x720, 1920x1080, 3440x1440; preview overlapping view; NaN/null thumbs; tiny panes; items <= 80.
- Editor: refuse cleanly against old 40-cap mod-config; unsaved-edit guard; bone loop terminates; Esc/Back `DISCARD`; SetRect churn within kMaxReapply; reset icon fallback order.
- Holster: drawn-gate table (combat 0/1 x window open/closed x owner unreadable -> hand); owner change invalidates cache; area change/save-load; party/NPC; left mirror; PLACEREFUSE never hides a WARN; COMB edges always logged.
- Logging: `log_verbose=0` default; WARN/ERR never rate-limited; `holster_log_check.py` and `holster_place_verdict.py` still parse.

## 5. Verification commands
`python3 scripts/run_tests.py -k holster_` ; `-k hse_view` ; `-k hse_layout` ; `-k reset_icon` ; `-k slider_art` ; `-k guikit` ; `-k menu_ui_logic` | `python3 scripts/build_check.py --mod holster_src --mod holster_editor_src --mod mod_config_src` | `python3 scripts/check_raw_addresses.py` | `python3 scripts/compat_check.py patch_manager_mods/_shared_src/GuiKit.h` | `python3 scripts/agent_diff_check.py --scope <globs>` | logs only via `python3 scripts/kotor2_logs.py check <log> --new`.

## 6. Status tracker
- Wave A (CAM2 probe, slider art, A0 validation): TODO
- Wave B (page rebuild 0.4.0-dev + Opus review): TODO
- Wave C1 (combat draw, DLL-only) + C2 (flourish hooks, reinstall): TODO (research swarm first)
- Wave D (sync, prepare, gate, GO, apply, play rows): TODO
- Wave E (U4 markers): BLOCKED on CAM2

## 7. Risks
| Risk | Mitigation |
|---|---|
| Widened view band hosts vanilla controls that take clicks | UX3b play row; fallback keeps old width, grows height only |
| Button fill (+0x70 border layout) not drawn | `resetIconApply` label fallback; ICON1 |
| 80 items vs engine control list growth (unverified) | mod-config first; refuse cleanly; UX3b |
| Drawn-gate leaves weapon stuck on hip or in hand | owner read failure = hand; pure-function tests; HW7 rows |
| Flourish detours (C2) need reinstall | isolated wave; GO and rollback first |
| Camera cause unexplained | CAM2 logs decisive fields; U4 stays blocked |
| Foreign uncommitted files make kpm_status DIRTY | commit own paths only |
