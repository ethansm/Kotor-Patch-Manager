# Companion/party AI engine fixes: KPM detour design (A1, P1, FM1, FM4, A4, C1; optional A3, FM3)

2026-10-01. Design only: nothing was built, installed or run, and Ghidra was not run. Steam Aspyr `swkotor2.exe` 6A522E71 (base) and 4AB72FC1 (LAA). Every window below is byte-identical in both executables (see "Window check").

Sources:
- `mods678_research/ai_review_2026-10-01/review_ai_engine.md` and its `README.md`;
- `13_ranged_ai_spacing.md` and `ranged_ai_src/` (E-H1/E-H2/E-H3 and the INI/log pattern);
- the dump `ghidra_dumps/steam_6a522e71/decomp/` (every quoted line is from the `.asm`);
- `KotorPatcher/src/core/wrapper_x86.cpp`;
- DB lessons 29, 172, 203 and 210.

## 0. Framework rules applied (re-read in `wrapper_x86.cpp`)

**Order of the wrapper:**
1. PUSHAD and PUSHFD, then FXSAVE.
2. cdecl call to the handler.
3. Restore the registers. EAX is not restored when it is in `exclude_from_restore`.
4. Run the stolen bytes, unless `skip_original_bytes=true`.
5. If a consumed exit is set: `PUSHFD; TEST EAX,EAX; JZ; POPFD; JMP exit; POPFD`.
6. `JMP` to `hook + len`.

**Consequences:**
- **(a) Stolen bytes.** In a consumed hook the stolen bytes run with EAX equal to the handler's result.
- **(b) Fall-through, a rule not written down before.** A consumed hook must exclude EAX from the restore. So on the fall-through path, EAX at the resume address is also the handler's result (0), not the game's value. The resume address therefore needs EAX dead as well. My copy of `check_windows.py` now checks EAX liveness on every path from both the resume address and the consumed exit.
- **(c) Skip-only windows.** Their stolen bytes never run, so they may contain a relative CALL. The handler must perform whatever those bytes did.
- **(d) Parameter sources.**
  - `esp+N` passes the address of the game's stack slot.
  - `[reg+N]` passes the value stored there.
  - The handler can write the game's locals through `ebp`, and pushed arguments through `esp+N`.
- **(e) Byte patches.** KPM `type="simple"` byte patches exist (for example K2FinesseMeleeFix) but are unconditional, so they cannot be INI-switched. Every fix here is a detour with an INI flag, hot-reloaded once a second like `ranged_ai.ini`.

**Correction to the triage README (A2), from the asm.** The 3 s fallback in `AIUpdate` has no "queue empty" test.
```
005699e1  83 BA 70 05 00 00 00   CMP [EDX+0x570],0x0 ; JG 0x569a7c
005699fc  8B 82 94 00 00 00      MOV EAX,[EDX+0x94] ; CALL EAX ; TEST EAX,EAX ; JNZ 0x569a6c
```
- Slot `vt+0x94` of the CSWSCreature vtable `0x994F5C` is `0x563460`: it returns 0 for party members, and otherwise `0x5477F0` (HP < 1 or HP < -9), so it is a GetDead test.
- So the slot-6 script fires every 3 s in combat even with actions queued, unless something keeps resetting `+0x570`.
- This matters for FM1 and the A1 re-think timing below.

## 1. Placement (recommendation)

**New patch id `companion-ai`** (DLL `CompanionAI.dll`, `companion_ai.ini`, `companion_ai_log.txt`) holds:
- every fix in this document;
- the AI trace layer being designed in parallel (one DLL, one INI, one shared log helper).

Reasons:
- These fixes change **vanilla** behaviour (stances 11/12, enemy perception, follow, save, crash guards). `ranged-ai` is the opt-in stance-16 feature; keeping them apart keeps each rollback independent.
- **Ordering with E-H3 is solved by site choice.** The A1 hook sits at `0x6D853E`, after E-H3 (`0x6D850F`) has written the effective stance `[ebp-0x174]`. Nothing branches into `0x6D8500..0x6D854F` (checked), so E-H3 always runs first. companion-ai therefore works unchanged with ranged-ai 0.2.0, with 0.3.0, or with no ranged-ai at all. The reviewer's `0x6D84FC` site runs **before** E-H3. It would have to guess ranged-ai's INI (`ranged_rules`) to know whether stance 16 becomes 11, which couples two DLLs.

**`ranged-ai` 0.3.0** gets one handler change only: E-H3 `kite_approach`, A1 part B. There are no new windows, so the hooks.toml is unchanged and only the DLL is rebuilt.

**Coordination with the trace-layer design** (its windows are in the shared scratchpad `check_windows_ai_trace.py`):
- Its **E4 (`0x586F43`, 5 bytes)** overlaps my FM1 window (`0x586F43`, 10 bytes, skip-only). Drop E4: the FM1 handler is the single hook there and emits the bump log line.
- Its E1 (`0x6D740F`), E1b (`0x6D8BE2`), E2 (`0x569A08`) and E3 (`0x56BDBD`) do not overlap anything here. E3 sits between my two P1 windows: `0x56BDB4+7`, then `JC` at `0x56BDBB`, then E3 at `0x56BDBD+8`, then my `0x56BDE3`.

Manifest `companion-ai`: `requires = []`, `conflicts = []`, `supported_versions` = both SHAs (copy from ranged-ai). Every hook below is `type = "detour"`.

## 2. INI (`companion_ai.ini`, section `[CompanionAI]`, re-read at most once a second)

```
; A1: Ranged/Stationary attack self-requeue
a1=1                    ; 0 = vanilla spin (log only)
a1_stances=11,12,16     ; RAW stance (stats+0x146) the fix applies to; the spin needs effective stance 11/12 (16 only via E-H3)
a1_party_only=1         ; 1 = party members only (+0x11AC); 0 = any creature that would spin
a1_retarget=0           ; 1 = before ending, take the nearest visible enemy in weapon range (vanilla PC tail)
a1_retarget_stances=11,12,16
a1_rethink_ms=3000      ; < 3000 = shorten the slot-6 fallback after an A1 end; 3000 = leave the engine value
; P1: perception throttle stamp
p1=0                    ; 1 = store the mode-1 perception stamp (behaviour change: see risks)
p1_interval_ms=250      ; throttle for non-party creatures perceiving the party (vanilla design constant 4000)
; FM1: leader bump
fm1=1
fm1_rethink_ms=0        ; >0 = in combat, cap +0x570 to this after a bump clear (0 = engine's own <=3 s)
; FM4: SaveCreature FollowInfo reset
fm4=1                   ; 0 vanilla, 1 snapshot/restore live FollowInfo (save bytes unchanged), 2 apply=0 (reviewer's 1-byte semantics)
; A4: NULL spell row in AIActionCombat
a4=1
; C1: ClearHostileActionsVersus restart index
c1=1
; optional
a3=0                    ; 1 = clear the orientation lock when the attack ends on a dead/undetected target
fm3_probe=0             ; log-only scheduler sentinel probe
log=1
log_all=0               ; 1 = also log non-party creatures for A1/FM1
```

