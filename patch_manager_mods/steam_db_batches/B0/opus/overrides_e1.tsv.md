# B0 Opus gate, part e1 (102 escalation rows: reviewer `match`, 100% read after the sample error)

Gate: Opus, 2026-10-03, following `OPUS_GATE_BRIEF.md`. Verdict rows are in `overrides_e1.tsv`.

## Method

For each row:
- Compared the GOG and Steam `fp.tsv` rows (size, retn, n_insn, disp, imm, callee count and string count).
- Matched every GOG callee to the Steam callee with the same fingerprint, and counted how many functions in each binary share that callee key.
- Read the GOG neighbours (±3) and Steam neighbours with their trusted mappings, from `B0/final_rows.tsv` and `steam_db_pr3/verified.tsv`.
- Found the Mac x86 symbol of the same name and compared its shape: callee names, disp and imm.
- Read the Steam asm or decompile.

Label check:
- A script scored each GOG body against the Mac bodies of its own name and of the GOG neighbours' names (±4).
- For the CClientExoApp thunks it also scored the GOG `CClientExoAppInternal` callee against the Mac Internal of the same name.
- Every row where a neighbour's Mac body fit better, or where no Mac symbol exists, was settled by hand.

Asm quotes: every asm line quoted in `overrides_e1.tsv` (156 lines) was checked on disk in `steam_6a522e71/decomp` with operand values. A negative test (a mutated offset and a mutated address) was caught.

Short forms:
- "fp=": the GOG and Steam fingerprints are identical.
- "unique": no other function in either binary has that key.
- "Mac": the Mac x86 symbol of the same name.

Result:
- 97 of the 102 pipeline pairs are fp=.
- The other 5 differ only by the GOG reference to the empty string 0x9858f5: CGameEffect ctor x2, AppendToMsgBuffer, CSWCObject::GetName, and the GetObjectName callee.
- HideGalaxyMapGui is the sixth special case. GOG 224 B + the 15-B tail chunk = Steam 239 B.

Verdicts: **verified 81, likely 16, label_suspect 5, rejected 0, unresolved 0**. The Steam address differs from the pipeline's in 3 rows: 0x474540, 0x4745d0 and 0x474a70. All three are label_suspect, with the body counterpart given.

## Per-row evidence

