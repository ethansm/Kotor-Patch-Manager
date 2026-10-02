# Kreia presets (Jedi Consular 4-30, single class, no conversion)

Files: `kreia__support_none_light.toml`, `kreia__support_none_neutral.toml`, `kreia__support_none_dark.toml`, `kreia__caster_none_light.toml`,
`kreia__caster_none_dark.toml`, `kreia__melee_single_neutral.toml`, `kreia__tank_single_light.toml`. All pass `check_paths.py` with 0 errors.
`wizard.convert = "n/a"`, `convert_at = 0`. The live path `paths/kreia.toml` is untouched; `support_none_dark` is an unmodified copy of it plus `[meta.wizard]`.

## Starting state (live_companions.json, p_kreia.utc)
Consular 3 (first row = level 4). STR10 DEX12 CON14 INT14 WIS16 CHA12; stealth 6, awareness 6, persuade 5, security 3, treat_injury 6, repair 0.
Powers: Burst of Speed, Energy Resistance, Fear, Force Camouflage, Force Push, Force Sight. Feats include Dueling (113), Finesse: Melee Weapons (194),
Jedi Defense (55), Force Focus (88), Class Skill: Stealth (188), Mentor, Force Chain. Elkripper's DEX16/CON16 start is stale (Thematic Companions; README Finding 1).

## Rules used
- 36 power picks (classpowergain: 2 powers at class levels 6,9,12,...,30, 1 otherwise) and 9 feat picks (levels 6,9,...,30); 4 skill points/level (base 2 + INT mod 2, `levelup_rules.py`).
- Consular attack bonus is only 0.5/level (mod #10, README Finding 3): melee/tank presets are weak damage dealers by design and lean on Dueling + Weapon Focus.
- Alignment tags come from spells.2da `goodevil` (G / E / -) used with mod #142 (`text/deadlystream_142_Alignment_Affects_Force_Powers.txt:82`, CHA mod reduces the opposite-side penalty 5%/pt; Kreia CHA12 = +1). Light presets contain only G and universal powers; dark preset only E and universal.
- Force Chain (`text/elkripper_kreia.txt:7-14`): Speed, Energy Resistance, Aura, Heal, Force Resistance, Barrier also affect the Exile, which is why the support presets lead with them.
- Consular class skills Awareness/Repair/Treat Injury (`elkripper_kreia.txt:15`); Stealth is class via granted feat 188.
- Attributes: only WIS +1 at 4,8,...,28 (elkripper table, `elkripper_kreia.txt` level 4 row), identical in all presets (engine fixed, README Finding 6).
- "Repair Affects Stun Droid" is cited in the mod #142 text (:68), basis for the tech preset.
- Other facts: Force Camouflage is unique to Kreia (`strategywiki_Force_powers.txt:402`); Finesse: Melee covers lightsabers (`strategywiki_Feats.txt:649`); class skills cost 1, cross-class 2 (`strategywiki_Skills.txt:270`); elkripper's own build commentary `elkripper_kreia.txt:301`.

## Presets -> wizard answers
| file | role | style | alignment | skills | rationale |
|---|---|---|---|---|---|
| support_none_light | support | none | light | combat | Valor, Aura, Heal line, Shield/Armor, Barrier line, Battle Meditation, Revitalize; Treat Injury maxed (Heal scales with it). Feats: Regenerate FP, Toughness, Conditioning, Jedi Defense. Default recommendation. |
| support_none_neutral | support | none | neutral | social | Universal powers first (Force Body line, Battle Meditation, Speed, Deflection, Resistance, Affect/Dominate Mind, Push line, Throw Lightsaber); Persuade focus. |
| support_none_dark | support | none | dark | combat | The existing elkripper-style path: light buffs early, dark debuffs (Plague, Affliction, Kill, Drain Force) from level 21. Hybrid, see Q3. |
| caster_none_light | caster | none | light | tech | Control caster: Stun/Stasis/Stasis Field, Disable/Destroy/Stun Droid, Whirlwind/Wave, Suppression/Breach, Dominate Mind; Repair 33, Computer Use 25 (cross-class). |
| caster_none_dark | caster | none | dark | combat | Shock, Lightning, Storm, Wound/Choke/Kill, Slow/Affliction/Plague, Drain Life/Death Field, Drain Force and Scream lines. Needs a dark Exile/Kreia alignment under #142. |
| melee_single_neutral | melee | single | neutral | stealth | Single saber: Improved/Master Dueling, Weapon Focus: Lightsaber, Critical Strike line, Advanced Jedi Defense, Toughness; universal powers. |
| tank_single_light | tank | single | light | combat | Toughness line, Jedi Defense, Dueling defense, self-buffs (Shield/Armor/Barrier/Valor/Resistance/Immunity), Heal. |
Not covered (nonsense for her): dual/double/unarmed/pistols/rifle styles (no TWF feats selected; she has only a blaster-pistol proficiency), `convert` answers.

## Deliberate warnings
- `support_none_light`, `support_none_neutral`, `support_none_dark`, `tank_single_light`: 1 skill point/level unspent from about level 10-25 (21-27 warnings). Awareness/Treat Injury/Repair reach the rank cap (level+3) and `check_paths.py` charges Stealth 2 points because it ignores feat 188 (Class Skill: Stealth); in game Stealth should cost 1, so the point buys +1 Stealth.
- No other warnings (`caster_none_light`, `caster_none_dark`, `melee_single_neutral`: 0).
- Attack-bonus and weapon-equipped conditions (Dueling) are not validated.

## Open questions
1. Neutral presets: the universal power pool is about 25 powers, 36 are needed. Filler after the pool is light-side (neutral alignment = vanilla strength, no penalty). Acceptable, or leave slots unspent?
2. `support_none_dark` is a hybrid; under #142 a dark Kreia halves her light buffs (Heal/Valor/Barrier). Should it be replaced by a pure dark-support build (Drain Force, Horror/Insanity, Slow, Speed/Battle Meditation)?
3. Does #142 read Kreia's own alignment or the Exile's? (Same unknown as Visas.) Alignment answers assume her own.
4. Elkripper says she should rarely melee (`elkripper_kreia.txt:301`); is a melee/tank preset wanted at all with 0.5 attack/level?
5. Confirm in game: stealth cost 1 via feat 188, skill rank cap level+3, Force Chain list; whether Kreia uses points at all when AI-controlled.
