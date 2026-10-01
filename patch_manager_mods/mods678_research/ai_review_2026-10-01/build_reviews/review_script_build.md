# Pre-install review: KOTOR II party AI script build (vanilla fixes + Mod 8 kite v2 + E2 routing)

Reviewer: independent Opus pass, 2026-10-01, read-only (nothing installed, launched, committed or rebuilt).
Repo: `KOTOR-II` worktree, HEAD `2e596e9` (contains `3dc974e`; the script files are unchanged since `3dc974e`, and their md5s match `manifest.json` inputs: `party_ai_patches.py 89a39f45`, `kite_routine.nss dcfc3f66`, `party_ai_fixes.nss 1a8512d6`).
Reviewed output: `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/` + `out_party_ai/` (manifest consistent: every `.ncs` and source md5 re-checked, `k_ai_master.ncs` `2ff61711b884…`, 130000 B). Live `override/k_ai_master.ncs` is still `94db8304` (Mod 8 0.2.0).
Line numbers below are 1-based in the generated `src_party_ai/*.nss` (CR stripped; CRLF does not shift lines). In `k_inc_generic.nss` the appended kite routine starts at 4113 (kite_routine.nss line N = generated 4112+N) and `party_ai_fixes.nss` at 4369 (fixes line N = 4368+N).

## Findings

| id | severity | where | summary |
|---|---|---|---|
| B1 | **BLOCKER** | `src_party_ai/k_inc_generic.nss:2305-2317` (M5) + `:2453-2477` (REMOVE_POISON) + `:4399-4404` (E1) | M5 turns on the party poison cure, but **Cure does not remove poison in TSL** and Heal/Master Heal only reach non-droid members **within 15 m**. A Cure-only companion (or a healer more than 15 m from the poisoned member) recasts every DCR until its Force points are gone: the H1 "useless talent every round" pattern. With E1 it also blocks the post-combat follow. |
| S1 | SHOULD-FIX | `k_inc_generic.nss:299-303`, `:4378-4413` (GN_PartyStanceCombatExit) | E1 fires on "no enemy **seen**", not "fight over": unseen-but-attacking enemies (beyond perception range, behind cover, stealth) make stances 10-13/16 CancelCombat + follow. Ping-pong loop with E3 (approach unseen intruder -> DCR on arrival -> E1 -> follow -> next leader shout -> approach ...). Pre-empts kite step 3 for a lone blocked target. No conversation/commandable guard (bypasses the vanilla Jedi Support cutscene guard). |
| S2 | SHOULD-FIX | `k_inc_generic.nss:2368-2397` (H2) | H2 makes oP1/oP2 heal targets, but Heal/Cure/Master Heal only reach members < 15 m. A target out of range keeps the healer recasting every DCR (and, via E1, not following) until FP run out. Vanilla had this only for the leader. |
| S3 | SHOULD-FIX | `src_party_ai/k_ai_master.nss:693-696` (E6), `:769-778` (E11) | Not party-gated. Hostile/non-party creatures with `k_hen_*` attacked/damaged scripts exist (scan below: `n_hk50_003` HK-502 faction 1 in 261tel/350nar, `n_trnremote` faction 1 in 745kor, HK-50/51s in 902mal, Telos sisters, Kreia boss 711kor/907mal). E11 changes their unseen-damager move: violates the binding "party only, enemies stay vanilla". |
| S4 | SHOULD-FIX | `k_inc_generic.nss:4419-4434` (GN_PartyApproachUnseen, E3) | Ranged (11) still approaches an unseen intruder (15 m, or **4 m with a melee weapon**), contradicting the user decision "vanilla Ranged/Stationary never approach, even with a melee weapon" and the E11 patch (which excludes 11/12/16). |
| S5 | SHOULD-FIX | `patch_manager_mods/mods678_research/apply_party_ai_scripts.py:64,104-112,141-143` | Backup dir `backups_party_ai_<tag>` is reused with `exist_ok=True` and overwritten. A re-run (same `--tag`, or the default HHMM colliding) replaces the `94db8304` backup with the party build. The printed rollback always says "restore 94db8304 from backups_party_ai_<this tag>", which is wrong on any re-run, and `cp` fails if `k_ai_master` was absent. |
| N1 | NIT | `k_inc_generic.nss:4147-4148`, `:4243` | Stale comments: with ranged-ai 0.3.0 (`kite_approach=1`) stance 16 *does* approach when out of range or holding a melee weapon. The 15 m filter is still right (every ranged base item has `maxattackrange` >= 17). |
| N2 | NIT | `k_inc_generic.nss:4184-4200` (K3) | Dead enemies inside 3.5 m use up the 3 scan slots, so a live melee 4th enemy is missed. Rare. |
| N3 | NIT | `k_inc_generic.nss:4290-4294` (E12) | The `!GetIsInCombat` reset is silent. If the combat flag drops mid-fight, K2 (streak cap) and the cooldown are defeated with no trace evidence. Add one print when it clears non-zero values. |
| N4 | NIT (pre-existing) | `k_inc_generic.nss:2059-2076` | A leader shout passing "Check 3" does `ClearAllActions()` + DCR, which kills an in-flight kite move. The cooldown guard (`GetCurrentAction()==ACTION_MOVETOPOINT`) then sees an empty queue. Same in v1. Test item: `Kite step 1` followed by `Shout Clear 1800` in the same second. |
| N5 | NIT | `party_ai_patches.py:415-431` | Anchor uniqueness is counted on the progressively patched text, and non-overlap is asserted nowhere. I re-checked all 31 + 26 + 3 anchors on the pristine vanilla text: each count is 1, there are no overlaps, and no anchor occurs inside another patch's new text. Assert both on the pristine text. |
| N6 | NIT | `build_party_ai_scripts.py:366-370` | The trace-string check proves only that the patched `k_inc_generic` was compiled in, not `k_inc_gensupport` (M2/M4/M7 have no strings). I verified it another way: the ACTION-count deltas between the vanilla and new `k_ai_master.ncs` equal the source deltas (IsObjectPartyMember +18, GetObjectSeen +2, GetIsEnemy +2, GetIsDead +6). Add a gensupport marker. |
| N7 | NIT | `build_party_ai_scripts.py:299-338` | The medpac/antidote guard is a sound text check for the current code. To make it robust, also assert that `GetCreatureTalentBest(0x1408` and `TalentSpell(84/127/128)` only ever bind `tSpe11_05`. |
| N8 | NIT | `build_party_ai_scripts.py:395`, manifest | `patched_with` for `k_ai_master.ncs` lists only E6/E13/E11/E10 (not the 27 include patches). The compile.sh and nwnnsscomp md5s are not recorded. |
| N9 | NIT | `apply_party_ai_scripts.py:34-41,49,90,117-122` | `find_ci` returns the first case-variant only, so a second case-duplicate goes unnoticed. The `arg_order` result is printed, not enforced. Manifest inputs are not compared with the repo files. A failed copy halfway exits without restoring. The `pgrep -f '^S:.*swkotor2\.exe'` pattern is the repo convention (also in `apply_mods68_data.py`); I could not test it with the game running. |
| N10 | NIT | design `design_ai_script_fixes.md` §4 "M5" | "Cure, Heal and Master Heal remove poison in TSL" is wrong for Cure (root cause of B1). Correct the doc. |

