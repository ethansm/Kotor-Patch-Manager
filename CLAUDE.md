# CLAUDE.md — KOTOR II Workspace Guidelines

## Task Routing (load the skill before the task)
- Address-DB rows / Steam DB batches / PR 5 → `kotor2-address-db` skill
- KPM install, DLL hot-swap, staging, release log tightening, kpm-pr-* worktrees → `kotor2-kpm-install` skill (reference: `patch_manager_mods/BUILD_AND_INSTALL.md`)
- Ghidra server (status, export, audit) → `openkotor-ghidra-server` skill
- Live mod logs → `kotor2-log-analysis` skill (`python3 scripts/kotor2_logs.py status | check <log> [--new]`; cross-mod combat picture: `check combat` = `scripts/combat_timeline.py`); NEVER `cat`/`tail`/`grep` raw `.txt` logs
- Crash/freeze/hang → `kotor2-crash-triage` skill
- Live-game-only checks never block or park an item: add a `pending` row (steps + pass signal) to `patch_manager_mods/PLAY_TEST_QUEUE.md`; the user plays the queue in batches. WI effort sizes: `.claude/work_items/SIZING_RUBRIC.md` + `sizing.tsv`
- Plan handoff / context clear → `/plan-handoff` skill; after clear run `python3 scripts/plan_manager.py resume <doc|phase|plan path>` (never bare: it reads the pointer shared across sessions; `handoff` refuses to run without a plan, pass `--plan <path>` or a shorthand). Stay in plan mode for "Clear context and approve"; do NOT `ExitPlanMode` early. Mark phases with `python3 scripts/plan_manager.py mark-done <phase> [--state <STATE>] [--commit <sha>] [--notes <txt>]` (auto-generates handoff & prints Saved State banner); NEVER sed the master plan. Both `/plan-handoff` and `plan_manager.py resume|handoff` accept shorthand (`25 §12`, `24:T6`, `19e`, `T5e`). Always output the 4-section protocol (`### Actions Completed`, `### What Now Works`, `### Agents Used`, `### Saved State`) upon step completion so context clearing is immediately actionable. Next actions ending turns should suggest concrete `/plan-handoff <batch>` targets rather than generic reviews.
- Dev work items (`.claude/work_items/dev_work_items.md`) → `/next-dev-work-item` skill (one fresh session = one committed item; parallel-safe via `python3 scripts/wi_driver.py next|verify|docs|close|report|park|status|lint|audit|backlog`); run `audit` every 5th item
- Parallel fix batches (Haiku variants → Sonnet consolidate/review) → `variant-fix-wave` skill (`python3 scripts/fix_wave.py select|draft-collect|plan|brief|status|apply|cost`; `plan` refuses oversized batches; `next` route = `select` picks the top ~10 ready WIs)

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
- Steam exe RE, free first: the static dumps answer size/RET/callers/callees/strings at $0 (`ghidra_knowledge/RE_AGENT_GUIDE.md` §2.1 lists the scripts); REA ledger hygiene `python3 patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py lint`; live REA only for indirect targets, body ranges, data xrefs, named decompile (plan `patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`)
- Agent tokens + est. cost for `### Agents Used`: `python3 scripts/agent_usage.py [--since <step start ISO8601>] [--all] [--json]` (reads the session's subagent transcripts; no model tokens)
- Reviewer upload bundles (one packed .txt per review): `python3 scripts/review_bundle.py list | plan <id>... | build <id>... [--out DIR] [--allow-over] [--line-numbers] [--all]` (manifest `patch_manager_mods/review_bundles.toml`; outputs in gitignored `.claude/bundles/`; over-cap bundles are not written)
- Search: `rg`; repo map: `python3 scripts/generate_repo_map.py [dir]`; docs: `python3 scripts/search_docs.py "<query>"`; subagent pack: `npx repomix [dir]`
- Logging: probe builds log richly (never guess blind live); release tightening rules live in `kotor2-kpm-install`.
- K1 -> K2 character import (install-time, assets never in the repo): `python3 patch_manager_mods/k1_port/k1_port_import.py npcs [--grep X] | scan <k1_utc> | doctor | build [--only ID] [--dry-run] | install | remove ID | list` (private config from `k1_imports.example.toml` (pick any K1 NPC as `utc:<resref>`; optional recipe overrides `k1_port/recipes/<id>.toml`), packs `.k2pack` loaded by `character_packs_src`; format `k1_port/K2PACK_FORMAT.md`); inspect a pack: `python3 patch_manager_mods/k1_port/k1_port_pack.py inspect <file>`
- Compat check: after touching `_shared_src` or a library, run `python3 scripts/compat_check.py <files>`
- Scoped tests: always scope `run_tests.py` (e.g. `run_tests.py <name|filter>` or `-k <filter>`; bare `run_tests.py` runs all 156 repo suites >60s). Positional filter names and suite IDs are supported.

## Verify-After-Fix Invariant
- Never call a test/gate failure "pre-existing" or "expected" without a known-issues block in the prompt or a written waiver; investigate first (stale smoke, version pin, allowlist).
- After editing code, manifests, `raw_addresses.toml` or kpatches to fix a failure, re-run that failing suite immediately.
- After `kpm_hotswap.py --prepare`, a fresh `scripts/pre_install_gate.sh --mod <x>_src` `GATE PASS` is required before `--apply`.
- No raw hex into `raw_addresses.toml` before `python3 scripts/query_ghidra.py address <hex>`.

## Context Budget & Read Policy (Anti-Bloat Standards)
- **Zero-Read / Self-Contained Plans**: In Plan Mode, inline the exact 5-10 line struct/interface or macro snippets directly in the plan. Never tell the executor to "Read first: `LargeHeader.h`".
- **Execution Fast-Path**: When executing an approved plan after `/clear`, the plan is self-contained. Proceed directly to targeted file edits and tests.
- **Forbidden Discovery Commands**:
  - NEVER run broad `ls` or recursive tree walks on mod directories on startup.
  - NEVER `cat` entire source files or headers just to inspect an API; use `sed -n 'X,Yp'` or `rg` for specific symbols.
  - NEVER read historical session context docs (`19*.md`, `15*.md`, etc.); active state is in your named handoff `.claude/handoffs/<plan_stem>.handoff.md` and the active plan. Several sessions share this worktree, so do not read `.claude/session_handoff.md`; it is no longer written.
  - Do NOT prepend redundant `cd $PWD` to shell commands.
  - Use native file edit tools (`Edit`/`Write`) instead of multi-line Python heredocs in Bash when modifying files.

## Subagent Contracts (supersede the ADHD style for subagents)
- See `patch_manager_mods/SUBAGENT_ROLES.md` for full role catalog, backend routing, and contract specs.
- Brief = `Contract: worker|recon` + `Write Scope: <globs>` + output of `python3 scripts/agent_contract.py print worker|recon`.
- **Backend policy (Claude Code)**: Do not use OpenCode subagents (`oc-subagent`, `oc-flow`, `oc-free`, `oc-local`) unless the user explicitly asks for them in Claude Code. Default to the Haiku swarm (parallel `model: haiku` Agent subagents).
- Wave recipe and budget gate: see `patch_manager_mods/SUBAGENT_ROLES.md` "Claude Code Haiku swarm (waves)".
- Order: worker CREATED,MODIFIED,TESTS,NOTES,STATUS · recon OUTPUT,EVIDENCE,FINDING,OPEN,VERDICT · review OUTPUT,EVIDENCE,BLOCKING,FINDING,DECISION. Verify: `agent_contract.py snapshot` before launch, `agent_contract.py check --scope <globs> -` after.
