# Fresh-session prompt: build the FIX pipeline (Haiku fixers -> Haiku reviewers -> Sonnet integrators)

You are the Lead Fix Architect for the KOTOR II C++ repo at `~/orca/workspaces/local-setup/KOTOR-II`.
A find-only audit (Map -> Reduce -> Verify) has just finished. Your job in THIS session is to
**investigate, design and build the workflow and tooling** that will apply fixes for the verified findings
with the same "army of Haiku agents" approach. Later (only after I approve) you run it.
Phase A (this session, no model-call spend): read, plan, write scripts/prompts/schemas, self-test with a stub `claude`.
Phase B (after my explicit go): canary, pilot batch, full run.

## 0. Hard rules (non-negotiable)
- Plan first. Write `.claude/plans/fix_pipeline_plan.md` and wait for my go before ANY real `claude -p` call
  beyond a tiny pilot I approve. Ask one short question at a time if blocked.
- Never edit the REAL working tree, never `git add`/commit/push/stash, never touch the live game dir.
  Never run `kpm_hotswap.py`, any `--apply`, install/deploy paths, `clef_dataset.py`, `shadow_replay.py`
  (they ignore `MASTER_LEDGER_PATH`). Never write `.claude/master_ledger.jsonl` (hash before/after any test run).
  Any test that touches ledgers must run with `MASTER_LEDGER_PATH` pointed at a scratch file.
- The real tree is DIRTY (many uncommitted files from other sessions; another interactive session may be
  editing it right now). Fix work happens only in an isolated copy (section 3). Final application to the
  real tree is a separate step I approve per batch.
