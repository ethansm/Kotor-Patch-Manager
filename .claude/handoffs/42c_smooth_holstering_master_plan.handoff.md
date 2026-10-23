# Session Handoff & Plan State: patch_manager_mods/42c_smooth_holstering_master_plan.md

## 0. Next-Phase Direction
holster 0.21.4-dev is BUILT and gated (GATE PASS at 697a957d), NOT installed. Next: (1) on the user's GO: `python3 scripts/kpm_hotswap.py --mod holster --prepare` -> commit KPM -> fresh `scripts/pre_install_gate.sh --mod holster_src --skip-default` GATE PASS -> `--apply` -> `kpm_status` LIVE; (2) user plays SM-L19 (variant ladder BASE/V1/V2/V3/V4, restart per variant); (3) read `holster_log_check.py <log> --row SM-L19`, then decide HW45 per the rules in section 5 and pick the HW50 variant (ini default for the mining laser). HW28b (auto-picked) is GATED, not the next step.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/42c_smooth_holstering_master_plan.md`
- **Waves:** .claude/waves/w1-42c-HW45b/
- **Waves:** .claude/waves/w2-42c-HW45b/
- **Waves:** .claude/waves/w3-42c-HW45/
- **Waves:** .claude/waves/w3-42c-HW50/
- **Waves:** .claude/waves/w4-42c-HW50/
- **Branch:** `KOTOR-II` (ef421722 holster_log_check: --row SM-L19 (HW50 cat/mname per gob, max pkdps per cat+dir; HW45 dq summary per category with verdict))
- **Generated:** 2026-10-22 20:11:48 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
ef421722 holster_log_check: --row SM-L19 (HW50 cat/mname per gob, max pkdps per cat+dir; HW45 dq summary per category with verdict)
 scripts/holster_log_check.py      |  98 +++++++++++++++++++++++--
 scripts/test_holster_log_check.py | 150 ++++++++++++++++++++++++++++++++++++++
 2 files changed, 243 insertions(+), 5 deletions(-)
```
- **Dirty Files (184):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M .claude/settings.json`
  - ` M CLAUDE.md`
  - ... +174 more (run `git status --short`)

## 2. Immediate Objective (HW28b)
**Draw cap raise (`draw_slide_cap`, default 570)**

Status: GATED
Task: Draw cap raise (`draw_slide_cap`, default 570)

