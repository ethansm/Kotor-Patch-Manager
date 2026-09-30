# Mod Idea 6: Inventory Sorting & Sub-Filters

**STATUS (2026-09-30, research complete; Stage 0 code built, not installed - see 'Build status' at the end):** all engine questions are answered; this file ends with the
**definitive build plan**. Evidence: `mods678_research/dump_excerpts/<addr>.{c,asm}` (decompile + disassembly of every cited
function), `mods678_research/check_windows_output_2026-09-30.txt` (splice validation), knowledge DB rows tagged
`mods678 batch3` (`functions`, `name_map` source `mods678-batch3-dump`, lessons 171-178).
Ships as its **own patch id `inventory-sort-filter`** (`Patches/InventorySortFilter`; decided in the 2026-09-30 build plan; same panel as Mod 1 but Mod 1 hooks `0x8A69AD`, `0x8A8172`, `0x8A7350` do not overlap).

## Goal

1. **Sort:** new items first, then usable-by-this-character, then grouped by base item; unusable rows tinted.
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
- [x] **R14** name getter is `CSWCItem::GetName 0x7FDFA0` (`thiscall(clientItem; CExoString* out, identified)` `RET 8`, string at `+0x18C`). **v1 does not sort by name** (base item id groups identical kinds); name sort is a later stage.

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

H6c, once, before the finalise call: stable-sort by `isNew desc, usable desc, baseItem asc, origIndex asc`; for each entry: if its id equals the remembered id ⇒ `*(int*)(ebp-0x20) = *(int*)(ebp-0x3C)` (current counter); `CreateItemEntry(panel, cobj, counter, list, item, 0, 0)`; row = `((void**)*(panel+0x24B0))[counterBefore]`; if `!usable` then `SetCanUse(row, 2)` (nothing for usable rows). Clear the buffer. The equipped block stays vanilla (equipped first).

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
6. Alphabetical name sort is not in v1.


## Build status (2026-09-30)

Source: `inventory_sort_filter_src/` (id `inventory-sort-filter`, log `inventory_sort_filter_log.txt`, INI `inventory_sort_filter.ini`; build copy
`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/InventorySortFilter/`). One DLL carries all stages; the INI selects them
(`sort`, `tint`, `cycle`, `cycle_states`, all default 0 = Stage 0 logging only, re-read at most once per second). Pure logic (sort comparator, cycle state machine,
open-slot predicate, INI parsing) is in `SortLogic.h` with host tests. Implementation notes vs the plan above:
- Rows always go through the buffer (even at Stage 0), so Stage 0 also proves the splice/flush path; order is unchanged unless `sort=1`.
- `g_upgrade` (Upgradeable filter) is reset when the panel pointer changes; a reopen at the same address with the flag still set is an open risk (in-game check 6).
- Stale-buffer guard: a new populate is detected by the loop index (`ebp-0x14`) not increasing.
- Install status: NOT installed.
- Review (independent Opus, 2026-09-30) applied: INI path made absolute from the cwd (a bare name goes to the Windows dir), `normaliseState()` rebuilds the upgrade flag/caption from the engine mode at every populate (bumper / reopen / `cycle=0` cannot leave stale state), overflow emits the buffer in order, unknown filter mode reads as 0xFF (never "All"), `Entry.id` stored, buffer 1024. No blockers found.
- Known UX point to confirm in-game: from New/Usable a click on All moves to the next state (3 clicks back to All with the default states).
