# Plan R2FX1: ingest Gemini round-2 findings into dev work items, then run the first variant fix wave

## Context
Gemini 4 Argon round 2 reviewed 5 mods (buff-hud, npc-diversity, npc-world spawn, queue-hud, companion-ai).
Validation landed in c360b15: 19 Haiku units, then one Sonnet consolidator per review file, $5.11.
Merged verdicts: `.claude/reviews/gemini_round2/validation/validated.tsv`. Columns: fid verdict real severity impact haiku_agree evidence why fix gate title.
The 53 rows are 52 findings plus SIDE-QH-TEST. By class: 1 reachable medium (R2-QH-02), 12 reachable low, 25 latent, 14 doc/test, 1 refuted (R2-BH-08).
Several Gemini fix diffs are wrong (BH-06, QH-01, QH-08, CA-03, BH-15). The `fix` column holds the corrected sketch, which wins over Gemini's diff.

## Phases (mark with `plan_manager.py mark-done <Id> --plan .claude/plans/gemini_r2_fix_wave1.md --no-handoff`)
| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| R2.0 | Validate round-2 findings (Haiku swarm + Sonnet consolidate) | - | - | DONE | c360b15 | validated.tsv |
| R2.1 | Ingest: create WIs from validated.tsv (reachable + SIDE-QH-TEST + test gaps ready; latent/doc grouped) | R2.0 | - | DONE | 36dd958 | WI-115..133 + WI-113 note; lint clean |
| R2.2 | Items TSV + `fix_wave.py plan` -> batch A | R2.1 | user approves batch A | DONE | a0de0ef | batch A planned + approved (run r2fx1-A) |
| R2.3 | Run batch A (Haiku variants -> Sonnet cons -> Sonnet rev) | R2.2 | - | DONE | | 12 Haiku, 10 cons, 10 rev; 0 REQUEST_CHANGES, 1 rebase |
| R2.4 | Apply in conflict order, close WIs, play-test rows | R2.3 | - | DONE | cdd2475 | 10/10 applied 823aea5..cdd2475; play-test rows a4e43d0 |
| R2.5 | Close: cost, memory yield line, handoff | R2.4 | - | DONE | | cost $6.83/32 agents (~$0.68/item); memory yield line |
| R2.6 | Wave 2 plan: items TSV (WI-125, WI-126) + `fix_wave.py plan --wave r2fx2` | R2.5 | user approves batch r2fx2-A | DONE | d64fa01 | batch r2fx2-A planned + approved |
| R2.7 | Wave 2 run: `/variant-fix-wave run r2fx2-A` (variants -> cons -> rev -> apply, close, play-test rows) | R2.6 | - | DONE | 01091d7 | 2/2 applied bba8463 (WI-126), 01091d7 (WI-125); 8 agents $2.32; play-test rows in item commits |

## fid -> WI map (R2.1, 2026-10-16)
| WI | fids | state | wave |
|---|---|---|---|
| WI-115 | R2-QH-02 | ready M | r2fx1 A (Q) |
| WI-116 | R2-QH-03 | ready | r2fx1 A (Q, after 115) |
| WI-117 | R2-QH-06, R2-QH-07 | ready | r2fx1 A (Q, after 116) |
| WI-118 | SIDE-QH-TEST, R2-QH-10 | ready | r2fx1 A (Q, after 115) |
| WI-119 | R2-ND-03, ND-04, ND-05 | ready | r2fx1 A (N) |
| WI-120 | R2-CA-01 | ready | r2fx1 A (C) |
| WI-121 | R2-CA-05 (+ WI-106/107 notes) | ready | r2fx1 A (C, after 120) |
| WI-122 | R2-NW-06, NW-07 | ready | r2fx1 A (free) |
| WI-123 | R2-BH-10, BH-11 | ready | r2fx1 A (free) |
| WI-124 | R2-ND-12, ND-13 | ready | r2fx1 A (N, after 119) |
| WI-125 | R2-CA-03 | ready M | wave 2 |
| WI-126 | R2-BH-03, BH-04 | ready | wave 2 |
| WI-127 | R2-ND-02 | hold | play-test decision |
| WI-128 | BH-01,02,05,06,07,09,13 | hold | latent |
| WI-129 | ND-07..11 | hold | latent |
| WI-130 | NW-01..05 | hold | latent |
| WI-131 | QH-01,04,05,08,09 | hold | latent |
| WI-132 | CA-02, CA-04 | hold | latent |
| WI-133 | BH-12,14,15, ND-06, ND-14, QH-11 | ready (docs) | /next-dev-work-item |
| WI-113 | R2-ND-01 | note appended | - |
R2-BH-08 refuted, not ingested. Batch A: `.claude/waves/r2fx1/items.input.tsv` -> plan 10 units / 32 agents / $13.18; play tests in `.claude/waves/r2fx1/playtests.tsv`.
