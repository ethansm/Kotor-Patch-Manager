# Mod Idea 6: Inventory Sorting & Sub-Filters

**STATUS (2026-09-30, research complete; Stage 0 code built, not installed - see 'Build status' at the end):** all engine questions are answered; this file ends with the
**definitive build plan**. Evidence: `mods678_research/dump_excerpts/<addr>.{c,asm}` (decompile + disassembly of every cited
function), `mods678_research/check_windows_output_2026-09-30.txt` (splice validation), knowledge DB rows tagged
`mods678 batch3` (`functions`, `name_map` source `mods678-batch3-dump`, lessons 171-178).
Ships as its **own patch id `inventory-sort-filter`** (`Patches/InventorySortFilter`; decided in the 2026-09-30 build plan; same panel as Mod 1 but Mod 1 hooks `0x8A69AD`, `0x8A8172`, `0x8A7350` do not overlap).

## Goal

1. **Sort:** new items first, then usable-by-this-character, then grouped by base item, then localised name (ASCII case-insensitive), then vanilla order; unusable rows tinted.
2. **Extra filters** by **re-clicking the active "All" tab** (user decision): All → New → Usable-by-me → Open upgrade slot → All.
   No new buttons.

## Engine map (Steam, all decompile-confirmed; addresses are function entries)

| Function | Steam | Convention / facts |
|---|---|---|
| `PopulateItemListBox` | `0x008A7BA0` | `fastcall(panel)`. Frame (EBP-relative): `-0x84`=panel, `-0x40`=current creature (`0x77D800`), `-0x38`=client party object (`0x7E5D80`), `-0x3C`=row counter, `-0x2C`=list, `-0x18`=item, `-0x20`=reselect index. Equipped loop `8A7C85..8A7DB7`, backpack loop `8A7DC5..8A7E71`, finalise `CALL 0x41FC30` at `8A7E8C` |
| `CreateItemEntry` | `0x008A75F0` | `thiscall(panel; creatureObj, int* counter, list*, item, slotFlag, equippedFlag)` `RET 0x18`. Row = `((void**)*(panel+0x24B0))[counter-before-call]`; ends with `*counter += 1` |
| `CheckFilter` | `0x008A8DF0` | `thiscall(panel; item, creature, slot)` `RET 0xC`. Mode byte at `GuiInGame+0xFC1`. Mode 5 = `0x58AE90(creature)` then `0x77B670(item,slot,0xFF)` and `0x5D2F80(creature)(item,0,1)` |
| `SetNextFilter` | `0x008A8B30` | `thiscall(panel, mode 0..11)` `RET 4`; lights tabs only for 0,6..11; sets `panel+0x24D0 \|= 4` |
| tab click handler | `0x008A8CF0` | `thiscall(panel, ctrl)` `RET 4`. All 7 `BTN_*` share it (ctor `8A6170` binds event `0x27`). Mode = `*(int*)(ctrl+0x5C)`; writes it to `GuiInGame+0xFC1`, un-highlights 7 tabs, highlights the clicked one, `panel+0x24D0 \|= 4`. Epilogue (`MOV ESP,EBP; POP EBP; RET 4`) at `0x8A8DE2` |
| `IsItemUsable` | `0x0077B670` | `thiscall(clientPartyObj; item, slot, 0xFF)` `RET 0xC`; 0 ⇒ click shows the NotUseable/CantEquip popup. `CreateItemEntry` calls it with ECX = its 1st stack arg |
| `SetCanUse` | `0x008B1D00` | `thiscall(row, byte mode)` `RET 4`. 0/1 = normal teal, **2/3 = (0.74, 0.11, 0.0) red-orange** at row `+0x98/0x1FC/0x110/0x274`; `row+0x3B0 = mode`. Vanilla inventory never calls it (only `8AC330`, another panel) |
| `SetItem` | `0x008B1250` | `thiscall(row; itemId, equipped, newFlag)`; vanilla already passes `item+0x2C8>>7&1` as the new marker |
| `GetUpgradeType` | `0x006076D0` | `thiscall(ECX=item; 1 unused stack arg)` `RET 4`. 0 if `item+0x2F0 <= 0`; by `baseitem+0xAC`: `0..2`→3, `3..5`→2, `0x27..0x29`→1, `0x1F..0x26`→4 |
| `GetUpgradeSlotCount` | `0x008CB230` | type 1→6, 2→3, 3→3, 4→2 |
| `AddUpgrade` / `RemoveUpgrade` | `0x00615D60` / `0x00615E90` | only tag/merge property entries (`+0x12 = slot+1`); they do **not** touch `+0x2F4` |
| installed upgrades | `item+0x2F4 + slot*4` | `0xFFFFFFFF` = empty. Written by the upgrade screen `0x8CDB00` (lines 109-110, 154-155, 207), loaded by `0x601740` (default -1), saved by `0x602DD0` only when non-zero (so upgrade-row 0 reloads as empty). **Slot 1 of a type-1 item is never written** (`0x8CDB00` skips it), so it stays -1 forever |
| `AddItem` | `0x00605090` | `thiscall(repo; item**, x, y, newFlag)`; `newFlag` sets `item+0x2C8` bit 7 |
| `ClearNewItems` | `0x00605F40` | only caller `8A7520` (OnPanelRemoved), which also **destroys every row** and nulls `panel+0x24B0[i]`. It clears only the ids in the list at `panel+0x24BC`, which the row-enter handler `8A8100` fills for items whose New bit is set; row-select `8A83D0` clears the bit directly. Items you never hovered stay New |
| item GFF | `0x00601740` / `0x00602DD0` | field **`NewItem`** is saved and loaded ⇒ the New flag survives save/load |

