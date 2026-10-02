# AI1 Pathfinding and movement (KOTOR II Steam swkotor2.exe, static, 2026-10-02)

## Summary
1. **FM2b settled: the pathfinder returns a PARTIAL path, not only FAIL.** `0x55AC80` turns a failed search (code 3) into SUCCESS (2) with a 2-point straight-line path (6 bisection probes along start->goal, `0x55B4FB..0x55B692`) whenever `PI+0x00` (client-path flag) is 0. FAIL(3) reaches `AIActionMoveToPoint` only for: client-path nodes, all probes failing / start off-mesh (-1), no inter-area route, request area != module area, null PI. So the FM2 "fail + 2-4 s wait" is the residual case; the repeating "move, partial straight walk, 0x11 not in range, re-approach" loop (FM2b) is the main unreachable-target loop (relative rate at runtime: SPECULATIVE, probes AI1-H3/H2). New: creature AI level (`creature+0x78`) == 0 makes PlotPath fail immediately (`0x55B1B5..0x55B1D0`), then only the partial straight path remains.
2. **Only one reader of the pathfinder**: `0x5BFC00` (`CALL 0x55AB30` at `0x5C1F18`, tests at `0x5C1F45` / `0x5C21C4`). Slice = `[0x9944C4]`=1000 us -> `[0xA7E0D0]`, soft (>= 1 iteration per call, path-point iteration unbounded), plus a 100-call cap (`0x55B3BB..0x55B3DD`); the SAME constant is also `RunActions`' per-creature action budget (`0x5D6D0D`).
3. **There is no generic "creature not making progress" detector.** The real ones: `PI+0x268` blocked-step counter (>5 aborts, "Maximum number of bumps happened", `0x5CBC18`), same-blocker-same-position abort, pathfinder caps, script force-move deadline (`ActionForceMove*`), follow state 3000 ms (`0x5C9EDB`), combat LOF position equality (`+0x558`, `0x6D7C5D`).
4. **Payload word 0x1F (OnBlocked) is posted from 6 movement sites, not 3** (R1 lists 3): `0x5C1240`, `0x5C3D7D`, `0x5C5A68`, `0x5C8BB4`, `0x5C9F8F`, `0x5CCBB4`. All unthrottled. Event arg order verified: `AddEvent(dayD, timeD, callerId, targetId, 10, data)`; the creature handler switches on `*(word*)data - 1` (`0x5703CA..0x5703ED`).
5. Bump: `GetIsCreatureBumpable 0x586940` + `BumpFriends 0x586B70` run inside the per-step test `0x5CC870`; a bumpee already holding action type 1 or 0x11 is NOT cleared (`0x586F13..0x586F1F`), but a following companion (queue = 0x3D) is (`ClearAllActions` `0x586F48` = the FM1 hook site `0x586F43`). Full 0x278-byte PI field table in (e); catalogue (f) proposes AI1-H3..H11.

Method: static only. Mapper not run (CPathfind absent from GOG DB, CPathfindInformation fails, AI0 §6). No `AI1_map_*.tsv`. Constants read from `backups/vanilla_clean/swkotor2.exe` (sha256 6a522e71...) with the PE reader (`read_exe_consts.py` logic): `0x9944B4/B8/BC/C0` = 1/2/3/4, `0x9944C4` = 1000, `0x99B5A0..B0` = 1..5, `0x99241C` = 10, `0x986B70` d=0.001, `0x987490` d=2.0, `0x9946E8` d=0.005, `0x98C014` f=0.5, `0x98BE34` f=0.25, `0x98BD78` f=0.6, `0x994784` f=700, `0x989ED0` d=25, `0x9884A8` d=4, `0x987538` d=0.01, `0x993768` d=1e-5, `0x9919F0` f=30.

## (a) Function table
C = CONFIRMED against decompile/asm (ret N, callees, strings); M = mapper/DB name only.
| addr | proposed name | conf | purpose | evidence |
|---|---|---|---|---|
| 0x55AB30 | PlotPath (ret 8; this = result of `0x51BD90` GetModule, see d.1) | high C | pathfind request entry; only caller 0x5BFC00 | `0055ab3f MOV EAX,[0x009944bc]` (PI null), `0055ab6a MOV EAX,[0x009944b8]` (path held), `0055ac21 CALL 0x0055a900` |
| 0x55A900 | PlotPathInterArea (route driver, ret 8) | high C | route planning + per-segment retry | `0055a9ab CALL 0x00557150 / 0055a9b4 MOV EAX,[0x9944bc]`; `0055aaed MOV EAX,[0x9944bc]`; `0055aaf4 MOV EDX,[0x9944b4]` |
| 0x55AC80 | PlotPathInArea (ret 8) | high C | straight test, partial paths, search dispatch, 100-call cap | `0055b3c4..0055b3dd`, `0055b628 MOV [ECX+0x8c],4` |
| 0x550E30 | PlotGridPath (ret 12) | high C | restartable iterative-deepening grid DFS, time limited | strings "Exceeded limits:Time limit"/"Max search deapth"/"Distance to goal" |
| 0x5532F0 | PlotPathPointPath (ret 12) | high C | path-point search, state `PI+0x1E0` 1..5 | `0055361b MOV [ECX+0x1e0],EDX`; "we exceeded 4096 levels of searching.. " |
| 0x550980 | WalkLineTest (ret 24) | med C | straight-line walkability: 1 clear, -1 off mesh, -2 box test fail, -3 creature on line, 0 other/100 steps | decompile `local_3c = -1/-2/-3/0` |
| 0x557150 | PlanInterAreaRoute (ret 4) | med C | depth 1..9 area routes into PI+0xAC | `*(param_2+0xa8)=10`, `while (local_10 < 10 ...)` |
| 0x557EF0 | GetModuleArea (cached by id `this+0x40`) | low | area object used by 0x55AC80 | `0055acd8 CALL 0x00557ef0`, `0055ace9 MOV ECX,[EAX+0x124]` |
| 0x51BD90 | GetModule (fastcall -> 0x52F4C0) | med C | PlotPath `this` | `005c1f11 CALL 0x0051bd90 / 005c1f16 MOV ECX,EAX / 005c1f18 CALL 0x0055ab30` |
| 0x6F9200/9420/9440 | CPathfindInformation ctor / dtor / Initialize | high C | 0x278 B block | `005621bf PUSH 0x278 ... 005621dc CALL 0x006f9200`; dtor `0056378b CALL 0x006f9420` |
| 0x6F99F0 | PI Shutdown | high C | frees 0x90,0x98,0xAC,0xB0,0xB8,0xC4,0xC8,0xD0,0xD4,0xE8,0xEC,0xF0(list),0x16C,0x1AC,0x238; NOT 0x18C,0x1B4,0x1D8,0x200 | `006f9bb6..6f9c5c` |
| 0x6F9C90 | PI ResetRequest | high C | zeroes goal/start/area, calls 0x6F9D40 | `*(param_1+0x194)=0 ... FUN_006f9d40()`; called from ClearAllActions `0x541080` |
| 0x6F9D40 | PI ResetAtEndOfPath | med C | frees 0xB8 list, resets 0x84/0x88/0xB4 | `006f9d7d MOV [EDX+0x88],1` |
| 0x5BFC00 | AIActionMoveToPoint (action 1; ret 4) | high C | the movement state machine (Q7) | `005c1f18 CALL 0x0055ab30` |
| 0x5BC1D0 | AIActionCheckMoveToObjectRadius (action 0x11; ret 4) | high C | in range + LOS or re-queue [wait, move, 0x11] | `005bc652 CALL 0x00540ca0`, `005bc6d1 CALL 0x005644b0`, `005bc6ea CALL 0x005646c0` |
| 0x5644B0 | AddMoveToPointActionToFront (ret 68) | high C | packs flags, pushes node (type 1) | `005645e4 CALL 0x00540ca0` (AddActionToFront). Mapper right, DB name wrong |
| 0x564770 | AddMoveToPointAction (append) | high C | same with AddAction | `005648a4 CALL 0x0053f7f0` (found by scan) |
| 0x5646C0 | AddPathfindingWaitActionToFront (word group, float secs) | high C | 0 -> 0.3 s (`[0x986B60]`), PC-controlled 0.1 s | R1 + `005646c9 FLD float ptr [0x00986b60]` |
| 0x5C3460 | WalkUpdateLocation (server walker) | high C | steps along PI path, door cases | `005c3d7d MOV ECX,0x1f`, `005c3dc5 CALL 0x0051e170` |
| 0x5C3F70 | WalkUpdateLocation_QuickWalk (ret 4) | high C | walker for objects with `+0x220 != 0`; run by UpdateState pre-pass | `0051d9e2 MOV [ECX+0x11b0],1` in 0x51D790 after `FUN_005c3f70` |
| 0x5CB430 | WalkUpdateLocationDistance (ret 16) | high C | advances along `PI+0x90` by distance; blocked-step logic | `005cbc18 MOV [EAX+0x268],ECX`; strings 0x9965F8/0x9965A8/0x996560 |
| 0x5CC870 | WalkUpdateLocationTestDistance (ret 16) | high C | per-step clear test: box test + NoCreaturesOnLine + BumpFriends | `005ccbb4 MOV EAX,0x1f` |
| 0x6FC5D0 | AvoidCreature (go-around planner, ret 8) | med C | detour via PI+0x90 rewrite; runs script `k_def_pathfail01` when blocked creature's reputation < 0xB | string 0x99B5E0 `k_def_pathfail01`, `FUN_006fd8d0 -> 0x7022A0` |
| 0x586940 | GetIsCreatureBumpable (ret 4) | high C | bumper `this`, bumpee arg | decompile; callers 0x586B70, 0x54D750 |
| 0x586B70 | BumpFriends (ret 12) | high C | displace bumpee ("Bump! %x (%s) bumps %x (%s)") | strings 0x99569C/0x995680 |
| 0x581070 | ForceMoveToPoint | med | forced teleport after deadline | `005c01ff CALL 0x00581070` |
| 0x5BF760 | TerminateClientSidePath | med (R1) | on fail when node bit1 | `005c176c CALL 0x005bf760` |
| 0x5C6340 / 5C87E0 / 5C8C20 | FollowLeader driver / planner st2 / st3 | M+C partial | OnBlocked sites 0x5C8BB4, 0x5C9F8F | asm quoted in b.4 |
| 0x6FA3F0/6FA480/6F9DD0 | PI FlipStartEndPoints / FlipTempEndPoints / ResetInterAreaPathSearchData | med (U3) | | |
| 0x51E170 | AddEventDeltaTime(dayD,timeD,callerId,targetId,eventId,data) ret 24 | high C | arg roles from UpdateState drain | `0051ed80` out order + `0051d790` handler call |
| 0x56EFD0 | CSWSCreature::EventHandler (ret 20) | high C | switch `*(word*)data - 1` via byte table 0x571010 | `005703ca SUB EAX,1 / 005703d3 CMP,0x1e / 005703ed JMP [EDX*4+0x570ff4]` |
| 0x57FCE0 | UpdatePersonalSpace | high C | fills PI+4/8/0x10/0x14/0x18 from appearance 2DA | `0057fd44 FSTP float ptr [ECX+0x4]` |
| 0x5D56B0 | RunActions (ret 16) | high C | 1000 us budget from AIUpdate start | `005d6d0d MOV ECX,[0x009944c4]`, `005d6d39 JA 0x005d6d5f` |
| 0x983DC0 | static init `[0xA7E0D0] := [0x9944C4]` | high C | slice constant | `00983dc3 MOV EAX,[0x009944c4] / 00983dc8 MOV [0x00a7e0d0],EAX` |
| 0x736220 / 0x73B810 | Timer::GetHighResolutionTimer / Now | high C | QPC -> us, fallback `GetTickCount()*1000` | decompile 0x73B810 |
| 0x6D7390 | AIActionPhysicalAttacks (AI2) | M | `+0x558` LOF block position | `006d7c56 ADD EDX,0x558` |

