# AI0 GOG->Steam name mapper run over AI classes (2026-10-02)

Read-only run. Mapper `gog_cluster_delta_map.py`, one run per class, outputs `ROOT/findings/AI0_map_<label>.tsv`
(ROOT = ghidra_dumps/steam_6a522e71). DB opened `mode=ro` (3007 name_map rows; `inputs/name_map.tsv` has 2835 and is a strict subset).

Headlines
- 1097 mapped rows over 38 classes (1069 vote, 28 drift); 407 vote rows are NEW (steam_addr not in DB name_map), 643 AGREE, 34 CONFLICT (unique).
- The mapper is deterministic: the 6 re-run classes are byte-identical to `U3_map_*.tsv` (0 diff lines); `U1_map_vm.tsv` is a subset of the new VM runs (+11 `CVirtualMachineStack` rows). The U3 rows (CServerAIMaster, CSWSCombatRound ... ) were largely never persisted to the DB (AIMaster, EffectListHandler, GameEffect, AOE; CSWSCombatRound and CSWSObject are persisted); that is where the "new" comes from.
- WARNING: a combined regex `CVirtualMachine` (several classes in one run) produced address collisions (two GOG names on one Steam addr, off by one slot vs U1). Use one class per run; the combined output was discarded and replaced by 4 per-class files.
- CSWSCreature run still contains a 19-row outlier cluster (4 delta groups) that includes 7 of the 9 known false addresses; all 7 are mapped again (see "Known false mappings"). Treat those 19 rows as FALSE until a decompile says otherwise.
- Mapper is WRONG on the 0x552A90/0x552C90 GridDFSTransTable pair (R9 correction right) and the DB (mods678 "high") is WRONG on 0x5644B0 (it is AddMoveToPointActionToFront, mapper right).
- `CPathfind` does not exist in GOG DB; `CPathfindInformation` fails (12/12 vote tie, 0 rows, same as U3). 15 other small classes fail (list at end).

## 1. Per-run table
Columns: total/vote/drift = rows in the TSV. new-vote = vote rows whose steam_addr is not in DB. agree = same addr and same `Class::Func` (case-insens; a DB name that starts with the mapper name, e.g. "X ret 8 (map OK)", counts as agree). conflict = DB has a different name at that addr, OR the same GOG name sits at a different addr in DB/U1/U3 (counted on every row, vote+drift). collide = Steam addr claimed by two GOG names inside the AI0 outputs. unmapped = GOG fns the mapper could not place (from its log). `_ai` rows are filtered views (name regex on the function part) of the full file above them and are excluded from the TOTAL.
Filter regexes used: CSWSCreature `AIAction|Combat|Attack|Perception|Follow|Path|Move|Reputation|Faction|Hostile|Enemy|Visible|Stealth|Talent|Spell|Effect|Event|Script|Party|Formation|Bump`; CSWSObject `Action|AI|Combat|Attack|Perception|Reputation|Faction|Hostile|Enemy|Visible|Stealth|Effect|Event|Script|Path|Move|Target|Heartbeat|Command|Queue|Spell|Talent`; CSWSArea `LOS|LineOfSight|Path|Walk|Height|Surface|Nearest|Perception|AIUpdate|Object`.

