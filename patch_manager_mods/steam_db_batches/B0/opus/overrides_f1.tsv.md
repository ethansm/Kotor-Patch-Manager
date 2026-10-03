# B0 Opus gate, part f1 (2026-10-03)

Scope: the 56 rows in `B0/opus/part_f1.md`. None had a Sonnet review. They are AUTO rows within 4 GOG positions of a confirmed GOG label error, plus 9 rows that G8 (mac_rotation) flipped to REVIEW. The main question for each row: does the GOG name fit the body?

Method: for each row I compared four things.
- The GOG fp (callees resolved to GOG names, disp, imm) against the Mac x86 symbol of the same name. Overloads `_2`/`_3` were compared to the Mac overloads by shape.
- The G8 neighbour against its own Mac symbol.
- The Steam asm, for a quoted anchor. Callee identity comes from size equality with the GOG callee, or from a known Steam DB row.
- Column-handle and string writers, traced in `006eb670` and `strings.tsv`.

Every row's Steam body equals its GOG counterpart in size, retn, disp and imm. The one exception is AddObjectToArea: 516 vs 515 B, with the fp otherwise equal.

Verdicts: **verified 48, likely 1, label_suspect 7, rejected 0, unresolved 0.**

## Per-row evidence (short form)

| # | GOG | GOG name | Steam | verdict | evidence |
|---|---|---|---|---|---|
| 1 | 0x4db260 | CGuiInGame::HideSubItemCreateMenu | 0x7cab60 | verified | `007cabfb CALL 0x8bf9f0` (ClearItems, 79 B) on [+0x5c]. Mac uses the same 6 callees |
| 2 | 0x4db430 | HideContainerGui | 0x7cad30 | verified | [+0x54] panel; `007cada5 CALL 0x740350` (SetPausedByCombat, 37 B). Mac matches |
| 3 | 0x4dc020 | ShowPartySelection | 0x7cb920 | verified | `PUSH 0x50b4` + `CALL 0x89cf30` (2904 B ctor); `CMP [+0xf18]`. Mac matches |
| 4 | 0x4dee80 | UpdateAreaTransitionGUI | 0x7ce780 | verified | calls 0x7ce970 (HideAreaTransition); `CMP [+0x1018],0x7f000000` |
| 5 | 0x4df070 | HideAreaTransition | 0x7ce970 | verified | SetTransitionObject 0x8b88b0 (487 B) + RemovePanel |
| 6 | 0x4df190 | GetBarkBubbleVisible | 0x7cea90 | verified | PanelExists on [+0x4c]; out-param; RET 4 |
| 7 | 0x4e1290 | GetCanClick | 0x7d0b90 | verified | [+0xfd0]/[+0xfd4] world-time compare. Mac 0xfc4-0xfcc, same 3 callees |
| 8 | 0x623a40 | CResGFF::GetFieldByLabel | 0x7178e0 | verified (G8 FP) | strncpy of a 16-char label, then GetField. G' AddField calls AddLabel/AddDataLayoutField, which this body does not |
| 9 | 0x625aa0 | CResGFF::AddStructToStruct | 0x719940 | verified (G8 FP) | `PUSH 0xe` (struct type), AddField + AddStruct. G' AddListElement uses 0xf + GetFieldByLabel |
| 10 | 0x626310 | CResGFF::WriteFieldVOID | 0x71a1b0 | verified (G8 FP) | `PUSH 0xd` (VOID) + memcpy. G' WriteFieldCResRef uses 0xb |
| 11 | 0x455e00 | CSWCArea::SetAsDay | 0x7a9400 | verified | [+0x90]/[+0x94], `ADD EAX,0x150` = Mac SetAsDay |
| 12 | 0x455f20 | SetAsNight | 0x7a9520 | verified | [+0x74]/[+0x78], `ADD EAX,0x12c` = Mac SetAsNight |
| 13 | 0x456060 | GetWeatherObject | 0x7a9660 | **label_suspect** | = **GetMapPin** (see the errors list) |
| 14 | 0x4560f0 | ClearWeatherObjects | 0x7a96f0 | **label_suspect** | = **ClearAllMapPins** (see the errors list) |
| 15 | 0x456180 | UpdateSoundObjectsTimeOfDay | 0x7a9780 | verified | [+0x1d4]/[+0x1d8] + UpdateTimeOfDay (253 B) |
| 16 | 0x456260 | SetLightningDone | 0x7a9860 | verified | rand, [+0x1e4], [+0x1a8] |
| 17 | 0x4562d0 | ApplyWeatherVisual | 0x7a98d0 | likely | No symbol of this name in Mac x86, Mac x64 or Linux. Body: arg 0/1/2 maps to flag 1/2/4, then `vt+0x4c` on scene [+0x188]; sole caller LoadArea. Counterpart is right (fp=); the name cannot be confirmed |
| 18 | 0x456390 | AddPlaceableObjectLight | 0x7a9990 | verified | 'placeablelight', 'fx_placeable01', NewCAurObject; [+0x21c] |
| 19 | 0x456940 | AddPlaceableObjectStatic | 0x7a9f40 | verified | 'LowGore', 'ModelName'; Mac callees match |
| 20 | 0x456ce0 | RemovePlaceableObjectLight | 0x7aa2e0 | verified (G8 FP) | DelIndex + free on the [+0x21c] list. G' (Get) has no callees |
| 21 | 0x456de0 | GetPlaceableObjectLight | 0x7aa3e0 | verified | bounds [+0x220], array [+0x21c], no callees = Mac 58 B |
| 22 | 0x456e60 | HiliteAreaObjects | 0x7aa460 | verified | [+0x210], [+0x2f4] |
| 23 | 0x4571a0 | UpdateShadowingLights | 0x7aa7a0 | verified | [+0x18c] + GetCreatureLight |
| 24 | 0x4571f0 | GetIsDark | 0x7aa7f0 | verified (G8 FP) | writes [+0x238], reads [+0x188] = Mac GetIsDark. G' has only 0x18c |
| 25 | 0x457340 | RestartMiniGame | 0x7aa940 | verified | `PUSH 0x11c` CSWMiniGame alloc, [+0x268] |
| 26-28 | 0x457540 / 0x457590 / 0x4575d0 | GetRoom / _2 / _3 | 0x7aab40 / 0x7aab90 / 0x7aabd0 | verified | Each matches a Mac overload: (Vector) calls GetRoomIndex; (int) is 0x264 + idx*0x48; (CExoString) uses CompareNoCase over 0x260/0x264 |
| 29 | 0x474660 | CSWCObject::AnimationLooping | 0x7ef500 | verified | reads `[0x00a0fe24]`, the 'Looping' column handle (`006eb8e6 PUSH 0x993244` -> `006eb91b MOV [0x00a0fe24]`) |
| 30 | 0x474730 | AnimationFireAndForget | 0x7ef5d0 | verified | reads `[0x00a0fe1c]`, the 'FireForget' handle (`006eb854 PUSH 0x99af98` -> `006eb889`) |
| 31 | 0x4748e0 | AnimationDialog | 0x7ef780 | verified | GetInGameGui, then 0x7cb010 (IsFireForgetDialogAnimation) + 0x7cae60 |
| 32 | 0x705570 | CSWSArea::GetRoom | 0x54b1d0 | verified (G8 FP) | only CheckAABBWalkable. G' GetRoomIndex also calls CheckAABBAll, as Mac GetRoomIndex does |
| 33 | 0x707770 | SetHitInfoForCreature | 0x54d3d0 | **label_suspect** | = **EvaluateOverlappingTargets** (see the errors list) |
| 34 | 0x707af0 | NoCreaturesOnLine | 0x54d750 | verified | [+0x11ac], GetIsCreatureBumpable (551 B), [+0x26a] |
| 35 | 0x708410 | ShrinkLineByCreatures | 0x54e070 | verified | IntersectCircle2d 0x5192c0 + GetFirstObjectIndiceByX |
| 36 | 0x709200 | SmoothCornerOptimize | 0x54ee60 | verified | ComputeBestCorner 0x54aee0 (578 B) |
| 37 | 0x709980 | SmoothPointsOnPath | 0x54f5e0 | **label_suspect** | = SmoothPointsOnPathRestartable (known from e2; re-checked: disp 0x214-0x234, calls 0x54fb10) |
| 38 | 0x709d30 | SmoothPointsOnPathRestartable | 0x54fb10 | **label_suspect** | = SmoothSelectionRestartable (e2; calls 0x5502d0 x3, [+0x218]) |
| 39 | 0x70a060 | SmoothSelectionRestartable | 0x54fe40 | **label_suspect** | = SmoothSelectNodes (e2; TestDirectLine + ComputePathDistance, no timer) |
| 40 | 0x70a4f0 | SmoothSelectNodesRestartable | 0x5502d0 | verified | GetHighResolutionTimer + [+0x230..0x238] = Mac |
| 41 | 0x70aba0 | TestDirectLine | 0x550980 | verified | NoCreaturesOnLine, GetRoom, NoNonWalkPolysDetailed |
| 42 | 0x70b050 | PlotGridPath | 0x550e30 | verified | 'Exceeded limits:Max search deapth'; calls 0x552a90 (Init), 0x551c30, 0x54f3d0 (real SmoothPointsOnPath) = Mac call set |
| 43 | 0x70be50 | GridDFSearch | 0x551c30 | verified | self-recursive; 0x551f50 + 0x552b70 (TransTableGet) |
| 44 | 0x70c170 | GridDFSGenerateSuccessors | 0x551f50 | verified | TestDirectLine is the only real callee, as on Mac |
| 45 | 0x70ccb0 | GridDFSTransTablePut | 0x552a90 | **label_suspect** | = GridDFSTransTableInitialize (b/c; `PUSH 0x2000`, imm 0x80000000) |
| 46 | 0x70cfb0 | NoNoneWalkPolysInStaticObject | 0x552d90 | verified | NoNonWalkPolys + NoNonWalkPolysInDoor, RET 0x24 |
| 47 | 0x70d080 | ClosestPathPoint | 0x552e60 | verified | TestDirectLine, [+0x260] |
| 48 | 0x70d510 | PlotPathPointPath | 0x5532f0 | verified | 'Error could not find closest path node to start or end' |
| 49 | 0x70f0d0 | PathPointDFSearch | 0x554eb0 | verified | '.\nwsareapathfind.cpp', calls 0x5555c0 (GenerateSuccessors, 1856 B) |
| 50 | 0x717ca0 | GetFirstObjectIndiceByX | 0x522af0 | verified | [+0x194]/[+0x198] binary search + GetGameObject |
| 51 | 0x717da0 | AddObjectToArea | 0x522bf0 | verified | calls RemoveObjectFromArea + GetFirstObjectIndiceByX; 516 vs 515 B is padding (disp/imm/callee count equal) |
| 52 | 0x717fb0 | UpdatePositionInObjectsArray | 0x522e00 | verified | IndexOf + [+0x194] array moves |
| 53 | 0x718240 | RemoveObjectFromArea | 0x523090 | verified | DecrementPlayersInArea 0x523360 + Remove |
| 54 | 0x718550 | IncreaseAILevelPriority | 0x5233a0 | verified | strlen + SetAILevel (only Increase has strlen on Mac) |
| 55 | 0x718750 | DecreaseAILevelPriority | 0x5235a0 | verified | SetAILevel, no strlen |
| 56 | 0x718860 | CSWSArea::LoadArea | 0x5236b0 | verified | LoadAreaHeader (8235 B) + LoadGIT (814 B) |

