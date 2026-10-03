# Opus gate B12b part z: evidence

Method: GOG/Steam/Mac x86 `fp.tsv` rows (size, retn, n_insn, disp, imm, callees), caller-size multisets on both Windows
builds, Steam callees named by size-matching against the GOG callees, Steam asm quoted from `steam_6a522e71/decomp`.
"fp 1:1" = size, retn, n_insn, disp and imm all identical between GOG and Steam. Every one of the 16 rows below is
fp 1:1 with an equal caller-size multiset, so every Steam counterpart is right; the question per row is the name.

## 100% rows (mac_rotation)

The AIAction cluster GOG 0x6c6080-0x6c7b80 (Steam 0x5bb2c0-0x5bcdc0, every size identical, delta -0x10adc0) has the
Mac x86 symbol order exactly, but 5 GOG labels are on the wrong slot:

| slot | GOG | GOG label | GOG body (callees / disp / imm) | Mac symbol it fits |
|---|---|---|---|---|
| 1 | 0x6c6080 | ChangeFacingObject | GetObjectArray, GetGameObject, GetArea, normalize, SetOrientation | ChangeFacingObject (same callees) |
| 2 | 0x6c6230 | **MoveAwayFromLocation** | normalize + SetOrientation only, 0x94/0x9c/0x11ac | **ChangeFacingPoint** (194 B, normalize + SetOrientation) |
| 3 | 0x6c6360 | **MoveToPointRadius** | GetObjectArray, linked-list GetHead/GetNext, GOG 0x68f7d0 (= CancelAction), ComputeAwayVector, imm 0x100/0x7f000000 | **MoveAwayFromObject** (CancelAction, GetNext, ComputeAwayVector) |
| 4 | 0x6c66e0 | **InterAreaPathfinding** | ComputeAwayVector, MoveToPointFront, PathWaitFront | **MoveAwayFromLocation** (part a) |
| 5 | 0x6c6930 | **ChangeFacingPoint** | GetModule, SetIntraAreaGoal, ResetAtEndOfPath, new/free, disp 0x5c/0x60/0x64/0x70, imm 0x1e | **InterAreaPathfinding** |
| 6 | 0x6c6d00 | MoveToObject | SetLockOrientationToObject, GetUseRange, GetIsInUseRange | MoveToObject |
| 7 | 0x6c6f90 | MoveToObjectRadius | GetUseRange, GetIsInUseRange, imm 0x11 | MoveToObjectRadius (imm 0x11) |
| 8 | 0x6c74c0 | MoveToPoint | disp 0x380/0x1114, imm 0xa | MoveToPoint (0x380, 0x1114, imm 0xa) |
| 9 | 0x6c7820 | **MoveToPoint_2** | imm 0x12, 0x50, 0x54 | **MoveToPointRadius** (imm 0x12) |
| 10 | 0x6c7b80 | MoveToFollowRadius | GetSWParty, ServerToClientObjectId, CSWParty::GetIndex | MoveToFollowRadius |

The GOG label `AIActionCheckMoveAwayFromObject` sits at 0x6c9d80, whose body is AIActionAreaWait (part r).

1. **0x6c6230 AIActionCheckMoveAwayFromLocation -> 0x5bb470: label_suspect.** fp 1:1 (299 B, RET 4, 5 callees). Steam
   callees by size: `005bb581 CALL 0x5143c0` (normalize, 143 B) and `005bb58d CALL 0x543f10` (SetOrientation, 51 B),
   plus Vector -/Dot/memcpy; `005bb498 CMP [EDX+0x11ac],0`. No GetArea, no AddActionToFront, no ComputeAwayVector, so
   it cannot be MoveAwayFromLocation. Fits Mac `AIActionChangeFacingPoint`. The GOG name lives at 0x6c66e0 -> 0x5bb920.
   Reviewer said `match` ("SetOrientation callee"): reviewer label error (100% row, not sample).
