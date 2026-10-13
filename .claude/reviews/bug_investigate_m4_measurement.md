# bug-investigate M4 measurement (pilot scale), 2026-10-12

Scope: swarm M4 measurement for the `bug-investigate` recipe. Pilot scale only: two known-answer cases, 5 repeat runs each. It does NOT decide D3 ("layer 3 off"). Every label below is a **rubric label**, not truth.

## Setup
- Cases: `hotbar_charkey.json` (log `force_power_hotbar_log.txt@1`, mod action-hotbar) and `queue_reinject_bg09.json` (log `queue-hud_log.txt@1`, mod queue-hud). Both carry a `score_rubric`; only `scripts/bug_investigate_score.py` reads it.
- Runs (all in `~/.cache/bug_investigate/`): hotbar `20261012T040049Z`, `T040138Z`, `T040224Z`, `T040314Z`, `T040409Z`; BG09 `T040703Z`, `T040814Z`, `T040930Z`, `T041037Z`, `T041135Z` (all suffix `-logic`). Per run: blind x3 (advocate, evidence, skeptic) and null x1, all Haiku 5.5, `--parallel 2`.
- Config A = blind + quote check (with the new short-id resolver). Config B = A + Sonnet 5.5 verify layer (no tools, can only reject).
- Frozen from the end of Phase A to the end of Phase B (re-checked before every paid stage, `sha256sum -c` clean at the end), sha256 prefixes: blind role.json `896c82b8d2ba`, schema `81f202c779d3`, system `97e0714dd666`, styles advocate `8f46b4b007ca` / evidence `2d294a25e006` / skeptic `9dd04349e1f0`; null role `5db391e69c71`, schema `f67901c27424`, system `a33f2841370e`; verify role `7deec8b004fc`, schema `eb59507b7c60`, system `7c35cddbb846`; `scripts/playtest_bundle.py` `f84e884676dd`.
- Code landed: resolver `ac551e2` (`evidence_quote_check.py`), scorer `63ecdb6`, cases `e4a188c`, rubric fixes `fc03108` (hotbar) and `58b1fda` (BG09).

## Results, config A (quote-valid hypotheses, Wilson 95%)
| case | n | HIT | PARTIAL | MISS | precision_hit | hit_or_partial |
|---|---|---|---|---|---|---|
| hotbar (5 runs) | 47 | 13 | 17 | 17 | 0.277 [0.169, 0.418] | 0.638 [0.495, 0.760] |
| BG09 (5 runs) | 32 | 2 | 16 | 14 | 0.063 [0.017, 0.201] | 0.563 [0.393, 0.718] |

- Rank-1 (top-ranked hypothesis is a HIT): hotbar 5 of 5 runs; BG09 0 of 5 (labels MISS, PARTIAL, PARTIAL, MISS, PARTIAL).
- Null unit verdict: `insufficient` in 10 of 10 runs (it never said "not a bug", never claimed a cause).
- Quote-valid rate overall: hotbar 47/49, BG09 32/41.
- Advocate: 27 of 28 hypotheses quote-valid across the 10 runs, versus 0 of 3 in the pilot. In the 10 runs the advocate wrote **0 short ids** and 220 full-id mentions, so the truncation did not recur. The resolver was therefore never exercised live; the $0 re-check of the pilot (copy) showed it would have rescued all 3 advocate hypotheses (HIT, PARTIAL, HIT). It is a harmless safety net, not a measured gain.

## Results, config B (Sonnet verify layer = "layer 3")
- The per-run cap ($0.10) stopped each run after 2 to 3 verify units, because Sonnet costs $0.014 to $0.040 per unit (estimate was $0.015 to $0.025) and the runner reserves the per-call cap. Remaining units stayed pending as the gate rule says. Coverage: **28 of 79 quote-valid hypotheses (35%) were verified**, skewed to the advocate unit (22 of 28), not random.
- Verdicts on those 28: `supported` 4 (all HIT), `inconclusive` 22, `rejected` 2 (both MISS, both in BG09). Hotbar: 4 supported-HIT, 3 inconclusive-HIT, 5 inconclusive-PARTIAL, 1 inconclusive-MISS. BG09: 2 rejected-MISS, 6 inconclusive-MISS, 6 inconclusive-PARTIAL, 1 inconclusive-HIT.
- Effect on precision: hotbar B = A (0 rejections); BG09 B n=30, precision_hit 0.067 [0.018, 0.213] vs 0.063. False rejections of a HIT: 0. Per-run rank-1 changed once (BG09 run 4, top MISS rejected so a HIT became rank 1).
- Cost: **$0.604 for 28 units (about $0.022 each) versus about $0.002 per Haiku unit**, roughly 10x, for 2 rubric-label MISS rejections among the 28 verified units (not 2 of 79: 51 units were never verified; "rubric label", not ground truth; corrected 2026-10-13, original wording kept in git). Verify said `inconclusive` for 22 of 28, i.e. it mostly abstains.

