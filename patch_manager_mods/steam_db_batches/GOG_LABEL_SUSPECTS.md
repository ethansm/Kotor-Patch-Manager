# GOG DB label suspects (for the user; the GOG DB is never edited without approval)

Rule (user, 2026-10-03): the PR changes only the Steam DB. GOG label bugs are recorded here and proposed only when
confirmed. Evidence comes from:
- the GOG binary on the OpenKotOR server (`fp.tsv`: callees, displacements and immediates of the function at the GOG
  address);
- the Mac x86 build's real symbol names (Mach-O symbol table) with its fingerprints;
- the Steam counterpart's decompile.

Rows were confirmed by the Opus calibration gate (`calibration/OPUS_REVIEW.md` section 5 and 4a-4c, 2026-10-03) and by
the B0 Opus gate (parts a, b, c, e1-e4, f1, f2: `B0/opus/overrides_*.tsv.md`), consolidated 2026-10-03.

How the Steam DB handles them: a row whose Steam body matches the GOG address but whose GOG name contradicts the body
is `label_suspect` and is not shipped under the GOG name. When the Steam function for a name is proven by meaning
(Mac symbol + Steam anchor), the Steam row ships under that name with an Opus override (`semantic:` note in
`B0/overrides.tsv`). Names with no GOG DB row (or a GOG DB row outside B0) are listed in `B0/semantic_extra.tsv`
and are not shipped yet.

Columns: **GOG addr** = the address the GOG DB gives the name. **Body is** = what that GOG function really is.
**Steam (body)** = Steam counterpart of that GOG body. **Name really at** = where the named function is (GOG -> Steam);
"shipped" = the Steam row ships under the name in B0 (verdict in brackets), "extra" = `semantic_extra.tsv`.
Addresses are hex; Mac addresses are Mac x86 symbol addresses.

## Confirmed label errors

### CGuiInGame (two rotations: the dialog block and the minimap block)
Mac order SetCombatMessage, ShowDialogMessage, GetMiniMapVisible, RemoveGUIPopUps vs GOG labels ShowDialogMessage,
SetCombatMessage, GetMiniMapVisible, RemoveGUIPopUps, ResetInterfaceFont: a **rotation**, not isolated swaps.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x4E0BF0 | `ShowDialogMessage` | GetInterfaceOffset-shaped (disp 0x98,0xA900) | Mac `GetInterfaceOffset` disp 0x98,0xA770 | to map | GOG 0x4E0C60 -> Steam 0x7D0560 (extra: GOG row not in B0) |
| 0x4E0C60 | `GetMiniMapVisible` | ShowDialogMessage: calls `vt+0x84` on `[+0x3C]` when `[+0x3C]==[+0x40]` (`007d0570 CMP EDX,[ECX+0x40]`) | Mac `ShowDialogMessage` 0x268B38, 33 B, disp 0x3C,0x40,0x88 (vt +4) | 0x7D0560 | GOG 0x4E0CA0 -> Steam 0x7D05A0 (shipped, verified) |
| 0x4E0CA0 | `RemoveGUIPopUps` | GetMiniMapVisible: GetClientOptions + GetMiniGame, minimap bit at `+0x59B4` (`007d0602`) | Mac `GetMiniMapVisible` same callees, disp 0x98,0x58E4 | 0x7D05A0 | GOG 0x4E0D20 -> Steam 0x7D0620 (shipped, likely: shape only) |
| 0x4E0D20 | `ResetInterfaceFont` | RemoveGUIPopUps | Mac shape disp 0x38,0x40,0x54,0x94,0xA0,0xA4,0xB0, one callee | 0x7D0620 | to locate |
| 0x4DBDA0 | `IsCameraDialog` | 4-arg forwarder to `CSWGuiDialogLetterbox::SetFade` on `[this+0x68]`, RET 0x10 (`007cb6cc CALL 0x008bb0e0`) | no Mac name found | 0x7CB6A0 | GOG 0x4DBE50 -> Steam 0x7CB750 (shipped, likely) |
| 0x4DBDE0 | `IsAnimationPlayingInDialog` | DialogGuiDoneScroll (`007cb6f3 MOV [ECX+0xfb4],1`, then `vt+0x84` on `[+0x3C]`) | Mac `DialogGuiDoneScroll` 0x26563C, disp 0x3C,0x64,0x88,0xFB4 | 0x7CB6E0 | GOG 0x4DB4B0 -> Steam 0x7CADB0 (shipped, likely) |
| 0x4DBE20 | `IsLoopingDialogStuntAnimation_2` | IsFadingInDialog: `[+0x68]` letterbox, returns `!IsFadeDone` (`007cb736 CALL 0x008bb200`) | Mac `IsFadingInDialog` 42 B, `[+0x68]`, 1 callee | 0x7CB720 | `IsLoopingDialogStuntAnimation` = GOG 0x4DB8F0 (not checked) |
| 0x4DBE50 | `IsFadingInDialog` | IsCameraDialog (`007cb75a CMP [EAX+0x3c],0` / `007cb769 CMP EAX,[EDX+0x40]`) | Mac `IsCameraDialog` disp 0x3C,0x40 | 0x7CB750 | GOG 0x4DBE20 -> Steam 0x7CB720 (shipped, verified) |
| 0x4DB4B0 | `DestroyDeathMenuGUI` | IsAnimationPlayingInDialog (`007cae2e CALL 0x007ef780` AnimationDialog over the `[+0x1C0]` list) | Mac disp 0x1B8/0x1C0 | 0x7CADB0 | to locate |
| 0x4DF1F0 | `GetCanOrientInDialog` | Vector3 getter at `[[this+0x9C]+0x440..0x448]` (`007ceb28 ADD EDX,0x440`) | no Mac counterpart (Batch 1 called it GetAreaTransitionPosition, unconfirmed) | 0x7CEAF0 | GOG 0x4DFE10 -> Steam 0x7CF710 (shipped, verified) |
| 0x4DFE10 | `IsOverlayDialogAnimation_2` | GetCanOrientInDialog (`007cf725 ADD ECX,0xf24` list walk, imm 0x7F000000) | Mac `GetCanOrientInDialog` 0x26161C | 0x7CF710 | `IsOverlayDialogAnimation` = GOG 0x4DB950 (correct label) |
| 0x4CDDA0 | `ShutDownGlobal` | compiler vector-deleting dtor of a 16 B {CExoString,CExoString} (`007bdb50 CALL 0x00919bcc`) | Mac `ShutDownGlobal` differs | 0x7BDB10 | to locate |

### CSWCObject (animation-flag block: Pause/PlayOutOfPlace swap + Walking/Running/HideEquippedItems 3-way rotation)
GOG and Steam are 1:1 across 0x474420..0x474B00 (run delta 0x37AEA0); each Steam body reads one 2DA column handle set in
the Steam init 0x6EB670 (`PUSH <string>` -> `MOV [handle],EAX`).

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x474890 | `AnimationPause` | AnimationPlayOutOfPlace: id-range test 0x4B0..0x6BF (`007ef752 CMP EDX,0x640`) | Mac `AnimationPlayOutOfPlace` 51 B, imm 0xFFFFF9C0,0x80 | 0x7EF730 | GOG 0x4744B0 -> Steam 0x7EF350 (shipped, verified) |
| 0x4744B0 | `AnimationPlayOutOfPlace` | AnimationPause: "Pause" handle `[0x00a0fe2c]` (`006eba0a PUSH 0x99af6c`) | Mac `AnimationPause` C2DA::GetINTEntry | 0x7EF350 | GOG 0x474890 -> Steam 0x7EF730 (shipped, verified) |
| 0x474540 | `AnimationHideEquippedItems` | AnimationWalking: "Walking" handle `[0x00a0fe30]` (`007ef437`) | Mac Pause/Walking/Running consecutive (0x22D042/0x22D0BE/0x22D13A) | 0x7EF3E0 | GOG 0x474A70 -> Steam 0x7EF910 (shipped, verified) |
| 0x4745D0 | `AnimationWalking` | AnimationRunning: "Running" handle `[0x00a0fe34]` (`007ef4c7`) | same | 0x7EF470 | GOG 0x474540 -> Steam 0x7EF3E0 (shipped, verified) |
| 0x474A70 | `AnimationRunning` | AnimationHideEquippedItems: handle `[0x00a0fe50]` (`007ef967`) | Mac `AnimationHideEquippedItems` 0x22C944 | 0x7EF910 | GOG 0x4745D0 -> Steam 0x7EF470 (shipped, verified) |

