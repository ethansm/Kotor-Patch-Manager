# Opus gate B10a part a: evidence

Method: GOG/Steam/Mac x86 `fp.tsv` rows (size, retn, n_insn, disp, imm, callees), caller sets on GOG and Mac, Steam
callees named by fp-exact match to the GOG callee, Steam asm quoted from `steam_6a522e71/decomp`. "fp 1:1" = size,
retn, n_insn, disp and imm all identical between GOG and Steam. Mac vtable slots are Windows +4.

## 100% rows

### 1-5. The Animation* window (GOG 0x474420-0x474a70 -> Steam 0x7ef2c0-0x7ef910, delta +0x37aea0)

The 13 GOG bodies and the 13 Steam bodies line up 1:1 by position and size (132, 129, 129, 129, 197, 207, 131, 68,
104, 129, 129, 129, 273). The 129-byte bodies all have the same shape, so fp cannot name them. Each one reads its own
`animations.2da` column handle. The handles are written by Steam 0x6eb670, which looks up the columns in alphabetical
order (decomp: "Attack" ... "Walking"):

| handle | column (store) | Steam body | GOG body (label) | name decided |
|---|---|---|---|---|
| 0xa0fe28 | Stationary (006ebad1) | 0x7ef2c0 | 0x474420 (Stationary) | Stationary (B0, REVIEW) |
| 0xa0fe2c | Pause (006eba3f) | 0x7ef350 `007ef3a7 MOV EAX,[0x00a0fe2c]` | 0x4744b0 (**PlayOutOfPlace**) | Pause |
| 0xa0fe30 | Walking (006ebb20) | 0x7ef3e0 `007ef437 MOV EAX,[0x00a0fe30]` | 0x474540 (**HideEquippedItems**) | Walking |
| 0xa0fe34 | Running (006eba88) | 0x7ef470 `007ef4c7 MOV EAX,[0x00a0fe34]` | 0x4745d0 (**Walking**) | Running |
| 0xa0fe24 | Looping (006eb91b) | 0x7ef500 | 0x474660 (Looping) | Looping (AUTO) |
| 0xa0fe1c | FireForget (006eb889) | 0x7ef5d0 | 0x474730 (FireAndForget) | FireAndForget (AUTO) |
| 0xa0fe20 | Overlay (006eb9ad) | 0x7ef6a0 `007ef6f9 MOV EAX,[0x00a0fe20]` | 0x474800 (Overlay) | Overlay (AUTO) |
| none | (id ranges 0x4b0-0x52f, 0x640-0x6bf) | 0x7ef730 `007ef752 CMP EDX,0x640` | 0x474890 (**Pause**) | PlayOutOfPlace |
| (in callees) | Dialog | 0x7ef780 | 0x4748e0 (Dialog) | Dialog (AUTO) |
| 0xa0fe3c | Parry (006eb9f6) | 0x7ef7f0 | 0x474950 (Parry) | Parry |
| 0xa0fe4c | Attack (006eb765) | 0x7ef880 | 0x4749e0 (Attack) | Attack |
| 0xa0fe50 | HideEquippedItems (006eb8d2) | 0x7ef910 `007ef967 MOV EAX,[0x00a0fe50]` | 0x474a70 (**Running**) | HideEquippedItems |

The callers confirm it against Mac:
- Mac `AnimationPlayOutOfPlace` is 51 B, has no callee and only imm 0x80/0xfffff9c0 (an id-range test). Its caller is
  AnimateFireAndForget. GOG 0x474890 is the 68 B no-callee range test, and AnimateFireAndForget calls it.
- Mac `AnimationPause` is called by AnimationUpdate and DriveAnimationUpdate. GOG 0x4744b0 has exactly those 2 callers.
- Mac `TestAndReEquipweapons` calls `AnimationHideEquippedItems`. GOG TestAndReEquipweapons 0x474040 calls 0x474a70, and
  Steam `007eeef8 CALL 0x007ef910`.

The 5 labels Pause/PlayOutOfPlace/HideEquippedItems/Walking/Running are a permutation of the 5 bodies. The gate picks
are the B0 `semantic:` addresses (B0 overrides.tsv lines 85-95, already in the branch DB). They name each Steam body by
its column, so all 5 are **verified (semantic)** at the gate pick. The reviewer `corrected_addr` values are the body
counterparts, which are right as positions but carry the wrong names. No Steam address is held twice: the 5 rows map
onto the 5 bodies one to one.

