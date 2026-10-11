### Task c: Document --json flags in SUBAGENT_ROLES.md
- **Contract**: worker
- **Write Scope**: `patch_manager_mods/SUBAGENT_ROLES.md`
- **Target Files**: `patch_manager_mods/SUBAGENT_ROLES.md:82-105`
- **Objective**: Add one bullet under **Tooling and caps** (after L99) in the "Claude Code Haiku swarm (waves)" section documenting both new flags: `python3 scripts/wave_budget.py check <entry>... --json` prints the check() dict as JSON (exit codes unchanged); `python3 scripts/wave_tools.py list <wave_id> --json` prints the units as a JSON array (id, est_tokens, description). Doc-only edit.
- **Data / Inlined Snippets** (copied from the w1-ACC1 u03 facts file; anchors re-checked with `rg -n` on 2026-10-11):
  ```
  patch_manager_mods/SUBAGENT_ROLES.md:82: ## Claude Code Haiku swarm (waves)
  patch_manager_mods/SUBAGENT_ROLES.md:96: - Tools: `python3 scripts/wave_budget.py check <entry>...`; `python3 scripts/wave_budget.py windows <path>`.
  patch_manager_mods/SUBAGENT_ROLES.md:98: **Tooling and caps**
  patch_manager_mods/SUBAGENT_ROLES.md:99: - `python3 scripts/wave_tools.py new|add|list|check|report`. Per-wave baseline: `agent_contract.py snapshot --out <file>`.
  patch_manager_mods/SUBAGENT_ROLES.md:100: - Caps: <= 12 parallel read-only recon; <= 6 parallel write workers. Above 12 units, print the estimated total first.
  scripts/wave_budget.py:129: p_check = sub.add_parser("check", help="check entries against the wave budget")   (no --json yet, task a)
  scripts/wave_tools.py:498: p_list = sub.add_parser("list", help="list the units of a wave")   (no --json yet, task b)
  scripts/wave_tools.py:505: p_check.add_argument("--json", ...)   (exists, undocumented; out of scope)
  ```
- **Guardrails**:
  - Gate: run only after tasks a and b have landed. As of 2026-10-11, `scripts/wave_budget.py` has no `--json` and `scripts/wave_tools.py` `list` has no `--json`. If either flag is missing when this task runs, stop and report FAIL. Do not document flags that do not exist.
  - Edit only `patch_manager_mods/SUBAGENT_ROLES.md`. No `.py` edits, no edits outside the Write Scope.
  - Leave the existing bullets at L96 and L98-100 unchanged. Add one bullet only.
  - Do not document `wave_tools.py check --json` (L505) unless the orchestrator asks.
- **Verification**: `python3 scripts/run_tests.py -k wave_tools`
