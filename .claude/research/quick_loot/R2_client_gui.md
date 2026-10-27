# R2 - Steam client container GUI (static recon, 2026-10-27)

Method: decomp/asm dumps, exe bytes (swkotor2_vanilla_6a522e71.exe), knowledge-DB name map, upstream symbol labels. No REA/MCP, nothing run in game.
Twin rule: GOG DB size equality plus callee shape. HIGH = size + shape (+ RTTI or DB name). MED = shape only.

## 1. Class, vtable, ctor (HIGH)
- Ctor `CSWGuiContainer::CSWGuiContainer` **0x008b1ea0** (1863 B, thiscall, ret 4). GOG 0x005a5a40 = 1863 B.
- vtable **0x009a77e4**. RTTI TypeDescriptor `.?AVCSWGuiContainer@@` at 0x00a11630, COL 0x009bf390, COL pointer at 0x009a77e0. Ctor stores vptr at 0x008b1eec. Upstream label `vftable CSWGuiContainer` agrees.
- Dtor: slot 0 **0x008b25f0** (46 B, scalar; GOG 0x005a6190 = 46 B) calls base **0x008b2620** (560 B; GOG 0x005a61c0 = 560 B).
- GUI resource: string `"container_p"` at 0x009a77d0, loaded by FUN_00710840 in ctor (loader name unknown, MED).
- Ctor binds controls by name via `CSWGuiPanel::InitControl` 0x0040f620 (DB verified). Byte offsets in `this`:
  LBL_MESSAGE +0x74, LBL_BUTTON +0x1bc, LBL_OK +0x304, **LB_ITEMS +0x824**, LBL_B +0x44c, LBL_X +0x6dc, LBL_CANCEL +0x594, **BTN_OK +0xb14**, **BTN_GIVEITEMS +0xeb4**, **BTN_CANCEL +0xce4**.
  No "Take All" string exists in ctor or SetContainer.
- Instance: CGuiInGame init **0x007be4c0** allocs 0x1084 B, calls ctor with GUI manager (CGuiInGame+0x38), stores at **CGuiInGame+0x54**. Sets CGuiInGame+0x128 = 1 at end (init-once flag).
- Vtable: slot 14 (+0x38) 0x008b35d0 give-toggle tick; **slot 15 (+0x3c) 0x008b2850 input handler**; slot 18 (+0x48) 0x008b3630 OnPanelAdded override; slot 19 (+0x4c) 0x008b3730 OnPanelRemoved override.

## 2. Show / Hide (HIGH)
- `CGuiInGame::ShowContainerGui` **0x007cacb0** (117 B, thiscall, ret 8; GOG 0x004db3b0 = 117 B). Single caller: **0x0082acb0 at call site 0x0082ad21** (E8 8A FF F9 FF).
  Body: if this+0x128: `SetContainer(this+0x54, objId, flag)`; `CSWGuiManager::AddPanel` 0x410530 (mgr=this+0x38, panel=this+0x54, 1, 1); `SetInputClass(2,1)` 0x73fee0; `SetPausedByCombat(1,3,0)` 0x740350.
- `HideContainerGui` **0x007cad30** (126 B, fastcall, ret 0; GOG 0x004db430 = 126 B). If this+0x128 and cache check 0x410800 true: RemovePanel 0x410670, SetInputClass(0,1), SetPausedByCombat(0,0,0). Cache check is Ghidra-labelled CMFCVisualManagerBitmapCache::IsCached (mislabel).
- Callers: 0x008b2850 (3 sites) and CSWSObject::StartDialog 0x00545760 (site 0x00545fb4). The dialog path hides without a P2S Close.

## 3. Populate: `CSWGuiContainer::SetContainer` **0x008b29e0** (1410 B, thiscall, ret 8; GOG 0x005a6580 = 1410 B) (HIGH)
- Args: objId -> this+0x68, flag byte -> this+0x6c. Sets this+0x70 bit0 (container view).
- Item source is server state read directly (same process): `CServerExoApp::ClientToServerObjectId` 0x51c8b0 -> `GetPlaceableByGameObjectID` 0x51c160 -> placeable+0x3c4 = CItemRepository; count at repo+0x10.
- Per item: `CItemRepository::ItemListGetItemObjectID` 0x606160 -> `CClientExoApp::ServerToClientObjectId` 0x73fec0 -> `new 0x3b4` + `CSWGuiInGameItemEntry::Constructor` 0x8b0a60 (vptr 0x9a76c4) -> `SetItem` 0x8b1250 -> list add 0x41fc30 (unnamed). Rows are not fed by a network list.
- Text: flag 1 -> TLK 0x189; flag 0 -> 0x18a, or 0x1d462 when placeable UseSearch int != 0.

## 4. Give view: `CSWGuiContainer::SetupGiveMode` **0x008b2f70** (1410 B, ret 4; GOG 0x005a6b10 = 1410 B) (MED-HIGH)
- Source: `CSWPartyTable::GetPartyItems` 0x5f7a30. Clears this+0x70 bit0. Called by 0x8b2850 (id 0x29, arg 0) and 0x8b35d0 (arg 1).