| # | GOG | GOG label | final Steam | body counterpart of this GOG addr |
|---|---|---|---|---|
| 1 | 0x4744b0 | PlayOutOfPlace | 0x7ef730 | 0x7ef350 (= Pause) |
| 2 | 0x474540 | HideEquippedItems | 0x7ef910 | 0x7ef3e0 (= Walking) |
| 3 | 0x4745d0 | Walking | 0x7ef3e0 | 0x7ef470 (= Running) |
| 4 | 0x474890 | Pause | 0x7ef350 | 0x7ef730 (= PlayOutOfPlace) |
| 5 | 0x474a70 | Running | 0x7ef470 | 0x7ef910 (= HideEquippedItems) |

### 6. 0x6a60a0 GetActionByGroupId -> 0x53f3c0: verified
fp 1:1 (209 B RET 8, disp 0x6c, imm 0xffff x2). `0053f44a CALL 0x005d7190` is GetAcceptableAction (GOG 0x6b3380, fp
62 B; B0 branch row), called inside a linked-list walk. Mac GetActionByGroupId calls GetAtPos/GetNext/GetAcceptableAction
with disp 0x6c. Callers: GOG ClearHostileActionsVersus + UpdateActionQueue, Mac the same 2. The `mac_rotation` flag is a
false alarm.

### 7. 0x6ad780 RemoveObjectFromDialog -> 0x546aa0: label_suspect
fp 1:1 (39 B, RET 0). Body: `00546ab7 CALL 0x0073f750` (GetInGameGui), then `00546abe CALL 0x007cadb0`, which is
CGuiInGame::IsAnimationPlayingInDialog (B0 semantic row: it walks [+0x1c0] and calls AnimationDialog). The only GOG caller
is IsDialogDelay. Mac IsDialogDelay calls `CGuiInGame::IsAnimationPlayingInDialog` directly, so this body is the Windows
forwarder: the sibling of `Global_thunk_ClientExoApp_IsSoundPlayingInDialog` (GOG 0x6ad730) and of 0x6ad7b0 (the
IsFadingInDialog forwarder). Mac RemoveObjectFromDialog calls RemovePlayer + StopDialog, and that body is GOG 0x6ad7e0
(`RemoveObjectFromDialog_2`, AUTO -> Steam 0x546b00: `00546b7f CALL 0x00546900` StopDialog,
`00546bad CALL 0x006c7910` RemovePlayer, caller StopDialog). The reviewer said label_suspect too.

### 8. 0x6adc90 AddMatchedExpressionString -> 0x546fb0: verified
fp 1:1 (144 B RET 4, imm 0x1c4). `00546fd8 PUSH 8` + new, CExoString copy 0x7335f0, `00547024 ADD ECX,0x1c4` + `CALL
0x0083ea60` (CExoArrayList Add). Mac: operator new, CExoString ctor, `CExoArrayList<CExoString*>::Add`, imm 0x1c4. There
is no layout difference: the imm sets are identical.

### 9. 0x6b3410 GetAQActionIDByID -> 0x5d7220: verified
fp 1:1 (388 B RET 8, 39 identical imm). Switch on the action type, `005d7277 CMP [EBP-0xc],0x46`, default 0xffff. Mac
GetAQActionIDByID uses imm 0x46 (and 0xb/0x18/0x1b/0x1e) and is called by ExecuteCommandGetCurrentAction +
UpdateActionQueue. GOG has those callers too (plus the inlined UpdateActionQueue helper 0x6a2600, see B12b).

## Sample rows (13)

| # | GOG | name | Steam | verdict | anchor |
|---|---|---|---|---|---|
| 10 | 0x472720 | Destructor | 0x7ed5c0 | verified | `007ed5ca CALL 0x007ed5f0` + `007ed5db CALL 0x009196fd` (delete); Mac D0 = dtor + delete |
| 11 | 0x475b50 | StopAllVisualEffects | 0x7f09f0 | verified | `007f0a26 CALL 0x00872300` (= GOG StopVisualEffect 0x4fdbe0, 337 insn) in a list loop at `007f09fc ADD ECX,0x6c` |
| 12 | 0x4726f0 | CSSyncHandler_Orientation | 0x7ed590 | verified | `007ed5a3 MOV EAX,[EDX+0x88]`; Mac slot 0x8c; Position uses 0x8c (Mac 0x90) |
| 13 | 0x475420 | RemoveAllFireForgetAnimations | 0x7f02c0 | verified | `007f02de MOVZX EDX,[ECX+0x5a]` + `007f02ea CALL 0x007f00b0` (fp= GOG RemoveTopFireForgetAnimation) |
| 14 | 0x472610 | GetPortraitId | 0x7ed4b0 | likely | `007ed4ba MOV AX,[EAX+0xb8]`; Mac disp 0xb8 |
| 15 | 0x472660 | TurnOffShadows | 0x7ed500 | verified | `007ed51e MOV EAX,[EDX+0xbc]`; Mac slot 0xc0 |
| 16 | 0x472630 | TurnOnShadows | 0x7ed4d0 | verified | `007ed4ee MOV EAX,[EDX+0xb8]`; Mac slot 0xbc |
| 17 | 0x4725e0 | GetPortrait | 0x7ed480 | verified | `007ed493 ADD ECX,0xa8` + `007ed499 CALL 0x0085e8c0` (fp= GOG CSWPortrait::GetPortrait) |
| 18 | 0x6b1160 | IsDirtyFlagSet | 0x54a480 | verified | `0054a4b0 AND EAX,[EDX+ECX*4+0x224]`; Set/Clear siblings OR/AND-NOT; caller UpdateState (Mac same) |
| 19 | 0x410c30 | SetId | 0x764f60 | likely | `00764f6d MOV [EAX+0x4],ECX`; Mac disp 0x4 |
| 20 | 0x477820 | DisableDistortion | 0x7f2680 | verified | `007f26a0 MOV EAX,[EDX+0xe8]`; Mac slot 0xec; Enable sibling 0xe4 |
| 21 | 0x410cb0 | GetCanSeeHeight | 0x764fe0 | likely | `00764fe7 FLD float ptr [0x0098b5c0]`; Mac 11 B constant getter |
| 22 | 0x6638a0 | GetPortraitId (S) | 0x53f000 | likely | `0053f00a MOV AX,[EAX+0x30]`; Mac disp 0x30 |

