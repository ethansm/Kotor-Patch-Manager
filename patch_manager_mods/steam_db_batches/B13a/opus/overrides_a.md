# Opus gate B13a part a: evidence (45 rows)

Method: GOG/Steam/Mac x86 `fp.tsv` rows (size, retn, n_insn, disp, imm, callees), a size-sequence alignment of the
whole class run (GOG 0x410200-0x42c200 vs Steam 0x764540-0x780200), caller multisets built from the fp callee
columns on both Windows builds, and Steam asm quoted from `steam_6a522e71/decomp`. "fp 1:1" = size, retn, n_insn,
disp and imm all identical between GOG and Steam. Mac vtable offsets are +4 vs Windows (Mac 0x50 = Win 0x4c).

Verdicts: verified 30, label_suspect 13, unresolved 2. Address changes vs the pre-Opus final_rows: 8 (0x410e50,
0x420070, 0x4200d0, 0x421820/40/60/80, 0x426490). 0x426430/0x426450 already carry the recheck correction, which is
confirmed. 0x41f730 moves from 0x7737e0 (shared with 0x41f6b0) to its positional 0x773860, still unresolved.

## Systematic finding: this class has a label rotation, not an address shift

Every B13a Steam counterpart is positionally exact (the alignment is 1:1 apart from 6 GOG-only CRT/operator stubs,
GOG 0x41f3d0 UpdateMonkVisuals with no Steam body, GOG 0x4213a0, one Steam insert 0x77b630, and GetBehaviors which
grew 813 -> 1206 B). The errors are GOG labels on the wrong bodies, chained across the class:

| GOG body | GOG label | body is (Mac) | that label's body is at |
|---|---|---|---|
| 0x413620 | StripEquippedItems | SetDesiredOrientationForLock | 0x41f5a0 |
| 0x41f5a0 | SetDesiredOrientationForLock | StripEquippedItems | 0x413620 |
| 0x4137c0 | ClearVisibilityList | UnloadMonkVisuals | 0x41f840 (AUTO, part z) |
| 0x413e30 | ForceAppearanceUpdate | ResetAnimationState (part z) | 0x420720 (AUTO) |
| 0x413e70 | UpdateRollingFootstepSound | ComputeWeaponOverlays | 0x42bb30 |
| 0x41ac10 | UpdateRangedAttackData | GetLastInstant (no Mac sym) | 0x41b7b0 |
| 0x41ade0 | GetLastInstant | SetItemUnEquippedByMe (AUTO, part z) | 0x41ac10 |
| 0x41bd70 | SetItemUnEquippedByMe | InCreatureCombat (AUTO, part z) | 0x41ade0 |
| 0x422e00 | InCreatureCombat | IsDriveAnimationBlocked | 0x41bd70 |
| 0x422dd0 | IsDriveAnimationBlocked | SetSWAnimationSpeed | 0x422e00 |
| 0x42bd00 | SetSWAnimationSpeed | GetLightSaberThrowRate | 0x422dd0 |
| 0x4213c0 | SetThrownLightsaber | UpdateAreaId | 0x4213a0 (no Steam fp twin) |
| 0x41f7b0 | ResetDesiredPositionTimer | GetPath callback | 0x414410 (label Unblur) |
| 0x42bae0 | IsInjured | StopRollingFootstepSound (no Mac sym) | 0x421970 (label HasFeat) |
| 0x4248c0 | GetEquipmentForcePowers | 4-arg compare helper | 0x424b60 (`_2`) |
| 0x41ae50 | BarkBubble | attack-index -> item helper | not found |

The thunk cluster and the head-turn getters are the opposite case: GOG labels right, gate picks rotated by one slot.

## 100% rows

