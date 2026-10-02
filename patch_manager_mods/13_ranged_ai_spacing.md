# Mod Idea 8: Ranged Combat AI (script + engine)

**STATUS (2026-09-30, research complete; Stage 0 logging DLL built but not reviewed/installed - see 'Build status' at the end):** definitive build plan below. Scope decided with the user:
**party first, enemies later**, delivered as a **new dedicated AI stance** ("Ranged (Kiting)", `aistate 16`) in the stance selector.
Community survey: [`13a_ai_community_research.md`](13a_ai_community_research.md) (no existing mod does kiting or LOS repositioning).
Evidence: `mods678_research/dump_excerpts/<addr>.{c,asm}`, `check_windows_output_2026-09-30.txt`, knowledge DB (`mods678 batch3`, lessons 171-178).

## Root cause (decompiled, Steam)

The party AI script re-issues `ClearAllActions(); ActionAttack(oTarget)` every round (`GN_RunDefaultAIRoutine`), so all movement is the
engine's attack action, `CSWSCreature::AIActionPhysicalAttacks 0x006D7390`. It computes `max` (`MaxAttackRange 0x571570`: blaster `baseitems.maxattackrange`,
0 ⇒ 30 m), `desired` (`DesiredAttackRange 0x5716A0`: 1.6 m + both radii, **weapon independent**) and `los` (`ClearLineOfSight 0x52BB60`, eye height +1.5 m on both ends)
and queues an approach move when **any** of these is true (`0x6D7B30-0x6D83E4`):

| # | Term | Meaning |
|---|---|---|
| 1 | `targetArea != myArea` | different area |
| 2 | `(max+0.1)² < dist²` | too far |
| 3 | `dist² < desired²-0.2` **and `this[0x46b]==0`** | too close (**party members are exempt**) |
| 4 | `los == 0` | **blocked line of sight: this is the bug** |
| 5 | target is a creature and `desired² < local_b4` | other geometry case |

So a blaster user whose shot is blocked keeps walking at the target, often into melee. Whatever the script queues, the next `ActionAttack` re-enters this logic.

## Engine facts established this session

| Fact | Evidence |
|---|---|
| `creature+0x11AC` (`this[0x46B]`) = **is party member** (PC included). The script command returns exactly this field | `0x68BBE0` (`ExecuteCommandIsObjectPartyMember`) |
| `creature+0x10E8` (`this[0x43A]`) = **IsPC**, cached from `stats+0x6C` at load and re-set at runtime by `0x5FE4B0`, whose only caller is `ExecuteCommandSwitchPlayerCreature 0x694400`; enables the LOS-blocked retarget | `0x5FE4B0`, `0x576750 @0x5768E3-0x5768FE`, `0x6D7C73` |
| `creature+0x52C` (`this[0x14B]`) ≠ 0 ⇒ the approach branch **ends the action instead of moving** (`EndAttackAction 0x6DAB20(1)`, returns *complete*). Set to 1 by `InitializeNewAttackTarget 0x6DAC70` | `0x6D83EA-0x6D8411` |
| Action result codes: `DAT_009944B4=1` in progress, `B8=2` failed, `BC=3` complete | `0x55AC80`, `0x6D9500` |
| Stuck check: on `los==0` the attacker compares `this+0x94` (pos) with `this+0x558` (last blocked pos, `Vec3::Equal 0x517980`); equal **and IsPC** ⇒ retarget nearest visible enemy within `max` (`0x57C990`) or fail; otherwise remember the position and carry on to the approach test | `0x6D7C36-0x6D7C86` |
| Stance field: `stats+0x146` (ushort, `stats = creature+0x1198`), read/written by the script commands and the stance menu; **`SetNPCAIStyle 0x696580` does not range-check** | `0x696580`, `0x696190` |
| Script `HasLineOfSight(vSrc, vDst, oSrc, oDst)` passes vectors unchanged to the same engine test; the engine uses foot position **+1.5 m** on both ends (then +0.01 on the source and -0.01 on the target, so effectively 1.51/1.49; for some target types the target point comes from `0x585F10`). OEI's own droid reposition loop (`k_inc_generic.nss:734-798`) passed foot vectors and no objects and notes "works about 60%"; that cause is our **inference**. OEI's `k_trp_power.nss:23` does pass both objects | `0x69AC60`, `0x6D7390 @0x6D7A22-0x6D7B2B` |
| Engine "back off" for ranged units only exists inside melee distance (term 3), and party members never take it | `0x5716A0` |

## Research items (all closed)

