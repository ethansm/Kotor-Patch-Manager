# 51 — Combat Stats page (hidden stats made visible)

Status: RESEARCH DONE + DESIGN set (2026-10-27): new Stats tab in the character-effects panel + vanilla page tooltips. Next: ST1.

## Why
The vanilla character sheet shows one Defense number, save totals, and skill totals as base + one lump bonus. It never shows
blaster deflection, crit range, real Force point cost, Force power DC, influence or exact alignment. Example that started this:
Advanced Jedi Defense (+3) plus Force Redirection (+3) in one level-up gives +6 deflection, and nothing in game shows it.

## How this was researched
Haiku swarm, $3.08 for 15 agents (agent_usage.py, 2026-10-27). Plan and budget: `.claude/research/combat_stats/WAVES.md`.
- W1: 10 recon units (5 categories x 2 lanes) -> `.claude/research/combat_stats/w1/*.md`.
- W2: 5 skeptical verifiers -> `.claude/research/combat_stats/w2/C1..C5.tsv` (~150 rows: ~95 VERIFIED, 7 WRONG and corrected, rest UNVERIFIED).
- W3 (Opus): resolved opens with field_census and the 2DA exports (below). All addresses are Steam.
- Budget lesson: peak ctx 112k-200k per Haiku agent against a ~100k target. Reading `.asm` and big `.c` files blows past a
  token cap given in the brief; next time cap by tool calls (<= 12) and name the line ranges in the brief.

## Verified stat catalog (what a patch can show)
Struct shorthand: `stats` = CSWSCreatureStats (= `[creature+0x1198]`), `rec` = attack record (`005908c0`: round + 4 + idx*0x17c, unverified).

### C1 Defense
| stat | formula (verified) | where | read path for a patch |
|---|---|---|---|
| Blaster deflection | d20 + BAB + (MJD feat 24 ? 6 : AJD feat 1 ? 3) + 3 if Force Redirection (spell 163) + WM Deflect (feat 168, (lvl+1)/2, Weapon Master class 0x0b, saber) + form term + effect deltas (EffectBlasterDeflection*, item prop) - attacker Precise Shot (I-V: -2..-10, highest only) | `ResolveDefensiveEffects 0x006e1c60` | no pre-roll getter; recompute from feats/spells/form/effects, or read `rec+0x168` (total) and `rec+0xc1/0xc2` (threshold) after a roll |
| Deflection outcome | 8 = deflected if total >= threshold; 9 = reflected if total >= threshold+6 and reflect allowed | same | `rec+0x5c` |
| Form deflection term | Makashi -5, Soresu +4, Ataru -4, Shien +2, Niman +1 (saber only) | `0x006e1c60` switch on `creature+0x1110` | matches all 5 descriptions |
| Defense parts | 10 + armor (`stats+0xfa`) + shield (`+0xfb`) + natural + class/ArmorMult + DEX mod + dodge (min 10) + effect AC by type + Total Defense (+4, or +6 for Scoundrel/Sentinel/Watchman/Assassin lvl > 3) + form term; -4 when flat-footed-like gate | `GetArmorClassVersus 0x006a5110` | call it, or read the breakdown record bytes `+0x105..+0x10b` it writes |
| Damage reduction / resistance | DR: amount, damage power, limit; resistance: per-type bitmask, per-hit, limit pool | effects `0x00673e10`, `0x00674020`; applied in `OnApplyDamage 0x005959e0` | walk the creature effect list (`creature+0x148`, count `+0x14c`) |

### C2 Offense
| stat | formula (verified) | where | read path |
|---|---|---|---|
| Crit threat range | min roll = 21 - threat; threat = w + (Keen ? w : 0) + m*w; w = baseitems crit-threat byte (`+0x1e`); m by active special attack (Critical Strike tiers 8/31 -> 1, 19/20 -> 2, 77/81 -> 3) | `GetCriticalHitRoll 0x006afa60`; Keen = item prop 0x1C (itempropdef row 28) | call per hand (arg 0 main, 1 off); cached threat count in `stats+0x11c` block `+0x0A`/`+0x10` |
| Crit multiplier | weapon `+0x1f` (2 bare) + 1 for Power/Sniper/Flurry-type specials {17,18,28,29,82,83} + 1 with saber in Shien | `0x006afcd0` | call per hand |
| Attacks per round | 1 + min(bonus attacks `round+0xaf4`, 2) + 1 for flurry/rapid-shot anims + 1 | `0x005905f0` writes `round+0xae0` | `GetAttacksPerRound 0x006a7170` is a stub returning 1 — do not use it |
| Attack bonus | BAB + ~13-15 parts (weapon, STR/DEX, feats, two-weapon penalties, form, effects) | `GetAttackModifierVersus 0x006a93a0` (`.c:916`) | call it; per-part values partly UNVERIFIED |