1. **0x410e50 `scalar -> 0x765180 verified (correction).** fp 1:1 (44 B, RET 4, 17 insns). `0076518a CALL 0x7651b0`
   (= GOG callee 0x410e80 ~CSWCCreature, 1931 B both), `0076519b CALL 0x9196fd` (operator delete), `007651a9 RET 0x4`.
   The gate pick 0x765110 is the 61 B GetName body (+0x310 CExoString copy). Reviewer correction confirmed.
2. **0x413620 StripEquippedItems -> 0x7678f0 label_suspect.** fp 1:1 (248 B). `0076791c MOV EAX,[EDX+0x30c]`
   (lock target), Vector-/Dot/normalize (`00767997 CALL 0x5143c0`), `007679b5 ADD ECX,0x1d8` (orientation). Sole caller
   AnimationUpdate, and Mac AnimationUpdate calls SetDesiredOrientationForLock (Mac disp 0x1d8/0x1e0/0x30c). Mac
   StripEquippedItems is an equip-slot loop calling SetItemUnEquippedByMe: that is GOG 0x41f5a0.
3. **0x4137c0 ClearVisibilityList -> 0x7679f0 label_suspect (reviewer match wrong, 100% row).** fp 1:1 (141 B).
   `007679fc CMP [EAX+0x178],0`, vcall +0x4c, `00767a55 MOV [ECX+0x178],0`, `00767a62 MOV [EDX+0x17c],0`. Mac
   UnloadMonkVisuals: disp 0x50 (= vt 0x4c)/0x178/0x17c/0x180, no callees. Mac ClearVisibilityList uses 0x378/0x37c,
   which is GOG 0x41f840 (labelled SetSpecialWalkAnimation_2, called by ~CSWCCreature).
4. **0x413e70 UpdateRollingFootstepSound -> 0x7680a0 label_suspect.** fp 1:1 (516 B, RET 8). `007680d1 PUSH 0x272e`,
   8 vcalls at +0x4c, reads +0x2e8/+0x3b0/+0x138. GOG callers (9): ShowEquippedItems, HideEquippedItems,
   UpdateVisibleWeapons, SetCombatState, AddFireForgetAnimation, FlourishWeapons. Mac ComputeWeaponOverlays: disp 0x50
   x8, 0x138, 0x2e8, 0x3b0, and Mac ShowEquippedItems calls it. Mac UpdateRollingFootstepSound (0x448/0x450/0x454) =
   GOG 0x42bb30.
5. **0x41ac10 UpdateRangedAttackData -> 0x76eda0 label_suspect.** fp 1:1 (365 B, RET 4). `0076ee5a MOV EAX,[ECX+0x280]`
   ... `0076eefd MOV EAX,[EAX+0x2b8]`: one-arg slot-mask getter of the fields Mac SetLastInstant writes
   (0x280..0x2c0). Callers SwitchWeaponEvent and EquipToInventorySlot. No Mac GetLastInstant (inlined there); name
   fits GetLastInstant. Mac UpdateRangedAttackData (212 B, IsCreature callee) = GOG 0x41b7b0.
6. **0x41ae50 BarkBubble -> 0x76efe0 label_suspect.** fp 1:1 (120 B, RET 4). Switch 1..6: `0076f00d CALL 0x76e610`
   GetEquippedItemID with masks 0x10/0x20/0x4000/0x8000/0x10000, default `0076f04d MOV EAX,0x7f000000`. Sole caller
   SpawnHitVisuals. Mac BarkBubble is 76 B with two unrelated callees. No Mac name found.
7. **0x41f5a0 SetDesiredOrientationForLock -> 0x7736d0 label_suspect.** fp 1:1 (265 B). `0077370a CMP ECX,0x14` loop,
   `00773717 CALL 0x76e610`, `00773720 CALL 0x76ef70` (GOG 0x41ade0, whose body is SetItemUnEquippedByMe), CExoString
   ctor/dtor, `0077376b MOV EAX,[EDX+0x7c]` vcall. Mac StripEquippedItems: imm 0x14, CExoString, SetItemUnEquippedByMe,
   disp 0x80 (= vt 0x7c). Swap partner of row 2.
8. **0x41f6b0 UnloadMonkVisuals -> unresolved (position: 0x7737e0).** Twin rule. Steam 0x7737e0 and 0x773860 differ
   only in the column global (`[0x00a0fde8]` vs `[0x00a0fda0]`, set at `006e83b4` from 'PERSPACE' and `006e7e71` from
   'CREPERSPACE'), and RegisterCallbacks 0x77f240 registers them as `0077f4f7 PUSH 0x98dae0` 'GetPersonalRadius' /
   `0077f4fc PUSH 0x7737e0` and `0077f527 PUSH 0x98dacc` 'GetCreatureRadius' / `0077f52c PUSH 0x773860`. Mac has
   GetPersonalRadius, GetCreatureRadius, GetPath, SetSpecialWalkAnimation in that order, matching GOG
   0x41f6b0/0x41f730/0x41f7b0/0x41f7e0. But the GOG pair has identical fp, no callers, and no GOG asm here, so the GOG
   side has no content anchor. Re-gate from the GOG RegisterCallbacks 0x42b260 PUSH order. The GOG label is wrong
   either way (Mac UnloadMonkVisuals = GOG 0x4137c0).
9. **0x41f730 ResetAnimationState -> unresolved (position: 0x773860).** Same twin. The pre-Opus row put 0x7737e0 on both
   GOG rows; by position this one is 0x773860 (GetCreatureRadius). GOG label wrong (Mac ResetAnimationState = GOG
   0x413e30).
10. **0x420070 GetHeadTurnHorizontal -> 0x7741a0 verified (correction).** fp 1:1 (92 B). Caller multiset identical:
    GOG LookAt 1448 / WalkUpdateLocation_QuickWalk 4082 / UpdateShotCamera 8198 = Steam 0x773bf0 / 0x5c6340 / 0x7c41f0.
    Column `[0x00a0fdc4]`. The gate pick 0x774200 is the 93 B sibling with one caller.
11. **0x4200d0 GetHeadTurnVertical -> 0x774200 verified (correction).** fp 1:1 (93 B). Sole caller LookAt on both sides
    (Mac LookAt calls only GetHeadTurnVertical, 0x2865e2). `MOV EDX,[0x00a0fdc8]`. The gate pick 0x774260 is
    ApplyShadowBlob (`00774328 PUSH 0x99f1a4` Fx_Shadow).
12. **0x4213c0 SetThrownLightsaber -> 0x7754d0 label_suspect.** fp 1:1 (66 B). `007754e8 MOV EAX,[EDX+0x108]`,
    `007754f7 MOV [ECX+0x3b4],EDX`, no callees; callers SetArea 0x7754a0 and LoadArea 0x7a2fb0 (= GOG). Mac UpdateAreaId:
    37 B, disp 0x20/0x108/0x3b4, no callees. Mac SetThrownLightsaber (disp 0x3e0) = GOG 0x4213a0 (label SetEffectSpellId,
    caller ApplyLightsaberThrow), which has no Steam body at its run slot.
13. **0x421650 CreateNewSpellLikeAbilityList -> 0x775760 verified.** fp 1:1. `0077578a CALL 0x919723` (new, imm 0xc),
    `007757cb MOV [ECX+0x3b8],EDX`. Mac: disp 0x3b8, imm 0xc. The "layout difference" flag is noise.
14. **0x421750 SetInParty -> 0x775860 verified.** fp 1:1 (203 B). `007758af CALL 0x73fb90` GetSWParty, `00775881 MOV
    [EDX+0x3bc],EAX`, imm 0xfffffffd. Mac SetInParty: disp 0x3bc, imm 0xfffffffd. The mac_rotation flag is noise; label right.
15-18. **Thunk cluster 0x421820/40/60/80 -> 0x775930/50/70/90 verified (4 corrections).** All 25 B, `MOV ECX,[EAX+0x224]`
    + one CALL. The callees pair fp 1:1: GOG 0x48d2f0 (95 B, extra disp 0x18) = 0x85d260 (`00775940`); 0x48d360 (80 B) =
    0x85d2d0 (`00775960`); 0x48d3b0 (164 B, one helper) = 0x85d320 (`00775980`); 0x48d460 (79 B, 2DA float) = 0x85d3d0
    (`007759a0`). Mac CSWCCreatureAppearance shapes: GetCameraFollowPart (extra disp 0x1c), GetFollowingBody,
    GetHeadHeightOffset (calls a helper), GetCameraOffset (2DA). GOG labels right; the pre-Opus picks were each rotated by
    one or two slots. 0x421880's 3 callers (SetCameraFollow, UpdateCameraFollow, RestoreCamera) = Steam 0x7f3770/0x7f4610/0x7f7220.
19. **0x422dd0 IsDriveAnimationBlocked -> 0x776e10 label_suspect.** fp 1:1 (37 B, RET 8). `00776e1d FSTP [EAX+0x3cc]`,
    `00776e29 FSTP [ECX+0x3c8]`. Mac SetSWAnimationSpeed: 34 B, disp 0x3c8/0x3cc. Caller GOG 0x4f0950 Control.
20. **0x422e00 InCreatureCombat -> 0x776e40 label_suspect (reviewer match wrong, 100% row).** fp 1:1 (126 B).
    `00776e4c MOV ECX,[EAX+0x13c]`, `00776e71 CALL 0x51c8b0`, `00776e8c CALL 0x51cbe0`, `00776ea7 CALL 0x7ef780`.
    Callers DriveAnimationUpdate, SetPrecalculatedPosition, AIUpdate, ProcessInput, Control. Mac IsDriveAnimationBlocked:
    3 callees, disp 0x48/0x13c, called by DriveAnimationUpdate and SetPrecalculatedPosition. Mac InCreatureCombat (IsCreature
    callee, disp 0x30 = vt 0x2c) = GOG 0x41bd70.
21. **0x4248c0 GetEquipmentForcePowers -> 0x778900 label_suspect.** fp 1:1 (55 B, RET 0x10, no callees, no disp).
    `00778910 CMP EAX,[EBP+0xc]`, `0077891b CMP ECX,[EBP+0x10]`. Callers UpdateMeleeAttackData, UpdateRangedAttackData_2.
    Mac GetEquipmentForcePowers (1110 B, disp 0x104/0x13c/0x194-0x19c/0x228) = GOG 0x424b60 -> 0x778ba0.
22-24. **Implant thunks verified (3 corrections).** fp 1:1 (26 B, RET 8, CALL ActionSwapImplant 0x77a280). Constant
    anchors: `0077a477 PUSH 0x408` (Regen), `0077a497 PUSH 0x409` (Str), `0077a4d7 PUSH 0x40a` (End). GOG imm and Mac imm
    agree per name (Mac Regen 0x408, Str 0x409, Agi 0x40b, End 0x40a). GOG labels right.
25. **0x42bae0 IsInjured -> 0x77fac0 label_suspect.** fp 1:1 (77 B). `0077faca CMP [EAX+0x448],0`, `0077faea CALL
    0x70af80` (sound Stop), `0077faf2 MOV [EAX+0x450],0`. Sole caller GOG 0x42bb30 (real UpdateRollingFootstepSound,
    which also calls PlayRollingFootstepSound). Mac inlines this stop into UpdateRollingFootstepSound. CSWCCreature::IsInjured
    (Mac: GetServerCreature + CSWSCreature::IsInjured) = GOG 0x421970 (label HasFeat) -> Steam 0x775a80 (`00775a8a CALL
    0x77d800`, `00775a91 CALL 0x58bc50` = GOG 0x6a4730 fp 1:1, disp 0x98/0x9c/0x11ec).
26. **0x42bd00 SetSWAnimationSpeed -> 0x77fce0 label_suspect.** fp 1:1 (59 B). `0077fcec CALL 0x77d800` GetServerCreature,
    `0077fd03 FLD [EAX+0x11dc]`. Caller ApplyLightsaberThrow. Mac GetLightSaberThrowRate: disp 0x11dc, GetServerCreature.

## Sample rows (11)

27. **0x422e80 SetEngagedState -> 0x776ec0 verified.** fp 1:1 (25 B RET 4). `00776ecd MOV [EAX+0x13c],ECX`. Mac
    SetEngagedState disp 0x13c.
28. **0x41f7b0 ResetDesiredPositionTimer -> 0x7738e0 label_suspect. SAMPLE ERROR.** Counterpart right (fp 1:1, 34 B), name
    wrong. The body is a cdecl callback writing into its third argument: `007738e4 MOV EAX,[EBP+0x10]`, `007738ed MOV
    [ECX+0x4],0`, `007738f7 MOV [EDX+0x8],0`. RegisterCallbacks registers it as `0077f557 PUSH 0x98dac4` ('GetPath') /
    `0077f55c PUSH 0x7738e0`. Mac GetPath: 22 B, disp 0x4/0x8. Mac ResetDesiredPositionTimer is `this+0x1f0 = 0x200`:
    GOG 0x414410 (label Unblur, disp 0x1f0, imm 0x200) = Steam 0x7685f0 (`007685fa MOV [EAX+0x1f0],0x200`). The reviewer
    ruled `match` on "zeroes two fields, fits timer reset (weak anchor)".
29. **0x415cf0 GetCurrentWayPoint -> 0x769ed0 verified.** `00769eda MOV EAX,[EAX+0x1c8]`; Mac disp {0x1c8} only.
30. **0x4208e0 Hilite -> 0x774a10 verified.** `00774a33 CALL 0x861400` (GOG callee 0x4c92d0 CSWCAnimBase::Hilite, fp 1:1);
    `00774a3f RET 0x10`.
31. **0x426470 ActionMenuSwapImplantAgi -> 0x77a4b0 verified.** `0077a4b7 PUSH 0x40b`; GOG imm 0x40b; Mac Agi 0x40b.
32. **0x428620 GetHostileItems -> 0x77c6a0 verified.** `0077c6fe CALL 0x77c1b0` (entry builder without format string).
33. **0x428690 GetHostileItems_2 -> 0x77c710 verified.** `0077c76e CALL 0x77c340` (builder with '%s (%d)' = GOG 0x4282c0).
34. **0x420920 UnHilite -> 0x774a50 verified.** `00774a6b CALL 0x861480` (= GOG 0x4c9350 CSWCAnimBase::UnHilite).
35. **0x41ce10 GetArmourClass -> 0x770fa0 verified.** `00770fb6 MOV ECX,[ECX+0x224]`, `00770fbc CALL 0x4558a0` (return-0
    stub). Mac: CALL CSWCCreatureAppearance::GetArmourClass (7 B stub) on +0x224.
36. **0x41cdb0 DeleteLoopingCreatureSound -> 0x770f40 verified.** `00770f52 CALL 0x70af80`, `00770f8c MOV [ECX+0x184],0`;
    Mac disp 0x184 x3.
37. **0x429f10 GetTargetInterfaceTargetType -> 0x77df90 verified.** Twins separated by constants and callers: 0x77df90
    `0077e08b MOV [EBP-4],2` (imm {2,2,3,4}) vs 0x77e0c0 `0077e1bb MOV [EBP-4],3` ({2,3,3,4,4}); Mac
    GetTargetInterfaceTargetType imm {2,2,3,4}. Caller PopulateMenus (782 B) = Steam `00743551 CALL 0x77df90`; the twin's
    callers GetTargetActions/GetDefaultActions = Mac GetTargetActions -> GetInterfaceTargetType.

## AUTO meaning checks

| # | GOG | name | Steam | result | anchor |
|---|---|---|---|---|---|
| 38 | 0x420410 | TurnOffShadows | 0x774540 | pass | fp 1:1; +0xbc/+0x114 (On: +0xb8/+0x110); Mac keeps the order (Off 0xc0/0x118, On 0xbc/0x114) |
| 39 | 0x416c70 | GetWayPoints | 0x76ae50 | pass | disp {0x1c4 x2, 0x1c8, 0x1f4 x2} = Mac GetWayPoints |
| 40 | 0x423970 | FollowAnimationUpdate | 0x7779b0 | pass | calls GetPauseReadyAnimation + RotateCreatureToOrientation + GetServerCreature = Mac |
| 41 | 0x428c00 | PartyMenuUseItem | 0x77cc80 | pass | `0077cc95 AND EDX,0xbfffffff`, `0077ccc3 CALL 0x775600` PerformUseItem = Mac |
| 42 | 0x4247a0 | CheckFeat | 0x7787e0 | pass | +0x310/+0x1198, HasFeat callee = Mac |
| 43 | 0x4244d0 | CanSee | 0x778510 | pass | `007786a5 CALL 0x7783b0` SceneHitCheck, `00778714` +0x31c = Mac |
| 44 | 0x41dbb0 | PlaySwingSound | 0x771d40 | pass | GetEquippedItemID callee, +0x64/+0x8c = Mac |
| 45 | 0x41a480 | GetEquippedItemID | 0x76e610 | pass | one-arg getter of +0x228..+0x268 (RET 4); Mac setter has the same offsets |

AUTO meaning checks: 8 of 8 pass.

sample_errors: 1 (row 28, 0x41f7b0: right address, wrong name). The batch escalates; the other reviewer `match` rows are in `overrides_z`.

Not counted (100% rows, reviewer `match` on a wrong name): 0x4137c0, 0x422e00. All reviewer address calls on real cards were right.