### CSWCArea (map-pin family labelled as weather objects)
Mac has `AddMapPin` / `GetMapPin` / `ClearAllMapPins` and no WeatherObject API.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x456040 | `AddWeatherObject` | AddMapPin (`007a964e ADD ECX,0x1c8` + array Add) | Mac `AddMapPin` imm 0x1C8 | 0x7A9640 | no such function (AddMapPin: extra) |
| 0x456060 | `GetWeatherObject` | GetMapPin (`007a9699 MOV EDX,[ECX+0x1c8]` / `007a96b1 [EDX+0x108]`); sole caller HandleServerToPlayerMapPinEnabled | Mac `GetMapPin` disp 0x108,0x10C,0x1C8,0x1CC | 0x7A9660 | no such function (GetMapPin: extra) |
| 0x4560F0 | `ClearWeatherObjects` | ClearAllMapPins (clears 0x1C8/0x1CC; sole caller UnloadArea) | Mac `ClearAllMapPins`, called by UnloadArea | 0x7A96F0 | no such function |
| 0x457310 | `ClearAllMapPins` | ClearTilePathNodes (`007aa91c ADD ECX,0x248` / `ADD ECX,0x23c` + 2 SetSize) | Mac `ClearTilePathNodes` disp 0x23C..0x250 | 0x7AA910 | GOG 0x4560F0 -> Steam 0x7A96F0 (shipped, verified) |
| 0x456350 | `ClearTilePathNodes` | +0x208 CAurObject-list drain (UnloadArea helper), no Mac name | - | 0x7A9950 | GOG 0x457310 -> Steam 0x7AA910 (shipped, verified) |

Not an error: 0x4562D0 `ApplyWeatherVisual` (no symbol of that name in any build; likely an inlined Mac helper; stays `likely`).

### CSWCLevelUpStats / CSWCCreature / CSWSPlaceable
| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x48F9B0 | `CSWCLevelUpStats::GetClass` (also a GOG DB row `CSWSCreatureStats::GetClass`) | `CSWCCreatureStats::GetClass` (`00844e60 MOV AL,[EDX+ECX+0xf8]`) | Mac `CSWCCreatureStats::GetClass` 0x184B08 disp 0xF8 imm 5; Mac CSWCLevelUpStats has no GetClass | 0x844E40 | CSWCCreatureStats::GetClass has no GOG DB row (extra) |
| 0x48F9F0 | `CSWCLevelUpStats::GetClassLevel` | `CSWCCreatureStats::GetClassLevel` | Mac 0x184B2C disp 0xF9 | 0x844E80 (not gated) | extra (unverified) |
| 0x4213A0 | `CSWSPlaceable::SetEffectSpellId` | correct, and also `CSWCCreature::SetThrownLightsaber` (ICF-shared body; only caller ApplyLightsaberThrow) | Mac `SetThrownLightsaber` 0x2870E4 disp 0x3E0, called by Mac ApplyLightsaberThrow | 0x5EF180 | SetEffectSpellId shipped (likely, ICF fold); SetThrownLightsaber -> 0x5EF180 (extra) |
| 0x4213C0 | `CSWCCreature::SetThrownLightsaber` | a SetArea helper Mac inlined (66 B, disp 0x20,0x108,0x3B4; callers SetArea, CSWCArea::LoadArea) | Mac `CSWCCreature::SetArea` 59 B disp 0x20,0x108,0x3B4 | not gated | ICF body GOG 0x4213A0 -> Steam 0x5EF180 |
| 0x42BD00 | `CSWCCreature::SetSWAnimationSpeed` | `CSWCCreature::GetLightSaberThrowRate` (59 B, retn 0, disp 0x11DC; called by ApplyLightsaberThrow) | Mac `GetLightSaberThrowRate` 0x28E084 62 B disp 0x11DC; Mac SetSWAnimationSpeed uses 0x3C8/0x3CC | not gated | to locate |

### CServerExoApp / CTwoDimArrays / CSWSItemPropertyHandler
| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x64C2F0 | `CTwoDimArrays::GetSwoopUpgradeRowProvider` | `CServerExoApp::GetPartyTable` (`0051c8dd ADD EAX,0x1f0b4` on `[this+4]`) | Mac `CServerExoApp::GetPartyTable` 0x324362 16 B disp 0x4 | 0x51C8D0 | to locate (GetPartyTable: extra) |
| 0x7C1880 | `CSWSItemPropertyHandler::Constructor` | base dtor `~CItemPropertyApplierRemover`: only `005aa47a MOV [EAX],0x99247c` (base vftable; Handler vptr is 0x992468); only caller an unwind funclet (GOG `Unwind@0097a380`) | no Mac symbol | 0x5AA470 | to locate (construction is inlined in 0x51D2F0) |

### CSWSItem
| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x667600 | `GetPropertyByTypeExists` | RestoreUsedActiveProperties (`00603e88 MOV byte [EDX+0x10],1` by subtype) | Mac `RestoreUsedActiveProperties` imm 2..5,0xA,0xFF, disp 0x288 | 0x603DD0 | GOG 0x667790 (unnamed) -> Steam 0x603F60 (shipped, verified; `0060a136` called with 0x3D by AddSubraceRequirements) |
| 0x667710 | `RestoreUsedActiveProperties` | unnamed 63 B static type predicate (`00603ef3 SUB ECX,0xa`, byte switch) | - | 0x603EE0 | GOG 0x667600 -> Steam 0x603DD0 (shipped, verified) |
| 0x6FCF40 | `SetNumActiveProperties` | `CExoArrayList<ushort>::Contains` in the template block; sole caller HasFeat | - | 0x6C0AA0 | GOG 0x667970 (unnamed) -> Steam 0x604140 (shipped, verified; `0060414f MOV [EAX+0x288],ECX`) |