## (b) Findings

### Q1 Pathfinder return codes (FM2b)  CONFIRMED: both FAIL and PARTIAL exist
Codes: `[0x9944B4]`=1 pending, `[0x9944B8]`=2 success, `[0x9944BC]`=3 fail (read from exe).
`PlotPath 0x55AB30`:
- `0055ab39 CMP [EBP+8],0 / 0055ab3f MOV EAX,[0x009944bc]` PI null -> 3.
- `0055ab4c..0055ab6a`: `+0x58 == 0 && +0x88 == 1` -> `MOV EAX,[0x009944b8]` 2 (a path is already held, nothing re-planned).
- else `+0x58 == 1` resets old state (`0055ab98 CALL 0x006fa080 / 0055aba0 CALL 0x006f9e90` or `0x6F9D40`), `[+0x3C]=0`, `[+0x88]=0`, `[+0x58]=0` (`0055abd7`, `0055abe1`, `0055abee`), then `0055ac21 CALL 0x0055a900`; result stored `0055ac26 MOV [EBP-8],EAX` and returned `0055ac75`.
`0x55A900`: FAIL at `0x55A9B4` (first `0x557150` returned 0) and `0x55AAED` (re-plan returned 0); PENDING at `0x55AAF4` (new route found after `0x55AC80` FAIL with `PI+0xA8 > 1`: `0055ab0a MOV ECX,[EAX+0xa8] / 0055ab17 JLE 0x0055ab21`; it appends the failed first area to the exclusion list `PI+0xB4/0xB8` at `0x55AA91/0x55AAC7`); otherwise the `0x55AC80` result (`0055ab21 MOV EAX,[EBP-4]`).
`0x55AC80` (`local_1c` = result, `[EBP-0x18]`):
- starts FAIL; whole body only if `557EF0` area exists and `*(area+0x124) == PI+0x80` (`0055ace9`, `0055acf8`), else returns 3 with no fallback.
- call #1 (`PI+0x3C == 1`): straight test `0x550980(start, goal)` -> `local_24`, cached in `PI+0x19C` (`0055af92` W, `0055af9d` R).
- `PI+0x198 == 1` (node flag bit3): if `local_24 != 1 && PI+0x84 == 0`, bisect the straight line (`0x55B006..0x55B0C9`), `local_24 = 1` or `-4`; then `PI+0x194 = PI+0x198 = 0` (`0055b0f3`, `0055b100`).
- `local_24 == 1`: 2-point path `PI+0x8C = 4`, `new(0x10)`, `PI+0x90 = buf`, result 2 (`0055b113..0055b165`).
- else `creature = 0x51C100(PI+0x2C)`; **`creature != 0 && creature+0x78 == 0` -> result 3** (`0055b1b5 MOV EAX,[EDX+0x78] / 0055b1be CMP,0 / 0055b1c5 JNZ 0x0055b1d5 / 0055b1c7 MOV ECX,[0x009944bc]`). `+0x78` is the AI level (`005694b8 MOV ECX,[EAX+0x78] / 005694c1 CMP,4` in AIUpdate).
- else cap value 100 (`0055b1d5 MOV [EBP-0x88],0x64`); `PI+0x194 != 0` -> `PlotGridPath` (`0055b205`); else if `local_24 in {-2,-1,-3,0}` (`0055b24d..0055b263`) run `PlotGridPath` (`area+0x25C == 0`, `0055b318`) or `PlotPathPointPath` (`0055b2c4`); any other `local_24` (`-4`) -> 3 (`0055b322`).
- cap: `0055b3bb MOV EDX,[ECX+0x3C] / 0055b3ca CMP EAX,[EBP-0x88] / 0055b3d0 JLE / 0055b3d5 CMP ECX,[0x9944b4] / 0055b3dd MOV EDX,[0x9944bc]` => pending call #101 becomes FAIL.
- **partial fallback**: `0055b3e9 CMP EAX,[0x009944bc] / 0055b3f8 MOV EDX,[ECX] / 0055b407 JNZ 0x0055b69a` (needs `PI+0 == 0`). Skips probing if `local_24 == -1` (`0055b4fb CMP [EBP-0x20],-0x1 / JZ 0x0055b61c`). Probe: `t=0.5` (`[0x98C014]`), step `0.25` (`[0x98BE34]`), `candidate = start + t*(goal-start)` (`0x45CFE0`, `0x44C8A0`), `0x550980(start, candidate)==1` -> `t += step` and remember it, else `t -= step`; `step /= 2.0` (`0055b60b FDIV double [0x00987490]`) until `step <= 0.005` (`0055b50b FCOMP double [0x009946e8]`): 6 probes, resolution ~1/128 of the segment. If any probe succeeded: `0055b628 MOV [ECX+0x8c],4`, `0055b632 PUSH 0x10 / 0055b634 CALL 0x00919723`, `0055b68c MOV [ECX+0x90],EDX`, `0055b692 MOV EAX,[0x009944b8]` => **2**. Note: the 100-call cap FAIL goes through this fallback too.
- tail: `PI+0x1E0 := 1` (`0055b6af`), flipped-path reversal when `PI+0x84 == 1` (`0055b6c4..0055b7f7`), `PI+0x88 := 1` on result 2 or 3 (`0055b812..0055b815`).
`0x550980` codes and who maps them: only `0x55AC80` (above) and the follow code (b.4) read them; `0x55AC80` maps {-1,-2,-3,0} -> search, -4 -> fail.
**Readers of 0x55AB30**: `callgraph.tsv` has exactly one call (`005bfc00 005c1f18`); no other reference to the address in any `.asm`. Mapping: `005c1f45 CMP EAX,[0x009944b8] / 005c1f4b JNZ 0x005c21c1` (2 -> walk), `005c21c4 CMP EAX,[0x009944bc] / JNZ 0x005c24c8` (3 -> fail branch), anything else = 1 (pending; `local_5c` initial `[0x9944b4]`, returned `0x5C25A5`).
Verdict for FM2b: **CONFIRMED** that unreachable targets usually get 2 (partial), so the loop is "wait 0.3 s, move, partial walk ends (`0x5C3460` returns 2), 0x11 not in range, repeat" (re-search each time: `ResetRequest 0x6F9C90` at path end zeroes the stored goal, so the next request is a new search). FAIL residue: client-path nodes (`PI+0` set only from node flag bit1, which only `0x65B4F0` / 5BFC00 re-queues pass), probe failure, `-1`, no route, area mismatch.

