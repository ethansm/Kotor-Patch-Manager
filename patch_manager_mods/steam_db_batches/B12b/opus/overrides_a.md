# Opus gate B12b part a: evidence

Method: GOG/Steam/Mac fp.tsv rows (size, retn, n_insn, disp, imm, callees), caller sets matched by caller size, Steam asm
quoted from `steam_6a522e71/decomp`. Mac +/-4 sweep run with Mac neighbours of the fitted symbol and of the GOG name.
"fp 1:1" = size, retn, n_insn, disp and imm all identical between GOG and Steam.

## 100% rows

1. **0x698df0 UpdateAppearanceForEquippedItems_2 -> 0x5802c0: label_suspect.** The fp is 1:1 (62b, disp 0xd8/0x510, imm
   0x2710). The 6 callers have the same sizes (UpdateMineCheck, DoPerceptionUpdateOnCreature, DoListen/SpotDetection, ...).
   Body: `005802d5 CMP [EBP-4],0x2710`, `005802e1 MOVZX EAX,[EDX+0x510]`, `005802ed MOV AL,0x2`. It fits Mac
   `CSWSCreature::GetDetectMode` (disp 0xd8/0x510, imm 2). The real UpdateAppearanceForEquippedItems is GOG 0x698430
   (GetItemInSlot, +0x1150). The GOG label `GetDetectMode` at 0x693490 (0x51c/0x11ac) is itself on a wrong body.
   Mac sweep: Mac neighbours (SplitItem, GetClientCreature) are not GOG neighbours, so the Mac order does not follow the
   GOG order and there is no simple rotation.
2. **0x69df00 BroadcastAttackOfOpportunity -> 0x585420: label_suspect.** The fp is 1:1. Body:
   `0058542c MOV CL,[EAX+0xff4]`, then the bool `(f&0x10) || ((f&8) && !(f&2))`. It fits Mac `GetBlind` (0xff4, imm
   8/0xa/0x10). Mac sweep: Mac GetBlind is followed by GetInvisible, and so is GOG 0x69df00 (0x69df60 GetInvisible).
   This confirms GetBlind. Mac BroadcastAttackOfOpportunity is a 5-byte empty stub. The GOG `GetBlind` label 0x68f7d0
   is list code and wrong.
3. **0x6a2600 UpdatePureGoodEvilPowers -> 0x589b20: label_suspect** (the reviewer said match, "name unverified"). The fp
   is 1:1 and the only caller is UpdateActionQueue (2856b both). Body: it loops over combat round entries with Steam
   0x591510/0x591450 (GOG 0x775b00/0x775bc0), `00589bac CALL 0x5d7220` (GetAQActionIDByID), and stores into
   +0x3e8/+0x3ec stride 0x1c. Mac UpdatePureGoodEvilPowers is 92b with disp 0x18a/0x1198 imm 0x64, which contradicts
   this body. In Mac this helper is inlined into UpdateActionQueue, so there is no Mac name. The likely real
   UpdatePureGoodEvilPowers is GOG `FUN_006a42d0` (130b, 0x18a/0x1198, imm 0x64).
4. **0x6a26d0 ComputeArmourClass -> 0x589bf0: label_suspect.** The fp is 1:1. Body: `00589c08 CALL 0x590180`
   (GetSpellActionPending) and `00589c21 CALL 0x591a70` (RemoveSpellAction) on +0x10dc. It fits Mac
   `CSWSCreature::RemoveSpellActionFromRound` (47b, disp 0x10dc x2, exactly those 2 callees). Mac ComputeArmourClass
   has disp 0x8a/0xfa/0x1150/0x1198. Only GOG 0x693ac0 (labelled AddPickUpItemActions, 116b) has exactly that disp set,
   so it is the likely ComputeArmourClass home.
5. **0x6a2e10 SetInParty -> CORRECTION 0x58a330: verified.** 0x58a610 is the counterpart of GOG 0x6a30f0
   GetSpikeDetails (810b, identical fp), so it is wrong. 0x58a330 has size 318, RET 0x8 at `0058a46b`, and a 1:1 fp.
   Its callees map by identical size: `0058a3ef CALL 0x51c280` = GetServerAIMaster (31b), `0058a3f6 CALL 0x51e120` =
   SetAILevel (71b RET 8), `0058a3a1 CALL 0x5752c0` = RemoveMindTrickEffects (120b), plus `0058a35e MOV
   [EAX+0x11ac],ECX` and +0x500. Mac SetInParty (0x78/0x500/0x11ac, imm 2/4/0x3c) fits. The reviewer was right.
