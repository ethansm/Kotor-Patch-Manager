# 08 - Level-up override (per-character fixed level-up path)

Phase 1 (read-only Ghidra investigation) record. Plan: `research/companion_levelup/PHASE1_PLAN.md`; overall:
`research/companion_levelup/MASTER_PLAN.md`. Steam Aspyr LAA `swkotor2.exe`. Every GOG claim is a hypothesis until decompiled.

## Status
2026-09-30: Step 0 done (skeleton). Steps 1-6 not started.

## Address table
| Function | GOG addr | Steam addr | Evidence | Calling conv | Confidence |
|---|---|---|---|---|---|
| (Step 2 candidates go here, marked UNCONFIRMED) | | | | | |

## Struct offsets
(CSWSCreatureStats, CSWCLevelUpStats: fields, container types, element sizes - Step 3a/3c)

## Questions
- Q1 shared routine / best hook point:
- Q2 package selection (class id vs starting_package):
- Q3 server trust of client picks:
- Q4 unspent picks / GUI close:
- Q5 runtime creature identity (Tag):
- Q6 struct layouts:
- Q7 LevelUpHenchman shares code?:
- Q8 hook overlap with installed patches:

## Hook-candidate shortlist (max 3)
(Steam addr, original_bytes from exe, convention, params, readable/writable state, risks)

## Recommended design (A vs B) and Phase 2 spike plan

## Open items

## Commands run