2. **0x6c6930 AIActionChangeFacingPoint -> 0x5bbb70: label_suspect.** fp 1:1 (962 B, 9 callees). `005bbdd3 CALL
   0x51bd90` (GetModule 22 B), `005bbdda CALL 0x55e610` (SetIntraAreaGoal 689 B), `005bbed2 CALL 0x6f9d40`
   (ResetAtEndOfPath), `005bbc25 CALL 0x919723` (new) and `005bbd21 CALL 0x9196fd` (free), 20 reads of `[+0x380]`,
   `005bbf1e PUSH 0x1e` (Mac InterArea imm 0x1e). Fits Mac `AIActionCheckInterAreaPathfinding`. ChangeFacingPoint is
   GOG 0x6c6230 -> 0x5bb470. Reviewer said `match`: reviewer label error.
3. **0x6c7820 AIActionCheckMoveToPoint_2 -> 0x5bca60: label_suspect.** fp 1:1 (848 B). `005bccf4 PUSH 0x12` before
   `005bccfc CALL 0x540ca0` (AddActionToFront). Mac MoveToPointRadius has imm 0x12; Mac MoveToPoint has imm 0xa and
   disp 0x1114, which the sibling has (`005bc9a4 PUSH 0xa`, `005bc796 MOVZX EDX,[EDX+0x1114]` in 0x5bc700 = GOG
   0x6c74c0). So the `_2` duplicate is `AIActionCheckMoveToPointRadius`, whose GOG label sits on 0x6c6360.
4. **0x6dc020 AddToVisibleList -> 0x5d1400: verified.** fp 1:1 (269 B, RET 0x14). `005d1410 CALL 0x5d16e0` (the
   visible-list contains helper, part a), `005d14fc ADD ECX,0x1028`, `005d1502 CALL 0x83ea60` (Add), `005d150a RET
   0x14`. The 6 GOG callers are, by name, the 6 Mac callers of `AddToVisibleList` (CSWSCreature ctor, SetId,
   AcceptParty, DoPerceptionUpdateOnCreature, GetInvisible, ReadStatsFromGff). The mac_rotation flag is false.
5. **0x6de780 CheckItemClassRestrictions -> 0x5d3b60: label_suspect.** fp 1:1 (298 B). `005d3b6b PUSH 0x3c` + `005d3b70
   CALL 0x603f60` (GetPropertyByTypeExists, property 0x3c), GetPassiveProperty x3, `005d3c48 MOV EAX,[EDX+0x88]` /
   `CALL EAX` (vcall; Mac +4 = 0x8c). No HasFeat. Mac `CheckItemGenderRestrictions` (185 B): imm 0x3c,0x3c, disp
   0x8c, callees exactly GetPropertyByTypeExists + GetPassiveProperty. Mac `CheckItemClassRestrictions` has HasFeat,
   disp 0x8d/0x1198, imm 0x2c/0x5d/0xab: that is GOG 0x6ddee0 (`005d32cb PUSH 0x2c`, `005d3311 CALL 0x6b83f0` HasFeat,
   `005d3443 MOV AL,[EDX+ECX+0xab]`). Mac CanUseItem calls 7 restriction checks incl. Gender; GOG CanUseItem calls the
   same 7 plus CNPC, with no Gender label. Reviewer said `match`, "name fits": reviewer label error.

## Escalated sample rows (reviewer `match`)

6. **0x6a3970 GetClientCreature -> 0x58ae90: verified.** fp 1:1 (53 B, RET 0). `0058aeb8 MOV EAX,[EDX+0x2c]` / `0058aebb
   CALL EAX` (Mac disp 0x30 = vtable +4), GetClientObject callee; 34 GOG callers and 34 Steam callers with the same
   size multiset; Mac callers (UpdateCombatMode, ShowObject, SetStealthState, AcceptParty, ...) appear in both.
7. **0x6a3810 GetCurrentlyFollowingLeader -> 0x58ad30: verified.** fp 1:1 (80 B). `0058ad3c ADD ECX,0x100` (action
   list), `0058ad6e CMP [ECX],0x3d` (action 0x3d = PartyFollowLeader, the RunActions case found in part r). Mac
   disp 0x100, imm 0x3d. Sole caller CSWCCreature::AIUpdate on both GOG and Mac.
