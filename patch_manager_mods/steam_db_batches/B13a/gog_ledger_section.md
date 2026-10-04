## B13a

### CSWCCreature, client creature (B13a: label chains, one swap, isolated errors)
B13a Opus gate parts a/r/z + consolidation, 2026-10-03 (`B13a/opus/overrides_{a,r,z}.md`, `B13a/B13a_SUMMARY.md`).
Each Steam counterpart below is fp 1:1 with its GOG body (size, retn, n_insn, disp, imm) and has the same caller-size
multiset; only the GOG name is wrong. 21 rows are marked "row": 19 are `label_suspect` in the B13a overrides and 2
(the 0x41F6B0/0x41F730 twins) are `unresolved`. None is shipped under its GOG name. Rows marked "info" have no gated row in B13a. Mac vtable offsets are +4 vs Windows.

**Chain 1: animation / appearance.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x413E30 | `ForceAppearanceUpdate` | ResetAnimationState: `0076806a CALL 0x7ed980` GetLoopingAnimation, `CMP EAX,0x2711`, vt +0x9C | Mac ResetAnimationState 74 B, GetLoopingAnimation, vt 0xA0; called by EquipToInventorySlot (as GOG) | 0x768060 (row) | GOG 0x420720 -> 0x774850 |
| 0x420720 | `SetPopUpBubbleDecayTime` | ForceAppearanceUpdate: `007748b7 CALL 0x76d200` SetAppearance, `PUSH 0xef`, +0x188/+0x328/+0x3A4 | Mac ForceAppearanceUpdate (SetAppearance, imm 0xEF), called by GetModel; Mac SetPopUpBubbleDecayTime = 5 B stub | 0x774850 (row) | stub (no body here) |
| 0x41F7B0 | `ResetDesiredPositionTimer` | GetPath callback (cdecl, writes arg3+4/+8); RegisterCallbacks `0077f557 PUSH 'GetPath'` / `PUSH 0x7738e0` | Mac GetPath 22 B, disp 0x4/0x8 | 0x7738E0 (row) | GOG 0x414410 (label `Unblur`, disp 0x1F0 imm 0x200) -> 0x7685F0 (info) |
| 0x41F6B0 | `UnloadMonkVisuals` | 2DA float callback, GetPersonalRadius or GetCreatureRadius (twin) | Mac GetPersonalRadius/GetCreatureRadius 107 B twins | 0x7737E0 (Personal) / 0x773860 (Creature), positional (row, unresolved) | GOG 0x4137C0 |
| 0x41F730 | `ResetAnimationState` | the other twin | as above | 0x773860 positional (row, unresolved) | GOG 0x413E30 |
| 0x4137C0 | `ClearVisibilityList` | UnloadMonkVisuals: `00767a55 MOV [ECX+0x178],0`, `[+0x17c]=0`, vt +0x4C | Mac UnloadMonkVisuals disp 0x50/0x178/0x17C/0x180 | 0x7679F0 (row) | GOG 0x41F840 -> 0x773970 |
| 0x41F840 | `SetSpecialWalkAnimation_2` | ClearVisibilityList: `0077397c [EAX+0x37c]`, `ADD ECX,0x378`, delete; caller ~CSWCCreature | Mac ClearVisibilityList 0x378/0x37C, called by the dtor | 0x773970 (row) | GOG 0x41F7E0 is the only SetSpecialWalkAnimation (+0xC8) |

**Chain 2: attack data / equipment.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x41BD70 | `SetItemUnEquippedByMe` | InCreatureCombat: `0076ff19 CALL 0x76fe00` IsCreature, vt +0x2C | Mac InCreatureCombat (IsCreature, vt 0x30), called by UpdateAttackData (as GOG) | 0x76FF00 (row) | GOG 0x41ADE0 -> 0x76EF70 |
| 0x41ADE0 | `GetLastInstant` | SetItemUnEquippedByMe: GetGameObject, vt +0x34, imm 0x14 | Mac SetItemUnEquippedByMe (vt 0x38, imm 0x14), called by EquipToInventorySlot + StripEquippedItems (as GOG) | 0x76EF70 (row) | GOG 0x41AC10 -> 0x76EDA0 |
| 0x41AC10 | `UpdateRangedAttackData` | GetLastInstant: one-arg slot-mask getter of +0x280..+0x2B8 (`0076ee5a`, `0076eefd`) | mirror of Mac SetLastInstant (same fields); no Mac GetLastInstant symbol | 0x76EDA0 (row) | GOG 0x41B7B0 -> 0x76F940 (Mac 212 B, IsCreature) |
| 0x41AE50 | `BarkBubble` | attack index -> GetEquippedItemID(mask), default 0x7F000000; caller SpawnHitVisuals | Mac BarkBubble 76 B, 2 unrelated callees | 0x76EFE0 (row) | not found |
| 0x4248C0 | `GetEquipmentForcePowers` | 4-arg compare helper, no calls, RET 0x10; callers Update{Melee,Ranged}AttackData | Mac GetEquipmentForcePowers 1110 B (0x104/0x13C/0x194-0x19C/0x228) | 0x778900 (row) | GOG 0x424B60 (`_2`) -> 0x778BA0 |
| 0x413E70 | `UpdateRollingFootstepSound` | ComputeWeaponOverlays: `007680d1 PUSH 0x272e`, vt +0x4C x8, +0x2E8/+0x3B0; callers Show/HideEquippedItems, UpdateVisibleWeapons | Mac ComputeWeaponOverlays vt 0x50 x8, 0x2E8/0x3B0, called by ShowEquippedItems | 0x7680A0 (row) | GOG 0x42BB30 -> 0x77FB10 |

