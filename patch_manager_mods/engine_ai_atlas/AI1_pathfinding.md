# AI1 Pathfinding and movement (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: server tick -> `CServerAIMaster::UpdateState 0x51D790` -> `CSWSCreature::AIUpdate 0x569320` -> perception -> scripts -> action queue (`RunActions 0x5D56B0`) -> AIAction handlers -> **movement** -> events back to scripts. This subsystem is the movement half of the handlers: `AIActionMoveToPoint 0x5BFC00` (action type 1) owns the state machine, asks the pathfinder (`0x55AB30`) for a path, walks it (`0x5C3460` server walker, `0x5C3F70` QuickWalk for player-visible creatures), runs the per-step blocker/bump logic, and posts OnBlocked (payload 0x1F) / door-trigger events (0x1E) back to the object through the AI event queue. Action `0x11` (`0x5BC1D0`) is the "re-approach until in use range" companion of the move. The pathfinder has exactly one reader (`0x5BFC00`), uses a soft 1000 us slice shared with RunActions, and returns 1 pending / 2 success / 3 fail (its own convention, U3).

## Status
- Sources: batch `findings/AI1.md` (Sonnet) + `GATE_wave1.md` (Opus gate, 2026-10-02, section 1 AI1 verdicts and 1.1 FM2b answer, section 3 confidence rules, section 4 names, section 5 windows) + `LEAD_SPOTCHECKS.md` (AI1 lines exist as quoted; partial-path gate, level-0 FAIL, bump abort confirmed).
- Date: 2026-10-02. Static analysis only (decomp `.asm`/`.c`, vanilla exe constants), no runtime evidence.
- Verification: gate-verified for the verdict table (FM2b answer, 0x11 LOS refuted, OnBlocked 6 sites, trigger-vs-door, `0x550980` codes, `+0x558`, bump counter). Runtime magnitudes and rates stay low. Full PI field table (0x278 B) is in the DB table `struct_fields`; this page lists the important rows.
- Conflicts resolved to the gate version are marked "batch said X; gate corrected".

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x55AB30 | CSWModule::PlotPath (this = GetModule 0x51BD90; ret 8) | med | pathfind request entry, only caller `0x5BFC00` (`005c1f18`). Batch/DB said `CSWSArea::PlotPath`; gate renamed (name_map) |
| 0x55A900 | PlotPathInterArea | high | route driver, per-segment retry, exclusion list `PI+0xB4/0xB8` |
| 0x55AC80 | PlotPathInArea | high | straight test, partial paths, search dispatch, 100-call cap |
| 0x550E30 | CSWSArea::PlotGridPath (returns closest path as SUCCESS) | high | restartable iterative-deepening grid DFS; identity high, closest-result rule added by gate (l.238-250) |
| 0x5532F0 | PlotPathPointPath | high | path-point search, state `PI+0x1E0` 1..5; at least one full iteration per call |
| 0x550980 | CSWSArea::WalkLineTest | med | 1 clear, 0 wall/100 steps, -1 face-walk or collision-mesh fail, -2 door/placeable box, -3 creature (codes corrected by gate) |
| 0x557150 | PlanInterAreaRoute | med | area routes depth 1..9 into `PI+0xAC` |
| 0x557EF0 | GetModuleArea | low | area compared to `PI+0x80` at `0055acf8` |
| 0x51BD90 | GetModule (fastcall wrapper of 0x52F4C0) | med | `this` of PlotPath |
| 0x6F9200 / 0x6F9440 / 0x6F9420 | CPathfindInformation ctor / Initialize / dtor | med | 0x278 B block at creature `+0x380` (`005621bf PUSH 0x278`) |
| 0x6F99F0 | CPathfindInformation::Shutdown | med | frees `0x90..0x238` fields, leaks `+0x18C/+0x1B4/+0x1D8/+0x200` (PFLK) |
| 0x6F9C90 | CPathfindInformation::ResetRequest | high | zeroes goal `+0x5C..+0x80`, `+0x58`; called at walker end `005c3efe` |
| 0x6F9D40 | CPathfindInformation::ResetAtEndOfPath | med | `+0x84=0, +0x88=1`, frees `+0xB8` |
| 0x5BFC00 | AIActionMoveToPoint (action 1; ret 4) | high | the movement state machine, 0x74 B node params |
| 0x5BC1D0 | AIActionCheckMoveToObjectRadius (action 0x11; ret 4) | high | same area && GetIsInUseRange else re-queue [wait, move, 0x11]. Batch said "in range + LOS"; gate corrected: NO LOS (dead block) |
| 0x5644B0 | CSWSCreature::AddMoveToPointActionToFront (ret 68) | high | `005645e4 CALL 0x00540ca0`; packs node flags, param_9 -> PI+0 |
| 0x564770 | CSWSCreature::AddMoveToPointAction | high | append variant (`005648a4 CALL 0x0053f7f0`) |
| 0x5646C0 | AddPathfindingWaitActionToFront | high | 0 -> 0.3 s (`[0x986B60]`), PC-controlled 0.1 s |
| 0x5C3460 | WalkUpdateLocation (server walker) | high | steps along PI path; OnBlocked site `0x5C3D7D` |
| 0x5C3F70 | WalkUpdateLocation_QuickWalk (ret 4) | high | walker for objects with `+0x220 != 0` (player-visible); run by UpdateState pre-pass `0051d9bc` |
| 0x5CB430 | WalkUpdateLocationDistance (ret 16) | high | advances `PI+0x90`; blocked-step counter `PI+0x268` |
| 0x5CC870 | WalkUpdateLocationTestDistance (ret 16) | high | per-step test: box test + NoCreaturesOnLine + BumpFriends; OnBlocked site `0x5CCBB4` |
| 0x6FC5D0 | CSWSCreature::AvoidCreature | med | go-around planner; runs `k_def_pathfail01` (string `0x99B5E0`) when blocker reputation < 0xB |
| 0x586940 | GetIsCreatureBumpable (ret 4) | high | bumper `this`, bumpee arg; callers `0x586B70`, `0x54D750` |
| 0x586B70 | BumpFriends (ret 12) | high | displace bumpee; clears a follower's queue via `0x541080` (`00586f48`) |
| 0x581070 | ForceMoveToPoint | med | forced teleport after deadline (cutscene moves) |
| 0x5BF760 | TerminateClientSidePath | med | on fail when node bit1 (`005c176c`) |
| 0x5C6340 / 0x5C87E0 / 0x5C8C20 | FollowLeader driver / planner st2 / st3 | med | OnBlocked sites `0x5C8BB4`, `0x5C9F8F` |
| 0x51E170 | CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data) | high | node stores `[2..5] = p3..p6`; renamed by gate (was ServerEvents::AddEventDeltaTime) |
| 0x56EFD0 | CSWSCreature::EventHandler (ret 20) | high | event 10 payload `*(word*)data - 1` switch; 0x1F case `0x5706A4` stores blocker `+0x37C` |
| 0x57FCE0 | UpdatePersonalSpace | high | fills `PI+4/8/0x10/0x14/0x18` from appearance 2DA |
| 0x5D56B0 | RunActions (ret 16) | high | 1000 us budget from AIUpdate start (`005d6d0d`) |
| 0x6D7390 | AIActionPhysicalAttacks (AI2) | med | owns the `+0x558` LOF block position |

