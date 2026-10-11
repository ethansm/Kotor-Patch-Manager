# Plan (verbatim): Agy (Gemini 3.8 Flash) as a second-model validator in the Haiku swarm

Approved by the user on 10-10. Copied verbatim from the session's plan message so it survives a context clear. Amendments and deviations are recorded in [agy_eval_session_log.md](agy_eval_session_log.md), not here.

**File-name mapping.** The body below uses the plan's original file names. The repo files use the `agy_eval_` prefix so they never collide with generic names (`plan.md`, `README.md`):

- `PREREG.md` → `agy_eval_prereg.md`
- `p1_findings.md` → `agy_eval_p1_findings.md`
- `p2_findings.md` → `agy_eval_p2_findings.md`
- `probe_log.jsonl` → `agy_eval_probe_log.jsonl`
- `pilot_run.py` → `agy_eval_pilot_run.py`
- `analyze.py` → `agy_eval_analyze.py`
- `decision.md` → `agy_eval_decision.md`
- `agy_probe.py` and `agy_gate.sh` keep their names.

---

# Investigation Plan: Agy (Gemini 3.8 Flash) as a second-model validator in the Haiku swarm

**Context.** The Haiku swarm (scripted `claude -p` stages: map, verify, adjudicate, fix review/judge) runs on Claude models only. The router eval (run `20261010T171139`) showed Haiku 5.5 alone is not clearly safe: 7/21 hard under-routes, top-1 0.775 vs gate 0.80. You proposed mixing in Antigravity's Gemini 3.8 Flash (`agy`) as an independent second voter, because a different model family may catch different errors, with graceful fallback when its weekly quota runs out. This plan investigates whether that is feasible, safe and worth the quota. It builds nothing in the flows. Outcome: a no-go / go-shadow decision memo plus an implementation outline for a separate plan.

**Reading taken:** "Agy validation" = Agy as a blind verifier, reviewer or judge (second opinion). Agy as a worker producing patches is out of scope.

**Honest expectation.** The gold sets are tiny (verify 8 decoys + 7 controls; fix 5 seeded-bad + up to 3 good), so one item moves a rate by 8 to 14 points. This plan can establish feasibility, safety and a disagreement catalog, and can reject Agy. It cannot prove Agy "adds value". The positive outcome is **go-shadow**: run Agy in shadow mode inside real flows (a separate plan) so evidence accrues at $0 USD.

**Review pass done:** a Haiku Plan agent challenged the first draft; 8 problems found, all addressed below (task equivalence, n, config confounds, probe safety, classifier, concurrency, cost caps, call arithmetic).

**Cost:** Agy = subscription quota, not dollars. The only USD spend is the paired Haiku run in P5, capped at **$0.05** (CLI-reported, sequential). Helper agents about $0.10 (token estimate, not spend). Time about 4.5 h across sessions.

**Approving this plan authorizes:**
- Read-only research and 3 Haiku Explore agents (P1), 1 Haiku worker (P3), 2 Haiku recompute agents (P6).
- At most **12 live `agy` calls** (P2). The pilot (P5: at most 80 `agy` calls + the $0.05 Haiku run) needs a **separate explicit "go"** from you after P4.
- Writes to exactly: `.claude/agy_eval/**` (new), `~/.cache/agy_eval/**` (new: neutral cwd and gate state), `.claude/handoffs/agy_flash_swarm.handoff.md`, memory `kotor2-agy-flash-swarm.md` + its MEMORY.md line, ledger rows via `master_ledger.py log`, and an append-only section in `.claude/plans/swarm_library_architecture.md` at P6.
- Unavoidable side effect: every `agy` call writes its own state under `~/.gemini/antigravity-cli/` (conversations, brain, plaintext `history.jsonl`, log). Prompt text persists there locally.
- It authorizes NO edit to any flow script, `router_eval.py`, `oc-*` wrappers, `CLAUDE.md`, `AGENTS.md` or `~/.claude/CLAUDE.md`; NO `--dangerously-skip-permissions`; NO commit.

---

