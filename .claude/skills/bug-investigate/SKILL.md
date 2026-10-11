---
name: bug-investigate
description: >-
  Investigate a KOTOR II player bug report blind, from the mod logs alone: build a scrubbed evidence bundle, run
  skeptic/advocate/evidence hypothesis units plus a "not a bug" null unit through the swarm runner (stub engine
  only for now), quote-check every citation, verify the survivors, and write a ranked report for a human to review.
  Use when a player reports a bug (crash, wrong behaviour, UI glitch, "it did X instead of Y") and mod logs exist,
  or when the user says "bug investigate", "blind investigation", "triage this report", or "what do the logs say
  about this bug". Not for reading raw logs by hand (use kotor2-log-analysis) or for crashes of the game process
  (use kotor2-crash-triage).
---

# Blind bug investigation

`scripts/bug_investigate.py` runs the pipeline. It is deterministic code: every model call goes through one function that shells out to `swarm_run.py`, and every claim a model makes is checked mechanically against the bundle. Status: paid pilot not yet run. Only the `stub` engine works.

## 1. When to use

A player (or the user) reports a bug and you have mod logs from the session. You want ranked, log-grounded hypotheses without anchoring on source code or docs. The output is a report for a human, never a fix.

## 2. Pipeline

Run data lives under `~/.cache/bug_investigate/` (a `--run` dir inside the repo is refused). `$RUN` is the dir that `new` prints.

```bash
python3 scripts/bug_investigate.py new --kind logic --symptom "what the player saw" --log force_power_hotbar_log.txt@1 --mod action-hotbar
python3 scripts/bug_investigate.py bundle --run $RUN
python3 scripts/bug_investigate.py blind --run $RUN --engine stub --cap-total 1.00
python3 scripts/bug_investigate.py null --run $RUN --engine stub --cap-total 1.00
python3 scripts/bug_investigate.py check --run $RUN
python3 scripts/bug_investigate.py verify --run $RUN --engine stub --cap-total 1.00
python3 scripts/bug_investigate.py report --run $RUN
```

- `new`: `--kind` is crash, logic, visual or ux. `--case FILE` replaces `--symptom` and `--kind` and turns on the forbidden-term check. `--run DIR` is optional (default `<stamp>-<kind>` under the cache root).
- `bundle`: runs `scripts/playtest_bundle.py` (the only code that ever reads a log). `bundle --run $RUN --from FILE` imports a prebuilt bundle after verifying its hashes; a tampered bundle exits 2.
- `blind`: three units (`blind-skeptic`, `blind-advocate`, `blind-evidence`), same excerpts, different working style.
- `null`: one unit (`null-1`) that tests "is this a bug at all?".
- `check`: reads unit state only through the runner's `status --json`, quote-checks complete units with `scripts/evidence_quote_check.py`, writes `check.json`. Incomplete units (even with a stale findings file) are listed, never read.
- `verify`: one unit per hypothesis whose citations all matched; a quote-valid `rejected` verdict drops the hypothesis.
- `report`: writes `report.json` and prints a summary. Exit 1 while a blind or null unit is incomplete. `--json` prints the full report.
- Stub runs need `--stub-script FILE` (scripted answers per unit id) and usually `--backoff-scale 0`. The runner path is `--swarm-run PATH` and goes before the subcommand.

## 3. Decisions in the report

- `needs_human_repro`: kind visual or ux. Hypotheses are still listed, but logs cannot settle it.
- `likely_not_a_bug`: the null unit says `not_a_bug` with every citation quote-valid and no hypothesis survived.
- `ranked`: at least one supported hypothesis, ordered by distinct valid evidence ids, then confidence, then id.
- `no_supported_hypothesis`: nothing survived and the null unit did not clear the symptom. `incomplete`: a unit has not finished.
- A hypothesis is dropped if any citation is fabricated or missing. A null verdict with a bad citation is `null_valid: false` and never counts.

## 4. Roles (`.claude/skills/bug-investigate/roles/`)

`blind`, `null` and `verify` each have `role.json`, `system.md` and `schema.json`. They declare no tools, so a unit sees only the excerpts in its prompt: no files, no code, no docs, no repo history. That is the point. A model that could grep the source would cite the answer instead of reasoning from the logs, and the pilot could not tell luck from signal. `blind` and `null` use haiku, `verify` uses sonnet.

## 5. Human gate

Every report ends with "a human must review this report before any fix is started". Read `report.json`, open the cited excerpts, and decide. Never start a fix, commit, or patch edit straight from a report. The case's `true_root_cause` is ground truth for a human scoring a pilot; the orchestrator never reads it.

## 6. Safety

- Stub only. `--engine real` exits 2 before anything is written, and stays disabled until a paid canary batch has a human go.
- Never `cat`, `tail` or `grep` a raw `*_log.txt`. The bundle builder is the only log reader; it scrubs paths and caps size.
- No environment variable picks a binary or the runner. `--swarm-run` is explicit.
- With a case, a unit prompt containing any `blind_forbidden_terms` entry is refused (case-insensitive) and nothing runs.

## 7. Case file

`.claude/skills/bug-investigate/cases/hotbar_charkey.json` is the pilot case (action hotbar: "Character not identified yet"). Limitation: no current-name log predates the fix, so the pre-fix evidence is partial (one legacy log, one party member). Score a pilot on the mechanism (invalid creature id, empty character key); `insufficient` is a correct answer for the multi-member part.
