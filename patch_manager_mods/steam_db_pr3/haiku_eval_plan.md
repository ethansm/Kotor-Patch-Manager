# Plan: evaluating Steam address DB entries with Haiku

Status: DRAFT, 2026-10-02. Not started. Depends on the Sonnet Phase 2/3 run (see README.md).

## Goal

Verify Steam addresses for GOG address-DB entries at scale (the ACC scope first, later
"fill in as we go" batches, eventually up to GOG's 10,425 functions / 778 offsets) at the
lowest token cost that still keeps wrong addresses out of the runtime DB.

A wrong row is worse than a missing one: a missing name makes a GameAPI class skip
initialization (the command does nothing), but a wrong one calls into unrelated code.
Every design choice below trades tokens for precision, never the other way around.

## Principle: scripts do the analysis, models only judge

Most of the work is deterministic and costs no tokens. Models only see a small,
pre-digested evidence card and answer a narrow question.

| Tier | Who | Job | Expected share of rows |
| --- | --- | --- | --- |
| 0 | Python + one Ghidra headless export | Compute signals, auto-reject contradictions, build cards | all |
| 1 | Haiku | Judge one card: match / mismatch / unsure | rows not decided by tier 0 |
| 2 | Sonnet | Rows Haiku marks unsure, disagreements, no-candidate rows | ~10-20% |
| 3 | Opus | Calibration review, sample audit, final gate, commit | sample only |

## Tier 0: signals (no model)

One batched Ghidra headless run per wave exports, for every candidate address, to disk:
size, `ret N`, ECX use before write, decompile (truncated), callees, string refs, data refs.
Scripted checks per row:

- `ret N` vs GOG `param_size_bytes`; ECX-as-this vs GOG `calling_convention`
- Argument count vs the GameAPI wrapper's `Fn` typedef (when one exists)
- Run order preserved and size within tolerance of the GOG address-delta estimate
- Callees that are already confirmed rows (call-graph agreement)

Hard contradictions are rejected here without a model. Rows where every signal
agrees still go to Haiku (content check), but with a "strong prior" flag.

## Tier 1: Haiku evidence cards

- One card per row, about 1-1.5k tokens: class::name, wrapper signature, GOG metadata,
  tier 0 signals, truncated Steam decompile, callees named where known, strings.
- Fixed instructions + rubric + few-shot examples sit in a cached prefix; batch 10-20
  cards per call so the prefix is paid once per batch.
- Output: strict JSON per row `{verdict, confidence, reason ≤ 20 words}`.
- Self-consistency for promotions: a `match` needs two independent Haiku passes (shuffled
  card order) to agree, or one pass plus a strong tier 0 prior. Any disagreement escalates.

## Calibration before scaling (required)

Per the calibrate-detectors rule: measure before trusting.

- Gold positives: decompile-confirmed rows (`name_map` high, `hooks_ported`, Phase 2 `verified`).
- Gold negatives: the same names paired with the neighbouring function (off-by-one), a
  same-`ret N` function in the same run, and a random function.
- Run Haiku on the gold set; target precision ≥ 99% on `match` before it may promote rows.
  If it misses, adjust the cards or rubric, or move the threshold up a tier.
- Re-run a small gold slice in every wave to catch drift.

## Waves

Order work so each wave enriches the next (confirmed callees make later cards stronger):

1. ACC scope leftovers after Sonnet Phase 2 (`likely` / `unresolved` rows).
2. Other upstream K2 patches that adopt GameAPI, and our own mods if we move them to DB lookups.
3. Whole classes, in GOG run order, as needed ("fill in as we go"). Full parity only if asked.

Each wave ends with: Opus spot-audit (random 5% of `match` rows + every escalation),
results to `verified.tsv` + knowledge DB `name_map`, commit, then a live check (below).

## Runtime check

Verified rows go into the live `addresses.db` first (backup of the 48-function file kept),
then the user exercises the patch that uses them (for example ACC commands). Rows that pass
both the static check and the play test go into the PR. Restore = copy the backup back.

## Execution and cost

- Mechanism: Agent tool with `model: haiku` per batch, or a Workflow if the user opts in
  (needed once batches run in parallel at scale).
- Rough budget for the full 10k-function case: ~12-15M input tokens at Haiku rates with
  caching, plus Sonnet for ~1-2k escalations. The ACC scope is ~2% of that. Measure actual
  tokens per row in wave 1 and re-estimate before wave 3.

## Open questions

- Can the Phase 2 Sonnet scripts produce the tier 0 export as-is, or do they need a card builder?
- Haiku via subagents vs a direct API script (cheaper, but needs an API key and runs outside
  the session's tooling).
- Offsets: is a Haiku pass needed at all, or are struct_fields + accessor reads enough?

## Pilot result (2026-10-02): ACC scope, 434 real rows + 40 decoys, single Haiku pass

Cards: `work/haiku/cards/` (self-name annotation removed, Sonnet verdicts hidden), key `work/haiku/answer_key.json`,
verdicts `work/haiku/out/`. Six Haiku agents, ~580k subagent tokens total.

- Decoys: Haiku said `match` on 13 of 40 (33% false accept). It mostly checks "ret_N fits the typedef", which
  neighbouring thunks often satisfy. **A single Haiku pass on these cards cannot promote rows.**
- Real rows: 82 flagged `mismatch`. Triage found 0 database errors:
  30 narrow-argument widths (x86 pushes 4-byte slots; GOG records 1/2/5/13/14), 15 known upstream typedef bugs,
  12 thiscall thunks that Ghidra prints without `__thiscall`, 7 spurious Ghidra library-signature labels,
  4 return-type-only, 12 other (same thunk/cdecl misreadings), 1 offset (read decimal `+ 100` as hex).
  All 54 flagged functions with a GOG-comparable size agree on ret_N; the 26 others differ only by widening,
  except ShowUpgradeScreen (pops 24) and ActionMenuSecurity (pops 8), whose GameAPI typedefs push 4: an
  upstream wrapper bug on GOG too (the address rows are right; calling either wrapper would corrupt the stack).

Changes for the next wave: put the ABI facts in the prompt (4-byte stack slots, Ghidra omits `__thiscall` on thunks,
library labels are unreliable, decompile offsets are decimal); require a content-specific reason (string, field,
callee) for `match`; keep decoys in every wave; use Haiku as a flagger feeding Sonnet, not as a confirmer, until a
re-run shows decoy false accepts near 0.
