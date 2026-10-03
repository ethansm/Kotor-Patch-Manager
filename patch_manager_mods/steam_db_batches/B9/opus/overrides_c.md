# B9 Opus gate, part c (rows 121-174)

Gate: Opus (claude-opus-5-5), 2026-10-03. Brief: OPUS_GATE_BRIEF.md, DESIGN 4.6. Escalated batch: every reviewer match in
this part (46 `sample` rows) was read in full, plus the 8 AUTO meaning checks.

## Method
For each row I read four things: the evidence.tsv row, every reviewer row for the GOG addr (decoy cards labelled from
`cards/key.tsv`), the GOG fp, and the Steam fp + asm. Checks:
- **FPEQ**: size, retn, n_insn, disp multiset and imm multiset equal between GOG and Steam. Callee count and string count also checked.
- **Callers**: GOG callers mapped through trusted rows (Steam branch DB joined on the GOG DB name; B0/B15/B9 final rows
  marked with their source) must appear in the Steam caller set.
- **Mac x86 real symbols**: the same-name body (callee names, disp with Mac +4 vtable shift) and the ±4 class-order window.

Note: `fp.tsv` drops displacements > 0x10000. The CServerExoAppInternal fields (0x100xx) of the getters and setters are
therefore not visible on the GOG side. For those rows the discriminator is the Steam asm field offset, cross-checked
against trusted callers (mostly script `ExecuteCommand*` handlers, whose GOG names come from the command table).