| label | regex | GOG fns | total | vote | drift | new-vote | agree | conflict | collide | unmapped | failed? |
|---|---|---|---|---|---|---|---|---|---|---|---|
| CAurPathInfo | `^CAurPathInfo$` | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | FAILED (0 rows) |
| CClientAIList | `^CClientAIList$` | 5 | 5 | 5 | 0 | 2 | 0 | 3 | 0 | 0 | ok |
| CClientAIMaster | `^CClientAIMaster$` | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | FAILED (0 rows) |
| CCombatInformation | `^CCombatInformation$` | 6 | 6 | 6 | 0 | 6 | 0 | 0 | 0 | 0 | ok |
| CCombatInformationNode | `^CCombatInformationNode$` | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | FAILED (0 rows) |
| CFactionManager | `^CFactionManager$` | 15 | 15 | 15 | 0 | 15 | 0 | 0 | 0 | 0 | ok |
| CGameEffect | `^CGameEffect$` | 21 | 21 | 21 | 0 | 20 | 1 | 0 | 0 | 0 | ok |
| CPathfind | `^CPathfind$` | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | FAILED (0 rows) |
| CPathfindInformation | `^CPathfindInformation$` | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 19 | FAILED (0 rows) |
| CSWCCreaturePartyStatus | `^CSWCCreaturePartyStatus$` | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | FAILED (0 rows) |
| CSWParty | `^CSWParty$` | 24 | 22 | 21 | 1 | 20 | 0 | 1 | 0 | 2 | ok |
| CSWPartyFollowDataArray | `^CSWPartyFollowDataArray$` | 10 | 8 | 8 | 0 | 8 | 0 | 0 | 0 | 2 | ok |
| CSWPartyFollowPathData | `^CSWPartyFollowPathData$` | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | FAILED (0 rows) |
| CSWPartyMemberData | `^CSWPartyMemberData$` | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | FAILED (0 rows) |
| CSWPartyTable | `^CSWPartyTable$` | 80 | 79 | 78 | 1 | 74 | 2 | 2 | 4 | 1 | ok |
| CSWSArea | `^CSWSArea$` | 118 | 107 | 103 | 4 | 0 | 104 | 4 | 0 | 11 | ok |
| CSWSAreaMap | `^CSWSAreaMap$` | 16 | 14 | 14 | 0 | 14 | 0 | 0 | 0 | 2 | ok |
| CSWSAreaOfEffectObject | `^CSWSAreaOfEffectObject$` | 22 | 19 | 19 | 0 | 17 | 2 | 0 | 0 | 3 | ok |
| CSWSArea_ai | `filter` | 118 | 24 | 24 | 0 | 0 | 22 | 2 | 0 | - | (filtered view) |
| CSWSCombatAttackData | `^CSWSCombatAttackData$` | 18 | 16 | 16 | 0 | 16 | 0 | 0 | 0 | 2 | ok |
| CSWSCombatRound | `^CSWSCombatRound$` | 53 | 49 | 48 | 1 | 0 | 49 | 0 | 0 | 4 | ok |
| CSWSCombatRoundAction | `^CSWSCombatRoundAction$` | 3 | 3 | 3 | 0 | 3 | 0 | 0 | 0 | 0 | ok |
| CSWSCombatSubAttack | `^CSWSCombatSubAttack$` | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | FAILED (0 rows) |
| CSWSCreaturePartyFollowInfo | `^CSWSCreaturePartyFollowInfo$` | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | FAILED (0 rows) |
| CSWSCreature_ai | `filter of CSWSCreature_all (name regex)` | 402 | 135 | 134 | 1 | 0 | 130 | 6 | 3 | - | (filtered view) |
| CSWSCreature_all | `^CSWSCreature$` | 402 | 349 | 345 | 4 | 0 | 339 | 11 | 7 | 53 | ok |
| CSWSEffectListHandler | `^CSWSEffectListHandler$` | 157 | 87 | 79 | 8 | 79 | 0 | 0 | 0 | 70 | partial |
| CSWSFaction | `^CSWSFaction$` | 30 | 28 | 28 | 0 | 28 | 0 | 0 | 0 | 2 | ok |
| CSWSForcedAction | `^CSWSForcedAction$` | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | FAILED (0 rows) |
| CSWSObject | `^CSWSObject$` | 144 | 114 | 112 | 2 | 0 | 113 | 1 | 0 | 30 | ok |
| CSWSObjectActionNode | `^CSWSObjectActionNode$` | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | FAILED (0 rows) |
| CSWSObject_ai | `filter` | 144 | 52 | 51 | 1 | 0 | 51 | 1 | 0 | - | (filtered view) |
| CSWTrackFollower | `^CSWTrackFollower$` | 47 | 45 | 44 | 1 | 44 | 0 | 0 | 0 | 2 | ok |
| CSWVirtualMachineCommands | `^CSWVirtualMachineCommands$` | 29 | 21 | 19 | 2 | 0 | 20 | 1 | 1 | 8 | partial |
| CServerAIBodyBagInfo | `^CServerAIBodyBagInfo$` | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | FAILED (0 rows) |
| CServerAIEventNode | `^CServerAIEventNode$` | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | FAILED (0 rows) |
| CServerAIList | `^CServerAIList$` | 6 | 6 | 5 | 1 | 3 | 0 | 2 | 0 | 0 | ok |
| CServerAIMaster | `^CServerAIMaster$` | 21 | 19 | 17 | 2 | 10 | 2 | 5 | 0 | 2 | ok |
| CVirtualMachine | `^CVirtualMachine$` | 23 | 22 | 22 | 0 | 20 | 2 | 0 | 0 | 1 | ok |
| CVirtualMachineInternal | `^CVirtualMachineInternal$` | 37 | 31 | 31 | 0 | 21 | 9 | 1 | 0 | 6 | ok |
| CVirtualMachineScript | `^CVirtualMachineScript$` | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | FAILED (0 rows) |
| CVirtualMachineStack | `^CVirtualMachineStack$` | 14 | 11 | 10 | 1 | 7 | 0 | 3 | 0 | 3 | ok |
| TOTAL (excl. filtered views) | | | 1097 | 1069 | 28 | 407 | 643 | 34 | | | |

Run 12 classes found by name (`AI|Faction|Reput|Path|Follow|Party`) and not in runs 1-11: CServerAIList, CServerAIBodyBagInfo, CServerAIEventNode, CClientAIList, CClientAIMaster, CCombatInformation(+Node), CSWSCombatAttackData, CSWSCombatRoundAction, CSWSCombatSubAttack, CSWSObjectActionNode, CSWSForcedAction, CSWParty, CSWPartyTable, CSWPartyFollowDataArray, CSWPartyFollowPathData, CSWPartyMemberData, CSWSCreaturePartyFollowInfo, CSWCCreaturePartyStatus, CAurPathInfo, CSWTrackFollower, CSWSAreaMap (all mapped above). Skipped as not AI: CSWGuiParty* (4 GUI classes), CAurBehaviorCameraFollow, CAuroraInterface, CSWGuiMainInterfaceAction/Char, CSWGuiTargetActionMenu, CSWGuiInterfaceAction.

## 2. Conflicts (34 unique, vote+drift) and verdicts
Verdict key: MAPPER OK = mapper name right, DB row wrong/stale; ALIAS = same function, DB uses a descriptive/other label (both fine); MAPPER WRONG; COLLISION = two mapper names on one addr, at most one right; UNVERIFIED.
Spot-checked rows quote the decompile first line `// FN addr ... size= ret=`.