## Cost
Total $0.704 of the $1.90 ceilings: hotbar Haiku $0.041, BG09 Haiku $0.058, Sonnet verify $0.604 (pilot $0.0135 earlier, not included). All costs known (`cost_complete: true`); no process left running.

## Rubric trust (hand checks, blind to the scorer)
- Hotbar: first sample 6 of 8 (rubric under-credited "no valid key"); after the fix the same sample was 7 of 8; a fresh sample was 7 of 8 until a false-positive pattern ("not re-resolved") was dropped, then 8 of 8 (in-sample: the rubric was tuned on these ids; the held-out figure was 7 of 8, see the 2026-10-13 review and the Wave D relabel). Pilot calibration still reproduces exactly (2 HIT, 3 PARTIAL, 2 MISS, same ids).
- BG09: first sample 5 of 8 (too lenient: any "again" matched); after tightening the same sample was 8 of 8 and a fresh sample 7 of 8.
- Caveat: the rubrics were tuned after seeing the samples, and the hand labeller is the same agent that wrote the rubric. The remaining disagreements are the rubric being stricter (HIT judged by hand, PARTIAL by rubric). Labels are comparable across configs and runs, not an absolute truth measure.

## What the data can and cannot say
- Can say: with this evidence, the blind Haiku roles put an identifiable partial or full mechanism in about 56% to 64% of their supported hypotheses; the hotbar case gets a HIT at rank 1 every time; the BG09 case almost never gets the full mechanism (the follower condition is absent from the excerpts, so a HIT was expected to be hard); the null unit abstains (10 of 10 `insufficient`: saturated, there was no not-a-bug control, so this shows nothing about the null unit's ability to say not_a_bug).
- Cannot say: anything about layer 3 beyond "on 28 non-random units it removed 2 of those 28 (not 2 of 79; 51 never ran) hypotheses, both rubric-MISS, and kept every HIT it saw". n is small, hypotheses from one run share evidence (intervals are optimistic), only two cases, and the verified subset is advocate-heavy.
- Additional known-answer cases for a D3 decision: the master plan wants about 155 paired labelled units at assumed discordance 0.20. Observed discordance between A and B is 2 of 28 verified units (0.07), so the effect to detect is small and the test would need more pairs, not fewer. At about 20 to 40 hypotheses per case and a safety margin for within-case correlation, plan for **at least 8 to 10 further known-answer cases** (an estimate, not computed from a power formula), plus a verify cap large enough to cover every unit (about $0.022 per unit, about $0.9 per case at 40 units).

## Go / no-go for M5
- Go for M5 recipes on the Haiku blind layers: stable (10 of 10 runs completed, quote-valid rate 79/90 = 88% overall), cheap ($0.008 to $0.012 per run), null abstains (10 of 10 `insufficient`; saturation, no control case).
- D3 stays **"layer 3 off"**: the data are not overwhelming (2 rubric-MISS rejections of 28 verified, 22 abstentions at 10x unit cost, 35% coverage) and more cases do not exist yet.
- Open defects: (1) the verify stage's per-call cap reservation (`--cap-call 0.05` with a $0.10 stage cap) leaves most units pending; raise the stage cap or lower the per-call cap before using verify at all. (2) Ground-truth cases are scarce (the hotbar log is a legacy pre-fix log with one party member; BG09's follower condition is not in the bundle).

