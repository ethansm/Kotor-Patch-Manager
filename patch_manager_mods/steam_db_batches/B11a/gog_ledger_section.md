## B11a

### CClientExoApp + CClientExoAppInternal (B11a: a template-instance cluster, the movie/video block, isolated errors)
B11a Opus gate parts a-e, r and z, plus the consolidation, 2026-10-03 (`B11a/opus/overrides_{a,b,c,d,e,r,z}.md`, `B11a/B11a_SUMMARY.md`).
Every Steam counterpart below is fp 1:1 with its GOG body (size, retn, n_insn, disp, imm, callee fps); only the GOG name is wrong.
All 16 rows are `label_suspect` in the B11a overrides and are not shipped under the GOG name.
The real Internal bodies were placed by following the GOG `CClientExoApp::X` forwarder (`MOV ECX,[this+4]; CALL Internal::X`) to its callee, then confirming against the Mac x86 symbol shape.

**Template-instance cluster (GOG 0x44DE70-0x44E0D0).** The GOG range 0x44DD60-0x44E910 holds compiler-generated
`CExoArrayList<T>` instances; 0x44DD60/0x44DDA0 `<CExoString>` ctor/dtor, 0x44E030 `<CSWGuiSkillStruct>` ctor and
0x44E7B0 `<CGuiStringStruct>::Allocate` are labelled correctly. Eight slots between them carry `CClientExoAppInternal`
names whose real bodies are elsewhere. Steam scatters the instances (three sit outside the Internal block).

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x44DE70 | `IsMoviePlaying` | `CExoArrayList<SJournalEntry>::~CExoArrayList`: `006d2cd2 CALL 0x6d2ee0` (vector delete, `006d2f1a PUSH 0x3c` element); callers = journal dtors 0x431750/0x4317F0/~CSWSJournal | Mac `~CExoArrayList<SJournalEntry>` (element 0x38); Mac IsMoviePlaying is 36 B, `+0x130` | 0x6D2CB0 | GOG 0x445220 -> Steam 0x798C00 (shipped) |
| 0x44DEB0 | `GetPausedByCombat` | 21 B `CExoLinkedList` RemoveTail wrapper (`005936dc CALL 0x0073a2a0`) | Mac GetPausedByCombat is 45 B, disp 0x8, imm 0x2 | 0x5936D0 | GOG 0x448500 (FUN_, returns CServerExoApp::GetPauseState(2), called by SetPausedByCombat) -> Steam 0x79BF00 |
| 0x44DED0 | `InitializeMoviePlayer` (`_2`) | find-with-callback: `007a17f7 CALL [EBP+0xc]` in a loop, `RET 0x8`, no callees; callers SelectNearestObject/HandleMouseOver | Mac InitializeMoviePlayer is 72 B, `+0x130` | 0x7A17A0 | GOG 0x445250 -> Steam 0x798C30 (shipped) |
| 0x44DF50 | `GetNearestVideoMode` | `CExoArrayList<8-byte T>::SetSize`: `00838f93 CALL 0x9196fd` free, `00838fb7 CALL 0x7a1bf0` (= GOG 0x733D10 `<_FILETIME>::Allocate`), `RET 0x4`; callers ShowObject, ~CSWTrackFollower | Mac GetNearestVideoMode is 257 B, calls CAuroraInterface::GetNearestVideoMode | 0x838F70 | GOG 0x43A510 -> Steam 0x78DF40 |
| 0x44DFB0 | `HandleExitGameButton` | `CExoArrayList<CSWRumblePattern>` ctor: `007a18f1 CALL 0x7a1ca0` (Allocate, 0x24 B elements); sole caller the Internal ctor 0x42C9E0 | Mac HandleExitGameButton is 76 B: GetInGameGui + ExitProgram, `+0x250/+0x254/+0xA0` | 0x7A18C0 | GOG 0x4433C0 (label `CSWGuiSkillFlowChart::BuildSortedPowerList`, 94 B, same 2 callees and disp, no direct callers) -> Steam 0x796DA0 (shape only) |
| 0x44DFF0 | `GetLoadScreenVisible` | `CExoArrayList<CSWRumblePattern>::~CExoArrayList`: `007a1922 CALL 0x7a1ae0` (`007a1b1a PUSH 0x24`); caller ~CClientExoAppInternal | Mac `~CExoArrayList<CSWRumblePattern>` imm 0x24; Mac GetLoadScreenVisible is 50 B, `+0x274/+0x278` | 0x7A1900 | GOG 0x444B60 -> Steam 0x798540 (that row is itself labelled IndicateObject, below) |
| 0x44E070 | `AddMoveToModuleMovie` (`_2`) | `CExoArrayList<12-byte T>::SetSize`: `008a6157 CALL 0x8a6060` (Allocate, imm 0xC; also called by the `<CSWGuiSkillStruct>` ctor); callers DoPassiveSelection, PlayRumblePattern | Mac AddMoveToModuleMovie is 39 B, `+0x2D8` | 0x8A6110 | GOG 0x445190 -> Steam 0x798B70 (shipped) |
| 0x44E0D0 | `SetGameOver` | `CExoArrayList<CGuiStringStruct>` ctor: `007a19d1 CALL 0x7a1df0` (= GOG 0x44E7B0 Allocate); sole caller the Internal ctor | Mac SetGameOver is 27 B, `+0x2C0/+0x2C4` | 0x7A19A0 | GOG 0x448830 (FUN_, 38 B, `+0x2C0/+0x2C4`; App::SetGameOver 0x40D880 forwards to it) -> Steam 0x79C230 |