6. **0x6a3e00 HasEffect -> 0x58b320: label_suspect.** The fp is 1:1, RET 8. Body: `if (k==0x10) [+0x11d4]=v; else if
   (k==0x20) [+0x11d0]=v` (`0058b343`, `0058b351`). It fits Mac `SetLastInstantEquip` (38b, 0x11d0/0x11d4, imm
   0x10/0x20). Mac neighbour GetLastInstantEquip is next to InstantUseItem, which matches GOG 0x6a3dc0 / 0x6a3e40 around
   this row. The GOG `SetLastInstantEquip` label 0x696db0 (RemoveEffectById) is wrong. Mac HasEffect (0x148/0x14c) has
   no clear GOG home.
7. **0x6a4d30 UseLoreOnItem -> 0x58c290: label_suspect.** The fp is 1:1. The 5 callers match by size: GetNearestObjects
   2890, 0x6a4da0 97, LoadEvent 729, Encounter ctor 894, LoadFractionalCRData 267. Body: grow (`0058c2cb CALL
   0x58c4a0`, initial cap 0x10, x2), `0058c2de FSTP [ECX+EDX*4]`, size+1. That is `CExoArrayList<float>::Add` (Mac 75b).
   Mac UseLoreOnItem is a 7-byte stub.
8. **0x6a4da0 LearnScroll -> 0x58c300: label_suspect.** The fp is 1:1, RET 8. The callers match: GetNearestObjects
   2890, ExecuteCommandGetNearestObject 4828. Body: `0058c31d CALL 0x58c290` (Add), a float shift loop at `0058c33a`,
   then a store at the index (`0058c357`). That is `CExoArrayList<float>::Insert`. Mac LearnScroll is a 7-byte stub.
9. **Rows 9-12, the four 286-byte QuickWalk_FollowLeader siblings: verified (corrections).** All 4 GOG bodies and all 4
   Steam bodies share size 286, RET 0x10, disp {4,4,4,8,0x24,0x34,0xfa4} and 3 callees (operator-, Dot,
   GetPathData). Only the stored constant differs. The table below checks each mapping independently on all three
   binaries.

| GOG | GOG name | GOG imm extra | Mac symbol imm extra | Steam | Steam store |
|---|---|---|---|---|---|
| 0x6d5a90 | OffsetWaitDest | none (const 1) | none | 0x5cacd0 | `005cada2 MOV [ECX+4],0x1` |
| 0x6d5bc0 | PathWaitDest | 0x3,0x3 | 0x3,0x3 | 0x5cae00 | `005caed2 MOV [ECX+4],0x3` |
| 0x6d5cf0 | ToOffsetWaitDest | 0x4,0x4 | 0x4,0x4 | 0x5caf30 | `005cb002 MOV [ECX+4],0x4` |
| 0x6d5e20 | ToPathWaitDest | 0x2,0x2 | 0x2,0x2 | 0x5cb060 | `005cb132 MOV [ECX+4],0x2` |

   The Steam imm sets are byte-identical to the GOG sets row for row, and Mac names agree with the constants. The
   address order is also preserved on all three binaries. There is no permutation.
10. **0x6d5f50 TeleportForward -> 0x5cb190: verified (correction).** The fp is 1:1 (412b, RET 0x10, disp 0x380). Anchors:
    `005cb277 CALL 0x5453c0` (GetArea), `005cb27e CALL 0x54b650` (TestSafeLocationPoint), `005cb263 [EAX+0x380]`.
11. **0x6dc300 GetCreatureReputation_2 -> 0x5d16e0: label_suspect.** The fp is 1:1 and the only caller is
    AddToVisibleList (269b RET 0x14 both). Body: it scans the +0x1028 list up to the +0x102c count and returns 1 if
    `*elem == id` (`005d16ec`, `005d1712`). That is a visible-list "contains" check. Mac has no symbol for it (inlined
    into AddToVisibleList). The real GetCreatureReputation is GOG 0x6db920 (238b, GetNPCFactionReputation callee,
    matching Mac 139b 0x78/0x1198/0x11ac). The `_2` is a duplicate label.

## Sample rows

- **0x6dfcc0 HasAction -> 0x5d50a0: verified.** The fp is 1:1. The callers match: QuickWalk 8036,
  RotateCreatureToOrientation 3515, ProcessPoint 4688. Anchor: `005d50ac ADD ECX,0x100`, then an id match returns 1.
  Mac HasAction (disp 0x100 x3) fits. Reviewer correct.
