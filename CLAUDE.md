# CLAUDE.md — KOTOR II Workspace Guidelines

## Task Routing (load the skill before the task)
- Address-DB rows / Steam DB batches / PR 5 → `kotor2-address-db` skill
- KPM install, DLL hot-swap, staging, release log tightening, kpm-pr-* worktrees → `kotor2-kpm-install` skill (reference: `patch_manager_mods/BUILD_AND_INSTALL.md`)
- Ghidra server (status, export, audit) → `openkotor-ghidra-server` skill
- Live mod logs → `kotor2-log-analysis` skill (`python3 scripts/kotor2_logs.py status | check <log> [--new]`); NEVER `cat`/`tail`/`grep` raw `.txt` logs
- Crash/freeze/hang → `kotor2-crash-triage` skill
- Plan handoff / context clear → `/plan-handoff` skill; after clear run `python3 scripts/plan_manager.py resume`. Stay in plan mode for "Clear context and approve"; do NOT `ExitPlanMode` early. Mark phases with `python3 scripts/plan_manager.py mark-done <phase> [--state <STATE>] [--commit <sha>] [--notes <txt>]` (auto-generates handoff & prints Saved State banner); NEVER sed the master plan. Both `/plan-handoff` and `plan_manager.py resume|handoff` accept shorthand (`25 §12`, `24:T6`, `19e`, `T5e`). Always output the `### Saved State` block upon step completion so context clearing is immediately actionable. Next actions ending turns should suggest concrete `/plan-handoff <batch>` targets rather than generic reviews.

## Upstream Issue & PR Notification Check
At session start or before mod work, run `python3 scripts/check_kpm_updates.py --quiet` to check `LaneDibello/Kotor-Patch-Manager` (silent when nothing new). If it prints replies, alert the user with issue number, author, and preview.

## GitHub Interaction Policy
- Never post comments, create issues, or submit PRs without the user's explicit, conversational approval in chat.

## Ghidra Server Credential Isolation Policy (Strict)
- `~/.config/ghidra/.server_credentials` is an opaque system credential vault (permissions: `0600`).
- **NEVER** call `view_file`, `cat`, `head`, `tail`, `grep`, or `echo` on `~/.config/ghidra/.server_credentials` or any file matching `*credential*` or `*secret*`.
- Always interact with the Ghidra Server autonomously via `python3 scripts/ghidra_server_api.py` or `scripts/export_server_knowledge.sh`, which consume credentials internally in memory and never expose them in command arguments, stdout, stderr, or transcript logs.

## Fast Local Tools
- Plan resume: `python3 scripts/plan_manager.py resume | next | handoff` (spec: `patch_manager_mods/MASTER_KEY.md`)
- Symbols/structs: `python3 scripts/query_ghidra.py symbol <name> | address <hex> | struct <class>` (never inline sqlite)
- Search: `rg`; repo map: `python3 scripts/generate_repo_map.py [dir]`; docs: `python3 scripts/search_docs.py "<query>"`; subagent pack: `npx repomix [dir]`
- Logging: probe builds log richly (never guess blind live); release tightening rules live in `kotor2-kpm-install`.
- Compat check: after touching `_shared_src` or a library, run `python3 scripts/compat_check.py <files>`

## Context Budget & Read Policy (Anti-Bloat Standards)
- **Zero-Read / Self-Contained Plans**: In Plan Mode, inline the exact 5-10 line struct/interface or macro snippets directly in the plan. Never tell the executor to "Read first: `LargeHeader.h`".
- **Execution Fast-Path**: When executing an approved plan after `/clear`, the plan is self-contained. Proceed directly to targeted file edits and tests.
- **Forbidden Discovery Commands**:
  - NEVER run broad `ls` or recursive tree walks on mod directories on startup.
  - NEVER `cat` entire source files or headers just to inspect an API; use `sed -n 'X,Yp'` or `rg` for specific symbols.
  - NEVER read historical session context docs (`19*.md`, `15*.md`, etc.); active state is in `.claude/session_handoff.md` and the active plan.
  - Do NOT prepend redundant `cd $PWD` to shell commands.
  - Use native file edit tools (`Edit`/`Write`) instead of multi-line Python heredocs in Bash when modifying files.

## Subagent Contracts (supersede the ADHD style for subagents)
Brief = `Contract: worker|recon` + `Write Scope: <globs>` + output of `python3 scripts/agent_contract.py print worker|recon`.
Order: worker CREATED,MODIFIED,TESTS,NOTES,STATUS · recon OUTPUT,EVIDENCE,FINDING,OPEN,VERDICT. Verify: `agent_contract.py snapshot` before launch, `agent_contract.py check --scope <globs> -` after.
