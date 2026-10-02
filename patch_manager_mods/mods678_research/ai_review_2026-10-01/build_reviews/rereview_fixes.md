# Pre-install re-review: party AI script fixes (3dc974e..2ddec63) + companion-ai DLL fixes (2e596e9..92db376)

Read-only. Nothing was edited, built, installed, launched or committed. HEAD is `92db376`.

**Artifacts checked**

| item | state |
|---|---|
| `out_party_ai/k_ai_master.ncs` | md5 `b003ea41`. Its manifest (built 19:50:21) records input md5s that equal the HEAD repo sources: `party_ai_patches.py aedd788d`, `kite_routine.nss 840fc346`, `party_ai_fixes.nss 59790b9b`. |
| `arg_order` | "0 script(s) compiled with swapped …" |
| Live `override/k_ai_master.ncs` | Still `94db8304` (Mod 8 0.2.0) |
| `backups_party_ai_*` | None exist yet |
| CompanionAI kpatch | Built 19:52:51. Its source copies (`CompanionAI.cpp`, `CompanionAiLogic.h`, `exports.def`, hooks.toml) are byte-identical to HEAD. |
| kpatch DLL | md5 `1961ded0`, imports only KERNEL32 and msvcrt |
| Live install | `companion-ai` is not in the live `patch_config.toml`; `ranged-ai` is. |

## Verdict

**Install: GO for both. No BLOCKERs.**
- B1 and every SHOULD-FIX from both earlier reviews is fixed.
- The SF3 refactor is behaviour-identical, checked line by line.
- SF2 now gives the same verdicts as the hand walk on every real consumed window.
- The new findings below are all NITs. None changes game behaviour for the shipped INI.

**A1 decision point.** Keeping A1 type-agnostic is safe. On a door or placeable it can only fire where vanilla already loops forever with no progress. Recommendation in section D.

## Findings (new)

