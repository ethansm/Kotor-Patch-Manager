# B12a Opus gate part r (REJECT/NOCAND rows)

Method: private whole-binary fp match (size, retn, n_insn, callee count, disp/imm multiset Jaccard) of each GOG body
against all 26,966 Steam fp.tsv rows. Every row below has an exact Steam fp twin (score 8.00/8, runner-up <= 7.54),
then confirmed by Steam asm and by Mac x86 symbol fp (imm/disp values). Floats were read read-only from
`backups/vanilla_clean/swkotor2.exe` (bytes at 0x84a170 match the dump).

## Combat block offset
GOG 0x6571b0..0x663170 -> Steam +0x83d00 (+0x83d50 for ResolveRangedAttack). The gate's 0x54xxxx picks (class-run
delta -0x118b30) are wrong for this block.

## ApplyOnHit* label rotation (systematic)
Anchor: the on-hit dispatch in ResolveOnHitEffect (Steam 0x6e2fa0, `006e3191 JMP [ECX*4+0x6e361c]`) stores one
handler per subtype; the case order equals the Mac declaration order (Sleep, Stun, Hold, Confusion, Fear, Slow,
AbilityDamage, Poison, SlayRacialGroup, SlayAlignmentGroup, InstantDeath, Knockdown) and each body's imm set matches
its Mac namesake.

| case | Steam (asm line) | GOG addr | GOG label | body = |
|---|---|---|---|---|
| 0 | 0x6e5860 (006e31ae) | 0x661b60 | ApplyOnHitPoison | ApplyOnHitSleep (PUSH 0x5e) |
| 1 | 0x6e5d60 (006e31d0) | 0x662060 | ApplyOnHitSlayRacialGroup | ApplyOnHitStun (PUSH 0x1d, 0xd0) |
| 2 | 0x6e5650 (006e31f2) | 0x661950 | ApplyOnHitSlow | ApplyOnHitHold (PUSH 0x5, 0xe8) |
| 3 | 0x6e5310 (006e3214) | 0x661610 | ApplyOnHitConfusion | same (PUSH 0x11, 0xd0) |
| 4 | 0x6e54b0 (006e3236) | 0x6617b0 | ApplyOnHitFear | same (PUSH 0x6, 0xda) |
| 5 | 0x6e5b10 (006e3258) | 0x661e10 | FUN_ | ApplyOnHitSlow (imm 0x5f) |
| 6 | 0x6e5f00 (006e327a) | 0x662200 | ApplyOnHitAbilityDamage | same |
| 7 | 0x6e62d0 (006e329c) | 0x6625d0 | ApplyOnHitInstantDeath | ApplyOnHitPoison (PUSH 0x17) |
| 8 | 0x6e6390 (006e32b4) | 0x662690 | FUN_ | ApplyOnHitSlayRacialGroup (disp 0xe0, 0x1198) |
| 9 | 0x42a470 (006e32cc) | - | - | SlayAlignmentGroup stub (Mac 5 bytes) |
| 10 | 0x6e6600 (006e32dc) | 0x662900 | ApplyOnHitKnockdown | ApplyOnHitInstantDeath (SetInt(0,0),(1,1)) |
| 11 | 0x6e6730 (006e32fb) | 0x662a30 | FUN_ | ApplyOnHitKnockdown (float 0.1 @0x9876fc = Mac imm 0x3dcccccd) |

Also: GOG 0x6614d0 "ApplyOnHitHold" (Steam 0x6e51d0, KDB name_map ApplyOnHitHold/medium) is NOT an on-hit handler:
it is the 3-arg CreateLinkedEffect helper every handler calls (`FUN_006e51d0(eff, DAT_00995f94, 0xcf)`); Mac has two
CreateLinkedEffect overloads (0x37807e, 0x37817c). Not in this part; flag for part a / KDB fix.
GOG 0x662b80 DebugValidatePath = GetFirstAttacker (Steam 0x6e6880), as B9 found.

## Per row

1. CanUseForcePowerOrSpell 0x494d00 -> **0x84a170 label_suspect**. Exact fp; neighbours also twin (Contains
   0x84a1d0 96/4, GetPortrait 0x84a230 939/8). Body loops `byte [this+0x32]` classes, `LEA ECX,[EDX+ECX*1+0xdc]`
   (stride 0x20), calls 0x849560 = fp twin of GOG CSWCCreatureStats_ClassInfo::HasKnownSpell 0x4940f0. Fits Mac
   CSWCCreatureStats::HasSpell 0x185508 (disp 0x32,0xdc; imm 5,8). Client stats method, not CSWSCreature.
