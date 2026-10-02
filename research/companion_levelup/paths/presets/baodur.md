# Bao-Dur presets

Files: `baodur__early_guardian.toml` (convert char 12), `baodur__late_guardian.toml` (convert char 22), `baodur__tech.toml` (convert char 30).
All start from the LIVE template `live_utc/p_baodur.utc`: Tech Specialist 6, STR14 DEX10 CON14 INT15 WIS14 CHA10, so levels run 7-30.
All pass `check_paths.py` with 0 errors. Warnings are only "1 skill point unspent" (see Skills).

## Conversion facts
- Joins on Telos (Restoration Zone), must stay in party until Polar Plateau: `text/strategywiki_Bao-Dur.txt:150`.
- Gate = influence + dialogue, NOT a level gate. Three chained Ebon Hawk conversations: "Why did you decide to fight?" (~influence 60),
  "I'm glad I found you again" (~75), then the Jedi talk (~89, final) `strategywiki_Bao-Dur.txt:296-413`. The wiki text is garbled ("< 11 Influence > 89"):
  thresholds 60/75/89 are my reading, UNVERIFIED. Wording "you don't want him to gain any more levels as a Tech Specialist" (line 366) means the player decides when.
- Alignment < 40 (final talk, dark branch needs alignment < 50 and 89 influence) -> "Dark Jedi Guardian"; otherwise "Jedi Guardian" (lines 411-413). Presets assume light side.
- Mechanics (coordinator, Ghidra): class added only by script a_makejedi (AddMultiClass), max 2 classes, auto level-up always levels the newest class, so after conversion every level is Guardian (all presets obey; no return to Tech).
- No source gives a level at conversion. elkripper only models switching at 30 (`text/elkripper_baodur.txt:395`, "Jedi Guardian 1 ... LOL"), and notes he "could become a Jedi earlier ... just for story purposes; converting requires joining the party for influence" (lines ~390-396).
  Influence sources in elkripper (lines 22-60): Telos, Dantooine sensors, Nar Shaddaa airspeeder/Rutum/Lootra/refugee, Duxn, Onderon. My estimate: earliest realistic after Nar Shaddaa = char ~12 (ESTIMATE, not sourced).
- TSLRCM: no source in `text/` mentions its effect on Bao-Dur conversion (README says 1.8 reworked post-Jedi content; unconfirmed).
- Unarmed Specialist: after conversion future ranks come at JEDI class level (2,6,10,14,...) not char level: `strategywiki_Feats.txt:1251`; feat.2da jgd_granted 212..219 = 2,6,10,14,18,22,26,30. Presets therefore pick no Unarmed Specialist (granted) and the early variant gets III at char 21, IV at 25, V at 29. Tech levels have no 2DA grant (tec_granted -1), the "Special Training" source of III-VII at Tech levels is not in feat.2da.
- Planned conversion char levels: early 12, late 22, tech 30 (`convert_at` meta key). The mod can hold/bank levels until the Jedi class appears.

## Variants
Common: INT 16/17/18/19/20/21 at char 8/12/16/20/24/28 (elkripper, fixed companion growth; assumed to continue after conversion - unverified). Heavy Armor (feat 4, selectable via mod #108) taken at char 8 in all. Light-side powers only (mod #142; Alignment Affects Force Powers).
No dark powers, no Class Skill feats (checker does not model them; Guardian already has Treat Injury as class skill).

1. early_guardian (convert 12): Tech 7-11 then Guardian 1-19. Melee lightsaber tank in heavy armor. Feats: Heavy Armor, Improved Power Attack (Tech); Weapon Focus/Specialization Lightsaber, Master Power Attack, Critical Strike line, Improved/Master Conditioning, Dueling line (Guardian). Powers (20): Valor line to Master Valor, Burst of Speed line, Heal/Improved/Master Heal, Force Deflection/Redirection, Force Barrier line, Energy Resistance line, Battle Meditation line, Force Shield. Skills: Tech phase +1 on computer/awareness/repair/security/demolitions plus Treat Injury; Guardian phase Treat Injury and Awareness to cap, then Demolitions/Repair/Computer/Security.
2. late_guardian (convert 22): Tech 7-21 as crafter (Improved/Master Gear Head, Caution, Improved Caution), then 9 Guardian levels (10 powers: Valor line, Speed line, Heal line to Master Heal, Force Deflection). elkripper: best crafts by 22 with Master Valor, by 28 without.
3. tech (convert 30): elkripper's literal plan: Tech to 29 (Heavy Armor, Gear Head line, Caution line, Toughness line), Guardian 1 at 30 (Master Toughness, Force Valor + Heal). Utility/crafting character; Guardian level is cosmetic.

## Skills
Allocation is a proposal (class skills first, one rank each per level to the char_level+3 cap). Checker-conservative: ranks counted at cross-class 2 for Treat Injury/Stealth (Tech) and Repair/Computer/Security (Guardian). Single "1 point unspent" warnings occur when all class skills are at cap and only cost-2 skills remain: deliberate (point banks, per elkripper "bank").

## Open questions for the user
1. Pick one variant per playthrough; is conversion level (12/22/30) acceptable, or should the mod convert when the dialogue becomes available and re-slot?
2. Lightsaber vs unarmed Guardian (Unarmed Specialist keeps being granted at Jedi levels)? Presets assume lightsaber (Weapon Focus/Spec Lightsaber).
3. Dark-side alternative preset (alignment < 40 conversion)?
4. Confirm in game: thresholds 60/75/89, INT growth after conversion, whether Heavy Armor mod gives the feat.

## Discrepancies sources vs live
- elkripper start (Flurry, Critical Strike, Improved Gear Head granted) vs live: Thematic Companions (#8) removed Flurry/Critical Strike and replaced Improved Gear Head with Gear Head. Live has feat 12 four times and no 119; presets therefore pick Improved/Master Gear Head (119, 120) in late/tech variants (elkripper "Master Gear Head" at 14 needs 119).
- Live also has feats 207 (Complex Unarmed), 212, 213, 220 (Shield Breaker); no Persuade.
- Live Tech Specialist skill points: base 4 + INT mod (matches elkripper 7 at INT16); Guardian base 1 + INT mod per classes.2da (only 4-6 points/level).