| id | sev | where | summary |
|---|---|---|---|
| R1 | NIT | `party_ai_fixes.nss` S1 block | `GetDistanceToObject(oAttacker) <= 30.0` is also true for the -1.0 that it returns for another area. Harmless: a KOTOR module has one area, and a skip falls back to vanilla. Fix: `float d = GetDistanceToObject(oAttacker); … d >= 0.0 && d <= 30.0`. |
| R2 | NIT | `k_inc_generic.nss:2083` (vanilla `Shout Clear 1900`) + `GN_PartyApproachUnseen` | Stances 11/12/16 now hold. The vanilla `ClearAllActions()` still runs before the call, so a holding companion that was walking with the leader drops its follow on every leader shout. It stands until the next heartbeat re-issues `ActionFollowLeader`. Cosmetic. Fix: for a party member that will hold, skip the clear (move the hold test before `ClearAllActions`, party only). The call-site comment ("Stationary holds") is stale. |
| R3 | NIT | design `design_ai_script_fixes.md:869` (test matrix row M5) | It still expects "Cure/Heal cast, Spells.2DA ID = 28/10/134". It should now read: a Cure-only companion does not cast (`Returning Invalid Poison Object` or no `Clear 2300`); a Heal companion casts once and the poison is gone. Add rows for a poisoned member over 15 m away and for a poisoned droid (no cast). |
| R4 | NIT | `apply_party_ai_scripts.py` `print_tail` | **Same-build re-run:** BEFORE.txt marks the 16 routers as "ours", so the rollback says "left alone". Meanwhile `k_ai_master` is restored from the FIRST install's Mod 8 backup. Following the printed rollback gives a mixed state (Mod 8 DCR + routers). That state is documented as safe, but it is not a full rollback. Fix: when `mod8_backup()` is used, also print `rm` for the files its BEFORE.txt lists as absent. |
| R5 | NIT | installer, upgrade path | A later rebuild (new `k_ai_master` md5) over this install is REFUSED as foreign: only `MOD8_MD5` or the current build count as known. This is safe, but the next fix round must roll back to `94db8304` first, or add a list of accepted earlier party builds. Document it in the replay README line. |
| R6 | NIT | installer, failure path | The restore runs inside `except Exception`, so Ctrl-C (KeyboardInterrupt) mid-copy skips it. An exception inside the restore aborts it silently. `mod8_backup()` trusts BEFORE.txt and does not re-md5 the backed-up file. Fix: `except BaseException`, a per-file try in the restore, and an md5 check of `<dir>/k_ai_master.ncs`. |
| R7 | NIT | `CompanionAiLogic.h` `getBool` | An empty value (`a1=`, or `a1= ; off` where the comment strips the value) keeps the built-in default, which is ON for a1/fm1/fm4/c1/a4. The host test asserts this on purpose. State it in the INI header ("empty = shipped default; use 0/off to disable"), or treat empty as OFF for fix flags. |
| R8 | NIT | `parseConfig` `shipbuild_scope` | The SF1 example `shipbuild_scope=none` (also `false`/`no`) still maps to `ai` and is not reported as bad. Effect: print-only (the ShipBuild scan showed it only guards prints). Fix: accept `none/false/no` as off and report unknown words via `noteBad`. |
| R9 | NIT | `getInt` | Word values are accepted for numeric keys too: `fm1_rethink_ms=on` gives 1 ms (fallback forced to 1 ms after each bump clear), and `a1_rethink_ms=yes` gives 1. Fix: apply `boolWord` only in `getBool` and for `fm4`. |
| R10 | NIT | `parseConfig` fm4 | "fm4 was bad" is detected by `strstr(t.bad, "fm4")` on a shared 256-byte list. If the list overflows (many bad keys first), fm4 stays ON. Very unlikely. Fix: a dedicated `bool fm4Bad` (or `getInt` returning a status). |
| R11 | NIT | `CompanionAiBumpClear` | FM1 "keep" lines (a fix decision) now go through `tline`, so the per-second cap can drop them; review N1 suggested keeping `eline` for decisions. The S counters still count every event, so this only affects forensics. Optional: `eline` for `keep`, `tline` for `clear`. |
| R12 | NIT | `check_windows.py` (fork probe, 65 synthetic cases + real run) | Gate gaps, no false pass on the shipped windows. (a) A callee walk that ends "unknown", or a callee missing from the dump, counts as "does not read EAX". (b) `pending_at` stops at the first balanced CALL, so outer pushes are invisible (`PUSH ESI; [X] PUSH 1; CALL g(RET 4)` reports 0, true depth +4). (c) "reset" is accepted at any depth, including an epilogue `POP EDI; POP ESI; MOV ESP,EBP` (10 such epilogues in segments 6d/57/58), and a bare `RET` with 0 pushed. (d) `depth_verdict` drops paths that ended in an unknown note instead of failing them. (e) The stolen-bytes EAX test is still regex-only (implicit `MUL`/`CDQ`/`LODSD`/`CALL [reg+x]`). (f) N10 parameter-source cross-check not done. Fix each before the gate is trusted without a hand walk. Details in section C. |
| R13 | NIT | `AiTracePerceptionMode1` (N4 fix) | E3 at `0x56BDBD` is only reached after P1i's JC with `[ebp-0x28] >= 0xFA0` (`0x56BDB4 CMP; 0x56BDBB JC 0x56BDE9`), so the saved elapsed is always the same creature's. One stale value is possible after a hot p1 1→0 flip, because the reset path does not refresh `g_p1iCr`. Log-only. |
| R14 | NIT | script review N2 (K3) | Still open: dead enemies inside 3.5 m use up the 3 scan slots. Rare, accepted. |
| R15 | NIT | kpatch provenance | As in review N13: the kpatch DLL postdates the sources by 2 s. Its exports, strings and imports match HEAD, but no byte-level rebuild comparison was done. |

## Verified-fixed table

