# Plan CAX1: fix the validated Agy combat-AI review findings (2026-10-29)

## Context
Agy (Gemini) reviewed companion-ai, ranged-ai and the party AI scripts: 14 findings. Validation landed in 4d559c24
(6 Haiku + 1 Sonnet, $1.17): 9 real (none high), 5 latent, 3 known. Merged verdicts:
`.claude/reviews/combat_ai_agy_20261029/validation/validated.tsv`, summary `summary.md` in the same dir.
The review's spell IDs are often wrong; the validated `fix_note` column wins. Correct IDs (vanilla nwscript.nss:873-890):
Enlightenment 180, Force Sight 176, Force Crush 177, Revitalize 173-175, Wookiee Rage 271-273.

## Phases (mark with `plan_manager.py mark-done <Id> --plan .claude/plans/combat_ai_agy_fix_wave1.md --no-handoff`)
| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| CAX.0 | Validate findings (Haiku swarm + Sonnet consolidate) | - | - | DONE | 4d559c24 | validated.tsv |
| CAX.1 | Ingest: WIs from validated.tsv + play row CA-T10 | CAX.0 | - | DONE | 5321abd2 | WI-169..178 (7 ready, 3 hold) |
| CAX.2 | Batch 1 (native DLLs): WI-169 ranged-ai controlled gate, WI-170 companion-ai F4, WI-171 Enlightenment 180 | CAX.1 | - | TODO | | plan + build + host tests |
| CAX.3 | Batch 2 (party scripts): WI-172 Defensive sticky target, WI-173 buff picker, WI-174 O3 series, WI-175 Drain Force droids | CAX.2 | - | TODO | | |
| CAX.4 | Hold items: WI-176 (O3 reject fallback), WI-177 (combo 29), WI-178 (intruder-first) | - | owner decisions | TODO | | ask user |

## fid -> WI map
| WI | fid | state |
|---|---|---|
| WI-169 | #1 | ready |
| WI-170 | #4 | ready |
| WI-171 | #2 | ready |
| WI-172 | #9 (defense + debilitated) | ready |
| WI-173 | #11 | ready |
| WI-174 | #3 | ready |
| WI-175 | #13 | ready |
| WI-176 | #6 | hold |
| WI-177 | #12-L1e | hold |
| WI-178 | #9b (intruder-first) | hold |
| play CA-T10 | #10 | pending |
