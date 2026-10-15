# hotbars triage (prefix HB; 21 findings, 15 topics, 0 dropped)

- HB-T01 P1: TwoDA::ParseImpl unbounded cell scan and 32-bit overflow (1a-01, 1a-02) plus matching twoda_test gaps (1d-01, 1d-02)
- HB-T02 P2: readCreatureTag copies raw tag bytes; ']' / newline can break INI section (1a-03)
- HB-T03 P1: saferead::readAt base+off 32-bit wraparound before probeRead (1a-04) [shared]
- HB-T04 P2: ensureWheelHook never falls back to WindowHook when gui-input subscribe fails (1a-05)
- HB-T05 P2: normalizeItemName truncates at first " (" (1a-06)
- HB-T06 P3: 07_force_power_hotbar.md character key / hook list stale vs code (1b-01)
- HB-T07 P2: clampOffset has no lower bound; chooseTarget with negative cur; untested (1b-02, 1b-03)
- HB-T08 P3: keybif_test misses malformed KEY/BIF error branches (1d-03)
- HB-T09 P3: tlk_test system rm -rf / no TempDir; twoda_test real-data TLK step no assertions (1d-04, 1d-05)
- HB-T10 P1: guikit::detail::wr checks readability, not writability, before store (1c-01) [shared]
- HB-T11 P2: label leak on failed createLabelFromTemplate / bindByTag after ctor (1c-02) [shared]
- HB-T12 P2: stale tooltip on empty name; name-cache orderSig collisions (1c-03, 1c-04)
- HB-T13 P2: arrow hide not restored / g_arrowState not reset on ini reload (1c-05)
- HB-T14 P3: vanilla tint read overwritten, dead fallback branches (1c-06)
- HB-T15 P2: WindowHook::install ignores recreated HWND (1c-07) [shared]

## Duplicates (within batch) and how folded
- 1d-01 is the test-side twin of 1a-01, and 1d-02 of 1a-02; folded into HB-T01 (same ParseImpl read). Not dropped, since the test-gap claims are separately checkable.
- 1a-04 (readAt) and 1c-01 (wr) share SafeRead.h probe semantics but are different defects; kept as T03 and T10.

## Findings citing files outside the bundle manifest
- None outside the bundles' included files. 1c-06 and 1c-03/04/05 cite only CombatHotbar.cpp (included in 1c). 1a-05 cites GuiInput.h (included in 1a).
- Gemini 1c-01/02/07 cite GuiKit.h and WindowHook.h; both are included in 1c (GuiKit.h is omitted from 1a).
- Gemini's 1a-03 refers to "Config::Serialize"; I did not verify where it lives (likely HotbarConfig.h).

## Cross-batch candidates (shared / library code other batches may flag)
- patch_manager_mods/_shared_src/SafeRead.h readAt wraparound: HB-1a-04 (also relevant to HB-1c-01 probeWrite)
- patch_manager_mods/_shared_src/GuiKit.h wr probe: HB-1c-01; label leak: HB-1c-02
- patch_manager_mods/_shared_src/WindowHook.h install: HB-1c-07 (also GuiKit.h wheelInstall)
- patch_manager_mods/_shared_src/GuiInput.h wheel_subscribe semantics: HB-1a-05
- patch_manager_mods/action_hotbar_src/HotbarConfig.h / TwoDA.h are mod-local, not shared.

## Backlog (not assessed / omitted)
- 1a: GuiKit.h and K2Addr.h omitted from the bundle (guikit::* and K2AddrTable not verified); HotbarTextures.inc omitted (over max_file_tokens); kpatch_api.h and KpLog.h not provided; CHANGELOG.md, PULL_REQUEST.md, README.md, baseline_h3_build26.txt omitted.
- 1b: testdata/spells_fixture.2da is binary and not embedded.
- 1d: action_hotbar_src/common_contract_test.cpp omitted; HotbarTextures.inc omitted; KpLog.h not provided; binary fixtures under testdata/ (fixture.tlk, spells_fixture.2da, keybif_fixture/**) omitted (generator scripts were seen).
- 1c: combat_hotbar_src/common_contract_test.cpp omitted; kpatch_api.h and KpLog.h not provided; hud_host_src/ provider implementation (hooks 0x00743540, 0x00743847, 0x0040F5A0, K2HudContext) not assessed.
- Gemini's "File Integrity Check" in the action_hotbar response reports all three bundles complete (not truncated).
