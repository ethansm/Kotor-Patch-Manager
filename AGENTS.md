# KOTOR-II Workspace Guidelines

## KPM install
- Run `python3 scripts/kpm_status.py` first; it gives one verdict per mod.
- Follow `patch_manager_mods/BUILD_AND_INSTALL.md` (gate, `kpm_hotswap.py --prepare`, `--apply`).
- Never use KPatchLauncher for a change that can be hot-swapped.
- Never run two installers at once; commit with explicit paths only.
- Ask the user for GO before `kpm_hotswap.py --apply` (game closed).

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
- **Master Plan & Context Resume:** `python3 scripts/plan_manager.py resume | next | handoff` (instant phase recovery < 200 tokens; never scrape past session logs). Spec & Legend: `patch_manager_mods/MASTER_KEY.md`.
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

## Context Management & Multi-Session Plan Mode Handoffs
- **Feature-Scoped Handoffs (Zero Collision)**: Concurrent sessions in the same worktree must track their state through their specific master plan doc (e.g. `22_compat_check_plan.md`, `33_ui_queue_hotbar_fixes.md`).
- Before context clearing: run `/plan-handoff <doc|phase>` (or `python3 scripts/plan_manager.py handoff <doc|phase>`). This writes `.claude/handoffs/<plan_stem>.handoff.md` without clobbering concurrent sessions.
- On fresh session startup after context clear: run `python3 scripts/plan_manager.py resume <doc|phase>` (e.g. `resume U3`, `resume 22`, `resume 33`) to instantly bootstrap that specific feature's state and scoped handoff direction (<200 tokens).
- Query all active projects: `python3 scripts/plan_manager.py list` shows progress and next phase across all plans in the worktree.
- Mark completed phases via `python3 scripts/plan_manager.py mark-done <phase> [--state <STATE>] [--commit <sha>] [--notes <txt>]` (auto-resolves target plan doc from `<phase>`, auto-generates the scoped handoff, and prints the `=== SAVED STATE (Safe to /clear) ===` banner; NEVER manually sed/grep the master plan doc).
- **End-of-Step Saved State Protocol**: When concluding a step or milestone, always include a `### Saved State` section detailing landed commit(s), plan-scoped handoff path, safe-to-clear confirmation, and the exact next resume command (`resume <phase>`) so the user never has to ask if state was saved.

## Context Budget & Read Policy (Anti-Bloat Standards)
- **Zero-Read / Self-Contained Plans**: In Plan Mode, inline the exact 5-10 line struct/interface or macro snippets directly in the plan. Never tell the executor to "Read first: `LargeHeader.h`".
- **Execution Fast-Path**: When executing an approved plan after `/clear`, the plan is self-contained. Proceed directly to targeted file edits and tests.
- **Forbidden Discovery Commands**:
  - NEVER run broad `ls` or recursive tree walks on mod directories on startup.
  - NEVER `cat` entire source files or headers just to inspect an API; use `sed -n 'X,Yp'` or `rg` for specific symbols.
  - NEVER read historical session context docs (`19*.md`, `15*.md`, etc.) to find session names or past context; all active state is captured in `.claude/session_handoff.md` and the active plan.
  - Do NOT prepend redundant `cd $PWD` to shell commands.
  - Use native file edit tools (`Edit`/`Write`) instead of multi-line Python heredocs in Bash when modifying files.

## Subagent Contracts (supersede the ADHD style for subagents)
Brief = `Contract: worker|recon` + `Write Scope: <globs>` + output of `python3 scripts/agent_contract.py print worker|recon`.
Order: worker CREATED,MODIFIED,TESTS,NOTES,STATUS · recon OUTPUT,EVIDENCE,FINDING,OPEN,VERDICT. Verify: `agent_contract.py snapshot` before launch, `agent_contract.py check --scope <globs> -` after.

## AI Agent Output Style (`i-have-adhd`)
- The `i-have-adhd` plugin is installed globally across Claude Code and Antigravity (`agy`).
- Shape all responses for an ADHD reader: lead with actions/commands/snippets, number multi-step work, cap lists to 5 items, give concrete time estimates, and end with one concrete next action doable in under 2 minutes.
- **Actionable Plan-Handoff Over Generic Reviews**: When ending with a next action and work remains on a feature or mod, provide concrete next development steps or plan-handoff commands (e.g. `/plan-handoff <batch>` or `python3 scripts/plan_manager.py handoff <batch>`), NEVER generic instructions to "review the changes made" or "inspect files".
- **Subagent Exemption**: When reporting back to an AI orchestrator or tool caller, the human ADHD rules are superseded by the worker/recon contract above. Do NOT suggest next steps, do NOT paste code snippets, and do NOT number multi-step prose.