## G8 rows (9): result
Five G8 flags are false positives; the name is right. Mac evidence for each:
- GetFieldByLabel: strncpy + GetField.
- AddStructToStruct: type 0xe.
- WriteFieldVOID: type 0xd.
- RemovePlaceableObjectLight: DelIndex/free.
- GetIsDark: 0x238.

A sixth, CSWSArea::GetRoom, is also a false positive: it has no CheckAABBAll. That makes 6 of 9.

The other 3 (0x709980 shown as SmoothPointsOnPath, 0x70a060, 0x70ccb0) are real rotations already known from parts b/c/e2. G8 named the right direction each time.

## GOG label errors found
New in this part:

| GOG addr | GOG name | the body is | Mac evidence | Steam (body) | where the named function really is |
|---|---|---|---|---|---|
| 0x707770 | CSWSArea::SetHitInfoForCreature | **EvaluateOverlappingTargets** | Mac EvaluateOverlappingTargets 0x4261de calls GetCreatureByGameObjectID + normalize, disp 0x2c,0x30,0x380. GOG 0x707770 calls the same, with the same disp. Mac SetHitInfoForCreature calls IntersectCircle2d + normalize, disp up to 0x44; GOG 0x707770 has no IntersectCircle2d. Steam `0054d404 CALL 0x51c100`, `0054d551 MOV EDX,[ECX+0x380]` | 0x54d3d0 (the knowledge DB already names it SetHitInfoForCreature [high]: wrong) | GOG **0x710260** (DB `FUN_00710260`, 1130 B, RET 0x14, IntersectCircle2d 0x811150, disp 0x44) -> Steam **0x556040** (fp equal; `0055609a CALL 0x5192c0` IntersectCircle2d). Both are called only by NoCreaturesOnLine, on GOG and on Steam, as on Mac |
| 0x456060 | CSWCArea::GetWeatherObject | **GetMapPin** | Mac GetMapPin disp 0x108,0x10c,0x1c8,0x1cc, exact. Mac has no WeatherObject API. Sole GOG caller is CSWCMessage::HandleServerToPlayerMapPinEnabled 0x4c1e60; Mac's handler calls GetMapPin. Steam `007a9699 MOV EDX,[ECX+0x1c8]` / `007a96b1 MOV EAX,[EDX+0x108]`, caller 0x82d5e0 (142 B = GOG handler) | 0x7a9660 | The name has no real function: Mac has no GetWeatherObject |
| 0x4560f0 | CSWCArea::ClearWeatherObjects | **ClearAllMapPins** | Mac ClearAllMapPins clears 0x1c8/0x1cc and is called by UnloadArea. GOG 0x4560f0 does the same (disp 0x1c8,0x1cc, SetSize), and its only caller is UnloadArea 0x4529a0. Steam `007a9722 MOV EDX,[ECX+0x1c8]`, `007a9770 CALL 0x840ef0`, caller 0x7a5fa0 UnloadArea | 0x7a96f0 | No real function: Mac has no ClearWeatherObjects. GOG `ClearAllMapPins` 0x457310 is ClearTilePathNodes (part a) |

