# Haiku-ladder replay harness ($0 until step 4)

Replays 8 adjudication batches (01,03,07,08,09,10,14,26 = 87 items) on a scratch run and scores them.
Run dir: `~/.cache/deep_haiku_review/kotor2-pilot/<run>/`. Never touches `p1/` (read-only source) or `locks/`.

1. Write `profile_rl1.toml` with `[ladder] adjudicate_styles = [..]` (orchestrator's job).
2. Preview: `python3 build_rl1.py --run rl1 --profile <profile_rl1.toml> --dry-run` (creates nothing).
3. Build: `python3 build_rl1.py --run rl1 --profile <profile_rl1.toml>` (dr.py init + copies p1 tree/snapshot/canary/
   settings + 8 dossier pairs; rewrites copied snapshot.json tree_path and head; restores global active.json).
   Never copies `verify_key.json` or stored verdicts. Idempotent; safe to re-run.
4. Paid stages (orchestrator only), each with `DR_RUN=rl1`, `--profile <profile_rl1.toml>`:
   `run_adjudicate.sh --dry-run --style S` (expect 8 pending), then `run_adjudicate.sh --style S` per style,
   `ladder.py merge`, `ladder.py compare-build`, `run_compare.sh`, `ladder.py merge`, `ladder.py escalate-build`,
   `run_escalate.sh`, `ladder.py merge` (scripts in `$B/engine/verify/`, `B=~/.claude/skills/deep-haiku-review/bundle`).
5. Score (read-only, any time; missing layers print n/a):
   `python3 score_replay.py --run rl1 [--run2 rl2]` -> <= 40-line markdown block.

Compared against: stored Sonnet `.claude/review/verify/adjudications/batch_NN.json` (`output.items`), old single-Haiku
`../replay_src/ab/batch_NN.json`, and ground-truth ids decoy e0a8aa228b90 (b10), K2 279efdfe2ece (b03),
K3a 257c4b579141 (b01), rejected 7bc29377aa57 (b08).
Final verdicts come from `verify/adjudications/` (`produced_by: ladder`) or `work/ladder/state.json`.
Gates run_adjudicate.sh needs: canary.json, worker_settings.json, tree/ == snapshot.json (+ HEAD pin), dossiers.
