# R3: server-side container take path (Steam swkotor2, static RE, $0)

Sources: decomp/asm dumps under ghidra_dumps/steam_6a522e71/decomp, static_facts.py, cg.py, query_ghidra.py, objdump on the Steam exe (.rdata bytes). No REA/MCP. Confidence: H/M/L.

## 1. Message handler (GuiContainer, major 0x19)
- Dispatcher `0x0065f8a0` (1449 B, RET 12), `case '\x19'` label "GuiContainer", calls `0x0065afd0` at `0x0065fb5c` (decomp/65/0065f8a0.c:113). H
- Handler `0x0065afd0`: 289 B, RET 8, __thiscall (ECX = CSWSMessage this). [EBP+8] = objid, [EBP+0xC] = minor byte. H
- Asm `0x0065afdd` `CMP EAX,2 / JNZ 0x0065b0e6`: every minor except 2 returns 1 with no action. H
- Minor 2 only. Objid resolved via GetGameObject `0x53dfb0`; type byte at +8 decides target:
  - type 9 (`DAT 0x994478`) -> vtbl+0x48 -> CSWSPlaceable::CloseInventory `0x005f5770`
  - type 6 (`DAT 0x994475`) -> vtbl+0x38 -> CSWSItem::CloseInventory `0x00606460`. H
- GOG twin `0x800d70` (289 B, RET 8): upstream_server swkotor2.exe_functions.tsv row 8744. Size, RET and dispatcher case match. Code bytes are not on disk, so not byte-verified. M-H (~85%)
- Correction to brief: this handler has NO take-item or take-all cases. Its only case is close.
- Checked, not take routes: GuiInventory 0x0d -> `0x65af60` (102 B, minor 1 only). Inventory 0x0c -> `0x65e670` (3973 B), no direct call to 0x5f4ed0, 0x5f5160 or 0x5bdb60. M

## 2. CSWSPlaceable::AcquireItemsFromObject `0x005f5160`
- Convention: __thiscall. ECX = DESTINATION placeable. Spilled at `0x005f5185`, reloaded at `0x005f5375` before `CALL 0x005f4ed0`. H
- Args: [EBP+8] = source objid S, [EBP+0xC] = bFilter. RET 8 (asm `0x005f546b`). Returns count of moved non-gold items. H
- Caller: CSWSObject::SpawnBodyBag `0x549780`, call at `0x549abc` with `(deadCreatureObjid, 1)`, ECX set. H
- Source S: GetCreatureByGameObjectID `0x51c100` and GetPlaceableByGameObjectID `0x51c160`. Neither found -> return 0.
  - Creature S needs creature+0x1150 != 0 and CSWSCreature::GetItemRepository(1) `0x58a2c0` != 0.
  - Dest needs placeable+0x3C4 != 0. H
- Gather: creature S = 16 equipped slots via CSWInventory::GetItemInSlot `0x6d0620` (bit 1<<i), then S repo. Placeable S = its repo. M-H
- Filter: bFilter != 0 keeps only items with `(item+0x2c8 >> 3) & 1`. Semantic name of bit 3 unverified (likely droppable). M
- Move: each item -> CSWSPlaceable::AcquireItem(this=dest, &item, S, 1) `0x5f4ed0`. H
- Gold (creature S only, NOT filtered, NOT in return count): if GetGold `0x5812e0` != 0, build "g_i_credits001" (0x710840 + 0x601610), new item `0x5ffd60`, stack = gold (+0x2cc), `0x581330(0)`, add to dest with source 0x7f000000. M-H
- No range check, no opener check, no journal or feedback string. Stack merge is not in this function (AddItem `0x605090` may merge; unverified). H

## 3. Per-item take path
- CSWSPlaceable::AcquireItem `0x005f4ed0`: __thiscall, RET 12, args (int* itemRef, uint sourceObjid, bool flag=1). H
  - Removes item from source by source type (5 creature / 9 placeable / 6 item).
  - Then, if dest repo (+0x3C4) != 0: AddItem `0x605090`(item,1,0,1) and SetPossessor `0x600380`(destObjid,1,1,source).
  - Caveat: removal runs before the dest-repo check, so a dest with no repo loses the item. H
