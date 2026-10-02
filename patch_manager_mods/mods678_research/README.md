# Mods 6-8 research workspace (session 2026-09-30)

Evidence and tools behind [`../11_inventory_sort_filter.md`](../11_inventory_sort_filter.md) (Mod 6),
[`../12_empty_container_indicators.md`](../12_empty_container_indicators.md) (Mod 7) and
[`../13_ranged_ai_spacing.md`](../13_ranged_ai_spacing.md) (Mod 8, community survey in
[`../13a_ai_community_research.md`](../13a_ai_community_research.md)). The mod docs hold the
conclusions; this folder holds the raw evidence so nothing has to be re-derived.

## Files

| File | What |
|---|---|
| `ghidra_batch1_decompile.txt` | Decompile + xrefs of 24 Steam functions (all confirmed function entries): inventory `CheckFilter` 8a8df0, `SetNextFilter` 8a8b30, `NotUseable` 8a86d0, `CantEquip` 8a8640, `PopulateItemListBox` 8a7ba0, `CSWSCreature::CanUseItem` 5d2f80, `CItemRepository::ClearNewItems` 605f40, `CSWSPlaceable::GetItemCount` 5f67b0, `HandleMouseOver` 7b3a00, `SetMouseCursor` 7b3870, `HiliteAreaObjects` 7aa460, `CSWCPlaceable::GetTargetActions` 842d40, `UpdateNameLabel` 742a00, `CSWVarTable::GetLocalBoolean` 5e9210, `MaxAttackRange` 571570, `DesiredAttackRange` 5716a0, `AIActionCombat` 6d9500, `AIActionPhysicalAttacks` 6d7390, `CreateNewAttackActions` 6da4f0, `CSWSArea::ClearLineOfSight` 52bb60, `ExecuteCommandHasLineOfSight` 69ac60, `RangeWeaponEquipped` 56e1e0, `AIActionCheckMoveToObjectRadius` 5bc1d0, `CSWSCombatRound::StartCombatRound` 58e730 |
| `ghidra_batch2_decompile.txt` | `CreateItemEntry` 8a75f0, `FUN_0058ae90` (current creature getter), `ActionIconToMouseCursor` 7b3ee0, `FUN_00797df0` (highlight OFF caller), `AddMoveToPointAction` 5644b0, `FUN_0057c990` (nearest hostile in range with LOS), `FUN_005ff950` (per-object LOS mesh test), `FUN_006d0620` (GetItemInSlot), `InitializeNewAttackTarget` 6dac70 |
| `utp_container_survey.json` / `utp_container_script_slots.txt` | 850 container UTPs: script slots, static item count, flags |
| `decompile.sh ADDRS OUT` | headless read-only Ghidra decompile wrapper around `DecompileList.java` |
| `read_exe_consts.py VA...` | float/double/int at a VA via the real PE section table |
| `extract_vanilla_nss.py OUT` | dump all 638 vanilla `.nss` from `Scripts.bif` (the AI includes live there) |
| `survey_container_utps.py OUT` | regenerate the container survey |
| `dump_excerpts/<addr>.{c,asm}` | **batch 3 (no new Ghidra run):** decompile + disassembly of ~100 cited functions copied from the full-program dump (`~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71`, lesson 166). This is the citable evidence for the "Definitive build plan" sections of docs 11-13 |
| `copy_dump_excerpts.sh ADDR...` | copy more functions from the dump into `dump_excerpts/` |
| `check_windows.py` / `check_windows_output_2026-09-30.txt` | validates every proposed splice window (whole instructions, relative branches inside, external branches into the interior, overlap with live hooks) and prints TOML-ready `original_bytes` |
| `live_hooks_2026-09-30.tsv` | the 63 hooks in the live `patch_config.toml` (address, window length, type, function, patch id) |
| `verify_windows_both_exes.py` | compares every window's original bytes against BOTH executables (LAA `4AB72FC1`, base `6A522E71`); 2026-09-30 all 11 identical |
| `persist_batch3_db.py` | idempotent upsert of batch-3 `functions` / `name_map` / lessons 171-178 into the knowledge DB |

