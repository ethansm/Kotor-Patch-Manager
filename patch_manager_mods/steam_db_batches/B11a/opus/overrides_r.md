# B11a Opus gate part r (14 REJECT + 3 NOCAND rows)

`B11a/opus/part_r.md` wasn't on disk when this ran. The 17 rows come from `gate_report.md` (REJECT list) and `evidence.tsv` (`gate_class` REJECT/NOCAND).
Method: a private-scratch whole-binary fp search (score = retn + size + mapped-callee Jaccard + imm/disp/str Jaccard; callee map = unique exact fps + validated rows from B0/B9/B12a/B12b/B15/B16). Every pick was then checked in the Steam asm and against Mac x86 disp sets, plus caller topology.

## Systematic finding
In CClientExoAppInternal, the gate picks are shifted by one function across the whole region: each REJECT pick is the neighbour's body.
- DisplayMainMenu took GetModuleCamera, and HandleMessage took DisplayMainMenu.
- ShowObject took HideLoadScreen, and HideLoadScreen took SetInputActive.
- SetMouseMode took GetNextLoadScreenHintSTRREF.
- StartMenuMusic took StopMenuMusic, and AquireLoadMusic took AddCreatureModelDirectlyToIgnoreList_2.

The GOG fp retn equals the Steam retn at every corrected pick. The `gog_db_param_bad` flags are GOG DB param-size errors, not body differences.

## Music cluster bijection (GOG 0x43a5e0-0x43b150 to Steam 0x78e010-0x78eba0)
The assignment is strictly monotone and one-to-one: AddCreatureModel..(0x43a920) 0x78e350, AddItemModel.. 0x78e520, AddSimpleObjectModel.. 0x78e5a0, **StartMenuMusic 0x78e620**, StopMenuMusic 0x78e7e0, FadeAndStopStreamingSoundSource 0x78e870, StartLoadMusic 0x78e8f0, StopLoadMusic 0x78ea50, **AquireLoadMusic 0x78ead0**, GetObjectName 0x78eba0.
- Each address is its row's unique top match.
- No Steam address is used twice.
- The final_rows addresses of the unresolved neighbours (StopMenuMusic, Fade, StartLoadMusic, StopLoadMusic, GetObjectName) are these same addresses.

## Rows

