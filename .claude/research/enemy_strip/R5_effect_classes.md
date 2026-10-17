# R5 effect classes (enemy strip design input)

Sources: `active_effects_panel_src/ChareffDecode.h` (decode arms, `stateName`), `_shared_src/EffectIcons.h` (icon lookup), `_shared_src/Effects.h`, `_shared_src/DebuffIconTable.inc`, R3 + R-a (`active_effects_panel_src/research/R3_effect_types_2026-10-02.txt`), AI8 (`engine_ai_atlas/AI8_effects.md`, med where noted), vanilla scripts in `~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/vanilla` (read-only), archived live tallies in `~/Games/KOTOR2_Mod_Workspace/logs_archive/`. Types hex, states decimal.

## 1. Type -> category (decode-named types, plus the icon and bookkeeping types)

| type | name | category | subtype / ints note |
|---|---|---|---|
| 0x08 | SetState | obvious-CC: st 1,2,3,4,5,6,8,16; cc-other: st 7,10,15,18,19 | ints[0] = state (decimal) |
| 0x12 | Entangle | obvious-CC | no decode arm, so no stat text |
| 0x14 | Knockdown | obvious-CC | no decode arm; spawns state 14, which is unnamed and hidden |
| 0x1d | MovementSpeedDecrease | obvious-CC in Slow family, else stat-debuff | ints[0] pct, raw |
| 0x23 | Poison | dot | ints[0] poison type |
| 0x25 / 0x24 | AbilityDecrease / Increase | stat-debuff / buff | ints[0] ability 0-5, [1] amount |
| 0x31 / 0x30 | ACDecrease / Increase | stat-debuff / buff | [1] amount, [0] mod type |
| 0x0b / 0x0a | AttackDecrease / Increase | stat-debuff / buff | [0] bonus, [1] hand |
| 0x0e / 0x0d | DamageDecrease / Increase | stat-debuff / buff | [0] enum 1-10, [1] type mask |
| 0x1b / 0x1a | SavingThrowDecrease / Increase | stat-debuff / buff | [0] value, [1] save, [2] save type |
| 0x38 / 0x37 | SkillDecrease / Increase | stat-debuff / buff | [0] skill 0-7 |
| 0x11 / 0x10 | DamageImmunityDecrease / Increase | stat-debuff / buff | [0] mask, [1] pct |
| 0x22 / 0x21 | ForceResistDecrease / Increase | stat-debuff / buff | [0] value |
| 0x5d / 0x5c | BlasterDeflectionDecrease / Increase | stat-debuff / buff | [0] value |
| 0x1c | MovementSpeedIncrease | buff | v >= 100 means pct = v-100 |
| 0x16 | Immunity | buff | [0] immunity 0-33 |
| 0x02 | DamageResistance | buff, or **shield pool** when sub&0x8 and ints[2] > 0 | pool: ints[2] = remaining absorb. Energy Resistance (sub 0x9, ints[2] = 0) is buff |
| 0x0c | DamageReduction | buff | [0] amount, [2] limit |
| 0x0f / 0x5b | TemporaryHP / TemporaryFP | buff | [0] amount |
| 0x07 | Regenerate | buff | [0] amount, [1] ms |
| 0x2f / 0x4c | Invisibility / Concealment | buff | |
| 0x2c / 0x70 / 0x71 | ModifyAttacks / FP regen / VP regen | buff | |
| 0x3d | DamageShield | shield | [0] amount, [1] random, [2] type |
| 0x6b | ForceShield | shield (stat label only) | [0] forceshields row; the pool is the 0x02 above |
| 0x1e | VisualEffect | visual-only | ints[0] = colour key |
| 0x43 | EffectIcon | visual-only carrier | ints[0] = effecticon row; icon source only |
| 0x09 / 0x17 / 0x44 | internal SetState sibling / AI-state mask / innate | visual-only (bookkeeping) | AI8; innate sub 0x4 |
| 0x3c / 0x67 / 0x13 / 0x1f / 0x39 / 0x01 | ForcePushed / ForceJump / Death / AoE object / HP-when-dying / Haste | transient or bookkeeping | 0x3c is AI8 med. 0x6e ForceBody and 0x6f Fury are buff with no decode arm |

## 2. Force powers and grenades: effects, spell row, icon source

Spell path = `iconForSpell(+0x1c, debuff=true)`; "yes" = DebuffIconTable row non-null. Rows from `k_inc_force.nss` / `k_sup_grenade.nss` case blocks.