## 0. Preserved Context and Anti-Patterns (Post-Clear Memory)

- **Direction (verbatim):** "could Antigravity Gemini Flash 3.8 be leveraged in our Haiku swarm as well? ... a completely separate model that might be able to catch different things in the process. We should have it attempt to mix in some Agy validation into the flows, gracefully handling it being at its weekly quota as well. (this would be through our scripted bash commands, so we should be able to easily mix it up)"
- **Branch / worktree:** `KOTOR-II` at `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`, HEAD `78807d4` (verify). Dirty tree about 224 entries, NOT ours; never stage them, never `git add -A`.
- **Read first after a clear:** `.claude/handoffs/agy_flash_swarm.handoff.md`. Memory: `kotor2-agy-flash-swarm.md`.
- **Policy in force:** `CLAUDE.md:44` (Claude Code: Haiku swarm default; OpenCode only if you ask). `AGENTS.md:70` (Antigravity: OpenCode first). Agy-as-validator is a NEW path and changes neither. Open and untouched: global `~/.claude/CLAUDE.md` still defaults to OpenCode (lines 55, 58-78).
- **Lessons from the router eval:** (1) pre-register metrics, gates and stop rules BEFORE any pilot call; (2) tiny n can reject, not certify; (3) project cost from the COLD first call; (4) the budget guard must reserve for in-flight calls (run sequentially); (5) store return code, error subtype and stderr on every failure; (6) a global rules file in the model's context is a confound, so measure it.
- **Anti-patterns (do NOT attempt):**
  - `plan_manager.py handoff|resume|mark-done` or `/plan-handoff` for this work (targets doc 10).
  - `agy ... --dangerously-skip-permissions`. The existing `~/.local/bin/agy-resume` and `agy-run` launch `agy --dangerously-skip-permissions -i`; do NOT copy them.
  - `agy -p` from the repo cwd (it reads `AGENTS.md`, `GEMINI.md`, `.agents/`). Use the neutral cwd `~/.cache/agy_eval/cwd`.
  - `--add-dir <tree>` with agy: there is no tool-disable flag, so it would grant unrestricted tools and bypass the review swarm's sandbox and secret denylist.
  - Agy TUI slash commands, `agy models`, `agy remote-control`, login/update/install subcommands (outside the P2 call budget).
  - Reading `~/.gemini/config/config.json` or any token/credential/auth file.
  - Reporting quota as dollars. Report calls and tokens; label USD "not applicable".
  - Editing `.claude/router_eval/cases.jsonl` (hash `55871c0b63a4`).

## 1. Verified ground truth (inline, so no re-reading)

**agy** (`/home/ethansm/.local/bin/agy`, 211 MB stripped ELF, version 1.3.2; checked directly):
- Non-interactive: `-p/--print`, `--print-timeout` (0 = wait), `--output-format text|json|stream-json`, `--input-format text|stream-json`, `--json-schema <str|file>`.
- Controls: `--model`, `--effort low|medium|high|xhigh|max`, `--mode accept-edits|plan`, `--sandbox` ("terminal restrictions" only), `--add-dir`, `--agent`, `--log-file`; print-mode `--disable-slash-commands`. NO tool-disable flag. No documented exit codes. No quota command in `--help`.
- Model strings inside the binary: display `Gemini 3.8 Flash (Low|Medium|High)`; ids `gemini-3.8-flash-low|medium|high`. Account access, accepted `--model` format and default model are UNVERIFIED.
- Binary strings hint at TUI `/quota`, `/usage`, `/stats` and RPCs `FetchQuotaStatus`, `GetRemainingFcaQuota`. Unverified.
- **Context that loads regardless of cwd (verified):** `~/.gemini/GEMINI.md` (7.9 KB, global rules: ADHD output style and a four-section end-of-step protocol; will fight strict JSON); `~/.gemini/config/hooks.json` (Orca status hook on PreInvocation, PostInvocation, PreToolUse, PostToolUse, Stop, running `~/.orca/agent-hooks/antigravity-hook.sh`, 57 lines; it prints JSON decisions, `PreToolUse` returns `{"decision":"ask"}`, and it spools events only when Orca env vars exist); plugin `i-have-adhd`; skill `opencode-subagents` (could shell out to `oc-subagent`).
- Project-level: reads `AGENTS.md`/`GEMINI.md` and `.agents/{skills,rules,plugins,hooks.json}`. Local docs: `~/.gemini/antigravity-cli/builtin/skills/antigravity_guide/references/{cli,app}.md`, `.../agy-customizations/SKILL.md`, `docs/json_configs.md`. Online: https://antigravity.google/docs/cli/reference.

