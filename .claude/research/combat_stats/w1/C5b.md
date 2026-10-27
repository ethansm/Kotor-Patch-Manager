# C5b: hidden skill-check numbers (Steam exe + vanilla scripts)

Skill ids (nwscript.nss:1043-1050): 0 ComputerUse, 1 Demolitions, 2 Stealth, 3 Awareness, 4 Persuade, 5 Repair, 6 Security, 7 TreatInjury.
Calling convention (asm 006b7bb0): ECX = stats (this), stack = skill, vs, flag. Flag 1 = base only (script 0x366 path).

| stat | player meaning | formula / parts | computed at | storage | shown in vanilla UI? | how a patch reads it | conf |
|---|---|---|---|---|---|---|---|
| Base rank | points spent | char[skill] | GetSkillRank 0x6B7BB0 (flag 1 returns here) | CSWSCreatureStats +0x174 (char x8) | yes: skill page reads it (8A48A0:18) | read `*(char*)(*(int*)(stats+0x174)+skill)`; script GetSkillRankBase | high |
| Total rank | number used in every roll | base + effect + ability + feat + spell + table bonus; signed-char clamp | 0x6B7BB0 flag 0 | not stored | yes: total minus base passed to row (8A48A0:20) | script GetSkillRank(skill,obj); or call 0x6B7BB0 | high (struct), med (terms) |
| Feat bonus | +1/+2/+3, top match only | Gear Head 12/119/120 (CU, Repair, Security); Caution 7/117/118 (Demo, Stealth); Empathy 10/121/122 (Awareness, Persuade, TreatInjury) | 0x6B7BB0 cases 0-7 | feat list | aggregated only | HasFeat 0x6B83F0(id) | high |
| Force Camouflage | Stealth +8 or +4; untrained -> 1 | HasSpell 158 -> +8, else 157 -> +4; if rank 0 and HasSpell 156 -> rank 1 | 0x6B7BB0 case 2 + top gate | spell list | aggregated | HasSpell 0x6BD9C0(0,id,0) | high |
| Ability mod | adds key ability | key = skill rec +0x18 vs globals DAT_009960F8..0C; mods at stats +0xEE, GetDEXMod 0x6B5DB0(0) on 2nd key, +0xF2, +0xF4, +0xF6, +0xF8; -4 if FUN_00585420()!=0 on the 0xEE and DEX keys | 0x6B7BB0 | stats chars | aggregated | read offsets (order inferred, see Open 1) | med/low |
| Effect bonus | magic/item skill bonus | GetTotalEffectBonus(cat 5, vs, out, 0,0,0,0, skill, 0xFF, 0,0) | 0x582010 via 0x6B7BB0 | effect list | aggregated | call 0x582010 with same args | med |
| Table bonus | per-skill flat bonus | if skill rec +0x1C bit 2: + char[stats+0x178] + char[+0x179] | 0x6B7BB0 | stats +0x178/+0x179 | no | read chars | low |
| Untrained gate | untrained skill = 0 | rank 0 and rec +0x1C bit 1 clear -> total 0, before any bonus | 0x6B7BB0 | skill table rec +0x1C | n/a | n/a | high (code), med (meaning) |
| Persuade 30 event | one-time event for PC | IsPC(+0x6C) and DAT_00A7E118==0 and total > 29 -> FUN_004010A0(0x22) | 0x6B7BB0 case 4 | global DAT_00A7E118 | no | GetSkillRank(4) for PC can fire it | high (code), effect unknown |
| Stealth vs NPC detect | seen or not, per tick | margin = Awareness + dieP - (Stealth + dieT) - distance - moving/combat; seen iff margin > 0; Stealth rank 0 -> seen (med) | 0x578850 spot, 0x577FA0 listen | dice at creature +0x1024/+0x1025 | no (AI only) | GetSkillRank(T,2,P) plus dice fields | med |
| Awareness detect | perceiver side | +Awareness full if detect mode 1/2 else half; dieP = rand%20+1 (mode 1/2) else rand%10+1 | 0x578850, 0x577FA0; also skill 3 at 0x56C310 (UpdateMineCheck) | dice +0x1026/+0x1027 | no | GetSkillRank(P,3,...) | med |
| Computer Use spike cost | spikes per action | cost = nDC - floor(CU/4); if < 1: 0 when <= -3, else 1; nDC 2-8 per action | script UT_DeterminesItemCost | count = PC item tag K_COMPUTER_SPIKE | yes: custom tokens 31-33 (k_con_tokens.nss:68-70) | call UT_DeterminesItemCost(nDC,0) logic | high |
| Security lock DC | lock difficulty | DC = GFF OpenLockDC byte; unlock passes if DC <= d20 + Security total (+ lockpick ushort, combat only); out of combat roll fixed 20 | AIActionUnlockObject 0x5DB8F0 (L613), skill 6 at L581 | placeable +0x2B4 (byte), door +0x2FF; CloseLockDC +0x2B5 | not confirmed (OPEN 4) | read byte at offsets; nwscript GetLockUnlockDC (540) | med |
| Demolitions trap/mine DC | detect and disarm | GFF DetectDCMod, DisarmDCMod (ints), owner snapshot OwnerDemolitionsSkill = GetSkillRank(1,0,0) at placement; mine roll rand%20+1 + rank vs base DC +/-5 | LoadTrigger 0x620250 (L279-293); AIActionSetMine 0x5B5AD0 (L332, L318-329); ToggleMode 0x694470 (L124-125) | trigger +0x318 DetectDC, +0x31C DisarmDC, +0x320 owner rank; TrapDetectable +0xC0, TrapDisarmable +0xC1 | not confirmed | nwscript GetTrapDisarmDC (535) / GetTrapDetectDC (536) | med |
| Treat Injury heal | HP restored by medpac | med pack rank+10+WIS; advanced 2*rank+20+WIS; life support 3*rank+30+WIS; droid kits use Repair (84: +INT+15; 127: 2x+25; 128: 3x+35) | k_sup_healing.nss:16-46, EffectHeal(nSkill) | none | not confirmed | script hook: read GetSkillRank(SKILL_TREAT_INJURY) | high |
| HP regen (per tick) | passive regen | (char stats+0xF2 x table rate) + rank(7 or 5 for droid type at +0xE0) x SkillRankBons float + feat 196 bonus | HandleHPRegeneration 0x56A230 (L70-88) | table SkillRankBons | unknown | recompute from parts | med |
| Persuade (dialogue) | talk gate | rank range compare only, no d20; thresholds are dialogue node params | c_sc_per_bet/gt/lt/eq.nss (GetSkillRank(4, speaker)) | dialogue GFF params (not read) | dialogue only | GetSkillRank(4, speaker) | high (no roll) |