### CSWSCreatureStats (weapon/talent block: a multi-way rotation)
Combined chain: GetWeaponFinesse sits at the DecrementFeatRemainingUses_2 slot; GetUseMonkAttackTables at the
GetWeaponFinesse slot; GetWeaponSpecialization at the UnReadySpell slot; UnReadySpell at the
TalentRandomSpecialAbility slot; SpecialAbility at the GetTotalDamageRange slot; TalentRandomPower at the
SetArcaneSpellFailure slot; GetSpellsPerDayLeft at the TalentRandomPower slot.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x6E41C0 | `GetForceResistance` | `min(100, b[+0x128]+b[+0x129]+b[+0x12A])` (`006a7d6b CMP ECX,0x64`), name unknown | Mac `GetForceResistance` 0x3FBAC6 = `[+0x12B]-[+0x12C]` | 0x6A7D40 | GOG 0x6F7190 (FUN_) -> Steam 0x6BAD10 (shipped, verified) |
| 0x6E5320 | `GetWeaponFinesse` | GetUseMonkAttackTables (`006a8eb5 CALL 0x5766f0`, `006a8ed7 PUSH 0x10`) | Mac `GetUseMonkAttackTables` disp 0x24,0x24,0x1150 imm 0x10, GetUseMonkAbilities + GetItemInSlot | 0x6A8EA0 | GOG 0x6F5220 -> Steam 0x6B8DA0 (shipped, verified) |
| 0x6E5390 | `GetWeaponSpecialization` | monk-level/size tier function (NWN GetUnarmedDamageDice-like), no HasFeat | Mac `GetWeaponSpecialization` = HasFeat via base item, disp 0xB0 | 0x6A8F10 | GOG 0x6F5360 -> Steam 0x6B8EE0 (shipped, verified) |
| 0x6E5460 | `GetUseMonkAttackTables` | monk unarmed die-size ladder 4..0x20 (`006a90cf MOV [EBP-0x10],0x14`), NWN GetUnarmedDamageDie-like | inlined on Mac | 0x6A8FE0 | GOG 0x6E5320 -> Steam 0x6A8EA0 (shipped, verified) |
| 0x6F5220 | `DecrementFeatRemainingUses_2` | GetWeaponFinesse (callees GetBaseItem, IsLightsaber, GetDEXMod, HasFeat; `006a7ed5` caller GetMeleeAttackBonus) | Mac `GetWeaponFinesse` 0x3FBD3E callees 4/4, disp 0x8 x3,0xEE, imm 0xC1/0xC2 | 0x6B8DA0 | to locate |
| 0x6F5360 | `UnReadySpell` | GetWeaponSpecialization (`006b8f09 MOV CX,[EAX+0xb0]` then HasFeat) | Mac `GetWeaponSpecialization` 0x3FBEA4 74 B | 0x6B8EE0 | GOG 0x6F5630 -> Steam 0x6B91B0 (shipped, verified) |
| 0x6F5630 | `GetCreatureTalentRandomSpecialAbility` | UnReadySpell (callees 6/6, no rand; sole caller DecrementSpellReadyCount `00581909`) | Mac `UnReadySpell` 0x409C6A | 0x6B91B0 | GOG 0x6F7340 -> Steam 0x6BAEC0 (shipped, verified) |
| 0x6F5A80 | `GetCreatureTalentRandomPower` | GetSpellsPerDayLeft (ClassInfo +0x19; callers GuiKnownSpellUses, PlayerUpdate) | Mac `GetSpellsPerDayLeft` same shape, same 2 callers; Mac TalentRandomPower is 161 B | 0x6B9600 | GOG 0x6F7280 -> Steam 0x6BAE00 (shipped, verified) |
| 0x6F7280 | `SetArcaneSpellFailure` | GetCreatureTalentRandomPower (181 B, disp 0x8D,0x8D,0xAB imm 0x28; calls KnownPower `006bae97`) | Mac Power 161 B same disp/imm; Mac SetArcaneSpellFailure is a 5 B stub | 0x6BAE00 | to locate |
| 0x6F7340 | `GetTotalDamageRange` | GetCreatureTalentRandomSpecialAbility (disp 0x104, 0x134 x8, imm 0xFE,0xFFFFFF9C) | Mac SpecialAbility same values; Mac GetTotalDamageRange is a 1555 B damage function | 0x6BAEC0 | to locate |
| 0x6F88F0 | `ComputeNumberKnownSpellsLeft` | skill index -> feat 0xB7..0xBD then HasFeat; caller AutoLevelup; name unknown | Mac version uses ClassInfo 0xAB/0xAC + GetSpellGain | 0x6BC470 | to locate |
| 0x6F8BB0 | `CheckInclusionMatch` | effect-type -> immunity-index helper of GetEffectLinkImmunity (jump table) | inlined on Mac | 0x6BC730 | GOG 0x6FA1E0 -> Steam 0x6BDD60 (shipped, verified) |
| 0x6FA1E0 | `AddSpellLikeAbilityToList` | CheckInclusionMatch (nibble wildcard, `006bdd73 AND EAX,0xf000`; callers TalentRandomPowerFromItem/Feat) | Mac `CheckInclusionMatch` same callers; Mac AddSLAToList is 97 B, disp 0x134, caller ReadStatsFromGff | 0x6BDD60 | to locate |
| 0x6FA4C0 | `GetIsInSpellLikeAbilityList` | class id of the last class slot (`006be081 MOV DL,[ECX+EAX+0xab]`), name unknown | Mac `GetIsInSpellLikeAbilityList` 0x408AE4 searches SLA list +0x134 stride 0xC | 0x6BE040 | GOG 0x6F6D70 (FUN_) -> Steam 0x6BA8F0 (shipped, verified) |

Correction recorded: part e2 called the 0x6F5630 body GetCreatureTalentRandomKnownPower (disp match); part f2 settled it
as UnReadySpell on callees and caller (`00581909 CALL 0x006b91b0` from DecrementSpellReadyCount, no rand). The real
KnownPower is the row's own label GOG 0x6F9420 -> Steam 0x6BCFA0 (calls rand `006bd6fc`; called by TalentRandomPower
at `006bae97`).

### CSWSArea (path/DFS + Smooth* rotation, object iterator swap, stealth XP, hit info)
Mac call graph: SmoothPointsOnPath -> SmoothSelection -> SmoothSelectNodes;
SmoothPointsOnPathRestartable -> SmoothSelectionRestartable -> SmoothSelectNodesRestartable; GridDFSearch -> Get/Put.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x707770 | `SetHitInfoForCreature` | EvaluateOverlappingTargets (`0054d404 CALL 0x51c100`, `0054d551 [ECX+0x380]`) | Mac `EvaluateOverlappingTargets` 0x4261DE disp 0x2C,0x30,0x380; Mac SetHitInfoForCreature calls IntersectCircle2d | 0x54D3D0 | GOG 0x710260 (FUN_) -> Steam 0x556040 (shipped, verified) |
| 0x709770 | `GridDFSTransTableInitialize` | SmoothPointsOnPath (callers PlotGridPath + PlotPathPointPath) | Mac disp 0x1C,0x8C-0x98,0x1B0 | 0x54F3D0 | GOG 0x70CCB0 -> Steam 0x552A90 (shipped, verified) |
| 0x709980 | `SmoothPointsOnPath` | SmoothPointsOnPathRestartable | Mac Restartable 753 B, disp 0x214-0x234 | 0x54F5E0 | GOG 0x709770 -> Steam 0x54F3D0 (shipped, verified) |
| 0x709D30 | `SmoothPointsOnPathRestartable` | SmoothSelectionRestartable (calls SelectNodesRestartable only) | Mac 750 B | 0x54FB10 | GOG 0x709980 -> Steam 0x54F5E0 (shipped, verified) |
| 0x709FD0 | `SmoothSelectNodes` | SmoothSelection (calls SelectNodes + free) | Mac `SmoothSelection` 200 B | 0x54FDB0 | GOG 0x70A060 -> Steam 0x54FE40 (shipped, verified) |
| 0x70A060 | `SmoothSelectionRestartable` | SmoothSelectNodes (TestDirectLine, magnitude, ComputePathDistance) | Mac `SmoothSelectNodes` 991 B, same 3 callees | 0x54FE40 | GOG 0x709D30 -> Steam 0x54FB10 (shipped, verified) |
| 0x70CCB0 | `GridDFSTransTablePut` | GridDFSTransTableInitialize (`00552aa5 PUSH 0x2000`, imm 0x80000000) | Mac Initialize imm 0x80000000, disp 0x1B8 x6 | 0x552A90 | GOG 0x70CEB0 -> Steam 0x552C90 (shipped, verified) |
| 0x70CD60 | `GridDFSTransTableGet` | TransTable hash helper `(x*0x11+y*0x17+0x1770)&0xff`, never reads `this` | no Mac symbol (inlined) | 0x552B40 | GOG 0x70CD90 -> Steam 0x552B70 (shipped, verified) |
| 0x70CD90 | `PlotSoundPath` | GridDFSTransTableGet (two-probe lookup in `[this+0x1B8]`) | Mac Get 0x429000 172 B; Mac PlotSoundPath is a 10 B stub | 0x552B70 | to locate |
| 0x70CEB0 | `PathPointDFSTransTableInitialize` | GridDFSTransTablePut (stores at `[+0x1B8]`, RET 0x10) | Mac PathPoint Init uses 0x274 and imm 0x7DCCCCCC (absent) | 0x552C90 | to locate |
| 0x718400 | `GetNextObjectInArea` | GetFirstObjectInArea (`0052325c MOV [EAX+0x1a0],0`) | swap | 0x523250 | GOG 0x718460 -> Steam 0x5232B0 (shipped, verified) |
| 0x718460 | `GetFirstObjectInArea` | GetNextObjectInArea (`005232c8 MOV [EDX+0x1a0],ECX` after +1) | swap | 0x5232B0 | GOG 0x718400 -> Steam 0x523250 (shipped, verified) |
| 0x7184D0 | `SetMaxStealthXP` | IncrementPlayersInArea (disp 0x12C x3, calls IncreaseAILevelPriority) | Mac `IncrementPlayersInArea` 45 B | 0x523320 | GOG 0x722EE0 -> Steam 0x52DE00 (shipped, verified) |
| 0x722EE0 | `GetClientArea_2` | SetMaxStealthXP (`0052de27 MOV [ECX+0x2e8],EDX`, clamps +0x2EC) | Mac `SetMaxStealthXP` disp 0x2E8,0x2EC,0x2EC | 0x52DE00 | `GetClientArea` = GOG 0x722C70 (Mac disp 0x334) |

