# Plan: `deep-haiku-review` skill (breakdown -> Haiku review -> Haiku verify -> Sonnet adjudicate -> Haiku fix -> Haiku review -> Sonnet sign-off)

Status (updated end of 2026-10-10): audit-half bundle BUILT and selftested ($0); pilot p1 paused at the first-packet gate; see `.claude/handoffs/deep_haiku_review_skill.handoff.md` for the full state and `deep_haiku_review_next_phases*.md` for the next plan. Original status: research + plan only.
Inputs read: `.claude/handoffs/deep_haiku_review.handoff.md`, `.claude/plans/fix_pipeline_plan.md` (other session, Phase A),
`.claude/review/INTERFACES.md`, `scripts/check_skills.py`, the 7 project skills, file-by-file KOTOR coupling count of the pipeline code.

## 1. What exists today (inventory)

| Half | Code | Lines | Status |
|---|---|---|---|
| Audit: prep, canary, Map, Reduce, merge | `.claude/review/{prep.py,merge.py,run_map.sh,tools/*}` | ~3.2k | DONE, ran 2026-10-10 ($1.6 for Map+Reduce) |
| Audit: Verify, Adjudicate, verify_merge | `.claude/review/verify/*` | ~2.0k | DONE ($7.6) |
| Fix: intake, fixers, checks, reviewers, integrators, report | `.claude/review/fix/*` (other session) | ~2.1k+ | Phase A (tooling) in progress; no model calls yet; pilot = 5 tasks ~ $1-3 |

Pipeline contract: every stage is an idempotent runner (`--first-only`, `--only`, `--dry-run`, per-call + total budget caps, 429/529 backoff),
every stage writes one JSON per unit, a deterministic script merges and gates coverage. Models never write to the real tree.

## 2. Coupling to KOTOR II (what must become a profile)

Counted by grep (kotor|patch_manager_mods|ledger|_src|kpm|WPn|K1-4|raw_addresses|hotswap|ghidra):
- Already generic (0-3 hits): `record_run.py`, `record_verify.py`, `build_dossiers.py`, `quote_check.py`, `reduce_prompt.md`, `run_map.sh`,
  `run_verify.sh`, `run_adjudicate.sh`, `run_reduce.sh`, `classify_hunks.py`, schemas, `verify_merge.py` (4).
- Needs a profile (hard-coded repo knowledge): `prep.py` (BASE, PINNED_HEAD, EXCLUDE patterns, WP path rules, class a/b/c rules, 37 hits),
  `merge.py` (known-issue answer key, domains, 31), `verify/select.py` (specialty routing, 17), `fix/fcommon.py` (NEVER_EDIT, `mod_of`, protected ledger, 19),
  `fix/build_integration_batches.py` (batch categories + build/test/compat gates, 18), `fix/fix_intake.py` (task class rules, 13),
  `tools/fact_runner.sh` (repo-specific deterministic facts, 14), `tools/scan_patterns.py`, `tools/assertion_delta.py` (test-file conventions, 7-9).
- Worker/verify/fix prompts: 2-6 hits each; domain notes + specialty notes are the repo-specific text.

Conclusion: ~70% of the code is already engine. The repo knowledge is ~8 files; put it in ONE `profile.toml` plus domain/specialty note files.

## 3. Target design

### 3.1 Shape: one skill, one bundle, one driver
```
deep-haiku-review/
  SKILL.md                    short router (<= ~120 lines): invariants, stage table, "always run `dr next`", gates
  references/                 one file per stage group, loaded only when that stage is active
    00-scope-and-profile.md   how Opus/main splits work into WPs + writes profile.toml (the only interactive stage)
    10-audit.md               canary, Map, Reduce, merge, coverage gate
    20-verify.md              Haiku verify, Sonnet adjudicate, calibration, human skim list
    30-fix.md                 intake, Haiku fixers, checks, Haiku reviewers, Sonnet integrators, apply
    90-lessons.md             the gotchas from the handoff (flags, pricing, failure modes)
  bundle/
    dr.py                     driver: `init|status|next|run <stage>|gate <name>|estimate|report` (state.json, like plan_manager.py)
    engine/                   prep, merge, select, runners (*.sh), quote_check, record_*, fix/*  (generic code)
    templates/                worker/verify/adjudicate/fixer/reviewer/integrator prompts, schemas, deny-settings templates
    profiles/example/         tiny fixture repo + profile used by `dr selftest` (stub `claude`, $0)
```
`dr next` prints the next single command, what gate it is behind, and the budget it will consume. The agent never hand-sequences stages.
This is the same ergonomics as `plan_manager.py resume` (survives `/clear`, matches the ADHD/clear-resume workflow).