## 5. Button handler `CSWGuiContainer::HandleInputEvent` **0x008b2850** (357 B, thiscall, ret 8; vslot 15)
- `switch(id - 0x27)`, acts only when event param != 0, always ends with base `CSWGuiPanel::HandleInputEvent` 0x0040eaa0.
  | id (case) | action | P2S sent |
  |---|---|---|
  | 0x27 (0), 0x2d (6) | Container view: Close(objId, flag = mode==1), then Hide. Give view: case 0 no-op; case 6 plays sound only | Close(objId, mode) in container view |
  | 0x28 (1), 0x2e (7) | PlayGuiSound, Close(objId, 0), Hide = Cancel | Close(objId, 0) |
  | 0x29 (2) | Container view: SetupGiveMode(0). Give view: SetContainer(objId, mode) = Give toggle | none |
- Control IDs 0x27-0x2e are not resolvable: container_p.gui is not in repo or dumps. Names above are inferred from ctor (BTN_OK / BTN_CANCEL / BTN_GIVEITEMS). MED.

## 6. P2S Close: `CSWCMessage::SendPlayerToServerGuiContainer_Close` **0x0087a550** (106 B, ret 8; GOG 0x00505e30 = 106 B) (HIGH)
- Wire: major **0x19**, minor **2**, body = DWORD objId, DWORD flag. Sent via `CSWCMessage*` from `GetSWCMessage` 0x73f810.
- Only caller 0x8b2850. Only P2S sender with major 25 among the 29 send helpers enumerated (0x8798xx-0x87afxx).
- Server: dispatcher 0x0065f8a0 case 0x19 -> `CSWSMessage::HandlePlayerToServerGuiContainerMessage` **0x0065afd0** (289 B, ret 8; GOG 0x00800d70 = 289 B). Minor 2 branch: objId = DWORD & 0x7fffffff, flag = low byte, then `CSWSPlaceable::CloseInventory(player, flag)` 0x005f5770.

## 7. S2P open chain (HIGH)
- Server `CSWSPlaceable::OpenInventory` **0x005f5590**: sets placeable+0xe4 = 1; flag = (repo count != 0); calls `SendServerToPlayerGuiContainerObject_Open` **0x0064b620** (121 B, ret 12; GOG 0x007e1d40 = 121 B). Payload: major 0x19, minor 1, DWORD objId, DWORD flag.
- Client: S2P dispatcher 0x00810cf0 case 0x19 -> `HandleServerToPlayerGuiContainerObjectMessage` **0x0082acb0** (129 B, ret 4; GOG 0x004bf300 = 129 B). Minor == 1 -> GetInGameGui 0x73f750 -> ShowContainerGui.
- Separate GameObjUpdate path: major 5 -> 0x80deb0 -> minor 0x47 -> 0x815b00 -> sub-op 0x43 -> **0x00816020** HandleServerToPlayerUpdate_GuiContainer (647 B, ret 0; GOG 0x004aa610). Sub-ops: A/M write obj+0x15c/0x15d, U writes +0x180, D/P no UI effect. None rebuild the list box.

## 8. Item movement, vanilla (MED-HIGH)
- `CSWSPlaceable::CloseInventory` **0x005f5770** (1170 B; DB verified; GOG 0x0076fb40). Runs only if flag != 0 and placeable+0xe4 set.
  Loop while repo count != 0: `CItemRepository::ItemListGetItem` 0x606100 (last index), `CItemRepository::RemoveItem` 0x605e80, `CSWSCreature::GetItemRepository` 0x58a2c0 -> `CItemRepository::AddItem` 0x605090, `CSWSItem::SetPossessor` 0x600380 (conditional). Clears +0xe4 at the end.
- Vanilla take = one client message; server drains all items, one per internal iteration. No per-item client message found.
- quick_loot README candidates: `AcquireItemsFromObject` 0x5f5160 has only caller SpawnBodyBag 0x549abc, so it is not the GUI path. README's `RemoveItem 0x5F4E60` disagrees with DB (use CItemRepository::RemoveItem 0x605e80).

## 9. Hook point (for a compact-list mod)
- Preferred: call site **0x0082AD21** (`E8 8A FF F9 FF`, CALL 0x007CACB0). Redirect the rel32 to a stub: ECX = CGuiInGame, stack args objId and flag (stdcall-style RET 8 contract).
- Stub MUST always send Close (0x0087a550: flag 0 = release, flag 1 = take all). Otherwise placeable+0xe4 stays 1 and OpenInventory's gate (0x5f5590 line 21) refuses re-open.
- Alternative: entry 0x007CACB0 prologue `55 8B EC 83 EC 0C 89 4D F4` (9 B). A 5-byte JMP needs the 6-byte PUSH/MOV/SUB relocated.
- If the Show is suppressed, HideContainerGui should be a no-op (it acts only when the panel is cached). Not run.

## 10. Confidence and OPEN
- HIGH: ctor, vtable, dtor, SetContainer, ShowContainerGui, HideContainerGui, S2P/P2S pair, OpenInventory, CloseInventory.
- MED: control-ID to button names; SetupGiveMode; CloseInventory per-iteration detail.
- OPEN 1: which P2S message does a single-item take use? None found in 0x8b2850 or any major-0x19 sender.
- OPEN 2: container_p.gui control-ID map (needed to confirm which id is OK / Take All).
- OPEN 3: dialog-hide path (0x545760) never sends Close; does anything else clear placeable+0xe4?