| row | power | effects applied | 0x43 row | spell-path icon |
|---|---|---|---|---|
| 16 | Fear | 0x08 st8 Horrified, +0x1e | 6 ip_fear | yes |
| 30 / 31 | Horror / Insanity | 0x08 st8 Horrified | 16 / 17 | yes |
| 29 | Stasis | 0x08 st5 Paralyzed, 0x1d -50, 0x31 AC-4 | 15 ip_hold | yes |
| 44 | Stasis Field (SLEEP const) | 0x08 st5 Paralyzed (not st6), 0x1d -50, 0x31 AC-4 | 27 ip_sleep | yes |
| 45 | Slow | 0x1d -50, 0x31 AC-2, 0x0b Att-2, 0x1b Reflex-2 | 28 ip_slow | yes |
| 46 | Stun | 0x08 st4 Stunned, 0x1d -50, 0x31 AC-4 | 29 ip_stun | yes |
| 47 / 12 | Stun Droid / Disable Droid | 0x08 st3 DroidStun (+ droid damage on 12) | 30 / 4 | yes |
| 9 | Choke | 0x25 x3 (-4 CON/STR/DEX), 0x08 st7 for 6 s | 3 ip_choke | yes |
| 7 / 27 | Affliction / Force Whirlwind | 0x23 poison + 0x1d -50 / 0x08 st10 | 1 / 14 | yes |
| 200 | Force Confusion | faction-modifier, no state (type UNPROVEN) | none | row exists, no tracked effect |
| 91 | Grenade Sonic (+186,187,255,256) | 0x25 DEX -6/-4/-2 for 30 s. NOT a state | 41 = stun icon | yes (iw_sonicgren) |
| 88 | Grenade Stun | 0x08 st4 for 9 s (droid st3) | 41 iw_stungren | yes |
| 92 | Grenade Adhesive | **0x12 Entangle** for 15 s. NOT slow | 43 iw_adhsvgren | yes |
| 93 | Grenade Cryoban | 0x08 st5 Paralyzed for 6 s | 44 iw_cryobgren | yes |
| 94 | Grenade Plasma | 0x08 st8 Horrified for 2 s | none | yes (iw_firegren) |
| 90 / 191 / 195 / 196 | Poison gren / Poison rocket / Poison dart / Kyber dart | 0x23 poison, no 0x43 | none | yes (rows exist) |
| 95, 87, 89, 185, 190 | Ion / Frag / Thermal / Explosive / Buster | instant damage only (Thermal adds a 0.1 s push, type UNPROVEN) | none | rows exist, never a persistent effect |
| 194 | Concussion rocket | 0x3c 0.1 s, then 0x08 st4 at +2.55 s for 9 s | none | yes (iw_rocket_001) |
| 197 | Paralysis dart | 0x08 st5 on fort fail (icon 44); slow with no icon on save | 44 | yes (iw_rocket_007) wins over 44 |
| 198 | Tranquilizer dart | 0x08 st4 for 9 s | 41 | yes (iw_rocket_011) |

Quirks: dart and rocket spells.2da icons are iw_rocket_*, so the spell path beats the 0x43 row. Sonic uses stun row 41, not 42.

spellId (+0x1c) status:
- Force powers: real row. INFERRED. Live script effects (sub 0x9) carry real rows (22, 8, 42, 69, 70, 75, 99, 110, 132). The archived logs have zero sub-0x9 entries with -1. Fear, Stun and similar were not sampled. UNPROVEN.
- Grenades: real row expected by the same mechanism (energy shield spell 99 from item use). INFERRED. No grenade sample in the repo. UNPROVEN.
- Item-property (sub 0x3) and innate (sub 0x4): -1. LIVE (about 6.5k lines with spell=-1, mostly item skill and damage).
- Non-creature creators (trap, mine, placeable): -1, since CSWSObject's vt[0xB8] returns -1 (code-verified, AI8/02). Live UNPROVEN. Only the 0x43 sibling can give them an icon.

## 3. Never give an icon

0x1e visual sibling; 0x43 as its own tile (icon source only, dedupes with its parent); 0x02 shield pool (sub&0x8, ints[2] > 0, the shield bar owns it); 0x09, 0x17, 0x44, 0x3c, 0x67, 0x13, 0x1f, 0x39, 0x01 (engine siblings, masks, transient, bookkeeping).

## 4. What the Buff HUD does with no spell id

Code is in `_shared_src/EffectIcons.h`, not BuffHudLogic.h (arrow counters and column geometry only).
- `iconForSpell` returns null for spell < 0 (:17). Debuff column only: falls back to a 0x43 sibling with a harmful effecticon row (good == 0), deduped by resref (`iconForEffectIconEffect` :28). Buff column has no fallback, so spell -1 buffs are never drawn.
- Effects that fail both are skipped and not counted, so the native arrow stays (`collectBuffs` :40).
- Diagnostic `logUntrackedDebuff` (BuffDurationHUD.cpp:345, called at :412) logs up to 40 lines when raw debuff > 0 and tracked = 0. No DBGMISS line exists in the repo or the archived logs. Play test WI-126 is pending (PLAY_TEST_QUEUE.md:35).
- Doc drift: 02_buff_duration_hud.md:1691 and :1704 say grenades and rockets have no icons. The generated DebuffIconTable.inc has them (rows 87-95, 185-199). Trust the table.

## 5. OPEN / UNPROVEN

- Live spellId for a grenade, Fear, or Stun: no sample.
- Frightened (2), Confused (1), Droid confused (16), Daze, Blind: no vanilla .nss producer (nwscript declarations only; EffectBlind only in RemoveEffect, k_ai_master.nss:317). "Frightened/cower" maps to st8 in vanilla. Sleep (6) is applied only at k_inc_generic.nss:1784.
- AI8 ids 0x12, 0x14, 0x3c, 0x67 and state 14 are med. EffectForcePushTargeted type not traced. Enemy monster abilities: no data.
- Creator objids seen live: 0x7ffffffd (force powers), 0x7fffffff (items). Party membership mapping UNPROVEN.