## Per-row evidence (all FPEQ = identical unless stated)
| # | Class::Name | GOG -> Steam | anchor | verdict |
|---|---|---|---|---|
| 121 | CServerExoApp::GetTriggerByGameObjectID | 0x64bb60 -> 0x51c140 | fwd 0x537cc0, vt slot +0x40 (GOG 0x7baf10 +0x40, Mac +0x44); db caller GetNumMinesSetByPlayerParty | verified |
| 122 | CServerExoApp::SetMoveToModulePending | 0x64b870 -> 0x51be50 | 0051be68 [EDX+0x10080]; 4/4 db callers (StartNewModule cmd, 3 EventHandlers) | verified |
| 123 | CServerExoAppInternal::Initialize | 0x7b4920 -> 0x5315a0 | 'globalcat', ADD ECX,0x100fc, ReadCatalogue 0x653e40 (fp = GOG 0x650d00) | verified |
| 124 | CServerExoApp::GetWorldTimer | 0x64bd90 -> 0x51c370 | 0051c37f [ECX+0x10048]; 70=70 callers, 36/36 db hit | verified |
| 125 | CServerExoApp::TogglePauseState | 0x64c0d0 -> 0x51c6b0 | fwd 0x538c20 (fp = GOG Internal 0x7bbe70); db OnApply/OnRemoveTimestop | verified |
| 126 | CSWSModule::SaveModuleStart | 0x72dfe0 -> 0x55b830 | PUSH 0xbc1, 'MOD V1.0', 19 callees; GOG DB param 0 wrong (RET 8) | verified |
| 127 | CServerExoAppInternal::CopyModuleToCurrentGame | 0x7b4f60 -> 0x531be0 | CopyFileA + SetFileAttributesA(0x80), RET 0xc; GOG DB param 8 wrong (12) | verified |
| 128 | CServerExoApp::LoadGame | 0x64b840 -> 0x51be20 | fwd 0x52fb80 Internal::LoadGame (db); SaveGame twin -> 0x52f610 | verified |
| 129 | CSWSModule::SavePrimaryPlayerInfo | 0x72ffa0 -> 0x55d7f0 | 4/4 strings, 11 db callees IN, Mac callee set equal | verified |
| 130 | CServerExoApp::GetModule | 0x64b7b0 -> 0x51bd90 | fwd 0x52f4c0; 123=123 callers, 71/71 db hit | verified |
| 131 | CServerExoAppInternal::MovePlayerToArea | 0x7b8d50 -> 0x535a10 | imm 0x1008c/0x1f0b4; PositionParty + ComputeSafeLocation (db) IN | verified |
| 132 | CServerExoApp::GetFactionOfObject | 0x64c0b0 -> 0x51c690 | fwd 0x538320 (fp = GOG 0x7bb570 386/8); 6/6 db callers | verified |
| 133 | CServerExoAppInternal::StopServices | 0x7b9ef0 -> 0x536c10 | 7/7 strings ('HD0:chitin', 'ERRORTEX:'), resman callees, imm 0xf8 | verified |
| 134 | CServerExoApp::SaveGame | 0x64b810 -> 0x51bdf0 | fwd 0x52f610 Internal::SaveGame (db), RET 0x10 | verified |
| 135 | CSWSModule::GetWaypoint | 0x72a670 -> 0x557ec0 | CALL FindObjectByTagTypeOrdinal 0x55ec70 (Mac same); 4/4 callers incl GetWaypointByTag cmd | verified |
| 136 | CServerExoApp::SetLastConversation | 0x64c6c0 -> 0x51cca0 | 0051ccb9 ADD ECX,0x1f314 (GOG imm), operator=; db callers Door/Placeable EH | verified |
| 137 | CServerExoApp::SetButtonMashCheck | 0x64c870 -> 0x51ce50 | 0051ce68 [EDX+0x100d8]; caller ExecuteCommandSetButtonMashCheck (db) | verified |
| 138 | CServerExoApp::GetButtonMashCheck | 0x64c850 -> 0x51ce30 | 0051ce3f [ECX+0x100d8]; callers AddAttackAction + Get cmd (db) | verified |
| 139 | CServerExoAppInternal::GetModuleName | 0x7b2920 -> 0x52f5a0 | GetModule + module+0x104 copy, imm 0x104 = GOG/Mac | verified |
| 140 | CServerExoApp::GetPlaceableByGameObjectID | 0x64bb80 -> 0x51c160 | fwd 0x537d30 vt +0x48 (GOG/Mac +0x4c); 12/12 db callers | verified |
| 141 | CServerExoApp::GetLoadFromSaveGame | 0x64c800 -> 0x51cde0 | 0051cdef [ECX+0x100c4]; 4/4 db callers incl GetLoadFromSaveGame cmd | verified |
| 142 | CServerExoApp::GetItemByGameObjectID | 0x64bb00 -> 0x51c0e0 | fwd 0x537b00 vt +0x38; 109=109 callers, 37/37 db hit | verified |
| 143 | CServerExoApp::GetRoomIndex | 0x64c620 -> 0x51cc00 | GetModule/GetArea/CSWSArea::GetRoomIndex (Mac same); db caller CSWCArea::GetRoomIndex; GOG DB param 0 wrong (12) | verified |
| 144 | CServerExoApp::GetSoundObjectByGameObjectID | 0x64bc40 -> 0x51c220 | fwd 0x537f60 vt +0x68; 10/10 SoundObject cmd callers (db) | verified |
| 145 | CServerExoAppInternal::RunModule | 0x7b9510 -> 0x5361d0 | ADD ECX,0x1f0b4 + CreateParty (db); LoadPrimaryPlayer/MovePlayerToArea callees | verified |
| 146 | CSWSModule::IsObjectInLimbo | 0x731fc0 -> 0x55f930 | disp 0x1fc/0x200 (GOG = Mac), 0 callees, 90 B (twins 101 B); caller 0x5be950 fp = GOG AIActionJumpToPoint | verified |
| 147 | CServerExoAppInternal::StoreCurrentModule | 0x7b39e0 -> 0x530660 | 'GAMEINPROGRESS:', SaveModuleStart callee, caller StartNewModule (db) | verified |
| 148 | CSWSModule::PlotPathInArea | 0x72d430 -> 0x55ac80 | TestDirectLine/PlotGridPath/PlotPathPointPath callees (Mac same), imm 0x2b4/0x186a0 | verified |
| 149 | CServerExoApp::GetCreatureByGameObjectID | 0x64bb20 -> 0x51c100 | fwd 0x537b70 vt +0x30; 226/227 db-mapped callers hit | verified |
| 150 | CServerExoApp::GetDoorByGameObjectID | 0x64bbc0 -> 0x51c1a0 | fwd 0x537da0 vt +0x18; 6/6 db door callers | verified |
| 151 | CServerExoAppInternal::UpdateClientGameObjects | 0x7ba760 -> 0x537510 | loop CALL 0x537590 + timer; Mac callee set equal | verified |
| 152 | CServerExoApp::ComputeSafeLocation | 0x64c7d0 -> 0x51cdb0 | fwd 0x53a840 (fp = GOG 0x7bda90 106/20), RET 0x14; db caller PositionParty | verified |
| 153 | CServerExoApp::SetMoveToModuleString | 0x64b8c0 -> 0x51bea0 | 0051beb9 ADD ECX,0x10084 (GOG imm), operator=; 4/4 db callers | verified |
| 154 | CServerExoAppInternal::DoModuleEnterSaveCleanup | 0x7bfb10 -> 0x53cd70 | 4/4 strings ('DeleteSaveGroupOnEnter'), RemoveFile, imm 0x809 | verified |
| 155 | CServerExoApp::GetGlobalVariableTable | 0x64c2b0 -> 0x51c890 | 0051c89d ADD EAX,0x100fc (GOG imm; same member Initialize reads); 16/16 db | verified |
| 156 | CServerExoApp::SetExportAllCharactersPending | 0x64b8f0 -> 0x51bed0 | 0051bee8 [EDX+0x100bc]; caller ExportAllCharacters cmd (db) | verified |
| 157 | CServerExoApp::GetClientObjectByObjectId | 0x64c020 -> 0x51c600 | fwd 0x52f330 (fp = GOG 0x7b26b0); 108=108, 35/35 db | verified |
| 158 | CServerExoApp::GetServerInfo | 0x64b760 -> 0x51bd40 | 0051bd4f [ECX+0x10004]; db callers SetExperience/GetPotentialLevel/CanLevelUp | verified |
| 159 | CServerExoAppInternal::DealWithLoadGameError | 0x7b8970 -> 0x535630 | 00535647 CALL UnloadModule 0x5315e0, disp 0x14/0x38 = Mac | verified |
| 160 | CServerExoAppInternal::UpdateClientGameObjectsForPlayer | 0x7ba7e0 -> 0x537590 | imm 0x30d40/0x7f000000, [+0x350], disp list = GOG | verified |
| 161 | CServerExoApp::SetAutoSavePending | 0x64b970 -> 0x51bf50 | 0051bf68 [EDX+0x100b8]; caller DoSinglePlayerAutoSave cmd (db) | verified |
| 162 | CServerExoAppInternal::GetItemByGameObjectID | 0x7bad50 -> 0x537b00 | GetGameObject + vtable +0x38 (GOG 0x38, Mac 0x3c) | verified |
| 163 | CSWSModule::SaveModuleFAC | 0x72f420 -> 0x55cc70 | 3/3 strings ('FactionList', 'RepList'), PUSH 0x7f6 + WriteGFFFile (db) | verified |
| 164 | CServerExoApp::GetPlayerCreatureId | 0x64c310 -> 0x51c8f0 | fwd 0x539d60 (fp = GOG 0x7bcfb0 87/0); 117=117, 42/42 db | verified |
| 165 | CServerExoApp::GetFactionManager | 0x64c6a0 -> 0x51cc80 | 0051cc8f [ECX+0x10054]; 5/5 db faction callers | verified |
| 166 | CSWSModule::PlotInterAreaPath | 0x72d0b0 -> 0x55a900 | CALL PlotPathInArea 0x55ac80; disp 0x70..0xb8 list = GOG; caller PlotPath | verified |
| 167-174 | AUTO rows | see table below | | verified |

