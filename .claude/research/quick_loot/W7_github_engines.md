# W7: GitHub engines and KOTOR repos, quick loot / take-all search (2026-10-27)

Method: GitHub API unauthenticated (code search returns 401, so no GitHub code search). Default-branch tarballs downloaded to scratchpad and grepped locally; nothing executed. Commits read: KotOR.js 002dfe03b9, reone 9417f636f2, xoreos 6774724d5c, KPM eaa7bdae41, K1CP d9b4a614d5, JCarter426 loot b98807e928.

## Repos and hits (URL, what, relevance)
1. https://github.com/KobaltBlu/KotOR.js (TS, master). `src/game/kotor/menu/MenuContainer.ts` (K1), `src/game/tsl/menu/MenuContainer.ts` (TSL, gui `container_p`), `src/module/ModulePlaceable.ts` `retrieveInventory()`, `src/actions/ActionTakeItem.ts`. Relevance: HIGH (readable container menu, both games).
2. https://github.com/seedhartha/reone (C++, master). `src/libs/game/gui/container.cpp`, `include/reone/game/gui/container.h`. `guiResRef()` appends `_p` on TSL. Relevance: HIGH.
3. https://github.com/xoreos/xoreos `src/engines/kotor/gui/ingame/container.cpp`, `ingame/hud.cpp` `showContainer()`. Relevance: MEDIUM (K1 menu, OK = take all).
4. https://github.com/LaneDibello/Kotor-Patch-Manager `Patches/K1MarkedEmptyContainers` (hover "(empty)" only). K1 hook 0x685E69, placeable vtable 0x7537D0, server repo at +0x324. K1WidescreenPatch README lists CSWGuiContainer vtable 0x1005AB758 (Mac build, not Steam). Issues #80, #111 are the empty marker only. 339 issues/PRs scanned (pages 1-4): no loot, take, or quick-loot item. Relevance: LOW for take, HIGH for K1 addresses.
5. https://github.com/KOTORCommunityPatches/K1_Community_Patch: NSS and README only. Changelog mentions a "Give Item UI" filter and loot-spawn fixes. No container GUI or take code. Relevance: LOW.
6. https://github.com/JCarter426/KOTOR2-Loot-Overhaul: NSS loot tables (`k_contain_unlock`, `k_contain_bash`). No GUI. Relevance: LOW.
7. No hit: "kotor quick loot", "kotor take all", "kotor2 container loot", "kotor container gui", "kotor_dll", "kotor hooks". KotorTool (artDeil4ek) is a C# inventory editor, not GUI. TSLRCM has no GitHub repo (DeadlyStream only).
8. Web: Nexus K1 mod 1815 ("(empty)" hover label, needs KOTOR Editable Executable). No K2 take-all mod found.

## Container GUI facts (from code, not from the binary container_p.gui)
- Control names, no numeric IDs in any repo: `LBL_MESSAGE`, `LB_ITEMS`, `BTN_OK`, `BTN_GIVEITEMS`, `BTN_CANCEL`. RE's 0x27/0x28/0x29 mapping cannot be checked here.
- TLK refs (KotOR.js and reone constants, text not verified): 393 "Container inventory"; 392 "Items available" (give mode); 38542 "Get Items" (OK, take mode); 38543 "Give Items"; 47884 "Switch to".
- OK in take mode = TAKE ALL. KotOR.js `retrieveInventory()` pops every item to the party, then runs PlaceableOnInvDisturbed. reone `transferItemsToPlayer()` moves all dropable items, then runs OnInvDisturbed. xoreos `showContainer()` (return 1) moves every item and calls removeAllItems.
- Cancel = close without take. KotOR.js also calls `placeable.close()`; reone only reopens in-game.
- BTN_GIVEITEMS toggles TAKE/GIVE in KotOR.js. reone sets its text but has no click handler (no give mode).
- Per-item take exists only as engine action: KotOR.js `ActionTakeItem` (removeItem plus add, no GUI).

## Discrepancy
- KotOR.js and reone fire OnInvDisturbed on take-all. RE R3 (static) says it fires only from CSWSPlaceable::AcquireItem, not on close or RemoveItem. Unresolved; needs a Steam trace.

## Verdict
No quick-loot, peek, or take-one code exists in the searched projects. Take-all exists only as the menu OK button in KotOR.js, reone, and xoreos. Claim "no quick-loot code exists" = CONFIRMED for this set (limited: no GitHub code search, forks not checked).