## FM2 / FM2b loop (gate 1.1; one loop, two phases)
Approach move = `0x11` (`005bc6d1 CALL 0x005644b0`, param_9 `005bc6a4 PUSH 0x0`, so PI+0 = 0 and the partial fallback is open for every engine/script approach move).
1. Request: `0x5BFC00` sets `PI+0x58 = 1` (`005c1d4a`), calls PlotPath (`005c1f18`). Call #1 runs the straight test start->goal (`0055af84`), caches the code in `PI+0x19C`; code 1 -> 2-point path, result 2.
2. Search (grid `0055b205/0055b318` or path-point `0055b2c4`), pending (1) across ticks, 1 ms slice per call. Result 2 via the grid closest-result rule (`0x550E30` l.238-250, >= 2 best points, `area+0x1D0` clear) or 3 from depth/time/no-progress, the 100-call cap (`0055b3dd`), or `-4` (`0055b322`).
3. On 3 the straight fallback runs (needs `PI+0 == 0`, `0055b3e9..0055b407`) unless the cached first code is -1 (`0055b4fb CMP [EBP-0x20],-0x1` / `JZ 0x0055b61c`). Six bisection probes `0x550980(start, cand) == 1`; any clear probe -> `0055b628 MOV [ECX+0x8c],4` -> **2** (`0055b692`); none -> 3 (`0055b623 JZ 0x0055b69a`).
4. Leg 1 (result 2): creature walks straight at the obstacle (or the closest grid path). Walker end calls `0x6F9C90` and returns 2 (`005c3eec..005c3f1b`). RunActions stops the loop (type 1 stops it, `0x5D7160`).
5. Next tick: `0x11` still not in range, re-queues at the front in order wait 0.3 s (`005bc6ea`), move (`005bc6d1`), `0x11` (`005bc652`); returns 2.
6. Leg 2: stored goal was zeroed, so the request is new (`005c1d4a`). Standing at the obstacle, the probes fail or creep ~1/128 of the segment -> 3: R3 branch `005c21e2` log, `005c2484 CALL 0x005646c0`, 2.00-3.98 s wait, return 3 (`005c25a5`). RunActions treats 3 like 2.
7. Steady state = R1 FM2: wait 2-4 s, `0x11`, wait 0.3 s, move, FAIL. Each move costs >= ~10 ticks of 1 ms search. In combat the E2 fallback (slot 6 every 3 s; moves and `0x11` do not reset `+0x570`) restarts with a fresh `[move, 0x13, 0x11, 0xC]`, each restart begins with a fresh partial leg 1. (Leg count SPECULATIVE; code CONFIRMED.)
- Design consequence: the fail-fast window `0x5C2438` sees only the FAIL legs. Key fail-fast on the `0x11` re-approach count per (creature, target) with a no-progress test at `0x5BC5F4`; use `0x5C2438` only to skip the 2-4 s wait. Probe `0x55B69A` splits partial vs fail per request.
- Batch said "FAIL residue = client-path, probe failure, -1, no route, area mismatch"; gate: FAIL(3) reaches `0x5BFC00` only for client-path nodes, cached code -1 (face-walk or collision-mesh sweep failure), all 6 probes blocked, no inter-area route, request area != module area, or null PI.

