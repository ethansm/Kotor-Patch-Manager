# GATE wave 1 (AI1-AI4), Opus verification, 2026-10-02

## Summary (8 lines)
1. FM2b CORRECTED: an unreachable target gives the companion SUCCESS(2) with a partial path in most cases (straight-line bisection fallback `0x55B3E9..0x55B692`, plus a grid "closest result" rule AI1 missed, `0x550E30` decompile l.238-250), and FAIL(3) only on residual cases. One partial leg walks it to the obstacle, then the next re-plans usually FAIL -> R1's 2-4 s wait loop. FM2 and FM2b are two phases of one loop.
2. AI1 REFUTED on one point: `0x5BC1D0` (action 0x11) has NO line-of-sight test. The LOS block is dead code (`005bc4b8 XOR EDX,EDX` / `005bc4ba JZ 0x005bc5de` always taken). R1 was right.
3. AI3 spot-distance sign: **BUG**. Spot adds `+ftol(d/3-2)` past 6 m (`00579007 NEG EAX` then `0057900f SUB ECX,[EBP-0x10]`); listen subtracts. Effect: hiders are easier to spot from far away. Fix = 1 byte at `0x57900F` (`2B`->`03`).
4. Confirmed: 6 OnBlocked post sites (R1 listed 3); `0x5C0732` is the trigger (type 7) case, the door (0xA) case posts 0x1F at `0x5C1240`; `+0x220` is the player update-set flag, not a party flag (AI4 over U3); action table `0x5D7038` (7 entries decoded); slot-6 site 3 = SetStateInternal removal (stun/horror/choke/whirlwind/droid-stun ending).
5. New gate facts: effect type 9 is created only by `OnApplySetState 0x59CBA0`, which also writes creature `+0xFF5` = current debilitating state (AI3's "class byte" reading is wrong). `0x550980` code -1 also covers the collision-mesh sweep (`0x5EA8F0`, swcollisionmesh.cpp), and -1 disables the partial fallback.
6. Contradictions: 9 found and resolved. AI1 vs R1 (FM2, FM5, door label, 0x11 LOS), AI4 vs U3 (+0x220), AI3 vs DB (3 names), AI3 +0xFF5, AI1's PI+0x24 and -1 code meanings.
7. DB: 12 corrections (renames, class fixes, upgrades, 1 demotion) and 24 new name_map rows (tables below). 14 lessons proposed.
8. Probe windows: all 20 checked windows are on instruction boundaries, with no relative branch inside and no inbound target inside. New cold window `0x55B69A` (6 B) splits partial and fail per request: it is the probe that settles FM2b.

Method: `.asm`/`.c` from `ROOT/decomp`, constants and jump tables read from `backups/vanilla_clean/swkotor2.exe` with a PE section reader (`.text` 0x401000/raw 0x400, `.rdata` 0x986000/raw 0x584C00). Window checker: boundary, relative-branch and inbound-target scan over all `.asm`. The lead's line checks (LEAD_SPOTCHECKS.md) were not redone.

## 1. Verdict table

### AI1 Pathfinding and movement
| claim | verdict | evidence |
|---|---|---|
| FM2b: unreachable target gets a partial path (2), not only FAIL | **CORRECTED** (direction right, conditions incomplete) | See 1.1 |
| AI level 0 -> PlotPath FAIL, then only the straight partial | CONFIRMED | `0055b1c7 MOV ECX,[0x009944bc]` then `0055b1d0 JMP 0x0055b3e6`, which is the fallback entry `0055b3e9 CMP EAX,[0x009944bc]` |
| Walker end resets the request, so the next move is a new search | CONFIRMED | `005c3eec CMP [EBP-0x58],1` / `005c3efe CALL 0x006f9c90` (zeroes PI goal `+0x5C..+0x80`, `+0x58`; `0x6F9D40` sets `+0x88=1`) / `005c3f1b MOV ECX,[0x009944b8]` (returns 2) |
| OnBlocked (payload 0x1F) posted from 6 movement sites, not 3 | CONFIRMED | Payload store then `CALL 0x0051e170` within 21-23 insns at `0x5C1240`(5BFC00), `0x5C3D7D`(5C3460), `0x5C5A68`(5C3F70), `0x5CCBB4`(5CC870), `0x5C8BB4`(5C87E0), `0x5C9F8F`(5C8C20). The other `MOV reg,0x1F` hits (`0x864170`, `0x5D734F`) are value-mapping switches. |
| `0x5C0732` is the trigger (type 7) case, not the door | CONFIRMED | 5BFC00 switch `005c0409 JMP [ECX*4+0x5c25bc]`, index = type-5. Table bytes: `0x5C07A7, 0x5C172F, 0x5C0410, 0x5C172F, 0x5C1555, 0x5C1143` (types 5..10). `0x5C0732` lies in the type-7 block `[0x5C0410,0x5C07A7)`, `0x5C1240` in the type-0xA block `[0x5C1143,0x5C1555)` |
| `0x5BC1D0`: "in range -> LOS eye-to-eye, LOS fail -> re-queue" | **REFUTED** | `005bc4b5 MOV [EBP-0x2C],EAX` (in-use-range result), `005bc4b8 XOR EDX,EDX`, `005bc4ba JZ 0x005bc5de`: ZF=1 always, so `0x5BC4C0..0x5BC5D7` (range<0.5 -> 3, eye +1.5, `CALL 0x0052bb60`, range/2, `[EBP-0x44]=0`) is dead. The only inbound branches (`JNP 0x5bc4e0`, `JP 0x5bc4ea`) come from inside the dead block. `[EBP-0x44]` stays 1 (`005bc45a`). Action 0x11 = same area && in use range. |
| `0x550980` code -1 = "off mesh" | CORRECTED | `0x550980.c`: -1 when `0x5FF730`(->`0x651090` face walk) fails or `0x5FF770`(->`0x5EA8F0`, "swcollisionmesh.cpp") returns 0. A wall (perimeter edge with no room transition, `0x5FF800 == -1`) gives **0**; -2 = door/placeable box; -3 = creature on the line. |
| `+0x558` is a combat LOF field, not movement | CONFIRMED (agrees with AI2; the brief was wrong) | writers `0x6D7CBC/0x6D7DDA/0x6D7E0F` + ctor only |
| Bump abort counter `PI+0x268`, > 5 | CONFIRMED (lead) | `005cbc18`, `005cbcb5 PUSH 0x9965f8` |
| Grid search failure always means FAIL(3) ("Distance to goal") | **CORRECTED** | 0x550E30 decompile l.238-250: `if ((PI+0x24 == 1) && (1 < PI+0x188) && result != 2 && local_7c == 1)` and `(4*limit <= elapsed) or (limit <= elapsed && best distance PI+0x190 unchanged)` -> `if (area+0x1d0 != 1) result = 2` + `_memcpy(PI+0x15c, PI+0x18c, n*8)`. `PI+0x24 = 1` is written on every call (l.171). So the closest-so-far grid path is returned as SUCCESS. `area+0x1D0` = the 64-expansion abort flag of `0x551C30` (`00551e52`). |

#### 1.1 FM2b answer (decides the FM2 fail-fast design)
The companion's approach move comes from 0x11: `005bc6d1 CALL 0x005644b0`. Its param_9 (= node flag bit1, client path) is `005bc6a4 PUSH 0x0`. `0x5644B0` packs `local_8 = param_9 << 1 | param_6`, so `005c1918 MOV [EAX],EDX` (after `005c190f SETNZ DL` on `[EBP-0x6C]`) stores **PI+0 = 0**. The partial fallback is therefore open for every engine/script approach move.

What `0x55AC80` returns for an unreachable goal, in order:
1. Call #1 of a request runs the straight test start->goal (`0055af84 CALL 0x00550980`) and caches the code in PI+0x19C (`0055af92`). If it returns 1, the result is a 2-point path, 2.
2. Otherwise a search runs: grid (`0055b205`/`0055b318`) or path-point (`0055b2c4`). It is pending (1) across ticks with a 1 ms slice per call. Then:
   a. **2 via the grid closest-result rule** (above): a multi-point path to the closest cell found, which can go around obstacles. Reached once an iteration crosses the slice without improving the best distance and >= 2 best points exist.
   b. **3** from depth/time/no-progress, from the 100-call cap (`0055b3dd`), or from `-4` (`0055b322`).
3. On 3, the straight fallback runs: `0055b3e9 CMP EAX,[0x009944bc]` / `0055b3f8 MOV EDX,[ECX]` / `0055b407 JNZ 0x0055b69a` (requires PI+0 == 0). Then `0055b4fb CMP [EBP-0x20],-0x1` / `JZ 0x0055b61c` (**the fallback is skipped when the cached start->goal code is -1**). Six bisection probes along the 2D segment (z zeroed at `0055b450`/`0055b46a`) follow, each `0055b5a7 CALL 0x00550980(start, candidate)==1`. Any clear probe leads to `0055b628 MOV [ECX+0x8c],4` (start, last clear candidate) and `0055b692 MOV EAX,[0x009944b8]`, i.e. **2**. No clear probe: `0055b623 JZ 0x0055b69a`, i.e. **3**.

So `0x5BFC00` sees 3 only for: client-path nodes, a cached code of -1 (face-walk failure, or a collision-mesh sweep failure along start->goal), all 6 probes blocked, no inter-area route, request area != module area, or a null PI.

What happens next (CONFIRMED code; leg count SPECULATIVE):
- **Leg 1, result 2:** `005c1f45 CMP EAX,[0x9944b8]` leads to the walk. The creature walks straight at the obstacle (or along the closest grid path). At the path end the walker calls `0x6F9C90` and returns 2. RunActions ends the loop (type 1 stops it: `0x5D7160`).
- **Next tick:** 0x11 is still not in range, so it re-queues at the front, in order: wait 0.3 s (`005bc6ea`), move (`005bc6d1`), 0x11 (`005bc652`). It returns 2.
- **Leg 2:** after the 0.3 s wait, `0x6F9C90` has zeroed the stored goal, so the request is new (`005c1d4a MOV [EAX+0x58],1`). Search runs again. The creature now stands at the obstacle, so the probes from there fail (or creep a few centimetres: the probe step is 1/128 of the remaining segment). The fallback gives 3 and the R3 branch runs: `005c21e2` log, then `005c2484 CALL 0x005646c0`, a 2.00-3.98 s wait (0.1 s if player-controlled), then return 3 (`005c25a5`). RunActions treats 3 like 2.
- **Steady state** = R1's FM2 loop: wait 2-4 s, 0x11, wait 0.3 s, move, FAIL. Each move costs >= ~10 ticks of 1 ms search. In combat the E2 fallback (slot 6 every 3 s; moves and 0x11 do not reset `+0x570`) restarts the chain with a fresh `[move, 0x13, 0x11, 0xC]`, and every restart starts with a fresh partial leg 1.

Reconciliation with R1 / lesson 275: "a failed path returns 3, treated like success" is right for the steady state. FM2b ("succeeds, walks, 0x11 not in range") is the first leg after every re-plan from a new position.

Design consequence: the FM2 fail-fast window `0x5C2438` sees only the FAIL legs. Key the fail-fast on the 0x11 re-approach count per (creature, target) with a no-progress test (distance not shrinking) at `0x5BC5F4`, and use `0x5C2438` only to skip the 2-4 s wait. The probe `0x55B69A` (section 5) measures the 2-vs-3 split and the -1 share.

### AI2 Combat round and attack pipeline
| claim | verdict | evidence |
|---|---|---|
| Round is a separate heap object, `new(0xB2C)` at creature `+0x10DC` | CONFIRMED (lead) | `0056223f PUSH 0xb2c`, `00562290 MOV [EDX+0x10dc],EAX` |
| Attack/approach test = 2D centre dist2 vs (Max+0.1)^2 | CONFIRMED | `006d7e42 CALL 0x004452b0` (target - attacker+0x94), `006d7e4a FLDZ` / `006d7e4c FSTP [EBP-0x74]` (z := 0), `006d7e57 CALL 0x0044b8a0` dot -> `[EBP-0xBC]` |
| Retarget metric (0x57C990) = 3D length minus both PI+8 radii | CONFIRMED | `0057cd51 CALL 0x005142f0` (3D length), `0057cd74 FLD [EDX+0x8]` (self PI) / `0057cd80 FSUB`, `0057cdb4 FLD [EAX+0x8]` (cand PI) / `0057cdc0 FSUB`, `0057cdcf FCOMPP` |
| Slot-6 site 3 = `0x59E860`, effect type 9 removal | CONFIRMED + extended | Remove dispatcher `0x5947A0` indexes `[handler+8][type*4]` by the effect type word (`005947ac MOV CX,[EAX+0x8]`, `005947f2 MOV EAX,[EAX+EDX*0x4]`). `0059414c MOV [ECX+0x24],0x59e860` = index 9. Type 9 is created only in `OnApplySetState 0x59CBA0` (`*(undefined2 *)(local_198 + 8) = 9;` with int0 = the state). The SetState type-8 creators load `[0x995F3C]`=8: EffectDroidStun `0067754e` (state 3), Choke `0067825e` (7), Horrified `0x677DFE` (8), Whirlwind `00677abe` (10). |
| ...can it fire mid-combat for party members? | **YES** | `0x59E860` runs slot 6 (`FUN_006fd8d0(iVar1 + 0x2a0, id, 1)`) for states 1, 3..10, 0xE..0x13, i.e. when a stun/choke/horror/whirlwind/droid-stun ends on a companion. It also fires when a stronger state replaces a weaker one: `0x59CBA0` first removes existing type-9 effects (`FUN_00544790` in the loop above the `new(0x8c)`). It has **no leader, dead or combat gate** (unlike EndCombatRound), then refreshes mode-0 perception for non-IsPC. |
| `+0xA9C` remaining budget; `+0xAB8` actions-this-round | CONFIRMED (asm quoted by AI2, consistent with the `DecrementRoundLength` decompile) | `0058ff70`, `006d90b0`, `006d96cc` |

### AI3 Perception, LOS and stealth
| claim | verdict | evidence |
|---|---|---|
| Spot distance term has the wrong sign | **BUG (CONFIRMED)** | See 1.2 |
| R1 scan abort reaches only modes 0/2 | CONFIRMED | `0x5D0F30.c`: `if (param_2 == '\x01') { if ((param_1[0x46b] == 0) && ...0x73FB90 party table loop... FUN_00576dc0 }` never calls `0x522AF0`; the `else` branch has `iVar1 = FUN_00522af0(&local_2c,local_1c); if (iVar1 == 0) return;` (skips the `param_1[0x472] = 0` reset too), after the existing-list refresh loop |
| 176 calls/s = 4 non-party x 44 AIUpdates/s | CONFIRMED (mechanism); the 44/s = frame rate equation is SPECULATIVE | The counted site `0x56BDB4` is reached only past `0056bd5b JNZ 0x56bde9` (party skips) and `0056bdac JNZ 0x56bde9` (IsPC skips), on the `[EBP+8]==1` path. `[EBP-0x28]` = TimeDiff(now, stamp `+0x3A0/+0x3A4`), and the only store after `0056bdc5 CALL 0x005d0f30` is `0056bde3 MOV [EAX+0x39c],EDX`, so the elapsed time is ms-of-day and is >= 4000 all day. One call per AIUpdate (`005694e3`). |
| P1 missing store | CONFIRMED (lead + gate re-read above) | |
| E3: creatures never block LOS | CONFIRMED static, conf **medium** (absence over callees; doors/placeables only via `0x552D90`) | |
| `+0xFF5` = "creature class byte / script package" | **CORRECTED** | `0x59CBA0`: `if ((int)(uint)*(byte *)(iVar2 + 0xff5) < iVar5) { *(undefined1 *)(iVar2 + 0xff5) = local_20; ...` = current (highest) SetState id. AI3's `0x56BB65` switch and the `{0x12,0x13}` test in 0x576DC0 are state tests. |
| DB names `0x585420`, `0x5802C0` wrong | CONFIRMED | `0x585420.c`: returns 1 iff `+0xFF4 & 0x10` or (`&8` and not `&2`); `0x5802C0.c`: `(+0xd8 == 10000 && +0x510 != 1) ? 2 : +0x510` |

#### 1.2 Spot sign verdict: BUG
Constants (vanilla exe): `[0x98B968]` = 3.0 (double), `[0x987490]` = 2.0 (double). Asm `0x578FD2..0x579012`:
`CALL 0x005142f0` (3D |P-T|) / `FDIV [0x98b968]` (d3 = d/3) / `FCOMP [0x987490]` / `TEST AH,0x41` / `JNZ 0x579015` (skip when d3 <= 2) / `FSUB [0x987490]` / `CALL 0x0091c860` (n = ftol(d3-2) >= 0) / `00579007 NEG EAX` / `00579009 MOV [EBP-0x10],EAX` (= -n) / `0057900f SUB ECX,[EBP-0x10]` / `00579012 MOV [EBP-0x48],ECX`, i.e. **score += n**.
`[EBP-0x48]` is the spotter's margin, not a DC. Spotted iff `0057901b CMP [EBP-0x48],0` / `0057901f JG` keeps the result. Every other term has the margin polarity:
- `00578ab5 SUB EDX,[EBP-0x54]` (target Stealth rank) and `00578ad4 SUB EAX,EDX` (target dice `+0x1024`);
- `00578b37 ADD ECX,[EBP-0x40]` (Awareness) and `00578b9b ADD ECX,[EBP-0x48]` (spotter dice `+0x1026`);
- `00578c19 SUB EAX,5` (moving), `00578e9c SUB ECX,0xa` (blind-spot cone), `00578ec7 SUB ECX,0xa` (in combat).

The listen check in the same file subtracts its distance term (`005787ab FDIV [0x98b968]` -> ftol -> `005787b9 SUB EDX,EAX`). The spot term is therefore a double negation: past 6 m a hider becomes EASIER to spot, by up to `PrimaryRange/3 - 2`. The swing is 2n (8 points at 18 m).

Scope: only reached for a target in stealth mode with Stealth rank > 0 (`0x578A37`/`0x578A5F`), so it hits the party sneaking past enemies. Fix: KPM simple 1-byte hook at `0x57900F` (`2B 4D F0` -> `03 4D F0`, `ADD ECX,[EBP-0x10]`; lesson 269). The flags are rewritten at `0x579015` before any read. This changes balance (stealth stronger at range), so it is a user decision. NOT A BUG in listen.

### AI4 Scheduler and action queue
| claim | verdict | evidence |
|---|---|---|
| `+0x220` = player update-set flag (contradicts U3 "party/PC flag") | CONFIRMED | `0x63C1E0.c`: object from `0x53DFB0(id)` -> `vt+0x10` (CSWSObject), `local_c = FUN_0063dd30(local_14,param_2); if (*(int *)(local_14 + 0x260) != 0) local_c = 1; *(int *)(local_14 + 0x220) = local_c;`. In `0x63DD30`, for type byte `[0x994474]` (5, creature): `(param_1[1] == param_2[1]) \|\| (... +0x11ac != 0)` gives 1 (self or party); otherwise in-range with `0x585480` not invisible, or the player's perception entry seen/heard bit. `+0x260` = ForceAlwaysUpdate. |
| Items are AI objects at level 0 | CONFIRMED (lead) + cost | Item vtable `0x9972CC` (stored at `005ffdc0 MOV [EAX+0x10],0x9972cc`) has slot +0x70 = `0x600F20` (rdata `0x99733C`). `0x600F20` returns after `FUN_00603f60(10,0)` (a scan of the item's property list for type 10) unless a timed property 0xE..0x12 exists. Per-item AIUpdate is cheap; the cost is the scheduler's per-iteration pre-pass lookup (AI4 Q4). |
| Action table `0x5D7038`, index type-1, 0x47 entries | CONFIRMED | Vanilla exe decode: type 1 -> `0x5D5C2F` (`005d5c4e CALL 0x005bfc00`), 0xC -> `0x5D631D` (`005d635e CALL 0x006d7390`), 0x3F -> `0x5D63AE` (`005d63cd CALL 0x006d9500`), 0x3D -> `0x5D5C5B` (leader id `0x73F470`==INVALID gives 1; `+0x11AC` gate; `005d5d35 CALL 0x005bf7f0`), 0x1E -> `0x5D656E` (`005d6578 CALL 0x005d9aa0`), 0x11 -> `0x5D5F79` (`005d5f98 CALL 0x005bc1d0`), 0x47 -> `0x5D682A` (`CALL 0x005df920`); 0xD/0x39/0x3B/0x42 -> `0x5D688E` (no handler). Matches the review Map. |
| Dismissed party member -> level 2 | CONFIRMED | `0058a448 PUSH 0x2` ... `0058a458 CALL 0x0051e120` |
| Level-pass cut by a pending event | CONFIRMED code, reach SPECULATIVE | `0051dcec CALL 0x0051ed10` / `0051dcf3 JNZ 0x0051dd33`; `0x51ED10` = head time <= tick snapshot. Perception (`0x576DC0`) and OnBlocked posts use delta 0, so they are due in the same ms (see section 2). |

## 2. Reconciliation
- **AI1/AI2 approach walk.** There are two different "in range" tests.
  - 0x11 (`0x5BC1D0`): same area && `GetIsInUseRange 0x585CC0` (range minus target radius, `0x5BC464`), **no LOS**.
  - 0xC (`0x6D7390`): 2D centre dist2 <= (MaxAttackRange+0.1)^2, plus LOS `0x52BB60` at eye 1.5 with the `+0x558` same-position memory.
  - `+0x558` is owned by AIActionPhysicalAttacks alone (writers `0x6D7CBC/0x6D7DDA/0x6D7E0F`, ctor `0x562066/0x562BC1`; read `006d7c5d CALL 0x517980`); movement never touches it.
  - Open: the metric of `0x585CC0` vs the 0xC 2D test. If 0x11 can say "in range" while 0xC says "approach", that is an approach ping-pong candidate (wave 2, AI1/AI2 overlap).
- **AI2/AI4 action types.** Consistent.
  - 0xC is reached with `obj+0x88` = node param0 around the call (`005d633f`/`005d636c`). 0x3F pops round actions from `+0xB00`. `AddAttackActions` queues one 0x3F (round path) or 0xC.
  - A paused round makes 0xC/0x3F return 1, so RunActions re-adds the node at the head and the loop stops (`005d6a88`).
  - 0xC is exempt from the target clear on 2/3 (`type != 0xC` before `vtbl+0xC0(INVALID)`).
- **AI3/AI4 perception cadence.** Consistent: `0x56B8A0(1)` once per AIUpdate (`005694e3`), AIUpdate once per scheduler pass.
  - Interaction: the 4 s block-3 tail drops level 4 -> 0 in a player-less area (`0056c01c`), and AIUpdate re-promotes it (`005694d6`). This is FM7 and one FM3 self-transition source (both batches agree).
  - New link: perception SEEN/HEARD/VANISHED posts (`0x51E170(0,0,...)` in 0x576DC0) and all OnBlocked posts are delta-0 events, stamped with world time from `0x51AD20` at posting. `0x51ED10` treats an event as pending when head <= the tick-start snapshot, so a delta-0 event posted in the same ms ends the current level pass early (AI4 Q5 "New"). Reach depends on ms granularity (`0x51AD20` divides a continuous counter by 1000) or a paused world timer (`+0x24==1`). Probe AI4-H02 with a "cut by event" reason.
- **AI1/AI4 QuickWalk.** AI1's "walker for objects with `+0x220`" and AI4's "pre-pass QuickWalk for `+0x220` creatures" agree once `+0x220` is read as player-visible: visible creatures are moved by the scheduler pre-pass (`0051d9bc`) and `0x5BFC00` returns pending at X2.
- **Contradictions with prior docs, resolved:**
  1. R1 FM5 "3 post sites" vs AI1 "6": AI1 right (table 1).
  2. R1 "door branch 0x5C0732 posts 0x1E" vs AI1 "trigger": AI1 right. The real door case posts OnBlocked at `0x5C1240` inside MoveToPoint. R1's "the pathfind-failure branch is silent" still stands.
  3. AI1 "0x11 tests LOS" vs R1 "LOS compiled out": R1 right (dead block).
  4. Lesson 275 / R1 "failed path returns 3" vs AI1 "partial 2": both, sequentially (1.1).
  5. AI1 "grid failure = FAIL" vs the closest-result rule: corrected.
  6. U3 "+0x220 party/PC flag" vs AI4: AI4 right.
  7. AI3 `+0xFF5` "class byte": corrected to SetState id.
  8. AI1 "-1 = off mesh": corrected (face-walk or collision-mesh failure; walls give 0).
  9. DB names `0x585420`, `0x5802C0`, `0x66B640` wrong (section 4).
  No contradiction found between AI2 and the review/R1/R3, or between AI4 and R1 FM3.

## 3. Confidence rules (apply to all four batches)
- **R-high:** the row quotes an asm line (address + mnemonic) for the identity (callee/ret/string) or for a reader/writer, and the gate or the lead re-read it, or the batch quoted it and nothing contradicts it. Gate-verified rows are high regardless of the batch's own label.
- **R-med:** a behavioural name with a quote but no string/save-key/2DA anchor; mapper-only rows; absence claims ("no reader", "never written") backed by `field_census --all` (lesson 168: never high).
- **R-low:** runtime magnitudes, rates, frequencies, "plausibly hot", names inferred from one callee.

Exceptions (overriding the batch's own labels):
| batch | row | change |
|---|---|---|
| AI1 | (a) `0x5BC1D0` purpose "in range + LOS" | identity high, purpose text corrected (no LOS) |
| AI1 | (a) `0x550980` codes | med (code meanings corrected in table 1) |
| AI1 | (a) `0x550E30` | identity high; Q3 description incomplete (closest-result rule) |
| AI1 | (e) PI+0x024 "grid search started" | rename "closest-result enabled (set every grid call, l.171)", med |
| AI1 | (e) PI+0x188/0x18C | "best (closest) path count/buffer, returned as SUCCESS", high |
| AI1 | (e) CSWSCreature+0x220 (low) | **up to high**: CSWSObject `+0x220` player update-set flag (`0063c257`) |
| AI1 | (b) Q1 FM2b loop rates | low (runtime) |
| AI2 | (a) `0x59E860` (med) | **up to high**, name OnRemoveSetStateInternal |
| AI2 | (a) `0x6A93A0/0x6A5110` names | low (bodies unread) |
| AI2 | (b) Q2 rows 1, 8, 9 | high (gate-verified) |
| AI3 | (e) `+0xFF5` (low) | renamed "current SetState id", med |
| AI3 | (b) X1 spot sign | high (gate-verified) |
| AI3 | (b) E3 / Q6 "creatures never block LOS" | med (absence over callees) |
| AI3 | (b) Q4 item 7 ModSpotCheck ignored | med (absence) |
| AI3 | (a) `0x57E280` | low until read |
| AI4 | (e) CSWSObject+0x220 (med) | **up to high** |
| AI4 | (a) `0x63DD30`, `0x63C1E0` | med -> high for the `+0x220` write, med for the names |
| AI4 | (b) Q4 AIQ magnitudes, Q3 reach | low |
| AI4 | action table rows not spot-checked here (60 of 67) | med (table bytes are high; handler identities rest on DB/mapper names) |

## 4. DB name_map corrections (for the persist script)
Corrections to existing rows:
| addr | current name (conf) | new name | conf | evidence (one line) |
|---|---|---|---|---|
| 0x0051d790 | ServerEvents::Update (med) | CServerAIMaster::UpdateState | high | `0051d79f MOV [EBP-0x18],0x2710`, levels 4..0, sole caller `0053396e` with `[EAX+0x10044]` |
| 0x005644b0 | CSWSCreature::AddMoveToPointAction (high) | CSWSCreature::AddMoveToPointActionToFront | high | `005645e4 CALL 0x00540ca0` (AddActionToFront); 17 args, packs node flags |
| 0x00564770 | CSWSCreature::AddMoveToPointAction (med) | (same) | high | `005648a4 CALL 0x0053f7f0` (AddAction) |
| 0x0051e170 | ServerEvents::AddEventDeltaTime ret 24 (med) | CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data) | high | `005c3ea5 CALL 0x0051c280 / MOV ECX,EAX / 005c3eac CALL 0x0051e170`; node stores `[2..5] = p3..p6` |
| 0x0051d1c0 | CServerAIMaster::NextObject (med) | CServerAIList::GetNextObject | med | `this` = list (`param_1[3]` cursor, `param_1[1]` count) |
| 0x0051d270 | CServerAIMaster::GetObjectAt (med) | CServerAIList::GetObjectAtPosition | med | `FUN_0051c0a0(*(int*)(*param_1 + param_2*4))` |
| 0x00585420 | CSWSCreature::BroadcastAttackOfOpportunity (med) | CSWSCreature::GetIsBlindOrInDarkness | med | body: `+0xFF4 & 0x10 \|\| (+0xFF4 & 8 && !(+0xFF4 & 2))` |
| 0x005802c0 | CSWSCreature::UpdateAppearanceForEquippedItems_2 (med) | CSWSCreature::GetDetectModeState | med | `(+0xd8 == 10000 && +0x510 != 1) ? 2 : +0x510` |
| 0x0066b640 | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest (med) | Global::ExecuteCommandSetCreatureAILevel | high | `0066b6e9 MOV [EAX+0x7c],CL` (saved level), switch -> `CALL 0x0051e120` |
| 0x0055ab30 | CSWSArea::PlotPath entry (med) | CSWSModule::PlotPath (pathfind request) | med | `005c1f11 CALL 0x0051bd90` (GetModule) / `005c1f16 MOV ECX,EAX` / `005c1f18 CALL 0x0055ab30`; only caller |
| 0x0057e280 | CSWSCreature::SetLastInstantEquip (med) | (keep text, demote) | low | called from perception for `+0xFF5` in {0x12,0x13} near a non-hostile target (AI3); name does not fit; not read |
| 0x00550e30 | CSWSArea::PlotGridPath (med) | CSWSArea::PlotGridPath (returns closest path as SUCCESS) | high | strings "Exceeded limits:*"; l.238-250 closest-result rule |

New rows:
| addr | name | conf | evidence (one line) |
|---|---|---|---|
| 0x0051e0c0 | CServerAIMaster::RemoveObject | high | `0051e10d MOV [ECX+0x78],0xffffffff` after the list erase |
| 0x0051ed10 | CServerAIMaster::EventPending(day,ms) | high | head time `0x51AFE0`-compared to args, `<1` -> 1 |
| 0x0051ed80 | CServerAIMaster::GetPendingEvent | med | 6 outs, called in the UpdateState drain `0051da2b` |
| 0x0063c1e0 | UpdateObjectInPlayerUpdateSet (writes CSWSObject+0x220) | med | `0063c257 MOV [EDX+0x220],EAX` = `0x63DD30(...) \|\| +0x260` |
| 0x0063dd30 | IsObjectInPlayerUpdateRange(obj, player) | med | party/self -> 1; range^2 vs max(spot,listen) + `0x585480` |
| 0x005ffd60 | CSWSItem::Constructor | med | vptr `005ffdc0 MOV [EAX+0x10],0x9972cc`; `005ffe52 CALL 0x0051e040` level 0 |
| 0x0059e860 | CSWSEffectListHandler::OnRemoveSetStateInternal | high | remove table idx 9 `0059414c`; slot-6 RunScript `0059e904` |
| 0x0059de30 | CSWSEffectListHandler::OnApplySetStateInternal | med | apply table idx 9 `0059413f MOV [EDX+0x24],0x59de30` |
| 0x0059cba0 | CSWSEffectListHandler::OnApplySetState | med | apply table idx 8 `00593d0b`; creates type-9 effect, writes creature `+0xFF5` |
| 0x0059db50 | CSWSEffectListHandler::OnRemoveSetState | med | remove table (`[+8]`) idx 8 `00593d15 MOV EAX,[EDX+0x8]` / `00593d18 MOV [EAX+0x20],0x59db50` |
| 0x005959e0 | CSWSEffectListHandler::OnApplyDamage | med | mapper + `005966d0 CALL 0x6fd8d0` slot 4 (OnDamaged) |
| 0x006e0b10 | CSWSCreature::ResolveAttackRoll | med | `006e0bd9 CALL 0x006a2000` (1,0x14) d20; result `attack+0x5C` |
| 0x006a2000 | CSWRules::RollDice(n,sides) | med | ECX=`[0xA1B4D0]`, `PUSH 0x14/PUSH 1` at `006e0bd1` |
| 0x00550980 | CSWSArea::WalkLineTest (1 clear, 0 wall/no transition, -1 face-walk or collision-mesh fail, -2 door/placeable box, -3 creature) | med | `0x550980.c` `local_3c = -1/-2/-3/0` |
| 0x006f9200 | CPathfindInformation::Constructor | med | `005621bf PUSH 0x278` ... `005621dc CALL 0x006f9200` |
| 0x006f99f0 | CPathfindInformation::Shutdown (leaks +0x18C/+0x1B4/+0x1D8/+0x200) | med | frees `0x90..0x238` at `006f9bb6..006f9c5c` (U3 PFLK) |
| 0x006f9c90 | CPathfindInformation::ResetRequest | high | zeroes goal `+0x5C..+0x80`, `+0x58`; called at walker end `005c3efe` |
| 0x006f9d40 | CPathfindInformation::ResetAtEndOfPath | med | `+0x84=0, +0x88=1`, frees `+0xB8` |
| 0x006fc5d0 | CSWSCreature::AvoidCreature (runs k_def_pathfail01) | med | string `0x99B5E0` "k_def_pathfail01" |
| 0x005d1400 | CSWSCreature::AddToVisibleList(id,seen,heard,invis,hidden) | med | `new(8)` entry, bit packing (AI3 Q1) |
| 0x0051bd90 | GetModule (fastcall wrapper of 0x52F4C0) | med | body `FUN_0052f4c0(param_1)` |
| 0x00590a50 | CSWSCombatRound::AddAttackAction | med | `new(0x88)` + ctor `0x58C7D0`; callers `005685e5`, `006da9c6` |
| 0x005905f0 | CSWSCombatRound::CalculateOnHandAttacks | med | `+0xAE0 = 0x6A7170() + min(+0xAF4,2) + bonuses` |
| 0x005df920 | CSWSCreature::AIActionType47_ReequipResumeAttack (type 0x47) | low | table entry `0x5D682A` -> `CALL 0x005df920`; body per AI4 |

## 5. Probe-window notes (checked against `.asm`; inbound-target scan over every `.asm`)
| window | len / stolen | boundary | rel branch inside | targets inside / at start | notes |
|---|---|---|---|---|---|
| CA-FM2 `0x5C2438` | 9 `8B 95 6C FA FF FF 89 51 70` | OK | none | none / none | Consumed exit `0x5C24C8` (`JMP 0x5C25A5`, then `MOV EAX,[EBP-0x58]`), so EAX is not read. Stolen bytes read ECX (=PI) and write PI+0x70: restore ECX. Sees FAIL legs only (1.1). Cold. |
| CA-FM2b `0x5BC5F4` | 9 `8B 4D E4 8B 91 24 01 00 00` | OK | none | none / `0x5BC5E2` JNZ, `0x5BC5E8` JZ (start, allowed) | Reached only for (other area \|\| !inRange). LOS is dead, so "LOS fail" never reaches it. Warm (per 0.3 s re-approach). Recommended FM2 trigger site. |
| **G-FM2c `0x55B69A`** (new) | 6 `8B 0D A0 B5 99 00` | OK | none (abs operand) | none / `0x55B407` (PI+0 != 0), `0x55B623` (no probe clear), fallthrough from `0x55B697` (partial granted) | Only reached when the search result was 3. Here `[EBP-0x18]` is 2 (partial) or 3 (fail), `[EBP-0x20]` = cached first code (-1 = skipped), `[EBP+8]` = PI (creature id `PI+0x2C`, target `PI+0x30`). Stolen writes ECX only. Cold. |
| AI1-H3 `0x55B625` | 13 `8B 4D 08 C7 81 8C 00 00 00 04 00 00 00` | OK | none | none / none | Handler runs before the stolen store (lesson 271): it sees the old `PI+0x8C`. Redundant with G-FM2c. |
| FM3 `0x51DBE1` | 6 `8B 4D D8 89 4D C4` | OK | none | none / none | Ends exactly where the unshipped hot window `0x51DBE7` starts: never ship both. |
| FM3 `0x51DD33` | 6 `8B 4D D0 2B 4D F0` | OK | none | none / `0x51DCF3`,`0x51DCF9`,`0x51DD22` | The stolen `SUB` produces CF for the later `SBB`. Safe only because KPM runs stolen bytes after the handler. |
| FM3 `0x51E0C9` | 6 `8B 45 08 8B 48 78` | OK | none | none / none | cold |
| FM5 R1 `0x5C8BC2` / `0x5C9F9D` / `0x5C3D8B` | 6 / 5 / 5 (`8B 0D A4 B4 A1 00`, `A1 A4 B4 A1 00` x2) | OK | none | none / none | After the payload word store. Payload pointer live in EAX (`005c8bbf MOV word [EAX],DX`) / EDX (`005c9f9a`) / EDX (`005c3d88`); stolen bytes do not clobber it, so a retype-to-0 handler can use it or the frame local. |
| FM5 AI1-H9a/b/c `0x5C124E` / `0x5C5A76` / `0x5CCBC2` | 6 / 5 / 6 | OK | none | none / none | Payload in ECX (`005c124b`, `005ccbbf`) / EDX (`005c5a73`); H9c stolen `MOV EDX,[EBP-0xD0]`. All six sites are needed for a complete FM5 throttle. |
| P2 `0x56BE79` | 6 `8B 85 2C FF FF FF` | OK | none | none / `0x56BE68`, `0x56BE6E` (start) | HOT (every AIUpdate of every creature): keep the handler to a few compares. 0x90 bytes from the shipped `0x56BDE3`. |
| RunActions dispatch `0x5D5C22` | 6 `8B 95 4C FE FF FF` | OK | none | none / none | HOT (per action). The next insn is the indirect `JMP [EDX*4+0x5D7038]` (not stolen). The type is in `[EBP-0x1B4]` (type-1), EDX is not yet loaded at handler time. |
| RunActions result `0x5D6A60` | 9 `8B 55 E4 3B 15 C0 44 99 00` | OK | none | none / none | The stolen CMP feeds `JNZ` (stolen bytes last). HOT. Gives the type x result histogram. |
| Party-flag census `0x58A35E` | 6 `89 88 AC 11 00 00` | OK | none | none / none | The only runtime writer of `+0x11AC` (census: ctor `0056214c` + this). Handler sees the OLD flag at `[EAX+0x11AC]` and the new value in ECX (lesson 271). Cold. The entry `0x58A330` has 15 call sites. |
| AI3-B1 `0x57900F` | 1-byte simple patch `2B` -> `03` | insn `2B 4D F0` OK | n/a | none | KPM simple hook (no detour). Cold. User decision (balance). |
| E2 (installed) `0x569A08` | 6 | OK | none | none / none | unchanged |
No candidate overlaps the live hooks (`live_hooks_2026-09-30g.tsv`: perf-lab `0x55AB30`, `0x55AC26`, `0x51D790`, `0x51DC70`, `0x51DC86`) or the shipped companion/ranged tomls (`0x56BDB4/BD/E3`). All are more than 12 B from the sibling self-patch list. Suspicious items: only the two noted above (`0x51DBE1` adjacent to `0x51DBE7`; `0x51DD33` relies on stolen-bytes-last).

## 6. Lessons to add (scope / topic / note <= 600 chars / anchor)
1. combat-ai / FM2b: unreachable target = partial leg then FAIL loop / `0x55AC80` returns 2 for an unreachable goal via (a) the straight bisection fallback (`0x55B3E9..0x55B692`, needs PI+0 == 0 and cached start->goal code != -1, 6 probes, 2-point path) or (b) the grid closest-result rule (`0x550E30` l.238-250). The walker end calls ResetRequest (`0x5C3EFE`), so every 0x11 re-approach re-plans. From the obstacle the probes fail and give 3: the 2-4 s wait at `0x5C2484`. FM2 and FM2b are phases of one loop. Key fail-fast on 0x11 re-approach (`0x5BC5F4`). Anchor: GATE_wave1 1.1.
2. combat-ai / WalkLineTest codes / `0x550980`: 1 clear; 0 = wall (perimeter edge without room transition), 100 steps, start not in a room; -1 = face-walk failure (`0x651090`) or swept collision-mesh failure (`0x5EA8F0`, swcollisionmesh.cpp); -2 = door/placeable box (`0x52D130`); -3 = creature on the line. A cached -1 disables the partial fallback (`0055b4fb CMP [EBP-0x20],-1`).
3. combat-ai / Action 0x11 has no LOS / `0x5BC1D0`: `005bc4b8 XOR EDX,EDX` / `005bc4ba JZ 0x5bc5de` is always taken, so the LOS + range-halving block `0x5BC4C0..0x5BC5D7` is dead. CheckMoveToObjectRadius = same area && GetIsInUseRange(range - target radius). LOS only matters in AIActionPhysicalAttacks (`+0x558`). AI1 misread it; R1 was right.
4. method / `XOR r,r` + `Jcc` is an unconditional jump: before calling a block live, check the flag source of the branch that guards it. `XOR reg,reg; JZ` (ZF=1) is the compiler's form of `if (0)`. Inbound branches into the block from elsewhere decide whether it is dead (grep every `.asm` for targets). Example `0x5BC4B8` (lesson 3).
5. combat-ai / OnBlocked has 6 post sites; door vs trigger in MoveToPoint: payload 0x1F at `0x5C1240` (5BFC00 door case 0xA), `0x5C3D7D`, `0x5C5A68`, `0x5CCBB4`, `0x5C8BB4`, `0x5C9F8F`, all delta-0, none throttled. 5BFC00 switch `JMP [ECX*4+0x5C25BC]` (index type-5): type 7 trigger -> `0x5C0410` (posts 0x1E at `0x5C0732`), type 0xA door -> `0x5C1143`. R1 had 3 sites and called `0x5C0732` the door.
6. perception / spot distance sign bug: DoSpotDetection `0x578850` adds `+ftol(d/3-2)` past 6 m (`00579007 NEG EAX`, `0057900f SUB ECX,[EBP-0x10]`; consts `0x98B968`=3.0, `0x987490`=2.0). The margin `[EBP-0x48]` is spotted iff > 0, and all other adverse terms SUB. Listen subtracts (`005787b9`). Stealthed targets are easier to spot far away. 1-byte fix `0x57900F` 2B->03 (KPM simple). Balance change = user decision.
7. perception / UpdatePerception mode 1 never calls `0x522AF0`: `0x5D0F30` mode 1 walks the party table (non-party perceivers only, `param_1[0x46b]==0`). Modes 0/2 refresh the existing list, then scan `area+0x194` from `0x522AF0`, whose 0 return skips the discovery and the `+0x11C8` reset. The R1 abort cannot block enemy aggro on the party.
8. scheduler / `+0x220` = player update-set flag: CSWSObject `+0x220` is written only by `0x63C1E0` (`0063c257`) = `0x63DD30(obj, player) || ForceAlwaysUpdate(+0x260)`. 1 for self/party, for creatures in perception range and not invisible, or seen/heard by the player. It gates the UpdateState pre-pass QuickWalk and the client sync. It is NOT a party flag (U3 was wrong); visible enemies are included.
9. scheduler / action table `0x5D7038`: RunActions `JMP [EDX*4+0x5D7038]` at `0x5D5C28`, index = type-1, 0x47 dword entries. Verified 1->0x5BFC00, 0xC->0x6D7390, 0x3F->0x6D9500, 0x3D->0x5BF7F0 (gated by leader valid + `+0x11AC`), 0x1E->0x5D9AA0, 0x11->0x5BC1D0, 0x47->0x5DF920. Types 0xD/0x39/0x3B/0x42 have no handler and are deleted as 3. Read the table from the vanilla exe (`.rdata` file offset = VA-0x986000+0x584C00).
10. scheduler / items are AI objects at level 0, but their AIUpdate is cheap: CSWSItem ctor `0x5FFD60` -> `AddObject(item,0)` (`0x5FFE52`). Item vtable `0x9972CC` (at item+0x10) slot 0x70 = `0x600F20`, which only scans the item's property list for type 10 (timed subtypes 0xE..0x12). The per-item cost is the scheduler pre-pass lookup per loop iteration (AIQ), not the item tick.
11. combat-ai / slot-6 also fires when a SetState effect ends: `OnApplySetState 0x59CBA0` (effect type 8: DroidStun 3, Choke 7, Horrified 8, Whirlwind 10 ...) creates an internal type-9 effect and sets creature `+0xFF5` = state. Its removal `0x59E860` (remove table idx 9, `0059414c`) runs slot 6 (OnEndRound) for states 1, 3-10, 0xE-0x13: no leader/dead/combat gate. It also fires when a stronger state replaces a weaker one.
12. combat-ai / range metrics: the attack decision in `0x6D7390` uses 2D centre dist2 (`006d7e4a FLDZ` zeroes z) vs (MaxAttackRange+0.1)^2. Melee reach = 1.6 + HITRADIUS(att) + HITRADIUS(tgt) + 0.5 (appearance HITRADIUS, not PERSPACE). GetNearestVisibleEnemyInRange `0x57C990` accepts by 3D length minus both `PI+8` radii (`0057cd80`, `0057cdc0`). A strict retarget must recompute the 2D test.
13. combat-ai / grid search returns a closest path as SUCCESS: `0x550E30` sets `PI+0x24=1` every call. When an iteration crosses 4x the slice, or 1x with no best-distance gain, and the best path has >= 2 points (`PI+0x188`) and the 64-expansion abort (`area+0x1D0`) is clear, it returns 2 with the best path. Do not read the "Exceeded limits" strings as "no path => FAIL".
14. scheduler / dismissed party member -> AI level 2: `SetPartyMemberFlag(0, arg2!=0)` sets level 2 (`0058a448 PUSH 0x2` / `0058a458 CALL 0x51e120`), not 0/1. Its own next AIUpdate demotes it to 0/1 (`0x56A0FE/0x56A111`), a self-transition (FM3 skip-one-object effect). `+0x11AC` has one runtime writer (`0x58A35E`).

## 7. Open items handed to wave 2
- `0x585CC0` GetIsInUseRange metric (2D/3D, radii) vs the 0xC approach test: ping-pong candidate.
- Runtime split of partial-vs-fail and the -1 share: G-FM2c `0x55B69A` + AI1-H3. Path-point search (`0x5532F0`) outcome rules for unreachable goals were not read; whether it uses the closest-result rule through its grid legs is open.
- `0x5EA8F0` (14.7 KB swept collision test): how often a straight line to a target behind geometry yields -1 rather than 0.
- `0x57E280` purpose; `+0xFF4` bit names; the state-id -> effect map for ids 1, 2, 4-6, 9, 0xB-0x13.
