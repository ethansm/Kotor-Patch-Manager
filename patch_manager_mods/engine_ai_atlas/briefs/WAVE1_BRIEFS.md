# Wave 1 briefs (AI1-AI4, mod-critical subsystems), 2026-10-02

## Common (every batch)
- Read, in order: `ROOT/CONTRACT.md`, `REPO/patch_manager_mods/engine_ai_atlas/CONTRACT_AI.md`, then
  `ROOT/findings/AI0_mapper_summary.md` and the `ROOT/findings/AI0_map_*.tsv` rows for your classes.
- Then the "do not re-derive" sources listed in CONTRACT_AI.md that touch your subsystem (the review "Map" table already
  names ~55 pipeline addresses; R1 already verified FM2, FM3, FM5, FM7, T1, R1, P2).
- Output `ROOT/findings/AI<n>.md`, sections (a)-(g) of CONTRACT_AI.md, <= ~600 lines, 5-line summary on top.
- Answer every question below with CONFIRMED / REFUTED / SPECULATIVE plus the quoted asm/decompile line and address.
- Use `tools/cg.py` and `tools/field_census.py` first (`python3 REPO/patch_manager_mods/engine_ai_atlas/tools/cg.py --help`).
- If a function you need is missing from `decomp/`, list it in (d); do not run Ghidra.
- Tool facts (Phase 0 gate): `cg.py` BFS uses direct CALL/TAILJMP only. The scheduler reaches the creature through
  vtable+0x70 (`0x51DC7B CALL EAX` after `0x51DC78 MOV EAX,[EDX + 0x70]`), so bridge it with
  `--bridge 0x51D790:0x569320`; `class_methods` has no CSWSCreature/CSWSObject rows, so `--via-vtable` finds nothing.
  `cg.py indirect --slot 0xNN --in ADDR` recovers slots from `CALL EAX` sites. `field_census.py --all` scans every
  function in ~0.3 s; provenance is a linear-scan hint. Negative offsets: `--offset=-0x8`.
- Known name_map errors (fix pending, do not propagate): `0x51D790` is CServerAIMaster::UpdateState (DB says
  ServerEvents::Update); `0x5644B0` is AddMoveToPointActionToFront (calls AddActionToFront at `0x5645E4`), `0x564770`
  is AddMoveToPointAction (calls AddAction); the 19-row CSWSCreature outlier cluster in `AI0_mapper_summary.md` is false.
- Never write anything except your output file (and optional mapper TSVs under `ROOT/findings/`).