- [x] **M8-E1** splice sites found and validated: **hold** at `0x6D83EA`, **retarget gate** at `0x6D7C73` (below). `local_d4` = `los` at `[ebp-0xD0]` (`ClearLineOfSight` returns non-zero when clear; the blocked branch is `los==0`).
- [x] **M8-E2** `this[0x43A]` = IsPC, `this[0x46B]` = IsPartyMember, `this[0x14B]` = "end attack instead of moving". Setting `0x43A` for companions is **not** an option (`CanUseItem 0x5D2F80` also reads it) ⇒ we hook the test instead.
- [ ] **M8-E3** *Do creatures block engine LOS?* **Not resolved statically.** `ClearLineOfSight` excludes src/dst objects, runs a walkmesh test (`0x52D130`, AABB-filtered id list at `param_1[0x14]`) and a mesh list at `area+0x254` (stride `0x4C`, count `+0x250`, entry `+0x38≠0` ⇒ skipped). Whether creatures are in the id list is undecidable from code. **Stage 0 instrumentation decides it** (log `los` at the hold site with an ally on the line).
- [x] **M8-E5** party predicate = `*(int*)(creature+0x11AC) != 0`, no call needed.
- [x] **M8-E6** return codes and flow (table above); "end this attack this round" = jump to the block at `0x6D83FF` (`PUSH 1; CALL 0x6DAB20; MOV EAX,[0x9944BC]; JMP 0x6D93FB`).
- [x] **M8-E7 (enemies, later, notes only)** Same function serves all creatures. For enemies `0x46B==0` and `0x43A==0`: they keep term 3 (step back to `desired` only inside melee distance), never retarget on a block, and always take term 4. Enemy version = same hold hook without the party test, plus a script mirror in `GN_RunSkirmishAIRoutine`'s neighbours. Out of scope for v1.
- [x] **M8-S1** `aiscripts.2da` (2DA.bif, no Override copy, columns `label, name_strref, aistate`): rows `0 Aggressive 123703 9`, `1 Defensive 123704 10`, `2 RangedSupport 123705 11`, `3 Stationary 123706 12`, `4 Support 126013 13`. Row count drives the menu (`*(table+0x24)`); rows with `aistate==10` are skipped.
- [x] **M8-S2** the engine does **not** accept `aistate 16` without code: `AddStanceActions 0x77A9E0` **resets any value outside {9,0xB,0xC,0xD} to 9** and its icon/id jump table (`0x77AE98`, `JA` for state>13) leaves the row empty; `StanceActionClicked 0x77D890` only maps ids `0x410..0x414`. Three small hooks fix it (below).
- [x] **M8-S3** icons are `Override/ib_*.tga`, 65 580 bytes each (128×128 RGBA). v1 reuses `ib_ranged`; v2 ships a recoloured `ib_kite.tga` (same size).
- [x] **M8-SC1..SC4** script flow, LOS fix, algorithm and compile path below.

## Definitive build plan

### 1. Engine DLL `ranged-ai` (party only; both Steam SHAs)

| # | Address | Len | Original bytes | skip | Consumed exit | Inputs → action |
|---|---|---|---|---|---|---|
| E-H1 hold | `0x006D83EA` | 12 | `8B 91 2C 05 00 00 89 95 8C FC FF FF` | false | `0x006D83FF` (`exclude_from_restore=["eax"]`) | `ecx`=attacker, `ebp`. Return 1 ⇒ end attack (hold) when: party (`+0x11AC`), **not PC** (`+0x10E8==0`), stance ∈ `hold_stances` (INI default `11`; `16` from Stage 3), `RangeWeaponEquipped(attacker)`, `los==0` (`[ebp-0xD0]`), `dist² <= (max+0.1)²` (`[ebp-0xBC]`, `[ebp-0x2C]`, 0.1f = float const at `0x987DF0`), same area (`0x5453C0(attacker)==[ebp-0x38]`, non-null), **target is a creature** (`*(u8*)(*(u32*)(ebp-0x50)+8)==5`; otherwise a companion attacking a door/placeable with no LOS would hold forever) |
| E-H2 retarget gate | `0x006D7C79` (was `0x6D7C73`, see rule below) | 6 | `89 8D 50 FD FF FF` | false | `0x006D7C8C` (`exclude_from_restore=["eax"]`) | `eax`=attacker. Return 1 (behave as IsPC) for the same party/ranged/stance test ⇒ second blocked round retargets a visible enemy in range or fails |
| S-H1 clamp | `0x0077AA5E` | 7 | `89 4D E8 83 7D E8 09` | false | `0x0077AAB4` (`exclude_from_restore=["eax"]`) | `ecx`=state. Return 1 if `state == 0x10` ⇒ skip the reset-to-9 |
| S-H2 row | `0x0077ACBC` | 9 (was 15, re-sited for the EAX rule) | `8B 55 EC 89 95 08 FF FF FF` | false | `0x0077AE77` (`exclude_from_restore=["eax"]`) | `[ebp-0x14]`=state, `[ebp-0x1C]`=row idx, `[ebp+8]`=list ptr (`*list`=row base, row = `base+idx*0x3C`). If `state==0x10`: `row+8 = 0x415`; `row+0x20 = CResRef("ib_ranged")` (16-byte, lower-case, zero padded; `0x710EB0` does it); return 1 |
| S-H3 click | `0x0077D8D4` | 6 (was 14, re-sited for the EAX rule) | `8B 55 08 89 55 CC` | false | `0x0077D9CA` (`exclude_from_restore=["eax"]`) | `[ebp+8]`=action id, `[ebp-0x10]`=stats, `ebp`. If id `== 0x415`: `*(u16*)(stats+0x146)=0x10`; `*(int*)(ebp-8)=<floaty strref>`; `*(int*)(ebp-4)=0x3F` (the value vanilla stores for the ranged stance; it is the first argument of `0x740A20`, exact meaning unverified); return 1 |
| E-H3 ranged rules (0.2.0, review) | `0x006D850F` | 6 | `89 85 8C FE FF FF` | **true** | none (resume `0x6D8515`) | `eax`=stance, `ebp`: `*(int*)(ebp-0x174) = (stance==16 && ranged_rules) ? 11 : stance` (that local is read only by the "Ranged/Stationary companion does not approach" CMPs at `0x6D876A/0x6D8773/0x6D8A7E/0x6D8A87`) |
| S-H4 (optional, cosmetic) | `0x0074DBDD` | 7 | `C7 45 E4 FF FF FF FF` | **true** | none (resume `0x74DBE4`) | `ebp`: `*(int*)(ebp-0x1C) = (*(int*)(ebp-0x28)==0x10) ? 0x415 : -1` (design: R-D result below; window validated).

All windows including S-H4 are validated by `check_windows.py` (see `check_windows_output_2026-09-30.txt`), no overlap with any of the 82 live hooks (`live_hooks_2026-09-30b.tsv`, `check_windows_output_2026-09-30d.txt`).
Order matters for the consumed hooks: the wrapper runs the stolen bytes **before** the consumed test (framework lesson), and every consumed target above is reachable with those bytes applied.