**oc-* wrappers** (bash, `~/.local/bin`): `oc-subagent` 222 lines; dispatcher `case "$ROLE"` :190-222; `run_local_or_cloud` :139-164; `run_cloud_chain` :167-188; `KNOWN_ROLES` :34; `log_ledger` :108-121. No 429/quota text detection: failure = non-zero exit, then `clef-gate cooldown <model> 300` (`~/.cache/clef-gate/telemetry.json`, clef-gate:288-302). No Haiku shell path in any wrapper.

**Flows** (`R=.claude/review`; bundle `B=~/.claude/skills/deep-haiku-review/bundle`):
- Model calls: `R/run_map.sh:76`, `R/verify/run_verify.sh:90`, `R/verify/run_adjudicate.sh:80` (Sonnet), `R/fix/run_fix_review.sh:118`, `run_fix_judge.sh:156`, `run_integrate.sh:154` (Sonnet). Model ids are literals; `claude` resolves from PATH (stub seam `tools/stub_claude`).
- **Haiku verify is agentic, not like-for-like with agy:** `run_verify.sh:92` runs `--tools "Read,Grep,Glob" --permission-mode dontAsk` inside a sanitized tree, with `worker_settings.json` (secret denylist), bwrap isolation and a tool-call budget. Existing backoff for rate limits is at `run_verify.sh:100-110`.
- **Gold labels:** `R/work/verify_key.json` = 8 decoys (false by construction) + 7 controls (+ a 5-entry `recall` dict, role to confirm in P1-B); `R/fix/fix_calibrate.py` = 5 seeded-bad + up to 3 known-good patches (`N_GOOD = 3`, line 50). About 275 real findings exist with Haiku verdicts but no gold labels.
- `R/verify/quote_check.py` is deterministic and model-independent (+/-3 lines, at least 6 chars); `R/verify/verify_merge.py` assigns tiers CONFIRMED / CONFIRMED-BY-SONNET-ONLY / CONTESTED-NEEDS-HUMAN / REFUTED and exits 1 on a confirmed decoy; `build_dossiers.py` builds self-contained dossiers (+/-15-line excerpt) for Sonnet adjudication.
- Bundle seam: `DR_CLAUDE_BIN` is documented at `B/engine/verify/runlib.sh:12-15` (a global swap for every stage; per-script `CLAUDE_BIN="${DR_CLAUDE_BIN:-claude}"`); a replacement must accept claude's flags and emit claude-style stream-json (`record_verify.py` parses it). Bundle cost caps use CLI-reported USD only (runlib.sh:12), so an agy shim would blind them. Ladder L1-L4: `B/engine/verify/ladder.py:2-44`, off by default; L3 compare judges and L4 Sonnet are the model-diversity slots.

**router_eval.py** (pattern to mirror for probe tooling): `ENGINES` :37, `OUTCOME_ACTIONS` :44-50, `haiku_argv` :360-365 (`--tools ""`, `--no-session-persistence`, `--max-budget-usd 0.01`), `default_runner` :368, `haiku_call` :416-472 (stderr dropped at :427), budget guard :968-973 and :578-580. Tests: `scripts/test_router_eval.py:191` (`TestAborts`).

## 2. Research questions and gates