8. **0x696f00 SetAttemptedAttackTarget -> 0x57e3d0: verified.** fp 1:1 (25 B). `0057e3dd MOV [EAX+0x550],ECX`, `0057e3e6
   RET 4`. Mac disp 0x550. 11 callers, size multiset equal; the 8 Mac callers (ClearAllActions, AIActionPhysicalAttacks,
   InitializeNewAttackTarget, RunActions, EndCombatRound, ...) are among them.
9. **0x69a160 AddChemical -> 0x581630: label_suspect (REVIEWER ERROR on label).** fp 1:1 (223 B, RET 8). `005816f1 PUSH
   0xe9` (feedback id). Mac `AddComponent` uses 0xe9 with fields 0x174/0x1104; Mac `AddChemical` uses 0xea with
   0x178/0x1108. The GOG callees are GetComponent 0x69a0b0 (0x174/0x1104, = Mac GetComponent imm 0x174) and
   SetComponent. Body = **AddComponent**. AddChemical is GOG 0x69a240 -> 0x581710. The reviewer said `match` and added
   "label may be swapped": it saw the swap but did not rule.
10. **0x699fd0 RemoveGold -> 0x5814a0: verified.** fp 1:1 (214 B, RET 8). `00581558 PUSH 0x95` (Mac RemoveGold imm
    0x95), `005814cb CALL 0x5812e0` GetGold, `005814f4 CALL 0x581330` SetGold. The 4 callers (TransferGold,
    AIActionGiveItem, AIActionDropItem, ExecuteCommandTakeGoldFromCreature) are the 4 Mac callers.
11. **0x6a3dc0 GetLastInstantEquip -> 0x58b2e0: verified.** fp 1:1 (59 B, RET 4). `0058b300 MOV EAX,[ECX+0x11d4]`,
    `0058b30b MOV EAX,[EDX+0x11d0]` keyed by 0x10/0x20. Mac disp 0x11d0/0x11d4, imm 0x10/0x20; callers
    WriteGameObjUpdate_WriteInventorySlotAdd/Delete on both. (Its Set twin is GOG 0x6a3e00, part a.)
12. **0x6a3cf0 GetListenCheckDistance -> 0x58b210: verified.** fp 1:1 (201 B). `0058b23b FLD [EAX+0x1020]`, `0058b247 MOV
    EDX,[ECX+0x520]`, `0058b29d MOV EDX,[ECX+0xbc]`, GetFLOATEntry `0058b2b3`, 'SecondaryRange'. Mac disp set
    0x8/0xbc/0x130/0x520/0x1020 identical, same 5 callees, same 7 callers by name.
13. **0x698e30 GetJournal -> 0x580300: verified.** fp 1:1 (39 B). `00580317 CALL 0x51c8d0` (GetPartyTable, B9) and
    `0058031e CALL 0x5f7ae0` (GetPartyJournal) = Mac GetJournal callees; 7 callers with the same sizes (Mac:
    Add/Get/RemoveJournalQuestEntry, AddJournalEntry, SetInt, JournalFullUpdate).
14. **0x69a240 AddComponent -> 0x581710: label_suspect (REVIEWER ERROR on label).** fp 1:1 (223 B, RET 8). `005817d1 PUSH
    0xea`; GOG callees GetChemical 0x69a320 (0x178/0x1108) and SetChemical. Body = **AddChemical**. AddComponent is
    GOG 0x69a160 -> 0x581630. Reviewer said `match` with "labels may be swapped".

`sample_errors: 2` (0x69a160 and 0x69a240: reviewer `match`, the counterparts are right but the GOG names are swapped;
no address error in the sample). Not counted (100% rows, not sample): the reviewer also said `match` on the rotated
rows 0x6c6230, 0x6c6930, 0x6c7820 and 0x6de780.

## AUTO rotation audit (job 2)

Scope: every AUTO row of `B12b/evidence.tsv` with GOG in 0x6c6000-0x6c7a00 or 0x6dde00-0x6df000: **14 rows checked**.
Each: GOG/Steam fp 1:1, caller-size multiset equal, and the GOG body compared with the Mac symbol of the same name.
**2 flagged**, added to `overrides_z.tsv` as `label_suspect` (overrides win over AUTO).