## 2b. Upcoming Batches in Pipeline
- **HW33** : Two hands, item 3 only: support-hand reach ONLY if play SM-L8 shows the left hand off the weapon (items 1, 2, 4 moved to HW33a)
- **HW32b** : Re-run the sweep with the measured back-mount quat, re-tune hinge out + extract defaults
- **HW34** : Saber blade gate (ignite after the swing clears, douse before the stow carry)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/42c_smooth_holstering_master_plan.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate HW28b` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
HW45-HW49 batch (2026-10-22, plan-handoff evidence wave w1-42c-HW45b):
- Cards: `.claude/waves/w1-42c-HW45b/cards/w1/{B,C1,D1,E}.md`, `cards/w2/{A,C2,D2}.md`; facts `u01..u08.md`; Opus decisions `cards/DECISIONS.md`. Each card passes `wave_cards.py verify` ALONE; the directory check fails on file-level overlap (Holster.cpp, HolsterReach.h) by design: variants run in worktrees, Opus applies consolidated diffs serially.
- Dead end HW45: "within kStepDeg of prev, then nearest dq" is a measured no-op (identical numbers, 31/31 pass). Rate-limiting the roll alone cuts error to 119 but adds a 120 deg swing step at 180.7 deg (fails holster_twist_test.cpp:241). Baseline: max error 149.61 vs candidate-0 119.96, first at 233.5 deg raw.
- Trap: an untracked stale `Holster.cpp` (2482 lines) sits at the repo root; bare `Holster.cpp` anchors resolve to it. Always use `patch_manager_mods/holster_src/Holster.cpp`.
- Trap: no host test compiles Holster.cpp; after every apply run `python3 scripts/build_check.py --mod holster_src`.
- Fact: `kpm_hotswap.py` never copies holster.ini; shipped ini has reach_test=0, so via keys are inert on a fresh install.
- Fact: X8 hover text is `patch_manager_mods/holster_editor_src/HseSpeed.h:61` speedTip() (HolsterEditor.cpp:1899 is a comment).
- Verification: `python3 scripts/run_tests.py holster` ; `python3 scripts/build_check.py --mod holster_src` ; `python3 scripts/build_check.py --mod holster_editor_src` ; `scripts/pre_install_gate.sh --mod holster_src --mod holster_editor_src --skip-default`.

Batch executed 2026-10-22 (wave dir `.claude/waves/w2-42c-HW45b/`: variant diffs, *_cons.md, *_review.md, gate.md, A_probe_{a,b,c}.md):
- LIVE 18:58: holster 0.21.3-dev + holster-editor 0.7.2-dev (repo 8278fad4, KPM 4406542), rollback `batch_swap_20261022_185836`; play row SM-L18 pending.
- Dead end HW45 (3 Haiku probes, table in doc 42c 4.16): (a) glide comb toward cand 0 -> 160 deg sw step at the roll flip (~233.5 deg), swing goes to 156 deg (cap 60 broken); (a2) glide toward dqn: best fidelity (max E 28.38) but swing over cap; (b) hysteresis band 5-20 deg -> 90-96 deg discrete step; (c) shared roll state 1-4 deg/call -> 120 deg sw step at 233.5 (target twist flips +80 -> -80). E0 (candidate 0) itself is ~120 deg off dq: inside the 80/60 caps dq past ~140 deg is unreachable, so the question is which wrong answer looks least wrong. Probe harness: `.claude/waves/w2-42c-HW45b/probe/measure.cpp` (+ per-mechanism sources in probe/{a,b,c}/).
- Trap: shared build staging dir races with parallel builds; workers use `build_check.py --build-root <scratch>/build`.
- Trap: doc 42c master table had a blank line before HW45 (rows invisible to mark-done); fixed in 8278fad4.
- Trap: never `git add -N .` (marked 1842 untracked files intent-to-add; undone with `git rm --cached --pathspec-from-file`).
- Research 2026-10-22 evening (commit cf5cbb36): `.claude/research/holster_hw45/README.md` (7 probes + Sonnet verify) and `.claude/research/holster_hw50/README.md` (8 Haiku angles + Sonnet verify). Read the READMEs, not the raw reports.
- HW45 verified: NOT sweep-only. reachWristDelta(180) + blendQ hand the split the sweep's exact inputs once a turn crosses 180 deg (scratch chain.cpp matches 149.61/266). The P3 "180 clamp truncates" claim is wrong in float32 (relies on cos(float(pi/2)) < 0, unpinned). With 80/60 caps, gates step<5 and E<=E0+1 look jointly unsatisfiable (7 mechanisms). Do NOT probe more pick mechanisms. Next: R2 log `dq=`/`dj=` + R1' chain test, then decide from play data.
- HW50 verified: `w_HvyBlstr` ("Heavy Blaster") is CatPistol (HolsterLogic.h:66); only repeaters are CatHeavy. For repeaters the via is the top suspect (path 90 -> ~355 deg at the documented barrel-down mount, peak x4). Heavy shares every ease curve; draw ends at full hand speed. SM-L15 checker does NOT measure head clip. All heavy motion keys are boot-only (restart per ini variant). No play log has a heavy SLIDE/REACH.
- Follow-ups from reviews: D6 leaves `twistOn` set; `targetDelayed` retarget skips the M1 via ease; frame-1 extract share falls back to the bone flag (B).

Batch executed 2026-10-22 20:00-20:12 (wave dir `.claude/waves/w4-42c-HW50/`: cards/, *_v*.diff, *_final.diff, *_cons.md, *_review.md, gate.md):
- Commits: L1 9ef7f917 (REACH dq=/dj=), L2 bebf5bab (SLIDE cat=/mname=, end gob=/pkdps=), E2 a6cda6f7 (ini docs), T1 c359aa86 (chain test), K1 3e156357 (reach_ease_soft_<cat>), K2 697a957d (reach_via_tilt_deg_<cat>), play row e4096829 (SM-L19), manifest 2b9da6f2 (0.21.4-dev), C1 ef421722 (--row SM-L19). GATE PASS head=697a957d (code identical at ef421722: later commits are docs/scripts only).
- HW45 decision rules after SM-L19 (no code before): dq<150 everywhere -> close DONE "synthetic only, pinned by T1"; dq>=170 seen but no visible wrong-way wrist -> R3(a) WONTFIX with data; crossing + visible -> R3(b) raise reach_wrist_max_deg toward 120/180 (ini), then R3(c) a2 + roll continuity (~20 lines, relaxed swing-cap gate), and R3(d) blendQ sign choice in reachWristDelta (~8 lines) if dj>60 coincides with the weight ramp.
- Fact: dq= logs dqTot, already scaled by w * reach_wrist_pct (reads low during the arm-weight ramp, whole turn at full weight); with reach_twist_deg=0 it is capped at reach_wrist_max_deg.
- Fact (K2 measured, whole turn incl. roll, hand axis forward): back mount near straight down 267/183/93 deg at tilt 0/45/90; 90 deg mount 270/315/180 (not monotone); up mount 90 constant. Tilt goes toward body forward = parent Gob +0xA4 quat * (0, sin t, cos t), unverified live (SM-L19 asks); if the body read fails the tilt falls back to world +y (K2 review).
- Fact: the checker's SLIDE regex needs dly=/slowx= right after ms= and ms= right after owner= on end, so new SLIDE fields are always appended at the end.
- Review notes (minor, not fixed): K1 test (b) partly self-compares, reversal tests skip extractPct>0, previewSeedSlide may copy a slide across categories; L2 pkdps is slightly high-biased by whole-ms dt; T1 assert (a) is sign-blind (b pins the flip).
- Not done: CLAUDE.md lists `--row SM-L11..SM-L15`; it now goes to SM-L19 (CLAUDE.md has another session's uncommitted edits, so it was left alone).