| RQ | Question | Answered by | Gate / pass criterion |
|---|---|---|---|
| RQ1 | Is scripted `agy -p` allowed on this subscription, and what may be sent to Google? | P1-A terms read | **G1** terms permit automated print-mode use; you confirm the data-egress rule (same bounded dossier text Haiku gets, no secrets). Unclear = ask you; prohibited = stop. |
| RQ2 | Is Gemini 3.8 Flash usable here; which `--model` string; latency? | P2 probes 1-2 | **G2** a call returns on the confirmed id |
| RQ3 | Can `agy -p` be driven safely: valid schema JSON, no tool/hook side effects, repeatable? | P2 probes 4-6 | **G2** 5/5 schema-conformant; canary probe shows what tool access exists; `git status` and the neutral cwd unchanged; repeats reported descriptively (n=3, temperature unknown) |
| RQ4 | How much do global rules, plugin and hooks distort output, and can they be neutralised? | P1-A static read + P2 probe 1 token overhead | **G2** overhead measured; relocation option found or recorded as a confound |
| RQ5 | What does quota exhaustion look like; can we fall back without breaking a flow? | P1-A docs + P2 stop rule + P3 stub | **G3** every failure class passes on the stub (section 5) |
| RQ6 | On identical dossiers, does Agy catch different things than Haiku, at acceptable false-confirm cost? | P4-P6 | **G5** descriptive decision rule (section 4) |
| RQ7 | Best slot and seam: second-opinion stage, `DR_CLAUDE_BIN`-style shim, L3 judge, or `oc-subagent` role? | P1-C gap list + P6 | recommendation in the memo |

## 3. Phases (sequential; never overlap a background job with a coding phase)

### P0. Preflight (10 min, $0, orchestrator)
- `git rev-parse --short HEAD`; `agy --version` must print `1.3.2`; `python3 scripts/agent_contract.py snapshot`.
- Create `.claude/agy_eval/` and `~/.cache/agy_eval/cwd` (empty dir). `ls -A ~/.cache/agy_eval/cwd` must print nothing.

### P1. Static research, $0, 30 min (3 Haiku Explore agents in parallel, read-only, bounded reply)
- **A. Agy docs, terms, loaded context:** read the local docs (section 1) for print-mode output schema, exit codes, config precedence and any env var that relocates the config dir. Statically read `~/.gemini/GEMINI.md`, `~/.gemini/config/hooks.json`, the `i-have-adhd` plugin and the `opencode-subagents` skill (none are credential files) and list what loads in print mode and whether it can run commands. WebFetch the online reference for quota, reset period and terms of automated use. Mark each fact CONFIRMED or UNSURE with source.
- **B. Benchmark inventory:** confirm the gold sets (8+7 verify, 5+up to 3 fix), what the 5-entry `recall` dict is, where the existing Haiku verdicts for the 23 gold items and the about 275 real findings live, and whether a dossier builder exists for fix patches.
- **C. Seam and cost gaps:** compare the stream-json events `record_verify.py` parses with agy's `json|stream-json` output; show where a second voter slots into `verify_merge.py`, `quote_check.py` and the bundle L2/L3 judges; list what an agy shim loses (USD cost fields feeding the caps at runlib.sh:12, bwrap isolation, secret denylist) and which backoff logic to reuse (`run_verify.sh:100-110`).
- **Exit:** G1 decided (or asked of you). Orchestrator saves findings to `.claude/agy_eval/p1_findings.md` (Explore agents cannot write).

### P2. Live probes, at most 12 `agy` calls, 45 min (orchestrator only; sequential)
Tool: new `.claude/agy_eval/agy_probe.py` (about 80 lines). Every call: neutral cwd, `--mode plan --sandbox --output-format json --print-timeout 120`, wrapped in `setsid timeout -k 5 130`, never `--dangerously-skip-permissions`. It appends ONE row per call to `.claude/agy_eval/probe_log.jsonl`: argv, cwd, return code, latency, full stdout and stderr (truncated at 20 KB), parsed fields. Optional (1 min, yours): note the Antigravity quota meter before and after P2; `agy` itself cannot report it.

