# C1a: blaster bolt deflection (Steam swkotor2.exe)

Flow: ResolveAttackRoll 0x006e0b10 → IsProjectileParryable 0x006e24b0 (gate) → ResolveDefensiveEffects 0x006e1c60 (roll). Per-attack record = CSWSCombatRound::GetAttack 0x005908c0 (idx = byte at combatround+0xabc; combatround = attacker+0x10dc). Defender = iVar3 (asm: `MOV EDX,[EBP-0x18]; MOV EAX,[EDX+0x1198]`). Defender stats = creature+0x1198.

Rule as coded: D = defender total roll; T = attacker's d20 + attack mod. D >= T → deflected (outcome 8). D >= T+6 and reflect allowed → deflected and reflected (outcome 9).

| stat | player meaning | formula / parts | computed at | storage (record = attack rec) | vanilla UI? | patch read path | conf |
|---|---|---|---|---|---|---|---|
| Total deflect D | total defender must beat T with | sum of rows below | 0x006e1c60 | rec+0x168 int32 | no | read rec+0x168 after resolve | high |
| Threshold T | attacker's roll to beat | rec+0xc1 (d20) + rec+0xc2 (attack mod) → rec+0x15c | 0x006e0b10 / 0x006e1c60 | rec+0x15c int32 | no | rec+0x15c | high |
| d20 | random base | RollDice(1,20) &0xffff | 0x006e1c60 | rec+0x158 byte | no | rec+0x158 | high |
| Advanced / Master JD | +3 / +6 (MJD replaces AJD) | HasFeat(24) ? +6 : HasFeat(1) ? +3 (feat rows 1, 24) | 0x006e1c60 | rec+0x160 id, +0x161 bonus | no | HasFeat 0x006b83f0 on defender stats | high |
| Base attack bonus | BAB added to deflect | GetBaseAttackBonus 0x006a6f80 (defender) | 0x006e1c60 | rec+0x164 byte | no | 0x006a6f80 | high |
| Force Redirection (spell 163) | +3; can reflect | HasSpell(163) on defender stats | 0x006e1c60 | rec+0x16c (3), +0x16d (0xa3) | no | HasSpell 0x006bd9c0 | med-high |
| Force Deflection (spell 162) | no bonus; no reflect; works without lightsaber | gate branch C of 0x006e24b0; 162 without 163 sets *param_6=0 (local_c) | 0x006e24b0 | out-param | no | HasSpell(162) | high |
| Deflect feat (168, WM) | +(WM level+1)/2 | HasFeat(168) && IsWieldingLightsaber(defender); class id 0x0B level via GetClassLevel 0x006ad2f0 | 0x006e1c60 | WRITTEN rec+0x178 id, +0x179 bonus; SUMMED from defender stats+0x178/+0x179 | no | HasFeat(0xa8) + GetClassLevel | med (write/read mismatch, see Open 1) |
| Stance bonus | -5..+4 by combat mode, lightsaber only | if IsWieldingLightsaber: switch(defender+0x1110): 0x103 -5, 0x104 +4, 0x105 -4, 0x106 +2, 0x107 +1 | 0x006e1c60 | rec+0x170 adj, +0x174 mode; creature+0x1110 | no | read creature+0x1110 | med-high |
| Precise Shot I-V | -2/-4/-6/-8/-10, highest only | HasFeat 240..244 on `this` (ResolveDefensiveEffects this = attacker, asm spill EBP-0xb8) | 0x006e1c60 | rec+0x17a id, +0x17b signed | no | HasFeat 240..244 on attacker | med |
| Effect delta | +/- from scripted/item effects | per defender effect: type==DAT_00996088 +GetInteger(1); type==DAT_0099608c -GetInteger(1) | 0x006e1c60 | rec+0x162 byte; creature+0x148 list, +0x14c count | no | walk creature+0x148 | med |
| Assured flags | auto-deflect latch | fe4 set on first hit (0x598f40); fe0 = GetInteger(0)!=0; when fe4 set roll is skipped, outcome 8, or 9 if fe0 | 0x598f40, 0x598fe0 clears, 0x006e1c60 | creature+0xfe0, +0xfe4 int32 | no | read creature+0xfe4 / +0xfe0 | med |
| Outcome code | 8 deflect, 9 deflect+reflect | 8 if D>=T; 9 if D>=T+6 and local_c!=0, or assured fe0 | 0x006e1c60 | rec+0x5c and rec+0x163 | string "AttackDeflected" 0x995a08 (refs 0x58d2f0, 0x58d5e0); binding unverified | read rec+0x5c | high (codes) |
| Item prop BlasterBoltDeflection | adds to D via effect | effect type 0x996088; int0 = itemprop subtype (+2), int1 = cost value (+6); dur flag 0x9967b0 | 0x005b2430 | effect object, applied via 0x544210 | no | count effects by type | med |
| Script EffectBlasterDeflectionIncrease | amount | pops nAmount into SetInteger(0) only; scan reads GetInteger(1) | 0x677b50 | effect int0 | no | not recommended | low |

