# W4: KOTOR loot-speed mods and vanilla container flow

Recon date: 2026-10-27. Method: web search plus page fetches. Nexus pages returned HTTP 403 on fetch, so Nexus 1815 is read from search snippets. About 25 queries; coverage is shallow, so absence below is not proof.

## Mods and tools found (none speed up looting)

1. Mark Empty Containers (K1). https://www.nexusmods.com/kotor/mods/1815 (snippet only)
   - Does: adds "(empty)" to the hover name of containers and lootable corpses that hold nothing.
   - Method: exe patch. Needs KOTOR Editable Executable; apply and revert batch scripts back up and restore swkotor.exe.
2. TSL credit-gain thread and TSLRCM loot fix (K2). https://deadlystream.com/topic/3202-tsl-i-need-help-in-increasing-the-credit-gained-from-containers (2015)
   - Does: raises credit payouts from containers and bodies. Not speed.
   - Method: script (edit k_inc_treasure and k_inc_treas_k2, recompile k_plc_tres* scripts) and template (raise stack size in g_i_credits015.uti).
3. Personal-item container concept (K2). https://deadlystream.com/topic/7258-personal-item-mod/ (2019, unfinished)
   - Does: copies a placeable UTC, strips its inventory and loot script, spawns it by NCS, triggers it from DLG. No looting change.
   - Method: UTC + NCS + DLG.
4. Prologue Item Recovery (K2). https://deadlystream.com/files/file/2211-prologue-item-recovery/ (snippet only)
   - Does: spawns a plasteel cylinder holding prologue items. No looting change.
5. Kotor-Patch-Manager repo. https://github.com/LaneDibello/Kotor-Patch-Manager
   - README: "Patching Framework for Star Wars: KOTOR 1 & 2". Example patches named: ScriptExtender, AdditionalConsoleCommands, Level-Cap. No loot or container patch named. Patches/ folder not enumerated.

Considered and excluded: K1SE script extender (https://www.nexusmods.com/kotor/mods/1852); Harbinger Arrival: Free Cam (K2 scene speed, snippet only); OpenMW QuickTakeAll and other take-all mods (other games, design reference only).

## Vanilla container flow (xoreos source, a reimplementation, not the binary)

- Placeable click(): if HasInventory, closes when open, otherwise opens. Source: https://xoreos.org/doxygen/d7/d89/kotor2_2placeable_8cpp_source.html
- open(): returns false when locked. Otherwise plays the opened sound, runs the OnOpen script, sets state to open. No animation call in that file. Same source.
- Container GUI (container.cpp): loads GUI "container". Handles TGuiPanel, LB_ITEMS, LBL_MESSAGE (TalkMan string 394 when empty), BTN_OK, BTN_CANCEL. No take-all or get-all handler. Caveat: a .gui button with no code handler would not appear here. Source: https://xoreos.org/doxygen/d4/d30/kotor_2gui_2ingame_2container_8cpp_source.html
- Item transfer to the player is not in container.cpp. Listbox and inventory source not inspected.
- Walk-to-container: no source reached. Placeable code does not handle walking.
- Footlocker contents: K2 footlker001.utp in end_m01aa_s.rim has a fixed base set. Class gear is added by the Endar Spire OnEnter script. https://deadlystream.com/topic/4790-editing-placeable-inventory-when-variable-based-on-class/
- K2 has no empty storage containers (2016 player opinion; a 2021 reply says containers keep items). https://steamcommunity.com/app/208580/discussions/0/458605613400551264
- Thread 3202 says k_inc_treasure is leftover in K2, yet also says to edit k_inc_treas_k2 for K2. Treat as unclear.
- Player claim, unverified: the starting plasma torch "instantly breaks open any container/locker" in K2. https://steamcommunity.com/app/208580/discussions/0/617328967245974010

## Player complaints about looting speed

- None found about speed, one-at-a-time taking, or a missing take-all.
- Credits too low from looting containers and bodies (DeadlyStream 3202, 2015).
- No storage containers in K2 (Steam 458605613400551264, 2016).
- Take-all requests found are for other games (BG3, Icarus, Adventure Creator).

## Not found

- No KOTOR quick-loot, auto-loot, take-all, loot-from-range, or skip-open-animation mod.
- Not reached: r/kotor (no hits), LucasForums archive (no hits), kotor.neocities (no hits), KotOR.js repo (no hits), Nexus listings (403).

## Open questions

- Does vanilla container.gui define a take-all button? Needs the real container.gui (extract from chitin or KotOR Tool).
- How does the vanilla take-one-item action work? Needs listbox and inventory source.
