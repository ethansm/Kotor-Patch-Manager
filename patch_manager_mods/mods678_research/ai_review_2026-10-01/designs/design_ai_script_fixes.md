# KOTOR II party AI: script-side fixes (design, not deployed)

Date: 2026-10-01. Status: **design + scratch prototype only. Nothing was written to the repo, the workspace or the game folder.**
Inputs: `patch_manager_mods/mods678_research/ai_review_2026-10-01/` (README, `review_ai_scripts_decision.md`, `review_ai_scripts_events.md`, and the engine report for local 8), `ranged_ai_src/script/build_kite_script.py` and `kite_routine.nss`, workspace `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/`.

Scratch prototype (all under `scratchpad/scriptfix/`; `S` below):
- `party_ai_patches.py`: the patch list (the single source of the replacements in section 6) plus a builder that applies it to the vanilla sources, asserts each anchor occurs exactly once, compiles everything with nwnnsscomp and runs the arg-order check.
- `kite_routine_v2.nss`: the new Mod 8 routine. `party_ai_fixes.nss`: the new helper bodies.
- `build/src/` and `build/ncs/`: patched sources and the 17 compiled outputs. `diffs/`: unified diffs against vanilla.
- `proof/vanilla_proof.txt`: per-script byte-identical proofs. `bif/`: vanilla .ncs/.nss read from `Scripts.bif`.
- `kit/`: a private copy of the recompile kit (`compile.sh`, `vanilla/` includes, its own `wineprefix`). The real kit writes `build.XXXX` dirs and wine state into its own folder, so I copied it to stay read-only. Invocation, compiler and includes are unchanged.

## 1. Results at a glance

- **Every replacement compiles and its anchor is unique.** The anchored replacements cover the kite fixes K1/K2/K3/E12, plus M1, H1, H2, M2-M5, M7, M9, M10, E1, E3, E6, E8, E10, E11, E13, L4 and E2.
  - The prototype applies all of them and asserts anchor count == 1 for each (section 6 lists the counts).
  - nwnnsscomp builds all 17 outputs.
  - `check_ncs_arg_order.py --override-only` reports **0 scripts with swapped arguments**.
- **Byte-identical vanilla rebuild proof: PASS for all 16 E2 scripts.** That is the 13 `k_hen_<name>dlg`, `a_atkonend`, `a_force_combat` and `k_combat_rnd`, each compared with its `Scripts.bif` .ncs (section 5). k_ai_master passes too, and so do `k_hen_leadchng`, `k_oei_endconv` and `k_oei_spawn` (spare, for a future E5).
- **The patch-list mechanism reproduces the live Mod 8 build byte for byte.** Applying only the two Mod 8 insertions with the old `kite_routine.nss` gives `k_ai_master.ncs` md5 `94db8304…`, the same as live `override/k_ai_master.ncs`. The generated `k_inc_generic.nss` is also byte-identical to `mod8_kite_script/src/k_inc_generic.nss`. So the refactor can be checked against today's build.
- **Nothing would be shadowed.** No module capsule and no override file holds any of the 16 E2 scripts. I scanned all 1284 `.mod/.rim/.erf` in `Modules/`; the scanner was checked against the known 003EBO `k_hen_combend01`/`k_hen_damage01` hits. Override has only our `k_ai_master.ncs`, the mod `k_def_*.ncs` (left alone) and the Obscured Tomb `k_oei_userdef.ncs` (not touched).
- **E2 decision: route, don't recompile (option b).** The 13 dlg scripts send shouts to `k_ai_master` event 2004. That event is their shout path line for line. The 3 wrappers call `k_ai_master` event 3001, which is `GN_DetermineCombatRound()`.
  - Outputs are 2.7-3.8 KB each (62 KB total) instead of about 112 KB each (1.8 MB).
  - Future include changes rebuild only `k_ai_master`.
  - Rolling back `k_ai_master` alone stays consistent with them.
- **All vanilla fixes are gated to party members**, so NPC behaviour stays vanilla on every path. That holds both through our `k_ai_master` and through the 52 NPC scripts that embed their own copy of the include. Making the enemy versions too is a user decision (section 9).
- **Local 8 is fine.** The engine's `SetCombatState(0)` zeroes local 8 for party members at combat exit (`0x57A5C1`). That is exactly the "new fight, no stale hit-in-melee mark" semantics the kite wants, and it already covers E6(b). Details in 3.6.

## 2. Per-fix table

Scope: **party** = changes behaviour only when `IsObjectPartyMember(OBJECT_SELF)`, either through an explicit gate or because the code is already inside a party-only path. NPCs run the vanilla code. **party context** = the gate is on the creature running the script (for example the party member who is being attacked). **hench** = `k_ai_master` hench events 2001-2014, which only fire for creatures that use `k_hen_*` scripts (the party, plus any NPC a module gives those scripts).

| id | scope | risk | file : anchor (vanilla line) | old -> new (short) |
|---|---|---|---|---|
| K1 | party (stance 16) | low | kite_routine (whole-file v2) | target from `GN_DetermineAttackTarget` dropped unless `GetIsEnemy && !GetIsDead`; steps 2/4 then run the vanilla way |
| K2 | party (16) | low-med (tunable) | kite_routine | new local 21 streak; at most `KITE_MAX_STREAK`=2 kites, then one DCR shoots; streak resets on any DCR with cooldown 0 and no kite |
| K3 | party (16) | low | kite_routine | `GN_KiteFindThreat`: the nearest 3 seen enemies within 3.5 m, first live melee one (any when hurt), instead of only the nearest |
| M1 | party (11, 16) | low | generic `GN_CheckRangedPreferred` (3800) | `if ( nType2 != 2 )` -> `if ( nType2 == 1 )` (never swap to an empty or non-weapon config 2) |
| E12 | party (16) | low | kite_routine + E1 helper | locals 20/21 cleared when the routine starts out of combat, and by the stance combat exit |
| local 8 | - | none | (engine) | no change; see 3.6 |
| H1 | party | low | generic 1006 + 2512-2517 | gate `!= -1` -> party: `== TRUE`; REVITALIZE `bValid` only with `nID != -1 && GetCreatureHasTalent`, else `tUse = TalentSpell(-1)` so the fallback at 2747 cannot cast it |
| H2 | party (by construction) | low-med | generic `GN_CheckIfInjured` 2322/2326/2336/2341 | race test on `oP1`/`oP2`; branches test `fDown01`/`fDown02` |
| M2 | party | low | gensupport `GN_FindAOETarget` 2523 | AoE centre must be `GetIsEnemy && !GetIsDead && GetObjectSeen` for party callers (the inner scan is skipped otherwise) |
| M3 | party | low | generic `GN_DetermineAttackTarget` 3429 | Aggressive fallback skips debilitated enemies for party callers; NPC loop unchanged |
| M4 | party | low | gensupport `GN_GetTargetedForcePower` 3369/3436/3574/3579 | local `nDroid` renamed `nDisable`; NPCs get `nDroid = FALSE` (the old shadowed result) |
| M5 | party (by construction) | low-med | generic `GN_CheckIfPoisoned` 2256 | `for(nCnt; nCnt > 2; nCnt++)` -> `for(nCnt = 0; nCnt < 3; nCnt++)` |
| E1 | party (10-13, 16) | **med** | generic DCR, before `if(nNPC_AI == NPC_AISTYLE_MELEE_ATTACK)` (292) | new `GN_PartyStanceCombatExit`: no seen enemy -> cure poison / heal, else `GN_DoPostDCRChecks()` (CancelCombat + follow), as Aggressive already does |
| E3 (+M6 shout path) | party | low-med | generic `GN_RespondToShout` 2049 | party: `GN_PartyApproachUnseen` (4 m melee / 15 m otherwise, Stationary holds, then `ActionDoCommand(DCR(oIntruder))`); NPCs keep the 2 m run |
| M7 | party context | low | gensupport `GN_GetWeaponTypeFromBaseItem` 2291, `GN_GetWeaponType` 2245 | baseitems 104/105 ranged, 106/107 melee (checked in the live override `baseitems.2da`, 109 rows) |
| M9 | party (stance 10) | low | generic 676 | `GetDistanceToObject(oClose)` -> `GetDistanceBetween(oClose, oLeader)` |
| M10 (30 m part) | party | low | generic 838-844 | target > 30 m: party switches to a live seen `oClose` within 30 m instead of `CancelCombat` |
| E6 (a) | hench | low | k_ai_master 2005 (693) | weapon type 0 counts as melee only if the attacker is valid and within 5 m |
| E8 | party | low | generic before 835 | a dead `oTarget` (stale last hostile target) is dropped |
| E10 | party (hench) | low (behaviour) | k_ai_master 2009 (816) | party members never `DOOR_ACTION_BASH` |
| E11 (+M6 2006 path) | hench | low | k_ai_master 2006 (764) | stand-off move (4/15 m) instead of a run onto the damager's spot; none for 11/12/16 |
| E13 | hench (leader) | low | k_ai_master 2005 (723) | third leader shout removed (`else` -> `else if(not leader)`) |
| L4 | party (PC-only buff trees) | low | generic 2623 | `GetCreatureHasTalent(tBuff_06)` -> `(tBuff_05)` |
| E2 dlg | party | low | 13 `k_hen_<name>dlg` (`nMatch` line; `GN_RespondToShout` line) | shouts -> `ExecuteScript("k_ai_master", OBJECT_SELF, 2004); return;`; the dead call is removed |
| E2 wrappers | all (NPC behaviour identical) | low | `a_atkonend`, `a_force_combat`, `k_combat_rnd` | `GN_DetermineCombatRound()` -> `ExecuteScript("k_ai_master", OBJECT_SELF, 3001)` |

