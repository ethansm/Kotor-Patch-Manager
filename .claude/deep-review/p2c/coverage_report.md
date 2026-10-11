# Coverage report

Result: **PASS**  (units: 446, gaps: 0, errors: 0)

| verdict | units |
|---|---|
| reviewed | 370 |
| mechanical | 18 |
| skipped | 58 |
| excluded | 0 |
| gap | 0 |

## Waived (truncated accepted by profile)

| packet | reason |
|---|---|
| other-024 | truncated=true accepted by profile [coverage] accept_truncated |
| other-035 | truncated=true accepted by profile [coverage] accept_truncated |

## Warnings: worker files_examined cross-check (not a failure)

| packet | problem | missing files |
|---|---|---|
| code-003 | files_examined misses packet files | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp |
| code-007 | files_examined misses packet files | patch_manager_mods/level_up_override_src/LevelUpOverride.cpp, patch_manager_mods/level_up_override_src/common_contract_test.cpp |
| other-011 | files_examined misses packet files | research/companion_levelup/tools/parity/unit_gap_test.cpp |
| other-020 | files_examined misses packet files | scripts/compat_graph.py, scripts/crash_triage.py, scripts/dedup_tga_tpc.py |
| other-023 | files_examined misses packet files | scripts/generate_ultrawide_gui_overrides.py |
| other-024 | files_examined misses packet files | scripts/generate_ultrawide_textures.py |
| other-027 | files_examined misses packet files | scripts/install_echoes_compatch.py, scripts/install_echoes_compatch.sh, scripts/kotor2_logs.py |
| other-028 | files_examined misses packet files | scripts/kotor2_toolset.py, scripts/kpm_hotswap.py, scripts/master_ledger.py |
| other-029 | files_examined misses packet files | scripts/plan_manager.py |
| other-031 | files_examined misses packet files | scripts/router_eval.py, scripts/run_tests.py |
| other-032 | files_examined misses packet files | scripts/run_tests.py, scripts/scaffold_mod.py, scripts/selfheal_log_check.py |
| other-034 | files_examined misses packet files | scripts/subagent_prompt.py, scripts/swarm_argv_probe.py, scripts/symlink_pass.py |

## Other warnings

- coverage_plan.json snapshot differs from sha256(snapshot.json)

## Per-file roll-up

