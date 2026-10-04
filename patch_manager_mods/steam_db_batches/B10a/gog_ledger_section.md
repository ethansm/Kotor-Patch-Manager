## B10a

### CSWSObject, CSWCObject (B10a: one Animation* permutation, one AddSwitchWeaponsAction chain, isolated errors)
B10a Opus gate parts a/r/z + consolidation, 2026-10-03 (`B10a/opus/overrides_{a,r,z}.md`, `B10a/B10a_SUMMARY.md`).
Every Steam body below is fp 1:1 with its GOG body (size, retn, n_insn, disp, imm); only the GOG name is wrong.

**Animation* permutation (GOG 0x4744B0-0x474A70).** Each 129 B body reads one `animations.2da` column handle; Steam
0x6EB670 stores the handles in alphabetical order. These 5 rows ship as `verified (semantic)`: each name sits at the
Steam body of its column, the same as the B0 branch rows. The GOG labels are wrong.

| GOG addr | GOG DB name | body is | evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x4744B0 | `AnimationPlayOutOfPlace` | AnimationPause: 'Pause' handle (`007ef3a7 MOV EAX,[0x00a0fe2c]`) | callers AnimationUpdate + DriveAnimationUpdate = Mac `AnimationPause` | 0x7EF350 | GOG 0x474890 -> Steam 0x7EF730 |
| 0x474540 | `AnimationHideEquippedItems` | AnimationWalking: 'Walking' handle (`007ef437 MOV EAX,[0x00a0fe30]`) | Mac Pause/Walking/Running consecutive | 0x7EF3E0 | GOG 0x474A70 -> Steam 0x7EF910 |
| 0x4745D0 | `AnimationWalking` | AnimationRunning: 'Running' handle (`007ef4c7 MOV EAX,[0x00a0fe34]`) | as above | 0x7EF470 | GOG 0x474540 -> Steam 0x7EF3E0 |
| 0x474890 | `AnimationPause` | AnimationPlayOutOfPlace: 68 B id-range test, no handle (`007ef752 CMP EDX,0x640`) | Mac `AnimationPlayOutOfPlace` 51 B, no callee, imm 0x80/-0x640; caller AnimateFireAndForget | 0x7EF730 | GOG 0x4744B0 -> Steam 0x7EF350 |
| 0x474A70 | `AnimationRunning` | AnimationHideEquippedItems: handle [0xa0fe50] (`007ef967`) | Mac `TestAndReEquipweapons` calls HideEquippedItems; GOG TestAndReEquipweapons calls 0x474A70 | 0x7EF910 | GOG 0x4745D0 -> Steam 0x7EF470 |

**AddSwitchWeaponsAction chain.** Each label sits on the wrong body: AddSwitchWeaponsAction -> Rest,
AddDoorSaberAction -> AddSwitchWeaponsAction, and the real AddDoorSaberAction body is an unlabelled `FUN_`.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x6969A0 | `CSWSObject::AddSwitchWeaponsAction` | CSWSCreature::Rest: GetNearestEnemy (`0057df52 CALL 0x0057c990`), feedback 0x36/0x11/0x12/0xBA, AddEventAbsoluteTime | Mac `CSWSCreature::Rest` 792 B, same 16 callees, caller HandlePlayerToServerInputMessage | 0x57DE70 | GOG 0x6AE890 -> Steam 0x547BB0 |
| 0x6AE890 | `CSWSObject::AddDoorSaberAction` | AddSwitchWeaponsAction: GetActionPending (`00547c1a CALL 0x00590110`), CSWSCombatRound::AddSwitchWeaponsAction (`00547c4e CALL 0x005917e0`) | Mac `AddSwitchWeaponsAction` same callees, callers AIActionCombat + HandlePlayerToServerInputMessage | 0x547BB0 | GOG 0x6AE820 (`FUN_`) -> Steam 0x547B40 |

**Isolated label errors.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x42B680 | `CSWCObject::MenuActionSetObjectMine` | CSWCCreature::PerformCombatFeat: GetSWCMessage, SendPlayerToServerInput_Attack (`0077f69a CALL 0x008799a0`), SetCombatMode(1) | Mac `CSWCCreature::PerformCombatFeat` 87 B, same 3 callees, caller CSWGuiTutorialBox::PerformCombatFeat; Mac has no MenuActionSetObjectMine | 0x77F660 | unknown (no Mac symbol) |
| 0x6638C0 | `CSWSObject::GetFirstName` | CSWSItem::GetFirstName: `0060009a ADD EAX,0x2b0`; next = CSWSItem dtor | Mac `CSWSItem::GetFirstName` imm 0x2B0; Mac CSWSObject::GetFirstName returns +0xC | 0x600090 | CSWSObject one: same body as GetLastName (Steam 0x53EF50, B0) |
| 0x6AD780 | `CSWSObject::RemoveObjectFromDialog` | GetInGameGui -> IsAnimationPlayingInDialog forwarder (`00546abe CALL 0x007cadb0`), caller IsDialogDelay | Mac RemoveObjectFromDialog calls RemovePlayer + StopDialog (caller StopDialog) | 0x546AA0 | GOG 0x6AD7E0 (`RemoveObjectFromDialog_2`) -> Steam 0x546B00 |

**Split and fold notes (no label error, but no shippable row).**
- GOG `CSWCObject::Destructor_2` 0x472750 (96 B) + `Destructor_3` 0x4727B0 (379 B) are one Steam body, 0x7ED5F0 (475 B).
  Only Destructor_2 ships; Destructor_3 is rejected (mid-function).
- GOG `CSWCObject::BarkBubble` 0x4774E0 = Steam 0x77B630, which sits in the CSWCCreature area. It is one body for
  CSWCObject and CSWCCreature (Mac has two identical 76 B copies).
- GOG `CSWCObject::LipSync` 0x6A2C90 = Steam 0x58A1B0, an empty `XOR EAX,EAX / RET 4` stub inside the CSWSCreature server
  block. It is probably ICF-shared, so do not hook it.

Below the batch (not B10a rows): GOG 0x427080 `CSWCDoor::MenuActionSetDoorMine` (113 B, calls the PerformCombatFeat body)
does not fit Mac MenuActionSetDoorMine (329 B); it is likely ActionMenuUseAttackFeat.
