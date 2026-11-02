# Session Handoff & Plan State: patch_manager_mods/52_quick_loot.md

## 0. Next-Phase Direction
QL3

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/52_quick_loot.md`
- **Waves:** .claude/waves/w1-52-QL0/
- **Waves:** .claude/waves/w2-52-QL1b/
- **Waves:** .claude/waves/w3-52-R2/
- **Waves:** .claude/waves/w4-52-QL2V/
- **Waves:** .claude/waves/w5-52-QL2A/
- **Branch:** `KOTOR-II` (357fd2d6 quick-loot: gate QL0 probe lines behind Verbose=0 (doc 52 QLR))
- **Generated:** 2026-11-01 21:20:55 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
357fd2d6 quick-loot: gate QL0 probe lines behind Verbose=0 (doc 52 QLR)
 patch_manager_mods/quick_loot_src/QuickLoot.cpp  | 11 +++++++----
 patch_manager_mods/quick_loot_src/quick_loot.ini |  2 ++
 2 files changed, 9 insertions(+), 4 deletions(-)
```
- **Dirty Files (212):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/50_k2_render_performance.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/52_quick_loot.handoff.md`
  - ` M .claude/handoffs/53_rich_menu_typography.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ... +202 more (run `git status --short`)

## 2. Immediate Objective (**QL3**)
**True hover peek: own floating name + list on hover (app+0x4B0), merge with doc 12 empty-container cursor**

Status: TODO
Task: True hover peek: own floating name + list on hover (app+0x4B0), merge with doc 12 empty-container cursor

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/52_quick_loot.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **QL3**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- TLK names carry {tag} prefixes ({08}, {CA}, {light 04}): strip every {...} (engine hides them). Disruptor damage word is "Unstoppable" (engine 0x6140c0 -> TLK 38552.. bits), NOT Energy.
- 1d10 renders as 1D10 (reads 1010) in dialogfont: show min-max ranges, also for cost-table values like "1d10".
- Hide: CastSpell props, UseLimitationRacial Human (all party = race 6 Human), Range rows, "Upgrades: None". Fold DecreaseAC into Defense, bonus damage into Damage.
- Gemini review checked 10-29: 7/8 issues valid; "wrong item in ql06_tooltip_2" was mockup sample data only.
- Sound holder for 0x4122a0 not reachable yet: narrow recon first, else CExoSoundSource fallback (0x70ab90 ctor + 0x70ad50 play, probable).
- plan_manager handoff regeneration wipes this section's text when it was appended before: re-append after every handoff run.
- Verify: python3 scripts/run_tests.py quick_loot; python3 scripts/build_check.py --mod quick_loot_src; gate fails 5 peer suites (level-up exports, doc 51 raw addresses), not ours.

## 5. Dead Ends & Verification (manual)
- `lockertsfinv` is NOT a stash container: per-item first-pickup notice inside CloseInventory (005f5770.c:85-130). R2/R3 guessed wrong.
- `AcquireItemsFromObject 0x5F5160` is SpawnBodyBag's corpse copy, not the GUI take path.
- Hooking the call site 0x82AD21 (E8 rel32) is avoided: KPM rel32 relocation unproven. Hook ShowContainerGui entry 0x7CACB0 (9 B, no rel branches) with consumed_exit_address 0x7CAD1F instead.
- Evidence wave: .claude/waves/w1-52-QL0/ (u01 scaffold, u02 early-exit, u03 item readers), check PASS.

## 5. Dead Ends & Verification (manual)
- TLK names carry {tag} prefixes ({08}, {CA}, {light 04}): strip every {...} (engine hides them). Disruptor damage word is "Unstoppable" (engine 0x6140c0 -> TLK 38552.. bits), NOT Energy.
- 1d10 renders as 1D10 (reads 1010) in dialogfont: show min-max ranges, also for cost-table values like "1d10".
- Hide: CastSpell props, UseLimitationRacial Human (all party = race 6 Human), Range rows, "Upgrades: None". Fold DecreaseAC into Defense, bonus damage into Damage.
- Gemini review claims checked 10-29: 7/8 issues valid; "wrong item in ql06_tooltip_2" was mockup sample data only.
- Sound holder for 0x4122a0 not reachable yet: one narrow recon (who owns +0xa0 guisounds table; global path) before QL12, else CExoSoundSource fallback.
- Verify: python3 scripts/run_tests.py quick_loot; python3 scripts/build_check.py --mod quick_loot_src; gate fails 5 peer suites (level-up exports, doc 51 raw addresses) not ours.