### Gob / Global
| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x84B0B0 | `Gob::SetScene` | global `FindGobIdx` (stricmp binary search on name +0x64; called by FindGob 0x84B050) | Mac `FindGobIdx` 96 B | 0x458AE0 | GOG 0x854030 (DB `SetScene_2`) -> Steam 0x461AA0 (shipped, verified; `0046d456` from Gob::AttachToScene) |

### CSWVirtualMachineCommands / Global (B15 command handlers)
**No confirmed GOG label error in B15** (Opus gate parts a/b + consolidation, 2026-10-03). The rows below are recorded
because a GOG name differs from a Mac symbol or the GOG body is shared; none is shipped against GOG parity.
Evidence: the NWScript command tables (`B15/cmdtable.tsv`, DESIGN 4.8; the Mac x86 table uses 8-byte slots).

**Slot-156 pair: Mac names differ; GOG label follows nwscript; shipped at GOG parity** (user decision 2026-10-03).
All three binaries store slot 156 (nwscript `EffectTemporaryForcePoints`) twice and the later store is the runtime
handler. GOG: `0077bd9d` -> 0x78D6B0 (dead, overwritten), `0077c1cd` -> 0x78C440 (live). Mac: `0032566d` -> 0x333C92, then
`00325c72` -> 0x337B80 (live). The Mac symbols are swapped against the GOG names, but the GOG name of the live handler is
the nwscript name of its slot (`name_slot = yes`), and the branch DB already ships it at the live Steam body (B0).

| GOG addr | GOG DB name | body | Mac symbol of this body | Steam (body) | shipped |
|---|---|---|---|---|---|
| 0x78C440 | `CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints` | live slot-156 handler (418 B, 10 callees incl. CGameEffect::SetCreator, disp 0x8 x5,0xA,0xA,0x30) | `ExecuteCommandEffectTemporaryFP` 0x337B80 (295 B, 10 callees, disp 0x8 x5,0xA,0xA,0x34) | 0x676E60 (`00666bed MOV [ECX+0x270],0x676e60` last store; `00676f8f CALL 0x005e5ea0` SetCreator) | GOG parity: 0x676E60 (verified; same as the B0 branch row) |
| 0x78D6B0 | `Global::ExecuteCommandEffectTemporaryFP` | dead first slot-156 store (282 B, 7 callees, no SetCreator, disp 0x8) | `ExecuteCommandEffectTemporaryForcePoints` 0x333C92 (165 B, 7 callees, disp 0x8) | 0x6780D0 (`006667bd MOV [EDX+0x270],0x6780d0`, overwritten at `00666bed`; 0 callers) | GOG parity: 0x6780D0 (verified; dead code) |

**ICF fold (Mac-confirmed), GOG label correct but not the only name of its body:**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x44F600 | `CSWArea::GetIsIDInExcludeList` | correct, and also the command handler of slots 321/327/403 (GetLastAssociateCommand, SetAssociateListenPatterns, ExploreAreaForPlayer): ICF-shared 15 B `return 0`, RET 8 (GOG stores `0077cc92`, `0077dd50`, `0077c2ca`) | Mac has four separate 7 B `return 0` stubs: `ExecuteCommandGetLastAssociateCommand` 0x341974 (slot 321), `ExecuteCommandSetAssociateListenPatterns` 0x34CDD6 (327), `ExecuteCommandExploreAreaForPlayer` 0x338B7C (403), `CSWArea::GetIsIDInExcludeList` 0x270766 | 0x681880 (`00681887 XOR EAX,EAX` / `0068188c RET 0x8`; table stores `006676b2`, `00668770`, `00666cea`; direct caller TestPlayerUpdateDifferences `0063eecd`) | the 3 command names have no GOG DB row (extra, not shipped); GetIsIDInExcludeList not gated |

Not errors (source names that differ from the nwscript names but are the Mac real symbols of the same slot set, shipped
`verified`): `ExecuteCommandGetEmotions` 0x791A10 (slots 235-237 GetIsEnemy/Friend/Neutral, Mac 0x3400AE) -> Steam 0x67C430;
`ExecuteCommandIsAIState` 0x7A0B00 (slot 140 GetIsDead, Mac 0x3498F4) -> Steam 0x68B4B0; `ExecuteCommandGetPartyLeader`
0x79CCE0 (slots 238/845, Mac symbols GetPCSpeaker and GetPartyLeader for one handler) -> Steam 0x69B7C0. 653 of the 655
GOG `ExecuteCommand*` handler rows carry the Mac symbol of their slot set; the two exceptions are the slot-156 pair.

### CServerExoApp / CServerExoAppInternal / CSWSModule (B9)
B9 Opus gate parts a/b/c + consolidation, 2026-10-03. Two isolated label errors, no rotation: the AUTO neighbours of
both rows (CreatureAcquireItem, TestSafeLocation, ComputeSafeLocation, UpdateMapData, ForceRunAreaOnExitScripts) keep
their Mac shapes (disp 0x2C8/0x30C, 0x90/0x380, 0x238, 0x124/0x1DC). Both rows are `label_suspect` in `B9/overrides.tsv`
and are not shipped.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x7BDA10 | `CServerExoAppInternal::Uninitialize` | TempDeathUpdate helper: loads the client app `[[0x00a1b4a4]+4]` and calls `CClientExoApp::RunDeathSequence` (`0053a7d7 CALL 0x007403a0` = GOG 0x40D860), 32 B, RET 0; sole caller TempDeathUpdate (GOG 0x7BD310 -> Steam 0x53A0C0) | Mac `CServerExoAppInternal::Uninitialize` 0x399CF4 is an empty 5 B stub; Mac `TempDeathUpdate` 0x3968D4 calls `CClientExoApp::RunDeathSequence` 0x1C0EFE itself (helper inlined on Mac) | 0x53A7C0 | to locate (Mac body is empty; no GOG `CServerExoApp::Uninitialize` forwarder row) |
| 0x662B80 | `CServerExoAppInternal::DebugValidatePath` | `CSWSCreature::GetFirstAttacker`: fastcall on a creature; rebuilds the id list at `[+0x11A4]` from the area creature array `+0x194/+0x198` where `[creature+0x548]` = this id, sets `[+0x11A8]=1`, returns the first id or 0x7F000000; sits in creature code, not the Internal block | Mac `CSWSCreature::GetFirstAttacker` 0x378234 (239 B): disp 0x194,0x198,0x548,0x11A4,0x11A8, imm 0x7F000000, callee `CServerExoApp::GetCreatureByGameObjectID`; Mac `ExecuteCommandGetAttacker` 0x33C84A calls it | 0x6E6880 (unique fp = GOG: 446 B, 127 insn, same disp/imm; sole caller `ExecuteCommandGetAttacker` 0x696BF0) | DebugValidatePath: no Mac x86 symbol; Linux `CServerExoAppInternal__DebugValidatePath` 0x085CFC34 (633 B); to locate on Windows (may be compiled out). GetNextAttacker probably GOG 0x662D40 (FUN_) -> Steam 0x6E6A40 (123 B, fp equal; shape only, not gated) |
| 0x64C2F0 | `CTwoDimArrays::GetSwoopUpgradeRowProvider` | `CServerExoApp::GetPartyTable` (already Confirmed above, B0). B9 adds a caller proof: GOG TempDeathUpdate 0x7BD310 calls 0x64C2F0 and Steam TempDeathUpdate 0x53A0C0 calls 0x51C8D0, as Mac TempDeathUpdate calls `GetPartyTable` 0x324362; +0x1F0B4 is the party table (Internal::RunModule `00536344 ADD ECX,0x1f0b4` then CreateParty) | Mac `CServerExoApp::GetPartyTable` 0x324362 16 B disp 0x4 | 0x51C8D0 (`0051c8dd ADD EAX,0x1f0b4`) | not a B9 row (class CTwoDimArrays); GetPartyTable: extra |