TurnOnShadows/TurnOffShadows: the reviewer's claim holds. TurnOnShadows = 0x7ed4d0 (vtable +0xb8) and TurnOffShadows =
0x7ed500 (+0xbc); Mac has +0xbc/+0xc0 (Windows +4). The CSWSObject member offsets match Mac exactly (0x30, 0x224, 0xe4);
no -4 shift anywhere in this part.

**Destructor split (lead note).** Steam 0x7ed5f0 is one 475 B body. GOG splits it into Destructor_2 0x472750 (96 B, no
RET) and Destructor_3 0x4727b0 (379 B): 96 + 379 = 475, and the Steam callees RemoveObjectFromHitcheckIgnoreList
(`007ed643 CALL 0x0073f8d0`) and KillAllVisualEffects (`007ed64b CALL 0x007f0a50`) sit in the head. Destructor_2 keeps
0x7ed5f0 (REVIEW C1 verified, no change). Destructor_3 was also `verified` at 0x7ed5f0 in `final_rows.tsv`, a duplicate.
It is rejected in `overrides_z.tsv` (GOG 0x4727b0 = Steam 0x7ed650, mid-function).

## AUTO meaning checks (8)

| # | GOG | name | Steam | route | result | anchor vs Mac |
|---|---|---|---|---|---|---|
| 23 | 0x6bd060 | AIActionSetCommandable | 0x5e0e70 | sig | pass | `005e0ea4 CMP [ECX+0x11ac],0`; disp 0xec/0x11ac = Mac (caller RunActions) |
| 24 | 0x475a80 | DeleteVisualEffect | 0x7f0920 | sig | pass | `007f09b5 CALL 0x00872300` StopVisualEffect; disp 0xc0/0xc4/0xdc = Mac 0xc0/0xc4/0xe0 |
| 25 | 0x6aa7c0 | DoSpellImmunity | 0x543ae0 | sig | pass | `00543b61 MOV EDX,[ECX+0x1198]`, `00543cf8 CALL 0x00584da0` SendFeedbackMessage; 0x160/0x1e4 = Mac |
| 26 | 0x472a30 | ReplaceTexture_2 | 0x7ed8d0 | sig | pass | `007ed8fe MOV EAX,[EDX+0x18]` = Mac ReplaceTexture overload with disp 0x18 |
| 27 | 0x474800 | AnimationOverlay | 0x7ef6a0 | sig | pass | `007ef6f9 MOV EAX,[0x00a0fe20]` = 'Overlay' column |
| 28 | 0x6b0c50 | ReportOverflow | 0x549f70 | c2 | pass | `0054a13b MOV EAX,[EDX+0x7c]`; disp 0x68-0x7c, imm 0xa3 = Mac; caller UpdateState |
| 29 | 0x6a70f0 | SaveActionQueue | 0x540410 | c2 | pass | `00540423 PUSH 0x99462c`; ActionList/ActionId/GroupActionId/NumParams = Mac (caller SaveObjectState) |
| 30 | 0x477360 | SetFeedbackInfo | 0x7f2200 | c2 | pass | `007f220e ADD ECX,0xe8`, `007f221c MOV [ECX+0xf0],0x1388` = Mac 0xe8/0xf0 |

8 of 8 pass.

## Counts

30 rows: verified 25 (incl. the 5 semantic Animation rows and the 8 AUTO), likely 4, label_suspect 1 (0x6ad780).
Every sampled reviewer `match` (13) has the right address and the right name.

sample_errors: 0