### C3 Saber forms
Active form: `creature+0x1110` = spells.2da row (0x102 Shii-Cho .. 0x108 Juyo; 0x109-0x10c Force forms). Code matches every
description value checked for Defense, attack, Makashi damage and deflection (W2 C3.tsv + W3).
Total Defense stance flag `creature+0x110c`: 0 = Total Defense (written by `AIActionStanceTotalDefense 0x005d29f0`),
1 = Meditative (`0x005d2ea0`), 0xFF = none.

### C4 Force
| stat | formula (verified) | where |
|---|---|---|
| Force power save DC | level + 5 + CHA mod (`stats+0xf8`) + WIS mod (`+0xf6`) + Force Focus (feat 88/89/90: +2/+3/+4) + 5 for spells 159-161 (Force Scream line) | `CalculateSpellSaveDC 0x0058a1c0` |
| Real FP cost | trunc(base `row+0x15a` x forceadjust[GoodEvil/10].goodcost or evilcost), +/- room rating /20, min 1, x1.2 in modes 0x10a/0x10c, then Force Body-type effect x50/40/30% | `0x005482a0`, `0x006c4120` |
| forceadjust.2da | row 0 (GoodEvil 0-9, dark) light x1.5 / dark x0.5 ... row 5 x1/x1 ... row 10 (pure light) light x0.5 / dark x1.5 | game data (`.claude/waves/combat_stats/data/forceadjust.tsv`) |
| Saves | Fort = `+0xf2` + `+0x1b0` + base + effects; Reflex = `+0x1b2` + DEX + base + effects; Will = `+0x1b1` + WIS + base + effects | `0x006ba620/0x006ba740/0x006ba6b0` |
| Force resistance | max(0, `stats+0x12b` - `stats+0x12c`) | `0x006bad10` |
| FP regen | 2DA float cells (FPRegenTime, In/OutOfCombatFPBase, WisModBonus) x mode multipliers; alignment bands at GoodEvil < 41 / < 60 | `0x0056a800` (partly UNVERIFIED) |

### C5 Characters / world
| stat | formula (verified) | where |
|---|---|---|
| Companion influence | int32 at party table `+0x11c + i*4`, i = 0..11 (atton 0, baodur 1, handmaiden 4, kreia 6, visasmarr 9, hanharr 10, disciple 11); 50 neutral, -1 unset; scripts clamp 0..100 (the setter itself does not) | `GetInfluence 0x005fa9c0`, `ExecuteCommandInfluence 0x00687230` |
| Exact alignment | int16 GoodEvil at `stats+0x18a`, clamped 0..100 by every writer | `ModifyAlignment 0x006ad400` |
| Skill total parts | base + effects + key ability + feat tiers (e.g. 120/119/12 = +3/+2/+1) + Force Camouflage (+4/+8 Stealth) | `GetSkillRank 0x006b7bb0` |
| Computer spike cost | DC - floor(Computer Use / 4), min 1 | `k_inc_utility.nss:1602` (script, not engine) |
| Lock DC | raw byte on the door/placeable; success iff DC <= d20 + Security total | `0x005db8f0:613` |

## Findings worth noting
1. **Shien/Niman description overstates deflection.** Both say "improvements to Attack Modifier also aid in Blaster Bolt
   Deflection" (net +4 / +2). The deflection roll adds only base attack bonus (`0x006a6f80` at asm `006e21e9`), not the form's
   attack modifier, so the real form bonus is +2 / +1. Confidence medium-high (W2 VERIFIED BAB term; no other attack term seen).
2. **Favored Enemy damage term is dead** (`0x006bada0` always returns 0). Not a bug: K2 has no Favored Enemy feat (NWN leftover).
3. **Weapon Master Deflect is counted once.** W1 claimed it was never summed (wrong, it is summed at `006e1ef9`). The second
   ungated term reads `stats+0x178/+0x179`, which only the constructor writes (0), so no double count (field_census, W3).
4. `GetAttacksPerRound 0x006a7170` is a stub (returns 1); the real count is computed in `0x005905f0`.
5. **Creature -> influence slot (ST5b, local dumps, $0).** HandleAlignmentInfluence `0x0056ae40` (asm `0x0056b470..0x0056b4a6`):
   `MOV EAX,[0xA1B4A4]; MOV ECX,[EAX+8]; CALL 0x0051c8d0` -> party table = `*(*(*(0xA1B4A4)+8)+4) + 0x1F0B4`; slot =
   GetNPCID `0x005fdbf0(table, *(creature+4))` = first i in 0..11 with `*(table+0x1C+i*4)` == object id (0x7F000000 skipped),
   else a stricmp chain on the tag (creature+0x18 CExoString): 0 atton, 1 baodur, 2 mand, 3 g0t0, 4 handmaiden, 5 hk47,
   6 kreia, 7 mira, 8 t3m4, 9 visasmarr, 10 hanharr, 11 disciple (strings `0x0099515c/54/40/38/30` read from the exe).
   Influence = `*(int*)(table+0x11C+i*4)`, -1 = unset. Kreia's alignment copies BaseCNPCAlignment instead (C5 KREIA_GUARD).