| # | Probe | Calls |
|---|---|---|
| 1 | trivial text prompt, no `--model`: default model, usage fields, token overhead | 1 |
| 2 | `--model gemini-3.8-flash-medium` | 1 |
| 3 | deliberately invalid model id: error shape, and whether it costs quota | 1 |
| 4 | verdict prompt with `--json-schema`, repeated 3 times: conformance, agreement | 3 |
| 5 | two gold dossiers (1 decoy, 1 control) from P1-B: conformance, and `quote_check.py` accepts the quotes | 2 |
| 6 | canary: put `canary.txt` (unique token) in the neutral cwd and ask agy to print it: shows whether tools work in print mode under the Orca `ask` hook | 1 |
| 7 | two parallel calls: concurrency and latency | 2 |
| 8 | reserve: display-name retry if probe 2's format fails; otherwise unused | 1 |

Total 12 (probe 4 + 5 = 5 schema calls).
- **Stop rule:** a `quota` or `auth` error halts ALL agy calls; a `rate_limit` error gets one retry after backoff (counts against the 12). Copy the verbatim error into `p2_findings.md` (the quota signature we need). Then continue with P3 only.
- Ledger: `AGY_PROBE calls=<n> model=<id> result=<ok|quota|auth|other>`. **Exit G2.**

### P3. Quota and fallback design, tested on a stub, $0, 50 min (1 Haiku worker + orchestrator review)
Spec in section 5. Deliverables in `.claude/agy_eval/` (not wired anywhere): `agy_gate.sh`, `stub_agy/agy` (bash stub; mode from `STUB_AGY_MODE`), `test_agy_gate.sh`. Worker brief: Contract `worker`, Write Scope `.claude/agy_eval/**`, at most 3 files; afterwards `agent_contract.py check - --scope '.claude/agy_eval/**'`.
- Stub modes: success, quota, rate_limit, auth, transient, hang, malformed JSON, corrupt state file, unwritable state dir, `agy` missing from PATH, and concurrent callers.
- **Exit G3:** `test_agy_gate.sh` passes all modes under `set -euo pipefail` and `xargs -P4`; the caller always receives a valid verdict JSON or `{"agy_skipped":"<reason>"}` with exit 0.

### P4. Pre-registration, $0, 30 min (orchestrator)
Write `.claude/agy_eval/PREREG.md` BEFORE any pilot call, with sha256 (first 12) of every input:
- **Task definition (same task for both models):** a self-contained dossier (claim or patch plus +/-15-line excerpt, as `build_dossiers.py` does), no tree access, no tools. Agy: neutral cwd, `--mode plan --sandbox`, `--json-schema`. Haiku: `claude -p --model claude-haiku-5-5 --tools "" --no-session-persistence`, neutral cwd, same schema. The existing agentic Haiku verdicts are a secondary comparison only.
- **Sets:** verify gold 15 (8 decoys, 7 controls); fix gold up to 8 (5 seeded-bad + up to 3 good); real sample at most 30 findings (for the disagreement catalog).
- **Calls:** Agy: gold items run twice (up to 46) + real sample (at most 30) = at most 76, cap **80**. Haiku: gold once + real sample = at most 53 calls, cap **$0.05**, sequential, stderr stored.
- Metrics, stop rules (first `quota` error = stop, no re-runs, no prompt tuning after call 1), cold-call cost projection from P2, and the decision rule (section 4).
- **STOP and ask you for a "go"** with the real call counts and the P2 findings.

### P5. Pilot (after your go; 60 to 90 min)
Run both models on the frozen dossiers through `agy_gate.sh` / `agy_probe.py`, sequentially, with the caps and stop rule. Raw outputs to `.claude/agy_eval/pilot/`.

### P6. Analysis, recompute, decision, persist (40 min)
- `.claude/agy_eval/analyze.py` computes section 4 metrics; it also writes a **blinded review pack** of disagreements (A/B labels shuffled per item, key in a separate file not opened until verdicts are recorded). 2 Haiku recon agents independently recompute 4 headline numbers each (3 files each), as in router-eval T8.
- `.claude/agy_eval/decision.md`: results, blinded review of Agy-only and Haiku-only catches, failure modes, quota cost, recommendation, and an implementation outline (seam, files, tests, `SWARM_AGY=off|shadow|on`) for a separate plan.
- Persist: append `## Agy second-model validator result` (at most 25 lines) to `.claude/plans/swarm_library_architecture.md`; update memory and handoff; `master_ledger.py log AGY_EVAL ...`.