`CheckFilter` modes: 0 All, **1 New**, 2 quest flag, 3 equippable, 4 not equippable, **5 usable by current char**,
6 Datapads, 7 Weapons, 8 Armor, 9 Useable (base items 53-56), 10 Quests, 11 Misc.
`LBL_FILTER` (`panel+0x16B8`) is the static caption **"Filter"** (strref 48548); no code rewrites it.

## Research items (all closed)

- [x] **R8** usable predicate: `IsItemUsable(ECX=clientPartyObj)(item, slotFlag, 0xFF)`, the same call `CreateItemEntry` uses for the popup (`dump_excerpts/008a75f0.asm` @`8A7785`). `CanUseItem(creature)(item,0,1)` is added only by filter mode 5. Creature = `0x77D800` of the party object (equipped loop and mode 5 use it). *In-game check 1 below confirms it follows the shown character.*
- [x] **R9** open-slot predicate: `t = GetUpgradeType(item)`; `n = {1:6,2:3,3:3,4:2}[t]`; open ⇔ `t != 0` and some `i < n` (**skipping `i == 1` when `t == 1`**, which the upgrade screen never fills) has `*(int*)(item+0x2F4+4*i) == -1`. Source: `8CDB00`, `8CB230`, `8CD8E0`, `601740`, `602DD0`.
- [x] **R10** New flag set by `AddItem(newFlag=1)`; persisted (`NewItem`); cleared on panel close and on select.
- [x] **R11** tab handler `0x8A8CF0` (see map); re-click ⇔ `ctrl[0x17] == GuiInGame[0xFC1]`.
- [x] **R12** splice chosen: buffer at the backpack `CreateItemEntry` site, flush before finalise (below).
- [x] **R13** `SetCanUse(row,2)` draws the red-orange tint (colours decoded from `0x9A17A0`=0.74, `0x98BA9C`=0.11). `SetItem` (called by `CreateItemEntry` on every row) rewrites all four colour triplets each time (teal, or the vanilla New colour `0xA7FE48` when the New bit is set; `8B1536-8B168B`), so **no reset is needed and `SetCanUse(row,0)` must NOT be called** (it would erase the vanilla New highlight). Call `SetCanUse(row,2)` only for unusable rows. `row+0x3B0` is never read by inventory code.
- [x] **R14** name getter is `CSWCItem::GetName 0x7FDFA0` (`thiscall(clientItem; CExoString* out, identified)` `RET 8`, string at `+0x18C`). **v1 does not sort by name** (base item id groups identical kinds); name sort is a later stage. **DONE in WI-21 (C132, `c24be8e`):** name key added as the tie-break after base item (see the WI-21 build entry under 0.2.0).

## Definitive build plan

### 1. Hooks (new `inventory_sort_filter_src/kotor2-steam-aspyr.hooks.toml`; both Steam SHAs)

All windows validated by `check_windows.py` (whole instructions, no external branch into the interior, no live-hook overlap).
Use `source = "ebp"` (register value) and compute frame addresses in C++ (`ebp-0x3C` etc.) so no negative address-form parsing is needed.

| # | Address | Len | Type | Original bytes | skip | Handler inputs → action |
|---|---|---|---|---|---|---|
| H6a | `0x008A8CF0` | 6 | detour + `consumed_exit_address = 0x008A8DE2`, `exclude_from_restore=["eax"]` | `55 8B EC 83 EC 14` | false | `ecx`=panel, `[esp+4]`=ctrl. Re-click state machine (§3). Return 1 ⇒ skip vanilla body |
| H6b | `0x008A7E22` | 79 | detour | `8B 55 E8 8B 42 14 89 45 9C 8B 8D 7C FF FF FF 8B 91 C8 24 00 00 3B 55 9C 75 16 8B 45 C4 89 45 E0 8B 8D 7C FF FF FF C7 81 C8 24 00 00 00 00 00 7F 6A 00 6A 00 8B 55 E8 52 8D 45 D4 50 8D 4D C4 51 8B 55 C8 52 8B 8D 7C FF FF FF E8 7F F7 FF FF` | **true** | `ebp`. Replaces "reselect check + push args + `CALL CreateItemEntry`" for **backpack** rows: buffer `item`, do the reselect bookkeeping, apply the upgrade filter. Resume at `0x8A7E71` |
| H6c | `0x008A7E76` | 22 | detour | `6A 00 6A 01 6A 00 8D 45 D4 50 8B 8D 7C FF FF FF 81 C1 88 05 00 00` | false | `ebp`. Sort the buffer, create the rows, tint, then original bytes + `CALL 0x41FC30` run |

The 79-byte window contains a relative `CALL`; that is safe only because it is `skip_original_bytes=true` (bytes never re-run).
Every buffered row is created later by the same `CreateItemEntry`, so vanilla row layout, popups and events are unchanged.

### 2. Engine calls (C++ typedefs; all `thiscall` unless noted)

