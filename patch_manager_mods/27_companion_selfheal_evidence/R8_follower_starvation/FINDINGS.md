# R8 — follower starvation fix route (static, Sonnet 10-05; written by Opus from the agent's report)

KOTOR II Steam swkotor2.exe 6a522e71. Excerpts in `dump/`. "UNVERIFIED" = no anchor in the dump.
Background (confirmed, not redone): doc 29 D5, `29_paused_command_evidence/PQ2.md` Q1.

## Structures (anchored)
- Action queue = CExoLinkedList wrapper at `cr+0x100`; wrapper dword 0 -> internal `{head@0, tail@4, count@8}`; node `prev@0, next@4, data@8`; action data `u32 type@0`, group word `+0x6C`.
- `0x54AA30` = `MOV ECX,[ECX]` + call `0x739E60` (AddTail body). `0x54AA10` -> `0x739DA0` (AddHead). `0x7F2A90` -> `0x73A200` ("RemoveHead() Removing from empty List"), frees node via `0x9196FD`, returns data. `0x7A1780` = empty test.
- RunActions `0x5D56B0` (this=creature, RET 0x10): head pop `005d5b6d..005d5b86` (ECX=this+0x100, `CALL 0x7A1780`, `CALL 0x7F2A90`); `[this+0x20C]` = node in flight (set `005d5bd3`, zeroed after). Stamps `+0xC0/+0xC4` set only if both 0 (`005d5b94..005d5bc4`), cleared only on results 2/3. Result 4 -> AddTail `005d6a7b`; result 1 -> AddHead `005d6aa0` + exit. After result 4 the loop continues unless >1000 us or `0x5D7160(type)==1` (types 1, 5, 0x30, 0x33 only).

## Q1 — `front` argument
`0x5648D0` AddItemCastSpellActions: `__thiscall` ECX=creature, **RET 0x2C** (11 args).

| # | arg | route A (call `0057b701`) | 0x3F expansion (`6d9500` case 0xA) |
|---|---|---|---|
| 1 | item OID | param_2 | node+0x64 |
| 2 | item-property byte | param_3 | node+0x68 |
| 3 | subtype byte | param_4 | node+0x6C |
| 4-6 | target x,y,z | param_6[0..2] | node+0x38/3C/40 |
| 7 | target OID (0x7F000000 none) | param_5 | node+0x44 |
| 8 | aux | param_8 | node+0x84 |
| 9 | no-target flag | param_5==0x7F000000 | node+0x48 |
| 10 | **front** `[ebp+0x2C]` | `local_a0` | 1 |
| 11 | **expand** `[ebp+0x30]` | 0 | 1 |

- `local_a0 = (cr+0x520 != 0 && 0x53F560(cr,0xC) != 0)` (`0057b6d2..0057b6f4`). **Dead in route A**: the expand==0 branch (`005648fe..005649f7`) never reads it. That branch: `0x591890` adds a round node (`0x590270` drops it if list >= 4 or `+0xB00` null); if `cr+0x520==0 && 0x590110` -> ActionManager `0x563C90` (no-op for non-controlled); if `0x53F560(cr,0x3F)==0` -> `0x53F7F0(0x3F,0xFFFF,1)` = **tail**. At most one 0x3F is ever queued.
- Calling `0x5648D0(…, front=1, expand=1)` front-inserts `[move, 0x31, 0x2E]` via `0x540CA0` but skips the round node, all `0x6D9500` gates and the `+0xB18` / SetActivity `0x563DA0` / round-free bookkeeping -> orphan round node, `0x5B8000` round reads UNVERIFIED. **Rejected.**
- No front variant of the whole use-item pipeline (`0x5644B0`, `0x540CA0` only build sub-actions).

