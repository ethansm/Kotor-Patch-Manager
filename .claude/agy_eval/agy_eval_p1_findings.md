# P1 findings (static research, $0, no agy calls)

Status: P1-A, P1-B, P1-C all reported. P1 complete.
Live agy calls: HELD (user reported weekly quota under 1%, 10-10). Zero agy calls so far.

## Corrections to the plan (verified in-repo or in agy docs)

- `verify_merge.py` does NOT exit 1 on a confirmed decoy. Exit 1 is for coverage gaps only (`verify_merge.py:328-334`). A decoy catch shows as a `LOUD FAILURE` line in the report (`:266-268`).
- The claim parser is `.claude/review/tools/record_verify.py`, not `.claude/review/verify/record_verify.py`.
- Real verify findings = 260. The 275 figure includes the 15 gold items (`work/verify_queue.json`).
- agy headless JSON envelope HAS `status`, `structured_output`, `error`, `num_turns`, `usage{input,output,thinking,cache_read,total}` (https://antigravity.google/docs/cli/headless/ section 1). P1-C had listed these as MISSING; that was wrong. Still MISSING: `total_cost_usd` (agy quota is not USD).
- agy exit codes (headless section 2): `0` success; non-zero = no response with reason on stderr; `1` = unknown `--model`, invalid JSON line, missing event field; `2` = control or slash command sent into a streaming session. Auth exit code NOT stated.
- Quota reset period: NOT FOUND in docs. Quota error string: NOT FOUND. The classifier stays UNVERIFIED until P2 captures real output, so quota/auth/rate_limit patterns start empty and unknown failures are `transient`.

## P1-A: agy docs, terms, loaded context

- Quota commands exist: `/usage` ("Display model quota usage.") and `/credits`. The `/credits` setting `useG1Credits` says: "Uses personal AI credits for model calls once plan quotas are exhausted." (External builds only.) **Risk:** if that is on, a quota-exhausted `agy` call could spend personal credits (real money). `useG1Credits` is NOT in `~/.gemini/antigravity-cli/settings.json`. Its location is unverified, and config.json is off-limits. **Needs your check in the Antigravity settings UI.**
- Terms (https://antigravity.google/terms): no CLI-specific automation clause found. Clause 6 bars "third party software, tools, or services to access the Service" (ambiguous for a scripted CLI) and "abuse, harm, interfere with, or disrupt". The vendor headless docs explicitly endorse scripting: "Run Antigravity CLI non-interactively to script agent tasks, integrate with CI pipelines, and capture machine-readable output." **G1 is therefore a judgment call for you.**
- Loaded in `agy -p` (all UNSURE for print mode, since no doc states it):
  - `~/.gemini/config/hooks.json`: loads, can run shell. PreToolUse matcher `*` returns `decision: ask` for every tool call. In print mode with no UI, that may stall or deny tools. Canary probe 6 must check this.
  - `~/.gemini/config/skills/opencode-subagents/SKILL.md`: loads (global root), directs the agent to run `oc-*` bash and to run the whole chain without asking. Confound and a tool-use risk.
  - `~/.gemini/config/plugins/i-have-adhd/skills/.../SKILL.md`: loads only if enabled (enablement in config.json, unread). Its no-preamble and no-closer rules conflict with strict JSON.
  - `~/.gemini/GEMINI.md`: probably does NOT load (not a documented root). If it does, it forces ADHD formatting and a four-section reply, which breaks strict JSON.
- Hook script `~/.orca/agent-hooks/antigravity-hook.sh`: prints JSON and exits 0 unless `ORCA_*` env vars are set. Whether those are set for agy is UNSURE.

## P1-B: gold-set inventory

- Verify gold: 8 decoys, 7 controls (`.claude/review/work/verify_key.json:77`, `:2`).
- `recall` dict keys K1, K2, K3a, K3b, K4: known-issue recall calibration, read by `verify/verify_merge.py:178`. Not a gold-label set.
- Fix gold: 5 seeded-bad (`fix_calibrate.py:49`), up to 3 known-good (`N_GOOD = 3`, `fix_calibrate.py:50`).
- Verdict locations: verify results in `verify/results/<id>.json`; fix blind-review verdicts in `fix/reviews/<ctid>.a1.review.json`.
- Fix-patch dossier builder exists only for the integration batch (`fix/build_integration_batches.py:277`). No per-finding equivalent of `verify/build_dossiers.py`.
- Gold overlap: 2 controls and 4 decoy originals are also real fix findings. No ID collisions.

## P1-C: seam and cost gaps

- Parser needs: `result` envelope; `assistant` events with `usage` and `tool_use` blocks; `structured_output` validated against `verify_schema.json` or `adjudicate_schema.json` (`tools/record_run.py:60-105`, `tools/record_verify.py:33-99`).
- agy stream-json adds `step_update` events with `usage` and `tool_info` (name, parameters, output, error). This may map to tool-call counting; shape is verified by docs only.
- Second-voter slots: `verify_merge.py:113-133` (verdict read and CONFIRMED gate), `build_dossiers.py:92-104` (Haiku block for Sonnet adjudicator), `ladder.py:15-16,59-60` (L2 off, L3 compare judges, L4 escalate). L3 model = `run_compare.sh:28`, L4 model = `run_escalate.sh:87`.
- Shim losses:
  - USD cost: no agy equivalent. Cost caps (`run_verify.sh:147-152`, total $6) would never trip.
  - bwrap: `runlib.sh:52-53` disables HOME-hiding bwrap when `DR_CLAUDE_BIN` is set.
  - Secret denylist: `R/worker_settings.json:4-52` has no agy equivalent (no `--settings`).
  - Tool allowlist: `--tools Read,Grep,Glob` has no agy equivalent.
- Backoff to reuse: `run_verify.sh:103-107`. Regex `429|529|rate.?limit|overloaded`; `2^attempt*15 + RANDOM%15` seconds. The regex may not match agy's failure text.
- Per-stage `CLAUDE_BIN` overrides: bundle stages use `${DR_CLAUDE_BIN:-claude}`. R-side scripts (`run_map.sh`, `run_verify.sh`, `run_adjudicate.sh`, `run_fix_review.sh`, `run_fix_judge.sh`, `run_integrate.sh`, `run_fix.sh`) hard-code `claude`.

## Flags confirmed locally (`agy --print --help`, no model call)

- `-p/--print` (prompt), `--output-format json`, `--json-schema <str|file>`, `--mode plan`, `--sandbox`, `--model`, `--print-timeout` (0 = wait), `--disable-slash-commands`.
- `--dangerously-skip-permissions` is listed; it is forbidden in this work.

## Gate decisions for P3 and P2

- G1 (terms + data egress): PASS (user, 10-10). Egress approved for the frozen prompts only (no secrets, no tree access). Terms: no explicit automated-use ban; vendor headless docs endorse scripting. Personal AI credits OFF (subscription only).
- G2 and P2: blocked by quota. Gemini Models group at 0.75%, refresh in 73h 18m (user, 10-10; expected about 2026-10-13).
