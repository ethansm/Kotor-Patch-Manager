# R1 engine follow-ups (2026-10-02)

Static only. Steam `swkotor2.exe` 6A522E71 (dump `ghidra_dumps/steam_6a522e71/decomp`). Constants come from `backups/vanilla_clean/swkotor2.exe` (sha 6a522e71…). Nothing was installed or launched, Ghidra was not run, and no live log was read. Every calling convention, stack depth and EAX fact below comes from the `.asm`.

"verified" means the asm lines are quoted here. "plausible" says what would settle it.

Constants (vanilla exe):
- `[0x9944B4]=1`, `[0x9944B8]=2`, `[0x9944BC]=3`, `[0x9944C0]=4`, `[0x9944C4]=1000`;
- `[0x98BA40]`=50.0 (double), `[0x987490]`=2.0 (double), `[0x986B60]`=0.3f, `[0x9876FC]`=0.1f, `[0x986B38]`=0.0;
- `0x996450` = "The Path find has Failed... Why?".

Scratch: `/tmp/claude-1000/r1_scratch/` holds `check_windows_r1.py` (a copy plus the R1 windows plus a sibling-site distance check), its output `check_windows_r1_output.txt`, and `verify_r1_output.txt`.

## Summary table

| item | verdict | severity (party AI) | proposed action | INI flag | trace marker |
|---|---|---|---|---|---|
| FM2 pathfind fail = success + wait, no OnBlocked | **verified** (code, every step). Loop: the 0x11 re-queue is verified; that the pathfinder returns 3 for an unreachable target is **plausible** | Medium | New hook CA-FM2 `0x5C2438`, consumed exit `0x5C24C8`: log-only first, then fail-fast (delete the move's action group, skip the 2-4 s wait, cap `+0x570`). Log-only probe CA-FM2b `0x5BC5F4`. **Not OnBlocked** (see FM2) | `fm2` (0 log / 1 fail-fast), `fm2_fail_limit=2`, `fm2_window_ms=12000`, `fm2_rethink_ms=500`, `fm2_party_only=1` | `FM2 FAIL`, `FM2b REAPPROACH` |
| A1 vs `+0x524` | **verified**: on the normal path combat does not lapse before the E2 fallback | Low | No new window. A1 handler: log `combat/c524/f570`, an A1→E2 follow-up counter, optional guard `a1_keep_combat_ms` | `a1_keep_combat_ms=0` | `A1 … c524=`, `A1 FOLLOWUP`, `A1 ORPHAN` |
| A1 retarget metric mismatch (new, T1-related) | **verified** | Low-Med (live `a1_retarget=1`) | A1 handler: reject a retarget whose 2D centre distance fails the attack's own range test | `a1_retarget_strict` (0 log, 1 reject) | `A1 … -> retarget-rejected` |
| FM3 scheduler wrap sentinel | **verified** (code). Reach is plausible: combat start/end of the first object in a level | Low (party is level 4 and runs first; indirect frame hitch). Amplifies candidate AIQ (O(n²) pre-pass) | 3 **cold** hooks: probe now; the fix (end the level, fix the cursor) is gated because it changes **enemy** scheduling | `fm3` (0 probe, 1 fix) | `FM3 LOST`, `FM3 REMOVE`, `FM3 summary` |
| FM5 unthrottled ON_PATH_BLOCKED (0x1F) | **verified** (3 post sites, no time gate) | Low (CPU: OnBlocked → `k_ai_master` per call; with E10 it only opens doors) | 3 cold hooks on the post path: log-only, then throttle by retyping the event to 0 (freed as a no-op) | `fm5` (0/1), `fm5_throttle_ms=1000` | `FM5 BLOCKED` |
| FM6 follow trail on non-UI leader change | **plausible** (partly verified) | Low | Optional log-only probe `0x7E6546` | `fm6_probe=0` | `FM6 LEADER` |
| FM7 AI-level churn 4↔0 | **verified** (code), reach plausible (rare) | Low | No hook. The FM3 RemoveObject hook logs the transitions | – | `FM3 REMOVE` |
| T1 nearest-enemy prune | **verified** | Low (alone) | No hook. Covered by `a1_retarget_strict` | – | – |
| R1 perception scan abort | **verified** code path, reach plausible | Low | No hook (the only probe site is hot) | – | – |
| P2 companion perception 4 s | **verified** | Low-Med (visible late reactions) | New hook CA-P2 `0x56BE79` (party only, hot path, minimal handler) | `p2=0`, `p2_interval_ms=500`, `p2_controlled_ms=200` | `P2 summary` |

All 10 candidate windows: `ALL OK`, 0 `[WARN]`, no overlap with the 149 live hooks or the 33 table windows, more than 12 bytes from every sibling self-patch site, and byte-identical in both executables (`ALL MATCH`).

## FM2 (evidence, design, risks)

### Evidence, `AIActionMoveToPoint 0x5BFC00` (creature = `[ebp-0x5A8]`, node = `[ebp+8]`, `RET 4`)

Path request and result:
```
005c1f18  e8138cf9ff            CALL 0x0055ab30                ; PlotPath
005c1f1d  8945a8                MOV dword ptr [EBP + -0x58],EAX ; result
005c1f45  3b05b8449900          CMP EAX,dword ptr [0x009944b8]  ; 2 = success
005c1f4b  0f8570020000          JNZ 0x005c21c1
005c204d  8b0db4449900          MOV ECX,dword ptr [0x009944b4]  ; success path: result := 1 (in progress), then walk
005c21c4  3b05bc449900          CMP EAX,dword ptr [0x009944bc]  ; 3 = fail
005c21ca  0f85f8020000          JNZ 0x005c24c8                  ; 1 (pending) -> return 1 (node re-queued)
```

Failure branch:
```
005c21e2  6850649900            PUSH 0x996450                  ; "The Path find has Failed... Why?"
005c21ef  e87c82e6ff            CALL 0x0042a470                ; debug print
005c2215  c782b011000002000000  MOV dword ptr [EDX + 0x11b0],0x2 ; walk state 2 (when +0x220)
005c224d  6810270000            PUSH 0x2710 ... 005c2263 CALL EDX ; vtbl+0x7C(0x2710) animation
005c2278  e8e3d4ffff            CALL 0x005bf760                ; TerminateClientSidePath (if [ebp-0x6C]==1)
005c2441  e845c73500            CALL 0x0091eb8b                ; rand()
005c2447  b964000000            MOV ECX,0x64 ; 005c244c IDIV ECX     ; %100
005c245a  dc3540ba9800          FDIV double ptr [0x0098ba40]   ; /50.0
005c2460  dc0590749800          FADD double ptr [0x00987490]   ; +2.0  -> 2.00..3.98 s
005c2479  0fb7426c              MOVZX EAX,word ptr [EDX + 0x6c]; group id of the move node
005c2484  e83722faff            CALL 0x005646c0                ; AddPathfindingWaitActionToFront(group, secs)
005c2489  833db8e0a70000        CMP dword ptr [0x00a7e0b8],0x0 ; never written in the dump (BSS) -> 0
005c249a  837d8400              CMP dword ptr [EBP + -0x7c],0x0; only written at 005bfd2d (=0)
005c249e  7628                  JBE 0x005c24c8
005c24c8  e9d8000000            JMP 0x005c25a5
005c25a5  8b45a8                MOV EAX,dword ptr [EBP + -0x58]; = 3 (no writer between 005c1f1d and here on this path)
005c25b6  c20400                RET 0x4
```

`AddPathfindingWaitActionToFront 0x5646C0` (thiscall; `[ebp+8]` word group, `[ebp+0xC]` float secs; `RET 8`):
```
005646c9  d905606b9800   FLD float ptr [0x00986b60]   ; default 0.3 s
005646d5  dc1d386b9800   FCOMP double ptr [0x00986b38]; arg == 0.0 ? keep the default : use the arg
005646fd  e8fe7efbff     CALL 0x0051c600              ; GetPlayerByControlledObjectId(self id)
0056470b  d905fc769800   FLD float ptr [0x009876fc]   ; player-controlled -> 0.1 s
0056474f  6a1e           PUSH 0x1e
00564754  e847c5fdff     CALL 0x00540ca0              ; AddActionToFront(0x1E wait, group, ...)
```
Action 0x1E is `AIActionWait 0x5D9AA0` (RunActions table `0x5D7038`, case `0x5D656E`). It returns 1 while elapsed < secs, then 2 (`005d9bd2 CMP EAX,[EBP-0xC] / 005d9bd7 MOV EAX,[0x9944b4] / 005d9bde MOV EAX,[0x9944b8]`).

RunActions treats 3 like 2. Only 4 and 1 are special: `005d6a63 CMP EDX,[0x009944c0]` → `0x54AA30`; `005d6a88 CMP ECX,[0x009944b4]` → re-queue. The node is already popped: `005d5b86 CALL 0x007f2a90` → `0x73A200` unlinks the head and frees the list cell, and the data stays in `[ebp-0x40]`.

`PlotPath 0x55AB30`:
- returns 3 when pathinfo is null (`0055ab3f MOV EAX,[0x9944bc]`);
- returns 2 when a path is already held (`0055ab6a`);
- otherwise returns the driver result: `0055ac21 CALL 0x0055a900 / 0055ac26 MOV [EBP-0x8],EAX / 0055ac75 MOV EAX,[EBP-0x8]`.

`0x55AC26` is a live perf-lab hook site (the result store), so perf-lab may already count the result codes.

**Step verdicts:**
1. Failure reported as success: **verified.** The move returns 3, which RunActions handles exactly like 2.
2. 2-4 s wait: **verified.** It is 2.00-3.98 s, or 0.1 s for a player-controlled creature.
3. OnBlocked never fired on this branch: **verified.** Between `0x5C21D0` and `0x5C25B6` the only callees are `0x42A470`, vtbl+0x7C, `0x5BF760`, `0x5453C0`, rand and `0x5646C0`. `0x5BF760` contains no `CALL 0x51E170`, nor does `0x543F50` (and the `0x543F50` branch is dead anyway).

**Corrections to the review:**
- (a) OnBlocked **is** posted during the walk itself, from `WalkUpdateLocation 0x5C3460`: `005c3d7d MOV ECX,0x1f … 005c3dc5 CALL 0x0051e170`. Only the pathfind-failure branch is silent.
- (b) The "door branch" of MoveToPoint does **not** post OnBlocked. It posts script event **0x1E** with object = the door and caller = the creature:
  ```
  005c0732 MOV EAX,0x1e
  005c0774 PUSH [EBP-0x10]
  005c0781 PUSH creature id
  005c0793 CALL 0x0051e170
  ```
  The creature EventHandler handles only events 1, 2, 7, 0xB, 0x1B and 0x1F (byte table `0x571010`, jump table `0x570FF4`). 0x1F → case 5 → `0x5706A4`, which runs slot 12 (OnBlocked) unless the creature is player-controlled (`005706cf CALL 0x51c600 / 005706e1 JNZ`).

**Why OnBlocked is the wrong tool.** Party OnBlocked is `k_hen_blocked01` → `k_ai_master` 2009, and that only opens a door: `GetBlockingDoor()` → `DoDoorAction(OPEN)` (`k_ai_master.nss:828-851`, E10 removed the bash). The event handler stores the event's caller as the blocker (`005706ec MOV [ECX+0x37C],EDX`). A pathfind failure has no door caller, so the script would do nothing.

**The loop.** For approach stances other than 11/12, the attack queues `[move, 0x13, 0x11, 0xC]`. `AIActionCheckMoveToObjectRadius 0x5BC1D0` re-queues itself and a fresh move whenever the target is not within radius:
```
005bc64a  6a11          PUSH 0x11
005bc652  e84946f8ff    CALL 0x00540ca0      ; re-add 0x11 (same group [node+0x6C])
005bc6d1  e8da7dfaff    CALL 0x005644b0      ; AddMoveToPointAction (front)
005bc6e3  51 / 005bc6ea e8d17ffaff  CALL 0x005646c0 ; wait, arg 0.0 -> 0.3 s (front)
005bc6ef  a1b8449900    MOV EAX,[0x009944b8] ; return 2
```
The steady state while the target stays unreachable is: wait 0.3 s, move, path fail, wait 2-4 s, then 0x11 again. The attack (0xC) is never reached. Moves and 0x11 do not reset `+0x570`, so in combat the slot-6 fallback re-runs the AI every ≤3 s. It normally re-picks the same nearest enemy.

**Plausible part.** That `0x55A900` returns 3 for an unreachable target, rather than 2 with a partial path to the nearest reachable point. If it returns 2, the loop instead becomes "succeed instantly, 0x11 not in radius, wait 0.3 s", and CA-FM2b sees it. **Settle:** the rates of `FM2 FAIL` vs `FM2b REAPPROACH` for the same creature and target, or perf-lab's `0x55AC26` result counts.

### Design

**CA-FM2** (window `0x5C2438`, consumed exit `0x5C24C8`):
- It sits after the walk-state, animation and client-path work and before the rand/wait.
- The exit is the very block vanilla reaches next (`0x5C249A` → `JBE 0x5C24C8`). So `return 1` = vanilla minus `rand()` and the wait, and the function still returns `[ebp-0x58]` = 3.

Handler pseudocode:
```
extern "C" int __cdecl CompanionAiFm2Fail(int ebp)
  cr=[ebp-0x5A8]; node=[ebp+8]; tgt=[ebp-0x10]; dest=([ebp-0x40],[ebp-0x3C],[ebp-0x38]); group=u16[node+0x6C]
  party=[cr+0x11AC]; rec=fm2Track(id)                      // per creature: key (tgt, or dest within 1 m), n, first, last
  if same key && now-rec.last < fm2_window_ms: ++rec.n else rec = {key, n=1, first=now}
  rec.last = now
  act = fm2 && n >= fm2_fail_limit && (party || !fm2_party_only) && !isControlled(id)   // the controlled PC keeps vanilla 0.1 s
  log (throttled per creature: first 20, then on n change / 2 s)
  if (!act) return 0                                       // vanilla: stolen bytes, rand, 2-4 s wait
  thiscall 0x53F6A0(cr, group)                             // delete the rest of this action group (0x13/0x11/0xC or the script's chain)
  if [cr+0x520]==1 && fm2_rethink_ms>0 && [cr+0x570] > fm2_rethink_ms: [cr+0x570] = fm2_rethink_ms
  return 1                                                 // -> 0x5C24C8: return 3, no wait queued
```

`0x53F6A0` is thiscall (obj; int groupId), `RET 4`; it does `ADD ECX,0x100` internally. Its use is copied from `ClearHostileActionsVersus`: `005868ac MOVZX EAX,[EBP-4] / 005868b0 PUSH EAX / 005868b1 MOV ECX,[EBP-0x2C] / 005868b4 CALL 0x0053f6a0`.

It cannot touch the running node, which RunActions has already unlinked (above).

**CA-FM2b** (`0x5BC5F4`, plain, log-only):
- It counts the 0x11 re-approaches per creature and target: creature `[ebp-0xA4]`, target `[ebp-0x28]` (`node+0x38`, `005bc25b..`).
- It runs at most once per ≥0.3 s per approaching creature.

**Risks:**
1. With fail-fast the companion re-thinks about every `fm2_rethink_ms` instead of waiting 2-4 s. If `k_ai_master` keeps picking the same unreachable enemy, it still idles, just with faster re-thinks. The real fix is target exclusion (open question 2).
2. A script move with an INVALID target is keyed by destination. Player-issued moves for the controlled character are excluded.
3. Deleting a group removes a partly executed chain. This is the same thing `ClearHostileActionsVersus` does.
4. Skipping `rand()` changes the RNG sequence. Harmless.

## A1 vs +0x524 (evidence, answer, fix option, how ai_trace distinguishes outcomes)

**Who touches `+0x524`.** In the whole dump, only 4 files reference `+ 0x524]`. One of them, `0x665F50`, is the VM command table (a function pointer store). The real ones:
```
0057a64f  c78024050000401f0000  MOV dword ptr [EAX + 0x524],0x1f40   ; SetCombatState(state != 0): 8000 ms
0057a67c  c7822405000000000000  MOV dword ptr [EDX + 0x524],0x0      ; SetCombatState(0)
0056264b  c7822405000000000000  MOV dword ptr [EDX + 0x524],0x0      ; ctor
00569945  8b8a24050000          MOV ECX,dword ptr [EDX + 0x524]      ; AIUpdate: -= dt (+0xD0)
0056994b  2b88d0000000          SUB ECX,dword ptr [EAX + 0xd0]
00569957  898a24050000          MOV dword ptr [EDX + 0x524],ECX
```

**AIUpdate `0x569320`, in tick order:**
```
005698b4  83bdf4feffff01  CMP [EBP-0x10C],0x1        ; +0x520 in combat
005698f6  83bdf0feffff00  CMP [EBP-0x110],0x0        ; +0x524
005698fd  7f2b            JG 0x0056992a
00569923  e8380c0100      CALL 0x0057a560            ; <=0: SetCombatState(0, +0x11E8, 0) -> leave combat
00569930  83b92805000000  CMP dword ptr [ECX+0x528],0x0 ; +0x528 != 0 freezes the timer (only SetFakeCombatState passes 1: 00699f22)
005699ce  83bde8feffff01  CMP [EBP-0x118],0x1        ; fallback block needs +0x520==1 (re-read after the exit above)
005699d5  0f85c5000000    JNZ 0x00569aa0             ; also skips the +0x570 decrement at 0x569a7c
```
So the combat exit runs before the fallback test in the same tick, and `+0x570` decrements only while in combat.

**Who refreshes it.** The 17 callers of `SetCombatState 0x57A560` do not include `AIActionPhysicalAttacks`, `StartCombatRound`, `EndAttackAction` or `InitializeNewAttackTarget`. `EndAttackAction` calls only `0x57D370`, vtbl, `0x5B7AE0` and `0x5915F0` (`006dab40/55/5e/72`). So neither the vanilla spin nor the A1 end touches `+0x524`.

The refresh for a companion is `AddAttackActions` (every script `ActionAttack`), via `0x57AC30`:
```
00568915  e816230100   CALL 0x0057ac30               ; this = attacker ([ebp-0xA8] = ECX at 0x568568), arg = own id
0057acc9  3b4db8       CMP ECX,[EBP-0x48]            ; arg == this id -> no skip
0057ad02  e859f8ffff   CALL 0x0057a560               ; SetCombatState(1,1,0)  (or)
0057ad15  e846f8ffff   CALL 0x0057a560               ; SetCombatState(1,2,0)
```
`0x57AC30` then also sets combat on nearby creatures hostile to the arg (`0057aef5 CALL 0x5e13c0 / CMP EAX,0xA / JG skip / 0057af42, 0057af5e CALL 0x57a560`). That is how enemies' attacks keep the party in combat. The `EventHandler` damage/attack cases (`0056f98d`, `0056f9d4`, `00570d8d`, `00570dd1`) also call it.

**Answer: no lapse on the normal path.**
1. The DCR's `ActionAttack` refreshes `+0x524 = 8000` at T0.
2. The queued 0xC action reaches the approach branch on its first pass at T ≈ T0 + 1 frame.
3. The prologue `006d740f MOV [EAX+0x570],0xBB8` runs in that same pass, before A1.
4. The fallback fires at T + 3000, while `+0x524` ≈ 8000 − 3000 − (T − T0) > 0.

The condition is `T − T0 < 8000 − a1_rethink_ms` (5 s with the shipped 3000). While the companion shoots, every round's end-of-round DCR re-issues `ActionAttack`, so `T − T0` ≤ about one round (3 s). After E2 fires, a DCR that attacks again refreshes `+0x524`. A DCR that does nothing combat-related lets combat lapse normally.

**When it can fail.** Only if the last `SetCombatState(≠0)` for that companion was ≥5 s before the A1 end. Example: the attack action was queued by something that skipped `0x568915` (the early exits `0x568671` GUI combat-round path, `0x5686D6`, `0x568709`, `0x5687A7`), no enemy within range kept attacking, and nobody hit the companion.

**Fix option (cheap, no new window), `a1_keep_combat_ms`, default 0.** In `CompanionAiA1Gate` on `end`:
```
if [cr+0x520]==1 && [cr+0x524] < rethink+500: [cr+0x524] = rethink+500
```
This guarantees exactly one fallback, and combat still lapses 500 ms later if nothing re-engages. Only AIUpdate reads `+0x524`. Prefer this over a shorter `a1_rethink_ms`, which only widens the margin.

**New finding, T1 interplay with the live `a1_retarget=1`.**
- `0x57C990` accepts a candidate when its 3D distance minus both radii is below the range:
  ```
  0057cd51 CALL 0x5142f0        ; |v|
  0057cd80 FSUB [pathinfo+0x8]  ; self radius
  0057cdc0 FSUB [pathinfo+0x8]  ; target radius
  0057cdcf FCOMPP / 0057cdd6 JNZ
  ```
- The attack tests the 2D **centre** distance² against `(max+0.1)²`:
  ```
  006d7e4a FLDZ / 006d7e4c FSTP [EBP-0x74]   ; z = 0
  006d7e57 CALL 0x0044b8a0                   ; dot
  006d7e5f FSTP [EBP-0xBC]
  ```
- `InitializeNewAttackTarget` sets passive: `006dacdb MOV dword ptr [ECX+0x52c],0x1`.
- So a target whose centre is within (max+0.1, max+r1+r2) is accepted by the retarget, and the next pass ends it at the approach branch's first test:
  ```
  006d83ea MOV EDX,[ECX+0x52C] / 006d83fd JZ / 006d83ff PUSH 1 / 006d8407 CALL 0x6dab20 / 006d840c MOV EAX,[0x9944bc]
  ```
- That is an end with no `A1` line. The fallback timing is the same as above.
- Fix in the A1 handler, `a1_retarget_strict`: compute the 2D centre dist² to the candidate (both `+0x94` positions) and require `≤ (max+0.1)²`, else treat the result as INVALID (end). With 0, only log `retarget-rejected(would)`.

**How `ai_trace.txt` tells the outcomes apart:**
- **Good:** `t=T A1 cr=X … -> end new=7f000000`, then `t≈T+a1_rethink_ms E2 FALLBACK cr=X … combat=1 … lastA1≈3000ms`.
- **Lapse:** `A1 cr=X … -> end` and no `E2 FALLBACK cr=X` within `rethink+1000` ms. X then shows nothing until perception or heartbeat re-engages it.
  - Caveat: E2 lines are throttled per creature (first 20, then 1 per 10 s, `g_subj.allow(SkE2,…,20,10000)`). So add, with no new hook:
  - (a) `c524=%d f570=%d combat=%d` to the A1 line. `c524 < f570` predicts a lapse.
  - (b) A per-creature `pendingA1`, set on end and cleared in E2. E2 then emits `eline("t=%lu A1 FOLLOWUP cr=%08x after=%ldms")`, and the summary emits `A1 ORPHAN cr=%08x` when it is still pending after `rethink+1000` ms.
- **`E1 SPIN cr=X`** for a party member with stance 11/12/16 while `a1=1` means A1 is not catching the spin (controlled-id or stance mismatch). It should never appear.
- **Retarget then silent end:** `A1 … -> retarget new=Y` with no later `A1`/`E1` activity and an `E2` ~3 s later.

## FM3 / FM5 / FM6 / FM7 / T1 / R1 / P2

### FM3: the scheduler wrap sentinel (verified code; Low for party)

`CServerAIMaster::UpdateState 0x51D790`, per level `[ebp-0x34]` (4..0):
```
0051d834  c745c801000000  MOV [EBP-0x38],0x1        ; "first" (once per level)
0051d83b  c745c000000000  MOV [EBP-0x40],0x0        ; done
0051d842  c745bc00000000  MOV [EBP-0x44],0x0        ; inner-loop head: the pre-pass over ALL objects (AIQ), then events
0051dbd3  e8e8f5ffff      CALL 0x0051d1c0           ; NextObject -> [ebp-0x28]
0051dbdb  837dc801        CMP [EBP-0x38],0x1 / 0051dbdf JNZ 0x0051dbe7
0051dbe4  894dc4          MOV [EBP-0x3C],ECX        ; sentinel = first object POINTER
0051dc5c  837dd800        CMP [EBP-0x28],0x0 / 0051dc60 JZ 0x0051dc7f
0051dc65  3b45c4          CMP EAX,[EBP-0x3C] / 0051dc68 JNZ 0x0051dc70 / 0051dc6e JNZ 0x0051dc7f
0051dc7b  ffd0            CALL EAX                  ; vtbl+0x70 AIUpdate
0051dc7f  c745c001000000  MOV [EBP-0x40],0x1        ; done only via NULL or obj==sentinel
0051dd1c  0f8220fbffff    JC 0x0051d842 / 0051dd2d 0f860ffbffff JBE 0x0051d842   ; else loop while elapsed <= slice
```

`NextObject 0x51D1C0`:
```
0051d1dc MOV EDX,[ECX+0xC] / 0051d1df ADD EDX,1 / 0051d1e5 MOV [EAX+0xC],EDX ; cursor++
0051d1f9 MOV [ECX+0xC],0                                                      ; wrap
0051d20b MOV EAX,[EDX+EAX*4]                                                  ; id
```
An unresolved id returns 0, which also ends the level.

Removal (`RemoveObject 0x51E0C0` → `0x51D170`):
```
0051d190 CALL 0x004142e0   ; index
0051d1a5 CALL 0x0049b840   ; DelIndex
```
`0x49B840` touches only the count (`0049b854 MOV [EDX+0x4],ECX`) and never `+0xC`. So removing index ≤ cursor skips one object, and removing the sentinel means `obj==sentinel` never matches again. The level then re-runs AIUpdates until its slice (60%, `0051d7e7 PUSH 0x3c … 0051d7fc CALL 0x91d220`) is spent, and **every extra iteration re-runs the O(n) pre-pass** (DB candidate AIQ).

Self level changes inside AIUpdate:
- `005694d6 CALL 0x51e120` (party/IsPC → 4);
- `0056a0b5` (combat → 2);
- `0056a0fe` / `0056a111` (2 → 0/1);
- `0056c01c` (perception: `area+0x12C==0` → 0).

Enemies trigger it at every combat start/end. Party members only with FM7, or right after FM4's save bounce.

**Hot loop?** Yes. The validated probe window `0x51DBE7` is executed on every dispatch iteration (both from `0051dbdf JNZ` and the fall-through), once per AIUpdate per object per frame. That is the reason it was dropped. **Replacement: cold sites** (hook designs below):
- `0x51DBE1` runs only when `[ebp-0x38]==1`, once per level per tick;
- `0x51DD33` is level end, once per level per tick;
- `0x51E0C9` (RemoveObject) runs only on level changes.

### FM5: unthrottled ON_PATH_BLOCKED (verified; Low)

Follow driver `0x5C6340`: switch on `FollowInfo+4` + 1 (`005c6abb CMP ..,0xb / 005c6ace JMP [ECX*4+0x5c7334]`). Case 3 = sub-state 2 calls `0x5C87E0` (`005c6b6b`), and case 4 = sub-state 3 calls `0x5C8C20` (`005c6b91`).

In `0x5C87E0`, after the planner `005c8a99 CALL 0x00550980`:
- result 1 → success;
- −3 with no client blocker → `005c8b42 MOV [EDX+0x4],0xa` (state 10, no event);
- anything else falls through to a fresh event every call: `005c8b5a CALL 0x919723 (new 0x34)`, `005c8bb4 MOV EDX,0x1f`, `005c8bfe CALL 0x51e170`, with no timestamp test and no state change.

The same pattern is at `005c9f8f … 005c9fd7` (`0x5C8C20`) and at `005c3d7d … 005c3dc5` (`WalkUpdateLocation`).

The events are freed by `EventHandler` at `00570f81 CALL 0x5e1af0 / 00570f97 CALL 0x9196fd` (also on the default path). With E10 the party OnBlocked only opens doors, so the cost is CPU (a `k_ai_master` run per call), not behaviour.

**The review's throttle (consumed skip to `0x5C8C03`) would leak the already allocated 0x34-byte event.** The design below retypes it instead.

### FM6: follow state on non-UI leader change (plausible; partly verified)

- `SetLeaderBySlot 0x7E5DF0` writes per-slot fields (stride 0x88) only at `007e605f/007e60dc MOV [EAX+EDX+0x24],ECX` and `007e6097/007e60f2 MOV [EAX+EDX+0xa4],ECX` (slot ids). It also sets `007e6546 MOV [EAX+0x94],1`, `007e6556 [ECX+0x11c]`, `007e6566 [EDX+0x1a4]` (a flag in each of 3 slots, +0x70 per slot).
- It never calls `ResetFollowTrail 0x7E8350`, which writes `slot+0x04=-1` at `007e837b` and the positions.
- The callers of `0x7E8350` are `0x797F30` (UI), `0x5F9A20`, `0x5BE950` and `0x5BE1F0`. The callers of `0x7E5DF0` include `0x696250` (script), `0x79ED60`, `0x89FBD0`, `0x775860` and `0x7E5730`.
- Unknown: the consumer of that per-slot flag. If it lazily resets the trail, FM6 is refuted. `ClearAllActions` also resets the creature's slot (`0054137d MOV [EAX+EDX+0x28],-1`).
- **Settle:** the log-only probe at `0x7E6546` (slot ids + `slot+0x04` before, and again at the next follow tick).

### FM7: AI-level churn 4↔0 (verified code; reach rare; Low)

```
0056948e CMP [stats+0x6C],1 / 005694a9 CMP [+0x11AC],0 / 005694c1 CMP [+0x78],4 / 005694d6 CALL 0x51e120   ; -> 4
0056bfc2 MOV EDX,[area+0x12C] / 0056bfce CMP ..,0 / 0056bfd5 JNZ / 0056bfe6 CMP [+0x78],0 / 0056c01c CALL 0x51e120 ; -> 0
```
The drop sits inside perception block 3, which runs every 4 s for companions. So the churn is 4→0→4 per ~4 s, not per tick, and only when `area+0x12C==0`. Its meaning (players in area) is not verified, and it is rare in play. Both moves happen inside the object's own AIUpdate, so FM7 is an FM3 trigger. No dedicated hook: the FM3 RemoveObject hook logs `old→new` level per object.

### T1: GetNearestVisibleEnemyInRange prune (verified; Low)

The metric is `d3D − r_self − r_tgt` (`0057cd51`, `0057cd80`, `0057cdc0`; accepted if `best > d`, `0057cdcf FCOMPP / 0057cdd6 TEST AH,0x41 / JNZ`).

The left scan stops when the raw X gap exceeds `best`:
```
0057ceb6 FLD [self+0x94] / 0057cec2 FSUB [cand+0x94] / 0057cec8 FLD [EBP-0x1C] / 0057cecb FCOMPP / 0057ced2 JP / 0057ced4 MOV [EBP-0x40],0
```
The right scan does the same at `0057d337`. A nearer candidate with `dx − r1 − r2 < best < dx` is skipped. The error is at most `r1+r2` (~1-2 m). The party impact is through A1/E-H2 retargets; see the stricter retarget check above, which matters more than the prune.

### R1: UpdatePerception aborts on an unresolvable id (verified code path; reach plausible; Low)

```
00522b6c  CALL 0x0053dfb0        ; resolve id at the bisection midpoint
00522b7b  CMP EAX,ECX / 00522b7d JZ 0x00522b83 / 00522b7f XOR EAX,EAX / 00522b81 JMP 0x00522bde   ; fail -> return 0
005d1287  CALL 0x00522af0 / 005d128c TEST EAX,EAX / 005d128e JNZ / 005d1290 JMP 0x005d13ec       ; skip the whole new-object scan
```
It needs a stale id on the bisection path of `area+0x194`, which is not shown to happen. No hook:
- the only probe sites are the `0x522AF0` epilogue (`0x522BDE`, called by every `UpdatePerception`, hot under P1), or
- `0x5D1290` (the 5-byte `JMP`). A skip-only consumed window there would change `UpdatePerception`'s return EAX from 0 to 1.

**Settle cheaply:** walk `area+0x194/+0x198` from the FM3 level-end probe once per area load and count unresolvable ids.

### P2: companion perception cadence (verified; Low-Med)

Block 3 of `0x56B8A0`:
```
0056be55 MOV EDX,[c+0x10e8]                    ; IsPC
0056be70 CMP [EBP-0x18],0xc8 / 0056be77 JNC   ; IsPC & mode 1: 200 ms
0056be8b CMP [..],0 / 0056be92 JNZ 0x56c021   ; IsPC below 200 -> skip
0056be98 CMP [EBP+0x8],1 / 0056bea2 CMP [EBP-0x18],0xfa0 / 0056bea9 JC 0x56c021   ; others: 4000 ms
0056bf71 PUSH 0 / 0056bf79 CALL 0x5d0f30      ; party: UpdatePerception(0)
0056bf96 MOV [EAX+0x3c8],ECX / 0056bfa5 MOV [EDX+0x3cc],EAX   ; stamp
```
`[ebp-0x18]` (elapsed) is read only at `0x56BE70` and `0x56BEA2`; its only writer is via `0056bdf8 LEA EAX,[EBP-0x18]` → `0x51AF60`. That makes the window `0x56BE79` safe to rewrite it.

**Hot:** reached for every non-IsPC creature with `+0x38C≠0` on every tick. The handler must test party first and do nothing else before that.

## Hook designs

Common framework (wrapper_x86.cpp, design doc §0):
- the stolen bytes run after the handler;
- plain detours restore every register, EAX included;
- consumed hooks exclude EAX, so EAX = the handler result on both exits.

Every new window starts with `saferead::beginScope()` (except the hot ones) and `tick(now)`, like the existing handlers. All are `type="detour"`, `skip_original_bytes=false`.

| id | addr / len | stolen bytes | consumed exit | params | hot? |
|---|---|---|---|---|---|
| CA-FM2 | `0x005C2438` / 9 | `8B 95 6C FA FF FF 89 51 70` (MOV EDX,[EBP-0x594]; MOV [ECX+0x70],EDX) | `0x005C24C8` (exclude eax) | ebp | no (fail path) |
| CA-FM2b | `0x005BC5F4` / 9 | `8B 4D E4 8B 91 24 01 00 00` | none | ebp | no |
| CA-FM3s | `0x0051DBE1` / 6 | `8B 4D D8 89 4D C4` | none | ebp | no (once per level per tick) |
| CA-FM3e | `0x0051DD33` / 6 | `8B 4D D0 2B 4D F0` | none | ebp | no (once per level per tick) |
| CA-FM3r | `0x0051E0C9` / 6 | `8B 45 08 8B 48 78` | none | ebp | no (level changes) |
| CA-FM5a | `0x005C8BC2` / 6 | `8B 0D A4 B4 A1 00` | none | ebp | no (post path) |
| CA-FM5b | `0x005C9F9D` / 5 | `A1 A4 B4 A1 00` | none | ebp | no |
| CA-FM5c | `0x005C3D8B` / 5 | `A1 A4 B4 A1 00` | none | ebp | no |
| CA-P2 | `0x0056BE79` / 6 | `8B 85 2C FF FF FF` | none | ebp | **yes** |
| CA-FM6 (opt) | `0x007E6546` / 10 | `C7 80 94 00 00 00 01 00 00 00` | none | ebp | no |

**EAX/stack hand-checks:**
- **CA-FM2 (consumed):**
  - The stolen bytes do not touch EAX. ECX was loaded at `005c2432 MOV ECX,[EBP-0x598]`, before the window, and is restored before the stolen store.
  - **EAX at the resume `0x5C2441`** is dead: `CALL rand` clobbers it, and the checker found it dead on all paths.
  - **EAX at the exit `0x5C24C8`** is dead: `JMP 0x5C25A5 → MOV EAX,[EBP-0x58]`.
  - **Stack:** nothing is pending at the window end. The last pushes before it were consumed by `005c2401 CALL 0x5453c0` (no args); the next push is `005c2472 PUSH ECX`, after rand. The exit path is `MOV ECX,[EBP-0xC]; MOV FS:[0],ECX; POP ECX; MOV ESP,EBP; POP EBP; RET 4`. The POP reads the same slot as vanilla because ESP equals vanilla's at `0x5C2441`, and ESP is then re-derived from EBP ("reset", checker).
  - **x87:** the window is before the `FILD`/`FDIV` sequence and the wrapper does FXSAVE.
- **All others are plain detours:** registers are restored, the stolen bytes re-run with no relative operands (`A1`/`8B 0D` are absolute moffs), no branch lands inside any window (checker), and ESP is unchanged.

Handler pseudocode (beyond FM2 above):
```
// FM2b  (log only)
extern "C" void __cdecl CompanionAiFm2Reapproach(int ebp)
  cr=[ebp-0xA4]; tgt=[ebp-0x28]; if (!party(cr) && !log_all) return; ++track(cr,tgt).reapproach; log throttled
  tline "t=%lu FM2b REAPPROACH cr=%08x tgt=%08x n=%u/10s fails=%u"

// FM3s  (sentinel set; cold)
extern "C" void __cdecl CompanionAiFm3Sentinel(int ebp)
  g3 = {active=1, ebp, level=[ebp-0x34], sentinel=[ebp-0x28], lostByRemove=0}

// FM3e  (level end; cold)
extern "C" void __cdecl CompanionAiFm3LevelEnd(int ebp)
  if (!g3.active || [ebp-0x34]!=g3.level) { g3.active=0; return }
  s=g3.sentinel; lost = s && (g3.lostByRemove || rd([s+0x78]) != g3.level)
  elapsed=u64([ebp-0x30],[ebp-0x2C])-u64([ebp-0x10],[ebp-0xC]); slice=u64([ebp-0x20],[ebp-0x1C])
  if lost: ++st.fm3Lost; eline "t=%lu FM3 LOST level=%d sentinel=%08x nowLevel=%d done=%d elapsed=%lluus slice=%lluus fix=%d"
  g3.active=0

// FM3r  (RemoveObject 0x51E0C0; cold)
extern "C" void __cdecl CompanionAiFm3Remove(int ebp)
  obj=[ebp+8]; aim=[ebp-0x10]; lv=rd([obj+0x78]); if (lv<0||lv>4) return
  list=aim+lv*16+4; idx=indexOf([list], [list+4], [obj+4]); cur=[list+0xC]
  if (fm3 && idx>=0 && idx<=cur) [list+0xC]=cur-1          // no skip after DelIndex
  if (g3.active && obj==g3.sentinel && lv==g3.level) { g3.lostByRemove=1; if (fm3) [g3.ebp-0x40]=1 }   // end the level after this AIUpdate
  tline throttled "t=%lu FM3 REMOVE id=%08x level=%d idx=%d cursor=%d sentinel=%d"
```
The FM3r write to `[g3.ebp-0x40]` is valid only while `g3.active`: the UpdateState frame is on the stack, because RemoveObject is reached from AIUpdate or from the event pump inside the level loop. `[ebp-0x40]` is read at `0x51DB81` (skip the dispatch) and `0x51DCF5` (end the level).

```
// FM5a/b/c  (one handler, three sites; post path only)
extern "C" void __cdecl CompanionAiFm5Blocked(int ebp)   // site constant baked via 3 thin wrappers
  (ev,cr,blk) = a:([ebp-0xA8],[ebp-0x128],[ebp-0x5C])  b:([ebp-0x118],[ebp-0x3D0],[ebp-0x3C])  c:([ebp-0x88],[ebp-0x15C],[ebp-0x10])
  if (!party(cr)) return; t=track(cr); ++t.posts
  if (fm5 && now - t.lastDelivered < fm5_throttle_ms && blk == t.lastBlk) { *(u16*)ev = 0; ++t.neutered }   // 0-1 > 0x1E -> 0x570F5A frees it, no script
  else { t.lastDelivered=now; t.lastBlk=blk }
  tline throttled "t=%lu FM5 BLOCKED cr=%08x site=%c blk=%08x posts/s=%u neutered=%u"

// P2  (hot)
extern "C" void __cdecl CompanionAiP2Cadence(int ebp)
  c=[ebp-0xD4]; if (!g_cfg.p2 || ![c+0x11AC] || [c+0x10E8] || [ebp+8]!=1) return
  want = (id(c)==cachedControlledId()) ? p2_controlled_ms : p2_interval_ms     // controlled id cached ~100 ms
  if ((u32)[ebp-0x18] >= want) { [ebp-0x18]=0xFA0; ++st.p2Runs }              // the 4000 test at 0x56BEA2 then passes

// FM6  (optional probe)
extern "C" void __cdecl CompanionAiFm6Leader(int ebp)
  p=[ebp-0x12C]; for k in 0..2: log id=[p+0x24+k*0x88], slot4=[p+0x28+k*0x88]; eline "t=%lu FM6 LEADER slots=..."
```

**A1 handler additions (no window):**
- `a1_keep_combat_ms`, `a1_retarget_strict`;
- `c524/f570/combat` fields in the A1 line;
- `pendingA1` → `A1 FOLLOWUP` / `A1 ORPHAN`.

**INI** (`companion_ai.ini` style; ship = log-only):
```
; FM2: pathfind failure returned "done" + a 2-4 s wait. 0 = log (FM2 FAIL / FM2b REAPPROACH); 1 = after
; fm2_fail_limit failures against the same target within fm2_window_ms, drop the action chain, skip the wait
fm2=0
fm2_fail_limit=2
fm2_window_ms=12000
; in combat, cap the end-of-round fallback timer after a fail-fast (0 = engine value)
fm2_rethink_ms=500
fm2_party_only=1
; A1 follow-ups: guarantee one fallback after an A1 end (0 = off); reject retargets the attack's own range test fails
a1_keep_combat_ms=0
a1_retarget_strict=0
; FM3: 0 = probe only (FM3 LOST / FM3 REMOVE); 1 = end the level when its sentinel leaves + fix the list cursor
; (changes scheduling for ALL creatures, enemies included: user decision)
fm3=0
; FM5: 0 = log; 1 = deliver at most one ON_PATH_BLOCKED per fm5_throttle_ms per party member (others become no-ops)
fm5=0
fm5_throttle_ms=1000
; P2: party perception refresh interval (vanilla 4000 for companions, 200 for the IsPC creature)
p2=0
p2_interval_ms=500
p2_controlled_ms=200
fm6_probe=0
```

**Scratch `check_windows_r1.py` output** (R1 rows. The copy adds the 10 windows `shipped=False` and a check that every window is more than 12 bytes from `0x40D69E 0x8ABA8C 0x8ADA08 0x8ADA81 0x8ADFA2 0x8AE02D 0x8AE19A 0x8AE1CD 0x8AE793`):
```
live snapshot: live_hooks_2026-09-30g.tsv (120 hooks); live patch_config.toml: 26 ids, 149 hooks
        EAX at resume 0x5c2441: dead on all paths
        EAX at consumed exit 0x5c24c8: dead on all paths
        stack: pending at the jump (end) 0, at consumed exit 0x5c24c8 reset ['reset']
[OK  ] CA R1 FM2 pathfind-fail gate (skip wait): 0x005c2438 len 9 consumed->0x005c24c8  bytes = [0x8B, 0x95, 0x6C, 0xFA, 0xFF, 0xFF, 0x89, 0x51, 0x70]
[OK  ] CA R1 FM2b 0x11 re-approach probe: 0x005bc5f4 len 9  bytes = [0x8B, 0x4D, 0xE4, 0x8B, 0x91, 0x24, 0x01, 0x00, 0x00]
[OK  ] CA R1 FM3 sentinel set (cold): 0x0051dbe1 len 6  bytes = [0x8B, 0x4D, 0xD8, 0x89, 0x4D, 0xC4]
[OK  ] CA R1 FM3 level end (cold): 0x0051dd33 len 6  bytes = [0x8B, 0x4D, 0xD0, 0x2B, 0x4D, 0xF0]
[OK  ] CA R1 FM3 RemoveObject: 0x0051e0c9 len 6  bytes = [0x8B, 0x45, 0x08, 0x8B, 0x48, 0x78]
[OK  ] CA R1 FM5a blocked post (follow st2): 0x005c8bc2 len 6  bytes = [0x8B, 0x0D, 0xA4, 0xB4, 0xA1, 0x00]
[OK  ] CA R1 FM5b blocked post (follow st3): 0x005c9f9d len 5  bytes = [0xA1, 0xA4, 0xB4, 0xA1, 0x00]
[OK  ] CA R1 FM5c blocked post (walk): 0x005c3d8b len 5  bytes = [0xA1, 0xA4, 0xB4, 0xA1, 0x00]
[OK  ] CA R1 P2 party perception cadence: 0x0056be79 len 6  bytes = [0x8B, 0x85, 0x2C, 0xFF, 0xFF, 0xFF]
[OK  ] CA R1 FM6 leader-change probe: 0x007e6546 len 10  bytes = [0xC7, 0x80, 0x94, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00]
Pairwise overlap among 43 windows: none
TOML cross-check (source hooks.toml vs these windows): all hooks match
ALL OK
```
`grep -c WARN` gives 0. All 33 existing windows also stay `[OK  ]`.

**`verify_windows_both_exes.py`** (scratch copy, given the scratch output):
```
CA R1 FM2 pathfind-fail gate (skip wait) 005c2438   9  LAA:True  base:True
CA R1 FM2b 0x11 re-approach probe 005bc5f4   9  LAA:True  base:True
CA R1 FM3 sentinel set (cold)    0051dbe1   6  LAA:True  base:True
CA R1 FM3 level end (cold)       0051dd33   6  LAA:True  base:True
CA R1 FM3 RemoveObject           0051e0c9   6  LAA:True  base:True
CA R1 FM5a blocked post (follow st2) 005c8bc2   6  LAA:True  base:True
CA R1 FM5b blocked post (follow st3) 005c9f9d   5  LAA:True  base:True
CA R1 FM5c blocked post (walk)   005c3d8b   5  LAA:True  base:True
CA R1 P2 party perception cadence 0056be79   6  LAA:True  base:True
CA R1 FM6 leader-change probe    007e6546  10  LAA:True  base:True
ALL MATCH
```

**Neighbours:**
- CA-FM3s ends exactly where the unshipped `0x51DBE7` probe starts. Never ship both.
- CA-FM3s/e are 0x8F and 0xAD bytes from perf-lab's `0x51DC70`/`0x51DC86`.
- Nothing is near perf-lab's `0x55AB30`/`0x55AC26` or any sibling site.

## Open questions for the play session / the lead

1. **FM2 reach.** In the play log (once the hooks are built), compare `FM2 FAIL n>=2` against `FM2b REAPPROACH` for the same creature and target near a ledge or turret enemy. Perf-lab's `0x55AC26` result counts (code 3) may answer it before a build.
2. **FM2 Stage 2 (lead).** Fail-fast alone cannot stop `k_ai_master` re-picking the same unreachable enemy. Options:
   - (a) in the FM2 handler, when the move belongs to an attack chain, call `0x57C990(cr, range, exclude=failedTgt, INVALID)` and re-point the group's 0xC node (`node+0x38`), clearing `+0x52C` afterwards because `InitializeNewAttackTarget` sets it;
   - (b) a script-side exclusion fed through a local number.
   Both need design.
3. **A1.** After the next play session, check `A1 … -> end` lines for a following `E2 FALLBACK … lastA1≈3000ms`. Any `A1 ORPHAN`, once added, means switching `a1_keep_combat_ms` on (e.g. 500).
4. **A1 retarget.** Is the "retarget then silent passive end" visible (a `retarget` line, no shot, E2 ~3 s later)? If yes, set `a1_retarget_strict=1`.
5. **FM3 fix.** It changes scheduling for every creature, enemies included. Does "enemies stay vanilla" cover scheduling fairness? Recommendation: ship the probe only, and decide from the `FM3 LOST` counts in crowded fights.
6. **P2** is party-only, so it is allowed by "party first", but it is a visible behaviour change (companions notice enemies within about 0.5 s instead of up to 4 s). Is it wanted? CPU cost: about 6 `UpdatePerception(0)` calls per second for a 3-member party, against P1's per-tick enemy calls.
7. **FM6.** Find the consumer of the per-slot flag set at `0x7E6546` (slot+0x70). If it resets the trail lazily, close FM6 as refuted.
8. **Review corrections to record in the DB:**
   - the MoveToPoint door branch posts event 0x1E to the door, not OnBlocked;
   - OnBlocked is posted from `WalkUpdateLocation` during walks;
   - the FM5 consumed-skip fix would leak the event;
   - `0x51DBE7` is per-iteration (hot) and `0x51DBE1`/`0x51DD33` are the cold FM3 sites;
   - `AddPathfindingWaitActionToFront` takes (word group, float secs) with 0.0 meaning the default 0.3 s.
