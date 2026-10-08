# Subagent Roles & Autonomous Delegation Specification

**Document:** `patch_manager_mods/SUBAGENT_ROLES.md`  
**Related:** `patch_manager_mods/MASTER_KEY.md`, `AGENTS.md`, `scripts/plan_manager.py`

This document defines the standard role-based subagent catalog (`opencode-subagents`), backend execution targets, output schemas, and contract rules. All active master plans and scaffolding templates reference this document.

---

## 1. Subagent Role Catalog (`oc-subagent`)

Invoke role-based subagents via CLI:
```bash
oc-subagent <role> [-m <model_override>] "<prompt>"
```

| Role | Target Backend | Primary Scope & Responsibility | Output Contract |
|---|---|---|---|
| **`scout`** | Local RTX 5070 (fails over to cloud) | Fast file discovery, offline log parsing (`kotor2_logs.py check`), and pattern lookup. | `recon` |
| **`verify`** | Local RTX 5070 (fails over to cloud) | Test result validation, log assertion matching, and behavioral checks. | `recon` |
| **`gate`** | Local RTX 5070 (fails over to cloud) | Build gate verification (`build_check.py`), hook span audits, and manifest checks. | `worker` |
| **`ghidra`** | Cloud-first (OpenRouter) | Ghidra address lookup, decompilation audits, calling convention analysis, and GameAddr resolution. | `recon` |
| **`code`** | Cloud-first (OpenRouter) | Targeted C++ / Python implementation strictly confined to the owning mod scope. | `worker` |
| **`test`** | Cloud-first (OpenRouter) | Host unit tests (`*_test.cpp`), contract tests (`common_contract_test.cpp`), and mutation test authoring. | `worker` |
| **`doc`** | Cloud-first (OpenRouter) | Master plan updates, handoff preparation, README/CHANGELOG sync, and architectural RFCs. | `worker` |
| **`review`** | Cloud-first (OpenRouter) | Code diff review, memory-safety audits, calling convention check, and regression hunting. | `recon` |
| **`deep`** | Cloud-first (OpenRouter) | Deep architecture planning, complex multi-file refactoring, and root-cause debugging. | `recon` |

---

## 2. Dedicated Backend Commands

- `oc-local "<prompt>"`: Strictly Local RTX 5070 (Qwen 3.5 9B). 100% offline, zero remote tokens.
- `oc-free "<prompt>"`: Strictly OpenRouter Free tier (auto-fallback cascade). Flags: `--type code|reasoning|fast`.
- `oc-pick-model [code|reasoning|deep|fast]`: Query best active bash model (`--list` for active context windows).

---

## 3. Plan-Review-Build Lifecycle (`oc-flow`)

- `oc-flow auto "<prompt>"`: End-to-end unsupervised plan and build in a single invocation.
- `oc-flow start "<prompt>"`: Read-only plan mode with high-context model (e.g. Nemotron 3 Ultra 1M ctx). Returns `SESSION_ID`.
- `oc-flow revise <session_id> "<feedback>"`: Injects critique into same session.
- `oc-flow fork-build <session_id> "[instr]"`: Clones plan to isolated session and executes with Laguna S 2.1.
- `python3 scripts/plan_manager.py delegate <phase> [--mode start|auto]`: Autonomously synthesizes self-contained prompt from active plan and launches `oc-flow`.

---

## 4. Subagent Output Contracts (`agent_contract.py`)

Subagents must format their final conclusion according to the assigned contract:

### A. Worker Contract (Code & Gate Changes)
```text
CREATED: <comma-separated repo-relative paths, or None>
MODIFIED: <comma-separated repo-relative paths, or None>
TESTS: <exact command> -> <result, e.g. 52/52 PASS>, or None
NOTES: <one sentence on blockers/traps, or None>
STATUS: <PASS | FAIL>
```

### B. Recon Contract (Read-Only Analysis)
```text
OUTPUT: <repo-relative output artifact path, or None>
EVIDENCE: <log line, Ghidra address, or code symbol confirming findings>
FINDING: <concise summary of discovery>
OPEN: <unresolved questions, or None>
VERDICT: <PASS | FAIL | INCONCLUSIVE>
```

---

## 5. Non-Negotiable Subagent Invariants

1. **Strict Owning Mod Scope**: Subagents MUST confine research and edits strictly to the specified mod directory (e.g. `patch_manager_mods/<mod>_src/`). Never modify unrelated mods.
2. **Zero Hardcoded Addresses**: Always use `K2Addr.h`, `GameAddr.h`, or resolved constants from Ghidra DB. Never hardcode magic hex offsets in logic.
3. **Automated Verification**: Always verify via `python3 scripts/build_check.py --mod <mod>_src` and `python3 scripts/run_tests.py -k <mod>`.
4. **No Polling**: Use offline log parsers (`python3 scripts/kotor2_logs.py check <log>`) instead of background loops.