6. **Force Mastery (ST6b, local dumps).** `FUN_0058b7f0(creature)` (called from ModifyAlignment `0x006ad400`, HandleAlignmentInfluence
   `0x0056ae40`, `0x006bdcd0`): GoodEvil (stats+0x18a) == 100 -> remove dark, apply Light marker effect (type = int at `0x009960c8`,
   `0x0058b880`); == 0 -> remove light, apply Dark marker (`0x009960cc`, `0x0058b990`); else remove both. SetStats `0x0084e6f0`
   shows `LBL_FORCEMASTERY` only while a marker is present (`0x0058bbe0` = has effect type), text by the LAST class slot
   (client +0xf8+i*0x20, i = count-1): Guardian line 3/11/14 -> 114161 "Light Mastery: +3 STR" / 114164 "Dark Mastery: +1-8 Dam";
   Consular line 4/12/15 -> 114163 "+3 WIS" / 114166 "+50 FP"; Sentinel line 5/13/16 -> 114162 "+3 CON" / 114165 "+3 DEX".
   Where the bonus itself is applied was not traced (the strings state it).
7. **Saving throws (ST6b research; shown by ST6c 0.5.2-dev).** Fort `0x006ba620` / Reflex `0x006ba740` / Will `0x006ba6b0`, thiscall(stats, int
   noEffects) RET 4: total = misc byte (stats+0x1b0 Fort, +0x1b2 Reflex, +0x1b1 Will) + ability (CON +0xf2 / GetDEXMod(0) / WIS +0xf6)
   + class term (`0x006b73a0` / `0x006b77e0` / `0x006b75c0`: sum of class save tables at each class level, + "SaveMult" scaling only
   for non-party creatures with owner+0x250, + Conditioning feats 13/21/22 = +1/+2/+3 top tier) + effects (`0x00582010` cat 3,
   subtype 1/2/3, only when noEffects == 0; the page passes 0). Breakdown plan: effects = get(0) - get(1); class+feat = get(1) - misc
   - ability; feat tier from HasFeat. The page colours the value by `0x0077d800(3,...)` effect delta.

## Design (owner decisions 2026-10-27)
- **Placement: a new "Stats" tab** in the character-effects panel (`active_effects_panel_src/`, character-effects-panel
  0.2.0-probe, doc 14). The panel already has the shown character's server creature (`party::serverCreature`), name/stat/time
  rows (`LBL_EFX_Nxx/Sxx/Txx`), hover tooltips (`TIPBG`, 10x10 wrapped body), scrolling, and a left-press hit test.
  Tab switch: click on the panel header `LBL_EFX_HEADER` (text "Effects | Stats", active tab named first) unless the mockup
  shows that 2 extra header labels are needed.
- **Vanilla page tooltips too** (user 2026-10-27): hovering a vanilla `character_p.gui` row shows what it is, a short
  description and, where we have it, the breakdown. Hover targets (live Override GUI rects): the 6 attribute rows
  (`LBL_STRENGTH..LBL_CHARISMA` + `_STAT`/`_MOD`), `LBL_VITALITY(_STAT)`, `LBL_FORCE(_STAT)`, `LBL_DEFENSE(_STAT)`,
  `LBL_FORTITUDE/REFLEX/WILL(_STAT)`, `SLD_ALIGN` (+ `LBL_LIGHT/DARK`), `LBL_EXPERIENCE/NEXT_LEVEL`, `LBL_FORCEMASTERY`.
  Tooltip bodies: Defense = verified parts; saves = base + ability + effects (+ form vs Force powers); alignment = exact
  GoodEvil 0-100 + forceadjust light/dark cost multipliers; Force = max FP, cost multipliers, regen; attributes = what the
  stat feeds (e.g. WIS -> Will, Force DC, FP). Description text is ours (English, a `.inc` table), not TLK.
- Stats tab sections: DEFENSE (deflection + parts, Defense, DR/resist), OFFENSE (attack per hand, crit range/mult, attacks
  per round), FORCE (power DC, light/dark cost multiplier, Force resistance), CHARACTER (exact alignment; influence only
  with `show_influence=1`, default 0: spoiler).
- Data: recompute from fields in a pure host-tested header (no calls into the combat path); the probe logs the recompute
  next to the engine's own deflection total so it can be proven live.