| addr | mapper name (tag, retn) | DB name (source, conf) | verdict |
|---|---|---|---|
| 0x005644b0 | CSWSCreature::AddMoveToPointActionToFront (vote, 68) | AddMoveToPointAction (mods678-batch3-dump, high) | MAPPER OK (checked). Body calls 0x540ca0 = CSWSObject::AddActionToFront (DB); sibling 0x564770 calls 0x53f7f0 = AddAction. DB has the SAME name on both addrs. Fix 0x5644b0 to ...ToFront. |
| 0x00564770 | CSWSCreature::AddMoveToPointAction (vote, 68) | (name at 0x5644b0 instead) | MAPPER OK (checked, same evidence) |
| 0x00552a90 | CSWSArea::GridDFSTransTablePut (vote, 0) | GridDFSTransTableInitialize (R9-Phase2 correction, medium) | MAPPER WRONG (checked): `size=173 ret=0 callees=00919723`, allocs 0x2000 table = Initialize. GOG param sizes for Put(0)/PathPointDFSTransTableInitialize(16) are swapped in the GOG DB. |
| 0x00552c90 | CSWSArea::PathPointDFSTransTableInitialize (vote, 16) | GridDFSTransTablePut (R9, medium) | MAPPER WRONG (checked): `size=252 ret=16 callees=00552b40` (Get = hash) = keyed store (Put). |
| 0x0054f3d0 | CSWSArea::GridDFSTransTableInitialize (vote, 0) | same name (mapper, medium) but 0x552A90 holds it via R9 | MAPPER WRONG: `size=518 ret=0 callees=0054fdb0,0054ee60`, caller 0x5532F0 PlotPathPointPath; not an init of a 0x2000 table. Stale DB row. |
| 0x007a2010 / 0x007a2110 / 0x007a21d0 | CClientAIList::Constructor / AddObject / RemoveObject (vote) | CClientExoAppInternal::GetLoadScreenVisible / AddMoveToModuleMovie_2 / SetGameOver (mapper drift, low) | MAPPER OK (checked). 0x7a2010 `size=44 ret=0`: `param_1[2] = 0; *param_1 = 0;` (list ctor); 0x7a2110 `ret=4 ... *param_1 = *param_1 + 1; ... (param_2 + 0x1c) = iVar1` (add node, count++); 0x7a21d0 `ret=4` uses `*(param_2+0x1c)` (remove). DB low rows should go. |
| 0x0051d1c0 | CServerAIList::GetNextObject (vote, 0) | CServerAIMaster::NextObject (round-robin cursor) (U3-Phase4, medium) | MAPPER OK on class (checked): `size=165 ret=0 callers=0051d790` body `param_1[3] = param_1[3] + 1; if (param_1[1] <= param_1[3]) param_1[3] = 0;` round-robin over an id array = AIList method, GOG has it under CServerAIList. |
| 0x0051d270 | CServerAIList::GetObjectAtPosition (vote, 4) | CServerAIMaster::GetObjectAt(list,i) (U3, medium) | ALIAS/class attribution; `size=122 ret=4 callees=0051c0a0` fits |
| 0x0051d790 | CServerAIMaster::UpdateState (vote, 0) | ServerEvents::Update (U1, medium) | ALIAS (checked): `size=2113 ret=0 callers=00533830`, calls EventPending 0x51ed10, GetPendingEvent 0x51ed80, AIList Next/At 0x51d1c0/0x51d270, DeleteEventData 0x520ce0. UpdateState is the GOG name. |
| 0x0051dfe0 / 0x0051e170 / 0x0051e220 / 0x00520ce0 | CServerAIMaster::ClearEventQueue / AddEventDeltaTime(24) / AddEventAbsoluteTime(24) / DeleteEventData(8) | Events::ClearAll / ServerEvents::AddEventDeltaTime / EventQueue::AddEvent / Events::FreePayload (U1, medium) | ALIAS (names differ, rets 24/24/8 match GOG) |
| 0x007e6ce0 | CSWParty::Update (vote, 4) | CClientParty::RangeMonitor (ai-review-2026-10-01, medium) | ALIAS (checked): `size=3101 ret=4`; calls TestAndResetPath 0x7e7900, RecaulateFollowPoint 0x7e8720, PointOnLineDistanceFromLeader 0x7ead30 (all CSWParty/FollowData mapper rows). RangeMonitor is a sub-behaviour of Update. |
| 0x005fd480 | CSWPartyTable::SpawnNPC (vote, 20) | AwardXPToPartyMember? (level-up-P1b, medium, with '?') | MAPPER OK (checked): `size=911 ret=20`, args include 2 position/orientation vec3 ptrs (param_4/5), string "PercentXP" (XP catch-up on spawn). DB label names the XP tail only; keep both as note. |
| 0x005fe4b0 | CSWPartyTable::SwitchPlayerCharacter (vote, 4) | CSWSCreature::MakePlayerCharacter (mods678, high) | MAPPER preferred (checked): `size=2012 ret=4 callers=00694400`, string "GAMEINPROGRESS:PC", callees incl. CSWSCreature ctor 0x561f30, SetInParty 0x58a330, GetClientCreature 0x58ae90. GOG has no MakePlayerCharacter; ret 4 matches Switch. Class prefix in DB is suspect. |
| 0x0058a330 | CSWSCreature::SetInParty (vote, 8) | SetPartyMemberFlag (mods678, medium) | ALIAS (checked): `ret=8`, `*(int *)(param_1 + 0x11ac) = param_2;` (the +0x11AC party flag). |
| 0x0057c990 | CSWSCreature::GetNearestEnemy (vote, 12) | GetNearestVisibleEnemyInRange (mods678, high) | ALIAS (checked): `size=2524 ret=12`, walks perceived-object list (this->+0x194 area), keeps hostile via `FUN_005e1680(...) == 2` (GetAIStateReputation). GOG has only GetNearestEnemy (ret 12). |
| 0x0052d130 | CSWSArea::NoNonWalkPolysDetailed (vote, 48) | LosObjectBoxTest (mods678, medium) | MAPPER likely OK (checked partial): `size=896 ret=48 callers=...005cc870 (WalkUpdateLocationTestDistance)`; ret 48 matches GOG and neighbours 0x52d0e0/0x52d4b0 are NoNonWalkPolys/InDoors. Walkmesh LOS test = same family; DB label is a guess. |
| 0x006dab20 | CSWSCreature::CancelAllCombatModes (vote, 4) | EndAttackAction (mods678, medium) | UNVERIFIED: `size=93 ret=4 callees=005915f0,0057d370,005b7ae0`; ret matches GOG CancelAllCombatModes (0x656dd0, 4). |
| 0x00563490 vs | CSWSCreature::GetPortrait AND ::Destructor (collision, ret 4) | GetPortrait (mapper, medium) | COLLISION: `size=44 ret=4 callees=005634c0,009196fd (operator delete)` = Destructor wrapper; GetPortrait wrong. |
| 0x00589ee0 | SetInvitedToParty AND DisplayFloatyDataToSelf (collision) | SetInvitedToParty (mapper) | COLLISION, UNVERIFIED (`size=74 ret=4`) |
| 0x0058ad30 | GetCurrentlyFollowingLeader AND GetForceItemMask (collision) | GetCurrentlyFollowingLeader (mapper) | COLLISION: body tests creature type `*piVar2 == 0x3d`, no follow logic; both suspect |
| 0x0058ae90 | CSWSCreature::GetClientCreature (vote, 0) | Creature::GetServerObjectVt2c (mods678, medium) | ALIAS probable, UNVERIFIED |
| 0x005fed30 / 0x005fed60 | SetPlanetAvailable AND SetSoloMode; GetPlanetAvailable AND UnstealthParty (collisions in CSWPartyTable) | (new) | COLLISION (2 pairs); only one of each pair is right |
| 0x00669020 | CSWSCreature::WalkUpdateLocation_..._Moving_To_Path (outlier cluster) vs CSWVirtualMachineCommands::RunScriptCallback (vote, 4) | WalkUpdateLocation... (mapper, medium); U1: RunScriptCallback | VM name OK (neighbour 0x668FD0 ExecuteCommand is U1-confirmed); creature-walk cluster FALSE |
| 0x00983c00 | CSWSCreature::AIActionRest (outlier cluster) | CGuiInGame::HandleDialogEntry (mapper, medium) | MAPPER WRONG (checked): `size=30 ret=0 callees=00449230,0091d3ba` tiny stub, 0x98xxxx is not creature code |
| 0x00405970 | CSWSCreature::CheckProficiencies (outlier cluster) | CSWCObject::ClientServerSyncHandler_Position (mapper) | MAPPER WRONG (outlier delta -0x2c2e90; `size=63 ret=4`, tiny); both unconfirmed |
| 0x0054aa10 | CSWSObject::ClearAction (drift+0x50, 4) | CExoLinkedList::AddHead (R9 correction) | MAPPER WRONG (checked): `size=27 ret=4`, ~33 callers incl. container code; drift row |
| 0x006fe460 / 0x006fe8e0 / 0x006ff2a0 | CVirtualMachineStack::AddToTopOfStack / CopyFromStack / ClearStack_2 | VMStack::Push-slot / CopyFrom / Clear (U1, medium) | ALIAS (same addrs) |
| 0x007037c0 | CVirtualMachineInternal::DeleteScriptSituation | "DeleteScriptSituation ret 4 (map OK)" (U1) | ALIAS (annotation) |

