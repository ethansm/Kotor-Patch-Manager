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
| **R2** | Static RE for proximity open: placeable locked/trapped/has-inventory flags, area object list + leader distance, direct OpenInventory vs click (OnUsed?), combat/dialog state reads | QL1b | - | DONE | aa8f0ece | facts .claude/waves/w3-52-R2/u01-u03 (lock +0x2A0, trap +0x2C8, inv +0x37C, area ids +0x194/+0x198, dialog byte 0xA309BC); Opus spot-check PASS |
| **QL2p** | Proximity peek 0.3.0-dev (user choice 10-27): unlocked, untrapped container within OpenM=3 m auto-opens (OpenInventory), list under its name, click/key = take all; hides + Close(objId,0) past CloseM=5 m, on combat/dialog/area change/menu, or after take; nearest only; no re-open until you leave CloseM; ini knobs | R2 | install GO | INSTALLED | aa8f0ece | 0.3.1-dev LIVE 10-27 22:44 hot-swap (rollback quick-loot_swap_20261027_224447); play QL2p-P1 |
| **QL2L** | Multi-row item list 0.4.0-dev (user 10-27 "build the list" + "scrollable if the mouse is over the widget"): header "[F] Take all (N)" / "Took (N)" + one row per item (ListRows=8 visible, names up to 40 chars, " xN" stacks), mouse wheel over the block scrolls it (gui-input k2.guiinput@1 wheel, consumed only when inside; scroll indicator "^ K above" / "v K below"); Took list stays up 3 s + 0.5 s per row past 3 (cap 6 s, held while hovered); one-line fallback when row labels fail; mockup + sim updated | QL2p | install GO (hot-swap: k2.guiinput resolved at runtime) | DONE | 7c640dbc | built, folded into 0.5.0-dev (never installed alone) |
| **QL2V** | Inventory-style list 0.5.0-dev (user 10-27): rows styled like the vanilla inventory / equipment item picker (protoitem border + row dividers, taller rows), item icon per row (Steam GetIconResRef RE), vanilla scrollbar with up/down arrows + thumb on the side (arrows clickable); recon .claude/waves/w4-52-QL2V/u01 (GUI) + u02 (icon RE) | QL2L | install GO | INSTALLED | 9146aef4 | 0.5.1-dev LIVE 10-28 00:43 hot-swap (menu gate fix + vanilla wording; rollback quick-loot_swap_20261028_004315); retest QL2V-P1 |
| **QL2A** | World anchor (user 10-28 screenshots): the list follows the container every frame, re-placed every frame from the target name tag after the engine positions it (lockstep); ini/page Anchor = under name tag (default, user 10-28) | below container (projection 0x77E970 recipe, falls back to under-name) | beside name tag; recon .claude/waves/w5-52-QL2A/u01 | INSTALLED | 4caeb38b | 0.6.0-dev LIVE 10-28 01:54 hot-swap (rollback quick-loot_swap_20261028_015449); play QL06-P1 | - | - |
| **QL8** | Item tooltip on row hover (user 10-28): the vanilla item description box (name, stats, description) beside the list, as the inventory screen shows it; recon w5 u03 | QL2V | install GO | INSTALLED | 4caeb38b | 0.6.0-dev LIVE 10-28 01:54 hot-swap (rollback quick-loot_swap_20261028_015449); play QL06-P1 |
| **QL9** | Tooltip redesign 0.7.0 (user 10-29 + Gemini review .claude/reviews/quick_loot_ux_agy/REVIEW_gemini.md, mockup v2 `scripts/quick_loot_tip_mockup.py` d03d4b72): strip {tag} in names/desc; header icon + title + category (armor = weight class from reqfeat0); rules block label/value from BASEITEMS row (Damage min-max + engine damage word TLK 38552.., Critical) + item props (+0x288/+0x290 stride 0x1C, itempropdef/iprp 2DA via C2DA getters 0x71bcf0/0x71b1d0) + desc 'Label: value' lines; drop CastSpell/Human/Range/'Upgrades: None'; full desc when no rules; top flush with list | QL8 | install GO | TODO | - | facts w6-52-QL9/u01.md, ledger 20261029_item-rules-text.tsv |
| **QL10** | List polish 0.7.0 (Gemini 3/5/6): ListRows default 4, panel fill opaque ~0.94, stack count right-aligned column, icon 48 px, hide range counter during the Acquired flash + show qty | QL9 | install GO | TODO | - | - |
| **QL11** | Sort quest (Plot) items and credits first; credit rows show the amount | QL10 | install GO | TODO | - | recon w6-52-QL9/u02 Q4 |
| **QL12** | Vanilla UI sounds: hover tick, take one, take all (engine 2D GUI sound) | QL10 | install GO | TODO | - | recon w6-52-QL9/u02 Q1 |
| **QL13** | Tooltip footer: 'Equipped: <item> (<stat>)' for weapons/armor vs the leader's slot, 'In inventory: N' for stackables | QL9 | install GO | TODO | - | recon w6-52-QL9/u02 Q2/Q3 |
| **QL2** | Peek on select: list under the name tag after the first click; row click = walk + take one item | QL1 | - | TODO | - | - |
| **PV1** | Pre-play verification (user 10-27 "1-3"): (1) offline label render `scripts/quick_loot_mockup.py` (real HUD GUI + dialogfont10x10; name-tag and 62% positions; empty/1/many/+K states; 420 px width vs 80/100-char text), (2) Sonnet UX/UI review of renders + strings, (3) frame-by-frame host simulation of the peek/take state machine (approach, OnOpen, leave, combat mid-peek, area change, no Show; Close exactly once per open) | QL2p | - | DONE | 0f85071a | render+UX review+sim done; 2 sim bugs + UX MUSTs fixed in 0.3.1-dev (LIVE 10-27 22:44) |
| **QL4** | Per-item take (user 10-28 picked): click a row in the peek list -> `CSWSCreature::AcquireItem 0x5BDB60` (mod enforces range); replicate CloseInventory's first-pickup notice (item local bool 0x5E, lockertsfinv compare) | QL2p | install GO | INSTALLED | 4caeb38b | 0.6.0-dev LIVE 10-28 01:54 hot-swap (rollback quick-loot_swap_20261028_015449); play QL06-P1 |
| **QL5** | Empty / looted container marker (K1 "Marked Empty Containers" has no TSL port): mark on hover/name tag; merge with QL3 and doc 12 | QL2p | install GO | TODO | - | - |
| **QL6** | Give / quest hand-in containers: identify them (container_p "Give" view, ctrl 0x29, gui+0x70 bit0) from live logs and always route them to the vanilla menu | QL0-P1 logs | install GO | TODO | - | - |
| **QL7** | Mod Configuration page (doc 45/46 declarative rows): Enabled, ProximityEnabled, OpenM, CloseM, TakeKey | QL2p | install GO | INSTALLED | 4caeb38b | 0.6.0-dev LIVE 10-28 01:54 hot-swap (rollback quick-loot_swap_20261028_015449); play QL06-P1 |
| **QLR** | Release log tightening before PR: probe lines (ITEM/EVTRAW/EVT/SCAN) behind Verbose=0, change-only logging, <50 startup lines (kotor2-kpm-install rules) | QL4-QL7 | install GO | TODO | - | - |
| **QL3** | True hover peek: own floating name + list on hover (app+0x4B0), merge with doc 12 empty-container cursor | QL2 | - | TODO | - | - |

## 3. Open questions (QL0 answers them live)
1. When does event 0x16 (OnOpen) run relative to the next frame, and does its loot reach the repo before our Close?
2. `container_p.gui` control-id map (ids 0x27 OK / 0x28 Cancel / 0x29 Give are inferred, MED).
3. Which containers need the give view (quest hand-ins)?
