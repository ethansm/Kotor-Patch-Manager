# Companion AI review (2026-10-01): index and triage

Three independent read-only Opus reviews of the live party AI (static only; nothing installed):
- `review_ai_scripts_decision.md`: combat decision layer (`k_ai_master` + `k_inc_generic`/`k_inc_gensupport`/`k_inc_switch`), plus our stance-16 kite routine.
- `review_ai_scripts_events.md`: companion event scripts (`k_hen_*`, `k_oei_*`, per-companion `k_hen_<name>dlg`) and the `k_ai_master` event cases.
- `review_ai_engine.md`: engine side (attack action, the 3 s fallback, perception, follow/movement, scheduler, the stance readers), with a pipeline map for the DB.

Basis: the vanilla `k_ai_master` include closure is proven byte-identical to the shipped NCS (nwnnsscomp), so the source is what runs. No module capsule ships `k_ai_master`, `k_inc_generic` or the core `k_hen_*`. Only 003EBO has its own `k_hen_combend01`/`k_hen_damage01`, and there is no source for them.

## Checked by the lead (Opus) on disk
- **H1** `k_inc_generic.nss:1006` `GN_CheckSeriesRevitalize() != -1` is always true (the function returns TRUE/FALSE). In the `GEN_TALENT_REVITALIZE` case (`:2512`), `bValid=TRUE` with talent id -1 still runs `ClearAllActions` + `ActionUseTalentOnObject(invalid)` + `return TRUE` (`:2725`). A Jedi Support companion without Revitalize wastes its rounds while anyone is knocked out.
- **H2** `GN_CheckIfInjured` (`:2318-2340`) is a copy-paste bug: all three branches test `fDown00`, and the race test for oP1/oP2 uses `oP0`. AI healing fires only when the leader is under 50%, and it heals the leader.
- **E2** `k_hen_miradlg.nss` (all 13 `k_hen_<name>dlg`) includes vanilla `k_inc_generic` and calls `GN_RespondToShout` directly; the `ExecuteScript("k_ai_master", ...)` is commented out (line 90). So a companion's shout-driven first combat action runs vanilla code (stance 16 -> default routine).
- **A1 / A2** `0x6D740F` (attack prologue) writes `[creature+0x570]=3000` on every entry. AIUpdate `0x5699CE..0x569A72` fires script slot 6 (end of round) when `+0x520==1`, the timer `+0x570<=0`, and `vt+0x94()==0`, then resets to 3000. Correction (engine-fix design, lead-checked): `vt+0x94` = `0x563460`, which returns 0 for any party member (`+0x11AC!=0`) and otherwise calls the GetDead-like `0x5477F0`. It is not a queue check, so for party members only the timer gates the fallback. An attack action that re-queues itself every frame therefore never gets the fallback. Not yet checked at runtime: that stance 11/12 with an out-of-range target really loops without a combat round.

All other findings: reviewer confidence as stated in the reports (`verified` = quoted lines / asm, `plausible` = needs a runtime check).

## Triage

**Tier 1: affects Mod 8 stance 16 directly (Phase 3 fix round)**
- **A1:** E-H3 maps 16->11, so stance 16 inherits the out-of-range idle loop. This is play-test item (b). Fix options: an engine fix at `0x6D84FC` (end the action through `0x6D83FF`), or apply the 16->11 mapping only within weapon range plus a script approach step.
- **E2:** recompile the 13 `k_hen_<name>dlg` against our include, or route shouts to `k_ai_master` event 2004.
- **K1:** kite target not checked with `GetIsEnemy`/`GetIsDead`.
- **M1:** swap to an empty weapon config 2 leaves the companion unarmed.
- **K2:** no cap on consecutive kites.
- **E12:** the cooldown at local 20 carries over between fights.
- Local 8: the engine zeroes it at combat exit (`0x57A5C1`, engine report).

**Tier 2: vanilla script bugs (fixable in our rebuilt `k_ai_master`, the 13 dlg scripts, and `a_atkonend`/`a_force_combat`/`k_combat_rnd`)**
- H1, H2;
- M2 (AOE on allies), M3 (prefers disabled enemies), M4 (`nDroid` shadowed), M5 (poison loop never runs);
- E1 (only Aggressive runs the post-DCR end-of-combat), E3 (2 m run at unseen enemies);
- M6-M10, E4-E13, L1-L8.

