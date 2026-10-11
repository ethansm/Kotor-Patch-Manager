# REA integration kickoff prompt (for Haiku 5.5)

Paste everything below the line into a fresh Haiku 5.5 session in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`.
Part A runs now. Part B is pasted after the human installs REA and restarts Claude Code (MCP tools do not exist until restart).

Corrections made to the Gemini draft (all checked 2026-10-10): see "Known-wrong assumptions" in the prompt.

---

# MISSION

Evaluate and prepare integrating `morluto/rea` (npm `rea-agents`, MCP server for reverse engineering) into the KOTOR II Patch Manager workspace, to replace or augment our home-grown Ghidra tooling. This session investigates, prepares and pilots. It does NOT change any mod source, address DB row, or tracked script.

You are the orchestrator. You do not do analysis yourself. Every step below is executed by narrow Haiku 5.5 workers (the Haiku swarm). You dispatch, collect, check, and report.

# KNOWN-WRONG ASSUMPTIONS (do not act on the original draft)

1. REA cannot connect to `ghidra.openkotor.com:13100`. It drives a LOCAL Ghidra 12.1.x headless against a LOCAL binary in a temporary read-only project. Server access stays with `python3 scripts/ghidra_server_api.py` (see skill `openkotor-ghidra-server`). Never read `~/.config/ghidra/.server_credentials` or any `*credential*`/`*secret*` file.
2. `mod_core.lib`, `mod_tests.exe`, `IEngineBridge`, `Patches/Common`, `latest.patch` do not exist here. Real layout: `patch_manager_mods/_shared_src/` (shared headers, `GameAddr.h`), per-mod `patch_manager_mods/<mod>_src/` with `test.toml`, `*_test.cpp`, `common_contract_test.cpp`; harness `tests/include/TestHarness.h`; runners `scripts/run_tests.py`, `scripts/build_check.py`. Do NOT design a bridge/mock layer in this session.
3. `0x00775703` is the QH4 hook site INSIDE `CSWCCreature::PerformCastSpell`; the function entry is `0x00775700` (`patch_manager_mods/_shared_src/GameAddr.h:69`). "CompanionSelfHeal member" does not exist: `CompanionSelfHeal` is a mod (`companion_selfheal_src/`), not a struct field.
4. REA docs do not use "direct" or "enhanced" tool families. Discover real tool names from the connected MCP tool list after restart.
5. `npx rea-agents setup` is interactive (pick agents, review diff, approve) and needs a Claude Code restart. You cannot run it unattended, and you cannot use REA tools in the same session that installs it.
6. `CLAUDE_CODE_DISABLE_ADAPTIVE_THINKING=1` and `--effort low` are unverified. Use the project's existing swarm argv conventions (`.claude/plans/swarm_library_architecture.md` section 17); do not invent flags.

# HARD RULES

- No `git add`, `git commit`, no pushes, no GitHub posts, no edits to `~/.claude/settings.json` or `~/.claude.json` by you. Commits and config changes belong to the human.
- Pin the REA version in every command (`rea-agents@6.3.0`), never `@latest`. Run downloaded code only from a scratch dir (`/tmp/claude-1000/rea_probe`), with Python as `python3 -I`.
- Never open the live game exe. REA works on a COPY in the scratch dir.
- Do not run `scripts/` that install, hot-swap, or touch the game dir. Do not cat/tail raw mod logs.
- Cost: every worker report must come from `python3 scripts/agent_usage.py --since <start ISO8601>`. Part A cap: 1.50 USD. Stop and ask before exceeding.

# SWARM RULES (apply to every step)

- Dispatch with the Agent tool, `model: haiku`. 4-6 workers in parallel, disjoint write scopes.
- One worker = one deliverable, at most 3 files read in full, files over 600 lines read by `rg`/`sed -n` ranges only, final report at most 200 words. Include a stop condition in every brief.
- Brief = `Contract: recon` (or `worker`) + `Write Scope: <glob>` + output of `python3 scripts/agent_contract.py print recon|worker`. Run `python3 scripts/agent_contract.py snapshot` before launch and `python3 scripts/agent_contract.py check --scope <globs> -` after each wave.
- Workers pass PATHS to each other, never pasted content. Worker output goes to `.claude/plans/rea_integration_2026-10-10/out/<ID>.md`.
- Scripts compute, models judge narrow questions (`patch_manager_mods/ghidra_knowledge/RE_AGENT_GUIDE.md` section 1): pre-compute hex/delta facts in a script and state them in the brief.
- Ladder: Layer 1 = N differently-styled Haiku workers; Layer 2 = comparator Haiku merges and lists disagreements; Layer 3 = a second differently-styled Haiku pass on disagreements only. Sonnet only for items still unresolved after Layer 3, and say so in the report.
- A worker's PASS is a claim, not proof. Verify one claim per wave yourself with `rg`/`sed -n` before trusting the wave.

# PART A: investigate and prepare (run now)

Start: `python3 scripts/check_kpm_updates.py --quiet` (print only if it outputs replies). Create `.claude/plans/rea_integration_2026-10-10/out/`.

Wave A1 (recon, 5 parallel Haiku workers, read-only):
- A1-docs: WebFetch `https://github.com/morluto/rea/blob/main/docs/installation.md`, `docs/cli.md`, `docs/mcp-contracts.md` (generated tool catalog). Report: Claude Code registration (exact `rea setup --client claude_code --dry-run --json` usage), Ghidra provider env vars, the exact tool names for native analysis (open/close, decompile, xrefs, function dossier, search), which tools write session metadata, and what files setup modifies.
- A1-ours: inventory our Ghidra tooling REA could replace or augment: `scripts/query_ghidra.py`, `scripts/ghidra_server_api.py`, `scripts/export_server_knowledge.sh`, `patch_manager_mods/ghidra_knowledge/tools/`, `patch_manager_mods/ghidra_knowledge/README.md`. One line each: purpose, inputs, consumers. Do not open credential files.
- A1-env: read-only readiness. `node -v`, `java -version`, Ghidra at `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC` (REA needs 12.1.x), and from the scratch dir `npx -y rea-agents@6.3.0 doctor --json` with `GHIDRA_INSTALL_DIR` set for that command only. Report doctor output verbatim (trimmed to errors/warnings).
- A1-exe: decide which binary to analyze. Find which exe hash our Steam address DB and `ghidra_knowledge` are keyed to (`patch_manager_mods/ghidra_knowledge/README.md`, `patch_manager_mods/BUILD_AND_INSTALL.md` for the game dir `$G`, memory note: live exe is vanilla `6A522E` + runtime 4gb patch). Report path and sha256 of the unpatched copy to analyze. Copy it to `/tmp/claude-1000/rea_probe/swkotor2.exe` (copy only).
- A1-tests: how host tests are structured today (`tests/include/TestHarness.h`, one mod's `test.toml` + `*_test.cpp`, `scripts/run_tests.py` filter syntax). Report whether any code path in tests/mods consumes Ghidra output, and where REA evidence could be stored as a fixture. No design.

Wave A2 (you, no worker): check A1 reports against the hard rules and the known-wrong list. If A1-env doctor fails, list the single blocking item.

Wave A3 (doc worker, 1 Haiku): write `.claude/plans/rea_integration_proposal.md` from the A1 outputs by path: what REA replaces, what stays (server export, address DB, KpLog), risks (version pinning, session metadata writes, headless startup time `REA_GHIDRA_STARTUP_TIMEOUT_MS` for a ~10 MB exe), and the Part B pilot definition. At most 2 pages.

Part A ends HERE. Do not run `setup`. Print the human steps (the human runs them with the `!` prefix because setup is interactive):
1. `cp ~/.claude/settings.json ~/.claude/backups/settings.json.pre-rea && cp ~/.claude.json ~/.claude/backups/claude.json.pre-rea`
2. `! GHIDRA_INSTALL_DIR=/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC npx rea-agents@6.3.0 setup --client claude_code --dry-run --json` (review), then rerun without `--dry-run`.
3. Restart Claude Code, open a new Haiku session, paste Part B.

# PART B: pilot parity test (paste after restart; REA MCP tools must be listed)

Goal: prove REA reproduces what our current tooling already knows, on one function, before any integration work.

Target: `CSWCCreature::PerformCastSpell`, entry `0x00775700` (Steam vanilla exe copy at `/tmp/claude-1000/rea_probe/swkotor2.exe`; Layer-1 workers must not be told the expected answers).

Layer 1 (3 Haiku workers, recon, three different prompt styles: "evidence-card", "free-form explain", "checklist"; each uses the REA MCP tools only, reads no local C++): open the copy with `provider_id: "ghidra"`, then return raw tool evidence for the function: decompilation, instruction bytes at entry, callees, callers/xrefs, stack-arg count and `RET` immediate. Save raw JSON to `out/B1-<style>.json` and a 150-word summary to `out/B1-<style>.md`. Stop after one function. Close the binary at the end.

Tier 0 (script, you write it in the scratch dir, not the repo): compare Layer-1 results against ground truth that ONLY the script sees: `rg` of `patch_manager_mods/29_paused_command_evidence/PQ1.md` (lines 39, 88) and `patch_manager_mods/30_queue_hud.md:77`; `python3 scripts/query_ghidra.py address 0x00775700`. Expected facts to check: prologue bytes at `0x00775703` are `83 ec 0c 89 4d f4`; `__thiscall`; `RET 0x1C`; 7 DWORD stack args; calls `0x0087a5c0`. Print per-worker match/mismatch counts, nothing else.

Layer 2 (1 Haiku comparator): merge by path, list disagreements and which tool calls produced them. Layer 3 only on disagreements. Report: did REA reproduce each fact, how many MCP calls and how long per worker (from `agent_usage.py`), and which of our scripts it could replace.

Then update `.claude/plans/rea_integration_proposal.md` with the pilot result (doc worker). Do not wire REA into any script.

# FINAL REPORT (to the human)

Use the 4 sections `### Actions Completed`, `### What Now Works`, `### Agents Used` (worker count, contract status, cost from `agent_usage.py`), `### Saved State` (working tree status, no commit made, `Safe to /clear`, next command). Lead with the answer. End with one next action doable in under two minutes.
