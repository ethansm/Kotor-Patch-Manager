# KOTOR II Workspace Master Key & Planning Legend

Quick reference guide decoding cryptic letter/number combinations, doc IDs, and phase prefixes across our active workspaces.

---

## 1. Phase Identifier Prefixes (The Letters)

When looking at batches in master plans (e.g. `T5d`, `S1a`, `C1`, `R10`), the leading letter identifies the work category:

| Prefix | Category | What it means | Example |
|---|---|---|---|
| **T** | **Testing & Contracts** | Colocated unit tests, SafeRead contract tests, or `--wine` smokes | `T5d`: contract test level_up_override |
| **S** | **Sync / Mirroring** | Syncing shared headers (`_shared_src/`) to KPM `ethansm/local-mods` | `S1a`: mirror sync buff + shield + portrait |
| **L** | **Library / Packaging** | KPM library DLL manifests, launcher integration, split DLL installs | `L2`: KPM manifest builder |
| **C** | **Common Extraction** | Extracting shared logic into `Patches/Common` (`CodePatch.h`, etc.) | `C1`: CodePatch adoption across fixes |
| **M** | **Meta / Setup** | Administrative master plan creation, status updates, scaffolding | `M0`: Create doc 24 master plan |
| **B** | **Database Batches** | Steam address DB reverse-engineering batches in `kpm-pr-steam-db` | `B12b`: Acc/Steam ABI normalization |
| **R** | **Research & Probes** | Live-game diagnostic probes, Ghidra decompilation, atlas analysis | `R10`: PC level-up filter research |
| **P** | **Parity Verification** | Byte-exact or behavioral parity comparison against vanilla exe | `P0`: Companion level-up baseline |
| **U** | **Unit / Compatibility** | Dependency graphs, contract testing, and tool units | `U1`: Graph builder `compat_graph.py` |

---

## 2. Document Number Directory (The Docs)

Our numbered markdown documents in `patch_manager_mods/` map directly to specific mods or subsystems:

| Doc # | Subsystem / Mod | Plain-English Focus |
|---|---|---|
| **01** | `inventory_3d_viewport` | 3D character preview model inside inventory menus |
| **02** | `buff_duration_hud` | Status effect and force power buff timers HUD |
| **05** | `build_plan` | Compilation matrix and binary build rules |
| **07** | `action_hotbar` | Expanded quick-cast Action Hotbar (formerly Force Power Hotbar) |
| **08** | `level_up_override` / `shield_bar` | Custom level-up logic and companion shield bar HUD |
| **09** | `load_hang_investigation` | 0-hang activation fix and area transition investigation |
| **10** | `engine_optimization_research` | Perf-lab telemetry, texture memory (TexMem), and frame pacing |
| **11** | `inventory_sort_filter` | Inventory categories, sort filters, and weapon grouping |
| **13** | `ranged_ai_spacing` | Companion combat AI and ranged attack spacing fixes |
| **14** | `character_active_effects_panel` | Detailed active effects and spell duration inspection panel |
| **15** | `engine_bug_candidates` | Engine crash triage, memory leaks, and vanilla bug candidate register |
| **17** | `dialogue_letterbox_and_wrap` | Ultrawide dialogue letterbox removal and word-wrap fixes |
| **18** | `mouse_camera_investigation` | High-DPI mouse camera jitter and XWayland dinput investigation |
| **19** | `shared_logging_design` | Universal logging library (`KpLog.h`) adoption |
| **20** | `TESTING_PLAN` | Testing harness architecture, `check.h`, and `run_tests.py` |
| **21** | `portrait_slots_and_split` | Splitting portrait-slots, buff-hud, and shield-bar into DLLs |
| **22** | `compat_check_plan` / `hotbar_shared_services` | Update compatibility checker tooling (`compat_check.py`) & shared services |
| **23** | `menu_3d_shared_migration` | Shared 3D rendering pipeline for inventory and equip screens |
| **24** | `master_plan_status` | **Active Master Plan**: Patches/Common integration and test coverage |
| **25** | `pc_levelup_filter` | **Active Mod Plan**: PC & companion level-up filter and stat allocation |
| **26** | `FORCED_STATE_TESTING` | Forced state regression suite and headless mock testing |
| **30** | `queue_hud` | Out-of-combat green queue HUD & companion strips |
| **31** | `combat_action_hotbar` | Dedicated 1-click combat action hotbar |
| **32** | `common_migration_round2` | Common migration round 2 |
| **33** | `ui_queue_hotbar_fixes` | **Active Master Plan**: UI, queue, hotbar polish & movement fixes |


---

## 3. Master Plan Table Specification

Every active master plan (e.g. `24_master_plan_status.md`, `31_combat_action_hotbar.md`) uses a standardized 7-column markdown schema:

| Column | Description | Format / Values |
|---|---|---|
| **Id** | Unique phase or batch identifier | Leading prefix letter + number (e.g. `T5d`, `CS2`, `M0`) |
| **Batch** | Scoped deliverable or component | Concise summary of code, test, or config changes |
| **Deps** | Upstream prerequisites | Comma-separated phase Ids, or `-` if none |
| **Owner gate** | Concurrency or user approval lock | `-`, session name (e.g. `split`, `hotbar`), or `user GO` |
| **Status** | Current phase lifecycle state | `TODO`, `PARTIAL`, `BUILT`, `STAGED`, `DONE`, `DEFERRED` |
| **Commit** | Short SHA of git commit | 7-char hash (e.g. `6fcb8ec`), or empty if uncommitted |
| **Next** | Immediate verification or next action | Verification metrics, test counts, or next batch pointer |

Progress is updated deterministically via CLI:
```bash
python3 scripts/plan_manager.py mark-done <Id> [--state <STATE>] [--commit <sha>] [--notes <txt>]
```

---

## 4. The 5-Stage Mod Milestone Lifecycle

Master plans track features through five standard lifecycle stages. Logging and diagnostics are strictly phased:

1. **Stage 1: Research & Probes (`R`, `P`)**
   - *Goal*: Reverse-engineer memory layouts, offsets, and engine calling conventions.
   - *Logging*: Instrument **rich diagnostic logging** (dedicated KpLog channels, raw struct dumps, first-draw coordinates). Never guess blind in live play.
2. **Stage 2: Core Logic & Contract Tests (`C`, `T`)**
   - *Goal*: Extract pure algorithms, implement SafeRead boundaries, and write host tests.
   - *Gates*: Colocated unit tests (`run_tests.py`), mutation testing, and `--wine` smoke tests.
3. **Stage 3: Live Probing & Calibration (`CS3`, `HL2`)**
   - *Goal*: Live in-game validation with user consent.
   - *Rule*: Gated behind explicit `user GO`; backup created before staging (`stage_and_verify_install.py`).
4. **Stage 4: Hardening & Log Tightening (`HP`, `L`)**
   - *Goal*: Clean up development telemetry before final integration or PR.
   - *Logging*: Gate high-volume and per-frame logs behind `Verbose=0` in INI; enforce state-change deduplication (log state changes, never per-frame redraws); drop temporary probe detours; verify clean logs (<50 lines on startup) via `python3 scripts/kotor2_logs.py check <log>`.
5. **Stage 5: Packaging, Sync & Release (`S`, `L`)**
   - *Goal*: Production deployment and upstream submission.
   - *Rule*: Header sync to `Patches/Common`, STAGE_INFO tracking, KPM install verification, PR branch cut in separate worktree.

---

## 5. Batch Protocol & Concurrency Standards

1. **One Batch = One Execution Turn**: Orchestrators focus strictly on the active batch rather than conflating multi-batch tasks.
2. **Pre-Flight Concurrency Check**: Run `git status --short` on target paths. If another session owns or is modifying files, coordinate via `SendMessage` or stop.
3. **Explicit Commit Scoping**: NEVER run broad `git commit -a`. Always commit explicit target paths (`git commit -- path/to/file`).
4. **Zero-Read Plans**: Plan steps inline exact struct layouts or 5-10 line snippets so executors do not scrape past context.
5. **Automated Status Sync**: Mark completed batches immediately using `scripts/plan_manager.py mark-done`.

---

## 6. Fast CLI Reference

```bash
# View the master key, prefixes, and active docs:
python3 scripts/plan_manager.py legend

# Check current active plan objective with category badge:
python3 scripts/plan_manager.py resume

# Resume a specific doc shorthand:
python3 scripts/plan_manager.py resume 24    # Common master plan
python3 scripts/plan_manager.py resume 25    # Level-up filter plan
python3 scripts/plan_manager.py resume 31    # Action hotbar plan

# Advance a master plan batch state:
python3 scripts/plan_manager.py mark-done T5e --state DONE --commit abc1234 --notes "32/32 tests PASS"
```

---

## 7. Turnkey Mod Scaffolding & Compliance Standards

To ensure every new mod or patch automatically complies with our scoping, testing, and subagent architecture:

```bash
# Automatically scaffold a compliant mod directory + master plan:
python3 scripts/scaffold_mod.py <name> [--doc-id <XX>] [--title "<Title>"]
```

Every new mod generated via `scaffold_mod.py` automatically configures:
1. **Standard Include Style**: `#include "SafeRead.h"` with `-I Common` (no relative `../_shared/` paths).
2. **Common Contract Test**: `<mod>_src/common_contract_test.cpp` and `test.toml` pre-configured to run via `scripts/run_tests.py`.
3. **Strict Mod Scope Boundary**: Master plan header declares `Owning Mod Scope`, automatically extracted by `scripts/subagent_prompt.py` to confine subagent write scopes.
4. **Role-Based Subagents**: Master plan template links to [SUBAGENT_ROLES.md](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/SUBAGENT_ROLES.md) and pre-populates `oc-subagent <scout|ghidra|code|test|verify|gate|doc>` commands across all 5 standard lifecycle phases.
5. **KPM Packaging**: `exports.def`, `kotor2-steam-aspyr.hooks.toml`, and `manifest.toml` ready for `scripts/build_check.py`.

See **[SUBAGENT_ROLES.md](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/SUBAGENT_ROLES.md)** for the complete catalog of subagent roles, backend routes, output contracts, and autonomous delegation rules.