**Ship defaults.** Use the values above. Stage 0 (first boot after install) is `a1=0 fm1=0 fm4=0 c1=0 p1=0 a4=1`, which is log-only except for the crash guard. Then flip the flags with no restart (section 10).

## 3. A1: Ranged/Stationary attack re-queues itself forever (HIGH)

### Bug (asm, `AIActionPhysicalAttacks 0x6D7390`, approach branch)

```
006d850f  89 85 8C FE FF FF        MOV [EBP-0x174],EAX          ; stance copy (E-H3 live, skip-only)
006d853e  C7 85 94 FE FF FF 00 00 00 7F  MOV [EBP-0x16C],0x7F000000   ; controlled id default
006d8548  83 BD 88 FE FF FF 00     CMP [EBP-0x178],0x0          ; p = 0x7E5DA0(...) party table
006d8557  E8 A4 52 0A 00           CALL 0x0077d800              ; q = controlled client creature
006d856b  89 8D 94 FE FF FF        MOV [EBP-0x16C],ECX          ; ctrl id = [q+4]
006d8729  E8 72 85 E6 FF           CALL 0x00540ca0              ; AddActionToFront(0xC attack), unconditional
006d8738  83 BD 98 FE FF FF 00     CMP [EBP-0x168],0x0          ; back-off flag (party: always 0)
006d876a  83 BD 8C FE FF FF 0B     CMP [EBP-0x174],0xb  / 006d8773 CMP ..,0xc
006d8791  3B 95 60 FC FF FF        CMP EDX,[EBP-0x3A0]          ; ctrl id vs attacker id -> JNZ skips 0x11
006d8842  E8 59 84 E6 FF           CALL 0x00540ca0              ; AddActionToFront(0x13 facing), unconditional
006d89c2  83 7D C4 00              CMP [EBP-0x3C],0x0 ; JNZ 0x6d8be2
006d8a7e  CMP [EBP-0x174],0xb / 006d8a87 CMP ..,0xc / 006d8aa5 CMP EDX,[EBP-0x3C8] ; JNZ 0x6d8be2 (skip the move)
006d8b18  E8 93 B9 E8 FF           CALL 0x005644b0              ; AddMoveToPointAction (all other stances)
006d8be2  A1 B8 44 99 00           MOV EAX,[0x009944b8]         ; return 2
```

**Exact spin condition:**
- back-off `[ebp-0x168]==0`;
- `[ebp-0x3C]==0` (always true here: `0x6D84E5` jumps away otherwise);
- `[ebp-0x174]` is 11 or 12;
- the controlled id differs from the attacker id.

The queue becomes `[0x13, 0xC]` with no move (type 1 is what stops `RunActions`). The prologue `0x6D740F MOV [EAX+0x570],0xBB8` resets the fallback on every pass, so neither the end-of-round script nor the 3 s fallback ever runs.

### Desired behaviour per stance (decided)

