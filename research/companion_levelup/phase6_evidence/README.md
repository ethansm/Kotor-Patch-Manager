# Phase 6 evidence (level-up override)

Plan: `../PHASE6_PLAN.md`. Progress and conclusions: `patch_manager_mods/08_level_up_override.md` "Session notes - Phase 6".

| Prefix | Content |
|---|---|
| `V_*` | Live run 4 (and later smoke runs): log excerpts, GLOBALVARS read-backs, ini snapshots |
| `R10_*` | Static Ghidra research for the PC level-up filter (C1): list build sites, availability tests, hook-site checks |
| `B_*` | Preset quality: warning triage, early-conversion re-base cases, build review |
| `A_*` | Release hardening: robustness review, mutation results |

Baseline before run 4 (2026-10-03): `levelup_override_log.txt` 386 lines; live dll `level-up-override.dll` md5 e64cb387;
`levelup_playthroughs.ini` = `[g11b3b802] atton=atton__melee_dual_light_early|instant=1|hold=1`; game-dir ini `mode=rewrite apply=1 verbose=1`.