### Q2 PI struct  CONFIRMED (table in e). Block `creature+0x380`, 0x278 B, `new(0x278)` at `0x5621BF`, init `0x6F9440`, freed `0x56379E` after `0x6F9420 -> 0x6F99F0`. PFLK: U3 verified (list in a).

### Q3 The 1 ms slice  CONFIRMED
- Source: `00983dc3/dc8` copies `[0x9944C4]` (1000) to `[0xA7E0D0]`; `0x5BFC00` pushes it at `0x5C1EF8..0x5C1EFD` as PlotPath's limit; `0x55A900/0x55AC80` forward it (`0055b2b6 MOV EDX,[EBP+0xc]`, `PUSH 0` high dword). Timer = `0x736220 -> 0x73B810`: QPC converted to us (GetTickCount*1000 fallback).
- Grid search (`0x550E30`): `local_8c = now` at entry, loop `while (depth < max && result == pending && elapsed < limit)` with the check after each deepening iteration; setup (allocs, `0x552A90` table) counts against it. Out of slice -> returns pending, state kept in `PI+0x168` (depth), `0x16C` (table), `0x170..0x184` (snapshots: table re-initialised if start or goal moved by dist2 >= 0.0001, `[0x98C1E8]`), `0x188/0x18C/0x190` (best). Hard rules: iteration count `==1` and `elapsed >= 40*limit` -> "Exceeded limits:Time limit" FAIL; `>= 10*limit` and best distance (`PI+0x190`, init 700) unchanged -> "Distance to goal" FAIL; depth >= `dist+36` -> "Max search deapth" FAIL.
- Path-point search (`0x5532F0`): `while (iter == 0 || (pending && elapsed < limit))` (loop test `0x553BE2..0x553C05`, body `0x553C6C CALL 0x00554eb0`) = **at least one full deepening iteration per call, no timer inside `0x554EB0`**; only the 4096-level guard ends a runaway. States `PI+0x1E0` 1..5 (`[0x99B5A0..B0]`) resume across calls.
- Resume path: pending -> `0x5BFC00` returns 1 -> `RunActions` re-queues (R1 `005d6a88`) -> next tick `PI+0x88 == 0 && +0x58 == 0` skips the reset and re-enters `0x55AC80`; `PI+0x3C` counts calls (cap 100 = ~100 ms of search per request, then FAIL + partial fallback).
- Starvation: `RunActions` uses the same 1000 us: `005d6d02 CALL 0x00736220 / 005d6d0d MOV ECX,[0x009944c4] / 005d6d39 JA 0x005d6d5f` (elapsed is measured from the AIUpdate start timestamp: `00569440 CALL 0x00736220 / 00569445 MOV [EBP-0x38],EAX`, passed to `005695b1 CALL 0x005d56b0`, i.e. perception `0x56B8A0` before it eats the same budget). A pending move ends the loop anyway. Per tick one searching creature costs ~1 ms + overshoot (grid single iteration up to 40 ms, path-point unbounded) inside its `AIUpdate` time; U3's 6 ms level budget therefore fits ~6 searching creatures, more starve the rest (U3 F3, CONFIRMED, runtime untested). Each pending call also re-allocs/frees `PI+0x15C` (`8*(dist+36)+16` B) and re-copies.

### Q4 OnBlocked posts  CONFIRMED, extended (6 movement post sites, R1 saw 3)
Event contract: `0x51E170(dayD,timeD,callerId,targetId,eventId=[0x99241C]=10,data)`; `UpdateState` drain: `0051da2b CALL 0x0051ed80` copies node[0..5] to `[EBP-0x64],[EBP-0x54],[EBP-0x6C],[EBP-0x5C],[EBP-0x58],[EBP-0x50]`; the target object is looked up from node[3] (`0051da58 MOV EDX,[EBP-0x5C] / 0051da62 CALL 0x0053dfb0`) and the handler is called as `handler(node[4] eventId, node[2] caller, node[5] data, node[0], node[1])` (`0051dad2 MOV EDX,[EAX+0x78] / 0051dad5 CALL EDX`, creature branch). `0x51E220` stores `puVar1[2..5] = param_3..param_6`, so `param_3` = caller, `param_4` = TARGET. Creature handler: `005703ad MOV CX,[EAX]` (`*(word*)data`), `SUB EAX,1`, byte table `0x571010`, `JMP [EDX*4+0x570ff4]`; payload 0x1F case `0x5706A4`: `005706cf CALL 0x0051c600` (PC-controlled -> skipped), `005706ec MOV [ECX+0x37C],EDX` (blocker := caller), runs slot 12.
Sites with payload `MOV E?X,0x1F` (grep over dump, 7 hits; `0x864170` is an unrelated value-mapping switch in `0x863650`):
| site | function | condition | target / caller | return |
|---|---|---|---|---|
| 0x5C1240 | 5BFC00 case type 0xA (door) | `0x61D000(door)` (door+0x3D4 in {1,2}) && `0x585CC0` in use range && `door+0x31C == 0` (`005c11c9 MOV DL,[ECX+0x31c]`) | target = creature, caller = door | 1 (`005c128f`) |
| 0x5C3D7D | 5C3460 case 0xA | same (`005c3d0c MOV AL,[EDX+0x31c]`) | creature/door | 1 |
| 0x5C5A68 | 5C3F70 case 0xA | same (`005c59f7`) | creature/door | 1 |
| 0x5CCBB4 | 5CC870 | step blocked by a static object, no creature blocker (`local_24 == 0x7f000000`), blocker is a door with `[+0xD8] == 0x2726` (closed anim) | creature/door | n/a (step test returns 0) |
| 0x5C8BB4 | 5C87E0 (follow st2) | `0x550980` result not 1 and not (-3 with no client blocker): `005c8aa1 CMP [EBP-0x1c],-3 / 005c8b33`, falls to post | creature / `[EBP-0x5C]` | 2 |
| 0x5C9F8F | 5C8C20 (follow st3) | `0x550980` result not 1 and not -3 (`005c9f07 CMP [EBP-0x10],-3`) | creature / blocker `local_40` | 2 |
`door+0x31C == 0` is the false branch of the same case: `!= 0` posts payload 0x1E to the door (`0x5C12F5`, `0x5C3E4B`, `0x5C5B4E`) and returns 2. Correction to R1: its FM2 "door branch" `0x5C0732` is the type-7 (TRIGGER; `0x6867A0` maps internal type 7 -> script type 4) case, whose 0x1E posts are `0x5C0732`, `0x5C3C10`, `0x5C58F2`; the real door case is internal type 0xA. `0x61D000 = (byte[+0x3D4] == 1 || == 2)`; door `+0x31C/+0x31D` = state index set by `0x61D6E0` (anim 0x2726/0x2742/0x2743/0x2758). Semantics of the state names: SPECULATIVE. No post is time gated (verified: none of the six has a timestamp compare); the only throttle is the PC check in the handler. FM5 R1 windows `0x5C8BC2`, `0x5C9F9D`, `0x5C3D8B` = sites 5, 6, 2; AI1-H9a..c cover 1, 3, 4.
`+0x558`: not a movement field. `ADD EDX,0x558` occurs only in `AIActionPhysicalAttacks 0x6D7390` (`006d7c56`, `006d7cbc`, `006d7dda`, `006d7e0f`) and the ctor (`00562066`, `00562bc1`): a ranged-attack blocked-position memory. Logic: LOF check `0x52BB60` fails -> if `Vec3::Equal(+0x558, creature+0x94)` (`006d7c5d CALL 0x00517980`) and `[+0x10E8] != 0` -> `0x584DA0(0xDA)`, zero `+0x558`, `+0x548 = invalid`, `GetNearestVisibleEnemyInRange` for another target (`006d7d24 CALL 0x0057c990`), none -> `EndAttackAction 0x6DAB20(1)`; else `+0x558 := +0x94` (`006d7dda..`). Zeroed on LOF success (`006d7e0f`). Exact float compare, never reset elsewhere (review `Stale line-of-fire position`: CONFIRMED). AI2/Mod 8 scope.

