# B12a Opus gate, part a (31 rows: 18 full-read, 5 sample, 8 AUTO)

How each row was checked:
- I compared the GOG and Steam fp rows (size, retn, n_insn, disp, imm, callee count) for the part's address and for every reviewer `corrected_addr`.
- I counted fp twins in both binaries. Only GetWeaponHitShots and the scalar deleting destructor have twins, and a string or callee anchor separates each of them.
- I read the Steam decomp/asm for every row. Every asm line quoted in `overrides_a.tsv` was re-checked on disk.
- I checked each name against the Mac x86 symbol's shape, using a Mac sweep of the combat block (0x36F000-0x378400) and of each label_suspect row's Mac neighbourhood.

Short forms: "fp=" means GOG and Steam are identical in size, retn, n_insn, disp, imm and callee count; "unique" means no other function in either binary has that fp.

Verdicts: verified 16, likely 0, label_suspect 14, rejected 1, unresolved 0.
Steam address differs from the part file in 3 rows, all to the reviewer's corrected address with fp= checked: 0x65c680 (0x543ae0 -> 0x6e0380), 0x67c0c0 (0x563390 -> 0x563490), 0x68ddf0 (0x575340 -> 0x5752c0).

## Systematic findings

1. **The GOG ranged-combat block has a label rotation**, not isolated swaps. Body (Mac shape) vs GOG label:

| GOG | GOG label | body fits (Mac) | Steam |
|---|---|---|---|
| 0x656D70 | IsInTotalDefense | IsInTotalDefense (disp 0xb1c,0x10dc,0x110c, imm 0xd) | 0x6DAAC0 |
| 0x657710 | IsInTotalDefense (dup) | CSWSCreature wrapper of CClientExoApp::GetClientRangedAnimation (inlined on Mac) | 0x6DB460 |
| 0x658540 | ResolvePostRangedDamage | ResolveRangedAnimations (0xa98/0xb08, calls 0x65ef50, 0x65c680, 0x65c900) | 0x6DC290 |
| 0x658710 | GetWeaponDischargeTime | ResolveRangedHit (anim 0x271c/0x271e, target pos to +0x68..0x70) | 0x6DC460 |
| 0x6587E0 | IsLightsaberEquipped | ResolveRangedMiss (calls ResolveProjectileMissLocation 0x658870) | 0x6DC530 |
| 0x65AAE0 | (FUN_) | GetWeaponDischargeTime (disp 0xe0,0x130, calls GetWeaponDischargeRow) | 0x6DE7E0 |
| 0x65B530 | ResolveRangedHit | dice helper (disp 0x59c x3, rand callees) | - |
| 0x65C680 | ResolveRangedMiss | animation-id predicate (no Mac symbol) | 0x6E0380 |
| 0x65C900 | ResolveRangedAnimations | attack-anim setter helper, called by ranged and melee anims (no Mac symbol) | 0x6E0600 |
| 0x65E710 | ApplyOnHitSlayAlignmentGroup | ResolveRangedMissToParry (disp 0x5c,0x5c,0xabc,0x10dc, imm 8,0xa) | 0x6E2410 |
| 0x65EF50 | ResolveRangedMissToParry | IsLightsaberEquipped | 0x6E2C50 |

   - The Steam counterparts all sit in 0x6DA000-0x6E4000 as the brief predicted. Every 0x54xxxx near-pick in this block was wrong (0x542e00, 0x543ae0, 0x544130, 0x546fb0, 0x547100).
   - The knowledge DB has medium `gog_cluster_delta_map` rows carrying these rotated names at 0x6DB460, 0x6DC460, 0x6DC290, 0x6DC530, 0x6E2410 and 0x6E2C50. The label_suspect demotion step handles the rows in this part. 0x6DC290, 0x6DC530 and 0x6E2410 are outside the part and need their own rulings.
2. **Add*Actions labels are shifted by one.**
   - GOG 0x680A70 "AddDropItemActions" has the AddPickUpItemActions body; 0x680B30 "AddHealActions" has the AddDropItemActions body; 0x680C50 "Rest" has the AddHealActions body (Mac disp 0xec,0x1198, imm 0x38) -> Steam 0x568020.
   - The Mac order is PickUp, Drop, Heal.
