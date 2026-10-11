# REA Part B parity pilot: Stage B (paste into a NEW session)

**Status: Stage B is complete (B0-B6 ran 2026-10-10). Do not paste this file to re-run it.** Re-running B1 overwrites `out/B1-*`, which must stay. Use scorer v2 and the observable gate (F1, F3, F4, F5) described in B2.

Restart Claude Code, open a new session in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`, then paste this whole file.

Stage A is done. The global `rea` registration is removed (A6). REA runs only inside the user-scope agents `rea-recon` and `rea-row-verify` (`~/.claude/agents/`). The main session has no `mcp__rea__*` tools.

## Ground rules
- Goal: check whether REA reproduces facts we already know for one function. Nothing is wired into repo scripts.
- No commit, no push. Write only to the paths named in each step.
- Cost cap 0.60 USD. Check with `python3 scripts/agent_usage.py --since <START>`, where START is the B0 timestamp. Stop and ask before exceeding.
- Workers are the custom agent `rea-recon` (Haiku). One deliverable each, at most 3 files read whole, a reply of 200 words or fewer, and a stop condition in every brief.
- Every worker brief starts with `Contract: recon`, then `Write Scope: <glob>`, then the output of `python3 scripts/agent_contract.py print recon`.
- Never read `~/.config/ghidra/.server_credentials` or any `*credential*` or `*secret*` file.
- Print `~/.claude.json` only through `jq` key selectors.
- Target: `/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/swkotor2_vanilla_6a522e71.exe`, sha256 `6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef`. Function entry `0x00775700`.
- Scratch and output: `.claude/plans/rea_integration_2026-10-10/out/`.

## B0. Preflight (orchestrator, 4 min)
1. Run `date -Iseconds` and save the output as START.
2. Run `python3 scripts/agent_contract.py snapshot`.
3. `sha256sum /home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/swkotor2_vanilla_6a522e71.exe` must match the hash above. `ls -d /tmp/rea-ghidra-* /tmp/claude-1000/rea-ghidra-* 2>/dev/null` must print nothing.
4. The Agent type list must show `rea-recon` and `rea-row-verify`. `jq -c '.mcpServers // {} | keys' ~/.claude.json` must not list `rea`. Any failure: STOP and report.
5. `out/B0-tools.md` (written in Stage A, A2) must exist. It is the record of which REA tools are served. Do not rewrite it. The tool list in `~/.claude/agents/rea-recon.md` is the read allowlist.

## B1. Three recon workers, ONE AT A TIME (5-10 min each)
Styles: `evidence-card`, `free-form`, `checklist`. The style only changes the filename label. The brief text is identical, so the three workers are repeats of one brief, not three prompt styles.

Each worker writes `out/B1-<style>.json` with these fixed fields: `entry`, `first_12_bytes_at_entry` (hex), `calling_convention`, `ret_immediate`, `stack_arg_count`, `callees` (hex list), `callers` (hex list, plus the containing-function entry if the tool gives one), `decompilation_excerpt` (at most 15 lines), and `tool_calls` (list of `{tool, args, ms, ok}`). It also writes `out/B1-<style>.md`, a summary of at most 150 words.

Launch each worker with the Agent tool, `subagent_type: "rea-recon"`. Brief template. Fill in `<style>` and the contract text. Add no expected values.
```
Contract: recon
Write Scope: .claude/plans/rea_integration_2026-10-10/out/B1-<style>.*
<output of python3 scripts/agent_contract.py print recon>

Task: describe one native function in /home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/swkotor2_vanilla_6a522e71.exe using REA MCP tools only.
Target: function entry 0x00775700.
1. Call open_binary with path "/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/swkotor2_vanilla_6a522e71.exe" and provider_id "ghidra". Never pass snapshot_path.
2. Fill the JSON fields listed above and record every tool call in tool_calls.
3. Write the two output files with the Write tool.

Budget: at most 25 tool calls.
Stop: call close_binary before replying, even after an error. Reply in 200 words or fewer.
```

For each worker:
1. Launch it only after the previous worker's notification. `ls -d /tmp/rea-ghidra-* /tmp/claude-1000/rea-ghidra-* 2>/dev/null` must print nothing before the next launch.
2. Contamination check, after the worker finishes: `rg -c '"name":"(Read|Grep|Glob|Bash)"' <that worker's output file>` must print nothing. Count only; never cat that file. A hit marks the worker contaminated: drop it from the parity score and rerun it with a fresh brief.
3. Confirm both output files exist.

After all three, pipe the worker reply in and run: `printf '%s\n' "<reply text>" | python3 scripts/agent_contract.py check --type recon - --scope '.claude/plans/rea_integration_2026-10-10/out/B1-*'`.

Timeouts:
- If the first `open_binary` times out (default 330000 ms): add `REA_GHIDRA_STARTUP_TIMEOUT_MS: "900000"` to the inline `env` of `rea-recon` in `~/.claude/agents/rea-recon.md`, restart Claude Code, and rerun B1 once.
- If a procedure call returns `provider_timeout` (330000 ms; seen in Stage A for `procedure_info` and `resolve_containing_procedure`): the env key for it is not confirmed. Do not guess a key. Record the call with `ok: false` and STOP.

## B2. Tier-0 scoring (orchestrator, 1 min)
```
python3 /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/tier0_compare.py .claude/plans/rea_integration_2026-10-10/out
```
Print its lines and nothing else.
Gate (decision D1): GO if every worker passes F1, F3, F4, and F5. F2 is the known REA label gap: it is printed but not gated. Otherwise go to B3.

## B3. Comparator (Haiku, 3 min, only if B2 is not GO or the three workers disagree on any field)
- One Haiku merges the three `B1-*.json` files by path. No REA calls. It lists disagreements and the tool calls that produced them. Write Scope: `out/B3-compare.md`.
- No Layer 3.

## B4. Measure and clean up (orchestrator, 3 min)
1. `python3 scripts/agent_usage.py --since <START>`. Must be at most 0.60 USD.
2. Per worker: MCP call count and total ms from `tool_calls`, and the duration of the first `open_binary`.
3. `ls -d /tmp/rea-ghidra-* /tmp/claude-1000/rea-ghidra-* 2>/dev/null`. Expect nothing. If any remain, list their sizes and ask before `rm -rf`.

## B5. Doc worker (Haiku, 3 min)
Write Scope: `.claude/plans/rea_integration_proposal.md` only.
- Add a "Pilot results" section of 250 words or fewer: facts reproduced (F1-F5 per worker), calls and time, and which repo scripts REA could replace.
- Update §6: item 3 answered (env keys `GHIDRA_INSTALL_DIR`, `JAVA_HOME`); item 5 answered from `out/B0-tools.md`.
- Keep the file at 1,100 words or fewer.

## B6. Memory and report (orchestrator, 3 min)
1. Save project memory `rea-integration` (type `project`) and add its one-line pointer to `MEMORY.md`.
2. Print the four-section report: Actions Completed, What Now Works, Agents Used, Saved State.
3. No commit. Offer one explicit-path commit command.

## B7. Optional, you (3 min)
Leave one session open, exit Claude Code, then run `ls -d /tmp/rea-ghidra-* /tmp/claude-1000/rea-ghidra-* 2>/dev/null`. This settles proposal §6 item 1.

## Stop conditions
- `rea-recon` or `rea-row-verify` is not in the Agent type list, or `rea` is in `~/.claude.json` (B0).
- An expected value appears in a brief or in a repo file you write.
- Cost is over 0.60 USD.
- A `rea-ghidra-*` directory remains and the user has not approved removing it.