| raw stance | weapon | target | engine today | new |
|---|---|---|---|---|
| 11 Ranged (vanilla) | any | out of range, or blocked shot, or melee weapon not in reach | spin | **end the action** (optionally retarget first). The 3 s fallback re-runs `k_ai_master`, which does `GN_CheckRangedPreferred` (weapon swap), a new target, powers, and so on. Never approaches (vanilla design kept). |
| 12 Stationary | any | same | spin | **end** (optionally retarget to an in-range visible enemy). Never moves. |
| 16 Ranged (Kiting), ranged-ai 0.3.0 `kite_approach=1` | ranged, target in range | other approach reasons (blocked shot not held by E-H1, door/placeable target, area) | mapped to 11, spin | E-H3 maps to 11, then A1 **ends**; the kite script repositions. |
| 16 | ranged, target **out of range** | | mapped to 11, spin | E-H3 keeps 16, so the vanilla Aggressive approach runs: a move with range `max` (`0x6D89D5` picks `[ebp-0x2C]` when `[ebp-0x164]` ranged). It walks until in weapon range, then shoots. |
| 16 | **melee** (after the kite script's "melee if cornered" swap) | | mapped to 11, spin | E-H3 keeps 16, so the Aggressive melee approach runs. |
| 16 with ranged-ai 0.2.0 or `kite_approach=0` | any | | mapped to 11 | A1 ends and the 3 s re-think follows. The script cannot close distance, so it idles but re-thinks. |
| controlled creature, 0/9/10/13 | | | vanilla approach | unchanged (the handler returns 0) |

### A1 part A (companion-ai): hook `CA-A1` at `0x6D853E`

| field | value |
|---|---|
| address / len | `0x006D853E` / 10 |
| stolen bytes | `C7 85 94 FE FF FF 00 00 00 7F` (`MOV [EBP-0x16C],0x7F000000`, EAX-free, still runs) |
| skip | false |
| consumed exit | `0x006D7D2F` (vanilla PC retarget tail), `exclude_from_restore=["eax"]` |
| params | `ebp` (int) |
| EAX at resume `0x6D8548` | dead on all paths (checked) |
| EAX at exit `0x6D7D2F` | dead on all paths (checked) |

**The consumed exit is the vanilla PC retarget tail** (`0x6D7D2F..0x6D7DC1`):
```
006d7d2f  CMP [EBP-0xE8],0x7F000000 ; JZ 0x6d7daf      ; none -> EndAttackAction(1) ; return 2
006d7d3b  ...0x51C0A0 GetGameObject(id) ; type == [0x994474] (creature) else EndAttackAction ; return 2
006d7d9e  CALL 0x006dac70  ; InitializeNewAttackTarget(id, [EBP+8]=node) (sets node+0x38, lock, +0x52C=1)
006d7da3  MOV EAX,[0x009944b4] ; JMP epilogue   ; return 1 (node kept, attacks the new target next tick)
```
One exit covers both outcomes: the handler writes `[ebp-0xE8]` as the new target id, or INVALID to end. ESP is balanced at both sites, and the tail needs only EBP. On a later approach for the new target, `+0x52C=1` makes the vanilla `0x6D83F6` test end the action instead of spinning.

**Game calls** (conventions from the asm):
- `0x77D800` thiscall(partyTable) → client creature, or 0 (`RET`).
- `0x57C990` thiscall(creature; float range, int excludeId, int refId) → id (`RET 0xC`). The vanilla PC path passes `[ebp-0x14]` (current target) and `0x7F000000`.

**Handler:**
```
extern "C" int __cdecl CompanionAiA1Gate(int ebp)
  attacker = [ebp-0x49C]; eff = [ebp-0x174]; p = [ebp-0x178]
  if (eff != 11 && eff != 12) return 0                       // not a spin: vanilla move follows
  if ([ebp-0x168] != 0 || [ebp-0x3C] != 0) return 0          // back-off / force-jump: vanilla
  ctrl = 0x7F000000; if (p) { q = thiscall 0x77D800(p); if (q) ctrl = [q+4] }   // = vanilla 0x6D8548..0x6D856B
  if (ctrl == [attacker+4]) return 0                         // controlled: vanilla moves
  raw = u16 [[attacker+0x1198]+0x146]; party = [attacker+0x11AC]
  reason = classify(dist2=[ebp-0xBC], max=[ebp-0x2C], los=[ebp-0xD0], ranged=[ebp-0x164],
                    sameArea = GetArea(attacker)==[ebp-0x38])      // "range" | "los" | "melee" | "area" | "other"
  track(attacker).wouldSpin++                                // the A1 confirm counter (also when a1=0)
  if (!a1 || !mask(a1_stances, raw) || (a1_party_only && !party)) { log(throttled, "spin"); return 0 }   // vanilla spin
  newId = 0x7F000000
  if (a1_retarget && mask(a1_retarget_stances, raw) && max > 0.5f)
      newId = thiscall 0x57C990(attacker, max, [ebp-0x14], 0x7F000000)
  [ebp-0xE8] = newId
  if (newId == 0x7F000000 && a1_rethink_ms < 3000 && [attacker+0x570] > a1_rethink_ms) [attacker+0x570] = a1_rethink_ms
  log(throttled, newId ? "retarget" : "end")
  return 1                                                   // -> 0x6D7D2F
```

**Parity:**
- **`a1=0`:** the handler only reads locals and calls `0x77D800`, the same pure getter vanilla calls 10 instructions later. It returns 0, the stolen store runs, and execution resumes at `0x6D8548` with every register restored except EAX, which is dead there.
- **`a1=1`:**
  - Only creatures that would spin are touched: effective stance 11/12, not controlled, approach branch, party members by default.
  - Instead of re-queueing `[0x13, 0xC]` and returning 2 forever, the action ends through the vanilla `EndAttackAction(1)` (lock cleared, ready animation, combat mode off, special attacks cleared) and returns 2, exactly what vanilla does when the PC's retarget finds nothing.
  - `+0x570` then counts down from the 3000 set at this entry, so the slot-6 script runs within 3 s.
  - With `a1_retarget=1`, the action switches to the nearest visible enemy within weapon range (current target excluded), as the PC does on a blocked shot.
- **Who:** effective stance 11/12 is set only by the party stance menu and `SetNPCAIStyle(11/12)` (the `NPC_AISTYLE_PARTY_*` constants), so in practice party members only. `a1_party_only` makes that explicit.
- **Save:** none. `+0x52C` and `node+0x38` are written by the vanilla `InitializeNewAttackTarget`; the queue that gets saved is the vanilla one, minus a spin.

**INI:** `a1`, `a1_stances`, `a1_party_only`, `a1_retarget`, `a1_retarget_stances`, `a1_rethink_ms`.

**Log lines:**
```
A1 t=123456 cr=8000021c party=1 raw=11 eff=11 reason=range dist=24.3 max=20.0 los=1 ranged=1 area=same -> end new=7f000000 (wouldSpin/s=412)
A1 t=... cr=... raw=16 eff=11 reason=los ... -> retarget new=80000431
A1 summary: cr=8000021c would-spin passes=4120/10s ends=3 retargets=1
```
**Confirm:**
- With `a1=0`, `would-spin` in the hundreds per second for one creature is the runtime proof of A1, which the review still lacks.
- With `a1=1`, expect one `end` line per ≤3 s per stuck companion, and the trace layer's E2 (slot-6 fallback fires) right after it.

**Risks:**
1. A creature that stays out of range re-thinks every 3 s. The vanilla Ranged script may pick the same target every time (no approach by design), so the idle shrinks from "forever" to "3 s cycles", plus whatever powers or items the script uses. `a1_retarget=1` removes most of the remainder.
2. **Combat state.** If nobody attacks the idling companion, `+0x520` times out after 8 s (`+0x524`) and the fallback stops (it requires `+0x520==1`). Recovery then depends on perception and heartbeat. Watch the log for `end` lines that are not followed by an E2 line.
3. **`0x57C990` cost** (it does an LOS test). It runs at most once per action end, not per frame.
4. **Retarget gaps.** `0x57C990` needs the seen bit, and companions refresh perception only every 4 s (P2), so a retarget can miss a fresh enemy. The fall-back is "end", which is safe.
5. **EndAttackAction side effects.** It turns combat mode off, which is visible as the weapon lowering until the next attack. This is the same as vanilla's E-H1 hold and the PC's "nothing to retarget" path.

### A1 part B (ranged-ai 0.3.0): E-H3 handler change, same window

E-H3 stays at `0x006D850F`, 6 bytes `89 85 8C FE FF FF`, skip-only, params `eax`, `ebp`. `[ebp-0x164]` (ranged, `0x6D836B`), `[ebp-0xBC]` (dist²) and `[ebp-0x2C]` (max) are written before E-H3 on every approach path.
```
extern "C" void __cdecl RangedAiStanceRules(int stance, int ebp)
  map = stance == 16 && g_cfg.rangedRules
  if (map && g_cfg.kiteApproach) {
      mx = f[ebp-0x2C]; lim = (mx + 0.1f)^2
      inRange = mx > 1.0f && f[ebp-0xBC] <= lim
      if (![ebp-0x164] || !inRange) map = false     // melee or out of range: Aggressive approach (move range = max for ranged)
  }
  [ebp-0x174] = map ? 11 : stance
```
- **INI:** `kite_approach=1` in `ranged_ai.ini`; 0 reproduces 0.2.0.
- **Parity:** stance 16 is opt-in and nothing else changes. Vanilla stances are untouched.
- **Log** (throttled, first 20, then on change): `E3 t=.. cr=.. stance=16 ranged=1 inRange=0 -> 16 (approach)`.
- **Risks:**
  - Stance 16 now walks toward an out-of-range target. That is bounded by weapon range, and the kite routine's 10 m leader leash (`GN_CheckRangeFromLeader(10)`) still runs every round.
  - The kite/approach ping-pong is unlikely: kiting triggers within 3.5 m, and approach only beyond `max` (about 20-30 m for blasters).

## 4. P1: perception throttle stamp never stored (MEDIUM)

### Bug (`SpawnInHeartbeatPerception 0x56B8A0`, block 2; called with 1 on every `AIUpdate`, `0x5694DB PUSH 1 ... 0x5694E3`, for every creature)

```
0056bcd7  E8 44 F0 FA FF          CALL 0x0051ad20      ; now -> day [EBP-0x24], ms [EBP-0x20]
0056bcf9  8B 88 A4 03 00 00       MOV ECX,[EAX+0x3a4]  ; stamp ms
0056bd06  8B 82 A0 03 00 00       MOV EAX,[EDX+0x3a0]  ; stamp day
0056bd22  E8 39 F2 FA FF          CALL 0x0051af60      ; elapsed: days -> [EBP-0x1C], ms -> [EBP-0x28] (returns -2, no write, if now < stamp)
0056bd40  74 7B                   JZ 0x56bdbd          ; +0x38C == 0 (spawn perception not done yet): always run
0056bd5b  0F 85 88 00 00 00       JNZ 0x56bde9         ; party: skip block 2
0056bdb4  81 7D D8 A0 0F 00 00    CMP [EBP-0x28],0xfa0 ; non-IsPC: 4000 ms
0056bdbb  72 2C                   JC 0x56bde9
0056bdbd  6A 01 ... E8 66 51 06 00  CALL 0x005d0f30    ; UpdatePerception(1)
0056bde3  89 90 9C 03 00 00       MOV [EAX+0x39c],EDX  ; only the interval is re-rolled; the stamp is never written
```
- The stamp `+0x3A0/+0x3A4` is written only by the ctor, so `elapsed` is measured from day 0 and is always ≥ 4000.
- Every non-party creature runs `UpdatePerception(1)` on every tick: a party walk with an LOS raycast per member.
- `0x51AF60` compares only the ms component; days go to `[ebp-0x1C]`.

### Fix (two hooks, both plain detours: no consumed exit, all registers restored, so the stolen bytes may use EAX)

**CA-P1i, interval:**

| field | value |
|---|---|
| address / len | `0x0056BDB4` / 7 |
| stolen bytes | `81 7D D8 A0 0F 00 00` (`CMP [EBP-0x28],0xFA0`; the following `JC` reads its flags, and the stolen bytes run after the handler) |
| skip | false |
| consumed exit | none |
| params | `ebp` (int) |

```
extern "C" void __cdecl CompanionAiP1Interval(int ebp)
  c = [ebp-0xD4]; count(c)                                  // P1 rate counter (works with p1=0)
  if (!p1) return                                           // vanilla
  nd=[ebp-0x24]; nm=[ebp-0x20]; sd=[c+0x3A0]; sm=[c+0x3A4]
  if ((sd|sm) == 0 || nd < sd || (nd == sd && nm < sm)) run = 1     // never stamped, or the clock went back: resync
  else run = [ebp-0x1C] != 0 || (u32)[ebp-0x28] >= p1_interval_ms
  [ebp-0x28] = run ? 0xFA0 : 0                              // the stolen CMP / JC then decide; [ebp-0x28] has no later reader
```
`[ebp-0x28]` is read only at `0x56BD88` (IsPC path, earlier) and `0x56BDB4` in this function after block 1. The time pair `[ebp-0x24]/[ebp-0x20]` is written at `0x56BCD7` and only read afterwards, by block 3.

**CA-P1s, stamp:**

| field | value |
|---|---|
| address / len | `0x0056BDE3` / 6 |
| stolen bytes | `89 90 9C 03 00 00` (`MOV [EAX+0x39C],EDX`; EAX and EDX are restored, so it is safe) |
| skip | false |
| consumed exit | none |
| params | `ebp` (int) |

```
extern "C" void __cdecl CompanionAiP1Stamp(int ebp)
  if (!p1) return
  c = [ebp-0xD4]; [c+0x3A0] = [ebp-0x24]; [c+0x3A4] = [ebp-0x20]    // pairing as pushed at 0x56BD06/0x56BCF9
```
This hook is reached only after block 2 actually ran `UpdatePerception(1)` (straight-line `0x56BDBD..0x56BDE3`, no other entry).

**Parity:**
- **`p1=0`:** both handlers return before writing, so execution is vanilla and only counting happens.
- **`p1=1`:** non-party, non-IsPC creatures (enemies, neutrals, ambient NPCs) re-perceive the party every `p1_interval_ms` instead of every AI tick. The IsPC branch (`+0x39C`, 300..699 ms) also starts working; no non-party IsPC creature is known.
- **Unchanged:** party members, the first spawn perception (`+0x38C==0`), block 3, and every event path.
- **Save:** none; `+0x3A0/+0x3A4` are not serialized (the reviewer found no reader or writer outside the ctor and block 2).

**INI:** `p1` (ship 0, test 1), `p1_interval_ms` (250; 4000 = the original design constant).

**Log:** `P1 summary: non-party creatures=37 perception-1 runs=21140/10s (57/s per creature) interval=off`, then with `p1=1`, `runs=1480/10s (4/s per creature) interval=250`.

**Confirm:** compare against the trace layer's E3 (`0x56BDBD`) and the perf-lab AI tick time.

**Risks (behaviour change: playtest gate):**
1. **Reaction timing.** Enemies have reacted on the next frame for 20 years. 250 ms keeps reactions faster than a human-visible round; 4000 (the designers' constant) would make enemies sluggish.
2. **Stealth.** The reviewer reports that `DoPerceptionUpdateOnCreature 0x576DC0` does the spot/listen rolls (medium confidence, sub-review). Today, enemies roll against a stealthed party every tick, so detection odds scale with frame rate. With P1 they roll every `p1_interval_ms`, so stealth gets noticeably stronger and frame-rate independent. **Test** a stealth approach on a known guard before shipping `p1=1`.
3. **Losing sight** of the party is also throttled. Expect up to `interval` of extra "still sees you" time.
4. **CPU.** Should drop (enemies × party raycasts per frame); measure it with perf-lab.

## 5. FM1: leader bump runs `ClearAllActions(1)` on a companion (MEDIUM)

### Bug (`BumpFriends 0x586B70`)

```
00586f0d  0F B7 02                MOVZX EAX,word ptr [EDX]   ; scan the bumpee queue (+0x100) for type 1 / 0x11
00586f39  83 7D 84 00             CMP [EBP-0x7C],0x0
00586f3d  0F 85 88 00 00 00       JNZ 0x586fcb               ; already moving: leave it
00586f43  6A 01                   PUSH 0x1
00586f45  8B 4D 08                MOV ECX,[EBP+0x8]          ; bumpee
00586f48  E8 33 A1 FB FF          CALL 0x00541080            ; ClearAllActions(1)  (RET 4)
00586fc6  E8 E5 D4 FD FF          CALL 0x005644b0            ; AddMoveToPointAction(push-aside spot) -> 0x540CA0 AddActionToFront
```
`AddMoveToPointAction` already queues the push-aside move at the **front** (`0x5645E4 CALL 0x540CA0`). So if the clear is skipped, the move runs first and the rest of the queue survives behind it.

### Fix: hook `CA-FM1` replaces the clear

| field | value |
|---|---|
| address / len | `0x00586F43` / 10 |
| stolen bytes | `6A 01 8B 4D 08 E8 33 A1 FB FF` (PUSH 1; MOV ECX; CALL ClearAllActions) |
| skip | **true**: the handler performs the call itself, which avoids the unbalanced-PUSH problem of a 5-byte consumed window |
| consumed exit | none (resume `0x586F4D`; EAX is dead there and the vanilla call's return value is unused) |
| params | `ebp` (int) |

Game calls, as used by vanilla `BumpFriends` and `ClearAllActions`:
- `0x541080` thiscall(obj; int clearAttacks), `RET 4`;
- `0x521060` thiscall(list) → head data;
- `0x7A1720` thiscall(list) → head position;
- `0x58C370` thiscall(list; int* pos) → next data, `RET 4`.

**Handler:**
```
extern "C" void __cdecl CompanionAiBumpClear(int ebp)
  bumpee = [ebp+8]; mover = [ebp-0x158]; party = [bumpee+0x11AC]
  if (fm1 && party && queueOnlyFollow(bumpee + 0x100)) { log("keep"); return }      // move-aside goes in front, follow resumes after
  thiscall 0x541080(bumpee, 1)                                                     // vanilla
  if (fm1 && party && fm1_rethink_ms > 0 && [bumpee+0x520] == 1 && [bumpee+0x570] > fm1_rethink_ms)
      [bumpee+0x570] = fm1_rethink_ms
  log("clear")
queueOnlyFollow(list): data = 0x521060(list); pos = 0x7A1720(list); n = 0
  while (data && n < 64) { t = u16 [data]; if (t not in {0x3D FollowLeader, 0x40 CheckMoveToFollowRadius, 0x46 FollowOwner}) return false
                            data = 0x58C370(list, &pos); ++n }
  return n > 0
```

**Parity:**
- **`fm1=0`:** the handler makes the identical `ClearAllActions(bumpee, 1)` call with the same `this`, the same argument and the same stack depth, then resumes at `0x586F4D`. That is vanilla.
- **`fm1=1`:**
  - Party bumpees whose whole queue is follow actions keep their follow. They step aside (front-queued move) and then resume following, instead of standing until the next heartbeat (3.0-4.2 s).
  - Every other queue, such as combat attacks or casts, is cleared exactly as vanilla. Partially executed casts or attack rounds are never resumed out of order.
  - The optional `fm1_rethink_ms` shortens the in-combat recovery: the slot-6 fallback runs regardless of queued actions (section 0), so values below about 800 ms risk the script cancelling the push-aside move. Ship 0.
- **Who:** party members bumped by the controlled leader (`GetIsCreatureBumpable 0x586940`: mover = leader, blocker party or puppet). Puppets (`+0x11FC` only) stay vanilla.
- **Save:** none.

**INI:** `fm1`, `fm1_rethink_ms`.

**Log:** `FM1 t=.. bumpee=80000220 mover=80000001 party=1 queue=[3d] -> keep` or `... queue=[c,13] -> clear (rethink=0)`.

**Confirm:** walk the leader through a following companion and check that the log shows `keep`. The companion should be following again within about 1 s.

**Risks:**
1. Jostling: a follower that resumes immediately may get bumped again. Vanilla's own "already has a move queued" test (`0x586F3D`) prevents a re-clear while the move is pending.
2. `AIActionFollowLeader 0x5BF7F0` resuming after an inserted move is assumed to re-establish its walk state (it sets `+0x11B0=3` per call, M confidence). Watch for a companion that freezes after a bump with `keep`.

## 6. FM4: `SaveCreature` resets FollowInfo on every save (LOW)

### Bug (`SaveCreature 0x574500`)

```
0057451f  8B 88 AC 11 00 00       MOV ECX,[EAX+0x11ac]  ; saved party flag -> [EBP-0x38]
00574528  6A 01                   PUSH 0x1              ; apply=1
0057452a  6A 00                   PUSH 0x0              ; flag 0
00574532  E8 F9 5D 01 00          CALL 0x0058a330       ; SetPartyMemberFlag(0,1): frees +0x500 FollowInfo, SetAILevel(2)
0057515f  6A 01 / 00575164 PUSH [EBP-0x38]
0057516b  E8 C0 51 01 00          CALL 0x0058a330       ; (flag,1): new(0x3C) FollowInfo (defaults, ctor 0x5752C0/0x575340), SetAILevel(4)
0057517e  E8 2D 00 00 00          CALL 0x005751b0       ; SaveFollowInfo writes the fresh defaults (FollowObject, FollowLocation, LastLeaderPos, LastFollowerPos, MaxSpeed, StickToPos, Result, TimeElapsed, InSafetyRange)
```
**Byte patches.** KPM `simple` patches cannot be INI-switched, so all modes are detours. FollowInfo is POD: 15 dwords written by `0x575340` and freed with plain `operator delete` (`0x9196FD`), so a 0x3C-byte copy is safe.

### Mode 1 (recommended): snapshot and restore. Live state survives; the bytes on disk are unchanged.

**CA-FM4s, snapshot:**

| field | value |
|---|---|
| address / len | `0x0057451F` / 6 |
| stolen bytes | `8B 88 AC 11 00 00` |
| skip | false |
| consumed exit | none |
| params | `eax` (creature) |

```
extern "C" void __cdecl CompanionAiFm4Snap(int cr)
  g_snapCr = 0
  if (fm4 == 1 && [cr+0x11AC] && (fi = [cr+0x500])) { memcpy(g_snap, fi, 0x3C); g_snapCr = cr }
```

**CA-FM4r, restore:**

| field | value |
|---|---|
| address / len | `0x00575183` / 7 |
| stolen bytes | `8B 45 0C 50 8B 4D 08` |
| skip | false |
| consumed exit | none |
| params | `ebp` |

Placement: after `SaveFollowInfo` has written the defaults, so the save content stays vanilla. The stolen PUSH runs on the game's restored stack.
```
extern "C" void __cdecl CompanionAiFm4Restore(int ebp)
  cr = [ebp-0x1D4]
  if (fm4 == 1 && g_snapCr == cr && [cr+0x11AC] && (fi = [cr+0x500])) memcpy(fi, g_snap, 0x3C)
  g_snapCr = 0
```

### Mode 2 (alternative, the reviewer's one-byte semantics): `apply=0` on the first call

**CA-FM4a:**

| field | value |
|---|---|
| address / len | `0x0057452C` / 6 |
| stolen bytes | `8B 8D 2C FE FF FF` |
| skip | false |
| consumed exit | none |
| params | `esp+4`, type `pointer` (the address of the pushed `apply`) |

`if (fm4 == 2) *(int*)p = 0;` The first call then only clears `+0x11AC`. The second call neither reallocates nor changes AI level for a party member. For a non-party creature the second call does what vanilla's first call did, so the end state is identical. `SaveCreature` does not read `+0x78`/`+0x500` between the two calls (grep).

**Parity:**
- **`fm4=0`:** all three handlers return without writing, so execution is vanilla.
- **`fm4=1`:**
  - The save file bytes are identical to vanilla, because `SaveFollowInfo` ran before the restore.
  - In memory, a party member's FollowInfo (follow target, trail positions, MaxSpeed `+0x28`, InSafetyRange `+0x38`) survives the save instead of resetting to defaults. Saving no longer re-fires `k_hen_enter5m`/`exit5m` and no longer resets the run-rate cap.
  - The AI-level bounce 4→2→4 (end of the level-4 list) stays as vanilla.
- **`fm4=2`:** the live state survives as well, **and** the save now carries live FollowInfo. That changes the save format in practice: the loader `0x575200` → `0x561D80` has only ever seen defaults. On an area transition, positions from the old area would be restored in the new one. **Not recommended** without a dedicated load test.
- **Who:** party members passing through `SaveCreature`. Its 10 callers are `0x55D080`, `0x55D7F0`, `0x55DDF0` (module/game save paths, labels not verified), `0x5F7F30`, `0x5F8130`, `0x5F8320`, `0x5F8520`, `0x5F9FF0`, `0x5FA3B0` (party-table paths, labels not verified) and `MakePlayerCharacter 0x5FE4B0`.

**INI:** `fm4` (0/1/2).

**Log:** `FM4 t=.. cr=.. fi=0a1b2c00 before: obj=80000001 maxSpeed=5.40 safety=1 | after save: restored (mode 1)`; also log the same fields with `fm4=0` to confirm the reset.

**Risks:**
1. The snapshot is single-slot, keyed by creature pointer and cleared after use. Re-entrancy is not expected because saves are synchronous.
2. A FollowInfo whose follow object was destroyed in between (impossible inside one synchronous save) would be restored stale. The engine already tolerates stale ids there (`0x7F000000` checks).

## 7. A4: NULL spell row dereference in `AIActionCombat` (LOW, crash)

### Bug

```
006d9dcc  E8 7F 76 FE FF          CALL 0x006c1450     ; GetSpell(id): 0 if out of range
006d9dd1  89 45 98                MOV [EBP-0x68],EAX
006d9dd4  83 7D 98 00 / 74 29     guarded only for the SetActivity call
006d9e03  83 7D 9C 00 / 0F 8F 97 00 00 00   JG 0x6d9ea4  ; FORCEHOSTILE > 0
006d9e22  3B 8D A8 FE FF FF / 74 7A          JZ 0x6d9ea4  ; target == self
006d9e2a  8B 55 98                MOV EDX,[EBP-0x68]
006d9e2d  8B 82 54 01 00 00       MOV EAX,[EDX+0x154]   ; NULL+0x154 -> crash
006d9e33  89 85 A4 FE FF FF       MOV [EBP-0x15C],EAX
006d9e39  83 BD A4 FE FF FF 00 / 74 62   JZ 0x6d9ea4
```

### Fix: hook `CA-A4`

| field | value |
|---|---|
| address / len | `0x006D9E2A` / 15 |
| stolen bytes | `8B 55 98 8B 82 54 01 00 00 89 85 A4 FE FF FF` |
| skip | **true**: the stolen bytes write EAX, and the handler must not dereference NULL |
| consumed exit | `0x006D9EA4`, `exclude_from_restore=["eax"]` |
| params | `ebp` |

**Why 15 bytes and not the reviewer's 9.** In a skip-only consumed hook EAX is the handler's result on the fall-through path. A 9-byte window would hand `MOV [EBP-0x15C],EAX` a 0 for every valid row (always taking the "not hostile" path). Covering the store lets the handler write the local itself. EAX is dead at `0x6D9E39` and `0x6D9EA4` (checked).

```
extern "C" int __cdecl CompanionAiA4(int ebp)
  row = [ebp-0x68]
  if (!row) {
      log("A4 NULL spell row: caller=%08x item-spell id? -> %s", a4 ? "skip" : "vanilla crash")
      if (!a4) return *(volatile int*)(0x154)      // vanilla parity: same fault address
      return 1                                      // -> 0x6D9EA4 (the path vanilla takes when row+0x154 == 0)
  }
  v = [row+0x154]; [ebp-0x15C] = v
  return v == 0                                     // 1 -> 0x6D9EA4 (= vanilla JZ), 0 -> resume 0x6D9E39 (CMP/JZ re-tests the same local)
```

**Parity:**
- **Non-null row:** identical (same local value, same branch); this holds for both flag values.
- **Null row, `a4=0`:** faults at address `0x154` as vanilla does, inside the DLL, after one log line.
- **Null row, `a4=1`:** takes the "not hostile / keep target" path instead of crashing.
- **Who:** any creature running a GUI-queued combat item spell whose spell id is outside `spells.2da`. This is reachable only with modded items.
- **Save:** none.

**INI:** `a4` (ship 1).

**Log:** the line above. It should never appear with vanilla data.

**Risk:** nil for valid data. For the bad row, the item's effect simply proceeds without the target-clear heuristic.

## 8. C1: `ClearHostileActionsVersus 0x5867E0` skips the new slot 0 (MEDIUM, optional but cheap)

### Bug

```
0058680f  8B 4D F0 / 83 C1 01 / 89 4D F0   ; i++
0058681b  3B 55 EC                         ; i < count (count taken once)
005868b4  E8 E7 8D FB FF  CALL 0x0053f6a0  ; delete group (type 0xC, target [act+0x38])
005868b9  C7 45 F0 00 00 00 00             ; i = 0, then the loop's ++ makes it 1: group 0 is never re-examined
005868f1  C7 45 F0 00 00 00 00             ; same for type 0xF ([act+0x4C])
00586929  C7 45 F0 00 00 00 00             ; same for type 0x2E ([act+0x44])
```

### Fix: three skip-only 7-byte hooks `CA-C1a/b/c`

| field | value |
|---|---|
| addresses | `0x005868B9`, `0x005868F1`, `0x00586929` |
| len | 7 each |
| stolen bytes | `C7 45 F0 00 00 00 00` |
| skip | **true**: the stolen bytes would overwrite the handler's value |
| consumed exit | none; resume at the next `JMP 0x586930` |
| params | `ebp` |

```
extern "C" void __cdecl CompanionAiC1(int ebp)   // one handler, three hooks
  [ebp-0x10] = c1 ? -1 : 0 ; ++g_c1Restarts
```
Indices past the end return 0 from `0x53F4A0`, so the stale count only adds empty iterations. Each restart follows a deletion, so the loop is bounded (at most groups² steps).

**Parity:**
- **`c1=0`:** writes 0, which is vanilla.
- **`c1=1`:** every hostile action against the target is removed, not just all but the one that slid into slot 0.
- **Who:** any creature. Callers are the perception "target became invisible/unattackable" path (`0x5773AE`), `ChangeFaction`/`Pacify` (`0x5864D0`, `0x586640`) and `CancelCombat`.
- **Save:** the queue that gets saved no longer carries the stale second hostile action. This only differs in the bug case.

**INI:** `c1`.

**Log:** a summary counter: `C1 summary: restarts=3/10s`. Optionally, a log-only rescan at the function exit `0x586935` (the reviewer's confirm hook) can show surviving 0xC/0xF/0x2E actions vs. the target. It is not proposed as a hook here, to keep the window count down; the counter suffices.

**Risk:** low. A behaviour change is visible only where vanilla left a stale attack against a creature that just surrendered or vanished.

## 9. Optional items

- **A3: orientation lock left on a dead or undetected target (cosmetic).** Window `0x006D78B7`, 7 bytes `C7 45 C4 00 00 00 00`, a branch target, skip=false, plain detour, params `ebp`.
  - Recommended variant (no consumed exit): `if (a3 && [ebp-0x30]==0 && party && not controlled) thiscall 0x57D370(attacker, 0x7F000000, 0)` (`RET 8`). That is the same lock clear `EndAttackAction` does at `0x6DAB36`, without its combat-mode/animation side effects.
  - The reviewer's consumed variant (exit `0x6D77AC`; EAX dead at both ends) also passes, but it would call `SetCombatMode(0)` between kills. That is visible for the controlled character, so I do not recommend it.
  - Ship `a3=0` and enable only if a facing-the-corpse symptom is seen. The lock's consumer `0x563200` is untraced.
- **FM3: scheduler sentinel (deferred).** Probe window `0x0051DBE7`, 6 bytes `8B 15 88 B4 A1 00`, log-only (object `[ebp-0x28]`, sentinel `[ebp-0x3C]`, level `[ebp-0x34]`, per-tick counts).
  - The fix (cap per-level iterations, then force `[ebp-0x28]=0`) sits in the hottest loop of the game, between perf-lab's `0x51DC70`/`0x51DC86` hooks. It needs the level/list bookkeeping (`param_1[level*4+2]` counts) proven first.
  - Not cheap enough to ship blind. Fold the probe into perf-lab's AI counters or the trace layer.

## 10. Build order (recommended)

1. **companion-ai 0.1.0**, all hooks in one build:
   - CA-A1, CA-P1i, CA-P1s, CA-FM1, CA-FM4s, CA-FM4r, CA-FM4a, CA-A4, CA-C1a/b/c;
   - optional CA-A3 (lock-clear variant), CA-FM3 probe;
   - plus the trace layer's T1-T4/E1/E1b/E2/E3, **minus E4**.

   Install with `a4=1` and the other flags at 0, `log=1` (Stage 0, log-only). Confirm on disk:
   - the A1 would-spin rate (expect hundreds per second per stuck Ranged companion);
   - the P1 rate (about the tick rate per enemy);
   - FM1 `clear` lines on walk-through;
   - FM4 resets.
2. **Stage 1 (INI only, hot reload):** `a1=1 fm1=1 fm4=1 c1=1`. Play-test items:
   - a Ranged companion with its target out of range re-thinks within 3 s;
   - followers keep following after a walk-through;
   - no `k_hen_*5m` re-fire after a save.
3. **ranged-ai 0.3.0** (E-H3 `kite_approach=1`; DLL rebuild, same hooks.toml). This closes stance-16 play-test item (b).
4. **Stage 2:** `a1_retarget=1`.
5. **Stage 3:** `p1=1 p1_interval_ms=250`, only after a stealth and aggro check against a known encounter. Then decide whether to keep it, tune it, or leave it off.
6. Later: decide on A3/FM3 from the logs.

**Rollback:** every behaviour has its INI flag, all at 0 is vanilla, and removing the patch id removes all hooks. No data files are written.

## 11. Window check (`check_windows.py` copy, run read-only)

**Script changes in my copy.** Saved as `scratchpad/check_windows_companion_ai.py`; output in `check_windows_companion_ai_output.txt`.
- The window table is my windows.
- Ranged-ai's 7 live hooks and the parallel trace design's planned windows are added to the overlap list.
- The EAX test is generalised to every consumed hook.
- New: an EAX-liveness walk (fall-through plus Jcc targets) from the resume address and from the consumed exit.

Result:
```
        EAX at resume 0x6d8548: dead on all paths
        EAX at consumed exit 0x6d7d2f: dead on all paths
[OK  ] CA A1 approach gate end/retarget: 0x006d853e len 10  bytes = [0xC7, 0x85, 0x94, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x7F]
[OK  ] CA P1 throttle interval: 0x0056bdb4 len 7  bytes = [0x81, 0x7D, 0xD8, 0xA0, 0x0F, 0x00, 0x00]
[OK  ] CA P1 throttle stamp store: 0x0056bde3 len 6  bytes = [0x89, 0x90, 0x9C, 0x03, 0x00, 0x00]
[FAIL] CA FM1 bump clear replace: 0x00586f43 len 10 (skip-only)  bytes = [0x6A, 0x01, 0x8B, 0x4D, 0x08, 0xE8, 0x33, 0xA1, 0xFB, 0xFF]
        ! overlaps live hook 0x586f43+5 (PLANNED ai-trace:E4)
[OK  ] CA FM4 snapshot FollowInfo: 0x0057451f len 6  bytes = [0x8B, 0x88, 0xAC, 0x11, 0x00, 0x00]
[OK  ] CA FM4 restore FollowInfo: 0x00575183 len 7  bytes = [0x8B, 0x45, 0x0C, 0x50, 0x8B, 0x4D, 0x08]
[OK  ] CA FM4 alt: apply=0 (mode 2): 0x0057452c len 6  bytes = [0x8B, 0x8D, 0x2C, 0xFE, 0xFF, 0xFF]
        EAX at resume 0x6d9e39: dead on all paths
        EAX at consumed exit 0x6d9ea4: dead on all paths
[OK  ] CA A4 NULL spell row: 0x006d9e2a len 15 (skip-only)  bytes = [0x8B, 0x55, 0x98, 0x8B, 0x82, 0x54, 0x01, 0x00, 0x00, 0x89, 0x85, 0xA4, 0xFE, 0xFF, 0xFF]
[OK  ] CA C1 restart idx (0xC): 0x005868b9 len 7 (skip-only)  bytes = [0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00]
[OK  ] CA C1 restart idx (0xF): 0x005868f1 len 7 (skip-only)  bytes = [0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00]
[OK  ] CA C1 restart idx (0x2E): 0x00586929 len 7 (skip-only)  bytes = [0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00]
[OK  ] CA A3 dead-target exit (opt): 0x006d78b7 len 7  bytes = [0xC7, 0x45, 0xC4, 0x00, 0x00, 0x00, 0x00]
        EAX at resume 0x6d78be: dead on all paths
        EAX at consumed exit 0x6d77ac: dead on all paths
[OK  ] CA A3 alt consumed (opt): 0x006d78b7 len 7  bytes = [0xC7, 0x45, 0xC4, 0x00, 0x00, 0x00, 0x00]
[OK  ] CA FM3 sentinel probe (opt): 0x0051dbe7 len 6  bytes = [0x8B, 0x15, 0x88, 0xB4, 0xA1, 0x00]
PROBLEMS FOUND
```
- **The only problem is intended.** The FM1 window overlaps the trace design's **planned** E4. Resolution: drop E4 and log from the FM1 handler.
- **No overlaps** with the 120 live hooks (`live_hooks_2026-09-30g.tsv`, including perf-lab `0x51D790/0x51DC70/0x51DC86/0x55AB30/0x55AC26/0x544210/0x544640` and post-combat-movement-fix `0x7AF198`), nor with ranged-ai `0x6D83EA 0x6D7C79 0x6D850F 0x77AA5E 0x77ACBC 0x77D8D4 0x74DBDD`.
- **Nearest neighbours:**
  - CA-A1 `0x6D853E` is 0x29 bytes after E-H3's end (`0x6D8515`).
  - CA-A3 `0x6D78B7` lies between the trace design's E1 (`0x6D740F`) and E-H2 (`0x6D7C79`), with no overlap.
  - CA-FM3 `0x51DBE7` is 0x89 bytes before perf-lab `0x51DC70`.

**Both executables** (`verify_windows_both_exes_ai_trace.py`, run on my output, read-only on the game folder): all 14 windows `LAA:True base:True`, **ALL MATCH** (`verify_both_exes_companion_ai_output.txt`).

## 12. Hook table for `kotor2-steam-aspyr.hooks.toml` (companion-ai)

| id | address | len | skip | consumed exit | exclude | params | handler |
|---|---|---|---|---|---|---|---|
| CA-A1 | `0x006D853E` | 10 | false | `0x006D7D2F` | eax | ebp | `CompanionAiA1Gate` |
| CA-P1i | `0x0056BDB4` | 7 | false | none | - | ebp | `CompanionAiP1Interval` |
| CA-P1s | `0x0056BDE3` | 6 | false | none | - | ebp | `CompanionAiP1Stamp` |
| CA-FM1 | `0x00586F43` | 10 | true | none | - | ebp | `CompanionAiBumpClear` |
| CA-FM4s | `0x0057451F` | 6 | false | none | - | eax | `CompanionAiFm4Snap` |
| CA-FM4r | `0x00575183` | 7 | false | none | - | ebp | `CompanionAiFm4Restore` |
| CA-FM4a | `0x0057452C` | 6 | false | none | - | `esp+4` (pointer) | `CompanionAiFm4Apply` |
| CA-A4 | `0x006D9E2A` | 15 | true | `0x006D9EA4` | eax | ebp | `CompanionAiA4` |
| CA-C1a/b/c | `0x005868B9` / `0x005868F1` / `0x00586929` | 7 | true | none | - | ebp | `CompanionAiC1` |
| CA-A3 (opt) | `0x006D78B7` | 7 | false | none | - | ebp | `CompanionAiA3` |
| CA-FM3 (opt) | `0x0051DBE7` | 6 | false | none | - | ebp | `CompanionAiFm3Probe` |

Original bytes are as printed in section 11.

**Handler signatures** (cdecl detours):
- every hook with a consumed exit returns `int`;
- every other hook returns `void`;
- all handlers call `saferead::beginScope()` and `loadIni(false)`, like RangedAI.cpp.

**Typed game calls** (MinGW `__thiscall` function pointers):
- `0x77D800` int(int);
- `0x57C990` int(int, float, int, int);
- `0x541080` void(int, int);
- `0x521060` int(int);
- `0x7A1720` int(int);
- `0x58C370` int(int, int*);
- `0x57D370` void(int, int, int);
- `0x5453C0` int(int) (GetArea, for the log).

## 13. Open questions

1. **A1 at runtime.** Does vanilla stance 11/12 really spin? Stage 0's would-spin counter answers it. If it does not, drop A1 to a watchdog.
2. **After an A1 end, does `+0x520` (in combat) stay 1** long enough for the fallback to fire when nobody is hitting the companion? If not, consider setting `a1_rethink_ms` around 1500 so the re-think lands before the 8 s combat timeout.
3. **Vanilla Ranged (11) holding a melee weapon:** should it approach (reviewer option 3) instead of ending? The design keeps "never approach" per the brief. An `a1_melee_approach_stances` knob (write `[ebp-0x174]=9`, return 0) fits the same handler if wanted.
4. **`a1_retarget` for Stationary (12).** It is in the default mask; confirm it is wanted.
5. **P1:** keep it off, or ship 250 ms after the stealth/aggro check? Should `p1_interval_ms` differ for creatures already in combat?
6. **FM4:** confirm mode 1 (snapshot/restore) over the reviewer's apply=0 (mode 2), which changes save content.
7. **Trace layer:** confirm dropping its E4, and whether fix events go to `companion_ai_log.txt` or the trace's `ai_trace.txt`.
8. **FM1:** confirm the keep set {0x3D, 0x40, 0x46}, and whether a combat queue should ever be kept (proposed: no).
9. **Persist to the DB** after review (lessons):
   - "consumed hook: EAX is also the handler result at the resume address";
   - "A2 fallback has no queue-empty test (`vt+0x94` = GetDead, `0x563460`)";
   - `0x6D7D2F` as a reusable retarget/end exit;
   - `AddMoveToPointAction` queues to the front.
