# B11a Opus gate, part a (60 rows, all "100%" section)

Verdicts: verified 51, likely 3, label_suspect 6, rejected 0, unresolved 0.
Corrections vs the part's steam_addr: 23 (rows 2,3,5-20,23,26,35,48,49). Every correction was body-checked against the GOG fp.

## Method
- Sequence-aligned the GOG and Steam fp tables over CClientExoApp (0x40c3d0-0x40e7c0 / 0x73ef30-0x741300) and
  CClientExoAppInternal (0x42dd90-0x43b150 / 0x781810-0x78eba0). The fingerprint is size, ret, n_insn, disp, imm, and the callee fp.
- The App alignment is 1:1 with one gap per side. GOG EndLoadBar 0x40c510 (a return_void forwarder) and GetClientOptions 0x40ccf0
  have no slot in this Steam range; Steam has an extra 0x7400d0.
- The Internal alignment is 1:1 except for GOG GetYAxis 0x4309a0 and ~CServerInfo 0x438080, plus extra Steam 0x78cf90 (16-byte return-1 stub) and 0x78eb80.
- Each forwarder is anchored by its CALL target. In every row the forwarded Steam body has the same fp as the GOG Internal body
  (size, ret, disp, imm). The CALL line is quoted in the TSV.
- The nine Get*ByGameObjectID getters were matched one by one on the vtable displacement, cross-checked with the Mac x86 names (Mac = Win + 4).

