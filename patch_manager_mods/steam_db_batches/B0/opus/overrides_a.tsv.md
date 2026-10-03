# B0 Opus gate, part a (74 rows, all from "Opus reads 100%")

Method per row:
- I compared the GOG fp row with the Steam fp row (size, retn, n_insn, disp, imm, callee count) for the pipeline address and for every reviewer `corrected_addr`.
- For thunks, I compared the GOG callee's fp with the Steam callee's fp.
- I checked the name against the Mac x86 symbol's shape.
- Every asm line quoted in `overrides_a.tsv` was re-checked on disk in `steam_6a522e71/decomp` (108 address-tagged lines plus 006ebad1, 0088253a and 0089af19, all present and as quoted).
- Short forms: "fp=" means the GOG and Steam fingerprints are identical in size, retn, n_insn, disp, imm and callee count; "Mac" is the Mac x86 symbol with the same name.

Verdicts: verified 58, likely 2, label_suspect 14, rejected 0, unresolved 0.
Steam address differs from the pipeline's (evidence.tsv) in 9 rows: 0x40e480, 0x410c80, 0x4744b0, 0x4dbe50, 0x588990, 0x605680, 0x605890, 0x61aa20, 0x66af00.

## Per-row evidence

| GOG | name | Steam | verdict | evidence (short) |
|---|---|---|---|---|
| 0x40d860 | CClientExoApp::RunDeathSequence | 0x7403a0 | verified | thunk fp=; callee 0x79c0e0 fp= GOG Internal 0x4486e0 (325 B) |
| 0x40dd60 | CClientExoApp::PlayerFlourishWeapons | 0x7408a0 | verified | thunk fp=; callee 0x79f430 fp= GOG Internal 0x44ba30 (642 B) |
| 0x40de60 | CClientExoApp::EnableVideoEffect | 0x7409a0 | verified | thunk fp=; callee 0x79e550 fp= GOG 0x44ab50 (1036 B) |
| 0x40de80 | CClientExoApp::DisableVideoEffect | 0x7409c0 | verified | thunk fp=; callee 0x79e960 fp= GOG 0x44af60; Mac imm -2 |
| 0x40e400 | CClientExoApp::SetMouseMode | 0x740f40 | verified | thunk fp=; callee 0x7a0490 fp= GOG 0x44ca90 |
| 0x40e480 | CClientExoApp::GetCharacterChangeInProgress | **0x740fc0** | verified | pipeline 0x740f00 is a busy-wait thunk (22 B, no 0x4ac); 0x740fc0 fp= (unique), `[[this+4]+0x4ac]`, setter 0x740fe0 next; Mac Get/Set pair (+0x4a8, Mac layout) |
| 0x410c30 | CSWCObject::SetId | 0x764f60 | likely | thin +4 setter; run SetId/ShowShadowBlob/HideShadowBlob/GetCanSeeHeight all fp=; fp twin 0x50e7b0 belongs to a different class (GOG twin 0x89d600 CAurTriangleBin) |
| 0x410c80 | CSWCObject::HideShadowBlob | **0x764fb0** | verified | pipeline 0x78bc50 (111 B, 4 callees) wrong; 0x764fb0 fp= (unique), vt+0xb0 on [this+0x68]; Mac vt 0xb4 (Mac vtable +4) |
| 0x44ef30 | CSWCArea::Constructor | 0x7a2590 | verified | 1741 B fp=, 2 callee anchors |
| 0x4550b0 | CSWCArea::Unknown_ReturnTrue_7Args | 0x7a86b0 | likely | stub `return 1; RET 0x1c`, unique in both binaries |
| 0x456040 | CSWCArea::AddWeatherObject | 0x7a9640 | label_suspect | counterpart fp=; body = Mac `AddMapPin` (imm 0x1c8, 1 callee). See systematic finding 1 |
| 0x456230 | CSWCArea::StopWeather | 0x7a9830 | verified | fp= unique; writes 0 to +0x1f5, 6000 to +0x1f0; Mac disp 0x1f0,0x1f5 |
| 0x457310 | CSWCArea::ClearAllMapPins | 0x7aa910 | label_suspect | counterpart fp=; body = Mac `ClearTilePathNodes` (disp 0x23c,0x240,0x244,0x248,0x24c,0x250) |
| 0x457880 | CSWCArea::AttachmentFromServerArea | 0x7aae80 | verified | fp= unique; `[+0x280]=arg`; Mac disp 0x280 |
| 0x462bc0 | CClientOptions::SetCameraMode | 0x7b5e50 | verified | fp=; mac_contra is only Mac-vs-Win layout (Mac 0x63/0x64, same control flow) |
| 0x472630 | CSWCObject::TurnOnShadows | 0x7ed4d0 | verified | fp= unique; vt+0xb8; Mac vt 0xbc (+4) |
| 0x474420 | CSWCObject::AnimationStationary | 0x7ef2c0 | verified | 'Stationary' 0x99af58 -> DAT 0xa0fe28, read at 007ef317 |
| 0x4744b0 | CSWCObject::AnimationPlayOutOfPlace | **0x7ef350** | label_suspect | known #10; body counterpart = 0x7ef350 (AnimationPause), not 0x7ef730 |
| 0x474890 | CSWCObject::AnimationPause | 0x7ef730 | label_suspect | known #9 (body = AnimationPlayOutOfPlace) |
| 0x4cdda0 | CGuiInGame::ShutDownGlobal | 0x7bdb10 | label_suspect | fp=; compiler `vector deleting destructor` of a 16-byte {CExoString,CExoString} (eh-vector-dtor PUSH 0x10, 2x ~CExoString at +0/+8, delete [p-4]); Mac ShutDownGlobal 189 B with disp 0xa0-0xb4 does not fit. Real ShutDownGlobal: to locate |
| 0x4dbda0 | CGuiInGame::IsCameraDialog | 0x7cb6a0 | label_suspect | fp=; forwarder to CSWGuiDialogLetterbox::SetFade (GOG 0x5af830 -> 0x8bb0e0) on [this+0x68], 4 args; Mac name not found (inlined?) |
| 0x4dbde0 | CGuiInGame::IsAnimationPlayingInDialog | 0x7cb6e0 | label_suspect | known #5 |
| 0x4dbe50 | CGuiInGame::IsFadingInDialog | **0x7cb750** | label_suspect | fp= at 0x7cb750 (pipeline 0x7cb720 has [+0x68] + callee: the GOG 0x4dbe20 body); body = Mac IsCameraDialog (27 B, disp 0x3c,0x40). Promotes the GOG_LABEL_SUSPECTS "Unverified" row |
| 0x4df1f0 | CGuiInGame::GetCanOrientInDialog | 0x7ceaf0 | label_suspect | known #7 |
| 0x4df580 | CGuiInGame::AddFloatyText | 0x7cee80 | verified | fp=; new CSWGuiFloatyText via 0x750f30; Mac disp 0x98, 1 callee |
| 0x4dfff0 | CGuiInGame::GetCanHeadFollowInDialog | 0x7cf8f0 | verified | fp=; 0xf28 (Mac 0x26167c) |
| 0x4e0c60 | CGuiInGame::GetMiniMapVisible | 0x7d0560 | label_suspect | known #1 |
| 0x4e0ca0 | CGuiInGame::RemoveGUIPopUps | 0x7d05a0 | label_suspect | known #2 |
| 0x53cf90 | CSWGuiMainMenu::OnLoadGamePicked | 0x8824f0 | verified | OPUS_REVIEW 4c; 'HD0:effects' at 0088253a |
| 0x5821d0 | CSWCAreaOfEffectObject::Destructor | 0x8ee7f0 | verified | deleting-dtor shape, fp= |
| 0x588990 | CSWSCreatureStats::GetClassByPosition | **0x89aef0** | verified | pipeline 0x844e40 = CSWCCreatureStats::GetClass shape (+0xf8, Mac 0x184b08); 0x89aef0 fp= (GUI-module copy of the server getter), +0x8d count, stride 0x28, +0xab = Mac CSWSCreatureStats layout |
| 0x59b420 | CSWGuiInGameInventory::OnPanelRemoved | 0x8a7520 | verified | fp= incl. 0x24b0/0x24b4/0x24bc; mac_contra = Mac layout (-0x54) |
| 0x5bce30 | CSWGuiUpgrade::OnPanelRemoved | 0x8cca70 | verified | fp=; base OnPanelRemoved 0x40f4c0; mac_contra = Mac layout |
| 0x5bd130 | CSWGuiUpgrade::SetDescription | 0x8ccd70 | verified | fp=; SetText 0x416e30; mac_contra = Mac layout |
| 0x5cfd00 | CSWGuiSaveLoad::Constructor | 0x850770 | verified | 2656 -> 3051 B: Steam adds CB_CLOUDSAVE (string Steam-only) and shifts members +0x2d0 (0x1068->0x1338, tail 0x1b70->0x1e40), same 21 strings count; R6 layout growth |
| 0x5d1bf0 | CSWGuiSaveLoad::VerifyLoadGame | 0x8528b0 | verified | only diff 0x1b70->0x1e40 (OPUS_REVIEW 4d) |
| 0x5d33b0 | CSWGuiSaveLoad::LoadGame | 0x853a00 | verified | only diff 0x1b70->0x1e40 (checked by multiset diff of disp/imm) |
| 0x5d86f0 | CSWGuiSaveLoad::LaunchMostRecentSave | 0x859270 | verified | fp=; calls LoadGame 0x853a00; mac_contra = Mac layout |
| 0x605680 | CExoString::Constructor_2 | **0x733570** | verified | pipeline 0x7338d0 is operator= (250 B); 0x733570 fp= char* ctor (strlen/new/strcpy) |
| 0x605890 | CExoString::Destructor | **0x733780** | verified | pipeline 0x733540 is the 33 B zero-init ctor; 0x733780 fp= (free + zero) |
| 0x61aa20 | CRes::Constructor | **0x7110d0** | verified | pipeline 0x7111a0 is the CRes dtor body; 0x7110d0 fp= (151 B) |
| 0x61aad0 | CRes::Destructor | 0x711170 | verified | deleting dtor -> 0x7111a0 + delete |
| 0x61af20 | CExoResMan::AddEncapsulatedResourceFile | 0x7115c0 | verified | AddKeyTable type 3 |
| 0x61af40 | CExoResMan::AddResourceImageFile | 0x7115e0 | verified | AddKeyTable type 4 |
| 0x61b890 | CExoResMan::Exists_2 | 0x711f30 | verified | fp=; GetKeyEntryFromTable |
| 0x61c540 | CExoResMan::GetResRefFromFile | 0x712be0 | verified | fp=; Find('.') + SubString |
| 0x61d550 | CExoResMan::ServiceFromEncapsulated | 0x713bf0 | verified | fp= 644 B |
| 0x61e490 | CExoResMan::ServiceFromImageRaw | 0x714b30 | verified | fp= 292 B |
| 0x61e5c0 | CExoResMan::ServiceFromResFileRaw | 0x714c60 | verified | fp= 370 B |
| 0x61f0a0 | CExoResMan::FileCopy | 0x715740 | verified | fp= 404 B |
| 0x623b60 | CResGFF::GetField_2 | 0x717a00 | verified | fp= 156 B |
| 0x6248c0 | CResGFF::GetList | 0x718760 | verified | type 0xf |
| 0x624970 | CResGFF::GetListCount | 0x718810 | verified | type 0xf |
| 0x624d60 | CResGFF::ReadFieldDWORD | 0x718c00 | verified | type 4 |
| 0x625240 | CResGFF::ReadFieldCExoLocString | 0x7190e0 | verified | type 0xc; mac_contra = compiler shape |
| 0x625b30..0x625e40 | CResGFF::WriteField BYTE/WORD/SHORT/DWORD/INT/FLOAT | 0x7199d0, 0x719ab0, 0x719b20, 0x719b90, 0x719bf0, 0x719ce0 | verified x6 | AddField type PUSH 0/2/3/4/5/8, each fp= with matching imm |
| 0x640920 | CVirtualMachine::Constructor | 0x6fd700 | verified | new(0x3d8) + Internal ctor |
| 0x640c20 | CVirtualMachine::StackPushFloat | 0x6fda00 | verified | callee fp= Internal::StackPushFloat |
| 0x640d50 | CVirtualMachine::StackPopCommand | 0x6fdb30 | verified | callee 0x703140 fp= GOG Internal 0x6463e0 (547 B) |
| 0x6421c0 | CVirtualMachineStack::LoadStack | 0x6fef80 | verified | GOG fp is split (chunk 0x642387, retn -1); strings Value/Stack/TotalSize/StackPointer/BasePointer on both; Mac 752 B |
| 0x645af0 | CVirtualMachineInternal::RunScriptFile | 0x702850 | verified | fp= 345 B |
| 0x646390 | CVirtualMachineInternal::StackPushObject | 0x7030f0 | verified | tag 6 |
| 0x64bac0 | CServerExoApp::GetGameObject | 0x51c0a0 | verified | fp=; CGameObjectArray::GetGameObject |
| 0x64c2f0 | CTwoDimArrays::GetSwoopUpgradeRowProvider | 0x51c8d0 | label_suspect | fp= unique; `[this+4]+0x1f0b4`, sits between CServerExoApp methods; fits CServerExoApp::GetPartyTable (Mac 0x324362) |
| 0x64ceb0 | CSWSEffectListHandler::Destructor | 0x51d4b0 | verified | deleting dtor, EffectListHandler vftable |
| 0x667600 | CSWSItem::GetPropertyByTypeExists | 0x603dd0 | label_suspect | fp=; body = Mac RestoreUsedActiveProperties (imm 2,3,4,5,0xa,0xff; +0x288) |
| 0x667710 | CSWSItem::RestoreUsedActiveProperties | 0x603ee0 | label_suspect | fp=; 63 B type-range predicate (x-0xa <= 0x2b, byte table), `this` unused; name unknown |
| 0x66af00 | CSWSItem::GetUpgradableType | **0x6076d0** | verified | pipeline 0x607790 (721 B) wrong; 0x6076d0 fp=, GetBaseItem callee; Mac disp 0xac,0x2f0 |
| 0x66d900 | CSWSItem::AddSubraceRequirements | 0x60a0d0 | verified | fp= 1056 B; PUSH 0x3d -> 0x603f60; Mac imm 0x3d, calls GetPropertyByTypeExists |