| Call | Address | Signature |
|---|---|---|
| CreateItemEntry | `0x008A75F0` | `void(__thiscall*)(void* panel, void* cobj, int* counter, void* list, void* item, int slot, int equipped)` |
| IsItemUsable | `0x0077B670` | `int(__thiscall*)(void* cobj, void* item, int slot, int mask)`; call with `cobj = *(void**)(ebp-0x38)`, `slot=0`, `mask=0xFF` |
| SetCanUse | `0x008B1D00` | `void(__thiscall*)(void* row, uint8_t mode)` |
| SetNextFilter | `0x008A8B30` | `void(__thiscall*)(void* panel, int mode)` |
| GetUpgradeType | `0x006076D0` | `uint8_t(__thiscall*)(void* item, int unused)` (pass 0) |
| label text | `0x00416E30` + Exo string ctor/dtor | reuse Mod 5 `setLabelText` recipe (`equipment_3d_character_src/EquipCharPreview.cpp:392`); label object = `panel+0x16B8` |

### 3. Behaviour spec

**Sort (H6b + H6c).** H6b, per backpack item (server item from `FUN_00606100(i)`, already filtered by `CheckFilter` and the hidden-bit test upstream):
1. If the upgrade filter is active and the item has no open slot (R9 predicate) ⇒ drop it.
2. Reselect: if `*(int*)(panel+0x24C8) == *(int*)(item+0x14)` ⇒ remember that id, set `*(int*)(panel+0x24C8)=0x7F000000` (exactly what the skipped vanilla code did).
3. Push `{item, origIndex, isNew = item+0x2C8>>7&1, usable = IsItemUsable(...)!=0, baseItem = *(int*)(item+0xC)}`.

H6c, once, before the finalise call: stable-sort by `isNew desc, usable desc, baseItem asc, name asc (ASCII case-insensitive; "" sorts first), origIndex asc`; for each entry: if its id equals the remembered id ⇒ `*(int*)(ebp-0x20) = *(int*)(ebp-0x3C)` (current counter); `CreateItemEntry(panel, cobj, counter, list, item, 0, 0)`; row = `((void**)*(panel+0x24B0))[counterBefore]`; if `!usable` then `SetCanUse(row, 2)` (nothing for usable rows). Clear the buffer. The equipped block stays vanilla (equipped first).

**Re-click cycle (H6a).** State = `(GuiInGame[0xFC1], g_upgradeActive)`. Let `clicked = *(int*)(ctrl+0x5C)`.
- If `clicked == 0` **and** state ∈ {All, New (mode 1), Usable (mode 5), Upgrade (mode 0 + flag)}: advance All→New→Usable→Upgrade→All using `SetNextFilter(panel, 1 | 5 | 0)` (Upgrade = engine mode 0 + `g_upgradeActive=true`); set `LBL_FILTER` to `Filter: New` / `Filter: Usable` / `Filter: Upgradeable` (or `Filter` for All); return 1 (consumed). Modes 1 and 5 leave all 7 tabs unlit (engine behaviour); acceptable.
- Otherwise clear `g_upgradeActive`, restore the caption `Filter`, return 0 (vanilla handles the click).
- Any panel repopulate that changes the mode from elsewhere (controller bumpers use `SetNextFilter`) must also clear the flag: cheap guard in H6b (`if mode byte != 0 → g_upgradeActive=false`).

### 4. Data / config
No 2DA/TLK/script changes. Captions are C strings set by the DLL. INI (`inventory_sort_filter.ini`, optional): `sort=1`, `tint=1`, `cycle=1`, `cycle_states=new,usable,upgrade`.

### 5. Build, staged test plan, rollback
- Stage 0 (logging only): H6a/H6b/H6c log mode, clicked mode, buffered count, sort keys; behaviour unchanged. **As built, rows always go through the buffer (created at H6c in vanilla order), so Stage 0 deliberately exercises the splice + flush path** rather than creating rows inside H6b.
- Stage 1 sort only → Stage 2 tint → Stage 3 cycle (New, Usable) → Stage 4 upgrade filter. Log every stage with the memory-dump style used in Mods 2-5.
- Rollback: delete the hooks from `patch_config.toml` / disable in the INI (`sort=0 tint=0 cycle=0` = vanilla order through the buffer; true rollback = remove the patch id). No save-game side effects (no data written to items).

