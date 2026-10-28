# KOTOR-II Workspace Guidelines

## KPM install
- Run `python3 scripts/kpm_status.py` first; it gives one verdict per mod.
- Follow `patch_manager_mods/BUILD_AND_INSTALL.md` (gate, `kpm_hotswap.py --prepare`, `--apply`).
- **Two-Stage Gate Flow**: Source gate (`pre_install_gate.sh --skip-default --mod <x>_src`) -> Prepare (`kpm_hotswap.py --prepare`) -> Full verification gate (`pre_install_gate.sh --mod <x>_src`) -> Apply (`--apply`).
- **Verify After Fix & Gate Invariant (CRITICAL)**:
  - NEVER dismiss a test or gate failure as "pre-existing" or "expected" unless it is listed in a known-issues block of the active prompt or an explicit written waiver. A gate failure is a hard stop.
  - Rebuilding `.kpatch` files via `--prepare` REQUIRES re-running `pre_install_gate.sh` to obtain a fresh `GATE PASS` before `--apply`.
  - Re-verify immediately after every edit to code, manifests, allowlists (`raw_addresses.toml`) or `.kpatch` files: re-run the specific failing suite right then; never batch verification after several untested edits.
  - NEVER dump raw addresses into `raw_addresses.toml` without prior lookup via `python3 scripts/query_ghidra.py address <hex>`.
- Never use KPatchLauncher for a change that can be hot-swapped.
- Never run two installers at once; commit with explicit paths only.
- Ask the user for GO before `kpm_hotswap.py --apply` (game closed).

## Persist Ad-hoc Research (saved knowledge)
- After any one-off investigation during mod/patch work (decompile read, address or offset worked out, log diagnosis, dead end, live finding), record it before ending the session or clearing context. Skill: `.claude/skills/kotor2-persist-knowledge/SKILL.md`.
- Sinks: exe names/roles -> REA ledger rows `patch_manager_mods/ghidra_knowledge/rea_findings/<YYYYMMDD>_<topic>.tsv` (then `rea_ledger.py lint`); offsets/hook sites -> the mod's plan doc; model traps -> `RE_AGENT_GUIDE.md` §3; write-ups -> `.claude/research/<topic>/`; dead ends -> plan doc or handoff; live-only checks -> `PLAY_TEST_QUEUE.md`; status -> memory.
- Never write `name_map` directly; names reach the knowledge DB only after a Haiku check with decoys and the user's OK. Commit with explicit paths.

## Upstream Issue & PR Notification Check
When starting a new session or beginning mod work:
- Run `python3 scripts/check_kpm_updates.py --quiet` to check for any new upstream replies on `LaneDibello/Kotor-Patch-Manager`.
- **Zero Token Overhead:** If there are no new replies, the script produces zero output (terminating silently).
- If new comments or PR reviews are found, alert the user with the issue number, author, and comment preview.

## GitHub Interaction Policy
- Never post comments, create issues, or submit PRs without the user's explicit, conversational approval in chat.

## Ghidra Server Credential Isolation Policy
- `~/.config/ghidra/.server_credentials` is an opaque system credential vault.
- **NEVER** call `view_file`, `cat`, `head`, `tail`, `grep`, or `echo` on `~/.config/ghidra/.server_credentials` or any file matching `*credential*` or `*secret*`.
- Always interact with the Ghidra Server autonomously via `python3 scripts/ghidra_server_api.py` or `scripts/export_server_knowledge.sh`, which consume credentials internally in memory and never expose them in command arguments, stdout, stderr, or transcript logs.

## Fast Local Development Tools
- **Master Plan & Context Resume:** `python3 scripts/plan_manager.py resume | next | delegate <phase> | handoff` (instant phase recovery & autonomous subagent execution < 200 tokens; never scrape past session logs). Spec & Legend: `patch_manager_mods/MASTER_KEY.md`.
- **Symbol & Struct Resolution:** `python3 scripts/query_ghidra.py symbol <name> | address <hex> | struct <class>` (instant lookup; never write inline sqlite scripts)
- **Fast Code Search:** `rg "<pattern>" [dir]` (ripgrep v15.2.0 installed in PATH)
- **Structural Repo Map:** `python3 scripts/generate_repo_map.py [dir]` (compact class/fn skeleton)
- **Doc Semantic Search:** `python3 scripts/search_docs.py "<query>"` (local vector search via LM Studio / FTS5)
- **Master Ledger & Collision Guard:** `python3 scripts/master_ledger.py tail | status | check-collision --mod <id>` (chronological audit log of live DLL hot-swaps, rollbacks, build gate verdicts, and plan milestones across all sessions).
- **Subagent Context Compactor:** `npx repomix [dir]` (packs directory into token-counted artifact for subagents)
- **Offline Log Analyzers:** `python3 scripts/kotor2_logs.py status | check <log> [--new]` (routes every live log to its parser; `--new` = only lines since last check, no manual line offsets). Details: `kotor2-log-analysis` skill. NEVER `cat`/`tail`/`grep` raw `.txt` logs.
- **Two-Stage Logging Lifecycle**:
  - *Dev / Probe*: Instrument rich diagnostics (KpLog channels, struct dumps, first-draw coordinates). Never guess blind in live play.
  - *Tightening / Release*: Before PR or final install, gate per-frame or high-volume logs behind `Verbose=0` in INI, enforce deduplication (log state changes, not per-frame redraws), drop probe detours, and verify clean logs (<50 lines on startup) via `kotor2_logs.py check`.
