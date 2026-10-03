# Visas Marr presets (Jedi Sentinel 7-30)

Files (8 presets): `visas__duelist.toml`, `visas__force_support.toml`, `visas__dark.toml` (original three, wizard block added) and `visas__melee_dual_light.toml`, `visas__melee_double_neutral.toml`, `visas__caster_none_light.toml`, `visas__melee_single_neutral.toml`, `visas__tank_single_light.toml`. All pass `check_paths.py` with 0 errors and 0 warnings (every feat/power slot and all 3 skill points per level are used).
Runtime tag is `VisasMarr` (utc Tag, live_utc/p_visas.utc); the engine compares case-insensitively. Starting state: live template Sentinel 6, level 6, STR12 DEX16 CON10 INT10 WIS12 CHA16, template alignment 35 (lean dark).

## Progression facts
- Already Sentinel (class 5); no conversion. Template level is 6 (live_companions.json p_visas.utc); companions otherwise follow the Exile's XP, so the first row is level 7.
- Prestige classes: StrategyWiki only lets the MAIN character take one (Kreia talk at level 15, `text/strategywiki_Force_powers.txt:296`). Coordinator's Ghidra finding: companions cannot gain prestige classes via the level-up GUI; only scripts (a_makejedi) add classes. So all presets stay class 5.
- Thematic Companions (#8) is applied to the live template (DEX 18->16, CON 12->10, CHA 15->16, vit 48, FP 36, -Two-Weapon Fighting, -Critical Strike, +Slow): `installed_mod_thematic_companions/details.txt:210-233`. Its OPTIONAL Sith Assassin module (:235) is NOT installed (live class is Sentinel).
- She is the only Jedi companion without Force Sensitive (+40 FP): `text/strategywiki_Feats.txt:938`, `details.txt:233`. Force is tight, so casting builds take Regenerate Force Points (143) first.
- Slots (levelup_rules.py): 3 skill points/level (INT mod 0); 1 power every level; feat slots at class levels 7,9,12,13,15,18,19,21,24,25,27,30 (12 feats, 24 powers total).
- Live known powers: Energy Resistance, Fear, Push, Horror, Shock, Wound, Slow, Force Sight, Improved Force Sight. Feats: Flurry, Improved Flurry, Weapon Focus: Lightsaber, Jedi Defense, Force Immunity Fear/Stun, Jedi Sense, Unarmed Specialist I/II.
- Class skills: Computer Use, Stealth, Awareness, Persuade, Security, Treat Injury (validator, skills.2da); elkripper lists no Persuade (`text/elkripper_visas.txt:4`), same omission as the Scoundrel note in PROJECT_CONTEXT.md section 9. At level 6 she has stealth/awareness/treat_injury at 9 = cap (level+3), so those can only gain +1 per level.
- Unlock/recruit: appears on the Hawk after ~1/4 alignment shift; Exile level ~12 fight (`text/strategywiki_Visas.txt:193,212`; boss stats DEX18 CHA15 are the pre-Thematic values, `:226-258`).

## Variants
- duelist: single saber (she has no TWF, mod design). Feats: Dueling line (113/114/115, +1/+2/+3 attack AND defense, `strategywiki_Feats.txt:606`), Finesse: Melee Weapons (194; uses DEX over STR, :638), Advanced/Master Jedi Defense (1, 24), Critical Strike line (8, 19, 81), Master Flurry (53), Toughness (84, 123). Powers: LS/universal buffs and defense (Valor chain, Speed chain, Aura/Shield/Armor, Barrier chain, Heal, Energy Resistance chain, Battle Meditation chain, Stun/Stasis, Force Body chain, Deflection). Class attack bonus is only 0.75/level (mod #10), hence Dueling + Finesse to compensate. Skills: treat_injury, awareness, stealth +1 each (ends 33 each, all at cap).
- force_support: Heal -> Improved/Master Heal, Revitalize line (173/174/175; Master needs level 21), Valor, Barrier, Battle Meditation line, Aura/Shield/Armor, Stun/Stasis/Stasis Field, Affect/Dominate Mind, Force Body. Feats: Regenerate FP (143) first, Conditioning, Toughness, Jedi Defense, Dueling, Regenerate VP (196). Skills: treat_injury (Heal scales with it, mod #142), awareness, persuade.
- dark: Affliction, Drain Force line, Force Lightning, Drain Life, Choke, Insanity, Plague, Kill, Force Scream line, Storm, Death Field, then Speed chain, Whirlwind/Wave, Suppression/Breach, Battle Meditation. Feats: Regenerate FP, Conditioning/Toughness lines, Jedi Defense, Dueling. Skills: awareness, stealth, computer_use (Treat Injury left at 9, she does not heal).

## Open questions for the user
1. Which variant is the default? Recommend duelist (matches her live template and Thematic design) or force_support.
2. Dark variant vs mod #142: DS powers lose up to 50% efficiency (CHA mod offsets 5%/pt; she has CHA16 = +3 so 15%) unless the CASTER's alignment is dark. Her template alignment is 35 (dark-leaning); it is unverified whether #142 reads the companion's own alignment or the Exile's, and whether her alignment moves in play. Her already-known Fear/Horror/Shock/Wound/Slow are dark-side regardless.
3. Skills are proposals only; companions' skills matter mainly when the player controls them (Treat Injury excepted for healing power). Reassign freely.
4. Attribute increases are omitted: no source table for Visas (elkripper has none). Engine auto-increases per finding 6; validator only needs attrs for feat prereqs, none of these feats need them. If the engine's fixed increases are known, add `attrs = {...}` rows.
5. Level-gate-sensitive picks (Master Revitalize 21, Master Heal/Battle Meditation/Storm 18) are fine for levels 7-30 but depend on the Exile's XP reaching that level.

## Source vs live discrepancies
- elkripper/StrategyWiki Visas stats (DEX18 CON12 CHA15, vit 24, FP 18, TWF, Critical Strike) are stale; live has DEX16 CON10 CHA16, vit 48, FP 36, no TWF/Critical Strike, +Slow (details.txt:210).
- Live utc lists the Force Sight/Improved Force Sight pair twice and Weapon Proficiency: Lightsaber three times (template duplication, harmless: validator uses sets).
- The StrategyWiki "Visas" 3-column feat tables on the Classes/Feats pages contain only nav text, no per-companion build data.

## Validator limitations observed
- Attack-bonus prereqs unchecked (no feats here needed it, no warnings emitted). Max rank assumed level+3 and cross-class cost 2 (not verified in game).
- No alignment or LS/DS awareness: dark-vs-mod-#142 conflict cannot be flagged automatically.
- Dueling-style conditions (single weapon equipped) and equipment are not modelled.

## Wizard answers -> preset -> rationale (convert = n/a for all; `convert_at = 0`)
| preset (file) | role | style | alignment | skills | rationale |
|---|---|---|---|---|---|
| duelist (`visas__duelist`) | melee | single | light | combat | Matches her live template/Thematic design (no TWF); Dueling + Finesse, LS buffs |
| force_support (`visas__force_support`) | support | single | light | combat | Heal/Revitalize/Barrier/Battle Meditation healer-controller, Treat Injury at cap (#142) |
| dark (`visas__dark`) | caster | none | dark | stealth | DS caster (Lightning, Drain, Scream, Storm, Kill); needs dark alignment for #142 |
| dual_light (`visas__melee_dual_light`) | melee | dual | light | combat | TWF/Improved/Master TWF (she lost TWF to Thematic; re-taken), Critical Strike line, Master Flurry, LS buffs/Heal, Push line |
| double_neutral (`visas__melee_double_neutral`) | melee | double | neutral | combat | Double-bladed (TWF line applies to double blades), Finesse + Critical Strike; ONLY universal powers (#142 neutral) |
| caster_light (`visas__caster_none_light`) | caster | none | light | social | Offensive Force without DS: Stun/Stasis/Field, Affect/Dominate/Mind Trick, Whirlwind/Wave/Breach; Regenerate FP first; Persuade-weighted skills |
| stealth_neutral (`visas__melee_single_neutral`) | melee | single | neutral | stealth | Scout style: Stealth Run, Caution line, Critical Strike, Dueling line, Mind Trick, Speed; Stealth/Awareness/Security |
| tank_light (`visas__tank_single_light`) | tank | single | light | combat | Toughness/Conditioning/Jedi Defense lines, Aura/Shield/Armor, Barrier, resistances, Deflection |

Notes: `support` role keys on Heal/Revitalize; `tank` and `support` both use `single`/`light`, so role disambiguates. Not covered (illogical or no data): `unarmed` (live has only Unarmed Specialist I/II, no unarmed power feats beyond that), `tech` skill focus (new presets spend points on Stealth/Awareness/Treat Injury/Persuade/Security; Computer Use only in `dark`), light/dark `ranged` (no ranged feats for her). `neutral` presets still keep her already-known dark powers (Fear, Horror, Shock, Wound, Slow) since the template has them. Dual Strike (190-192) is NOT dual wield (teamwork attack bonus), so it is not used for the dual presets; TWF (3/9/85) is. Universal vs light vs dark power groups follow `text/strategywiki_Force_powers.txt:905-1560`; Whirlwind/Wave are Force Push line (universal), Stasis/Stun lines are light.

## B4 preset fixes (2026-10-03, from phase6_evidence/B3_build_review.md)
`melee_dual_light`: TWF stays at L7, Finesse: Melee Weapons (194) at L9; Improved TWF L12, Master TWF L13, Advanced Jedi Defense L15, Critical Strike L18; Master Flurry dropped.
