# Mod Idea 8: Ranged Combat AI (script + engine)

**STATUS (2026-09-30, research complete, no code):** definitive build plan below. Scope decided with the user:
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
| E-H1 hold | `0x006D83EA` | 12 | `8B 91 2C 05 00 00 89 95 8C FC FF FF` | false | `0x006D83FF` (`exclude_from_restore=["eax"]`) | `ecx`=attacker, `ebp`. Return 1 ⇒ end attack (hold) when: party (`+0x11AC`), **not PC** (`+0x10E8==0`), stance ∈ `hold_stances` (default `{16}`), `RangeWeaponEquipped(attacker)`, `los==0` (`[ebp-0xD0]`), `dist² <= (max+0.1)²` (`[ebp-0xBC]`, `[ebp-0x2C]`, 0.1 = double at `0x987DF0`), same area (`0x5453C0(attacker)==[ebp-0x38]`), **target is a creature** (`*(u8*)(*(u32*)(ebp-0x50)+8)==5`; otherwise a companion attacking a door/placeable with no LOS would hold forever) |
| E-H2 retarget gate | `0x006D7C73` | 12 | `8B 88 E8 10 00 00 89 8D 50 FD FF FF` | false | `0x006D7C8C` (`exclude_from_restore=["eax"]`) | `eax`=attacker. Return 1 (behave as IsPC) for the same party/ranged/stance test ⇒ second blocked round retargets a visible enemy in range or fails |
| S-H1 clamp | `0x0077AA5E` | 7 | `89 4D E8 83 7D E8 09` | false | `0x0077AAB4` (`exclude_from_restore=["eax"]`) | `ecx`=state. Return 1 if `state == 0x10` ⇒ skip the reset-to-9 |
| S-H2 row | `0x0077ACBC` | 15 | `8B 55 EC 89 95 08 FF FF FF 8B 85 08 FF FF FF` | false | `0x0077AE77` (`exclude_from_restore=["eax"]`) | `[ebp-0x14]`=state, `[ebp-0x1C]`=row idx, `[ebp+8]`=list ptr (`*list`=row base, row = `base+idx*0x3C`). If `state==0x10`: `row+8 = 0x415`; `row+0x20 = CResRef("ib_ranged")` (16-byte, lower-case, zero padded; `0x710EB0` does it); return 1 |
| S-H3 click | `0x0077D8D4` | 14 | `8B 55 08 89 55 CC 8B 45 CC 2D 10 04 00 00` | false | `0x0077D9CA` (`exclude_from_restore=["eax"]`) | `[ebp+8]`=action id, `[ebp-0x10]`=stats, `ebp`. If id `== 0x415`: `*(u16*)(stats+0x146)=0x10`; `*(int*)(ebp-8)=<floaty strref>`; `*(int*)(ebp-4)=0x3F` (the value vanilla stores for the ranged stance; it is the first argument of `0x740A20`, exact meaning unverified); return 1 |
| S-H4 (optional, cosmetic) | `0x0074DA40` region | — | — | — | — | maps 16 → `0x415`; not designed in detail. Without it, for state 16 `0x74DA40` sets `local_20=-1` and leaves `HUD+0x1C8C` unchanged, so the HUD keeps showing the **previous** stance |

All windows except S-H4 are validated by `check_windows.py` (see `check_windows_output_2026-09-30.txt`), no overlap with any of the 63 live hooks.
Order matters for the consumed hooks: the wrapper runs the stolen bytes **before** the consumed test (framework lesson), and every consumed target above is reachable with those bytes applied.

**Engine engine calls:** `RangeWeaponEquipped 0x0056E1E0` (`thiscall(creature)`), `GetArea 0x005453C0` (`fastcall(obj)`), `CResRef ctor 0x00710EB0` (`thiscall(dst; out, "str")` `RET 8`). Stance test: `*(u16*)(*(u32*)(creature+0x1198)+0x146)`.

**INI (`ranged_ai.ini`):** `enabled=1`, `hold_stances=16`, `retarget_stances=16`, `kite_strref_name`, `kite_strref_msg`, `debug=0`.

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
