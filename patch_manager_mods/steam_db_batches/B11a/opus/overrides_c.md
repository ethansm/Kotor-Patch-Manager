# B11a Opus gate, part c (rows 121-180, all `sample` section)

Verdicts: 60 verified, 0 likely, 0 label_suspect, 0 rejected, 0 corrections, 0 unresolved.

## Method (applied to every row)
1. GOG fp row vs Steam fp row: (size, retn, n_insn, disp, imm, #str, #callees) are equal on 58/60 rows. The 2 that differ are the AddFeedbackMessage pair: the upstream Steam fp truncates Internal 0x78f160 to 496 bytes (see row 169).
2. Forwarders (`MOV ECX,[EAX+4]; CALL Internal::X`): the forwarded callee's fp shape must equal the GOG callee's shape. Within +-0x600 of the Steam address, exactly one same-shape forwarder has a callee with that shape. The one exception is row 132 (Enable/Disable twins), settled by its store value and callers.
3. Callers: the sorted caller-size multiset of the GOG addr equals that of the Steam addr on 54/60 rows. The 6 differences are all one or two callers whose size grew; the caller counts are equal:
   - row 130: the truncated AddFeedbackMessage caller;
   - rows 150, 158: WinMain-level caller 3581->3611;
   - row 138: one caller 625->627;
   - rows 163, 179: 31/31 and 62/62 callers, of which 224->239 and 406->438 grew.
4. Spot-checked 33 reviewer-quoted asm lines and 7 string refs in the Steam dump. All exist and say what the reviewer claimed.

## Per-row evidence (short)
| # | name | steam | anchor |
|---|---|---|---|
| 121 | BusyWaitMenuMusic | 0x740ee0 | ->0x7a0580 = GOG 0x44cb80 (imm 0x2ee/0x9c4); twin MuteSound -> 0x7a0630 |
| 122 | UnloadModule | 0x73f250 | 0073f25d CALL 0x785410 = GOG 0x431a30 sz520 |
| 123 | TurnOffKeyboardModal | 0x73fe60 | ->0x797df0 = GOG 0x444410 sz199 |
| 124 | Internal::MovieWindowActivated | 0x7a1110 | 007a111d CALL 0x70bbe0 = ResumeMovieStreamingSounds shape; Deactivated -> 0x70bc60 |
| 125 | GetCanSendDialog | 0x740440 | ->0x79c5b0 sz1033 = GOG 0x448bb0 |
| 126 | GetPlayerByGameObjectID | 0x73f510 | 0078bd4e [EAX+0x28] = GOG disp 0x28 |
| 127 | GetDisplayCharacterForKey | 0x741300 | ->0x7a1380 = GOG 0x44d980 (imm 0xbe54) |
| 128 | TogglePartyFollow | 0x740840 | 007980c8 PUSH 'k_sup_solo' |
| 129 | ServerModeChange | 0x73f890 | 007856af MOV [ECX+0x14c] |
| 130 | GetDoorByGameObjectID | 0x73f5f0 | 0078c033 [EAX+0x14] = GOG disp {8,0x14,0x14} |
| 131 | IsSoundPlayingInDialog | 0x740290 | ->0x79bcc0 = GOG 0x4482c0 sz376 |
| 132 | EnableInput | 0x7402f0 | 007b339a MOV [+0x2bc],0 vs Disable 007b337a ,1; caller sz508 = GOG QueryExitProgram (only caller of GOG 0x40d7b0); Disable callers = LoadGame/OnModulePicked/ExecuteLoadModule on both sides |
| 133 | Internal::EmptyHitcheckIgnoreList | 0x78d220 | 0078d22c ADD ECX,0x190 |
| 134 | Internal::GetPlayerCharacterName | 0x7985b0 | 007985ba [+0x290]=0, 007985c7 ADD EAX,0x294 |
| 135 | ShutDownToMainMenu | 0x73fb40 | ->0x78e0f0 (UnloadArea+UnloadModule) |
| 136 | SetPlayerCreature | 0x73f910 | 00798673 CMP [+0x270] |
| 137 | PlayerFlourishWeapons | 0x7408a0 | ->0x79f430 sz642 = GOG 0x44ba30 |
| 138 | GetAutoPaused | 0x740960 | 0074096d [ECX+0x384]; SetAutoPaused 0079e4fa writes it |
| 139 | ShowMouse | 0x740fa0 | 007b393c NOT/AND [+0x3d8], 007b3963 AND 0x4a (GOG imm 0x4a); Hide 0x7b38d0 ORs, sz92 = GOG Hide |
| 140 | GetMoveToModuleMovies | 0x7401a0 | 007401ad ADD EAX,0x2d8 |
| 141 | HandleInputEvent | 0x73ff80 | 0073ff95 CALL 0x7b12c0 sz7087 |
| 142 | SetMouseMode | 0x740f40 | ->0x7a0490 (Hide/Show around mode) |
| 143 | SendSaveGameRequest | 0x73f6f0 | ->0x785620->0x87a430: 0087a494 PUSH 0x4 (GOG SaveGame imm 4); Load 0x87a4b0 PUSH 0x5 (GOG LoadGame imm 5) |
| 144 | BusyWaitMuteSound | 0x740f00 | ->0x7a0630; callers DoQuickLoad/LoadGame |
| 145 | AddFeedbackMessage | 0x73f100 | 0073f160 CALL 0x78f160 + CExoString ctor/dtor/== |
| 146 | GetClientMeleeAnimation | 0x740500 | ->0x79c9c0 RET 0x18 (Ranged RET 0x10) |
| 147 | Internal::SetDebugMode | 0x78d680 | 0078d68d MOV [+0x194],ECX |
| 148 | IndicateObject | 0x7401f0 | ->0x796b50 = GOG 0x443170 shape |
| 149 | SetPlayerCharacterName | 0x73ffc0 | ->0x798580 |
| 150 | StopServices | 0x73fb20 | ->0x78ac40 'HD0:MOVIES' 0078b66e |
| 151 | SetTexturePack | 0x73fbf0 | 0078d7f3 PUSH 'texpacks' |
| 152 | Internal::GetDefaultActions | 0x7b40f0 | 007b45db PUSH 'i_dialog' |
| 153 | Internal::ShutDownToMainMenu | 0x78e0f0 | 0078e190 CALL 0x7a5fa0 |
| 154 | Internal::StopCreditSequence | 0x7a12c0 | 007a12ca [+0x4c0]=0 |
| 155 | Internal::ReadVideoModeSettings | 0x78a7a0 | 0078a96b PUSH 'RefreshRate' |
| 156 | GetAreaByGameObjectID | 0x73f570 | 0078beb7 [EDX+0x24] |
| 157 | Internal::PlayMovieQueue | 0x7a0f50 | 007a0f7a CALL 0x7a0fb0 |
| 158 | MainLoop | 0x73f9c0 | ->0x781be0 sz8599 |
| 159 | Internal::AddSimpleObjectModelDirectlyToIgnoreList | 0x78e5a0 | 0078e5bb CALL GetGameObject, imm 0xff |
| 160 | ChangeCharacter | 0x740380 | ->0x797f30 sz355 |
| 161 | ServerToClientObjectId | 0x73fec0 | 0073fed1 CALL 0x798160 |
| 162 | GetNextLoadScreenHintSTRREF | 0x740dc0 | 0079fcbc PUSH 'loadscreenhints' |
| 163 | SetPausedByCombat | 0x740350 | 0074036a CALL 0x79bf40 |
| 164 | SetAutoPaused | 0x740930 | 0079e4fa MOV [+0x384] |
| 165 | Internal::AddItemModelDirectlyToIgnoreList | 0x78e520 | 0078e553 CMP EDX,0xfb |
| 166 | Destructor | 0x73eff0 | 0073f01a CALL 0x7810a0 sz767 + free |
| 167 | Internal::GetActivePauseState | 0x784440 | 0078444c CALL 0x784420 x2 |
| 168 | Internal::HandleMouseOver | 0x7b3a00 | 007b3c5d CALL 0x7a17a0, imm 0x2a8/0x4b4 |
| 169 | Internal::AddFeedbackMessage | 0x78f160 | 0078f1e4 CMP 0xeb + jump table 0x796788; PUSH 0x59b/0x59c/0x289 (all in GOG imm list); common exit 0x79663f ~= 0x78f160+30173. The upstream fp sz496 is a truncation |
| 170 | StopSoundPlayingInDialog | 0x7402b0 | ->0x79be40 |
| 171 | GetModuleCamera | 0x73f490 | ->0x781810: 0078182d [EAX+0x40] |
| 172 | SetStartMenuMusicOnMovieEnd | 0x73fdd0 | 0073fde8 MOV [+0x1ec] |
| 173 | IsClientImeUI | 0x73f370 | 0073f37f [ECX+0xdc] |
| 174 | GetAreaNotReadyToDisplay | 0x740020 | 0074002f [ECX+0x288] |
| 175 | SetLastPartyMemberTempKilled | 0x740400 | ->0x79c4e0 |
| 176 | GetPlayerMovementInput | 0x740aa0 | 00740aad CALL 0x740ac0 (00740acc [+0x2a0]) |
| 177 | GetDisableMovies | 0x73fcc0 | 0073fccf [ECX+0x1f8] |
| 178 | PlayLoadModuleMovies | 0x7400f0 | ->0x7986a0 |
| 179 | SetInputClass | 0x73fee0 | 0073fef5 CALL 0x7b3050 |
| 180 | GetServerInfo | 0x73f830 | 0073f83f [ECX+0x8] |

## Cross-part findings (not this part's rows; for the lead)
- **GetModuleCamera Internal (part a row 26, 0x42dd90 -> 0x7819e0) is wrong.**
  - 0x7819e0 is sz511 and equals GOG 0x42df60 Internal::Initialize.
  - The App forwarder 0x73f490 calls 0x781810: sz46, `0078182d MOV ECX,[EAX+0x40]`, matching GOG 0x42dd90 (sz46, disp 0x40, imm 0xc).
  - The correct Steam address is **0x781810**.
- **GOG label rotation for IndicateObject.**
  - GOG App::IndicateObject 0x40d6b0 forwards to GOG 0x443170. That function is labelled `CSWCCreature::SwitchMode`, but its shape (disp 0x4,0x40; imm 0x7f000000) matches Mac `CClientExoAppInternal::IndicateObject`.
  - GOG 0x444b60, labelled Internal::IndicateObject (disp 0x274/0x278), is a different function (cf. part list row 82, label_suspect).
- **FID/auto labels to ignore:** Steam 0x73f6f0 and 0x740440 (`SchedulerResourceManagement::Statistics`), and 0x78d680 (`UMSThreadScheduler::SetCompletionList`).

sample_errors: 0

AUTO meaning checks: none in this part.
