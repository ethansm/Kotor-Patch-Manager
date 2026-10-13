# Holstered weapon workaround (armor/belt bake + script swap): web recon

Date: 2026-10-13. Searches: 21 WebSearch. Fetches: 8 WebFetch (Nexus 1866 and 1185 returned 403; snippet only).

## (a) Holster / belt-hilt mods
- NOT FOUND: no KOTOR 1/2 mod shows a holstered lightsaber or blaster on the hip (9 queries, DeadlyStream and Nexus).
- UNVERIFIED: "Switch Weapons for K1" adds holster animations. No DeadlyStream or Nexus page for it surfaced. Nexus 1866 is a 403 with unknown content; the Ahsoka K1 hilt mod is DeadlyStream 3065 (snippet, no holster mention).
  - https://deadlystream.com/files/file/3065-ahsoka-lightsabers-tcw-kotor-1/
- https://deadlystream.com/topic/2736-moddefault-hilt-replacement-megapack : K2 hilt-model replacement only; hilt model tied to crystal color.
- https://deadlystream.com/files/file/2339-kotor-ii-switch-modding-framework : "Switch" = Nintendo Switch port tooling, not weapon swapping.
- https://www.gamebanshee.com/forums/viewtopic.php?p=453865 : K1 has no quick weapon-set hotkey; K2 added a 2-set hotkey (per Steam thread quoted in search).
- https://www.nexusmods.com/fallout4/articles/5165 : non-KOTOR "Visible Weapons" holster logic (Always vs sheathe-before-switch); design reference only.
- https://www.nexusmods.com/oblivion/mods/34766 : non-KOTOR; offhand lightsaber hanging on hip was fixed by deleting a quest script (snippet).

## (b) Belt slot as a visible model
- REFUTED (for a separate belt body model): TSL "originally planned on a belt model slot, but that was removed"; only options are body+head or full-body (DarthParametric).
  - https://deadlystream.com/topic/12156-a-few-questions-about-armor-modification/
- appearance.2da body-model columns are hardcoded; new columns are not recognized by the game. Slot L (K1 dancer) and K (flight suit) are the usual reuse targets.
  - https://deadlystream.com/blogs/entry/424-workaround-for-the-limited-number-of-body-model-slots-in-kotor-and-tsl/
- Body letters (community, from snippet): A underwear, B clothes, C-H armour, I/M/N robes, J unused, K flight suit.
- baseitems.2da (community column list): modeltype 1 = armor else 0; partenvmap 1 = armor; equipableslots is a hex bitfield; no belt column described.
  - https://deadlystream.com/topic/4725-detailed-information-of-2da-files/
- Belt equip-slot bitmask value for KOTOR: NOT FOUND (only NWN/WoW/Oblivion pages).
- Visas robes mod edits appearance.2da only, no attached model (no belt or weapon).
  - https://deadlystream.com/topic/9733-modvisas-visible-jedi-robes

## (c) Feasibility and downsides
- Scripted appearance swap: DarthParametric calls it "not particularly practical" for belt or weapon swaps; a TSL function makes it "more practical, but still extremely clunky."
  - https://deadlystream.com/topic/12156-a-few-questions-about-armor-modification/
- Save compat: appearance is "permanently stored in your save" once the area is entered; loads from before first entry needed (K1 blueprint thread, no scripts).
  - https://deadlystream.com/topic/4431-k1-changing-appearance/
- Heartbeat: real interval about 3-6 s, frame-time dependent; per-effect scans every few seconds "not ideal"; local booleans reportedly break for party members after unequip/save/load.
  - https://deadlystream.com/topic/11349-onheartbeat-calling-frequency/
- Hilt model is chosen by crystal color via upcrystals.2da, so a hilt-only bake cannot vary independently (snippets).
  - https://deadlystream.com/topic/1643-how-to-convert-a-hilt-model-between-kotor-and-tsl/
- Equip or unequip stutter cost, and extra-model render cost: NO source found.

## Scripting references (NWN docs only; KOTOR behaviour unverified)
- ActionUnequipItem / ActionEquipItem / GetItemInSlot: https://nwnlexicon.com/ActionUnequipItem ; https://nwnlexicon.com/ActionEquipItem ; https://nwnlexicon.com/GetItemInSlot (NWN slot range 0-17).
- OnCombatRoundEnd about every 6 s; OnPerception fires only on state change: https://nwnlexicon.com/OnCombatRoundEnd ; https://nwnlexicon.com/OnPerception
- SetCreatureAppearanceType: https://nwnlexicon.com/SetCreatureAppearanceType (NWN only).
- INVENTORY_SLOT_RIGHTWEAPON value in KOTOR: NOT FOUND. A Steam thread uses INVENTORY_SLOT_RIGHT_HAND (unverified).
- No KOTOR source found for engine auto-equip on attack.

## Open
- Does the K1/TSL engine re-equip a hand weapon on attack or combat start, and does a script unequip stick?
