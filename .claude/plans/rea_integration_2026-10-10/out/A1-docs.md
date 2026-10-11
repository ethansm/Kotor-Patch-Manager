# A1 docs recon: morluto/rea (main branch)

Fetched 2026-10-10 via WebFetch. Every fact below came from a WebFetch summary, not verbatim page text. Names are as the summary returned them. Re-check exact spelling against the raw page before using in code.

Sources:
- S1 = docs/installation.md
- S2 = docs/cli.md
- S3 = docs/mcp-contracts.md

No fetch failed. No credential files were read. No commands were run.

## 1. `rea setup --client claude_code --dry-run --json`

Verdict: PARTIAL. The flags are stated. The exact combined command is not shown as an example.

- Usage: `rea setup --client <id>`, and `--client` is repeatable (S1). `claude_code` is a listed client ID (S1).
- Flags stated: `--client`, `--dry-run`, `--json`, `--yes`, `--all-detected`, `--install-hopper`, `--skill=false`, `--accessible` (S1).
- `--dry-run` gives a "read-only plan" (S1, example `rea setup --client codex --client cursor --skill=false --dry-run`). Dry-run returns status `planned` with exit 0 (S1, S2).
- `--json`: `rea setup --json` "returns the plan without applying it" (S1). Output is "indented on a terminal and compact when piped or redirected" (S2).
- The exact combination `rea setup --client claude_code --dry-run --json`: not stated. S1 gives the pi form `rea setup --client pi --dry-run --json`.
- Output shape: a plan plus a compact `.doctor` readiness projection (S1).
- Status values: `planned` (dry-run, exit 0), `cancelled` (exit 0), `needs_confirmation` (unapproved apply, exit 1) (S1). S2 also gives exit 1 for `needs_human`.
- Skill readiness fields: `identity.skill.state`, `installed_version`, `installed_tool_count`, `identity.catalog`. `installed_catalog_digest` was removed (S1).
- Doctor scoped fields: `scope.mode`, `scope_checks`, `informational_checks`, `environment_healthy`, `healthy` (S1).
- Full field schema: not stated.

## 2. Ghidra env vars

Verdict: CONFIRMED for both expected names.

| Name | Default | Meaning | Source |
|---|---|---|---|
| `GHIDRA_INSTALL_DIR` | not stated | Absolute path to an existing Ghidra 12.1.x install. Set before running doctor or setup. | S1 table |
| `REA_GHIDRA_STARTUP_TIMEOUT_MS` | 330,000 ms | Deadline for Ghidra import, analysis, bridge, and health. Integers 1 to 2,147,483,647. Empty or invalid values fail validation. | S1 table; S3 gives default and range |
| `GHIDRA_HEADLESS_MAXMEM` | falls back to `GHIDRA_MAXMEM`, then `2G` | JVM `-Xmx` value, e.g. `512M`. Takes precedence over `GHIDRA_MAXMEM`. | S1 table |
| `GHIDRA_MAXMEM` | default not stated | Used when `GHIDRA_HEADLESS_MAXMEM` is unset. | S1 table |
| `JAVA_HOME` | not stated | Optional JDK path if `java`/`javac` are not on `PATH`. | S1 table |

- S2 reports `REA_GHIDRA_STARTUP_TIMEOUT_MS` as "not stated". S1 and S3 both name it, so this is a gap in the S2 summary, not a conflict.

## 3. Native-analysis MCP tool names

Verdict: PARTIAL. 2 of 6 names confirmed.

- open: `open_binary`. Accepts `provider_id` and `snapshot_path` (S2, S3).
- close: `close_binary`. Accepts `snapshot_path` and `overwrite: true` (S2, S3).
- decompile: MCP name not stated. S2 lists the CLI command `decompile`.
- xrefs: MCP name not stated. S2 lists the CLI command `xrefs`.
- function dossier: MCP name not stated. S2 lists the CLI command `function`. S3 names `analyze_function` as Evidence-producing, but the page does not map it to a dossier. Not inferred.
- search: MCP name not stated. S2 lists the CLI command `search`.
- Other MCP names on the pages, not mapped to the six above: `binary_overview`, `binary_session`, `get_navigation_context`, `inspect_address_context`, `export_evidence_bundle`, `extract_artifact`, `cleanup_incomplete` (S2, S3).

## 4. Tools that write session metadata, and where

Verdict: PARTIAL. The disk location of the session bundle is not stated.

- Snapshot files: written via `--snapshot` (CLI) or `close_binary`'s `snapshot_path` (S2). Files are "local and use owner-only permissions" (S2). Default snapshot location: not stated.
- `export_evidence_bundle`: writes a bundle to a caller-supplied destination path (S3).
- `extract_artifact`: writes to a fresh temporary directory (S3).
- Evidence-producing tools return full records "retained in the session bundle". Retention is tied to the connection's Evidence ledger, which `close_binary` clears (S3).
- Which tool writes session metadata to a specific disk path: not stated.

## 5. Files `setup` modifies

Verdict: PARTIAL. For `claude_code`, only the skills path is stated.

Claude Code:
- Skills: `~/.claude/skills`, or `$CLAUDE_CONFIG_DIR/skills` when that variable is set (S1).
- MCP registration file for Claude Code: not stated (S1, S2, S3).
- Backups: `.rea.backup` files are "retained rather than overwritten" (S1). Which file gets the suffix: not stated.
- Hopper, macOS only, and only with `--install-hopper`: `~/Applications/Hopper Disassembler.app` (S1).
- `--purge-data` on uninstall removes `~/.rea` (cache and state) (S1).
- Setup "requires confirmation before changing configuration or installing Hopper" (S2).

Other clients, for reference (S1):
- Pi: `~/.pi/agent/mcp.json`. Personal skills: `~/.agents/skills`.
- Qwen Code: `~/.qwen/settings.json` or `$QWEN_HOME/settings.json`.
- Gemini CLI: `$GEMINI_CLI_HOME/.gemini/settings.json` and `$GEMINI_CLI_HOME/.agents/skills`.
- OMP: `~/.omp/agent/mcp.json`, or `~/.omp/profiles/<name>/agent/mcp.json`.
- Hermes: `config.yaml` under `HERMES_HOME` or the platform default.
- Grok Build: `$GROK_HOME/config.toml` or `~/.grok/config.toml`.
- Devin: `devin/mcp_config.json` under `XDG_CONFIG_HOME`.
- Antigravity 2.0 skills: `~/.gemini/config/skills`.
- DeepSeek Harness: `$DSH_HOME/cordis.patch.yml` is edited by hand and not written by setup.
- Codex config file: not named. OpenCode config file name: not stated beyond JSON/JSONC.

## OPEN

- The Claude Code MCP registration file path and the on-disk session bundle location are not stated on S1, S2, or S3.

## Evidence anchors

- docs/installation.md: Ghidra env table; Claude Code skills path; setup file list
- docs/mcp-contracts.md: `REA_GHIDRA_STARTUP_TIMEOUT_MS` default and range; `open_binary`/`close_binary`; `export_evidence_bundle`/`extract_artifact`
- docs/cli.md: `open_binary`/`close_binary`; `--snapshot`; CLI commands `decompile`/`xrefs`/`function`/`search`