## Details and concrete fixes

### B1 (BLOCKER): the M5 poison cure loops with Cure, or with Heal out of range
Generated `k_inc_generic.nss:2453-2477` (party reaches it now that M5 makes `GN_CheckIfPoisoned` return party members):
```c
tSpe11_01 = TalentSpell(FORCE_POWER_HEAL);
tSpe11_02 = TalentSpell(FORCE_POWER_CURE);
...
if(GetCreatureHasTalent(tSpe11_01) || GetCreatureHasTalent(tSpe11_02) || GetCreatureHasTalent(tSpe11_04) )
{   bValid = TRUE;  ... else if(GetCreatureHasTalent(tSpe11_02)) tUse = tSpe11_02; ...
```
The `vanilla_all/k_inc_force.nss` impact code shows two things:
- `case FORCE_POWER_CURE` (line 895ff) has `//Sp_RemoveSpecificEffect(EFFECT_TYPE_POISON, oParty);` **commented out**.
- Heal (2289ff) and Master Heal (2355ff) remove poison only from members that pass `GetRacialType(oParty) != RACIAL_TYPE_DROID && GetDistanceBetween(OBJECT_SELF, oParty) < 15.0`.

So for a party member:
- **Cure-only caster.** While anyone is poisoned, every DCR casts Cure (`Clear 2300`) and returns TRUE. Nothing attacks, and the poison stays.
- **Heal caster.** The same loop runs whenever the poisoned member is 15 m or more away.

