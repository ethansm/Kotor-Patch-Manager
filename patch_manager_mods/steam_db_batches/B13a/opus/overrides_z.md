# Opus gate B13a part z: escalation rows + AUTO rotation audit

Why this part exists: part a found 1 sample error (0x41f7b0 ResetDesiredPositionTimer: right address, wrong name), so
B13a escalates. Every other REVIEW row with a reviewer `match` on a real card (decoys in `cards/key.tsv` ignored) gets an
Opus read: 41 rows. The AUTO rotation audit adds 4 rows.

Method: same as part a (GOG/Steam/Mac fp, caller multisets from the fp callee columns, Steam asm from
`steam_6a522e71/decomp`). Mac vtable offsets are +4 vs Windows.

Verdicts: verified 40, label_suspect 5 (1 escalation row + 4 AUTO audit rows). No address corrections: every Steam
counterpart in this part is the positional twin, fp 1:1 except GetBehaviors (below).

## Escalation rows (41)

40 of 41 are verified. Each is fp 1:1 with the GOG body, and each name matches the Mac symbol's shape: disp set, imm
set, and callee identity where the callee is already named. Per-row anchors are in `overrides_z.tsv`. Rows worth a note:

- **0x413e30 ForceAppearanceUpdate -> 0x768060 label_suspect (reviewer match wrong, not sampled).** 59 B:
  `0076806a CALL 0x7ed980` (CSWCObject::GetLoopingAnimation = GOG 0x472ae0), `00768072 CMP EAX,0x2711`, vcall +0x9c.
  Sole caller EquipToInventorySlot. Mac ResetAnimationState is 74 B: GetLoopingAnimation, disp 0xa0 (= vt 0x9c), and Mac
  EquipToInventorySlot calls it. Mac ForceAppearanceUpdate (SetAppearance callee, 0x188/0x224/0x328/0x3a4, imm 0xef) is
  GOG 0x420720 (audit row below). The reviewer wrote "name not contradicted".
- **0x4269a0 GetBehaviors -> 0x77a9e0 verified, not fp 1:1.** GOG 813 B vs Steam 1206 B, because Steam adds behaviour ids
  0x410-0x414 (Mac GetBehaviors has the same imm 0x410/0x412/0x413/0x414). Anchors: `0077aaf8 PUSH 0x99f2e4` 'NAME_STRREF',
  `0077ab2f PUSH 0x99f2dc` 'AISTATE' (both GOG str_refs), sole caller GetPersonalActions (`0077e6c6 CALL 0x77a9e0`), as GOG
  0x4269a0's sole caller is GOG GetPersonalActions 0x42a5d0. The KDB name AddStanceActions is wrong (kdb_overrides).
- **0x414210 MoveCreatureToPosition -> 0x768440 verified.** `0076846d CMP EDX,[ECX+0x1f0]`, +0x44, imm 0x1e4. Callers
  AIUpdate/AnimationUpdate/ClientServerSyncHandler_Position, the same 3 sizes on both sides; Mac AnimationUpdate and
  SyncHandler_Position call MoveCreatureToPosition. The KDB name SetCombatState is wrong (kdb_overrides).
- **0x421370 SetArea, 0x420310 RemoveShadowBlob, 0x415d50 GetStartWayPoint, 0x428d50 PerformUseHostileItem:** pre-Opus
  `likely`, now `verified`. Each has an identity anchor: a CSWCObject::SetArea + UpdateAreaId callee pair; a thunk to
  CSWCObject::RemoveShadowBlob; no imm 0xc, where Mac GetEndWayPoint has one; SetCombatMode + PartyMenuUseItem callees.
- **0x41f940 / 0x41f980 Hide/ShowEquippedHeadItem:** the twins are told apart by vcall slot and flag. Hide is
  `00773a90 [EDX+0x88]` + `[+0x3e8]=1`, Show is `00773ad0 [EDX+0x8c]` + `[+0x3e8]=0`; Mac Hide 0x8c and Show 0x90 are the
  same slots +4. The macmatch heuristic's "Show looks like Hide" hit was the +4 vtable shift.
- **0x429780 GetServerCreature:** vt +0x30 vs Mac +0x34 (+4 shift, not a contradiction). 133/132 callers.

## AUTO rotation audit (Job 2)

Window: AUTO rows within +/-0x300 GOG of every row ruled label_suspect, unresolved-for-rotation or address-corrected in
parts a/r/z. The centres were 0x413620, 0x4137c0, 0x413e30, 0x413e70, 0x41ac10, 0x41ae50, 0x41f5a0, 0x41f6b0,
0x41f730, 0x41f7b0, 0x420070, 0x4200d0, 0x4213c0, 0x421820-0x421880, 0x422dd0, 0x422e00, 0x4248c0, 0x426430-0x426490,
0x42bae0, 0x42bd00 and 0x443170. That gives 21 AUTO rows, each checked against the Mac symbol of the same name.