### 6. Remaining risks and in-game checks
1. **Whose "usable"?** Confirm the tint follows the character whose equipment is shown when switching party members (check 1).
2. `GetUpgradeType` slot counts assume `type 1 = 6`; slot 1 of type 1 is excluded in the predicate (above). Verify a single-bladed saber, a double saber and an armor show the right "has empty slot" result (check 2).
3. **Tint is lost when the row is entered/selected:** `8A83D0` clears the New bit and `8B1CC0` re-runs `SetItem`, which resets the colours. Test hover/click on a tinted row; if it matters, re-apply `SetCanUse(row,2)` after that path (not in v1). Clearing New this way also moves the item out of the "new first" group at the next repopulate. Rows are reused only across repopulates while the panel is open (closing destroys them), so test tab switches, not open/close.
4. **"Usable" is not one predicate:** tint and sort use `IsItemUsable` (what the popup uses); the cycle's Usable state is engine mode 5, which also requires `CanUseItem` and a different ECX (`0x58AE90(creature)`), so an item can be untinted yet hidden by `Filter: Usable`. Decide in Stage 3 whether to call it "Usable" or add `CanUseItem` to the tint. The Upgrade filter drops backpack rows only; equipped rows still show.
5. H6b's window is large (79 bytes). If the framework rejects that `original_bytes` length, split it at instruction boundaries into two `skip_original_bytes=true` windows (add them to `check_windows.py` and re-validate; the internal `JNZ` at `0x8A7E3A` targets `0x8A7E52`, so split there) with the same handler state.
6. **Name tie-break (shipped in WI-21, C132, `c24be8e`; not yet seen in game):** within one base item, rows sort by localised name, ASCII case-insensitive (`foldCmp`, `SortLogic.h`). An empty name (failed `GetString` lookup) sorts before any non-empty name. Equal names fall to vanilla order.
7. **`GetString 0x7356B0` side effects (REA 20261015, verdict UNSURE):** the call inserts into the CExoLocString cache, sets the container `+8` dirty flag (consumer unknown), and writes gender into the talk manager at `+0x3C`. Check these before relying on the name key in a live session. Since WI-146, only rows tied on new/usable/base item read a name (`names=` in the sort timing line). **WI-149 (REA 2026-10-25, ledger `20261025_getstring-gender-callers.tsv`):** the gender write cannot leak. FUN_0071df90 stores TalkTable `+0x3C` right before every Fetch, and Fetch (0x71d970) clears it to 0. All item/UI callers (36 callers, 47 sites) pass constant 0, including the inventory-name path 0x8bf54f; only the dialogue/speaker code passes a real gender. Keep 0, so the sort name matches the displayed name.


## Build status (2026-09-30)

Source: `inventory_sort_filter_src/` (id `inventory-sort-filter`, log `inventory_sort_filter_log.txt`, INI `inventory_sort_filter.ini`; build copy
`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/InventorySortFilter/`). One DLL carries all stages; the INI selects them
(`sort`, `tint`, `cycle`, `cycle_states`, all default 0 = Stage 0 logging only, re-read at most once per second). Pure logic (sort comparator, cycle state machine,
open-slot predicate, INI parsing) is in `SortLogic.h` with host tests. Implementation notes vs the plan above:
- Rows always go through the buffer (even at Stage 0), so Stage 0 also proves the splice/flush path; order is unchanged unless `sort=1`.
- `g_upgrade` (Upgradeable filter) is reset when the panel pointer changes; a reopen at the same address with the flag still set is an open risk (in-game check 6).
- Stale-buffer guard: a new populate is detected by the loop index (`ebp-0x14`) not increasing.

- Review (independent Opus, 2026-09-30) applied: INI path made absolute from the cwd (a bare name goes to the Windows dir), `normaliseState()` rebuilds the upgrade flag/caption from the engine mode at every populate (bumper / reopen / `cycle=0` cannot leave stale state), overflow emits the buffer in order, unknown filter mode reads as 0xFF (never "All"), `Entry.id` stored, buffer 1024. No blockers found.
- Known UX point to confirm in-game: from New/Usable a click on All moves to the next state (3 clicks back to All with the default states).
- **Install status: Stage 0 INSTALLED 2026-09-30 11:07** (game closed, all 19 live ids re-staged from the newest matching kpatch, md5 of all DLLs verified, 66 hooks = 63 + 3, `AR_ERROR.LOG` unchanged). Backup `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups_mod6s0_1107`; stage dir `/home/ethansm/Games/KOTOR2_Mod_Workspace/stage_mod6s0_1107`. `inventory_sort_filter.ini` (all flags 0) copied to the game folder. Awaiting the user's in-game Stage 0 run: open the inventory, click tabs (All/Weapons/...), switch party member, close/reopen; then review `inventory_sort_filter_log.txt` and the Proton log (`Parsed consumed_exit_address`, `Applied DETOUR hook at 0x8A8CF0/0x8A7E22/0x8A7E76`).

- **Pending for the next build (from sibling review `11_inventory_sort_filter_feedback.md`, verified 2026-09-30):** (1) `itemHasOpenSlot` should default failed reads to `-1` (fail-open: show the item) instead of `0`; (2) stale `g_upgrade` on panel reopen at the same address is real (same as open question A4c) - confirm from the S0 log (`populate: ... upgrade=`) and at `cycle=1` in game, then decide between hooking `OnPanelRemoved 0x8A7520` and a cheaper freshness test; (3) NOT adopted: clearing `PanelSelectedIdOff` when the selected item is dropped by the upgrade filter (vanilla filtering leaves the same stale id and falls back to row 0).

- **Stage 0 in-game result (2026-09-30 13:24, user: inventory "functioned as normal")**: log review OK. Proton log: `Parsed consumed_exit_address = 0x008A8DE2`, `Applied DETOUR` at `0x8A8CF0/0x8A7E22/0x8A7E76`, `AR_ERROR.LOG` unchanged. Config line `sort=0 tint=0 cycle=0`, INI path = game folder. All tab reports `clicked=0`; other tabs `clicked=6..11` = engine modes 6..11. Populate/flush counters consistent (`seen == rows`, `dropped=0`, no counter/frame errors). `creation order` = `0..66` for All (vanilla order preserved) and ascending original backpack indices for every filtered tab (so the vanilla loop index equals the backpack index). 24 of 67 rows `usable=1`. **Not yet exercised in this run:** party-member switch, close/reopen at the same panel address, picking up an item (no populate after the last tab click).

