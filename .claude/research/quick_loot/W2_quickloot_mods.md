# W2: QuickLoot family, container peek technique (Skyrim)

Recon 2026-10-27. Source read: raw files of WaterFace/QuickLootEE (curl, not summarised), repo API metadata, search snippets. Nexus pages returned 403 (Cloudflare) and were not read. The brief's URL github.com/Ryan-rsm-McKenzie/QuickLootRE returns 404 (API and web).

## 0. Lineage and sources
- Root repo: SVG1/QuickLootRE (not a fork, last push 2018-11-03, README is one line). https://api.github.com/repos/SVG1/QuickLootRE
- Eloquence4/QuickLootEE forks it; WaterFace/QuickLootEE forks that. README: "QuickLootRE with TCC icon support". https://api.github.com/repos/WaterFace/QuickLootEE ; https://raw.githubusercontent.com/WaterFace/QuickLootEE/master/README.md
- Release v1 "1.6.1170 Support (DLL Only)": replacement DLL for QuickLoot EE (Nexus 69980). Needs latest Address Library, install QLEE first, includes a Which Key? NG patch, no known issues listed. https://github.com/WaterFace/QuickLootEE/releases
- Original Quick Loot (himika, Nexus 73096), search snippet only: SKSE 1.7.1+, LootMenu.swf in Interface folder, window not configurable. https://nexusmods.com/skyrim/mods/73096 ; https://stepmodifications.org/forum/topic/9894-quick-loot-by-himika/

## 1. Technical approach (source-verified; src/ = https://raw.githubusercontent.com/WaterFace/QuickLootEE/master/src/)
1a. Crosshair target: SKSE::CrosshairRefEvent via SKSE::GetCrosshairRefEventSource (src/Events/Events.h, CrosshairRefManager). Caches the ref; Evaluate → CanOpen → loot.SetContainer, else loot.Close.
1b. Other sinks: RE::TESLockChangedEvent (ScriptEventSourceHolder, only when OpenWhenContainerUnlocked); RE::TESCombatEvent (CombatManager, only when CloseInCombat); TESContainerChangedEvent filtered by container FormID → RefreshInventory (src/ContainerChangedHandler.cpp).
1c. Death: no death event. Code patch on Actor::SetLifeState (REL::ID 37612 on AE, 36604 otherwise; offsets 0x494 / 0x503) with an Xbyak stub branched via SKSE trampoline (src/Events/Events.cpp).
1d. Overlay: not a HUD overlay. A full RE::IMenu subclass "LootMenu" loads its SWF via BSScaleformManager::LoadMovieEx (src/Scaleform/LootMenu.h). Rows and bars are CLIK ActionScript controls (ScrollingList, ButtonBar, InfoBar, ItemInfo under swf/LootMenu/). Show/hide by UIMessageQueue kShow / kHide (src/Loot.cpp).
1e. Native prompt: HUDManager overwrites vtable slot 0x4C (GetActivateText) on TESNPC, TESObjectACTI, TESObjectCONT to suppress the engine activate text while enabled (src/HUDManager.h). ViewHandler pushes SetCrosshairTarget into the HUD SWF (src/ViewHandler.h).
1f. Take: Activate key via ControlMap. On release, Loot::TakeStack → Item::TakeAll → InventoryItem::DoTake → container->RemoveItem(obj, n, kRemove or kSteal, extra, &player), plus StealAlarm when stolen (src/Items/InventoryItem.h). Ground items: a_dst.PickUpObject (src/Items/GroundItem.h). Hold past fZKeyDelay → player->StartGrabObject, then close (src/Input/InputListeners.h, TakeHandler::TryGrab).
1g. Not a fake transfer, and the real container menu is never opened. It calls the engine's RemoveItem, so engine ownership and steal logic apply.

## 2. Controls and config
- Keys: Activate tap = take selected stack; Activate hold = engine grab; Up/Down and wheel = select; Left/Right and PgUp/PgDn = page; gamepad d-pad mapped too (src/Input/InputListeners.cpp).
- Config: 14 ESP globals read by editor ID, no INI in this version (src/Settings.cpp): QLEECloseInCombat, QLEECloseWhenEmpty, QLEEDispelInvisibility, QLEEOpenWhenContainerUnlocked, QLEEIconShowBookRead/Enchanted/DBMDisplayed/DBMFound/DBMNew, QLEEDisableForAnimals, QLEEWindowX/Y/W/H.
- QLEEDisableForAnimals is read, but its check in CanOpen is commented out, so it has no effect in this source (src/Events/Events.h).
- How a user edits these globals (MCM, xEdit, other) is not documented in anything read. OPEN.

