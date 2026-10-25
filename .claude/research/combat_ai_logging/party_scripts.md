# Party AI trace coverage (recon, 2026-10-25)

Source: `patch_manager_mods/ranged_ai_src/script/party_ai_patches.py` PATCHES list, lines 72-497 (36 entries). E2_DLG (502-516) and E2_WRAP (517+) are separate lists and are not in the table; they print nothing.

## ShipBuild question

No party print runs without the companion-ai DLL. `GN_MyPrintString` prints only when `!ShipBuild()` (vanilla `tools/vanilla/k_inc_gensupport.nss:3898-3904` under `~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/`). Vanilla ShipBuild returns 1 (`mods678_research/ai_review_2026-10-01/designs/design_ai_trace.md:67`; "ShipBuild stays TRUE" when trace is off at :330). The party fixes never call ShipBuild (`mods678_research/ai_review_2026-10-01/build_reviews/review_script_build.md:170`), so the DLL is needed for trace lines only, not for behaviour.

Trace coverage: all patched code compiles only into `k_ai_master` (`ranged_ai_src/script/build_party_ai_scripts.py:199-207`); the 16 routers use vanilla includes (`:307`). `k_ai_master` is in `companion_ai_src/companion_ai.ini:15` trace_scripts, so T1 (0x692C50, design_ai_trace.md:122) flips every party print.

## Table (one row per PATCHES entry; line = party_ai_patches.py)

| id (line) | scope | what it changes | prints when it acts? | print prefix |
|---|---|---|---|---|
| MOD8-proto (74) | party | adds stance-16 and party-fix prototypes | no | none |
| MOD8-dispatch (75) | party | stance 16 to kite routine after 10 m leash | no (leash return silent) | none (kite prints in kite_routine.nss) |
| F5 (76) | party | skips duplicate round-end runs, max 3 | yes | `Party F5:` duplicate combat round run skipped |
| E1 (95) | party | calls combat exit for stances 10-13, 16 | yes, via helper | `Party E1:` (party_ai_fixes.nss:55,61,64; skip path :45) |
| E8 (104) | party | drops dead last target | yes | `Party E8:` dropped dead last target |
| M10b (114) | party | beyond 30 m: closest seen enemy, else cancel | yes (switch branch only) | `Party M10:` |
| M9 (133) | party | distance test measured from leader | no | none |
| H1a (137) | party | Revitalize gate uses TRUE/FALSE | no | none |
| H1b (143) | party | no Revitalize without power or Force points | yes | `Party H1:` no usable Revitalize |
| H2a (158) | party | heal race test on member, within 15 m | no | none |
| H2b (163) | party | same test for member 2 | no | none |
| H2e (166) | party | leader counts only within 15 m | no | none |
| H2c (170) | party | ratio test uses member 1 ratio | no | none |
| H2d (174) | party | ratio test uses member 2 ratio | no | none |
| M5 (177) | party | poison loop runs; heal-reachable members only | no | none |
| M5b (213) | party | Cure removed from poison choice | no | none |
| M3 (223) | party | skips debilitated enemies (test was inverted) | no | none |
| M1 (230) | party | switch only to melee config 2 | yes (switch only) | `Party M1:` |
| E3 (239) | party | unseen intruder: helper holds or stands off | yes, via helper | `Party E3:` (party_ai_fixes.nss:78,86) |
| L4 (254) | party (PC-only trees) | tier V buff test checks tier V talent | no | none |
| MEDPAC-a (266) | party | medpac/repair-kit branch closed for party | no | none |
| MEDPAC-b (278) | party | Med Pack line for party, bValid FALSE | no in practice: print (:285) sits inside the closed branch | `Party MEDPAC:` (unreachable) |
| ANTIDOTE (297) | party | Remove Poison Item branch closed for party | no | none |
| F2a (318) | party | skip buff whose family is already active | yes | `Party F2:` buff family active, no buff cast |
| F2b (335) | party | drop force power whose family is active on target | yes | `Party F2:` family active, dropped spell N |
| M2 (347) | party (S) | AoE centre must be live, seen enemy | no | none |
| M4a (358) | party (S) | fixes nDroid shadow; prints droid flag | yes, on every entry (text 0 or 1) | `Party M4:` targeted force power, droid flag N |
| M4b (372) | party (S) | droid disable sets nDisable, not nDroid | no | none |
| M4c (379) | party (S) | droid disable roll test uses nDisable | no | none |
| M4d (382) | party (S) | droid disable branch uses nDisable | no | none |
| M7a (385) | party context | items 104-105 ranged, 106-107 melee for party | no | none |
| M7b (404) | party context | same mapping in GN_GetWeaponType, returns early | no (vanilla "Return No Weapon Type" only on fall-through) | none (vanilla text, no prefix) |
| E6 (420) | hench | weapon-type 0 is melee only within 5 m | no | none |
| E13 (427) | hench (leader) | member-only shout (leader already shouted) | no | none |
| E11 (448) | hench | party stand-off; ranged, stationary, kite do nothing | yes for stand-off; silent for the 3 holding styles | `Party E11:` unseen damager, stand off |
| E10 (470) | party (hench) | party opens doors, never bashes | no | none |

## Gaps

1. MEDPAC-a (266) silently closes the medpac branch for party, so `Party MEDPAC:` (285) can never fire. The medpac rule leaves no trace line.
2. M5 (177), M5b (213), ANTIDOTE (297): poison pick and Cure or antidote removal change silently. Evidence is only the absent "cure poison first" line (party_ai_fixes.nss:55).
3. H1a (137), H2a/H2b/H2e (158-166), H2c/H2d (170-174): heal-target filters and gate change silently. Only H1b prints.
4. M3 (223), M9 (133), M2 (347): target choice and AoE centre change silently.
5. M4b-d (372-382): droid-disable branch becomes reachable for party with no print. M4a (370) shows the flag, not the choice.
6. M7a (385), M7b (404): item mapping changes silently. M7b's early return also hides the vanilla "Return No Weapon Type" line, so its absence is ambiguous.
7. E6 (420), E13 (427), E10 (470), E11 else branch (448): hench skips (no stand-off for ranged, stationary, kite; no bash; weapon reach; no leader shout) print nothing.
8. M1 (230) no switch when config 2 is empty (type 0); M10b (114) else branch CancelCombat; MOD8-dispatch (75) leash return; L4 (254): silent decision changes.
9. Volume: no PATCHES print sits inside a loop, and none prints varying text per call in a loop. Per-round repeats (F5 skips, E8 dead target, E1 skip line) reuse identical text; companion-ai dedupe_ms=1000 (companion_ai.ini:31) collapses same-object repeats within 1 s. Not flagged.
10. Coverage check: no gap from script scope. Patched code lives only in k_ai_master, which is traced (see ShipBuild section).

## Other party AI sources (ls of script/, one level)

- `build_kite_script.py`: builder, no game prints.
- `build_party_ai_scripts.py`: builder, no game prints.
- `kite_routine.nss` (stance 16, not in PATCHES): prints at :184 (E12 reset), :208 (K1 drop target + name), :223 (step 1 streak N), :233 (K2 cap), :252 (step 3 regain). None in a loop.
- `party_ai_fixes.nss` (helpers): prints at :45, :55, :61, :64 (E1), :78, :86 (E3). No print in the F2 helpers (GN_PartyPowerFamily, GN_PartyFamilyActive).
- `party_ai_patches.py`: the patch text itself; prints listed in the table.
- `test_party_ai_patches.py`: no game prints (no GN_MyPrintString match).
- `__pycache__/`: not source.
