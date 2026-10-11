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

## 1. Model Tiers
- **Opus (lead)**: synthesis, plan, decomposition, review, gates, commits.
- **Haiku swarm (default workers and recon)**: parallel `model: haiku` Agent subagents, narrow briefs (1 deliverable, <=3 files).
- **Sonnet**: only after three failed Haiku layers.
- **Inline**: trivial work (git status, 1-3 files, one test command). Never spawn subagents for it.

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

## 3. Step 2: Switch to Plan Mode
Call `EnterPlanMode` (load via `ToolSearch` `select:EnterPlanMode` if deferred). Write the plan, then present it with `ExitPlanMode`: that call is the deliberate approval step ("clear context and apply"), not an early exit. No repo writes after `EnterPlanMode` except the plan file.

### Opus Task Decomposition Rules
1. **Section 0 Post-Clear Memory**: embed branch, dirty files, resume command, handoff path and the "Do NOT Attempt" list so they survive the wipe.
2. **Discrete scopes**: modular tasks for Haiku subagents.
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

## 1. Modular Task Units (for Haiku Subagents)

### Task 1: [Component / Unit Name]
- **Contract**: worker | recon
- **Write Scope**: `path/glob/*` (validator rejects edits outside it)
- **Target Files**: `path/to/file.ext`
- **Objective**: [Specific behavioral change]
- **Data / Offsets**: [Hex addresses, struct layouts, formulas]
- **Guardrails**: [What NOT to do]
- **Verification**: `[exact validation command]`

### Task 2: [Component / Unit Name]
- ...

## 2. Final Integration & Regression Verification
- [ ] Comprehensive verification command
```

## 4. Step 3: Present for User Review
Present the plan so the user can select **`Clear context and approve (bypass mode)`**. After the clear, the fresh session starts from the Resume line in Section 0.