Not errors: Mac orders two CServerExoApp forwarders differently (GetFPS last; ClientToServerObjectId between IsInjured
and GetPauseState), but GOG and Steam agree and both bodies fit their names. Windows `GetModuleLanguage` forwards to
`Global::return_zero` (GOG/Steam agree; Mac calls the Internal function).

**GOG DB param sizes (B9).** 15 B9 rows have a GOG DB `param_size_bytes` that disagrees with the RET n of the GOG
function itself. The Steam RET n is the same as GOG's in every row, so the GOG DB values are wrong. The Steam rows ship
the RET n; finalize records the GOG value in the note. Format: GOG DB value -> RET n.
- Found by the Opus parts: `CServerExoAppInternal::PlayerListChange` 0x7B8F70 8 -> 12; `CSWSModule::SaveModuleStart`
  0x72DFE0 0 -> 8; `CServerExoAppInternal::CopyModuleToCurrentGame` 0x7B4F60 8 -> 12; `CServerExoApp::GetRoomIndex`
  0x64C620 0 -> 12; `CServerExoAppInternal::SaveGame` 0x7B2990 12 -> 16.
- Found by the consolidation sweep: `CServerExoApp::TestDirectLine` 0x64C330 4 -> 32; `CServerExoApp::ComputeHeight`
  0x64C460 0 -> 12; `CSWSModule::Constructor` 0x728E20 8 -> 12; `LoadModuleInProgress` 0x72CC10 4 -> 8;
  `PackModuleIntoMessage` 0x72CF90 0 -> 4; `SaveModuleFinish` 0x72E330 4 -> 8; `SavePlayers` 0x72F830 8 -> 16;
  `PackPlayerCharacterListIntoMessage` 0x730B00 0 -> 8; `CServerExoAppInternal::SetEstimatedSaveSize` 0x7B6860 0 -> 8;
  `StartServices` 0x7B96E0 0 -> 4.

## GOG pseudo-functions (not functions)
| GOG addr | GOG DB name | what it is | Steam |
|---|---|---|---|
| 0x720DC0 | `CSWSArea::Constructor_2` | 2nd Ghidra chunk of ClearLineOfSight 0x720D10 (176 + 352 + 239 = 767 B = Steam 0x52BB60) | none (inside 0x52BB60); real ctor GOG 0x716560 |
| 0x720F20 | `CSWSArea::Destructor_3` | 3rd chunk of ClearLineOfSight | none (inside 0x52BB60); real dtor GOG 0x716B80 |
| 0x4DB100 | `CGuiInGame::ShowDebugMenu` | 15 B tail chunk of HideGalaxyMapGui 0x4DB020 (224 + 15 B = Steam 0x7CA920, tail `007caa06 CALL 0x0070bc60` SetSoundMode) | none; Mac ShowDebugMenu is a 5 B stub |

GOG function splits seen but harmless (the GOG label is on the head chunk): `CVirtualMachineInternal::ReadScriptFile`
(64 B head 0x645270 + body 0x6452B0 = Steam 0x701FD0), `ExecuteCode`.

## Unverified
| GOG addr | GOG name | hint |
|---|---|---|
| 0x6E0580 | `CSWSCreatureStats_ClassInfo::GetSpellGain` | reads +0x19, which is Mac GetSpellsPerDayLeft's field; not checked |
| 0x722F20 | (FUN_) | fits Mac `CSWSArea::SetCurrentStealthXP` (twin of SetMaxStealthXP, Steam 0x52DE40); shape only |

## Impact
- KPM GameAPI's `CGuiInGame::GetMiniMapVisible` wrapper (resolved in AdditionalConsoleCommands' init) calls
  `ShowDialogMessage` on GOG: a function with a side effect, not a predicate. `RemoveGUIPopUps` on GOG resolves to the
  minimap predicate (no effect).
- On GOG, every name in the tables above resolves to a different function. Any GOG-side hook or call by those names
  (CSWSCreatureStats weapon/talent block, CSWSArea pathfinding, CSWCObject animation flags, CSWCArea map pins) runs the
  wrong code. On Steam the B0 rows now ship the meaning-correct addresses, so for these names Steam != GOG + run delta.
- The knowledge DB names Steam 0x54D3D0 `CSWSArea::SetHitInfoForCreature` [high]; that body is EvaluateOverlappingTargets.
- The Steam sig route (fingerprint equality + run position) cannot see GOG label errors: the pre-Opus draft shipped
  `GridDFSTransTablePut = 0x552A90`, `SetArcaneSpellFailure = 0x6BAE00`, `GetTotalDamageRange = 0x6BAEC0` and
  `DecrementFeatRemainingUses_2 = 0x6B8DA0` AUTO-verified under wrong names.

Found by: Steam DB calibration (chunks 1-2) and the B0 Opus gate, 2026-10-03.

## B12b

### CSWSCreature, GOG >= 0x696F00 (B12b: two rotations, one swap, isolated errors)
B12b Opus gate parts a/r/z + consolidation, 2026-10-03 (`B12b/opus/overrides_{a,r,z}.md`, `B12b/B12b_SUMMARY.md`).
Every Steam counterpart below is fp 1:1 with its GOG body (size, retn, n_insn, disp, imm); only the GOG name is wrong.
All 20 rows are `label_suspect` in the B12b overrides and are not shipped under the GOG name.