## Key behaviours
1. Pathfinder codes `[0x9944B4/B8/BC]` = 1 pending / 2 success / 3 fail (read from vanilla exe). Only reader of `0x55AB30` is `0x5BFC00` (`005c1f45` 2 -> walk, `005c21c4` 3 -> fail branch, else pending).
2. PlotPath: `PI` null -> 3 (`0055ab3f`); `+0x58 == 0 && +0x88 == 1` -> 2 without re-planning (`0055ab6a`); else reset and `0x55A900`.
3. AI level 0 (`creature+0x78 == 0`) -> result 3 at once, then only the straight partial remains (`0055b1b5..0055b1c7`, CONFIRMED by gate and lead; `+0x78` is the AI level).
4. All searching requires `*(0x557EF0(this)+0x124) == PI+0x80`; area mismatch returns 3 with no partial fallback (`0055ace9`, `0055acf8`). Reach SPECULATIVE.
5. Slice: `[0x9944C4]` = 1000 us copied to `[0xA7E0D0]` (`00983dc3/dc8`), passed by `0x5C1EF8`; grid search checks the timer after each deepening iteration (single iteration up to 40 ms), path-point search runs at least one iteration with no timer inside `0x554EB0` (4096-level guard only). RunActions uses the same 1000 us (`005d6d0d`). 100-call cap (`0055b3bb..0055b3dd`) ~100 ms of search per request.
6. Grid hard rules: iteration `== 1` and `elapsed >= 40*limit` -> "Exceeded limits:Time limit"; `>= 10*limit` and best distance unchanged -> "Distance to goal"; depth >= `dist+36` -> "Max search deapth". The closest-so-far path can instead be returned as SUCCESS (gate). batch said "grid failure always means FAIL"; gate corrected.
7. `PI+0x24 = 1` is written on every grid call (l.171), so the closest-result rule is always enabled (batch named it "grid search started"; gate renamed, med).
8. `0x550980` codes: 1 clear; 0 wall (perimeter edge without room transition `0x5FF800 == -1`), 100 steps; -1 = face-walk (`0x651090`) or collision-mesh sweep (`0x5EA8F0`) failure; -2 door/placeable box (`0x52D130`); -3 creature on the line. A cached -1 disables the partial fallback. batch said "-1 = off mesh"; gate corrected.
9. Walker end: `0x6F9C90` then return 2 (`005c3eec CMP [EBP-0x58],1` / `005c3efe` / `005c3f1b`), so each move is a new search.
10. OnBlocked (payload 0x1F, event id `[0x99241C]` = 10, delta 0, caller = blocker/door, target = creature) is posted from **6** movement sites, none throttled, none time gated: `0x5C1240` (5BFC00 door case type 0xA), `0x5C3D7D` (5C3460), `0x5C5A68` (5C3F70), `0x5CCBB4` (5CC870), `0x5C8BB4` (5C87E0), `0x5C9F8F` (5C8C20). Other `MOV reg,0x1F` hits (`0x864170`, `0x5D734F`) are value-mapping switches. R1 listed 3 (gate: AI1 right).
11. `5BFC00` switch `005c0409 JMP [ECX*4+0x5c25bc]`, index type-5, table `0x5C07A7, 0x5C172F, 0x5C0410, 0x5C172F, 0x5C1555, 0x5C1143` (types 5..10). `0x5C0732` is inside the type-7 (TRIGGER) block `[0x5C0410,0x5C07A7)` and posts 0x1E; the door (type 0xA, block `[0x5C1143,0x5C1555)`) posts 0x1F at `0x5C1240` when `door+0x31C == 0`, else 0x1E at `0x5C12F5/0x5C3E4B/0x5C5B4E`. R1 called `0x5C0732` the door; gate: it is the trigger. Door `+0x31C` state names SPECULATIVE.
12. Event consumer: `0x56EFD0` switch (byte table `0x571010`, `JMP [EDX*4+0x570ff4]`), 0x1F case `0x5706A4`: PC-controlled creature skipped (`005706cf CALL 0x0051c600`), `005706ec MOV [ECX+0x37C],EDX` (blocker := caller), runs slot 12. The only throttle is that PC check.
13. No generic "not making progress" detector. Real ones: `PI+0x268` blocked-step counter (> 5 aborts, "Maximum number of bumps happened", `005cbc18`, `005cbcb5 PUSH 0x9965f8`; not reset by good steps, zeroed at new request `005c1dd4`), same-blocker-same-position abort (`PI+0x254/0x258..0x260`, `0x5CBECF`, string `0x996560`), "we are totaly blocked" (`0x5CBFE9`), pathfinder caps, script force-move deadline (30 s `[0x9919F0]`, `0x68CEE0/0x68CB50`, `005c0156`, `005c01ff`), follow state 3000 ms (`005c9edb CMP [EBP-0x3c8],0xbb8`), combat LOF position equality (`+0x558`).
14. Bump abort (`0x5CB430` returns 1) makes `0x5C3460` call ResetRequest and return SUCCESS: the move ends "done" silently (a kite move blocked 6 times ends silently).
15. `0x586940` returns 1 for the leader bumping a companion (`+0x11AC`/`+0x11FC`); NPC-vs-NPC needs an idle friendly bumpee (queue empty, `+0x520 == 0`, `round+0xAA8 == 0`). `0x586B70`: bumpee with action 1 or 0x11 queued is not cleared (`00586f13/00586f19`); a following companion (queue = 0x3D) IS cleared (`00586f48 CALL 0x00541080`) and replaced by one MoveToPoint = FM1 (installed hook `CompanionAiBumpClear 0x586F43`); else teleport (`00587123 CALL 0x00543f50`).
16. `+0x558` is a combat LOF position, not movement: writers `0x6D7CBC/0x6D7DDA/0x6D7E0F` + ctor only (`AIActionPhysicalAttacks`); exact float equality via `0x517980`. Brief was wrong; gate agrees with AI2.
17. RunActions ends the loop after any finished type 1 (`ActionTypeStopsLoop 0x5D7160`), so the next queued action runs one AIUpdate later (relevant to FM2b timing).

