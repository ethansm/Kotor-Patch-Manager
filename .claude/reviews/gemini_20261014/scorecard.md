# Gemini 4 Argon review 2026-10-14: scorecard (G2)

real = defect or gap confirmed reachable today; latent = true in code but unreachable/benign today; refuted = false. Severity of real rows: low 48, med 13; Gemini claimed: high 39, medium 64, - 2, low 30.

## By set

| set | findings | real | latent | refuted | unsure | anchor hit | precision (real) | precision (real+latent) |
|---|---|---|---|---|---|---|---|---|
| action_hotbar | 14 | 5 | 8 | 1 | 0 | 71% | 36% | 93% |
| combat_hotbar | 7 | 3 | 3 | 1 | 0 | 100% | 43% | 86% |
| companion | 15 | 6 | 5 | 3 | 1 | 100% | 40% | 73% |
| holster + NPC injection | 13 | 2 | 5 | 5 | 1 | 85% | 15% | 54% |
| k1_converter | 13 | 7 | 4 | 2 | 0 | 100% | 54% | 85% |
| k1_tooling | 13 | 6 | 5 | 2 | 0 | 100% | 46% | 85% |
| perf_lab_a | 7 | 2 | 4 | 1 | 0 | 86% | 29% | 86% |
| perf_lab_b | 11 | 6 | 4 | 1 | 0 | 100% | 55% | 91% |
| queue_hud | 16 | 7 | 6 | 2 | 1 | 100% | 44% | 81% |
| shared_lib_a | 17 | 9 | 5 | 2 | 1 | 88% | 53% | 82% |
| shared_lib_b | 9 | 8 | 0 | 1 | 0 | 89% | 89% | 89% |
| **all** | 135 | 61 | 49 | 21 | 4 | 93% | 45% | 81% |

## By Gemini's stated confidence

| confidence | findings | real | latent | refuted | unsure | anchor hit | precision (real) | precision (real+latent) |
|---|---|---|---|---|---|---|---|---|
| - | 1 | 0 | 0 | 1 | 0 | 100% | 0% | 0% |
| certain | 118 | 59 | 41 | 14 | 4 | 94% | 50% | 85% |
| likely | 15 | 2 | 8 | 5 | 0 | 80% | 13% | 67% |
| unsure | 1 | 0 | 0 | 1 | 0 | 100% | 0% | 0% |

## By category (real rows)

| category | real | latent | refuted |
|---|---|---|---|
| logic | 18 | 22 | 9 |
| test-gap | 34 | 6 | 5 |
| memory-safety | 2 | 17 | 2 |
| tooling | 6 | 4 | 3 |
| hook/ABI | 0 | 0 | 2 |
| doc | 1 | 0 | 0 |

Claimed high/critical: 39; validated real at any severity: 12; validated high: 0.