**AIAction rotation (GOG 0x6C6230-0x6C7820).** The bodies follow the Mac x86 order exactly (ChangeFacingObject,
ChangeFacingPoint, MoveAwayFromObject, MoveAwayFromLocation, InterAreaPathfinding, MoveToObject, MoveToObjectRadius,
MoveToPoint, MoveToPointRadius, MoveToFollowRadius); the labels on slots 2-5 and 9 are rotated.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x6C6230 | `AIActionCheckMoveAwayFromLocation` | ChangeFacingPoint: only normalize + SetOrientation (`005bb581 CALL 0x5143c0`, `005bb58d CALL 0x543f10`) | Mac `AIActionChangeFacingPoint` 194 B, same 2 callees, 0x94/0x98/0x11AC | 0x5BB470 | GOG 0x6C66E0 -> Steam 0x5BB920 |
| 0x6C6360 | `AIActionCheckMoveToPointRadius` | CheckMoveAwayFromObject: CancelAction (`005bb76f CALL 0x576ca0`), action list `+0x100`, ComputeAwayVector `005bb843` | Mac `AIActionCheckMoveAwayFromObject` 947 B, CancelAction + GetNext + ComputeAwayVector, imm 0x7F000000 | 0x5BB5A0 | GOG 0x6C7820 -> Steam 0x5BCA60 |
| 0x6C66E0 | `AIActionCheckInterAreaPathfinding` | CheckMoveAwayFromLocation: ComputeAwayVector, AddMoveToPointActionToFront, AddPathfindingWaitActionToFront | Mac `AIActionCheckMoveAwayFromLocation` same 5 callees | 0x5BB920 | GOG 0x6C6930 -> Steam 0x5BBB70 |
| 0x6C6930 | `AIActionChangeFacingPoint` | CheckInterAreaPathfinding: GetModule, SetIntraAreaGoal (`005bbdda CALL 0x55e610`), new/free, 20x `[+0x380]`, `005bbf1e PUSH 0x1e` | Mac `AIActionCheckInterAreaPathfinding` 953 B, disp 0x5C/0x60/0x64/0x70, imm 0x1E | 0x5BBB70 | GOG 0x6C6230 -> Steam 0x5BB470 |
| 0x6C7820 | `AIActionCheckMoveToPoint_2` | CheckMoveToPointRadius (`005bccf4 PUSH 0x12`) | Mac `AIActionCheckMoveToPointRadius` imm 0x12, disp 0x50/0x54; Mac MoveToPoint has imm 0xA, +0x1114 = GOG 0x6C74C0 | 0x5BCA60 | GOG 0x6C6360 -> Steam 0x5BB5A0 |
| 0x6C9D80 | `AIActionCheckMoveAwayFromObject` | AIActionAreaWait (`005befcc MOV ECX,[EAX+0x10e8]`, GetArea; RunActions case 0x3C) | Mac `AIActionAreaWait` 54 B, 0x10E8, before RandomWalk | 0x5BEFC0 | GOG 0x6C6360 -> Steam 0x5BB5A0 |

**CanUseItem restriction block (GOG 0x6DDEE0-0x6DEE60).** Mac CanUseItem calls 7 checks (Alignment, Class, Race, Feat,
Gender, PC, Attribute); GOG CanUseItem calls the same 7 plus CNPC, and no GOG label says Gender.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x6DDEE0 | `RemoveFromInvitationsOffered` | CheckItemClassRestrictions: property 0x2C (`005d32cb PUSH 0x2c`), HasFeat 0x5D (`005d3311`), class count `+0x8D`, classes `+0xAB` | Mac `CheckItemClassRestrictions` HasFeat, disp 0x8D/0x1198, imm 0x2C/0x5D/0xAB; Mac RemoveFromInvitationsOffered is a 122 B `+0x1044` list remove | 0x5D32C0 | GOG 0x6DF1B0 (FUN_, 119 B, +0x1044, caller ReplyToInvitation) -> Steam 0x5D4590 (shape only) |
| 0x6DE780 | `CheckItemClassRestrictions` | CheckItemGenderRestrictions: property 0x3C (`005d3b6b PUSH 0x3c`), vcall `[EDX+0x88]` (`005d3c48`), no HasFeat | Mac `CheckItemGenderRestrictions` 185 B, imm 0x3C, disp 0x8C (vt +4), 2 callees | 0x5D3B60 | GOG 0x6DDEE0 -> Steam 0x5D32C0 |
| 0x6DEE60 | `AIActionAnimalEmpathy` | 21 B 2-arg false stub (`005d4249 MOV byte [EBP-1],0` / `XOR EAX,EAX` / `RET 8`), sole caller the Class check 0x6DDEE0 | Mac AnimalEmpathy is a 12 B stub; not called by CheckItem* | 0x5D4240 | Steam 0x5D1990 (RunActions case 0x29, ICF-folded with Taunt 0x2B and PickPocket 0x36) |

**AddChemical / AddComponent swap.**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x69A160 | `AddChemical` | AddComponent: feedback `005816f1 PUSH 0xe9`, callees GetComponent/SetComponent (+0x174/+0x1104) | Mac `AddComponent` imm 0xE9, fields 0x174/0x1104 | 0x581630 | GOG 0x69A240 -> Steam 0x581710 |
| 0x69A240 | `AddComponent` | AddChemical: feedback `005817d1 PUSH 0xea`, callees GetChemical/SetChemical (+0x178/+0x1108) | Mac `AddChemical` imm 0xEA, fields 0x178/0x1108 | 0x581710 | GOG 0x69A160 -> Steam 0x581630 |

**Isolated label errors (no rotation).**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x698DF0 | `UpdateAppearanceForEquippedItems_2` | GetDetectMode (`005802e1 MOVZX EAX,[EDX+0x510]`, imm 0x2710) | Mac `GetDetectMode` disp 0xD8/0x510 | 0x5802C0 | UpdateAppearanceForEquippedItems = GOG 0x698430 -> 0x57F900 (shipped AUTO) |
| 0x69DF00 | `BroadcastAttackOfOpportunity` | GetBlind (`0058542c MOV CL,[EAX+0xff4]`, masks 0x10/8/2) | Mac `GetBlind` 0xFF4, next = GetInvisible as GOG 0x69DF60; Mac BroadcastAoO is a 5 B stub | 0x585420 | to locate |
| 0x6A2600 | `UpdatePureGoodEvilPowers` | combat-round action helper of UpdateActionQueue (`00589bac CALL 0x5d7220` GetAQActionIDByID, +0x3E8/+0x3EC stride 0x1C) | inlined on Mac; Mac UpdatePureGoodEvilPowers is 92 B, 0x18A/0x1198 imm 0x64 | 0x589B20 | likely GOG 0x6A42D0 (FUN_, 130 B, 0x18A/0x1198, imm 0x64) |
| 0x6A26D0 | `ComputeArmourClass` | RemoveSpellActionFromRound (`00589c08 CALL 0x590180`, `00589c21 CALL 0x591a70` on +0x10DC) | Mac `RemoveSpellActionFromRound` 47 B, same 2 callees | 0x589BF0 | likely GOG 0x693AC0 (label AddPickUpItemActions; Mac CAC disp set) |
| 0x6A3E00 | `HasEffect` | SetLastInstantEquip (`0058b343 MOV [ECX+0x11d4]` / `0058b351 MOV [EAX+0x11d0]` keyed 0x10/0x20) | Mac `SetLastInstantEquip` 38 B, 0x11D0/0x11D4 | 0x58B320 | to locate (Mac HasEffect 0x148/0x14C) |
| 0x6A4D30 | `UseLoreOnItem` | `CExoArrayList<float>::Add` (`0058c2cb CALL 0x58c4a0` grow, `0058c2de FSTP [ECX+EDX*4]`) | Mac Add 75 B; Mac UseLoreOnItem is a 7 B stub | 0x58C290 | to locate |
| 0x6A4DA0 | `LearnScroll` | `CExoArrayList<float>::Insert` (`0058c31d CALL 0x58c290`, shift loop) | Mac Insert; Mac LearnScroll is a 7 B stub | 0x58C300 | to locate |
| 0x6DC300 | `GetCreatureReputation_2` | visible-list contains (+0x1028/+0x102C scan), sole caller AddToVisibleList | inlined on Mac | 0x5D16E0 | GetCreatureReputation = GOG 0x6DB920 |
| 0x6F5300 | `CSWSCreature::GetRelativeWeaponSize` | `CSWSCreatureStats::GetWeaponFocus` (`006b8ea9 MOV CX,[EAX+0xb2]`, `006b8ec4 CALL 0x6b83f0` HasFeat) | Mac GetWeaponFocus 0xB2 + GetBaseItem + HasFeat | 0x6B8E80 | to locate |

Below the batch (B12a range, found while gating B12b; not B12b rows):
- 0x68F7D0 `GetBlind` = `CSWSCreature::CancelAction`: callees GetAtPos, GetNext, Remove, ~CSWSObjectActionNode, free
  = Mac CancelAction (284 B); called by 0x6C6360 as Mac MoveAwayFromObject calls CancelAction. Steam 0x576CA0 (fp 1:1).
