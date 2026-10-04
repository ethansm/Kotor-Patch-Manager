# Opus gate B11a part z: evidence (8 unresolved rows + AUTO rotation audit)

Method: a private `mktemp -d` scratch scan over the GOG, Steam and Mac x86 `fp.tsv` files.
- Key: the function's own fp (size, retn, n_insn, disp, imm), plus the fp of every callee larger than 20 B. CRT leaves such as `_free` 0x921321 / 0x9196fd differ only by address and are left out.
- Caller sets come from the `callees` column on both Windows builds.
- Each pick was then read in `steam_6a522e71/decomp`.
- "Unique" means exactly one Steam function has this key, and no other GOG function has it either, so there are no twins to separate.

Counts: verified 2, label_suspect 6, likely / rejected / unresolved 0. There are 7 address corrections: every row except MovieWindowDeactivated.

## App forwarders (rows 1-2)

1. **0x40c510 CClientExoApp::EndLoadBar -> 0x7400d0: verified (correction from 0x73f430).**
   - GOG body: a 22 B forwarder, `MOV ECX,[this+4]; CALL return_void 0x6133b0`.
   - Steam: exactly one App-style forwarder (sz22 r0 n10, disp 0x4) calls an empty 11 B body: `007400dd CALL 0x00623320`. Steam 0x623320 = GOG 0x6133b0 (11 B, 7 insns, no callees; 162 vs 161 callers).
   - GOG 0x40c510 has 7 callers and Steam 0x7400d0 has 7, with equal sizes (125, 508, 739, 3450, 4064, 8599, 178). Among them are QueryExitProgram 0x796ba0 (AUTO verified, fp 1:1, it calls 0x7400d0 where GOG calls 0x40c510), SetLoadBarProgress 0x79cfb0 and Internal MainLoop 0x781be0.
   - Mac `CClientExoApp::EndLoadBar` (25 B) forwards to Mac `CClientExoAppInternal::EndLoadBar`, a 5 B stub. The name fits.
   - The gate pick 0x73f430 is the GetModule forwarder (part d).
   - Steam put this forwarder out of slot, between GetMiniGame 0x7400a0 and PlayLoadModuleMovies 0x7400f0. That is the "extra Steam 0x7400d0" noted in part a.
   - Hook note: the body is a no-op, so a hook only sees the call.
2. **0x40e710 CClientExoApp::MovieWindowDeactivated -> 0x741250: verified (pipeline address kept).**
   - Fp 1:1 (sz22 r0).
   - `0074125d CALL 0x007a10f0`: 0x7a10f0 is Internal::MovieWindowDeactivated (GOG 0x44d6f0, verified in part b; `007a1105 CALL 0x0070bc60` is the SetSoundMode wrapper = GOG 0x63ac00).
   - Sole caller: GOG ExoMoviePlayerWindowProc 0x91f690 (693 B) = Steam 0x40c4a0 (693 B).
   - The twin MovieWindowActivated 0x741270 calls 0x7a1110 (the 22 B body = GOG 0x44d710), so the callee separates the pair.
   - Mac x86 has no symbol for this name (a Windows-only window hook). The reviewer said "unsure"; nothing contradicts the pick.

## Template-instance cluster GOG 0x44de70-0x44e0d0 (rows 3-8)

The GOG range 0x44dd60-0x44e910 is a block of compiler-generated `CExoArrayList<T>` instances (ctor, dtor, SetSize, Allocate, vector-delete). Its own correct labels show this: 0x44dd60/0x44dda0 `CExoArrayList<CExoString>` ctor/dtor, 0x44e030 `CExoArrayList<CSWGuiSkillStruct>` ctor, 0x44e7b0 `CExoArrayList<CGuiStringStruct>::Allocate`.

Six slots carry `CClientExoAppInternal` names that the Mac symbols place elsewhere. Every one of those names has its real body found at another GOG address, listed below.

The instances share one body shape per kind:
- ctor: sz63 r4, `CALL Allocate`;
- dtor: sz64 r0, `PUSH 3; CALL vecdel`;
- SetSize: sz91 r4.

The **callee** (element-type-specific) and the **callers** are the only anchors. Each row's (fp + callee fp) key is unique in both builds, and the caller sets match by size.

Steam does not keep the block together: three of the six counterparts sit at 0x6d2cb0, 0x838f70 and 0x8a6110, next to their other users. The pipeline's drift-based picks (0x7a1820 to 0x7a21d0) therefore all landed on neighbouring instances or on unrelated code:
- 0x7a1820 is the `<_FILETIME>` ctor (= GOG 0x7337d0, 6 callers);
- 0x7a1bf0 / 0x7a1ca0 / 0x7a1df0 are Allocate bodies;
- 0x7a2010 / 0x7a21d0 are unrelated.