3. **Misc creature labels are swapped:**
   - GOG 0x693490 "GetDetectMode" is ResolveInitiative.
   - GOG 0x68A4B0 "ResolveInitiative" is an unnamed `arg<10` predicate.
   - GOG 0x693AC0 "AddPickUpItemActions" is ComputeArmourClass.
   - GOG 0x696DB0 "SetLastInstantEquip" is RemoveMindTrickEffects; kDB already holds it as RemoveMindTrickAndScrambleEffects.
   - GOG 0x68DDF0 "RemoveMindTrickEffects" is the CFollowInfo ctor (kDB already renamed).
   - Possible real homes, by Mac shape only and not gated: SetLastInstantEquip = GOG 0x6A3E00 "HasEffect" (disp 0x11d0,0x11d4) -> 0x58B320; GetDetectMode = GOG 0x698DF0 (disp 0xd8,0x510) -> 0x5802C0; SetAmbientAnimationState = GOG 0x6A4260 (disp 0xd8,0x504, imm 0xe) -> 0x58B780.
4. **The pipeline gave one Steam address to two rows:**
   - 0x563390 was given to both GetPortrait (GOG 0x67BFC0, correct: 100 B getter) and Destructor (GOG 0x67C0C0). The Destructor is 0x563490.
   - The kDB row 0x563490 = "CSWSCreature::GetPortrait" (gog_cluster_delta_map, medium) is stale.
5. **GOG 0x692F53 is a fragment** (0x692F50 + 3; GOG 0x692F50 "BroadcastPoisoned" is a 3-byte stub label).
   - Rejected per the rule: never ship a mid-function address.
   - The whole body (Steam 0x57A420, PUSH 0x1b, caller OnApplyPoison) is BroadcastPoisoned, which the kDB already holds.
   - Real BroadcastStealthDetectionData (Mac imm 0x10) is probably GOG 0x692330, labelled "BroadcastForcePointUsageData" (imm 0x10) -> Steam 0x579800. Not gated.

## Per-row evidence

