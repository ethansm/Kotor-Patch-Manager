# Plan VW: `variant-fix-wave` skill + `scripts/fix_wave.py` (with a batch-size limiter)

## Context
FX1-FX3 (Gemini review fixes, 2026-10-15) proved a ladder: Haiku variant proposals, then a Sonnet consolidator per item (worktree), then an independent Sonnet reviewer, then Opus applies in conflict order and commits. Every step except the agents was done by hand, using a generator copied three times (`.claude/reviews/gemini_20261014/tools/fix{1,2,3}_briefs.py`).

FX3 was too big: 23 units, 107 agents, $26.22. Opus context peaked at about 409k and needed a 2nd session. FX2 was the sweet spot: 11 units, 56 agents, $9.08, about 30 min.

Goal: a reusable repo skill whose script makes those steps deterministic and **refuses oversized batches**.

Research (5 Haiku recon, 10-15) is persisted in memory `kotor2-variant-fix-wave-skill.md`. Measured per agent: Haiku $0.045, consolidator $0.62, reviewer $0.34. Revisions add about 30%.

## 0. Preserved context and anti-patterns
- Worktree `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`, branch KOTOR-II. Peer sessions share it, so commit with `git commit -- <paths>` only.
- Plan copy: `.claude/plans/variant_fix_wave_skill.md`. Handoff: `.claude/handoffs/variant_fix_wave_skill.handoff.md`. Resume with `python3 scripts/plan_manager.py resume .claude/plans/variant_fix_wave_skill.md`.
- **Keep `/home/ethansm/tmp`** until VW.4. Then ask the user before deleting it.
- Do not edit `fix1/2/3_briefs.py` or anything under `.claude/waves/fix*`. The FX3 items TSV is read only, as fixture input.
- No installs, no pushes, no GitHub posts, no oc-* backends. Never view `*credential*` or `*secret*` files.
- Subagents: Haiku workers (`model: haiku`), disjoint write scopes, at most 5 in parallel. Opus reviews, integrates and commits.

## 1. Design (binding)

### 1a. Items TSV
Tab-separated, `;` lists, one row per fix unit. Columns: `id`, `kind` (bug|gap), `size` (S|M|L, empty=M), `group` (`-`/empty = free), `after` (ids), `fids` (keys into `--findings <tsv> --cols a,b,c`), `write`, `tests`, `mods`, `entries` (`path:lo-hi`; first 3 budgeted, rest named windows), `anchors` (rg patterns), `constraint`, `context` (`path#heading`), `doc` (`path#heading`). Paths repo-relative; `patch_manager_mods/` prepended unless the path starts with `scripts/` or `patch_manager_mods/`.

### 1b. Variants
bug S: A · bug M/L: A,B,C · gap S/M: T2 · gap L: T1,T2. Texts = `VAR` in `fix3_briefs.py:25-37`, verbatim into `variants.md`.

### 1c. Batch limiter (`fix_wave.py plan`)
- Limits from `.claude/skills/variant-fix-wave/limits.toml` (max_units 12, max_agents 60, max_cost_usd 15.0, max_inflight 20; rates haiku 0.045, consolidator 0.62, reviewer 0.34, revision_factor 1.3).
- Per unit: `agents = len(variants)+2`, `cost = (len(variants)*haiku + consolidator + reviewer) * revision_factor`.
- Rows sharing a `group` form a block; free row = block of 1. Blocks ordered topologically by `after`, ties by file order. Greedy in-order packing; a block never splits.
- Block alone over a cap: exit 2 `BLOCK <group> over cap (<which>): split the group or raise the limit with --max-*`. Cycle or unknown `after` id: exit 2.
- Output: table (batch, units, agents, est $, ids) + `<wave_dir>/batches.tsv`; wave ids `<wave>-A`, `<wave>-B`. Overrides `--max-units/--max-agents/--max-cost`.
- `brief` refuses without `batches.tsv`; generates exactly one batch (`--batch A`).