- **Turnkey Mod Scaffolding:** `python3 scripts/scaffold_mod.py <name> [--doc-id <XX>]` (generates compliant mod dir, standard includes, common contract test, and master plan with oc-subagent matrix).
- **Update Compatibility Check:** after touching `_shared_src` or a library, run `python3 scripts/compat_check.py <files>` (traces transitive dependents, contract tests, and mod builds; doc 22).

## Context Management & Multi-Session Plan Mode Handoffs
- **Feature-Scoped Handoffs (Zero Collision)**: Concurrent sessions in the same worktree must track their state through their specific master plan doc (e.g. `22_compat_check_plan.md`, `34_npc_diversity_dispatcher.md`).
- **NEVER Bare Resume**: In concurrent worktrees, NEVER run bare `resume`, bare `python3 scripts/plan_manager.py resume`, or bare `agy-resume`. Bare resume falls back to `.claude/active_plan.txt` which collides across concurrent sessions. ALWAYS pass `<doc|phase>` (e.g. `resume 34`, `agy-resume 34`, `resume C4`).
- **Relaunch vs Clear Flow**:
  - *If exiting to shell (`/exit` / `Ctrl+D`)*: Run `agy-resume <doc|phase>` from terminal to boot directly into the scoped plan.
  - *If staying inside agy (`/clear`)*: Type `/clear`, then enter `python3 scripts/plan_manager.py resume <doc|phase>` (or `resume <doc|phase>`). Never run `agy-resume` inside an active `agy` session to avoid nested CLI processes.
- Before context clearing: run `/plan-handoff <doc|phase>` (or `python3 scripts/plan_manager.py handoff <doc|phase>`). This writes `.claude/handoffs/<plan_stem>.handoff.md` without clobbering concurrent sessions.
- On fresh session startup after context clear: run `python3 scripts/plan_manager.py resume <doc|phase>` (e.g. `resume U3`, `resume 22`, `resume 34`) to instantly bootstrap that specific feature's state and scoped handoff direction (<200 tokens).
- Query all active projects: `python3 scripts/plan_manager.py list` shows progress and next phase across all plans in the worktree.
- Mark completed phases via `python3 scripts/plan_manager.py mark-done <phase> [--state <STATE>] [--commit <sha>] [--notes <txt>]` (auto-resolves target plan doc from `<phase>`, auto-generates the scoped handoff, and prints the `=== SAVED STATE (Safe to /clear) ===` banner; NEVER manually sed/grep the master plan doc).
- **End-of-Step Output Protocol**: When concluding a step or milestone, always structure output into the 4 explicit sections: `### Actions Completed`, `### What Now Works`, `### Agents Used`, and `### Saved State` (detailing landed commit(s), plan-scoped handoff path, safe-to-clear confirmation, and the exact next resume command `resume <phase>` or `agy-resume <phase>`). For `### Agents Used`, paste the table from `python3 scripts/agent_usage.py --since <step start ISO8601>` (use `--all` if more than 5 agents), then add one line per agent: short id, what it did, and PASS/FAIL contract status.

## Context Budget & Read Policy (Anti-Bloat Standards)
- **Zero-Read / Self-Contained Plans**: In Plan Mode, inline the exact 5-10 line struct/interface or macro snippets directly in the plan. Never tell the executor to "Read first: `LargeHeader.h`".
- **Execution Fast-Path**: When executing an approved plan after `/clear`, the plan is self-contained. Proceed directly to targeted file edits and tests.
- **Forbidden Discovery Commands**:
  - NEVER run broad `ls` or recursive tree walks on mod directories on startup.
  - NEVER `cat` entire source files or headers just to inspect an API; use `sed -n 'X,Yp'` or `rg` for specific symbols.
  - NEVER read historical session context docs (`19*.md`, `15*.md`, etc.) to find session names or past context; all active state is captured in your named handoff `.claude/handoffs/<plan_stem>.handoff.md` and the active plan. Several sessions share this worktree, so do not read `.claude/session_handoff.md`; it is no longer written.
  - Do NOT prepend redundant `cd $PWD` to shell commands.
  - Use native file edit tools (`Edit`/`Write`) instead of multi-line Python heredocs in Bash when modifying files.