| GOG | Name | gate pick | final | Anchor |
|---|---|---|---|---|
| 0x0040d990 | CClientExoApp::GetClientRangedAnimation | 0x740930 (SetAutoPaused fwd: callers OnPauseButtonPressed etc.) | 0x007404d0 verified | `007404e9 CALL 0x0079ca90` (Internal::GetClientRangedAnimation, sz130 RET 0x10, disp 0x400/0xc5 = Mac); RET 0x10; sole caller IsInTotalDefense 0x6db460 (GOG caller 0x657710) |
| 0x0042db10 | CClientExoAppInternal::DisplayMainMenu | 0x781810 | 0x00781590 verified | `0078173c PUSH 0x1ca8`, `007817c1 CALL 0x0078e620` (StartMenuMusic), RET 4; disp 0x274/0x280/0x284/0x49c |
| 0x0042ddc0 | CClientExoAppInternal::HandleMessage | 0x781590 | 0x00781840 verified | `007818cb CALL 0x0073f3b0` GetWorldTimer, `007818e6 PUSH 0x99f4a4` "%d %c %d %d %d ", RET 0x10; Mac disp 0x150/0x1c0 |
| 0x00431cc0 | CClientExoAppInternal::ServerModeChange | 0x785030 | 0x007856a0 verified | `007856af MOV word ptr [ECX+0x14c],AX`, RET 4; Mac disp 0x14c; only caller is forwarder 0x73f890 (GOG fwd 0x40cd70). Thin setter, but the offset is unique and matches Mac |
| 0x004353b0 | CClientExoAppInternal::StartServices | NOCAND | 0x00788d90 verified | RET 0x14; CALLs CreateQuestJournal 0x785030, CreateWorldJournal 0x785230, SetTexturePack 0x78d6a0, InitializeMoviePlayer 0x798c30, LoadOptions 0x79f6f0, CNetLayer/CTlkTable/CExoResMan ctors; imm/disp sets nearly identical; size +208 (Steam growth, imm 0x594 to 0x5b4) |
| 0x00438300 | CClientExoAppInternal::GetPlayerByGameObjectID | NOCAND | 0x0078bd20 verified | symmetric pair with GetPlayerByPlayerID: `0078bd4e MOV ECX,[EAX+0x28]` (Mac GetPlayerByGameObjectID disp 0x28), and twin 0x78bcc0 reads `[EAX+0x4]` (Mac GetPlayerByPlayerID disp 0x4). Callers are fwd 0x73f510 and 0x73f7d0 respectively |
| 0x00439c70 | CClientExoAppInternal::SetTexturePack | 0x78cf50 | 0x0078d6a0 verified | strings "TEXTUREPACKS:", "texpacks", "DynMemRatio"; `0078da5d CALL 0x0093c7a0` CAuroraInterface::SetTexturePack; RET 8; called from StartServices |
| 0x0043abf0 | CClientExoAppInternal::StartMenuMusic | 0x78e7e0 | 0x0078e620 verified | `0078e648 PUSH 0x99f77c` "mus_sion", `0078e6cb CALL 0x0078e7e0` StopMenuMusic, RET 0xc; streaming-sound Play/SetVolume/SetPriorityGroup |
| 0x0043b0a0 | CClientExoAppInternal::AquireLoadMusic | 0x78e350 | 0x0078ead0 verified | `0078eb02 CMP [EAX+0x23c],0`, `0078eb29 ADD EDX,0x304`, `0078eb33 CALL 0x00734d60` CompareNoCase, RET 4; Mac disp 0x23c/0x304 |
| 0x00444a00 | CClientExoAppInternal::HideLoadScreen | 0x797d90 | 0x007983e0 verified | `00798446 CALL 0x007cbb40` StartGlobalFade, `00798509 CALL 0x00409ed0` UpdateScreen, `007983f6 CMP [EAX+0x278],0`, RET 0xc; Mac disp 0x274/0x278/0x40 |
| 0x004451c0 | CClientExoAppInternal::PlayMovie | 0x798580 | 0x00798ba0 verified | `00798bc6 CALL 0x00798b70` AddMoveToModuleMovie, `00798be4 ADD EDX,0x2d8`, `00798bee CALL 0x007a0fb0` PlayMoviesInExoArrayList, RET 0x14 |
| 0x00445220 | CClientExoAppInternal::IsMoviePlaying | 0x798c00 | 0x00798c00 verified | `00798c20 CALL 0x0093b570`. The CExoMoviePlayer ladder is GOG 9211a0/c0/e0/921210/30 to Steam 93b550/570/590/5c0/5e0, all same sizes, so 93b570 = IsMoviePlaying. CancelMovie twin 0x7a0500 calls 93b550. CClientExoApp::IsMoviePlaying fwd 0x740160 calls 0x798c00. G6 "trusted_holder=IsMoviePlaying" is the same function under its bare name, not a conflict |
| 0x00445390 | CClientExoAppInternal::ShowObject | 0x7983e0 | 0x00798d70 verified | sz5780 RET 0x10; callers DoPassiveSelection 0x79a4c0 and SelectNearestObject 0x79b700 (= GOG callers); disp set identical (35), imm identical except 0x356a vs 0x355a; str ".\\clientexoappinternal.cpp" |
| 0x00447d00 | CClientExoAppInternal::SelectNearestObject | NOCAND | 0x0079b700 verified | `0079bbf9 CALL 0x00798d70` ShowObject, `0079b82d CALL 0x00778510` CanSee, RET 8; disp 0x2a8/0x2ac/0x2b4 = Mac |
| 0x0044c240 | CClientExoAppInternal::GetNextLoadScreenHintSTRREF | 0x79f770 | 0x0079fc40 verified | strings "loadscreenhints", "loadstoryhints"; `0079ff09 CALL 0x0051d070` GetModulePreviouslyVisited; byte field +0x490/+0x491; RET 8 |
| 0x0044ca90 | CClientExoAppInternal::SetMouseMode | 0x79fc40 | 0x007a0490 verified | `007a049c CALL 0x007b38d0` HideMouse, `007a04e9 CALL 0x007b3930` ShowMouse, `[ECX+0x274]`, RET 4; only caller is fwd CClientExoApp::SetMouseMode 0x740f40 |
| 0x00460b10 | CClientExoAppInternal::ActionIconToMouseCursor | 0x7b3ca0 | 0x007b3ee0 verified | `007b3f37 CMP [EBP-0xc],0x3e9`, `007b3f50 SUB EDX,0x3f4`; same 22-value switch imm set as GOG, no callees, RET 8; sole caller HandleMouseOver 0x7b3a00 (= GOG 0x460630) |

## Notes for other parts (not decided here)
- 0x7404d0 is also the final_rows pick of CClientExoApp::BarkString (0x40d720) and EndPazaakGame (0x40dbc0), both unresolved. It belongs to GetClientRangedAnimation. Whole-binary best for BarkString is 0x740260, which calls Internal::BarkString 0x79bc40.
- GetPlayerByPlayerID 0x4382a0 should be 0x78bcc0 (it reads +0x4). ShutDownMoviePlayer 0x4381a0 is nearest 0x78bbc0 (score 3.4); the reviewer's 0x78bd20/0x78bcc0 picks for it are wrong.

sample_errors: 0 (not applicable to part r)
