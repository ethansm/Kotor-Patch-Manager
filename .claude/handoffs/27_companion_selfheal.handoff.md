# Session Handoff & Plan State: patch_manager_mods/27_companion_selfheal.md

## 0. Next-Phase Direction
Execute CS12: selfheal 0.6.0-dev live, await play test in 27_companion_selfheal.md

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/27_companion_selfheal.md`
- **Branch:** `KOTOR-II` (c79ee0c doc 27: CS12 0.6.0-dev installed 01:55 + live checklist)
- **Last Landed Commit:**
```
c79ee0c doc 27: CS12 0.6.0-dev installed 01:55 + live checklist
 patch_manager_mods/27_companion_selfheal.md | 6 ++++--
 1 file changed, 4 insertions(+), 2 deletions(-)
```
- **Dirty Files (132):**
  - `M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M .claude/skills/plan-handoff/SKILL.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/README.md`
  - ` M patch_manager_mods/_shared_src/GuiKit.h`

## 2. Immediate Objective (hidden)
**the slot has no creature, or it's the leader**

Status: HIDDEN
Task: the slot has no creature, or it's the leader

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

- 10-05 ~01:58: sibling hot-swapped level-up-override.dll (1.5.0-dev, 91bbda85). Next full install must restore 4 keep dlls: shield-bar, perf-lab, mouse-probe, level-up-override (copy live LUO dll into the new backup first).
- 10-05: INI path fix (45bb926 / KPM c5f5393) is source-only; live 968041f1 still reads [General] Log / [probe] call from %WINDIR% (defaults 1/0 = intended). Ships with the next selfheal build.
- 10-05 03:40: combat-hotbar 0.6.0-dev hot-swapped (60c4ecbb, KPM e1a2e04 kpatch matches). Before ANY full install, always re-run the stager and copy back every dll=NO-MATCH live dll; as of 03:40 that is shield-bar, perf-lab, mouse-probe, level-up-override, ranged-ai, texmem, equip-icon, activation (8).
- 10-05 11:36: combat-hotbar 0.7.0-dev hot-swapped (f5a02668, KPM 286427f, backup ch4e_swap_20261005_1136). Keep it like the other NO-MATCH dlls on any full install.
