# Opus gate B10a part z: evidence

Not an escalation. Part a found 0 sample errors, so the other 48 REVIEW reviewer matches were not re-read. Part z holds
2 rows:
- `[z AUTO audit]`: 1 AUTO row that the rotation audit (job 2) contradicted.
- `[z dup]`: 1 REVIEW row that the lead brief rules out ("only one row may hold Steam 0x7ed5f0").

## 1. 0x6ae890 AddDoorSaberAction -> 0x547bb0: label_suspect (AUTO c2, was verified)

fp 1:1 (422 B RET 4, 7 callees, disp 0x30/0xec/0x520/0x10dc, imm 0x3f/0x47). The Steam callees, named by fp-exact match
to the GOG callee:
- `00547c1a CALL 0x00590110` = CSWSCombatRound::GetActionPending (GOG 0x7747c0)
- `00547c4e CALL 0x005917e0` = CSWSCombatRound::AddSwitchWeaponsAction (GOG 0x775e90; branch DB row 0x5917e0)
- `00547c53 PUSH 0x3f` / `00547c58 CALL 0x0053f560` = GetNodeById (GOG 0x6a6240), plus ActionManager 0x563c90,
  ClearAllActions 0x541080, AddAction 0x53f7f0

Mac `CSWSObject::AddSwitchWeaponsAction` (614 B) calls ActionManager, GetNodeById, AddAction, ClearAllActions,
GetActionPending and `CSWSCombatRound::AddSwitchWeaponsAction`, and is called by AIActionCombat +
HandlePlayerToServerInputMessage. Those are exactly the GOG callers of 0x6ae890. Mac `CSWSObject::AddDoorSaberAction`
is 271 B, calls only AddAction, and has imm 0x43. Its Windows body is GOG 0x6ae820 (`FUN_`, 111 B, imm 0x43, AddAction,
caller HandlePlayerToServerInputMessage) -> Steam 0x547b40 (fp 1:1, `00547b9a PUSH 0x43` /
`00547b9f CALL 0x0053f7f0`).

So the AddDoorSaberAction label sits one function too far. The GOG AddSwitchWeaponsAction label (0x6969a0, part r) is on
CSWSCreature::Rest.

## 2. 0x4727b0 Destructor_3 -> 0x7ed5f0: rejected (REVIEW C2, was verified)

GOG 0x472750 (Destructor_2, 96 B, no RET) + 0x4727b0 (Destructor_3, 379 B) = 475 B = Steam 0x7ed5f0, which is one body.
The head holds `007ed643 CALL 0x0073f8d0` (RemoveObjectFromHitcheckIgnoreList) and `007ed64b CALL 0x007f0a50`
(KillAllVisualEffects); the tail holds `007ed65e CALL 0x0073f790` and `007ed777 CALL 0x00733780`. GOG 0x4727b0 corresponds
to Steam 0x7ed650, which is not a function entry. The reviewer itself wrote "GOG split fragment; duplicate of Destructor_2
row" but ruled `match`, and the pipeline made both rows verified at 0x7ed5f0. Destructor_2 keeps 0x7ed5f0. This row
ships nothing, because a mid-function address cannot ship.

## AUTO rotation audit (job 2)

Windows: the Animation* window GOG 0x474400-0x474b00, plus +/-0x300 GOG around every row ruled label_suspect (0x42b680,
0x6638c0, 0x6969a0, 0x6ad780, 0x6ae890). Every AUTO row in those windows was checked against the Mac symbol of the same
name (callees, disp/imm, callers).

| GOG | AUTO name | Steam | Mac check | result |
|---|---|---|---|---|
| 0x474660 | AnimationLooping | 0x7ef500 | 'Looping' handle [0xa0fe24]; Mac imm 0x5 + GetINTEntry; caller MainLoop | pass |
| 0x474730 | AnimationFireAndForget | 0x7ef5d0 | 'FireForget' handle [0xa0fe1c]; Mac imm 0x5/0x9; caller InsertFireForgetAnimation | pass |
| 0x474800 | AnimationOverlay | 0x7ef6a0 | 'Overlay' handle [0xa0fe20]; sole caller AnimateFireAndForget (Mac same) | pass |
| 0x4748e0 | AnimationDialog | 0x7ef780 | GetInGameGui + IsFireForgetDialogAnimation (`007ef7a3 CALL 0x007cb010`) = Mac | pass |
| 0x6ad490 | UpdateDialog | 0x5467b0 | 8 callees = Mac (UpdateDialogLipSync, IsDialogDelay, Send*, StopDialog); caller MainLoop | pass |
| 0x6ad5e0 | StopDialog | 0x546900 | GetGameObject, SetLockOrientationToObject, RemoveObjectFromDialog, RunEndConversationScript = Mac | pass |
| 0x6ad750 | StopSoundPlayingInDialog | 0x546a70 | 1 callee, disp 0x25c = Mac | pass |
| 0x6ad7e0 | RemoveObjectFromDialog_2 | 0x546b00 | RemovePlayer + StopDialog, caller StopDialog = Mac RemoveObjectFromDialog (name fits; `_2` is a suffix artifact) | pass |
| 0x6ad8b0 | GetNearestObjectByName | 0x546bd0 | CompareNoCase, Get{First,Next}ObjectInArea, disp 0x260; same 4 callers as Mac | pass |
| 0x6ae5a0 | AddOpenDoorAction | 0x5478c0 | AddAction, imm 0x14 = Mac | pass |
| 0x6ae620 | AddCloseDoorAction | 0x547940 | AddAction, imm 0x15 = Mac | pass |
| 0x6ae6a0 | AddUnlockObjectAction | 0x5479c0 | GetCanUseSkill + SendFeedbackMessage, imm 0x26 = Mac | pass |
| 0x6ae770 | AddLockObjectAction | 0x547a90 | GetCanUseSkill, imm 0x27 = Mac | pass |
| 0x6ae890 | AddDoorSaberAction | 0x547bb0 | body = Mac AddSwitchWeaponsAction (above) | **fail -> label_suspect** |
| 0x6aea40 | BroadcastSpellData | 0x547d60 | 9 Mac callees present, caller SpellCastAndImpact = Mac | pass |

Checked 15, flagged 1. The windows around 0x42b680, 0x6638c0 and 0x6969a0 hold no AUTO rows of this batch. The REVIEW
rows in the Animation window (Stationary [0xa0fe28], Parry [0xa0fe3c], Attack [0xa0fe4c]) were also checked and are right.
Together with the 8 AUTO meaning checks of part a (all pass; 0x474800 is in both sets), 22 distinct AUTO rows of 161 got
an Opus meaning read, and 1 was wrong.

sample_errors: 0 (no sample rows in part z)
