# 35 — Master Audit Ledger & Clef-Flash System-1 Routing Engine

**Document:** `patch_manager_mods/35_clef_systemone_matcher_and_ledger.md`  
**Related:** `patch_manager_mods/MASTER_KEY.md`, `patch_manager_mods/SUBAGENT_ROLES.md`, `AGENTS.md`  
**Components:** `scripts/master_ledger.py`, `scripts/clef_dataset.py`, `scripts/clef_matcher.py`, `scripts/test_master_ledger.py`, `scripts/test_clef_matcher.py`

---

## 1. System Overview

A dual-tier telemetry and routing architecture providing worktree-wide multi-agent safety, live audit logging, and sub-130ms System-1 routing via `Cloudflare/clef-flash` (port 8100, RTX 5070 / LM Studio / OpenRouter).

```
  Orchestrators (Antigravity / Claude Code)
                     │
                     ▼
  [scripts/clef_matcher.py] ◄─── (Cached Priors <1ms) ─── [scripts/clef_dataset.py]
         │ (HTTP System-1)                                         ▲
         ▼                                                         │
  Cloudflare/clef-flash (Port 8100, ~125ms)                        │
         │ (Role, Model, Write Scope)                              │
         ▼                                                         │
  Subagents (oc-subagent / OpenRouter / RTX 5070)                  │
         │                                                         │
         ▼ (Structured Telemetry: latency, tokens, actual_model)   │
  [.claude/master_ledger.jsonl] ───────────────────────────────────┘
```

---

## 2. Implementation Batches

| Id | Batch | Status |
|---|---|---|
| **CL1** | Master audit ledger & dynamic process-tree agent attribution | DONE |
| **CL2** | Test isolation guard & automated test data archival | DONE |
| **CL3** | Canonical mod identifier normalization | DONE |
| **CL4** | Bayesian Laplace smoothing & role-partitioned durations | DONE |
| **CL5** | MTime-gated sub-millisecond prior disk cache | DONE |
| **CL6** | Calibrated 9-class confidence threshold in clef_matcher | DONE |

---

## 3. Batch Details

### CL1: Master Audit Ledger & Process-Tree Attribution
- **Target**: `scripts/master_ledger.py`
- Append-only JSONL logging (`.claude/master_ledger.jsonl`) for high-impact actions: `HOTSWAP_APPLY`, `HOTSWAP_ROLLBACK`, `BUILD_GATE`, `PHASE_DONE`, `SESSION_HANDOFF`, `CONTRACT_CHECK`, `SUBAGENT_DIFF_CHECK`, `CLEF_ROUTE`, `OPENCODE_RUN`.
- Process tree walk inspection (`/proc/<pid>/cmdline`) dynamically tags `antigravity:<id[:8]>`, `claude:<id[:8]>`, `opencode:<role>`, and establishes `parent_agent` linking.
- Verification: `scripts/test_master_ledger.py` PASS.

### CL2: Test Isolation Guard & Legacy Archival
- **Target**: `scripts/master_ledger.py`, `scripts/run_tests.py`, `scripts/test_plan_manager.py`
- Automatic test harness detection (`is_test_environment()`, `MASTER_LEDGER_TEST=1`) drops writes to `.claude/master_ledger.jsonl` during unit testing while preserving caller in-memory records.
- Added `master_ledger.py prune-test-data` which moved 438 fake test entries (`demo`, `gate-unittest`, test plans) to `.claude/master_ledger.test_archive.jsonl`, leaving clean production history.
- Verification: Running 6 full test suites causes 0 lines added to `.claude/master_ledger.jsonl`.

### CL3: Canonical Mod Identifier Normalization
- **Target**: `scripts/master_ledger.py`, `scripts/clef_dataset.py`, `scripts/clef_matcher.py`
- Added `canonical_mod_name()` and `kpm_mod_name()`, unifying KPM manifests (`queue-hud`), plans, and source trees into canonical `queue_hud_src`.
- Filters and collision checks (`check-collision --mod queue-hud`) seamlessly match source-tree events.

### CL4: Bayesian Laplace Smoothing & Role-Partitioned Durations
- **Target**: `scripts/clef_dataset.py`
- Replaced raw empirical ratios with Laplace smoothing: $\hat{p} = \frac{\text{passes} + 1.9}{\text{runs} + 2.0}$, preventing dirty-tree checks from collapsing untested mods to 0.00%.
- Partitioned durations by role (`scout`: 2.5s, `verify`: 3.5s, `gate`: 4.0s, `code`: 45.0s, `deep`: 120.0s) instead of injecting a 141.7s unweighted global average.
- Extracted and indexed `tokens_in`, `tokens_out`, `tokens_cached`, and `actual_model` from session headers.

### CL5: MTime-Gated Prior Disk Cache
- **Target**: `scripts/clef_dataset.py` (`.claude/clef_priors.json`)
- Prior calculations cached to disk with atomic write/replace.
- Checks `clef_priors.json.st_mtime >= master_ledger.jsonl.st_mtime`, dropping router prior load times from 5ms to <1ms.

### CL6: Calibrated 9-Class Confidence Threshold
- **Target**: `scripts/clef_matcher.py`
- Replaced hardcoded `0.60` ambiguous threshold with a calibrated statistical dual check:
  - Absolute floor: Top-1 confidence $\ge 0.32$ (~3x uniform random chance of 11.1%).
  - Top-margin ratio: $\frac{P(\text{top1})}{P(\text{top2})} \ge 1.20$.
- Clef-Flash prompt `Fix pointer math in QueueHud hook detour` routes directly to `code` with `poolside/laguna-s-2.1:free` in ~125ms without false demotions to `review`.

---

## 4. Execution & Verification Log

```bash
# Test isolation & mod normalizer tests
python3 scripts/test_master_ledger.py  # 8/8 PASS in 0.004s

# System-1 router & prior tests
python3 scripts/test_clef_matcher.py   # 9/9 PASS in 0.22s

# Full ecosystem suite check
python3 scripts/run_tests.py test_clef_matcher test_master_ledger test_plan_manager test_agent_contract test_kpm_hotswap test_pre_install_gate  # 6/6 PASS in 1.6s

# Live System-1 Routing Proof
python3 scripts/clef_matcher.py route "Fix pointer math in QueueHud hook detour"
# Output: [CLEF-FLASH ROUTE (125.3ms, conf=0.41)] -> role=code, model=openrouter/poolside/laguna-s-2.1:free, scope=['patch_manager_mods/queue_hud_src/*']
```
