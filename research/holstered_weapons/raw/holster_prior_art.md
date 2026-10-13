# Holster / sheathed-weapon prior art (KOTOR 1, TSL, Aurora/Odyssey)
Searched 2026-10-13 via WebSearch/WebFetch only. Repo not touched. About 30 queries and fetches.

## Bottom line
- No KOTOR 1 or TSL mod found that shows weapons holstered on hip/back out of combat.
- Aurora (NWN:EE) has proposals and one abandoned server attempt. No release found.
- Jade Empire: no hits.
- Odyssey precedent is only the K2 floating-saber disguise rig (body-attached, not hip-worn).

## Odyssey / Aurora hits
1. https://deadlystream.com/topic/6525-porting-lightsaber_floating-from-tsl-to-k1 : DarthVarkor 2018 thread. DarthParametric: floating saber is an animated rig ("invisible hand") needing a UTC with a saber on paperdoll plus K1 model recompile. Released as invisible disguise belt g_i_belt123; a 2020 reply says it cannot be equipped. Technique: model + UTC + appearance.2da.
2. https://forums.beamdog.com/discussion/comment/949944 : NWN:EE, Eldrick, Jan 2018, "showing sheathed weapons on character when out of combat". Ancarion: add rhandweapon/lhandweapon nodes and ready poses. JFK: Location var + OnEquip script. Sherincall: a server tried it and scrapped it (inventory too big). Beamdog dev JuliusBorisov added Trello card 127, no feasibility claim. Status: roadmap idea.
3. https://forums.beamdog.com/discussion/comment/455017/ : BelgarathMTH, 2014. Vanilla NWN has a holster mechanic for some NPC talks, plus a quick-button sheathe. No bone info.
4. https://forums.nexusmods.com/topic/168570-weapon-holster-position/ : 2009 thread on moving holstered weapons from behind to waist. One reply says it needs animation changes, another says a 2DA. Game not named; status unknown.
5. https://forums.nexusmods.com/topic/559928-weapon-orientation-when-sheathed : modder says sheathed orientation comes from model coords, placement from skeleton; changing placement seemed to need scripting. Game not named in result.

## Model / rig tooling evidence (K1 and TSL)
6. https://deadlystream.com/topic/7006-%D3%84hrizbys-switching-models-animations-tutorial/ : Ӄhrizby, 2019, K1 and TSL. Weapon helpers "usually named 'rhand' and 'lhand'"; unmoved helpers leave weapons floating. Covers animation transfer. Says nothing on holstering.
7. https://deadlystream.com/topic/1643-how-to-convert-a-hilt-model-between-kotor-and-tsl/ : hilt conversion using KotOR Tool + MDLOps 0.5. Tooling only.

## Other-game analogs (technique only, different engines)
8. https://www.nexusmods.com/fallout4/articles/2496 : FO4 ClassicHolsteredWeapons. Empty node flags a custom skeleton for holster handling; option hides weapon parts while holstered.
9. https://www.nexusmods.com/dragonage/mods/6719 : Dragon Age hip-sheath with matching draw animations. Animation-heavy.
10. https://www.nexusmods.com/oblivion/mods/34766 : Oblivion off-hand saber hanging on hip fixed by deleting OffhandLightsaberQuest script. Script-side fix.
11. https://github.com/SWTOR-Slicers/WikiPedia/wiki/Attaching-weapons-and-other-objects-to-a-character : SWTOR skeleton bones for holstered and wielded weapons.
12. https://modding-openmw.com/mods/weapon-sheathing : OpenMW engine-side sheathing, unready weapons on hip or back.

## Checked, nothing relevant
- KOTOR 2 Nexus 1122 (T3-M4 construction), 1010 (recolor), DeadlyStream 2736 (hilt packs). No holster.
- Steam K2 crossguard thread (app 208580, discussions/3199241400253566544, 2021). No holster mention.
- Restored Content Mod feature text retrieved: no sheathing mention.
- Casey Hudson Q&A https://www.gamebanshee.com/6ux : animation set depends on weapon in hand. No sheathing.
- Reddit r/kotor, r/kotormods, YouTube, GitHub PyKotor/KotorBlender: no matching threads surfaced.
- Obsidian forum 18602 (how K1 holds lightsaber): HTTP 403, not read.

## Feasibility (inference, unsourced)
- Hip/back display likely needs extra dummy nodes on the body MDL plus a visibility toggle tied to equip state (script or appearance row).
- Vanilla K1/TSL out-of-combat hilt behaviour is unverified; no source found.