Related tool: `../ghidra_knowledge/gog_cluster_delta_map.py` (+ `../ghidra_knowledge/gog_map_mods678.tsv`,
2518 GOG→Steam rows for the classes below). Knowledge DB (`../ghidra_knowledge/kotor2_ghidra_knowledge.db`)
got 2435 `name_map` rows, 14 `functions` rows and lessons 161-164 in session 1; batch 3 added 57 `functions` rows, 57 `name_map` rows (source `mods678-batch3-dump`, `high` where the address is a real entry and its behaviour was read, `medium` where only the label is interpretive) and lessons 171-178.

## Methods learned this session (use them, don't rediscover)

1. **GOG names → Steam without Ghidra.** `gog_cluster_delta_map.py CLASS_REGEX ...` votes a per-run
   delta (Steam starts whose `ret N` equals GOG `param_size_bytes`), peels, then drift-fills.
   `--selftest` checks 10 known anchors (10/10 PASS). Tags: `vote` = strong, `seeded` = medium,
   `drift±x` = hypothesis. Failure modes: tiny classes (<~10 fns) tie on votes; GOG `param_size`
   values of 1/5/6/57 are data errors (bools or bad counts), so those never match; Aspyr inserted
   functions shift a class, e.g. `CSWGuiInGameItemEntry` has 2 extra Steam fns (`0x8b0c30`, `0x8b0ec0`).
   Fallback for a small class: list `steam_functions.tsv` rows in the window at the neighbour
   class's delta and match by order + `ret N` (that's how ItemEntry was placed).
   Widening voters to neighbouring classes made results worse (their deltas differ); don't.
2. **Batch decompile, read selectively.** Put 10-25 addresses in one `decompile.sh` run (about 2
   min), write to a file, then `awk '/=== FUN_x/,/=== FUN_y/'` the parts you need. Never print a
   whole batch.
3. **Constants:** Ghidra prints `_DAT_x` for both float and double literals; read both with
   `read_exe_consts.py`.
4. **Script sources:** the AI includes are in `Scripts.bif` as `.nss`; the override has none of
   `k_ai_master`/`k_hen_*`/`k_inc_generic`, so vanilla AI is live. Compile with nwnnsscomp only
   (`~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/compile.sh OUT.ncs SRC.nss [includes]`).
5. **pykotor** on this install: `logging.disable(logging.CRITICAL)` first, or `Installation()` prints
   ~55 MB of debug output.

## Batch 3 (2026-09-30, plan execution): what was learned

6. **Full-program dump first.** Almost every open item was answered with `grep`/`sed` over the dump (`decomp/<bb>/<addr>.c|.asm`, `callgraph.tsv`, `strings.tsv`)
   and no new Ghidra run. To prove "nothing else calls X", scan the PE with python for `E8`/`E9` rel32 and 4-byte data references (that is how
   `HiliteAreaObjects(on=1)` was shown to be unreachable).
7. **Framework `consumed_exit_address`** (installed `KotorPatcher.dll` supports it): a detour whose handler returns non-zero jumps to a chosen address
   after the stolen bytes ran; keep the return value with `exclude_from_restore=["eax"]`. For entry hooks the target must be the function's own epilogue.
   With `skip_original_bytes=true` the window may contain a relative `CALL` (never re-run). See lesson 172.
8. **Read the asm for the calling convention**, not the pretty-print: `GetUpgradeType 0x6076D0` is `thiscall(item; 1 unused stack arg) RET 4`,
   and `CheckFilter` mode 5 hid three ECX values.

### Results by mod (details in the docs)

| Mod | Key results |
|---|---|
| 6 inventory | 3 hooks (`0x8A8CF0`, `0x8A7E22` skip-window, `0x8A7E76`); usable = `0x77B670`; open upgrade slot = `item+0x2F4+4*i == -1` for `i < {1:6,2:3,3:3,4:2}[GetUpgradeType]`; New flag persists (`NewItem` GFF field); rows are rebuilt by `SetItem` (so never call `SetCanUse(row,0)`, only `(row,2)` for unusable); `LBL_FILTER` is the static caption "Filter" |
| 7 containers | two-step UI: "Empty" label is selection-only; highlight `on=1` has no caller; hooks `0x7B3870` (cursor), `0x7AA747` (case-9 colour), `0x7AC470` (tick + own hotkey) |
| 8 ranged AI | `+0x11AC` party, `+0x10E8` IsPC, `+0x52C` end-attack; hold `0x6D83EA` + retarget gate `0x6D7C73`; stance 16 needs `0x77AA5E`, `0x77ACBC`, `0x77D8D4`; `SetNPCAIStyle` has no range check; E3 (do allies block LOS) is the one item left for Stage 0 |


