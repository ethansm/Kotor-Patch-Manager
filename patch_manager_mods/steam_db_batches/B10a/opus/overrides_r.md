# Opus gate B10a part r: evidence (REJECT/NOCAND rows)

Method: a whole-binary scan of the Steam `fp.tsv` for each GOG row on the exact key (size, retn, n_insn, disp, imm,
callee count, string count). Then the neighbour run, Steam asm, and Mac x86 symbols. All 14 rows have exactly **one**
fp-exact Steam body. None of the 7 gate picks was right.

Local deltas: CSWCObject client block +0x37aea0 (0x4726c0); CSWCCreature block +0x353fe0 (0x42b680); CSWSObject
0x6637xx run -0x1248a0; CSWSCreature block -0x118ae0/-0x118b30 (0x6a2c90, 0x6969a0); AIAction block -0xdc1f0 (0x6b5760 to
0x6bd340, the same as AIActionSetCommandable 0x6bd060 -> 0x5e0e70). The REJECT picks at 0x5564b0 came from a wrong
batch delta.

| # | GOG | GOG name | gate pick | final Steam | verdict |
|---|---|---|---|---|---|
| 1 | 0x42b680 | MenuActionSetObjectMine | - | 0x77f660 | label_suspect (= CSWCCreature::PerformCombatFeat) |
| 2 | 0x4726c0 | ClientServerSyncHandler_Position | 0x405970 | 0x7ed560 | verified |
| 3 | 0x4774e0 | BarkBubble | 0x7f23f0 | 0x77b630 | verified (one body, see below) |
| 4 | 0x663800 | GetMaxHitPoints | 0x4597f0 | 0x53ef70 | likely |
| 5 | 0x663830 | GetLockOrientationToObject | - | 0x53ef90 | likely |
| 6 | 0x6638c0 | GetFirstName | 0x4598b0 | 0x600090 | label_suspect (= CSWSItem::GetFirstName) |
| 7 | 0x6969a0 | AddSwitchWeaponsAction | - | 0x57de70 | label_suspect (= CSWSCreature::Rest) |
| 8 | 0x6a2c90 | LipSync | - | 0x58a1b0 | likely (shared stub) |
| 9 | 0x6b5760 | AIActionPauseDialog | - | 0x5d9570 | verified |
| 10 | 0x6b5880 | AIActionResumeDialog | - | 0x5d9690 | verified |
| 11 | 0x6bbb10 | AIActionSwitchWeapons | - | 0x5df920 | verified |
| 12 | 0x6bd120 | AddUseObjectAction | 0x5564b0 | 0x5e0f30 | verified |
| 13 | 0x6bd190 | AddGiveItemActions | 0x5564b0 | 0x5e0fa0 | verified |
| 14 | 0x6bd340 | AddTakeItemActions | 0x5564b0 | 0x5e1150 | verified |

## Per row

1. **0x42b680 -> 0x77f660: label_suspect.** fp 1:1 (85 B RET 8). Steam: `0077f677 CALL 0x0073f810` (GetSWCMessage),
   `0077f69a CALL 0x008799a0` (SendPlayerToServerInput_Attack), `0077f6a5 PUSH 0x1` / `0077f6aa CALL 0x0077fd20`
   (SetCombatMode = GOG 0x42bd40 + 0x353fe0). Mac `CSWCCreature::PerformCombatFeat` is 87 B with exactly these 3 callees
   and is called by `CSWGuiTutorialBox::PerformCombatFeat`; the GOG callers are 0x586cb0 (labelled
   CSWGuiTutorialBox::PerformCombatFeat) and 0x427080. Mac has no `MenuActionSetObjectMine` symbol at all. The Steam
   neighbours fit too: 0x77f600 = GOG GetDriveAcceleration 0x42b620, 0x77f6c0 = GOG LoadRollingFootstepSound 0x42b6e0.
2. **0x4726c0 -> 0x7ed560: verified.** fp 1:1 (33 B RET 0xc). `007ed573 MOV EAX,[EDX+0x8c]` / `007ed579 CALL EAX` /
   `007ed57e RET 0xc`. Mac ClientServerSyncHandler_Position uses slot 0x90 and Orientation 0x8c (Windows +4: 0x8c/0x88).
   The Orientation sample row is 0x7ed590 (0x88). The gate pick 0x405970 (63 B RET 4, `00405983 MOV ECX,[EAX+0x24]`) is
   unrelated code.
3. **0x4774e0 -> 0x77b630: verified.** fp 1:1 (62 B RET 0x10). `0077b657 CALL 0x0073f750` (GetInGameGui) +
   `0077b65e CALL 0x007ca6e0` (fp= GOG CGuiInGame::ShowBarkBubble 0x4dade0, 74 B RET 0x10). Mac has two identical 76 B
   bodies, `CSWCObject::BarkBubble` and `CSWCCreature::BarkBubble` (GetInGameGui + ShowBarkBubble). Each Windows build has
   only one: GOG kept it in the CSWCObject run, Steam put it in the CSWCCreature area (0x77b600). In the Steam CSWCObject
   run, QueuedCallback 0x7f2280 is followed directly by ClientServerSyncHandler_Animation 0x7f2380. So a hook on this
   address fires for both classes. The gate pick 0x7f23f0 is MenuActionUseSecuritySpike (GOG 0x477590, fp 1:1).
