# Review pipeline file formats (contract between prep.py, tools/, run_map.sh, merge.py)

All paths are relative to the repo root unless marked. Everything written by the pipeline lives
under `.claude/review/` (repo) or `$KOTOR2_REVIEW_HOME` (default `$HOME/.cache/kotor2_review/`).
Python is stdlib only (no jsonschema, no ctags on this machine). All JSON is UTF-8, sorted keys,
indent 1, trailing newline, so reruns are byte-identical (deterministic).

## Security rule for every script
Never open, `git show`, or diff a path matched by `prep.EXCLUDE_PATTERNS`
(`.env`, `*.pem`, `.credentials.json`, `sunshine/credentials/*`, `endpoint.env`,
`tests/static/raw_addresses.toml`, `*credential*`, `*secret*`, case-insensitive, any depth).
Tools only process hunks listed in `work/hunks.jsonl`, which prep.py has already filtered,
and they read post-image file content only from the sanitized worker tree (`--tree`).
They may call `git show <BASE>:<path>` (read-only) for a path that appears in hunks.jsonl.

## work/hunks.jsonl  (written by prep.py; one JSON object per line)
One record per COVERAGE UNIT. A coverage unit is one `git diff -U0` change block, or one
file-level pseudo-hunk (untracked file, pure rename, symlink, binary, mode-only change).
```
{"hunk_id": "h-<12 hex>",          # sha256(file|source|old_start,old_len|new_start,new_len)[:12]
 "file": "path/in/post-image",      # posix, repo-relative
 "old_path": "path/in/base"|null,   # differs from file on rename; null when added
 "source": "committed"|"dirty"|"both"|"untracked",
 "diff_base": "11e1284"|null,       # base ref for the diff (null for untracked)
 "diff_target": "HEAD"|"WORKTREE"|null,
 "kind": "diff"|"whole-file"|"rename"|"symlink"|"binary"|"mode",
 "status": "A"|"M"|"D"|"R"|"T",
 "old_start": int, "old_len": int, "new_start": int, "new_len": int,
 "minus": [str], "plus": [str],     # line text without the leading -/+ and without \n
 "symlink_target": str|null,        # for kind == symlink
 "class": "a"|"b"|"c"|"excluded",   # a=review, b=docs-out-of-scope, c=generated/vendored/binary
 "class_reason": str,
 "domain": "wp1".."wp7"|null,
 "commits": [short hashes in 11e1284..HEAD touching the file]}
```
Excluded paths get ONE record each with `kind:"binary"`-free shape: minus/plus empty,
`class:"excluded"`; their content is never read.

## work/classifier_verdicts.json  (tools/classify_hunks.py)
```
{"generated_by": "tools/classify_hunks.py", "base": "11e1284",
 "verdicts": [{"hunk_id": str, "file": str, "range": "-a,b +c,d", "rule": str,
               "pass": bool, "proof": {...}}]}
```
Rules (a hunk is mechanical only when `pass` is true; record failing attempts too, `pass:false`,
with the reason in proof, for the rules that were applicable):
- `whitespace`: minus and plus are equal after deleting all whitespace (joined).
- `comment-only`: every changed line is blank or a comment for the file type
  (C/C++: `//`, `/* */`, `*` continuation; Python/TOML/sh: `#`). Python docstrings do not count.
- `include-style`: every changed line is a `#include` line (C/C++), and the multiset of included
  basenames is identical on both sides (path style changed only). For a test file
  (`*_test.cpp`, `test_*.py`, `*_test.py`, or a file under `tools/parity/`) the rule also requires
  the file's total assertion-macro count to be unchanged between base and post-image
  (count with the same regex as tools/assertion_delta.py); proof records both counts.