### Q5 Stuck / no-progress detectors  REFUTED for a generic detector; CONFIRMED specific ones
No code compares the creature's own position between ticks. What exists (all with addresses):
1. Pathfinder: `PI+0x3C` > 100 (`0x55B3BB`), grid time/progress/depth rules (b.Q3), 4096 levels (`0x5532F0` string).
2. Walker `0x5CB430` (blocked step = `0x5CC870 == 0`, `005cbbc6 CALL 0x005cc870 / 005cbbcd JNZ`): `PI+0x268 += 1` (`005cbc18`); `> 5` -> `PI+0x8C := 0`, free `PI+0x90`, log "aborted walking, Maximum number of bumps happened" (`005cbcb5 PUSH 0x9965f8`), return 1. The counter is NOT reset by successful steps; it is zeroed only when a new path request starts (`005c1dd4`). Return 1 makes `0x5C3460` call `ResetRequest 0x6F9C90` and return SUCCESS (action ends "done"): `if (local_5c == 1) { FUN_006f9c90(); (**(code **)(*param_1 + 0x7c))(); return DAT_009944b8; }`.
3. Same blocker: `PI+0x254` (blocker id) and `PI+0x258..0x260` (its position when it blocked); `0x5CBECF FCOMP double [0x993768]` (1e-5): "aborted walking, Bumped into this creature at this position already." (`0x5CC1E3 PUSH 0x996560`). Also reached for ANY blocker that is not a creature (static geometry): `LAB_005cc1c4` abort + return 1.
4. "we are totaly blocked" (`0x5CBFE9 PUSH 0x9965A8`): `0x6FC5D0` returned `[0x99B5D0]`=0 (no detour) -> abort, and for the PC `0x73F750(0xBAF3)` + `CGuiInGame::SetCombatMessage`. Blocker == move target (`PI+0x30`) -> done (`0x5CBD02/0x5CBD27` reads of `PI+0x30`).
5. Script force-move deadline: `ActionForceMoveToLocation` cmd `0x17E` (`0x68CEE0`) and `ActionForceMoveToObject` cmd `0x17F` (`0x68CB50`), default timeout 30.0 s (`[0x9919F0]`, `if (param_2 == 0x17f)` / `0x17e` in the decompiles): node `+0x58` (secs) is converted on first run to an absolute deadline in node `+0x64/+0x68` (`005c0290 FMUL [0x986B48]`(1000.0) ... `005c02b9 CALL 0x0051aef0`, re-queue `005c0329 CALL 0x005644b0`), expiry test `005c0137 CALL 0x0051afe0 / 005c013e JL`, then `005c0156 PUSH 0x9964d0` ("Force Timeout, executing a forced move to point."), `005c01ff CALL 0x00581070` (ForceMoveToPoint), returns 2. Only script-forced moves; the QuickWalk copy of the check (`PI+0x26C != 0`) has only zero writers in vanilla (dead, SPECULATIVE).
6. Follow driver: `FollowInfo+0x6C += creature+0xD4` (ms per AI update, `005c8cb7..005c8cd2`); state 3 with clear line and `>= 3000` (`005c9edb CMP [EBP-0x3c8],0xbb8`) -> `FollowInfo+4 := 4`.
7. Combat: `+0x558` LOF (b.Q4).
8. Nothing else found: `0x51AFE0` (time compare) has only two movement callers (`5BFC00`, `5C3F70`); follow code `0x5C6340/5C7AD0/5C87E0/5C8C20` has no time-compare callee besides the counter above.

### Q6 Bump vs in-progress move  CONFIRMED
- `0x586940` (bumper `this`, bumpee arg): 0 if `GuiInGame+0xCC != 0` (`0x73F750`); 0 if there is no party member 0; 1 if the bumper's client id equals party member 0 (the leader/PC) and `bumpee+0x11AC != 0 || bumpee+0x11FC != 0` (`005869d7 MOV EDX,[ECX+0x11ac] / 005869e9 MOV ECX,[EAX+0x11fc]`): the leader can always bump a companion. Else, with global `[0xA0F5AC] == 0`, only for a non-party bumpee with `GetReputation(bumpee,0) > 9` (`0x5E13C0`): if the bumper is the controlled creature the result is the bumpee's party flag (0 here) and 0 when its head action is 0xC/0x3F/0xF/0x2E; if neither is the controlled creature: 1 only when the bumpee's queue is empty (`0x7A1780`), `bumpee+0x520 == 0` (not in combat) and `round+0xAA8 == 0`. So NPC-vs-NPC bumps need an idle friendly bumpee.
- `0x586B70`: logs, computes a side spot `r1+0.1+r2` (`PI+8` of both, `[0x987DF0]`=0.1) from the segment (`0x514620`), walkmesh check `0x54BDC0` + `0x52D4B0`; if OK: scan the bumpee's queue (`bumpee+0x100`): type 1 or 0x11 present -> skip (`00586f13 CMP [EBP-0x78],1 / 00586f19 CMP,0x11`), else `ClearAllActions(bumpee,1)` (`00586f48 CALL 0x00541080`, sets `+0x11B0 := 1`) and `AddMoveToPoint(group 0xFFFF, spot, area, run=1)` (`00586fc6 CALL 0x005644b0`); returns 1 (`0058701e MOV [EBP-0xb4],1 ... 00587037`). Else tries the opposite side (`0x54B970`) and TELEPORTS the bumpee (`00587123 CALL 0x00543f50`, clears the party follow-path entry `+0x28`), return 1; failure restores position and returns 0.
- Interaction: called from the per-step test (`5CC870`, bumper = walker): after a successful bump the test re-runs `NoCreaturesOnLine` and only if clear lets the step through. So (i) a bumpee mid-move (type 1/0x11) keeps its queue, is not displaced by the first branch, and the walker stays blocked (counter `PI+0x268` ticks up, abort after 6); (ii) a following companion (queue = `0x3D` only, FollowLeader, `+0x11B0=3`) is not 1/0x11, so its queue IS cleared and replaced by one MoveToPoint: it stops following afterwards (FM1; installed hook `CompanionAiBumpClear` at `0x586F43` skips exactly that call). `0x586940` is also called from `NoCreaturesOnLine 0x54D750` (`0054db23`), i.e. per candidate creature on every walked segment (warm, SPECULATIVE hot).