4. **0x663800 -> 0x53ef70: likely.** fp 1:1 (23 B RET 4). `0053ef7a MOV AX,word ptr [EAX+0xe4]` / `0053ef84 RET 0x4` =
   Mac CSWSObject::GetMaxHitPoints (disp 0xe4). It is a getter inside the B0-anchored run 0x53ef30 GetDialogResref,
   0x53ef50 GetLastName, 0x53efa0 GetPortrait ... 0x53f000 GetPortraitId. The gate pick 0x4597f0 reads +0x1dc and is a
   different function.
5. **0x663830 -> 0x53ef90: likely.** fp 1:1 (16 B). `0053ef97 MOV EAX,0x7f000000` / `0053ef9f RET` = Mac
   GetLockOrientationToObject (OBJECT_INVALID stub). Same run.
6. **0x6638c0 -> 0x600090: label_suspect.** fp 1:1 (19 B). `0060009a ADD EAX,0x2b0` / `006000a2 RET`. Mac
   `CSWSItem::GetFirstName` 0x465630 is imm 0x2b0; Mac `CSWSObject::GetFirstName` returns +0xc (11 B), like GetLastName
   (Steam 0x53ef50). The next function on both builds is the CSWSItem deleting destructor: GOG 0x6638e0 -> 0x663910
   (667 B) and Steam 0x6000b0 -> 0x6000e0 (667 B, fp 1:1). So this is CSWSItem::GetFirstName. The gate pick 0x4598b0 is
   trusted Gob::DisableInViewVolume.
7. **0x6969a0 -> 0x57de70: label_suspect.** fp 1:1 (716 B RET 4, 16 callees). The GOG callees (CExoString,
   GetModule, GetServerAIMaster, AddEventAbsoluteTime, ActionManager, SetActivity, ClearActivities, GetNearestEnemy,
   SendFeedbackMessage, AddAction, ClearAllActions, GetArea, CSWCCMessageData, SetInteger, new) are the 16 callees of Mac
   `CSWSCreature::Rest` (792 B). That function's only caller is HandlePlayerToServerInputMessage, the same as GOG. Steam:
   `0057df52 CALL 0x0057c990` (GetNearestEnemy = GOG 0x6954c0 - 0x118b30), feedback `CALL 0x00584da0` with ids
   0x36/0x11/0x12/0xba. The real AddSwitchWeaponsAction body is GOG 0x6ae890 (labelled AddDoorSaberAction), which goes to
   part z.
8. **0x6a2c90 -> 0x58a1b0: likely.** fp 1:1 (15 B): `0058a1b4 MOV [EBP-4],ECX` / `0058a1b7 XOR EAX,EAX` /
   `0058a1bc RET 0x4`. That fits Mac CSWCObject::LipSync (a 7 B empty stub). The body is the only one of its shape on
   both Windows builds, and it sits inside the CSWSCreature server block (delta -0x118ae0, as SetInParty). That points to
   one ICF-shared "return 0" stub: a hook here fires for every function folded into it.
9. **0x6b5760 -> 0x5d9570: verified.** fp 1:1 (274 B RET 4, no direct callee, vcalls only). `005d95a4 CMP
   [ECX+0x11ac],0`; disp 0x1124 x3 / 0x11ac = Mac AIActionPauseDialog (0x1124, 0x11ac, no callees). The KDB already has it
   as the RunActions handler for type 0x1F.
10. **0x6b5880 -> 0x5d9690: verified.** fp 1:1 (320 B RET 4). `005d96c4 CMP [ECX+0x11ac],0`; its 2 callees are
    ActivityManager + SetActivity = Mac AIActionResumeDialog. KDB: RunActions handler type 0x20.
11. **0x6bbb10 -> 0x5df920: verified.** fp 1:1 (696 B RET 4). `005df93e CALL 0x00541080` (ClearAllActions, B0),
    `005dfba9 PUSH 0x5dc`, `005dfbc8 CALL 0x00568540` (AddAttackActions) = Mac AIActionSwitchWeapons (GetItemInSlot,
    RunEquip, RunUnequip, AddAttackActions, ClearAllActions, RemoveAllActions; imm 0x5dc; disp 0x520/0xb20). The KDB name
    `CSWSCreature::AIActionType47_ReequipResumeAttack` is a placeholder (see kdb_overrides).
12. **0x6bd120 -> 0x5e0f30: verified.** fp 1:1 (111 B RET 4). `005e0f85 PUSH 0xffff` / `005e0f8a PUSH 0x28` /
    `005e0f8f CALL 0x0053f7f0` (AddAction) = Mac AddUseObjectAction (imm 0x28, disp 0xec). The loose shape twin 0x547b40
    pushes 0x43 (that is the real AddDoorSaberAction, see part z).
13. **0x6bd190 -> 0x5e0fa0: verified.** fp 1:1 (419 B RET 0x10). `005e1065 MOV ECX,[EAX+0x11ac]`, `005e1130 PUSH 0x22`
    / `005e1135 CALL 0x0053f7f0` = Mac AddGiveItemActions (imm 0x22, disp 0x2a8/0x2cc/0x11ac, 3 Get*ByGameObjectID callees).
14. **0x6bd340 -> 0x5e1150: verified.** fp 1:1 (486 B RET 0xc). `005e12df CALL 0x00564770` (AddMoveToPointAction, see
    RE_AGENT_GUIDE 3.5), `005e1323 PUSH 0x23` / `005e1328 CALL 0x0053f7f0` = Mac AddTakeItemActions (imm 0x23).

None of the 14 Steam addresses is held by any row in the branch DB or in the B10a final rows (duplicate check clean).

## Counts

14 rows: verified 8, likely 3, label_suspect 3, rejected 0, unresolved 0.

sample_errors: 0 (part r has no sample rows)
