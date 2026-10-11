# Fix pipeline plan: verified findings -> tested patch series (Haiku fixers, Sonnet integrators)

Status: Phase A (tooling, no model calls for the pipeline itself). Phase B waits for an explicit "go".
Input: `.claude/reviews/verified_findings.json` (sha in `work/fix/fix_queue.json`). HEAD = pinned
`22b56b427125e12bb7b2914d96b820973d06c445`; `prep.py verify-snapshot` OK; the 3330 snapshot text files show zero drift
from the real tree (checked 2026-10-10). Production ledger sha256 at start:
`73c49d59df5103c4b33070ac95f549f2e595a1d3f2c6c029e264e7aba594cf8f`.

## 1. Queue (computed by `fix/fix_intake.py` from the real findings)

| | count |
|---|---|
| Fixable findings (CONFIRMED 152 + CONFIRMED-BY-SONNET-ONLY 10) | 162 |
| Tasks after clustering (same file + overlapping lines; 2 true duplicates, 25 co-located distinct findings grouped) | 135 |
| auto-fixable (Haiku fixer) | 86 tasks / 92 findings |
| needs-review (Sonnet-only tier; Haiku fixer, never auto-accepted) | 4 / 4 |
| needs-design (Sonnet fixer; hook-asm, build-config, gate-integrity, toml/def, _shared_src, multi-file, span > 40, severity high, live-dir/ledger writers) | 41 / 62 |
| human-only (AGENTS.md / CLAUDE.md policy text, never-edit paths) | 4 / 4 |
| CONTESTED-NEEDS-HUMAN -> `work/fix/human_list.json` | 22 |
| REFUTED cpp-logic/build-config -> `refuted_skim.json` (30) + Sonnet second pass | 30 |