- **Stage 0 run 2 (2026-09-30, user: tabs, party switches, open/close all fine; log session 2, 40 populates, `AR_ERROR.LOG` unchanged, no errors):**
  - **The engine does NOT reset the filter mode on close/reopen or party switch**: mode 6 and mode 9 stayed active across reopen and across character changes (populate via `ebp=..ec48/..ec18` paths, `mode=` unchanged). Panel address stayed `072d3050` for the whole session (only one "panel instance" line). So the sibling review's "engine reinitialises mode to 0 on reopen" is wrong: filter persistence is vanilla behaviour. Consequence for Upgradeable: the flag staying set on reopen is *consistent* with vanilla persistence as long as the caption is re-applied (`normaliseState` does that) - so the stale-flag issue is downgraded from bug to "verify at cycle=1 that reopening shows Upgradeable with the right caption" and the `OnPanelRemoved` hook stays deferred (A4c).
  - Party switches were visible as different `rows-so-far` (5, 8, 10 equipped rows) with the same 67 backpack rows, `new` count dropped 46 -> 42 -> 41 (New flags cleared on panel removal, as expected).
  - **OPEN, important for S2 (tint):** the `usable` column was identical (same 8 of 67 rows, same per-row signature) for all three characters, and **zero weapon/armor rows were usable for anyone** (modes 6/7/8 = 0 usable; mode 9 = 7/7; mode 11 = 1). Either this save's party genuinely cannot use those items, or `IsItemUsableByCurrent 0x77B670` with `(item, 0, 0xFF)` does not track the shown character. Decide with a tint=1 look at a character who certainly can equip some backpack weapon/armor (and compare with the vanilla NotUseable/CantEquip popup on row click).
  - Log hygiene: the game-folder log appends across sessions and is ~3 KB per populate with `log_items=1`; add truncate-on-load / cap before the final build.

## 0.2.0 build (2026-09-30 night): feature-complete, not installed

- **Tint / Usable predicate replaced.** `0x77B670` only says "has an activatable property". 0.2.0 `canEquip(creature, item)` copies the equipment-slot list `0x8AC330`: baseitem `0x6D6E30(ECX=item)`; slot mask `base+4` (0 => not equippable, never tinted); gender `base+0xB4` 1/2 vs `*(u16*)(stats+0xE0)` == `*(u16*)0x99A8BC` / `0x99A8B8`; then `0x5BD6A0(ECX=creature; item, &slotBit, 0,0,0)` RET 0x14 (AL) for each **single** slot bit until one passes (it dispatches on single values; params 4/6 = 0 => no feedback message, pure query). Creature = Populate `[ebp-0x40]` (shown character; S0 log shows it changes on party switch). **Tint** = equippable AND NOT canEquip. **Sort key "usable"** and the **Usable cycle state** = canEquip OR activatable, so filter, sort and tint agree.
- **Usable cycle state** no longer uses engine mode 5: it is engine mode 0 + our filter (`isf::X_Usable`), like Upgradeable (`X_Upgrade`); backpack rows only (equipped rows always show). Engine mode 5 set by anything else is not a cycle state (a click on All goes vanilla).
- **Reopen (A3):** kept `g_extra` persistence (vanilla keeps the filter mode across close/reopen and party switches); `normaliseState` re-applies the caption on every populate and clears the extra filter only when the engine mode is not 0 or `cycle=0`. No `OnPanelRemoved` hook.
- **Tint on row select (A4):** `0x8A83D0` (row select) re-runs `SetItem` via `0x8B1CC0` **only for New items**, which resets the colours. New hook **H6d `0x8A8478`** (epilogue `MOV ESP,EBP; POP EBP; RET 4`, all paths end there; not consumed) re-applies `SetCanUse(row,2)` for rows tinted by the last populate (row-pointer list, cleared when backpack rows are rebuilt). Validated in `check_windows_output_2026-09-30e.txt`.
- **Pending fixes applied:** `itemHasOpenSlot` failed reads default `-1` (fail-open); game-folder log truncated in `DllMain` + cap marker; `log_items` default 0.
- **Shipped INI:** `sort=1 tint=1 cycle=1 cycle_states=new,usable,upgrade log=1 log_items=0`. Rollback: a flag to 0 (hot reload) or remove the id.
- Build: host tests ALL PASS, mingw `-Wall -Wextra` clean, kpatch md5 `9b3d5520` (DLL `17f6874e`), exports == 4 hook functions. Opus review: see below.

### 0.2.0 Opus review (2026-09-30 night)
- **BLOCKER fixed:** `0x8A83D0` is also the equipment screen's row handler (`0x8B01B0` registers it), and inventory rows freed on close (`0x8A7520`) can be reallocated there; a stale re-tint would set `row+0x3B0=2`, which the equipment click handler `0x8AF2D0` treats as "cannot use". H6d now acts only when the handler's panel (`[ebp-0x18]`) is the panel we tinted and the row is still in its live `+0x24B0` array. Rebuilt kpatch md5 `8fdf18cb`.
- NITs: set-2 weapon bits are tested as `0x40000/0x80000` (engine list ORs the set-1 bit; same result in practice); item level requirement is not checked (same as the equipment list); **the install must overwrite the Stage 0 INI in the game folder** (code defaults are 0).
- Verified: canEquip ABI/creature/pure query, state machine, H6d window + wrapper RET, log hygiene, build contents.

