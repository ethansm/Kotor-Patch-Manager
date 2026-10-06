# Session Handoff & Plan State: patch_manager_mods/24_master_plan_status.md

## 0. Next-Phase Direction
HP5 & HP7 in parallel, HP6 when HP5 is done

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/24_master_plan_status.md`
- **Branch:** `KOTOR-II` (53f7110 smokes: queue-hud 0.4.0-dev (5 HudKit textures), selfheal 0.7.0-dev (16 widgets, 14 textures))
- **Last Landed Commit:**
```
53f7110 smokes: queue-hud 0.4.0-dev (5 HudKit textures), selfheal 0.7.0-dev (16 widgets, 14 textures)
 .../perf_lab_src/harness/smoke_queuehud.cpp          |  8 ++++----
 .../perf_lab_src/harness/smoke_selfheal.cpp          | 20 ++++++++++----------
 2 files changed, 14 insertions(+), 14 deletions(-)
```
- **Dirty Files (142):**
  - `M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M .claude/skills/kotor2-crash-triage/SKILL.md`
  - ` M .claude/skills/kotor2-forced-state-testing/SKILL.md`
  - ` M .claude/skills/kotor2-log-analysis/SKILL.md`
  - ` M AGENTS.md`

## 2. Immediate Objective (L4)
**Design-only: hotkeys, hud-host, k2.guiinput**

Status: DEFERRED
Task: Design-only: hotkeys, hud-host, k2.guiinput

## 2b. Upcoming Batches in Pipeline
- **HP5** : Action Hotbar test hygiene + extraction: freezeEffective, useRoute, collectAutos, classifyRow, pickerSignature/modeBtnWhy (HB04, HB05, HB07, HB08, A11); spells fixture usertype/hostilesetting + healing row (HB09); Config edge cases (HB10-HB19); golden hashes, drop constant-restating checks, fold hotbar_bugs_test, `kt::TempDir`, quiet output, fixture freshness (HB20-HB26, HB30)
- **HP6** : Action Hotbar constants + minor: `S_Count` (A14), RingSize helpers (A24), key/flag constants (A25, A26), named engine offsets (A27), timing constants + fallback rect (A28), dead code (A30), naming (A32), log tag style (A34), ignored results (A35), strtol/MaxEntries (A39), saveDirFor probe + TwoDA stack buffer (A40)
- **HP7** : spells.2da fallback (RE gate first): read via the resource manager when `override/` lacks it, or WARN once + document (A5)

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.
