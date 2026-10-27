# Hidden combat stats: Haiku swarm wave plan (2026-10-27)

Goal: list every stat KOTOR II computes but never shows the player (blaster deflection, Defense parts, crit range,
resistances, saber form modifiers, Force DCs and costs, influence, exact alignment, skill DCs). For each one, find
where the engine computes it and how a patch could read it, as input for a "Combat Stats" page.

## Context budget (per agent, target ~100k peak)
- Fixed Haiku overhead ~45k (measured, SUBAGENT_ROLES.md).
- Brief ~2k.
- Reads cap ~45k tokens (~160 KB). Never read a whole NSS script (k_inc_force.nss 325 KB); `rg -n -C` only.
  Decomp `.c` files over 30 KB: `sed -n` ranges.
- Output: facts file <= 60 lines + 5-line RECON reply.

## Inputs ($0, W0 done by Opus)
- 2DA exports with TLK names/descriptions resolved: `.claude/waves/combat_stats/data/{feat,spells,iprp_*,effecticon}.tsv`
  (gitignored; game data).
- Steam static dumps: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<addr[2:4]>/<addr>.c|.asm`,
  `strings.tsv` (addr, text, n, fn list), `callgraph.tsv`.
- Tools: `python3 scripts/query_ghidra.py symbol|address`, `python3 patch_manager_mods/engine_ai_atlas/tools/cg.py callers|callees <hex>`,
  `python3 patch_manager_mods/ghidra_knowledge/tools/static_facts.py addr <hex>`.
- Vanilla scripts: `~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/vanilla/*.nss`.

## Waves
| wave | units | model | input | output |
|---|---|---|---|---|
| W0 seeds | Opus | - | 2DA export, seed addresses | this file |
| W1 recon | 10 (5 categories x 2 lanes) | Haiku | seeds below | `w1/<unit>.md` |
| W2 verify + gaps | 5 (one per category) | Haiku | both W1 files of its category | `w2/<cat>.tsv` (verified rows) |
| W3 gate | Opus | - | W2 TSVs, 2-3 spot checks | design doc + candidate list |

W2 starts only after all W1 units of that category return. Escalation: retry once on Haiku, then split, then one Sonnet.

## W1 units (seeds = Steam addresses)
| unit | category | scope | seeds |
|---|---|---|---|
| C1a | defense | blaster bolt deflection roll: formula, bonuses, where resolved | ResolveAttackRoll 0x006e0b10, ResolveDefensiveEffects 0x006e1c60, BlasterDeflectionIncrease effect 0x00677b50 / itemprop 0x005b2430, OnApplyAssuredDeflection 0x00598f40, SetDeflectArrow 0x005922f0; feat rows 1,24,55,168 |
| C1b | defense | Defense breakdown + damage reduction/resistance/immunity/shields | GetArmorClassVersus 0x006a5110, ResolveDamage 0x006e1540, ResolveDamageShields 0x006e3650, ApplyDamageReduction 0x005b04f0, ApplyDamageResistance 0x005b0820; iprp_damagetype/immunity tsv |
| C2a | offense | attack bonus per hand, attacks per round, two-weapon penalties | GetAttackModifierVersus 0x006a93a0, GetAttacksPerRound 0x006a7170 |
| C2b | offense | crit range/multiplier, damage bonus parts, sneak attack | GetCriticalHitRoll 0x006afa60, GetCriticalHitMultiplier 0x006afcd0, GetDamageBonus 0x006abf70, ResolveSneakAttack 0x006e1000 |
| C3a | forms | saber forms + stances in game data: what each claims to change | feat.tsv / spells.tsv (usertype 2/4/6 rows, labels FORM_/STANCE) |
| C3b | forms | where the engine applies form/stance modifiers | "(Total Defense Stance Bonus)" in 0x006a5110, form feat/spell ids found in 0x006a5110 / 0x006a93a0 / 0x006abf70 |
| C4a | force | power save DC, saving throws, Force resistance | CalculateSpellSaveDC 0x0058a1c0, ExecuteCommandGetSpellSaveDC 0x00688750, GetFortSavingThrow 0x006ba620, GetWillSavingThrow 0x006ba6b0, GetReflexSavingThrow 0x006ba740, OnApplyForceResistanceIncrease 0x0059f710 |
| C4b | force | real FP cost (alignment), FP/Vitality regen | strings FPRegenTime (fn 0x0056a800), OnApplyRegenerate 0x0059fcb0, ApplyRegeneration 0x005b21c0; spells.tsv forcepriority/goodevil; k_inc_force.nss |
| C5a | world | companion influence, exact alignment | GetInfluence 0x005fa9c0, ExecuteCommandInfluence 0x00687230, Load2DArrays_Influence 0x006ed110; nwscript GetGoodEvilValue / GetInfluence |
| C5b | world | skill checks: Awareness vs Stealth, Computer/Security/Demolitions DCs, Treat Injury heal amount | GetSkillRank 0x006b7bb0; `patch_manager_mods/engine_ai_atlas/AI3_perception.md`; vanilla nss |

## Status
- W0-W3 DONE 10-27 ($3.08, 15 Haiku). Result: patch_manager_mods/51_combat_stats_page.md.
