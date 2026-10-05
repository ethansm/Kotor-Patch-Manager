# KOTOR-II Workspace Guidelines

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
- **Subagent Context Compactor:** `npx repomix [dir]` (packs directory into token-counted artifact for subagents)
- **Offline Log Analyzers:** `python3 scripts/kotor2_logs.py status | check <log> [--new]` (routes every live log to its parser; `--new` = only lines since last check, no manual line offsets). Details: `kotor2-log-analysis` skill. NEVER `cat`/`tail`/`grep` raw `.txt` logs.
- **Two-Stage Logging Lifecycle**:
  - *Dev / Probe*: Instrument rich diagnostics (KpLog channels, struct dumps, first-draw coordinates). Never guess blind in live play.
  - *Tightening / Release*: Before PR or final install, gate per-frame or high-volume logs behind `Verbose=0` in INI, enforce deduplication (log state changes, not per-frame redraws), drop probe detours, and verify clean logs (<50 lines on startup) via `kotor2_logs.py check`.

## Context Management & Plan Mode Handoffs
- Before context clearing, trigger the `/plan-handoff` skill (runs `python3 scripts/plan_manager.py handoff`).
- On fresh session startup after context clear: run `python3 scripts/plan_manager.py resume` to immediately bootstrap active plan, git state, and next chunk with zero token waste.
- Switch to built-in plan mode so the user can select "Clear context and approve (bypass mode)". Do NOT prematurely exit plan mode with `ExitPlanMode` when waiting for context-clear approval.
- **Shorthand Plan Inputs:** Both `/plan-handoff` and `plan_manager.py resume|handoff` accept shorthand doc numbers (e.g. `25 §12`, `doc 25`, `24:T6`, `19e`) and phase IDs (`T5e`, `T6`), auto-resolving target plan paths.
- Mark completed phases via `python3 scripts/plan_manager.py mark-done <phase> [--state <STATE>] [--commit <sha>] [--notes <txt>]` (updates markdown table rows or checkboxes automatically; NEVER manually sed/grep the master plan doc).

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
- **Subagent Exemption**: When reporting back to an AI orchestrator or tool caller, the human ADHD rules are superseded by the worker/recon contract above. Do NOT suggest next steps, do NOT paste code snippets, and do NOT number multi-step prose.