**Not in this round** (section 9): E5 (skipped as instructed), E4, E7, E9/L7, M8, M10 (sticky-target part), E6(c), L1, L2, L3, L5, L6, L8, and the H2 "self first + medpacs" extension.

## 3. Mod 8 kite routine (stance 16)

`kite_routine.nss` is replaced as a whole file; there are no anchors, because it is ours and is appended at EOF. The full v2 text is in Appendix A, and the diff against the live routine is `S/diffs/kite_routine.diff`.

### 3.1 K1: target must be a live enemy
After `oTarget = GN_DetermineAttackTarget(oIntruder)`:
`if (GetIsObjectValid(oTarget) && (!GetIsEnemy(oTarget) || GetIsDead(oTarget))) oTarget = OBJECT_INVALID;`
This is the fallback to vanilla handling:
- Step 1 then shoots at the threat itself, which is a seen enemy by construction.
- Step 3 needs a valid target, so it is skipped.
- Step 4 (`GN_RunDefaultAIRoutine`) does its own `GetIsEnemy` gating, as vanilla does.

I deliberately did **not** add `GetObjectSeen` to step 3. Perception sight is LOS-based, so a target whose shot is blocked is often unseen, and requiring "seen" would disable step 3 in exactly the case it exists for.

### 3.2 K2: streak cap
New local number **21** (`SW_NUMBER_KITE_STREAK`). Vanilla uses no index 21 (literals 0, 8, 9, 11-16, 29; `SW_NUMBER_*` 3-11; walkways 0-2). mod120 For Mandalore uses 16-22, but only on summoned non-party Mandalorians, which never run stance 16.
- A kite increments it.
- On any DCR with cooldown 0 that does not kite, it resets to 0. That covers three cases: no threat, cap reached, or no spot found.
- With `KITE_MAX_STREAK = 2` the pattern under constant pressure is kite, kite, shoot (a whole round, since nothing re-plans until the next DCR), kite, kite, shoot, and so on.
- Cooldown DCRs (`nCool > 0`) skip step 1 and leave the streak alone.

The other option in the review was to kite only when `GetAttackTarget(oThreat) == OBJECT_SELF`. I did not use it: nwscript says `GetAttackTarget` "only works when oCreature is in combat", and an approaching enemy may not count yet. It is listed as an optional tweak.

### 3.3 K3: threat scan
`GN_KiteFindThreat(bHurt)` checks the nearest 1..3 seen enemies. It stops at the first one beyond 3.5 m, because `GetNearestCreature` is ordered by distance. It returns the first live one that is melee (or any, when we are below 60% HP). That costs up to 3 `GetNearestCreature` calls, and only on DCRs with cooldown 0 and a ranged weapon.

### 3.4 M1 (the vanilla fix, used by step 2)
The fix goes into the shared `GN_CheckRangedPreferred`, which is party-only: only stance 11 and the kite call it. A kite-local copy is not needed. With M7, base items 106/107 in config 2 count as melee too.

### 3.5 E12: cooldown leak
Two resets, both cheap:
- (a) At routine entry, `if (!GetIsInCombat(OBJECT_SELF))` clears 20 and 21. The first DCR of a fight usually runs before the combat state is set.
- (b) `GN_PartyStanceCombatExit` (E1) clears both when no enemy is seen.

Worst case, if (a) also fires mid-fight while the combat state is briefly off, a kite is re-planned one DCR call early. That is harmless. A time-based cooldown is not worth it: NWScript only has the game clock.