| # | GOG | GOG label | Steam | verdict | evidence (short) |
|---|---|---|---|---|---|
| 1 | 0x657710 | IsInTotalDefense_2 | 0x6db460 | label_suspect | fp= unique; `006db491 CALL 0x7404d0` (fp= GOG 0x40d990 GetClientRangedAnimation) with ECX=[0xa1b4a4]+4, pushes [this+4] id; caller ResolveRangedAttack 006dafdb. Melee twin GOG 0x6576c0 |
| 2 | 0x658710 | GetWeaponDischargeTime | 0x6dc460 | label_suspect | fp= unique; `006dc4b5 MOV EDX,0x271e`, `006dc4d7 MOV EAX,0x271c`; target +0x94..0x9c to attack +0x68..0x70; = Mac ResolveRangedHit shape. Mac DischargeTime is a 2DA getter (0xe0,0x130) = GOG 0x65aae0 |
| 3 | 0x65b5a0 | ResolveMeleeSpecialAttack | 0x6df2a0 | verified | fp= 4075 B, 20 callees; `006dfc11 MOV byte [EDX+0x145],0x4`; sole caller ResolveMeleeAttack `006dec87`; Mac disp 0x3a,0xee,0x122,0x145,0x154 |
| 4 | 0x65c680 | ResolveRangedMiss | **0x6e0380** | label_suspect | 0x543ae0 = 761 B, called by ExecuteCommandResistSpell: wrong. 0x6e0380 fp= unique, `006e0395 MOV EDX,[ECX+0xd8]`, `006e03ec CMP ..,0x2af8`; returns 1 for anim ids 0x271f-0x2724, 0x274d, 0x2756/7, 0x28a1, 0x2af8 |
| 5 | 0x65c900 | ResolveRangedAnimations | 0x6e0600 | label_suspect | fp= unique; `006e0625 MOV [EAX+0x14],CX`, `006e0645 MOV [EAX+0x10],DX` on GetAttack; callers 0x6dc290 (real ResolveRangedAnimations) + 0x6e0410 (ResolveMeleeAnimations) |
| 6 | 0x65ef50 | ResolveRangedMissToParry | 0x6e2c50 | label_suspect | fp= unique; `006e2c71 CALL 0x6d0620` GetItemInSlot(0x10/0x20), `006e2c82 CALL 0x615c80` IsLightsaber; Mac IsLightsaberEquipped (disp 0x1150, imm 0x10,0x20) |
| 7 | 0x65fd40 | DoCombatStep | 0x6e3a40 | verified | fp= 3044 B; `006e3d54 CALL 0x56e1e0` RangeWeaponEquipped, `006e3ccb CALL 0x5716a0` DesiredAttackRange; Mac disp 0x1b0,0x380,0x1114, imm 0x7f000000 |
| 8 | 0x67c0c0 | Destructor | **0x563490** | verified | 0x563390 is GetPortrait (100 B, 1 callee). 0x563490 fp= 44 B; `0056349a CALL 0x5634c0` (fp= GOG ~CSWSCreature 0x67c0f0, 1913 B), `005634ab CALL 0x9196fd` delete |
| 9 | 0x680a70 | AddDropItemActions | 0x567e40 | label_suspect | fp= unique; `00567ee0 PUSH 0x7` + `CALL 0x53f7f0` AddAction; caller ExecuteCommandPickUpItem `0068dae8`; Mac AddPickUpItemActions disp 0x8,0xec, imm 3,3,7 |
| 10 | 0x680b30 | AddHealActions | 0x567f00 | label_suspect | fp= unique; `00568005 PUSH 0x9`, `00567f54 [EAX+0x520]`, `00567f95 PUSH 0xc2`; caller ExecuteCommandPutDownItem; Mac AddDropItemActions disp 0x520,0x1150, imm 9,0xc2 |
| 11 | 0x68a4b0 | ResolveInitiative | 0x571980 | label_suspect | fp= unique 28 B; `00571987 CMP [EBP+8],0xa` -> `arg<10`; callers ComputeAIStateOnAction, 0x563c90. The Mac ResolveInitiative shape is row 16 |
| 12 | 0x68ddf0 | RemoveMindTrickEffects | **0x5752c0** | label_suspect | 0x575340 = 161 B inner init (no GOG fp=). 0x5752c0 fp= 120 B; zeroes +4..+0x24, `0057532c CALL 0x575340`; callers LoadFollowInfo 0x575200, 0x58a330; kDB = CFollowInfo::CFollowInfo (high) |
| 13 | 0x68f7d0 | GetBlind | 0x576ca0 | label_suspect | fp= unique 277 B RET 8; loops action list, `00576ce0 MOVZX EDX,[ECX+0x6c]` == arg, `00576d31 MOV EAX,[EDX+0x74]` vcall, frees the node; Mac GetBlind is 39 B on +0xff4. Possibly Mac RemoveAction (vt 0x78) - name not proven |
| 14 | 0x692510 | SetAmbientAnimationState | 0x5799e0 | label_suspect | fp= unique 44 B; `005799ed MOV [EAX+0x578],ECX`, `005799fc MOV [EDX+0x57c],0x5dc`; sole caller AIUpdate 0x569320. Mac inlines 0x578/0x57c/0x5dc into AIUpdate; Mac SetAmbientAnimationState = +0x504 via 0xd8 (0xe,0xfd). Reviewer `match` overruled |
| 15 | 0x692f53 | BroadcastStealthDetectionData | 0x57a420 | rejected | GOG address is mid-function (0x692f50+3). Steam 0x57a420 = whole body: `0057a538 PUSH 0x1b`, caller OnApplyPoison 005a037a = Mac BroadcastPoisoned (imm 0x1b); kDB already BroadcastPoisoned |
| 16 | 0x693490 | GetDetectMode | 0x57a960 | label_suspect | fp= unique 43 B; `0057a96c MOV ECX,[EAX+0x11ac]`, `0057a981 MOV [EAX+0x51c],EDX` (SETZ); Mac ResolveInitiative 27 B disp 0x51c,0x11ac; caller AddAttackActions (Mac has 0x51c) |
| 17 | 0x693ac0 | AddPickUpItemActions | 0x57af90 | label_suspect | fp= unique; `0057afa5 PUSH 0x2` GetItemInSlot, GetBaseItem, `0057afd0 MOV AL,[EDX+0x8a]` -> `0057aff8 MOV [EDX+0xfa],AL`; callers EquipItem/UnequipItem; Mac ComputeArmourClass |
| 18 | 0x696db0 | SetLastInstantEquip | 0x57e280 | label_suspect | fp= unique; `0057e306 CMP EAX,0x12`, `0057e315 CMP EAX,0x13`, `0057e333 CALL 0x544790` RemoveEffectById over +0x148/+0x14c; Mac RemoveMindTrickEffects; kDB RemoveMindTrickAndScrambleEffects (high) stays |
| 19 | 0x67be30 | GetLockOrientationToObject | 0x563200 | verified | sample; fp= unique; `0056320a MOV EAX,[EAX+0x104c]`; Mac Get/Set on 0x104c |
| 20 | 0x67bf80 | GetEffectSpellId | 0x563350 | verified | sample; fp= unique; `0056335a MOV EAX,[EAX+0xff8]`; Mac Get/Set on 0xff8 |
| 21 | 0x690a00 | DoStealthDetection | 0x577ed0 | verified | sample; fp= unique; `00577f3e CALL 0x577fa0` (2223 B = GOG DoListenDetection 0x690ad0), `00577f57 CALL 0x578850` (2662 B = GOG DoSpotDetection 0x691380) |
| 22 | 0x65aa20 | GetWeaponHitShots | 0x6de720 | verified | sample; fp twin 0x6de660; `006de766 PUSH 0x99a94c` = "Hits" (twin `006de6a6 PUSH 0x99a968` = "Shots") |
| 23 | 0x68dce0 | SaveFollowInfo | 0x5751b0 | verified | sample; fp= unique; `005751c7 PUSH 0x99547c` "FollowInfo", `005751d7 CALL 0x719940` AddStructToStruct; [+0x500] as Mac |
| 24 | 0x681ef0 | ResetUpdateTimes | 0x5692c0 | verified | AUTO sig; disp 0xac,0xb0,0x390,0x394,0x3c8,0x3cc = Mac exactly |
| 25 | 0x65a5b0 | GetWeaponImpactTime | 0x6de2b0 | verified | AUTO sig; `006de304 MOV ECX,[EAX+0xec]` (rules+0x130 2DA); Mac disp 0xec,0x130; caller ResolveMeleeAttack |
| 26 | 0x692f00 | BroadcastDeflectBreakdown | 0x57a3d0 | verified | AUTO sig; `0057a3dd PUSH 0x1a`; Mac imm 0x1a; called from SignalDamage `006dda59` |
| 27 | 0x692420 | BroadcastMineDetectionData | 0x5798f0 | verified | AUTO sig; `005799b1 PUSH 0x13`; Mac imm 0x13 |
| 28 | 0x692dc0 | BroadcastThreatBreakdown | 0x57a290 | verified | AUTO sig; `0057a29d PUSH 0x15`; Mac imm 0x15; called from SignalDamage `006dd190` |
| 29 | 0x65c950 | SignalMeleeDamage | 0x6e0650 | verified | AUTO c2; fp= 1206 B; sole caller SignalDamage `006ddae2`; Mac SignalDamage calls SignalMeleeDamage |
| 30 | 0x689b60 | UpdateCombatRoundTimer | 0x571030 | verified | AUTO c2; fp= 952 B; `005711a8 CALL 0x6dcbe0` SignalDamage (Mac same callee); caller AIUpdate `00569578` |
| 31 | 0x67d2f0 | AddPathfindingWaitActionToFront | 0x5646c0 | verified | AUTO c2; `0056474f PUSH 0x1e` + `00564754 CALL 0x540ca0` AddActionToFront; Mac imm 0x1e |

