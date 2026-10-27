# Quick loot (Fallout-4-style container peek) — feasibility, 2026-10-27

Sources: W1 Fallout 4, W2 Skyrim QuickLoot (source-read), W3 modern RPGs, W4 KOTOR mods (web, Haiku);
local recon on the engine container path and hover/target widget (Haiku, static only, nothing run).

## Prior art
- Fallout 4: look at a container -> small list beside the crosshair; scroll to select, E takes one, Take All
  key, a separate key opens the full transfer menu (sources disagree on which key is which, W1).
- Skyrim QuickLoot EE (W2): crosshair-ref event -> own Scaleform menu; take = engine `RemoveItem` (ownership and
  steal logic still apply); never opens the real menu; fires Activate + OpenClose events itself on the FIRST take
  only, so OnOpen scripts see the post-take inventory. Peek alone fires nothing. Closes at >= 32 entries.
- BG3: highlight key + empty-container flag + Loot All in the container pop-up (W3).
- KOTOR: no quick-loot / take-all / auto-loot mod found (W4, shallow); closest is "Mark Empty Containers".

## Engine facts (Steam)
- Vanilla is two-step: hover = select cursor only; the name/HP widget (`CSWGuiTargetActionMenu`, hud+0xCC)
  shows for the SELECTED target (HUD+0x68, `OnObjectSelected 0x798D70`). Hovered id = app+0x4B0
  (`HandleMouseOver 0x7B3A00`). Doc 12 M7-3.
- Overlay attach point: hud-host `K2HUD_TARGET_MENU_POST` 0x743847 (shared, priority-ordered; used by
  enemy-effects-strip and combat hotbar).
- Clicks: `HandleMouseClickInWorld 0x7B3CA0` (mouse-probe already detours it; KPM forbids two patches per address).
- Open chain (names/votes, not call-graph-checked): UseObject P2S 0x879CC0 -> `AIActionUseObject 0x5DFBE0` ->
  `CSWSPlaceable::OpenInventory 0x5F5590` -> S2P GuiContainer open 0x64B620 (vote) -> client
  `HandleServerToPlayerUpdate_GuiContainer 0x816020` -> `ShowContainerGui 0x7CACB0` (medium); hide 0x7CAD30.
- Transfer: `CSWSPlaceable::AcquireItemsFromObject 0x5F5160` (take-all candidate), `CSWSPlaceable::RemoveItem 0x5F4E60`,
  `CSWSCreature::AcquireItem 0x5BDB60`. Item count: `GetItemCount 0x5F67B0` via `GetServerPlaceable 0x842D00` (doc 12).
- Unmapped on Steam: `CSWGuiContainer` ctor/`SetContainer`, `HandlePlayerToServerGuiContainerMessage`.
- OPEN: does `OpenInventory` / `CSWSPlaceable::EventHandler 0x5F2FE0` run OnOpen / OnInvDisturbed? 11 containers in
  kasm22ab generate loot on open (doc 12 M7-R4), so a peek that skips the open path shows them empty.

## Recommended staging
1. QL1 fast container: keep vanilla walk + open (scripts fire), hook `ShowContainerGui` and show a compact list
   under the target name instead of the full-screen menu; click row = take, Take All button/key, Shift = full menu.
2. QL2 peek on select: after the first click, list items under the name label before walking (read-only;
   loot-on-open containers show "?"). Row click = walk + take.
3. QL3 true hover: own floating name + list on hover (needs app+0x4B0 hover path, doc 12 H7a sibling).
Merge with doc 12 (empty-container indicators) as one loot mod.

## RE results 2026-10-27 (static dumps, $0 REA; R1 scripts, R2 client GUI, R3 server take)
- Vanilla has NO per-item take. OK = `SendPlayerToServerGuiContainer_Close 0x87A550` (major 0x19, minor 2,
  objId, flag=1); server `HandlePlayerToServerGuiContainerMessage 0x65AFD0` -> `CSWSPlaceable::CloseInventory 0x5F5770`
  drains the whole repo into the player. flag=0 = Cancel. Close is mandatory: it clears placeable+0x390 (open flag),
  else `OpenInventory` refuses the next open.
- Hook: sole `ShowContainerGui` call site `0x82AD21` (`E8 8A FF F9 FF`), in `HandleServerToPlayerGuiContainerObjectMessage
  0x82ACB0`. Alt entry 0x7CACB0 prologue `55 8B EC 83 EC 0C 89 4D F4`. `CSWGuiContainer`: ctor 0x8B1EA0, vtable 0x9A77E4,
  SetContainer 0x8B29E0 (reads server repo directly: placeable+0x3C4, count +0x10, `ItemListGetItemObjectID 0x606160`),
  input handler 0x8B2850 (ids 0x27 OK, 0x28 Cancel, 0x29 Give; MED), instance at CGuiInGame+0x54. GUI file `container_p`.
- Scripts: `OpenInventory 0x5F5590` sends the client open msg FIRST, then queues event 0x16 -> OnOpen (+0x324) runs later
  from `CSWSPlaceable::EventHandler 0x5F2FE0`. Loot made by OnOpen is not in the list at Show time. OnInvDisturbed
  (+0x30C, event 0x1B) fires from `CSWSPlaceable::AcquireItem` only, not on RemoveItem.
- Per-item take (mod-side): `CSWSCreature::AcquireItem 0x5BDB60` thiscall ECX=creature, (int* itemRef, holderObjId=P,
  0x7F000000, 1), RET 0x10; item possessor must equal P. No range/opener check anywhere -> mod enforces range.
- `AcquireItemsFromObject 0x5F5160` is SpawnBodyBag's corpse -> remains copy, not a GUI path.
- `CloseInventory` drain loop (0x5F586B..): per item, if `CSWItem::GetBaseItem 0x6D6E30` +0xAC == 0x18, it compares a string
  with `"lockertsfinv"` (`CompareNoCase 0x734F60`), else reads item local bool 0x5E (`GetLocalBoolean 0x5E9210`); when unset it sets it and
  shows a one-time pickup pop-up (`fnt_d16x16`). So it is a per-item first-pickup notice, NOT a stash container (RE R2/R3 guess wrong,
  checked 005f5770.c:85-130). Using vanilla Close(objId,1) keeps these notices; a custom per-item take (QL2) must replicate them.
- `HideContainerGui` is also called by `CSWSObject::StartDialog 0x545760` with no Close.
- OPEN: container_p.gui control-id map; when event 0x16 runs relative to the next frame; OnOpen loot reaching the client.

## Revised QL1 (cheapest)
Redirect 0x82AD21 to a stub: read the repo (names, counts) -> send Close(objId, 1) -> floating "Took: ..." list under
the name tag; Shift held or give-quest containers -> call vanilla ShowContainerGui. Defer the Close a few
frames (or until OnOpen ran) so loot-on-open containers are not drained before their loot spawns.