### 1d. Subcommands of `scripts/fix_wave.py`
| Subcommand | Module |
|---|---|
| `plan <items.tsv> --wave W [--findings F --cols C] [--max-*]` | `fix_wave_plan.py` |
| `brief W --batch A [--dry-run] [--force]` | `fix_wave_brief.py` |
| `status W-A` | `fix_wave_collect.py` |
| `apply W-A <id> [--dry-run]` | `fix_wave_apply.py` |
| `cost W-A` | `fix_wave_collect.py` |

`brief`: idempotent (`--force` regenerates); exit 3 listing every unit `wave_tools add` rejects; `--dry-run` writes nothing; BASE = `git rev-parse --short HEAD`; build root `/tmp/claude-1000/fixwave-<wave>-<id>`; cons/rev briefs add: scratch KPM dir (`Patches/Common`, `Patches/_shared`, `lib/` → `build_check.py --kpm-dir`), k1 SKIP reported as SKIP, every constraint name addressed or deferred in `verdict.md`; reviewer: any BLOCKING forces REQUEST_CHANGES.

`status` stages: haiku-pending, cons-ready, rev-ready, revise (REQUEST_CHANGES or BLOCKING>0, rev_round<2), park, apply-ready, applied. Copies review.md out of agent worktrees. First line: in-flight count and free slots up to max_inflight.

`apply` gates: stage apply-ready; `git apply --check` → `-3` → markers → `-R`, exit 4 `REBASE <id>: ...`; `run_tests.py <tests>` (+compat_check, check_shared_headers --sync for `_shared_src`); `build_check.py --mod <m> --quiet --build-root ...`; on PASS record `state.tsv`, print one close command (`wi_driver.py close` for `WI-\d+`, else `git commit -m ... -- <paths>`). Never commits.

### 1e. Review contract schema
`agent_contract.py` SPECS `review`: OUTPUT, EVIDENCE, BLOCKING (int), FINDING, DECISION (APPROVE|APPROVE_MINOR|REQUEST_CHANGES); verdict_key DECISION. WARN `LABEL_CONTRADICTS_BLOCKING` when BLOCKING>0 with APPROVE*.

### 1f. SKILL.md (≤110 lines)
Routes `plan` / `run`; hard rules: Haiku → Sonnet consolidator (worktree) → independent Sonnet reviewer; one batch per session; ≤20 in flight; status-driven polling; 1 revision then park; Opus inline nits ≤5 lines test/log-only; `VW haiku|cons|rev <id>` description prefixes; close = cost, memory yield, /plan-handoff, 4-section output.

## 2. Phases
| Id | Batch | Deps | Status |
|---|---|---|---|
| VW.0 | Persist plan, pre-flight | - | DONE |
| VW.1 | Haiku worker wave, 5 parallel (H1 plan, H2 brief, H3 collect, H4 apply, H5 agent_contract) | VW.0 | DONE |
| VW.2 | Opus integrate: CLI, SKILL.md, review of worker diffs | VW.1 | DONE |
| VW.3 | Verify: tests, check_skills, FX3 dry run | VW.2 | DONE |
| VW.4 | Commit, memory, docs, `~/tmp` question, handoff | VW.3 | DONE |
| VS.0 | `next` route: pre-flight, limits max_units 10 | VW.4 | TODO |
| VS.1 | Haiku workers: fix_wave_select.py (select + collect + claims), draft template, collect heartbeat | VS.0 | TODO |
| VS.2 | Opus integrate: fix_wave.py select/draft-collect, SKILL.md `next` route | VS.1 | TODO |
| VS.3 | Verify: suites, check_skills, select --dry-run on the live backlog | VS.2 | TODO |
| VS.4 | Commit, memory, handoff | VS.3 | TODO |

## Verification
- All 7 scoped suites PASS; `check_skills.py` clean.
- FX3 fixture: ≥2 batches, none over 12 units / 60 agents / $15.
- `brief --dry-run` writes nothing; `apply` on a conflicting diff leaves the tree clean.
- Each commit lists only this plan's paths.