## Per row (short)
| # | GOG | name | final | verdict | anchor |
|---|---|---|---|---|---|
| 1 | 0x40c3d0 | App ctor | 0x73ef30 | verified | CALL 0x780460 sz3132, imm 0x4dc |
| 2 | 0x40c470 | GetFPS | 0x73efd0 | likely | sz17 FLD [0xa10724]; was 0x73f410 = GetConnectionLib |
| 3 | 0x40c4f0 | BeginIntro | 0x73f050 | verified | CALL 0x7813a0 sz481 imm 0x7d0 |
| 4 | 0x40c730 | CreateModule | 0x73f270 | verified | CALL 0x785380 sz132 imm 0x13c |
| 5 | 0x40cd90 | AddObjectToHitcheckIgnoreList | 0x73f8b0 | verified | callee sz141 + alloc 0x919723 |
| 6 | 0x40cdb0 | RemoveObjectFromHitcheckIgnoreList | 0x73f8d0 | verified | callee sz132 + free 0x9196fd |
| 7 | 0x40ce40 | ContinueMessageProcessing | 0x73f960 | likely | CALL 0x78cf90 return-1 stub (= GOG 0x61aac0 sz16) |
| 8 | 0x40ce60 | Initialize | 0x73f980 | verified | CALL 0x7819e0 sz511 imm 0x6f8 |
| 9 | 0x40cf30 | SetCaptureEvents | 0x73fa50 | verified | CALL 0x7858d0 sz635 |
| 10 | 0x40cf50 | SetCaptureEventPlayBack | 0x73fa70 | verified | CALL 0x785b50 sz586 |
| 11 | 0x40cf70 | SetCaptureEventPlayBackRate | 0x73fa90 | verified | CALL 0x785da0 sz66 disp 0x94 |
| 12 | 0x40cf90 | SetDebugMode | 0x73fab0 | verified | CALL 0x78d680 sz25 disp 0x194 |
| 13 | 0x40d070 | GetSWParty | 0x73fb90 | verified | [ECX+0x270] + 4 mapped callers |
| 14 | 0x40d1c0 | RestartMoviesIfNecessary | 0x73fce0 | verified | CALL 0x78e250 sz109 disp 0x130 |
| 15 | 0x40d1e0 | GetApplicationId | 0x73fd00 | likely | sz27 global->[+0x18] |
| 16 | 0x40d250 | App::StopMenuMusic | 0x73fd70 | verified | callee disp 0x238 imm 0x1f4 |
| 17 | 0x40d270 | App::StopLoadMusic | 0x73fd90 | verified | callee disp 0x23c, Stop |
| 18 | 0x40d290 | AquireLoadMusic | 0x73fdb0 | verified | CALL 0x78ead0 sz168 imm 0x304 |
| 19 | 0x40d4c0 | GetPlayerCharacterName | 0x73ffe0 | verified | CALL 0x7985b0 sz32 imm 0x294 |
| 20 | 0x40d720 | BarkString | 0x740260 | verified | CALL 0x79bc40 sz125 r16 |
| 21 | 0x40d860 | RunDeathSequence | 0x7403a0 | verified | CALL 0x79c0e0 sz325 |
| 22 | 0x40db90 | StartPazaakGame | 0x7406d0 | verified | callee sz442 r20 imm 0xa974 |
| 23 | 0x40dbc0 | EndPazaakGame | 0x740700 | verified | callee sz436 r16, clears +0x70 |
| 24 | 0x40df80 | (Internal)GetPlayerMovementInput | 0x740ac0 | verified | CMP [EAX+0x2a0],0, imm 0x44, sz124 |
| 25 | 0x40e4f0 | StartCreditSequence | 0x741030 | verified | CALL 0x7a1130 sz397 disp 0x4c0 |
| 26 | 0x42dd90 | Int::GetModuleCamera | 0x781810 | verified | GetModule callee 0x726f80 + [EAX+0x40] |
| 27 | 0x42df60 | Int::Initialize | 0x7819e0 | verified | imm 0x6f8/0x154 sz511 |
| 28 | 0x430300 | Int::StallServerCode | 0x783d80 | label_suspect | returns CExoString copy of this+0x10 |
| 29 | 0x430a40 | Int::GetPauseState | 0x784420 | verified | MOVZX [EAX+0x1cc]; 2 calls from 0x784440 |
| 30 | 0x431750 | Int::DestroyQuestJournal_2 | 0x785130 | label_suspect | member-array dtor |
| 31 | 0x4317f0 | Int::AddToPauseExclusionList_2 | 0x7851d0 | label_suspect | compiler dtor (Free + array dtor) |
| 32 | 0x431850 | Int::CreateWorldJournal | 0x785230 | verified | PUSH 0x20/new/ctor into [+0x3c] |
| 33 | 0x4318e0 | Int::DestroyWorldJournal | 0x7852c0 | verified | ~CSWCWorldJournal + delete [+0x3c] |
| 34 | 0x436970 | Int::LoadItemDropSoundList | 0x78a420 | verified | 'inventorysnds' 0x99f6ec |
| 35 | 0x4381a0 | Int::ShutDownMoviePlayer | 0x78bbc0 | label_suspect | scalar deleting dtor (AND 1/JZ/free/RET 4) |
| 36 | 0x438230 | Int::CleanUpModuleServices | 0x78bc50 | verified | CALL CGuiInGame::ShutDown 0x7c7030 |
| 37 | 0x4382a0 | Int::GetPlayerByPlayerID | 0x78bcc0 | verified | [EAX+0x4] vs ByGameObjectID [EAX+0x28] |
| 38-46 | 0x438360..0x4386e0 | Int::Get{Item,Creature,Area,Trigger,Projectile,Placeable,Door,AoE,Sound}ByGameObjectID | 0x78bd80..0x78c100 | verified | vslots 34/2c/24/3c/44/4c/14/54/6c, each equal to GOG and to Mac-4 |
| 47 | 0x438750 | Int::AttachPlayerGameObjectToArea | 0x78c170 | verified | CALL GetPlayerCreature, new 0xcc |
| 48 | 0x4388e0 | Int::ResetDriveAcceleration | 0x78c300 | verified | [+0x2a0] vcall +0x2c; App fwd calls it |
| 49 | 0x438910 | Int::InitializeInGameGUI | 0x78c330 | verified | CALL CGuiInGame::Initialize sz5472 |
| 50 | 0x438930 | Int::UpdateCamera | 0x78c350 | verified | '%d %c %f %f %f\n' sz2883 |
| 51 | 0x4394c0 | Int::GetPlayerCreature | 0x78cee0 | verified | CALL App::GetCreatureByGameObjectID |
| 52 | 0x439570 | Int::ContinueMessageProcessing | 0x78cfa0 | verified | CALL ShutDownToMainMenu, imm 0x10bbd |
| 53 | 0x43a1c0 | Int::SetVideoMode | 0x78dbf0 | verified | CALL ResetInterfaceForSize 0x7cfca0 |
| 54 | 0x43a510 | Int::EndLoadBar | 0x78df40 | label_suspect | = Mac CClientExoAppInternal::GetNearestVideoMode |
| 55 | 0x43a5e0 | Int::AddCreatureModelDirectlyToIgnoreList | 0x78e010 | label_suspect | GetExactVideoMode wrapper |
| 56 | 0x43a610 | Int::SetCharacterFile | 0x78e040 | verified | new + memcpy into +0x1b4 |
| 57 | 0x43adb0 | Int::StopMenuMusic | 0x78e7e0 | verified | [+0x238], FadeAndStop(500) |
| 58 | 0x43ae40 | Int::FadeAndStopStreamingSoundSource | 0x78e870 | verified | timer/1000, FadeAndStop, Render |
| 59 | 0x43aec0 | Int::StartLoadMusic | 0x78e8f0 | verified | [+0x2dc]/[+0x23c], imm 0x304 |
| 60 | 0x43b020 | Int::StopLoadMusic | 0x78ea50 | verified | [+0x23c], Stop, dtor, delete |