Best read point: no getter for the pre-roll total. Hook or call-after 0x006e1c60 and read rec+0x168 (D) and rec+0x15c (T). For a pre-roll value, recompute from the parts (HasFeat 0x18/0x01/0xa8, HasSpell 0xa3, GetBaseAttackBonus 0x006a6f80, creature+0x1110, effect list).

## Evidence
- 0x006e1c60 decomp: `local_24 = FUN_006a2000(1,0x14)`; `FUN_006b83f0(0x18)` → +6 else `FUN_006b83f0(1)` → +3.
- 0x006e1c60 asm 006e1d87 / 006e1dce: `MOV EAX,[EDX+0x1198]` with EDX=[EBP-0x18]=defender → JD/AJD/MJD checks on defender stats.
- 0x006e1c60 asm 006e1c69: `MOV [EBP-0xb8],ECX` (this); asm 006e2075: `MOV EDX,[EBP-0xb8]; MOV EAX,[EDX+0x1198]` → PS checks on this.
- 0x006e1c60 asm 006e1f02 / 006e1f0f write `[EAX+0x178]`, `[ECX+0x179]` with EAX/ECX=[EBP-4] (record); asm 006e2227 / 006e2234 read `[ECX+0x178]`, `[EAX+0x179]` from defender stats.
- 0x006e24b0 decomp: `FUN_006b83f0(0x37) || FUN_006b83f0(1) || FUN_006b83f0(0x18)`; `FUN_006bd9c0(0,0xa2,0)` and `(0,0xa3,0)`; `*param_6 = 0` when 162 and not 163.
- 0x598f40 decomp: `if (fe0==0 && fe4==0 && param_3!=0) { fe4=1; if (GetInteger(0)) fe0=1; }`; 0x598fe0 decomp: `fe4=0; fe0=0`.
- 0x006e0b10 decomp: `if (DAT_00a7e11c != 0) local_28 = DAT_00a7e11c;` (forces attack d20 only; no equivalent in 0x006e1c60).
- feat.tsv rows 1, 24, 55, 168, 240-244: texts match the bonus values above; JD text says "beaten by 10 or more" for reflect.
- spells.tsv rows 162 ("can't reflect blaster bolts back") and 163 ("can also reflect").

## Open questions
1. Deflect WM bonus: written to rec+0x178/0x179 but summed from defender stats+0x178/0x179. Either a real engine bug (feat never adds) or a decompile artifact. Check writers of CSWSCreatureStats+0x178/0x179 at asm level. Highest priority.
2. Reflect margin: code uses T+6, feat text says 10. Which is live needs a play log, not static.
3. JD "lightsaber equipped" requirement: not seen in 0x006e24b0 branch A; may be in unresolved callee 0x006e2c50.
4. Effect index base: scan reads GetInteger(1), script path writes SetInteger(0). Confirm CGameEffect index base; DAT_00996088 / DAT_0099608c IDs unresolved.
5. Precise Shot `this` attribution (attacker) is inferred from the ECX spill, not from a live trace.