- 0x693490 `GetDetectMode` (0x51C/0x11AC) and 0x696DB0 `SetLastInstantEquip` (RemoveEffectById-like) are on wrong
  bodies; 0x693AC0 `AddPickUpItemActions` has exactly Mac ComputeArmourClass's disp set.

Not errors: the AIAction neighbours ChangeFacingObject 0x6C6080, MoveToObject 0x6C6D00, MoveToObjectRadius 0x6C6F90,
MoveToPoint 0x6C74C0, and the checks Alignment/Race/Feat/PC/Attribute/CNPC, ReplyToInvitation, AddToInvitationsOffered
keep their Mac shapes (14 AUTO rows audited, 12 pass).

**GOG DB param sizes (B12b).** 26 rows have a GOG DB `param_size_bytes` that disagrees with the GOG function's RET n
(gate_report `G4 metadata flags`), e.g. AIActionRest/Taunt/Appear 0 vs RET 4 and the QuickWalk WaitDest family 8 vs
RET 0x10. Steam ships its own RET n.

## B12a

### CSWSCreature, GOG < 0x696F00 (B12a: the combat-block rotation, ApplyOnHit* dispatch, Add*Actions shift, isolated errors)
B12a Opus gate parts a/r/z + consolidation, 2026-10-03 (`B12a/opus/overrides_{a,r,z}.md`, `B12a/B12a_SUMMARY.md`).
Every Steam counterpart below is fp 1:1 with its GOG body (size, retn, n_insn, disp, imm); only the GOG name is wrong.
All 29 rows are `label_suspect` in the B12a overrides and are not shipped under the GOG name. Steam = GOG + 0x83D00 in
the combat block.

**Combat-block rotation (GOG 0x657710-0x6614D0).** Ranged-combat and helper labels sit on the wrong bodies; three
bodies have no Mac symbol (inlined there).

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x657710 | `IsInTotalDefense_2` | CSWSCreature wrapper of CClientExoApp::GetClientRangedAnimation (`006db491 CALL 0x7404d0`) | inlined on Mac; Mac IsInTotalDefense 0xB1C/0x10DC/0x110C | 0x6DB460 | IsInTotalDefense = GOG 0x656D70 -> 0x6DAAC0 (AUTO, correct) |
| 0x658540 | `ResolvePostRangedDamage` | ResolveRangedAnimations (`006dc338 MOV [EAX+0xa98],ECX`, `006dc370 [ECX+0xb08]`, calls IsLightsaberEquipped 0x6E2C50) | Mac `ResolveRangedAnimations` 0xA98/0xB08, IsLightsaberEquipped + DecrementRoundLength + CheckActionLengthAtTime | 0x6DC290 | GOG 0x65DC30 -> 0x6E1930 |
| 0x658710 | `GetWeaponDischargeTime` | ResolveRangedHit (anim ids `006dc4b5 MOV EDX,0x271e` / 0x271C, target pos to attack +0x68..0x70) | Mac `ResolveRangedHit` 0x68/0x70, 0x94-0x9C (shape) | 0x6DC460 | GOG 0x65AAE0 (FUN_, 0xE0/0x130, calls GetWeaponDischargeRow) -> 0x6DE7E0 (shape only) |
| 0x6587E0 | `IsLightsaberEquipped` | ResolveRangedMiss (`006dc57e CALL 0x006dc5c0` ResolveProjectileMissLocation) | Mac `ResolveRangedMiss` calls ResolveProjectileMissLocation + magnitude + GetAttack | 0x6DC530 | GOG 0x65EF50 -> 0x6E2C50 |
| 0x65B530 | `ResolveRangedHit` | 0x59C dice helper (`006df24a`..`006df288 MOV [+0x59c],0/1/2`), caller ResolveMeleeAttack | inlined in Mac ResolveMeleeAttack (0x59C x3) | 0x6DF230 | GOG 0x658710 -> 0x6DC460 |
| 0x65C680 | `ResolveRangedMiss` | animation-id predicate (`006e03ec CMP ..,0x2af8`, ids 0x271F-0x2724, 0x274D..) | no Mac symbol | 0x6E0380 | GOG 0x6587E0 -> 0x6DC530 |
| 0x65C900 | `ResolveRangedAnimations` | attack-anim setter (`006e0625 MOV [EAX+0x14],CX`), called by ranged + melee anims | no Mac symbol | 0x6E0600 | GOG 0x658540 -> 0x6DC290 |
| 0x65DC30 | `ApplyOnHitSleep` | ResolvePostRangedDamage (`006e1bb3 CALL 0x580c50` GetIsWeaponEffective, GetTotalDamage, `PUSH 0x75`), caller ResolveRangedAttack | Mac `ResolvePostRangedDamage` same callee set, imm 0x34/0x75 | 0x6E1930 | GOG 0x661B60 -> 0x6E5860 |
| 0x65E710 | `ApplyOnHitSlayAlignmentGroup` | ResolveRangedMissToParry (`006e2440 CALL 0x6e24b0` IsProjectileParryable, `[+0x5c]=0xa/8`) | Mac `ResolveRangedMissToParry` 0x5C, imm 8/0xA; Mac SlayAlignmentGroup is a 5 B stub | 0x6E2410 | GOG 0x65EF50 -> 0x6E2C50 |
| 0x65EF50 | `ResolveRangedMissToParry` | IsLightsaberEquipped (`006e2c71 CALL 0x6d0620` GetItemInSlot 0x10/0x20, `006e2c82 CALL 0x615c80` IsLightsaber) | Mac `IsLightsaberEquipped` 0x1150, imm 0x10/0x20 | 0x6E2C50 | GOG 0x65E710 -> 0x6E2410 |
| 0x6614D0 | `ApplyOnHitHold` | CreateLinkedEffect, 2-effect overload (2x new 0x8C, `006e52e8 CALL 0x5e5260` SetLinked; 7 handler callers) | Mac `CreateLinkedEffect` 0x37817C (same 5 callees); 3-effect 0x37807E = GOG 0x661330 -> 0x6E5030 | 0x6E51D0 | GOG 0x661950 -> 0x6E5650 |

**ApplyOnHit* handlers (GOG 0x661950-0x662A30).** Identity from the ResolveOnHitEffect 12-case dispatch (Steam `006e3191
JMP [ECX*4+0x6e361c]`, case order = Mac declaration order) and each body's imm set.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x661950 | `ApplyOnHitSlow` | ApplyOnHitHold (case 2, `006e5664 PUSH 0x5`) | Mac Hold imm 5, 0xE8 | 0x6E5650 | GOG 0x661E10 (FUN_, case 5, imm 0x5F) -> 0x6E5B10 |
| 0x661B60 | `ApplyOnHitPoison` | ApplyOnHitSleep (case 0, `006e58ed PUSH 0x5e`) | Mac Sleep imm 0x5E | 0x6E5860 | GOG 0x6625D0 -> 0x6E62D0 |
| 0x662060 | `ApplyOnHitSlayRacialGroup` | ApplyOnHitStun (case 1, `006e5e03 PUSH 0x1d`) | Mac Stun imm 0x1D, 0xD0 | 0x6E5D60 | GOG 0x662690 (FUN_, case 8, 0xE0/0x1198) -> 0x6E6390 |
| 0x6625D0 | `ApplyOnHitInstantDeath` | ApplyOnHitPoison (case 7, `006e62e2 PUSH 0x17`) | Mac Poison imm 0x17 | 0x6E62D0 | GOG 0x662900 -> 0x6E6600 |
| 0x662900 | `ApplyOnHitKnockdown` | ApplyOnHitInstantDeath (case 10, SetInt(0,0),(1,1)) | Mac InstantDeath 6 callees | 0x6E6600 | GOG 0x662A30 (FUN_, case 11, float 0.1) -> 0x6E6730 |