| # | GOG | GOG label | Steam (final) | Callee anchor (Steam asm) | Callers GOG = Steam | Body is | Name really at |
|---|---|---|---|---|---|---|---|
| 3 | 0x44de70 | IsMoviePlaying(_2) | **0x6d2cb0** | `006d2ccd PUSH 0x3` / `006d2cd2 CALL 0x006d2ee0`: 188 B vector delete, `006d2f1a PUSH 0x3c` (elem 0x3c) = GOG 0x44e2d0, which GOG `CExoArrayList<SJournalEntry>::SetSize/Allocate` (0x547a10/0x547f20 = Steam 0x84c0a0/0x6d2fa0) also call | 6/6: 0x431750/0x4317f0/~CSWSJournal 0x778b60 + 3 unwinds = 0x785130/0x7851d0/0x6d0cb0 + 3 unwinds | `CExoArrayList<SJournalEntry>::~CExoArrayList` (Mac 0x1a89da; Mac elem 0x38) | GOG 0x445220 -> Steam 0x798c00 (part r) |
| 4 | 0x44df50 | GetNearestVideoMode | **0x838f70** | `00838f93 CALL 0x009196fd` (free when n==0), `00838fb7 CALL 0x007a1bf0` (166 B = GOG 0x733d10 `CExoArrayList<_FILETIME>::Allocate`), `00838fc2 MOV [ECX+0x4],EDX`, `00838fc8 RET 0x4` | 2/2: ShowObject 0x445390 + ~CSWTrackFollower 0x540b20 = 0x798d70 + 0x833de0 | `CExoArrayList<8-byte T>::SetSize` (the reviewer's "stride-8 SetSize" was right) | GOG 0x43a510 -> Steam 0x78df40 (part a, label EndLoadBar) |
| 5 | 0x44dfb0 | HandleExitGameButton | **0x7a18c0** | zero +0/+4/+8, `007a18f1 CALL 0x007a1ca0` (331 B Allocate = GOG 0x44e5b0, whose callee 0x7a1ae0 deletes 0x24 B elements), `007a18fc RET 0x4` | 1/1: Internal ctor 0x42c9e0 = 0x780460 | `CExoArrayList<CSWRumblePattern>::CExoArrayList(int)` | shape only: GOG 0x4433c0 (label `CSWGuiSkillFlowChart::BuildSortedPowerList`, 94 B, no direct callers, calls GetInGameGui + ExitProgram, disp 0x250/0x254/0xa0) = Mac HandleExitGameButton (76 B, same 2 callees, disp 0x250/0x254/0xa0) -> Steam 0x796da0 (fp 1:1) |
| 6 | 0x44dff0 | GetLoadScreenVisible | **0x7a1900** | `007a191d PUSH 0x3` / `007a1922 CALL 0x007a1ae0` (173 B = GOG 0x44e390, `007a1b1a PUSH 0x24` = Mac `~CExoArrayList<CSWRumblePattern>` imm 0x24) | 3/3: ~CClientExoAppInternal 0x42d620 + 2 unwinds = 0x7810a0 + 2 unwinds | `CExoArrayList<CSWRumblePattern>::~CExoArrayList` | GOG 0x444b60 -> Steam 0x798540 (part b, label IndicateObject) |
| 7 | 0x44e070 | AddMoveToModuleMovie(_2) | **0x8a6110** | `008a6133 CALL 0x009196fd`, `008a6157 CALL 0x008a6060` (174 B = GOG 0x44e700, imm 0xc; also called by the `<CSWGuiSkillStruct>` ctor 0x44e030 = 0x8a5f70), `008a6168 RET 0x4` | 3/3: DoPassiveSelection 0x446ae0, PlayRumblePattern 0x4499f0, 0x595cc0 = 0x79a4c0, 0x79d3f0, 0x8a6030 | `CExoArrayList<12-byte T>::SetSize` (ICF-shared) | GOG 0x445190 -> Steam 0x798b70 (fp 1:1, imm 0x2d8 = Mac disp 0x2d8) |
| 8 | 0x44e0d0 | SetGameOver | **0x7a19a0** | zero +0/+4/+8, `007a19d1 CALL 0x007a1df0` (352 B = GOG 0x44e7b0 `CExoArrayList<CGuiStringStruct>::Allocate`), `007a19dc RET 0x4` | 1/1: Internal ctor 0x42c9e0 = 0x780460 | `CExoArrayList<CGuiStringStruct>::CExoArrayList(int)` | GOG 0x448830 (FUN_, 38 B, writes +0x2c0/+0x2c4 = Mac SetGameOver; App::SetGameOver 0x40d880 forwards to it) -> Steam 0x79c230 (App fwd 0x7403c0 calls it) |

Why ship these as label_suspect, not unresolved:
- the counterpart is unique;
- it is anchored on a type-specific callee and the full caller set;
- the identical-looking siblings (other ctor/dtor/SetSize instances) differ in callee fp, so none of them is a symmetric twin.

The GOG names contradict both the bodies and the Mac symbols, which are 27-257 B member functions with Internal field offsets.

No conflicts:
- 0x6d2cb0, 0x838f70, 0x8a6110 and 0x7a19a0 appear in no batch's final rows.
- 0x7a18c0 and 0x7a1900 appear only as candidates of these same rows.

## AUTO rotation audit (job 2)

Scope: every AUTO row of `B11a/evidence.tsv` within +-0x400 of the suspect GOG ranges, that is:
- the App forwarder slot-shift block GOG 0x40c510-0x40d5b0, widened to 0x40c110-0x40d9b0;
- 0x43a510-0x43a920;
- 0x443170, 0x444b60, 0x4452f0;
- 0x44de70-0x44e0d0.

That gives **45 AUTO rows checked** (34 App, 11 Internal). The 0x43a510-0x43a920 window has no AUTO rows; all its rows were REVIEW/REJECT and were Opus-read in parts a/r. 4 rows (GetCaptureEvents, GetClientName, GetWorldTimer, GetPlayerCreatureId) were also Opus-read in part e.

Per row:
- GOG fp = Steam fp (45/45);
- callee fps equal;
- the GOG body checked against the Mac x86 symbol of the same name and namespace: field disp set, callee identity, string.

**0 flagged.** A layout offset of Mac = Windows - 4 above Internal +0x3c0 (0x3c4/0x3c0, 0x48c/0x488, 0x4c4/0x4c0, 0x4d8/0x4d4) is uniform layout drift, not a contradiction.

| GOG -> Steam | name | check vs Mac symbol | result |
|---|---|---|---|
| 0x40c530 -> 0x73f070 | App::GetQuestJournal | disp 0x4,0x38 = Mac | pass |
| 0x40c550 -> 0x73f090 | App::GetWorldJournal | 0x4,0x3c = Mac | pass |
| 0x40c660 -> 0x73f1a0 | App::ResetCharacterFile | no Mac symbol; 0x1b4/0x1b8/0x1bc + free, fp 1:1 | pass (no contradiction) |
| 0x40c770 -> 0x73f2b0 | App::GetCaptureEventFile | 0x88 = Mac | pass |
| 0x40c790 -> 0x73f2d0 | App::GetCaptureEvents | 0x8c = Mac (part e) | pass |
| 0x40c7b0 -> 0x73f2f0 | App::GetCaptureEventPlayBack | 0x90 = Mac | pass |
| 0x40c7d0 -> 0x73f310 | App::GetClientName | imm 0x140 + CExoString ctor = Mac (part e) | pass |
| 0x40c810 -> 0x73f350 | App::GetClientLanguage | 0x148 = Mac | pass |
| 0x40c850 -> 0x73f390 | App::GetImeUI | no Mac symbol; 0xe0 getter | pass (no contradiction) |
| 0x40c870 -> 0x73f3b0 | App::GetWorldTimer | 0x24 = Mac (part e) | pass |
| 0x40c890 -> 0x73f3d0 | App::GetPauseTimer | 0x2c = Mac | pass |
| 0x40c8b0 -> 0x73f3f0 | App::GetAnimationTimer | 0x30 = Mac | pass |
| 0x40c930 -> 0x73f470 | App::GetPlayerCreatureId | 0x20 = Mac (part e) | pass |
| 0x40c970 -> 0x73f4b0 | App::GetGameObjectArray | 0x14 = Mac | pass |
| 0x40c990 -> 0x73f4d0 | App::GetGameObject | 0x14 + CGameObjectArray::GetGameObject (0x53dfb0) = Mac | pass |
| 0x40cb10 -> 0x73f650 | App::SetInStartNewModule | 0x220 = Mac | pass |
| 0x40cc30 -> 0x73f770 | App::GetPlayModulePanel | 0x50 = Mac | pass |
| 0x40cc50 -> 0x73f790 | App::GetClientAIMaster | 0xc = Mac | pass |
| 0x40ccb0 -> 0x73f7f0 | App::GetNetLayer | 0x10 = Mac | pass |
| 0x40ccd0 -> 0x73f810 | App::GetSWCMessage | 0x150 = Mac | pass |
| 0x40cec0 -> 0x73f9e0 | App::CreatureAcquireItem | ClientToServerObjectId 0x51c8b0 + Server CreatureAcquireItem 0x51cc60 = Mac 2 callees | pass |
| 0x40d040 -> 0x73fb60 | App::SetApplicationIdsMatch | 0x208 = Mac | pass |
| 0x40d0f0 -> 0x73fc10 | App::GetSoundSetList | 0x1c4 = Mac | pass |
| 0x40d110 -> 0x73fc30 | App::GetItemDropSound | 0x1c8 + CResRef ctor = Mac | pass |
| 0x40d180 -> 0x73fca0 | App::GetLoadBarState | 0x18c = Mac | pass |
| 0x40d380 -> 0x73fea0 | App::GetSWGuiManager | 0x274 = Mac | pass |
| 0x40d3e0 -> 0x73ff00 | App::GetInputClass | 0x9c = Mac | pass |
| 0x40d520 -> 0x740040 | App::SetAreaNotReadyToDisplay | 0x288 = Mac | pass |
| 0x40d5d0 -> 0x740110 | App::PlayMovie | Internal::PlayMovie 0x798ba0 + CStr + EvaluateAchievementMovie = Mac 3 callees | pass |
| 0x40d680 -> 0x7401c0 | App::RemoveMoveToModuleMovies | imm 0x2d8 list + SetSize (Mac inlines, writes 0x2dc = list+4) | pass |
| 0x40d6d0 -> 0x740210 | App::ReattachCreatureToScene | 0x28c = Mac | pass |
| 0x40d8a0 -> 0x7403e0 | App::GetGameOver | 0x2c0 = Mac (same field as the real SetGameOver 0x448830) | pass |
| 0x40d930 -> 0x740470 | App::SetDialogSent | 0x230 = Mac | pass |
| 0x40d960 -> 0x7404a0 | App::SetDialogModelsSent | 0x234 = Mac | pass |
| 0x4431c0 -> 0x796ba0 | Int::QueryExitProgram | GetPlayerCreature, SetAutoPaused, SetPausedByCombat, 0xd8/0x138/0x2c0/0x384 = Mac; calls EndLoadBar fwd 0x7400d0 | pass |
| 0x443420 -> 0x796e00 | Int::InitializeClientOptions | SetEnableHardwareMouseCursor 0x7bc1b0, CExoIni r/w, str 'EnableHardwareMouse' = Mac | pass |
| 0x4447b0 -> 0x798190 | Int::ShowLoadScreen | StartLoadMusic 0x78e8f0, SetInputActive 0x797d90, StartGlobalFade 0x7cbb40, 0x274/0x278/0x30c = Mac | pass |
| 0x444ba0 -> 0x798580 | Int::SetPlayerCharacterName | CExoString::operator= on +0x290 = Mac | pass |
| 0x444bf0 -> 0x7985d0 | Int::IsPlayerCreature | ServerToClientObjectId + Server GetPlayerCreatureId, imm 0x7f000000 = Mac | pass |
| 0x444c80 -> 0x798660 | Int::SetPlayerCreature | CSWParty::AddCharacter 0x7e5730, 0x20/0x270 = Mac | pass |
| 0x444cc0 -> 0x7986a0 | Int::PlayLoadModuleMovies | ShowLoadScreen, GetWorldTimer, GetMoveToModuleMovies; disp set identical to Mac (0x130 x3, 0x1f8, 0x200) | pass |
| 0x444e40 -> 0x798820 | Int::WaitForMovie | SetInputActive, GetWorldTimer, GetInGameGui, IsMoviePlaying 0x798c00; 0x130/0x200/0x254 = Mac | pass |
| 0x44da90 -> 0x7a1490 | Int::RestoreMousePositionTo | stores 2 args at 0x3c8/0x3cc (Mac 0x3c4/0x3c8, uniform -4) | pass |
| 0x44dac0 -> 0x7a14c0 | Int::DoScreenShot | str 'K2_%05i.tga', 0x154/0x158 = Mac | pass |
| 0x44db90 -> 0x7a1590 | Int::UnloadSuperModelSet | 'SUPERMODELS:smsetc', RemoveEncapsulatedResourceFile = Mac; 0x4d8 (Mac 0x4d4) | pass |

Every AUTO row in the rotated windows is right. The rotations hit only REVIEW rows: the template cluster, the 0x43a5xx video-mode pair, 0x444b60, 0x4452f0 and 0x4381a0. The App slot shift moved the pipeline's G6 picks, not the AUTO fp matches.

## Side findings (not B11a rows, not shipped)
- GOG 0x4433c0 (label `CSWGuiSkillFlowChart::BuildSortedPowerList`) has the shape of Mac `CClientExoAppInternal::HandleExitGameButton` (see row 5) -> Steam 0x796da0. This is a shape match only.
- Real Internal SetGameOver is GOG 0x448830 (unnamed) -> Steam 0x79c230. Real Internal EndLoadBar is ICF-folded into `return_void` (GOG 0x6133b0 -> Steam 0x623320, 162 callers): nothing to hook.

`sample_errors: 0` (part z has no sample rows).