It ends only when `GetCreatureHasTalent` fails for lack of FP. That check does test FP: `0x67ACC0` -> `0x6B8840` -> `0x6B8F40` compares the alignment-adjusted cost with current FP.

With E1 (`:4399`, cure first when no enemy is seen), the loop also runs after combat. Each cast is a combat action, so round-end 2003 -> DCR -> E1 -> Cure again. The companion stands there instead of following. In vanilla the party loop never ran, so this is new. It is exactly the "invalid/useless talent idle" class the brief excludes.

Fix (two small patch-list edits, party-only):
1. After the unique anchor `            tSpe11_03 = TalentSpell(67); //Remove Poison Item\n` add:
   ```c
   //Party AI fix M5b: Cure heals but never removes poison in TSL (k_inc_force CURE: the poison removal is commented out).
   if(IsObjectPartyMember(OBJECT_SELF)) { tSpe11_02 = TalentSpell(-1); }
   ```
   `GetCreatureHasTalent(TalentSpell(-1))` is FALSE (`0x6B8840` returns 0 for id -1). The antidote assertion is unaffected (`TalentSpell(67)` still occurs once).
2. Widen the M5 anchor to the whole loop and only report members a Heal/Master Heal reaches:
   ```c
   for(nCnt = 0; nCnt < 3; nCnt++)
   {
       object oM5 = GetPartyMemberByIndex(nCnt);
       if(GetIsObjectValid(oM5) && GetRacialType(oM5) != RACIAL_TYPE_DROID &&
          GetDistanceBetween(OBJECT_SELF, oM5) < 15.0)
       {
           ePoison = GetFirstEffect(oM5);
           while(GetIsEffectValid(ePoison))
           {
               if(GetEffectType(ePoison) == EFFECT_TYPE_POISON) { return oM5; }
               ePoison = GetNextEffect(oM5);
           }
       }
   }
   ```
Test: poison a member. A Cure-only companion keeps fighting (no `Clear 2300`). A Heal companion casts once, and the poison is gone.

### S1 (SHOULD-FIX): E1 cancels live fights against unseen attackers and loops with E3
`k_inc_generic.nss:4390-4396`:
```c
if (GetIsObjectValid(GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, 1,
                                        CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN)))
{   return FALSE; }
... return GN_DoPostDCRChecks();   // CancelCombat + ClearAllActions + ActionFollowLeader
```
The design says this matches Aggressive, but it does not. Aggressive reaches `GN_DoPostDCRChecks` only when `GN_RunDefaultAIRoutine` finds **no target at all**: a live, unseen `tPR.oLastTarget` still gets `ActionAttack` (`:888-925`). E1 ignores both `oIntruder` and the last target. Three concrete effects:
- **Unseen shooters.** Blaster rifles reach 28 m, beyond typical perception range. Enemies behind cover or stealthed are also unseen. Against them every 2003/2006/shout DCR makes Ranged/Support/Defensive/Stationary/Kite companions CancelCombat and follow the leader while they are being shot. E11 (`k_ai_master.nss:776`) is then dead for 10/13 as well, because it is queued behind the follow.
- **Loop with E3.** Leader hit -> shout -> companion (no attack target after the CancelCombat) -> `GN_PartyApproachUnseen` moves to 15/4 m and queues `DCR(oIntruder)` -> on arrival, if the intruder is still unseen, E1 -> follow -> next leader hit -> approach again. The E3 row of the design's own test matrix ("move to about 15 m stand-off, then a combat round") ends in E1's follow whenever the sniper stays unseen.
- **Kite step 3.** When the only enemy's shot is blocked it is usually unseen too, so E1 runs before the kite routine can reposition (design §4 E1(b) concedes this).
- **Cutscenes.** There is no `GetIsConversationActive()` / `GetCommandable()` guard. Vanilla Jedi Support returns early in conversations (`:1020-1023`), and the heartbeat follow requires both. E1 now runs ClearAllActions + follow for 13 (and the others) when a DCR lands during a conversation.

