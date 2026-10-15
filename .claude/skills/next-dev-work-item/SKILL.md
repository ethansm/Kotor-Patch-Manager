---
name: next-dev-work-item
argument-hint: "[WI-NN | batch N | batch WI-NN,WI-NN,... | audit | status | backlog ...]"
description: >-
  /next-dev-work-item [WI-NN | batch N | batch WI-NN,WI-NN | audit | status]: claim the next eligible dev work item from .claude/work_items/dev_work_items.md, run it through Haiku worker, checker and docs waves, commit only that item's paths, report, then stop so the session can be cleared. With no input it does exactly one item. `batch` runs several small items in one session, with parallel workers and checkers. Safe to run in several sessions at once (claims, locks, dependency order). Use for "next work item", "next dev work item", "run the next WI", "batch work items", "audit the work item loop".
---

# Next Dev Work Item

By default, one fresh session = one finished, committed work item. Only an explicit `batch` input (Section 1c) widens that to several items. You are the orchestrator (Sonnet); every worker, checker and docs agent is Haiku (`model: haiku`) except the escalation: after one Haiku retry fails, `wi_driver.py` prints an `ESCALATE` spawn with `model=sonnet` (Sonnet 5.5) for one last worker attempt, and the checker for that attempt is Sonnet too. Only after the Sonnet attempt also fails does the driver park the item. Spawn exactly the model the driver prints. This overrides the Opus-lead note in plan-handoff. Never read the per-item detail files under `.claude/work_items/` or the source files yourself: the driver prints what you need and the workers read the rest. The one exception is the rescue route (Section 1a): read only the recon OUTPUT files that `rescue --collect` lists, and write the decision note `decision_r<N>.md`.

User input: $ARGUMENTS

## 0. Route
- Empty or `WI-NN`: do Section 1 (one item; this is the default).
- `batch N` (N = 2..4) or `batch WI-NN,WI-NN,...` (comma or space separated, 2..4 ids; ids keep their suffix, e.g. `WI-20b`, and ranges are not supported): do Section 1c.
- `audit`: run `python3 scripts/wi_driver.py audit --apply` (no confirmation needed) and show the findings plus the commit it prints (max 5 findings).
- `status`: run `python3 scripts/wi_driver.py status` and stop.
- `backlog ...`: pass the arguments to `python3 scripts/wi_driver.py backlog` (add, list, promote).

## 1. The loop (8 calls)
Run each command with Bash from the repo root. Each prints the next command under `THEN:`; follow it. Spawn exactly the agents it prints (`SPAWN 1 agent ...`), with the printed description and prompt, and nothing more (the rescue route prints up to 7 recon spawns at once: 7 in round 1, 4 in round 2).

1. `python3 scripts/wi_driver.py next [WI-NN]`: claims the item and prints one Haiku worker spawn. If it prints `NOTHING CLAIMABLE`, show its reasons (max 5 lines) and stop. A `STALE claim` line is the user's call: offer `next WI-NN --adopt` or `release WI-NN --force`, never take it silently.
2. Spawn the worker. The driver already gave you the Agent description and prompt.
3. `python3 scripts/wi_driver.py verify WI-NN`: checks the worker reply and scope, runs the gate (a SKIP or an empty test filter is a FAIL). On failure it prints a `RETRY` spawn (Haiku) or an `ESCALATE` spawn (Sonnet); do it, then run `verify` again. A failed checker verdict (PARTIAL/REFUTED) at `docs` follows the same ladder. Only `PARKED` ends the ladder. A `PARKED` that prints `THEN: ... rescue` goes to Section 1a; any other `PARKED` goes to step 8.
4. Spawn the Haiku checker it printed.
5. `python3 scripts/wi_driver.py docs WI-NN`: marks plan rows with plan_manager and may print a docs-worker spawn. A REFUTED checker verdict triggers the same single retry.
6. Spawn the docs worker only if one was printed.
7. `python3 scripts/wi_driver.py close WI-NN`: commits only this item's paths plus the state files. `HARD STOP` means an in-scope path was dirty before the claim: tell the user, do not force.
8. `python3 scripts/wi_driver.py report WI-NN` (on the rescue route, only once its plan is decided): paste its Saved State block, then write the other three sections of the End-of-Step protocol (Actions Completed, What Now Works, Agents Used) in the ADHD format with one next action under two minutes.

Park only when the ladder is exhausted (Haiku, Haiku retry, Sonnet) or the item cannot be finished: `python3 scripts/wi_driver.py park WI-NN --reason "<one line>"`. Parking records the reason, frees the claim, and never reverts files. A park caused by a worker `STATUS: FAIL` continues in Section 1a instead of ending the turn.

