---
name: plan-handoff
argument-hint: "[doc <N> | <phase> | direction, e.g. '25 §12', 'T5e', '24:T6 verify']"
description: >-
  /plan-handoff [shorthand | direction]: persist session state to the plan's named handoff file, then enter plan mode and write a self-contained plan for "Clear context and approve". Use for "plan handoff", "persist context", "hand off and plan", "clear context".
---

# Plan Handoff & Plan Mode Transition Skill

Use when context is bloated or a course correction is needed. Order matters: ALL file writes happen before `EnterPlanMode`.

## 0. Direction & Shorthand
User input: $ARGUMENTS

- `25 §12`, `doc25:12`: resolves the doc file; `§12` is recorded as direction text only (it does not select a phase).
- `24:T6`, `T5e`, `M0`: resolves the plan holding that phase; leftover text becomes the direction.
- `19e`, `25`: doc number. `<plan_stem>` or a repo-relative path also work. Ambiguous matches (two `PHASE4_PLAN.md`) are refused with a list: pass the repo-relative path.
- Empty `$ARGUMENTS`: pick the plan and phase from THIS session's work (`python3 scripts/plan_manager.py resume <doc>` lists the next open phase). Never run a bare `handoff`: `.claude/active_plan.txt` is shared with other sessions and the script refuses to guess from it.

## 1. Model Tiers (Haiku first)
- **Haiku 5.5 (default for every delegated task)**: parallel `model: haiku` Agent subagents for scouting, recon, drafting cards, verification and mechanical edits. One deliverable, <=3 entries, task budget <=60k tokens (hard ceiling 100k). Single pass by default; the multi-layer Haiku ladder is opt-in (measured ~5x cost, no gain).
- **Opus (lead, thin)**: Section 0, spot-checks, gates, user-facing text, commits, escalation decisions, anything touching live game files or shared state. Opus does not read the repo to write cards.
- **Sonnet**: escalation only: retry once on Haiku with the failure excerpt, split the task, and only then one Sonnet.
- **Inline (no agent)**: `git status`, one test command, one-file lookups, `mark-done`/`handoff`.
- Caps: read-only recon <=12 parallel; write workers <=6 parallel with disjoint Write Scopes; >12 units in a wave: print the estimated token total first. The "1-3 parallel" cap is for Opus agents only.
- RE and address-DB matches: Haiku output is a sieve (`ghidra_knowledge/RE_AGENT_GUIDE.md`), the Opus gate decides.
- Recipe, budget formula and tooling: `patch_manager_mods/SUBAGENT_ROLES.md` "Claude Code Haiku swarm". No `oc-*` backends and no swarm runner real engine.

## 2. Step 1: Persist State (handoff is the LAST writer)
1. Name the plan: its repo-relative path (`<plan>`), taken from the work done this session, not from `active_plan.txt`.
2. If a phase landed, record it WITHOUT regenerating the handoff:
   `python3 scripts/plan_manager.py mark-done <ID> --state <STATE> [--commit <sha>] --notes "<summary>" --no-handoff`
   - Use the exact row ID (`T5e`, not `T5`). `--state DONE` only when committed and verified; `PARTIAL` mid-phase; skip this step when nothing landed.
   - `INSTALLED` counts as done for agents; a pending play test is the user's to report, never an agent task.
   - It exits 1 and changes nothing when no row matches. Do not retry with free text.