## Q2 — rotate
- RemoveHead `0x7F2A90`: ECX=`cr+0x100`, RET 0, returns data, frees node (RunActions uses it identically at `005d5b86`).
- AddTail `0x54AA30`: ECX=`cr+0x100`, push data, RET 4, allocates a 0xC node (= RunActions result 4).
- No state to touch: data + group id persist, stamps only change inside RunActions; `+0x11B0` is written by 0x3D (`005bf925`=3), ClearAllActions (`00541210`=1), follow driver `0x5C7370` (-1).
- Hard precondition `[cr+0x20C]==0` (not inside this creature's RunActions).
- 0x40 / MoveToPoint front-inserts: the driver finds 0x3D by type (`0x5D5120(0x3D)`), but rotating a move to the tail would run it late -> **rotate only when every node ahead of the 0x3F is 0x3D**.
- Sequence: `[0x3D,0x3F]` -> `[0x3F,0x3D]`; RunActions pops 0x3F, `0x6D9500` expands with front=1 -> `[move,0x31,0x2E,0x3D]`, returns 4 -> 0x3F re-added at tail; loop continues same call.

## Q3 — 0x3D resumes
Yes: expansion only front-inserts; neither `0x6D9500` nor `0x5B8000` calls `0x541080` / `0x563C90`. The leftover 0x3F behind 0x3D with an empty round list returns 2 at `006da493` and is freed; while it stays, `0x53F560` finds it, so a later route-A use adds no 2nd 0x3F (rotate again).
UNVERIFIED: follow driver `0x5C6340/0x5C7370` may front-insert `[MoveToPoint,0x40]` mid-expansion (worst case one extra move).

## Q4 — paused
`0x6D9500` gates: `[round+0xAA8]!=0` -> 1 (`006d9554`); paused (`0x51C720(ECX=[[0xA1B4A4]+8],2)` -> `byte[app+0x10078]&2` via `0x538CD0`) -> 1; `0x590110==0` or `[round+0xAB8]>=1` (`006d96cc`) -> `006da425`: 1 if `[round+0xA84]!=0` else **2**.
Head 0x3F while paused returns 1 (harmless). **Do not rotate while paused.** Rotating when it would return 2 frees the 0x3F and orphans the round node -> rotate only when `0x590110!=0`, AB8<1, AA8==0, unpaused.

## Q5 — round nodes
Each route-A call adds one round node (dropped at >=4) but at most one 0x3F. Two uses = 2 round nodes + 1 0x3F (it returns 4 after each expansion and serves the next). If the 0x3F is cleared, both round nodes orphan (ClearAllActions never empties `+0xB00`). -> queue a 2nd use only after the 1st is consumed; never ClearAllActions a companion.

## RECOMMENDED ROUTE B (rotate)
Keep route A (`0x57D8B0` -> `0x57B4C0` -> `0x5648D0(expand=0)`). Per frame, game thread, per companion with a queued 0x3F:
- Walk (SEH/saferead, <= 8 nodes): find index `k` of the first 0x3F.
- Preconditions (else no change this frame): `[cr+0x20C]==0`; `[cr+0x11AC]!=0` (meaning UNVERIFIED, logged); 1 <= k <= 3 and every node before it is 0x3D; `round=[cr+0x10DC]` non-null, `[round+0xAA8]==0`, `(int)[round+0xAB8]<1`, `0x590110(__fastcall ECX=round, RET 0)!=0`; not paused.
- Action: k times `d = 0x7F2A90(ECX=cr+0x100)`; `0x54AA30(ECX=cr+0x100, d)`. Idempotent (k=0 once 0x3F is head). Log gates + types before/after.
- Fallback: any failed gate / faulted read -> no change, edge-triggered log, retry next frame; after ~5 s unpaused log one "starved" line and leave the queue. Never ClearAllActions, never `0x5648D0` expand=1.

UNVERIFIED (live check): (a) our hook point is outside RunActions; (b) `0x5C6340` after the cast; (c) meaning of `+0xA84` / `+0x11AC`; (d) pop+add needs no lock (RunActions does the same unlocked).

## Address table (16-byte aligned, prologue, RET n checked)
| address | name | prologue | RET |
|---|---|---|---|
| 0x5648D0 | AddItemCastSpellActions (DB high) | 55 8B EC 81 EC | 0x2C |
| 0x57B4C0 | UseItem (DB high) | 55 8B EC 6A FF | 0x1C |
| 0x57D8B0 | AddUseTalentOnObjectActions (DB high) | 55 8B EC 83 EC | 0x20 |
| 0x5D56B0 | RunActions (DB medium) | 55 8B EC 6A FF | 0x10 |
| 0x6D9500 | AIActionCombat (DB high) | 55 8B EC 6A FF | 0x4 |
| 0x5BF7F0 | PartyFollowLeader 0x3D (DB high) | 55 8B EC 83 EC | 0x4 |
| 0x53F7F0 | AddAction (DB medium) | 55 8B EC 6A FF | 0x70 |
| 0x540CA0 | AddActionToFront (DB high) | 55 8B EC 6A FF | 0x70 |
| 0x53F560 | GetActionByType (no DB row, body anchored) | 55 8B EC 83 EC | 0x4 |
| 0x54AA30 | AddTail thunk (no DB row, body anchored) | 55 8B EC 51 | 0x4 |
| 0x54AA10 | AddHead thunk (DB medium) | 55 8B EC 51 | 0x4 |
| 0x7F2A90 | RemoveHead thunk (no DB row, string anchored) | 55 8B EC 51 | 0 |
| 0x590110 | GetActionPending (DB high) | 55 8B EC 83 EC | 0 |
| 0x590270 | round-list insert (no DB row) | 55 8B EC 83 EC | 0x8 |
| 0x591890 | add round node | 55 8B EC 6A FF | 0x24 |
| 0x51C720 / 0x538CD0 | pause test / bit test | 55 8B EC 51 | 0x4 |
