# Handmaiden presets

Files: `handmaiden__unarmed_jedi.toml`, `handmaiden__saber_guardian.toml`, `handmaiden__unarmed_soldier.toml` (tag `handmaiden`, utc `p_handmaiden.utc`).
Start state = live template: Soldier 6, STR14 DEX16 CON14 INT10 WIS10 CHA13, feats incl. Two-Weapon Fighting, Improved TWF, Conditioning,
Toughness, Improved Toughness, Improved Power Attack, Unarmed Specialist I/II, Echani Strike I; skills awareness 3, stealth 2, treat_injury 2
(`live_companions.json:182-228`). Every file runs char 7-30. INT 10 means 1 skill point per level (Soldier and Guardian), max rank = char level + 3.

## Conversion facts
- Soldier at recruit, "can optionally become a Guardian" (`text/elkripper_handmaiden.txt:4-5`). Only joins a MALE Exile in vanilla
  (`strategywiki_Handmaiden.txt:198`, `elkripper_handmaiden.txt:3`); the user's Exile is female, so she is here via PartySwap (PROJECT_CONTEXT s3, s9).
- Duels in the Ebon Hawk cargo hold: first duel no level gate; second needs the Exile able to level to 14 (91,000 XP), third to 18 (153,000 XP)
  (`strategywiki_Handmaiden.txt:767-790, 806-813`; elkripper says "levels 10, 14 and 18" at `elkripper_handmaiden.txt:12`, and "you can only switch her
  to Guardian after defeating her in three duels" at `:50`). Influence also matters: the mother/training conversation needs 41+ / 26+ / 75+ influence
  checks (`strategywiki_Handmaiden.txt:853-999`). Kreia conversation about her mother is a prerequisite in one branch (`:853-905`).
- Class is added by dialog script (Ghidra-confirmed by coordinator: a_makejedi AddMultiClass), then "you can now level her up as a Jedi Guardian"
  (`strategywiki_Handmaiden.txt:999`). Max 2 classes (Soldier + Guardian). Auto level-up levels the NEWEST class, so after conversion every level is Guardian;
  no preset ever returns to Soldier. AddMultiClass itself may grant XP for one extra level.
- "Do not level or equip her before the duels": her stats stay at Soldier 6 so she is much easier to beat (`strategywiki_Handmaiden.txt:641`;
  `elkripper_handmaiden.txt:51`: "keep her as a level 6 Soldier until pretty far into the game ... she gains a ton of Jedi levels all at once").
  Equipping matters too: duels strip armor/weapons and the robe is regenerated (`:666`).
- Unarmed Specialist is class-level based (class levels 2,6,10,14,18,22,26,30), not character level (`elkripper_handmaiden.txt:43`;
  `strategywiki_Feats.txt:1251`; live feat.2da grants 212-219 at jgd class levels 2/6/10/14/18/22/26/30). Converting at char 7 keeps the line
  advancing (Guardian level k = char level - 6): US III at char 16, IV 20, V 24, VI 28.
- Battle Precognition comes from the conversation after the first duel (`strategywiki_Handmaiden.txt:767-782`), independent of leveling.
- No source mentions TSLRCM or PartySwap changing her conversion gates; unverified.

## Hold-until-Jedi design
Because levels 7+ are all Guardian in all conversion variants, the file is correct whatever Exile level conversion happens at: hold_until_class = 3 tells
the mod to bank (not auto-spend) levels until class 3 exists; then it replays levels 7... in order, one per pending level. If the player manually levels her
as Soldier before converting, the preset is off (Soldier 7+ would exist and Guardian levels would be class level 1.. at a later char level, slots and
Unarmed Specialist timing shift). `convert_at = 7` records "first char level taken as Guardian"; the real-world earliest is when the third duel unlocks
(Exile 18 per elkripper; possibly earlier, see questions).

## Variants
1. `unarmed_jedi`: elkripper "Unarmed Jedi" (`text/elkripper_handmaiden-unarmed-jedi.txt`). Feats Improved/Master Conditioning, Master Toughness, Regenerate
   Vitality Points, Dueling line, Master Power Attack, Flurry line, Dual Strike line. Powers follow elkripper's order (Burst of Speed, Heal, Force Barrier line,
   Speed line, Deflection/Redirection, Heal line, Resistance, Immunity, Aura/Shield/Armor, Energy Resistance line, Valor line) with the three dark powers
   (Drain Life, Death Field, Drain Force) replaced by Battle Meditation / Improved / Master (138, 139, 140) per Alignment Affects Force Powers
   (README finding 4). Valor line is late (as in elkripper) since Valor is a buff she may get from the Exile; could be moved up.
2. `saber_guardian`: same conversion, but lightsaber: Weapon Focus + Specialization: Lightsaber, Dueling line, Advanced/Master Jedi Defense, Close Combat,
   Master Power Attack, Conditioning line, Regenerate VP. Powers lead with Burst of Speed, Heal, Valor, Knight Speed, Barrier, Master line later. Classic Class
   Attack Bonus favors Guardian (README finding 3).
