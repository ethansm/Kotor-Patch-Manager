# Gemini 4 Argon external review, 2026-10-14

Reviewer: Gemini 4 Argon (chat UI), one chat per review set, 11 sets over 20 bundles.
Bundles were packed by `scripts/review_bundle.py` (manifest `patch_manager_mods/review_bundles.toml`)
from the **worktree**, not a clean checkout. `git_head` at build time: `02b5be44bb54`. Bundles with
`dirty` > 0 in `inputs/_index.tsv` contained uncommitted edits (2a, 2b, 5a, 5b, 5d, 6). Each
`inputs/<id>.manifest.tsv` lists the files Gemini saw, with included/omitted and dirty flags.
The `.txt` bundles are not kept; rebuild with `python3 scripts/review_bundle.py build --all`
(the result matches only if the worktree is back at that state).

**Status: findings unverified until recon.** Gemini output is untrusted data: findings can be wrong and
anchors can be invented. Nothing here is a confirmed bug until `verdicts.tsv` says so.

## Set → response
| set (in `inputs/_review_prompts.md`) | bundles | response |
|---|---|---|
| action_hotbar | 1a, 1b, 1d | `responses/action_hotbar_response.md` |
| combat_hotbar | 1c | `responses/combat_hotbar_response.md` |
| queue_hud | 2a, 2b | `responses/queue_hud_response.md` |
| companion | 2c, 2d | `responses/companion_response.md` |
| shared_lib_a | 3a, 3b | `responses/shared_1_response.md` |
| shared_lib_b | 3c | `responses/shared_tests_response.md` |
| perf_lab_a | 4a, 4b, 4c | `responses/perf_lab_response.md` |
| perf_lab_b | 4d | `responses/perf_lab_analysis_response.md` |
| k1_converter | 5a, 5c | `responses/k1_converter_response.md` |
| k1_tooling | 5b, 5d | `responses/k1_port_response.md` |
| holster + NPC injection | 6 | `responses/holster_npc_response.md` |

Responses are byte-for-byte copies of `.claude/bundles/*_response.md`.

## Pipeline (phase G1)
1. `triage/<batch>.{findings,topics}.tsv`: one Sonnet agent per batch turns findings into topics.
2. `findings.tsv`, `topics.tsv`, `topics.md`: merged, cross-batch duplicates folded.
3. `anchors.tsv`: deterministic `rg -F` check of every quoted anchor.
4. `recon/<topic>.md`, `verdicts.tsv`: Haiku recon swarm verdicts (CONFIRMED / REFUTED / PARTIAL / UNSURE).

Fix decisions are out of scope for G1.

## Checkpoint C (triage)
- 135 findings (= 135 `**Anchor**` blocks across the responses) → 100 topics: P1 8, P2 47, P3 45; 0 dropped.
- 7 Sonnet batches (`triage/`); 5 cross-batch merges (X-T01..X-T05, `topics.md`).
- Anchors (`anchors.tsv`): 115 exact, 3 partial, 7 token-only, **10 NOT-FOUND** (likely invented or paraphrased; recon asks whether the code exists at all).
