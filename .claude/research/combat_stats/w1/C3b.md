# C3b: lightsaber forms and stances (Steam swkotor2.exe)

Decomp: `ghidra_dumps/steam_6a522e71/decomp/6a/`. Form IDs are **spells.2da rows**, not feat rows (feat.tsv has no FORM_ rows).
Active form: `CSWSCreature +0x1110` (dword), reached as `*(stats+0x24) + 0x1110`. Read from GFF `CurrentForm` in `CSWSCreatureStats::ReadStatsFromGff` 0x006afed0 (l.531-532); written in `SaveStats` 0x006b3d10 (l.155).
Gate: every form effect below sits behind `CSWSCreature::IsWieldingLightsaber` 0x005763e0 (no saber, no form effect).

| stat | what it means for the player | formula / parts | computed at (addr, .c line) | storage (struct+offset) | shown in vanilla UI? | how a patch could read it | confidence |
|---|---|---|---|---|---|---|---|
| CurrentForm | active saber or Force form | 0x102=258 Shii-Cho, 0x103 Makashi, 0x104 Soresu, 0x105 Ataru, 0x106 Shien, 0x107 Niman, 0x108 Juyo; 0x109-0x10C = 265-268 Force I-IV | GFF r/w 0x006afed0 / 0x006b3d10 | creature +0x1110 dword | no form text in these fns | `*(int*)(*(int*)(stats+0x24)+0x1110)`; hotbar already reads it (`patch_manager_mods/07_force_power_hotbar_evidence/target_modes/decomp_objid_and_checks.txt:102`) | high |
| Shii-Cho (0x102) | defensive, +1 attack | attack +1 (006a93a0 l.814); AC +3 vs non-targets, 0 vs target (006a5110 l.442-448) | 0x006a93a0, 0x006a5110 | +0x1110; attacker id = param_2+4 | breakdown lines are `" + %d"` (unlabeled), gated by global DAT_00a7e084 | compare attacker id to +0x550/+0x548 | high |
| Makashi (0x103) | anti-saber, +3 attack vs saber target, +3 damage | attack +3 only if `IsWieldingLightsaber(param_2)` (l.818-821; asm call 006ab952 ECX=[EBP+8]); damage +3 with own saber (006abf70 l.316-318); no AC | 0x006a93a0, 0x006abf70 | +0x1110 | not in these fns | same as CurrentForm + target check | high |
| Soresu (0x104) | anti-blaster AC | AC +2 vs target, 0 vs others (006a5110 l.449-454); no attack term | 0x006a5110 | +0x1110, +0x548/+0x550 | unlabeled AC line | compare attacker id | high |
| Ataru (0x105) | single-target AC | AC +3 vs target, -2 vs others (l.455-461) | 0x006a5110 | same | unlabeled AC line | same | high |
| Shien (0x106) | aggressive, +2 attack, -5 AC vs target | attack +2 (006a93a0 l.824-827); AC -5 vs target (006a5110 l.462-467) | 0x006a93a0, 0x006a5110 | same | unlabeled | same | high |
| Niman (0x107) | balanced, +1 attack, +1 AC all | attack +1 (l.828-831); AC +1 unconditional (006a5110 l.468-470) | 0x006a93a0, 0x006a5110 | same | unlabeled | same | high |
| Juyo (0x108) | heavy single-target | AC -4 vs others, -2 vs target (l.471-478); attack term absent in these fns | 0x006a5110 | same | unlabeled | same | high (AC) |
| Total Defense stance | +4 AC, or +6 with a qualifying class | `IsInTotalDefense` 0x006daac0 true gives +4; +6 if slot 0 or 1 has class id in {2,5,13,16} and `GetClassLevel(slot,1) > 3` (006ad2f0) | 0x006a5110 l.387-436; label l.542 | class id `stats+0xab+i*0x28`; eff. level `+0xac - +0xad`; flag `creature+0x110c`; `[creature+0x10dc]+0xb1c == 13` | label "(Total Defense Stance Bonus)" under DAT_00a7e084 | call 0x006daac0 and read class bytes | med |
| Feat Dodge | none (dead) | `local_a8` only zeroed (l.212), never set non-zero; label l.220 unreachable | 0x006a5110 | n/a | n/a | n/a | high |

Form totals: form AC adds into the AC sum (006a5110 l.568 region, `local_1c` into `local_7c`). Form attack adds into `local_38` and writes the form ID to output `+0xe8` and the bonus to `+0xec` (006a93a0 ~l.835-837).

## Evidence
- 006a5110.c:441 `switch(*(undefined4 *)(*(int *)(param_1 + 0x24) + 0x1110))` with `case 0x102..0x108` (AC).
- 006a93a0.c:814 `case 0x102: local_20 = 1;` ; 818 `case 0x103:` with `param_2 != 0 && FUN_005763e0()` (attack).
- 006abf70.c:317 `== 0x103` then l.318 `local_18 = 3;` (Makashi damage).
- 006a5110.c:542 `" + %d (Total Defense Stance Bonus)"`; l.431/434 `local_3c = 4 / 6`.
- 006daac0 (CSWSCreature::IsInTotalDefense): `if (*(char*)(p+0x110c)==0 || ... *(p+0x10dc)+0xb1c == 0x0D) return 1`.
- 006afed0.c:531 `FUN_00718c00(param_3,"CurrentForm",...)` ; 532 `*(iVar11+0x1110) = uVar8`.
- spells.tsv rows 258-264 (FORM_SABER_I..VII), 265-268 (FORCE_I..IV); text e.g. row 262 "Attack Modifier: +2 ... Defense vs. Current Target: -5" matches code.
- 0057e3d0 = CSWSCreature::SetAttemptedAttackTarget writes `+0x550`; 0057a560 = SetCombatState writes `+0x548 = 0x7f000000`.
- Asm: 006a5110.asm call 006a6472 ECX=[creature+0x24]; 006a93a0.asm call 006ab8d7 ECX=[+0x24] and 006ab952 ECX=param_2.

## Open questions
- Class ids {2,5,13,16}: no text export; `replay/pre_checkpoint_2da/classes.2da` is binary V2.b, not decoded.
- IsInTotalDefense logic returns TRUE when `+0x110c == 0`, which looks inverted vs its name. No decomp writer for +0x110c found. Sub-object `+0x10dc +0xb1c` writers (006d9500, 00665f50) not traced.
- +0x548 name is a hypothesis ("current attack target"); +0x550 is verified as attempted target.
- DAT_00a7e084 (gates all breakdown strings) not identified.
- Form effects outside these 3 fns, not verified: HandleFPRegeneration 0x0056a800 (0x107, 0x109, 0x10b); CalculateOnHandAttacks 0x005905f0 (0x108 extra attack); GetCriticalHitMultiplier 0x006afcd0 (0x106 crit +1); ResolveAttackRoll 0x006e0b10 (0x104/0x105/0x108); ResolveDefensiveEffects 0x006e1c60; global "FormMask" 0x00999cf4 at 0x006c1490 (no decomp file).
- Blaster-bolt deflection, Force-save and threat-range terms from spells.tsv are not in the 3 seed fns.
