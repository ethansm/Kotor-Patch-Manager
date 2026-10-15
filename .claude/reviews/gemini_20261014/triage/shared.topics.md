# shared batch triage (prefix SH): sets shared_lib_a (3a, 3b), shared_lib_b (3c)

26 findings, 21 topics, 0 dropped. Gemini rated every finding "certain" (unverified).

## Topics
- SH-T01 P1: GuiKit `wr<T>` only checks readability before writing (no probeWrite).
- SH-T02 P2: `saferead::readAt` base+off wrap on 32-bit bypasses low-address probe.
- SH-T03 P1: `codepatch::writeChecked` memcpy reads target before commit/protect check; n<=0 unguarded.
- SH-T04 P2: `imageFor` pools null results so `hudkit::image` retry never reloads; hudkit_test misses ensure().
- SH-T05 P2: `embtex::heal` logs give-up on 20th refill even if fill succeeded; embtex_test only counts the line.
- SH-T06 P2: label memory leak on createLabelFromTemplate / bindByTag failure paths.
- SH-T07 P2: guikitWheelCb passes engine screen size where client rect size is expected.
- SH-T08 P2: stale HWND in WindowHook/wheelInstall; `subOk` never reset; k2input::Client no reportedMissing guard.
- SH-T09 P2: EffectIcons.h ignores K2AddrTable offsets, unbounded `seen[]`; checkEffects zero-offset assertion vacuous.
- SH-T10 P2: Party::vitals hardcoded vtable slots; party_test never runs real callVitalSlot.
- SH-T11 P3: guikit_test misses GuiKit setters/getters success paths.
- SH-T12 P3: k2addr_test field-list omissions (Border_FillStyle, SceneView) and duplicated Menu_Char checks.
- SH-T13 P3: effects_test lacks ii_/iw_ and null-resref icon rows.
- SH-T14 P3: pendingDone2 now==0 sentinel untested.
- SH-T15 P3: sceneview_test never exercises default call lambdas.
- SH-T16 P3: gui2d_test never exercises K2AddrTable-backed defaults or null clipNoFill guard.
- SH-T17 P3: actq::playerPaused untested; truncation bounds and rn spellId unasserted.
- SH-T18 P3: filewatch_test never uses Stamp* out; mtimeMs check conditional.
- SH-T19 P3: guitip_test missing unreadable-mgr and null-out measure cases.
- SH-T20 P3: item_read_test missing unreadable-entry, count bounds, 16-byte icon cases.
- SH-T21 P3: textfit_test missing space-trim, off-on-space, negative off, long-word cases.

## Duplicates (within batch) and how folded
- SH-3a-05 and SH-3c-01 (imageFor negative caching): one topic SH-T04.
- SH-3a-06 and SH-3c-03 (heal give-up log timing): one topic SH-T05.
- SH-3a-11 and SH-3c-08 (hardcoded Party HP slots): one topic SH-T10.
- SH-3a-10 and SH-3b-03 (EffectIcons bypasses K2AddrTable; Gemini links them): one topic SH-T09.
- SH-3a-08 and SH-3a-09 (wheel hook install/uninstall state): one topic SH-T08. 3a-09 bundles two claims (subOk, reportedMissing) in one finding.
- SH-3a-03 also mentions `sceneview::gobSetScale` pointer wrap; entry included in SH-T02.

## Findings citing files outside the bundle manifest
- None missing from the repo. Gemini cites `QueueHud` bundles 2a/2b (SH-3c-04) and `kpatch_api.h`, `windows.h` stub (not assessed): out of this batch's manifests.
- SH-3a-01 / SH-3a-09 depend on `K2WheelEvent` / `K2GuiInputApi` (kpatch_api.h and GuiInput.h; GuiInput.h is in 3a).
- 3b and 3c manifests list GuiKit.h and K2Addr.h as manifest-omit (0 bytes); 3c also omits HudKitTextures.inc. Findings in those bundles relied on 3a text.

## Cross-batch candidates (shared code other batches may also flag)
- `_shared_src/SafeRead.h` readAt/probeWrite: SH-3a-02, SH-3a-03.
- `_shared_src/GuiKit.h` wheel, imageFor, label creation: SH-3a-01, 3a-05, 3a-07, 3a-08, 3a-09.
- `_shared_src/EmbedTex.h` heal: SH-3a-06, SH-3c-03 (hotbar/queue mods using embtex may flag the same).
- `_shared_src/ActQueue.h` playerPaused and QueueHud readPaused: SH-3c-04 (queue_hud batch).
- `_shared_src/Party.h`, `Effects.h`, `EffectIcons.h`: SH-3a-10, SH-3a-11, SH-3c-08 (companion / buff HUD / shield bar batches).
- `_shared_src/CodePatch.h` writeChecked: SH-3a-04 (any mod using codepatch).

## Backlog (not assessed / omitted)
- `_shared_src/HudKitTextures.inc` omitted (over max_file_tokens, 391334 bytes) in 3a and 3c.
- `kpatch_api.h` not in bundle (included by GuiInput.h, HudHost.h, NpcWorld.h, PortraitSlots.h, RangedAI.h, SaveEvents.h).
- `research/companion_levelup/tools/parity/stub/windows.h` host stub not in bundle.
- `MemberFunctionThunk.h` and `VTableOverride.h` unresolved; Gemini could not assess member_function_thunk_test.cpp or vtable_override_test.cpp.
- No findings returned for 3a headers beyond the first 11 (NpcWorld.h, PortraitSlots.h, RangedAI.h, SaveEvents.h, HudHost.h, IniKit.h, HealItems.h, PanelSlots.h, GameAddr.h, K2Addr.h logic): no review remarks, so treat as unreviewed in practice.
- No findings for saferead_test.cpp, codepatch_test.cpp, inikit_test.cpp, heal_items_test.cpp, panelslots_test.cpp, strfmt_test.cpp, windowhook_test.cpp (in bundles, no remarks).
