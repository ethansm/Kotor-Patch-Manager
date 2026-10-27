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

## RE needed before a probe (one rea-recon batch)
Decompile Steam 0x5F5590, 0x5F2FE0, 0x5F5160, 0x7CACB0, 0x816020; find the Steam client container-GUI take/close
message senders and the `CSWGuiContainer` vtable.