| file | reviewed | mechanical | skipped | excluded | gap |
|---|---|---|---|---|---|
| .agents/skills/hot-swap/SKILL.md | 0 | 0 | 1 | 0 | 0 |
| .agents/skills/resume/SKILL.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/10_engine_optimization_research.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/18_mouse_camera_investigation.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/22_compat_check_plan.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/31_combat_action_hotbar.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/35_clef_systemone_matcher_and_ledger.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/handoffs/swarm_m2_safety.handoff.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_library_architecture.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/claims.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/corrections.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/design_draft.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/K1.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/K2.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/K3.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/K4.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/K5.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/K6.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/V1.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/V2.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/V3.md | 0 | 0 | 1 | 0 | 0 |
| .claude/plans/swarm_review_2026-10-10/out/V4.md | 0 | 0 | 1 | 0 | 0 |
| AGENTS.md | 0 | 0 | 1 | 0 | 0 |
| CLAUDE.md | 0 | 0 | 1 | 0 | 0 |
| COMMUNITY_COMPATCH_GUIDE_ECHOES_OF_THE_REPUBLIC.md | 0 | 0 | 1 | 0 | 0 |
| COMPATCH_PLAN_ECHOES_OF_THE_REPUBLIC.md | 0 | 0 | 1 | 0 | 0 |
| LEGACY_MOVIES_UPSCALING_PIPELINE.md | 0 | 0 | 1 | 0 | 0 |
| kotor2_ghidra_knowledge.db | 0 | 0 | 1 | 0 | 0 |
| neocities_mods.json | 1 | 0 | 0 | 0 | 0 |
| neocities_mods_detailed.json | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/00_code_review_feedback_2026-09-29.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/00_code_review_feedback_2026-09-30.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/00_ui_queue_hotbar_feedback_2026-10-06.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/01_inventory_3d_viewport_feedback.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/10_engine_optimization_research.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/10_equipment_3d_character_feedback.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/11_inventory_sort_filter_feedback.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/13_ranged_ai_spacing.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/13_ranged_ai_spacing_feedback.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/18_mouse_camera_investigation.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/19b_cross_patch_sharing_survey.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/27_companion_selfheal.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/30_queue_hud.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/33_ui_queue_hotbar_fixes.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/35_clef_systemone_matcher_and_ledger.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/_shared_src/GameAddr.h | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/_shared_src/GuiInput.h | 0 | 1 | 0 | 0 | 0 |
| patch_manager_mods/_shared_src/SelfHealUi.h | 2 | 0 | 0 | 0 | 0 |
| patch_manager_mods/action_hotbar_src/ActionHotbar.cpp | 9 | 2 | 0 | 0 | 0 |
| patch_manager_mods/action_hotbar_src/HotbarLogic.h | 1 | 1 | 0 | 0 | 0 |
| patch_manager_mods/action_hotbar_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/action_hotbar_src/manifest.toml | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/activation_fix_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/active_effects_panel_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/buff_duration_hud_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/companion_ai_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp | 39 | 0 | 0 | 0 | 0 |
| patch_manager_mods/equip_icon_fix_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/gui_input_src/GuiInputProvider.cpp | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/gui_input_src/gi_registry_test.cpp | 4 | 0 | 0 | 0 | 0 |
| patch_manager_mods/gui_input_src/kotor2-gog-aspyr.hooks.toml | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp | 7 | 0 | 0 | 0 | 0 |
| patch_manager_mods/inventory_3d_viewport_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/inventory_sort_filter_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/level_up_override_src/LevelUpOverride.cpp | 63 | 2 | 0 | 0 | 0 |
| patch_manager_mods/level_up_override_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/level_up_override_src/common_contract_test.cpp | 1 | 1 | 0 | 0 | 0 |
| patch_manager_mods/level_up_override_src/exports.def | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/level_up_override_src/manifest.toml | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/level_up_override_src/test.toml | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/mouse_probe_src/MouseFixLogic.h | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/mouse_probe_src/MouseProbe.cpp | 1 | 1 | 0 | 0 | 0 |
| patch_manager_mods/mouse_probe_src/common_contract_test.cpp | 0 | 1 | 0 | 0 | 0 |
| patch_manager_mods/queue_hud_src/QueueHud.cpp | 24 | 0 | 0 | 0 | 0 |
| patch_manager_mods/queue_hud_src/QueueHudLogic.h | 15 | 2 | 0 | 0 | 0 |
| patch_manager_mods/queue_hud_src/exports.def | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml | 1 | 0 | 0 | 0 | 0 |
| patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp | 2 | 2 | 0 | 0 | 0 |
| patch_manager_mods/ranged_ai_src/PULL_REQUEST.md | 0 | 0 | 1 | 0 | 0 |
| patch_manager_mods/shield_bar_src/ShieldBar.cpp | 0 | 1 | 0 | 0 | 0 |
| research/companion_levelup/logs/live/levelup_override_log_run3_20261002.txt | 0 | 0 | 1 | 0 | 0 |
| research/companion_levelup/phase4_evidence/M1_tags.txt | 0 | 0 | 1 | 0 | 0 |
| research/companion_levelup/tools/parity/apply_test.cpp | 1 | 0 | 0 | 0 | 0 |
| research/companion_levelup/tools/parity/gate_test.cpp | 4 | 0 | 0 | 0 | 0 |
| research/companion_levelup/tools/parity/parity.py | 1 | 0 | 0 | 0 | 0 |
| research/companion_levelup/tools/parity/scenario_test.cpp | 6 | 1 | 0 | 0 | 0 |
| research/companion_levelup/tools/parity/scenarios.py | 1 | 0 | 0 | 0 | 0 |
| research/companion_levelup/tools/parity/stub/LuoLogHost.h | 1 | 0 | 0 | 0 | 0 |
| research/companion_levelup/tools/parity/ui_test.cpp | 3 | 0 | 0 | 0 | 0 |
| research/companion_levelup/tools/parity/unit_gap_test.cpp | 11 | 0 | 0 | 0 | 0 |
| scripts/01_sanitize_textures.py | 1 | 0 | 0 | 0 | 0 |
| scripts/02_merge_2da_and_tlk.py | 1 | 0 | 0 | 0 | 0 |
| scripts/03_compile_galaxy_map.py | 1 | 0 | 0 | 0 | 0 |
| scripts/04_merge_modules.py | 1 | 0 | 0 | 0 | 0 |
| scripts/05_merge_dlgs_and_jrl.py | 1 | 0 | 0 | 0 | 0 |
| scripts/06_deploy_assets.py | 1 | 0 | 0 | 0 | 0 |
| scripts/_common.py | 1 | 0 | 0 | 0 | 0 |
| scripts/abi_check.py | 5 | 0 | 0 | 0 | 0 |
| scripts/agent_contract.py | 2 | 0 | 0 | 0 | 0 |
| scripts/agent_diff_check.py | 1 | 0 | 0 | 0 | 0 |
| scripts/agent_usage.py | 1 | 0 | 0 | 0 | 0 |
| scripts/agent_usage_rates.json | 1 | 0 | 0 | 0 | 0 |
| scripts/case_dedup.py | 1 | 0 | 0 | 0 | 0 |
| scripts/check_ncs_arg_order.py | 1 | 0 | 0 | 0 | 0 |
| scripts/check_txi_cycles.py | 1 | 0 | 0 | 0 | 0 |
| scripts/clef_dataset.py | 1 | 0 | 0 | 0 | 0 |
| scripts/clef_matcher.py | 1 | 0 | 0 | 0 | 0 |
| scripts/compat_check.py | 6 | 0 | 0 | 0 | 0 |
| scripts/compat_graph.py | 5 | 0 | 0 | 0 | 0 |
| scripts/crash_triage.py | 1 | 0 | 0 | 0 | 0 |
| scripts/dedup_tga_tpc.py | 1 | 0 | 0 | 0 | 0 |
| scripts/generate_diversity_2da.py | 4 | 0 | 0 | 0 | 0 |
| scripts/generate_ultrawide_gui_overrides.py | 1 | 0 | 0 | 0 | 0 |
| scripts/generate_ultrawide_textures.py | 1 | 0 | 0 | 0 | 0 |
| scripts/gpu_mode.py | 1 | 0 | 0 | 0 | 0 |
| scripts/install_echoes_compatch.py | 1 | 0 | 0 | 0 | 0 |
| scripts/install_echoes_compatch.sh | 1 | 0 | 0 | 0 | 0 |
| scripts/kotor2_ghidra_knowledge.db | 0 | 0 | 1 | 0 | 0 |
| scripts/kotor2_logs.py | 6 | 1 | 0 | 0 | 0 |
| scripts/kotor2_toolset.py | 1 | 0 | 0 | 0 | 0 |
| scripts/kpm_hotswap.py | 3 | 0 | 0 | 0 | 0 |
| scripts/master_ledger.py | 1 | 0 | 0 | 0 | 0 |
| scripts/normalize_diversity_textures.py | 3 | 0 | 0 | 0 | 0 |
| scripts/plan_manager.py | 8 | 0 | 0 | 0 | 0 |
| scripts/pre_install_gate.py | 3 | 0 | 0 | 0 | 0 |
| scripts/query_ghidra.py | 3 | 0 | 0 | 0 | 0 |
| scripts/router_eval.py | 1 | 0 | 0 | 0 | 0 |
| scripts/run_tests.py | 12 | 0 | 0 | 0 | 0 |
| scripts/scaffold_mod.py | 8 | 0 | 0 | 0 | 0 |
| scripts/selfheal_log_check.py | 5 | 0 | 0 | 0 | 0 |
| scripts/shadow_replay.py | 1 | 0 | 0 | 0 | 0 |
| scripts/snapshot_live.py | 1 | 0 | 0 | 0 | 0 |
| scripts/subagent_prompt.py | 29 | 2 | 0 | 0 | 0 |
| scripts/swarm_argv_probe.py | 1 | 0 | 0 | 0 | 0 |
| scripts/symlink_pass.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_abi_check.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_agent_usage.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_clef_matcher.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_compat_check.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_compat_check_rc.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_compat_graph.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_crash_triage.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_kotor2_logs.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_master_ledger.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_plan_manager.py | 8 | 0 | 0 | 0 | 0 |
| scripts/test_router_eval.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_run_tests_select.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_selfheal_log_check.py | 1 | 0 | 0 | 0 | 0 |
| scripts/test_subagent_tools.py | 1 | 0 | 0 | 0 | 0 |
| scripts/upscale_legacy_movies.py | 1 | 0 | 0 | 0 | 0 |
| scripts/verify_2da.py | 1 | 0 | 0 | 0 | 0 |
| scripts/verify_all_compatch.py | 1 | 0 | 0 | 0 | 0 |
| swkotor2.ini | 1 | 0 | 0 | 0 | 0 |
| tests/benchmarks/transcript_prompt_eval.json | 1 | 0 | 0 | 0 | 0 |
| tests/static/subclassers.toml | 3 | 0 | 0 | 0 | 0 |
| tests/suites.toml | 1 | 0 | 0 | 0 | 0 |