| GOG | name (GOG DB) | Steam | verdict | evidence (short) |
|---|---|---|---|---|
| 0x40c5a0 | CClientExoApp::GetObjectName | 0x73f0e0 | verified | thunk fp=; callee 0x78eba0 = Internal 0x43b150 (1424 B, only empty-str diff); Mac Internal::GetObjectName best fit (0.51 vs <=0.11) |
| 0x40c990 | GetGameObject | 0x73f4d0 | verified | calls trusted CGameObjectArray::GetGameObject; disp 0x4,0x14 = Mac |
| 0x40ca10 | GetCreatureByGameObjectID | 0x73f550 | verified | callee 0x78bdf0 fp= Internal 0x4383d0, vt+0x2c (Item +0x34, Area +0x24); Mac Creature vt 0x30 (+4) |
| 0x40cc10 | GetInGameGui | 0x73f750 | likely | getter [[this+4]+0x40]; Mac disp 0x4,0x40 |
| 0x40ccf0 | GetClientOptions | 0x72fb00 | verified | ICF fold. The Steam CClientExoApp block goes GetSWCMessage 0x73f810 -> GetServerInfo 0x73f830 with no slot for it. The Steam GetMiniMapVisible 0x7d05a0 (fp= GOG 0x4e0ca0, which calls 0x40ccf0) calls 0x72fb00. Hooking it also hits OptionsHolder::GetGameOptions |
| 0x40cdf0 | SetPlayerCreature | 0x73f910 | verified | callee fp= Internal 0x444c80; Mac Internal same name |
| 0x40d020 | ShutDownToMainMenu | 0x73fb40 | verified | callee fp= Internal 0x43a6c0 (339 B) |
| 0x40d0d0 | SetTexturePack | 0x73fbf0 | verified | callee fp= Internal 0x439c70 (1345 B) |
| 0x40d300 | QueryExitProgram | 0x73fe20 | verified | callee fp= Internal 0x4431c0 (508 B) |
| 0x40d3c0 | SetInputClass | 0x73fee0 | verified | callee fp= Internal 0x45fc60 (774 B) |
| 0x40d640 | AddMoveToModuleMovie | 0x740180 | verified | callee fp= Internal 0x445190 |
| 0x40d750 | IsSoundPlayingInDialog | 0x740290 | verified | callee fp= Internal 0x4482c0 |
| 0x40d790 | DisableInput | 0x7402d0 | verified | callee 0x7b3370 writes [+0x2bc]=1 (part c settled the Enable/Disable twin) |
| 0x40d7d0 | DismissInGameGUI | 0x740310 | verified | callee fp= Internal 0x45ffc0 |
| 0x40d840 | ChangeCharacter | 0x740380 | verified | callee fp= Internal 0x444550 |
| 0x40d880 | SetGameOver | 0x7403c0 | verified | callee 0x79c230 fp= GOG FUN_00448830: disp 0x2c0,0x2c4 = Mac Internal::SetGameOver; GetGameOver 0x7403e0 reads +0x2c0 |
| 0x40d8e0 | SetCanSendDialog | 0x740420 | verified | callee fp= Internal 0x448b50 |
| 0x40ddb0 | GetGUIString | 0x7408f0 | verified | callee fp= Internal 0x44a810 |
| 0x40de20 | GetAutoPaused | 0x740960 | likely | bit getter [[this+4]+0x384]&1; Mac disp 0x384 |
| 0x61afd0 | CExoResMan::AddFixedKeyTableFile | 0x711670 | verified | AddKeyTable type 1 (Encapsulated 3, Image 4, Directory 2) |
| 0x61aff0 | AddResourceDirectory | 0x711690 | verified | AddKeyTable type 2; Mac imm 0x2 |
| 0x61b010 | RemoveEncapsulatedResourceFile | 0x7116b0 | verified | RemoveKeyTable type 3 |
| 0x61b070 | RemoveResourceDirectory | 0x711710 | verified | RemoveKeyTable type 2 |
| 0x61b090 | UpdateResourceDirectory | 0x711730 | verified | UpdateKeyTable type 2 |
| 0x61bf10 | GetResOfType | 0x7125b0 | verified | Merge/GetResOfTypeFromTable/~CExoStringList callees = Mac 346-B overload |
| 0x61c5f0 | GetResTypeFromFile | 0x712c90 | verified | Find('.') + SubString + GetResTypeFromExtension, all 6 callees = Mac |
| 0x61c7b0 | GetTableCount | 0x712e50 | verified | GetTable + GetEntryCount = Mac |
| 0x61c7f0 | GetIsStaticType | 0x712e90 | verified | type switch 0x7d9..0x80a and 0xbbb = Mac bounds |
| 0x61d910 | ServiceFromResFile | 0x713fb0 | verified | Malloc callee; Mac uses GetResourceExtension like this body (Raw/Image/Encapsulated twins settled in part a) |
| 0x61de00 | UpdateKeyTable | 0x7144a0 | verified | CompareNoCase + RebuildTable = Mac |
| 0x61f040 | CleanDirectory | 0x7156e0 | verified | WipeDirectory p3=0 (keeps the directory) |
| 0x61f070 | NukeDirectory | 0x715710 | verified | WipeDirectory p3=1 -> RemoveDirectoryA |
| 0x620700 | CacheResource | 0x716da0 | verified | overload wrapper of CacheResource 0x716de0 (part c); Mac has one overload only |
| 0x4eaed0 | CExoString::Constructor | 0x733540 | verified | zero-init default ctor (unique 33 B); Mac CExoString() 21 B disp 0x4 |
| 0x605790 | CExoString::Constructor_4 | 0x733680 | verified | (char*,len): new + strncpy = Mac 0x2ecdf4 |
| 0x7345e0 | CGameEffect::Constructor | 0x5e4920 | verified | plain ctor (id counter +0x20, SetNumIntegers(8)); Mac 290 B |
| 0x734780 | CGameEffect::Constructor_2 | 0x5e4ac0 | verified | copy ctor (reads src +0x30); Mac 305 B |
| 0x734b20 | GetInteger | 0x5e4e60 | likely | [+0x34][i], RET 4; Mac 0x34 |
| 0x734b40 | SetInteger | 0x5e4e80 | likely | RET 8 store; Mac 0x34 |
| 0x734b60 | GetFloat | 0x5e4ea0 | likely | FLD [+0x38+i*4]; Mac 0x38 |
| 0x734b80 | SetFloat | 0x5e4ec0 | likely | FSTP [+0x38+i*4]; Mac 0x38 |
| 0x734ba0 | GetObjectID | 0x5e4ee0 | likely | [+0x78+i*4]; Mac 0x78 (Set twin RET 8, part c) |
| 0x734c40 | SetExpiryTime | 0x5e4f80 | likely | writes +0x10/+0x14; Get twin 0x5e4fa0 reads them (part c) |
| 0x4ce720 | CGuiInGame::AppendToMsgBuffer | 0x7be090 | verified | 0x3f/0x40 cap on +0x11c; combat twin +0x118/+0x124 |
| 0x4d8430 | GetMessageBufferSize | 0x7c7d40 | likely | getter +0x11c; Mac 0x11c |
| 0x4da930 | NextSWInGameGui | 0x7ca230 | verified | +1 with wrap at 8 |
| 0x4daac0 | PrevSWInGameGui | 0x7ca3c0 | verified | -1 with wrap to 7; Mac Prev imm 7 |
| 0x4db020 | HideGalaxyMapGui | 0x7ca920 | verified | SetSoundMode tail lives in the GOG chunk 0x4db100 (see GOG label errors) |
| 0x4dbba0 | HideLoadModuleDebugMenu | 0x7cb4a0 | verified | panel +0x70 = Mac |
| 0x4dbc40 | HidePowersFeatsSkillsDebugMenu | 0x7cb540 | verified | panel +0x74 = Mac |
| 0x4dc340 | IsGlobalFading | 0x7cbc40 | verified | IsFadeDone on +0x6c = Mac |
| 0x4de2e0 | HideUpgradeScreen | 0x7cdbe0 | verified | +0x88 and 4 callees = Mac; Chemical twin +0x8c |
| 0x4df250 | RePopulateMainInterface | 0x7ceb50 | verified | PopulateMenus on +0x98 = Mac |
| 0x668830 | CItemRepository::Destructor | 0x605000 | verified | ~CExoArrayList on +0xc; Mac deletes [+0xc] |
| 0x6695b0 | GetItemInRepository | 0x605d80 | verified | 3 callees incl. self = Mac |
| 0x6696b0 | RemoveItem | 0x605e80 | verified | DelIndex + bit-7 clear on +0x2c8 = Mac |
| 0x669770 | ClearNewItems | 0x605f40 | verified | GetItemByGameObjectID + bit-7 clear = Mac |
| 0x6699d0 | CalculateContentsWeight | 0x6061a0 | verified | ItemListGetItem + GetWeight = Mac |
| 0x669a30 | ClearItems | 0x606200 | verified | SetSize(0) on +0xc array, count +4 = 0 |
| 0x61ab40 | CRes::Demand | 0x7111e0 | verified | -> CExoResMan::Demand = Mac |
| 0x61ac10 | CRes::Release | 0x7112b0 | verified | -> CExoResMan::Release = Mac |
| 0x561f80 | CResGFF::Destructor | 0x716fa0 | verified | deleting dtor -> trusted ~CResGFF 0x7171b0 |
| 0x622ff0 | CResGFF::Constructor | 0x716e60 | verified | default ctor (CRes ctor + 0x286/0x72c/0xfd4); Mac 222-B ctor |
| 0x623710 | OnResourceFreed | 0x7175b0 | verified | 9-field clear, disp set = Mac |
| 0x6247f0 | GetElementType | 0x718690 | verified | GetStruct callee = Mac |
| 0x624820 | GetFieldCount | 0x7186c0 | likely | pointer overload ([p+8] or -1); Mac only has the GetStruct overload (GOG 0x624840) |
| 0x624870 | GetFieldType | 0x718710 | verified | GetFieldByLabel + GetField = Mac |
| 0x624b30 | GetTopLevelStruct | 0x7189d0 | likely | stub: out=0, return 1 |
| 0x624b60 | ReadFieldBYTE | 0x718a00 | verified | type 0 (CHAR twin type 1) |
| 0x624c60 | ReadFieldWORD | 0x718b00 | verified | type 2 |
| 0x6257a0 | ReadFieldVOID | 0x719640 | verified | type 0xd + GetDataField + memcpy |
| 0x44f660 | CSWCArea::non_virtual_thunk_to_AsSWCArea | 0x7a2c60 | verified | this-0x104; Mac AsSWCArea thunk imm -0x104 |
| 0x44f680 | CSWCArea::Destructor | 0x7a2c80 | verified | deleting dtor -> trusted body 0x7a2cb0 |
| 0x456350 | CSWCArea::ClearTilePathNodes | 0x7a9950 | **label_suspect** | body drains a CAurObject linked list at +0x208, called only from UnloadArea 0x7a5fa0 (GOG 0x4529a0). Mac ClearTilePathNodes (disp 0x23c..0x250, delete[]) is GOG 0x457310 -> Steam 0x7aa910 (part a). No Mac name (likely inlined in Mac UnloadArea) |
| 0x4578a0 | DetachFromServerArea | 0x7aaea0 | verified | [+0x280]=0; Attach twin stores arg |
| 0x582200 | CSWCAreaOfEffectObject::Destructor_2 | 0x8ee820 | verified | +0x154 vcall, UnloadModel, ~CSWCObject = Mac |
| 0x506a50 | CSWCDoor::Destructor | 0x87b170 | verified | deleting dtor -> trusted body |
| 0x506a80 | CSWCDoor::Destructor_2 | 0x87b1a0 | verified | 3 strings at 0xfc/0x134/0x140 = Mac |
| 0x5070c0 | CSWCDoor::GetState | 0x87b7e0 | likely | byte getter +0x120; no Mac symbol |
| 0x507760 | GetServerDoor | 0x87be80 | verified | GetServerObject + vt 0x18 (Mac 0x1c) |
| 0x48f9b0 | CSWCLevelUpStats::GetClass | 0x844e40 | **label_suspect** | the body is `CSWCCreatureStats::GetClass` (Mac 0x184b08, disp 0xf8, imm 5). It sits in the GOG CSWCCreatureStats block, and Mac CSWCLevelUpStats has no GetClass. Wrong class qualifier |
| 0x472580 | CSWCObject::GetName | 0x7ed420 | verified | base GetName returns "" = Mac |
| 0x4725e0 | GetPortrait | 0x7ed480 | verified | portrait routine on +0xa8 = Mac |
| 0x472610 | GetPortraitId | 0x7ed4b0 | likely | word getter +0xb8 |
| 0x472660 | TurnOffShadows | 0x7ed500 | verified | vt+0xbc on [+0x68]; Mac 0xc0 |
| 0x472930 | GetCurrentAnimation | 0x7ed7d0 | likely | word getter +0x48 |
| 0x472ae0 | GetLoopingAnimation | 0x7ed980 | likely | word getter +0x58 |
| 0x474540 | AnimationHideEquippedItems | **0x7ef3e0** (pipeline 0x7ef910) | **label_suspect** | body = AnimationWalking (see systematic finding 1) |
| 0x4745d0 | AnimationWalking | **0x7ef470** (pipeline 0x7ef3e0) | **label_suspect** | body = AnimationRunning |
| 0x4749e0 | AnimationAttack | 0x7ef880 | verified | 'Attack' handle 0xa0fe4c |
| 0x474a70 | AnimationRunning | **0x7ef910** (pipeline 0x7ef470) | **label_suspect** | body = AnimationHideEquippedItems |
| 0x477660 | ClearAllActions | 0x7f24c0 | verified | Server ClearAllActions callee = Mac |
| 0x4776a0 | ClearAllQueuedCombatActions | 0x7f2500 | verified | Server ClearAllQueuedCombatActions callee = Mac |
| 0x4777b0 | AttachmentFromServerObject | 0x7f2610 | verified | [+0xf8]=arg; Mac 0xf8 |
| 0x5349c0 | CSWCPlaceable::GetName | 0x841140 | verified | copies the +0xfc string = Mac |
| 0x534a50 | CSWCPlaceable::Destructor | 0x8411d0 | verified | deleting dtor -> trusted body |
| 0x534a80 | CSWCPlaceable::Destructor_2 | 0x841200 | verified | UnloadLight + strings at 0xfc/0x104/0x148 = Mac |
| 0x5355c0 | BashPlaceable | 0x841d40 | verified | ShowTutorialWindow(0x21) + PerformAttack = Mac |
| 0x536580 | GetServerPlaceable | 0x842d00 | verified | GetServerObject + vt 0x48 (Mac 0x4c) |
| 0x4ce500 | CSWGuiControl::Destructor | 0x418640 | verified | non-deleting body dtor (members +0x1c/+0x2c/+0x3c) = Mac 79 B. The GOG "Destructor" here is the body, not the deleting dtor (that is 0x4ce600) |
| 0x4ce5d0 | CSWGuiControl::SetActive | 0x4185d0 | likely | bit-0 setter on +0x48; Mac imm 0xfe |
| 0x59af30 | CSWGuiInGameInventory::Destructor | 0x8a7030 | verified | deleting dtor -> trusted body |