Fix: pass `oIntruder` in (prototype at `:194` and call site `:300`) and add these guards before the poison/heal/exit code:
```c
if (GetIsConversationActive() || !GetCommandable() || GetCurrentHitPoints(OBJECT_SELF) < 1) return FALSE;
object oLT = GetAttackTarget(GetPartyMemberByIndex(0));
object oHA = GetLastHostileActor(OBJECT_SELF);
if ((GetIsObjectValid(oIntruder) && !GetIsDead(oIntruder) && GetIsEnemy(oIntruder)) ||
    (GetIsObjectValid(oLT) && !GetIsDead(oLT) && GetIsEnemy(oLT)) ||
    (GetIsObjectValid(oHA) && !GetIsDead(oHA) && GetIsEnemy(oHA) && GetDistanceToObject(oHA) <= 30.0))
{
    return FALSE;   // the fight is still on: leave it to the stance routine (vanilla behaviour)
}
```
This keeps E1's purpose (the end of a fight: everyone dead or gone) without cancelling live fights. A trace line for the skip helps the A/B. The heartbeat still recovers the idle cases, as in vanilla.

### S2 (SHOULD-FIX): H2 heal target out of the 15 m power radius
After H2, `GN_CheckIfInjured` (`:2368-2397`) returns any member under 50%. The HEALING branch casts the power on `OBJECT_SELF` (a party-area power, 15 m, no droids), so a target 15 m or more away is never healed. The companion recasts every DCR, and through E1 also after combat without following, until FP run out. Fix (party branch only): add `&& GetDistanceBetween(OBJECT_SELF, oPn) < 15.0` to the three ratio conditions (`:2368`, `:2373`, `:2377`). The leader line needs its own anchor (`if(GetIsObjectValid(oP0) && GetRacialType(oP0) != RACIAL_TYPE_DROID)`, unique inside the function; assert it). That also fixes the vanilla leader-out-of-range case.

### S3 (SHOULD-FIX): E6 and E11 reach non-party creatures
```c
// k_ai_master.nss:695
if ( nWeaponType == 1 || (nWeaponType == 0 && GetIsObjectValid(oAttacker) && GetDistanceBetween(OBJECT_SELF, oAttacker) <= 5.0) )
// k_ai_master.nss:771-778  (E11 move/stand-off)
```
Cases 2005/2006 fire for every creature with `k_hen_attacked01`/`k_hen_damage01`. A UTC scan of all Modules capsules + override (113 resrefs) finds hostile or later-hostile ones:
- `n_hk50_003` (HK-502, faction 1), `n_hk50_002/004`, `n_hk50_001` and `n_hk51_*` (902mal);
- `n_trnremote` (745kor, faction 1, also uses `k_hen_remotedlg`);
- the 262tel sisters;
- `p_kreia_evil001` (711kor/907mal).

E11 changes their response to an unseen damager. E6 is behaviour-inert for NPCs except `NPC_AISTYLE_HEALER`, which uses local 8 as its threshold, but it is still outside the binding rule. Fix: `if(IsObjectPartyMember(OBJECT_SELF) && …)` around the new E6 term (NPCs keep `nWeaponType == 1 || nWeaponType == 0`). For E11, use `if(!IsObjectPartyMember(OBJECT_SELF)) { vanilla ActionMoveToLocation(GetLocation(oTarget), TRUE); ActionDoCommand(DCR); } else { new code }`.

### S4 (SHOULD-FIX): E3 still moves vanilla Ranged
`GN_PartyApproachUnseen` (`:4419-4434`) holds only for Stationary (12). Ranged (11) runs to 15 m, or to 4 m when it holds a melee weapon. User decision: "Vanilla Ranged/Stationary: never approach, even with a melee weapon". E11 already excludes 11/12/16. Fix: hold for 11 and 12. For 16, pick one rule for both E3 and E11 (today E3 moves Kite and E11 does not). With 0.3.0 `kite_approach`, letting 16 take the 15 m stand-off in both is defensible; holding in both is the simplest.

### S5 (SHOULD-FIX): installer backup/rollback on re-runs
```python
bak = os.path.join(W, f"backups_party_ai_{a.tag}")      # :64
os.makedirs(bak, exist_ok=True)                          # :104
shutil.copy2(dst, os.path.join(bak, e["name"]))           # :108 overwrites a previous backup
print(f'  cp "{os.path.join(bak, MASTER)}" ... # expect {MOD8_MD5}')   # :142
```
Fixes:
- Refuse if `bak` exists.
- When the live `k_ai_master` is "already ours", do not write a fresh backup over the Mod 8 one. Point the rollback at the backup dir whose `BEFORE.txt` shows `94db8304` (glob `backups_party_ai_*/BEFORE.txt`).
- Print `rm` instead of `cp` when `k_ai_master` was absent.
- Print the expected md5 from `BEFORE.txt`, not a constant.

