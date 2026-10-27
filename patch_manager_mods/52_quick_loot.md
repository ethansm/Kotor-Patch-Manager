# Doc 52: Quick Loot (Fallout-4-style container peek)

Goal: fewer slow menus. Clicking a container takes its loot and shows a compact "Took: ..." list under the
target name tag instead of the full-screen container menu; later, peek on select and on hover.

Research and RE: `.claude/research/quick_loot/README.md` (web W1-W4, static RE R1-R3; commits 7c12ec16, 34100d1e).

## 1. Key engine facts (Steam 6A522E71, static RE)
- Vanilla has no per-item take. OK = `SendPlayerToServerGuiContainer_Close 0x0087A550` (thiscall-ish, RET 8; args
  objId, flag; major 0x19 minor 2). flag 1 = server `CSWSPlaceable::CloseInventory 0x005F5770` drains every item into the
  player; flag 0 = Cancel. A Close is mandatory: it clears placeable+0x390 (open flag), else the next open is refused.
- Sole `CGuiInGame::ShowContainerGui 0x007CACB0` (RET 8, ECX = CGuiInGame, args objId, flag) call site:
  `0x0082AD21` `E8 8A FF F9 FF` inside `HandleServerToPlayerGuiContainerObjectMessage 0x0082ACB0`.
- Items: server placeable via `CServerExoApp::ClientToServerObjectId 0x51C8B0` -> `GetPlaceableByGameObjectID 0x51C160`;
  repo = placeable+0x3C4, count = repo+0x10, `CItemRepository::ItemListGetItem 0x606100` (ECX repo, index, RET 4).
- OnOpen (+0x324) runs from a queued event 0x16 AFTER the client open message (`OpenInventory 0x5F5590` -> 
  `AddEventDeltaTime 0x51E170`); loot created by OnOpen is not in the repo at Show time.
- `CloseInventory` drain loop (0x5F586B..): per item, if `CSWItem::GetBaseItem 0x6D6E30` +0xAC == 0x18, it compares a string
  with `"lockertsfinv"` (`CompareNoCase 0x734F60`), else reads item local bool 0x5E (`GetLocalBoolean 0x5E9210`); when unset it sets it and
  shows a one-time pickup pop-up (`fnt_d16x16`). So it is a per-item first-pickup notice, NOT a stash container (RE R2/R3 guess wrong,
  checked 005f5770.c:85-130). Using vanilla Close(objId,1) keeps these notices; a custom per-item take (QL2) must replicate them.
- `HideContainerGui 0x7CAD30` is also called by `CSWSObject::StartDialog 0x545760` without a Close.
- Per-item take for QL2: `CSWSCreature::AcquireItem 0x5BDB60` ECX = creature, (int* itemRef, holderObjId, 0x7F000000, 1),
  RET 0x10. No range or opener check: the mod must enforce range.
- Overlay: hud-host `K2HUD_TARGET_MENU_POST` 0x743847 (shared; enemy-effects-strip, combat hotbar).

## 2. Master Plan Phasing (7-Column Schema)

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **R1** | Web survey + static RE of container open/take path | - | - | DONE | 34100d1e | web survey + static RE |
| **QL0** | Probe DLL `quick-loot` 0.0.1-probe: entry detours ShowContainerGui 0x7CACB0 (consumed exit 0x7CAD1F), EventHandler 0x5F2FE0, CloseInventory 0x5F5770; logs objId, flag, item list at Show, event 0x16/0x17/0x1B timing, n at Close; no behaviour change | R1 | install GO | INSTALLED | fc102dfc | quick-loot 0.0.1-probe LIVE 10-27 02:00 via kpm_reinstall (KPM a769ca7; rollback backups/quick-loot_reinstall_20261027_020048); kplog [quick-loot] added; play QL0-P1 |
| **QL1** | Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu | QL0 | install GO | INSTALLED | 94cfdf05 | quick-loot 0.2.0-dev LIVE 10-27 19:08 (rollback quick-loot_reinstall_20261027_190809); play QL1-P1 |
| **QL1a** | quick-loot 0.1.0-dev: consume Show (flag 1, no Shift, hud-host connected), send Close(objId,1) from K2HUD_FRAME 2 frames after event 0x16 (timeout 1 s); logs only, no toast | QL0 | install GO | INSTALLED | 94cfdf05 | quick-loot 0.2.0-dev LIVE 10-27 19:08 (rollback quick-loot_reinstall_20261027_190809); play QL1-P1 |
| **QL1b** | "Took: ..." label under the target name tag (K2HUD_BUILD label, shown ~3 s) | QL1a | install GO | INSTALLED | 94cfdf05 | quick-loot 0.2.0-dev LIVE 10-27 19:08 (rollback quick-loot_reinstall_20261027_190809); play QL1-P1 |
| **QL2** | Peek on select: list under the name tag after the first click; row click = walk + take one item | QL1 | - | TODO | - | - |
| **QL3** | True hover peek: own floating name + list on hover (app+0x4B0), merge with doc 12 empty-container cursor | QL2 | - | TODO | - | - |

## 3. Open questions (QL0 answers them live)
1. When does event 0x16 (OnOpen) run relative to the next frame, and does its loot reach the repo before our Close?
2. `container_p.gui` control-id map (ids 0x27 OK / 0x28 Cancel / 0x29 Give are inferred, MED).
3. Which containers need the give view (quest hand-ins)?
