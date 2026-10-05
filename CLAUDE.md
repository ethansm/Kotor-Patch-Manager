# CLAUDE.md — KOTOR II Workspace Guidelines

## Upstream Issue & PR Notification Check
When starting a new session or beginning mod work:
- Run `python3 scripts/check_kpm_updates.py --quiet` to check for any new upstream replies on `LaneDibello/Kotor-Patch-Manager`.
- **Zero Token Overhead:** If there are no new replies, the script produces zero output (terminating silently).
- If new comments or PR reviews are found, alert the user with the issue number, author, and comment preview.

## GitHub Interaction Policy
- Never post comments, create issues, or submit PRs without the user's explicit, conversational approval in chat.

## Ghidra Server Credential Isolation Policy (Strict)
- `~/.config/ghidra/.server_credentials` is an opaque system credential vault (permissions: `0600`).
- **NEVER** call `view_file`, `cat`, `head`, `tail`, `grep`, or `echo` on `~/.config/ghidra/.server_credentials` or any file matching `*credential*` or `*secret*`.
- Always interact with the Ghidra Server autonomously via `python3 scripts/ghidra_server_api.py` or `scripts/export_server_knowledge.sh`, which consume credentials internally in memory and never expose them in command arguments, stdout, stderr, or transcript logs.

## OpenKotOR Ghidra Server Tooling
Use the dedicated `openkotor-ghidra-server` skill and CLI wrapper:
- Status check: `python3 scripts/ghidra_server_api.py status`
- Browse repository: `python3 scripts/ghidra_server_api.py list-tree /TSL`
- Export program symbols: `python3 scripts/ghidra_server_api.py export k2_win_gog_aspyr_swkotor2.exe`
- Run comparison audit: `python3 scripts/ghidra_server_api.py audit`
- Local knowledge DB query: `python3 scripts/query_ghidra.py symbol <name>` / `address <0xADDR>` / `struct <class>`

## Fast Local Development Tools
- **Master Plan & Context Resume:** `python3 scripts/plan_manager.py resume | next | handoff` (instant phase recovery < 200 tokens; never scrape past session logs)
- **Symbol & Struct Resolution:** `python3 scripts/query_ghidra.py symbol <name> | address <hex> | struct <class>` (instant lookup; never write inline sqlite scripts)
- **Fast Code Search:** `rg "<pattern>" [dir]` (ripgrep v15.2.0 installed in PATH)
- **Structural Repo Map:** `python3 scripts/generate_repo_map.py [dir]` (compact class/fn skeleton)
- **Doc Semantic Search:** `python3 scripts/search_docs.py "<query>"` (local vector search via LM Studio / FTS5)
- **Subagent Context Compactor:** `npx repomix [dir]` (packs directory into token-counted artifact for subagents)
- **Offline Log Analyzers:** `python3 scripts/kotor2_logs.py status | check <log> [--new]` (routes every live log to its parser; `--new` = only lines since last check, no manual line offsets). Details: `kotor2-log-analysis` skill. NEVER `cat`/`tail`/`grep` raw `.txt` logs.

## Address Database & R&D Development Standards
- **Worktree:** `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db`
- **Branch:** `addressdb/k2-steam-acc`
- **Target DB:** `AddressDatabases/kotor2_steam_aspyr.db`
- **Git Commits:** Keep commits granular by subsystem during active R&D. Do NOT squash until all batches are completed and the user explicitly directs opening PR 5.
- **Validation Standard (Zero Forced Guesses):**
  - 16-byte boundary alignment (`0x...0`).
  - Standard x86 prolog (`55 8b ec ...` / `56 8b f1 ...` / `e9 ...`).
  - x86-32 ABI stack cleanup normalization: `RET n == ceil(param_size_bytes / 4) * 4`.
  - Semantic callee topology and struct offset verification.
- **Integrity Gate:** Always validate with `SqliteTools` before committing:
  ```bash
  /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/tools/SqliteTools/bin/Debug/net8.0/SqliteTools validate /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db
  ```
- **Live Game Deployment:** Sync validated database to:
  `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/addresses.db`

## KPM Install Rule
- `ethansm/local-mods` in `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager` mirrors what is live. Commit the mod's source there (explicit paths) **before** staging an install.
- Stage with `python3 patch_manager_mods/mods678_research/stage_and_verify_install.py stage <dir>`; it writes `<dir>/STAGE_INFO.txt` (branch + HEAD) and exits 2 on uncommitted mod sources (`--allow-dirty` to override). Note the `head=` sha in the mod's doc.
- Install with `KPATCH_NO_LAUNCH=1 .../bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches <dir> <all live ids>` (game closed; doc 10 "Standing rules").
- PR branches are cut from `master` in a separate worktree: `git -C $K worktree add ../kpm-pr-<name> -b <branch> master`.

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

## Subagent Output Contract (5-Line Summary Schema)
When spawning subagents via `Agent(...)`, always append this exact contract to the prompt:
```markdown
## Output Schema (Strict 5 lines max - NO conversational prose, NO diffs, NO compiler output):
STATUS: <PASS | FAIL>
CREATED: <comma-separated relative paths, or None>
MODIFIED: <comma-separated relative paths, or None>
TESTS: <e.g. 52/52 PASS, COV: 84%, mutation verified>
NOTES: <one sentence on blockers/traps, or None>
```
Orchestrators verify via `git status --short` and `run_tests.py`, never by reading raw subagent prose.

## AI Agent Output Style (`i-have-adhd`)
- The `i-have-adhd` plugin is installed globally across Claude Code (`i-have-adhd@i-have-adhd`) and Antigravity (`agy`).
- **Human Reader Scoping**: When communicating with the human user, shape all responses for an ADHD reader: lead with actions/commands/snippets, number multi-step work, cap lists to 5 items, give concrete time estimates, and end with one concrete next action doable in under 2 minutes.
- **Subagent Exemption**: When reporting back to an AI orchestrator or tool caller, the human ADHD rules are superseded by the 5-line schema above. Do NOT suggest next steps, do NOT paste code snippets, and do NOT number multi-step prose.