- CSWSCreature::AcquireItem `0x005bdb60`: __thiscall (ECX = creature), RET 0x10, args (int* itemRef, uint holderId, uint destId, bool flag). H
  - holderId must equal item possessor (+0x2a8), else the nested-bag branch runs.
  - Removal by holder type: 9 -> vtbl+0x48 + CSWSPlaceable::RemoveItem `0x5f4e60`; 5 -> vtbl+0x30 + CSWSCreature::RemoveItem `0x5bde40`; 6 -> vtbl+0x38 + `0x605e80`.
  - destId 0x7f000000 (`DAT 0x994484`, verified in .rdata) = creature's own repo; possessor set to creature objid. H
- Engine call sites:
  - CSWSCreature::AIActionPickUpItem `0x005cf8a0` line 144: `FUN_005bdb60(&item, 0x7f000000, dest, flag)` for ground items (possessor == 0x7f000000). H
  - CSWSObject::AIActionTakeItem `0x005dab80`, call at `0x005dac70`: `(&item, holder, 0x7f000000, flag)`. Holder provenance M.
- Mod recipe, one item from placeable P into creature C: ECX = C, args (&itemRef, P_objid, 0x7f000000, 1). Item possessor must equal P.
- Mod recipe, take all: loop P repo from last index down, re-read count +0x10 each pass, call the above per item. M
- Not a take route: CServerExoApp::CreatureAcquireItem `0x51cc60` (RET 8) -> `0x539f80` (RET 8) takes (creatureObjid, const CResRef* template), creates a NEW item. H

## 4. Repository walk
- Placeable repo pointer: +0x3C4. Seen in 5f5160, 5f4ed0, 5f5770, 5f67b0. H
- Count: repo+0x10 (5f5160, 5f67b0). H
- Item getter `0x606100` CItemRepository::ItemListGetItem(index): ECX = repo, stack arg index, RET 4 (asm `0x00606150`), bounds check vs +0x10 at `0x00606112`. Body uses GetItemByGameObjectID `0x51c0e0`. H
- Brief said "per doc 12": doc 12 not consulted.
- Helper: CSWSPlaceable::GetItemCount `0x005f67b0` (__thiscall, RET 4, bFilter). Returns 0 if +0x37C == 0, else counts repo items (optional bit 3). H

## 5. Gating (range, open state, last opener)
- No range check in AcquireItemsFromObject, CSWSPlaceable::AcquireItem or CSWSCreature::AcquireItem (full decomp read). H
- CSWSPlaceable::OpenInventory `0x005f5590` (467 B, __thiscall, RET 8, args (openerObjid, bool feedback)):
  - Gate: this+0x390 == 0 AND this+0x37C (HasInventory) != 0 AND GetClientObjectByObjectId(opener) != 0. Else feedback string 0x275c.
  - Sets this+0x390 = 1. Stores no opener objid on the placeable. H
- CSWSPlaceable::CloseInventory `0x005f5770` (RET 8): returns early unless this+0x390 != 0, and clears +0x390 at end. H
- Its flag != 0 branch empties the repo one item at a time, with a "lockertsfinv" resref branch and GUI font/string calls. Decomp has arg-recovery artifacts (pcStack_bc). L. Do not use as a take path.
- No "last opener" field found. Fields: +0x37C HasInventory (set by LoadBodyBag `0x5f0f10`), +0x390 opened flag, +0x3C4 repo, +0x38C unnamed.

## Gaps
- Vanilla client-to-server take message not located. GuiContainer minors != 2 are no-ops. Inventory 0x65e670 has no direct AcquireItem or RemoveItem call. OPEN.
- Param 5 (flag) of CSWSCreature::AcquireItem not verified. Brief uses 1, as 0x539f80 does.
- Pre-gate calls 0x5e3de0 / 0x5e3e40 in the GuiContainer handler: names unverified.
