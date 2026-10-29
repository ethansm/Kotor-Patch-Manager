# ISF VU2 static RE: inventory panel buttons, list rows, per-frame (2026-10-29)

Haiku recon (static, $0; dump_excerpts + static_facts + name_map), report persisted by Opus. Opus spot-checked (a) 008a6170.asm:499-508 and
(d) 008a75f0.asm:347-352: both CONFIRMED.

## Correction
0x8A7350 is `CSWGuiInGameInventory::Draw` (name_map steam-db-B0, GOG 0x0059B250), not "Update". inventory-3d-viewport's H3 hook
(`InventoryTick`) and doc 11 call it Update. Same address, wrong label.

## (a) Tab click binding: CONFIRMED storage, PARTIAL clone
- Per control: the ctor calls `CSWGuiControl::AddEvent 0x418AF0` (thiscall, RET 12), ECX = control, args (0x27, panel, 0x8A8CF0).
  008a6170.asm:501-508 (BTN_ALL at panel+0x1800), repeated at :536/:571/:606/:641/:676/:711.
- Tabs inline in the panel: ALL +0x1800 (mode 0), DATAPADS +0x19D0 (6), WEAPONS +0x1BA0 (7), ARMOR +0x1D70 (8), USEABLE +0x1F40 (9),
  QUESTS +0x22E0 (10), MISC +0x2110 (11). Mode = ctrl+0x5C.
- `createButtonFromTemplate` (_shared_src/GuiButton.h ~:55-85) copies no event table, so a clone needs its own AddEvent (ECX = clone;
  push 0x8A8CF0, panel, 0x27; CALL 0x418AF0) + clone+0x5C = our id, inside the ctor. Then ISF's existing H6a tab-click detour sees the
  click: NO per-frame tick needed for the buttons. Caveat: the handler un-highlights only the 7 fixed tabs, so ISF must highlight its own.

## (b) LB_ITEMS: CONFIRMED offsets, UNSURE row height
- Inline at panel+0x588, InitControl 0x40F620 (008a6170.asm:155-158, 740-764). CreateItemEntry reads LB+0x2A4 (dword) and LB+0x2D0
  (byte); width arg = LB[+0x2A4] - 2*LB[+0x2D0].
- Rows are NOT protoitem clones: built by `CSWGuiInGameItemEntry` ctor 0x8B0A60 (CSWGuiBorder 0x414A50 + CSWGuiText 0x4171A0).
- Scrollbar: UNSURE.

## (c) CreateItemEntry 0x8A75F0: CONFIRMED mechanics, UNSURE font/icon/height
- Pool: if counter == [panel+0x24B4], new 0x3B4-byte row (008A7649) + ctor 0x8B0A60, appended to pool panel+0x24B0 via 0x83EA60; else
  reuse pool[counter]. Then row vt+0xA0(vt58+0x160, vt58+0x70, vt58+0xE8, width, 1), vt58 = vt+0x58 of *(panel+0x864) (rect inputs:
  likely where row height comes from). Then events: hover 0x8A8100, select 0x8A83D0, click 0x27; SetItem 0x8B1250; row vt+0x40(0);
  append to param_4 list (0x83EA60); counter += 1.
- Font, text size, icon size: not set in CIE; must be in 0x8B0A60 or SetItem (no decompile in repo).

## (d) Divider rows: CONFIRMED null-item deref, PARTIAL recipe
- CIE cannot take a NULL item: item+0x2C8 at 008a7b22, item+0x14 at 008a7b37, IsItemUsable(item) at 008A7785, unchecked.
- A divider needs its own path: ctor 0x8B0A60, append to pool (0x83EA60 on panel+0x24B0), append to the list, counter += 1, skip
  SetItem / SetCanUse; otherwise OnPanelRemoved 0x8A7520 (frees pool rows) leaks it.
- Index logic: reselect (008a7ba0.c local_24; engine map -0x20): if panel+0x24C8 == item+0x14 then reselect = counter. A divider that does
  not bump the counter desyncs counter, pool and reselect. ISF's tint indices (PanelRowArrayOff) shift too.
- Keyboard / controller skipping a divider: UNSURE, not analysed.

## (e) Per-frame: PARTIAL
- Draw 0x8A7350 callees: CSWGuiPanel::Draw 0x40EC90 (generic, 27 callers), PopulateItemListBox 0x8A7BA0, 0x623320 (name suspicious),
  creature lookups. PopulateItemListBox callers: OnPanelAdded 0x8A74C0, Draw 0x8A7350, 0x7D1FE0. Draw is reached by vtable only.
- Open: does Draw repopulate every frame, or only when panel+0x24D0 bit 2 is set (set by 0x8A8CF0 / 0x8A8B30)? Needs Draw's asm.
- Do not add a second detour on 0x8A7350 (inventory-3d-viewport H3).

## Implications
1. Existing tabs need no re-bind; H6a on 0x8A8CF0 already sees them.
2. ISF buttons = clone in the ctor + AddEvent(0x27, panel, 0x8A8CF0) + clone+0x5C id: clicks arrive through H6a, no tick.
3. Dividers: follow the pool + counter rules in (d); host-test the index mapping.
4. VU7 knobs (row height, icon size, font): decompile 0x8B0A60, SetItem 0x8B1250 and the vt58 rect object at *(panel+0x864) first.
5. Next free step: named_decomp / knowledge-DB for 0x8A7350, 0x8B0A60, 0x418AF0, 0x8B1250 before any REA call.