## Systematic findings (GOG label errors, for GOG_LABEL_SUSPECTS.md)
1. **CSWCArea map-pin family mislabelled as weather objects.**
   - Mac has `AddMapPin` (imm 0x1c8), `GetMapPin` (disp 0x108,0x10c,0x1c8,0x1cc) and `ClearAllMapPins` (disp 0x1c8..0x1d0), and no WeatherObject API.
   - The GOG labels are:
     - `AddWeatherObject` 0x456040 = AddMapPin;
     - `GetWeatherObject` 0x456060 (same disp as Mac GetMapPin) = GetMapPin;
     - `ClearWeatherObjects` 0x4560f0 (0x1c8) is likely ClearAllMapPins.
   - The GOG `ClearAllMapPins` 0x457310 is Mac `ClearTilePathNodes`.
   - Steam: 0x7a9640 = AddMapPin, 0x7aa910 = ClearTilePathNodes. GetMapPin and ClearAllMapPins are to re-gate.
2. **CGuiInGame dialog block, a second rotation** (same pattern as GOG_LABEL_SUSPECTS #1-#8):
   - 0x4dbda0 "IsCameraDialog" = a letterbox SetFade forwarder;
   - 0x4dbde0 = DialogGuiDoneScroll (known);
   - 0x4dbe20 "IsLoopingDialogStuntAnimation" = IsFadingInDialog by shape (Mac 42 B, [+0x68], 1 callee), Steam 0x7cb720 (not gated here);
   - 0x4dbe50 "IsFadingInDialog" = IsCameraDialog, Steam 0x7cb750.
3. **CSWSItem**:
   - 0x667600 is RestoreUsedActiveProperties.
   - The real GetPropertyByTypeExists is the unnamed GOG 0x667790 -> Steam 0x603f60. Evidence: fp equal (475 B); Mac shape disp 0x2,0x11,0x288,0x28c,0x2d4, imm 0xff; it is called with 0x3d by AddSubraceRequirements on Mac, GOG and Steam.
   - 0x667710 is an unnamed type predicate.
4. 0x64c2f0 is CServerExoApp::GetPartyTable, not a CTwoDimArrays method.
5. 0x4cdda0 is a compiler vector-deleting destructor, not ShutDownGlobal.

## mac_contra rows
All 6 (ClearAllMapPins, SetCameraMode, InGameInventory/Upgrade OnPanelRemoved, Upgrade SetDescription, LaunchMostRecentSave, ReadFieldCExoLocString) were re-checked:
- ClearAllMapPins is a real label error (finding 1).
- The others are Mac-vs-Windows layout or compiler differences with GOG == Steam exactly; none of them changes a verdict.

## AUTO meaning checks
None: part a has no AUTO rows.

## Sample
Part a contains no rows from "Sample of the other reviewer matches".

sample_errors: 0