**Engine engine calls:** `RangeWeaponEquipped 0x0056E1E0` (`thiscall(creature)`), `GetArea 0x005453C0` (`thiscall(creature)`, no stack args), `CResRef ctor 0x00710EB0` (`thiscall(dst; out, "str")` `RET 8`). Stance test: `*(u16*)(*(u32*)(creature+0x1198)+0x146)`.

**Framework rule for `consumed_exit_address` hooks (found by the 2026-09-30 Opus review):** the wrapper calls the handler, restores registers (except those in `exclude_from_restore`, here EAX), **then runs the stolen bytes, then `TEST EAX,EAX`**. So during the stolen bytes EAX = handler result, not the game's EAX: **the stolen bytes must not read or write EAX** (a read faults or misbehaves; a write overrides the handler's verdict). E-H1 (ECX/EDX only) and S-H1 are fine; the original E-H2 window at `0x6D7C73` (`MOV ECX,[EAX+0x10E8]`) would have faulted on first hit and was moved to `0x6D7C79`. The original S-H2 (15 bytes ending in `MOV EAX,[EBP-0xF8]`) and S-H3 (14 bytes with `MOV EAX,..; SUB EAX,0x410`) violated the rule; **re-sited in 0.2.0** to the EAX-free 9- and 6-byte windows in the table. `check_windows.py` now prints `[WARN]` for such windows.

**INI (`ranged_ai.ini`, 0.2.0):** `hold=1`, `retarget=1`, `hold_stances=16`, `retarget_stances=16`, `ranged_rules=1`, `kite_strref_msg` (fallback 123710), `kite_icon=ib_ranged`, `log`, `log_all`.

### 2. Data files

- `Override/aiscripts.2da` (binary V2.b via pykotor, complete file = vanilla 5 rows + one): `5 | RangedKite | <strref_name> | 16`. Nothing else in the repo edits it (checked `scripts/*.py`); the compatch replay must include it if the Override is rebuilt.
- TLK: two new entries appended to `dialog.tlk` by a dedicated idempotent append script (backs up `dialog.tlk`, refuses if the strings already exist, records itself in `replay/README.md`; design = plan item R-F; `scripts/02_merge_2da_and_tlk.py` is a one-shot Echoes merge that refuses to re-run, so it cannot carry these): name `Ranged (Kiting)`, message `Ranged (Kiting) Behavior\nKeeps its distance.\nRepositions when blocked.\nSwitch to melee if\nopponent closes.` (same style as strref `0x1E33E`). The two new strref numbers go into the 2DA and the INI. Fallback with no TLK edit: reuse `123705` ("Ranged") and `0x1E33E`; the menu then shows two "Ranged" rows.
- `Override/ib_ranged.tga` is reused in v1.

### 3. Script layer (`k_ai_master.ncs`)

Vanilla `k_ai_master.nss` is unchanged; everything lives in `k_inc_generic.nss` (compiled into `k_ai_master.ncs`; the live Override has no `k_ai_master`, `k_inc_generic` or `k_hen_*`).

1. **Constant** `int NPC_AISTYLE_PARTY_KITE = 16;`
2. **Dispatch** in `GN_DetermineCombatRound` (next to `NPC_AISTYLE_PARTY_RANGED`, `k_inc_generic.nss:405`):
   `else if (nNPC_AI == NPC_AISTYLE_PARTY_KITE) { if (GN_CheckRangeFromLeader(10.0f) == TRUE) return; GN_RunKiteAIRoutine(oIntruder); return; }`
3. **`GN_RunKiteAIRoutine(oIntruder)`** (party non-leader, alive; otherwise `return 0`):
   - Weapon config first: reuse `GN_CheckRangedPreferred()` (swap to melee if hit in melee, exactly like `PARTY_RANGED`).
   - If no ranged weapon in the right hand ⇒ `GN_RunDefaultAIRoutine(oIntruder)` (behave as vanilla).
   - `oTarget = GN_DetermineAttackTarget(oIntruder)`. If invalid ⇒ default routine.
   - **Kite trigger** (all true): cooldown local `SW_NUMBER_KITE_COOLDOWN == 0`; nearest seen enemy `oThreat` within **3.5 m**; and (`GetCurrentHitPoints/GetMaxHitPoints < 0.6` **or** `oThreat` holds a melee weapon via `GN_GetWeaponTypeFromBaseItem`); and we are not the leader.
   - **Candidate search** (≤ 12 tries): `v = GetRandomDestination(OBJECT_SELF, 8)`; reject unless `dist(v, oThreat) >= dist(me, oThreat) + 2.0`, `dist(v, leader) <= 10`, and `HasLineOfSight(Vector(v.x,v.y,v.z+1.5), Vector(tp.x,tp.y,tp.z+1.5), OBJECT_SELF, oTarget)`. Keep the candidate with the largest distance from `oThreat`.
   - Found ⇒ `ClearAllActions(); ActionMoveToLocation(Location(vBest, GetFacing(OBJECT_SELF)), TRUE); SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, 2);` skip the attack this round.
   - Not found ⇒ fall through.
   - **LOS reposition** (independent of kiting): if `oTarget` valid and `!HasLineOfSight(eye(me), eye(target), OBJECT_SELF, oTarget)`: same candidate loop with score = LOS true and `dist(v, target) <= 15.0` and near the leader; found ⇒ move (cooldown 1); none ⇒ fall through (the engine hold then keeps us from walking in).
   - Otherwise `GN_RunDefaultAIRoutine(oIntruder)` (normal attack).
   - Decrement the cooldown each call. Per round budget: **one movement or one attack**, never both.
4. **`HasLineOfSight` spec (M8-SC2):** always call `HasLineOfSight(vSrc, vDst, oSrc, oDst)` with **both z+1.5 m eye heights** and **both objects**; then it matches the engine test to within 0.01 m. (If `GetArea(attacker)` is 0 the engine skips the LOS test and leaves `los=0`, which also satisfies the hold condition.)
5. `SW_NUMBER_KITE_COOLDOWN`: pick an unused `SetLocalNumber` index after grepping `k_inc_generic.nss`/`k_inc_gensupport.nss` (vanilla uses 0,1,2,8,9,15 and others; verify at build time).

