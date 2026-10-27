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
| ST1 | Pin read paths: HasFeat 0x006b83f0, HasSpell 0x006bd9c0, BAB 0x006a6f80, IsWieldingLightsaber 0x005763e0, GetLevel 0x006a4e00, effect-type globals (DAT_00996088/8c, AC types DAT_00996170..80), ability bytes stats+0xee..0xf8; `query_ghidra.py address` each, Haiku verify + decoys, into the mod's GameAddr | - | - | TODO | | sigs table in this doc |
| ST2 | Pure `CombatStats.h` + host test: deflection, Defense parts, crit range/mult, Force DC, FP cost multipliers (forceadjust), saves breakdown; vectors: AJD+Force Redirection = +6, Shien BAB-only, forceadjust rows 0/5/10 | ST1 | - | TODO | | `run_tests.py <suite>` |
| ST3 | Mockup: `scripts/character_page_mockup.py` renders the Stats tab + one vanilla-row tooltip (Defense) | ST2 | user review | TODO | | user OK on look |
| ST4 | Probe 0.3.0-probe: STATS log block per shown character (no UI) + read-only deflection check (engine total vs recompute) | ST2 | user GO install | TODO | | play row in PLAY_TEST_QUEUE.md |
| ST5 | Stats tab UI (header toggle, sections, rows, breakdown tooltips, `show_influence`) | ST3, ST4 | user GO install | TODO | | play row |
| ST6 | Vanilla page tooltips (hover rects by tag, `vanilla_tooltips=1`, text table) | ST3, ST5 | user GO install | TODO | | play row |

## Open (UNVERIFIED in W2)
- Two-weapon penalties, STR/DEX attack term, Offensive Training sign (`006a93a0` lines 177-222, 403-491).
- Damage immunity percent (vtable `+0xa8` in `0x005959e0`), energy shield absorb, damage shield reflect.
- Sneak attack dice, Power Attack mode terms, weapon specialization (`0x006abf70`, `0x006adec0`).
- Stealth vs Awareness detection margins (`0x00578850`); treat-injury heal formula.
- Force forms (spells 265-268) effects: not in the cited lines; likely in `0x0056a800` / FP cost path.
