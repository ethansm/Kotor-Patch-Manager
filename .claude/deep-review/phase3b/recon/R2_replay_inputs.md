# R2 replay inputs (recon, plan-mode write)

## Inputs found
- Original dossiers: `.claude/review/work/adjudicate/batch_NN.{md,json}` (59 files, about 29 batches). `.md` = dossier, `.json` = manifest `{batch, items}`.
- Sonnet verdicts (ground truth source): `.claude/review/verify/adjudications/batch_NN.json` (+ `.check.json`). Schema key not yet confirmed.
- Single-Haiku A/B outputs: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/119b7354-18a5-4d2f-baeb-679c9d7c173a/scratchpad/ab/` holds batch_01,03,07,08,09,10,14,26 as `.json` (`{batch,cost,rc,sec,output,subtype}`), `.out`, `.err`. `ab2/` has 9 files (partial second run).
- `ab_run.py` exists at `.../scratchpad/ab_run.py`. CLI: `ab_run.py <OUT_DIR> <CAP_USD> <comma-batch-list>`. Reads `.claude/review/work/adjudicate/<b>.md`, calls `claude -p --model claude-haiku-5-5` with `worker_settings.json` and `adjudicate_schema.json`, 4 parallel, cap per batch 0.12.
- Bundle: `/home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/verify/`.
  - `run_adjudicate.sh`: `--style NAME` sets `DR_ADJ_DIR=$VERIFY/adjudications_l2/$STYLE` (line ~51); pending via `ladder.py pending l2 --style` (line ~62); input `WORK/adjudicate/batch_NN.{md,json}` (line ~10).
  - `ladder.py` docstring lines 5-30: L2 record path `VERIFY/adjudications_l2/<style>/batch_NN.json`; final `adjudications/batch_NN.json` gets `produced_by:"ladder"`.
  - `build_dossiers.py` lines 6, 229, 237: `--keep-existing` skips the rebuild if adjudications already exist. Without `--force` it refuses to overwrite.

## Proposal: scratch run `rl1` (10 steps)
1. Run `dr.py init` for run id `rl1` under `~/.cache/deep_haiku_review/kotor2-pilot/` (verify flags first). Never touch `p1` or `locks/`.
2. Copy the 8 dossier pairs `batch_{01,03,07,08,09,10,14,26}.{md,json}` into `rl1/work/adjudicate/`.
3. Keep the original `verify/results/` for those findings in `rl1/verify/results/` so quote checks resolve.
4. Do not copy `work/verify_key.json` into the run tree. Keep ground truth in a scratch scoring file only.
5. Confirm pickup with `DR_RUN=rl1 run_adjudicate.sh --dry-run --style S` (expect 8 pending).
6. Run `DR_RUN=rl1 run_adjudicate.sh --style S` for each style; records land in `rl1/verify/adjudications_l2/S/`.
7. `ladder.py --run rl1 pending l2 --style S` should return 0 after the run.
8. Run `run_compare.sh` then `run_escalate.sh` with `DR_RUN=rl1`.
9. Score final verdicts against decoys/K2/K3a/7bc2 ground truth in the scratch file.
10. Only use `build_dossiers.py --keep-existing` on a rebuild. A first build needs no flag.

## Open
- Confirm the Sonnet verdict JSON key (`output.items` vs `items`) and whether `dr.py init` needs `--profile`.