**Video-mode / ignore-list / load-screen / movie block.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x43A510 | `EndLoadBar` | Internal::GetNearestVideoMode: memset + `0078dfa6 CALL 0x93ca40` (= GOG CAuroraInterface::GetNearestVideoMode), imm 0x9C/0xAC | Mac GetNearestVideoMode 257 B, imm 0x9C/0xBC; Mac Internal::EndLoadBar is a 5 B stub | 0x78DF40 | App::EndLoadBar 0x40C510 forwards to `return_void` 0x6133B0 (Steam fwd 0x7400D0 -> 0x623320): ICF-folded, no body of its own |
| 0x43A5E0 | `AddCreatureModelDirectlyToIgnoreList` | GetExactVideoMode wrapper (`0078e028 CALL 0x0093cd00` = GOG CAuroraInterface::GetExactVideoMode), `RET 0x10` | Mac AddCreatureModelDirectlyToIgnoreList 434 B | 0x78E010 | GOG 0x43A920 (`_2`, 450 B) -> Steam 0x78E350 (shipped) |
| 0x444B60 | `IndicateObject` | Internal::GetLoadScreenVisible: `00798572 CALL 0x410800` (PanelExists), `0079854a CMP [EAX+0x278]` | Mac GetLoadScreenVisible 50 B, `+0x274/+0x278`; Mac IndicateObject 53 B, `+0x40`, imm 0x7F000000 | 0x798540 | GOG 0x443170 (label `CSWCCreature::SwitchMode`, 70 B, `+0x40`, imm 0x7F000000; target of App::IndicateObject 0x40D6B0) -> Steam 0x796B50 |
| 0x4452F0 | `StopMovie` | Internal::ShutDownMoviePlayer: `00798cf2 CALL 0x93b570` IsMoviePlaying loop, `00798d09 CALL 0x93b550` CancelMovie, Sleep(5), ~CExoMoviePlayer, `[+0x130]=0` | Mac ShutDownMoviePlayer 138 B, 5x `+0x130`, imm 5; Mac StopMovie is a 5 B stub | 0x798CD0 | to locate (Mac stub; possibly ICF-folded) |
| 0x4381A0 | `ShutDownMoviePlayer` | scalar deleting dtor (CExoString members `+0xC`/`+0x30`; `0078bc1c AND EAX,1` / `CALL 0x9196fd` / `RET 4`) | see 0x4452F0 | 0x78BBC0 | GOG 0x4452F0 -> Steam 0x798CD0 |

**Isolated label errors.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x430300 | `StallServerCode` | CExoString getter on this+0x10 (`00783d93 ADD EAX,0x10` / `00783d9a CALL 0x7335f0`) | Mac StallServerCode 97 B, `+0x264` | 0x783D80 | to locate |
| 0x431750 | `DestroyQuestJournal_2` | member-array destructor (`00785162 ADD ECX,0x44` / `CALL 0x7a1740`, free) | duplicate label | 0x785130 | DestroyQuestJournal = GOG 0x4316E0 -> Steam 0x7850C0 (shipped) |
| 0x4317F0 | `AddToPauseExclusionList_2` | compiler dtor of 2 arrays (`00785203 CALL 0x64ddb0`, `00785212 CALL 0x6d2cb0` = the SJournalEntry list dtor above) | Mac AddToPauseExclusionList 185 B | 0x7851D0 | GOG 0x430CA0 -> Steam 0x784680 (shipped) |

Outside B11a rows (found while gating; GOG labels in other namespaces):
- 0x443170 `CSWCCreature::SwitchMode` = `CClientExoAppInternal::IndicateObject` (App::IndicateObject 0x40D6B0 forwards to
  it; Mac shape `+0x4/+0x40`, imm 0x7F000000) -> Steam 0x796B50.
- 0x4433C0 `CSWGuiSkillFlowChart::BuildSortedPowerList` has the shape of `CClientExoAppInternal::HandleExitGameButton` -> Steam 0x796DA0.
  This is a shape match only.
- 0x448500 and 0x448830 are unnamed in GOG. They are Internal::GetPausedByCombat and Internal::SetGameOver.
- Not errors:
  - 0x4F0520 `CSWPlayerControlCamRelative::GetPlayerWalking` is an ICF-folded 17 B `[ecx+0x18]` getter that App::GetModule also uses.
  - 0x733D10 / 0x7337D0 / 0x7E4430 `CExoArrayList<_FILETIME>` are ICF-shared 8-byte-element instances.

Not errors in the audited windows:
- 45 AUTO rows near the rotations were checked against the Mac symbol of the same name, and all pass. Of these, 34 are App getters and forwarders in GOG 0x40C110-0x40D9B0. The other 11 are Internal rows: QueryExitProgram, InitializeClientOptions, ShowLoadScreen, SetPlayerCharacterName, IsPlayerCreature, SetPlayerCreature, PlayLoadModuleMovies, WaitForMovie, RestoreMousePositionTo, DoScreenShot and UnloadSuperModelSet.
- Mac Internal offsets above +0x3C0 are Windows - 4. This is uniform layout drift.

**GOG DB param sizes (B11a).** 8 rows have a GOG DB `param_size_bytes` that disagrees with the GOG function's RET n
(gate_report `G4 metadata flags`), for example:
- DisplayMainMenu: 0 vs RET 4;
- StartMenuMusic: 0 vs RET 0xC;
- PlayMovie: 8 vs RET 0x14;
- ActionIconToMouseCursor: 4 vs RET 8.

Steam ships its own RET n.
