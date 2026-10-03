# Mira presets (tag `mira`, template `live_utc/p_mira.utc`)

Start state (live): Scout 6, STR14 DEX16 CON13 INT12 WIS12 CHA10 (`live_companions.json` p_mira.utc). First level-up is char level 7. INT 12 gives 4 skill points/level (Scout and Sentinel skillpointbase 3 + INT mod 1). Light/neutral Mira only (see Conversion).

Files: the 8 presets in the wizard table below (3 original + 5 added 2026-10-02). All 8: 0 errors, 0 warnings in `check_paths.py`. Each [meta] carries `variant`, `summary`, `convert_at` (first Jedi char level, 0 = none).

## Conversion facts
- Engine side (coordinator, Ghidra): the Jedi class is added only by script (`a_makejedi.nss`, AddMultiClass), never via the level-up GUI; AddMultiClass itself grants XP for one extra level; auto level-up always levels the newest class, so after conversion every level is Sentinel (all presets obey this: Scout levels, then Sentinel only); max 2 classes (Scout+Sentinel only).
- Gate is dialogue + influence, not a character level. strategywiki_Mira.txt:578-640: "Jedi" talk on Ebon Hawk or Nar Shaddaa ("You seem restless" -> Nar Shaddaa line), requires Influence 10-90 (line 582-586), then take her to the Refugee Landing Pad east side of the pit (590) and play the Kreia "listen to the Force" scene (needs the Kreia lesson already done there, elkripper_mira.txt:100). Then "I can train you" ends with the mira-becomes-Jedi note; Alignment >= 40 gives light Jedi Sentinel (669-690), < 40 dark Sentinel (649-668, influence -6). Conversion happens on the spot.
- Exile Awareness 11 (elkripper_mira.txt:98-99; strategywiki_Mira.txt:389,401,492 "[Awareness (11)]"): elkripper lists it as a requirement for the conversion route; StrategyWiki shows it only on the "why do you hunt/not kill" options which raise influence (+8 each, lines 428-466). Influence sources (elkripper_mira.txt:101-125): Ebon Hawk talks, Nar Shaddaa refugee heal, Dantooine (Akkerre), Duxn, Onderon. TSLRCM: elkripper says enough influence is available from Ebon Hawk dialogue alone (line 107); no source documents a TSLRCM change to the conversion level.
- Earliest realistic: she is recruited late on Nar Shaddaa (strategywiki_Mira.txt:194, only after Visquis message, Alignment >= 40), so the Kreia scene and the influence talks can be done before she gains any XP. `early_sentinel` therefore assumes conversion at char 6 with the first Sentinel level = 7 (a mod that banks levels until the Jedi class exists is required; AddMultiClass's free level covers this). If conversion happens later in practice the preset must be re-based (a variant with another convert_at); see questions.

## Variants
1. `ranged_sentinel` (convert_at 17): elkripper_mira.txt:7-95 (switch table, "17/18 strongest late game", priority list Master Speed/Heal/Deflection, Precise Shot line, Master Rapid Shot, Master TWF) and per-level table :129-352. Scout 7-16 takes Improved Rapid Shot, Improved Close Combat, Master Rapid Shot, WF Blaster Pistol, TWF (5 slots; Targeting/Precise Shot/Uncanny Dodge granted). Sentinel 17-30: Improved TWF, Master TWF, Precise Shot V, Improved/Master Sniper Shot, Improved/Master Critical Strike. Powers (15): Burst of Speed, Heal, Knight/Master Speed, Improved/Master Heal, Force Deflection/Redirection, Battle Meditation line, Force Barrier line, Stun. Master Valor dropped (elkripper: Exile usually has the Valor line; Mira "might consider it" otherwise). Skills: Scout phase Treat Injury/Demolitions/Computer Use/Repair; Sentinel phase Treat Injury/Awareness/Computer Use/Security/Stealth. Force Heal scales with Treat Injury under mod #142.
2. `early_sentinel` (convert_at 7): elkripper explicitly notes early switch = more Force powers (table row "13 -> 18 powers"); here 25 powers at 30. Dual-wield saber: TWF/Improved/Master TWF, Dual Strike line, WF Lightsaber, Advanced/Master Jedi Defense, Improved Close Combat, Conditioning. Light-side kit: Speed line, Heal line, Force Valor line (Mira buffs herself; note Exile may also), Deflection/Redirection, Force Barrier + Battle Meditation lines, Aura/Shield/Armor, Stun/Stasis/Stasis Field, Revitalize I/II (spells.2da goodevil = G, no E powers used). Sentinel skills: Treat Injury, Awareness, Computer Use, Security, Stealth (Demolitions/Repair frozen: cross-class).
3. `scout` (convert_at 0): weakest sourcing. elkripper_mira.txt:3-4 only says to delay conversion, not to skip it. Included for a player who never converts (Hanharr route, dark Exile, or conversion missed). Scout feat slots at 7,9,...,29: pistols/rapid shot/TWF line, Improved Close Combat, sniper and critical chains, Toughness. No Force powers. Demolitions/Repair/Treat Injury/Computer Use all class skills.

Attributes (all variants, `attrs` key): DEX 17/18/19/20 at levels 8/12/16/20 (elkripper), CON 14 at 24, 15 at 28 (elkripper's CON +1 at 24/28 applied to live CON13; the `attrs` values are what the validator uses, the engine applies companion attributes automatically; unverified live).

## Wizard answers -> preset (v1, 8 presets; each file has `[meta.wizard]`, `hold_until_class = 5` on converting presets)
| preset file (`mira__...`) | role | style | alignment | convert | skills | rationale |
|---|---|---|---|---|---|---|
| `early_sentinel` | melee | dual | light | early (7) | combat | dual-saber light Sentinel, 25 powers, Valor/Heal/Barrier kit (elkripper "early switch = more powers") |
| `ranged_sentinel` | ranged | pistols | light | late (17) | combat | elkripper pistol/rapid-shot Scout then Sentinel at 17 (his "strongest late game" timing) |
| `scout` | ranged | pistols | none | never (0) | tech | no Force; Demolitions/Repair/Computer Use/Treat Injury are all Scout class skills (cheap), grenade+rocket+medic utility |
| `melee_dual_dark_early` | melee | dual | dark | early (7) | combat | dark Sentinel (strategywiki 649-668, needs Exile Alignment < 40): Speed/Deflection + Wound/Shock/Choke/Lightning/Drain Life/Fear line/Kill/Plague/Storm/Death Field + Push line; no light powers (#142) |
| `support_single_light_early` | support | single | light | early (7) | social | Heal/Improved/Master Heal, Revitalize I-III, Valor, Barrier, Battle Meditation, Stasis; Dueling line (single saber); Persuade is a Sentinel class skill so it costs 1 per rank |
| `caster_none_light_early` | caster | none | light | early (7) | combat | offensive Force: Push/Whirlwind/Wave/Breach/Suppression/Throw Lightsaber, Stun/Stasis/Stasis Field/Disable/Destroy Droid, Heal late; defensive feats only (Conditioning, Toughness, Mobility, Jedi Defense) |
| `ranged_rifle_none_never` | ranged | rifle | none | never (0) | combat | Scout 7-30 with rifle: WF Blaster Rifle, Sniper/Critical Strike/Power Blast lines, Awareness/Treat Injury |
| `ranged_rifle_light_late` | ranged | rifle | light | late (13) | stealth | sniper Scout to 12 (WF rifle, Improved Sniper, Improved Critical), Sentinel from 13 for Master Sniper/Critical, Speed/Heal/Deflection/Valor; Stealth/Security (Sentinel class skills) + Awareness |

Not covered on purpose: tank, double-bladed, unarmed, pistols+early (Sentinel feat slots come too late to keep the pistol chain), social Scout (Persuade is cross-class for Scout), dark+late/ranged (dark Mira is a story branch, one preset is enough for v1). Notes: Critical Strike (8) is already granted to Mira, Dueling (113) too; conversion level 13 for `ranged_rifle_light_late` is a proposal (no source), the Kreia scene + influence gate in the Conversion section must be met by then or the mod holds levels.

## Open questions for the user
1. Convert early (early_sentinel), at 17 (ranged_sentinel) or never? Early melee Sentinel vs late pistol/Scout hybrid is the main choice.
2. Will the Exile reach Awareness 11 when Mira is converted (elkripper) - or rely on influence only (StrategyWiki)? Test in game.
3. Valor line and Heal line: should Mira duplicate the Exile's? (early_sentinel takes Valor, ranged_sentinel does not.)
4. Dark-side Exile (Alignment < 40) gives a Dark Sentinel Mira; these presets are light side only.
5. Max level used: 30 (elkripper assumes 29; level cap with TSLRCM unverified).
6. Skill points are proposals; priorities OK (Treat Injury high for Heal potency; no Persuade)? Class Skill feats (Security/Stealth for Scout, Demolitions/Repair for Sentinel) not used.

## Discrepancies sources vs live
- Elkripper start table (STR10 INT14 CON12 WIS11, skills Repair 8/CU 8/Dem 8/Awareness 8, 39 pts) is vanilla; live is STR14 INT12 CON13 WIS12, CU 6 Dem 6 Awareness 6 Repair 5 (Thematic Companions, details.txt:174-198). Fewer skill points per level (4, elkripper 5).
- Live feats at level 6 match elkripper's granted list (including Precise Shot I and Wrist Mounted Rockets proficiency); no discrepancy found.
- Elkripper's table has Scout level 7 skills "5 points"; live is 4.
- elkripper end level 29/30 vs this task 30; Sentinel slot table (featgain/classpowergain) confirmed from live rules via check_paths.
- Elkripper plans CON bumps at 24/28 from CON 12 (=13/14), live +1 gives 14/15 (unverified).

## B4 preset fixes (2026-10-03, from phase6_evidence/B3_build_review.md)
- `early_sentinel` and `melee_dual_dark_early`: TWF stays at L7, Finesse: Melee Weapons (194) takes the next slot (L9); later feats up to the dropped Master Dual Strike shift one slot, Master Dual Strike dropped. In `early_sentinel` the ranged-only Improved Close Combat (L25) became Toughness (net after the shift).
- `ranged_rifle_light_late`: Treat Injury first (Scout and Sentinel class skill), end Treat Injury 33 (was 11); Stealth 29, Security 24, Awareness 19, Computer Use 12 (was 33/33/25/15).