## Session log 2026-09-30 (research + planning; read this first when resuming)

**What was done (in order):** (1) plan executed from the approved research plan; the full-program dump replaced the planned Ghidra batches B3-B6, so no new
Ghidra run happened; (2) evidence copied to `dump_excerpts/`, splice windows validated (`check_windows.py`), live hooks tabulated, DB persisted
(`persist_batch3_db.py`, 57 functions, 57 name_map rows, lessons 171-178); (3) definitive build plans written into docs 11/12/13; (4) an independent
Opus review found 4 real errors (wrong `LBL_FILTER` offset 0x16B8, `SetCanUse(row,0)` erasing the New highlight, `ClearNewItems` scope, upgrade-slot
predicate) and ~10 smaller ones, all fixed in the docs, DB and this README; (5) windows verified byte-identical in both executables.
**State:** everything is **uncommitted** (docs 11-13/13a, this folder, mapper, DB, README index). No code, no installs, no game-folder writes.
**Decisions taken with the user:** Mod 6 extra filters by re-clicking the active All tab (no new buttons); Mod 8 = party first, enemies later,
new dedicated stance 16 "Ranged (Kiting)"; Mod 7 Q1 answered statically (Empty label is selection-only).
**Still open research** (each has an owner section in the docs):
- Mod 8 **E3**: do allies block engine LOS? (Stage-0 log at the hold site); confirm `creature+0x558` survives a hold.
- Mod 8: what is feedback message `0xDA` (`0x584DA0`) that the retarget path sends (spam risk for companions); does the AI keep firing after an attack-less "complete".
- Mod 8: S-H4 (HUD current-stance marker, `0x74DA40`) not designed; TLK strings + `aiscripts.2da` build + `ib_kite` icon; free `SetLocalNumber` index; script compile test.
- Mod 7: `HiliteAreaObjects(on=1)` was never exercised by the shipped game (flicker/fade, stuck tint on toggle-off) - Stage 2 testing; hotkey default `Z` free?
- Mod 6: tint lost on row select (`8A83D0`→`8B1CC0`); "Usable" predicate difference (`IsItemUsable` vs mode 5); name sort (`CSWCItem::GetName 0x7FDFA0`) deferred.
- In-game checks listed at the end of each doc (user-run).

## Classes mapped (for the next rounds)

`CSWGuiInGameInventory` (18/18), `CItemRepository`, `CSWSItem`, `CSWCItem`, `CSWGuiUpgrade`, `CSWCCreature`,
`CSWSPlaceable`, `CSWCPlaceable`, `CClientExoAppInternal`, `CClientExoApp`, `CSWCArea`, `CSWCObject`,
`CSWGuiTargetActionMenu`, `CSWVarTable`, `CSWSScriptVarTable`, `CSWGuiFloatyText`, `CGuiInGame`,
`CSWCAnimBase`, `CSWSCreature`, `CSWSCombatRound`, `CSWSArea`, `CSWSObject`, `CPathfindInformation`,
`Global` (ExecuteCommand*), `CSWVirtualMachineCommands`. Still unmapped (need Ghidra or order-matching):
`CSWSItem::AddUpgrade`, `CSWCCreature::GetUsableItems`, `CSWCPlaceable::UsePlaceable`, `CSWCObject::Hilite/UnHilite`,
`CSWSCreature::DoCombatStep`, `CSWSCreature::AddAttackActions`, `CSWSCreature::UpdateVisibleList`,
`CSWSCombatRound::AddAttackAction`.

## Key facts not repeated elsewhere

- Game data: `baseitems.2da` (override) blasters `maxattackrange` 17-28, `prefattackdist` 20; melee 0.5.
  `placeables.2da` (override, 402 rows) `UseSearch` 0 on 281 rows, 1 on 112 (corpses/remains).
  TLK 125550 "Empty", 125551 "Nothing", 42486 = inventory NotUseable popup (`0xa5f6`).