## Subagent Contracts (supersede the ADHD style for subagents)
- See `patch_manager_mods/SUBAGENT_ROLES.md` for full role catalog, backend routing, and contract specs.
- Brief = `Contract: worker|recon` + `Write Scope: <globs>` + output of `python3 scripts/agent_contract.py print worker|recon`.
- **Backend policy (Claude Code)**: Do not use OpenCode subagents (`oc-subagent`, `oc-flow`, `oc-free`, `oc-local`) unless the user explicitly asks for them in Claude Code. Default to the Haiku swarm (parallel `model: haiku` Agent subagents).
- Wave recipe and budget gate: see `patch_manager_mods/SUBAGENT_ROLES.md` "Claude Code Haiku swarm (waves)".
- Order: worker CREATED,MODIFIED,TESTS,NOTES,STATUS · recon OUTPUT,EVIDENCE,FINDING,OPEN,VERDICT · review OUTPUT,EVIDENCE,BLOCKING,FINDING,DECISION. Verify: `agent_contract.py snapshot` before launch, `agent_contract.py check --scope <globs> -` after.

## AI Agent Output Style (`i-have-adhd`)
- The `i-have-adhd` plugin is installed globally across Claude Code and Antigravity (`agy`).
- Shape all responses for an ADHD reader: lead with actions/commands/snippets, number multi-step work, cap lists to 5 items, give concrete time estimates, and end with one concrete next action doable in under 2 minutes.
- **Actionable Plan-Handoff Over Generic Reviews**: When ending with a next action and work remains on a feature or mod, provide concrete next development steps or plan-handoff commands (e.g. `/plan-handoff <batch>` or `python3 scripts/plan_manager.py handoff <batch>`), NEVER generic instructions to "review the changes made" or "inspect files".
- **Subagent & Orchestrator Delegation Exemption**: When reporting back to an AI orchestrator or delegating work to OpenCode/subagents, the human ADHD rules are superseded. The orchestrator MUST run the full plan/review/build delegation chain to completion in a single turn without pausing midway to ask the human for approval. The human ADHD rules apply ONLY to the final user-facing response.

## Exhaustive Plan Specification Standard (Subagent Plan Mode)
When in plan mode (`plan` agent / `oc-flow start`), subagents MUST output an exhaustive 4-pillar specification before build:
1. **Exact Code Changes**: Target file paths, function anchors, line numbers, exact struct offsets, texture names, RGB vectors, and calling conventions.
2. **Edge Cases & Test Matrix**: State transitions (pause/unpause, save/load, area transition), boundary conditions, null pointer safety, and unit test vectors in `*_test.cpp`.
3. **Tooling & Verification**: Exact commands for `scripts/build_check.py`, `scripts/run_tests.py`, and `scripts/agent_diff_check.py`.
4. **Documentation & Contract**: Master plan status updates and compliance with the 5-line WORKER schema.

## Terminal & Task Execution Rules (CRITICAL)
- **Synchronous Execution:** Set `WaitMsBeforeAsync` to `10000` (or `300000`) for test/build scripts. Note: Antigravity CLI's `run_command` tool implementation clamps synchronous wait to 10s (10000ms); commands exceeding 10s will automatically detach to a background task (`task-XXX`).
- **Unbuffered Python Output & Stdin Detachment:** Always invoke Python test/parity scripts with `PYTHONUNBUFFERED=1 python3 -u <script> < /dev/null 2>&1` so child processes never deadlock on stdin or buffer stdout.
- **NEVER Poll ManageTask, Logs, or ps:** If a command demotes to a background task (`task-XXX`), NEVER call `ManageTask(status task-XXX)`, NEVER repeatedly `view_file` on `.system_generated/tasks/task-*.log`, and NEVER run `ps aux | grep ...` to monitor it. Reading task logs before completion is polling.
- **Yield the Turn on Background Tasks:** If you have no parallel file edits or reading to perform while a background task runs, STOP calling tools and end your turn immediately with a one-line status notice. Antigravity's reactive wakeup listener will automatically resume you upon completion.
- **No Polling Schedules:** Do NOT call `Schedule(30s)` or recurring timers to check on background tasks; wait for reactive task completion messages.
- **Scoped Test Execution:** NEVER run bare `python3 scripts/run_tests.py` without a filter. Bare runs execute all 156 repository suites (>60s, background detach). Always scope test runs to the active feature (e.g. `python3 scripts/run_tests.py <name|filter>` or `-k <filter>`). Positional arguments (e.g. `run_tests.py suite:check-hook-bytes`, `run_tests.py diversity`) are supported.
- **Execute CLI Prompts Directly:** If the user's prompt begins with or consists of a console command (e.g. `python3 scripts/...`, `./scripts/...`, `git ...`, `oc-subagent ...`, `oc-flow ...`, `python3 scripts/plan_manager.py delegate ...`), immediately execute it via `run_command` on turn 1 without asking for confirmation or explaining the command.
