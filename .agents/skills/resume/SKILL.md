---
name: resume
description: >-
  Resumes an active master plan or phase in the KOTOR-II workspace.
  Use when the user invokes /resume <phase|doc> or enters resume <phase>.
---

# Master Plan Resume

When invoked with a target plan or phase (e.g. `/resume 34`, `/resume C4`, `/resume U3`, or delegating via `delegate <phase>`):
1. **Turn 1 Execution**: Immediately execute `python3 scripts/plan_manager.py resume <target>` (or `python3 scripts/plan_manager.py delegate <target>` if delegating) via `run_command`.
2. **Bootstrap Context**: Parse the returned active objective, handoff direction, and upcoming batches.
3. **Autonomous Continuation / Delegation**: If subagent delegation is requested or appropriate, execute `python3 scripts/plan_manager.py delegate <target>` or `oc-subagent <role> "<prompt>"` autonomously and monitor completion. Otherwise, proceed directly to execute the next objective in the same turn without asking for confirmation or stalling.