3. Write the handoff last: `python3 scripts/plan_manager.py handoff --plan <plan> "<direction>"` (direction = the user's text verbatim, or `Execute <ID>: <name>`). Plain `mark-done` without `--no-handoff` would rewrite it.
4. Verify: the output line reads `Plan-scoped handoff written to <path> for plan <plan>`. If the plan is not the one you meant, STOP and tell the user; do not write the handoff by hand or retarget it.
5. Append runtime lessons under `## 5. Dead Ends & Verification (manual)` in that same file with Edit (kept across regeneration): failed strategies and why, plus exact verification commands.
6. `--dry-run` on `handoff` previews the path and text without writing.

## 2b. Step 1b: Evidence Wave (BEFORE EnterPlanMode; plan mode blocks subagent writes)
Skip for trivial plans (<=2 tasks, files already known). Otherwise build the evidence with Haiku waves; wave id = `w<N>-<plan prefix>-<phase>`, e.g. `w1-24-H1`. Artifacts live untracked under `.claude/waves/<id>/`.
1. **W0 Scout** (1-3 Haiku, read-only): input `python3 scripts/plan_manager.py phase-text <plan> <ID>` (capped ~12k tokens). Output TSV `task<TAB>entry (path or path:lo-hi)<TAB>question`, one row per planned task.
2. **W1 Recon** (N Haiku, <=12 parallel): `python3 scripts/wave_tools.py new <wave> --kind recon`, then per unit `wave_tools.py add <wave> --id uNN --task "<question>" --entries <entry>...`. It runs the budget gate (exit 3 prints line windows from `scripts/wave_budget.py windows <path>`: re-add with those), writes `uNN.brief.md`, prints the Agent `description` and one-line `prompt`. Launch each as `model: haiku`, `subagent_type: general-purpose`. Each writes a facts file (<=60 lines): exact signatures, 5-10 line snippets, line anchors, existing tests, hazards.
3. **W2 Draft** (one Haiku per task): reads its facts files plus the phase text and writes one worker card (template below) to `.claude/waves/<id>/cards/<task>.md`. Use a separate `--kind worker` wave.
4. **W3 Check**: `python3 scripts/wave_tools.py check <wave>` (reply contracts; prints `RERUN:` ids) then `python3 scripts/wave_cards.py verify .claude/waves/<id>/cards` (free: paths, anchors, scopes disjoint, budget). Then one Haiku per card for a semantic cross-check against the phase text. Failures: rerun once on Haiku with the failure excerpt, then split, then one Sonnet.
5. **Opus gate**: read the one-line-per-unit check table, spot-check the 2-3 riskiest cards on disk, write Section 0, order the execution waves. Never pass long briefs through Opus context: briefs are files, agent prompts are one line.
6. After a live wave run `python3 scripts/wave_tools.py report <wave>` (estimate vs actual `peak_ctx`; any peak >60k is a failure and prints the new `WAVE_OVERHEAD_TOKENS`).
7. In plan mode, extra recon uses read-only agents that reply inline (no files).

## 3. Step 2: Switch to Plan Mode
Call `EnterPlanMode` (load via `ToolSearch` `select:EnterPlanMode` if deferred). Write the plan, then present it with `ExitPlanMode`: that call is the deliberate approval step ("clear context and apply"), not an early exit. No repo writes after `EnterPlanMode` except the plan file.

### Opus Task Decomposition Rules
1. **Section 0 Post-Clear Memory**: embed branch, dirty files, resume command, handoff path and the "Do NOT Attempt" list so they survive the wipe.
2. **Discrete scopes**: cards come from the Step 1b wave (drafted by Haiku, gated by Opus); each task <=3 entries, one verification command, Write Scopes disjoint within an execution wave.
3. **Explicit file targets**: exact paths and symbols; no open-ended searches by workers.
4. **Self-verification gates**: every task names its verification command (compiler, test, audit).
5. **Output contracts**: the plan read by the user follows the Output style rules in `~/.claude/CLAUDE.md` (lead with actions, numbered steps, lists capped at 5, concrete time estimates). Subagent briefs instead end with `python3 scripts/agent_contract.py print worker` (edits) or `print recon` (read-only); subagents never use the ADHD format. Run `python3 scripts/agent_contract.py snapshot` before launch and `python3 scripts/agent_contract.py check --scope <globs> -` on the reply before `mark-done`.
6. **Master plan alignment**: session plans cover only the assigned batch; the two-stage logging lifecycle lives in the master plan.
7. **Zero-read plans**: inline the 5-10 line struct/macro snippets the executor needs; never "read LargeHeader.h first".

### Plan Structure Template
```markdown
# Implementation Plan: [Goal Name]

## 0. Preserved Context & Anti-Patterns (Post-Clear Memory)
- **Next-Phase Direction**: [user's /plan-handoff text, verbatim, or "inferred"]
- **Active Branch & Worktree**: [Path / Branch]
- **Plan**: `<repo-relative plan path>`
- **Resume**: `python3 scripts/plan_manager.py resume <repo-relative plan path>`
- **Handoff**: `<path printed in Step 1.4>`
- **Current State**: [Summary of verified progress]
- **Dirty Files**: [List of modified paths]
- **Anti-Patterns & Traps**:
  - [Failed Approach]: [Reason it failed]

## 1. Modular Task Units (worker cards, drafted by Haiku in W2)

### Task 1: [Component / Unit Name]
- **Contract**: worker | recon
- **Write Scope**: `path/glob/*` (disjoint from every other task in the wave)
- **Target Files**: `path` or `path:lo-hi` (<=3 entries, each <=12k tokens, total est <=60k)
- **Objective**: [Specific change]
- **Data / Inlined Snippets**: [5-10 line struct/signature/macro text with `path:line` anchors]
- **Guardrails**: [What NOT to do]
- **Verification**: `[one exact command]`

### Task 2: [Component / Unit Name]
- ...

## 2. Wave Schedule (for the post-clear executor)
- **Write waves**: <=6 Haiku workers each, disjoint Write Scopes; `agent_contract.py snapshot` before, `check --scope <globs> -` after.
- **Verify wave**: Haiku agents run the scoped test/build commands and report counts only.
- **Opus gate**: `git diff --stat`, then <=3 files at a time; spot-check the top 3 claims on disk. Commit by explicit paths.
- **Escalation**: retry once with the failure excerpt, split, then one Sonnet.
- **Waves dir**: `.claude/waves/<id>/` (cards, facts); the handoff lists it.

## 3. Final Integration & Regression Verification
- [ ] Comprehensive verification command
```

## 4. Step 3: Present for User Review
Present the plan so the user can select **`Clear context and approve (bypass mode)`**. After the clear, the fresh session starts from the Resume line in Section 0.