Top 3 conflicts by importance (see above): (1) 0x5644B0/0x564770 AddMoveToPointAction pair (DB high is wrong; used by move/follow hooks), (2) CSWSArea GridDFSTransTable chain 0x54F3D0/0x552A90/0x552C90 (pathfinding; mapper wrong, R9 right), (3) the CSWSCreature outlier cluster + 0x669020 collision (19 false rows incl. AIActionPartyFollowLeader 0x971DC0).
Not conflicts but note: 3 CSWSCreature collision pairs and 2 CSWPartyTable pairs mean the mapper steps one slot at neighbouring same-retn functions; any row sitting next to a same-retn sibling needs a callee check.

## 3. Known false mappings (watch list)
Re-mapped by this run (all `vote`, all in the outlier cluster): 0x412C00 (WalkUpdateLocation_..._OffsetWaitDest, size=1496 ret=8), 0x418990 (AIActionWaitForEndOfRound), 0x971DC0 (AIActionPartyFollowLeader; no decomp file exists), 0x983C20 (AIActionTaunt, size=30 stub), 0x983D10 (AIActionDisappear), 0x984DE0 (AIActionSurrenderToEnemies), 0x9852D0 (AIActionStanceMeditative).
Not mapped by any AI0 run: 0x681880, 0x696BF0.
Full outlier cluster in `AI0_map_CSWSCreature_all.tsv` (19 rows, deltas +0x2a7810 / -0x2c2e90 / -0x6a580 / -0x57510; treat as false):
  00405100:CanEquipWeapon; 00405970:CheckProficiencies; 00412c00:WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest
  00418990:AIActionWaitForEndOfRound; 0041bfd0:AIActionAnimalEmpathy; 00669020:WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Path
  0066b640:WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest; 0066b770:WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest; 0066b8a0:WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest
  00670c90:CanEquipWeapon2; 0067ac20:WalkUpdateLocation_QuickWalk_FollowLeader_FindPath; 0067df50:WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath
  00971dc0:AIActionPartyFollowLeader; 00983c00:AIActionRest; 00983c20:AIActionTaunt
  00983c40:AIActionAppear; 00983d10:AIActionDisappear; 00984de0:AIActionSurrenderToEnemies
  009852d0:AIActionStanceMeditative

## 4. Spot-check evidence for 7 new vote rows (decomp/<bb>/<addr>.c first line + body)
| addr | mapper name | GOG retn | Steam evidence | fits? |
|---|---|---|---|---|
| 0x0051ed80 | CServerAIMaster::GetPendingEvent | 24 | `// FN 0051ed80 size=147 ret=24 callers=0051d790 callees=007f2a90,009196fd,007a1780`; copies 6 fields out via param_2..7 then frees node | YES (6 out-params = 24 B; only caller is UpdateState) |
| 0x0051ed10 | CServerAIMaster::EventPending | 8 | `size=105 ret=8 callers=0051d790 callees=0051afe0,007a1780,00521060`; compares head-event time vs (param_1,param_2) | YES |
| 0x0051f920 | CServerAIMaster::LoadReputationAdjustments | 0 | `size=2143 ret=0 callers=0055a650`; strings "PERSONALREP", "FACTIONREP", "WITFRIA", "WITFRIB" | YES |
| 0x006d3ff0 | CSWSFaction::GetAverageReputation | 4 | `size=172 ret=4 callees=0051c100,005e13c0`; loop sums `FUN_005e13c0(member,0)` (= CSWSObject::GetReputation, DB), divides by count, clamps 0..100 | YES |
| 0x00664580 | CFactionManager::GetNPCFactionReputation | 8 | `size=197 ret=8 callees=00664580,00664650` (self-recursive + GetIsNPCFaction 0x664650, mapped ret 4) | YES |
| 0x0058e120 | CSWSCombatAttackData::IsSubAttackReady | 4 | `size=120 ret=4 callers=00571030 callees=00623320,009196ee,0058de40`; callee 0x58de40 = GetTopSubAttack (mapped, ret 4, size=203) | YES |
| 0x005fe470 | CSWPartyTable::SetPartyFollowState | 4 | `size=61 ret=4 callees=007e8640,0073fb90`: `*(param_1+0x110)=0; FUN_007e8640()`; 0x7e8640 = CSWParty::FollowStateChanged (mapped) | YES |
Also checked: 0x51e0c0 CServerAIMaster::RemoveObject (`size=93 ret=4 callees=0051d170` = CServerAIList::RemoveObject), 0x51d150 CServerAIList::AddObject (`size=30 ret=4`), 0x7e7900 CSWParty::TestAndResetPath (`size=1133 ret=0`; calls FindClosestPartyMember 0x7eab10, RecaulateFollowPoint 0x7e8720, FlipLeaderFormationOffsets 0x7e6670, GetPathData 0x7e89e0: all AI0 rows). Perception: no NEW row (CSWSCreature::DoPerceptionUpdateOnCreature 0x576DC0 and SpawnInHeartbeatPerception 0x56B8A0 are already in DB and AGREE).
Overall: 0 contradictions in the 9 checked new vote rows; the mapper is reliable for new rows inside a class that is >90% one delta group (AIMaster, Faction, CombatAttackData, PartyFollowData, CSWParty). It is unreliable inside CSWSCreature outliers, CSWSArea Grid/PathPoint DFS neighbours, and next to same-retn siblings.