## 1a. Rescue route (worker FAIL parked the item)
Runs for up to 2 rounds per item, starting when `PARKED` printed `THEN: python3 scripts/wi_driver.py rescue WI-NN`. Nothing is edited before the plan of each round is approved.
1. `python3 scripts/wi_driver.py rescue WI-NN`: writes up to 7 recon briefs and prints one `SPAWN` per brief. Round 1 writes 7 briefs; it refuses a non-FAIL park. A second run is refused until a new park follows the rescue (or `--residual`, see step 8).
2. Spawn all printed recon agents in ONE message, all `model: haiku`, exactly as printed.
3. `python3 scripts/wi_driver.py rescue WI-NN --collect`: flags missing or malformed replies (respawn only those, then collect again) and prints the reading order.
4. Read only the OUTPUT files it lists.
5. Run `/plan-handoff WI-NN`. It enters plan mode and writes a self-contained plan. The plan picks exactly one outcome: split into sub-items (`backlog add`), a docs-only closure (`close WI-NN --manual --paths ... --reason ...`), a defer to an existing item, or a code plan whose steps touch at most 3 files each.
6. Stop at the approval gate. After approval the work runs through the normal loop or the manual close (allowed on a done item that had a rescue).
7. Write `.claude/research/<wi>_rescue/decision_r<N>.md` (N = round just finished, e.g. `wi03_rescue`): the outcome plus the residual rows, at most 15 lines.
8. Run `python3 scripts/wi_driver.py rescue WI-NN --status` (read-only) and follow its `THEN:` line: `rescue WI-NN` (re-parked on a worker FAIL), `rescue WI-NN --residual` (`[OPEN WI-NN` doc markers or unresolved plan rows), or "no residual work; report" then `report`. Round 2 writes 4 briefs (gap, spec, code, tests), lists the decision note, and ends at the same plan-mode gate via `/plan-handoff WI-NN`.
9. After 2 rounds the driver refuses a third and prints "2 rescue rounds used: stop and ask the user". Stop and ask; do not run `report` or claim "Safe to /clear".

## 1b. Never hand the user a fix you can do yourself
When a park or report would end with a mechanical unblock (a Step that names an edit the source already has, a missing file:line anchor, `unpark`, rerun), do it in the same turn instead of listing it as the user's next action: use `rg`/`sed -n` on just the cited lines to confirm, fix the Steps in the item's detail file under `.claude/work_items/` with Edit (pin file:line; say "already satisfied, no edit" where true), commit that Steps edit alone with explicit paths (`git commit -- <path>`), run `python3 scripts/wi_driver.py unpark WI-NN --reason "<what changed>"`, then restart the loop with `next WI-NN`. Do this at most once per park (not once per item): a checker PARTIAL whose finding cites a Step instruction that the tree already satisfies (a deletion of a line that is gone, an absence the diff cannot show) is a stale-Steps defect, so fix the Steps this way before the ladder moves on, never park over it. Ask the user only when the choice is a design decision, not a mechanical correction.

## 1c. Batch route (only when the input starts with `batch`)
Same driver, same per-item commands as Section 1. The only change is that the spawn steps for all items happen in ONE message, so Haiku workers and checkers run in parallel. Cap: 4 items. More than 4 ids or N > 4: stop and ask the user to trim the batch.
1. Claim: `batch N` runs `python3 scripts/wi_driver.py next` N times (each call claims the next distinct item); `batch WI-a,WI-b` runs `next WI-a`, `next WI-b`. Run these claims one Bash call at a time. An item that prints `NOTHING CLAIMABLE`, or an explicit id that cannot be claimed, is dropped with its reason (max 2 lines) and the batch continues with the rest. A `STALE claim` is the user's call, as in Section 1. If no item was claimed, stop.
2. Spawn every printed worker in ONE message, exactly as printed.
3. Run `verify WI-NN` for each item, one Bash call at a time (the gates share the build). Collect the printed checker spawns and spawn all of them in ONE message. Items whose `verify` printed `RETRY` or `ESCALATE` form a retry group: spawn that group together in ONE message, then run `verify` again for each. Each item keeps its own ladder (Haiku, Haiku retry, Sonnet, park); one item's failure never blocks the others.
4. Run `docs WI-NN` for each item; spawn any docs workers together in ONE message.
5. Run `close WI-NN` for each item, strictly one after another (the commits are serial). `HARD STOP` on one item does not stop the others.
6. Run `report WI-NN` for each closed item. Final answer: one combined End-of-Step protocol, a Saved State line per item (commit SHA each), and one next action under two minutes.
7. Parked items: a park that prints `THEN: ... rescue` is handled only after all other items are closed and reported, one rescue at a time (Section 1a), since the rescue route ends at a plan-mode approval gate. A park with `rescue` unavailable is listed in the final answer with its reason.
8. `AUDIT DUE` is checked once at the end of the batch (Section 2), not per item.
Do not use a batch for items the driver flags as large (size L) or items whose write scopes overlap: if two claimed items share a path, run the second after the first closes.

## 2. Backlog and audit
- A worker reply whose OPEN field names real follow-up work: push it with `python3 scripts/wi_driver.py backlog add --title "..." --why "..." --origin WI-NN`. New items start on hold; the user promotes them.
- `report` prints `AUDIT DUE` every 5th finished run. Then run `python3 scripts/wi_driver.py audit --apply` before ending the turn (after the rescue route ends, if one ran), without asking the user, and list the findings plus the audit commit SHA.

## 3. Hard rules
- Never install, hot-swap, play-test, push, post to GitHub, or run `git add -A`, `git commit -a`, `git reset`, `git checkout`, `git stash`. The driver does the only git writes, scoped to explicit paths.
- Other sessions' dirty files are none of your business: report the count, never touch them.
- Do not use `oc-subagent` or `oc-flow`. Do not spawn more agents than the driver prints (at most 7 recon spawns from `rescue`; the Sonnet escalation is one of those prints).
- After the report, stop (on the rescue route: stop at plan approval). The next item (or batch) starts in a fresh session after `/clear`. Without a `batch` input, never run more than one item per session.
- In a batch, never exceed 4 items and never spawn agents the driver did not print.