## Struct fields
Full set (PI 0x278 bytes, ~75 rows) is in DB table `struct_fields`. PI = `*(creature+0x380)`, offsets PI-relative.
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| PI | 0x000 | u32 | bClientPath (node flag bit1) | high | `005c1918 MOV [EAX],EDX`; read `0055b3f8`/`0055b407 JNZ 0x0055b69a` |
| PI | 0x004 | f32 | PERSPACE radius (obstacle/walkmesh) | high | `0057fd44 FSTP [ECX+0x4]` |
| PI | 0x008 | f32 | CREPERSPACE radius (spacing, bump, ranges; used by retarget metric) | high | `006d7bd9`, `0057cd80` |
| PI | 0x024 | u32 | closest-result enabled (set every grid call, l.171) | med | `00551139 MOV [EDX+0x24],1`; batch named it "grid search started" (low); gate renamed |
| PI | 0x02C / 0x030 | id | owner creature id / move target id (0x7F000000 none) | high | `005c1947`, `005c1971` |
| PI | 0x03C | u32 | pathfind call counter (cap 100) | high | `0055abd7`, `0055b3bb` |
| PI | 0x058 | u32 | bNewRequest | high | `005c1d4a MOV [EAX+0x58],1` |
| PI | 0x05C..0x064 / 0x074..0x07C | vec3 | goal / start position | high | `005c1ba1`, `005c1c93` |
| PI | 0x068 | f32 | approach range | high | `005c1c55` |
| PI | 0x070 / 0x080 | id | goal area / start area id | high | `005c1c28`, `005c1d2c` |
| PI | 0x084 | u32 | start/end flipped | med | `0055b410` |
| PI | 0x088 | u32 | search done (1) / running (0) | high | `0055b815 MOV [EAX+0x88],1` |
| PI | 0x08C / 0x090 | u32 / ptr | path length in floats / path float[] | high | `0055b113`, `0055b15f` |
| PI | 0x0A8 | u32 | areas on route (10 = planning) | high | `0055715c` |
| PI | 0x0B4 / 0x0B8 | u32 / ptr | excluded-area list (re-route) | high | `0055aa91` |
| PI | 0x15C | ptr | grid temp path buffer (alloc/free per call) | high | `00551190` |
| PI | 0x168 / 0x16C | i32 / ptr | grid resume depth / DFS table cache (freed) | high | `005518f4`, `006f9bda` |
| PI | 0x188 | u32 | best (closest) path point count, returned as SUCCESS | high | gate rename (batch med "best path point count") |
| PI | 0x18C | ptr | best (closest) path buffer, NOT freed by Shutdown | high | `005511ea W`; memcpy to `PI+0x15C` l.238-250 |
| PI | 0x190 | i32 | best distance-to-goal (init 700 = `006f97f5 MOV [ECX+0x190],0x2bc`) | high | `00551403` |
| PI | 0x194 | u32 | force grid search flag | high | `005c1e75 MOV [EAX+0x194],1` |
| PI | 0x198 | u32 | straight-bisect request (node bit3) | high | `005c1db6` |
| PI | 0x19C | i32 | cached first straight-test code (-1 disables fallback) | high | `0055af92 W`, `0055af9d R` |
| PI | 0x1B4 / 0x1D8 / 0x200 | ptr | DFS cache / second array / result ownership, NOT freed | high/high/med | `00553d63`, `005539f5`, `00554204` |
| PI | 0x1E0 | u32 | path-point search state 1..5 | high | `0055b6af` |
| PI | 0x240 | u32 | bRangeStopInWalker (node bit10) | high | `005bfec3` |
| PI | 0x254 / 0x258..0x260 | id / vec3 | last blocker id / its position | high / med | `005c1fa8`, `0x5CBECF` |
| PI | 0x264 | id | blocker for `k_def_pathfail01` | med | `0x6FC83B` |
| PI | 0x268 | u32 | consecutive blocked steps (abort > 5) | high | `005cbc18`, `005c1dd4` (=0) |
| PI | 0x26C..0x274 | u32 | QuickWalk forced-move flag/deadline (zero writers only) | med | `0056455f` |
| CSWSCreature | 0x380 | ptr | PI | high | `005637c1` |
| CSWSCreature | 0x11B0 | i32 | walk state: -1 idle, 0 moving, 1 stopped, 2 requested/failed, 3 follow, 4/5/6 client walk | high | `00564546` (=2), `0051d9e2` (=1) |
| CSWSObject | 0x220 | u32 | player update-set flag (written only by `0x63C1E0`, `0063c257`) | high | gate: up from low; not a QuickWalk "selector"/party flag |
| CSWSCreature | 0x37C | id | last blocker (OnBlocked caller) | high | `005706ec` |
| CSWSCreature | 0x558 | vec3 | LOF-blocked position (AI2) | high | `006d7c56` |
| CSWSObject | 0x78 | i32 | AI level (0..4, -1 none) | high | `0055b1b5`, `005694c1` |
| ActionNode(type 1) | 0x38..0x6E | params | pos, area, target id, flags `+0x4C` (bit0 run, bit1 client path, bit2 range/timeout, bit3 partial-straight, bit9 no-retarget, bit10 range-stop), range `+0x50`, timeout `+0x58`, deadline `+0x64/68`, group `+0x6C` | high | `005bffff` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| Unreachable-target approach loop (FM2 + FM2b are phases of one loop) | CONFIRMED (gate-corrected) | companion keeps walking at an obstacle then waits 2-4 s, repeating; in combat the E2 fallback restarts it every 3 s | fail-fast keyed on the `0x11` re-approach count with no-progress test at `0x5BC5F4`; `0x5C2438` only skips the wait | FM2, FM2b |
| `0x5BC1D0` LOS | REFUTED (AI1 batch); R1 right | none: no LOS in action 0x11, so "LOS fail" never reaches the re-queue | no change; do not build on an LOS gate there | FM2b |
| OnBlocked unthrottled at 6 sites | CONFIRMED | script event storm on blocked steps; party OnBlocked script only opens doors | throttle all six: FM5 R1 hooks `0x5C8BC2/0x5C9F9D/0x5C3D8B`, AI1-H9a/b/c `0x5C124E/0x5C5A76/0x5CCBC2` | FM5 |
| Bump clears a follower queue (`0x586F48`) | CONFIRMED | companion stops following after being bumped | installed `CompanionAiBumpClear 0x586F43` skips that call | FM1 |
| PI Shutdown leak (+0x18C, +0x1B4, +0x1D8, +0x200) | CONFIRMED (U3) | KB-scale leak when a creature dies mid-search | free the four blocks in Shutdown | PFLK |
| Bump abort ends the move as "done" | CONFIRMED | a blocked kite/approach move ends silently, loop re-enters | handle by re-issue logic in the mod | FM2, Mod 8, E4, enemy-E7 |
| Level-0 creatures cannot path (only straight partial) | CONFIRMED code, reach SPECULATIVE | idle-level creatures moved by script fail to path | none yet | FM2, AIQ |
| `k_def_pathfail01` per blocked step vs hostile blocker | CONFIRMED (code), unthrottled | runs `k_ai_master` 1012 for non-leaders each blocked step | throttle | enemy-AI, FM5, perf |
| Shared 1 ms slice, path-point search unbounded | CONFIRMED structure, magnitude SPECULATIVE | ~6 searching creatures fit a 6 ms level budget | none yet | perf, AIQ, FM3, FM7 |
| `+0x558` exact-equality LOF memory | CONFIRMED (AI2 scope) | stale position after movement | see AI2 | Mod 8, A1, a1_retarget_strict |

