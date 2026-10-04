# R10 feat/power tag table (doc 25 sections 2 + 11)

`gen_r10_tags.py` writes `../runtime/r10_tags.ini` (INI sections `[feats]`, `[powers]`; `<2da row id>=<tag>,<tag>`; ids ascending, tags in
vocabulary order, LF, ASCII, lines < 200 chars, byte-identical reruns). Hand fixes live in `tag_overrides.toml` (each with a reason).

```
cd research/companion_levelup
../../.venv/bin/python3 r10/gen_r10_tags.py             # write the ini
../../.venv/bin/python3 r10/gen_r10_tags.py --check     # stale diff + coverage + preset fit; exit 0 only when clean
../../.venv/bin/python3 r10/gen_r10_tags.py --selftest  # format / sort / fits rule, no game files
```

## Vocabulary (canonical order)
role `melee ranged caster support tank` | style `single dual double unarmed pistols rifle` | alignment `light dark neutral` |
other `general force combat`. `general` = fits every build; `force` = Force feats and every Force power; `combat` = non-Force combat feats.
`force`/`combat` do not take part in the Build fit rule; they exist for the Combat / Force filter modes (doc 25 section 11).

## Wizard answer -> build mapping (WIZARD_SCHEMA.md)
| wizard key | build field | values used by the fit rule |
|---|---|---|
| `role` | build.role | `melee ranged caster support tank` (same words as the role tags) |
| `style` | build.style | `single dual double unarmed pistols rifle`; `none` matches no style tag |
| `alignment` | build.alignment | `light dark neutral`; `none` (no-Force build) conflicts with nothing |
`convert` and `skills` are not used by the tag filter.

## Fit rule (identical for `--check` and the C++ filter)
A row FITS a build when it has `general`, OR (any role tag == build.role OR any style tag == build.style) AND its alignment tag is not
opposite to build.alignment. "Opposite" = `light` row vs `dark` build, or `dark` row vs `light` build only. `neutral`, untagged alignment, and a
build alignment of `neutral`/`none` never conflict. Implemented as `fits()` in gen_r10_tags.py.

## Selectable scope (exact definitions)
- Feats: `feat.2da` row with non-empty label (not `****`) and `<cls>_list` in (`0`,`1`) (0/1 = selectable, 3 = granted, 4 = n/a, as in
  levelup_rules.feat_selectable) for at least one class column of scd sol sct jcn jgd jsn sas sld sma jwa jma jwm tec drx drc. 82 rows.
  Rows with only `allclassescanuse=1` but no list entry are class-granted, not selectable, and are not tagged.
- Powers: `spells.2da` row with non-empty label, `usertype == 1` (Force power; 2/4/6 = special/item/form abilities, -2 = dead rows), and at
  least one class-level column (`guardian consular sentinel weapmstr jedimaster watchman marauder sithlord assassin`) in 0..98 (-1 = class
  cannot learn it, 99 = never selectable, e.g. Force Sight). 73 rows.

## Feat rules (feat.2da label regex, first match wins; tags in vocabulary order)
| rule | label match | tags |
|---|---|---|
| F1 | ADVANCED_/MASTER_JEDI_DEFENSE | melee tank combat (both overridden to general combat) |
| F2 | TWO_WEAPON_*, SUPER_WEAPON_FOCUS_2_WEAPON* | melee dual pistols combat |
| F3 | *DUAL_STRIKE | melee dual combat (all three overridden to general combat) |
| F4 | ARMOUR_PROF_* | melee ranged tank support combat |
| F5 | (IMPROVED_/MASTER_) CRITICAL_STRIKE, POWER_ATTACK, FLURRY; WHIRLWIND_ATTACK | melee tank combat |
| F6 | (IMPROVED_/MASTER_) POWER_BLAST, SNIPER_SHOT, RAPID_SHOT; MULTI_SHOT; PRECISE_SHOT_*; (IMPROVED_) CLOSE_COMBAT | ranged pistols rifle combat |
| F7 | WEAPON_FOCUS/PROF/SPEC_BLASTER_RIFLE | ranged rifle combat |
| F8 | WEAPON_FOCUS/PROF/SPEC_BLASTER | ranged pistols combat |
| F9 | WEAPON_FOCUS/PROF/SPEC_LIGHTSABER or _MELEE_WEAPONS; FINESSE_*; SUPER_WEAPON_FOCUS_LIGHTSABER* | melee single dual double combat |
| F10 | *DUELING | melee single combat |
| F11 | REGENERATE_FORCE_POINTS, FORCE_* | force caster support |
| F12 | *CONDITIONING, *TOUGHNESS, *CAUTION, CAUTIOUS, *EMPATHY, GEAR_HEAD*, CLASS_SKILL_*, MOBILITY, REGENERATE_VITALITY*, STEALTH_RUN | general |
Feat alignment: no selectable feat carries an alignment tag (the 2DA has no alignment column for feats).

## Power rules
- Tags = `force` + all five roles + alignment. Alignment comes from `spells.2da` column **`goodevil`**: `G` -> light, `E` -> dark, `-` -> neutral.
- Why every role: the preset check showed melee/ranged/tank companion presets take the same offensive and control powers as casters (Choke,
  Wound, Stun, Speed, Suppress Force). A first attempt with role narrowing (caster only, +support for `forcefriendly`, +tank/melee by label)
  gave hundreds of preset conflicts and was dropped. For powers only alignment narrows the Build filter; `force` drives the Force mode.

## Overrides
`tag_overrides.toml`: 16 feat overrides (every one found by `--check`, reason inline) and 0 power overrides.
`[[preset_waiver]]` (2 entries, 13 power rows): `kreia__support_none_dark.toml` and `handmaiden__melee_single_dark_early.toml` are labelled
alignment=dark but take light-side powers (Heal, Cure, Valor, Force Armor/Shield/Barrier/Aura; Force Barrier). Those are preset label
defects, so the tags stay truthful (`goodevil=G` -> light) and the conflict is waived instead of loosening the rule. Fix = relabel the preset.

## Coverage and preset check (2026-10-04)
- feats 82/82 tagged, powers 73/73 tagged (every selectable row has at least one tag).
- 80 companion presets checked (every feat and power any level takes must fit that preset's wizard answers): conflicts 0 after overrides;
  waived 13 power picks in 2 presets (above). First run before overrides: about 60 distinct feat/preset conflicts and 14 power conflicts.
- Known limit: the rule-derived feat tags are judgement calls from labels (e.g. Empathy, Caution, Gear Head -> general); preset coverage only proves
  the tags are not too narrow for the 80 curated builds, not that they are minimal.

## Model spot-check
PENDING: Haiku spot-check of 20 random rows