- `pure-rename`: kind == rename with 100% similarity (no content hunks).
- `symlink-in-tree`: kind == symlink, the target resolves (relative to the link's directory)
  inside the repo to an existing regular file that is not excluded; proof = resolved path and
  sha256 of the target in the worker tree.
Everything else is not mechanical.

## work/scanner_candidates.json  (tools/scan_patterns.py)
```
{"candidates": [{"candidate_id": "S-<10 hex>", "scanner": str, "file": str, "lines": "a-b",
                 "hunk_id": str|null, "detail": str}]}
```
candidate_id = sha256(scanner|file|lines|detail)[:10]. Scanners (only over class-a records,
only over `plus` lines plus the post-image file where noted):
- `sentinel-range`: hex literals 0xFFFFFFF0..0xFFFFFFFF (any case, optional u/U suffix) and the
  identifiers isExile / kInvalid* / OBJECT_INVALID in added lines.
- `creature-lifecycle`: per post-image file, every line calling destroyCreature / allocCreature /
  createCreature / delete / free( ... in changed hunks; detail reports alloc vs destroy counts
  for the whole post-image file.
- `pointer-math`: added lines with reinterpret_cast, C-style pointer casts `(T*)`, `(uintptr_t)`,
  `*(T*)(x + 0x..)`, or `+ 0x[0-9a-f]+` offsets.
- `hook-detour`: for every `*.hooks.toml` record, each added `[[hooks]]`/`[[patch]]`-style block
  with name, address, original_bytes, exit addresses, params, exclude_from_restore.
- `export-no-definition`: every symbol in a changed `exports.def` (post-image) with no
  definition-looking match (`<name>(` at line start after a return type, or `extern "C"` block)
  in any `.cpp`/`.c`/`.h` of the same mod directory in the worker tree.

## work/assertion_delta.json  (tools/assertion_delta.py)
```
{"files": [{"file": str, "base_count": int, "new_count": int,
            "by_macro": {"MACRO": [base, new]},
            "removed": [{"old_line": int, "text": str,
                         "nearest_added": [{"new_line": int, "text": str}]}]}]}
```
Assertion regex (C/C++): `\b(KT_CHECK\w*|CHECK\w*|EXPECT_\w+|ASSERT_\w+|REQUIRE\w*|assert)\s*\(`;
Python: `\b(self\.assert\w+|assert\s|pytest\.raises)`. Definition lines (`#define CHECK`) are not
counted. nearest_added = up to 3 added assertion lines from the same or the closest -U0 hunk in
the same file (within 40 lines of the removal's mapped position).

## packets/<id>.json  (prep.py)
```
{"packet_id": "wp3-007", "domain": "wp3", "files": [str], "mods": [str],
 "hunks": [hunk_id], "parts": [{"file": str, "mode": "W"|"U12"|"whole"|"range",
                                "new_range": "a-b", "hunk_ids": [str]}],
 "candidates": [candidate_id], "est_bytes": int, "est_tokens": int,
 "content": "packets/<id>.txt"}
```
A coverage unit may appear in several packets (oversized hunk split into parts). It counts as
reviewed only when EVERY packet that lists it has a complete run.

## coverage_plan.json  (prep.py)
```
{"snapshot": "snapshot.json sha256", "units": [{"hunk_id", "file", "range", "planned":
  "review"|"mechanical"|"skipped"|"excluded", "packets": [id], "reason": str}]}
```

## findings/<packet>.json and runs/<packet>.json  (run_map.sh)
findings = the worker's structured output (worker_schema.json) plus `"_usage"` (the CLI's
usage block and total_cost_usd). runs = `{"packet_id", "files", "exit_code", "attempts",
"schema_valid": bool, "truncated": bool, "status": "ok"|"failed"|"rate_limited"|"invalid",
"usage": {...}, "cost_usd": float|null, "duration_s": float, "started": iso, "ended": iso}`.
A run is COMPLETE when status == ok, schema_valid, and truncated == false.

## Verify stage (verify/, tools/record_verify.py)  -- appended; see plans/master_review_audit_plan.md section 12
Data dir `$VERIFY_DIR` (default `.claude/review/verify`); work dir `$VERIFY_WORK` (default `.claude/review/work`,
or `$VERIFY_DIR/work` when VERIFY_DIR is overridden). `KOTOR2_VERIFY_OUT` moves verified_findings.* (default
`.claude/reviews`). All JSON as above (sorted keys, indent 1); no timestamps in merge outputs.

### work/verify_queue.json  (verify/select.py)
```
{"generated_by", "source_master_sha256", "params": {"max", "min_severity", "decoys"},
 "counts": {"items", "real_selected", "controls", "decoys", "by_specialty": {...}},
 "reduce_records": {"<id>": {"verification", "severity", "confidence", "reason"?, "cited_lines_checked"?}},
 "items": [{"id", "file", "lines", "category", "claim", "specialty"}]}     # the ONLY fields a verifier sees
```
specialty in hook-asm | test-weakening | gate-integrity | python-tooling | cpp-logic | build-config.
`claim` = Reduce-stage evidence + why_it_matters, with verdict/severity/confidence/verification stripped and
tokens that name an excluded path redacted. `reduce_records` is shown to the Sonnet adjudicator only.
### work/verify_key.json  (answer key; never rendered into a prompt, dossier or worker-visible path)
`{"controls": {"<id>": {"label", "role", "expected_haiku": [..], "expected_final": [..]}},
  "decoys": {"<id>": {"label": "dcy-N", "mode": "shift"|"pair", "original_id", "original_file", "original_lines",
             "decoy_file", "decoy_lines", "expected_haiku", "expected_final"}},
  "recall": {"K1": {"hit", "matched_ids", "queued_ids"}, ...}}`. Decoy ids are 12-hex like real ids (the label
is only in this file). The recall key TEXT exists only in merge.py.
### verify/results/<id>.json  (tools/record_verify.py verify)  and  <id>.check.json  (verify/quote_check.py)
result = `{"kind":"verify","finding_id","status":"ok"|"invalid"|"failed"|"rate_limited","schema_valid","schema_errors",
"warnings","exit_code","attempts","rate_limit_hits","result_subtype","budget_hit","usage","cost_usd","num_turns",
"tool_calls","duration_s","started","ended","prompt_bytes","turn_usage","turn_prompt_tokens","model","raw","output"}`
where `output` follows verify/verify_schema.json. COMPLETE iff status == ok and output.finding_id == id.
check = `{"finding_id","verdict","quotes_total","quotes_ok","quote_ok_ratio","fabricated":[{group,file,line,text,reason}],
"trivial":[...],"supporting":{total,ok},"refutation":{total,ok},"verdict_supported"}`.
### work/adjudicate/batch_NN.{md,json}, index.json  (verify/build_dossiers.py)
`batch_NN.json = {"batch","items":[ids]}`; `.md` = the dossier (claim, Reduce record, Haiku verification, quote check,
+-15 line excerpt). `index.json = {"params","batches":{batch:[ids]},"missing_results":[ids],"items_total"}`.
### verify/adjudications/batch_NN.json  (tools/record_verify.py adjudicate) and .check.json
Same envelope as a result with `"kind":"adjudicate","batch"`, `output` per verify/adjudicate_schema.json
(`final_verdict` confirmed|refuted|needs-human, `final_severity`, `agrees_with_haiku`, `haiku_quality`).
COMPLETE iff status == ok and every manifest item is answered exactly once.
### .claude/reviews/verified_findings.{json,md}  (verify/verify_merge.py)
`{"tiers": {"CONFIRMED","CONFIRMED-BY-SONNET-ONLY","CONTESTED-NEEDS-HUMAN","REFUTED"}, "counts", "calibration"
(decoys, controls, key_recall, haiku_vs_sonnet, per_specialty_agreement, fabricated_quotes, haiku_quality_by_batch),
"cost", "coverage": {"complete","missing_verifier_result","missing_adjudication"}}`. Exit 1 when coverage is incomplete.

## Fix stage (fix/, tools/fix_canary.sh, tools/fix_selftest.sh)  -- see plans/fix_pipeline_plan.md
Code + prompts + schemas: `.claude/review/fix/`. Shared helpers: `fix/fcommon.py` (paths, `is_excluded` incl. the
root-level `.env` guard, `safe_rel`, `never_edit`, `write_json`, `spent`). Env overrides (every script):
`FIX_DIR` (data: raw/ results/ patches/ checks/ reviews/ integ/; default = code dir), `FIX_WORK` (default
`.claude/review/work/fix`, or `$FIX_DIR/work` when FIX_DIR is overridden), `KOTOR2_REVIEW_HOME` (worktree/ + fix/),
`KOTOR2_FIX_OUT` (patch series, default `.claude/fixes`), `KOTOR2_FIX_REPORT` (default `.claude/reviews`),
`FIX_FINDINGS` (default `.claude/reviews/verified_findings.json`), `FIX_SANDBOX` = bwrap|none (default bwrap;
`none` only for self-tests with the stub claude). All JSON via `fcommon.dumps` (sorted keys, indent 1).
Python: `python3 -I`, stdlib only. Every runner: `--dry-run`, `--first-only`, `--only T..,T..`, per-call cap, total cap.

### Workspace layout ($FIX_HOME = $KOTOR2_REVIEW_HOME/fix, outside the repo; refuse if inside)
`base/` buildable git repo: sanitized pinned worktree (text) + binary overlay from the real tree (non-excluded,
not stripped, <= 20 MB, roots patch_manager_mods/ scripts/ tests/ research/), one commit "pinned <HEAD>".
`base.json` = {head, snapshot_manifest_sha256, overlay: {path: sha256}, base_commit, built}.
`tasks/<tid>.a<N>/` fixer copy (allowed files writable 0644 + their dirs 0755; context 0444, dirs 0555; text only).
`tasks/<tid>.a<N>.pre.json` = {path: sha256} of the copy at creation (outside the copy).
`tasks/<tid>.a<N>.review/` reviewer copy: same file set with the patch applied, plus `_pre/<path>` for changed
files; all read-only. `slots/<n>/` git worktrees of base for checks (flock `slots/<n>.lock`).
`integ/<batch>/` git worktree of base for one integrator. `bin/fixgate.sh` (+ `bin/fixgate_<batch>.sh` wrappers
with the root baked in). `ledger/<slot|batch>.jsonl` scratch MASTER_LEDGER_PATH.

### work/fix/fix_queue.json  (fix_intake.py)
`{generated_by, source_sha256, pinned_head, counts, tasks: [Task]}`; Task =
`{task_id "T<10 hex>", class auto-fixable|needs-review|needs-design|human-only, class_reasons, specialty, flags
{needs_live_check, gate_semantics, sandbox_tests_only, relocate}, finding_ids, duplicates, colocated, primary_file,
lines, mod, allowed_files, context_globs, line_cap, severity, tiers, order, wave, findings: [FixerView]}`;
FixerView = `{id, file, lines, category, specialty, severity, claim, verifier_reasoning, evidence_quotes,
location_check: [{file, cited, found, status ok|moved|not-found|missing|trivial|range-mismatch}]}` — the ONLY
finding fields a fixer/reviewer sees. `human_list.json` = {contested, human_only_tasks}; `refuted_skim.json`;
`refuted_skim_pass2.json` (Sonnet second pass, human skim only).

### fix_workspace.py
`build-base [--force]` (requires `prep.py verify-snapshot`; refuses if base exists unless --force),
`verify-base` (exit 0 iff base clean at base_commit and no excluded path), `make-task <tid> --attempt N
[--from-patch P]`, `diff <tid> --attempt N` -> `patches/<tid>.aN.patch` + `patches/<tid>.aN.scope.json`
= `{task_id, attempt, ok, empty, changed_files, violations: [{path, reason}], lines_added, lines_removed,
lines_changed, line_cap, over_cap}` (violation reasons: not-allowed, added-file, deleted-file, binary, excluded,
symlink, mode), `make-review-copy <tid> --attempt N`, `slot-reset <n>`, `clean <tid>`, `--self-test`.
Patch format: `diff -u` per file with `--- a/<path>` / `+++ b/<path>` headers; `git apply --check` clean on base.

### fix_settings.py
`fixer <tid> --attempt N`, `reviewer <tid> --attempt N`, `integrator <batch>` -> `$FIX_WORK/settings/<name>.json`
(deny secret patterns as Read/Edit/Write in the three forms of prep.write_settings, real repo + parent,
~/.claude ~/.config ~/.ssh, //**/.git/**, Bash (except integrator), WebFetch WebSearch Agent Task Skill
NotebookEdit; allow Edit/Write on each allowed file by absolute `//` path only (fixer), on `//integ/<batch>/**`
(integrator), and `Bash(<FIX_HOME>/bin/fixgate_<batch>.sh:*)` (integrator); defaultMode dontAsk).
`bwrap-argv claude|gate <rw_dir>...` prints a NUL-separated bwrap argv prefix: `--ro-bind / /`, `--dev /dev`,
`--proc /proc`, `--tmpfs /tmp`, tmpfs over $HOME/orca, ~/.config, ~/.ssh, ~/Games, ~/.local/share/Steam,
$KOTOR2_REVIEW_HOME; rw bind of each rw_dir; `claude` adds rw ~/.claude + ~/.claude.json; `gate` adds
`--unshare-net`, tmpfs ~/.claude, ro-bind of $KPM_DIR; `--die-with-parent`. `--self-test`.

### fix_checks.py (F1b; no LLM)
`check <tid> --attempt N` -> `checks/<tid>.aN.check.json` = `{task_id, attempt, ok, gates: {apply, scope,
excluded, binary, assertion_delta, skip_scan, build, tests, compat}: {status pass|fail|skip, detail},
baseline: {filter: {failing: [...]}}, new_failures, ledger_unchanged, duration_s}`. Build/tests/compat run via
`fixgate.sh` in a slot (gate sandbox; MASTER_LEDGER_PATH scratch, MASTER_LEDGER_TEST=1, KOTOR2_ROOT=/nonexistent,
TMPDIR per slot). Tests are compared to a cached baseline of the same filter on clean base (`checks/baseline/
<sha(filter)>.json`), so only NEW failures fail the gate. Production ledger sha256 checked before/after every
check; a change aborts with exit 3. `stack <batch>` (dry apply of the batch series with --3way, lists conflicts),
`verify-series <batch>` (fresh slot, apply final series from integ output, re-run the batch gates ->
`integ/<batch>.verify.json`), `--self-test`.

### fixgate.sh <root> <subcmd> [args]   (copied to $FIX_HOME/bin; runs inside the gate sandbox)
`apply <patch>` | `reset` | `diff [path]` | `status` | `build <mod_src>` | `test <filter>` | `compat <file>...` |
`diffcheck`. Arguments must match `^[A-Za-z0-9_.:/+-]+$`, no `..`, paths relative to root. Anything else exit 2.

### fix_prompts.py
`fixer <tid> --attempt N [--error-file F] [--revise-file F]` (user prompt on stdout), `fixer-system <tid>`,
`reviewer <tid> --attempt N`, `reviewer-system <tid>` (reviewer specialty note set, never the fixer's),
`integrator <batch>`. The reviewer prompt holds the findings, the patch and +-15 lines of post-image; never the
fixer's rationale/test_plan/notes, never calib_key data.

### record_fix.py  (reuses tools/record_run.py)
`fix|review|integrate <id> <raw_out> <raw_err> <rc> <attempts> <rl_hits> <started> <dur> <prompt>` ->
`results/<tid>.aN.json` | `reviews/<tid>.aN.review.json` | `integ/<batch>.run.json`; envelope as verify results
(`kind, status ok|invalid|failed|rate_limited, schema_valid, schema_errors, cost_usd, usage, num_turns,
tool_calls, budget_hit, model, output`). Review results also get `quote_check` (quotes against the review copy).

### Task status (fcommon/fix_merge)
fixer: pending -> fixed-candidate | not-fixable | not-reproducible | invalid; checks: ok | failed (one retry with
the error) ; review: accept | revise (one loop) | reject; integration: keep | modified | drop; terminal:
fixed+tested | fixed+needs-live-check | rejected | human-only | not-fixable | failed.

### work/fix/calib_key.json (fix_calibrate.py; answer key, never rendered into any prompt or copy)
`{"items": {"<tid>": {"kind": "seeded-bad"|"known-good", "mode": "assert-delete"|"unrelated-file"|"no-op-fix"|
"off-by-one"|"swallow-error"|"good", "source_task": tid, "expected": "reject"|"revise-or-reject"|"accept"}}}`.
Seeded patches are wrong in CONTENT at the true location (not mis-cited). Calibration task ids look like real ids.

### integration batches (build_integration_batches.py) and final outputs (fix_merge.py)
`work/fix/batches/batch_NN.{json,md}`: `{batch, mods, files, tasks: [{task_id, attempt, patch}], gates:
{build: [mods], tests: [filters], compat: [files]}, baseline}`; order: _shared_src, mods, scripts, tests; <= 8
patches. Final series: `$KOTOR2_FIX_OUT/<batch>/NNNN-<tid>.patch` + `series.json`. Report:
`$KOTOR2_FIX_REPORT/fix_report.{json,md}` (per finding terminal status, patch paths, test evidence, calibration
catch rate, fixer->reviewer agreement, Sonnet override rate, per-stage CLI-reported cost, coverage: every queue
task terminal; exit 1 + re-queue list otherwise).
