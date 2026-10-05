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
| **23** | `menu_3d_shared_migration` | Shared 3D rendering pipeline for inventory and equip screens |
| **24** | `master_plan_status` | **Active Master Plan**: Patches/Common integration and test coverage |
| **25** | `pc_levelup_filter` | **Active Mod Plan**: PC & companion level-up filter and stat allocation |
| **26** | `FORCED_STATE_TESTING` | Forced state regression suite and headless mock testing |

---

## 3. Fast CLI Reference

```bash
# View the master key and legend anytime:
python3 scripts/plan_manager.py legend

# Check current objective with plain-English category badge:
python3 scripts/plan_manager.py resume

# Resume a specific doc with shorthand:
python3 scripts/plan_manager.py resume 24    # Common master plan
python3 scripts/plan_manager.py resume 25    # Level-up filter plan
```
