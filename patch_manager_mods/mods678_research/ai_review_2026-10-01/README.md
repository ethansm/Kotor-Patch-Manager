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
- **A1 / A2** `0x6D740F` (attack prologue) writes `[creature+0x570]=3000` on every entry. AIUpdate `0x5699CE..0x569A72` fires script slot 6 (end of round) only when `+0x520==1`, the timer `+0x570<=0`, and `vt+0x94()==0` (no queued action), then resets to 3000. An attack action that re-queues itself every frame therefore never gets the fallback. Not yet checked at runtime: that stance 11/12 with an out-of-range target really loops without a combat round.

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
