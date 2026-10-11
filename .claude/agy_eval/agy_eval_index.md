# agy (Antigravity, Gemini 3.8 Flash) second-model validator study: index

Question: can Gemini 3.8 Flash, run through the `agy` CLI, act as an independent second voter for the Haiku review swarm, and does it catch different errors at an acceptable false-confirm cost? Outcome is a go / no-go decision for a shadow run, not a claim that Agy adds value.

**Status (10-10): PARKED on quota.** Gemini weekly quota (Flash and Pro share one bucket) was 0.75% with refresh in 73h 18m. No live `agy` calls have been made. Restart at P2 when the quota resets (about 2026-10-13; confirm on the meter).

## Read in this order

1. [agy_eval_session_log.md](agy_eval_session_log.md): what happened, decisions, corrections, deviations, hashes.
2. [agy_eval_approved_plan_2026-10-10.md](agy_eval_approved_plan_2026-10-10.md): the approved plan, verbatim (with a file-name mapping at the top).
3. [../handoffs/agy_flash_swarm.handoff.md](../handoffs/agy_flash_swarm.handoff.md): short resume card.
4. [agy_eval_prereg.md](agy_eval_prereg.md): draft v1 pre-registration (not final until P2).
5. [agy_eval_p1_findings.md](agy_eval_p1_findings.md): static research results and corrections.

## Naming rule

Every document in this folder uses the `agy_eval_` prefix. Do not create bare `plan.md`, `PLAN.md`, `README.md` or `SESSION_LOG.md`. They collide: the repo already has 32 `README.md` files, and `scripts/plan_manager.py` globs `*plan*.md` and `*_plan.md`.

## Phase status

| Phase | Status | Output |
|---|---|---|
| P0 preflight | done | HEAD `78807d4`, agy `1.3.2`, neutral cwd created |
| P1 static research | done | `agy_eval_p1_findings.md` |
| P2 live probes (up to 12 calls) | **held** (quota) | `agy_probe.py` (stub-tested only) |
| P3 quota gate | done | `agy_gate.sh`, stub, `test_agy_gate.sh` (11/11 PASS) |
| P4 pre-registration | draft v1 | `frozen/` (53 prompts), `agy_eval_prereg.md` |
| P5 pilot (agy 76 calls, Haiku 53, cap $0.05) | **held** (needs P2, final pre-registration, user go) | not started |
| P6 analysis and decision | not started | |

## Files

| Path | What it is |
|---|---|
| `agy_gate.sh` | Unwired gate around `agy -p`. Off by default (`SWARM_AGY`). Always exits 0; returns a verdict or `agy_skipped`. |
| `agy_probe.py` | P2 runner. Logs every call to `agy_eval_probe_log.jsonl`. Refuses after 12 starts and after any unacknowledged non-zero call (`--ack` to continue). Removes canary files. |
| `stub_agy/agy` | Bash stub for tests. Never the real binary. |
| `test_agy_gate.sh` | 11-mode stub suite for the gate. |
| `prompts/PROMPT_verify.txt`, `prompts/PROMPT_fix.txt` | Shared instruction text (both models get the same prompt). |
| `make_dossiers.py` | Deterministic builder for `frozen/`. |
| `frozen/verify/`, `frozen/fix/`, `frozen/real/` | Blinded prompt files (53). `frozen/MANIFEST.json` lists hashes. |
| `frozen/KEY.json` | Blinding key. Do not open or print until scoring (P6). |
| `agy_eval_prereg.md` | Pre-registration: sets, call counts, cap, metrics, decision rule, stop rules. |
| `agy_eval_p1_findings.md` | P1 results and corrections. |
| `agy_eval_approved_plan_2026-10-10.md` | Approved plan, verbatim. |
| `agy_eval_session_log.md` | Session record. |

Not present yet (created later, with the same prefix): `agy_eval_probe_log.jsonl`, `agy_eval_p2_findings.md`, `agy_eval_pilot_run.py`, `pilot/`, `agy_eval_analyze.py`, `agy_eval_decision.md`.

## Hard rules

- No `--dangerously-skip-permissions`, `--add-dir`, or `--settings` with agy.
- Run agy only from the neutral cwd `~/.cache/agy_eval/cwd` (must be empty), never from the repo.
- Do not read `~/.gemini/config/config.json` or any token, credential, or auth file.
- Do not edit flow scripts, `CLAUDE.md`, `AGENTS.md`, or `.claude/router_eval/cases.jsonl`.
- Do not stage the dirty tree. Commit only when asked, with explicit paths.
- Egress approved (user, 10-10): frozen prompt text only, no secrets, no tree access.
- Personal AI credits are off. Subscription quota only.

## Resume

Read-only status (no model call): `python3 -I .claude/agy_eval/agy_probe.py status`

Restart P2 only after the user confirms the quota has reset. Then follow the probe table in [agy_eval_approved_plan_2026-10-10.md](agy_eval_approved_plan_2026-10-10.md), section 3 (P2). Record every outcome in `agy_eval_p2_findings.md`.