### 0.2.0 installed (2026-09-30 23:57)
Live with Mod 8 in one combined install (22 ids, kpatch md5 `8fdf18cb`); `$G/inventory_sort_filter.ini` = `sort=1 tint=1 cycle=1 log=1 log_items=0`. Backup/stage/rollback: `mods678_research/HANDOFF_2026-09-30.md` "INSTALLED". Awaiting the user's play session (checklist in the HANDOFF), then the log review.

### Play-session status (2026-10-02)
The 10-01 22:13 session did not open the inventory: the Mod 6 checklist is still open (detours applied, config `sort=1 tint=1 cycle=1` loaded). Live together with the companion AI round since 10-01 23:36.

### v2 research (2026-10-02, static)
`mods678_research/ai_research_2026-10-02/R4_medpac_mod6v2.md`:
- **R14 name sort:** the sort works on server items (`0x606100`). The tie-break key is the server item's loc string `+0x2C0` via `CExoLocString::GetString 0x7356B0` (bool return, `CExoString* out`). REA 20261015 verdict: **UNSURE** for safe binding; side effects are listed in risk 7. `CSWCItem::GetName 0x7FDFA0` (`thiscall(clientItem; CExoString* out, int identified)`, `RET 8`; client item via `0x73F530`) was planned as a fallback and is **NOT implemented**. Estimated cost was 0.1-0.3 ms per populate; the per-flush sort timing line in the WI-21 live check is the measurement (pending).
- **Level-requirement tint:** unreachable with the current `0x5BD6A0(...,0,0,0)` call (`0x5BD6AA` gate). It needs `p4=1, p5=0, p6=0`, applies to the PC only, and also needs a module flag `[..+0xD0]` of unknown value. Log that flag before building anything.


### WI-21 build (C132 only, 2026-10-15)

- Name tie-break built (`c24be8e`): host tests, `compat_check` 63/0 and `build_check` pass. Not yet tried in game; queued in [`PLAY_TEST_QUEUE.md`](PLAY_TEST_QUEUE.md) as WI-21. No game install.

### Perf follow-ups (review 2026-10-25, WI-145..WI-150; WI-145..147 LIVE in 0.2.2; WI-148..150 LIVE in 0.2.3 2026-10-25 19:53)

- **WI-145 (`9b8e467c`):** the sort timing line now also reports the buffer-phase total and the name-read total: `sort timing: sortEntries rows=N took X us; buffer B us (names M us) names=K`.
- **WI-146 (`09f4ff02`):** names are read at flush, only for rows tied on (new, usable, base item) (`isf::fillTiedNames`), and the client language is fetched once per populate. Same order as reading every name (host test).
- **WI-147 (`1391e39f`):** the INI is re-read only when its last-write time changes. A missing file is read once (defaults).
- **WI-148 (`94bcc9b8`):** a populate that never reached SortFilterFlush no longer leaves rows behind. The buffer restarts when the loop index does not advance, the engine row counter goes backwards, or the populate frame (ebp) changes. A new panel instance also clears it. The discard line names the case: `stale buffer of N rows discarded (reason=idx|counter|ebp ...)`. **Live 2026-10-27:** F_Counter does not start at 0. It reads 6 or 9 at populate start (per creature) and is unchanged at flush, so it is not a per-row count. The `counter` reason almost never fires; `idx` and `ebp` do the work. No false discards in 5 flushes (ebp moved between populates, 00dbec18 -> 00dbecd8).
- **WI-149 (`41b32404`, verify-only):** the name read keeps gender 0 (see risk 7).
- **WI-150 (`95bebe72`):** logging keeps the current session plus one previous (`inventory_sort_filter_log.txt.1`). At attach, the log moves to `.1` if it is older than 60 s or larger than 256 KB, so a launcher+game pair stays in one file. 0.2.1 (`7788c58d`) dropped the old truncate without a stated reason.
- Live checks ISF-T1..T3 are in [`PLAY_TEST_QUEUE.md`](PLAY_TEST_QUEUE.md). The live log (2026-10-25) showed one 86-row flush with the sort taking 14 us, before these changes. **0.2.3 live (2026-10-27, ISF-T1 PASS):** rows 98-106 per flush. Sort 15-25 us, buffer phase 634-1007 us, name reads 41-74 us with names=68-73 (about 70% of rows tie on new/usable/base item, so WI-146 saves only about 30% of name reads). The cost is canEquip/IsUsable, about 1 ms per open, once per open (not per frame). ISF-T5 PASS: the log rotated to `.1` between two sessions.

### UX wave vw2-A (2026-10-27, WI-152..WI-155; built, not installed)