## Verified OK (per item)

**E2 routing**
- **Parity with the dlg shout path.** All 13 `k_hen_<name>dlg` `main()` bodies are identical (one md5; only `SelectAndRunDLG` differs). Vanilla dlg for `nMatch != -1`: AI_OFF check -> `GetCommandable` -> `GN_RespondToShout(GetLastSpeaker(), nMatch, GetLastHostileActor(oShouter))` -> no `ResetDialogState` -> 1004 if `SW_FLAG_EVENT_ON_DIALOGUE`. `k_ai_master.nss:643-673` (case 2004, unpatched) is the same sequence. The early `return` prevents a double 1004. Conversations (`-1`) keep `SelectAndRunDLG`, `ResetDialogState` and 1004 as before.
- **Event context across `ExecuteScript`.** `ExecuteScript(s, oTarget, nVar)` runs with `OBJECT_SELF = oTarget`; `nVar` is read by `GetRunScriptVar()` (`k_ai_master.nss:17`). `GetLastSpeaker` (`0x682900`) reads field `+0x1A0` of the OBJECT_SELF object, not VM state, so it survives the nested run. This is the same mechanism as BioWare's `k_hen_dialogue01`/`k_def_dialogue01` (-> `k_ai_master` 2004/1004), which every non-companion henchman and every default NPC uses for shouts.
- **Wrappers.** 3001 = `GN_DetermineCombatRound()` (`k_ai_master.nss:23-27`). In `a_atkonend`, `DelayCommand(0.3, ExecuteScript(...))` binds OBJECT_SELF at scheduling time, as before.
- **Not in the party, puppets, enemies.** Non-party creatures using these dlg scripts (Kreia boss, `n_trnremote`, HK-50s) and puppets reach only party-gated code or the unchanged NPC paths, so they behave as vanilla. The Remote is either party (E1 excludes 14; E3 gives a 15 m stand-off, then the owner leash) or not (vanilla).
- **Nesting.** dlg -> `k_ai_master` is depth +1. No `ExecuteScript` exists inside the includes, so the cap of 8 is not at risk.
- **Rollback.** With the BIF `k_ai_master`, router + vanilla 2004/3001 equals vanilla exactly. Restoring only `94db8304` keeps the routing with the Mod 8 v1 DCR. Both are consistent.
- **Compiled routers.** 2.7-3.8 KB, `ExecuteScript` present, no combat ACTIONs, arg-order checker 0.

**Medpac and antidote guards**
- MEDPAC-a closes the whole item/Drain/Death-Field branch for the party (`:2544-2547`), and MEDPAC-b is unreachable for the party. `tUse` stays the default (invalid) talent, so the generic fallback (`:2827`, `GetIsTalentValid(tUse)`) is closed too. Vanilla already relies on that: SUPRESS_FORCE never assigns `tUse`.
- ANTIDOTE (`:2483`) closes `TalentSpell(67)` for the party.
- No other medpac/antidote path exists in `k_inc_generic`/`gensupport`/`k_ai_master` (grep for `0x1408`, `TalentSpell(67|84|127|128)`).
- REVITALIZE (H1b), BUFF (FP-checked) and HEALING are all free of invalid-talent idles. The one useless-talent loop left is B1, plus the S2 range variant.

**Fixes**
- **H1.** Party gate `== TRUE`. H1b invalidates `tUse` (`TalentSpell(-1)`) when the power is missing or unaffordable. `GetCreatureHasTalent` checks FP (engine, see B1). NPCs keep the vanilla gate.
- **H2.** Index and race corrections are right. Droids are never targets (ratio 10.0, matching Heal skipping droids). See S2 for range.
- **M1.** `nType2 == 1` only. With M7, base items 106/107 count as melee.
- **M2.** Live, seen enemy centre for the party; NPCs unchanged. It also cuts the nested-shape cost.
- **M3.** The party loop skips debilitated enemies and falls back to the first one. The NPC loop is unchanged.
- **M4.** `nDisable` replaces the shadowed local. NPCs are forced to `nDroid=FALSE` (the vanilla result). The callers pass the droid flag (`k_inc_generic.nss:1084/1104`).
- **M7.** The live `baseitems.2da` rows are 104 Mining_Laser and 105 Adv_Mining_Laser (ranged, 23/28 m), and 106 Lightsaber_No_Sound and 107 DM_ElectroStaff (melee). Both functions are party-gated.
- **M9.** Defense-only path; measures from the leader.
- **M10.** Party switches to a seen live `oClose` within 30 m.
- **E8.** Only drops a dead target when no intruder or closest enemy was picked.
- **E10.** Party never bashes.
- **E13.** Leader no longer shouts a third time. A restrict-mode member still shouts. NPCs are unchanged.
- **L4.** Tier V typo; effectively party-only (trees 6-8 require `GetFirstPC()`).
- **No ShipBuild dependency.** The new code never calls `ShipBuild()`; the new strings go only through `GN_MyPrintString`. The trace strings are present in the `.ncs`.

