# KOTOR II (Steam 6a522e71) engine-side party AI review

2026-10-01. Static only: Ghidra dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` plus constants read from `backups/vanilla_clean/swkotor2.exe`. Read-only; nothing was installed or run.

Method: one lead reviewer (attack action, combat round, action loop) and three parallel sub-reviews (perception/events, follow/movement/priority, stance readers/combat reset). Every "verified" finding below was re-checked in the `.asm` by the lead reviewer, except where it is marked (sub-review).

## Summary table

| id | severity | confidence | symptom |
|---|---|---|---|
| A1 | **High** | verified | A Ranged or Stationary companion whose target is out of reach, blocked, or out of melee reach while it holds a melee weapon re-queues `[facing, attack]` forever. It burns ~1 ms of CPU per frame, never attacks or moves, and its 3 s combat-script fallback is starved, so it stands inert until the player intervenes. Mod 8 stance 16 inherits it. |
| P1 | Medium | verified | The perception throttle for "non-party creature perceives the party" never stores its timestamp. Every enemy re-perceives the party (LOS raycast per member) on every AI tick instead of every 4 s / 0.3-0.7 s. |
| C1 | Medium | verified (code) / plausible (reach) | `ClearHostileActionsVersus 0x5867E0` restarts its scan at index 1 after a deletion, so a second queued hostile action against a target that just turned friendly or invisible survives. |
| FM1 | Medium | verified code / plausible reach | When the player-controlled leader bumps a companion that has no move action queued, `BumpFriends` runs `ClearAllActions(1)` on it. The companion stops following or shooting until its next heartbeat or round script. |
| FM2 | Medium | verified code / plausible loop | A pathfind failure in `AIActionMoveToPoint` returns 3 (the same as success) plus a 2-4 s wait, with no OnBlocked. An approach to an unreachable target loops move-fail-wait, so the companion idles. |
| FM3 | Low-Med | verified | `UpdateState`'s wrap sentinel is the first object's pointer. If that object changes AI level during its own update, the level re-runs AIUpdates until its slice is spent (frame hitch) and skips one object. |
| P2 | Low (design, visible) | verified | Non-controlled companions refresh perception only every 4 s (controlled character every 200 ms), so a companion notices a new enemy late. Also feeds stale data to the engine retarget used by Mod 8 E-H2. |
| A3 | Low | verified code / plausible effect | An attack that ends on a dead, missing or undetected target skips `EndAttackAction`, so the orientation lock stays on the corpse until the next move. |
| A4 | Low | verified | `AIActionCombat 0x6D9500` item-spell branch dereferences a NULL spell row (`0x6D9E2D`). Data-triggered crash with modded items. |
| T1 | Low | verified (sub-review) | `GetNearestVisibleEnemyInRange 0x57C990` prunes on the raw X gap but compares radius-adjusted distance. It can pick a farther target than the nearest large-radius one. |
| R1 | Low | plausible (sub-review) | `UpdatePerception 0x5D0F30` aborts the whole new-object scan when the X-sorted binary search hits an id that does not resolve. |
| FM4 | Low | verified | `SaveCreature` toggles the party flag with apply=1, so live FollowInfo is freed and re-created as defaults on every save. One-byte fix. |
| FM5 | Low | verified (sub-review) | ON_PATH_BLOCKED is posted every call while a follower stays blocked, so OnBlocked can run about once per frame. |
| FM6 | Low | plausible | Script and deferred leader changes rotate party slot ids but not each slot's follow trail state. |
| FM7 | Low | plausible | AI level churns 4↔0 for a party member in an area with no player. |
| F2 | Low | verified (gating) | OnEndCombatRound is skipped when a MoveToPoint heads the queue (`0x58F39C`). The 3 s in-combat fallback (A2) bounds the resulting idle to at most ~3 s, so this is a hitch, not a freeze. |
| PCM | info | verified | The live `post-combat-movement-fix-v1` (`0x7AF198`) fixes **player keyboard input** after combat at high FPS. It is not party AI (section PCM). |

## A1. Ranged/Stationary companion: the attack action re-queues itself forever (HIGH, verified)

**Path.** `AIActionPhysicalAttacks 0x6D7390` takes the approach branch when any of these holds:
- the target is in another area;
- `(max+0.1)² < dist²`;
- `los == 0`;
- the door/placeable term.

See 13_ranged_ai_spacing.md for the full term list. In that branch:
```
006d8508  0F B7 82 46 01 00 00   MOVZX EAX,word ptr [EDX+0x146]          ; stance
006d850f  89 85 8C FE FF FF      MOV [EBP-0x174],EAX                      ; (live Mod 8 E-H3 window)
006d8721  6A 0C                  PUSH 0xc
006d8729  E8 72 85 E6 FF         CALL 0x00540ca0     ; AddActionToFront(ATTACK, same params), unconditional
006d876a  83 BD 8C FE FF FF 0B   CMP [EBP-0x174],0xb
006d8773  83 BD 8C FE FF FF 0C   CMP [EBP-0x174],0xc
006d8791  3B 95 60 FC FF FF      CMP EDX,[EBP-0x3a0]  ; EDX=[ebp-0x16C] controlled id (0x7E5DA0 -> 0x77D800 -> +4), [ebp-0x3A0]=attacker id
006d8797  75 63                  JNZ 0x006d87fc       ; stance 11/12 and not controlled: skip 0x11 (CheckMoveToObjectRadius)
006d883a  6A 13                  PUSH 0x13
006d8842  E8 59 84 E6 FF         CALL 0x00540ca0     ; AddActionToFront(0x13 = AIActionChangeFacingObject 0x5BB2C0), unconditional
006d8a7e  CMP [EBP-0x174],0xb / 006d8a87 CMP [EBP-0x174],0xc / 006d8aa5 CMP EDX,[EBP-0x3c8]
006d8aab  0F 85 31 01 00 00      JNZ 0x006d8be2       ; skip the move (0x5644B0)
006d8be2  A1 B8 44 99 00         MOV EAX,[0x009944b8] ; return 2
```

**Action loop.** In `CSWSObject::RunActions 0x5D56B0` the node is popped before the handler runs. Return 1 puts it back and stops the loop. Returns 2 and 3 delete it, and the loop **continues** while time remains:
```
005d6cf1  3B 05 B4 44 99 00   CMP EAX,[0x009944b4]     ; 1 = in progress -> stop
005d6cf7  74 66               JZ 0x005d6d5f
005d6d02  E8 19 F5 15 00      CALL 0x00736220          ; now, QPC microseconds (0x73B720 picks QPC when available)
005d6d07  2B 45 10            SUB EAX,[EBP+0x10]       ; minus the stamp taken at AIUpdate entry (0x569440)
005d6d0a  1B 55 14            SBB EDX,[EBP+0x14]
005d6d0d  8B 0D C4 44 99 00   MOV ECX,[0x009944c4]     ; = 1000 (vanilla exe)
```
`0x5D7160` ends the loop early only after action types 1, 5, 0x30 and 0x33.

So the queue becomes `[0x13, 0xC]`:
- facing completes immediately (`0x5BB2C0` returns 2 or 3 only);
- the attack runs again and finds the same geometry;
- it re-adds both nodes and returns 2.

This repeats until **1000 us** have passed since this creature's AIUpdate began, and again on every frame. Each pass costs a `ClearLineOfSight 0x52BB60` raycast, the range math, two `AddActionToFront` (`new(0x74)` node plus up to 13 parameter allocations, no de-dup in `0x540CA0`/`TestActionList 0x53FC10`), one free, and the party GUI write `0x7E5120(0x1B2C7)`.

**No re-think can happen.**
1. No combat round starts, so `EndCombatRound` (the normal OnEndCombatRound source) never runs.
2. The 3 s in-combat fallback in `AIUpdate` (A2) runs script slot 6 when `+0x570` reaches 0. But the attack prologue resets it on every entry:
   `006d740f  C7 80 70 05 00 00 B8 0B 00 00   MOV dword ptr [EAX+0x570],0xbb8   ; EAX = attacker ([ebp-0x49C])`
   With hundreds of entries per frame it never fires.
3. The queue always holds an action, so "if idle" scripts (heartbeat follow, etc.) see a busy creature. Only a player order, taking control (the controlled id is exempt at `0x6D8791`/`0x6D8AA5`), or an event script that calls `ClearAllActions` (OnDamaged / OnAttacked / perception) breaks it.
4. When combat state times out (8 s, `+0x524`), the spin continues out of combat: `SetCombatState(0)` clears the combat round's queue, not the creature's action queue.

**Vanilla triggers.**
- (a) **Ranged stance plus melee weapon.** The rule never tests the weapon: `[ebp-0x164]` RangeWeaponEquipped is used only for the move distance at `0x6D8758`. This covers a Jedi set to Ranged, and also the vanilla `GN_CheckRangedPreferred` swap to melee after a melee hit, with the next target not adjacent.
- (b) Ranged stance, target beyond `maxattackrange + 0.1`.
- (c) Ranged stance, eye-level LOS blocked.
- (d) Stationary, target out of reach. Not moving is the design; spinning and never re-thinking are not.

**Mod 8.** The live E-H3 maps stance 16 to 11, so Ranged (Kiting) inherits A1 for every case E-H1 does not hold: target out of range, or a kiter that swapped to melee. This is the most likely explanation for the open play-test item "companion idling with its target out of range".

**Player-visible.** A companion stands facing its target, neither shooting nor moving, until the player does something. Frame time rises by about 1 ms per affected companion per frame. Because the 10 ms `CServerAIMaster::UpdateState` budget is shared, other creatures' AI updates slip too.

**Confirm at runtime (log only):**
- `0x6D740F`, 10 bytes `C7 80 70 05 00 00 B8 0B 00 00`, EAX = attacker. Count entries per creature per frame (expect 1-2 normally, dozens or more when spinning). Log `[EAX+4]` id, `[EAX+0x11AC]` party, `*(u16*)([EAX+0x1198]+0x146)` stance.
- `0x6D8BE2`, 5 bytes `A1 B8 44 99 00` (absolute operand only; also reached from the back-off and Force-jump exits). Log `[ebp-0x174]` stance, `[ebp-0x168]` back-off flag, `[ebp-0x3A0]` attacker id, `[ebp-0x16C]` controlled id, `[ebp-0x164]` ranged, `[ebp-0xBC]` dist², `[ebp-0x2C]` max, `[ebp-0xD0]` los. Gate taken = stance 11/12, ids differ, back-off 0.
- Signal: matching high rates at both sites, no `StartCombatRound 0x58E730` for the creature, and no slot-6 script (log at `0x5699EE`, 6 bytes `8B 85 6C FE FF FF`).

**Fix options:**
1. **Preferred, small.** Hook `0x6D84FC`, 6 bytes `89 8D 7C FC FF FF` (`MOV [ebp-0x384],ECX`; ECX = stats; no EAX in the window; does not overlap E-H1 `0x6D83EA` or E-H3 `0x6D850F`). Consumed exit `0x6D83FF` (`PUSH 1; CALL EndAttackAction; MOV EAX,[0x9944BC]; JMP epilogue`, the block E-H1 already uses).
   - Condition: stance in {11, 12, 16}. Read 16 here because E-H3 runs later.
   - And attacker id != controlled id (replicate `0x6D8515..0x6D856B`), back-off `[ebp-0x168]==0`, Force-jump `[ebp-0x3C]==0` (already true here).
   - Result: one clean end, no re-queue. `+0x570` then counts down from 3000, so the script picks again within 3 s.
2. **Better (on top of 1).** Before ending, try `GetNearestVisibleEnemyInRange 0x57C990(max, self, 0x7F000000)`. If it finds a target, call `InitializeNewAttackTarget 0x6DAC70(id, node)` and exit through the existing in-progress return (the PC path at `0x6D7C86..` returns `[0x9944B4]`).
   - Note: `InitializeNewAttackTarget` sets `+0x52C=1` (passive), which is correct for a non-approaching stance.
3. **Stance 11 with a melee weapon should probably approach.** Extend E-H3 (`0x6D850F`, skip-only) to store 9 when stance is 11/16 and `[ebp-0x164]==0` (written at `0x6D836B`, before E-H3).

## A2. 3-second in-combat script fallback (INFO, verified; corrects assumptions in 13_ranged_ai_spacing.md)

```
005699ce  83 BD E8 FE FF FF 01   CMP [EBP-0x118],0x1         ; +0x520 in combat
005699e1  83 BA 70 05 00 00 00   CMP dword ptr [EDX+0x570],0x0
005699e8  0F 8F 8E 00 00 00      JG 0x00569a7c
00569a02  FF D0                  CALL EAX                     ; vtbl+0x94 dead? -> skip
00569a38  8D 8C 10 70 02 00 00   LEA ECX,[EAX+EDX*1+0x270]    ; slot 6 -> +0x2A0 (OnEndCombatRound)
00569a67  CALL 0x006fd8d0                                     ; RunScript
00569a72  C7 81 70 05 00 00 B8 0B 00 00  MOV [ECX+0x570],0xbb8
```
There is no party or controlled test. `+0x570` is reset by the attack (`0x6D740F`), cast (`0x5B97F3`), item-cast (`0x5B82A4`) and total-defense (`0x5D2A05`) prologues (sub-review); moves do not reset it.

Consequences:
- **Mod 8 E-H1 hold** ends the action with no round, but OnEndCombatRound still runs within ≤3 s while the companion is in combat. This closes 13_ranged_ai_spacing.md risk 3(b) and "not verified: whether an E-H1-held attack still yields the end-of-round event". The held companion re-thinks every ≤3 s.
- **F2** (event skipped when a move heads the queue) is bounded the same way.
- **Kite moves.** A long `ActionMoveToLocation` from the kite routine is re-evaluated every 3 s in combat. `k_ai_master` may `ClearAllActions` it mid-way, so the routine's cooldown logic should tolerate that.
- **A1 is the exception:** it is the one path that resets the fallback continuously.

## A3. Attack ends on a dead, missing or undetected target without EndAttackAction (LOW)

```
006d78af  CALL 0x006dab80        ; HasTargetBeenDetected (skipped when the target is dead / party HP<1)
006d78b4  MOV [EBP-0x30],EAX
006d78b7  C7 45 C4 00 00 00 00   MOV [EBP-0x3c],0x0
006d78be  CMP [EBP-0x30],0x0
006d78c2  JZ 0x006d93f6          ; -> MOV EAX,[0x9944b8]; epilogue (return 2)
```
Every other early exit calls `EndAttackAction 0x6DAB20`. That function forces its own argument to 1 (`0x6DAB29 MOV [EBP+8],1`), then:
- SetLockOrientationToObject(INVALID) at `0x6DAB40`;
- vtbl+0x7C(0x2710) (ready animation);
- SetCombatMode(0,1) at `0x6DAB5E`;
- ClearAllSpecialAttacks at `0x6DAB72`.

On the dead-target exit the lock `+0x104C` keeps the corpse id. It is cleared at the next `AddMoveToPointAction` (`0x5645F3 CALL 0x57D370`) or round start. EndCombatRound resets combat mode only for non-controlled creatures (`0x58F317`).

Symptom (plausible): the companion keeps facing the body and stays in combat-ready animation until its next move. Its consumer is the vtable getter `0x563200`, which has no direct callers; not traced.

- Confirm: log hook at `0x6D78B7` (7 bytes `C7 45 C4 00 00 00 00`), read `[ebp-0x30]`.
- Fix: same site with consumed exit `0x6D77AC` (`EndAttackAction(1)`, return 3) when `[ebp-0x30]==0`. This is cosmetic, low priority.

## A4. AIActionCombat item-spell NULL dereference (LOW, verified)

```
006d9dcc  CALL 0x006c1450        ; CSWSpellArray::GetSpell(id): 0 if id<0 || id>=count
006d9dd4  CMP [EBP-0x68],0x0 / JZ 0x6d9e03   ; guards only the "+0x154 != 0 -> SetActivity" call
006d9e03  CMP [EBP-0x64],0x0 / JG 0x6d9ea4   ; FORCEHOSTILE (0 when the 2DA read failed)
006d9e1f  MOV ECX,[EAX+0x44] / 006d9e22 CMP ECX,[EBP-0x158] / JZ   ; target != self
006d9e2a  8B 55 98               MOV EDX,[EBP-0x68]
006d9e2d  8B 82 54 01 00 00      MOV EAX,[EDX+0x154]   ; NULL+0x154
```
- Trigger: an item whose cast-spell property maps (after `ConvertGrenadeSpellID 0x576430`) to a spell id outside spells.2da, used on another creature from the combat queue. This is a modded-item crash.
- Fix: 9-byte window `0x6D9E2A` (`8B 55 98 8B 82 54 01 00 00`); when `[ebp-0x68]==0`, take the "not hostile" path `0x6D9EA4`.

## P1. Perception throttle timestamp never stored (MEDIUM, verified)

In `SpawnInHeartbeatPerception 0x56B8A0`, block 2 (mode-1 perception: a non-party creature perceives the party):
```
0056bcf9  8B 88 A4 03 00 00      MOV ECX,[EAX+0x3a4]   ; stamp ms
0056bd06  8B 82 A0 03 00 00      MOV EAX,[EDX+0x3a0]   ; stamp day
0056bd22  E8 39 F2 FA FF         CALL 0x0051af60       ; elapsed -> [EBP-0x28]
0056bd5b  0F 85 88 00 00 00      JNZ 0x0056bde9        ; party members skip block 2
0056bd8b  3B 88 9C 03 00 00      CMP ECX,[EAX+0x39c]   ; IsPC creatures: 300..699 ms
0056bdb4  81 7D D8 A0 0F 00 00   CMP [EBP-0x28],0xfa0  ; others: 4000 ms
0056bdc5  E8 66 51 06 00         CALL 0x005d0f30       ; UpdatePerception(1)
0056bde3  89 90 9C 03 00 00      MOV [EAX+0x39c],EDX   ; re-rolls the interval only; the stamp is not written
```
The whole dump has no creature-side write to `+0x3A0/+0x3A4` except the ctor zeroing them (`0x562366`, `0x562376`). That was checked by direct operand and by `ADD/LEA reg,0x3A0` arithmetic; the other hits belong to placeable, door and GUI classes. Compare block 3, which does store its stamp at `0x56BF96`/`0x56BFA5` (`+0x3C8/+0x3CC`).

Effect:
- Every non-party creature that gets an AIUpdate runs `UpdatePerception(1)` every tick. That walks the active party and calls `DoPerceptionUpdateOnCreature 0x576DC0` (LOS raycast plus stealth/listen rolls) per member.
- Enemies notice the party, and lose sight of it, immediately instead of on the intended cadence.
- CPU scales with enemies × party per frame and eats the shared 10 ms AI budget.
- A side effect of comparing against (day 0, ms 0) depends on `0x51AF60`'s day handling (not resolved).

Confirm: hook `0x5D0F30` (9 bytes `55 8B EC 81 EC C4 00 00 00`; ECX = creature, `[esp+4]` = mode). Count mode-1 calls per second against the number of non-party creatures; designed is ~0.25/s each, the bug gives about the tick rate.

Fix: hook `0x56BDE3` (6 bytes `89 90 9C 03 00 00`, skip=false; not a branch target). Handler: `c=[ebp-0xD4]; c+0x3A0=[ebp-0x24]; c+0x3A4=[ebp-0x20]` (the current day and ms from `0x51AD20` at `0x56BCD7`).
- **This changes 20-year-old enemy reaction timing.** Ship it behind an INI switch, or with a shorter interval, and play-test.
- The CPU win alone may justify a gentler throttle such as 500 ms.

## P2. Companion perception cadence 4 s (LOW, verified)

Block 3 of `0x56B8A0`:
```
0056be70  81 7D E8 C8 00 00 00   CMP [EBP-0x18],0xc8     ; IsPC (+0x10E8): 200 ms
0056bea2  81 7D E8 A0 0F 00 00   CMP [EBP-0x18],0xfa0    ; everyone else (companions have IsPC 0): 4000 ms
0056bea9  0F 82 72 01 00 00      JC 0x0056c021
0056bf71  6A 00 ... 0056bf79 CALL 0x005d0f30             ; party: UpdatePerception(0)
```
- A non-controlled companion with no orders sees new enemies up to 4 s late. The exception is when OnAttacked/OnDamaged wakes it first.
- `GetNearestVisibleEnemyInRange` needs the seen bit (`entry+4 & 1`), so engine retargets for companions (Mod 8 E-H2) use data up to 4 s old.
- After a GUI leader switch, IsPC stays on the Exile (only `MakePlayerCharacter 0x5FE4B0` changes it). The AI-driven Exile then keeps the 200 ms cadence and the companion the player controls gets 4 s.
- Fix (optional): hook `0x56BE79` (6 bytes `8B 85 2C FF FF FF`, branch target, safe). For party members set `[ebp-0x18]=0xFA0` once elapsed ≥ N ms (INI, e.g. 500).

## C1. ClearHostileActionsVersus skips the new slot 0 after a deletion (MEDIUM, verified code / plausible reach)

```
00586806  C7 45 F0 00 00 00 00   MOV [EBP-0x10],0x0
0058680f  8B 4D F0               MOV ECX,[EBP-0x10]      ; loop increment
00586812  83 C1 01               ADD ECX,0x1
0058681b  3B 55 EC               CMP EDX,[EBP-0x14]      ; count computed once
0058682b  CALL 0x0053f4a0                                 ; group i (0 past the end)
0058688d  CMP [ECX],0xc   / 005868a4 CMP EDX([act+0x38]),target
005868b4  CALL 0x0053f6a0                                 ; delete group
005868b9  C7 45 F0 00 00 00 00   MOV [EBP-0x10],0x0      ; "restart", but the next step is ++ -> 1
005868c0  JMP 0x00586930 -> 00586930 JMP 0x0058680f
(same at 0x5868F1 for type 0xF [act+0x4C], 0x586929 for type 0x2E [act+0x44])
```
When group 0 is deleted, the group that slides into slot 0 is never examined.

Callers:
- the perception "became unattackable/invisible" path (`0x576DC0 @0x5773AE`);
- `0x5864D0` (ChangeFaction / Pacify) and `0x586640`;
- `CancelCombat 0x586750` (which first runs `ClearAllActions`, so it is mostly unaffected).

Symptom: when an enemy surrenders, goes friendly or stealths, a creature that had two hostile actions queued against it (attack plus queued power, or the player's double-queued attacks) still performs the second one. Possible "keeps attacking the surrendered NPC" quest breaks.

Confirm: entry hook `0x5867E0` (6 bytes `55 8B EC 83 EC 2C`) and exit `0x586935` (6 bytes `8B E5 5D C2 04 00`, a JGE target, safe). At exit, re-scan the groups (`0x53F5E0` count, `0x53F4A0` get) for surviving 0xC/0xF/0x2E actions that target `[ebp+8]`.

Fix: three simple patches `C7 45 F0 00 00 00 00` → `C7 45 F0 FF FF FF FF` at `0x5868B9`, `0x5868F1` and `0x586929`. Indices past the end return 0, so the stale count is harmless.

## Other low items (sub-review, spot-checked)

- **T1 `GetNearestVisibleEnemyInRange 0x57C990`.**
  - It prunes the X-sorted scan on `|dx|` vs `best`, where `best` already has both personal-space radii subtracted (`0x57CEB6..`, `0x57D32B..`), so a nearer large-radius enemy can be skipped.
  - With a reference creature (`param_4`, only `CreateNewAttackActions` for the controlled character), distance is measured from the reference, but pruning and the left-scan LOS use `this`.
  - Companions (callers `0x6D7390`, `0x57DC20`, `0x57DE70`) pass INVALID and see only the radius issue.
- **R1 `UpdatePerception 0x5D0F30`.** It returns early when `0x522AF0` (binary search on `area+0x194`) fails on an unresolvable id (`0x522B7D`). A stale id in the area list blinds discovery of new objects for every creature in that area until the list is rebuilt. Plausible.
- **F2.** `EndCombatRound` sets `[ebp-0x1C]=0` when the head action is type 1 (`0x58F39C CMP [EAX],1`), so no script runs. Bounded by A2 (≤3 s) while in combat.
- **Local 8 at combat exit.** `SetCombatState(0)` zeroes **local number 8** for party members (`0x57A5C1 PUSH 0; PUSH 8; ... CALL 0x5E9340`). That is design, but Mod 8's kite routine uses local 8 as "hit in melee", and it resets at every combat exit.
- **Stale line-of-fire position.** `+0x558..+0x560` (last blocked position) is never reset (not by SetCombatState, ClearAllActions or EndCombatRound). This is in the line-of-fire scope (Mod 8). It only matters for a creature that never moved, because of the exact float compare.

## B. Follow, movement and AI priority (sub-review; FM1, FM3 and FM4 re-checked in the asm by the lead reviewer)

### FM1. The leader bumping a companion wipes its whole action queue (MEDIUM, verified code / plausible reach)

**Bumpable rule.** `GetIsCreatureBumpable 0x586940`: the mover is the controlled leader and the blocker is a party member (`+0x11AC`) or a puppet (`+0x11FC`). Neither its combat state nor its actions are considered.

**The clear.** `BumpFriends 0x586B70` looks for a type 1 (MoveToPoint) or 0x11 (CheckMoveToObjectRadius) action in the bumpee's queue. If it finds neither, it clears everything:
```
00586f0d  0F B7 02               MOVZX EAX,word ptr [EDX]     ; action type
00586f13  83 7D 88 01            CMP [EBP-0x78],0x1
00586f19  83 7D 88 11            CMP [EBP-0x78],0x11
00586f39  83 7D 84 00            CMP [EBP-0x7c],0x0
00586f3d  0F 85 88 00 00 00      JNZ 0x00586fcb
00586f43  6A 01                  PUSH 0x1
00586f45  8B 4D 08               MOV ECX,[EBP+0x8]            ; bumpee
00586f48  E8 33 A1 FB FF         CALL 0x00541080              ; ClearAllActions(1): every action plus combat-round attacks
00586fc6  CALL 0x005644b0                                     ; then AddMoveToPointAction(push-aside spot)
```

**Why it hits companions.**
- A follower's queue holds only 0x3D (FollowLeader): it walks via walk state `+0x11B0=3`, not action 1.
- A companion shooting in range holds 0xC.
- Both are wiped when the player walks into them.

**Symptom.** After the player walks or click-moves through the party, a companion stops following or stops shooting. It recovers at its next heartbeat (3.0-4.2 s) or at the next slot-6 script (≤3 s in combat, A2).

**Confirm.** Log hook `0x586F43`, 5 bytes `6A 01 8B 4D 08`. Log bumpee `[ebp+8]`, its id, `+0x11AC`, head action type, and mover `[ebp-0x158]`.

**Fix.** Consumed hook at `0x586F43`, exit `0x586F4D`. For party bumpees, skip the clear and queue the push move with `AddActionToFront` so the rest of the queue survives.

### FM2. Pathfind failure is indistinguishable from success (MEDIUM, verified code / plausible loop)

In `AIActionMoveToPoint 0x5BFC00`:
```
005c1f18  CALL 0x0055ab30        ; path request
005c21c4  CMP EAX,[0x009944bc]   ; 3 = fail
005c21ef  CALL 0x0042a470        ; "The Path find has Failed... Why?"
005c2215  MOV [EDX+0x11b0],0x2
005c2484  CALL 0x005646c0        ; AddPathfindingWaitActionToFront: rand 2-4 s (0.1 s if player-controlled)
```
It then returns 3. RunActions deletes the node exactly as on success, and no ON_PATH_BLOCKED (0x1F) is posted on this path.

**Attack approach loop.** The approach queue `[move, 0x11, 0x13, 0xC]` toward an unreachable target runs: move fails, wait 2-4 s, attack, approach again, fail again. The slot-6 fallback (A2) re-runs the script every ≤3 s, and it usually re-picks the same nearest enemy.

**Symptom.** A companion idles in place against an enemy on an unreachable ledge or behind a locked door.

**Confirm.** Hook `0x5C21D0`, 6 bytes `8B 8D 58 FA FF FF`. Creature = `[ebp-0x5A8]`; destination at `[creature+0x380]+0x5C..0x64`.

**Fix.** For party members, post 0x1F on failure (as the door branch does), or set a per-creature "last move failed against target X" flag that the attack hooks (Mod 8) read to retarget.

### FM3. The scheduler's wrap sentinel breaks when an object changes AI level during its own update (LOW-MEDIUM, verified)

```
0051dbd3  CALL 0x0051d1c0              ; NextObject
0051dbdb  CMP [EBP-0x38],0x1 / JNZ     ; first in this level?
0051dbe4  89 4D C4                     MOV [EBP-0x3c],ECX   ; sentinel = first object pointer
0051dc65  3B 45 C4                     CMP EAX,[EBP-0x3c]   ; stop only when the cursor returns to it
0051dc7b  FF D0                        CALL EAX             ; vtbl+0x70 AIUpdate
0051dd1c / 0051dd2d  JC/JBE 0x0051d842 ; otherwise loop while elapsed <= allotment
```
**How the sentinel is lost.** `AIUpdate` moves its own object between levels in several cases:
- party → 4 (`0x5694D6`);
- in-combat 0/1 → 2 and 2 → 0/1;
- `0x56B8A0` → 0.

`SetAILevel → AddObject 0x51E040 → RemoveObject` erases it from the list. If that object was the sentinel, the level never wraps. It then re-runs `AIUpdate` on its objects until the level's slice (60% of the remaining budget) is gone. The erase also shifts the array under the cursor, so one object is skipped that tick.

**Effect.** This does not starve the party (level 4 runs first). It causes frame hitches of up to ~6 ms at combat start/end in crowded areas, and repeated near-zero-dt AIUpdates.

**Confirm.** Hook `0x51DBE7` (6 bytes `8B 15 88 B4 A1 00`): log object `[ebp-0x28]`, sentinel `[ebp-0x3C]`, level `[ebp-0x34]`, and the per-tick AIUpdate count per object. The live perf-lab hooks at `0x51DC70`/`0x51DC86` can share counters.

**Fix.** Per-level iteration counter in that hook. Once it reaches the list count snapshotted at the first object, set `[ebp-0x28]=0`, which takes the existing `JZ 0x51DC7F` "done" path.

### FM4. SaveCreature resets FollowInfo on every save (LOW, verified)

```
0057451f  MOV ECX,[EAX+0x11ac] / 00574525 MOV [EBP-0x38],ECX
00574528  6A 01  PUSH 0x1  /  0057452a 6A 00 PUSH 0x0
00574532  CALL 0x0058a330      ; SetPartyMemberFlag(0, apply=1): frees +0x500 FollowInfo, SetAILevel(2)
0057515f  PUSH 0x1 / 00575164 PUSH EAX([EBP-0x38])
0057516b  CALL 0x0058a330      ; (flag, 1): new(0x3C) FollowInfo via ctor 0x5752C0, SetAILevel(4)
0057517e  CALL 0x005751b0      ; SaveFollowInfo writes the fresh default block
```
- Live follow state (`+0x500`, 0x3C bytes: follow object, last leader position, MaxSpeed `+0x28` read by GetRunRate `0x57611C`, InSafetyRange `+0x38` driving `k_hen_enter5m`/`exit5m` in `0x7E6CE0`) is reset at every save, and saved as defaults.
- The creature also goes to level 2 and back to 4, which puts it at the end of the level-4 list.
- The `k_hen_*5m` scripts can re-fire after a save.

**Fix.** One byte: `0x574529` `01 → 00` (apply=0). The first call then only clears the flag, so the second neither reallocates nor changes level. Confirm first by logging `+0x500`, `+0x38` and `+0x78` around `0x574532`/`0x575170`.

### FM5-FM7 (LOW)

- **FM5: unthrottled ON_PATH_BLOCKED (0x1F).** It is posted on every call while a follower stays blocked (follow sub-states 2/3: `0x5C8BB4 MOV EDX,0x1f ... 0x5C8BFE CALL 0x51E170`), and from the MoveToPoint closed-door branch. OnBlocked scripts can therefore run about once per frame.
  - Confirm: hook `0x5C8BC2` (6 bytes `8B 0D A4 B4 A1 00`): creature `[ebp-0x128]`, blocker `[ebp-0x5C]`.
  - Fix: per-creature 1 s throttle, skipping to `0x5C8C03`.
- **FM6: follow state not reset on non-UI leader changes (plausible).**
  - Only the UI switch `0x797F30` resets the trail (`0x7E8350`) and removes 0x3D.
  - Script `SetPartyLeader 0x696250`, the deferred switch `0x79ED60` and the party-select screen `0x89FBD0` go through `0x7E5DF0` alone. That rotates the slot ids but not each slot's follow state, so followers inherit another slot's waypoint and trail index.
  - Confirm: hook `0x7E6546` (10 bytes `C7 80 94 00 00 00 01 00 00 00`).
  - Fix: reset slot+4 to -1 for all slots there, or call `0x7E8350`.
- **FM7: AI-level churn outside the player's area (plausible).** A party member in an area with no player toggles 4↔0: raised in AIUpdate, dropped by `0x56B8A0` when `area+0x12C==0`. That churns the lists and feeds FM3.

**Follow code checked, no bug:**
- Squared distances are used consistently (`0x5C6340`, `0x7E6CE0`, `0x5BCDC0` uses `(r+0.01)²`).
- The breadcrumb ring is bounded (100 entries, stride 0x28).
- `ClearAction(0x3D)` and `ClearAllActions` reset the follow slot.
- QuickWalk recomputes dt per call, so pre-pass calls do not multiply speed.
- In `AIActionCheckMoveToObjectRadius 0x5BC1D0` the LOS check is compiled out (`0x5BC4B8 XOR EDX,EDX; JZ 0x5BC5DE`).
- `0x5752C0` is the CFollowInfo ctor (name_map "RemoveMindTrickEffects" is wrong).
- `0x66B640`'s mapper name is wrong (it is SetCreatureAILevel).

## PCM. What the live post-combat-movement-fix-v1 addresses (INFO, verified)

Hook `0x7AF198`, 31-byte replace, inside `0x7AC470`. That function is the per-frame client input/controlled-creature handler (sole caller `0x781BE0`, main loop).

Vanilla code, verified:
- While the controlled creature has user actions pending (`0x776E40()==1`, `0x7AF070`) and W/S input is non-zero, a timer `[obj+0x2A4]` is set to 0.3 (`[0x986B60]`) and decremented by frame dt (`0x7AF13E FSUB [EBP+8]`).
- At expiry it asks the input-axis object `[obj+0x2A0]` vtbl+0x1C (`0x7AF1B5 CALL EAX`) for the **smoothed** axis value and requires `> 0.25` (`0x7AF1B7 FCOMP [0x98C1D8]`, 0.25) to cancel the queued actions and give movement back (`0x7F24C0`, `0x7F0300`, `0x77FC10(INVALID)`, `0x776EC0`). Otherwise it re-arms the timer (`0x7AF26E`).
- At high frame rates the smoothed value is pushed back toward 0 before it reaches 0.25, so the player cannot walk out of a finished attack, Force power or item use.

The patch replaces the smoothed read with `max(|[ebp-0x48]|, |[ebp-0x50]|)`, the raw forward and back input.

This is **player-character input only**: companions never pass through `0x7AC470`. It does not interact with any finding here. "Companions stand around after combat" has other causes: A1, P2, and the follow items in section B.

## Map: party AI pipeline (for the DB)

Confidence: H = verified in asm this review, M = decompile or sub-review.

| addr | name | conf | role |
|---|---|---|---|
| 0x51D790 | CServerAIMaster::UpdateState | H | 10 ms budget, 5 levels; calls vtbl+0x70 AIUpdate |
| 0x569320 | CSWSCreature::AIUpdate | H | forces party/PC to AI level 4 (`0x5694B2..0x5694D6`); perception `0x56B8A0`; `0x571030`; effects; RunActions; 3 s slot-6 fallback (`0x5699CE..0x569A72`); combat-exit timer `+0x524` |
| 0x5D56B0 | CSWSObject::RunActions | H | pop, dispatch by type, 1 = re-queue+stop, 2/3 = delete+continue, 4 → `0x54AA30`; 1000 us budget `[0x9944C4]`; empty queue clears `+0x550/+0x548/+0x568/+0x56C/+0x54C` |
| 0x5D7160 | ActionTypeStopsLoop(type) | H | 1, 5, 0x30, 0x33 |
| 0x6D7390 | CSWSCreature::AIActionPhysicalAttacks (type 0xC) | H | see A1; resets `+0x570` at `0x6D740F` |
| 0x6D9500 | CSWSCreature::AIActionCombat (type 0x3F) | H | executes GUI-queued combat-round actions |
| 0x6DAB20 | EndAttackAction | H | ignores its argument (`MOV [EBP+8],1`) |
| 0x6DAC70 | InitializeNewAttackTarget(id, node) | H | writes node+0x38, lock, `+0x88`, attempted target, `+0x52C=1` |
| 0x6DAB80 | HasTargetBeenDetected | H | party attackers always pass |
| 0x6DA4F0 | CreateNewAttackActions | H | auto-continue, controlled creature only (from EndCombatRound) |
| 0x6DACF0 | ResolveAttack | M | NULL target safe |
| 0x568540 | CSWSCreature::AddAttackActions | H | param_8==0 → combat-round queue (cap 4) + one 0x3F; else 0xC action; `+0x52C = bPassive` |
| 0x540CA0 / 0x53F7F0 | AddActionToFront / AddAction | H | `new(0x74)` + params, no de-dup |
| 0x5644B0 | AddMoveToPointAction | H | type 1; params (pos, area, target, run, range, ...); clears orientation lock |
| 0x5BB2C0 | AIActionChangeFacingObject (type 0x13) | H | instant (2/3) |
| 0x5BC1D0 | AIActionCheckMoveToObjectRadius (type 0x11) | M | |
| 0x5BFC00 | AIActionMoveToPoint (type 1) | M | range shortcut uses foot-level LOS with no exclusion |
| 0x590FF0 / 0x5912B0 | ResolveEngagedState / ResolveMasterState | H | party attacker is always master |
| 0x58E730 / 0x58E990 | StartCombatRound / EndCombatRound | H | 3000 ms round; slot 6 at `0x58F50F`; skip if head = move, controlled, `+0xA8C`, `+0xA90`, dead |
| 0x571030 | UpdateCombatRoundTimer | H | |
| 0x58F7A0 | CSWSCombatRound::IncrementTimer | H | |
| 0x58FB40 | DecrementPauseTimer | H | |
| 0x58FC60 | SetRoundPaused(flag, masterId) → `+0xAA8/+0xAAC` | H | |
| 0x58FCD0 | SetPauseTimer(ms, waitMaster) → `+0xAB0/+0xAB4` | H | |
| 0x58E6D0 | SetSpellCastRound → `+0xA88/+0xA8C` | H | |
| 0x590270 | CSWSCombatRound::AddAction (cap 4, frees overflow) | M | U3 |
| 0x5867E0 | ClearHostileActionsVersus | H | C1 |
| 0x57A560 | SetCombatState | H | party: SetLocalNumber(8,0) |
| 0x5E9340 | CSWVarTable::SetLocalNumber | H | |
| 0x56B8A0 | SpawnInHeartbeatPerception | H | heartbeat slot 0 every 3000-4199 ms; P1/P2 |
| 0x5D0F30 | UpdatePerception(mode 0 all / 1 party / 2 non-party) | H | |
| 0x576DC0 | DoPerceptionUpdateOnCreature | M | |
| 0x5D1750 / 0x5D1400 / 0x5D15F0 / 0x5D1510 | visible list get / add / remove / clear (`+0x1028/+0x102C`) | M | |
| 0x57C990 | GetNearestVisibleEnemyInRange | M | T1 |
| 0x56EFD0 | CSWSCreature::EventHandler | M | OnBlocked only when not player-controlled |
| 0x6FD8D0 | RunScript(slot, oid, 1) | H | |
| 0x73B720 / 0x73B810 | timer init / now (QPC us when available) | H | |
| 0x7AC470 | client controlled-creature input per frame | M | PCM |
| 0x5915F0 | CSWSCombatRound::ClearAllSpecialAttacks | M | |
| 0x51E120 / 0x51E040 | CServerAIMaster::SetAILevel / AddObject | H | 4 = highest |
| 0x586940 / 0x586B70 | GetIsCreatureBumpable / BumpFriends | H | FM1 |
| 0x541080 | CSWSObject::ClearAllActions(clearAttacks) | H | |
| 0x58A330 | SetPartyMemberFlag(flag, apply) | H | FollowInfo alloc/free, level 4/2 |
| 0x5752C0 | CFollowInfo ctor | M | not RemoveMindTrickEffects |
| 0x574500 | CSWSCreature::SaveCreature | H | FM4 |
| 0x5BF7F0 | AIActionFollowLeader (type 0x3D) | M | sets walk state `+0x11B0=3` |
| 0x5BF940 / 0x5BCDC0 | AIActionFollowOwner (0x46) / CheckMoveToFollowRadius (0x40) | M | |
| 0x5C3F70 / 0x5C6340 | QuickWalk / FollowLeader state driver | M | |
| 0x5646C0 | AddPathfindingWaitActionToFront (type 0x1E) | M | |
| 0x7E5DF0 / 0x797F30 / 0x7E8350 | client party SetLeaderBySlot / UI leader switch / ResetFollowTrail | M | FM6 |
| 0x7E6CE0 | per-frame party range monitor (k_hen_enter5m/exit5m) | M | |

Creature fields (H unless noted):
- Script slots `+0x270+8*i`: 0 HB, 1 Notice, 2 SpellAt, 3 Attacked, 4 Damaged, 5 Disturbed, **6 EndRound (+0x2A0)**, 7 Dialogue, 8 Spawn, 9 Rested, 10 Death, 11 UserDef, 12 Blocked, 13 EndDialogue. (M, sub-review; slot 6 verified.)
- `+0x380` pathinfo; `+0x3A0/+0x3A4` dead perception stamp; `+0x39C` mode-1 interval.
- `+0x520` in combat; `+0x524` combat-exit ms; `+0x52C` bPassive / "end instead of approach"; `+0x550` attempted target; `+0x558` last blocked pos; `+0x570` slot-6 fallback ms.
- `+0x104C` orientation lock; `+0x10DC` combat round; `+0x10E8` IsPC; `+0x1198` stats (`+0x146` stance); `+0x11AC` party.
- Combat round: `+0xA84` active, `+0xA88` spell round, `+0xA8C`, `+0xA94` timer, `+0xA9C` length, `+0xAA8` paused/attack in flight, `+0xAB0` pause ms, `+0xAB4` wait master, `+0xADC` NewAttackTarget (load only), `+0xB04` owner, `+0xB08`/`+0xB0C` engaged/master, `+0xB10` master id, `+0xB14` slave id, `+0xB20` round target.

**Name-map rows to demote** (mapper errors; the bodies do not match):
- `0x412C00` WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest (GUI code, callers 0x747010/0x7AC470);
- `0x418990` AIActionWaitForEndOfRound (GUI);
- `0x971DC0` AIActionPartyFollowLeader (not a function in the dump);
- `0x983C20` AIActionTaunt, `0x983D10`, `0x984DE0` AIActionSurrenderToEnemies, `0x9852D0` AIActionStanceMeditative (15-30 byte static-init stubs);
- `0x681880` GetLastAttacker (returns constant 0) and `0x696BF0` GetAttacker (sub-review: names doubtful).

## Stance field stats+0x146: readers (sub-review, spot-checked)

| addr | function | effect |
|---|---|---|
| 0x6D8508 | AIActionPhysicalAttacks | ==11 or ==12 and not controlled → no approach (A1). **The only engine behaviour the stance changes.** |
| 0x5BC66F | AIActionCheckMoveToObjectRadius | dead read (`[ebp-0x74]` never used) |
| 0x77AA57 | AddStanceActions (GUI, controlled only) | not in {9, 11, 12, 13} → writes 9 |
| 0x74DB87 | HUD refresh | 9..13 → ids 0x410+n |
| 0x696222 / 0x696636 | Get/SetNPCAIStyle | Set has no range check (harmless) |
| 0x77D908.. | StanceActionClicked | menu writes |
| 0x6A473C | stats ctor | 0 (behaves as Aggressive) |
| 0x6B146A / 0x6B4C88 | load / save "AIState" | |

Defensive (10) and Support (13) are script-only. The engine treats 0, 9, 10, 13 (and 16 without E-H3) identically.

## Checked, no bug

- **Starvation.** `AIUpdate` puts party and IsPC creatures in AI level 4 every tick (`0x5694B2..0x5694D6`: `CMP [EAX+0x78],4; ... PUSH 4; CALL 0x51E120`), so companions are never in a low list. They can still be slowed by the shared budget, which A1 and P1 inflate.
- **Engaged master/slave.** A party attacker is always master (`0x5912B0`), so companions never wait as a slave. The master unpauses the slave's round in `EndCombatRound` with ECX = **slave's** round (`0x58EB36 MOV ECX,[EBP-0x74]` = slave `+0x10DC`). Correct. A slave whose master is missing is released by `DecrementPauseTimer` ("Master cannot be found").
- **Round end.** `EndCombatRound` clears dead or unresolved ids in `+0x550/+0x568/+0x548/+0x56C/+0x564` and round `+0xB20`. No stale target across rounds. It resets combat mode for non-controlled creatures (`0x58F317`) by design.
- **Target death mid-action.** The attack re-resolves the target id on every entry (`0x51C0A0`), never caching a pointer across ticks. `ResolveAttack 0x6DACF0` handles a NULL target.
- **NewAttackTarget** (`+0xADC`, `0x6D9247..0x6D92C1`) is written only by `LoadCombatRound` (`0x592978`), so it is effectively dead in play. Its `== INVALID` re-test is redundant.
- **NULL combat round** paths (`0x6D8C07` → `EndAttackAction` → `0x5915F0` dereferences ECX) are unreachable: the ctor allocates `+0x10DC` (`0x562290`) and only the dtor clears it.
- **Distance math** in the attack, perception and spot/listen code is squared vs squared. The `desired² - 0.2` slack is a tuning quirk, not a bug.
- **SetSpellCastRound `+0xA8C`** suppresses the end-of-round script for that spell's own parameter (design). It persists until the next StartCombatRound.
- **Event dispatch.**
  - OnBlocked fires for companions (only the player-controlled creature is excluded, `0x5706CF`).
  - OnAttacked, OnDamaged and OnSpellAt skip only dead or dying party members.
  - OnNotice runs only for active-party members.
  - Heartbeat is not party-gated.
  - No engine auto-targeting for idle companions (`CreateNewAttackActions` is controlled-only); they depend on scripts by design.
- **HasTargetBeenDetected** passing all party attackers is design (shared party sight), with one side effect: a companion keeps attacking a target that turned stealthed until C1's clear or a script retargets.
- **Two LOS tests.** The attack uses eye-level LOS excluding src/dst; the MoveToPoint range shortcut uses foot-level LOS with no exclusion. They disagree and drive the Mod 8 symptoms; that scope is already covered.
- **Return codes.** Actions: 1 in progress, 2 and 3 terminal (identical to RunActions), 4 special. U3.md's "2 success / 3 fail" is the pathfinder's convention only.

## Coverage

- **Read fully (asm-checked):**
  - `0x6D7390` (all non-LOS paths, approach branch 0x6D83E4-0x6D8BE7, combat-round tail);
  - `0x5D56B0` (dispatch, result handling, time budget);
  - `0x5D7160`, `0x6DAB20`, `0x6DAC70`, `0x6DAB80`, `0x6DA4F0`, `0x590FF0`, `0x5912B0`;
  - `0x58E730`, `0x58E990`, `0x571030`, `0x58F7A0`, `0x58FB40`, `0x58FC60`, `0x58FCD0`, `0x58E6D0`;
  - `0x5BB2C0`, `0x5644B0`, `0x540CA0`, `0x53FC10`, `0x568540`;
  - `0x569320` (level forcing, fallback, combat timers);
  - `0x5867E0` (C1), `0x56B8A0` blocks 2/3 (P1/P2), `0x57A560` (local 8);
  - `0x73B720`/`0x73B810` (clock);
  - `0x7AC470` around the patch.
- **Read partially:** `0x6D9500` (case 0xA in asm), `0x5BFC00` (parameter unpack and range shortcut only), `0x5BC1D0` (returns).
- **From sub-reviews, spot-checked by the lead reviewer:** C1, P1, P2, local 8, stance table, FM1, FM3, FM4. **Not re-verified:** T1, R1, FM2, FM5-FM7, the event-slot table beyond slot 6, the `+0x570` writers outside `0x6D7390`/`0x569320`.
- **Sub-review disagreement resolved.** The perception sub-review concluded "no dead-end where a companion idles in combat with no event" from the 3 s fallback. A1 is exactly such a dead-end, because the attack prologue resets the fallback on every entry.
- **Follow internals not covered:** `0x5C7AD0`, `0x5CA000`, `0x5CAE00`, `0x5CAF30`, `0x5CB060`, `SetFormation 0x5D4B60`, `MakePlayerCharacter 0x5FE4B0`, `WalkUpdateLocation 0x5C3460`. The mapper's "FollowLeader_*" names at `0x669020`, `0x66B770`, `0x66B8A0`, `0x67AC20` and `0x67DF50` are unverified and several look mis-mapped.
- **Not covered:** `ResolveAttack` damage internals, effect handlers, `0x52BB60` LOS internals, script-side `k_ai_master` behaviour (inferred only), the multiplayer/network paths.
