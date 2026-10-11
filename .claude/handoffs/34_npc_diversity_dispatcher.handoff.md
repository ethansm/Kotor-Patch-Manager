# Session Handoff & Plan State: patch_manager_mods/34_npc_diversity_dispatcher.md

## 0. Next-Phase Direction
Wave 6 remaining: Gate 1 play log + PTR dumps (Task 6 tag route), Task 8 policy 0.1.4-probe (ShouldVary 35%, family allowlists from DiversitySlots.h, keep-default via DiversityRoles.h), Task 9 hot-swap, Gate 2, Task 10 check, Task 11 cleanup; alien-commoner and alt-body-outfit follow-ons ready

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/34_npc_diversity_dispatcher.md`
- **Branch:** `KOTOR-II` (c9b8064 feat(npc-diversity): slot and role header generators, user-adjusted texture labels)
- **Generated:** 2026-10-11 19:49:42 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
c9b8064 feat(npc-diversity): slot and role header generators, user-adjusted texture labels
 .../npc_diversity_src/DiversityRoles.h             | 294 +++++++++++++++++++++
 .../npc_diversity_src/DiversitySlots.h             |  26 ++
 .../npc_diversity_src/texture_labels.json          | 155 ++++++++++-
 scripts/generate_diversity_roles.py                | 138 ++++++++++
 scripts/generate_diversity_slots.py                |  83 ++++++
 scripts/npc_div_merge_labels.py                    |  15 +-
 scripts/test_generate_diversity_roles.py           |  74 ++++++
 scripts/test_generate_diversity_slots.py           |  67 +++++
 scripts/test_npc_div_merge_labels.py               |  14 +-
 9 files changed, 854 insertions(+), 12 deletions(-)
```
- **Dirty Files (138):**
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/34_npc_diversity_dispatcher.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/activation_fix_src/ActivationFix.cpp`
  - ` M patch_manager_mods/equip_icon_fix_src/EquipIconFix.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_activationfix.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_equipiconfix.cpp`
  - ... +128 more (run `git status --short`)

## 2. Immediate Objective (**Phase 2**)
**Detour Hook Prototyping**

Status: REOPENED (10-11 REVIEW)
Task: Detour Hook Prototyping

## 2b. Upcoming Batches in Pipeline
- ****Phase 3**** : Fat Body & Alien Model Distribution
- ****Phase 4**** : Exclusion List & Savegame Compatibility
- ****Phase 5**** : Production Packaging

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/34_npc_diversity_dispatcher.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **Phase 2**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
### Wave 6 state (2026-10-11, session end)
LIVE: `0.1.3-probe` (repo `1c4c7a5`, KPM `177933f`, md5 `8c01ab2c`; rollback `python3 scripts/kpm_hotswap.py --rollback npc-diversity_swap_20261011_184033`). Gate 1 (user play of Telos, PTR dumps) NOT yet reported.
Landed commits: `1c4c7a5` probe, `8e392ab` labelling pipeline + registry + roles, `cc81e78` alien recon, `c9b8064` generators + DiversitySlots.h/DiversityRoles.h.
Generated, byte-stable (`--check`): `DiversitySlots.h` (M 1-16, F 1-17, FatM 34, FatF 35), `DiversityRoles.h` (112 keep tags, 158 keep positions, 1.1% ambiguity). Sources: `texture_labels.json`, `.claude/npc_div/registry.jsonl`, `.claude/npc_div/roles/out_*.json`.

### Dead ends / findings
- Pool slots 17-33 were HEAD textures, 28-30 robes, 31-33 armor, F33/F34/M35 other-mesh (Onderon/Sith) clothing: they belong to other body meshes (UV mismatch), NOT plain N_CommM/F. Never put them in the plain pool. User liked them: use via alt-body ROW remap (Onderon rows 472-478/502-506, Sith 291-302/42/43) in a later tier (`texture_labels.json` field `alt_body`).
- Haiku (86% vanilla recall) and Sonnet (76%) vision on flat atlases cannot separate clothing from robe/uniform; the UV-layout signature (vanilla min 0.28, decoy max 0.09, AUC 1.0) + one Sonnet vote (69/70 agree) is the decider. Vote B was skipped as redundant.
- F17 (uv 0.20) and M16 (uv 0.27) are USER-APPROVED TRIAL slots (plain FD/MD mesh): check them at Gate 2; if any look scrambled, drop via `ADJUDICATED` in `scripts/npc_div_merge_labels.py`.
- Conversation-set is NOT a named signal: generic commoners share bark conversations (`200comm`, `201travm`); registry rule uses conv_n>=4 as generic.
- Engine tag not readable (+0x1C/+0x20 are pointers); position fallback is the keep-default route unless Gate 1 PTR dumps show an ASCII tag.
- Alien-commoner recon (`.claude/npc_div/aliens/00_overview.md`) is a follow-on, not Wave 6.

### Verification commands
`python3 scripts/run_tests.py diversity generate_diversity_slots generate_diversity_roles npc_div_merge_labels npc_div_registry npc_div_texture_sheets npc_div_log_check`; `python3 scripts/generate_diversity_slots.py --check`; `python3 scripts/generate_diversity_roles.py --check --report`; `python3 scripts/build_check.py --mod npc_diversity_src`; `python3 scripts/check_hook_bytes.py --mod npc-diversity`; `python3 scripts/check_hook_spans.py`; `python3 scripts/kpm_status.py --only npc-diversity`.