**Compile recipe (M8-SC4):**
`~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/compile.sh "$PWD/out/k_ai_master.ncs" k_ai_master.nss k_inc_generic.nss` (**absolute output path**: `compile.sh` does `cd "$work"` before `cp script.ncs "$out"`, a relative path aborts; the second file is the modified include; `compile.sh` lower-cases it over the vanilla one). Use nwnnsscomp only ([[kotor2-script-compiler]]).
**Conflicts:** GearHead's *KotorII Improved AI* (deadlystream 2417) and any other `k_ai_master` replacer conflict at that file; it is **not installed** here. If ever combined, merge its optional source into the same `k_inc_generic.nss` (the kite branch is one `else if` plus one new function).

### 4. Engine/script contract

- The DLL never moves anyone: it only **declines to approach** (hold) and **extends the vanilla retarget** to ranged party members in stance 16.
- The script never fights the DLL: repositioning is a single `ActionMoveToLocation` issued in `k_ai_master`; the engine's next `ActionAttack` re-enters the attack logic where the hold applies only when the shot is blocked and the target is in range.
- Shared state = the stance itself (`stats+0x146`, read by both). No extra channel is needed. If one is ever needed, use a script-side local boolean that the DLL reads via `CSWVarTable`.
- Round timeline for a ranged companion blocked by an ally: round N script repositions (or does nothing) → engine holds (the first blocked check only stores the position) → the retarget needs a **second blocked check at exactly the same position** (`Vec3::Equal` is exact float equality), i.e. at least 2 rounds after any move, and any push resets it → then retarget a visible enemy in range or fail → script re-evaluates.

### 5. Build, staged test plan, rollback

