# Session Handoff & Plan State: patch_manager_mods/42_holstered_weapons_master_plan.md

## 0. Next-Phase Direction
42:HW5 direction: deep research on holster weapon pose. Investigate the ready/idle animation pose (supermodels S_Female02/S_Female01 g3r1/g4r1 sets, rhand rotation in arms-down pose), other options (per-frame bone transform in the 0x5047B0 detour with rotation+offset, using hand-held rotation, animation-driven attach, blender posed render), and how to see it on an animated character in game. Also decide: should we play test soon (run-1 ini + bone_test) after fixing what the rendered previews allow? Holster 0.1.0-dev is INSTALLED (HW4, 43 patches); preview tool has --rot/--offset/--hand-rot (d31771a); Haiku sheet check done.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/42_holstered_weapons_master_plan.md`
- **Waves:** .claude/waves/WI-42-a1/
- **Waves:** .claude/waves/WI-42-a2/
- **Waves:** .claude/waves/WI-42-b2/
- **Branch:** `KOTOR-II` (5673d94 feat(kpm_reinstall): snapshot, flat-stage KPatchLauncher run, peer DLL restore, rollback (doc 24 H10))
- **Generated:** 2026-10-14 00:04:25 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
5673d94 feat(kpm_reinstall): snapshot, flat-stage KPatchLauncher run, peer DLL restore, rollback (doc 24 H10)
 .claude/skills/kotor2-kpm-install/SKILL.md |   2 +-
 patch_manager_mods/BUILD_AND_INSTALL.md    |   2 +-
 scripts/kpm_reinstall.py                   | 285 +++++++++++++++++++++++++++++
 scripts/test_kpm_reinstall.py              | 271 +++++++++++++++++++++++++++
 4 files changed, 558 insertions(+), 2 deletions(-)
```
- **Dirty Files (149):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ... +139 more (run `git status --short`)

## 2. Immediate Objective (HW5)
**P0 play runs 1 (probe) and 2 (bone test) by the user; read logs**

Status: TODO
Task: P0 play runs 1 (probe) and 2 (bone test) by the user; read logs

## 2b. Upcoming Batches in Pipeline
- **HW6** : Answer gates G2-G7 in doc 42 from logs + preview; choose placement offsets and bone per category
- **HW7** : P1 MVP: instant hip<->hand swap, PC then party, all weapons on hip (sub-plan C)
- **HW8** : P2 placement: per-category mounts, hilt-only bounds, alt-set mounts

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/42_holstered_weapons_master_plan.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate HW5` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- Dead end: `k1_port_tex_preview.py mesh --models P_BastilaBA ...` -> "MDL/MDX not found" (model name not in the K2 install; note head_wearability.tsv also spells it `P_BastillaBB`/`P_BastilaBA` inconsistently). The batch aborted on the first model, so NO weapon/body JSON exists yet; check each name alone.
- Dead end: check_hook_bytes rejects a 4-byte immediate inside .text (kind "data" needs .data/.rdata); patch the whole 5-byte `push` with kind "code" (done in 4e85582).
- Preview commands: mesh dump `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_tex_preview.py mesh --textures --models <names> --out <scratchpad>/hm`; render with `.venv/bin/python -I ...`. Weapon model names to confirm from `.claude/research/k1_items/items_k2.tsv` column model_resref.
- Verification: `python3 scripts/run_tests.py holster k2addr`; `python3 scripts/check_hook_bytes.py --mod holster`; scratch-tree `build_check --kpm-dir <scratch> --mod holster_src`.
- RESOLVED (HW3, 8abfc22): the `P_BastilaBA` dead end above. `mesh` now prints NOT FOUND and continues. Working bodies are `n_commm`, `n_commf`; P_*BA, PMBA, PFBA are not in the K2 install. Pass LOWERCASE model names (HelmetKit/WeaponKit look up `<name>.json` in lowercase).
- Dead end: identity-rotation weapon placement (the first HW3 sheets) says nothing about how a holstered weapon sits: the bone node supplies no weapon rotation, and skin bones `pelvis_g/torso_g/torsoUpr_g` have identity rotation, thighs about 6 deg. A weapon keeps its model axes (saber hilt +z emitter up, blaster barrel +y forward, grip at origin; double saber origin at the LOWER end). `--hand-rot rhand` is also wrong: the rest pose is a T-pose, so the held rotation points forward/outward.
- Pose presets that look right (N_CommM rest pose, `weapons --rot/--offset`): blaster barrel-down on thigh `--rot -90 0 0 --offset 0.12 0 -0.03` (bones rthigh_g/lthigh_g; mirror x for left); saber emitter-down on hip `--rot 180 0 0 --offset 0.2 0 0` (pelvis_g, double saber reaches the knee). x = character's right, y = forward, z = up, applied in the bone frame.
- P1 consequence: the detour of 0x5047B0 (UpdateDependentPosition) must apply ROTATION + offset to the substituted-bone transform, not only an offset (sub-plan C wording is stale).
- Animation data is reachable offline: `n_commm` has 0 anims, supermodel S_Female02 (232 anims in s_male02, 88 in s_female02: names like `g3r1`, `g4r1`, `pause1`), chain continues to S_Female01. Probe script pattern: `venvs/k1port/bin/python -I <script> <game> <model...>` with `k1_port_uvmask.install_get` + `pk.read_mdl`, then `mdl.anims[i].name`. Not yet used to pose a body.
- Install facts (HW4): holster is a NEW id, so `kpm_hotswap.py` refuses it and `scripts/kpm_reinstall.py` does NOT exist. Install used `stage_and_verify_install.py stage <dir> <Holster.kpatch> --allow-dirty` + `KPATCH_NO_LAUNCH=1 KPatchLauncher ... --patches <dir> <ids>` (backup `$G/backups/holster_full_20261013_225517`). `Holster.kpatch` is gitignored; build it with `kpm_hotswap.mirror` + `run_create_patch` (python import). Default-tier gate fails 3 suites in peers (npc_world/voice_probe raw-addr, area-hook-probe vs k1-spawn-probe spans, kvoice lip): not holster; holster-scoped gate used `pre_install_gate.py --mod holster_src --skip-default`.
- State now: holster 0.1.0-dev LIVE but INERT (no `holster.ini` in the game dir, probe defaults 0, nothing logged). No play test has run. Haiku sheet check (a.md, b.md in `.claude/research/holstered_weapons/haiku_check/`) found nothing refuted; N_CommF double saber also shows above the neck at torso_g.
- Contract caveat: `agent_contract.py check` needs the reply as a positional argument (`check --scope <globs> -` with stdin); I ran it without `-` and it errored, so the two Haiku replies were NOT contract-checked.