- Item flags at `item+0x2c8`: bit 3 counted by `GetItemCount(onlyFlagged)`, bit 7 = new item, bit 8 =
  hidden from inventory. `item+0x10c` = quest/plot flag (filter modes 2/10). `item+0xc` = base item id.
  `baseitem+4` = equip slot mask, `+0x1a` = ranged flag, `+0x34` = maxattackrange, `+0xac` = item
  class (0x18 datapad), `+0xb4` = weapon wield type.
- Placeable: `+0x37c` HasInventory, `+0x3c4` item repository (count at repo `+0x10`, items via `FUN_00606100(i)`).
- Inventory filter mode byte: `FUN_0073f750()+0xfc1` (GuiInGame object). Panel repopulate flag:
  `panel+0x24d0 |= 4`.
- Cursor ids: `ActionIconToMouseCursor` maps 0x3e9→0x19, 0x3ea→0xb, 0x3eb→0x33, 0x3f2→0x17, 0x3f4→0x21,
  0x3f5→7, 0x3f7→0x19, 0x402→0x25, -1/default→5; `HandleMouseOver` also uses 1 and 0x2d.


## Build phase log (started 2026-09-30, after commit f883104)

**R-C (framework acceptance) - confirmed from source.** Parameter source forms live in `src/KPatchCore/Models/Parameter.cs`: `"ecx"` = register value,
`"esp+N"` = address of the game's stack slot, `"[reg+-off]"` = dereference (live K2 hooks already use `[ebp-0x38]`), `"const:0x.."`. `Hook.IsValid` has no max
`original_bytes` length; `consumed_exit_address` needs `"eax"` in `exclude_from_restore` and `wrapper_x86.cpp` runs the stolen bytes (unless
`skip_original_bytes`) *before* the `PUSHFD/TEST EAX,EAX/JZ/POPFD/JMP consumed` test. No live K2 hook used it before (K1SecuritySpikes is K1-only), so Mod 6 H6a is its
first K2 use: **Stage 0 install must show `Parsed consumed_exit_address` in the Proton log.** The "negative address-form" worry in doc 11 is dropped.

**R-A (Mod 7)** answered by the user (in-game check 1): containers already show "(empty)", broken droids "(nothing)"; no K1-style hover hook needed (doc 12 tail).

**Build tooling note:** `create-patch.py` fails ("no such table: game_version") while `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/AddressDatabases/kotor1_steam.db`
is a 0-byte file (appeared 2026-09-30 10:43, not created by this work). Workaround used (does not touch the shared dir):
`KPATCH_ADDRESS_DATABASES=<dir of symlinks to the kotor2_*.db files> python3 ../create-patch.py`.

**Mod 6 S0 built** (`../inventory_sort_filter_src/`, build copy `Patches/InventorySortFilter/`): own patch id `inventory-sort-filter`, all three hooks, one DLL, INI-staged
(`sort`/`tint`/`cycle` default 0 = logging only). Host tests `sort_logic_test.cpp` ALL PASS; `-Wall -Wextra` clean; hook `original_bytes` byte-identical to `check_windows_output_2026-09-30.txt`.
Not installed. Independent Opus review requested before any install offer.

## Session log 2026-09-30 (build phase, part 2) - see `HANDOFF_2026-09-30.md` for the full hand-off

Mod 6 review fixes applied and Stage 0 **installed live 11:07** (not yet user-tested); Mod 7 cancelled by the user; Mod 8 research R-B/R-D/R-E/R-F closed (doc 13) and Stage 0 logging DLL built (`../ranged_ai_src/`, not reviewed/installed);
`stage_and_verify_install.py` added; `check_windows` table extended with the S-H4 window (`..._2026-09-30b.txt`). A sibling session re-installed at 11:14 (82 hooks) - Mod 6 survived.

## Session log 2026-09-30 (build phase, part 3)

