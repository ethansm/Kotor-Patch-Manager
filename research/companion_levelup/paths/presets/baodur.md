# Bao-Dur presets

Files: 8 presets (table below). Original three: `baodur__early_guardian.toml` (convert char 12), `baodur__late_guardian.toml` (22), `baodur__tech.toml` (30); added 2026-10-02: unarmed/dual/dark/support early (12) and a never-converting Tech tank.
All start from the LIVE template `live_utc/p_baodur.utc`: Tech Specialist 6, STR14 DEX10 CON14 INT15 WIS14 CHA10, so levels run 7-30.
All pass `check_paths.py` with 0 errors. Warnings are only "1 skill point unspent" (see Skills).

## Wizard answers -> preset (WIZARD_SCHEMA.md v1; `[meta.wizard]` in every file)
| preset file | role | style | alignment | convert (char) | skills | rationale |
|---|---|---|---|---|---|---|
| `baodur__early_guardian` | tank | single | light | early (12) | combat | Heavy armor + Power Attack/Critical/Dueling lines, Valor/Speed/Heal/Barrier/Energy Resistance buffs; the default "Guardian" Bao-Dur |
| `baodur__late_guardian` | melee | single | light | late (22) | tech | Crafts (Gear Head, Caution lines) to 21 as elkripper advises, then short saber/Valor/Heal stretch |
| `baodur__tech` | support | single | light | late (30) | tech | elkripper's literal plan: Tech to 29 (crafting/utility, Toughness), one cosmetic Guardian level |
| `baodur__melee_unarmed_light_early` | melee | unarmed | light | early (12) | combat | Bao-Dur starts with Unarmed Specialist I/II + Complex Unarmed Animations; Guardian levels grant further ranks; no armor feats; Force Body/Valor/Speed buffs |
| `baodur__melee_dual_light_early` | melee | dual | light | early (12) | combat | TWF line from Tech levels 8/11, Dual Strike line, Weapon Focus/Spec Lightsaber; DEX 10 makes to-hit mediocre (user choice) |
| `baodur__melee_single_dark_early` | melee | single | dark | early (12) | combat | DARK Jedi Guardian (conversion talk needs alignment < 40, strategywiki_Bao-Dur.txt:411): Drain Force/Force Scream/Drain Life/Death Field lines; heavy armor, single saber; mod #142 penalizes dark powers only via CHA (CHA 10) |
| `baodur__support_single_light_early` | support | single | light | early (12) | combat | Healer/buffer: Heal line, Valor, Barrier, Revitalize, Battle Meditation, Regenerate Force Points; Treat Injury scales Heal (#142) |
| `baodur__tank_single_none_never` | tank | single | none | never (0) | tech | Tech Specialist 7-30, no Jedi class: heavy armor, Toughness line, Power Attack line, Dueling; for a dark Exile or players who skip the talk |

Coverage notes: no `caster` (Guardian power list has no offensive Force line worth it for Bao-Dur), no `ranged` (blaster/rifle feats exist; not requested), no `neutral` alignment preset. `hold_until_class = 3` (Jedi Guardian) in every converting preset: the mod banks levels from `convert_at` until the story script adds the class. `late_guardian` and `tech` share light/late answers; `role` separates them (melee vs support); the wizard should treat `tech` as the "convert last" fallback.
New presets use the same INT 16-21 progression, Tech slots at char 8/11, Guardian feat slots at Guardian levels 1,2,3,5,7,...; skill allocation is generated round-robin on class skills (combat: Awareness/Treat Injury first; tech: Computer Use/Repair/Security/Demolitions).

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
