# Session Handoff & Plan State: patch_manager_mods/27_companion_selfheal.md

## 0. Next-Phase Direction
play test complete next step is log analysis and fixing any issues. Feedback: (1) looking a lot better, can we make the on/off toggle not as wide? (2) hover interactions for self-heal ux buttons underneath the window are interacted with, they shouldn't be while its open.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/27_companion_selfheal.md`
- **Waves:** .claude/waves/w1-27-leaderpop/
- **Waves:** .claude/waves/w2-27-leaderpop/
- **Waves:** .claude/waves/w3-27-polish/
- **Branch:** `KOTOR-II` (7c5ecad docs(38): AI texture pipeline research report (Haiku swarm + verify wave))
- **Generated:** 2026-10-12 17:28:05 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
7c5ecad docs(38): AI texture pipeline research report (Haiku swarm + verify wave)
 .claude/research/texture_ai_pipeline/report.md | 131 +++++++++++++++++++++++++
 1 file changed, 131 insertions(+)
```
- **Dirty Files (100):**
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/engine_ai_atlas/tools/check_atlas_windows.py`
  - ` M patch_manager_mods/queue_hud_src/QueueHud.cpp`
  - ... +90 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/27_companion_selfheal.md`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

### Session 10-11/12 lessons (selfheal 0.7.0-dev, repo dcc0bde, KPM a2b5614, live dll ad5af355)
- Popover text was invisible because align 18 = 0x12 has the 0x10 vertical-centre bit and engine Draw 0x47B500 drops a vertically-centred single line when box h < line h (V3.md F3). Align 17 = 0x11 is STILL vertical-centre. Align 10 (0x08 top | 2 centre-h) works: play-tested 10-12, text visible. Text-object gate / colour readback were fine (DIAG pop rgbOk=1), so it was never a null-renderer problem.
- Button backdrop is now csh_disc / csh_disc_hi (circular, hover halo); W_Frame* widgets are never drawn; hudkit::ensure() removed from selfheal.
- Play log 10-12 (selfheal_log_check.py): P1 P3 P4 P5 PASS, USE=1 DONE=1 TIMEOUT=0, rows h=24 at 3840x1600, DISC lines hit the 48 cap (hover=19), CS12 REFUSE-queued=5.
- Screenshot 10-12: popover box too small, "Auto: ON / Use / Keep 4" wraps to 3 lines -> popUnit (SelfHealUi.h:157, clamp 16..24) and Auto width 8h must grow; ON/OFF row does not look clickable.
- Verification: `python3 scripts/selfheal_log_check.py`, `python3 scripts/run_tests.py -k selfheal`, `python3 scripts/build_check.py --mod companion_selfheal_src --quiet`, install = kotor2-kpm-install fast path (kpm_hotswap --prepare / --apply; user GO).
- Raw log reads are forbidden; use selfheal_log_check.py or a script filter capped at a few lines.
- 10-12 11:xx 0.8.1-dev play test (installed dll a98b5866, repo e9ff67d): selfheal_log_check.py = P1-P7 PASS, popsize n=1 min_slack=26, rows h=52 at 3840x1600 (unit = ceil(1.5*line), NOT 24: the pill is 4*(unit-4) = 192 px wide, that is why the toggle looks too wide), POPHOVER=11 POPCLICK=1, leader CLICK0 ok/noneed/noitem=0 and USE=0 DONE=0 (leader click path NOT exercised yet), REFUSE-queued=2 (known CS12, WARN class). The `ARROW neg=/pos=` log line is not summarised by the checker: add it (INFO + WARN when 00000000).
- User feedback 10-12: (1) toggle pill too wide; (2) while the popover is open, self-heal buttons/discs UNDER it still react to hover (sound + disc halo): hover must be suppressed when the cursor is inside the popover rect (uiFrame computes onBtn/hover BEFORE popLayoutFor/inPop; reorder).
- Build gate trap: `build_check` resolves SelfHealUi.h from KPM Patches/Common (stale until copied). Use a scratch KPM tree (`cp -r $K/Patches/Common` + `ln -s $K/lib`, overlay repo `_shared_src/SelfHealUi.h`) with `--kpm-dir`, or `check_shared_headers.py` shows DRIFT; copy only the one header by hand and commit it in KPM separately (never `--sync`).

### Session 10-12/14 (selfheal polish round 3)
- 0.8.2-dev INSTALLED 10-12 17:44 (repo 1d65bd2, doc 26dcfe2, KPM 563870a, dll a4544278, backup companion-selfheal_swap_20261012_174417): pill 2:1 (pillW = 2*pillH, textures 64x32), cursor inside open popover never hovers buttons beneath (`POPOCCLUDE m= btn=` log), checker summarises ARROW + POPOCCLUDE. Live now is 0.8.3-dev (other session, 23934f0).
- Play test of 0.8.2/0.8.3 only partly seen: no selfheal feedback received yet; leader left-click heal path still unverified (USE=0 in latest log). Next: `python3 scripts/selfheal_log_check.py` after a play session.
- Unrelated queue-hud feedback received in this session: see `.claude/handoffs/30_queue_feedback_1014/FINDINGS.md` (cancel button missing out of combat; medpack command duplicated to another member after Tab).