**Kite v2**
- **K1.** Drops a non-enemy or dead target. Step 1 then shoots the threat, step 3 is skipped, and step 4 is vanilla.
- **K2.** Streak cap 2, reset on any cooldown-0 DCR without a kite. Events do not DCR while an attack is queued (2005/2006/shout all require no attack target), so the "shoot" round is a real round.
- **K3.** At most 3 `GetNearestCreature` calls. Uses the party-context M7 classification.
- **M1.** Via the shared `GN_CheckRangedPreferred`.
- **E12.** Two resets: on entry and in E1.
- **Local 8.** Zeroed by every kite and by the engine at combat exit; only party-only code reads it.
- **E-H3 0.3.0.** Every ranged weapon's `maxattackrange` is >= 17 m, so the 15 m spot filter keeps stance 16 in range: no approach is triggered by a kite move. With a melee weapon, steps 1/3 are skipped and the engine approaches like Aggressive, as designed.

**Locals 20/21**
- No vanilla script uses 20/21 as a literal index (grep of all 638 sources).
- Scanned all 272 override `.ncs`. Hits are For Mandalore's `tr_attkre`/`851nih_enter` (set 21) and `m_cnd_mandcheck` (reads 20), all on `henchmand1..4`. Those are non-party `k_def_*` creatures, which our code never writes.
- Scanned 8535 DLGs (all Modules capsules + override) for `a_localn_*`/`c_localn_*`/`k_con_obj_count` with index 20/21:
  - `701KOR kreia_kr` writes 20/21 on `kreia_invis` (non-party);
  - the `702KOR termquiz`/`termtrn` terminals write 20 (placeables);
  - `001EBO combat` sets 20=1 on its owner, and nothing reads it (worst case one DCR of kite cooldown).
- No collision on party members.

**Instruction budget (0x20000 per run)**
- The new loops are bounded:
  - K3: 3 iterations;
  - FindSpot: at most 2 x 12;
  - M3: seen debilitated enemies;
  - M5: 3 members x their effects, about 1k instructions worst case per call, at most 3 calls per DCR.
- M2 reduces the vanilla nested-shape cost for party callers.
- No unbounded loop is new.
- The router adds about 50 instructions per shout.

**Build script**
- The proofs prove what they claim: 20/20 unmodified sources rebuild byte-identical to the Scripts.bif `.ncs`; inputs are byte-compared with the BIF; the Mod 8-only insertions + v1 routine give live `94db8304`.
- The anchor check is sound in effect (independently re-verified on pristine text, see N5). The patch-count check catches file-name typos.
- The medpac/antidote assertion is correct for this code (see N7 for hardening).
- compile.sh layers the patched includes by basename, and they are in the output (N6 deltas).

**Installer**
- Dry run by default.
- Refuses when:
  - the game is running (repo-standard pattern);
  - the manifest is missing or does not list 17 files;
  - any built file's md5/size differs from the manifest;
  - any foreign target exists (only `94db8304` or the new build is allowed for `k_ai_master`, and identical files for the routers).
- Writes `BEFORE.txt`, re-md5s each copy and then all 17.
- Handles case: `find_ci` over the real lowercase `override/` (`Override` is a symlink); new files are written lowercase.

## Verdict
**Not ready as built: fix B1 first** (two small, party-only patch-list edits). Then rebuild and re-run the build gates (proofs, anchors, arg-order 0, guard check). S1-S4 are also small patch-list edits and should go into the same rebuild; S5 is an installer fix. Everything else checks out: E2 parity and rollback safety, the medpac/antidote closure, locals 20/21, kite v2 and the E-H3 interaction, the instruction budget, and the build proofs. Rollback is safe in every combination. If the user wants to deploy before S1-S4, B1 is the only item I would not ship without. Watch the trace for `Party E1:` lines while an enemy is still shooting (S1) and for repeated `Clear 2300` (S2).