2. ResolveRangedAttack 0x6571b0 -> **0x6daf00 verified**. `006daf8e MOV CL,[EAX+0xac4]`, `006db406 RET 0x1c`.
3. ResolveMeleeAnimations 0x65c710 -> **0x6e0410 verified**. `006e051a MOV [EDX+0xa98],EAX`,
   `006e057f MOV EAX,[EDX+0xb08]`, `006e05f0 RET 0x14`.
4. CalculateMaxElementalDamage 0x65da20 -> **0x6e1720 verified**. `[param_2+0xfc]` guard, `006e176f CALL 0x00582010`
   with (2,...,0xff,0xff) = Mac imm 2,0xff,0xff disp 0xfc; `RET 0x8`.
5. ResolveOnHitEffect 0x65f2a0 -> **0x6e2fa0 verified**. `006e30ad MOV EDX,[ECX+0x28c]`, `006e310d AND EDX,[EAX+0x2d4]`,
   the 12-case dispatch above, `006e3616 RET 0xc`.
6. SavingThrowRoll 0x660930 -> **0x6e4630 verified**. `006e4d03 MOV byte [EDX+0x14c],1`, `006e480d MOV DX,[ECX+0x18a]`,
   `006e4fac RET 0x1c`; called from ResolveOnHitEffect `006e3567` (Mac has the same edge 0x375c7c -> 0x3719c2).
7. ApplyOnHitConfusion 0x661610 -> **0x6e5310 verified** (case 3, PUSH 0x11/0xd0).
8. ApplyOnHitFear 0x6617b0 -> **0x6e54b0 verified** (case 4, PUSH 0x6/0xda).
9. ApplyOnHitSlow 0x661950 -> **0x6e5650 label_suspect** = ApplyOnHitHold. Slow is GOG 0x661e10 -> 0x6e5b10.
10. ApplyOnHitPoison 0x661b60 -> **0x6e5860 label_suspect** = ApplyOnHitSleep. Poison is GOG 0x6625d0 -> 0x6e62d0.
11. ApplyOnHitSlayRacialGroup 0x662060 -> **0x6e5d60 label_suspect** = ApplyOnHitStun. SlayRacialGroup is GOG
    0x662690 -> 0x6e6390.
12. ApplyOnHitInstantDeath 0x6625d0 -> **0x6e62d0 label_suspect** = ApplyOnHitPoison. Gate pick 0x6e6880 is
    GetFirstAttacker. InstantDeath is GOG 0x662900 -> 0x6e6600.
13. ApplyOnHitKnockdown 0x662900 -> **0x6e6600 label_suspect** = ApplyOnHitInstantDeath. Knockdown is GOG 0x662a30 ->
    0x6e6730.
14. SetAttackSummaryFeedbackEffectData 0x663170 -> **0x6e6e70 verified**. `006e6f24 MOV dword [EAX+0x150],1`,
    `006e6f00 MOV EAX,[EDX+0x1198]`, `006e718d RET 0x8`; disp set = Mac (0x145, 0x150, 0x1198 x16). Gate pick
    0x6e71d0 is CTwoDimArrays::Constructor.
15. GetNearestEnemy 0x6954c0 -> **0x57c990 verified**. Gate pick was right; G6 conflict only. `0057cd62 MOV ECX,
    [EAX+0x380]`, `0057cc1d CMP [EAX+0x11ac],0`, OBJECT_INVALID `0057c9b3 MOV EAX,0x7f000000`. Mac has no
    GetNearestVisibleEnemyInRange; its GetNearestEnemy (0x3c01f2) is the only CSWSCreature fn with 0x380 x4 + 0x11ac x2.
    KDB name is a descriptive alias: rename to GetNearestEnemy (keep alias in notes).
16. SetLastInstantEquip 0x696db0 -> **0x57e280 label_suspect** = RemoveMindTrickEffects (Mac 0x3b7866: imm 0x12,0x13,
    disp 0x148/0x14c; KDB RemoveMindTrickAndScrambleEffects). Mac SetLastInstantEquip is a 38-byte setter on
    +0x11d0/+0x11d4: real one = GOG 0x6a3e00 (labelled HasEffect, B12b range) -> Steam 0x58b320 (`if(p==0x10)
    [this+0x11d4]=v; else if(p==0x20) [this+0x11d0]=v`, RET 8). Reviewer row agrees (label_suspect, 0x57e280).

## Counts
verified 8, label_suspect 8, rejected 0, unresolved 0.

sample_errors: 0