Rows 13-14 confirm the part-a hypothesis: the map-pin family is labelled as weather objects. The Steam rows can now ship as `CSWCArea::GetMapPin = 0x7a9660` and `CSWCArea::ClearAllMapPins = 0x7a96f0`. Both need an Opus override; the evidence is above.

Already known and re-confirmed here (no new decision): 0x709980, 0x709d30 and 0x70a060 (Smooth* rotation, e2), and 0x70ccb0 (GridDFSTransTableInitialize, b/c).

Not a confirmed error: 0x4562d0 `ApplyWeatherVisual`. No symbol of that name exists in any build. The body sets a 1/2/4 flag through scene `vt+0x4c`, and LoadArea calls it. It could be an inlined Mac helper given a descriptive name. It stays `likely`.

## Systematic
- The CSWCArea weather/map-pin block (0x456040..0x4560f0) is mislabelled as a group. 0x456040, 0x456060 and 0x4560f0 are AddMapPin, GetMapPin and ClearAllMapPins.
- The rest of the CSWCArea block (SetAsDay..GetRoom_3) is correct. The near-error flag there comes only from the map-pin rows and 0x457310 / 0x456350.
- In the CSWSArea pathfinding block, the labels are correct outside 0x709770..0x70ceb0. One new error is outside that run: 0x707770 is EvaluateOverlappingTargets, and SetHitInfoForCreature is the unlabelled 0x710260. G7 (0.57) and G8 both passed 0x707770, so the gates missed this error.
- The CGuiInGame rows near the known rotations (0x4db260..0x4e1290) and the CSWCObject rows near the swapped Pause/PlayOutOfPlace pair are all correct.
- The KPM/knowledge DB names Steam 0x54d3d0 `SetHitInfoForCreature` [high]. That row needs the same correction.