How I split verified and likely:
- A pure field getter or setter is `likely` even when its fingerprint is unique and the Mac disp matches. This is the part c convention.
- A function that writes a constant or has a meaning-bearing anchor is `verified`. Examples: Detach writes 0, Attach stores its argument, a callee or caller anchor.

## Systematic findings

### 1. CSWCObject animation-flag block: a 3-way GOG label rotation

This is the same block as the known Pause/PlayOutOfPlace swap (GOG_LABEL_SUSPECTS #9/#10).
- The GOG and Steam layouts are 1:1 across 0x474420..0x474b00 with the constant delta 0x37aea0. All the 129-B bodies share one fingerprint, so the delta defines the body counterpart.
- Each Steam body reads one 2DA column handle. The handles are set in the Steam init 0x6eb670 by `PUSH <string>` followed by `MOV [handle],EAX`.

| GOG | GOG label | body (by handle) | Steam | handle evidence |
|---|---|---|---|---|
| 0x474540 | AnimationHideEquippedItems | **AnimationWalking** | 0x7ef3e0 | `007ef437 MOV EAX,[0x00a0fe30]`; `006ebae5 PUSH 0x99af50` "Walking" -> `006ebb20 MOV [0x00a0fe30],EAX` |
| 0x4745d0 | AnimationWalking | **AnimationRunning** | 0x7ef470 | `007ef4c7 MOV EAX,[0x00a0fe34]`; `006eba53 PUSH 0x99af64` "Running" -> `006eba88` |
| 0x474a70 | AnimationRunning | **AnimationHideEquippedItems** | 0x7ef910 | `007ef967 MOV EAX,[0x00a0fe50]`; `006eb89d PUSH 0x99af84` "HideEquippedItems" -> `006eb8d2` |
| 0x4749e0 | AnimationAttack | AnimationAttack (correct) | 0x7ef880 | `007ef8d7 MOV EAX,[0x00a0fe4c]`; `006eb730 PUSH 0x99afb4` "Attack" -> `006eb765` |

Mac x86 order supports the body reading: AnimationPause 0x22d042, AnimationWalking 0x22d0be, AnimationRunning 0x22d13a are consecutive. That matches the true GOG body order 0x4744b0 (Pause), 0x474540 (Walking), 0x4745d0 (Running). Mac AnimationHideEquippedItems is elsewhere (0x22c944). There is a caller hint too: Walking and Running (0x7ef3e0, 0x7ef470) have the single caller 0x769780, while HideEquippedItems 0x7ef910 has 3 callers.

Steam rows by name, proven by meaning:
- `AnimationWalking = 0x7ef3e0`
- `AnimationRunning = 0x7ef470`
- `AnimationHideEquippedItems = 0x7ef910`
- `AnimationAttack = 0x7ef880`

These are the pipeline's addresses. Only the GOG references are rotated, so they can ship under these names with an Opus override.

The reviewer matched each row by the handle string, which is right for the name. It did not notice that the GOG address's body is a different twin. These are escalation rows, not sample rows.

### 2. CSWCArea ClearTilePathNodes

GOG 0x456350 `ClearTilePathNodes` is a list drain at +0x208 with no Mac counterpart. The Mac ClearTilePathNodes body is GOG 0x457310, which the GOG DB labels `ClearAllMapPins` (part a, finding 1). This extends the map-pin / weather mislabel family in part a.

### 3. CSWCCreatureStats accessors labelled as CSWCLevelUpStats

GOG 0x48f9b0 `CSWCLevelUpStats::GetClass` and 0x48f9f0 `CSWCLevelUpStats::GetClassLevel` (disp 0xf9) sit between CSWCCreatureStats::GetClassesString and CSWCCreatureStats::SetClass. They equal Mac `CSWCCreatureStats::GetClass` 0x184b08 (0xf8) and `GetClassLevel` 0x184b2c (0xf9). Mac CSWCLevelUpStats defines neither. Proposed Steam name: `CSWCCreatureStats::GetClass = 0x844e40`. 0x48f9f0 -> 0x844e80 is not gated here.

### 4. GOG function-boundary error at 0x4db100

GOG `CGuiInGame::ShowDebugMenu` 0x4db100 (15 B, its only callee is CExoSound::SetSoundMode) is the tail of HideGalaxyMapGui 0x4db020:
- GOG 224 B + 15 B = Steam 0x7ca920 (239 B), whose tail is `007caa06 CALL 0x0070bc60` SetSoundMode;
- Mac HideGalaxyMapGui also calls SetSoundMode;
- Mac ShowDebugMenu is a 5-B stub.

Steam has no separate function there.

### Other notes
- CClientExoApp::GetClientOptions is ICF-folded on Steam (0x72fb00, outside the class block, shared with OptionsHolder::GetGameOptions). The row is right, but a hook there affects both callers' paths.
- DB naming of destructors is not uniform: for CSWGuiControl, "Destructor" is the body dtor. For the other classes in this part, "Destructor" is the deleting dtor and "Destructor_2" is the body. The bodies are correct in every case.

## GOG label errors found (for GOG_LABEL_SUSPECTS.md)

| GOG addr | GOG name | what it is | Mac evidence | Steam |
|---|---|---|---|---|
| 0x474540 | CSWCObject::AnimationHideEquippedItems | AnimationWalking | Mac Pause, Walking, Running consecutive (0x22d042/0d0be/13a); "Walking" handle | 0x7ef3e0 |
| 0x4745d0 | CSWCObject::AnimationWalking | AnimationRunning | same; "Running" handle | 0x7ef470 |
| 0x474a70 | CSWCObject::AnimationRunning | AnimationHideEquippedItems | Mac HideEquippedItems 0x22c944; "HideEquippedItems" handle | 0x7ef910 |
| 0x456350 | CSWCArea::ClearTilePathNodes | +0x208 CAurObject-list drain (UnloadArea helper), no Mac name | Mac ClearTilePathNodes disp 0x23c..0x250 = GOG 0x457310 | 0x7a9950 |
| 0x48f9b0 | CSWCLevelUpStats::GetClass | CSWCCreatureStats::GetClass | Mac 0x184b08 disp 0xf8 imm 5; no Mac CSWCLevelUpStats::GetClass | 0x844e40 |
| 0x48f9f0 | CSWCLevelUpStats::GetClassLevel (not in this part) | CSWCCreatureStats::GetClassLevel | Mac 0x184b2c disp 0xf9 | 0x844e80 (not gated) |
| 0x4db100 | CGuiInGame::ShowDebugMenu | tail chunk of HideGalaxyMapGui 0x4db020 | Mac HideGalaxyMapGui calls SetSoundMode; Mac ShowDebugMenu is a 5-B stub | (inside 0x7ca920) |

## Sample / AUTO

Part e1 contains only escalation rows. None of them is in "Sample of the other reviewer matches", and there are no AUTO rows, so there is no AUTO pass/fail table.

Reviewer `match` calls that were wrong on the GOG body in this part (escalation, not sample):
- 3 rows: the animation rotation. Each matched the right Steam function for the name but not the GOG body.
- 2 rows matched the body but missed a GOG label error: ClearTilePathNodes and CSWCLevelUpStats::GetClass.

sample_errors: 0