### Q7 AIActionMoveToPoint state machine  CONFIRMED
Node params (0x74 B node, `_memcpy` at `0x5BFD..0x5C0090`, gated by `*(u16*)(node+0x6E)` = count at `005bffff`): `+0x38/3C/40` pos, `+0x44` area id, `+0x48` target obj id (0x7F000000 none; `local_14`), `+0x4C` flags (bit0 run, bit1 client path, bit2 range/timeout pending, bit3 partial-straight, bits4-6, 7-8, bit9 "no retarget", bit10 range-stop-in-walker), `+0x50` range f32, `+0x54` u32, `+0x58` force timeout secs, `+0x5C/60` offset, `+0x64/68` deadline, `+0x6C` u16 group. Creature (`[EBP-0x5A8]`): `+0x11B0` walk state, `+0x220` walker selector, `+0x380` PI, `+0x11AC` party flag, `+0x1114` capability bits. Result locals: `[EBP-0x58]` = PlotPath result / action result, `[EBP-0x24]` = previous walk state.
Phases in order, exits quoted:
| # | addr | condition | returns |
|---|---|---|---|
| X1 | 0x5BFC9E | dead (`vt+0x94 == 1`, `005bfc44`) or party member with `vt+0x9C(0) <= 0` (`005bfc56..005bfc7e`) | 3 |
| X2 | 0x5BFCEA | `+0x220 != 0` and walk state in {0,4,5,6} (`005bfcd2..005bfce8`); if `+0x220 == 0` the state is forced to -1 (`005bfcfc`) | 1 |
| X3 | 0x5BFD15 | `creature+0x380 == 0` | 3 |
| -  | 0x5BFFE1 | node has param 7: `0x52BB60` fails -> range := 0 | continue |
| X4 | 0x5C0204 | force deadline passed (`0x5C0137..0x5C013E`), log "Force Timeout...", `0x581070`, state 1 (`005c01e8`) | 2 |
| X5 | 0x5C0395 | bit2 && `+0x58 > 0`: deadline conversion (above), re-queue shorter node (`005c0329`), PI `+0x26C..274 = 0`, state 1 | 2 |
| X6 | 0x5C0931, 0x5C0FB9, 0x5C10BC, 0x5C1134 | target type 5 (creature; `0x6867A0` GetObjectType: 5 creature, 7 trigger, 9 placeable, 10 door): stored node position stale (> 2 m, `[0x9884A8]`=4.0 dist2) or area differs -> re-queue move to the target's current position (`005c092c`, `005c0fb4 CALL 0x005644b0`); in use range (`0x585CC0`) or within 0.001 (`[0x986B70]`) -> `0x6F9C90`, `vt+0x7C(10000)`, state 1 (`005c109a`, `005c112a`) | 2 |
| X7 | 0x5C06A6 / 0x5C0798 | target type 7 (trigger): `+0x2FC != 0` -> re-queue to the point 0.5 m (`[0x98C014]`) from the object (`005c06a1`); else in use range -> payload 0x1E to the object (`005c0732..005c0793`) | 2 |
| X7b | 0x5C1546 / 0x5C1725 | target types 9 / 0xA: object moved -> re-queue with bit9 set (`005c1531`, `005c1710`) | 2 |
| X8 | 0x5C128F | target type 0xA (door) open-ish && in range && `door+0x31C == 0`: payload 0x1F (b.Q4) | 1 |
| X9 | 0x5C13B4/0x5C12F5 | door `+0x31C != 0`: payload 0x1E, anim 10000, `0x6F9E90/9F70/A080`, state 1 | 2 |
| X10 | 0x5C1795 | `(+0x1114 & 2) != 2` ("I have no legs! I can't feel my legs!!!!", `005c1783`; `0x5BF760` if bit1) | 3 |
| X11 | 0x5C17AE | `GetArea(creature) == 0` | 1 |
| X12 | 0x5C18ED | goal within 0.1 m (`[0x987538]`) in same area: `0x54B650` safe -> `SetPosition(dest)` else "Bailed the desired position is unsafe." (`005c18c3`); anim 0x2711 | 2 |
| goal change | 0x5C1B3F..0x5C1D69 | stored goal differs (area or dist2 >= 0.01): PI goal/start/areas/range := new (`+0x5C..+0x80`), `PI+0x58 := 1`, `PI+0x88 := 0` | -> request |
| request | 0x5C1D98.. | `PI+0x88 == 0`: `PI+0x198 := bit3`, `PI+0x268 := 0`, `PI+0x194 := 1` if `creature+0x550 != invalid` and dist2 to goal < 25 (`005c1e75`), `+0x11B0 := 2` if `+0x220`, `PI+0x240 := bit10`, `PlotPath` (`005c1f18`) | by result |
| R2 | 0x5C204D.. | PlotPath 2: PI+0x28/0x30 sync, `[creature+0x54C] := PI+0x30` (`005c1f8a`), `PI+0x254 := 0x7F000000`, anim 0x2712 walk / 0x2714 run (`creature+0x11BC` = run flag, `0x5C209C`), then `+0x220 == 0` -> `WalkUpdateLocation` (`005c217c CALL 0x005c3460`); `+0x220 != 0` -> stamp `creature+0xB4/0xB8` (`005c2148 CALL 0x0051c350 / 005c214f CALL 0x0051ad20`), state 0 (`005c215a`), `QuickWalk(1)` (`005c216c`); then `0x52F330` (`005c21b1`) | walker result (1/2) |
| R1 | 0x5C258C | PlotPath 1 (also path-held branch with `+0x220 != 0`, state 0 `005c2567`) | 1 |
| R3 | 0x5C21C4 | PlotPath 3: log "The Path find has Failed... Why?" (`005c21e2`), state 2 if `+0x220`, anim `vt+0x7C(0x2710)` (`005c224d`), `0x5BF760` if bit1 (`005c2278`), PI goal := creature pos (`005c2392/23C5/23F8`) and area (`005c243e`), wait `rand()%100/50.0 + 2.0` = 2.00..3.98 s at `0x5C2484` (`CALL 0x005646c0`), then, if `[0xA7E0B8] == 0` (never written, BSS) and the node carries a force deadline (`+0x64/+0x68` non-zero), `SetPosition(dest)` (`0x543F50`) and return 2, else | 3 (`005c25a5`) |
| path held | 0x5C24CD.. | `PI+0x88 == 1`: anim, state 0, `+0x220 == 0` -> `WalkUpdateLocation 0x5C259D`, else PENDING | walker / 1 |
The wait node inserted at `0x5C2484` is a front action (type 0x1E `AIActionWait`) in the same group; the finished move node is already popped by `RunActions` (R1). RunActions treats 2 and 3 identically.
`creature+0x11B0` values written: -1 idle (ctor `00562f41`, X2 reset), 0 moving server path, 1 stopped/arrived (`ClearAllActions 0x541210`, X4..X9, walker done), 2 path requested / failed, 3 follow (`005bf925`), 4/5/6 client-driven walk set by `0x5A7820/0x5A7E70` (`005a7bf0=6`, `005a7e3c=5`, `005a8339=4`; they also write 2-point paths straight into PI: `005a7c0f/005a82ef MOV [..+0x8c],4`).
`0x5BC1D0` (type 0x11) contract: dead/no legs -> 3; target missing or not an object -> 3 (`005bc41b`); range := `max(0, range - target radius)` (`005bc461..005bc48a`), `GetIsInUseRange 0x585CC0` (`005bc4b0`); if in range, LOS `0x52BB60` eye-to-eye (+1.5 `[0x989F10]`) and when LOS fails `local_48 := 0` (`005bc5d7`); `if (otherArea || !inRange || !los)` (`005bc5e2..005bc5ee`) -> re-queue: `0x540CA0(0x11,...)` (`005bc652`), `0x5644B0` move to target (`005bc6d1`), `0x5646C0(group, 0.0)` (`005bc6ea`); always returns 2 (`005bc6ef`).

### N1 AI level gate and module-area gate (new)  CONFIRMED code, reach SPECULATIVE
`creature+0x78 == 0` (AI level 0) -> PlotPath 3 then partial straight only (a). And all searching requires `*(0x557EF0(this)+0x124) == PI+0x80`; `this` of PlotPath is `GetModule` (`005c1f11`), so requests whose start area differs from the module's current area return 3 with NO partial fallback.

### N2 `k_def_pathfail01`
`0x6FC5D0` (the go-around planner called from `0x5CB430` when a creature blocks) runs script `k_def_pathfail01` (`FUN_006fd8d0` -> RunScript `0x7022A0`) with ObjectSelf = the blocked creature when `GetReputation(self,0) < 0xB` (`0x5E13C0`; `iVar4 < 0xb`), and stores `PI+0x264 := blocker` (read by `ExecuteCommandGetFoundEnemyCreature 0x697D50`). Review notes `k_def_pathfail01` runs `k_ai_master` 1012 for non-leaders: unthrottled per blocked step (`DAT_00a7e12c++` counter at entry).

### N3 PFLK recheck  CONFIRMED (U3)
`6F99F0` frees listed fields (a) and zeroes `+0x1AC`; `+0x18C` (`005511ea W`, best path buffer), `+0x1B4` (DFS table cache while pending, `00553d63`), `+0x1D8` (`005539f5`) and `+0x200` (`00554204`) are not freed. Dtor chain `0x5634C0` then frees the 0x278 block. Creature destroyed mid-search leaks those blocks (KB scale).

