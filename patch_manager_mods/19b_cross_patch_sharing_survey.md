# 19b. Cross-patch sharing survey (2026-10-02)

Companion to doc 19. Question (user): can the RFC #180 registry simplify our patches with shared functions? Survey of all `*_src` patches by a Sonnet agent. Main checked the key claims on disk: 4 WndProc subclasses; identical icon tables (md5); 0x84D82C/0x84D871 69 bytes apart; GuiKit used only by level-up. Line numbers are as of 10-02 and drift as sibling sessions edit.

**Answer:** two different tools.
- **Shared runtime state or hooks** call for the registry (bucket A).
- **Duplicated helper code** calls for headers in `Patches/Common` (bucket B, as Phase 0 did with SafeRead/GuiKit).

**Upstream constraint:** none of the surveyed patches exist upstream (all are local-only, checked against upstream/master). A real patch-to-patch example therefore cannot ship in the #180 PR unless its patches go upstream too.

## A. Registry candidates (runtime, patch-to-patch)
| # | Candidate | Patches | Benefit | Interface sketch |
|---|---|---|---|---|
| A1 | **Game window + wheel/mouse input** | inv3d `Inventory3DViewport.cpp:473/498/764`, equip3d `EquipCharPreview.cpp:337/361/1099`, effects `CharEffectsPanel.cpp:228/252/1229`, hotbar `ForcePowerHotbar.cpp:738-750/1302` | 4 stacked `SetWindowLongPtrA` subclasses become 1. Detach is only safe "if still on top" (hotbar:1302 comment), so stacking hazards go away. One EnumWindows scan, foreground check and cursor-to-engine transform. ~25-30 LOC less per consumer. | `k2.guiinput` 1: `game_hwnd`, `cursor_engine(x*,y*)`, `wheel_subscribe(cb,ud)`/`unsubscribe`, `foreground` |
| A2 | Character-screen co-tenancy | level-up (ctor 0x84D82C, tick 0x84F221; `LevelUpUi.h:7-14`) + effects (ctor 0x84D871; `FirstId=69` `CharEffectsPanel.cpp:65`) | Replaces coordination by comments and magic numbers (id ranges, x-ranges, append order) with one ctor hook and one tick, plus id and rect reservation | `k2.charscreen` 1: `on_bind/on_tick/on_destroy`, `reserve_ids(n)`, `claim_rect` |
| A3 | Engine state | perf_lab `PerfLab.cpp:59-61,723-753` + load_hang_probe `LoadHangProbe.cpp:44-52,150-170` (same AppGlobal/LoadScreen/GuiMgr/Modals constants; both poll); hotbar modal stack `:185-200` | One poller; load_hang already owns the MainLoop hook 0x781BE0 | `k2.enginestate`: `loading`, `frame`, `main_tid`, `modal_top` |
| A4 | Active-effect snapshot | buff HUD `BuffDurationHUD.cpp:141-262` + effects `CharEffectsPanel.cpp:658-690` (same creature+0x148/0x14C scan, same tables; both scan the leader each frame on the char screen) | One scan per creature per frame | `k2.effects`: `snapshot`, `world_time`, `classify` |
| A5 | Kite stance | ranged_ai E-H3 0x6D850F writes `[ebp-0x174]`; companion_ai A1 0x6D853E reads it (`CompanionAI.cpp:870-876`); stance 16 hard-coded in both | Fixes a silent miss when ranged_ai is absent or `ranged_rules=0` | `k2.rangedai`: `kite_stance`, `maps_to_ranged` |
| A6 | Image wrapper pools (low) | effects `:288`, hotbar `:423`, buff `:124` | Small (8-byte wrappers) | `gfx.image_for` |

## B. Compile-time candidates (Patches/Common), by duplicated LOC
1. ~600: `EffectIconTable.inc`/`DebuffIconTable.inc` byte-identical in buff HUD and effects; also `currentTime`, `isItemAbility`.
2. ~450-550: GUI plumbing re-implemented in 6 patches although GuiKit.h exists (only level-up uses it). Covers `bindCover`, `cursorEngine` (identical x3), `syncScreen`, `ctlRect`, `setCtlVisible`, `textObjOk`, `setFill`, `imageFor`, `setLabelText`, `gameWindow`.
3. ~200: reloadable ini skeleton (`trim`, `parseFloats`, `maybeReloadIni`, loadIni loops in 8 patches). Overlaps doc 19's kplog.ini reloader.
4. ~130: 3D scene-view construction, inv3d `:590-654` vs equip3d `:942-1005`, with identical constexpr blocks.
5. ~160: code-patch helpers: `writeCode` (x5), `patchIat` (perf_lab and load_hang, identical), TX cave builder, `enabled=0` flag parser.
6. ~50: party/leader accessor chain (`[[0xA1B4A4]+4]` -> 0x73FB90/0x7E5DA0/0x77D800) in 5 patches. Upstream GameAPI `GetPlayerCreature`/`GetWorldTime` exist but are bypassed.
7. ~40: `rd`/`wr`, `parseMask`, CExoString read helpers.
8. ~45: panel-keyed slot table (`claimSlot`/`findSlot`).

## Existing ad-hoc coupling (today)
- No DLL-to-DLL GetProcAddress anywhere.
- load_hang_probe reads other patches' log files by a hard-coded, stale name list (`:504-508`).
- Stack-frame coupling ranged_ai -> companion_ai (A5).
- Id, layout and append-order coupling level-up <-> effects (A2).
- Hand-coordinated sites: robustness skips 0x726505 for perf_lab 0x72650B; texmem and perf_lab tx=1 both write 0x4261CF (second writer refused by byte verify); ActivationFix 0x40D69E vs load_hang 0x40D692.

## C. Not worth sharing
- Tooltip and input logic (low similarity).
- TwoDA/Tlk (hotbar only).
- EquipIconFix vs EquipCharPreview, and level-up vs buff HUD in UpdatePortraits: different sites, no shared state.
- companion_ai IniTable (table-driven and tested).

## Recommendation
- **#180 PR:** registry + `kpatch.log` + provider/consumer *test* patches; logging examples use patches that are upstream or come in the same PR.
- **First real patch-to-patch interface: A1 `k2.guiinput` on our fork.** Provider ~120 LOC. Consumers `require` lazily and fall back to their own subclass, so migration is near zero-risk. It goes upstream with those patches (their PULL_REQUEST.md drafts exist). A2 has the highest conflict value but the most work; A5 fixes a latent bug.
- **B1-B3 first for LOC:** shared icon tables, GuiKit adoption, ini helper (fold into the kplog.ini reloader).
- All consumers are sibling-owned: migrate with owner agreement only.