## Label rotations / GOG label notes
- **ShutDownMoviePlayer** belongs at GOG 0x4452f0, currently labelled StopMovie (sz158, disp 0x130 x5, imm 4/5/0x10). This matches Mac
  ShutDownMoviePlayer (sz138, disp 0x130 x5, imm 5/0x10); Mac StopMovie is a 5-byte stub. GOG 0x4381a0 is a scalar deleting dtor
  (CExoString members at +0xc and +0x30).
- **AddCreatureModelDirectlyToIgnoreList** belongs at GOG 0x43a920 (labelled `_2`, sz450; Mac sz434). GOG 0x43a5e0 is a GetExactVideoMode wrapper.
- **EndLoadBar** (GOG 0x43a510) is CClientExoAppInternal::GetNearestVideoMode (Mac sz257 calls CAuroraInterface::GetNearestVideoMode).
  The real Internal EndLoadBar is a stub (Mac 5 bytes; GOG App::EndLoadBar forwards to return_void 0x6133b0).
- AddToPauseExclusionList_2 / DestroyQuestJournal_2 are spurious duplicate labels. The real AddToPauseExclusionList is GOG 0x430ca0 (Mac sz185).
- StallServerCode (0x430300) does not match Mac StallServerCode (sz97, disp 0x264). The body is a CExoString getter on this+0x10.

## Batch-level finding: both "accepted decoys" are decoy-key errors
- B11a-2-11 0x4381a0 -> 0x78bbc0 (decoy_prev): 0x78bbc0 is the true counterpart (fp identical: sz129 r4, imm 4/4/c/14/30, callees
  ~CExoString / return_void / free). The pipeline's 0x78bcc0 is GetPlayerByPlayerID.
- B11a-3-52 0x40d250 -> 0x73fd70 (decoy_prev): 0x73fd70 is the true StopMenuMusic forwarder (callee disp 0x238 imm 0x1f4). The pipeline's 0x73fd90 is StopLoadMusic.
- The reviewers were right both times. The decoy generator built "prev" decoys from a mis-slotted pipeline address.
- The "Internal 0x4381a0-0x439570 shifted by one" trap applies to cand_orig, not to the gated steam_addr. In this part only rows 35, 48 and 49 had a wrong gated address.

sample_errors: 0 (part a has no sampled rows; all 60 are 100%-section rows).
AUTO meaning checks: none in part a.