- **0x6c66e0 AIActionCheckInterAreaPathfinding -> 0x5bb920: label_suspect (REVIEWER ERROR on label).** The counterpart
  is right (fp 1:1, 581b). The GOG callees are AddMoveToPointActionToFront, AddPathfindingWaitActionToFront,
  AddActionToFront, GetArea and ComputeAwayVector, and imm 0x2c/0x38-0x4c. That is exactly Mac
  `AIActionCheckMoveAwayFromLocation`. Mac InterAreaPathfinding (GetModule, SetIntraAreaGoal, new[]/delete[], disp
  0x5c/0x60/0x64/0x70, imm 0x1e) matches GOG **0x6c6930** (labelled AIActionChangeFacingPoint). Mac ChangeFacingPoint
  (normalize + SetOrientation, 0x94/0x9c/0x11ac) matches GOG 0x6c6230 (labelled MoveAwayFromLocation). This is a
  **3-label rotation** across 0x6c6230/0x6c66e0/0x6c6930. GOG 0x6c6360 (labelled MoveToPointRadius: GetObjectArray +
  ComputeAwayVector) looks like Mac AIActionCheckMoveAwayFromObject. The rotation affects the B12b rows 0x6c6230 and
  0x6c6930 (cards 01/26, not in this part).
- **0x6dc370 GetVisibleListElement -> 0x5d1750: verified.** The fp is 1:1, and all 26 callers match by size
  (OnPanelAdded 0x894320, GetArmorClassVersus 0x6a5110, ...). Anchor: `005d17b5 MOV EAX,[EDX+EAX*4]` returns the
  +0x1028 element. Reviewer correct.
- **0x6dba10 AdjustReputation -> 0x5d0df0: verified.** The fp is 1:1 (178b RET 8). Anchor: `005d0e97 CALL 0x6644d0`
  (SetNPCFactionReputation), plus Get/GetIsNPCFaction. These are the Mac AdjustReputation callees. The mac_rotation flag
  (0x6db8d0) is a false alarm: 0x6db8d0 is GetFaction (78b) and matches Mac GetFaction. Reviewer correct.
- **0x698350 CalculateDamagePower -> 0x57f820: verified.** The fp is 1:1 and the only caller is GetDamageRoll (7065b
  both). Anchor: `0057f85a CALL 0x582010`, PUSH 0xff. Mac imm 0xff,0xff. Reviewer correct.

`sample_errors: 1` (0x6c66e0: the reviewer said `match`, the counterpart is correct but the name is wrong via label
rotation; no address error in the sample.)

## AUTO meaning checks

All 8: GOG and Steam fp identical, caller-size multiset identical, and the GOG named callees equal the Mac callees of the
same-named symbol.

| row | GOG -> Steam | check | result |
|---|---|---|---|
| sig AIActionUnequipItem | 0x6db6a0 -> 0x5d0a80 | RunUnequip, UnequipCancel, ActivityManager = Mac; RET 4 | pass |
| sig UpdatePersonalSpace | 0x698810 -> 0x57fce0 | GetFLOATEntry + ComputeStepTolerance = Mac; 4 callers | pass |
| sig CutWaypointPath | 0x69db00 -> 0x585020 | disps 0x8c/0x9c/0xf8-0x104 = Mac; callers 1634/1874 | pass |
| sig CheckItemAlignmentRestrictions | 0x6de0a0 -> 0x5d3480 | HasFeat, GetPassiveProperty; 0x18a/0x28c/0x2d4 = Mac | pass |
| sig CleanInvitationLists | 0x6df450 -> 0x5d4830 | 8/8 named callees = Mac (SetInvitedToParty, ...) | pass |
| c2 UnsummonMyself | 0x6e0170 -> 0x5d5550 | ClearAllActions, AddEventDeltaTime, imm 0x8c = Mac | pass |
| c2 CanEquipItem | 0x6c8460 -> 0x5bd6a0 | GetMinEquipLevel, CanEquipWeapon(2) = Mac; RET 0x14 | pass |
| c2 IsPartyVisible | 0x6a3420 -> 0x58a940 | GetCreatureByGameObjectID; 0x1028/0x11ac = Mac | pass |

## Systematic

The GOG CSWSCreature labels in this range are unreliable beyond these rows:
- `_2` duplicates sit on unrelated bodies (rows 1, 14).
- The AI-action cluster at 0x6c6230-0x6c6930 is rotated by one.
- These GOG labels are on wrong bodies: GetBlind (0x68f7d0), GetDetectMode (0x693490), SetLastInstantEquip (0x696db0)
  and AddPickUpItemActions (0x693ac0, which looks like ComputeArmourClass).

The Steam counterparts are positionally exact in every case except SetInParty.