## (c) Runtime probes (log-only; hooks in f)
| item | counter / log | hook | expected signal |
|---|---|---|---|
| FM2b: partial vs fail | per creature: results {1,2,3} (existing H2) + partial-granted count | AI1-H3 `0x55B625` | `partial / (partial+fail)` per (creature,target); a FAIL rate near zero with high partial = FM2b dominant |
| direct / bit3-bisect path (no search) | count of 2-point paths granted before any A* | AI1-H3b `0x55B110` | moves resolved without A* (first straight test, or bit3 bisect) |
| AI level 0 FAIL | creature id, `PI+0x2C` | AI1-H5 `0x55B1C7` | non-zero only for level-0 creatures that move |
| 100-call cap | `PI+0x3C`, creature | AI1-H4 `0x55B3DD` | cap hits; each is also a partial candidate |
| new search starts | creature, goal | AI1-H10 `0x5C1D4A` | searches per request, per second |
| blocked-step abort | `[EBP+0xffffff1c]` count, creature `[EBP+0xfffffc88]`, blocker = out param `local_cc[0]` of `0x5CC870` | AI1-H6 `0x5CBC1E` | 6th tick per path = abort; rate vs OnBlocked |
| OnBlocked rate per site | site id, creature, blocker | AI1-H9a/b/c + R1 FM5a/b/c | posts/s per creature; confirms "no throttle" |
| force timeout | creature | AI1-H11 `0x5C0156` | only in forced-move cutscenes |
| bump | calls, result (return 1 at `0x587037` or alternate `0x587113`, return 0 via `0x58724C`), bumpee queue head | AI1-H7/H8 | 1/0x11 skip vs clear ratio |
| slice | per-call us, pending chain length per creature | existing H1/H2 (+ QPC pair) | median ~1000 us, max overshoot |

## (d) Open questions
1. `this` of PlotPath/PlotPathInterArea/PlotPathInArea is the result of `0x51BD90` (GetModule); DB/mapper call them `CSWSArea::*` and the search internals (`0x550E30`, `0x5532F0`, `area+0x25C`, `+0x1B0`, `+0x274`) use an AREA `this` (`0055b2c1 MOV ECX,[EBP-0x1c]`). Class names need a decompile of `0x52F4C0`/`0x557EF0` owners; behaviour unaffected.
2. Writer of `creature+0x220` (QuickWalk selector): only `0x53EDC3` (ctor, 0) found by census; a client-side function `0x63C1E0` writes `[obj+0x220]` of an object from `vtbl+0x10` with value `0x63DD30(...)` or 1 when `+0x260 != 0`. Meaning ("observed by a client") SPECULATIVE. Needs the Ghidra database.
3. Exact meaning of door `+0x3D4` (`0x61D000`) and `+0x31C` states for the 0x1E/0x1F split.
4. Who sets `PI+0x26C/0x270/0x274` non-zero (none found; QuickWalk forced-move branch dead?).
5. `0x6FC5D0` return enum (`[0x99B5D0]=0`, `[0x99B5D4]=-1`, `[0x99B5D8]=-2`): a `-2` result ends the walk silently inside `0x5CB430` (return 1, no log); details of the detour rewrite (`0x6FD270`) not decoded.
6. The relative frequency of partial vs fail results at runtime; whether `557EF0(module)` differs from the creature area during normal play (area mismatch FAIL reach).
7. Missing from `decomp/`: none needed; `0x73B810` helper `0x91C896` (QPC scaling) not read (assumed us).
8. Contradiction notes: R1 (`FM5: 3 post sites`) vs this file (6 movement posts, grep list in b.Q4); R1 "plausible part: whether 0x55A900 returns 3 or 2 with a partial path" resolved as above (both). U3/R1 call PlotPath `CSWSArea::`; see d.1.