## AI1 Pathfinding and movement
Serves FM2 (and the FM2b reachability question), E4, Mod 8 kite moves, enemy E7.
Seeds: `PlotPath 0x55AB30`, `PlotPathInArea 0x55AC80`, `AIActionMoveToPoint 0x5BFC00` (the whole state machine, incl.
`0x5C21C4`, `0x5C2438`..`0x5C25A5`), `AIActionCheckMoveToObjectRadius 0x5BC1D0` (incl. `0x5BC5F4`, `0x5BC652`,
`0x5BC6D1`, `0x5BC6EA`), `AddMoveToPointAction 0x5644B0`, `AddPathfindingWaitActionToFront 0x5646C0` (word group, float
secs), `WalkUpdateLocation 0x5C3460` (OnBlocked post), door event 0x1E (`0x5C0732`), `QuickWalk 0x5C3F70`, the CPathfind /
CPathfindInformation rows from AI0, PFLK (`0x6F99F0` pathinfo Shutdown), bump `0x586940` / `0x586B70`.
Questions:
1. For an unreachable target, does the pathfinder return FAIL or a PARTIAL path? Give the exact return codes of
   0x55AB30 / 0x55AC80 and every reader of them (who maps them to the action's 1/2/3). This settles FM2b.
2. Map the `+0x380` pathinfo struct (CPathfindInformation): field table (section e) with readers/writers.
3. Map the 1 ms slice (U3: `[0x9944C4]`=1000 -> `[0xA7E0D0]`): what happens when a search runs out of slice, how it
   resumes, and whether a long search can starve the AI budget.
4. When exactly is OnBlocked (event 0x1F, ON_PATH_BLOCKED) posted? All 3 FM5 post sites, their conditions, and the
   `+0x558` last-blocked-position logic.
5. Is there ANY stuck / no-progress detector (position-delta, time-in-action, retry counter) anywhere in movement?
6. Moves vs bumpable: how `0x586940` / `0x586B70` interact with an in-progress move (FM1 context).
7. AIActionMoveToPoint full state machine: states, the locals that hold them, every exit with its return code.

## AI2 Combat round and attack pipeline
Serves A1, `a1_retarget_strict`, M8, enemy AI, Mod 8.
Seeds: `AIActionCombat 0x6D9500`, `AIActionPhysicalAttacks 0x6D7390` (incl. `0x6D740F`, `0x6D7E4A`-`0x6D7E5F`,
`0x6D83EA`..`0x6D8A7E`), `CreateNewAttackActions 0x6DA4F0`, `InitializeNewAttackTarget 0x6DAC70`, `EndAttackAction
0x6DAB20`, `ResolveAttack 0x6DACF0` (internals NOT yet covered), `AddAttackActions 0x568540`, `StartCombatRound
0x58E730`, `EndCombatRound 0x58E990`, `ResolveEngagedState 0x590FF0`, `ResolveMasterState 0x5912B0`, `SetCombatState
0x57A560`, `MaxAttackRange 0x571570`, `DesiredAttackRange 0x5716A0`, `GetNearestVisibleEnemyInRange 0x57C990`,
`ClearHostileActionsVersus 0x5867E0`, the `0x57AC30` refresh, `CSWSCombatRound` rows from AI0.
Questions:
1. The full `CSWSCombatRound` field table (`+0xA84`..`+0xB20` and beyond; the round object lives at creature `+0x10DC`,
   so give offsets both as round-relative if the round is a separate object, and say which it is).
2. Attack-range semantics everywhere: 2D vs 3D, which radii are added/subtracted, at every range test (attack,
   retarget, desired range, max range, GetNearestVisibleEnemyInRange). One table.
3. Every reader of the stance `stats+0x146` (review has a table: confirm/extend it, especially in the combat path).
4. The round timer and pause logic (`+0xA94`, `+0xA9C`, `+0xAA8`, `+0xAB0`, `+0xAB4`): who sets, who counts down, what
   unpauses.
5. Who picks the next target: engine (CreateNewAttackActions, controlled only?) vs script (slot 6 EndRound). List
   every engine-side target choice.
6. Exactly when the slot-6 (EndRound) script fires: all call sites of RunScript with slot 6 (`+0x2A0`), conditions.
7. ResolveAttack internals: hit roll, damage application, events posted (OnAttacked/OnDamaged ids), and what it
   writes back to the round.

## AI3 Perception, LOS and stealth
Serves P1, P2, E3, enemy aggro.
Seeds: `SpawnInHeartbeatPerception 0x56B8A0` (incl. `0x56BDBD`, `0x56BE79`), `UpdatePerception 0x5D0F30` (modes 0 all /
1 party / 2 non-party), `DoPerceptionUpdateOnCreature 0x576DC0`, visible list `0x5D1750 / 0x5D1400 / 0x5D15F0 /
0x5D1510` (`+0x1028/+0x102C`), LOS `0x52BB60` (internals NOT yet covered), the GetIsEnemy / reputation calls inside
perception, the stealth / invisibility checks.
Questions:
1. The perception list struct (seen / heard / vanished flags) and the event posts (perceived / vanished / heard: which
   script slot, which event ids, which conditions).
2. The real cadence per creature type. Runtime baseline (02:38 launch, 5 creatures): 176 calls/s total, max 44/s per
   creature, `elapsed>=4000` true 100% with p1=0. Explain these numbers from the code (who calls 0x56B8A0, how often).
3. Why P1's timestamp is never stored (lesson 230): confirm and give the exact missing store.
4. The stealth detection formula (spot/listen vs stealth/move silently, distance, LOS), with the constants.
5. The R1 scan-abort reach: when does UpdatePerception abort on an unresolvable id, and what does that skip.
6. LOS 0x52BB60: inputs (eye vs foot height, exclusions), what geometry it tests (walkmesh? objects? doors? creatures?).
   Settle E3: do allies (creatures) block LOS?

## AI4 Scheduler and action queue
Serves FM3, FM7, AIQ, E4, perf.
Seeds: all `CServerAIMaster` methods (AI0 rows plus `0x51D790 UpdateState`, `0x51E120 SetAILevel`, `0x51E040
AddObject`, `0x51D1C0`, `0x51D270`), `CSWSArea::AIUpdate 0x521E60`, `CSWSCreature::AIUpdate 0x569320` (full branch map
incl. `0x5694B2` and `0x5699CE`..`0x569A72`), `RunActions 0x5D56B0`, `ActionTypeStopsLoop 0x5D7160`, `AddAction
0x53F7F0`, `AddActionToFront 0x540CA0`, `ClearAllActions 0x541080`.
Questions:
1. The COMPLETE action-type table: every type id -> its `AIAction*` handler address, from the RunActions dispatcher
   (switch / jump table: decode the table bytes from the .asm if it is an indirect JMP; the dump's `globals.tsv` or
   raw table addresses may help; give the table address). Also the 0x74-byte action node layout (field table).
2. The AI-level policy: who sets levels 0-4 and when (every SetAILevel caller with its condition).
3. The FM3 sentinel path: the cold sites `0x51DBE1` / `0x51DD33` / `0x51E0C9`; how often can it trigger (reach).
4. AIQ cost model: the per-object rescan (0x51D842..0x51DD1C, CALL 0x51D270): cost per tick as a function of n.
5. The order of the per-object budget across levels, and what an object that exceeds budget does to the others.
6. CSWSCreature::AIUpdate full branch map: every branch, in order, with the condition and what it calls.