- **WI-152 (`31c44026`):** a filtered caption shows its place in the enabled cycle states and the row count, e.g. `Filter: Usable 2/3 (14)` (owner choice: cycle position). No `n/m` part when only one state is enabled. Longest text is 30 chars (`kCaptionBytes` 32). Play row ISF-T11 checks that the label does not clip it.
- **WI-154 (`fd958494`):** `[Inventory] sort_order = category|base|name|vanilla` (or digits 0..3, default category). Category order (owner-approved): melee, ranged, armour/robes, other gear (head/hands/arms/belt/implant), upgrades (baseitems rows 11 and 101, from `.claude/research/k1_items/baseitems_k2.tsv`), consumables, misc/quest. The category comes from GetUpgradeType, the slot mask and IsItemUsable. `fillTiedNames` follows the active order's tie key. Risk: a weapon or armour that GetUpgradeType reports as 0 lands in "other gear" (ISF-T9).
- **WI-155 (`d3f1d4e2`):** one Mod Configuration row, "Sort order", that writes the index to `sort_order` (`SortOrderPage.h`, `KPatchInit` export). Registration is lazy, from the inventory hooks, so mod-config must be listed before inventory-sort-filter in the load order. The page shows a hand-set word value correctly (ISF-T10).
- **WI-153 (`65d5313c`):** controller bumpers join the cycle. H6e 0x8A8997 (input 0x2F, prev) and H6f 0x8A89F7 (input 0x30, next) are 9-byte skip-only windows over PUSH/MOV/CALL SetNextFilter in `CSWGuiInGameInventory::HandleInputEvent` 0x8A8880. `isf::bumperNext` makes the ring All -> New -> Usable -> Upgradeable -> 6..11; with the cycle off it passes the vanilla ring through. Which physical bumper is 0x2F is unverified: every press logs `bumper: input=...` (ISF-T8). Recon: `.claude/research/isf_controller_filter.md`.

### UX wave vw3 (2026-10-28/29): visible sort + filter state (mockup variant v2)

Owner verdict 10-28 on 0.3.1-dev: "no indicator of what is going on or that it's sorted or filtered; the caption strip piece is broken".
Mockup `scripts/isf_inventory_mockup.py --variant all OUTDIR` (`.venv/bin/python`; real inventory_p gui, dialogfont, icons; commits
9b0ecdc6, 4a87eb83) measured it: LBL_FILTER is 345 px, every filtered caption overflows into the All tab ("Filter: New 1/3 (4)" 348 px,
"Filter: Usable 2/3 (13)" 420, "Filter: Upgradeable 3/3 (3)" 514, worst "(1024)" 576), so the WI-152 "not clipped" claim is wrong if the
mock's font scale is right (VU0 calibrates it). Also: ISF "Usable" clashes with vanilla's own "Useable" tab; sort order shown nowhere;
category groups not perceivable (6 rows visible); tint and New have no legend. Owner picked **v2** ("continue" after the v2 recommendation):
- LBL_FILTER = short caption that fits: "Filter" in All, else the filter name only ("New", "Can Equip", "Upgradeable"); Usable renamed **Can Equip**.
- Free band B (gui 657..3676 x 1388..1452 px @3840x1600, under LBL_BAR3/4, right of BTN_EXIT): segmented filter buttons
  "All 22 | New 4 | Can Equip 13 | Upgradeable 3" (active lit) + status "Showing: Can Equip 13 of 22" + legend (red-orange swatch "can't equip", New marker "new").
- Free band A (1319..1953 x 1262..1336, right of BTN_USEITEM): button "Sort: Category", a click cycles Category -> Base item -> Name -> Vanilla (writes `sort_order`).
- Re-clicking All and the bumpers keep working.
- **Owner 10-29: "I really like the in list clues UX"** -> v3 category divider rows are IN scope (VU6), not deferred.
- **Owner 10-29: "we need a better UX for the top bar, its too small"** -> the top tab strip goes back to pure vanilla (LBL_FILTER always
  "Filter"; even "Upgradeable (1024)" is 358 px > 345). ISF state moves to its own bar: mockup v4a (framed bar in bottom band B + A) vs v4b
  (framed strip above LB_ITEMS, list one row shorter); owner picks from `isf_v4_compare.png`. VU1 is replaced by that choice.
- **Owner 10-29: "v4b, I like the new strip being above"** -> ISF strip directly above LB_ITEMS (LB_ITEMS 357..1234 px split into a 133 px
  strip at 357 + items 490..1234; window 5 rows, 4 at the top with a divider): line 1 filter buttons with counts, line 2 "Sort: <order>"
  + "N of M shown"; legend as a quiet line in band B. Needs LB_ITEMS resized at panel build (VU2 (d) open).
- **Owner 10-29: rule A + tile 3 default.** Category sort key becomes new -> category -> usable -> base item -> name (usable-first INSIDE each
  category, so each divider appears once; New stays the first group). Default list look: Row height Compact (96 px @3840x1600), Icon
  88 px (vanilla 125 clamped to the row), List font Large (dialogfont16x16), Category dividers On; Small icon (64 px) dropped (choices
  Vanilla / Large). Both land with VU6 / VU7; SortLogic.h beforeBy(SO_Category) + sort_logic_test vectors + the mockup port change together.
- Config-page mockup (`scripts/isf_config_page_mockup.py`, 33ba911e): duplicate dividers ("Armour" twice: cannot-equip rows sort after
  usable ones, so categories restart) -> owner to pick rule A (usable-first inside each category) or B (one "Can't equip" divider at the
  end); Small icon (64 px) unreadable -> drop it; suggested default = Compact rows 96 px + icon 88 px + Large font + dividers on (4 items visible).

