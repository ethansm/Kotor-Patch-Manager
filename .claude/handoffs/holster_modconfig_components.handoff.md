# Handoff: holster config shared components, UX fixes, body-rotation plan (session holster-toggle-key-delay, 10-18 night)

## State at 10-19 00:30 (body rotation plan DONE)
LIVE: holster 0.4.0-dev + holster-editor 0.5.0-dev (one batch; rollback `kpm_hotswap.py --rollback batch_swap_20261019_002931`). Repo 7e17b5ec, play rows 59f4ad2d (BR1-BR3, SC1, RS1; DB2 part b is superseded: no rotation lock any more). KPM 6ac94b1, 667806a.
- Provider: place_rot_body_<name> (q = qbase(D) * qbase(R), left mirrorRot, left_extra outermost, src[0] bit 16 = K2HOLSTER_SRC_BODYROT); weapon/type saves never bake the body rot (overlayStoreLevel stores conj(bodyQ) * placeQ); ResetScope body unsets both keys. sizeof(TuneCfg) 3924 < 4096.
- Editor: labels "This weapon model" | "All <plural>" | "This body + armor"; live "Save changes go to ..." sentence; armorClassLabel from the body resref 4th char (vanilla baseitems verified, A = appearance default only); "..." toggle (item 69) + PI_REMOVE item 72 (page 79/80, ONE spare); HseLayout.h + 12 HseView helpers pruned, probes kept.
- Gate waiver (10-19): default tier failed on k1_port (another session's uncommitted k1_port_recipe.py body_b/head rule) and test_subagent_tools; both PASS on a clean HEAD copy with this patch applied; GATE PASS with --skip-default (mod steps + builds); holster/hse/view_drag/mockup/providerclient/iniwrite suites pass.
- 10-19 later: shared-helper prune ON HOLD by owner decision (relayed by the doc 46 session): keep EditKit.h/SliderArt.h/ResetIcon.h helpers until MCF5 (slider row, doc 46 session builds it) is installed and played; then only still-unused ones are candidates (list: midClick, fmtValue, sliderArtTextures/Resref/Apply, sliderMarkLeft, sliderClamp01, resetIconTextures/Count/Resref, arrowIconResref/Index, flipIconResref; verify dragMode + sliderWedgeWidth first). Installed 9226ec1a (View=p3d::ViewState, inspTrackValue via editkit, trimIni via inikit; rollback batch_swap_20261019_012025). HolsterIniWrite.h stays (iniw not equivalent).
- Left open: KPM Patches/Common + Patches/_shared HolsterApi.h are stale (comment + unused #define only; next check_shared_headers --sync outside an install); KPM still holds the deleted Patches/HolsterEditor/HseLayout.h (unused); dead accessors in shared EditKit.h (dragMode/midClick/fmtValue), SliderArt.h (7), ResetIcon.h (6) need the doc 46 session's OK before pruning; 1920x1080 / 1280x720 scope labels overflow (did before too; needs a layout change, not text).

## State at 10-18 23:40
LIVE: holster 0.3.0-dev (F8 toggle key + shorter combat/flourish exit; rollback holster_swap_20261018_221317), holster-editor 0.4.11-dev (rollback holster-editor_swap_20261018_233719).
Game closed at install time. Nothing of the new work is play-tested; play rows pending in patch_manager_mods/PLAY_TEST_QUEUE.md: HK1, HD1-HD3 (holster 0.3.0), DB1, DB2 (editor boxes + body-scope lock).

## Commits (this session)
dfdc343e editor crash fix (SetFont on label) 0.4.8-dev | 72880d39/16bb3c62 holster toggle key + delay 0.3.0-dev | 4ed751c0 play rows HK1/HD*
8ca6daab, 394d7880, 7777a497 research/mod_config_components (R1-R4, S1 component map, S2 reconcile with doc 46)
9e329f0e HseParty.h/SliderArt.h/ResetIcon.h moved to _shared_src (namespace hse) | 2e2f3b88 IniWrite.h (iniw::setKey/removeKey) + ProviderClient.h (k2pc::ClientSet, registerOnce)
71c23324 desc boxes bottom aligned, own right box (0.4.9) | 1925761a ProviderClient adoption (0.4.10) | 6dcbee15 contract test for 19-slot K2MenuUiApi
48eaf2ce cover LB_DESC frame (desc_cover_px), body-scope rotation lock (0.4.11) | d0703427 DB1 | fafd87a1 DB2

## Other session (doc 46, patch_manager_mods/46_mod_config_declarative_layer.md)
Names seen: combat-state-logging-queue-hud, then mod-config-selfheal-options-page. Owns mod_config_src, ModConfigApi.h (19 slots), MenuUiLogic.h, TomlMini.h, ModConfigDecl.h, Self-Heal page. Reported MCF1-MCF4, MCF2b built and committed, NOT installed/played (a1c80c88, 3b4aa96b, e9a17f47, 2a2e53d1, b6e30a50). mod-config shows DIRTY in kpm_status. They said they will not edit holster_editor_src and will announce before editing GuiKit.h.
Split agreed: WE do holster-only extractions and holster_editor_src/holster_src; THEY do provider, declarative layer, Self-Heal, strip. Open: who builds the look-alike slider (their MCF5 vs our CMP8), wheel ownership on the holster page.

## Findings that matter
- Tilt/Roll-Turn "not working" = scope "This body" is OFFSET ONLY by design (HolsterApi.h:23, Holster.cpp body delta in HolsterTune.h resolveFull). All 11 Tilt/Roll gestures in the log were scope=2. Editor now dims them in body scope.
- "Body" key = body model resref of the Gob parent ([parent+0x84]+0x08), e.g. PFBCM; changes with armor class (model), not with texture. No written rationale for offset-only found in doc 42.
- Right description box is our own framed label over LB_DESC (list box scrollbar cannot be hidden from the consumer); needs frame cover margin (desc_cover_px, default 6, play check DB2).
- holster-editor connects to mod-config once at KPatchInit (hookless); fine because manifest requires mod-config; retry would need a frame hook.
- queue-hud ini reader 4095-byte cut found by us, owned by the other session (QL18).
- Gate can fail transiently on check-shared-headers when another session edits _shared_src during the gate; rerun once after check_shared_headers is 0 drift.

## Open decisions (owner)
1. Add rotation nudge for "This body" (and later an armor-class level)? Plan requested next.
2. UX: separate "Fit for this character" section vs third scope button; dynamic "Save changes: ..." sentence; scope label wording.
3. Dead-code prune and the ~500 lines of probes in holster-editor: remove, gate, keep.
4. Slider owner (MCF5 vs CMP8); GuiKit additive ops; IconButton/Radio helpers.
5. RFC drafts (new_issue_drafts/, untracked, never posted) need doc 46 results folded in.

## Resume
Read this file, research/mod_config_components/S2_reconcile_doc46.txt, doc 46. Plan file: ~/.claude/plans/ (body rotation + UX). Never post to GitHub without explicit approval.

## State at 10-20 (remaining-work plan written)
Plan `patch_manager_mods/42b_holster_remaining_plan.md` (master rows HW12-HW17): flourish window, slide (Slide replaces the unwired Fsm), unarmed probe then gated override, weapon-swap guard. Evidence: `research/holstered_weapons/hr1/R1..R5_review.md` (12 Haiku + 5 Sonnet swarm). Nothing built yet. Next: wave A (4 disjoint Sonnet workers), see plan sec 3. Unarmed scope shrank: holstered idle is already unarmed (R3).

## State at 10-20 19:35 (doc 42b waves A-D INSTALLED)
LIVE: holster 0.5.2-dev (rollback `kpm_hotswap.py --rollback holster_swap_20261020_193331`), holster-editor 0.5.0-dev. Commits 240f320d (A guards/Slide/AnimStyle/log-check), b16085e8 (B flourish window), baed1b86 (C slide + Fsm retired), f389c1cd (D unarmed probe), c8a6eac1 (play rows). KPM 2f85e8d. Gate PASS (378 tests). Reviews: B APPROVE, C APPROVE_MINOR (nits: flicker-unreadable gives a cosmetic stow slide; 60-line SLIDE log budget drops end lines; HolsterLogic.h:2 and docs still mention Fsm; FLOURISH s_open goes stale when a slot leaves the party).
Pending (user plays): FL1-FL4, SL-DRAW/REENTRY/EDGE, SW1/SW2, HW7a-P (+ older BR1-3, SC1, RS1, HK1, HD1-3). After play: `holster_log_check.py` prints GOBREUSE/FLOURISH/SLIDE counters. HW16 (unarmed override, REINSTALL) only if HW7a-P shows gate=1 + style {2,3,4,7,9} on 2712/2714.
Next: read play results; flip D1/D2/D3 defaults only on evidence; optional nit fixes.

## State at 10-20 21:05 (editor 0.5.1-dev LIVE)
holster-editor 0.5.1-dev LIVE (rollback `kpm_hotswap.py --rollback holster-editor_swap_20261020_210347`): section titles full-height (WEAPON/BONE/APPLIES TO were 28 px boxes that drew nothing; cause unproven, CAPBOX log line confirms), inspector rows up to 70, English item/armor names in the save/remove sentences, kV3ViewGap 16 px between view bars and the right description box. Commits 46e53288, e2b8d6b7; KPM 1f95e8c, d27d646. Play row UX4. holster 0.5.2-dev still LIVE (play rows FL*/SL-*/SW*/HW7a-P pending).