## 4. Pilot metrics and decision rule (frozen in P4)

- **Descriptive only:** counts with Wilson 95% CIs: decoy false-confirm k/8, control miss k/7, bad-patch-approved k/5, good-patch-rejected k/3, invalid-output rate, quote-check pass rate, latency p50/p95, calls and tokens per catch, Agy repeatability on gold items (k/23). No McNemar, phi or percentage-point gates: they are meaningless at n=15 and n=8 (one item = 8 to 14 points).
- **Disagreement catalog:** all items where Agy and Haiku differ, blind-reviewed (shuffled labels). Each Agy-only catch is classified real / spurious / unclear.
- **Decision rule:**
  - **No-go:** G1, G2 or G3 fails; or Agy schema-valid output < 90% (fewer than 21/23 on gold); or Agy false-confirms at least 2 more decoys than Haiku.
  - **Go-shadow (expected positive outcome):** feasibility and safety shown and no disqualifier. Next plan wires `SWARM_AGY=shadow` into the flows; promotion to `on` needs a new pre-registered rule using shadow data.
  - There is **no "go-on" from this pilot**: n is too small.

## 5. Quota and fallback specification (P3 deliverable)

- **Classifier** per call (return code, parsed JSON, stderr): `ok`; `quota` (weekly exhaustion); `rate_limit` (short-term throttle); `auth`; `timeout`; `invalid` (rc 0 but schema fail); `transient` (any other non-zero or 5xx). Unknown failures default to `transient`. Promote to `quota` or `auth` ONLY on the signature observed in P2 or a documented string. Generic words ("limit", "403") never qualify.
- **State** `~/.cache/agy_eval/gate/state.json` (flock + atomic rename): status, `cooldown_until`, calls this run, calls this week, last raw error (2 KB).
- **Rules:** `quota` = cooldown to the parsed reset time, else +12 h, never retried; `rate_limit` = backoff modeled on `run_verify.sh:100-110`; `auth` = disabled until manual fix; `transient` = one retry after 20 s, 3 in a row = 30 min cooldown. Pre-call: read state, skip if cooling.
- **Never fail a flow:** the caller gets a verdict or `{"agy_skipped":"<reason>"}` on stdout (JSON, not free text) plus one stderr line, always exit 0. A missing Agy vote means abstain, never disagree; tiers for Haiku-only items stay unchanged and the manifest records Agy coverage. A ledger row `AGY_SKIPPED reason=...` makes degraded coverage visible.
- **Quota hygiene** (the weekly quota is shared with your interactive Antigravity use): per-run `AGY_MAX_CALLS` and weekly soft cap `AGY_WEEKLY_RESERVE`. Rollout switch `SWARM_AGY=off` (default) | `shadow` (runs and logs, no effect on tiers) | `on`.

## 6. Verification / done checklist

- [ ] P0: HEAD recorded; neutral cwd exists and is empty; no `agy` call yet
- [ ] G1 (terms + data egress) recorded before the first live call
- [ ] `probe_log.jsonl` has at most 12 rows; none uses `--dangerously-skip-permissions` or `--add-dir`
- [ ] Verbatim quota/auth error saved if one occurred; otherwise stated "not observed" and the classifier marked UNVERIFIED
- [ ] `test_agy_gate.sh` passes all 11 stub modes
- [ ] `PREREG.md` hashes written before the first pilot call; pilot ran only after your go; Haiku spend (CLI-reported) at most $0.05
- [ ] Decision memo + swarm-plan section (insertions only) + memory + one `AGY_EVAL` ledger row
- [ ] `git status --short` shows only authorized paths; no flow script, `CLAUDE.md`, `AGENTS.md` or global file changed; no commit
- [ ] Final report uses the 4-section protocol; Agents Used lists the 6 Haiku agents (token estimate, not spend)