## 5. Persist candidates (pending lead decompile check) - 20 most AI-relevant new rows
All `vote`. [C]=decompile first line + callee consistency checked above.
1. 0x0051ed80 CServerAIMaster::GetPendingEvent /24 [C] (scheduler pump)
2. 0x0051ed10 CServerAIMaster::EventPending /8 [C]
3. 0x0051d1c0 CServerAIList::GetNextObject /0 [C] (round-robin object picker; replaces DB label CServerAIMaster::NextObject)
4. 0x0051d270 CServerAIList::GetObjectAtPosition /4 [C]
5. 0x0051d150 CServerAIList::AddObject /4 [C]
6. 0x0051d170 CServerAIList::RemoveObject /4 [C]
7. 0x0051e0c0 CServerAIMaster::RemoveObject /4 [C]
8. 0x0051efd0 CServerAIMaster::OnEffectApplied /12 (EFFOVF)
9. 0x0051f000 CServerAIMaster::OnEffectRemoved /8 (EFFOVF)
10. 0x0051f920 CServerAIMaster::LoadReputationAdjustments /0 [C] (faction)
11. 0x006d3ff0 CSWSFaction::GetAverageReputation /4 [C]
12. 0x00664580 CFactionManager::GetNPCFactionReputation /8 [C]
13. 0x006644d0 CFactionManager::SetNPCFactionReputation /12 (size=166 ret=12 checked header only)
14. 0x0058e120 CSWSCombatAttackData::IsSubAttackReady /4 [C] (combat round)
15. 0x0058de40 CSWSCombatAttackData::GetTopSubAttack /4 (size=203, callees consistent)
16. 0x0058da20 CSWSCombatAttackData::ResolveAttackToMeleeSubAttack /8 (size=481 ret=8)
17. 0x0058dc10 CSWSCombatAttackData::ResolveAttackToRangedSubAttack /20
18. 0x007e9cc0 CSWPartyFollowDataArray::SmoothFollowPath /0 (size=3650; follow path, PFLK)
19. 0x007e7900 CSWParty::TestAndResetPath /0 [C] (follow)
20. 0x005fe470 CSWPartyTable::SetPartyFollowState /4 [C] (party-flag/follow)
Reserve (same quality, not in top 20): 0x7e6720 CSWParty::ApplyLeaderFormation/4, 0x7eb460 CSWParty::IsPartyInCombat/0, 0x5fe410 CSWPartyTable::SetLeader/4, 0x6d48a0 CSWSFaction::GetLeader/0, 0x6d4ab0 CSWSFaction::TransferLeadership/8, 0x5fd480 SpawnNPC/20 [C], 0x5fe4b0 SwitchPlayerCharacter/4 [C].

## 6. Classes not in GOG DB / mapper failures
- Not in GOG function DB: `CPathfind` (only `CPathfindInformation`, 19 fns), `CSWSAI*`, `CFaction*` other than CFactionManager, CSWSFaction.
- CPathfindInformation: 0 rows (vote tie delta +0x33d180 12 vs 12; seeded retry took 3/19, STOP). Same as U3. Hand-map from CSWSArea pathfinding rows instead.
- 0 rows (too small / ties): CAurPathInfo(3), CClientAIMaster(6), CCombatInformationNode(3), CSWCCreaturePartyStatus(2), CSWPartyFollowPathData(1), CSWPartyMemberData(3), CSWSCombatSubAttack(1), CSWSCreaturePartyFollowInfo(2), CSWSForcedAction(2), CSWSObjectActionNode(3), CServerAIBodyBagInfo(2), CServerAIEventNode(2), CVirtualMachineScript(2).
- Partial (>= 25% unmapped): CSWSEffectListHandler 87/157 (70 unmapped; large GOG gap, deltas -0x1a28f0/-0x1a28d0/-0x1a28e0 differ by 0x10..0x20 steps, so spot-check before use), CSWSCreature 349/402 (53 unmapped), CSWSObject 114/144 (30 unmapped), CSWVirtualMachineCommands 21/29, CSWSArea 107/118, CVirtualMachineInternal 31/37 (the `_2.._5` split bodies unmapped), CVirtualMachine::SetScriptParams (ret 21 odd param size).
- Combined-regex failure mode: `CVirtualMachine` (unanchored, 4 classes) mapped different GOG fns onto the same Steam addr (e.g. 0x6FD980 StackPopInteger AND StackPushFloat); anchored per-class runs are clean (CVirtualMachine delta +0xBCDE0 22/22).

## 7. New vote rows by class (steam addr: Func/retn; steam addr w/o 0x)
Total 407 (full rows in the per-class TSVs). Classes with no new rows: CSWSCombatRound, CSWSCreature, CSWSObject, CSWSArea, CSWVirtualMachineCommands (all already in DB).
**CClientAIList** (2)
  007a2040:Destructor/0; 007a2270:GetNextObject/0
**CCombatInformation** (6)
  0069cab0:Constructor/0; 0069cbe0:Destructor/0; 0069cdb0:SaveData/8; 0069d1e0:LoadData/8; 0069d9e0:operator!=/4; 0069dc50:operator=/4
