# Hanharr presets (tag `hanharr`, template `live_utc/p_hanharr.utc`)

Start state (live, `live_companions.json`): Scout 4, STR18 DEX13 CON18 INT10 WIS12 CHA8, HP base 32, skills Awareness 4 / Demolitions 6 / Stealth 3 / Treat Injury 5 / rest 0. First level-up is char level 5. Scout skillpointbase 3 + INT 0 = 3 points/level. Scout feat slots at 5,7,...,29 (13 slots). Never converts (not a Jedi candidate; dark-side Mira replacement, "Choose Mira or Hanharr" #95). Class skills: Computer Use, Demolitions, Awareness, Repair, Treat Injury; Stealth/Persuade/Security are cross-class (2 points/rank).

## Sources
- `text/elkripper_hanharr_{melee,pistol,rifle}.txt` (raw: `raw/web/hanharr_elkripper_*.html`): per-level tables, vanilla start (STR20 CON20, Repair 4, Close Combat).
- `text/gamebanshee_hanharr.txt` (raw: `raw/web/hanharr_gamebanshee.html`): overview only.
- StrategyWiki Hanharr page: Varnish 503 on repeated fetches (both URL forms), not saved. The Party page (`text/strategywiki_Party.txt:720-`) gives the same vanilla start stats as elkripper.
- `installed_mod_thematic_companions/details.txt:76-100` (Thematic Companions, live): STR/CON 20 -> 18, Repair 4 -> 0, Demolitions 6 -> 4 (live shows 6), Stealth 0 -> 3, Close Combat removed, Toughness added, Heavy -> Light armor proficiency. Live UTC confirms STR18/CON18/Stealth 3/Toughness/no Close Combat, so elkripper's Improved Close Combat and (later) Toughness picks are adapted.
- Steam "Guide to building all the meatbag party members": not fetched (elkripper judged sufficient).

## Adaptations vs elkripper
- Improved Close Combat dropped (needs Close Combat 139, removed by the mod). Toughness already known, so the Toughness line starts at Improved Toughness (123, min char 4).
- Wookiee Toughness/Fury/Frenzy, Targeting, Precise Shot, Uncanny Dodge, Evasion are granted automatically (not in the picks).
- Attribute bumps at 8/12/16/20/24/28 (absolute values for the validator) from live STR18 DEX13 CON18; elkripper uses STR (melee) or DEX (ranged) every time.

## Wizard answers -> preset (5 presets, all `alignment = "none"`, `convert = "n/a"`, `convert_at = 0`; check_paths: 0 errors, 0 warnings each)
| preset (`hanharr__...`) | role | style | skills | rationale |
|---|---|---|---|---|
| `melee_single_none_na` | melee | single | combat | two-hander: Improved/Master Flurry, Master Power Attack, Conditioning line, Regenerate, Improved/Master Toughness, Critical Strike chain, Mobility; STR 19-24 |
| `melee_dual_none_na` | melee | dual | combat | elkripper melee core (TWF/Improved/Master) plus Master Power Attack, Dual Strike line, Toughness/Conditioning lines; STR/DEX alternating |
| `tank_single_none_na` | tank | single | combat | Toughness + Conditioning lines first, Regenerate, Caution line, Master Power Attack, Flurry; CON/STR/DEX bumps (he cannot use heavy body armor, so durability comes from feats) |
| `ranged_pistols_none_na` | ranged | pistols | tech | elkripper pistol: TWF line, Improved/Master Rapid Shot, WF Pistol, Conditioning line, Critical Strike; DEX 14-19; Demolitions/Repair/Awareness |
| `ranged_rifle_none_na` | ranged | rifle | stealth | Improved/Master Rapid Shot, WF Rifle, Sniper Shot chain, Critical Strike chain, Conditioning; Stealth (cross-class) + Awareness, the shadowlands hunter flavor |

Skill points: round-robin over a priority list (combat = Awareness, Treat Injury, Demolitions; tech = Demolitions, Repair, Awareness; stealth = Stealth 1 rank + Awareness 1 rank per level), capped at char_level+3. Not covered on purpose: unarmed (Scout has no Martial Arts feats), caster/support (no Force), pistols+melee hybrids.

## Open questions
1. Is the "break Hanharr" conversation effect live (elkripper: STR +2, INT -2, so only 2 skill points/level afterwards)? Presets assume 3/level; INT 8 would make the skill proposals overspend by 1 point per level, and STR +2 shifts the melee bump targets.
2. Does the Wookiee Rage/Fury/Frenzy chain depend on class level and grant on schedule with the auto level-up (not covered by check_paths)?
3. Dual Strike (190-192) and Master Power Attack: confirm they behave as expected with the Thematic Companions rules; Master Flurry needs Improved Flurry + Flurry (validated).
4. Does the game allow Hanharr to equip body armor (light proficiency mod) - affects whether `tank` should favor DEX over CON?
5. StrategyWiki page missing (503) - retry later if a second source for Scout-specific feat order is wanted.
