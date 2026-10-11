# PREREG: Agy (Gemini 3.8 Flash) vs Haiku, blinded second-opinion pilot

**Status: DRAFT v1 (10-10).** NOT final. Finalise after P2 (live probes) and before ANY pilot call. Pending: P2 findings (agy output shape, quota signature, cold-call cost), runner hash, your G1 answer and your "go".

Repo HEAD: `78807d4`. Input hashes (sha256, first 12):

| Input | Hash |
|---|---|
| prompts/PROMPT_verify.txt | e1562c15ca7f |
| prompts/PROMPT_fix.txt | ee50fd868fb8 |
| review/verify/verify_schema.json | 751fe36c8fe9 |
| review/fix/review_schema.json | 1d47f89291cd |
| make_dossiers.py | e2c3c46cec37 |
| frozen/MANIFEST.json | 7be247354fd9 |
| frozen/KEY.json (blinding key; not opened until scoring) | 6addaa3b492e |
| agy_gate.sh | fcc944749d20 |
| test_agy_gate.sh | 5980544e44fa |
| stub_agy/agy | 90f5dca6dd10 |
| review/work/verify_key.json | 09cfa126a48e |

## 1. Task (identical for both models)

- Self-contained prompt file per item: instructions + claim + file + line range + numbered excerpt (+/-15 lines). Fix items add the patch text. No tree access, no tools.
- Verify items: `verdict` in {reproduced, refuted, inconclusive, intended-behaviour}.
- Fix items: `verdict` in {accept, revise, reject}, and `per_finding.defect_fixed` in {yes, partial, no}.
- Frozen prompts say `UNSCORED` for every id (no harness ids are supplied). Ids are not scored.

## 2. Sets (frozen by make_dossiers.py, deterministic)

- Verify gold: 15 (8 decoys, 7 controls).
- Fix gold: 8 (5 seeded-bad, 3 known-good).
- Real sample: 30 of the 260 real verify findings, `random.Random(20261010).sample(sorted(real_ids), 30)`.
- Frozen prompt counts: `frozen/verify` 15, `frozen/fix` 8, `frozen/real` 30.

## 3. Calls and argv

- **Agy:** via `agy_gate.sh`, one prompt file per call, neutral cwd `~/.cache/agy_eval/cwd` (must be empty). The argv is fixed by the gate: `-p <prompt> --model <id> --mode plan --sandbox --output-format json --json-schema <schema> --print-timeout 120`. Never `--dangerously-skip-permissions` or `--add-dir`. Model id: confirmed in P2 (the pilot uses only the confirmed id).
  - Gold items run twice (23 x 2 = 46 calls). Real sample once (30 calls). **Total 76, cap 80.**
- **Haiku:** `claude -p <prompt> --model claude-haiku-5-5 --tools "" --no-session-persistence --disable-slash-commands --output-format json --json-schema <schema> --max-budget-usd 0.005`, neutral cwd. Mirrors the bundle's verify flags (run_verify.sh:92-97) minus the agentic tools.
  - Gold once (23) + real sample once (30). **Total 53, sequential.**
  - **Cap: $0.05 CLI-reported total.** Guard before each call: stop if `spent + 0.005 > 0.05`. Router-eval Haiku calls cost $0.0008-0.0011 each, so 53 calls is about $0.04-0.06. The cap may cut the last real items. Truncation is recorded in the results, not hidden.
  - Order: gold first, then real.
- **Confound, recorded not removed:** both models load their own global rules (Haiku: `~/.claude/CLAUDE.md`; agy: whatever P2 shows loads in print mode). The real swarm runs with these rules too, so this matches production.

## 4. Metrics (descriptive; Wilson 95% CI on each rate)

- Decoy false-confirm: verdict = reproduced, k/8.
- Decoy caught: verdict in {refuted, intended-behaviour}; inconclusive is reported separately.
- Control miss: verdict in {refuted, intended-behaviour}, k/7.
- Bad-patch approved: verdict = accept, k/5.
- Good-patch rejected: verdict in {revise, reject}, k/3.
- Schema-valid rate: gate class `ok` or the raw call returned a schema-valid object. Gold first runs (23) and all 76 calls reported separately.
- Quote-check pass rate: `quote_check.py` on verify outputs (supporting and refutation quotes).
- Latency p50 / p95; calls and tokens per catch.
- Agy repeatability: verdict agreement between the two runs of each gold item (k/23).
- Secondary: the existing agentic Haiku verdicts for the gold items (already in repo; not like-for-like).

## 5. Disagreement catalogue

- Every item where agy's verdict class differs from Haiku's (reproduced vs not, accept vs not).
- Blind review pack: A/B labels shuffled per item; the key is written to a separate file and not opened until verdicts are recorded.
- Each agy-only catch is classified real / spurious / unclear.

## 6. Decision rule (frozen)

- **NO-GO** if any of:
  - G1 fails (terms or data egress not confirmed by you);
  - G2 or G3 fails (see the plan);
  - agy schema-valid on the 23 gold first runs is fewer than 21/23;
  - agy false-confirms at least 2 more decoys than Haiku.
- **GO-SHADOW** if no disqualifier: next plan wires `SWARM_AGY=shadow` into the flows.
- **There is no "go-on" from this pilot.** n is too small. Promotion to `on` needs a new pre-registered rule based on shadow data.

## 7. Stop rules

- First `quota` class result: stop all agy calls. No re-runs. Record the verbatim error.
- First `auth` or `config` class: stop all agy calls.
- `rate_limit` or `transient`: gate backoff only; no manual re-runs of an item.
- No prompt tuning after the first pilot call. Every run checks the prompt hashes above before starting.

## 8. Decisions and open items

- **G1 (terms + egress): PASS (user, 10-10).** Egress approved for the frozen prompts only (claim plus about 30 lines of code; no secrets; no tree access). Terms: no explicit ban on scripted print-mode use found; vendor headless docs endorse it.
- **Personal credits: OFF (user, 10-10).** Subscription quota only. If quota runs out, calls fail; nothing is charged.
- **Weekly quota rate: UNKNOWN.** User will check the meter on return. P2 does not start until the user reports the rate.
- **Haiku cap: $0.05 (default; user did not answer).** Change only on instruction.
- Still open: P2 findings (agy output shape, quota or auth error verbatim, cold-call cost, model id confirmed); pilot runner `agy_eval_pilot_run.py` (not yet written; hash to record here); your "go" with the real call counts.
- P2 runner `agy_probe.py` written and stub-tested (12-call cap, failure halt, `--ack`, canary cleanup). Not yet run against the real agy.