Evidence:
- 006b7bb0.c (body): `if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123))`; `local_8 = (int)*(char *)(*(int *)(param_1 + 0x174) + (uint)param_2);`
- 006b7bb0.c case 2: `iVar2 = FUN_006bd9c0(0,0x9e,0)` (HasSpell per name map); case 4: `(0x1d < local_8)` then `FUN_004010a0(0x22)`
- 008a48a0.c:18-20: `FUN_006b7bb0(local_18 & 0xff,0,1)` base and `(..,0,0)` total, then `FUN_008a5fb0(iVar4,(int)cVar2,(int)cVar3 - (int)cVar2)`
- feat.tsv rows 7,10,12,117-122 (names match the case tables); spells.tsv 156-158 ("Force Camouflage", "+4" in 157)
- 5d/005db8f0.c:581 `FUN_006b7bb0(6,0,0)`; L613 `if ((int)local_2dc <= local_1c + local_14)`
- 5e/005ef570.c:193-194 `"OpenLockDC"` -> `*(undefined1 *)(param_1 + 0xad)`; 61/0061a210.c:273-274 same field at +0x2FF
- 62/00620250.c:283-293 `"OwnerDemolitionsSkill"` -> `param_1[200]`; `"DisarmDCMod"` -> `param_1[199]`; `"DetectDCMod"` -> `param_1[0xc6]`
- 5b/005b5ad0.c:332 `cVar1 = FUN_006b7bb0(1,0,0);`; 69/00694470.c:124-125 `*(int *)(local_40 + 800) = (int)cVar1;`
- k_inc_utility.nss:1599-1601 `fModSkill = IntToFloat(GetSkillRank(nSkill, GetPartyMemberByIndex(0)))`; `nUse = nDC - FloatToInt(fModSkill)`; k_act_com33.nss:14 `UT_DeterminesItemCost(8, SKILL_COMPUTER_USE)`
- k_inc_utility.nss:1694-1697 UT_ReturnSpikePartAmount: `"K_COMPUTER_SPIKE"` / `"K_REPAIR_PART"`
- k_sup_healing.nss:16 `int nSkill = GetSkillRank(SKILL_TREAT_INJURY);`; `eHeal = EffectHeal(nSkill);`
- c_sc_per_bet.nss: `GetSkillRank( SKILL_PERSUADE, GetPCSpeaker() ) >= nCompareLowAmt`
- nwscript.nss:4599-4617 GetTrapDisarmDC / GetTrapDetectDC / GetLockUnlockDC prototypes (engine bodies not located)
- patch_manager_mods/engine_ai_atlas/AI3_perception.md:46-49 (gate, rank, perceiver rows); :53 (distance-sign bug); :55 (dice rule)

Open questions:
1. Ability offsets: 0xEE/0xF2/0xF4/0xF6/0xF8 assumed STR/CON/INT/WIS/CHA with DEX at 0xF0 (2-byte stride, GetDEXMod on key 2); per-skill key and flag bits need skills.2da (not in repo exports).
2. Effect category 5 is not nwscript EFFECT_TYPE_SKILL_INCREASE (54, nwscript.nss:379); meaning unresolved.
3. FUN_00585420 reads flags at +0xFF4 of ECX, but ECX is the stats object here; receiver of the -4 is unresolved.
4. Engine bodies for GetTrapDisarmDC / GetLockUnlockDC not located; DetectDCMod/DisarmDCMod base values live in template files (not read). Spike item: tag K_COMPUTER_SPIKE vs resref g_i_progspike01 (a_givespikes.nss:9), match unverified.
5. Whether the skill row renders the bonus as text: 8A5FB0 only appends a 12-byte row record in the trace.
6. Feat text says "at least one skill point" but code gates on rec +0x1C bit 1; confirm all Gear Head/Caution/Empathy skills have bit 1 clear.