## Hook sites
Gate-checked windows (section 5, all on instruction boundaries, no relative branch inside, no inbound target inside; no overlap with live perf-lab `0x55AB30/0x55AC26`).
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| CA-FM2 | 0x5C2438 | 9 | `8B 95 6C FA FF FF 89 51 70` | cold | FAIL-leg fail-fast; consumed exit `0x5C24C8` (`JMP 0x5C25A5`), stolen bytes read ECX (=PI) and write `PI+0x70`: restore ECX |
| CA-FM2b | 0x5BC5F4 | 9 | `8B 4D E4 8B 91 24 01 00 00` | warm (per 0.3 s re-approach) | reached only for other area or not in range; recommended FM2 trigger; start is JNZ/JZ target (`0x5BC5E2/0x5BC5E8`, allowed) |
| G-FM2c (new) | 0x55B69A | 6 | `8B 0D A0 B5 99 00` | cold | partial vs fail split: `[EBP-0x18]` = 2 or 3, `[EBP-0x20]` = cached first code, `[EBP+8]` = PI |
| AI1-H3 | 0x55B625 | 13 | `8B 4D 08 C7 81 8C 00 00 00 04 00 00 00` | cold (per granted partial) | sees old `PI+0x8C` (handler runs before stolen store); redundant with G-FM2c |
| FM5 R1 a/b/c | 0x5C8BC2 / 0x5C9F9D / 0x5C3D8B | 6 / 5 / 5 | `8B 0D A4 B4 A1 00` / `A1 A4 B4 A1 00` x2 | cold | after payload word store; payload ptr in EAX / EDX / EDX; retype-to-0 handler can use it |
| AI1-H9a/b/c | 0x5C124E / 0x5C5A76 / 0x5CCBC2 | 6 / 5 / 6 | `8B 15 A4 B4 A1 00` / `A1 A4 B4 A1 00` / `8B 95 30 FF FF FF` | cold | payload in ECX / EDX / ECX; all six sites needed for a complete FM5 throttle |