**Chain 3: drive animation.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x422E00 | `InCreatureCombat` | IsDriveAnimationBlocked: `00776e4c [EAX+0x13c]`, 3 callees; callers DriveAnimationUpdate, SetPrecalculatedPosition | Mac IsDriveAnimationBlocked 102 B, disp 0x48/0x13C, same callers | 0x776E40 (row) | GOG 0x41BD70 -> 0x76FF00 |
| 0x422DD0 | `IsDriveAnimationBlocked` | SetSWAnimationSpeed: `00776e1d FSTP [EAX+0x3cc]`, `FSTP [ECX+0x3c8]`, RET 8 | Mac SetSWAnimationSpeed 34 B, 0x3C8/0x3CC | 0x776E10 (row) | GOG 0x422E00 -> 0x776E40 |
| 0x42BD00 | `SetSWAnimationSpeed` | GetLightSaberThrowRate: `0077fcec CALL 0x77d800` GetServerCreature, `FLD [EAX+0x11dc]`; caller ApplyLightsaberThrow | Mac GetLightSaberThrowRate 0x11DC + GetServerCreature | 0x77FCE0 (row) | GOG 0x422DD0 -> 0x776E10 |

**Swap and isolated errors.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x413620 | `StripEquippedItems` | SetDesiredOrientationForLock: `0076791c [EDX+0x30c]`, `ADD ECX,0x1d8`, normalize; caller AnimationUpdate | Mac SetDesiredOrientationForLock 0x1D8/0x1E0/0x30C, called by AnimationUpdate | 0x7678F0 (row) | GOG 0x41F5A0 -> 0x7736D0 |
| 0x41F5A0 | `SetDesiredOrientationForLock` | StripEquippedItems: `CMP ECX,0x14` slot loop, GetEquippedItemID + SetItemUnEquippedByMe, vt +0x7C | Mac StripEquippedItems imm 0x14, vt 0x80 | 0x7736D0 (row) | GOG 0x413620 -> 0x7678F0 |
| 0x4213C0 | `SetThrownLightsaber` | UpdateAreaId: `[+0x20]->+0x108` into +0x3B4, no calls; callers SetArea, LoadArea | Mac UpdateAreaId 37 B, 0x20/0x108/0x3B4 | 0x7754D0 (row) | GOG 0x4213A0 (label `SetEffectSpellId`, +0x3E0, caller ApplyLightsaberThrow; no Steam fp twin in the run) |
| 0x42BAE0 | `IsInjured` | stop rolling footstep sound: `[+0x448]`, `CALL 0x70af80` Stop, clears +0x450/+0x454; sole caller 0x42BB30 | Mac inlines it in UpdateRollingFootstepSound; Mac IsInjured = GetServerCreature + CSWSCreature::IsInjured | 0x77FAC0 (row) | GOG 0x421970 (label `HasFeat`) -> 0x775A80 (info) |
| 0x443170 | `CSWCCreature::SwitchMode` | CClientExoAppInternal::IndicateObject: 0x7F000000 default, `[EAX+0x40]`, CALL SetMainInterfaceTarget; caller CClientExoApp::IndicateObject | Mac CClientExoAppInternal::IndicateObject 53 B, same shape | 0x796B50 (row) | GOG 0x422D70 (FUN_, disp 0x3C0 x3, imm 2) -> 0x776DB0 (info) |

**Correct GOG labels whose gate picks were rotated (not label errors):** the thunk cluster 0x421820/40/60/80
(GetCameraFollowPart, GetFollowingBody, GetHeadHeightOffset, GetCameraOffset -> 0x775930/50/70/90; callees match the Mac
CSWCCreatureAppearance shapes), the head-turn getters 0x420070/0x4200D0 (-> 0x7741A0/0x774200), and the implant thunks
0x426430-0x426490 (PUSH 0x408/0x409/0x40B/0x40A, matching Mac per name).