Reviewer side notes ("DB label SaveRepute / SaveMultiplayerPlayer / SaveModuleForGameInProgress / ServerApp::GetPlaceableById")
refer to alias names that are not in the branch Steam DB or the GOG DB (checked read-only), so these rows add no duplicate address.
Rows 124, 128, 134, 141, 149, 155, 164, 174 are already in the branch Steam DB at the same address; they agree.

## ±4 label sweep (GOG class order vs Mac x86 real symbols)
- **CServerExoApp**: the GOG order matches the Mac order slot for slot in every window of my rows, except for 2 Mac-only
  symbols (GetApplicationId, GetPartyTable). Mac also places ClientToServerObjectId after IsInjured. Shapes agree, with no
  forwarder-vs-getter swap. All 11 GetXByGameObjectID forwarders call the same-named Internal function. The Internal
  vtable slots line up with Mac (+4) for all 11 types (Door 0x18, Area 0x28, Creature 0x30, Item 0x38, Trigger 0x40,
  Placeable 0x48, AoE 0x50, Waypoint 0x58, Encounter 0x5c, Store 0x64, Sound 0x68). No rotation.
- **CServerExoAppInternal**: gcc orders this class very differently, so a positional comparison says nothing. Every row's name
  is fixed by content instead: strings, callee sets equal to the Mac same-name callee sets, vt slots. No rotation.
