# Session Handoff & Plan State: patch_manager_mods/32_common_migration_round2.md

## 0. Next-Phase Direction
32:MG4 — Ranks 6 & 7: Menu 3D & Logging Common Migration (32_common_migration_round2.md:48 + :49 + 24_master_plan_status.md:58). IniKit/PanelSlots in EquipCharPreview.cpp + CharEffectsPanel.cpp; KpLog across perf-lab and robustness-fixes

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/32_common_migration_round2.md`
- **Branch:** `KOTOR-II` (1ec5c5b action-hotbar 1.2.2-dev INSTALLED (HP4 hot-swap eeb6144f, KPM 9136ea5): doc 07 status, manifest + CHANGELOG mirror)
- **Last Landed Commit:**
```
1ec5c5b action-hotbar 1.2.2-dev INSTALLED (HP4 hot-swap eeb6144f, KPM 9136ea5): doc 07 status, manifest + CHANGELOG mirror
 patch_manager_mods/07_force_power_hotbar.md        | 6 +++---
 patch_manager_mods/action_hotbar_src/CHANGELOG.md  | 4 ++++
 patch_manager_mods/action_hotbar_src/manifest.toml | 2 +-
 3 files changed, 8 insertions(+), 4 deletions(-)
```
- **Dirty Files (133):**
  - `M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/23_menu_3d_shared_migration.md`
  - ` M patch_manager_mods/24_master_plan_status.md`

## 2. Immediate Objective (MG4 equip3d)
**MG4 equip3d**

Status: TODO
Task: MG4 equip3d

## 2b. Upcoming Batches in Pipeline
- **MG5 chareffects** [Meta / Planning Setup] : MG5 chareffects

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## MG4/MG5/C5c planning notes (10-05, refactor-mg3 session)
- Recon: `.claude/handoffs/recon_mg4_mg5.md` (equip3d + chareffects sites) and `.claude/handoffs/recon_c5c.md` (perf-lab + robustness logging + log consumers).
- MG3 recipe = commit 8aed2f5 (inv3d); installed 10-05 18:04 (DLL 5d073e98), play check pending.
- Dead ends / do NOT:
  - inikit::parseFloats for equip3d: it accepts hex floats; equip3d's F19 guard rejects them. Keep eq3d::parseFloats local.
  - guikit::setVisible in place of setCtlVisible (equip3d/chareffects): it toggles 0x20 and breaks click-through labels.
  - party::controlledCreature for the chareffects leader: it calls 0x7E5D80; the mod uses PartyGetAt(table,0) = party::member(party::table(),0).
  - forEachLine (kills the per-line reject logs); check_shared_headers --sync; editing Common headers.
  - Starting MG4 while `equip-3d-vertical-pan` has uncommitted pan edits in EquipCharPreview.cpp/Equip3dLogic.h.
- plan_manager.py bug: `handoff "<very long direction>"` raises OSError ENAMETOOLONG in resolve_plan_shorthand (Path(token).exists()); workaround `--plan <doc>` + shorter text. File is dirty from another session: not fixed.
- Verify: `.venv/bin/python scripts/run_tests.py` (137 PASS), `--wine` (221 PASS), `-k equipment_3d|active_effects|perf_lab|robustness`, `scripts/check_subclassers.py`, `scripts/check_raw_addresses.py`.