**CFactionManager** (15)
  00664280:Constructor/0; 00664360:Destructor/0; 00664490:GetFaction/4; 006644d0:SetNPCFactionReputation/12; 00664580:GetNPCFactionReputation/8; 00664650:GetIsNPCFaction/4
  00664680:LoadFactions/4; 00664800:LoadFactionsFromSaveGame/8; 00664a00:ResolveFactionName/8; 00664ae0:LoadReputations/0; 00664cb0:LoadReputationsFromSaveGame/8; 00664d80:SaveFactions/8
  00664e40:SaveReputations/8; 00664f30:CreateDefaultFactions/0; 006651f0:DeleteFaction/4
**CGameEffect** (20)
  005e4920:Constructor/4; 005e4ac0:Constructor_2/4; 005e4ca0:Destructor/0; 005e4dd0:SetNumIntegers/4; 005e4e60:GetInteger/4; 005e4e80:SetInteger/8
  005e4ea0:GetFloat/4; 005e4ec0:SetFloat/8; 005e4ee0:GetObjectID/4; 005e4f00:SetObjectID/8; 005e4f20:GetString/8; 005e4f60:SetString/8
  005e4f80:SetExpiryTime/8; 005e4fa0:GetExpiryTime/8; 005e4fd0:CopyEffect/8; 005e5260:SetLinked/8; 005e5290:UpdateLinked/0; 005e5b40:SaveGameEffect/8
  005e5ea0:SetCreator/4; 005e5f60:GetEffectTypeScriptConstant/0
**CSWParty** (20)
  007e5730:AddCharacter/4; 007e5c40:IsPartyMember/4; 007e5ca0:RemoveCharacter/4; 007e5d80:GetPlayerCharacter/0; 007e5da0:GetCharacter/4; 007e5df0:SetLeader/4
  007e6670:FlipLeaderFormationOffsets/0; 007e6720:ApplyLeaderFormation/4; 007e6960:GetWorldFormationPosition/16; 007e6be0:GetIndex/4; 007e6c40:RemoveCharacter_2/4; 007e6c80:ClearParty/0
  007e7900:TestAndResetPath/0; 007e7d70:GetFollowRange/0; 007e7ea0:Reset/12; 007e8350:ResetAllFollowData/12; 007e8640:FollowStateChanged/0; 007e8720:RecaulateFollowPoint/0
  007eb360:IsPartyHealthy/0; 007eb460:IsPartyInCombat/0
**CSWPartyFollowDataArray** (8)
  007e88a0:Constructor/0; 007e8970:ResetPath/8; 007e89e0:GetPathData/4; 007e9cc0:SmoothFollowPath/0; 007eab10:FindClosestPartyMemeber_Current/0; 007eac20:FindFurthestPartyMemeber_Current/0
  007ead30:PointOnLineDistanceFromLeader/8; 007eb050:GetPointBackFromStart/16
**CSWPartyTable** (74)
  005f7510:Constructor/0; 005f7590:Destructor/0; 005f76a0:ClearTable/0; 005f7a30:GetPartyItems/0; 005f7ae0:GetPartyJournal/0; 005f7b80:SaveJournal/8
  005f7cc0:LoadJournal/8; 005f7ec0:UpdateMembers/4; 005f7f30:SaveMember/8; 005f8130:SaveMemberByObject/8; 005f8320:SavePuppet/8; 005f8520:SavePuppetByObject/8
  005f8710:UpdateInventory/4; 005f8920:ClearInventory/0; 005f8960:TransferInventory/4; 005f8b00:RemoveMember/4; 005f8c60:RemovePuppet/4; 005f8d80:AddMember/8
  005f8f90:AddPuppet/8; 005f9100:GetFilename/8; 005f9190:GetPupFilename/8; 005f9220:CreateParty/0; 005f9a20:PositionParty/0; 005f9ff0:AddNPC/8
  005fa3b0:AddPUP/8; 005fa600:AddNPC_2/8; 005fa740:AddPUP_2/8; 005fa890:AssignPUP/8; 005fa920:RemoveNPC/4; 005fa960:GetIsNPCAvailable/4
  005fa990:GetIsPUPAvailable/4; 005fa9c0:GetInfluence/4; 005fa9f0:SetInfluence/8; 005faa30:GetNumNPCAvailable/0; 005faa80:SetNPCObject/8; 005faab0:SetPUPObject/8
  005faaf0:GetNPCObject/12; 005fad70:GetPUPObject/12; 005fafd0:GetIsMember/4; 005fb070:GetIsMember_2/4; 005fb0f0:GetIsAvailable/4; 005fb140:GetNPCSelectability/4
  005fb1a0:SaveTableInfo/4; 005fd440:Save/8; 005fd810:SpawnPUP/20; 005fd9a0:KillNPCObject/8; 005fdaf0:KillPUPObject/8; 005fdbf0:GetNPCID/4
  005fdc40:GetPUPID/4; 005fdca0:AddGameInProgress/0; 005fdd30:RemoveGameInProgress/0; 005fddc0:ClearMembers/0; 005fde00:ClearPuppets/0; 005fde40:ClearPuppet/4
  005fde70:ClearMember/4; 005fdea0:GivePlotXP/8; 005fdfb0:GivePlotXP_2/8; 005fe3b0:GetIsLeader/4; 005fe410:SetLeader/4; 005fe470:SetPartyFollowState/4
  005fec90:SetReturnStrref/12; 005fecd0:SetPlanetSelectable/8; 005fed00:GetPlanetSelectable/4; 005fed30:SetPlanetAvailable/8; 005fed30:SetSoloMode/8; 005fed60:GetPlanetAvailable/4
  005fed60:UnstealthParty/4; 005fed90:SetSelectedPlanet/4; 005fee60:GetTotalPlayedSeconds/0; 005ff000:ResetCurrentSessionStartTime/0; 005ff040:GetPCFirstName/4; 005ff150:GetPCLastName/4
  005ff2c0:GetPUPOwner/4; 005ff3b0:GetNextPC/0
