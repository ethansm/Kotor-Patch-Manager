# Session Handoff & Plan State: patch_manager_mods/33_ui_queue_hotbar_fixes.md

## 0. Next-Phase Direction
QL15: play check, then read the logs. Everything is LIVE as of 10-17 01:30:
- queue-hud 0.4.2-dev (`5978bdd`); rollback `queue-hud_swap_20261017_013024`.
- combat-hotbar 0.9.2-rc, action-hotbar 0.1.5-dev and companion-selfheal 0.8.4-dev (`baca6a9`); rollback `batch_swap_20261017_013047`.
- KPM commits: 4f452db, 1bb19ea (CombatTrace headers), then the 3 mod mirrors.
PLAY_TEST_QUEUE rows MODE1 and CST1 are pending.
After play:
1. Freeze the logs to `.claude/handoffs/ql12/logs/` before any relaunch.
2. Run 3 read-only Haiku units:
   - H1: `queue_hud_log_check.py <frozen> --no-advance` for MODES and flags L/M/N.
   - H2: `combat_timeline.py <frozen files>` for cross-mod DISAGREE rows.
   - H3: MODE lines per minute and cap hits; the target is under ~200 per combat.
3. Compare against the MODE1/CST1 pass signals.
Skipped in QL16: companion-ai (no frame callback; its E1/E2 lines already feed the timeline), and buff HUD / shield bar / enemy strip (no HUD pointer). The combat-hotbar CSTATE is sampled only on target-menu rebuilds.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/33_ui_queue_hotbar_fixes.md`
- **Waves:** .claude/waves/WI-33-a1/
- **Waves:** .claude/waves/WI-33-b1/
- **Waves:** .claude/waves/w1-33-QL7/
- **Waves:** .claude/waves/w2-33-QL9/
- **Branch:** `KOTOR-II` (df2c7ec docs(33): QL12-QL14, QL16 BUILT + handoff (install needs GO))
- **Generated:** 2026-10-17 01:31:04 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
df2c7ec docs(33): QL12-QL14, QL16 BUILT + handoff (install needs GO)
 .../handoffs/33_ui_queue_hotbar_fixes.handoff.md   | 104 ++++++++++++++-------
 patch_manager_mods/33_ui_queue_hotbar_fixes.md     |   8 +-
 2 files changed, 73 insertions(+), 39 deletions(-)
```
- **Dirty Files (183):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M CLAUDE.md`
  - ... +173 more (run `git status --short`)

## 2. Immediate Objective (QL15)
**[Queue HUD] Play check MODE1 (PLAY_TEST_QUEUE): medpack out of combat, stim on a member with no enemies (soft), hostile appears (soft->hard + BANNER restore), fight to the end, menu open/close; read with `queue_hud_log_check.py` MODES + flags L/M/N**

Status: PLANNED
Task: [Queue HUD] Play check MODE1 (PLAY_TEST_QUEUE): medpack out of combat, stim on a member with no enemies (soft), hostile appears (soft->hard + BANNER restore), fight to the end, menu open/close; read with `queue_hud_log_check.py` MODES + flags L/M/N

## 2b. Upcoming Batches in Pipeline
- **QH4** : [Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution)
- **QL1** : [Queue HUD] Non-combat queue fix: pause restore (active-command verdict), ghost `cur`, identity guard, combat hand-off (doc 30 "Queue-fix QL1")
- **QL2** : [Queue HUD] Strip flicker (active-cmd slot + 400 ms hold), hand-off clears engine queue first (`handoff_clear_first`), ROUND/HANDOFF_CLEAR trace; medpack = recon only (doc 30 "Queue-fix QL2")

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/33_ui_queue_hotbar_fixes.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate QL15` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- 2026-10-11: QL3/QL4 hot-swapped (queue-hud ada41e82, repo 37325e9, KPM d9e807c, rollback `kpm_hotswap.py --rollback queue-hud_swap_20261011_135120`). Play session logs frozen under `.claude/handoffs/ql6/logs/` (queue-hud 517 lines, cmdq-probe 2011, action-hotbar 1874, hud-host 58, companion-selfheal 138; written 18:07-18:17).
- QL3 diagnosis was from the 13:07 log: our ServerClearAllActions killed no trigger (0/3); engine-held head was skipped. The user says the first click is STILL eaten: do not assume QL3 is the cause, re-read the new log. Attack clicks are unhooked/untracked.
- Do not raw-read `*_log.txt`; use `queue_hud_log_check.py <copy>` and `cmdq_probe_log_check.py <copy>` plus bounded `sed -n` windows (<=40 lines) on the frozen COPIES.
- Dead end (QL5): forcing the engine combat panel out of combat (bit 1 of `[CSWGuiMainInterface+0xA904]` / 0x74C11B) drags in the minimap slide + red Cancel Combat. The new "hide the combat banner, keep combat camera" ask must NOT reuse that seam blindly.
- Verify: `python3 scripts/run_tests.py queue_hud`, `python3 scripts/build_check.py --mod queue_hud_src`.
- 2026-10-12: QL6a-d INSTALLED (queue-hud 83c8a25b, repo 1b49189, KPM d387cf5, rollback `kpm_hotswap.py --rollback queue-hud_swap_20261011_195114`). Peer commits da75b91 (BG09), b64a59c (BG08), 209de3a (BG23) touch queue_hud_src AFTER the live DLL: the next build includes them; run `kpm_status.py` first.
- Play result 10-12 (user): medpacks work in the non-combat queue; buffered force powers still do not fire; per-item cancel still absent. Frozen logs: `.claude/handoffs/ql7/logs/` (queue-hud 1721 lines, cmdq-probe 836 incl. FIRE/CLR, action-hotbar, hud-host). Wave facts: `.claude/waves/w1-33-QL7/u01..u15.md` (untracked, Haiku sieve, PARTIAL verdicts).
- QL9 finding: spells sent straight through (`dec=dispatch_now`: cmds 9,10,11,14,15,16) FIRE; spells BUFFERED then replayed by `dispatchAction` (cmds 4,13) never reach the server cast routine (cmdq-probe hooks 0x565310 and logs no CMD/FIRE for them; items replayed the same way DO show CMD real=0/1). The hotbar lost nothing: 10/10 invokes have a queue-hud CMD. Suspects, unproven: (a) replay always passes a NON-NULL `pos` (zeroed local copy) where the engine's original `posPtr` may be NULL (QueueHud.cpp:1741 posVal/posPtr, 882-895 dispatchAction); (b) captured target `0x00dbee74` is identical on every spell (probe shows tgt=1). Do NOT assume either; add a SPELL_ARGS trace first.
- cmdq-probe CLR callers: 0x66f392 = 46/70 clears, 26 within 1 ms of ROUND ev=start (engine combat-start clear); 0x7519ec / 0x51cb38 clear q=3f within ~12 ms of a spell click/buffer. Unresolved: whether they eat spells or coincide.
- Soft-combat data (CMBT_FIELDS, 8 edges): t550/t564/t568 hold object ids (low 24 bits 1af,1b3,1b5,1b7 look like party members; 7f000000 = none). Edges after ally items show cr520 dword0=1 and dword1 = a nonzero counter (0x1f1e,0x11fc,0x1390,0x1122); the zero-pattern edges are unlabeled. NO eaten-first-click was found in this session (clicks before edges left an entry). Ground truth needs a labeled play step.
- Flag gate (QL6b): `WAIT why=flags` holds of 1.2-2.6 s occurred; one `hold=flag_cap` (cmd 54, ab8=1 for 2.0 s with an empty engine queue). Spells held by flags (cmd 13) still did not fire after release, so the gate is not the spell cause.
- Per-item cancel (QL7) recon: icon rects exist (`Draw` x/y/w/h, `StripDraw.r[j]`, QueueHud.cpp:525-528, 586-596) but nothing maps an icon to a queue entry; only the 3 Cancel controls are clickable (g_cw[3], click poll 1559-1568). A VISIBLE GuiKit control swallows the click (GuiKit.h:233); open: does a transparent-art visible control still swallow it.
- 2026-10-12 P2 play (QL9 INSTALLED, queue-hud 157562a8, repo 6d2ca9e, KPM c56b57f, rollback `kpm_hotswap.py --rollback queue-hud_swap_20261012_012108`): force powers work (user). Analysis + frozen logs: `.claude/handoffs/ql8/findings.md`, `.claude/handoffs/ql8/logs/`, wave `.claude/waves/w2-33-QL9/` (Haiku, PARTIAL). Whole session was mostly PAUSED, so probe `real=0` is the paused signature, not a failure. No buffered spell appears in that log.
- QL8 G4 data: SOFT_CAND hostile triple per combat edge. S3 (ally medpack, no enemies) = 0,0,0 with party=1 on m0; all 7 other edges have hostile=1 on >=1 member. Rule chosen: soft = combat AND some member targets a party id AND no member targets a non-party id. All-none counts as HARD (conservative). n=1 soft sample: the live check is the real test.
- Banner flags: engine builder 0x747210 writes `param_1[0x1be7] &= ~2` for LBL_CMBTMODEMSG (control at hud+0x6F54, flags dword hud+0x6F9C) and LBL_CMBTMSGBG (control hud+0x709C, flags hud+0x70E4). Unknown: whether the engine re-sets the visible bit every frame in combat; the plan clears it every K2HUD_FRAME and logs BANNER lines to find out.
- QL10 (user choice: old queue first, then the new click) needs recon: `handoff_clear_first` wipes the engine's new 3f attack at combat start (L316 cmd 8, L380 cmd 9 of the ql8 log). Do NOT touch it in QL8.
