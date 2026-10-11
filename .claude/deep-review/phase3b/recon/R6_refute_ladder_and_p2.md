# R6: refute ladder wiring and p2 profile (recon, read-only)

## Part 1: refute ladder status (unbuilt)
- Hard fail: `dr.py:1057-1058` returns "refute ladder not wired yet" when `[ladder].refute_ladder = true` (`dr_profile.py:45` default false).
- Refute runner: `engine/verify/run_refute_audit.sh:45` makes ONE call, `dr_call_with_retry "${MODEL_REFUTE:-$MODEL_SONNET}"`, with no style argument and no second session. Haiku can already be selected via `[models].refute` (`dr_profile.py` models block), but that is single-pass, not k=2.
- Ladder merge no-op: `engine/verify/ladder.py:439-440` returns "ladder: off" when `adjudicate_styles` is empty. The merge only covers adjudicate batches (`vc.ADJ`, `ladder.py:447`), not refutations.

Minimal changes for a Haiku k=2 refute ladder:
1. `dr.py:1057-1058`: drop the hard fail and route `refute-audit` to a ladder runner when `refute_ladder` is true.
2. `engine/verify/run_refute_audit.sh:45`: loop over k=2 Haiku sessions (`MODEL_REFUTE=claude-haiku-5-5`), writing to `refute_raw/<style>/` with a per-style record.
3. `engine/verify/refute_audit.py` merge: read both k=2 records; on disagreement escalate to Sonnet or mark needs-human.
4. `engine/verify/ladder.py:437-447`: add a refute branch, or a separate `merge_refute()`, so the ladder merge is not adjudicate-only.
5. `dr_profile.py:85`: keep `refute_ladder` validation; add a template `templates/refute_style_*.md` if the prompt differs.

## Part 2: p2 profile deltas vs pilot (`.claude/deep-review/pilot/profile.toml`)
- `[run]`: `base = "22b56b4"` (was `902b770`); `head = "HEAD"` (same); `include_dirty = true` (was `false`); `slug = "kotor2-p2"` (was `kotor2-pilot`). `repo` stays `"../../.."`.
- `[protected]`: `files = []` (was `[".claude/master_ledger.jsonl"]`). `forbidden_commands` stays. With no protected files, `dr_finish` (`runlib.sh:109-111`) cannot abort on ledger drift.
- Keep `[exclude] path_suffixes = ["tests/static/raw_addresses.toml"]` and `path_parts = ["sunshine/credentials/"]`.
- Optional hardening: add `name_globs = [".env*", "*.pem"]` to cover `.envrc`, which the built-ins miss.

## Exclusion guarantees (from `dr_profile.py:17-18, 213-238`; `prep.py:127, 219, 228, 274`)
- `tests/static/raw_addresses.toml`: NOT built-in. Guaranteed only by the `[exclude].path_suffixes` entry above. Keep it.
- `.env`: built-in `.env`, `.env.*`, `*.env` (basename, any depth, case-insensitive). Guaranteed for `.env` and `.env.local`. `.envrc` not covered.
- `*.pem`: built-in name glob. Guaranteed.
- `*credential*`, `*secret*`: built-in name globs matched per path part, so directories also match. Guaranteed.
- Enforcement point: `is_excluded` at `prep.py:219, 228, 274` for tree build. Selftest asserts at `dr_profile.py:252-255`; not executed here.

## Facts not verified
- Did not run dr.py, the selftest, or any stage. Did not open secret-named files.
- The 205 dirty entries are not checked against `is_excluded` at the packet stage (`prep.py:516, 850`).