## 5. Dead Ends & Verification (manual, 0.7.0-dev 10-29)
- STATE: quick-loot 0.7.0-dev committed 40198d3c (doc ea4a31de), prepared KPM 1006ab8, NOT installed. Swap: game closed + user GO + gate waiver, then `python3 scripts/kpm_hotswap.py --mod quick-loot --apply`; play row QL07-P1.
- Gate 10-29: only check-raw-addresses fails, all 21 FAILs in active_effects_panel_src (doc 51 peer); quick_loot clean.
- Untested live guesses (check the QL07-P1 log): 0x614E10 9th arg flag=0 (PROPTXT lines), GetGUIString text in the out temp (TLK-EMPTY), Plot = item+0x10C (SORT plot=), body slot mask 0x2 (EQUIP).
- TipLineH = 34 (engine multi-line label pitch of 0.6.x), not the mockup's 30: a 30 px layout clips the last line.
- Equipped foot is range only "(1-8)": damage words overflow the 660 px tip; name cut with "..." first.
- Recon r01 (.claude/waves/w7-52-QL9/r01.md, gitignored; ledger 20261029_quick-loot-07-verify.tsv committed): 0x614E10 callers unread; 20261029_quick-loot-sound-equip.tsv has 24 lint issues (older wave; lint total 108).
- Verify: python3 scripts/run_tests.py quick_loot (6 suites); python3 scripts/build_check.py --mod quick_loot_src.

## 5. Dead Ends & Verification (manual, 0.7.0-dev 10-29)
- STATE: quick-loot 0.7.0-dev LIVE 10-29 01:08 (dll b2c9d82f, code 40198d3c, KPM 1006ab8, rollback `kpm_hotswap.py --rollback quick-loot_swap_20261029_010838`). Gate waived by user (only doc 51 active_effects_panel raw-address FAILs). Next: user plays QL07-P1, then `python3 scripts/kotor2_logs.py check quick-loot_log.txt --new`.
- Untested live guesses (check the QL07-P1 log): 0x614E10 9th arg flag=0 (PROPTXT lines), GetGUIString text in the out temp (TLK-EMPTY), Plot = item+0x10C (SORT plot=), body slot mask 0x2 (EQUIP).
- TipLineH = 34 (engine multi-line label pitch of 0.6.x), not the mockup's 30: a 30 px layout clips the last line.
- Equipped foot is range only "(1-8)": damage words overflow the 660 px tip; name cut with "..." first.
- Recon r01 (.claude/waves/w7-52-QL9/r01.md, gitignored; ledger 20261029_quick-loot-07-verify.tsv committed): 0x614E10 callers unread; 20261029_quick-loot-sound-equip.tsv has 24 lint issues (older wave).
- Verify: python3 scripts/run_tests.py quick_loot (6 suites); python3 scripts/build_check.py --mod quick_loot_src.
- 10-29 02:38: 0.7.1-dev LIVE (page registers at init + rows Item tooltip / Quest first / Compare with equipped / Panel background; FOOT + EQUIP-MISS logs), rollback quick-loot_swap_20261029_023816, play QL071-P1. 0.7.0 log: no EQUIP lines at all (foot path cause open, 0.7.1 logs it).
- 0.7.2-dev committed ed553180, KPM 90f57c4, status REINSTALL (new hook H4 0x0074FB20 target-arrow draw, consumed exit 0x00750764 epilogue; recon .claude/waves/w8-52-QL14/r01.md). Install: user GO + game closed -> `python3 scripts/kpm_reinstall.py --mod quick-loot`. Risk: skipping the draw may leave the arrow sprite stale (owner+0x56c8 flush unknown) -> QL072-P1 checks it; kill switch HideTargetArrow=0.
- mod-config binds its entry list once at the Options CTOR: any page registered later never shows (quick-loot 0.7.0 hit it). Peer fix idea for doc 46: rebind on LIST enter when g_reg.count() grew.
- 10-29 02:44: 0.7.2-dev LIVE via kpm_reinstall (58 patches, 0 peer restores), rollback kpm_reinstall.py --rollback backups/quick-loot_reinstall_20261029_024402; play QL071-P1 + QL072-P1.