| item | status | evidence |
|---|---|---|
| **B1** Cure poison loop | FIXED | Generated `k_inc_generic.nss:2473-2476`: `if(IsObjectPartyMember(OBJECT_SELF)) tSpe11_02 = TalentSpell(-1);` (M5b). `GN_CheckIfPoisoned` (`:2303-2327`) scans `GetPartyMemberByIndex(0..2)` and keeps only `!GetIsDead`, `!= RACIAL_TYPE_DROID`, `GetDistanceBetween(OBJECT_SELF, oPM5) < 15.0`. The NPC branch is unchanged. Details in A. |
| B1, Heal/Master Heal cover poison | CONFIRMED | `vanilla_all/k_inc_force.nss` impact code. Heal `:2328-2334` and Master Heal `:2393-2400` loop over the same `GetPartyMemberByIndex(nCnt)` set and use the same predicate (valid, not droid, `< 15.0` from the caster), then `Sp_RemoveSpecificEffect(EFFECT_TYPE_POISON)`. `:665ff` removes every effect of that type. Cure `:931-937` has the removal commented out. The cast is `ActionUseTalentOnObject(tUse, OBJECT_SELF)` (`:2829`), so the caster never moves and the 15 m is measured from where it stands. |
| B1, no recast loop left | CONFIRMED | Cure-only party caster: all three `GetCreatureHasTalent` are false (the Cure talent is `TalentSpell(-1)`, the same mechanism as H1b), and ANTIDOTE closes the item for the party. So `bValid=FALSE`, `tUse` stays invalid, the `Clear 2500` fallback is closed, and the routine returns FALSE. Heal caster: every member reported is one the cast reaches, so the poison is removed. The only residue is a member walking out of 15 m during the cast animation: one wasted cast, then the next DCR excludes them. HEALING path: see S2. |
| **S1** E1 skip conditions | FIXED | `GN_PartyStanceCombatExit(nNPC_AI, oIntruder)` (`:4407-4460`, call `:301`). It returns FALSE (vanilla stance routine) for: a conversation (global or self), not commandable, dead or KO; a seen enemy; a live `GetIsEnemy` intruder, leader attack target, or last hostile actor within 30 m. The E12 reset now runs only after the skip test. Details in B. |
| S1, "never exits combat"? | NO REGRESSION POSSIBLE | A skip returns FALSE, and the caller then runs the unchanged vanilla stance routine. These five stances had no script combat exit in vanilla, so the skip can only remove E1's benefit and can never add a stuck state. Recovery is the vanilla hench heartbeat (`k_ai_master.nss:523-555`, case 2001): with `!GetIsConversationActive()`, `GetCommandable()`, no ACTION_MOVETOPOINT or FOLLOWLEADER running, `!GN_GetIsFighting(OBJECT_SELF)` and no SEEN enemy, it runs `ClearAllActions(); ActionFollowLeader();`. `GN_GetIsFighting` (`k_inc_gensupport.nss:355-375`) is TRUE only while there is an attempted attack or spell target, an attack target, or an enemy movement target. So the "unreachable/unseeing enemy within 30 m" or "invulnerable but still hostile" case reproduces vanilla exactly: the companion idles, then follows on the next heartbeat. A neutral-turned object fails `GetIsEnemy`, so E1 runs normally. A stuck engine attack action (A1 territory) keeps `GN_GetIsFighting` TRUE in vanilla too. |
| S1, conversation/commandable | OK | `GetIsConversationActive()` is global (any dialog), `GetIsInConversation(OBJECT_SELF)` is per creature, and `GetCommandable()` is FALSE in cutscenes that lock the party. All three only cause a skip, so the vanilla heartbeat guard (which uses the same two tests) is preserved. |
| S1, E3 ping-pong | BROKEN (as intended) | A unit approaching an unseen intruder gets `DCR(oIntruder)` on arrival. The intruder is a valid live enemy, so E1 skips, and 11/12/16 no longer approach (S4). |
| **S2** heal range | FIXED | `GN_CheckIfInjured` `:2377-2391`: all three ratio conditions require not-droid and `GetDistanceBetween(OBJECT_SELF, oPn) < 15.0`, the leader included (new anchor H2e, unique in the pristine-text lint). Out of range keeps the ratio at 10.0, so the member is never returned. A remaining recast is a real heal (the member stays under 50% after one cast), bounded by FP, as in vanilla. |
| **S3** E6/E11 party gates | FIXED | `k_ai_master.nss:696-697`: for a non-party unit `nWeaponType==1 \|\| (nWeaponType==0 && TRUE)`, which equals vanilla `==1 \|\| ==0`. `:773-777`: non-party units keep vanilla `ActionMoveToLocation(…, TRUE); ActionDoCommand(DCR)` byte for byte. |
| **S4** E3/E11 hold for 11/12/16 | FIXED | `GN_PartyApproachUnseen` `:4465-4473` holds for 11, 12 and 16. E11 `:780-787` excludes the same three. One rule for both (see R2 for the leftover clear). |
| **S5** installer | FIXED, with NITs R4-R6 | The backup dir must not exist (checked before any write, in a dry run too). BEFORE.txt records the pre-install md5 or `-`. The rollback is built from BEFORE.txt: `rm` only for files that were absent; `cp` only when `k_ai_master` existed; on an "already ours" re-run it points at the backup dir whose BEFORE.txt shows `94db8304`, else it says to rebuild. A failed copy removes new files and copies backups back. Also: case-duplicate detection, `arg_order` enforced, repo sources compared with the manifest. **Second install over the first** (same build): every file "already ours"; the new backup holds our files; the `k_ai_master` restore points at the first dir. Correct except R4. **Failed copy:** correct except R6. |
| Script NITs N1, N3, N5-N10 | FIXED | N1 comments; N3 E12 print; N5 `lint_patches` on pristine text (unique, non-overlapping, not inside another patch's replacement); N6 `Party M4:` gensupport marker plus 13 trace strings asserted; N7 stray `tSpe11_05` binder check; N8 `patched_with` + `compiler_md5`; N9 installer; N10 design correction (`design_ai_script_fixes.md:120`). N2 open (R14), N4 test item. |
| Instruction budget | OK | New per-DCR cost: E1 adds about 8 engine calls before any work. M5 adds 3 x (valid/dead/race/distance) and skips the effect walk for out-of-range members, so it is cheaper than before in the worst case. No new loop is unbounded. Far below 0x20000. |
| **SF1** INI words | FIXED, with NITs R7-R10 | `boolWord` accepts on/true/yes and off/false/no, case-insensitive, exact match. In `getBool` an unparseable value gives `false` and `noteBad`; in `getInt` it gives the default and `noteBad`; fm4 is forced to 0 when bad. The DLL logs `# config: unparseable value for: …` after each load. The INI parser strips `;`/`#` comments that follow whitespace, so `a1=off ; x` gives `off`. Host test covers words, typos (`of`, `maybe`, `fast`) and the shipped INI (no bad keys). |
| **SF3** refactor identity | IDENTICAL | Details in E. |
| **SF2** check_windows | FIXED for the EAX walk; NITs R12 | Real run at 92db376: ALL OK. T2 (`0x68F308`/`0x68F39F`), A1 (`0x6D8548`/`0x6D7D2F`) and A4 (`0x6D9E39`/`0x6D9EA4`) are EAX-dead with pending stack 0/0, matching the hand walk. The old checker on the same inputs gives identical window verdicts (it fails only on FM3's absence from the toml, as expected). 65 synthetic probes were all correct or conservative: implicit readers (CDQ/CWDE/one-op IMUL/DIV/STOS/SCAS/CMPXCHG/XCHG/SAHF/PUSHAD/CMOVcc); `CALL [EAX+x]` and `JMP EAX` as reads; indirect JMP through another register as unknown, which fails the check; partial writes (`MOV AL`, `SETcc`, `FNSTSW AX`) not treated as kills; `XOR/SUB EAX,EAX` as kills; `__alloca_probe` detected. |
| **FM3** removed | CONFIRMED | Absent from the repo hooks.toml (comment only), `exports.def`, `Config`, `parseConfig`, `sameConfig`, `logConfig` and Stats. The kpatch hooks.toml has no `0x0051DBE7`. The kpatch DLL `1961ded0` exports 16 functions with no `CompanionAiFm3Probe` and contains no "fm3" string; the old `733ab1b1` had 17 exports. The host test asserts 18 toml hooks, 16 exports and no "Fm3". `check_windows.py` keeps the window as `shipped=False`. |
| **Throttles** | OK | E2 is first 20 per creature, then one per 10 s, via `tline`; the counter `++g_st.e2` runs before the throttle. FM1 is first 20 per bumpee, then on keep/clear change or every 2 s (R11). A4 is first 5 per creature, then one per 10 s, still `eline`. `SubjectThrottle` is a 32-slot LRU that prefers empty slots, and the host test covers its budget, period, signature and eviction. The `g_tr` track table grew to 32 and evicts empty slots first, then non-party least-recently-used tracks (N6); `party` is set by E1/E1b. Thread ids are logged at load, the first T1 flip and on every FM4 line (N8). |

## A. B1 detail
- Party flow for poison:
  - DCR, `GN_CheckIfPoisoned` (only members a Heal cast reaches), then `GN_TalentMasterRoutine(REMOVE_POISON)`.
  - Heal (`tSpe11_01`) or Master Heal (`tSpe11_04`) only when `GetCreatureHasTalent`, which includes the FP check.
  - `Clear 2300`, then `ActionUseTalentOnObject(tUse, OBJECT_SELF)`.
  - The impact loop removes poison from the very member the search reported.
- Paths that reach the poison check: `:637-640` (Default), `:949-952`, `:1024/1065` (Jedi Support), E1 `:4444`. All use the same two functions, so there is no unpatched variant.
- No other caller assigns a poison cure.

## B. S1 detail: why the skip cannot strand a companion
- `GN_PartyStanceCombatExit` is the only code added on this path, and its FALSE result reproduces vanilla exactly. For 10/11/12/13/16, vanilla `GN_DetermineCombatRound` returns straight after the stance routine and never reaches `GN_DoPostDCRChecks`. That gap is the original E1 bug.
- So with the skip, the worst case is precisely vanilla. Vanilla's way out is the 2001 heartbeat follow described in the table, plus the engine combat-state reset (`0x57A5C1`).
- In the residual cases (a reachable-but-unseen live enemy within 30 m, a plot/invulnerable hostile, a leader target that is a hostile-faction door) the companion does what vanilla does. It holds or fights through the stance routine, and when it has no action target and no enemy is seen, it follows on the heartbeat.
- The `Party E1: skipped` trace line makes these cases visible for the A/B.

## C. SF2 residual gate gaps (R12; from the probe fork, scratch `probe_cw.py`/`probe_cw2.py`)
1. `callee_reads_eax` is True only on a "LIVE…" verdict. An "unknown" callee walk, or a callee missing from the dump, counts as a non-reader. Repro: callee `MOVSS XMM0,[ECX]; MOV [EDX],EAX; RET` makes the caller "dead". Fix: unknown or missing callee means LIVE, except IAT `CALL [imm]`.
2. `pending_at` stops at the first CALL with `pushed <= n`. Repros: `PUSH 1; [X] PUSH 2; CALL g(RET 4); PUSH EAX; CALL f(RET 8)` reports 0 (true 4); `PUSH ESI; [X] PUSH 1; CALL g(RET 4); POP ESI` reports 0 (true +4). Fix: walk to a reset, a RET or a horizon, and compare absolute depth from the function entry.
3. "reset" is accepted at any depth even when POPs come before `MOV ESP,EBP`, and a bare `RET` with 0 net is accepted too. Fix: any POP or `ADD ESP` before the reset invalidates it; a bare RET must equal the jump depth.
4. `depth_verdict` ignores paths with notes (unknown-size indirect CALL, indirect JMP, step limit). Fix: any note means None, which fails the check.
5. The stolen-bytes EAX test is regex-only. Fix: run `classify()` on each stolen instruction and fail on anything but "none" or on any CALL.
6. N10 parameter-source cross-check not done.

## D. A1 (N12) decision: should A1 be limited to creature targets?

**What the code does.** `CompanionAiA1Gate` has no target-type test. It gates on effective stance 11/12, backoff==0, forceJump==0, not the controlled creature, the raw stance mask, and party membership.

**Vanilla `0x6D7390` (from the dump)**

Type constants: creature 5, placeable 9, door 0x0A.

- **Bash is the attack action.** Doors and placeables use the same attack action 0xC (dispatcher `0x5D56B0`). `DoDoorAction(BASH)` and `ActionAttack(door)` use it. The bash point comes from `0x585F10` (`0x6D7A46..0x6D7A68`).
- **Where A1 sits.** The approach branch is type-agnostic, and A1 (`0x6D853E`) comes before the door/placeable split at `0x6D8571`.
- **Doors far from the bash point.** These set backoff `[ebp-0x168]=1`. The backoff move `0x5644B0` ignores stance, so vanilla Ranged/Stationary DO walk to a far door, and A1 never fires there (`a1SpinCandidate` needs backoff==0). A1 can catch a door only when the companion is near the bash point but the approach branch still runs (LOS failure).
- **Placeables out of range, non-controlled 11/12.**
  - Vanilla re-queues the attack, and the move `0x11` is skipped by the stance test at `0x6D876A`. Only a facing action `0x13` follows. The result is an endless standing spin with zero progress: the placeable is never hit.
  - A1 turns this into a retarget to a visible enemy creature in weapon range (the tail checks type 5 at `0x6D7D64`), or an end.

**Who can cause it.**
- **Player orders.** Only the controlled character takes GUI orders, and `ctrl == id` returns vanilla, so a player order never reaches A1. The single route is ordering a bash and then switching control before it completes.
- **AI bashing is off for the party.** BioWare removed party help-bashing (`k_inc_generic` post-DCR, door and placeable branches both just follow), and our E10 blocks party `DoDoorAction(BASH)` on blocked.
- **Scripts.** None of the 638 vanilla sources does `ActionAttack` on a door or placeable for a party member. Module-only `.ncs` without source were not scanned (the only unknown).

**Recommendation: keep A1 type-agnostic for this install; limiting it is optional.**
- **Why it is safe.** In every non-creature case A1 can reach, vanilla is already a permanent no-progress spin. Ending it or retargeting cannot break a script that vanilla would not already leave stuck, since the placeable is never damaged either way. The only observable difference is cosmetic: the companion stops facing the object, or shoots an enemy instead.
- **Add to the A1 log line now (log-only):** the target type (`byte [[ebp-0x50]+8]`), so non-creature would-spins show up in the trace.
- **If you prefer the strict "combat only" scope:** gate with `obj=[ebp-0x50]` (the target's AsSWSObject, non-null on this path), `saferead` the byte at `+8`, require `== 5`. That is the same test vanilla uses at `0x6D7A50`/`0x6D7D6E`. Put it in `a1Applies(c, raw, party, tgtType)` with a host test and an optional `a1_targets=creature|all` key. Either choice is low risk; this is a scope call, not a correctness one.

## E. SF3 line-by-line identity (old 2e596e9 → new 92db376)

**A1**
- **Spin test.** `eff!=11&&eff!=12 → 0; backoff||jump → 0` becomes `!a1SpinCandidate(eff, backoff, jump) → 0`. Same result. The new code reads two frame slots unconditionally, which is harmless.
- **apply.** Same expression.
- **Retarget.** `if (a1Retarget && inMask && mx>0.5) { id=fn(); if(!id) id=Invalid; }` becomes `a1TryRetarget` + `a1NewTarget`. Same, including the argument order `(attacker, mx, tgt, Invalid)`.
- **Rethink.** `newId==Invalid && ms<3000 → cur=rd(+0x570); if(cur>ms) wr` becomes `a1RethinkWanted && a1RethinkWrite(rd(...))`. Same, and the short-circuit keeps the read conditional.

**P1i**
- The `!p1` reset uses `p1ParityReset(false,a,b) == (a|b)!=0` and `p1Local(true)=0xFA0`. Same.
- The run write `p1Local(run)` is the same as `run?0xFA0:0`.
- Added: two globals for the E3 log only.

**FM1**
- Same condition, with a redundant re-test inside `fm1RethinkWrite`. Both `rd`s are now evaluated as arguments, which is harmless (SafeRead).
- The vanilla `ClearAllActions(bumpee,1)` call is unchanged.

**FM4**
- `begin` = old `g_snapCr=0; if(fm4!=1 && !log) return;`.
- `commit` = `g_snapCr=cr`.
- `take` = old `if(!g_snapCr||g_snapCr!=cr){g_snapCr=0;return;} g_snapCr=0;`.
- `fm4ShouldRestore` = `ok && fm4==1`.

**A4**
- a4=0: `a4VanillaResult(v)` = `v==0?1:0`.
- a4=1: `a4Decide(!readable)` = `{1,0,guarded}` = old `frw(0); return 1`; `a4Decide(readable,v)` = `{v==0?1:0, v}`.
- Only the log is throttled now.

**C1**
- `who = !partyOnly || (type==5 && party)`. `ObjTypeCreature==5` (`CompanionAI.cpp:65`). Party is read only for creatures, as before.
- `capOk = who && cap.allow` keeps the old call condition, so the cap is consumed only when `who`.
- `c1Value` = -1 only if enabled && who && capOk. Same.

**loadIni**
- `reloadDue` = the old 1 s gate.
- `reloadPre`/`reloadPost` reproduce the old branch order: force wins on missing → defaults; seen → keep log; else silent. Unchanged → skip. Open failure → retry. No section and not force → keep.
- The only extra effect is that `g_iniText` is zeroed when the open fails; it is not used afterwards.