Engine facts (10-29, static): labels and button clones exist only while the panel's layout GFF is open (`GuiKit.h:176`; `GuiButton.h:55`
createButtonFromTemplate, used by mod-config). hud-host already detours StopLoadFromLayout 0x40F5A0 for every panel ctor but dispatches
K2HUD_BUILD only for the HUD vtable (`HudHostProvider.cpp:204`, `HhDispatch.h:85`). inventory-3d-viewport owns the inventory ctor hook
0x8A69AD and the inventory Update prologue 0x8A7350 (`InventoryTick`). Mod clicks elsewhere are polled release edges + cursor hit test per
frame (mui::ClickTracker, gui-input CursorEngine), so the buttons need a per-frame tick while the inventory is open.

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **VU0** | Calibrate the mockup on one owner screenshot (inventory open, cycle on Usable): font scale, New colour, tab highlight, does the engine clip LBL_FILTER | - | owner screenshot | TODO | - | owner sends a screenshot path |
| **VU1** | SUPERSEDED 10-29 (top bar back to vanilla; ISF bar VU4). Was: quick fix 0.3.2-dev (hot-swap): short captions + "Can Equip" rename in SortLogic.h captionWithCount (drop " P/N (n)"; counts move to the buttons); sort_logic_test vectors; mockup `current` = new captions; ISF-T11 rewritten | - | install GO | SKIPPED | - | superseded by the own-bar decision |
| **VU2** | Static RE ($0 first): (a) per-frame tick while the inventory is open without a second detour on 0x8A7350 (KPM one detour per address? alternative site or an inv3d/hud-host provider callback); (b) does a button cloned from a BTN_* tab template keep the event-0x27 binding to the tab handler 0x8A8CF0 (then ISF's existing SortFilterTabClick sees the click, ctrl+0x5C = our id, no tick needed); (c) how to add a non-item divider row to LB_ITEMS (protoitem clone in the list box control list vs a CreateItemEntry variant), and keep selection index, reselect, tint indices (PanelRowArrayOff) and keyboard / controller navigation skipping it; (d) for v4b: can ISF resize LB_ITEMS at panel build (guikit::setRect) without breaking its scrollbar | - | - | PARTIAL | - | (a) buttons: clone + AddEvent 0x418AF0(0x27, panel, 0x8A8CF0) in the ctor, clicks reach H6a, no tick; (b)-(d) LB_ITEMS +0x588, rows = CSWGuiInGameItemEntry 0x8B0A60 (not protoitem), CIE derefs item (divider needs own path); 0x8A7350 is Draw. Open: Draw rebuild gate, 0x8B0A60 font/icon/height. Findings .claude/research/isf_vu2/FINDINGS.md |
| **VU3** | hud-host provider: new phase K2HUD_PANEL_BUILD fired from HH_LayoutClosing for EVERY panel (ctx.panel + panel vtable), additive API, consumers filter by vtable; existing K2HUD_BUILD unchanged; hh_dispatch_test + common_contract_test pins | VU2 | install GO (hot-swap) | TODO | - | compat_check.py on _shared_src edits |
| **VU4** | ISF 0.4.0-dev own strip above LB_ITEMS (v4b, owner pick 10-29), labels first: subscribe to K2HUD_PANEL_BUILD for the inventory vtable, clone LBL_* templates into bands A/B: status line + legend + read-only filter / sort indicators, refreshed at each populate (no tick needed) | VU1, VU3 | install GO | TODO | - | play row ISF-T13 |
| **VU5** | ISF 0.5.0-dev buttons: segmented filter + Sort button clickable (path from VU2), keyboard / controller unchanged | VU2, VU4 | install GO | TODO | - | play row ISF-T14 |
| **VU6** | ISF 0.6.0-dev in-list cues: non-selectable divider rows ("New", "Weapons", "Armour", "Gear", "Upgrades", "Consumables", "Other") in Category sort, from the VU2(c) path; host test for the row-index mapping | VU2, VU5 | install GO | TODO | - | play row ISF-T15 (selection, scroll, controller) |
| **VU7** | List look configurable (owner 10-29: "make the list UX configurable through a mod config menu, with a mock inventory list to show what the changes would render like; icon/font size, row height etc"): page rows Row height, Icon size, List font (10x10 / 16x16), Category dividers on/off; a live preview mini list on the page (DeclHooks onBuilt + AddCustom, holster-editor precedent); applied at the next inventory build. Mockup of the page + inventory at several settings first | VU2 (b)/(c), VU4 | owner mockup pick, install GO | TODO | - | mockup job queued after the v4 render |

## Planned shared state and shared code (doc 19 / 19b, added 2026-10-02)
**Nothing here is built yet.** When a piece lands, use it instead of extending this patch's local copy. The ids (A1, B2, ...) refer to [`19b_cross_patch_sharing_survey.md`](19b_cross_patch_sharing_survey.md), which has the evidence. Line numbers there are from 10-02.
- **Registry interfaces** (A-ids) use the RFC #180 registry from [`19_shared_logging_design.md`](19_shared_logging_design.md). Call `require` lazily at first use, and keep the current local code as the fallback when the provider is not installed.
- **Common helpers** (B-ids) live in `Patches/Common` on the KPM fork. Edit them in `patch_manager_mods/_shared_src/` and sync with `scripts/check_shared_headers.py --sync`. New code includes `"SafeRead.h"`, not `"../_shared/SafeRead.h"`.
- **Rollout:** shared pieces are consolidated on our fork first. Each one goes upstream inside the first mod PR that uses it, so order this mod's PR after the PR that introduces anything it reuses. Migrating this patch needs its owner's go.
- **Logging:** move to `KpLog.h` channels and `kplog.ini` (doc 19 Phase 6). Keep the tag text exactly as it is, because the parsers depend on it.
- **B2 GuiKit:** `setLabelText` (:142).
- **B3 ini helper:** loadIni (:113).