**CSWSAreaMap** (14)
  005f6ab0:Constructor/0; 005f6b50:Destructor/0; 005f6b90:Initialize/48; 005f6dc0:LoadSavedAreaMapData/16; 005f6e20:SetWorldPointExplored/16; 005f6f50:SetMapPointExplored/8
  005f7010:GetMapData/12; 005f71e0:GetGridPixelFromMapPixel/16; 005f7280:SetPartyMemberWorldLocation/16; 005f7300:SetPartyMemberWorldOrientation/16; 005f7350:GetPartyMemberMapLocation/12; 005f73b0:GetPartyMemberMapOrientation/4
  005f73f0:IsWorldPointExplored/12; 005f74e0:SetEntireMapExplored/0
**CSWSAreaOfEffectObject** (17)
  0062bfc0:Constructor/4; 0062c180:GetEffectSpellId/0; 0062c1a0:SetEffectSpellId/4; 0062c1c0:Destructor/4; 0062c1f0:Destructor_2/0; 0062c800:AddToArea/20
  0062cca0:RemoveFromArea/0; 0062d410:SetShape/12; 0062d540:SetTargetObjID/4; 0062d640:GetPosition/4; 0062d6f0:SetDuration/8; 0062d750:LoadAreaEffect/4
  0062db90:LoadEffect/8; 0062dec0:SaveEffect/8; 0062eb70:JumpToPoint/8; 0062ec10:RemoveFromSubAreas/4; 0062ee30:SetCreator/4
**CSWSCombatAttackData** (16)
  0058cb70:Constructor/0; 0058ccd0:Destructor/0; 0058cd80:ClearAttackData/0; 0058d160:SetBaseDamage/4; 0058d1a0:GetBaseDamage/0; 0058d1c0:SetDamage/8
  0058d200:AddDamage/12; 0058d2f0:SaveData/8; 0058d5e0:LoadData/8; 0058da20:ResolveAttackToMeleeSubAttack/8; 0058dc10:ResolveAttackToRangedSubAttack/20; 0058de40:GetTopSubAttack/4
  0058df10:RemoveTopSubAttack/0; 0058df40:ResolveTopSubAttackToAttack/0; 0058dfd0:ResolveSubAttackToAttack/4; 0058e120:IsSubAttackReady/4
**CSWSCombatRoundAction** (3)
  0058c7d0:Constructor/0; 0058c960:SaveData/8; 0058ca50:LoadData/8
**CSWSEffectListHandler** (79)
  00593960:Destructor_2/0; 00593a10:OnRemoveForceSight/0; 00593a30:InitializeEffects/0; 00594740:OnEffectApplied/12; 005947a0:OnEffectRemoved/8; 005950b0:OnRemoveFury/8
  005951a0:OnApplyFactionModifier/12; 005953e0:OnRemoveFactionModifier/8; 005959e0:OnApplyDamage/12; 00597750:OnApplyHeal/12; 00597d90:OnApplyDeath/12; 00599260:OnApplyAbilityIncrease/12
  00599330:OnRemoveAbilityIncrease/8; 00599380:OnApplyAbilityDecrease/12; 005994b0:OnRemoveAbilityDecrease/8; 00599500:OnApplySummonCreature/12; 0059a200:OnRemoveACIncrease/8; 0059a730:OnRemoveACDecrease/8
  0059aa40:OnApplyAttackDecrease/8; 0059acc0:OnRemoveTemporaryHitpoints/8; 0059b1a0:OnRemoveDamageImmunityIncrease/8; 0059b3e0:OnRemoveDamageImmunityDecrease/8; 0059b520:OnApplyEntangled/12; 0059b8d0:OnApplyKnockdown/12
  0059be80:OnRemoveKnockdown/8; 0059c420:OnRemoveSetAIState/8; 0059c550:OnApplyDeaf/12; 0059c890:OnRemoveEnemyAttackBonus/8; 0059ca50:OnRemoveArcaneSpellFailure/8; 0059cba0:OnApplySetState/12
  0059db50:OnRemoveSetState/8; 0059de30:OnApplySetStateInternal/12; 0059e860:OnRemoveSetStateInternal/8; 0059ea10:OnRemoveMovementSpeedIncrease/8; 0059f120:OnRemoveVisualEffect/8; 0059f4f0:OnRemoveAreaOfEffect/8
  0059f5a0:OnApplyBeam/12; 0059f6f0:OnApplyForceResistanceIncrease/8; 0059f7d0:OnRemoveForceResistanceIncrease/8; 0059fae0:OnRemoveForceResistanceDecrease/8; 0059fd20:OnApplyPoison/12; 005a0640:OnRemovePoison/8
  005a06f0:OnApplyDisease/12; 005a0c50:OnApplyHasteOrSlow/12; 005a1210:OnRemoveHasteOrSlow/8; 005a1680:OnApplyLink/12; 005a1700:OnApplyHasteInternal/12; 005a18d0:OnApplySlowInternal/12
  005a1cc0:OnRemoveInvisibility/8; 005a2070:OnRemoveSanctuary/8; 005a2450:OnApplyLight/12; 005a26a0:OnRemoveModifyNumAttacks/8; 005a2710:OnApplyCurse/12; 005a2c70:OnApplySilence/12
  005a3800:OnApplyTaunt/12; 005a3a80:OnRemoveLimitMovementSpeed/8; 005a3cd0:OnApplyHitPointChangeWhenDying/12; 005a3e80:OnRemoveHitPointChangeWhenDying/8; 005a42e0:OnRemoveSpecialWalkAnimation/8; 005a48b0:OnRemoveEffectIcon/8
  005a4a40:OnApplyDisguise/12; 005a4bb0:OnRemoveDisguise/8; 005a5020:OnRemoveSeeInvisible/8; 005a5130:OnApplyUltraVision/12; 005a53c0:OnRemoveUltraVision/8; 005a57e0:OnRemoveTrueSeeing/8
  005a5990:OnApplyBlindness/12; 005a5d60:OnRemoveBlindness/8; 005a5ee0:OnApplyDarkness/12; 005a6280:OnRemoveDisappearAppear/8; 005a65c0:OnApplyNegativeLevel/12; 005a6c60:OnRemoveNegativeLevel/8
  005a6d60:OnRemoveBonusFeat/8; 005a6dd0:OnApplyWounding/12; 005a6ef0:OnRemoveWounding/8; 005a7490:OnRemoveTemporaryForce/8; 005a8530:OnApplyDamageForcePoints/8; 005a9520:OnApplyForceResisted/12
  005a9ff0:IsEffectToIgnoreOnDeath/4