## sample_errors: 0
This part has no sampled reviewer `match` rows.

## AUTO meaning checks
"pass" means the GOG name fits the body (Mac shape + a Steam anchor).

| GOG | name | meaning check |
|---|---|---|
| 0x4db260 | HideSubItemCreateMenu | pass |
| 0x4db430 | HideContainerGui | pass |
| 0x4dc020 | ShowPartySelection | pass |
| 0x4dee80 | UpdateAreaTransitionGUI | pass |
| 0x4df070 | HideAreaTransition | pass |
| 0x4df190 | GetBarkBubbleVisible | pass |
| 0x4e1290 | GetCanClick | pass |
| 0x455e00 | SetAsDay | pass |
| 0x455f20 | SetAsNight | pass |
| 0x456060 | GetWeatherObject | **FAIL** (GetMapPin) |
| 0x4560f0 | ClearWeatherObjects | **FAIL** (ClearAllMapPins) |
| 0x456180 | UpdateSoundObjectsTimeOfDay | pass |
| 0x456260 | SetLightningDone | pass |
| 0x4562d0 | ApplyWeatherVisual | n/a (no Mac symbol) -> likely |
| 0x456390 | AddPlaceableObjectLight | pass |
| 0x456940 | AddPlaceableObjectStatic | pass |
| 0x456de0 | GetPlaceableObjectLight | pass |
| 0x456e60 | HiliteAreaObjects | pass |
| 0x4571a0 | UpdateShadowingLights | pass |
| 0x457340 | RestartMiniGame | pass |
| 0x457540 / 0x457590 / 0x4575d0 | GetRoom / _2 / _3 | pass |
| 0x474660 | AnimationLooping | pass |
| 0x474730 | AnimationFireAndForget | pass |
| 0x4748e0 | AnimationDialog | pass |
| 0x707770 | SetHitInfoForCreature | **FAIL** (EvaluateOverlappingTargets) |
| 0x707af0 | NoCreaturesOnLine | pass |
| 0x708410 | ShrinkLineByCreatures | pass |
| 0x709200 | SmoothCornerOptimize | pass |
| 0x709d30 | SmoothPointsOnPathRestartable | **FAIL** (known, e2) |
| 0x70a4f0 | SmoothSelectNodesRestartable | pass |
| 0x70aba0 | TestDirectLine | pass |
| 0x70b050 | PlotGridPath | pass |
| 0x70be50 | GridDFSearch | pass |
| 0x70c170 | GridDFSGenerateSuccessors | pass |
| 0x70cfb0 | NoNoneWalkPolysInStaticObject | pass |
| 0x70d080 | ClosestPathPoint | pass |
| 0x70d510 | PlotPathPointPath | pass |
| 0x70f0d0 | PathPointDFSearch | pass |
| 0x717ca0 | GetFirstObjectIndiceByX | pass |
| 0x717da0 | AddObjectToArea | pass |
| 0x717fb0 | UpdatePositionInObjectsArray | pass |
| 0x718240 | RemoveObjectFromArea | pass |
| 0x718550 | IncreaseAILevelPriority | pass |
| 0x718750 | DecreaseAILevelPriority | pass |
| 0x718860 | LoadArea | pass |

AUTO meaning: 47 rows checked (counting each GetRoom overload). 42 pass, 4 FAIL (3 new + 1 known), 1 n/a.
