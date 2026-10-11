# REA integration handoff (not a patch_manager_mods plan)

Hand-written 2026-10-10. Not from `plan_manager.py handoff`: that command writes `.claude/handoffs/<active plan stem>.handoff.md` and rewrites `.claude/active_plan.txt`, and both point at the equipment plan.

## 0. Next-phase direction (verbatim)
Plan-mode request, 2026-10-10 evening (verbatim): "persist context, then switch to plan mode and create a detailed plan for fixing the score and any other remaining tasks. I'll use clear context in apply". Plan file: see §3.

Original direction. REA subagent rollout. Next phase: (1) smoke-test the rea-recon subagent (user-scope ~/.claude/agents/rea-recon.md, inline rea server, Haiku, read-only allowlist) and confirm the tools syntax loads; (2) run the Part B one-function parity pilot through rea-recon instead of main-session MCP tools, updating .claude/plans/rea_integration_partB_prompt.md accordingly; (3) user removes the global user-scope rea registration (claude mcp remove rea --scope user, separate terminal, session idle) only after the smoke test passes; (4) confirm in B0 that Ghidra provider serves each allowlisted tool, including resolve_containing_procedure (for decoys) and whether batch_decompile is served (currently removed from the allowlist); (5) build rea-row-verify (Haiku, batch 20+ rows, four checks from .claude/skills/kotor2-address-db/SKILL.md plus neighbour decoys, PASS/FAIL/UNKNOWN with evidence addresses, reuses the pilot's tier0 scoring); (6) decide hook-abi-check (separate agent because it needs Read/Grep on mod sources; compares hook convention and RET n against annotations; refuses on exe sha256 mismatch with target_versions). Deferred: crash-addr-resolve. No commit, no push.

Plan changes accepted at approval: item 5 re-scoped to a card reviewer (`batch.py gates` already runs the G2/G4 checks at $0); item 6 dropped (a detour's `address` is a mid-function patch site, so comparing it to the engine function is a false premise).

## 1. Stage A status (plan start 2026-10-10T18:42:03-04:00)
- **A1 preflight: PASSED** (rerun 2026-10-10 evening, after restart). `rea-recon` is in the Agent type list. Exe sha256 starts `6a522e71631d`, no `/tmp/rea-ghidra-*` dirs, grep gates empty, `agent_contract.py snapshot` taken.
- **A2 tool gate: done.** `out/B0-tools.md` (see §2).
- **A3 smoke test: PASSED on rerun.** First run (`out/A3-smoke.md`, global registration present): PARTIAL, both procedure calls `provider_timeout` at 330000. Rerun (`out/A3-smoke-rerun.md`, global removed): 5/5 steps OK through `rea-recon`. `procedure_info` → `FUN_00775700`, 82 bytes. `procedure_callees` → 2 callees (values in `out/A3-smoke-rerun.md`). No `timeout_ms` appeared on any call, so the rerun alone does not prove the override. The env check below does.
- **A3 env check: server env confirmed.** A server spawned by the `rea-recon` subagent has `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000` (captured from `/proc`, scratchpad `env-check.txt`). Subagent check (`out/A3-env-check.md`): `procedure_info` OK, no `timeout_ms` key in any result. A successful call cannot show a timeout, so the captured env is the evidence.
- **A4 `rea-row-verify`: written** (`~/.claude/agents/rea-row-verify.md`). Not loaded yet.
- **A5 Part B prompt: updated.** Verify grep for ground-truth strings is empty.
- **A6 global removal: DONE** (user, `claude mcp remove rea --scope user`). `~/.claude.json` has no `mcpServers.rea`. Ran before the A3 rerun, as §3 changed the order.

## 2. Corrections to the previous state
- **`batch_decompile`:** the old note said it is not served. The live A2 call reached the REA workflow layer and failed per item with `provider_timeout`, not a capability error. Added to both agent `tools:` lists.
- **`resolve_containing_procedure`:** declared by the Ghidra adapter (`dist/ghidra/GhidraInventoryValues.js`). The live call timed out, not refused. Added to both lists.
- **Timeouts (root cause, verified):** `REA_GHIDRA_STARTUP_TIMEOUT_MS` sets the Ghidra startup deadline (`dist/config/ghidraStartupTimeout.js`, `GhidraClient.js` `#start`). Default 330000. The first procedure call on a binary runs the Ghidra import and analysis, so it hits this deadline.
- **Env override is the fix, and it works in the CLI.** Outside MCP, `rea function <exe> 0x00775700 --provider ghidra` with `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000` exited 0 in 356 s. Output: `FUN_00775700` at `0x775700`. Raw JSON: `fn-900.json` in the session scratchpad (`44dd8866-…/scratchpad`). The default (330 s) is just short of the 356 s needed.
- **Env did not reach the subagent's server while the global registration existed (run 1).** The one REA server running had only `GHIDRA_INSTALL_DIR` and `JAVA_HOME`, the global registration keys. The inline `env` block was not in it. Best explanation: the subagent reused the global `rea` server by name. **After A6 the inline env arrives** (see §1, A3 env check). No `command: env` wrapper is needed.
- `rea doctor` reports `unsupported_host` for pop 24.04 (REA lists Ubuntu 24.04+). Effect on Ghidra unknown; the CLI run worked anyway.
- The temp dir `/tmp/rea-ghidra-XwrE7q` existed while the binary was open. `close_binary` removed it.

## 3. Next action
**Post-clear resume:** do NOT run `python3 scripts/plan_manager.py resume` or `handoff`. They read the equipment plan. Resume this work with: `Continue .claude/handoffs/rea_integration.handoff.md at §9 (commit scope set; non-blocking plan in the plan-mode file)`.

**Stage B (Part B pilot) ran 2026-10-10 21:46 to 22:20 (B0 to B6). Tier-0 gate: NOT GO.** Scores 5/5, 4/5, 3/5 (evidence-card, checklist, free-form). Two scorer artifacts: F2 matched a negative sentence, and F3 failed to parse a correct value that had trailing text. Results: `out/B1-*`, `out/B3-compare.md`, proposal §7. Cost $0.0417 (5 Haiku runs, cap $0.60). No commit. **Re-score under scorer v2 (D1):** 4/5 each, observable gate GO (3/3). The NOT GO above is the v1 result, kept as the record.
- Scorer defects, fixed in v2 (`/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/tier0_compare.py`; v1 kept as `tier0_compare_v1.py`): F2 was a substring test on the convention field, now an affirmative lead with no hedge words. F3, F4, and F5 parsed the whole field as one number, now a leading token. F6 is informational by design (not in `FIELDS`; printed with `--info`), not a defect.
- Decided (D1): the gate is observable F1, F3, F4, F5. F2 is reported as the known REA label gap. Re-score done with no new REA calls: 4/5 each, gate GO (3/3).
- T5 `rea-row-verify` smoke test, run once (start 2026-10-10T23:43:24-04:00; Haiku; $0.0277; output `out/R1-smoke.tsv`). **Pass rule NOT met.** Pass needs 2 of 3 positives `match` with an anchor, and the control not `match`. Result: card 1 (B13a-1-39, expected match) `unsure`; card 2 (B13a-1-38, expected match) `match` with anchors; card 3 (B13a-1-08, expected match) `unsure`; control (B13a-1-01 GOG side, decoy candidate) `mismatch`. Header matches the agent definition. Write scope clean. No temp dirs left. Not retried (plan rule).
- Causes to settle before any rerun: (a) card 1: GOG string `AFx_Shadow_L` is not in Steam (Steam holds `Fx_Shadow_L`). The leading `A` may be a GOG fingerprint artifact. OPEN. (b) card 3: the brief stripped the GOG-to-Steam callee mappings, so the callee anchor could not be checked. That was a brief-design choice on my side, not a verifier failure. (c) card 1's reason cites two exact GOG string matches yet returns `unsure`. Check that reasoning before relying on the verifier.
- T5 rerun (the user approved one rerun; start 2026-10-10T23:55:47-04:00; $0.0293; output `out/R1-smoke-rerun.tsv`; callee Steam mappings restored; `in candidate` flags kept out). **Pass rule still NOT met.** Card 1 `unsure` (same GOG string issue as run 1). Card 2 `match` with anchors (exact size 592, offsets 0x30c and 0x11b0). Card 3 `unsure`: the verifier measured the candidate at 55 bytes, but the card records 57, and `SendPlayerToServerInput_Attack` stayed unverified (`GetSWCMessage` fit). Control `mismatch`. Run 1's size reading is not checked. No further retry without a new user decision.
- Run 3 would test two fixes: the card 1 string corrected to the true literal, and a size rule in the agent definition. See plan Step 6.
- Optional user step B7: exit Claude Code, then `ls -d /tmp/rea-ghidra-* 2>/dev/null` (settles proposal §6 item 1).

**Stage A is done** (A1, A2, A3 rerun, A4, A5, A6). The `agent_contract.py check` fails for unrelated dirty paths from other sessions, and each check appends a `CONTRACT_CHECK` line to `.claude/master_ledger.jsonl`.
- Restart fallback: if a future session shows `timeout_ms=330000` from `rea-recon`, the global registration has come back. Remove it with `claude mcp remove rea --scope user`, restart, and re-run A3.

## 4. Files
Repo (nothing committed):
- `.claude/handoffs/rea_integration.handoff.md` (this file)
- `.claude/plans/rea_integration_partB_prompt.md` (updated, A5)
- `.claude/plans/rea_integration_2026-10-10/out/B0-tools.md` (A2)
- `.claude/plans/rea_integration_proposal.md` (untracked; §6 awaits the pilot)
- `.claude/plans/rea_integration_2026-10-10/` (investigation, plus `out/`)
- `out/B1-evidence-card.{json,md}`, `out/B1-free-form.{json,md}`, `out/B1-checklist.{json,md}` (Stage B worker outputs; do not overwrite)
- `out/B3-compare.md` (comparator), proposal §7 (pilot results), `out/A3-*.md` (Stage A)
- `.claude/plans/rea_integration_proposal.md` §6 and §7 updated in Stage B. §7 has no expected values.

Outside repo:
- `~/.claude/agents/rea-recon.md`: tools and `env` edited.
- `~/.claude/agents/rea-row-verify.md`: new. Not smoke-tested.
- `~/.claude.json`: `mcpServers.rea` removed by the user (A6).
- `~/.claude/projects/-home-ethansm-orca-projects-local-setup/memory/rea-integration.md` + `MEMORY.md` line: created in Stage B.
- `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/` (durable home, 0700, outside any repo): `tier0_compare.py` (v2, current), `tier0_compare_v1.py` (v1 record), `fixtures/{good,bad,invalid,negated,hedged,trailing}`, `leak_patterns.txt` (ground-truth patterns for the leak check; never copy them into a repo file or brief), `smoke/` (smoke-test brief cards and key; ground truth).
- Target exe: `/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/swkotor2_vanilla_6a522e71.exe` (sha256 starts `6a522e71631d`, same as the /tmp copy). The /tmp originals stay until a reboot.
- Scorer v1 vs v2 outputs: scratchpad `v1_baseline.txt` and `v2_result.txt` (session scratchpad, not in repo).

## 5. Dead ends (do NOT repeat)
- `plan_manager.py handoff "<text>"` routes to the active plan stem (equipment). Write this file directly.
- `setup` without `--yes` prints `needs_confirmation` and writes nothing.
- `rg -h` is ripgrep's help flag. Use `--no-filename`.
- `agent_contract.py check`: the reply goes before `--scope`.
- Writing `~/.claude.json` during an active turn: Claude Code rewrites it.
- Four procedure-level REA calls at once: all four timed out at 330 s (see §2).
- Previous note "`batch_decompile` not served": wrong (see §2).
- Running A3 while the global `rea` registration existed: the subagent's inline `env` is ignored, so the timeout stays at 330000 (see §2).
- `agent_contract.py check` appends a `CONTRACT_CHECK` line to `.claude/master_ledger.jsonl` on every run. It also flags 170+ unrelated dirty paths, so filter its output. Do not run it in a loop.
- A worker's OUTPUT line must name one path. `B1-evidence-card` named two, and the check rejects that as `NO_PATH`.
- Workers have no clock. The `tool_calls[].ms` fields are empty or the string "not reported". Do not gate on timings.
- Do not re-run B1 workers to get a different score. Re-score the existing outputs, since a re-run is a new sample and changes the pilot.
- Do not ToolSearch `mcp__rea__*` in the main session. The system notice says those tools are gone.
- Never put a ground-truth value in a brief or a repo file. Check with `rg -n -i -f /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/leak_patterns.txt <file>`.

## 6. Verification commands
- `sha256sum /home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/swkotor2_vanilla_6a522e71.exe` → starts `6a522e71631d`
- `ls -d /tmp/rea-ghidra-* /tmp/claude-1000/rea-ghidra-* 2>/dev/null` → no output
- `rg -n -i -f /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/leak_patterns.txt .claude/plans/rea_integration_partB_prompt.md .claude/plans/rea_integration_proposal.md .claude/handoffs/rea_integration.handoff.md` → no output (currently §1 and §2 may still list observed callees; see the plan)
- `rg -n 'mcp__rea(,|$)' ~/.claude/agents/rea-row-verify.md` → no output (`snapshot_path` appears only in prohibition lines)
- `rg -n 'mcp__rea__(resolve_containing_procedure|batch_decompile)' ~/.claude/agents/rea-recon.md ~/.claude/agents/rea-row-verify.md` → two matches per file
- `python3 /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/tier0_compare.py /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/fixtures/good` → `B1-evidence-card: match=5/5 mismatch=none`
- `jq -c '.mcpServers // {} | keys' ~/.claude.json` → `[]` (A6 done)
- `python3 /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/tier0_compare.py .claude/plans/rea_integration_2026-10-10/out` → `B1-checklist: match=4/5 mismatch=F2`, `B1-evidence-card: match=5/5 mismatch=none`, `B1-free-form: match=3/5 mismatch=F2,F3` (pre-fix baseline)
- `python3 /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/tier0_compare.py .claude/plans/rea_integration_2026-10-10/out --info` → adds `info_F6` per worker (F6 is unscored)
- **Post-fix baseline (scorer v2, 2026-10-10):** `python3 /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/rea_probe/tier0_compare.py .claude/plans/rea_integration_2026-10-10/out` → `B1-checklist: match=4/5 mismatch=F2`, `B1-evidence-card: match=4/5 mismatch=F2`, `B1-free-form: match=4/5 mismatch=F2`, then `gate(observable F1,F3,F4,F5): GO (3/3 workers 4/4)`. The v1 values above (5/5, 4/5, 3/5) are the record of the NOT GO result.
- Scorer fixtures: `fixtures/negated` → `B1-negated: match=4/5 mismatch=F2`; `fixtures/hedged` → `B1-hedged: match=4/5 mismatch=F2`; `fixtures/trailing` → `B1-trailing: match=5/5 mismatch=none`.

## 7. Open questions (answer by test, do not assume)
- Does `tools:` accept the `mcp__rea__*` names as a comma-separated list? **Yes** (A3: ToolSearch loaded all 4, `open_binary` OK).
- Do procedure calls succeed with `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000`? **Yes in the CLI** (356 s, exit 0). **Yes through the subagent** (A3 rerun, 5/5). The env reaches the subagent's server (captured `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000`).
- Verdicts in `rea-row-verify` are match, mismatch, or unsure, per the plan. The name check (`label_suspect`) is not scored; name contradictions go in `reason`. Decide before the benchmark.
- Does a normal exit orphan the temp dir? B7 answers.

## 8. Unrelated
- The equipment handoff §0 question is not blocking: keep or replace it with equipment work?

## 9. Commit scope and non-blocking items (persisted 2026-10-11)
**Commit scope (user decision):** commit `out/` except these 7 files. They stay on disk, untracked, and are not moved. They match the ground-truth patterns (§5 rule):
- `out/A3-smoke-rerun.md`, `out/B1-checklist.json`, `out/B1-evidence-card.json`, `out/B1-evidence-card.md`, `out/B1-free-form.json`, `out/B1-free-form.md`, `out/B3-compare.md`

These 7 files remain in `out/` on disk. Never stage `out/` as a directory.

Nothing is committed yet. Commit only on the user's `commit`, with an explicit pathspec.

**Evidence from read-only checks (2026-10-11, no REA calls):**
- Card 3 size is 57. `steam_functions.tsv` says 57, and the exe bytes agree: `RET 4` at 0x7756f6, `CC` padding to 0x7756ff, next function at 0x775700. Run 2's "55" was not a REA value (REA returned 28, 31, 33, 60, 116 in run 2). It equals 0x7756f6 - 0x7756c0 + 1, so the verifier likely counted to the RET's first byte.
- Card 1 literal: the `A` is a Ghidra string-table artifact. GOG `.rdata` at 0x9867b8 holds `00 00 20 42 00 00 f0 41` (floats 40.0 and 30.0) and then `Fx_Shadow_L`. The `A` is the high byte of float 0x41f00000. GOG function 0x420130 pushes 0x9867c0 (at 0x420216), the start of `Fx_Shadow_L`, never 0x9867bf. GOG and Steam use the same literal. `fp.tsv` lists 0x9867bf because it snaps to the table's string start. `ACAMERAHOOK` (GOG) vs `[@CAMERAHOOK` (Steam) shows the same stray-prefix artifact.
- `functions.csv` line 79 is correct. No DB row change.
- Both run-2 `unsure` verdicts trace to data or arithmetic (card 1: bad GOG string in the card; card 3: self-computed size and an unmapped second callee), not to REA.
- B13a string-artifact count (read-only, 2026-10-11, script kept in the session scratchpad, not in the repo): 99 cards in `chunk_1.md` and `chunk_2.md`. 4 cards carry GOG strings (8 strings). 6 match a Steam string exactly. 1 is a Steam string plus one stray leading character (the card 1 case). 1 has no Steam match. The artifact shows in 1 card. A `tools/batch.py` or `bdlib.load_server_strings` fix would be a separate plan with colocated tests.

**Non-blocking items:**
1. Card 3 size: CLOSED (57).
2. Card 1 literal: CLOSED (artifact).
3. `rea-row-verify`: stays off DB rows until a third smoke run is approved and passes (plan Step 6). User chose skip on 2026-10-11; no third run.
4. Part B prompt: mark Stage B complete (plan Step 2).
5. Scorer home: durable copy (plan Step 3).