**Tier 3: engine (KPM hooks)**
- A1 for vanilla stances 11/12;
- P1 (perception throttle never stored: an O(enemies x party) LOS per tick);
- C1, FM1 (leader bump clears the companion's queue), FM2, FM3, FM4, A4 (NULL spell row);
- P2, A3.

**Not a companion-AI issue:** live `post-combat-movement-fix-v1` (`0x7AF198`) fixes the controlled character's keyboard input at high FPS.

## Logging (user question 2026-10-01)
The scripts already contain about 230 `GN_MyPrintString`/`Db_MyPrintString` calls gated by `if(!ShipBuild())`, plus `AurPostString` probes. Plan:
- DLL hooks on the VM commands `ShipBuild` (return FALSE only while an AI script runs) and `PrintString`/`AurPostString`, writing to `ai_trace.txt` with time and creature, with INI filters.
- Engine log points from the reports: the A1 loop counter at the attack prologue, the `+0x570` fallback firing, the P1 perception rate, and FM1 bump clears.
- Not built yet; designed after the user picks a scope.

## Designs (2026-10-01; user chose all four scopes: trace logging, Mod 8 fixes, vanilla script fixes, engine fixes)
`designs/`: `design_ai_trace.md`, `design_ai_engine_fixes.md`, `design_ai_script_fixes.md`, plus `scriptfix/` (patch list + prototype builder, diffs, proof log) and `windows/` (validator copies and outputs).
- **Placement:** new patch id `companion-ai` (trace + engine fixes, every fix INI-gated; all flags 0 = vanilla). ranged-ai 0.3.0 changes only the E-H3 handler (`kite_approach`). Scripts: new `build_party_ai_scripts.py`. 13 `k_hen_<name>dlg` + `a_atkonend`/`a_force_combat`/`k_combat_rnd` are ROUTED to `k_ai_master` (events 2004/3001), not recompiled against the include. The 17 vanilla proofs pass, and the new build reproduces live `k_ai_master` `94db8304` with the Mod 8 insertions only.
- **Trace:** ShipBuild `0x692C40` (idx 761; hook `0x692C50` len 10 overrides `[ebp-4]`), PrintString `0x68F260` (idx 1, hook `0x68F301`), AurPostString `0x6F6200` (idx 582, an empty retail stub, hook `0x6F62F2`). VM `[0xA1B4A8]`, VMI `[VM+0x1C]`, depth `[VMI+0x28]`, script name `VMI+0x44+depth*0x28`, OBJECT_SELF `[VMI+0x18C+depth*4]`. The ShipBuild scan of all 4883 loadable script names: it only guards prints. Engine log points E1 `0x6D740F`, E1b `0x6D8BE2`, E2 `0x569A08`, E3 `0x56BDBD`. **Drop E4** (it shares FM1's site; log from the FM1 handler).
- **Engine fixes:** A1 `0x6D853E` len 10 -> exit `0x6D7D2F` (the retarget tail); P1 `0x56BDE3` + `0x56BDB4`; FM1 `0x586F43`; FM4 mode 1 `0x57451F` + `0x575183`; A4 `0x6D9E2A` len 15; C1 `0x5868B9`/`0x5868F1`/`0x586929`. Framework fact: in a consumed-exit hook, EAX = the handler result on the normal resume path too.
- Status: designs only, not built. Next: user decisions -> build -> static gates -> Opus review -> ONE combined user-gated install.

## User decisions (2026-10-01)
- Vanilla Ranged/Stationary: never approach, even with a melee weapon (A1 only ends the stuck action and retargets in range). Stationary also retargets (lead default).
- **Party first:** every script fix is party-gated; enemy improvements are noted in `ENEMY_AI_NOTES.md` for a future iteration.
- P1 perception: built, shipped OFF until a stealth/aggro test.
- H2 healing: Force powers/talents only; no medpacs from the shared inventory.
- Lead defaults: FM4 mode 1 (save files unchanged); trace E4 dropped (FM1 handler logs); one log file `ai_trace.txt`; E5 skipped; E10 on (no door bashing); E6(c) Remote flag unchanged.
- **Future medpac rule (user, 2026-10-01):** a later iteration may let companions use medpacs depending on the party's stock (e.g. only above a threshold), and only when no heal power is available. Needs: a count of medpacs in the shared inventory (`GetItemPossessedBy`/item stack size, or a DLL helper), a `GetCreatureHasTalent(tSpe11_05)` guard (design section 4), and an INI/global threshold. This round: an explicit guard so party members never take the medpac/repair-kit branch.

## Build reviews (2026-10-01; `build_reviews/`)
- **DLLs** (`review_dll_build.md`): no BLOCKER.
  - SF1: word INI values (`a1=off`) fell back to the built-in default ON. FIXED by the lead: on/off/true/false/yes/no; an unparseable flag means OFF and is logged.
  - SF2: the check_windows EAX analysis is incomplete (fixing).
  - SF3: the behaviour decisions are not host-tested (fixing).
  - FM3 probe dropped from the shipped hooks.
- **Scripts** (`review_script_build.md`): BLOCKER B1, lead-verified. TSL Cure does NOT remove poison (`k_inc_force.nss` ~937 commented out); Heal/Master Heal remove it only from non-droids within 15 m, so the M5 fix would loop Cure casts.
  - S1: E1 ends fights while unseen enemies still attack.
  - S2: heal targets beyond 15 m.
  - S3: E6/E11 not party-gated (hostile HK-50s etc. use k_hen_*).
  - S4: E3 still approaches for 11/12.
  - S5: the installer backup can be overwritten on a re-run.
  - All being fixed.
