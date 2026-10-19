# Handoff: holster config shared components, UX fixes, body-rotation plan (session holster-toggle-key-delay, 10-18 night)

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