### 3.6 Local 8 ("hit in melee") vs the engine reset at `0x57A5C1`
`SetCombatState(0)` zeroes local 8 for party members (engine report, "Other low items"). This is correct for the kite:
- Local 8 is only read by `GN_CheckRangedPreferred` (step 2, and stance 11).
- It is written by `k_ai_master` 2005 (set to 2) and by every kite (set to 0).
- So a zero at combat exit means a new fight starts with no stale melee mark. That is what we want, and it already resolves E6(b).
- A combat-state drop in mid-fight would only make the companion switch back to its ranged config sooner, which is right for a kiter.
- The same reset probably also clears E6(c) (the Remote's "heal droids" read of index 8) at each combat exit, if the engine counts the Remote as a party member (`+0x11AC`). That is unverified, which is why E6(c) is deferred.

**No change needed.** If a later design wants the mark to survive the combat exit, it must move to a free index (22/23), not to 8.

## 4. Vanilla fixes: notes beyond the table

- **H1:** Both halves are needed:
  - the gate (Jedi with no Revitalize);
  - the branch (a Jedi who has Revitalize but lacks the Force points). Without `tUse = TalentSpell(-1)`, the generic fallback at line 2746-2756 would still `ClearAllActions` + cast the unaffordable power and return TRUE. Vanilla's own buff code uses `TalentSpell(-1)` as "invalid".

  For NPCs: an NPC on Jedi Support goes idle in vanilla whenever a *party member* is knocked out, because `GN_CheckIfKnockedOut` reads the player's party. Fixing that makes enemies stronger, so it is user decision U2.
- **H2:** This only fixes detection. Heal, Cure and Master Heal are cast on `OBJECT_SELF` as party-area powers, so nothing else changes.
  - Expect more Force-point spending: a companion with Heal now heals any member under 50% on each DCR until that member is above 50% or the Force points run out.
  - The "self first" extension (which would make medpacs and repair kits reachable) is **not** included. Without an extra `GetCreatureHasTalent(tSpe11_05)` guard, a companion with Drain Life but no medpac talent (for example Visas) below 50% would hit the medpac branch with a talent it does not have, which is the H1 idle loop again. It would also let the AI consume shared-inventory medpacs (U3).
- **M5:** Cure, Heal and Master Heal remove poison in TSL. If a poison survives the cure, the companion re-casts each DCR until its Force points run out (`GetCreatureHasTalent` stops it). That is acceptable.
- **M2:** The extra filter also lowers the L8 instruction cost for party callers, because non-candidates skip the inner 4 m scan. It also lets the area-global shape cursor see more of the outer list.
- **E1** is the only medium-risk item:
  - **What it does.** For stances 10-13 and 16, when the member sees **no** enemy, the DCR:
    - cures poison or heals first (as Default AI does);
    - otherwise runs `GN_DoPostDCRChecks()`: `CancelCombat`, `ClearAllActions`, `ActionFollowLeader`, plus the door and placeable cases.
  - **Why it matches Aggressive.** Aggressive already behaves this way in vanilla whenever Default AI finds nothing. With E8 it also no longer "succeeds" on a corpse.
  - **Behaviour changes to watch:**
    - (a) Ranged/Support/Kite companions follow the leader as soon as they see no enemy, instead of idling until the 6 s heartbeat.
    - (b) Kite step 3 (regain a shot) and Jedi Support buffing no longer run while *no* enemy is seen.
    - (c) The k_ai_master 2006 "move to an unseen damager" is effectively dead for these stances, because it is queued behind `ActionFollowLeader`. That is also already true for Aggressive in vanilla.
  - **Placement and safety.** It sits inside the party branch of `GN_DetermineCombatRound`, which is gated by `!GetUserActionsPending()`, so player orders are never wiped.
  - **Alternative not taken.** I considered a post-routine test on `GetCurrentAction()`. I rejected it because the engine semantics of `GetCurrentAction` right after queueing are not verified (the Ghidra dump has no `0x7906E0`).
- **E3:** `GN_MyPrintShoutString` is a no-op in vanilla (its `PrintString` is commented out), so only the `GN_MyPrintString` lines ("Shout Clear 1800/1900") show the shout path in a trace.
- **E10** is a behaviour choice that I count as clearly unintended, given the designers' own "Party members won't help with bashing anymore". Revert it if you want companions to bash.
- **E11:** With E1 in place it is mostly defense-in-depth (see E1 (c)). It still matters for Aggressive and for solo mode.
- **E2 wrappers (all creatures):** NPCs that run `a_atkonend` now get our `k_ai_master` DCR. Every fix in it is party-gated, so for NPCs this is the vanilla DCR, the same as their k_ai_master-routed events today.
- **NPC parity statement:** For non-party creatures, the only behaviour change anywhere in this design is E6(a) and E11, and only for NPCs that a module gives `k_hen_*` event scripts. Everything else is either gated by `IsObjectPartyMember` or sits in code only party members reach.

## 5. E2: routing vs recompiling, with the proofs

### 5.1 Byte-identical vanilla rebuild proof (run in scratch, same invocation as `build_kite_script.py`)
**Method:**
- `compile.sh <abs out.ncs> <script.nss>` with no extra includes. The kit layers the 638 vanilla `.nss` and runs nwnnsscomp (TSLPatcher build) under Proton Wine.
- The output is compared with `cmp` against the `.ncs` read from `Scripts.bif`.
- I read the BIF with my own KEY/BIF reader (`S/kotorio.py`, `S/extract.py`). That reader also gives the `.nss`, and every source I read matched `mod8_kite_script/vanilla_all/` exactly. The workspace `vanilla/k_ai_master.ncs` (the one `build_kite_script.py` proves against) is byte-identical to the BIF copy.

| script | result | BIF md5 | bytes |
|---|---|---|---|
| k_hen_attondlg | PASS | 435e845d | 106407 |
| k_hen_baodurdlg | PASS | 42c774de | 106033 |
| k_hen_discipdlg | PASS | a7259a42 | 106256 |
| k_hen_g0t0dlg | PASS | 3feab2fb | 106033 |
| k_hen_hanharrdlg | PASS | 3feab2fb | 106033 |
| k_hen_hk47dlg | PASS | 3feab2fb | 106033 |
| k_hen_hndmaiddlg | PASS | b87e6702 | 106148 |
| k_hen_kreiadlg | PASS | a5b5a973 | 106382 |
| k_hen_manddlg | PASS | 3feab2fb | 106033 |
| k_hen_miradlg | PASS | 3feab2fb | 106033 |
| k_hen_remotedlg | PASS | 3feab2fb | 106033 |
| k_hen_t3m4dlg | PASS | 3feab2fb | 106033 |
| k_hen_visasdlg | PASS | 8cf24bb4 | 106315 |
| a_atkonend | PASS | 70324753 | 102631 |
| a_force_combat | PASS | 2f0815d0 | 102556 |
| k_combat_rnd | PASS | 2f0815d0 | 102556 |
| k_ai_master (re-check) | PASS | fc102c65 | 121637 |
| spare: k_hen_leadchng / k_oei_endconv / k_oei_spawn | PASS / PASS / PASS | ff894416 / 51f7b30b / e0ca230a | 3160 / 735 / 16950 |

Seven dlg scripts compile to the same bytes (`3feab2fb`), because their `SelectAndRunDLG` is the empty template. `a_force_combat` and `k_combat_rnd` are also identical to each other.

Both options recompile these 16 sources, so the proof is required either way. It passes for all of them.

### 5.2 Option (a): recompile the 16 against our include
- Tried for 5 samples against the patched include: `k_hen_attondlg` 114118 B, `k_hen_miradlg` 113744, `k_hen_visasdlg` 114026, `a_atkonend` 109927, `a_force_combat` 109852. That is about 1.8 MB for the 16.
- **Parity:** each script embeds its own copy of the full combat AI. It matches `k_ai_master` only as long as all 17 are rebuilt together on every include change.
- **Risk:** 17 files must always move as a set. A partial rollback (for example only `k_ai_master`) leaves 16 scripts running a different DCR than the 2003/2005/2006 events. A mod that later ships its own `k_hen_<name>dlg` would need a full merge.

### 5.3 Option (b), recommended: route to `k_ai_master`
- **dlg scripts (E2a/E2b):**
  - Right after `int nMatch = GetListenPatternNumber();` add `if(nMatch != -1) { ExecuteScript("k_ai_master", OBJECT_SELF, 2004); return; }`.
  - The now-unreachable `GN_RespondToShout(...)` call is removed. Leaving it in would keep the whole vanilla combat AI compiled into the file.
  - **Parity, line for line:** for `nMatch != -1` the vanilla dlg runs: AI-off check, `GetCommandable()`, `GN_RespondToShout(GetLastSpeaker(), nMatch, GetLastHostileActor(shouter))`, no `ResetDialogState`, then `SignalEvent(1004)` if `SW_FLAG_EVENT_ON_DIALOGUE`. Case 2004 (`k_ai_master.nss:643-673`) runs exactly the same.
  - The early `return` avoids a double 1004 signal. A naive replacement of just the call line would have fired it twice.
  - The event context (`GetListenPatternNumber`, `GetLastSpeaker`) survives `ExecuteScript`. That is BioWare's own design: `k_hen_dialogue01` and every live `k_hen_*01` event script are exactly `ExecuteScript("k_ai_master", OBJECT_SELF, event)`.
  - The literal `2004` is used because `KOTOR_HENCH_EVENT_ON_DIALOGUE` lives in `k_inc_switch`, which the dlg scripts do not include.
  - Conversations (`nMatch == -1`) are untouched, so the per-companion `SelectAndRunDLG` is kept.
- **Wrappers (E2c-e):** `GN_DetermineCombatRound()` becomes `ExecuteScript("k_ai_master", OBJECT_SELF, 3001)`. Case 3001 is exactly `GN_DetermineCombatRound()`.
  - In `a_atkonend` it becomes `DelayCommand(0.3, ExecuteScript(...))`, because `ExecuteScript` is a void action.
  - `OBJECT_SELF` is evaluated when the command is scheduled, as before.
- **Verified in the compiled output:** the dlg NCS pushes `CONSTI 2004; CONSTO 0x0 (OBJECT_SELF); CONSTS "k_ai_master"; ACTION 8 (ExecuteScript, 3 args)`, the correct last-first order, and the arg-order check is clean. Its engine calls are now only `BeginConversation, ClearAllActions, EventUserDefined, ExecuteScript, GetCommandable, GetFirstPC, GetLastSpeaker, GetListenPatternNumber, GetLocalBoolean, GetPartyLeader, ResetDialogState, SignalEvent`: no combat AI at all.
- **Sizes:** the 13 dlg are 3432-3806 B and the wrappers 2711-2786 B, 62 KB total. `k_ai_master` is 129812 B.
- **Risk:** one extra script instance per shout (negligible).
  - Rollback stays consistent in every combination, because the routers always run whatever `k_ai_master` is live. With BIF vanilla `k_ai_master`, router + vanilla 2004 is exactly vanilla behaviour.
  - Re-applying the 2 edits to some mod's future copy of a dlg script is trivial.
- **Decision: (b).**

## 6. Full anchored replacement list

Conventions:
- **Old** is the anchor. It must occur exactly once in the vanilla file and is replaced whole by **New**.
- The texts are shown with LF. The sources are CRLF, and the builder converts the texts with `crlf()` as `build_kite_script.py` does.
- "anchor count" was measured on the vanilla file. The prototype builder asserts the same count on the CRLF text, and for the 13 dlg scripts it asserts it per file. All counts are 1 (see `S/build_log.txt`).
- Line numbers are 1-based in vanilla (CR stripped).
- The replacements are applied top to bottom, and no two of them overlap.
- **Appended to `k_inc_generic.nss` at EOF**, in this order: `kite_routine.nss` v2 (Appendix A), then `party_ai_fixes.nss` (Appendix B). Their prototypes come from the MOD8-proto replacement.
- `k_ai_master.nss` is now patched too (E6, E10, E11, E13). The proof keeps using the unmodified file.

### `k_inc_generic.nss`

#### MOD8-proto (party) - vanilla line 188, anchor count 1

Old (the anchor; replaced whole):
```c
int GN_CheckRangedPreferred();
```
New:
```c
int GN_CheckRangedPreferred();
//Mod 8 (ranged-ai) - party stance 16 "Ranged (Kiting)": keeps its distance, repositions when the
//shot is blocked, switches to melee if cornered. Body at the end of this file.
int NPC_AISTYLE_PARTY_KITE = 16;
int GN_RunKiteAIRoutine(object oIntruder = OBJECT_INVALID);
//Party AI script fixes (design_ai_script_fixes.md). Bodies at the end of this file.
int GN_PartyStanceCombatExit(int nNPC_AI);
void GN_PartyApproachUnseen(object oIntruder);
```

#### MOD8-dispatch (party) - vanilla line 419, anchor count 1

Old (the anchor; replaced whole):
```c
                    GN_RunRangedSupportAIRoutine(oIntruder);
                    return;
                }
                else if (nNPC_AI == NPC_AISTYLE_PARTY_STATIONARY)
```
New:
```c
                    GN_RunRangedSupportAIRoutine(oIntruder);
                    return;
                }
                else if (nNPC_AI == NPC_AISTYLE_PARTY_KITE)
                {
                    //Mod 8: same leash as Ranged Support, then the kite routine.
                    if (GN_CheckRangeFromLeader(10.0f) == TRUE)
                    {
                        return;
                    }
                    GN_RunKiteAIRoutine(oIntruder);
                    return;
                }
                else if (nNPC_AI == NPC_AISTYLE_PARTY_STATIONARY)
```

#### E1 (party) - vanilla line 292, anchor count 1

Old (the anchor; replaced whole):
```c
                if(nNPC_AI == NPC_AISTYLE_MELEE_ATTACK)
```
New:
```c
                //Party AI fix E1 (party only): stances 10-13 and 16 never reach GN_DoPostDCRChecks below.
                if(GN_PartyStanceCombatExit(nNPC_AI) == TRUE)
                {
                    return;
                }
                if(nNPC_AI == NPC_AISTYLE_MELEE_ATTACK)
```

#### E8 (party) - vanilla line 835, anchor count 1

Old (the anchor; replaced whole):
```c
    //RWT-OEI 10/31/04 - If our target is more than 30 meters away,
```
New:
```c
    //Party AI fix E8 (party only): never keep a dead last target (GetLastHostileTarget has no not-dead
    //guarantee); with no seen enemy the routine now returns FALSE and the combat exit runs.
    if(IsObjectPartyMember(OBJECT_SELF) && GetIsObjectValid(oTarget) && GetIsDead(oTarget))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E8: dropped dead last target");
        oTarget = OBJECT_INVALID;
    }
    //RWT-OEI 10/31/04 - If our target is more than 30 meters away,
```

#### M10b (party) - vanilla line 840, anchor count 1

Old (the anchor; replaced whole):
```c
        //Okay, target is too far. Just drop out of combat all
        //together.
        oTarget = OBJECT_INVALID;
        CancelCombat(OBJECT_SELF);
```
New:
```c
        //Okay, target is too far. Just drop out of combat all
        //together.
        //Party AI fix M10 (party only): a party member switches to the nearest seen enemy instead.
        if(IsObjectPartyMember(OBJECT_SELF) && GetIsObjectValid(oClose) && GetObjectSeen(oClose) &&
           !GetIsDead(oClose) && GetDistanceToObject(oClose) <= 30.0f)
        {
            GN_MyPrintString("GENERIC DEBUG *************** Party M10: target beyond 30 m, closest becomes target");
            oTarget = oClose;
        }
        else
        {
            oTarget = OBJECT_INVALID;
            CancelCombat(OBJECT_SELF);
        }
```

#### M9 (party) - vanilla line 676, anchor count 1

Old (the anchor; replaced whole):
```c
                if ( GetIsObjectValid(oClose) && GetDistanceToObject(oClose) > 5.0f )
```
New:
```c
                //Party AI fix M9: measure from the leader, as the comment above says (was from OBJECT_SELF).
                if ( GetIsObjectValid(oClose) && GetDistanceBetween(oClose, oLeader) > 5.0f )
```

#### H1a (party) - vanilla line 1006, anchor count 1

Old (the anchor; replaced whole):
```c
    if (GN_CheckSeriesRevitalize() != -1)
```
New:
```c
    //Party AI fix H1: GN_CheckSeriesRevitalize returns TRUE/FALSE, never -1, so this gate was always open.
    //Party members only; NPCs keep the vanilla gate (user decision).
    if ((IsObjectPartyMember(OBJECT_SELF) && GN_CheckSeriesRevitalize() == TRUE) ||
        (!IsObjectPartyMember(OBJECT_SELF) && GN_CheckSeriesRevitalize() != -1))
```

#### H1b (party) - vanilla line 2515, anchor count 1

Old (the anchor; replaced whole):
```c
            tUse = TalentSpell(nID);
            bValid = TRUE;
            bHostile = FALSE;
```
New:
```c
            tUse = TalentSpell(nID);
            bValid = TRUE;
            //Party AI fix H1: only with the power and the Force points for it; an invalid talent also
            //keeps the generic fallback below from casting it.
            if (IsObjectPartyMember(OBJECT_SELF) && (nID == -1 || !GetCreatureHasTalent(tUse)))
            {
                GN_MyPrintString("GENERIC DEBUG *************** Party H1: no usable Revitalize");
                bValid = FALSE;
                tUse = TalentSpell(-1);
            }
            bHostile = FALSE;
```

#### H2a (party) - vanilla line 2322, anchor count 1

Old (the anchor; replaced whole):
```c
        if(GetIsObjectValid(oP1)&& GetRacialType(oP0) != RACIAL_TYPE_DROID)
```
New:
```c
        //Party AI fix H2: race test on the member itself (was oP0 for all three).
        if(GetIsObjectValid(oP1)&& GetRacialType(oP1) != RACIAL_TYPE_DROID)
```

#### H2b (party) - vanilla line 2326, anchor count 1

Old (the anchor; replaced whole):
```c
        if(GetIsObjectValid(oP2) && GetRacialType(oP0) != RACIAL_TYPE_DROID)
```
New:
```c
        if(GetIsObjectValid(oP2) && GetRacialType(oP2) != RACIAL_TYPE_DROID)
```

#### H2c (party) - vanilla line 2336, anchor count 1

Old (the anchor; replaced whole):
```c
        else if(GetIsObjectValid(oP1) && !GetIsDead(oP1) && (fDown00 < 0.5 && fDown00 > 0.0))
```
New:
```c
        //Party AI fix H2: each branch tests its own member's ratio (all three tested fDown00).
        else if(GetIsObjectValid(oP1) && !GetIsDead(oP1) && (fDown01 < 0.5 && fDown01 > 0.0))
```

#### H2d (party) - vanilla line 2341, anchor count 1

Old (the anchor; replaced whole):
```c
        else if(GetIsObjectValid(oP2) && !GetIsDead(oP2) && (fDown00 < 0.5 && fDown00 > 0.0))
```
New:
```c
        else if(GetIsObjectValid(oP2) && !GetIsDead(oP2) && (fDown02 < 0.5 && fDown02 > 0.0))
```

#### M5 (party) - vanilla line 2256, anchor count 1

Old (the anchor; replaced whole):
```c
        for(nCnt; nCnt > 2; nCnt++)
```
New:
```c
        //Party AI fix M5: the loop never ran (nCnt = 0 is never > 2).
        for(nCnt = 0; nCnt < 3; nCnt++)
```

#### M3 (party) - vanilla line 3429, anchor count 1

Old (the anchor; replaced whole):
```c
            while(!GetIsDebilitated(oTarget) && GetIsObjectValid(oTarget))
```
New:
```c
            //Party AI fix M3 (party only): skip debilitated enemies (the test was inverted, so the nearest
            //stunned/held/asleep enemy was preferred). NPCs keep the vanilla order (user decision).
            int nPartyM3 = IsObjectPartyMember(OBJECT_SELF);
            while(GetIsObjectValid(oTarget) &&
                  ((nPartyM3 && GetIsDebilitated(oTarget)) || (!nPartyM3 && !GetIsDebilitated(oTarget))))
```

#### M1 (party) - vanilla line 3800, anchor count 1

Old (the anchor; replaced whole):
```c
            if ( nType2 != 2 )
            {
                //Our second config is NOT ranged, switch to it
```
New:
```c
            //Party AI fix M1: switch only to a melee config 2 (an empty or non-weapon config 2 is type 0).
            if ( nType2 == 1 )
            {
                //Our second config is NOT ranged, switch to it
                GN_MyPrintString("GENERIC DEBUG *************** Party M1: hit in melee, switch to melee config 2");
```

#### E3 (party) - vanilla line 2049, anchor count 1

Old (the anchor; replaced whole):
```c
                                    if(GetDistanceBetween(OBJECT_SELF, oIntruder) < 20.0 && !GetObjectSeen(oIntruder))
                                    {
                                        ActionMoveToObject(oIntruder, TRUE, 2.0);
                                    }
```
New:
```c
                                    //Party AI fix E3/M6 (party only): GN_PartyApproachUnseen (stand-off, Stationary holds,
                                    //DCR on arrival). NPCs keep the vanilla 2 m run (user decision).
                                    if(IsObjectPartyMember(OBJECT_SELF))
                                    {
                                        GN_PartyApproachUnseen(oIntruder);
                                    }
                                    else if(GetDistanceBetween(OBJECT_SELF, oIntruder) < 20.0 && !GetObjectSeen(oIntruder))
                                    {
                                        ActionMoveToObject(oIntruder, TRUE, 2.0);
                                    }
```

#### L4 (party (PC-only trees)) - vanilla line 2623, anchor count 1

Old (the anchor; replaced whole):
```c
            else if ( GetIsTalentValid(tBuff_05) &&
                      GetCreatureHasTalent(tBuff_06) )
```
New:
```c
            //Party AI fix L4: tier V tested the tier VI talent.
            else if ( GetIsTalentValid(tBuff_05) &&
                      GetCreatureHasTalent(tBuff_05) )
```


### `k_inc_gensupport.nss`

#### M2 (party) - vanilla line 2523, anchor count 1

Old (the anchor; replaced whole):
```c
        nEnemy = GN_CheckEnemyGroupingOnTarget(oCheck,4.0);
        if(nEnemy > 2 && nEnemyCnt < nEnemy)
```
New:
```c
        //Party AI fix M2 (party only): the AoE centre must be a live, seen enemy (no ally, corpse or
        //unseen creature); skipping the inner scan for the rest also saves instructions (L8).
        nEnemy = 0;
        if(!IsObjectPartyMember(OBJECT_SELF) || (GetIsEnemy(oCheck) && !GetIsDead(oCheck) && GetObjectSeen(oCheck)))
        {
            nEnemy = GN_CheckEnemyGroupingOnTarget(oCheck,4.0);
        }
        if(nEnemy > 2 && nEnemyCnt < nEnemy)
```

#### M4a (party) - vanilla line 3369, anchor count 1

Old (the anchor; replaced whole):
```c
    int nChoke, nAfflict, nPlague, nPush, nWind, nLightning, nKill, nHorror, nWound, nStasis, nDroid, nKnock, nHowl, nCnt;
```
New:
```c
    //Party AI fix M4 (party only): the local nDroid shadowed the parameter, so organic-only powers were
    //picked against droids and Droid Disable never was. The flag is now nDisable; NPCs keep the vanilla
    //result (nDroid forced FALSE, as the shadow did).
    int nChoke, nAfflict, nPlague, nPush, nWind, nLightning, nKill, nHorror, nWound, nStasis, nDisable, nKnock, nHowl, nCnt;
    if(!IsObjectPartyMember(OBJECT_SELF))
    {
        nDroid = FALSE;
    }
```

#### M4b (party) - vanilla line 3434, anchor count 1

Old (the anchor; replaced whole):
```c
    if(GetHasSpell(FORCE_POWER_DROID_DISABLE) && nDroid == TRUE)
    {
        nDroid = 1;
```
New:
```c
    if(GetHasSpell(FORCE_POWER_DROID_DISABLE) && nDroid == TRUE)
    {
        nDisable = 1;
```

#### M4c (party) - vanilla line 3574, anchor count 1

Old (the anchor; replaced whole):
```c
    if(nDroid== 1 && nRoll == 1)
```
New:
```c
    if(nDisable == 1 && nRoll == 1)
```

#### M4d (party) - vanilla line 3579, anchor count 1

Old (the anchor; replaced whole):
```c
    if(nDroid == 1 && nRoll > 1)
```
New:
```c
    if(nDisable == 1 && nRoll > 1)
```

#### M7a (party context) - vanilla line 2291, anchor count 1

Old (the anchor; replaced whole):
```c
    else
    {
        return 0;//Return 0 by default.
    }
```
New:
```c
    //Party AI fix M7 (party context only): TSL base items missing above. baseitems.2da 104 Mining_Laser,
    //105 Adv_Mining_Laser (ranged); 106 Lightsaber_No_Sound, 107 DM_ElectroStaff (melee).
    else if(IsObjectPartyMember(OBJECT_SELF) && (nBaseType == 104 || nBaseType == 105))
    {
        return 2;
    }
    else if(IsObjectPartyMember(OBJECT_SELF) && (nBaseType == 106 || nBaseType == 107))
    {
        return 1;
    }
    else
    {
        return 0;//Return 0 by default.
    }
```

#### M7b (party context) - vanilla line 2245, anchor count 1

Old (the anchor; replaced whole):
```c
    GN_MyPrintString("GENERIC DEBUG *************** Return No Weapon Type");
```
New:
```c
    //Party AI fix M7 (party context only): see GN_GetWeaponTypeFromBaseItem.
    if(GetIsObjectValid(oItem) && IsObjectPartyMember(OBJECT_SELF))
    {
        if(nBaseType == 104 || nBaseType == 105)
        {
            return 2;
        }
        if(nBaseType == 106 || nBaseType == 107)
        {
            return 1;
        }
    }
    GN_MyPrintString("GENERIC DEBUG *************** Return No Weapon Type");
```


### `k_ai_master.nss`

#### E6 (hench) - vanilla line 693, anchor count 1

Old (the anchor; replaced whole):
```c
            if ( nWeaponType == 1 || nWeaponType == 0)
```
New:
```c
            //Party AI fix E6: an attacker with no known weapon in the right hand (creature weapons on
            //droids, turrets and beasts; mod base items) counts as melee only when it is close.
            if ( nWeaponType == 1 || (nWeaponType == 0 && GetIsObjectValid(oAttacker) &&
                                      GetDistanceBetween(OBJECT_SELF, oAttacker) <= 5.0) )
```

#### E13 (hench (leader)) - vanilla line 723, anchor count 1

Old (the anchor; replaced whole):
```c
                else
                {
                    //Shout that I was attacked
                    SpeakString("GEN_I_WAS_ATTACKED", TALKVOLUME_SILENT_TALK);
                }
            }
            if(GN_GetSpawnInCondition(SW_FLAG_EVENT_ON_ATTACKED))
            {
                SignalEvent(OBJECT_SELF, EventUserDefined(1005));
```
New:
```c
                //Party AI fix E13: the leader already shouted twice above; only a member held back by
                //player restrict mode shouts here.
                else if(GetPartyMemberByIndex(0) != OBJECT_SELF)
                {
                    //Shout that I was attacked
                    SpeakString("GEN_I_WAS_ATTACKED", TALKVOLUME_SILENT_TALK);
                }
            }
            if(GN_GetSpawnInCondition(SW_FLAG_EVENT_ON_ATTACKED))
            {
                SignalEvent(OBJECT_SELF, EventUserDefined(1005));
```

#### E11 (hench) - vanilla line 764, anchor count 1

Old (the anchor; replaced whole):
```c
                                            ActionMoveToLocation(GetLocation(oTarget), TRUE);
                                            ActionDoCommand(GN_DetermineCombatRound());
```
New:
```c
                                            //Party AI fix E11/M6: stand off by weapon as case 1006 does (was a run onto the
                                            //damager's spot); Ranged, Stationary and Kite do not approach at all.
                                            int nStyle2006 = GetNPCAIStyle(OBJECT_SELF);
                                            if(nStyle2006 != NPC_AISTYLE_PARTY_RANGED && nStyle2006 != NPC_AISTYLE_PARTY_STATIONARY &&
                                               nStyle2006 != NPC_AISTYLE_PARTY_KITE)
                                            {
                                                GN_MyPrintString("GENERIC DEBUG *************** Party E11: unseen damager, stand off");
                                                ActionMoveToObject(oTarget, TRUE, (GN_GetWeaponType(OBJECT_SELF) == 1) ? 4.0 : 15.0);
                                                ActionDoCommand(GN_DetermineCombatRound());
                                            }
```

#### E10 (party (hench)) - vanilla line 805, anchor count 1

Old (the anchor; replaced whole):
```c
        case 2009: //KOTOR_HENCH_EVENT_ON_BLOCKED
        {
            object oDoor = GetBlockingDoor();
            if(!GN_GetSpawnInCondition(SW_FLAG_AI_OFF))
            {
                if(GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 5)
                {
                    if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_OPEN) && GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 7 )
                    {
                        DoDoorAction(oDoor, DOOR_ACTION_OPEN);
                    }
                    else if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_BASH))
```
New:
```c
        case 2009: //KOTOR_HENCH_EVENT_ON_BLOCKED
        {
            object oDoor = GetBlockingDoor();
            if(!GN_GetSpawnInCondition(SW_FLAG_AI_OFF))
            {
                if(GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 5)
                {
                    if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_OPEN) && GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 7 )
                    {
                        DoDoorAction(oDoor, DOOR_ACTION_OPEN);
                    }
                    //Party AI fix E10: party members open doors but never bash them (k_inc_generic post-DCR:
                    //"Party members won't help with bashing anymore").
                    else if(!IsObjectPartyMember(OBJECT_SELF) && GetIsDoorActionPossible(oDoor, DOOR_ACTION_BASH))
```


### The 13 `k_hen_<name>dlg.nss` (the same two replacements in each; anchor count 1 in every file, asserted per file by the builder)

#### E2a (party) - k_hen_miradlg line 55

Old:
```c
    int nMatch = GetListenPatternNumber();
```
New:
```c
    int nMatch = GetListenPatternNumber();
    //Party AI fix E2: a shout goes through k_ai_master case 2004, which is this script's shout path line for
    //line (AI-off and commandable checks, GN_RespondToShout, user-defined 1004), so it runs the live party
    //AI (stance 16, the fixes). Conversations (nMatch == -1) stay here for SelectAndRunDLG.
    if(nMatch != -1)
    {
        ExecuteScript("k_ai_master", OBJECT_SELF, 2004); //KOTOR_HENCH_EVENT_ON_DIALOGUE
        return;
    }
```

#### E2b (party) - k_hen_miradlg line 76

Old:
```c
                GN_RespondToShout(oShouter, nMatch, GetLastHostileActor(oShouter));
```
New:
```c
                //Party AI fix E2: unreachable now (shouts return above); kept empty so this script no longer
                //embeds its own copy of the vanilla combat AI.
```


### The DCR wrappers

#### E2c `a_force_combat.nss` (all) - line 12, anchor count 1

Old:
```c
    GN_DetermineCombatRound();
```
New:
```c
    //Party AI fix E2: run the live combat AI (k_ai_master case 3001 = GN_DetermineCombatRound()).
    ExecuteScript("k_ai_master", OBJECT_SELF, 3001);
```

#### E2d `k_combat_rnd.nss` (all) - line 8, anchor count 1

Old:
```c
    GN_DetermineCombatRound();
```
New:
```c
    //Party AI fix E2: run the live combat AI (k_ai_master case 3001 = GN_DetermineCombatRound()).
    ExecuteScript("k_ai_master", OBJECT_SELF, 3001);
```

#### E2e `a_atkonend.nss` (all) - line 17, anchor count 1

Old:
```c
    DelayCommand(0.3,GN_DetermineCombatRound());
```
New:
```c
    //Party AI fix E2: run the live combat AI (k_ai_master case 3001 = GN_DetermineCombatRound()).
    DelayCommand(0.3, ExecuteScript("k_ai_master", OBJECT_SELF, 3001));
```

### New helper bodies and the kite routine
See Appendix A (kite_routine.nss v2) and Appendix B (party_ai_fixes.nss).

## 7. The build: `build_party_ai_scripts.py`

### 7.1 Files (repo, `patch_manager_mods/ranged_ai_src/script/`)
- `build_party_ai_scripts.py` (new) replaces `build_kite_script.py`.
  - Keep the old script until the new one is installed live. After that, the new `--baseline` mode reproduces the old output, so the old script can go.
- `party_ai_patches.py` (new): the patch list, as in `S/party_ai_patches.py`. It is data (`PATCHES`, `E2_DLG`, `E2_WRAP`, prototype block, Mod 8 dispatch). Each entry has an id that maps to this doc.
- `kite_routine.nss` becomes v2 (Appendix A). `party_ai_fixes.nss` is new (Appendix B), appended after the kite routine.
- Workspace: add `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla_ncs/` with the 17 reference `.ncs` from `Scripts.bif`, plus `MD5SUMS`.
  - They are written by the builder's `--extract` step from `chitin.key`, read-only.
  - Use the KEY/BIF reader from `S/kotorio.py` (40 lines, no pykotor needed), or pykotor's `Installation(...).chitin_resources()` as `mods678_research/extract_vanilla_nss.py` does.

### 7.2 Steps (each one fails the build; nwnnsscomp exits 0 on errors, so only the output file counts)
1. **Inputs.** The vanilla sources in `vanilla/` and `vanilla_all/` must be byte-identical to the BIF `.nss` (the extract step checks this). The reference `.ncs` md5s must match `MD5SUMS`.
2. **Toolchain proofs.** Rebuild all 17 unmodified sources (k_ai_master + 13 dlg + 3 wrappers) and require byte equality with `vanilla_ncs/`. About 0.7 s each.
3. **Baseline proof (`--baseline`, run in the default build too).** Apply only `MOD8-proto` (old prototype lines) + `MOD8-dispatch` with the old `kite_routine.nss` and require md5 `94db8304`, today's live file. This proves the anchor mechanism. It needs the old routine kept as `kite_routine_v1.nss`, or the builder can skip it after the v2 install.
4. **Patch.** Apply the full list with `count == 1` asserted per anchor and per file (the 13 dlg files each). Write `src/` (CRLF).
5. **Compile** the 17 outputs to `out/party_ai/`:
   - `k_ai_master` against `src/k_inc_generic.nss` + `src/k_inc_gensupport.nss` (`compile.sh` layers both, lower-cased);
   - the 16 routers against the vanilla includes.
6. **Arg-order check.** `scripts/check_ncs_arg_order.py <checkroot> --override-only` over a checkroot holding all 17. Required: "0 script(s) compiled with swapped engine-call arguments".
7. **Static sanity:**
   - each router is under 8 KB, contains `CONSTS "k_ai_master"` followed by `ACTION 8`, and no longer calls the combat AI (for example, no `ActionAttack` in the ACTION set);
   - `k_ai_master` contains the new trace strings ("Party E1:", "Kite K2 cap").
   Both are cheap to read with pykotor `read_ncs`, which is read-only and fine. Only pykotor's *compiler* is banned.
8. **Manifest + stage:**
   - Write `out/party_ai/MANIFEST.tsv` with these columns: file, bytes, md5, vanilla md5, patch ids.
   - Write the stage dir `~/Games/KOTOR2_Mod_Workspace/stage_party_ai_<HHMM>/Override/` with the 17 `.ncs`, the MANIFEST and the `src/` used.
   - Print the install and rollback lines.

The prototype (`S/party_ai_patches.py`) already does steps 4-6 and part of 7. Steps 2-3 were run separately here (section 5.1 and section 1).

### 7.3 Install (only on the user's go, game closed; never part of the build)
Extend `mods678_research/apply_mods68_data.py`, or add `install_party_ai_scripts.py [--apply]` (dry run by default):
- **Refuse** if any target in `override/` exists and is not ours. Today only `k_ai_master.ncs` exists, and it is ours (`94db8304`). The 16 routers are new names.
- **Back up** each existing target to `~/Games/KOTOR2_Mod_Workspace/backups/party_ai_<tag>/` with md5s.
- **Copy** the 17 files to `$G/override/` (the real dir is lowercase `override`; `Override` is a symlink to it).
- **Re-md5** everything after the copy (sibling sessions overwrite installs).
- **Record** the Override writes in `~/Games/KOTOR2_Mod_Workspace/replay/README.md` and in doc 13 / the handoff.
- **Order relative to DLLs:** none. These are plain override scripts. Mod 8's DLL hooks (E-H1/E-H3) and `aiscripts.2da` stay as they are.

### 7.4 Rollback
- **Full:** delete the 16 routers (`k_hen_{atton,baodur,discip,g0t0,hanharr,hk47,hndmaid,kreia,mand,mira,remote,t3m4,visas}dlg.ncs`, `a_atkonend.ncs`, `a_force_combat.ncs`, `k_combat_rnd.ncs`) from `override/`. Then restore `k_ai_master.ncs` from the backup (Mod 8 build `94db8304`). The result is exactly today's state.
- **Partial, and safe in any combination:**
  - Restore only `k_ai_master.ncs` to `94db8304`: the fixes are gone, routing stays, and stance 16 now also runs on shout rounds.
  - Delete `k_ai_master.ncs`: fully vanilla behaviour, because the routers then run the BIF `k_ai_master` (2004 == vanilla dlg shout path).
- **Saves:** scripts are not stored in saves. Locals 20/21 live on the creatures. They are harmless without the scripts, and the routine resets them at the start of a fight.

## 8. Test matrix

**Trace.** The AI trace layer turns `ShipBuild()` FALSE for AI scripts and logs `PrintString`/`AurPostString`. All `GN_MyPrintString` lines then print, including the new ones (`GENERIC DEBUG *************** Party …` / `… Kite …`). `GN_MyPrintShoutString` stays silent: its `PrintString` is commented out in vanilla.

Recommendations for the trace layer:
- (1) Log the running script resref. That shows E2 directly: shout-round lines come from `k_ai_master` instead of `k_hen_<name>dlg`.
- (2) Keep `AurPostString` on (H1's vanilla lines).
- (3) Allow a filter on `"Party "` / `"Kite "`.

**Setup notes:**
- A/B testing is easy. The pre-fix state is the current live install. For per-fix A/B, the builder can take `--only ID[,ID]` to build a subset; every patch is independent except E1 (it needs the kite constants, so it is always built with the kite v2).
- **Never launch the game from the session.** The user runs the tests.

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| K1 | Stance 16 companion. Bash a footlocker with them, or fight next to a creature that turns neutral, then start a fight. | No kite/attack toward the placeable or neutral; normal fighting. | `Kite K1 drop target <name>` |
| K2 | One fast melee enemy vs one stance-16 companion (others on Passive, or solo the pair). | At most 2 kite moves, then a full round of shooting, then repeat. | `Kite step 1 streak 1 from <x>`, `… streak 2 …`, then `Kite K2 cap: shoot this round` |
| K3 | A ranged enemy and a melee enemy both rush the kiter; the ranged one is nearest. | It kites from the melee one behind. | `Kite step 1 streak 1 from <melee enemy name>` (not the nearest) |
| M1 | Ranged (11) or Kite (16), blaster in config 1, **empty** config 2; let a melee enemy hit them while cornered. | Blaster stays out, keeps shooting. With a melee weapon in config 2: swaps. | Swap case: `Party M1: hit in melee, switch to melee config 2`; empty-config case: no such line |
| E12 | Kite in fight 1; finish it with a kite in the last round; start fight 2 next to a melee enemy. | Kites on the first DCR of fight 2. | First line of fight 2 for that companion: `Kite step 1 streak 1 …` (no silent cooldown DCR before it) |
| local 8 | (no change) Optional: Ranged companion swapped to melee late in a fight. | Next fight starts on the ranged config (engine zeroes local 8 at combat exit). | Next fight: no `Party M1` line before the first melee hit |
| H1 | Jedi Support (13) companion **without** Revitalize. Let another member drop. | Keeps casting and attacking while the member is down. | No `Someone is knocked out.` (AurPostString) for that Jedi; `Jedi Support AI: AOE Target = …` / `Clear 1450` continue. With Revitalize but 0 FP: `Party H1: no usable Revitalize` |
| H2 | Companion with Heal/Cure on Aggressive or Support. Another *companion* below 50%, leader above 50%. | Heal is cast. | `Return oP1` or `Return oP2` (new in practice), `I have Heal, Improved Heal, or Master Heal`, `Clear 2300`, `Spells.2DA ID = <heal id>` |
| M2 | Melee companion surrounded by 3+ enemies, Jedi Support with Storm/Wave nearby. | AoE centred on an enemy (target ring on an enemy), never on the companion. | `Jedi Support AI: AOE Target = 1` then `Clear 1450` (watch the cast target) |
| M3 | Stun or hold one enemy; Jedi Support with no AoE cluster. | Next power goes to an active enemy. | `Boss AI: Initial oFind Search = <active enemy>` |
| M4 | Jedi Support with Disable Droid + Choke vs 1-2 droids. | Casts Disable Droid (combat log), no melee rush. | `Jedi Support AI: Valid oTarget Set As = <droid>` then `Clear 1450` (talent = Droid Disable in the combat log) |
| M5 | Poison a party member; companion with Cure/Heal. | Cure/Heal cast. | `Starting Poison Check` **without** `Returning Invalid Poison Object`, then `Clear 2300` / `Spells.2DA ID = 28 / 10 / 134` |
| E1 | One companion Aggressive, one Ranged (and one Kite). Kill the last enemy and walk away at once. | Ranged/Kite follow at the end of that round, like Aggressive (no 0-6 s idle). | `Party E1: no enemy seen, stance 11 combat exit` (or 16, 13, 12, 10), `Post DCR Checks for <name>`, `Clear 1200` |
| E3 | A stealthed or out-of-sight sniper hits the PC while a Ranged/Kite companion stands 10-15 m away. | No sprint to 2 m. A move to about 15 m stand-off, then a combat round. Stationary does not move. | `Shout Clear 1900`, then `Party E3: unseen intruder, stand off 15.0` (or `… Stationary holds`) |
| E2 | Kite companion idle next to a melee enemy; the PC starts the fight. | Kite on the **first** (shout-driven) round, not one round later. | `Shout Clear 1800` and `Kite step 1 …` in the same tick, logged from script `k_ai_master` (resref logging) |
| M7 | Peragus: Ranged companion with a melee config 2 vs mining-laser enemies. | Mining-laser hits do not trigger the melee swap. | No `Party M1` line from ranged hits |
| M9 | Defensive (10) melee companion; enemy hits the leader from the far side (> 5 m from the companion, < 5 m from the leader). | Companion engages it. | `Closest becomes Target` with that enemy |
| M10 | Last target flees out to more than 30 m while closer enemies remain. | Switches to the closer enemy, no `CancelCombat` idle. | `Party M10: target beyond 30 m, closest becomes target` |
| E6 | Ranged companion shot by a droid or turret with a creature weapon from more than 5 m. | No melee swap. | No `Party M1` line |
| E8 | Kill the last enemy (Aggressive companion). | No wind-up toward the corpse; follows. | `Party E8: dropped dead last target`, then `Post DCR Checks …` |
| E10 | Lead the party around a locked bashable door so the follow path crosses it. | Companion does not bash it. | (no trace line; visual + no door damage feedback) |
| E11 | Aggressive companion idle; an unseen enemy damages it. | Moves to 4/15 m stand-off, not onto the shooter's spot. Ranged/Stationary/Kite: no move. | `Party E11: unseen damager, stand off` |
| E13 | Leader attacked while 2 companions are idle. | Each companion gets at most 2 shouts per hit (was 3). | Count `Shout Clear 1800` per companion per hit (≤2) |
| L4 | Exile with Inspire Followers V but not VI, not the controlled character, on Support. | Casts tier V. | `Spells.2DA ID = 171` after `Clear 2300` |

**Regression checks (every build):**
- (a) Byte-identical proofs and arg-order check (automatic).
- (b) In game: companion conversations still open the right DLG per module. Atton's 262TEL/003EBO/101PER branches are in `SelectAndRunDLG`, which is untouched.
- (c) `k_oei_userdef` 1004 is still signalled once per shout, if that flag is set.
- (d) Remote still follows its owner.
- (e) No `AR_ERROR.LOG` growth.

## 9. Deferred: user decisions

1. **E5** (switching characters, or the heartbeat, cancels non-combat orders such as open lock, disarm mine or use object). Skipped as instructed. Possibly intended. A fix needs `k_hen_leadchng` (its vanilla rebuild proof already PASSES, section 5.1) plus case 2001.
2. **Enemy versions** of the party-gated fixes. Each makes enemies smarter or stronger, and each would reach NPCs only through `k_ai_master`, not the 52 NPC scripts that embed the include. For each one the gate is a single `IsObjectPartyMember` term to drop:
   - **H1:** an enemy Jedi Support no longer goes idle while a party member is knocked out.
   - **M3:** enemies stop preferring stunned party members.
   - **M2:** boss AoE centres must be enemies; this makes boss AoE *rarer*.
   - **M4:** enemy Jedi use Droid Disable on party droids and stop organic-only powers on them.
   - **E3:** enemies stop sprinting to 2 m of unseen party members.
   - **M7:** NPC mining-laser classification.
3. **H2 extension, "self first + medpacs/repair kits":** the AI would consume shared-inventory medpacs. It needs a `GetCreatureHasTalent(tSpe11_05)` guard (see section 4).
4. **E6(c)**, the Remote's local-8 "heal droids" collision. It is probably moot if the engine zeroes local 8 for the Remote at combat exit; verify with the trace. If not, move the melee mark to index 22.
5. **E9/L7**, companions answering allied NPCs' `GEN_COMBAT_ACTIVE` shouts. This is a feature change.
6. **E7**, restoring the 5 m enter/exit hooks. Measure the engine event rate with the trace first.
7. **E4**, `MOVETOPOINT` stuck-move recovery (leash + heartbeat). It needs trace evidence plus the engine lock data. Candidate local 23.
8. **M8**, the per-round swap counter. This is a redesign: decrement in 2003 only.
9. **M10, sticky-target part** (a dangerous attacker overriding the player-chosen last target). It changes the "party members only attack what you want" design.
10. **L1** combo table, **L2** combo buffs on enemies, **L3** party-droid utilities, **L5** AoE picker flags. These are all-creature or feature changes, with low impact.
11. **L6** (Remote action queue, plausible) and **L8** (instruction budget). L8 is partly mitigated for party callers by M2.
12. **E10** is included by default. Revert it if bashing is wanted.
13. **Tunables:** `KITE_MAX_STREAK` (2), `KITE_THREAT_SCAN` (3), the E6 melee distance (5 m), the E3/E11 stand-off (4/15 m, from vanilla).
14. **Not script work:** A1 (stance 11/12/16 idle loop with an out-of-range target) is the engine fix at `0x6D84FC` (Tier 1/3 in the README). The side finding (combat-zone typo, NPC-only) is not done.

## 10. Artifacts (scratch only)

All under `scratchpad/scriptfix/`:
- `party_ai_patches.py` (patch list + prototype builder), `build_log.txt` (anchor counts, output md5s, arg-order result);
- `kite_routine_v2.nss`, `party_ai_fixes.nss`;
- `build/src/*` (patched sources), `build/ncs/*` (17 outputs), `build/checkroot/`;
- `diffs/*.diff`: k_inc_generic, k_inc_gensupport, k_ai_master, k_hen_miradlg (the same in all 13), the 3 wrappers, kite_routine;
- `proof/vanilla_proof.txt` (19 PASS), `bif/` (BIF `.ncs`/`.nss`), `kotorio.py` + `extract.py` (KEY/BIF/ERF/RIM reader, capsule scan);
- `optA/` (option (a) size samples), `mod8only/` (baseline reproduction of live `94db8304`), `kit/` (private kit copy).

Output md5s of this prototype (they change with any edit):
- `k_ai_master.ncs` `a5bdf830…` (129812 B);
- the 7 template dlg routers `f24d41ab…` (3432 B);
- the wrappers `d1c46a01…` (a_force_combat = k_combat_rnd) and `e7cdfed1…` (a_atkonend).

## Appendix A: `kite_routine.nss` v2 (whole file; replaces ranged_ai_src/script/kite_routine.nss)
```c
//::///////////////////////////////////////////////
//:: Mod 8 "Ranged (Kiting)" party AI (aistate 16)
//:: Doc: patch_manager_mods/13_ranged_ai_spacing.md sec. 3 (local-setup/KOTOR-II)
//:: v2 (party AI script fixes): K1 target check, K2 kite streak cap,
//:: K3 threat scan of the 3 nearest enemies, E12 fresh-fight reset.
//:://////////////////////////////////////////////
/*
    Party members (not the leader) on stance 16 with a ranged weapon:
      1. Kite: one of the 3 nearest seen live enemies within 3.5 m fights in
         melee (or any of them while we are below 60% HP) => step to a random
         walkable spot that is at least 2 m further from it, within 10 m of
         the leader and with a clear shot at our target (and forget the "hit
         in melee" mark). At most KITE_MAX_STREAK kites in a row: then one
         DCR call shoots instead (K2).
      2. Cornered (no such spot) => the vanilla Ranged Support weapon rule:
         swap to the melee config if we were hit in melee recently (the
         vanilla rule now requires a melee weapon in config 2, fix M1).
      3. Blocked shot (HasLineOfSight with both eye heights and both
         objects, the same test the engine uses) => move to a spot with a
         clear shot within 15 m of the target and 10 m of the leader, and
         outside the kite radius of the nearest enemy.
      4. Otherwise the default attack routine.
    The target must be a live enemy (K1); anything else is dropped and
    steps 2 and 4 handle the round the vanilla way.
    A move is always followed by a queued ActionAttack, so the creature
    never idles after it arrives. The engine half (ranged-ai DLL) keeps a
    blocked creature from walking into melee instead of shooting.
    Locals (vanilla uses none of 20/21): 20 = SW_NUMBER_KITE_COOLDOWN keeps
    a move from being re-planned at once (counts GN_DetermineCombatRound
    calls, not rounds); 21 = SW_NUMBER_KITE_STREAK counts kites in a row.
    Both are cleared when a fight starts (not in combat at entry, E12) and
    by the stance combat exit (GN_PartyStanceCombatExit, E1). Local 8
    ("hit in melee", set by k_ai_master 2005) is zeroed by the engine at
    combat exit (SetCombatState 0x57A5C1) and by every kite here.
    The engine rule "Ranged companions do not walk to their target" also
    applies to stance 16 (DLL hook E-H3).
*/

int SW_NUMBER_KITE_COOLDOWN = 20;
int SW_NUMBER_KITE_STREAK = 21;
float KITE_THREAT_RANGE = 3.5;
float KITE_LEASH = 10.0;
float KITE_SHOT_RANGE = 15.0;
int KITE_TRIES = 12;
int KITE_MAX_STREAK = 2;
int KITE_THREAT_SCAN = 3;

vector vKiteSpot;   // result of GN_KiteFindSpot

vector GN_KiteEye(vector v)
{
    return Vector(v.x, v.y, v.z + 1.5);
}

// The engine's attack LOS test: foot position + 1.5 m on both ends, both objects excluded.
int GN_KiteHasShot(vector vFrom, object oTarget)
{
    return HasLineOfSight(GN_KiteEye(vFrom), GN_KiteEye(GetPosition(oTarget)), OBJECT_SELF, oTarget);
}

float GN_KiteDist(vector a, vector b)
{
    return VectorMagnitude(a - b);
}

// K3: the nearest of the KITE_THREAT_SCAN nearest seen enemies that is alive, within KITE_THREAT_RANGE
// and fights in melee (any weapon when bHurt). GetNearestCreature is sorted by distance, so the scan
// stops at the first one out of range.
object GN_KiteFindThreat(int bHurt)
{
    int i;
    for (i = 1; i <= KITE_THREAT_SCAN; i++)
    {
        object oCheck = GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, i,
                                           CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN);
        if (!GetIsObjectValid(oCheck) || GetDistanceBetween(OBJECT_SELF, oCheck) > KITE_THREAT_RANGE)
        {
            return OBJECT_INVALID;
        }
        if (!GetIsDead(oCheck))
        {
            object oCheckWeapon = GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON, oCheck);
            if (bHurt || !GetIsObjectValid(oCheckWeapon) ||
                GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oCheckWeapon)) != 2)
            {
                return oCheck;
            }
        }
    }
    return OBJECT_INVALID;
}

// nMode 1 = away from oThreat (keep the spot furthest from it), 2 = regain a shot at oTarget (keep the nearest spot).
// Returns TRUE and sets vKiteSpot when a spot is found.
int GN_KiteFindSpot(int nMode, object oThreat, object oTarget, object oLeader)
{
    vector vMe = GetPosition(OBJECT_SELF);
    vector vLeader = GetPosition(oLeader);
    vector vThreat = vMe;
    float fThreatNow = 0.0;
    if (GetIsObjectValid(oThreat))
    {
        vThreat = GetPosition(oThreat);
        fThreatNow = GN_KiteDist(vMe, vThreat);
    }
    int nFound = FALSE;
    float fBest = 0.0;
    int i;
    for (i = 0; i < KITE_TRIES; i++)
    {
        vector v = GetRandomDestination(OBJECT_SELF, (nMode == 1) ? 8 : 10);
        int nOk = TRUE;
        if (GetIsObjectValid(oLeader) && oLeader != OBJECT_SELF && GN_KiteDist(v, vLeader) > KITE_LEASH)
        {
            nOk = FALSE;
        }
        float fThreat = 99.0;
        if (nOk && GetIsObjectValid(oThreat))
        {
            fThreat = GN_KiteDist(v, vThreat);
            // at least 2 m further and outside the kite trigger radius
            if (nMode == 1 && (fThreat < fThreatNow + 2.0 || fThreat < KITE_THREAT_RANGE + 0.5))
            {
                nOk = FALSE;
            }
            if (nMode == 2 && fThreat < KITE_THREAT_RANGE + 0.5)   // never land inside the kite trigger radius
            {
                nOk = FALSE;
            }
        }
        // stay in shot range: stance 16 does not walk back to its target (vanilla Ranged engine rule, DLL E-H3)
        if (nOk && GN_KiteDist(v, GetPosition(oTarget)) > KITE_SHOT_RANGE)
        {
            nOk = FALSE;
        }
        if (nOk && !GN_KiteHasShot(v, oTarget))
        {
            nOk = FALSE;
        }
        if (nOk)
        {
            float fScore = (nMode == 1) ? fThreat : -GN_KiteDist(v, vMe);
            if (!nFound || fScore > fBest)
            {
                nFound = TRUE;
                fBest = fScore;
                vKiteSpot = v;
            }
        }
    }
    return nFound;
}

void GN_KiteMoveThenAttack(object oTarget, int nCooldown)
{
    ClearAllActions();
    ActionMoveToLocation(Location(vKiteSpot, GetFacing(OBJECT_SELF)), TRUE);
    ActionAttack(oTarget);
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, nCooldown);
    // We got away: forget the "hit in melee" mark (set by k_ai_master on attack, only counted down by
    // GN_CheckRangedPreferred), or the next round would swap to the melee config and walk back in.
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, 0);
}

int GN_RunKiteAIRoutine(object oIntruder)
{
    if (!IsObjectPartyMember(OBJECT_SELF))
    {
        return GN_RunDefaultAIRoutine(oIntruder);
    }
    object oLeader = GetPartyLeader();
    if (OBJECT_SELF == oLeader || GetCurrentHitPoints(OBJECT_SELF) < 1)
    {
        return 0;
    }

    // E12: a new fight starts without the last fight's cooldown or streak.
    if (!GetIsInCombat(OBJECT_SELF))
    {
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, 0);
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, 0);
    }

    int nCool = GetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN);
    if (nCool > 0)
    {
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, nCool - 1);
        // Still walking to the last spot: let it finish (the attack is queued behind it).
        if (GetCurrentAction(OBJECT_SELF) == ACTION_MOVETOPOINT)
        {
            return 1;
        }
    }

    object oWeapon = GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON);
    int nRanged = GetIsObjectValid(oWeapon) && GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oWeapon)) == 2;
    object oTarget = GN_DetermineAttackTarget(oIntruder);
    // K1: only a live enemy is a target here. A neutral, a bashed door or placeable, or a corpse
    // (GetLastHostileTarget has no such checks) is dropped; steps 2 and 4 then run the vanilla way.
    if (GetIsObjectValid(oTarget) && (!GetIsEnemy(oTarget) || GetIsDead(oTarget)))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Kite K1 drop target " + GN_ReturnDebugName(oTarget));
        oTarget = OBJECT_INVALID;
    }

    // 1. Kite away from a close melee threat.
    if (nRanged && nCool == 0)
    {
        int nHurt = GetCurrentHitPoints(OBJECT_SELF) * 10 < GetMaxHitPoints(OBJECT_SELF) * 6;
        object oThreat = GN_KiteFindThreat(nHurt);
        int nStreak = GetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK);
        if (GetIsObjectValid(oThreat) && nStreak < KITE_MAX_STREAK)
        {
            object oShootAt = GetIsObjectValid(oTarget) ? oTarget : oThreat;
            if (GN_KiteFindSpot(1, oThreat, oShootAt, oLeader))
            {
                GN_MyPrintString("GENERIC DEBUG *************** Kite step 1 streak " + GN_ITS(nStreak + 1) + " from " + GN_ReturnDebugName(oThreat));
                SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, nStreak + 1);
                GN_KiteMoveThenAttack(oShootAt, 2);
                return 1;
            }
        }
        // K2: no kite on this call (no threat, streak capped or no spot): the streak ends here, and
        // the attack queued below keeps a whole round before the next kite can start.
        if (GetIsObjectValid(oThreat) && nStreak >= KITE_MAX_STREAK)
        {
            GN_MyPrintString("GENERIC DEBUG *************** Kite K2 cap: shoot this round");
        }
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, 0);
    }

    // 2. Weapon config (cornered and hit in melee => melee config; safe again => ranged config), as Ranged Support.
    if (GN_CheckRangedPreferred())
    {
        return 0;
    }

    // 3. Blocked shot => find a spot with a clear shot.
    if (nRanged && nCool == 0 && GetIsObjectValid(oTarget) && !GetIsDead(oTarget) &&
        !GN_KiteHasShot(GetPosition(OBJECT_SELF), oTarget))
    {
        object oNear = GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, 1,
                                          CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN);
        if (GN_KiteFindSpot(2, oNear, oTarget, oLeader))
        {
            GN_MyPrintString("GENERIC DEBUG *************** Kite step 3 regain shot at " + GN_ReturnDebugName(oTarget));
            GN_KiteMoveThenAttack(oTarget, 1);
            return 1;
        }
    }

    // 4. Normal attack behaviour (re-issues ActionAttack every round).
    GN_RunDefaultAIRoutine(oIntruder);
    return 0;
}
```

## Appendix B: `party_ai_fixes.nss` (new; appended after the kite routine)
```c
//::///////////////////////////////////////////////
//:: Party AI script fixes: helper bodies (appended after the kite routine)
//:: Design: scratchpad design_ai_script_fixes.md; review ai_review_2026-10-01
//:://////////////////////////////////////////////

// E1 (party only): stances 10-13 and 16 return straight after their routine, so they never reach
// GN_DoPostDCRChecks, the only party combat exit (CancelCombat + follow the leader); Aggressive does.
// When this member sees no enemy: cure poison and heal first, as the Default AI does, else run the exit.
// Returns TRUE when the caller (GN_DetermineCombatRound) must return.
int GN_PartyStanceCombatExit(int nNPC_AI)
{
    if (!IsObjectPartyMember(OBJECT_SELF) || GetPartyMemberByIndex(0) == OBJECT_SELF)
    {
        return FALSE;
    }
    if (nNPC_AI != NPC_AISTYLE_PARTY_DEFENSE && nNPC_AI != NPC_AISTYLE_PARTY_RANGED &&
        nNPC_AI != NPC_AISTYLE_PARTY_STATIONARY && nNPC_AI != NPC_AISTYLE_PARTY_SUPPORT &&
        nNPC_AI != NPC_AISTYLE_PARTY_KITE)
    {
        return FALSE;
    }
    if (GetIsObjectValid(GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, 1,
                                            CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN)))
    {
        return FALSE;
    }
    // E12: the kite cooldown and streak end with the fight.
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, 0);
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, 0);

    object oPoisoned = GN_CheckIfPoisoned();
    if (GetIsObjectValid(oPoisoned) && GN_TalentMasterRoutine(GEN_TALENT_REMOVE_POISON, oPoisoned))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E1: no enemy seen, cure poison first");
        return TRUE;
    }
    object oInjured = GN_CheckIfInjured();
    if (GetIsObjectValid(oInjured) && GN_TalentMasterRoutine(GEN_TALENT_HEALING, oInjured))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E1: no enemy seen, heal first");
        return TRUE;
    }
    GN_MyPrintString("GENERIC DEBUG *************** Party E1: no enemy seen, stance " + GN_ITS(nNPC_AI) + " combat exit");
    return GN_DoPostDCRChecks();
}

// E3/M6 (party only): GN_RespondToShout, shout 1, intruder valid but not seen. The vanilla code ran every
// such member to 2 m of the intruder (the "< 20 m and not seen" test is always true there) and queued
// nothing after the move. Use the weapon stand-off (4 m melee, 15 m otherwise), never move on Stationary,
// and plan a combat round on arrival.
void GN_PartyApproachUnseen(object oIntruder)
{
    if (GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_PARTY_STATIONARY)
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E3: unseen intruder, Stationary holds");
        return;
    }
    float fRange = 15.0;
    if (GN_GetWeaponType(OBJECT_SELF) == 1)
    {
        fRange = 4.0;
    }
    GN_MyPrintString("GENERIC DEBUG *************** Party E3: unseen intruder, stand off " + FloatToString(fRange, 4, 1));
    ActionMoveToObject(oIntruder, TRUE, fRange);
    ActionDoCommand(GN_DetermineCombatRound(oIntruder));
}
```