- Secrets: never read/include `.env`, `*.pem`, `.credentials.json`, `sunshine/credentials/*`, `endpoint.env`,
  `tests/static/raw_addresses.toml`, anything matching `*credential*` or `*secret*`, or
  `~/.config/ghidra/.server_credentials`. Reuse `prep.py` `is_excluded` logic (note: it has a bug for a
  root-level `.env` because of `lstrip("./")`; guard against it in any new code, don't edit prep.py).
- No GitHub comments/issues/PRs without my explicit chat approval.
- Project rules: CLAUDE.md in repo root. Scoped tests only (`python3 scripts/run_tests.py <filter>`; a bare run
  is 156 suites and >60s). Use `rg`/`sed -n`, never `cat` whole source files. Use native Edit/Write for edits.
- Final user-facing replies: ADHD style (answer first, numbered steps, <=5-item lists, end with one <2-minute
  next action) plus the 4-section protocol (Actions Completed / What Now Works / Agents Used / Saved State).
  Subagent-to-orchestrator messages use terse key-value output instead.

## 1. Read these first (use `sed -n` ranges for big files)
- `.claude/reviews/verified_findings.md` + `.json` (the input; tiers: CONFIRMED, CONFIRMED-BY-SONNET-ONLY,
  CONTESTED/needs-human, REFUTED). Use ONLY these. Ignore `work/verify_key.json`, decoys, controls and
  `withheld_leads.md` (audit answer keys; not for fixers).
- `.claude/reviews/master_findings.md`, `coverage_report.md` (context only).
- `.claude/review/INTERFACES.md`, `.claude/plans/master_review_audit_plan.md` (esp. sections 2, 5, 12: the
  lockdown flags, three-layer exclusion, per-request pricing, what the canary proved).
- Pipeline code to reuse/mirror: `.claude/review/run_map.sh`, `verify/run_verify.sh`, `verify/run_adjudicate.sh`,
  `verify/vcommon.py`, `verify/quote_check.py`, `verify/verify_merge.py`, `tools/record_run.py`,
  `tools/record_verify.py`, `tools/assertion_delta.py`, `tools/classify_hunks.py`, `prep.py`
  (`build_tree`, `assert_tree_clean`, `is_excluded`, `write_settings`), `tools/canary_test.sh`, `tools/fact_runner.sh`.
- Repo gates/tools: `scripts/build_check.py`, `scripts/run_tests.py`, `scripts/agent_diff_check.py`,
  `scripts/compat_check.py` (after touching `_shared_src`), `scripts/pre_install_gate.py` (read-only understanding;
  do not run install paths), `patch_manager_mods/SUBAGENT_ROLES.md`, `scripts/agent_contract.py` (worker/recon
  contracts). Check how `build_check.py` and `run_tests.py` select per-mod targets so the pipeline can run them
  per affected mod.

## 2. Facts learned building the audit pipeline (do not re-learn them)
- Claude Code CLI 2.1.296 lockdown flags that work: `--restricted --safe-mode --settings <deny file> --tools "<list>"
  --permission-mode dontAsk --max-budget-usd N --json-schema <schema> --strict-mcp-config
  --mcp-config '{"mcpServers":{}}' --disable-slash-commands --no-session-persistence
  --output-format stream-json --verbose --append-system-prompt <file>`; prompt on stdin; cwd = the sandbox dir.
- `--json-schema` REJECTS a `$schema` key. A `StructuredOutput` tool is added by the CLI. `--max-budget-usd` is
  enforced even on OAuth (`error_max_budget_usd`). `--bare` breaks OAuth. There is no `--max-turns`.
- Auth is OAuth, so dollar figures are API-equivalent estimates. Haiku 5.5: <=100K prompt $0.10 in / $0.125 cache
  write / $0.01 cache read / $0.50 out per MTok (>100K: 5x). Sonnet 5.5: $2 in / $10 out / $0.10 cache hit.
- Observed audit costs: Map 80 Haiku calls $0.61; Reduce 2 Sonnet sessions $1.01; verify 275 Haiku ~$1.5;
  Sonnet adjudication ~$0.22/batch of ~10 items. Waves of 8 Haiku in parallel hit no rate limits; 5 parallel
  Sonnet worked. Keep the 3-try exponential backoff on 429/529 and halve parallelism on repeated hits.
- A Haiku call occasionally fails with `error_max_structured_output_retries`; one re-queue fixes it.
- Haiku fabricates quotes occasionally (3/275 in verify): never trust a quote without the deterministic check.
- Idempotency pattern: completed = valid result file exists; every runner supports `--first-only`, `--only`,
  `--dry-run`, total-budget cap, per-call cap.
- Known landmine: tests that write the production ledger (the audit's K1 finding). Never run unscoped tests.

## 3. Design requirements (you refine; I approve)
Pipeline stages and invariants:
- **F0 Intake (script, no LLM):** from `verified_findings.json` build `fix_queue.json`. Keep only fixable tiers
  (CONFIRMED, CONFIRMED-BY-SONNET-ONLY); CONTESTED goes to a human list. Classify each item: `auto-fixable`
  (small, local, mechanical), `needs-design` (cross-file / semantic / hook-ASM / address-DB), `human-only`
  (policy, gate semantics, anything needing a game run, anything touching install/deploy). Group items by file
  and by mod so no two fixers edit the same file concurrently; define an apply ORDER (shared headers first,
  then mods, then scripts/tests). Dedupe overlapping findings. Cap patch size per task.
- **Isolation:** a fix workspace OUTSIDE the repo, e.g. `~/.cache/kotor2_review/fix/`. (a) `base/` = a real
  buildable copy of the pinned snapshot (HEAD + the audit's dirty overlay, secrets excluded by the same
  pathspec/rsync filters; NOT the stripped sanitized worker tree if that cannot build; investigate and decide);
  (b) one throwaway per-task copy of only the files the task may touch + read-only context. A per-task
  copy is the fixer's cwd, so Edit/Write cannot leave it. The harness, not the agent, produces the unified diff
  (`diff -u` against base) and rejects patches touching anything outside the task's allowed-file list, any
  excluded path, any binary, or exceeding the line cap.
- **F1 Haiku fixers (1 task per call, specialised by the same six specialties as verify: hook-asm,
  test-weakening, gate-integrity, python-tooling, cpp-logic, build-config):** tools limited to
  `Read,Grep,Glob,Edit,Write` (NO Bash), deny rules for secrets and for paths outside the task copy. Input:
  verified finding + evidence + source excerpt. Output (schema): list of edits are applied in the task copy;
  plus structured summary {finding_id, files_changed, rationale, test_plan, risk, not_fixable_reason}.
  Rules: minimal diff; fix the verified defect only; no refactors; never weaken/delete/skip a test or
  assertion to make something pass (enforce mechanically with `tools/assertion_delta.py`); never touch gate
  logic semantics without flagging `human-only`; follow the repo include style and Gate Invariant; hook/ASM
  fixes must be flagged `needs-live-check` and are not auto-accepted.
- **F1b Deterministic checks (script):** `git apply --check` against base, scope check, assertion-delta,
  excluded-path scan, compile of the affected translation unit / `build_check.py <mod>` in the task copy,
  and the mod's scoped test (`run_tests.py <filter>`) with a scratch `MASTER_LEDGER_PATH`. Record results as
  JSON next to the patch. A patch that fails deterministic checks gets at most ONE fixer retry with the error
  appended, then goes to the human list.
- **F2 Haiku reviewers (blind, independent):** sees the finding + the patch + a source excerpt, NOT the
  fixer's rationale. Read-only tools (`Read,Grep,Glob`). Must try to refute: does it fix the cited defect,
  regressions, scope creep, weakened tests, hook/ASM/calling-convention risk, unchecked return codes, boundary
  changes. Verdict enum accept | revise | reject, verbatim quotes (run `quote_check.py`). Use a different
  specialty note set than the fixer. At most one revise loop back to F1.
- **Calibration:** plant seeded bad patches (e.g. a patch that deletes an assertion, one that edits an
  unrelated file, one that does not actually fix the issue, one that introduces an off-by-one) and report the
  reviewer catch rate; also feed 3-5 known-good fixes. Keep the answer key outside every prompt, dossier and
  worker-visible path (same discipline as `work/verify_key.json`).
- **F3 Sonnet integrators (batch):** batch by mod/file in dependency order. Each Sonnet session gets the accepted
  patches, the reviewer verdicts and deterministic-check results; applies them in an integration worktree
  copy (not the real tree), resolves conflicts, runs the mod's scoped `build_check.py` + `run_tests.py` +
  `compat_check.py` when `_shared_src` changed, bisects any failure to a single patch, drops or flags it,
  and re-verifies. Output: an ordered, tested patch series per batch plus a verdict table. Sonnet is the ONLY
  stage that may run Bash, and only an allow-listed set (build_check, run_tests with filters, compat_check,
  agent_diff_check, git diff/apply in the scratch worktree). No install/hot-swap/--apply anywhere.
- **F4 Final merge (script):** `fix_merge.py` -> `.claude/reviews/fix_report.md/.json`: per finding final status
  (fixed+tested, fixed+needs-live-check, rejected, human-only, failed), patch series paths, test evidence,
  calibration stats (reviewer catch rate, fixer->reviewer agreement, Sonnet override rate), per-stage cost,
  coverage check (every queue item has a terminal status). Patch series live under `.claude/fixes/` (new dir);
  applying them to the real tree and committing is a separate, human-approved step with explicit paths.

## 4. Deliverables for Phase A (this session)
1. `.claude/plans/fix_pipeline_plan.md` with: queue size and class breakdown computed from the real findings;
   the isolation design and why; stage diagram; flag/tool lists per stage; cost and wall-clock estimates using
   the observed numbers above; risks; the exact Phase B launch sequence; open questions for me (max 5).
2. `.claude/fixes/` or `.claude/review/fix/` tooling (pick one dir, keep it parallel to `verify/`):
   `fix_intake.py`, `fix_workspace.py` (build base + per-task copies, produce diffs, scope checks),
   `fix_settings.py` (deny/allow settings per stage), `fixer_prompt.md` + `specialty_notes/`, `fix_schema.json`,
   `reviewer_prompt.md` + schema, `fix_checks.py` (deterministic gates), `run_fix.sh`, `run_fix_review.sh`,
   `build_integration_batches.py`, `integrator_prompt.md` + schema, `run_integrate.sh`, `fix_merge.py`,
   `tools/fix_canary.sh`, `INTERFACES` section. All stdlib Python, `python3 -I`, idempotent, `--dry-run`,
   `--first-only`, `--only`, budget caps, `VERIFY_DIR`-style env override so tests never touch real outputs.
3. `fix_canary.sh` that PROVES (before any real fixer call): a fixer cannot write outside its task copy, cannot
   read secret-pattern files (dummy markers in a dummy mirror, scan every saved stream byte), has no Bash,
   and the budget cap works. Mirror `tools/canary_test.sh` incl. a deny-probe call with a plain sandbox prompt
   so the harness rules, not the worker's own refusal, are what is being tested. Phase B is refused unless
   the canary JSON says pass.
4. Self-tests with a stub `claude` on PATH for every script; `bash -n` on shells; confirm no secret-pattern
   path appears in any queue/prompt/patch artifact; confirm the production ledger hash is unchanged.
5. A one-screen summary for me (ADHD format) with the exact launch commands for the pilot (3-5 findings across
   different specialties, end to end F0-F3) and the full run, plus a cost range. Then STOP and wait for go.

## 5. Delegation
Use Sonnet 5.5 subagents for routine script writing (one agent per component with a precise interface spec) and
a Haiku 5.5 subagent for spot-checking artifacts against the snapshot; you design, review, integrate and gate.
Prefer fewer, well-specified agents (2-4). Do not let agents run real model calls.

## 6. Start
1. Confirm `.claude/reviews/verified_findings.json` exists; if not, tell me the audit is still finishing and
   work from `.claude/reviews/master_findings.json` for sizing only.
2. Run `python3 -I .claude/review/prep.py verify-snapshot` (read-only) and note HEAD vs pinned
   `22b56b427125e12bb7b2914d96b820973d06c445`; if HEAD moved, say so and plan for it.
3. Compute queue sizes by tier/specialty, then write the plan, then build tooling.