Batch-only (not gate-checked), all log-only, copied from `.asm` by the batch:
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI1-H1 (perf-lab PerfLabPathEnter) | 0x55AB30 | 6 | `55 8B EC 83 EC 30` | hot | PlotPath entry (installed) |
| AI1-H2 (PerfLabPathResult) | 0x55AC26 | 10 | `89 45 F8 83 3D D4 E0 A7 00 00` | hot | result 1/2/3 (installed) |
| AI1-H3b | 0x55B110 | 13 | `8B 45 08 C7 80 8C 00 00 00 04 00 00 00` | cold | direct 2-point path (no search) |
| AI1-H4 | 0x55B3DD | 9 | `8B 15 BC 44 99 00 89 55 E8` | cold | 100-call cap hit |
| AI1-H5 | 0x55B1C7 | 6 | `8B 0D BC 44 99 00` | cold | AI level 0 FAIL |
| AI1-H6 | 0x5CBC1E | 7 | `83 BD 1C FF FF FF 05` | cold-warm | blocked-step abort; stolen CMP flags feed `JLE 0x5CBC25` |
| AI1-H7 / H8 | 0x586B70 / 0x586940 | 5 / 6 | `55 8B EC 6A FF` / `55 8B EC 83 EC 58` | cold / warm | bump entries; H8 is a branch target at `0x54DB23` |
| AI1-H10 | 0x5C1D4A | 7 | `C7 40 58 01 00 00 00` | cold | new search request |
| AI1-H11 | 0x5C0156 | 7 | `68 D0 64 99 00 6A 00` | cold | force timeout |