Fixer tasks by specialty: python-tooling 60, cpp-logic 33, test-weakening 24, gate-integrity 12, hook-asm 1, build-config 1.
Flags: relocate 3 (cited range disagrees with the finding's own quotes; fixer must re-locate), gate_semantics 14,
sandbox_tests_only 12, needs_live_check 2. 61 files; 11 same-file waves.

**REFUTED second pass (done this session, 4 Sonnet recon agents, 7-8 items each, file-grouped; 49/49 quotes verified by
script):** refute-holds 21, design-risk 8, real 1 (`f9d03b5a7018`, equip3d `animLen` blind 2.0 s fallback also hits rows
3/538/539 that deliberately return n=0). Report: `.claude/reviews/refuted_skim_pass2.md`. Not fixed by the pipeline;
human decides whether to promote any item.

## 2. Isolation design (why)

- **base/** = `$KOTOR2_REVIEW_HOME/fix/base`: a git repo (outside the real repo) = the sanitized pinned worker tree
  (text, hash-pinned by snapshot.json) + a binary overlay from the real tree (non-excluded, <= 20 MB) so build_check and
  tests can run. The stripped worker tree alone cannot build (no .db/.dll/testdata binaries). Secrets never enter base
  (prep exclusion rules + the root-level `.env` guard in `fcommon.is_excluded`; prep.py is not edited).
- **Per-task copy** `tasks/<tid>.aN/`: only allowed files (writable) + context globs (read-only 0444, dirs 0555). The
  fixer's cwd. The harness diffs it against its pre-manifest and base: any non-allowed, added, deleted, binary,
  excluded or over-cap change is a scope violation and the patch is rejected before review.
- **Four layers for fixers:** (0) bwrap: real repo (`~/orca`), `~/.config`, `~/.ssh`, `~/Games`, Steam, the rest of
  `kotor2_review` hidden by tmpfs; only the task copy (+ `~/.claude` for the CLI) writable. Verified: `claude --version`
  starts inside it and `~/orca` lists empty. (1) the copy holds no secrets. (2) settings: dontAsk, secret deny rules,
  Edit/Write allowed ONLY on the absolute paths of the allowed files; Bash/Web/Agent denied; `--tools
  Read,Grep,Glob,Edit,Write`. (3) harness-side diff + scope check. The canary proves 0+2 (variant A) or 2 alone (B).
- **Gates (F1b, F3) run in `fixgate.sh`:** bwrap with `--unshare-net`, `~/.claude` hidden, real repo + game dir hidden,
  KPM dir read-only, `MASTER_LEDGER_PATH`=scratch, `KOTOR2_ROOT=/nonexistent`. So even `shadow_replay.py`/`clef_dataset.py`
  or a planted test cannot reach the production ledger, the game, credentials or the network. The ledger hash is also
  checked before/after every gate (exit 3 on change).
- **Integrator (F3):** git worktree of base; Edit/Write only inside it; Bash allow-list = one per-batch wrapper
  `fixgate_<batch>.sh` (apply/reset/diff/status/build/test/compat/diffcheck). Compound commands, pipes, `$(...)` are
  denied by dontAsk. A script re-applies the final series from scratch and re-runs the gates (`verify-series`).
- **Real tree:** never touched. Output = patch series under `.claude/fixes/<batch>/`; applying + committing them is a
  separate, per-batch, human-approved step with explicit paths.

## 3. Stages

```
verified_findings.json
  F0 fix_intake.py ............ fix_queue.json (135 tasks) + human_list + refuted_skim        [script]
  W  fix_workspace.py build-base base/ (git) + bin/fixgate.sh                              [script]
  C  tools/fix_canary.sh ...... canary.json (fixer + integrator lockdown; Phase B refuses w/o pass)  [4-6 Haiku calls]
  F1 run_fix.sh ............... per task: make-task -> Haiku|Sonnet fixer -> diff -> F1b      [LLM]
  F1b fix_checks.py check ..... apply/scope/excluded/binary/assert-delta/skip-scan/build/tests(new failures only)
        fail -> ONE retry (a2) with the check json appended -> else human list
  K  fix_calibrate.py ......... 5 seeded-bad (content-wrong at the true location) + 3 known-good, ids look real
  F2 run_fix_review.sh ........ blind Haiku reviewer (no rationale), quote check; revise -> ONE loop (a3)  [LLM]
  F3 build_integration_batches.py -> run_integrate.sh (Sonnet, <= 8 patches/batch, dependency order)       [LLM]
        -> fix_checks.py verify-series (script re-applies + re-gates) -> .claude/fixes/<batch>/NNNN-<tid>.patch
  F4 fix_merge.py ............. .claude/reviews/fix_report.{json,md}; exit 1 + re-queue lines on gaps   [script]
```

| stage | model | tools | flags (all: `--restricted --safe-mode --settings <stage file> --permission-mode dontAsk --max-budget-usd N --json-schema <schema> --strict-mcp-config --mcp-config '{"mcpServers":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <prompt+specialty note>`, prompt on stdin) |
|---|---|---|---|
| F1 fixer | haiku-5-5 (auto/needs-review), sonnet-5-5 (needs-design) | Read,Grep,Glob,Edit,Write | cwd = task copy; bwrap claude profile; cap $0.15 Haiku / $0.60 Sonnet |
| F2 reviewer | haiku-5-5 | Read,Grep,Glob | cwd = review copy (post-image + `_pre/`); reviewer specialty notes (different set); cap $0.12 |
| F3 integrator | sonnet-5-5 | Read,Grep,Glob,Edit,Write,Bash(fixgate wrapper only) | cwd = integ worktree; cap $1.50/batch |

Apply order: `_shared_src` -> mods -> scripts -> tests -> other. Line caps: 40 auto / 60 needs-review / 120 needs-design.
Hook/ASM patches end as `fixed+needs-live-check`, never auto-accepted. gate_semantics patches are marked for your sign-off.

## 4. Cost and wall-clock (CLI-reported basis; audit: Haiku verifier $0.0054/call, Sonnet adjudicator $0.21/batch)

| stage | calls | per call (CLI) | total |
|---|---|---|---|
| canary | 4-6 Haiku | ~$0.01 | ~$0.05 |
| F1 Haiku fixers | ~90 tasks x 1.4 | $0.01-0.03 | $1.3-3.8 |
| F1 Sonnet fixers (needs-design) | ~41 x 1.4 | $0.10-0.40 | $6-23 |
| F2 Haiku reviewers | ~120 incl. 8 calib | $0.006-0.015 | $0.8-2 |
| F3 Sonnet integrators | ~16-20 batches | $0.25-0.75 | $4-15 |
| **full run** | | | **~$12-44** (caps: fix $6 Haiku+design set separately, review $3, integrate $8; raise deliberately) |
| **pilot (5 tasks, F0-F3)** | | | **~$1-3** |

Wall-clock (8 parallel Haiku, 3-5 parallel Sonnet, no rate limits seen at that level): fixers ~80 min (11 same-file
waves), reviews ~25 min, integration ~60-90 min, total ~3-3.5 h. Pilot ~20-30 min.

## 5. Risks

1. **CONFIRMED is not clean:** 3/8 decoys were confirmed in the audit. Fixers may report `not-reproducible`; that is a
   good outcome and lands in the report, not as a failure.
2. **Cited lines can be wrong** (3 relocate tasks): the fixer must quote the real location; the reviewer checks it.
3. **Baseline failures in the dirty snapshot:** gates compare against a cached clean-base run of the same filter, so
   only NEW failures block a patch. A flaky test can still block; integrator bisect + script re-verify catch most.
4. **`.venv`-based unittest items** are not in base (no .venv copy); those items SKIP in gates. Noted per patch.
5. **Real tree drift:** another session edits the real tree. Patches apply to the pinned snapshot; the final per-batch
   apply runs `git apply --check` against the real tree first and stops on any conflict.

## 6. Phase B launch sequence (after "go"; run from the repo root; each step idempotent)

```bash
python3 -I .claude/review/fix/fix_intake.py                                  # 0. queue (no model)
python3 -I .claude/review/fix/fix_workspace.py build-base                    # 1. base/ + bin/ (no model)
bash .claude/review/tools/fix_canary.sh                                      # 2. canary (4-6 small Haiku calls) -> fix/canary.json
# --- pilot: 5 tasks across 5 specialties, both fixer models ---
P=Td18cc50320,T32db00261f,T49ca9cb835,Tc307b197f4,T106b5fe355
bash .claude/review/fix/run_fix.sh --dry-run --only $P                       # 3. prompts + exact command
bash .claude/review/fix/run_fix.sh --only $P --class auto-fixable,needs-review,needs-design   # 4. F1 + F1b
python3 -I .claude/review/fix/fix_calibrate.py prepare && python3 -I .claude/review/fix/fix_calibrate.py queue   # 5. calibration items (no model)
bash .claude/review/fix/run_fix_review.sh --only $P                          # 6. F2 (pilot; calib runs in the full pass)
bash .claude/review/fix/run_fix.sh --revise --only $P                        # 7. one revise loop if any
python3 -I .claude/review/fix/build_integration_batches.py && bash .claude/review/fix/run_integrate.sh --first-only  # 8. F3
python3 -I .claude/review/fix/fix_merge.py                                   # 9. report (exit 1 lists gaps)
# --- full run (after you review the pilot report) ---
bash .claude/review/fix/run_fix.sh                                           # auto-fixable + needs-review, Haiku
FIX_TOTAL_BUDGET_USD=25 bash .claude/review/fix/run_fix.sh --class needs-design   # Sonnet fixers
python3 -I .claude/review/fix/fix_calibrate.py prepare && python3 -I .claude/review/fix/fix_calibrate.py queue && bash .claude/review/fix/run_fix_review.sh
bash .claude/review/fix/run_fix.sh --revise && bash .claude/review/fix/run_fix_review.sh
python3 -I .claude/review/fix/build_integration_batches.py
INTEG_TOTAL_BUDGET_USD=15 bash .claude/review/fix/run_integrate.sh
python3 -I .claude/review/fix/fix_merge.py
```
Pilot picks (from the real queue): `Td18cc50320` queue_hud QueueHudPerformCastSpell export/H10 (hook-asm,
needs-design, Sonnet, needs-live-check; implement-vs-remove is reported, not decided silently), `T32db00261f`
compat_check `run_build_mods` ignores returncode (gate-integrity, needs-design), `T49ca9cb835` generate_diversity_2da
unconditional premerge backup + non-atomic copy (python-tooling, live-dir writer, sandboxed tests), `Tc307b197f4`
gui_input `ensureSubclass()` result discarded (cpp-logic, auto, Haiku), `T106b5fe355` test_compat_check guardrail test
(test-weakening, auto, Haiku).

## 7. Open questions (max 5)

1. needs-design (41 tasks): Sonnet fixer as planned (~$6-23), or Haiku, or straight to the human list?
2. gate-integrity (12 tasks): fix and mark for your sign-off (planned), or human-only?
3. Known-good calibration patches: may I hand-write 3 small correct patches in `work/fix/calib_good/` (Phase A, not in
   the tree)?
4. Integrator conflict resolution by Edit inside its scratch worktree (planned), or drop conflicting patches only?
5. Final apply: per batch, `git apply --check` then apply + `git commit -- <paths>` on a new branch, each batch after
   your OK. Branch name?

## 8. Agents used (Phase A)
- 4 Sonnet recon agents: REFUTED second pass (7-8 items each).
- 3 Sonnet worker agents: (A) fix_workspace/fix_settings/fix_checks/fixgate, (B) fix_prompts/record_fix/run_fix/
  run_fix_review/fix_calibrate/stub claude/fix_canary, (C) build_integration_batches/run_integrate/fix_merge.
- 1 Haiku spot-check of artifacts against the snapshot (after build).
- Written directly (design-critical): fcommon.py, fix_intake.py, prompts, schemas, specialty notes, INTERFACES section.

## 9. Phase A status (2026-10-10)
Tooling built; all self-tests PASS; Haiku spot-check PASS; 4 cross-script bugs found by the e2e stub run and fixed
(bwrap prefix, canary path, integrator_ok key, stub repo detection). e2e re-run pending inspection. Handoff:
`.claude/handoffs/fix_pipeline_plan.handoff.md`. No pipeline model calls made; ledger unchanged.