3. `unarmed_soldier`: elkripper "Unarmed Soldier" (`elkripper_handmaiden-unarmed-soldier.txt`), Soldier 7-30, never converts, no powers. Feats beyond elkripper's
   char 20 limit (22-30) are my fill (Improved/Master Dual Strike, Close Combat, Improved Close Combat, Mobility). Hold is not needed.
All skill allocations are proposals: Awareness to rank 15 by char 18, then Treat Injury to 14 (Treat Injury also scales Force Healing per mod #142).

## Wizard presets (8)
Questions: role / style / alignment / convert / skills (WIZARD_SCHEMA.md). All files char 7-30, check_paths 0 errors, 0 warnings. Skill focus is `combat`
(Awareness then Treat Injury) except support (`social`, Persuade is a Guardian class skill; stealth is cross-class for Guardian, so no stealth preset).
| Preset file | role | style | alignment | convert | skills | Rationale |
|---|---|---|---|---|---|---|
| `unarmed_jedi` | melee | unarmed | light | early | combat | elkripper Unarmed Jedi; Echani unarmed identity |
| `saber_guardian` | melee | single | light | early | combat | classic Guardian: Weapon Focus/Spec, Dueling line |
| `unarmed_soldier` | melee | unarmed | none | never | combat | elkripper Unarmed Soldier, no Force |
| `melee_dual_light_early` | melee | dual | light | early | combat | Master TWF, Dual Strike line, Critical Strike line; she starts with TWF/Improved TWF |
| `tank_single_light_early` | tank | single | light | early | combat | Toughness/Conditioning/Jedi Defense line, Barrier/Deflection/Armor first |
| `melee_single_dark_early` | melee | single | dark | early | combat | DARK: Lightning, Drain Life/Force, Choke, Scream, Death Field (#142 needs a dark Exile) |
| `melee_unarmed_light_late` | melee | unarmed | light | late | combat | Soldier 7-12 (unarmed Soldier feats), Guardian from char 13 (Guardian 1-18); convert_at = 13 |
| `support_single_light_early` | support | single | light | early | social | Heal line, Revitalize, Battle Meditation, Valor; minimal weapon feats, Persuade |

Not covered (no logical/valid build): ranged/pistols/rifle (feasible via Soldier feats only, no source), caster (Guardian lacks the power depth), `convert = never`
for anything but unarmed (would need an invented Soldier feat path), dual + dark, tank + dark. `skills = tech/stealth` impossible (cross-class, 1 point/level).
Note on `late`: Soldier levels 7-12 happen BEFORE the duels' level gates are known to matter; leveling her as Soldier before the duels makes them harder (see Conversion facts).

## Validation notes
- check_paths: 0 errors, 0 warnings on all three files (every feat/power slot filled; attack-bonus prereqs are not verified by the checker).
- Attribute lines (STR +1 at 8/12/24/28, CON +1 at 16/20) are copied from elkripper (auto-increases); not verified against engine/live.

## Open user questions
- Is she meant to convert at all this playthrough (unarmed_soldier vs Jedi)? Which weapon style (unarmed vs saber)?
- Convert as early as the sources allow, or after Exile 18? (Needs to be known only for the mod's hold timing, not for the picks.)
- Valor line late or early? Keep Force Aura/Shield/Armor for her at all, or give priority to Revitalize (173-175) instead?
- Accept Battle Meditation as the light-side replacement for the Drain line?

## Source vs live discrepancies
- elkripper starts CHA14, skills 21 points (Awareness 8 etc.); live CHA13, Awareness 3/Stealth 2/Treat Injury 2 (Thematic Companions, `details.txt:62-74`).
  Live skills total 7 of 9 intended points: 2 may be unspent in the template (unknown).
- Strategywiki says conversion talk can start via first-duel influence branches (`:853-905`) whereas elkripper and README say three duels are required;
  not resolved from text alone. Verify in game.
- live feat.2da grants Soldier no Unarmed Specialist/Echani Strike at class levels (sol_granted = -1): Soldier's progression (elkripper's Echani Strike II at
  Soldier 11/17, US III at 10 ...) comes from script or another mechanism, not in the 2DA; the Guardian side is 2DA-granted. Cannot be checked statically.
- Elkripper's Jedi table assumes Guardian 1 at char 7; reality is char level at conversion (hold design above).
- `late` preset: the semantics of `hold_until_class` there (banking only from convert_at, not from level 7) are my assumption; the mod must replay Soldier levels 7-12 at once and bank 13+.
- Dark preset: which of these powers #142 treats as dark (Drain Force/Improved, Scream, Choke) was not re-verified from the mod data; Barrier line added as neutral filler.