**Add*Actions shifted by one (GOG 0x680A70-0x680C50).** Mac order: AddPickUpItemActions, AddDropItemActions,
AddHealActions; the GOG labels are one slot late.

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x680A70 | `AddDropItemActions` | AddPickUpItemActions (`00567ee0 PUSH 0x7` + AddAction; caller ExecuteCommandPickUpItem) | Mac `AddPickUpItemActions` disp 0x8/0xEC, imm 3,3,7 | 0x567E40 | GOG 0x680B30 -> 0x567F00 |
| 0x680B30 | `AddHealActions` | AddDropItemActions (`00568005 PUSH 0x9`, `[EAX+0x520]`, `PUSH 0xc2`; caller ExecuteCommandPutDownItem) | Mac `AddDropItemActions` 0x520/0x1150, imm 9/0xC2 | 0x567F00 | GOG 0x680C50 -> 0x568020 |
| 0x680C50 | `Rest` | AddHealActions (`00568035 PUSH 0x7` GetCanUseSkill TreatInjury, `005680b1 PUSH 0x38` + AddAction) | Mac `AddHealActions` 0xEC/0x1198, imm 7/0x38; Mac Rest is 792 B | 0x568020 | to locate (the other GOG `Rest` label, 0x696E80, is GetRelativeWeaponSize) |

**Isolated label errors (no rotation).**

| GOG addr | GOG DB name | body is | Mac evidence | Steam (body) | name really at |
|---|---|---|---|---|---|
| 0x494D00 | `CSWSCreature::CanUseForcePowerOrSpell` | `CSWCCreatureStats::HasSpell` (`0084a1af LEA ECX,[EDX+ECX*1+0xdc]`, calls HasKnownSpell twin) | Mac CSWCCreatureStats::HasSpell 0x32/0xDC, imm 5/8 | 0x84A170 | to locate |
| 0x68A4B0 | `ResolveInitiative` | 28 B `arg<10` predicate (`00571987 CMP [EBP+8],0xa`) | no Mac symbol | 0x571980 | GOG 0x693490 -> 0x57A960 |
| 0x68DDF0 | `RemoveMindTrickEffects` | CFollowInfo ctor (zeroes +4..+0x24, `0057532c CALL 0x575340`) | KDB CFollowInfo::CFollowInfo (high) | 0x5752C0 | GOG 0x696DB0 -> 0x57E280 |
| 0x68F7D0 | `GetBlind` | CancelAction (GetAtPos/GetNext/Remove/~CSWSObjectActionNode/free, `00576ce0 [ECX+0x6c]==arg`) | Mac `CancelAction` 284 B, same callees, vt 0x78 | 0x576CA0 | GOG 0x69DF00 -> 0x585420 (B12b) |
| 0x692510 | `SetAmbientAnimationState` | AIUpdate helper (`005799ed MOV [EAX+0x578],ECX`, `[+0x57c]=0x5dc`) | inlined in Mac AIUpdate; Mac setter writes +0x504 | 0x5799E0 | likely GOG 0x6A4260 -> 0x58B780 (shape only) |
| 0x693490 | `GetDetectMode` | ResolveInitiative (`0057a96c [EAX+0x11ac]`, SETZ, `0057a981 MOV [EAX+0x51c],EDX`) | Mac `ResolveInitiative` 27 B, 0x51C/0x11AC | 0x57A960 | GOG 0x698DF0 -> 0x5802C0 (B12b) |
| 0x693AC0 | `AddPickUpItemActions` | ComputeArmourClass (GetItemInSlot(2), `0057afd0 [EDX+0x8a]` -> `0057aff8 [EDX+0xfa]`; callers Equip/UnequipItem) | Mac `ComputeArmourClass` 0x8A/0xFA/0x1150/0x1198 | 0x57AF90 | GOG 0x680A70 -> 0x567E40 |
| 0x696DB0 | `SetLastInstantEquip` | RemoveMindTrickEffects (`0057e306 CMP EAX,0x12` / 0x13 -> RemoveEffectById) | Mac `RemoveMindTrickEffects` imm 0x12/0x13, 0x148/0x14C; KDB RemoveMindTrickAndScrambleEffects (high) | 0x57E280 | GOG 0x6A3E00 -> 0x58B320 (B12b) |
| 0x692330 | `BroadcastForcePointUsageData` | BroadcastStealthDetectionData (`005798c1 PUSH 0x10` message id, sole caller DoSpotDetection 0x578850) | Mac `BroadcastStealthDetectionData` 197 B, imm 0x10, sole caller DoSpotDetection; no Mac ForcePointUsageData symbol | 0x579800 | to locate (if it exists on Windows at all) |
| 0x696E80 | `Rest` (batch name `Rest_2`) | GetRelativeWeaponSize (`0057e35c [EAX+0x53c]` creature size vs `0057e38c [ECX+0x1b]` weapon size, `0057e3af MOV EAX,0xfffffff6`) | Mac `GetRelativeWeaponSize` 71 B, 0x1B/0x34/0x53C, imm -10, callers Get{Ranged,Melee}AttackBonus/GetAttackModifierVersus | 0x57E350 | to locate (Mac Rest 792 B); label `GetRelativeWeaponSize` is on 0x6F5300 = Stats::GetWeaponFocus (B12b) |

GOG pseudo-function: 0x692F53 `BroadcastStealthDetectionData` is 0x692F50 + 3 (mid-function). The whole body is Steam
0x57A420 = BroadcastPoisoned (`0057a538 PUSH 0x1b`); real BroadcastStealthDetectionData is probably GOG 0x692330
(label BroadcastForcePointUsageData, imm 0x10) -> 0x579800 (shape only). Row rejected.

Not errors (37 AUTO rows read by hand in the windows + opus_gate meaning checks, 32 pass; a scripted same-name Mac
sweep of all 150 AUTO rows found only the 2 isolated rows 0x692330 / 0x696E80 above): ResolveRangedSpecialAttack,
ResolveSafeProjectile, ResolveProjectileMiss{Location,Impact}, SignalDamage, Signal{Ranged,Melee}Damage, the
GetWeapon*{ImpactTime,DischargeRow,DischargeHand} getters, ResolveMeleeAttack, ResolveAttackRoll, ResolveSneakAttack,
ResolveDamage, ResolvePostMeleeDamage, ResolveDefensiveEffects, IsProjectileParryable, ResolveMoveToForceJump,
ResolveOnHitVisuals, ResolveDamageShields, DoCombatStep, CreateLinkedEffect 0x661330, ApplyOnHitAbilityDamage,
SendAttackSummaryFeedback, AddMineActions keep their Mac shapes.

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

**B17 (fingerprint twins, 10-28).** 7 label_suspect rows, none shipped (full list `B17/gog_label_suspects.tsv`):
- CSWGuiCreateItem / CSWGuiCreateMedicalItem labels are rotated. GOG 0x5C5330 `InitializeStoreType` and 0x5CA130
  `DeleteItemEntries` have the body of Mac `OnControlXButton` (toggle +0x3ed8 / +0x33f8, ShowSellGUI/ShowBuyGUI).
  GOG 0x5C61F0 `UpdateCreateFilters` has the body of Mac `BuyItem` (`PUSH 0x30c` new item + CopyItem). GOG's own
  `OnControlXButton` 0x5C54B0 (3391 B, 'base_skill') is mislabelled too; its real name is unchecked.
- 0x5977A0 `CSWGuiBlackenedLabel::Destructor` = CSWGuiLabel::~CSWGuiLabel (writes the CSWGuiLabel vftable).
- 0x645160 `CVirtualMachineInternal::ExecuteCode_5` = Vector::operator/(float) (1/s, then operator*).
- 0x829060 `GLRender::FrameBufferModificationsATI` = VBO + multitexture array setup; right name unknown.
- 0x867DC0 `Global::GetResource` removes an array entry (calls CExoArrayList::DelIndex); right name unknown.
