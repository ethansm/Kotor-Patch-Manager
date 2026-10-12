---
name: next-dev-work-item
argument-hint: "[WI-NN | audit | status | backlog ...]"
description: >-
  /next-dev-work-item [WI-NN | audit | status]: claim the next eligible dev work item from .claude/work_items/dev_work_items.md, run it through Haiku worker, checker and docs waves, commit only that item's paths, report, then stop so the session can be cleared. Safe to run in several sessions at once (claims, locks, dependency order). Use for "next work item", "next dev work item", "run the next WI", "audit the work item loop".
---

# Next Dev Work Item

One fresh session = one finished, committed work item. You are the orchestrator (Sonnet); every worker, checker and docs agent is Haiku (`model: haiku`). A Sonnet worker is allowed only after one Haiku retry has failed, and `wi_driver.py` already schedules that retry. This overrides the Opus-lead note in plan-handoff. Never read the per-item detail files under `.claude/work_items/` or the source files yourself: the driver prints what you need and the workers read the rest. The one exception is the rescue route (Section 1a): read only the recon OUTPUT files that `rescue --collect` lists.

User input: $ARGUMENTS

## 0. Route
- Empty or `WI-NN`: do Section 1.
- `audit`: run `python3 scripts/wi_driver.py audit --apply` (no confirmation needed) and show the findings plus the commit it prints (max 5 findings).
- `status`: run `python3 scripts/wi_driver.py status` and stop.
- `backlog ...`: pass the arguments to `python3 scripts/wi_driver.py backlog` (add, list, promote).

## 1. The loop (8 calls)
Run each command with Bash from the repo root. Each prints the next command under `THEN:`; follow it. Spawn exactly the agents it prints (`SPAWN 1 agent ...`), with the printed description and prompt, and nothing more (the rescue route prints up to 7 recon spawns at once).

1. `python3 scripts/wi_driver.py next [WI-NN]`: claims the item and prints one Haiku worker spawn. If it prints `NOTHING CLAIMABLE`, show its reasons (max 5 lines) and stop. A `STALE claim` line is the user's call: offer `next WI-NN --adopt` or `release WI-NN --force`, never take it silently.
2. Spawn the worker. The driver already gave you the Agent description and prompt.
3. `python3 scripts/wi_driver.py verify WI-NN`: checks the worker reply and scope, runs the gate (a SKIP or an empty test filter is a FAIL). On failure it prints one `RETRY` spawn (do it, then run `verify` again) or `PARKED`. A `PARKED` that prints `THEN: ... rescue` goes to Section 1a; any other `PARKED` goes to step 8.
4. Spawn the Haiku checker it printed.
5. `python3 scripts/wi_driver.py docs WI-NN`: marks plan rows with plan_manager and may print a docs-worker spawn. A REFUTED checker verdict triggers the same single retry.
6. Spawn the docs worker only if one was printed.
7. `python3 scripts/wi_driver.py close WI-NN`: commits only this item's paths plus the state files. `HARD STOP` means an in-scope path was dirty before the claim: tell the user, do not force.
8. `python3 scripts/wi_driver.py report WI-NN` (on the rescue route, only once its plan is decided): paste its Saved State block, then write the other three sections of the End-of-Step protocol (Actions Completed, What Now Works, Agents Used) in the ADHD format with one next action under two minutes.

If a step fails twice or the item cannot be finished: `python3 scripts/wi_driver.py park WI-NN --reason "<one line>"`. Parking records the reason, frees the claim, and never reverts files. A park caused by a worker `STATUS: FAIL` continues in Section 1a instead of ending the turn.

## 1a. Rescue route (worker FAIL parked the item)
Runs once per item, only when `PARKED` printed `THEN: python3 scripts/wi_driver.py rescue WI-NN`. Nothing is edited before the plan is approved.
1. `python3 scripts/wi_driver.py rescue WI-NN`: writes up to 7 recon briefs and prints one `SPAWN` per brief. It refuses a non-FAIL park and a second run.
2. Spawn all printed recon agents in ONE message, all `model: haiku`, exactly as printed.
3. `python3 scripts/wi_driver.py rescue WI-NN --collect`: flags missing or malformed replies (respawn only those, then collect again) and prints the reading order.
4. Read only the OUTPUT files it lists.
5. Run `/plan-handoff WI-NN`. It enters plan mode and writes a self-contained plan. The plan picks exactly one outcome: split into sub-items (`backlog add`), a docs-only closure (`close WI-NN --manual --paths ... --reason ...`), a defer to an existing item, or a code plan whose steps touch at most 3 files each.
6. Stop at the approval gate. After approval the work runs through the normal loop or the manual close; then `report`.

## 2. Backlog and audit
- A worker reply whose OPEN field names real follow-up work: push it with `python3 scripts/wi_driver.py backlog add --title "..." --why "..." --origin WI-NN`. New items start on hold; the user promotes them.
- `report` prints `AUDIT DUE` every 5th finished run. Then run `python3 scripts/wi_driver.py audit --apply` before ending the turn (after the rescue route ends, if one ran), without asking the user, and list the findings plus the audit commit SHA.

## 3. Hard rules
- Never install, hot-swap, play-test, push, post to GitHub, or run `git add -A`, `git commit -a`, `git reset`, `git checkout`, `git stash`. The driver does the only git writes, scoped to explicit paths.
- Other sessions' dirty files are none of your business: report the count, never touch them.
- Do not use `oc-subagent` or `oc-flow`. Do not spawn more agents than the driver prints (at most 7 recon spawns from `rescue`).
- After the report, stop (on the rescue route: stop at plan approval). The next item starts in a fresh session after `/clear`.