## Design only: not-a-bug control bundles for the null unit (R5.3, 2026-10-13; building needs its own go)
- Why: the null unit answered `insufficient` in 10 of 10 runs, so its ability to say `not_a_bug` is untested. Both cases have a real bug in the logs.
- Build 1 to 2 controls: bundles from a log slice of a build or session where the symptom does NOT occur (for the action hotbar: a post-fix log with 2+ party members and no "Character not identified yet"; for BG09: a session without the follower re-queue). Same symptom text as the real case, same scrubber, same excerpt caps.
- Expected answers: null `not_a_bug` or `insufficient` with cited reasons; blind roles should end with low confidence or empty `hypotheses`. A control where blind roles produce HIT-labelled hypotheses measures the false-positive rate of the rubric and of the pipeline.
- Cost: about $0.01 per control run (3 blind + 1 null Haiku units, M4 numbers); needs a log slice chosen by a human, then `bundle --from`.

## Addendum 2026-10-13: held-out relabel and matcher v2 (remediation plan Wave D)
Setup: 38 held-out quote-valid hypotheses (hotbar 20, BG09 18; seed 7, the 29 tuned-on ids excluded; ids committed in `37bfbd1` BEFORE labelling, `heldout_ids.json`). Two Haiku labellers per case saw only the symptom, the true root cause, a plain HIT/PARTIAL/MISS definition, and the claim and mechanism text (ids shuffled and renamed). They never saw the rubric or the scorer. Opus adjudicated the 5 disagreements from the same text. Rubrics frozen; nothing edited after labelling. Files: `.claude/reviews/bug_investigate_m4_review/heldout_labels/`.

| case | labeller kappa (raw) | consensus HIT/PARTIAL/MISS | scorer v1 vs consensus: raw, kappa | scorer v2 vs consensus: raw, kappa |
|---|---|---|---|---|
| hotbar (n=20) | 0.612 (0.80) | 0 / 10 / 10 | 0.700, 0.500 | 0.700, 0.500 |
| BG09 (n=18) | 0.906 (0.944) | 10 / 4 / 4 | 0.111, -0.200 | 0.167, -0.098 |

Confusion rows = consensus, columns = scorer (v2): hotbar PARTIAL row HIT 4 / PARTIAL 6 / MISS 0, MISS row 0 / 2 / 8. BG09 HIT row 0 / 4 / 6, PARTIAL row 1 / 2 / 1, MISS row 0 / 3 / 1.

Reading (as measured, no rubric edit): kappa(consensus, v2) is below 0.6 in both cases, so **the rubrics are not trusted for HIT claims**. The two failure directions differ:
- Hotbar: the scorer credits 4 hypotheses HIT that the labellers call PARTIAL (the consensus found no HIT at all: none names the party-size gate in `characterKeyFor`). The 0.277 hotbar HIT rate is therefore probably inflated.
- BG09: the labellers call 10 of 18 HIT, the scorer 0 of those 10 (4 PARTIAL, 6 MISS under v2). Opus read two of the 6 (`...040703Z-logic/blind-advocate:h1`, `.../blind-skeptic:h2`): both describe a re-injection after an `eaten_paused`, `exec=0` vanish and never name the follower entry or a missing "saw executing" signal, so Opus would call them PARTIAL, i.e. the Haiku labellers look lenient here and the rubric may be nearer the truth than the kappa suggests. Same model family on both labeller seats, so this is not independent; a human spot-check of 8 labels is the independent check (offered, not done).

Matcher v2 versus v1 on the 10 M4 runs (copies, original `score.json` untouched; `--matcher v1` reproduces the stored files exactly apart from the new keys `matcher`, `rubric_sha256`, `case_id`, `top_matches_report`, `rank1_alt_label`):

| case | config | v1 HIT / n (precision_hit) | v2 HIT / n (precision_hit) |
|---|---|---|---|
| hotbar | A and B | 13 / 47 (0.277) | 11 / 47 (0.234) |
| BG09 | A | 2 / 32 (0.062) | 1 / 32 (0.031) |
| BG09 | B | 2 / 30 (0.067) | 1 / 30 (0.033) |

Other: rank-1 mismatch runs 0; tie-break decides the rank-1 label in 1 run per case. The cluster-bootstrap interval over 5 runs is not wider than the pooled Wilson one here (hotbar v2 A: cluster [0.174, 0.292], Wilson [0.136, 0.372]); with only 5 runs per case the bootstrap is itself unstable, so treat both as optimistic.
