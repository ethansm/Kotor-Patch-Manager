---
name: variant-fix-wave
argument-hint: "[plan <items.tsv> | run <W-A> | status <W-A> | apply <W-A> | close <W-A>]"
description: >-
  Fix a batch of known defects or test gaps in parallel with the FX1-FX3 ladder: Haiku variant proposals, a Sonnet
  consolidator per item (own worktree), an independent Sonnet reviewer, then Opus applies through gates in conflict
  order and commits. `scripts/fix_wave.py` plans batches (refuses oversized ones), writes briefs, tracks stages,
  gates each apply and sums cost. Use for "variant fix wave", "fix these items in parallel", "same escalation as the
  Gemini fixes", "Haiku variants then Sonnet consolidate".
---

# Variant Fix Wave

You are the Opus orchestrator. You brief, relay and apply; agents do the fixing. User input: $ARGUMENTS

CLI: `python3 scripts/fix_wave.py plan|brief|status|apply|cost ...` (`--help` on each). Templates and caps live
next to this file: `variants.md`, `consolidate.md`, `review.md`, `limits.toml`.

## Input: items TSV
One row per fix unit; tab-separated, `;` lists. Columns: `id kind size group after fids write tests mods entries
anchors constraint context doc`. `kind` = bug|gap, `size` = S|M|L (empty = M), `group` = conflict group (`-` = free),
`after` = ids this unit follows, `entries` = `path:lo-hi` read windows, `context`/`doc` = `path#heading`. Paths are
repo-relative; `patch_manager_mods/` is implied unless a path starts with `scripts/`. Example:
`scripts/testdata/fix_wave/fx3_items.tsv`. Variants: bug S = A; bug M/L = A,B,C; gap S/M = T2; gap L = T1,T2.

## Route
### `plan <items.tsv>`
1. `python3 scripts/fix_wave.py plan <items.tsv> --wave <W> [--findings F --cols C]`.
2. Show the batch table (batch, units, agents, est $, ids). Exit 2 = a group over a cap or a bad `after`: report it,
   do not raise caps on your own.
3. Stop. The user picks the batch. One batch per session; the next batch is the next plan phase after /clear.

### `run <W-A>`
1. `python3 scripts/fix_wave.py brief <W> --batch A`. Exit 3 lists rejected units: fix the row (entries too big) or
   drop it with the user's OK; never launch a partial batch silently.
2. Haiku wave: `agent_contract.py snapshot`, then one `model: haiku` Agent per `uNN.brief.md` (description
   `VW haiku <id>-<V>`), in one message, up to the free slots `status` reports.
3. Loop on `python3 scripts/fix_wave.py status <W-A>` (one line per item: `id stage next-action`):
   - `cons-ready`: `status <W-A> --mark <id> cons`, then a `model: sonnet` Agent with `isolation: worktree`, brief =
     `<id>/consolidate.md` (description `VW cons <id>`).
   - `rev-ready`: `--mark <id> rev`, then a fresh `model: sonnet` worktree Agent, brief = `<id>/review.brief.md`
     (description `VW rev <id>`). Never reuse the consolidator's agent or worktree.
   - `revise`: `status <W-A> --revise <id>` (archives round N), then relaunch the consolidator with its brief plus the
     reviewer's issues from `review.rN.md` as binding numbered checks.
   - `park`: second REQUEST_CHANGES. Record it in the handoff; no more agents for it.
   - `apply-ready`: go to step 4.
4. Apply in group order (`after` first): `python3 scripts/fix_wave.py apply <W-A> <id>`.
   - Exit 4 `REBASE`: send the consolidator one rebase request; a second conflict parks the item.
   - Exit 5: a gate failed; read the named gate output, `git apply -R` as printed, send it back to the consolidator.
   - Exit 0: run the one printed close command (`wi_driver.py close ...` or `git commit ... -- <paths>`), then
     `fix_wave.py apply <W-A> <id> --commit <sha>`.
5. When every item is `applied` or `park`: go to `close`.

### `status <W-A>` / `apply <W-A>`
Run the CLI subcommand and report its lines; nothing else.

### `close <W-A>`
1. `python3 scripts/fix_wave.py cost <W-A>` (current session only; add `--session <id>` per extra session).
2. Append one yield line to memory `feedback-variant-wave-yield.md`: wave, items applied/parked, agents, $ per item.
3. `/plan-handoff <plan>` last, then the 4-section output (`### Actions Completed`, `### What Now Works`,
   `### Agents Used`, `### Saved State`).

## Hard rules
- Ladder: Haiku variants → Sonnet consolidator (`isolation: worktree`) → independent Sonnet reviewer (fresh
  worktree). Opus only briefs, relays and applies. This Sonnet use is a sanctioned exception to the SUBAGENT_ROLES
  "Sonnet = escalation" rule, at the user's request (10-15).
- One batch per session. Batches come from `plan`; never hand-merge two batches.
- At most `max_inflight` (20) agents in flight; take free slots from the first `status` line.
- After launching, do not read replies per notification. Run `status` when about 5 notifications have arrived or
  slots are free. Save a reviewer reply to `<id>/rev.reply.txt` only when `status` cannot find its review.md.
- One revision per item. The 2nd REQUEST_CHANGES parks it.
- Opus inline nits after APPROVE_MINOR: at most 5 lines, test-only or log-text-only, listed in the commit body.
  Anything else goes back to the consolidator.
- Agent descriptions start `VW haiku <id>`, `VW cons <id>`, `VW rev <id>` so `cost` can bucket them.
- Commit with explicit paths only (`git commit -- <paths>`); peer sessions share the worktree.
- No installs, pushes or GitHub posts from this skill.