| GOG -> Steam | GOG name | check vs Mac symbol of that name | result |
|---|---|---|---|
| 0x6c6080 -> 0x5bb2c0 | AIActionChangeFacingObject | GetObjectArray/GetGameObject/GetArea/normalize/SetOrientation = Mac | pass |
| 0x6c6360 -> 0x5bb5a0 | AIActionCheckMoveToPointRadius | has CancelAction (`005bb76f CALL 0x576ca0`), `005bb70e ADD EAX,0x100`, ComputeAwayVector `005bb843`; Mac MoveToPointRadius has none, imm 0x12 absent | **fail: = MoveAwayFromObject** |
| 0x6c6d00 -> 0x5bbf40 | AIActionCheckMoveToObject | SetLockOrientationToObject, GetUseRange, GetIsInUseRange, GetModule = Mac | pass |
| 0x6c6f90 -> 0x5bc1d0 | AIActionCheckMoveToObjectRadius | GetUseRange/GetIsInUseRange, imm 0x11 = Mac; no SetLockOrientation | pass |
| 0x6c74c0 -> 0x5bc700 | AIActionCheckMoveToPoint | `005bc796 [EDX+0x1114]`, `005bc877 [ECX+0x380]`, `005bc9a4 PUSH 0xa` = Mac | pass |
| 0x6ddee0 -> 0x5d32c0 | RemoveFromInvitationsOffered | HasFeat(0x5d), item properties, +0x8d/+0xab; Mac RemoveFromInvitationsOffered is a 122 B +0x1044 list remove | **fail: = CheckItemClassRestrictions** |
| 0x6de0a0 -> 0x5d3480 | CheckItemAlignmentRestrictions | HasFeat, 0x18a, imm 0x2b/0x5d/0x64 = Mac (part a) | pass |
| 0x6de270 -> 0x5d3650 | CheckItemRaceRestrictions | GetBaseItem, GetPropertyByType, 0xb4/0xb8/0xe0/0xec, RET 8 = Mac | pass |
| 0x6de640 -> 0x5d3a20 | CheckItemFeatRestrictions | HasFeat + GetFeatRemainingUses, imm 0x39 = Mac | pass |
| 0x6de8b0 -> 0x5d3c90 | CheckItemPCRestrictions | GetCExoStringEntry, _stricmp, 0x130/0x134/0x1200, imm 0x3e = Mac | pass |
| 0x6deb00 -> 0x5d3ee0 | CheckItemAttributeRestrictions | GetIPRPCostTable, GetINTEntry, 0xed-0xf7, imm 0x45 = Mac | pass |
| 0x6dedb0 -> 0x5d4190 | CheckItemCNPCRestrictions | CompareNoCase, imm 0x61-0x66; no Mac symbol (inlined in Mac CanUseItem, which has CompareNoCase + imm 0x66) | pass (no contradiction) |
| 0x6dee80 -> 0x5d4260 | ReplyToInvitation | GetCreatureByGameObjectID, SetInvitedToParty, GetIsInInvitationsOffered, AddToInvitationsIgnored, AddMember, +0x1034 = Mac | pass |
| 0x6def90 -> 0x5d4370 | AddToInvitationsOffered | GetWorldTimer/GetWorldTime, Add, new, +0x1044 = Mac; caller CSWSFaction::InviteMember | pass |

Side findings (not B12b rows, not shipped):
- GOG 0x6df1b0 (FUN_, 119 B, +0x1044 x4, caller ReplyToInvitation) fits Mac `RemoveFromInvitationsOffered` (122 B,
  +0x1044); Steam 0x5d4590 is fp 1:1 (shape only, not gated).
- GOG 0x68f7d0 (label `GetBlind`, 277 B) = `CSWSCreature::CancelAction`: callees GetAtPos, GetNext, Remove,
  ~CSWSObjectActionNode, free = Mac CancelAction (284 B); called by 0x6c6360 as Mac MoveAwayFromObject calls
  CancelAction. Steam 0x576ca0.

## Counts

| verdict | rows |
|---|---|
| verified | 8 |
| label_suspect | 8 (6 part rows + 2 AUTO audit) |
| likely / rejected / unresolved | 0 |