## (e) Struct field tables
CPathfindInformation (PI) = `*(creature+0x380)`, 0x278 bytes. Offsets are PI-relative. R/W = representative addresses (census over PI accessors; provenance `field 0x380` / arg1 / this).
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| PI | 0x000 | 4 | u32 | bClientPath (node flag bit1) | 0x55B3F8 | 0x5C1918, Init 0x6F9440 | high | `005c1918 8910 MOV dword ptr [EAX],EDX` after `005c190f SETNZ DL`; `0055b3f8 8b11 MOV EDX,dword ptr [ECX]` + `0055b407 JNZ 0x0055b69a` |
| PI | 0x004 | 4 | f32 | PERSPACE radius (obstacle/walkmesh), def 0.6 | 0x585F4B, 0x550EDD | 0x57FD44, Init 0x6F945B | high | `0057fd44 FSTP float ptr [ECX+0x4]`; `00585f4b FLD float ptr [EDX+0x4]` |
| PI | 0x008 | 4 | f32 | CREPERSPACE radius (creature spacing, bump, ranges) | 0x6D7BD9, 0x586C50, 0x57170C | 0x57FDC4, 0x6D7C15 (temp 0.01 `[0x98C628]`) | high | `006d7bd9 FLD float ptr [EAX+0x8]`, `006d7c15 FSTP float ptr [EAX+0x8]` |
| PI | 0x010 | 4 | f32 | CAMERASPACE (def = +0x8) | 0x54E474 | 0x57FE36 | med | 2DA column order in `0x6E7C70` (CAMERASPACE `0x99AAA8`) |
| PI | 0x014 | 4 | f32 | HEIGHT (def 1.0) | 0x586DC2, 0x55ACCC | 0x57FF27 | high | `0055accc FLD float ptr [EDX+0x14]` |
| PI | 0x018 | 4 | f32 | hitdist (def 0.5) | - | 0x57FED1 | low | string `hitdist` 0x99A9F0 -> `[0xA0FDD8]` -> `0057fed1` |
| PI | 0x024 | 4 | u32 | grid search started | 0x551368 | 0x551139 | low | `00551139 MOV [EDX+0x24],1` |
| PI | 0x028 | 4 | u32 | client path id low byte | 0x6358D6 | 0x5C204A | low | `local_94 & 0xff` |
| PI | 0x02C | 4 | id | owner creature object id | 0x55AC35, 0x55B176 | 0x5C1947, 0x56223C | high | `005c1947 89422c MOV [EDX+0x2c],EAX` |
| PI | 0x030 | 4 | id | move target object id (0x7F000000 none) | 0x5C1F5D, 0x5C3B2F, 0x5CB566 | 0x5C1971, 0x56E811 | high | `005c1971 895130 MOV [ECX+0x30],EDX` |
| PI | 0x038 | 4 | id | attack/last target id (init invalid) | 0x585FDC | 0x6D7402 | low | `006d7402 MOV [EDX+0x38],0x7f000000` |
| PI | 0x03C | 4 | u32 | pathfind call counter | 0x55B3BB, 0x55AD1F | 0x55ABD7 (=0), 0x55AD34 (+1) | high | `0055abd7 MOV [ECX+0x3c],0`; `0055b3bb MOV EDX,[ECX+0x3C]` |
| PI | 0x058 | 4 | u32 | bNewRequest (1 = reset+search) | 0x55AB4C | 0x5C1D4A (=1), 0x55ABEE (=0) | high | `005c1d4a MOV [EAX+0x58],0x1` |
| PI | 0x05C/60/64 | 12 | vec3 | goal position | 0x55ACA5, 0x550EC8 | 0x5C1BA1, 0x5C2392 (:= creature pos on fail) | high | `005c1ba1 FSTP float ptr [ECX+0x5c]` |
| PI | 0x068 | 4 | f32 | approach range | 0x55AD46, 0x5CB714 | 0x5C1C55 | high | `005c1c55 FSTP float ptr [EAX+0x68]` |
| PI | 0x070 | 4 | id | goal area id | 0x55A90D, 0x5C1A42 | 0x5C1C28, 0x5C243E | high | `005c1c28 MOV [EDX+0x70],EAX` |
| PI | 0x074/78/7C | 12 | vec3 | start position | 0x55AC93 | 0x5C1C93 | high | `005c1c93 FSTP float ptr [ECX+0x74]` |
| PI | 0x080 | 4 | id | start area id | 0x55ACF8, 0x55A916 | 0x5C1D2C | high | `005c1d2c MOV [EDX+0x80],EAX` |
| PI | 0x084 | 4 | u32 | start/end flipped | 0x55AFCF, 0x55AB..B7 | 0x55B410, 0x55ABCE | med | `0055b410 MOV [EAX+0x84],0` |
| PI | 0x088 | 4 | u32 | search done (1) / running (0) | 0x55AB5B, 0x5C1D7F | 0x5C1D69 (=0), 0x55B815 (=1), Init (=1) | high | `0055b815 MOV [EAX+0x88],1` |
| PI | 0x08C | 4 | u32 | path length in floats (2/point) | 0x5CB466, 0x55B6E3 | 0x55B113 (=4), 0x5CBC43 (=0) | high | `0055b113 MOV [EAX+0x8c],4` |
| PI | 0x090 | 4 | ptr | path float[] | 0x5CB9AA, 0x55B6F2 | 0x55B15F, 0x5CBC8C | high | `0055b15f MOV [EAX+0x90],ECX` |
| PI | 0x094/098 | 8 | u32/ptr | grid/path-point result count + array | - | 0x551B9C, 0x554D70 | med | `00551b9c MOV [ECX+0x94],EAX` |
| PI | 0x09C | 4 | u32 | path cursor | 0x5C3862, 0x5CB47B | 0x5C206E (=0), 0x5CC3A3 | med | census |
| PI | 0x0A0 | 4 | u32 | inter-area route found | 0x5571CC | 0x5571A7, 0x55A93C | high | `0055a93c MOV [ECX+0xa0],1` |
| PI | 0x0A8 | 4 | u32 | areas on route (10 = planning) | 0x55A9ED, 0x55AB0A | 0x55715C, 0x55A949 | high | `0055715c MOV [EAX+0xa8],0xa` |
| PI | 0x0AC/0B0 | 8 | ptr | route arrays | 0x55A92A, 0x55AA71 | 0x55717F, 0x55719E | med | `0055717f MOV [EDX+0xac],EAX` |
| PI | 0x0B4/0B8 | 8 | u32/ptr | excluded-area list (re-route) | 0x55AA03, 0x55AA51 | 0x55AA91, 0x55AAC7 | high | `0055aa91 MOV [EDX+0xb4],ECX` |
| PI | 0x0BC | 4 | id/u32 | ignore-object for box test (init invalid) | 0x5CC986 | 0x670FAA | low | arg to `0x52D130` |
| PI | 0x108..0x258 (vec3 members) | - | vec3 x n | scratch vectors zeroed by ctor `0x6F9200` | - | `006f9220..006f93eb` | low | ctor |
| PI | 0x120..0x158 | 0x3C | mixed | grid transform basis | 0x550E30 | 0x551082..0x55109D | med | `*(param_2+0x120)=local_90 ...` |
| PI | 0x15C | 4 | ptr | grid temp path buffer (alloc/free per call) | 0x551677 | 0x551190 | high | U3 + `00551190` |
| PI | 0x160 | 4 | u32 | grid result point count | 0x551A02 | 0x551650 | med | |
| PI | 0x164 | 4 | f32 | slice marker (10.0 during search, 700 after) | - | 0x5512FE | low | `_DAT_00987704`/`_DAT_00994784` |
| PI | 0x168 | 4 | i32 | grid resume depth | 0x5512DA | 0x5518F4 | high | `005518f4 MOV [EDX+0x168],EAX` |
| PI | 0x16C | 4 | ptr | grid DFS table cache (freed by Shutdown) | 0x551264 | 0x551915, 0x6F9BDA | high | `006f9bda MOV [EAX+0x16c],0` |
| PI | 0x170/17C | 24 | vec3 x2 | start/goal snapshot (restart test) | 0x550E30 | 0x550E30 tail | med | `FUN_004452b0(local_4c,&local_2c,param_2+0x170)` |
| PI | 0x188 | 4 | u32 | best path point count | 0x551537 | 0x5511DA | med | |
| PI | 0x18C | 4 | ptr | best path buffer, NOT freed by Shutdown | 0x551199 | 0x5511EA | high | U3 + census |
| PI | 0x190 | 4 | i32 | best distance-to-goal (init 700) | 0x5513E0 | 0x551403, Init `006f97f5 MOV [ECX+0x190],0x2bc` | high | |
| PI | 0x194 | 4 | u32 | force grid search flag | 0x55B1E2 | 0x5C1E75 (=1), 0x55B0F3 (=0) | high | `005c1e75 MOV [EAX+0x194],1` |
| PI | 0x198 | 4 | u32 | straight-bisect request (node bit3) | 0x55AFA9 | 0x5C1DB6, 0x5C1F38, 0x55B100 | high | `005c1db6 MOV [EAX+0x198],ECX` |
| PI | 0x19C | 4 | i32 | cached first straight-test code | 0x55AF9D | 0x55AF92 | high | |
| PI | 0x1A4/1A8 | 8 | i32 | path-point start/goal node ids | 0x5537AB | 0x55363D | med | |
| PI | 0x1AC | 4 | ptr | path-point array (freed by Shutdown) | 0x553836 | 0x553860, 0x6F9C4F | high | |
| PI | 0x1B4 | 4 | ptr | DFS table cache while pending, NOT freed | 0x553A8E | 0x553D63 | high | U3 |
| PI | 0x1B8..0x1CC | 24 | vec3 x2 | path-point start/goal snapshot | - | 0x5532F0 | med | |
| PI | 0x1D0/1D4 | 8 | f32 | f-limit / best (iterative deepening) | 0x553B30 | 0x553D5A | med | |
| PI | 0x1D8 | 4 | ptr | second array, NOT freed | 0x5539A0 | 0x5539F5 | high | U3 |
| PI | 0x1E0 | 4 | u32 | path-point search state 1..5 | 0x5533AE, 0x79976F | 0x55361B, 0x55B6AF | high | `0055b6af MOV [EDX+0x1e0],EAX` |
| PI | 0x200 | 4 | ptr | result ownership, NOT freed | 0x5541D4 | 0x554204 | med | U3 |
| PI | 0x210 | 4 | u32 | path-point `bFirstLast` flag | - | 0x554008, 0x5BFC00@5C1EDF (`creature+0x26A` -> =1) | low | |
| PI | 0x23C/240 | 8 | u32 | `+0x240` = bRangeStopInWalker (node bit10) | 0x55AD69, 0x5CB669 | 0x5BFEC3 | high | `005bfec3`: `*(PI+0x240) = local_2c` |
| PI | 0x244 | 4 | u32 | go-around side flag (mirrored between two blockers) | 0x5CBF15 | 0x5CC0D0 | med | |
| PI | 0x248..0x250 | 12 | vec3 | last target position | - | 0x5BFC00 case 5/7/9/0xA | med | `*(iVar13+0x248)=*(local_f4+0x94)` |
| PI | 0x254 | 4 | id | last blocker creature id | 0x5CBE34 | 0x5C1FA8 (invalid), 0x6FC5D0 | high | `005c1fa8 MOV [ECX+0x254],0x7f000000` |
| PI | 0x258..0x260 | 12 | vec3 | blocker position at block | 0x5CBECF | 0x6FC5D0 | med | |
| PI | 0x264 | 4 | id | blocker for `k_def_pathfail01` | 0x697DAA | 0x6FC83B | med | |
| PI | 0x268 | 4 | u32 | consecutive blocked steps (abort > 5) | 0x5CBBDF | 0x5CBC18 (+1), 0x5C1DD4 (=0) | high | `005cbc18 MOV [EAX+0x268],ECX` |
| PI | 0x26C/270/274 | 12 | u32 | QuickWalk forced-move flag/deadline | 0x5C426F | zero only: 0x56455F, 0x5C035B, 0x5C45B6 | med | `0056455f MOV [EDX+0x26c],0` |
Other fields touched (creature unless noted):
| class | offset | size | type | name | readers | writers | conf | evidence |
|---|---|---|---|---|---|---|---|---|
| CSWSCreature | 0x380 | 4 | ptr | PI | everywhere | 0x562209 ctor, 0x5637C1 dtor | high | `005637c1 MOV [EAX+0x380],0` |
| CSWSCreature | 0x11B0 | 4 | i32 | walk state (b.Q7) | 0x5BFCA8, 0x51D8xx | 0x5644B0 (=2), 0x5BF925 (=3), 0x5A7BF0 (=6) ... | high | `00564546 MOV [EDX+0x11b0],2` |
| CSWSCreature | 0x220 | 4 | u32 | QuickWalk/pre-pass selector | 0x5BFCBD, 0x51D8A2 | ctor only (see d.2) | low | |
| CSWSCreature | 0x37C | 4 | id | last blocker (OnBlocked caller) | script via event | 0x5706EC | high | `005706ec MOV [ECX+0x37C],EDX` |
| CSWSCreature | 0x558 | 12 | vec3 | LOF-blocked position | 0x6D7C56 | 0x6D7CBC, 0x6D7DDA, 0x6D7E0F | high | b.Q4 |
| CSWSObject | 0x78 | 4 | i32 | AI level (0..4; init -1) | 0x55B1B5, 0x5694B8 | 0x53EB61 | high | `005694c1 CMP [EBP-0xbc],4` |
| CSWSCreature | 0x1114 | 2 | u16 | capability bits (bit 2 = can move) | 0x5C1745 | - | med | `005c1745 MOVZX EDX,word ptr [EDX+0x1114]` + `AND` 2 |
| CSWSCreature | 0xD4 | 4 | u32 | ms since last AI update | 0x5C8CB7 | - | med | added into FollowInfo+0x6C |
| FollowInfo | 0x6C | 4 | u32 | ms accumulated in state | 0x5C9ED2 | 0x5C8CD2 | high | `005c8cd2 MOV [ECX+0x6c],EAX` |
| ActionNode(type 1) | 0x38..0x6E | 0x38 | params | see b.Q7 | 0x5BFD..0x5C00C1 | 0x5644B0 | high | `005bffff MOVZX ECX,word ptr [EAX+0x6e]` |

