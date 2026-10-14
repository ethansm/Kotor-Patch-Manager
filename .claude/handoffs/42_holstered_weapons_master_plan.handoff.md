# Session Handoff & Plan State: patch_manager_mods/42_holstered_weapons_master_plan.md

## 0. Next-Phase Direction
P0 bone_test committed 4e85582; next: HW3 offline weapon-on-bone preview (weapons command in k1_port_tex_preview.py; find valid K2 body names), then HW4 install on user GO

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/42_holstered_weapons_master_plan.md`
- **Waves:** .claude/waves/WI-42-a1/
- **Waves:** .claude/waves/WI-42-a2/
- **Waves:** .claude/waves/WI-42-b2/
- **Branch:** `KOTOR-II` (4e85582 feat(holster): flag-file bone_test (rhand/lhand attach node literal repoint), code_patches sidecar, contract test, play-test checklist)
- **Generated:** 2026-10-13 22:09:43 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
4e85582 feat(holster): flag-file bone_test (rhand/lhand attach node literal repoint), code_patches sidecar, contract test, play-test checklist
 patch_manager_mods/42_holstered_weapons.md         | 10 +++++-
 patch_manager_mods/holster_src/Holster.cpp         | 36 ++++++++++++++++++++--
 patch_manager_mods/holster_src/HolsterBonePatch.h  | 24 +++++++++++++++
 patch_manager_mods/holster_src/code_patches.toml   | 22 +++++++++++++
 .../holster_src/common_contract_test.cpp           | 17 ++++++++++
 patch_manager_mods/holster_src/holster.ini         |  6 ++++
 patch_manager_mods/holster_src/manifest.toml       |  2 +-
 7 files changed, 113 insertions(+), 4 deletions(-)
```
- **Dirty Files (160):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/holstered_weapons.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ... +150 more (run `git status --short`)

## 2. Immediate Objective (HW3)
**Offline weapon-on-bone preview: `weapons` command in `k1_port_tex_preview.py` (sub-plan A)**

Status: PARTIAL
Task: Offline weapon-on-bone preview: `weapons` command in `k1_port_tex_preview.py` (sub-plan A)

## 2b. Upcoming Batches in Pipeline
- **HW4** : P0 install: Common sync into the real KPM tree, `pre_install_gate.sh --mod holster_src` GATE PASS, `kpm_hotswap` dry run + `--prepare`, REINSTALL (sub-plan B)
- **HW5** : P0 play runs 1 (probe) and 2 (bone test) by the user; read logs
- **HW6** : Answer gates G2-G7 in doc 42 from logs + preview; choose placement offsets and bone per category

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/42_holstered_weapons_master_plan.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate HW3` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- Dead end: `k1_port_tex_preview.py mesh --models P_BastilaBA ...` -> "MDL/MDX not found" (model name not in the K2 install; note head_wearability.tsv also spells it `P_BastillaBB`/`P_BastilaBA` inconsistently). The batch aborted on the first model, so NO weapon/body JSON exists yet; check each name alone.
- Dead end: check_hook_bytes rejects a 4-byte immediate inside .text (kind "data" needs .data/.rdata); patch the whole 5-byte `push` with kind "code" (done in 4e85582).
- Preview commands: mesh dump `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_tex_preview.py mesh --textures --models <names> --out <scratchpad>/hm`; render with `.venv/bin/python -I ...`. Weapon model names to confirm from `.claude/research/k1_items/items_k2.tsv` column model_resref.
- Verification: `python3 scripts/run_tests.py holster k2addr`; `python3 scripts/check_hook_bytes.py --mod holster`; scratch-tree `build_check --kpm-dir <scratch> --mod holster_src`.
