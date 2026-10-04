---
name: plan-handoff
argument-hint: "[next-phase direction, e.g. 'B10 Steam DB batch, skip GOG']"
description: >-
  Usage: /plan-handoff [next-phase direction]. Optional text after the command sets the goal of the new plan. Persists session context, active scratchpad, working hypotheses, dead ends, and git state to disk, then switches into built-in plan mode. Instructs Opus to decompose the new implementation plan for Sonnet subagents, designed for 'Clear context and approve (bypass mode)'. Use when the user says "plan handoff", "execute plan handoff", "persist context", "hand off and plan", "clear context", or "switch to plan mode and break down the plan".
---

# Plan Handoff & Plan Mode Transition Skill

Use this skill when running in bypass mode and context window bloat is reached, or when a course correction is needed.

## 0. Next-Phase Direction (user arguments)
User direction for the next phase: $ARGUMENTS

- If direction text is present above, it is the goal of the new plan. It overrides your own guess about what comes next.
- Copy it verbatim into the handoff file under `## 0. Next-Phase Direction`, and into plan Section 0 as **Next-Phase Direction**.
- If it is empty, infer the next phase from the session and say in the plan that it was inferred.

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
5. **Output Contracts**: The plan itself, read by the user, follows the `i-have-adhd` rules (lead with actions, number steps, cap lists to 5, concrete time estimates). Subagent prompts instead end with the CLAUDE.md "Subagent Output Contract" 5-line schema; subagents never use the ADHD format.

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