## (f) Hook-site catalogue
All log-only (no consumed exits). "stolen" copied from `.asm`; the check (`chkwin.py` over the whole dump) found no relative CALL/Jcc in any stolen range and NO branch target inside any window (targets exactly at a window start are listed). Nearest sibling self-patch site is >= 0x14D000 away for every row (rule 12 B satisfied). Existing installed hooks nearby are named.
| id | address | length | stolen bytes | convention / live regs | consumed exit? | EAX/stack notes | hot/cold | nearest sibling |
|---|---|---|---|---|---|---|---|---|
| AI1-H1 (existing perf-lab `PerfLabPathEnter`) | 0x55AB30 | 6 | `55 8B EC 83 EC 30` | thiscall, ECX=module, `[esp+4]`=PI, `[esp+8]`=limit us | no | entry; target `0x5C1F18 CALL` at start | hot (per pathing creature/tick) | 0x55AC26 live hook 0xF6 away |
| AI1-H2 (existing `PerfLabPathResult`) | 0x55AC26 | 10 | `89 45 F8 83 3D D4 E0 A7 00 00` | EAX = result 1/2/3; PI `[ebp+8]`, creature id `[PI+0x2C]` | no | abs operand | hot | H1 0xF6 |
| AI1-H3 | 0x55B625 | 13 | `8B 4D 08 C7 81 8C 00 00 00 04 00 00 00` | ebp frame: PI `[ebp+8]`; first stolen insn loads ECX (ECX dead); `[ebp-0x20]` = first straight-test code | no | EAX dead (next `CALL 0x919723`) | cold (once per granted partial) | 0x55AC26 0x9FF |
| AI1-H3b | 0x55B110 | 13 | `8B 45 08 C7 80 8C 00 00 00 04 00 00 00` | PI `[ebp+8]`; EAX loaded by stolen insn | no | EAX dead after | cold | 0x55AC26 0x4EA |
| AI1-H4 | 0x55B3DD | 9 | `8B 15 BC 44 99 00 89 55 E8` | reached only on the 100-call cap path; PI `[ebp+8]`, count `[PI+0x3C]` | no | EDX loaded by stolen insn; EAX dead | cold | 0x55AC26 0x7B7 |
| AI1-H5 | 0x55B1C7 | 6 | `8B 0D BC 44 99 00` | reached only when creature AI level == 0; creature `[ebp-0x84]`... (`0x51C100(PI+0x2C)`) | no | ECX loaded then stored at `0x55B1CD`: handler preserves all regs | cold | 0x55AC26 0x5A1 |
| AI1-H6 | 0x5CBC1E | 7 | `83 BD 1C FF FF FF 05` | creature `[ebp+0xfffffc88]`, new blocked count `[ebp+0xffffff1c]`; stolen CMP sets flags for `JLE 0x5CBC25` (preserve) | no | flags consumed by next insn | cold-warm (blocked steps only) | 0x586F43 0x44CDB |
| AI1-H7 | 0x586B70 | 5 | `55 8B EC 6A FF` | thiscall entry: ECX = bumper, `[esp+4]` = bumpee, `[esp+8/0xC]` = positions | no | targets at start: 3 CALL sites (`0x5CCAAA` in 5CC870, `0x5CDC2C` in AIActionDrive 5CCC50, `0x7763BA` in 775AA0) | cold | 0x586929 (installed `CompanionAiC1`) 0x247 |
| AI1-H8 | 0x586940 | 6 | `55 8B EC 83 EC 58` | thiscall entry: ECX = bumper, `[esp+4]` = bumpee | no | branch targets at start: `0x54DB23` (in `0x54D750`), `0x586BA8`; called per candidate: warm | warm | 0x586929 0x17 |
| AI1-H9a | 0x5C124E | 6 | `8B 15 A4 B4 A1 00` | after payload store: event `[ebp+0xfffffe88]`, creature `[ebp+0xfffffa58]`, door `[ebp-0x10]` | no | EDX loaded by stolen insn | cold | 0x586F43 0x3A30B |
| AI1-H9b | 0x5C5A76 | 5 | `A1 A4 B4 A1 00` | event `[ebp+0xfffffe9c]`, creature `[ebp+0xfffffc04]`, door `[ebp-0x10]` | no | EAX loaded by stolen insn | cold | 0x586F43 0x3EB33 |
| AI1-H9c | 0x5CCBC2 | 6 | `8B 95 30 FF FF FF` | event `[ebp+0xffffff7c]`, creature `[ebp+0xffffff30]`, door `[ebp-0x24]` | no | EDX loaded by stolen insn | cold | 0x586F43 0x45C7F |
| AI1-H10 | 0x5C1D4A | 7 | `C7 40 58 01 00 00 00` | EAX = PI; creature `[ebp+0xfffffa58]`; goal x,y,z `[ebp-0x80]`, `[ebp+0xffffff78]`, `[ebp+0xffffff74]` | no | one write | cold (per new request) | 0x586F43 0x3AE07 |
| AI1-H11 | 0x5C0156 | 7 | `68 D0 64 99 00 6A 00` | creature `[ebp+0xfffffa58]`; node `[ebp+8]` | no | two pushes (balanced by the following call) | cold | 0x586F43 0x39213 |
| R1 FM2 (candidate, R1) | 0x5C2438 | 9 | `8B 95 6C FA FF FF 89 51 70` | ECX = PI | consumed exit `0x5C24C8` | EAX dead | cold | `0x586F43` 0x3B4F5 |
| R1 FM2b | 0x5BC5F4 | 9 | `8B 4D E4 8B 91 24 01 00 00` | targets at start: `JNZ 0x5BC5E2`, `JZ 0x5BC5E8` | no | | cold | 0x586F43 0x356B1 |
| R1 FM5a/b/c | 0x5C8BC2 / 0x5C9F9D / 0x5C3D8B | 6/5/5 | `8B 0D A4 B4 A1 00` / `A1 A4 B4 A1 00` / `A1 A4 B4 A1 00` | event typed already (post path) | no | | cold | far |
No new row overlaps the installed `CompanionAiBumpClear 0x586F43..0x586F4C`, `CompanionAiC1 0x5868B9`, `0x5868F1`, `0x586929`, nor perf-lab `0x55AB30/0x55AC26`.

## (g) Mod relevance
- Q1 partial path / FM2b loop -> FM2, FM2b (fail-fast gate must also consider partial-granted loops; probe H3 splits them).
- AI level 0 -> FAIL (`0x55B1B5`) -> FM2, AIQ (levels set by `SetAILevel`), perf.
- Slice 1000 us shared with RunActions; path search 100-call cap; path-point unbounded iteration -> perf, AIQ, FM3, FM7.
- 6 OnBlocked sites, unthrottled -> FM5 (extend the throttle design to sites `0x5C1240`, `0x5C5A68`, `0x5CCBB4`, `0x5C8BB4`, `0x5C9F8F`; the party OnBlocked script only opens doors).
- No generic stuck detector; `PI+0x268` abort ends the move as "done" (SUCCESS) -> FM2 (loop re-enters), Mod 8 kite moves (a kite move blocked 6 times ends silently), enemy-E7 (`hold_enemies`), E4.
- Bump: following companion queue cleared by `BumpFriends` -> FM1 (already a hook; bumpee with action 1/0x11 is untouched).
- `k_def_pathfail01` per blocked step (hostile blocker) -> enemy-AI, FM5, perf.
- `+0x558` LOF block position -> Mod 8 (kite), A1, a1_retarget_strict (AI2).
- Force-move deadline (`ActionForceMove*`) -> none (cutscene only), noted for T1.
- PFLK: PI Shutdown does not free 4 blocks (+0x18C, +0x1B4, +0x1D8, +0x200) -> PFLK (KB scale, creature destroyed mid-search).
- Creature `+0x11AC` (party flag) is read by `AIActionMoveToPoint` guard (`005bfc56`) and `GetIsCreatureBumpable` -> party-flag.
- Node param map (0x74 B action node) -> AIQ (`AddMoveToPoint` flag packing `0x5644B0`).
