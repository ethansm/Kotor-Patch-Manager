# Session Handoff & Plan State: patch_manager_mods/31_combat_action_hotbar.md

## 0. Next-Phase Direction
CH4c: vanilla-styled target-menu buttons, zero-lag (anchor in menu draw), replace vanilla group buttons, wider rows

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/31_combat_action_hotbar.md`
- **Branch:** `KOTOR-II` (9661710 doc 27: 0.5.0 first look + user direction (cross v3, CS11 queue kit not visible))
- **Last Landed Commit:**
```
9661710 doc 27: 0.5.0 first look + user direction (cross v3, CS11 queue kit not visible)
 patch_manager_mods/27_companion_selfheal.md | 4 ++++
 1 file changed, 4 insertions(+)
```
- **Dirty Files (149):**
  - `M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M .claude/skills/plan-handoff/SKILL.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/README.md`

## 2. Immediate Objective (CH1c)
**Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3**

Status: TODO
Task: Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## User direction (verbatim, 2026-10-05 01:11, after first CH4b play test)
"good proof of concept. We now need to build real UX button elements that use the same style of the vanilla combat quick actions, similar to how the action hotbar is based on the styling on the quickbar in the bottom left. Use the same border, fill, highlight, sound etc. Can we replace the existing vanilla action buttons completely? Only noticed issue is that the buttons are not tight to the vanilla UX. When moving the camera, they can lag behind. can we replace that Vanilla UX piece directly in the game code that renders it, so it renders our new buttons instead, and then is also 'stuck' to it? We could make our rows of buttons wider since there's extra room left and right of the current buttons. Not a lot bigger but room for the actual Button UX with padding/spacing"
Screenshot: /home/ethansm/Documents/Screenshot_2026-10-05_01-11-46.png (CH4b live: 2-col icon grid under g0 + g2, plain icons, no frames).

## Live state (10-05 00:54)
hud-host 0.3.0-dev c6121ae2 (owns 0x743540, 0x743850, 0x40F5A0), companion-selfheal 0.4.2-dev 3e763e45 (src a9ee44d), combat-hotbar 0.4.0-dev ee1112e0 (src d75d4d0). KPM 0e8942d. Backup $G/backups/combathud3_install_20261005_005409. Peer hot-swapped DLLs perf-lab/mouse-probe/queue-hud were copied back post-install (verify shows 3 expected mismatches).

## Known cause of the lag
POST callback runs at the ENTRY of 0x743850, which itself writes the menu origin (+0x1648/+0x164C) from the projected target point. So our rects use LAST frame's origin = 1-frame lag (plus our setRect happens before the draw of the same frame, fine). Fix candidates: post-exit point of 0x743850 (RET 8), or make our controls children of the target menu so the engine moves them.

## Dead ends / traps
- Installing while a peer session writes $K/Patches/<Dir>: stage picks the half-written kpatch. Check `git -C $K status --short Patches/<Dir>` + mtimes right before staging; message the owner session.
- Stage NO-MATCH on peer hot-swapped DLLs: back up, install, copy those DLLs back (hooks SAME).
- check_shared_headers --sync would push sibling uncommitted header edits into KPM: copy only our header.