### 3.2 `profile.toml` (the only per-repo input)
Fields: `base` / `head` (pinned) / `boundary`; `exclude` globs (secrets: .env, *.pem, *credential*, *secret*, plus repo-specific); `workpackages[]`
(name, path globs, domain note file, reduce-domain group); `docs_out_of_scope` globs; `classes` (generated/vendored/binary rules);
`specialties[]` (globs -> hook-asm / test-weakening / gate-integrity / python-tooling / cpp-logic / build-config or custom + note file);
`facts[]` (deterministic commands the runner may execute, each with sandbox flags); `answer_key` (optional path; blind recall only if present);
`protected[]` (files hashed before/after every stage, e.g. the production ledger; any change aborts); `forbidden_paths` / `never_edit` globs;
`gates` for fix stage (`build_cmd`, `test_cmd`, `compat_cmd` templates with `{unit}` + batch grouping regex + apply order);
`budgets` (per-call and per-stage caps, defaults from section 6).
KOTOR II gets `.claude/review/profile.toml` (checked in to the repo or kept untracked; the skill only reads it).

### 3.3 Stage list, owner model, gate
| # | Stage | Model | Gate before the NEXT stage |
|---|---|---|---|
| S0 | Scope + profile (WP split, excludes, answer key optional) | Opus/main, interactive | user approves `profile.toml` |
| S1 | Prep: sanitized tree outside repo, hunks, packets, coverage units, snapshot | none | `verify-snapshot` OK, secrets scan clean |
| S2 | Canary incl. deny probe | Haiku x4-6 | `canary.json` pass (refuse otherwise) |
| S3 | Map: `--first-only` -> resize -> full | Haiku xN | user OK after first packet |
| S4 | Reduce per domain | Sonnet | schema + cross-reference check |
| S5 | Merge + coverage gate | none | 0 coverage gaps |
| S6 | Verify (specialty-routed blind, quote-checked) | Haiku x~0.65/finding | all items have a result |
| S7 | Adjudicate (10 items/batch, devil's advocate on refutations) | Sonnet | all batches ok |
| S8 | Verify merge + calibration; **refutation audit** (Sonnet second pass on REFUTED in hard specialties) | none + Sonnet | **HUMAN GATE: skim CONTESTED + audited REFUTED** |
| S9 | Fix intake: cluster duplicates, classify (auto / needs-review / needs-design / human-only) | none | **HUMAN GATE: "go" for fix stage + budget** |
| S10 | Fix canary (fixer bwrap + integrator lockdown), pilot of 5 | Haiku/Sonnet | user OK on pilot report |
| S11 | Fixers in isolated per-task copies; deterministic checks (apply, scope, assertion-delta, skip-scan, build, tests = new failures only) | Haiku (Sonnet for design tasks) | check json pass or one retry |
| S12 | Blind Haiku reviewers + seeded-bad calibration; one revise loop | Haiku | calibration catches seeded-bad |
| S13 | Sonnet integrators per batch (<= 8 patches, allow-listed wrapper), script re-applies + re-gates series | Sonnet | verify-series pass |
| S14 | fix_merge report | none | **HUMAN GATE: per-batch `git apply --check` + apply + commit with explicit paths** |

Never automatic: commits, `git add`, deploy/install paths, posting to GitHub, writing protected files. The skill is user-invoked only
(it spends money): `disable-model-invocation: true` if the field is supported (see section 5).

## 4. Known gaps to fix while extracting (from the audit)
1. **Decoys:** claim text must not carry the real file/line; make decoys wrong in content (3/8 came back "confirmed").
2. **Over-refutation:** verifiers refuted K2/K4/K3a. Make the Sonnet refutation audit standard for cpp-logic/build-config (the fix session's pass found 21 hold / 8 design-risk / 1 real of 30).
3. **Duplicate clusters before Verify**, not only before fix (5cacb52504ba / c8b1757d9b9a cost two verifier calls and two adjudications).
4. **Root-level `.env`:** `prep.is_excluded` bug (`lstrip("./")`); the fix session's `fcommon.is_excluded` already guards. One shared function, tested.
5. **Self-matching waiters:** `until ! pgrep -f "run_x.sh"` matches its own command line and never exits (seen today: 5 stuck tasks). `dr` uses pid/lock files and a `[r]un_x.sh` pattern.
6. **Cost:** record CLI-reported `total_cost_usd` only; the list-rate estimate is ~2.5x low for Sonnet. `dr estimate` uses observed per-unit costs (section 6).
7. **`verify/select.py` shadows stdlib `select`:** rename (`pick_units.py`).
8. **Testing hygiene:** facts and fix gates run in the bwrap/no-network sandbox with `MASTER_LEDGER_PATH` scratch + protected-file hash before/after (fix session already built `fixgate.sh`); reuse for facts.
9. Answer key and blind recall: optional in profile; stripped from every worker prompt by construction (separate file the Reduce stage never sees).

## 5. Packaging / location decision (needs your call)
Docs facts (claude-code-guide agent, code.claude.com/docs/en/skills.md + plugins/overview.md):
- Bundled files live beside `SKILL.md`; SKILL.md refers to them as `${CLAUDE_SKILL_DIR}/bundle/dr.py` (also usable inside `allowed-tools` Bash rules). Plugins add `${CLAUDE_PLUGIN_ROOT}` / `${CLAUDE_PLUGIN_DATA}`.
- SKILL.md under 500 lines, detail in separate files (matches the `references/` layout above). `description` + `when_to_use` truncate at 1,536 chars (linter caps description at 1024).
- Frontmatter we want: `disable-model-invocation: true` (spends money, user-invoked only), `argument-hint`, `allowed-tools` (pre-approves only, does NOT restrict; use `disallowed-tools` to remove), optional `effort`.
- Precedence: enterprise > personal `~/.claude/skills` > project `.claude/skills`. Plugins are namespaced (`/plugin:skill`) and are the documented way to ship skills + scripts across repos; every enabled plugin adds its descriptions to context each turn.
- `context: fork` runs the skill as a subagent that does not see the conversation (not wanted for the interactive S0/gates; fine for a pure status/report sub-skill).
- Docs say nothing about skills launching background `claude -p`: our driver starts jobs detached, writes pid + log, returns both (already how the runners behave).
- Linter note: `check_skills.py` ignores paths containing `$`/`{`, so `${CLAUDE_SKILL_DIR}/...` refs are NOT existence-checked. The plan adds a `dr selftest` that asserts every bundle file named in SKILL.md/references exists.

Options:
- A. **Project skill** `.claude/skills/deep-haiku-review/` in this repo: gets `check_skills.py` linting for free (frontmatter, path existence, `--help` of every `python3 scripts/...` it names). Only usable in KOTOR II until copied.
- B. **User skill** `~/.claude/skills/deep-haiku-review/` (real dir; note `~/.claude/skills/*` here are symlinks into `~/.agents/skills`): available in every repo; not linted by `check_skills.py` (it would WARN DIVERGED only for same-named project copies).
- C. **Plugin** (if the docs recommend it for scripts + multi-repo): versioned, installable.
Recommendation: build as A (linted, tested against the real KOTOR II run), keep the bundle self-contained with no `scripts/` or repo imports so it can be promoted to B/C by a copy. The linter only checks repo-relative paths written as `scripts/`, `.claude/` etc., so reference bundle files with the skill-dir variable/placeholder rather than hard-coded `.claude/review/...` paths.

## 6. Cost model (CLI-reported, from the audit; scales per unit)
| Stage | Observed | Formula |
|---|---|---|
| Map | $0.61 / 80 packets (735 units) | ~$0.0076/packet, ~$0.0008/unit |
| Reduce | $1.01 / 2 domains | ~$0.50/domain (427 findings) |
| Verify | $1.49 / 275 | $0.0054/finding verified |
| Adjudicate | $6.06 / 29 batches | $0.21/batch of ~10 (dominant cost; 2.5x the list-rate estimate) |
| Fix (projection) | $12-44 for 135 tasks (other session) | Haiku $0.01-0.03/task, Sonnet design $0.10-0.40, integrators $0.25-0.75/batch |
`dr estimate` prints low/high before every paid stage; stage caps default to 1.5x the high estimate; raising a cap needs an explicit user OK.
Rule of thumb: audit of ~700 coverage units ~ $9 and ~2.5 h wall; add ~ $12-44 and ~3.5 h for fixes.

## 7. Build plan for the skill (after your decisions; Sonnet worker agents per step, main reviews)
1. **S-A decisions (you):** section 8 questions.
2. **S-B extract engine** into `bundle/engine` with profile loading; no behaviour change. Acceptance: re-run `merge.py` + `verify_merge.py` from the profile on the existing KOTOR data and get byte-identical `master_findings.json` / `verified_findings.json` (existing data is the regression oracle, $0).
3. **S-C driver `dr.py`** (`init/status/next/run/gate/estimate/report`, state.json, lock/pid files) + `dr selftest`.
4. **S-D fixture + stub `claude`** (the fix session already wrote a stub): a 6-file fixture repo with seeded bugs and a decoy; `dr selftest` runs S1-S14 end to end with the stub, $0. Acceptance: coverage PASS, seeded bugs found, decoys refuted.
5. **S-E SKILL.md + references**; run `python3 scripts/check_skills.py` -> PASS (project option).
6. **S-F fix half merge:** after the other session's Phase B pilot report lands, port `fix/` into the bundle (don't extract it before it has run live once).
7. **S-G live pilot on a different small range** (e.g. 10 commits, ~$1) to prove repo-agnosticism; then done.
Wall estimate: S-B..S-E ~3-4 h of Sonnet-worker time with 2-3 parallel workers; S-F ~1 h after pilot; S-G ~30 min.

## 8. Decisions (answered 2026-10-10)
1. Location: **global user skill** `~/.claude/skills/deep-haiku-review/` (no project-specific logic; repo knowledge only via `profile.toml`).
2. **One skill** automating audit/review AND fix/review stages (human gates between).
3. **Audit half first**; fix half after the other session's Phase B pilot runs and lessons are in.
4. (implied by 1) bundle is the source of truth; `.claude/review/` in KOTOR II is left untouched while the other session uses it.
5. Live pilot: APPROVED again by the user (2026-10-10, hard $1 ceiling; an earlier duplicate message was sent in error but the approval stands). Runs after R9 (dr.py edits) finishes. Profile: `.claude/deep-review/pilot/profile.toml` (base 902b770, last 5 commits, committed only; est. $0.6-0.9).
Build status: spec + loader in `~/.claude/skills/deep-haiku-review/bundle/` (no SKILL.md yet, so the skill is not discoverable until finished).

## 8b. Original questions (kept for reference)
1. Location: A project skill (recommended start), B user skill, or C plugin?
2. One skill with a hard human gate between audit and fix (recommended), or two skills (`deep-haiku-review`, `deep-haiku-fix`)?
3. Extract the audit half now and the fix half only after the other session's Phase B pilot report (recommended)?
4. After the regression check, should the bundle become the single source of truth and `.claude/review/` shrink to run output + `profile.toml` (recommended), or keep both copies?
5. OK to spend ~$1 on a live pilot (S-G) plus the S-D stub tests at $0?

## 9. Sub-agent sizing rules (user request 2026-10-10: smaller agents, more of them)
Round 1 used 3 broad workers (A prep/Map, B merge/verify, C driver); all hit high token use because each owned ~10 files plus a regression diff.
Rules for every later round (also in memory `feedback-narrow-subagents`):
1. One deliverable per agent: <= 3 files / ~1,500 lines written, one verification step, report <= 200 words.
2. Never read a source file > ~600 lines whole; prompts give function names or line ranges.
3. Regression = its own agent, driven by a compare script that prints counts + first N diffs only.
4. Disjoint write scopes; agents list problems in other agents' files instead of fixing them.
5. Planned round 2 (after reports): R1 prep.py vs original (hunks/coverage), R2 run_map/reduce/canary dry-run, R3 merge.py regression, R4 verify_merge regression, R5 dedupe+decoy review, R6 driver selftest on the real engine, R7 SKILL.md + references (docs only).
