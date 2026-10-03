# B3 - companion preset build review (Phase 6b, read-only)

Scope: all 80 presets in `paths/presets/*.toml` plus the two legacy live paths `paths/atton.toml` and `paths/kreia.toml`
(82 specs). No preset, runtime table or game file was edited. Line numbers below are the `[[level]]` block start (`char_level = N`)
of the cited level; the `recommended = true` line is line 10 of each recommended preset.

Severity: **BUG** = wrong / no-op by the project's own rules sources; **QUALITY** = suboptimal but legal; **TASTE** = opinion.

## 0. Method and automated audit results

- `../../.venv/bin/python3 check_paths.py paths/*.toml paths/presets/*.toml`: 82/82 specs, **0 errors**, 22 warnings (all the accepted Bao-Dur
  Tech "1 point, only cost-2 skills left" cases from `paths/presets/WARNINGS.md`). `tools/parity/parity.py` was NOT re-run (not needed for a
  read-only data review; the table is unchanged since B1/B2).
- Extra scripted audit (scratchpad, not committed) over every record against live `spells.2da` / `feat.2da`:
  - **Dark (E) powers in light / neutral / none presets: 0.**
  - **Light (G) powers in dark presets: 13** (2 presets, see Kreia K-1/K-2 and Handmaiden H-6).
  - **Light (G) powers in `neutral` presets: 32** (3 presets: K-3, K-4, D-6). WIZARD_SCHEMA says neutral = "universal powers only".
  - Power picks below the spells.2da minimum level: **0**. Feat picks below `mincharlevel`: **0**. (B2's advisory about early Jedi picks below
    the 2da minimum does not occur in any shipped record.)
  - Feats the companion can never take (list column 4/3 for the class): **0** (check_paths already errors on these). Droid Upgrade Class 2/3
    are class-granted at class levels 7/13 (`drx_granted`/`drc_granted`), Logic Upgrade: Tactician/Battle Droid and Droid Interface are not
    selectable for droids, so "missing droid upgrades" is not a finding. Force Focus / Improved / Master are Consular-granted only (jcn
    levels 1/6/12); Guardian/Sentinel cannot select them, so "missing Force Focus" is not a finding either (Kreia/Disciple get it free).