## Open items
- `0x585CC0` GetIsInUseRange metric (2D/3D, radii) vs the `0xC` 2D approach test: can 0x11 say "in range" while 0xC says "approach" (ping-pong candidate; wave 2 AI1/AI2 overlap).
- Runtime split of partial vs fail and the -1 share (G-FM2c `0x55B69A`, AI1-H3). Path-point search (`0x5532F0`) outcome rules for unreachable goals not read; does it use the closest-result rule via its grid legs?
- `0x5EA8F0` (14.7 KB swept collision test): how often a straight line behind geometry yields -1 rather than 0.
- Class of PlotPath/PlotPathInterArea/PlotPathInArea `this` (GetModule vs area; search internals use an area `this`): decompile `0x52F4C0`/`0x557EF0` owners. Behaviour unaffected.
- Door `+0x3D4` (`0x61D000`) and `+0x31C` state names behind the 0x1E/0x1F split; writers of `PI+0x26C/0x270/0x274` (none found); `0x6FC5D0` return enum (`[0x99B5D0..D8]`, -2 ends the walk silently); partial-vs-fail frequency and area-mismatch reach in play.

## Mod relevance
| item | served by |
|---|---|
| FM2, FM2b | FM2/FM2b loop (one loop), fail-fast window choice (`0x5BC5F4` key, `0x5C2438` wait-skip), probe `0x55B69A`, level-0 FAIL, bump abort |
| FM5 | six OnBlocked post sites (not three), none throttled |
| FM1 | `BumpFriends` clears follower queue (`0x586F48`); hook exists |
| PFLK | PI Shutdown leaks four blocks |
| AIQ, perf, FM3, FM7 | shared 1000 us slice, unbounded path-point iteration, level-0 FAIL |
| enemy-AI, enemy-E7, Mod 8, E4 | `k_def_pathfail01`, silent bump abort, no generic stuck detector, `+0x558` |
| party-flag (`+0x11AC`) | read by the `0x5BFC00` guard (`005bfc56`) and `GetIsCreatureBumpable` |
| T1 | force-move deadline noted (cutscene only) |
