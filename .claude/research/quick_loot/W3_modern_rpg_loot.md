# W3: Modern mouse-driven RPG looting (survey, 2026-10-27)

Method: WebSearch plus WebFetch. "Fetched" = page opened and read. "Snippet" = search result only; the page was blocked (402/403/404) or not opened. Coverage is partial; see OPEN.

## Per game

### Baldur's Gate 3
- Hold Left Alt: highlights lootable objects in view and shows their names; click a name to pick up or interact (fetched: pcgamesn, gfinityesports).
- Alt also reveals whether a container is marked empty (fetched: dexerto).
- Container window has a "Loot All" button; it misses hidden loot until container contents are touched (fetched: dexerto, 2023-08-14).
- Alt skips barrels; the binding is under Keybinds > Expand Tooltips (fetched: gfinityesports).
- Snippet only: Shift/Ctrl multi-select, then right-click "add to wares"; a Steam request asks for a "Take All To Wares" button.

### Pillars of Eternity 1/2, Pathfinder WotR / Kingmaker
- No verified loot-flow source. Pillars fandom wiki returned 402; Kingmaker wiki returned 404.
- WotR snippet: players take loot on the exit/map-leave screen with a take-all option; a ToyBox command-window entry "Open Mass Loot Window" exists; Enhanced Inventory mod highlights important loot in the loot window.
- Kingmaker snippet: no loot-all button confirmed.

### Dragon Age Origins / Inquisition
- Origins snippet: container page says "Take All" grabs everything at once. Page returned 402 (not opened).
- Inquisition: not confirmed. Snippet says the wiki page is flagged incomplete.

### Mass Effect Legendary Edition
- No Legendary Edition source found.
- Proxy (Mass Effect Andromeda, snippet only, wiki 402): a Take All entry appears when 2+ items exist; items can be taken one at a time without closing the container.

### Cyberpunk 2077 / The Witcher 3
- Witcher 3 base game (snippet only, Prima page 403): approach a lootable object, press X (A on Switch) for an item popup; Triangle takes all (PlayStation mapping).
- Witcher 3 AutoLoot mod v1.7.4 (fetched, gamepressure, 2016): "eliminate the tedious loot popup"; filter-based auto-loot from containers; manual looting kept. No radius or quest-item rule stated.
- Cyberpunk 2077 (snippet only, 2021 Steam thread): junk must be disassembled from inventory one item at a time. An auto-disassemble perk is unverified. No native loot-all confirmed.

## Patterns (ranked by fit to mouse point-and-click with hover name labels)
1. Hover or held-modifier name labels on lootable objects, click to loot. BG3 Alt does this (fetched). Fit: high. Gap: BG3 Alt misses barrels and objects outside facing direction (fetched, gfinityesports; snippet, Steam).
2. Single container window with a Take All button beside per-item click. BG3 "Loot All" (fetched); WotR and Witcher 3 take-all (snippet). Fit: high. Gap: Loot All can skip hidden items (fetched).
3. Empty-state tell on containers so players skip empty opens. BG3 Alt marks empties (fetched, dexerto). Fit: medium-high. Cost: one hold-key to read.
4. Right-click or hover to send items to a stash or wares. BG3 (snippet), Dragon Age II/DOS2 (snippet). Fit: medium. Solves overflow, not loot speed.
5. Opt-in auto-loot by filter or radius, as mods. Witcher 3 AutoLoot (fetched). Fit: low-medium for a pausable party RPG where authored loot matters. Shows player demand, not a native design.

## Sources
Fetched:
- https://www.gfinityesports.com/article/baldurs-gate-3-highlight-objects
- https://www.pcgamesn.com/baldurs-gate-3/highlight-objects
- https://www.dexerto.com/baldurs-gate/baldurs-gate-3-missing-loot-bug-psa-2248973/
- https://www.gamepressure.com/download/the-witcher-3-wild-hunt-autoloot-v174-mod/z4e384
- https://bg3.wiki/wiki/Box (no loot-UI text; negative result only)

Snippet only (blocked or not opened):
- https://dragonage.fandom.com/wiki/Containers (402)
- https://masseffectandromeda.fandom.com/wiki/Item_Pickup (402)
- https://primagames.com/eguides/the-witcher-3-wild-hunt-the-complete-edition-eguide/training/reactions-and-interactions (403)
- https://steamcommunity.com/app/1086940/discussions/0/3819669231700180355 (BG3 Take All thread)
- https://steamcommunity.com/app/1091500/discussions/0/4477106086834895317 (Cyberpunk, 403)
- https://steamcommunity.com/app/1184370/discussions/0/3044985412472744122 (WotR)
- https://www.nexusmods.com/witcher3/mods/7554/ (403)

## OPEN
- Official BG3, Pillars, Kingmaker, Mass Effect LE, and Cyberpunk loot docs or patch notes were not reachable; confirm Take All and auto-loot status there before design decisions.
