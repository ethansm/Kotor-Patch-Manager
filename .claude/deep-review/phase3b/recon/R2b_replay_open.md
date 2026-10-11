# R2b replay open items (recon, read-only)

1. Verdict JSON key: `output.items` (not top-level `items`). Verdict file top level has `output` plus run metadata (`status`, `cost_usd`, `schema_valid`, ...). Item example from `verify/adjudications/batch_01.json`: `{"finding_id":"257c4b579141","final_verdict":"refuted","final_severity":"none","agrees_with_haiku":true,"reasoning":"...","evidence_quotes":[{"file","line","text"}],"human_check_steps":[...]}`.
   `verify/adjudications/batch_NN.check.json` is separate: `{batch, items:{<finding_id>:{quotes_ok,quotes_total,quote_ok_ratio,fabricated,trivial}}, fabricated, quotes_ok, quotes_total, quote_ok_ratio}`.

2. `dr.py init <profile.toml>` takes the profile path positionally (no `--profile` needed; it is remembered in `$DR_HOME/active.json`). Run id via `--run rl1` or `DR_RUN`. Run dir = `P["home"]/<run>`, so `~/.cache/deep_haiku_review/kotor2-pilot/rl1/` (matches existing `p1`, `canary_p1`). Init refuses a run dir inside the repo. An existing run resumes unless `--force`.

3. `run_adjudicate.sh` gates need, under the run dir: `canary.json` (recorded pass), `worker_settings.json` (or `$DR_SETTINGS`), `tree/` (sanitized worker tree from `prep build`, must match `snapshot.json`), `work/adjudicate/batch_NN.{md,json}`. `quote_check.py` checks evidence quotes against `RUN/tree`, not `verify/results`. So the proposed step 3 (copy `verify/results`) is unnecessary. Ground truth (`work/verify_key.json`) is not consumed by these runners.

4. Ids: each `work/adjudicate/batch_NN.json` is `{batch, items:[<finding_id strings>]}`. Counts: 01=11, 03=11, 07=11, 08=11, 09=10, 10=11, 14=11, 26=11 (total 87). Ground truth: decoy e0a8aa228b90 in batch_10; K2 279efdfe2ece in batch_03; K3a 257c4b579141 in batch_01; rejected 7bc29377aa57 in batch_08.