Opus review of the Mod 8 Stage 0 DLL found a **BLOCKER**: with `consumed_exit_address` the framework runs the stolen bytes after the handler with EAX = handler result, so E-H2's stolen `MOV ECX,[EAX+0x10E8]` at `0x6D7C73` would fault on first hit. E-H2 moved to `0x6D7C79` (6 bytes `89 8D 50 FD FF FF`, same consumed exit `0x6D7C8C`); S-H2 and S-H3 (Stage 3) have the same flaw and must be re-sited. Also fixed: throttle slots (party-only claim), log truncated per session + cap marker, `approach` renamed `backoff`, `0.1f`, non-null area. Rebuilt `RangedAI.kpatch` (md5 94642e19...). Collision snapshot refreshed to the live 82 hooks (`live_hooks_2026-09-30b.tsv`); `check_windows_output_2026-09-30d.txt` ALL OK, `verify_windows_both_exes.py` ALL MATCH. DB lesson via `persist_batch4_db.py`. The 0-byte `kotor1_steam.db` was already moved aside (`.empty_moved_0930`); `create-patch.py` works without the override again.

## Session log 2026-09-30 (build phase, part 5: 0.2.0 UX builds)

Mod 6 0.2.0 (equip-based tint/usable via `0x5BD6A0`, own Usable filter, H6d re-tint at `0x8A8478`, log hygiene) and Mod 8 0.2.0 (stance 16: S-H1..S-H4 re-sited EAX-free, TLK/2DA tools, kite script with byte-identical toolchain proof) built, not installed. `live_hooks_2026-09-30e.tsv`, `check_windows_output_2026-09-30e.txt` (ALL OK, 13 windows, both exes match), `persist_batch5_db.py`. Details: doc 11/13 tails and `HANDOFF_2026-09-30.md` part 5.

## Session log 2026-09-30 (build phase, part 5 continued: reviews, READY TO INSTALL)

Opus review Mod 8: BLOCKER manifest TOML (fixed); S1 = vanilla Ranged/Stationary companions never approach (`[ebp-0x174]` CMPs at `0x6D876A/0x6D8A7E`) -> new skip-only hook **E-H3 `0x6D850F`** maps stance 16 -> 11; S2-S6 + nits fixed (`7fe77f1`). Opus review Mod 6: BLOCKER H6d stale row on the equipment screen (shared handler `0x8A83D0`) fixed (`b56a2be`). Focused re-review: no blockers, nits applied (`dad84a5`). `check_windows_output_2026-09-30f.txt` (14 windows) ALL OK / ALL MATCH. New: `apply_mods68_data.py` (install-time data, dry-run default), `stage_and_verify_install.py` handles an extra kpatch that upgrades a live id. Final artifacts and the runbook/checklist: `HANDOFF_2026-09-30.md` "Ready to install".

## Session log 2026-09-30 23:57 (combined install, Phase 1 of the next-phase plan)

Mods 6+8 0.2.0 INSTALLED (user go). A sibling changed the live set while the go was pending (level-up-override, perf-lab 0.3.0, load-hang-probe 1.4.0), so the windows were re-checked first: `live_hooks_2026-09-30g.tsv` (120 hooks), `check_windows_output_2026-09-30g.txt` ALL OK / ALL MATCH. Live set: 22 ids, 128 hooks, verify 0 mismatches. Data: TLK strrefs 136538/136539, `override/aiscripts.2da` + `override/k_ai_master.ncs`, both INIs. Details: `HANDOFF_2026-09-30.md` "INSTALLED".

## Session log 2026-10-01 (companion AI review)

The user asked whether the rest of the companion AI is understood and to review it for bugs. Three read-only Opus reviews (decision scripts, event scripts, engine) -> `ai_review_2026-10-01/` (README = triage + logging plan). The lead verified H1, H2, E2 and the A1/A2 timer logic on disk. DB lessons via `persist_ai_review_db.py`. Waiting for the user to choose the scope (Mod 8 fix round vs a separate companion-AI-fixes mod vs AI trace logging).

## Session log 2026-10-01/02 (companion AI round)

Three Opus reviews -> three designs -> builds (companion-ai 0.1.0, ranged-ai 0.3.0, party AI scripts) -> two build reviews (scripts B1 Cure-poison loop + S1-S5; DLL SF1-SF3) -> fixes -> focused re-review clean -> **installed 2026-10-01 23:36** (26 ids/149 hooks, 17 override scripts). Everything is in `ai_review_2026-10-01/`; install record and research backlog in `HANDOFF_2026-09-30.md` ("Companion AI round INSTALLED", "Session close"). DB: `persist_ai_review_db.py`, `persist_ai_round_db.py`.
