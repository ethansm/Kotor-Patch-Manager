# Mandalore presets (tag `mand`, template `live_utc/p_mand.utc`)

Start state (live): Soldier 5, STR15 DEX12 CON14 INT12 WIS14 CHA12, 50 base HP, no Force powers. First level-up is char level 6. INT 12 gives only 2 skill points per level (Soldier base 1 + mod 1 per live skills table; check_paths confirms). Soldier feat slots at class levels 6-16 and every even level 18-30 = 18 picks; none at 17/19/.../29. Never converts (`convert_at = 0`, all levels class 0).

Live feats at level 5 (ids): armor prof heavy/light/medium (4,5,6), Improved Power Blast 18, Power Attack 28, Power Blast 29, Rapid Shot 30, Sniper Shot 31, WF Blaster Rifle 33, prof pistol/rifle/melee (39,40,44), Close Combat 139, Mandalorian Courage 200, Implant Switching 203. Live has NO Toughness (84) and NO Weapon Specialization: Blaster Rifle (47); Thematic Companions removed both (details.txt:153-175; CON 15->14, WIS 12->14, Demolitions 0->2, Treat Injury 9->5). Elkripper's table assumes the vanilla start, so its "Improved Toughness at 6" does not work here: presets pick Toughness first.

## Sources
- `raw/strategywiki/Mandalore.html` (archive.org copy, live site 503s) -> `text/strategywiki_Mandalore.txt`: dialogue only, no build or stat data.
- `raw/elkripper/kotor2-builds-mandalore.html` -> `text/elkripper_mandalore.txt`: only build available: "basic melee-focused" survivability path (Toughness/Conditioning/Dueling/Power Attack lines, WF/WS Melee at 18/20, Dual Strike line 22-26; CON +1 at 8/12/16, STR +1 at 20/24/28), with a note that a damage build would push STR first and take the Two-Weapon line instead of Dueling. Class skills Demolitions, Awareness, Treat Injury.
- `raw/web/mandalore_gamebanshee.html` -> `text/web_mandalore_gamebanshee.txt` (stats/feats list, vanilla).
- `installed_mod_thematic_companions/details.txt` (live changes above). Not fetched: Steam/GameFAQs guides (search hits only: Steam "Basic Companion Build Guide" 1234247381, GameFAQs board 45583736 "Mandalore's best battle style").
- Classic Class Attack Bonus (#10): Soldier attack bonus unchanged for this analysis (not checked by check_paths).

## Presets (all 0 errors, 0 warnings in check_paths; 18 feat slots all used, 2 skill points/level all spent)
| preset (`mand__...`) | role | style | alignment | convert | skills | rationale |
|---|---|---|---|---|---|---|
| `ranged_rifle_none_na` | ranged | rifle | none | n/a | combat | he starts with a rifle: WS Blaster Rifle (restores the removed feat), Improved/Master Rapid Shot, Improved/Master Sniper Shot, Master Power Blast, Precise Shot I-V, Toughness/Conditioning, Critical Strike line; Awareness + Treat Injury; DEX +1 at 8/12/16/20 then CON |
| `ranged_pistols_none_na` | ranged | pistols | none | n/a | tech | dual blasters: TWF/Improved/Master, WF/WS Blaster Pistol, Rapid Shot line, Precise Shot I-IV, Toughness/Conditioning; Demolitions (class skill, grenades/mines) + Treat Injury; DEX first |
| `melee_single_none_na` | melee | single | none | n/a | combat | vibroblade/Mandalorian blade: Dueling line, WF/WS Melee, Improved/Master Power Attack, Toughness/Conditioning, Mobility, Regenerate VP, Critical Strike; STR first (elkripper damage note), CON from 20 |
| `melee_dual_none_na` | melee | dual | none | n/a | combat | TWF line + Dual Strike line (elkripper takes Dual Strike), WF/WS Melee, Power Attack line, Toughness/Conditioning; STR first |
| `tank_single_none_na` | tank | single | none | n/a | combat | elkripper's survivability build adapted: Toughness line, Conditioning line, Regenerate VP, Dueling line, Power Attack line, WF/WS Melee; CON first 15/16/17, STR from 20; Treat Injury-heavy |

Not covered on purpose: caster/support (no Force powers; medic play is just Treat Injury and appears in `tank`), double-bladed/unarmed, any Jedi/convert/alignment variants (he cannot become a Jedi), stealth/social skill sets (cross-class for Soldier, cost 2).

## Open questions
1. Attribute auto-increases: elkripper shows CON +1 at 8/12/16 and STR +1 at 20/24/28 (vanilla CON15). Presets apply the cadence to live STR15/CON14 (ranged variants DEX +1 at 8/12/16/20, my proposal, no source). Unverified in game; only the `attrs` values matter to check_paths (feat prereqs have no attribute minimums here).
2. Mod 5 / Thematic Companions removed Toughness and WS Rifle: if the in-game level-up grants them back, drop 84/47 and shift feats.
3. Does the Mandalorian Heavy Repeater use the Blaster Rifle feat line (WF/WS/Sniper)? Assumed yes (rifle proficiency); confirm. Dual Strike line weapon requirement (any melee pair vs lightsaber only) unverified.
4. Attack-bonus prerequisites are not checked by check_paths; Classic Class Attack Bonus (#10) could affect minattack on masters (none of the used feats lists minattack > 0 in the rules tables).
5. Skills: Class Skill feats (Computer Use/Repair/Security/Stealth) unused; maxing Treat Injury/Awareness reaches 30 ranks by level 30. Persuade/Stealth left at live 2. Elkripper marks levels 28 and 30 feat slots "situational".
6. Elkripper models level 30 as end; TSLRCM level cap unverified.