## Phases
| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| ST1 | Pin read paths: HasFeat 0x006b83f0, HasSpell 0x006bd9c0, BAB 0x006a6f80, IsWieldingLightsaber 0x005763e0, GetLevel 0x006a4e00, effect-type globals (DAT_00996088/8c, AC types DAT_00996170..80), ability bytes stats+0xee..0xf8; `query_ghidra.py address` each, Haiku verify + decoys, into the mod's GameAddr | - | - | DONE | fb39ff72 | sigs table in this doc |
| ST2 | Pure `CombatStats.h` + host test: deflection, Defense parts, crit range/mult, Force DC, FP cost multipliers (forceadjust), saves breakdown; vectors: AJD+Force Redirection = +6, Shien BAB-only, forceadjust rows 0/5/10 | ST1 | - | DONE | fb39ff72 | `run_tests.py <suite>` |
| ST3 | Mockup: `scripts/character_page_mockup.py` renders the Stats tab + one vanilla-row tooltip (Defense) | ST2 | user review | DONE | ad46e86d | user OK on look 10-27 |
| ST4 | Probe 0.3.0-probe: STATS log block per shown character (no UI) + read-only deflection check (engine total vs recompute) | ST2 | user GO install | INSTALLED | 265ae68b | LIVE 10-27 19:10 (KPM d5384ec, rollback character-effects-panel_reinstall_20261027_190959); play ST4-P1 |
| ST5a | Stats tab UI 0.4.0-dev: header click toggles Effects/Stats (`stats_tab=1`), 4 sections, label + value rows (pure `StatsRows.h`, host test `stats_rows_test`), breakdown tooltips; no "Attacks / round" (stub), off-hand attack/crit rows when dual-wielding; hot-swap (hooks unchanged) | ST3, ST4 | user GO install (after ST4-P1) | INSTALLED | 7a4bcc82 | LIVE 10-27 20:21 (KPM a211580, rollback character-effects-panel_swap_20261027_202152); play ST5-P1 |
| ST5b | `show_influence` rows (needs RE: creature -> NPC slot 0..11 for GetInfluence) | ST5a | RE + user GO install | INSTALLED | 88200304 | 0.5.0-dev LIVE 10-27 20:47 (KPM ac7ac0f, rollback character-effects-panel_swap_20261027_204746) |
| ST6 | Vanilla page tooltips (hover rects by tag, `vanilla_tooltips=1`, text table) | ST3, ST5 | user GO install | INSTALLED | 2d701706 | 0.5.0-dev LIVE 10-27 20:47 (KPM ac7ac0f, rollback character-effects-panel_swap_20261027_204746) |
| ST6b | 0.5.1-dev: header hint tip, Force Mastery tip (marker effects, class line, finding 6), Level row + class lines (Stats tab, Experience tip) | ST6 | user GO install | INSTALLED | 1701c24c | 0.5.2-dev LIVE 10-27 22:45 (KPM c67e7b9, rollback character-effects-panel_swap_20261027_224511) |
| ST6c | 0.5.2-dev: save breakdown tooltips (Fortitude/Reflex/Will: class levels + ability + Conditioning feat + other + effects, finding 7) | ST6b | user GO install | INSTALLED | b7142165 | 0.5.2-dev LIVE 10-27 22:45 (KPM c67e7b9, rollback character-effects-panel_swap_20261027_224511) |
| ST6d | 0.5.3-dev play feedback 10-28: vanilla tips over their own row + shrunk to the text, Alignment tip multipliers only, value label slack (Deflection/Attack main values wrapped away) | ST6c | user GO install | INSTALLED | 2583d864 | 0.5.3-dev LIVE 10-28 00:43 (KPM 444ada8, rollback character-effects-panel_swap_20261028_004336) |
| ST6e | 0.5.4-dev tooltip text trim (user 10-28: not verbose for no gain): no tip on Vitality/Next level, no text on Defense/Force points/Experience, shorter attributes/saves/Mastery/Deflection/Influence | ST6d | user GO install | BUILT | 14d83efe | 0.5.4-dev gate PASS; SWAP ready; not installed |

## Open (UNVERIFIED in W2)
- Two-weapon penalties, STR/DEX attack term, Offensive Training sign (`006a93a0` lines 177-222, 403-491).
- Damage immunity percent (vtable `+0xa8` in `0x005959e0`), energy shield absorb, damage shield reflect.
- Sneak attack dice, Power Attack mode terms, weapon specialization (`0x006abf70`, `0x006adec0`).
- Stealth vs Awareness detection margins (`0x00578850`); treat-injury heal formula.
- Force forms (spells 265-268) effects: not in the cited lines; likely in `0x0056a800` / FP cost path.
