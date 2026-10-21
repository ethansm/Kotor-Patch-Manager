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

## State at 10-20 21:45 (editor 0.6.0-dev + preview-3d 0.2.0-dev LIVE)
LIVE: holster-editor 0.6.0-dev, preview-3d 0.2.0-dev (rollback `kpm_hotswap.py --rollback batch_swap_20261020_213934`), holster 0.5.2-dev. Added: Prev/Next word buttons + framed amber name plate (b0ac359d), Pause/Play button (4afd044c, page 80/80 items, NO spare item left), preview-3d SetAnimPaused slot 12 (bbc67f97, optional via Client::canPause; same-clip replay at speed 0 keeps the playback position, engine speed field is a FLOAT so slow motion would be possible without a hook if ever wanted). Play row UX5. KPM bdee024, b9d9fa0. Note: I ran check_shared_headers --sync, which left other sessions' newer headers (GuiKit.h, ModConfigDecl.h, ...) uncommitted in KPM Patches/Common.

## State at 10-20 ~22:30 (IN FLIGHT: 3 workers; context persisted for compact)
LIVE now: holster 0.5.2-dev, holster-editor 0.6.0-dev, preview-3d 0.2.0-dev (rollback batch_swap_20261020_213934). HEAD 961e2107 at the time of writing (check `git log -1`).

### Two new play findings (from the user, with log diagnosis from holster_log via kplog_parse, no raw reads)
1. "Flourish while standing still, then start moving: weapons attach to hands, after the time a flourish would take they holster. Should stay in the flourish animation while moving." Cause: ~140 ms after FLOURISH open the engine sets the client combat flag (COMB new=1), then runDelay cuts server +0x524 = 0 at 1500 ms (`DELAY ... kind=combat`) because no hostile within hostile_range (30), so the flag drops and the weapons stow. Fix designed: no DELAY cut while the creature is moving (loco anim ids 0x2712/0x2713/0x2714 resolved every frame in HolsterAnimResolve, recorded in a MoveTable) plus `move_hold_ms` (default 1500) after it stops.
2. "In combat mode, after not attacking for a few seconds, weapons go holstered." Cause: client combat flag [owner+0x138] is PER creature; the PC's flag rose and fell within 286 ms while companions stayed in combat, PC weapons stowed mid-fight. Fix designed: party-wide hold, key `party_combat_hold` (default 1): while ANY party member's combat flag is set, all party members' weapons stay drawn. NOT done (option if still wrong in play): hostile_range default 30 -> larger, combat_exit_ms longer (companions' cut fires when no hostile within 30 m).
3. UI: user wants Pause/Play as its OWN layer: Pause full-width bar under the 3D view, then Idle/Walk/Run, then Alert/Combat (3 layers). View height shrinks by one bar row. (Always shown; the user's "only when active" wording was ambiguous: I chose always-visible, mention it.)

### Workers launched at ~22:25 (all Sonnet, disjoint files, results arrive as task notifications; if the session was cleared they may be stopped: check `git status`, scratch dirs, and re-run the worker from the brief below)
- W1 (agent acfd69016926e0823) scope holster_src/{Holster.cpp,HolsterMove.h(new),holster_move_test.cpp(new),HolsterDrawGate.h,holster_drawgate_test.cpp,holster.ini,manifest.toml}: holster 0.6.0-dev: MoveTable/isLocoAnim/anyCombat pure header + tests + mutation, g_move noted at the TOP of HolsterAnimResolve, `move_hold_ms`, `party_combat_hold` keys, runDelay skip while moving (MOVEHOLD log), g_partyCombat computed in holsterTick (PARTYHOLD on/off log), hip gate `|| (g_partyCombatHold && g_partyCombat && ownerIsParty(owner))`, early-return widened. Scratch .claude/waves/hr5/w1.
- W1b (agent a1eed324d1bbe0a34) scope scripts/{holster_log_check.py,test_holster_log_check.py}: PARTYHOLD on/off, MOVEHOLD total, DELAY cuts counters + planted-positive tests.
- W2 (agent a20f7176138bdd64c) scope holster_editor_src/{HseLayoutV3.h,HolsterEditor.cpp,hse_layout_v3_test.cpp,hse_view_test.cpp,manifest.toml} + scripts/{holster_editor_mockup.py,test_holster_editor_mockup.py}: holster-editor 0.6.1-dev: Pause full-width layer above Idle/Walk/Run above Alert/Combat; PNG .claude/waves/hr5/w2/editor.png.

### After the workers return (Opus steps)
1. Read each diff (agent_contract check scopes), view the W2 PNG. Run `python3 scripts/check_raw_addresses.py` (the gate fails on raw hex addresses in comments: cite dump file names instead).
2. Commit explicit paths (3 commits). Optionally a Sonnet review of W1 (movement/party-hold logic) before install.
3. `python3 scripts/check_shared_headers.py` (0 drift; DO NOT run --sync; I ran it once at 21:30 and it dumped other sessions' newer headers into KPM Patches/Common uncommitted, left for their owners), `scripts/pre_install_gate.sh --mod holster_src --mod holster_editor_src` (rerun once if check-shared-headers drifts transiently), `python3 scripts/kpm_hotswap.py --mod holster --mod holster-editor --prepare` (both must say SWAP), commit the KPM side with explicit paths, ask for GO with the game closed (`pgrep -a swkotor2`), `--apply`, play rows to PLAY_TEST_QUEUE.md via the index-blob trick (git show HEAD:file + row, `git update-index --cacheinfo`, `git commit` with a clean index, then append the same row to the working copy), handoff, mark.
4. Play rows to add: FL5 (flourish, then walk/run: weapons stay drawn while moving and ~1.5 s after, `MOVEHOLD` lines, no `DELAY cut` while moving), PC1 (party fight where the PC stops attacking: PC weapons stay drawn while a companion is in combat, `PARTYHOLD on/off`), UX6 (three-layer stance bars, Pause on top).

### Still pending from before
User play rows: FL1-FL4, SL-DRAW/REENTRY/EDGE, SW1/SW2, HW7a-P, UX4, UX5, BR1-3, SC1, RS1, HK1, HD1-3. HW16 (unarmed override, REINSTALL) only if HW7a-P shows gate=1 with style {2,3,4,7,9} on 2712/2714. Page item budget: holster-editor is at 80/80 (no spare). Plan doc: patch_manager_mods/42b_holster_remaining_plan.md; master rows HW12-HW17 INSTALLED. Resume: python3 scripts/plan_manager.py resume "42:HW12".

## State at 10-20 ~22:15 (INSTALLED)
holster 0.6.0-dev (move_hold_ms, party_combat_hold) + holster-editor 0.6.1-dev (three-layer stance bars) LIVE via batch_swap_20261020_221335 (repo 45e26618/cac3a056, KPM 7a01fc7/a3cff04). Play rows FL5, PC1, UX6 added (1b006fdf). Open: Pause "only when active" interpretation (kept always visible); tune hostile_range/combat_exit_ms if PC1 fails; HW16 only if HW7a-P shows gate=1.