## AUTO meaning checks

| row | GOG | name | Steam | result |
|---|---|---|---|---|
| sig | 0x681ef0 | ResetUpdateTimes | 0x5692c0 | pass |
| sig | 0x65a5b0 | GetWeaponImpactTime | 0x6de2b0 | pass |
| sig | 0x692f00 | BroadcastDeflectBreakdown | 0x57a3d0 | pass |
| sig | 0x692420 | BroadcastMineDetectionData | 0x5798f0 | pass |
| sig | 0x692dc0 | BroadcastThreatBreakdown | 0x57a290 | pass |
| c2 | 0x65c950 | SignalMeleeDamage | 0x6e0650 | pass |
| c2 | 0x689b60 | UpdateCombatRoundTimer | 0x571030 | pass |
| c2 | 0x67d2f0 | AddPathfindingWaitActionToFront | 0x5646c0 | pass |

## Reviewer calibration

- Full-read rows where I disagree with the reviewer:
  - Row 2 and row 6: reviewer `match` on mac_contra rows. The names are wrong: the bodies are Mac ResolveRangedHit and Mac IsLightsaberEquipped.
  - Row 14: reviewer `match`. The Mac setter writes a different field.
  - Row 15: reviewer label_suspect. The GOG address is a fragment, so I rejected it.
- The reviewer's corrected addresses were right in every case: 0x6df2a0, 0x6e0380, 0x6e0600, 0x6e3a40, 0x563490, 0x5752c0, 0x57e280.

sample_errors: 0