| GOG | AUTO name | result | evidence |
|---|---|---|---|
| 0x41a990 | SetLastInstant | pass | disp 0x280..0x2c0 = Mac |
| 0x41ade0 | GetLastInstant | **FLAG** | = Mac SetItemUnEquippedByMe (vt 0x34/0x38, imm 0x14; callers EquipToInventorySlot + StripEquippedItems body 0x41f5a0) |
| 0x41aee0 | UpdateAttackData | pass | callees Melee/Ranged/ResolveReaction/ResolveSpecial/IsCreature/0x41bd70 = Mac set (InCreatureCombat incl.) |
| 0x41b0c0 | ResolveAttackIndex | pass | disp 0x3d8/0x3dc = Mac |
| 0x41f430 | DoFootstepAudio | pass | imm 0x5/0x13/0x64 = Mac |
| 0x41f810 | SetRunLooksAwful | pass | byte +0xcc = Mac |
| 0x41f840 | SetSpecialWalkAnimation_2 | **FLAG** | = Mac ClearVisibilityList (0x378/0x37c, delete; caller ~CSWCCreature) |
| 0x41f8e0 | HideEquippedItems | pass | vt 0x80 (Mac 0x84), +0x3e4 |
| 0x41f9c0 | ShowEquippedItems | pass | calls 0x413e70 = Mac calls ComputeWeaponOverlays; vt 0x84/0x94 (Mac 0x88/0x98) |
| 0x420330 | TurnOnShadows | pass | lower field pair (0xb8/0x110) as Mac On (0xbc/0x114) vs Off |
| 0x422fa0 | DriveAnimationUpdate | pass | calls 0x422e00 (IsDriveAnimationBlocked body) + GetPauseReadyAnimation = Mac |
| 0x4247a0 | CheckFeat | pass | 0x310/0x1198 |
| 0x424b60 | GetEquipmentForcePowers_2 | pass | the real GetEquipmentForcePowers (0x104/0x13c/0x194-0x19c/0x228 = Mac) |
| 0x426240 | ActionSwapImplant | pass | disp 0x98 = Mac; callers are the four thunks |
| 0x4264b0 | GetHostileForcePowers | pass | calls 0x424b60 + GetDroidConfusion = Mac |
| 0x42ba20 | PlayRollingFootstepSound | pass | 0x448..0x454 = Mac |
| 0x42bb30 | UpdateRollingFootstepSound_2 | pass | calls Play + stop (0x42bae0) = Mac UpdateRollingFootstepSound (stop inlined) |
| 0x42bc30 | SetLockOrientationToObject | pass | 0x30c/0x11ac = Mac |
| 0x42bd40 | SetCombatMode | pass | 0x458 x3 = Mac |
| 0x42bdb0 | PlayDrawAnimation | pass | imm 0x5dc = Mac |
| 0x42be70 | IsStealthCapable | pass | imm 0x9c/0x9d/0x9e/0xc9 = Mac |

**AUTO audit: 21 checked, 2 flagged (0x41ade0, 0x41f840).**

Two more AUTO label errors outside the window turned up while tracing where the rotated names really live. Both are
verified and included as `label_suspect`:

- 0x41bd70 SetItemUnEquippedByMe -> 0x76ff00 = Mac InCreatureCombat (`0076ff19 CALL 0x76fe00` IsCreature, vt 0x2c = Mac
  0x30, caller UpdateAttackData as Mac).
- 0x420720 SetPopUpBubbleDecayTime -> 0x774850 = Mac ForceAppearanceUpdate (`007748b7 CALL 0x76d200` SetAppearance,
  `007748a6 PUSH 0xef`, +0x188/+0x328/+0x3a4; callers AIUpdate + GetModel, and Mac GetModel calls ForceAppearanceUpdate).
  Mac SetPopUpBubbleDecayTime is a 5 B stub.

Every one of the 4 has the same Steam caller-size multiset as GOG, so the counterparts are certain. Only the names are wrong.

Not flagged, but outside the audit window and not fully checked: GOG 0x412130 `AIUpdate` (47 B, one callee, ret 4) next to
the real AIUpdate body 0x412180. It may be a virtual-overload wrapper; it needs a look in a later pass.

sample_errors: 0 (no sampled rows in this part; the escalation rows are not sampled). Reviewer `match` on a wrong
name among the 41 escalation rows: 1 (0x413e30).