## 3. Edge cases handled (from code)
- Locked: CanOpen requires !IsLocked; ViewHandler disables when locked. With OpenWhenContainerUnlocked, TESLockChangedEvent re-evaluates the crosshair ref.
- Owned / stealing: per-item stolen = !IsOwnedBy(player, !stealing), where stealing = dst->WouldBeStealing(src). Stolen stacks use kSteal + StealAlarm. Ground items use PickUpObject with no StealAlarm in this file.
- Corpses: only dead, non-summoned Actors with a container. Living NPCs never peek.
- Ash piles: activators resolve through extraList.GetAshPileRef().
- Empty: shows an empty list unless CloseWhenEmpty.
- 32 or more entries: window closes silently (RefreshInventory, size >= 32).
- Blocked: ShouldOpen refuses while grabbing, while an actor has a command, or in combat if CloseInCombat. ViewHandler disables in kill move, furniture, beast form, or non-gameplay input context (src/Loot.h, src/ViewHandler.h).
- Take side effects: DispelInvisibility on take.
- Animation: the menu has none. After take, OpenCloseHandler sets the container open state if closed (BGSOpenCloseForm::SetOpenState). Its wait is a detached std::thread that sleeps endKeyTime + 0.1 s, then posts an SKSE task (src/OpenCloseHandler.h). Timed sleep, not an animation event.

## 4. Known bugs and compat
- Version breakage: 2020 Steam thread says QuickLoot RE fails on latest SKSE; 2024 thread reports a startup crash. Search snippets only, bodies not readable. https://steamcommunity.com/app/489830/discussions/0/2264691750500907611
- QLEE v0 was a Debug build (needed VS); v1 fixed it. Tested only on 1.6.1170, SKSE version not stated.
- Code: Loot::Process clears _taskQueue and iterates it without _lock, while AddTask locks (src/Loot.cpp). Race risk, not reproduced.
- Code: the `else if` in Loot::Process skips RefreshInventory when RefreshUI is pending. Harmless, since RefreshUI calls RefreshInventory.
- Compat: an Enderal Papyrus patch binds RegisterForMenu("LootMenu") and Game.GetCurrentCrosshairRef(), so the menu name is a script-facing contract. https://git.eddoursul.win/enderal/enderalse/raw/branch/experimental-crash-fixes/source/scripts/_00e_quicklootpatch.psc

## 5. Scripted container events (the open-event question)
- Take path, first take per container only (OneShotOpen, src/OpenCloseHandler.h): InitChildActivates, then ScriptEventSourceHolder SendActivateEvent(src, player), SendOpenCloseEvent(src, player, true), ChestsLooted::SendEvent. This runs after RemoveItem.
- Close path (OneShotClose): SendOpenCloseEvent(src, player, false), once per source. SetSource calls Close(), so moving the crosshair to another container closes the previous source. A peeked container that was never taken from can still get a close event with no matching open. Code inference, untested.
- Peek alone sends no activate or open event.
- So "open scripts never fire" is partly wrong: QLEE fires them itself, but only on take. OnOpen code that reads container contents sees the post-removal inventory.
- Anecdotal only: a 2016 Fallout forum thread says the F04 "shortcut looting" skips scripted events. That is Fallout 4's native method, not QLEE. https://taleoftwowastelands.com/viewtopic.php@t=4777

## Open
- In game: peek then leave, check whether OpenClose(false) fires with no prior open. Needs a logged container script.
- Nexus pages unreadable (403): mod description and config instructions unverified.

## Sources
- Source: https://raw.githubusercontent.com/WaterFace/QuickLootEE/master/src/OpenCloseHandler.h ; .../src/Scaleform/LootMenu.h ; .../src/Events/Events.h ; .../src/Loot.cpp ; .../src/Items/InventoryItem.h ; .../src/Settings.cpp
- Repo meta: https://api.github.com/repos/WaterFace/QuickLootEE ; https://api.github.com/repos/Eloquence4/QuickLootEE ; https://api.github.com/repos/SVG1/QuickLootRE