- Rule facts used (all from the project's own text copies):
  - **Close Combat / Improved Close Combat only work with "Blaster pistol(s) or blaster rifle"** (`text/strategywiki_Feats.txt:557-575`).
  - **Finesse: Melee Weapons lets DEX replace STR for to-hit and covers lightsabers; Finesse: Lightsabers is not selectable by any class**
    (`strategywiki_Feats.txt:648-649`; feat.2da `193` list = 4 for every class, `194` = 1 for Soldier/Scout/Scoundrel/Jedi/Tech).
  - Companion Persuade / dialogue skills are not used by the engine: "Companions cannot assist with ship conversations, droid repairs, or
    player dialogue checks" and Persuade is "Player character exclusive" (`exile_playthrough_guide_snapshot.md:51-53`); the T3-M4 / Mira /
    Visas dialogue gates quote "you ... Computer Use rank 15" (`strategywiki_T3-M4.txt:215,372-378`) = the Exile's ranks.
  - Stealth works only for the party leader in Solo Mode with a camouflage device or Force Camouflage (`strategywiki_Skills.txt:732-739`).
  - Mod #142: opposite-alignment powers -50% .. 0, CHA modifier offsets 5%/pt, neutral = vanilla (`text/deadlystream_142_*.txt:82`);
    its "Treat Injury Affects Force Healing" half is a separate combined-install option (`:86,104`). Whether the combined install is live is
    **unverified** (every Heal finding below is conditional on it). Whose alignment #142 reads (companion vs Exile) is also unverified.
  - Attributes are engine-fixed per companion (`README.md:36`, `MASTER_PLAN.md:116`); `attrs` rows only drive validator arithmetic (INT for
    skill points). So "attribute allocation contradicts role" is reported once (section 13), not per preset.
  - Unspent skill points carry on the creature (`08_level_up_override.md:141,214`: server stores remaining points; held levels let points
    carry); the checker/table do NOT model carry-over. This matters for the B1 question.

## 1. Atton (Scoundrel 3 start; STR12 DEX16 CON12 INT10 WIS10 CHA15; Sniper Shot, WF Pistol, Critical Strike, Sneak Attack II)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| A-1 | BUG | `atton__melee_dual_light_early.toml:127,171` (L19 Close Combat, L25 Improved CC) **recommended** | Ranged-only feats in a dual-lightsaber build (2 of 14 feats wasted). | Replace with Regenerate Force Points, the Toughness line or the Conditioning line (Dueling is single-weapon only). |
| A-2 | BUG | `atton__melee_dual_dark_early.toml:126,170` (L19, L25) | Same Close Combat no-op in the dark dual-saber build. | Same replacement. |
| A-3 | QUALITY | `atton__melee_dual_light_early.toml` (whole), `atton__melee_dual_dark_early.toml` (whole) | DEX 16->23 vs STR 12 but **Finesse: Melee Weapons (194) never taken**; saber to-hit would use STR12 (+1) instead of DEX. Scoundrel and Sentinel can both select it. | Take 194 at the first Scoundrel/Sentinel slot (L5 or L8). |
| A-4 | QUALITY | `atton__ranged_pistols_light_late.toml:121` and `atton.toml:109` (Heal at L20, Improved Heal L23) | Heal line with **Treat Injury 0 ranks** all game (Scoundrel cross-class; Sentinel class skill only from L20) - Heal scales with TI under the combined #142 install. | Put Sentinel-phase points into TI (Class skill for Sentinel, 3 pts/level) instead of Stealth/Awareness/Security duplicates. |
| A-5 | QUALITY | `atton__melee_dual_dark_early.toml:46` (L8 Shock, L10 Wound, L11 Drain Life ... ) | Dark kit under #142 loses up to -50% unless Atton's alignment is dark; summary already says "needs dark Exile conversion dialogue". CHA 15 (+2) offsets only 10%. | Keep, flagged; confirm whose alignment #142 reads (Q-U9). |
| A-6 | TASTE | `atton__melee_dual_light_early.toml`, `atton__ranged_pistols_light_early.toml`, `atton__ranged_pistols_neutral_early.toml` | 25-slot Jedi kits (Heal/Valor/Barrier/Battle Meditation) with no Regenerate Force Points (143, Sentinel-selectable, mincharlevel 4). | Add 143 to the first Sentinel feat slot if the FP pool runs dry. |
| A-7 | TASTE | `atton__ranged_pistols_neutral_early.toml` (style pistols) | Rapid Shot/Precise Shot gunner with no TWF line although Atton keeps WF Blaster Pistol; reads as single-pistol. | Fine if `pistols` = single or dual; otherwise add TWF. |
| A-8 | TASTE | `atton__melee_dual_dark_early.toml`, `atton__ranged_pistols_light_late.toml`, `atton__ranged_rifle_none_never.toml` | Stealth ranks 26-29 on a companion (useless unless he is the solo leader). | Optional, see Q-U8. |

## 2. Bao-Dur (Tech Specialist 6; STR14 DEX10 CON14 INT15 WIS14 CHA10; heavy armor via mod #108)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| B-1 | BUG | `baodur__melee_unarmed_light_early.toml:26,59` (L8 Close Combat, L13 Improved CC) | **Unarmed** build spends 2 of its 9 feats on ranged-only feats; the unarmed-relevant Dueling line (works with "Unarmed", `strategywiki_Feats.txt:606-612`) is absent. | Replace with Dueling / Improved / Master Dueling. |
| B-2 | BUG | `baodur__melee_dual_light_early.toml:124,138` (L22 Close Combat, L24 Improved CC) | Dual-saber build, ranged-only feats. | Replace (Conditioning / Toughness line). |
| B-3 | QUALITY | `baodur__early_guardian.toml:155-184` (**recommended**, Persuade L26-30) | B1 spare-point sink. Persuade is companion-useless (guide 51-53). See Q-U1. | per Q-U1. |
| B-4 | QUALITY | `baodur__melee_dual_light_early.toml`, `baodur__melee_unarmed_light_early.toml`, `baodur__support_single_light_early.toml` (Persuade L24-30 = 7 ranks; Stealth L8-11) | Same B1 sinks, plus Stealth rank bought at cross-class cost 2 during Tech levels L8-11. | per Q-U1. |
| B-5 | QUALITY | `baodur__tank_single_none_never.toml:25-125` (Stealth 30 ranks at cost 2, L24-30 two ranks/level) | 30 Stealth ranks (60 points) on a heavy-armor tank; no tech skill is short (CU/Repair/Sec/Demo all 24). | Leave unspent (carry) or Class Skill feat; see Q-U1. |
| B-6 | TASTE | `baodur__melee_dual_light_early.toml` | DEX 10 dual-wielder (summary already calls it weakest to-hit). Intentional. | none |
| B-7 | TASTE | `baodur__melee_single_dark_early.toml:26-51` | Dark kit with CHA 10 (mod #142 penalty not offset); no Heal. Fine for a dark Exile. | none |
| B-8 | TASTE | `baodur__tech.toml:127,165` | Heal at Guardian 1 (L30) only; TI 28 ok. Cosmetic Guardian level, matches elkripper. | none |

Checked OK: dual-wield TWF/Improved/Master present (dual preset L8/11/13); Heavy Armor L8 everywhere; Weapon Specialization: Lightsaber is
Guardian-selectable; light presets contain no E powers; recommended `early_guardian` is a sound canon pick.

## 3. Disciple / Mical (Soldier 6; STR12 DEX14 CON14 INT15 WIS15 CHA8; has TWF, Improved TWF, Flurry, Power Attack/Blast, Close Combat, Superior WF Lightsaber II)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| D-1 | BUG | `disciple__melee.toml:179` (L30 Improved Close Combat) | Ranged-only feat in the dual-saber build (his Close Combat is a live leftover). | Use the slot on Improved Conditioning/Toughness or Master Flurry. |
| D-2 | QUALITY | `disciple__caster.toml:37` (L9 Master Two-Weapon Fighting) **recommended** | Caster/healer Consular spending a Consular feat slot on Master TWF (and WF Lightsaber at L30); he never dual-wields in this build. | Swap for Toughness/Conditioning chain or Regenerate VP. |
| D-3 | QUALITY | `disciple__support.toml:46,138` (L11 Master TWF; L24-30 Dual Strike line), wizard style = `single` | Style label `single` but 4 of 9 feats are dual-wield/team-attack feats. | Retarget feats to style (Dueling line) or relabel. |
| D-4 | QUALITY | `disciple__support_none_light_early_tech.toml:21` (Heal L7, Improved Heal L12) | Heal line with Treat Injury 3 ranks for the whole game (all points go to Repair/Computer Use/Security; CU/Security are cross-class 2:1). | Put TI first (Class skill for Consular), keep Repair as the second skill. |
| D-5 | QUALITY | `disciple__caster_none_dark_early.toml` (Persuade L23-30, B1 sink) | Companion-useless sink; see Q-U1 logic. | Leave unspent or TI. |
| D-6 | QUALITY | `disciple__caster_none_neutral_early_stealth.toml:159-188` (L26-30: Stun, Stun Droid, Stasis, Stasis Field, Disable/Destroy Droid, Force Aura) | 7 light-side powers filling a `neutral` ("universal only") preset. | Accept as filler (Q-U6) or leave powers for the player. |
| D-7 | TASTE | `disciple__ranged_rifle_none_never.toml` | Superior Weapon Focus: Lightsaber II (live) unused; Precise Shot III only at L26, IV/V never. | none |
| D-8 | TASTE | `disciple__melee.toml` | Wizard answers on `disciple__caster` = role support and `disciple__support` = role tank (names and answers differ). Harmless; confuses file-based review. | Rename for clarity. |

Checked OK: Finesse: Melee (194) at L11 in `disciple__melee` is correct for DEX 14 (live utc lacks it despite mod #8 text; verify in game as the
preset doc already says); Class skills correct; no E powers in light presets.

## 4. G0-T0 (Expert Droid 6; DEX18 INT16; TWF, Close Combat, Gear Head line, Cloaking Shield)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| G-1 | QUALITY | `g0t0__support_pistols_none_na.toml:10` (**recommended**), feats at `:34` (Master Gear Head L9) and `:71` (Improved TWF only L15, Master TWF L18, WF Pistol L21) | The canon pick spends his first slots on skill-bonus feats and delays his only damage feats to L15-21, with Awareness 4. His skills are only usable when he is the active leader. | Recommend `g0t0__ranged_pistols_none_na` instead (TWF line L9/12, WF L15). |
| G-2 | TASTE | `g0t0__ranged_pistols_none_na_stealth.toml` | Stealth 26 at cross-class cost 2 for a companion; works only solo with his shield. | Optional. |
| G-3 | TASTE | all 5 | Dual Strike (taken in 3 presets) needs an ally on the same target; unverified for droids (own doc Q3). | Verify in game. |

Checked OK: no droid upgrades/Logic feats missing (granted or unselectable); only 8 feat slots, all used legally; no Force.

## 5. Handmaiden (Soldier 6; STR14 DEX16 CON14 INT10 -> 1 skill point/level; TWF, Improved TWF, Echani Strike I, Unarmed Specialist I/II, Toughness+Improved)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| H-1 | BUG | `handmaiden__melee_single_dark_early.toml:166,181` (L27 Close Combat, L29 Improved CC) | Ranged-only feats in a single-saber build. | Replace (Critical Strike line / Toughness line / Dual Strike). |
| H-2 | BUG | `handmaiden__saber_guardian.toml:166` (L27 Close Combat) | Same. | Replace. |
| H-3 | BUG | `handmaiden__support_single_light_early.toml:181` (L29 Close Combat) | Same. | Replace. |
| H-4 | BUG | `handmaiden__tank_single_light_early.toml:166,181` (L27, L29) | Same (two feats). | Replace. |
| H-5 | BUG | `handmaiden__unarmed_soldier.toml:139,151` (L26 Close Combat, L28 Improved CC) | Unarmed Soldier: ranged-only feats; as a Soldier she also never gets Unarmed Specialist III+ (granted only at Jedi class levels, `handmaiden.md` conversion facts). | Replace with Dueling-free filler (Mobility, Toughness line); document the US gap. |
| H-6 | QUALITY | `handmaiden__melee_single_dark_early.toml:188` (L30 Force Barrier [G]) | One light power in the dark preset (CHA 13 = +1 offsets only 5%). | Swap for a universal power (Force Breach is already taken: Force Immunity / Battle Meditation). |
| H-7 | QUALITY | `handmaiden__unarmed_jedi.toml:22` (**recommended**), `melee_dual_light_early.toml:21`, `melee_unarmed_light_late`, `saber_guardian.toml`, `support_single_light_early.toml:21`, `tank_single_light_early.toml` | Heal at L7-13 and Improved/Master Heal later with **Treat Injury 2 start + 12 ranks (final 14)**; Awareness is bought first (L7-18) and TI only from L19 (support: L11). Only 1 skill point/level, so the order matters. | Put the single point into TI first in every Heal preset (Guardian class skill). |
| H-8 | QUALITY | `handmaiden__support_single_light_early.toml` (Persuade L21-30, wizard skills `social`) | Persuade 10 ranks for a companion; support role would rather have TI. | Recast as `combat`. |
| H-9 | TASTE | `handmaiden__unarmed_jedi.toml:22` | Valor line only at L27-29 (elkripper ordering). | Move earlier if she is the only buffer. |

Checked OK: Dueling applies to unarmed; Unarmed Specialist ranks arrive automatically at Guardian levels 2/6/10/14... (presets do not pick them);
STR rises 14->18 so no Finesse needed (unlike Atton/Mira).

## 6. Hanharr (Scout 4; STR18 DEX13 CON18; Flurry, Improved Power Attack, Power Attack, Rapid Shot, WF Melee, Wookiee Toughness/Rage)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| N-1 | TASTE | `hanharr__melee_single_none_na.toml:10` (**recommended**; Critical Strike line only L23-27) | Fine pick; Critical Strike (selectable) arrives after the Toughness chain. | none |
| N-2 | TASTE | `hanharr__ranged_*` (pistols/rifle) | Wookiee with WF Melee already known and Rapid Shot; ranged presets are against his identity but legal (all feats selectable, DEX rises). | none |
| N-3 | TASTE | `hanharr.md` open Q1 | "Break Hanharr" (INT -2) would cut skill points to 2/level and overspend every preset by 1 point. | Verify in game. |

No BUG/QUALITY findings for Hanharr; no Close Combat, no Force.

## 7. HK-47 (Combat Droid 6; has Sniper + Improved Sniper + WF Rifle + Toughness)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| HK-1 | TASTE | `hk47__ranged_rifle_none_na.toml:10,84,159` (**recommended**; Precise Shot I at L17, V at L27, Master Sniper Shot only L29) | Rifle sniper whose Master Sniper Shot (core of his kit, prereq already met) is the second-to-last feat; Precise Shot (mincharlevel 4/8/12/16/20) could come earlier. Follows elkripper. | Consider Master Sniper Shot at L12 instead of Weapon Specialization. |
| HK-2 | TASTE | `hk47__tank_rifle_none_na.toml` (Close Combat L12 / Improved L15) | Legal for a rifle droid (ranged weapon), but ranks above Rapid Shot. | none |
| HK-3 | TASTE | all | Awareness-heavy skills; Security >= 10 for the HK-50 factory is met (3 + 17). | none |

No BUG/QUALITY findings.

## 8. Kreia (Jedi Consular 3; STR10 DEX12 CON14 INT14 WIS16 CHA12; Force Focus, Dueling, Finesse: Melee, **Class Skill: Stealth**, Force Camouflage; no TWF)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| K-1 | QUALITY | `kreia__support_none_dark.toml:20,93-136` (wizard dark; 12 light powers L4-L20: Valor/Aura/Heal/Shield/Barrier/Armor/Master Heal/Master Valor ...) | A "dark" preset whose first 20 levels are light-side buffs: under #142 a dark Kreia halves exactly these (summary admits "hybrid"). | Pure-dark support (Drain/Horror/Slow + universal) or relabel as neutral hybrid (Q-U7). |
| K-2 | QUALITY | `kreia.toml:9-133` (live legacy path, identical powers to K-1) | Same hybrid, and this is the file actually loaded for the live `kreia` tag. | Decide with K-1. |
| K-3 | QUALITY | `kreia__support_none_neutral.toml:10,152-210` (**recommended**; 12 G powers L22-30; Heal first at L23, Force Valor L22) | The canon pick delays Heal/Aura/Barrier/Valor (Force Chain shares them with the Exile, `elkripper_kreia.txt:7-14`) to L22+ and uses 28 Persuade ranks (companion-useless). `kreia.md` says `support_none_light` is the default recommendation. | Recommend `kreia__support_none_light` (Exile is light side per the guide snapshot). |
| K-4 | QUALITY | `kreia__melee_single_neutral.toml:144-209` (L21-30: 13 G powers) | `neutral` melee preset ends in light buffs; attack bonus 0.5/level makes it weak anyway (`kreia.md`). | Keep as filler or drop preset. |
| K-5 | QUALITY | `kreia__caster_none_light.toml:35,129` (Heal L6, Improved Heal L19) | Heal line with TI frozen at 6 (points go to Repair/Computer Use cross-class 2:1). | Spend on TI. |
| K-6 | QUALITY | `kreia__support_none_light.toml:20-64`, `support_none_dark`, `tank_single_light`, `kreia.toml` | Repair 33 ranks (2 points/level L4-9, 1/level after) on builds with **no droid power**; the 4th point per level is the B1 spare. See Q-U2. | per Q-U2. |
| K-7 | TASTE | `kreia__caster_none_light.toml` | 3 slots on Stun/Disable/Destroy Droid; "Repair Affects Stun Droid" is only referenced in #142's compat text, not in `installed_mods_progression_relevant.json`. | Verify mod installed. |
| K-8 | TASTE | `kreia__support_none_dark.toml:136-144` (Improved/Master Dueling), `kreia.toml:133` | Dueling line for a staff-caster; Conditioning/Toughness are better filler. | none |

Checked OK: Force Focus line is granted (no feat needed); Regenerate Force Points is the first feat in all casters/supports; Kreia cannot take
TWF and no preset takes it; Finesse: Melee already owned; no E powers in light presets.

## 9. Mandalore (Soldier 5; STR15 DEX12 CON14 INT12 WIS14; WF Blaster Rifle, Power Blast/Improved, Close Combat, Rapid/Sniper)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| M-1 | BUG | `mand__tank_single_none_na.toml:170` (L30 Improved Close Combat) | Ranged-only feat in a melee tank. | Replace (Master Critical Strike / Regenerate VP). |
| M-2 | TASTE | `mand__melee_*` | Melee presets discard his start (WF Rifle, Rapid/Sniper, Power Blast chain). Legal; the rifle preset is the natural recommended pick and **is** the recommended one (`mand__ranged_rifle_none_na.toml:10`, WS Rifle L6 restores the thematic-removed feat). | none |

## 10. Mira (Scout 6; STR14 DEX16 CON13 INT12 WIS12; Dueling, Critical Strike, Close Combat, Rapid/Sniper, Precise Shot I; no TWF)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| R-1 | BUG | `mira__early_sentinel.toml:152` (L25 Improved Close Combat) | Ranged-only feat in a dual-saber build. | Replace. |
| R-2 | QUALITY | `mira__early_sentinel.toml:21`, `mira__melee_dual_dark_early.toml:21` | Dual sabers with DEX 16->20 vs STR 14 and no Finesse: Melee (194, Sentinel-selectable). | Take 194 at L7 or L12. |
| R-3 | QUALITY | `mira__ranged_rifle_light_late.toml:59,95` (Heal L13, Improved Heal L17) | Heal with TI **11 total** (Stealth 31 + Security 31, Awareness only 19). | Re-point Sentinel points to TI/Awareness. |
| R-4 | QUALITY | `mira__support_single_light_early.toml:21` (Persuade 33 ranks, L7-9 all 4 points; TI starts L10) | Support Sentinel whose first Heal comes at L7 with TI 5 and a social (Persuade) focus that no engine path uses. | Recast as `combat`: TI first. |
| R-5 | QUALITY | `mira__melee_dual_dark_early.toml:21` | TI 28 / Computer Use 27 with no Heal power in the dark preset. | Reassign to Awareness/Security. |
| R-6 | TASTE | `mira__ranged_sentinel.toml:10,85,121` (**recommended**, convert_at 17, PS V at L22) | Clean (Scout grants PS II-IV, V picked by hand), but it assumes late conversion for the canon pick; B2 proved re-base if she converts earlier. | none |

## 11. T3-M4 (Expert Droid 3; DEX15 INT16; Caution, Gear Head, Blaster Integration)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| T-1 | QUALITY | `t3m4.md` ("skill plans reach Computer Use 15 / Repair 21 gates") and all six `t3m4__*.toml` skills lines | The memory-core/maintenance dialogues check **the Exile's** ranks (`strategywiki_T3-M4.txt:215,372-378,537-681`); T3-M4's own CU/Repair are used only if he is the active leader. The plan's stated goal is mis-aimed. | Treat his skills as free choice; reword doc. |
| T-2 | QUALITY | `t3m4__support_single_none_na.toml:10,34,146` (**recommended**; Improved/Master Gear Head L6/L9, WF Pistol only at L24, no TWF) | Weakest-damage variant is the canon pick: skill-bonus feats first, one weapon feat in 9 slots. | Recommend `t3m4__ranged_pistols_none_na` or `tank_pistols_none_na` (elkripper build). |

Checked OK: no impossible feats; slots (9) all used; TWF/Dual Strike chains legal.

## 12. Visas Marr (Jedi Sentinel 6; STR12 DEX16 CHA16; WF Lightsaber, Flurry, Improved Flurry; no TWF; no Force Sensitive)

| ID | Sev | Where | Finding | Suggested fix |
|---|---|---|---|---|
| V-1 | QUALITY | `visas__melee_dual_light.toml:20` | Dual-saber DEX 16 vs STR 12, **no Finesse: Melee**. | Add 194 at L7 or L12 (the other three melee presets take it). |
| V-2 | QUALITY | `visas__caster_none_light.toml:20` (Persuade 33 ranks, 3/level L7-10), `visas__tank_single_light.toml:20` (Persuade 30), `visas__force_support.toml` (Persuade 24) | Heavy Persuade focus on a companion; TI ends at 29 (caster) / 30 (tank). | Switch to TI/Awareness. |
| V-3 | TASTE | `visas__duelist.toml:10,56`, `visas__melee_single_neutral.toml:104`, `visas__melee_double_neutral.toml` | Finesse taken only at L12 (after Dueling/Advanced Jedi Defense); Dueling line last in the single-neutral build. | Move Finesse to L7. |
| V-4 | TASTE | `visas__dark.toml` (Dueling L27/L30; computer_use 24, stealth 24) | Caster dark build with melee-defence filler and Computer Use. | none |

Checked OK: the dark preset is flagged and uses only E/universal; light presets contain no E powers; Regenerate FP first for FP-starved
casters; Visas' known E powers (Fear/Horror/Shock/Wound/Slow) are untouchable.

## 13. Cross-cutting findings

| ID | Sev | Where | Finding |
|---|---|---|---|
| X-1 | QUALITY | `paths/presets/WIZARD_SCHEMA.md:13` | The `social` (Persuade) and `stealth` skill answers have no gameplay value for a companion (engine/guide). Presets built on them: Kreia neutral (rec), Mira support, Visas caster/tank/support, Handmaiden support. |
| X-2 | QUALITY | all Heal presets | Heal-line builds should front-load TI if the combined #142 "Treat Injury Affects Force Healing" is installed; unverified (see Section 0). Affected files listed in A-4, D-4, H-7, K-5, R-3, R-4. |
| X-3 | TASTE | all `attrs` rows | Attribute gains are engine-fixed; Bao-Dur's presets put every gain in INT (STR14 DEX10 for a melee Guardian), Atton/Mira DEX-only, Kreia WIS-only. Only INT matters for skill-point arithmetic; if the real gains differ, per-level skill points shift. |
| X-4 | TASTE | `recommended` list | 12 picks, one per companion. Sound: Baodur early_guardian, Hanharr melee_single, HK-47 rifle, Mandalore rifle, Visas duelist, Mira ranged_sentinel, Handmaiden unarmed_jedi. Weak/odd: Kreia neutral (K-3), G0-T0 support (G-1), T3-M4 support (T-2), Atton melee_dual_light (A-1, A-3), Disciple caster (D-2). |

## 14. Counts (rows in sections 1-13)

Rows by severity: BUG 12, QUALITY 31, TASTE 25 (one row can cover several files). BUG = 12 rows (19 wasted feat picks: Close Combat / Improved Close Combat in melee/unarmed/saber
presets of Atton x2, Bao-Dur x2, Disciple, Handmaiden x5, Mandalore, Mira).

## 15. Questions for the user

Each is a decision; the recommendation is my reading, not a default.

### Q-U1 Bao-Dur: where should the spare skill point go? (B1 choice)
Context: after conversion every Guardian preset has Awareness, Demolitions and Treat Injury at the rank cap (level+3); Persuade is the only
other **cost-1** skill (Guardian class skills: Demolitions, Awareness, Persuade, Treat Injury). 5-7 spare points per preset (L24-30), 1 in
`late_guardian`, plus Stealth at cross-class cost in Tech levels L8-11 and 30 Stealth ranks in `tank_single_none_never`
(`baodur__early_guardian.toml:155-184`, `baodur__melee_dual_light_early.toml:138+`, `baodur__tank_single_none_never.toml:25-125`).
Persuade is Exile-only, so none of these options changes gameplay; the real differences are cost, flexibility and checker noise.
- A. Keep Persuade (current). Pros: zero warnings, no feat slot, harmless. Cons: purely cosmetic.
- B. Leave the point unspent (revert B1 for these levels). Pros: honest; unspent points carry on the creature (doc 08:141,214), so a later
  manual level-up could buy a 2-point cross-class rank (Repair/Computer Use/Security, his signature skills). Cons: ~80 checker warnings return
  (accepted-list entry needed); presets that auto-level every record would not spend them.
- C. Spend one Guardian feat slot on **Class Skill: Repair (186)** (Guardian-selectable) so the spare point buys Repair at cost 1. Pros: real
  (if small) use of the points when he is the leader. Cons: costs a feat slot (Dueling/Conditioning level).
- D. Model carry-over in the checker/table and bank two points for a cross-class rank. Pros: best skill use. Cons: engine/table change, out of B scope.
Recommendation: **A** for 1.0.0 (no risk), noting C if the user ever plays him as leader. The cross-class Stealth ranks (Tech levels, tank preset) are worse than
leaving the points unspent: recommend moving them to B or to Repair/Security. Question: "Keep Persuade as the harmless Bao-Dur sink, or leave the spare point unspent?"

### Q-U2 Kreia: Stealth for levels 10-30 (B1 choice)
Context: her 4 points/level = Awareness + Treat Injury + Repair (all +1/level, tracking the cap) + 1 spare; the spare goes to Stealth because her
utc carries **Class Skill: Stealth** (the one extra feat Thematic Companions deliberately left her, tied to Force Camouflage, `details.txt` Kreia
section). Neutral preset also puts +1 Stealth from L4 (26 ranks) and +Awareness L4-5. Repair already reaches 33 on builds with no droid power.
- A. Stealth (current). Pros: only option with an in-game use: Stealth works only for the leader in Solo Mode with a device or Force Camouflage,
  and Kreia owns that power; matches the mod's thematic rationale; cost 1. Cons: useful only if she is ever the solo leader.
- B. Persuade (Consular class skill, cost 1, start 5). Pros: fits the existing `social` answer. Cons: no engine use at all.
- C. Treat Injury ahead of the cap is impossible (already at cap); alternative is unspent points (carry; see Q-U1 B).
- D. Drop Repair to 0 on the builds without droid powers (support_light/dark, tank) and put those points into Persuade/Stealth/TI instead.
Recommendation: **A**, and apply D to the three builds without droid powers (K-6) only if the user wants Repair for something specific.
Question: "Keep Kreia's spare points in Stealth (usable via Force Camouflage), or move them to Persuade or leave them unspent?"

### Q-U3 Close Combat in melee presets (12 presets, 19 feats)
- A. Replace each with the best remaining style feat (Dueling/Toughness/Conditioning/Critical chains as listed in the tables). Pros: removes dead feats. Cons: edits 12 presets, regenerate table + gates.
- B. Leave (harmless). Cons: wasted slots in shipped data.
Recommendation: **A**, as one batch. (All are late-game slots except Bao-Dur unarmed L8/L13 and Atton L19.)

### Q-U4 Finesse: Melee Weapons on DEX saber builds (Atton x2, Mira x2, Visas dual; Visas others late)
- A. Add 194 at the first available slot in those five presets. Pros: DEX 20-23 actually helps to-hit. Cons: displaces one feat (e.g. a Dual Strike rank).
- B. Leave. Cons: attack uses STR 12-14.
Recommendation: **A** (the guide snapshot also lists Finesse as a Level-3 priority for the DEX Exile).

### Q-U5 Heal builds and Treat Injury
Is "Treat Injury Affects Force Healing" (the combined #142 install) actually installed? If yes: re-order TI first in the Handmaiden presets (6), Kreia caster_light,
Disciple support_tech, Mira rifle_light_late, Atton light_late + live `atton.toml`. If no: ignore X-2/H-7. (I did not touch the game dir to check.)

### Q-U6 Neutral presets filled with light powers (Kreia support_neutral rec, Kreia melee_neutral, Disciple neutral_stealth: 32 powers)
WIZARD_SCHEMA says neutral = universal only; the universal pool is about 25 powers, 36 are needed. Under #142 a neutral character has no penalty on either side.
- A. Keep light filler (current; documented in `kreia.md` Q1). B. Relabel these as "gray" (universal first, then light). C. Leave leftover power slots unspent (the Powers panel forces picks when picked manually, so not recommended).
Recommendation: **B** (label only) and change Kreia's recommended pick (next question).

### Q-U7 Kreia's recommended preset and the dark hybrid
- Recommended = `support_none_neutral` today. Alternative: `support_none_light` (Heal L6, Aura/Barrier/Valor early, Force Chain gives those to the Exile; Exile is light side).
- `kreia__support_none_dark` and the live `kreia.toml` are light-buffs-then-dark hybrids (12 light powers). Options: replace by a pure dark support, relabel as hybrid/neutral, or keep.
Recommendation: recommended -> `support_none_light`; relabel the hybrid.

### Q-U8 Persuade / Stealth wizard answers for companions
`skills = social` (Persuade) and `stealth` do nothing for a non-leader. Keep as flavour, rename (`social` -> `utility`), or remove the answers from the wizard schema?
Recommendation: rename and map to TI/Awareness; keep `stealth` only for Kreia/Visas/G0-T0 (Camouflage/Cloaking Shield).

### Q-U9 Mod #142 alignment source
Unverified whose alignment #142 reads (companion's own vs the Exile's). It decides whether dark presets (Atton, Bao-Dur, Handmaiden, Mira, Disciple, Visas, Kreia caster) are sane for a light Exile, and whether light presets are sane for a dark Exile. A single in-game test (cast Heal/Choke with a known-alignment companion) answers it.

### Q-U10 Recommended-list changes (feeds B4)
Replace G0-T0 `support_pistols` -> `ranged_pistols`; T3-M4 `support_single` -> `ranged_pistols`/`tank_pistols`; Kreia `neutral` -> `light`; consider Disciple caster after Master TWF is removed; fix Atton's pick before shipping (Close Combat + Finesse). OK to change these in B4?
