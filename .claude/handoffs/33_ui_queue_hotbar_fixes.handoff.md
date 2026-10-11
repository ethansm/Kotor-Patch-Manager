# Session Handoff & Plan State: patch_manager_mods/33_ui_queue_hotbar_fixes.md

## 0. Next-Phase Direction
Execute QH4: [Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution)

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/33_ui_queue_hotbar_fixes.md`
- **Branch:** `KOTOR-II` (22b56b4 fix(equip3d): destroy creature on character change, add anim fallback, sync screen)
- **Last Landed Commit:**
```
22b56b4 fix(equip3d): destroy creature on character change, add anim fallback, sync screen
 .../EquipCharPreview.cpp                           | 16 ++++++---
 .../equipment_3d_character_src/PULL_REQUEST.md     | 42 ++++++++++++++++++++++
 2 files changed, 54 insertions(+), 4 deletions(-)
```
- **Dirty Files (190):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/10_engine_optimization_research.md`
  - ` M patch_manager_mods/13_ranged_ai_spacing.md`
  - ` M patch_manager_mods/18_mouse_camera_investigation.md`
  - ` M patch_manager_mods/19b_cross_patch_sharing_survey.md`

## 2. Immediate Objective (QH4)
**[Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution)**

Status: PLANNED
Task: [Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume QH4` (or `resume 33_ui_queue_hotbar_fixes`).

## 4. Subagent Delegation Direction
- **Delegate to Subagent**: `python3 scripts/plan_manager.py delegate QH4`
- **Supported Commands**: `oc-subagent <scout|code|ghidra|verify|gate|doc>` or `oc-flow auto "<task>"`

## HK1 (2026-10-11): cmdq-probe 0.1.0 BUILT at db70c05, NOT installed
- Source `patch_manager_mods/cmdq_probe_src` (11 detours, channels pause/cmd/fire/round/clr/med; ini `probe=0` default, `ch_<name>`, `party_only`, `max_lines`). Checker `scripts/cmdq_probe_log_check.py` (flags M1 medpack refused, M2 item never fired, C1 clear before round start). Evidence `.claude/handoffs/hk1/t1_windows.md`, `t2_attack_callers.md`.
- Plan correction: CanUseItem window is **0x5D32AE** (7 B `5E 8B E5 5D C2 0C 00`), read `eax`; 0x5D32AB has an inbound `JMP` from the null-item return (0x5D2F99). Hook 11 steals a `RET 0xC` (check_hook_bytes accepts it; untested live: first thing to verify after install).
- T2: AddAttackActions (0x568540) never re-adds 0x3F (all 12 callers a8!=0, it re-adds 0x0C). Cold re-add candidate = AIActionType47_ReequipResumeAttack 0x5DF920 (hooked as `REEQ`). Batch 2 (warm, not built): gate 0x6D9500/0x6DA49F, mode 0x563C90, AIActionCombat clear+attack 0x6D9931/0x6D996B. Open: is the 0x47 node queued at live combat start (AddDoorSaberAction 0x547BB0 vs script 0x69B980)?
- `CLR` filter uses creature+0x11AC (meaning UNVERIFIED); `party_only=0` drops that test if no CLR lines appear.
- Install = full KPatchLauncher reinstall of cmdq-probe: needs the user's GO + a KPM `Patches/CmdqProbe` mirror (mouse-probe precedent) + `pre_install_gate.sh --mod cmdq_probe_src`. Then S4 with `ch_med=1`: `python3 scripts/kotor2_logs.py check cmdq-probe --new`.

## HK1 INSTALLED (2026-10-11 ~11:00)
- Full reinstall of 40 ids (39 + cmdq-probe), stage `$K/cmdq_stage/`, KPM mirror commit e634f5a (Patches/CmdqProbe). Verified: all peer DLL md5s identical to backup, exe md5 unchanged, LAA 0x23, `kpm_status` cmdq-probe LIVE 6677783b.
- Backup: `/home/ethansm/.steam/steam/steamapps/common/Knights of the Old Republic II/backups/cmdqprobe_full_20261011T105828` (30 DLLs, patch_config.toml, kpm_install_state.json, exe copy, live_md5.txt). Rollback = copy DLLs/config/state back (config changed by the reinstall: do NOT use kpm_hotswap --rollback blindly).
- Game dir `cmdq_probe.ini`: probe=1, all ch_*=1, party_only=1, max_lines=2000. Set probe=0 to silence. Log: cmdq-probe_log.txt -> `python3 scripts/kotor2_logs.py check cmdq-probe --new`, then `python3 scripts/cmdq_probe_log_check.py`.
- Play check S4: click a medpack then a portrait. First thing to verify: CanUseItem hook (0x5D32AE, steals RET 0xC) does not crash; expect `MED site=entry` then `MED site=canuse`.