- **Order (reordered 2026-09-30 build plan): engine Stage 0 logging runs BEFORE the script prototype** (E3 decides whether the script's LOS-reposition is worthwhile).
- **Stage 0 (logging only, no behaviour change):** hooks E-H1/E-H2 log `creature id, +0x11AC, +0x10E8, stance, ranged, los, dist², max, area equality`. **Settles E3** (does an ally on the line give `los==0`) and confirms the two flags per companion.
- **Stage 1:** script-only prototype with the **existing** stance 11 (no engine hooks): kite + LOS fix; Peragus/Telos fight test. Rollback: delete the `.ncs`.
- **Stage 2:** E-H1/E-H2 with `hold_stances=11`.
- **Stage 3:** stance 16 (S-H1..S-H3 + 2DA + TLK + `.ncs` dispatch).
- Rollback at every stage: remove the DLL from `patch_config.toml`, delete `Override/aiscripts.2da` and `Override/k_ai_master.ncs`. If a save had stance 16 stored the clamp resets it to 9 (Aggressive) at the next menu build, so it is safe.

### 6. Remaining risks and in-game checks

1. **E3** unresolved until Stage 0 (whether allies block engine LOS). If creatures do not block, hold matters only for pillars/doors and the mod's value is mostly the kite/LOS script.
2. `this[0x14B]` is also set by `InitializeNewAttackTarget`; the hold path relies on the already-existing "end action" block, but a new-target/hold combination should be watched in logs. The round-2 stuck check needs the remembered blocked position (`creature+0x558`, compared by `Vec3::Equal 0x517980`, returns 1 when equal) to survive the hold: no `+0x558` access appears in `EndAttackAction 0x6DAB20`, `0x57D370`, `0x5915F0` or `0x5B7AE0` (a scan of the whole dump for `[reg+0x558]` finds only `0x665F50`; `0x6D7390` reaches the field through `ADD reg,0x558` address arithmetic, so that scan is not exhaustive for it), but **Stage 0 must confirm it in the log** (`pos == lastBlockedPos` on round 2).
3. **Understated risks (reviewer):** (a) E-H2 makes the consumed retarget path call `SendFeedbackMessage 0x584DA0(0xDA)`, which vanilla ran only for the PC, so companions may spam the feedback log; check in Stage 0/2. (b) Each hold ends the action as *complete*; if the script does not re-issue `ActionAttack` next round the companion just stands (unverified whether end-of-round AI keeps firing after an attack-less action) - Stage 2 must watch for idling. (c) closed: `mods678_research/verify_windows_both_exes.py` shows all 11 windows byte-identical in the LAA exe (`4AB72FC1`) and the base exe (`6A522E71`).
4. The kite algorithm depends on `GetRandomDestination` quality (it returns walkable points but no LOS/leash guarantees); tune tries/scores in play.
5. Text: the two new strrefs (or the fallback) must be settled before the 2DA is built.
6. **Check A (user, 2 min):** open the stance menu today and confirm **four** rows (Aggressive, Ranged, Stationary, Jedi Support; aistate 10 is skipped before it is appended), so the new row appears as the fifth.
7. **Check B:** after Stage 3, select the new stance, close and reopen the menu (the clamp must keep it), save, reload (persistence in `stats+0x146`).

## R-E result (2026-09-30): kite cooldown local-number index

Engine range: `CSWVarTable::GetLocalNumber 0x5E9310` accepts index 0..0x1F (bytes at `table+0x14+idx`), values 0..255. Vanilla scripts (all 638 `.nss` from `Scripts.bif`, grep of Get/SetLocalNumber literals + the `SW_NUMBER_*` / `WALKWAYS_*` constants) use 0-6, 8-16, 29 and 99 (99 is another call, out of range). Named constants: COMBO_ROUTINE 3, COMBO_INDEX 4, LAST_COMBO 5, ROUND_COUNTER 6, COMBAT_ZONE 7, HEALERAI_THRESHOLD 8, HEALERAI_PERCENTAGE 9, COOLDOWN 10, COOLDOWN_FIRE 11.
**Chosen: index 20** (inside the documented 12..28 window, unused by vanilla). Re-grep `Override/*.nss` of installed mods before the script ships (none present at last check).

## R-B result (2026-09-30): feedback message `0xDA` and attack-less completion

- `SendFeedbackMessage 0x584DA0(this=creature; msgId, msgObj)` starts with `0x51C600(creature+4)` (asm: `ECX=[ECX+4]` then `CALL 0x52F330`, `RET 4`). `0x52F330(this=serverApp, id)` walks the player list at `serverApp+0x10064` and returns the entry whose controlled object (`0x632A20(entry)` = `GetObject(entry+0x28)`, type byte `+8` above a threshold) has object id `== id`; returns 0 for id `0x7F000000`. So the function sends its client message **only when the creature is currently controlled by a player** (the active PC/leader); for every other party member it returns at the first test with nothing sent. E-H2's extended retarget therefore cannot spam feedback for companions; when the companion is the controlled character it already behaved as IsPC in vanilla. Confidence: medium-high (static); Stage 2 log should still show no extra `0xDA` (no way to observe it in our log, so check for on-screen/feedback-log spam instead).
- Attack-less completion: the hold uses the game's own "end this attack" block (`EndAttackAction 0x6DAB20(1)`, returns complete `DAT_009944BC`), the same thing vanilla does when `this[0x14B] != 0`. The party script re-issues `ClearAllActions(); ActionAttack` at the next combat round (`GN_RunDefaultAIRoutine`), so there is no dependency on end-of-round AI firing; worst case a held creature idles for a round while blocked, which is the intended behaviour. No design change. Stage 2 still watches for permanent idling.

## R-D result (2026-09-30): S-H4 (HUD current-stance marker) designed

`FUN_0074DA40` (HUD refresh on character change) maps the stance `*(u16*)(stats+0x146)` to an action id with a jump table (`9..13 -> 0x410..0x414`); the default branch at **`0x74DBDD`** is `MOV dword ptr [EBP-0x1C],-1` (7 bytes, `C7 45 E4 FF FF FF FF`; reached by the `JA` at `0x74DBA4`; merges at `0x74DBE4`). The remembered id is stored to `HUD+0x1C8C` only when `> 0`, and `0x74CFE0` then looks for the row whose id (`row+8`, stride `0x3C`) equals `*(int*)(HUD+0x1C7C+slot*4)` in the stance action list built by `AddStanceActions`, which S-H2 extends with id `0x415`. **Spec:** hook `0x74DBDD`, `skip_original_bytes=true`, handler(`ebp`): `stance = *(int*)(ebp-0x28)` (`local_2c`); `*(int*)(ebp-0x1C) = (stance == 0x10) ? 0x415 : -1`. Resume at `0x74DBE4` (no consumed exit needed). Added to `check_windows.py` (`M8 HUD stance default branch`), output `check_windows_output_2026-09-30b.txt`, both executables byte-identical (`verify_windows_both_exes.py` ALL MATCH). Only cosmetic; it is part of Stage 3 and can be dropped if the HUD behaves without it. `HUD+0x1C8C` is also reached indexed as `[reg*4+0x1C7C]` by `0x751750/0x751B00/0x752450/0x7525D0` (review 2026-09-30); they do not care which id is stored.

## R-F result (2026-09-30): data delivery (TLK + 2DA)

Live state: `dialog.tlk` has 136 538 entries (last = "Allows one to channel the Dark Side..." from the Sith Sabers mod); vanilla stance strings: `123703 Aggressive`, `123704 Defensive`, `123705 Ranged`, `123706 Stationary`, `126013 Jedi Support`, messages `123709..123711` (`0x1E33E` = 123710 is the Ranged message). No `override/aiscripts.2da` exists yet (vanilla row set comes from `2DA.bif`).
Design (all live writes are user-gated, backed up, recorded in `~/Games/KOTOR2_Mod_Workspace/replay/README.md`):
1. **`scripts/append_ranged_kite_tlk.py`** (new, repo `.venv` pykotor): default = dry run; reads live `dialog.tlk`; **idempotent**: if the last two entries already equal our two strings it prints their strrefs and exits 0 (no second append); otherwise `--apply` copies `dialog.tlk` to `~/Games/KOTOR2_Mod_Workspace/backups_kitetlk_<HHMM>/`, appends name `Ranged (Kiting)` and message `Ranged (Kiting) Behavior\nKeeps its distance.\nRepositions when blocked.\nSwitch to melee if\nopponent closes.`, writes `ranged_ai_strrefs.json` (`{"name": N, "message": N+1}`), then the replay README line. Expected strrefs `136538`/`136539` unless another session appends first, so always read the JSON, never hard-code.
2. **`aiscripts.2da`**: read the vanilla table with pykotor from `2DA.bif`, append row `5 | RangedKite | <name strref> | 16`, write binary to a **staging dir**, `scripts/verify_2da.py`, then copy to `Override/` (user-gated). Patched repo `.venv` only (stock pykotor 2.3.12 blanks some 2DAs).
3. **Fallback** without any data write: INI `kite_strref_name=123705`, `kite_strref_msg=123710` (menu shows two "Ranged" rows, one of them kiting). Stage 3 can ship and be tested with the fallback first; the TLK append is a cosmetic upgrade.
4. The DLL reads `kite_strref_name`/`kite_strref_msg` from `ranged_ai.ini` for the floaty text of S-H3; the 2DA only needs the name strref.


## Build status (2026-09-30)

- **Stage 0 engine DLL built; independently reviewed by Opus 2026-09-30, one BLOCKER fixed (E-H2 moved to `0x6D7C79`), S2/S3/N1-N3 fixed; rebuilt (`RangedAI.kpatch` md5 94642e19...), NOT installed:** `ranged_ai_src/` (id `ranged-ai`, `RangedAI.cpp`, `kotor2-steam-aspyr.hooks.toml`, `manifest.toml`, `exports.def`, `ranged_ai.ini`); build copy `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/RangedAI/` with `RangedAI.kpatch`. `-Wall -Wextra` clean, both hook windows byte-identical to `check_windows_output_2026-09-30b.txt`. Hooks E-H1 `0x6D83EA` (`RangedAiHold(ecx, ebp)`, consumed exit `0x6D83FF`) and E-H2 `0x6D7C73` (`RangedAiRetarget(eax, ebp)`, consumed exit `0x6D7C8C`) are installed **with** the consumed exits, but the handlers return 0 until `ranged_ai.ini` has `hold=1` / `retarget=1`, so stage 0 is behaviour-neutral and stage 2 is an INI change (hot reload, no reinstall). Stage 3 (stance 16: S-H1..S-H4, 2DA, TLK, script dispatch) needs more hooks and a new build.
- **What the S0 log records** (`ranged_ai_log.txt`; party members only unless `log_all=1`; throttled to state changes / first 16 / every 2 s per creature + a 10 s summary): per hold-site event creature id, `+0x11AC` party, `+0x10E8` IsPC, stance, ranged, `los`, `dist2`, `max`, `desired`, in-range, my/target area, target pointer + type, back-off flag `[ebp-0x168]` (review: it is the too-close/back-off flag, set at `0x6D83DA`; reaching the hook already means the approach branch was taken), ranged local `[ebp-0x164]`, `+0x52C`, positions of attacker/target/`+0x558`, and the hold decision it would take. E-H2 events are logged always (rare). Frame slots (from the asm): `los [ebp-0xD0]`, `dist2 [ebp-0xBC]`, `max [ebp-0x2C]`, `desired [ebp-0x4C]`, `desired2 [ebp-0xB0]`, `targetArea [ebp-0x38]`, `target [ebp-0x50]`, attacker `[ebp-0x49C]`.
- **E3 test protocol (user, after install):** in an OPEN area with no walls (any `los==0` there can only come from creatures/objects), set a companion with a blaster to stance Ranged, line up an ally directly between the companion and an enemy, fight a few rounds; note the clock time of each scenario so the log lines (`t=`) can be matched. Also fight near a pillar/door for the wall case. Decision gate: if allies never produce `los==0`, hold only matters for static geometry.
- Open questions the log answers: (closed by review: E-H1 is only reached on the approach path; `backoff` is the too-close flag.) Does `lastBlocked` equal `me` on round 2 (stuck-position persistence)? Do companions have `pc=0` and `party=1` as assumed?
- Next for Mod 8 (review done): user-gated Stage 0 install (procedure: `mods678_research/stage_and_verify_install.py`), E3 log review, then script prototype (Stage 1), INI `hold=1` (Stage 2), stance 16 build (Stage 3), user checks A and B.

## 0.2.0 build (2026-09-30 night): stance 16 "Ranged (Kiting)" end to end, not installed

**Design decision:** hold/retarget apply only to stance 16 (`hold_stances=16 retarget_stances=16`); stance 11 and every vanilla stance stay vanilla. Party members of every stance are still logged (E3 evidence from the combined play session).

- **Windows re-sited (EAX rule):** S-H2 `0x77ACBC` len 9 `8B 55 EC 89 95 08 FF FF FF` (consumed `0x77AE77`), S-H3 `0x77D8D4` len 6 `8B 55 08 89 55 CC` (consumed `0x77D9CA`, which reloads EAX first). S-H1 `0x77AA5E` unchanged: the wrapper's PUSHFD/POPFD around `TEST EAX,EAX` keeps the stolen `CMP [ebp-0x18],9` flags for the `JZ` on the resume path (`wrapper_x86.cpp` consumed-exit block). S-H4 `0x74DBDD` skip-only. `check_windows_output_2026-09-30e.txt` ALL OK (no WARN), both exes ALL MATCH.
- **B2 answers:** `[ebp-4]` in `StanceActionClicked` is the **tutorial.2da row** passed to `0x740A20` -> `0x79E9D0` (`0x7CF960` option flag + `0x898EB0` "Message_PC0" lookup + shown-once); 0x3D/0x3F/0x40/0x41/0x42 per stance, 0xFF none. Stance 16 uses **0x3F** (the Ranged tutorial, shows only if never seen). `[ebp-8]` = message strref shown in the stance message box (`GUI+0x98 -> +0x15410`). Row fields: the jump-table case bodies set only `row+8` (action id) and `row+0x20` (icon CResRef via `0x710EB0`); name, callback `0x77D890`, `+0x1C`, `+0x38` come from the common path. S-H2 writes the CResRef bytes directly (lower case, zero padded).
- **DLL 0.2.0** (`RangedAI.cpp`): handlers `RangedAiStanceClamp/Row/Click/HudStance`; INI `kite_strref_msg` (fallback 123710) and `kite_icon` (default `ib_ranged`); hold counter in the 10 s summary; each stance click, row fill and HUD marker logged. kpatch md5 `40556b7d` (DLL `f224d4f3`), 6 exports == 6 hooks, bytes == check output.
- **Data:** `scripts/append_ranged_kite_tlk.py` (dry run: 136538 `Ranged (Kiting)`, 136539 message; 123710 uses `\n`), `scripts/build_ranged_kite_2da.py` (row `5 RangedKite <N> 16`). Both write only at install time (user-gated).
- **Script:** `ranged_ai_src/script/kite_routine.nss` + `build_kite_script.py`. Dispatch `NPC_AISTYLE_PARTY_KITE=16` after PARTY_RANGED (leash `GN_CheckRangeFromLeader(10)` first). Routine order: cooldown (local 20; let an own move finish) -> kite from a seen enemy within 3.5 m that fights in melee (or any enemy below 60% HP): 12 `GetRandomDestination(8)` candidates, >= +2 m from it, <= 10 m from the leader, clear shot (both eye heights + both objects), furthest wins -> move + queued `ActionAttack`, cooldown 2 -> else `GN_CheckRangedPreferred()` (cornered + hit in melee => melee config) -> blocked shot => nearest candidate with a clear shot within 15 m of the target, not within 2.5 m of the nearest enemy -> move + attack, cooldown 1 -> else `GN_RunDefaultAIRoutine`. Toolchain proof byte-identical; output `k_ai_master.ncs` md5 `c174539b`, arg-order clean. Other vanilla direct callers of `GN_DetermineCombatRound` (a_atkonend, a_force_combat, k_combat_rnd, k_def_*) run the default routine for stance 16.
- Rollback: `hold=0 retarget=0`; delete `Override/aiscripts.2da` + `Override/k_ai_master.ncs`; restore `dialog.tlk` from the backup; remove the id. A save with stance 16 and no DLL is reset to 9 by the vanilla clamp at the next menu build.

### Mod 8 0.2.0 Opus review (2026-09-30 night) and fixes

- **BLOCKER fixed:** `manifest.toml` description had bare `"` inside a TOML string (Tomlyn/tomllib reject it) -> quotes removed; every toml now parse-checked before a build.
- **S1 (design gap, fixed): the root-cause table above is only true for stances 9/10/13.** `AIActionPhysicalAttacks` copies the stance to `[ebp-0x174]` at `0x6D850F` and, at `0x6D876A` / `0x6D8A7E`, skips the approach move entirely for stance 11 (Ranged) / 12 (Stationary) when the attacker is not the controlled creature. So vanilla Ranged companions already never walk in; stance 16 would have been treated like Aggressive. New skip-only hook **E-H3 `0x6D850F`** stores 11 for stance 16 (`ranged_rules=1`); window in `check_windows_output_2026-09-30f.txt`, both exes match. E-H1 still covers the in-range blocked case for any listed stance. Consequence for E3: for stance 11 the hold hook is rarely reached; the log still shows whether allies give `los==0`.
- **S2 fixed:** a successful kite now clears the "hit in melee" local (8), otherwise the next round swapped to melee and walked back in. **S3 fixed:** the LOS reposition never lands within `KITE_THREAT_RANGE + 0.5` of the nearest enemy (no kite/LOS ping-pong).
- **S4 fixed:** S1/S2 log lines (AddStanceActions runs on every action-bar rebuild) limited to 20 each, so the 6000-line cap stays for E-H1/E-H2.
- **S5 (doc): rollback nuance.** `AddStanceActions` only runs for the controlled character, and save/load (`0x6B3D10`/`0x6AFED0`, "AIState") keeps 16 unclamped. After removing the DLL a companion saved on stance 16 keeps it until the player takes control of it; until then the vanilla scripts run the default routine for it and the engine treats it as Aggressive. Harmless; picking any stance fixes it.
- **S6 fixed:** `build_ranged_kite_2da.py` refuses when a live `Override/aiscripts.2da` exists that is not vanilla (or vanilla + our row).
- NITs fixed: message shortened to 4 lines like 123710 (`Ranged (Kiting) Behavior / Keeps its distance, / finds a clear shot. / Melee if cornered.`); TLK flags compared with vanilla 123710; dated backup dir; the TLK tool prints the `kite_strref_msg=` INI line. Not changed: `wr<T>` probes readability only (all targets are stack/heap); cooldown counts DCR calls and is saved with the creature (harmless).
- Rebuilt: `RangedAI.kpatch` md5 `385e274a` (DLL `7392073e`), 7 exports == 7 hooks; `k_ai_master.ncs` md5 `de7654cb` (proof byte-identical, arg-order clean).
- Verified by the reviewer: wrapper order and S-H1 flags, S-H2 row fields/0x710EB0, S-H3 `[ebp-4]`/`[ebp-8]`, S-H4 frame, window bytes, no other users of ids 0x410..0x415, other stats+0x146 readers (no clamp in Get/Set 0x696190/0x696580), script dispatch parity, TLK apply on a copy. Not verified: whether an E-H1-held attack still yields the end-of-round event (watch for idling in the play session), the Message_PC0 link (static only).
- **Focused re-review (Opus, 2026-09-30 night): no blockers.** Verified E-H3 (EAX = zero-extended stance, `[ebp-0x174]` written once / read only by the 4 CMPs, no aliasing, no branch into the window, E-H1 on every path before it), the H6d fix, script fixes, data tools. Nits applied: E-H3 stores directly (no per-call probe); kite spots must be outside `KITE_THREAT_RANGE + 0.5` **and within 15 m of the target** (with E-H3 stance 16 never walks back to an out-of-range target, like vanilla Ranged - play-test item: watch for a companion idling with its target out of range); 2DA tool names a stale own row. Final: `RangedAI.kpatch` md5 `0b86bcf6` (DLL `d3fe4148`), `k_ai_master.ncs` md5 `94db8304`.

### 0.2.0 installed (2026-09-30 23:57)
Live with Mod 6 in one combined install (22 ids, kpatch md5 `0b86bcf6`, DLL `d3fe4148`). Data: dialog.tlk strrefs **136538** (`Ranged (Kiting)`) / **136539** (message); `override/aiscripts.2da` (row 5 `RangedKite 136538 16`) md5 49a735c7; `override/k_ai_master.ncs` md5 94db8304; `$G/ranged_ai.ini` `hold=1 retarget=1 ranged_rules=1 hold_stances=16 retarget_stances=16 kite_strref_msg=136539`. Backup/stage/rollback: `mods678_research/HANDOFF_2026-09-30.md` "INSTALLED". Awaiting the play session: E3, idling after an E-H1 hold, far-target idling, tutorial link, message fit.

### Companion AI review (2026-10-01): items for the Mod 8 fix round
Three Opus reviews of the whole party AI: `mods678_research/ai_review_2026-10-01/` (README = triage). These hit stance 16 directly:
- **A1:** E-H3 maps 16->11, so a stance-16 companion whose target is out of range loops in the attack action. The prologue `0x6D740F` resets the 3 s fallback timer `+0x570` every entry, so the end-of-round script never runs, and the companion idles until given an order. This is play-test item (b), and the same applies to vanilla Ranged/Stationary.
- **A2:** an E-H1 hold ends the action, so the fallback re-runs the end-of-round script within 3 s. That closes open item (a) statically.
- **E2:** the shout-driven first combat action runs the vanilla `GN_DetermineCombatRound` compiled into `k_hen_<name>dlg`.
- **K1:** the kite target lacks `GetIsEnemy`/`GetIsDead` checks.
- **K2:** consecutive kites are uncapped.
- **M1:** a swap to an empty weapon config 2 leaves the companion unarmed.
- **E12:** the cooldown at local 20 persists between fights.

### 0.3.0 + companion AI installed (2026-10-01 23:36)
- `ranged-ai` 0.3.0 is live (`RangedAI.kpatch` a7c902ef, DLL d41a9123, `kite_approach=1`: stance 16 maps to 11 only with a ranged weapon and the target in range).
- The kite routine v2 (K1-K3, M1, E12) and E2 routing are live in `override/k_ai_master.ncs` b003ea41 with 16 router scripts.
- The new `companion-ai` patch carries the AI trace (`ai_trace.txt`) and engine fixes A1/FM1/FM4/A4/C1. A1 fixes the stance 11/12/16 out-of-range idle loop (open item (b)).
- Install record: `mods678_research/HANDOFF_2026-09-30.md` "Companion AI round INSTALLED". Play checklist: given in chat on 10-01. Backlog: "Session close".

### Background research (2026-10-02, static, during the play session)
Reports in `mods678_research/ai_research_2026-10-02/`:
- **R1:** FM2 is verified (pathfind failure = success + wait loop). A1 does not let combat lapse before the E2 fallback.
- **New issue:** the `a1_retarget` metric mismatch. A retarget up to ~2 body radii out of range ends silently; proposed fix `a1_retarget_strict`.
- **R2:** script designs for E4, M8, L1-L3, E9 (user decision) and the E7 decision (don't).
- **R3:** enemies already run our `k_ai_master`, so enemy AI = gate flips. The design input for E7 `hold_enemies` is also there.
- **R4:** the medpac rule design.

Summary: `mods678_research/HANDOFF_2026-09-30.md` "Phase 0 background research DONE".

### Kiting vs vanilla Ranged (summary for the user, 2026-10-02)
| | vanilla Ranged (11) | Ranged (Kiting) (16) |
|---|---|---|
| melee enemy within 3.5 m | holds position and shoots; may swap to melee after melee hits (M1 rule) | steps to a spot at least 2 m further away (any close enemy when HP < 60%), within 10 m of the leader and 15 m of the target |
| kite limit | - | at most 2 in a row, then a shooting round (K2) |
| cornered | - | the vanilla melee-swap rule |
| blocked shot | the engine never walks it (frozen; companion-ai A1 now retargets or rethinks within ~3 s) | moves to a clear-shot spot (step 3); the engine holds instead of walking into melee (E-H1) and retargets on the 2nd block (E-H2) |
| target out of range | never approaches (A1 retarget/rethink) | walks in until in range (0.3.0 `kite_approach=1`; also with a melee weapon) |
| after a move | - | always queues an attack |

Shared by both: A1 freeze fix (11/12/16), E1 follow after the fight, H2 heal any party member, FM1 bump. `hold`/`retarget` are scoped to stance 16 in `ranged_ai.ini`, so vanilla Ranged stays vanilla. None of this is play-tested yet.


## Planned shared state and shared code (doc 19 / 19b, added 2026-10-02)
**Nothing here is built yet.** When a piece lands, use it instead of extending this patch's local copy. The ids (A1, B2, ...) refer to [`19b_cross_patch_sharing_survey.md`](19b_cross_patch_sharing_survey.md), which has the evidence. Line numbers there are from 10-02.
- **Registry interfaces** (A-ids) use the RFC #180 registry from [`19_shared_logging_design.md`](19_shared_logging_design.md). Call `require` lazily at first use, and keep the current local code as the fallback when the provider is not installed.
- **Common helpers** (B-ids) live in `Patches/Common` on the KPM fork. Edit them in `patch_manager_mods/_shared_src/` and sync with `scripts/check_shared_headers.py --sync`. New code includes `"SafeRead.h"`, not `"../_shared/SafeRead.h"`.
- **Rollout:** shared pieces are consolidated on our fork first. Each one goes upstream inside the first mod PR that uses it, so order this mod's PR after the PR that introduces anything it reuses. Migrating this patch needs its owner's go.
- **Logging:** move to `KpLog.h` channels and `kplog.ini` (doc 19 Phase 6). Keep the tag text exactly as it is, because the parsers depend on it.
- **A5 `k2.rangedai` (provider):** companion AI's A1 hook (0x6D853E) reads the `[ebp-0x174]` stance this patch's E-H3 (0x6D850F) writes. It also hard-codes stance 16 (`companion_ai.ini:51-58`, `CompanionAiLogic.h:216-217`), while this patch owns the definition (`RangedAI.cpp:44`, `:87`). If this patch is absent or `ranged_rules=0`, companion AI silently misses stance 16. Provide `kite_stance()` (0 when the mapping is off) and `maps_to_ranged(raw, ebp)`.
- **B3 ini helper:** loadIni (:107).
- **B7:** `rd`/`wr`, `parseMask` (~:66, identical to companion AI).