**CSWSFaction** (28)
  006d30f0:Constructor/4; 006d3200:Constructor_2/0; 006d32d0:Destructor/0; 006d3330:AddMember/8; 006d3430:AttemptRemove/8; 006d3470:GetMemberList/4
  006d3490:InviteMember/8; 006d3530:RemoveMember/4; 006d3680:SendChatMessage/8; 006d3720:SendFactionUpdateList/0; 006d3920:GetWeakestMember/8; 006d3a80:GetStrongestMember/8
  006d3be0:GetMostDamagedMember/8; 006d3db0:GetLeastDamagedMember/8; 006d3f80:GetGold/0; 006d3ff0:GetAverageReputation/4; 006d40a0:GetAverageGoodEvilAlignment/0; 006d4220:GetAverageLevel/0
  006d42c0:GetAverageXP/0; 006d4360:GetMostFrequentClass/0; 006d4560:GetWorstAC/8; 006d46d0:GetBestAC/8; 006d4840:GetCreatureInParty/4; 006d48a0:GetLeader/0
  006d4a10:GetSingletonParty/0; 006d4ab0:TransferLeadership/8; 006d4af0:GetFactionMember/8; 006d4bf0:Initialise/0
**CSWTrackFollower** (44)
  00833de0:Destructor/0; 00833ff0:Load/8; 008341f0:LoadScripts/12; 008344e0:RemoveGunBank/4; 00834510:Update/4; 00834630:GetGunBank/4
  00834670:GetModel/4; 00834740:RemoveModel/4; 008347b0:SetTrack/8; 008347e0:ForceSetTrack/8; 00834a00:SetSpeed/4; 00834aa0:Go/0
  00834c70:Stop/0; 00834d10:OnDeath/4; 00834fd0:Die/0; 00835010:Hit/0; 00835090:AttachGuns/0; 008350e0:AttachGuns_2/4
  008352b0:SetSphereRadius/4; 008352f0:DoBulletHitCheck/4; 008353a0:OnHitObstacle/4; 008354c0:DoObstacleHitCheck/4; 00835620:SetMaxHitPoints/4; 00835650:SetInvincibility/4
  00835690:SetHitPoints/4; 008356c0:OnDamage/4; 00835820:AdjustHitPoints/4; 00835850:GetScriptName/8; 008358b0:SetScriptName/8; 008358f0:GetSound/4
  00835920:GetSoundName/8; 00835980:SetSoundName/8; 00835c70:GetSoundFrequency/4; 00835ca0:SetSoundFrequency/8; 00835cd0:GetSoundFrequencyIsRandom/4; 00835d00:SetSoundFrequencyIsRandom/8
  00835d30:GetSoundVolume/4; 00835d60:SetSoundVolume/8; 00835db0:PlayAnimation/12; 00835e90:GetName/4; 00835f00:GetLastObstacleHit/0; 00835f50:GetPosition/4
  00838e10:OnAccelerate/4; 00838ec0:OnBrake/4
**CServerAIList** (3)
  0051d100:Destructor/0; 0051d150:AddObject/4; 0051d170:RemoveObject/4
**CServerAIMaster** (10)
  0051e0c0:RemoveObject/4; 0051ed10:EventPending/8; 0051ed80:GetPendingEvent/24; 0051ee20:SaveEventQueue/8; 0051eed0:LoadEventQueue/8; 0051efd0:OnEffectApplied/12
  0051f000:OnEffectRemoved/8; 0051f030:OnItemPropertyApplied/20; 0051f070:OnItemPropertyRemoved/16; 0051f920:LoadReputationAdjustments/0
**CVirtualMachine** (20)
  006fd700:Constructor/0; 006fd7b0:Destructor/0; 006fd960:GetRunScriptReturnValue/8; 006fd980:SetCommandImplementer/4; 006fd9a0:StackPopInteger/4; 006fd9c0:StackPushInteger/4
  006fd9e0:StackPopFloat/4; 006fda00:StackPushFloat/4; 006fda20:StackPopVector/4; 006fda40:StackPushVector/12; 006fda70:StackPopString/4; 006fda90:StackPushString/4
  006fdab0:StackPopEngineStructure/8; 006fdad0:StackPushEngineStructure/8; 006fdaf0:StackPopObject/4; 006fdb10:StackPushObject/4; 006fdb30:StackPopCommand/4; 006fdb50:SaveScriptSituation/12
  006fdb80:LoadScriptSituation/12; 006fdbb0:DeleteScriptSituation/4
**CVirtualMachineInternal** (21)
  006ff570:Constructor/0; 006ff7b0:Destructor/4; 006ff7e0:Destructor_2/0; 00701f20:PopInstructionPtr/4; 00701f80:PushInstructionPtr/4; 007029b0:GetRunScriptReturnValue/8
  00702b60:StackPopInteger/4; 00702be0:StackPushInteger/4; 00702c30:StackPopFloat/4; 00702cb0:StackPushFloat/4; 00702d00:StackPopVector/4; 00702d70:StackPushVector/12
  00702dd0:StackPopString/4; 00702e60:StackPushString/4; 00702f20:StackPopEngineStructure/8; 00702fe0:StackPushEngineStructure/8; 00703070:StackPopObject/4; 007030f0:StackPushObject/4
  00703370:SaveScriptSituation/12; 00703530:LoadScriptSituation/12; 00703810:SetCommandImplementer/4
**CVirtualMachineStack** (7)
  006fe440:Destructor/0; 006fe630:AssignLocationToLocation/8; 006fe890:AddToIntegerLocation/8; 006fed80:SaveStack/8; 006fef80:LoadStack/8; 006ff440:GetStackPointer/0
  006ff450:SetStackPointer/4

## 8. Files
ROOT/findings/AI0_map_*.tsv (per class, plus _ai filtered: CSWSCreature_ai 135 rows, CSWSObject_ai 52, CSWSArea_ai 24) and this file.
