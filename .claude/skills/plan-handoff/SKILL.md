---
name: plan-handoff
argument-hint: "[doc <N> | <phase> | direction, e.g. '25 §12', 'T5e', '24:T6']"
description: >-
  Usage: /plan-handoff [shorthand | direction]. Accepts shorthand doc numbers ('25 §12', '24', '19e'), phase IDs ('T5e', 'T6'), or custom directions. Persists session context, active scratchpad, working hypotheses, dead ends, and git state to disk, then switches into built-in plan mode. Instructs Opus to decompose the new implementation plan for Sonnet subagents, designed for 'Clear context and approve (bypass mode)'. Use when the user says "plan handoff", "execute plan handoff", "persist context", "hand off and plan", "clear context", or "switch to plan mode and break down the plan".
---

# Plan Handoff & Plan Mode Transition Skill

Use this skill when running in bypass mode and context window bloat is reached, or when a course correction is needed.

## 0. Next-Phase Direction & Shorthand Controls
User input: $ARGUMENTS

- **Shorthand Supported:**
  - Doc + Section: `25 §12`, `doc 25 §12`, or `doc25:12` (auto-resolves `patch_manager_mods/25_pc_levelup_filter.md §12`).
  - Phase ID: `T5e`, `T6`, `M0` (auto-resolves to that batch in doc 24 master plan).
  - Doc Number: `25`, `24`, `19e` (auto-resolves to that master plan markdown file).
- If text is present, it is passed directly to `scripts/plan_manager.py handoff "$ARGUMENTS"`.
- If empty, infer the next phase from the active plan and session context.

## 1. Multi-Tier Model & Overhead Policy
- **Opus (Lead Orchestrator)**: High-level synthesis, scratchpad archival, plan formulation, and task decomposition.
- **Sonnet (Worker Subagents)**: Targeted, modular execution of scoped tasks (code generation, refactoring, local bug fixes).
- **Haiku (Reconnaissance Subagents)**: High-volume read-only scans (broad regex, multi-directory greps, large log filtering) to isolate raw tool outputs from context.
- **Inline Execution (No Subagents)**: Minor tasks (checking git status, inspecting 1-3 files, running a single test command). **Never spawn subagents for trivial operations.**

## 2. Step 1: Snapshot and Persist Working State
1. Run `python3 scripts/plan_manager.py handoff "$ARGUMENTS"` directly (or with `--plan <path>` if working on a specific sub-plan). This writes both `.claude/session_handoff.md` and `.claude/handoffs/<plan_stem>.handoff.md` to prevent clobbering concurrent sessions in the same worktree.
2. Append to the handoff file if specific runtime anti-patterns occurred:
   - **Dead Ends & Anti-Patterns (Do NOT Attempt)**: Specific strategies attempted that failed and reasons why (compiler errors, runtime asserts).
   - **Verification Commands**: Exact commands to validate progress.
3. Also update the durable records the fresh session will read: the active project doc's "RESUME HERE" section or memory index.
4. Finish all writes in this step: plan mode blocks file edits.

## 3. Step 2: Switch to Built-in Plan Mode
Switch to Claude Code's built-in plan mode by calling the `EnterPlanMode` tool (load it first with `ToolSearch` `select:EnterPlanMode` if it is deferred). When the plan is ready, present it with `ExitPlanMode`. As the Opus orchestrator, formulate the implementation plan adhering to these rules:

### Opus Task Decomposition Rules:
1. **Section 0 Post-Clear Memory**: The top of the plan must embed the active branch, modified files, and critical "Do NOT Attempt" list from the handoff file so it survives the context wipe.
2. **Discrete Scopes**: Break implementation work into modular tasks for Sonnet subagents.
3. **Explicit File Targets**: Provide exact file paths and symbols; prevent open-ended exploratory searches by workers.
4. **Self-Verification Gates**: Every subagent task must define an explicit verification command (compiler, test, audit).
5. **Output Contracts**: The plan itself, read by the user, follows the `i-have-adhd` rules (lead with actions, number steps, cap lists to 5, concrete time estimates). Subagent prompts instead end with the output of `python3 scripts/agent_contract.py print worker` (edits) or `python3 scripts/agent_contract.py print recon` (read-only); subagents never use the ADHD format. The orchestrator runs `python3 scripts/agent_contract.py snapshot` before launch and `python3 scripts/agent_contract.py check --scope <globs> -` on the reply before `mark-done`.
6. **Master Plan Alignment**: Session plans focus strictly on the immediate assigned batch/tasks. The Two-Stage Logging Lifecycle (verbose dev/probing -> tightening/hardening) is tracked across milestone phases in the **Master Plan**, not forced into every sub-session.

### Plan Structure Template:
```markdown
# Implementation Plan: [Goal Name]

## 0. Preserved Context & Anti-Patterns (Post-Clear Memory)
- **Next-Phase Direction**: [user's /plan-handoff text, verbatim, or "inferred"]
- **Active Branch & Worktree**: [Path / Branch]
- **Current State**: [Summary of verified progress]
- **Dirty Files**: [List of modified paths]
- **Anti-Patterns & Traps**:
  - ❌ [Failed Approach]: [Reason it failed]
- **Handoff Reference**: `.claude/session_handoff.md`

## 1. Modular Task Units (for Sonnet Subagents)

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
Present the plan so the user can select:  
**`Clear context and approve (bypass mode)`**.