- **CSWSModule**: same order apart from gcc moves. Callee sets and disp lists match the Mac same-name bodies. No rotation.
- label_suspect: none in this part.

## AUTO meaning checks (5 sig + 3 c2)
| # | route | Class::Name | GOG -> Steam | check | result |
|---|---|---|---|---|---|
| 167 | sig | CServerExoAppInternal::LoadPrimaryPlayer | 0x7b46f0 -> 0x531370 | callees GetModule/LoadCharacterFinish/GetPrimaryPlayerIndex = Mac; disp 0x8/0x44 = Mac | PASS |
| 168 | sig | CServerExoAppInternal::UpdateMapData | 0x7bdb00 -> 0x53a8b0 | GetModule + AreaMap setters (Mac), disp 0x238; caller MainLoop | PASS |
| 169 | sig | CServerExoAppInternal::ResolvePlayerByFirstName | 0x7bc8a0 -> 0x539650 | 10 callees, disp 0x30/0x8c (Mac 0x34/0x90); caller is the CServerExoApp forwarder | PASS |
| 170 | sig | CSWSModule::TimeStopSanityCheck | 0x732020 -> 0x55f990 | callees GetCreature/TogglePauseState/GetActiveExclusionList = Mac; 0x148 | PASS |
| 171 | sig | CServerExoAppInternal::GetWaypointByGameObjectID | 0x7bb0d0 -> 0x537e80 | vtable +0x58 (Mac +0x5c), distinct from AoE/Encounter | PASS |
| 172 | c2 | CServerExoApp::ClearAllQueuedCombatActions | 0x64c5c0 -> 0x51cba0 | disp 0x10dc + RemoveAllActions (Mac); db caller CSWCObject::ClearAllQueuedCombatActions | PASS |
| 173 | c2 | CSWSModule::RemoveModuleResources | 0x732700 -> 0x560070 | 23/23 callees, 9/9 strings, 3 Module callers | PASS |
| 174 | c2 | CServerExoAppInternal::SaveGame | 0x7b2990 -> 0x52f610 | 'SAVES:%06d - %s', 'SAVEGAME', screenshot fmt; RET 0x10 (GOG DB param 12 wrong) | PASS |

AUTO meaning checks: 8/8 pass.

## Totals
verified 54 (46 sample + 8 AUTO), likely 0, label_suspect 0, rejected 0, corrections 0, unresolved 0.

GOG DB param sizes contradicted by RET (record only, Steam DB uses the RET value): SaveModuleStart 0 -> 8,
CopyModuleToCurrentGame 8 -> 12, CServerExoApp::GetRoomIndex 0 -> 12, Internal::SaveGame 12 -> 16.

sample_errors: 0
