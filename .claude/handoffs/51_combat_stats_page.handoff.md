# Session Handoff & Plan State: patch_manager_mods/51_combat_stats_page.md

## 0. Next-Phase Direction
51 ST3 user review + ST4 install GO

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/51_combat_stats_page.md`
- **Branch:** `KOTOR-II` (265ae68b character-effects-panel 0.3.0-probe (doc 51 ST4): STATS log block + DEFLECT check hook H-C5 at 0x006E2351 (log only))
- **Generated:** 2026-10-27 02:18:12 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
265ae68b character-effects-panel 0.3.0-probe (doc 51 ST4): STATS log block + DEFLECT check hook H-C5 at 0x006E2351 (log only)
 patch_manager_mods/PLAY_TEST_QUEUE.md              |   1 +
 .../active_effects_panel_src/CharEffectsPanel.cpp  | 136 ++++++++++++++++++++-
 .../active_effects_panel_src/CombatStatsAddr.h     |  27 ++++
 .../active_effects_panel_src/chareffects.ini       |   2 +
 .../active_effects_panel_src/exports.def           |   1 +
 .../kotor2-steam-aspyr.hooks.toml                  |  28 +++++
 .../active_effects_panel_src/manifest.toml         |   2 +-
 7 files changed, 193 insertions(+), 4 deletions(-)
```
- **Dirty Files (186):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M .claude/settings.json`
  - ... +176 more (run `git status --short`)

## 2. Immediate Objective (ST3)
**Mockup: `scripts/character_page_mockup.py` renders the Stats tab + one vanilla-row tooltip (Defense)**

Status: PARTIAL
Task: Mockup: `scripts/character_page_mockup.py` renders the Stats tab + one vanilla-row tooltip (Defense)

## 2b. Upcoming Batches in Pipeline
- **ST4** [Sync / Header Mirroring] : Probe 0.3.0-probe: STATS log block per shown character (no UI) + read-only deflection check (engine total vs recompute)
- **ST5** [Sync / Header Mirroring] : Stats tab UI (header toggle, sections, rows, breakdown tooltips, `show_influence`)
- **ST6** [Sync / Header Mirroring] : Vanilla page tooltips (hover rects by tag, `vanilla_tooltips=1`, text table)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/51_combat_stats_page.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate ST3` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
- W1 Haiku claimed a Weapon Master Deflect "engine bug" (bonus never summed): REFUTED by W2 (summed at asm 006e1ef9). Never ship a bug claim from one Haiku layer.
- stats+0x178/+0x179 (ungated deflection/skill term): only CSWSCreatureStats::Constructor writes them (0) per `field_census.py --offset 0x178 --all --writers`; treat as always 0.
- GetAttacksPerRound 0x006a7170 is a stub returning 1; the real count is written by CalculateOnHandAttacks 0x005905f0 into round+0xae0.
- All ST1 functions are already `verified` in the Steam DB (`python3 scripts/query_ghidra.py address <hex>`); no new DB rows or Haiku decoy verification needed, only mod-local constants.
- Haiku RE briefs: token caps are ignored (peaks 112-200k); cap tool calls <= 12 and name line ranges.
- ST1+ST2 landed fb39ff72 (3 Haiku workers, $0.04 total, all first-pass PASS). API: `cstats::deflection/forceDC/fpCostMult/critRange/critMult/formDefense` return `Breakdown{total,count,parts[12]{label,value}}` or small structs; `cstats::kDeflectNote` = Precise Shot tooltip line.
- ST3 recon (10-27): `scripts/character_page_mockup.py` (637 lines, doc 14) already exists but is broken: `texel_widths()` (line ~144) reads `NAME_TABLE` (KPM Patches/CharEffectsPanel/EffectNameTable.inc), whose font tables moved to `patch_manager_mods/_shared_src/FontWidths.inc`. With `src = (SHARED_SRC / "FontWidths.inc").read_text()` it renders all 7 PNGs in about 6 s. Run with `.venv/bin/python` (system python has no PIL). The live `override/character_p.gui` ALREADY has the thirds layout + LBL_EFX_* panel, so `char_before.png` = what the live game shows; `thirds()` on it double-shifts (char_after*.png are stale). The Stats mockup must draw on the live ctls as loaded.
- `agent_contract.py check`: put the reply file BEFORE `--scope` (`--scope` is nargs+ and swallows a trailing `-`). Unclaimed `.claude/research/quick_loot/*` hits are another session's doc 52 work, not ours.
- Verify: `python3 scripts/run_tests.py combat_stats_test` (after ST2 registers it in active_effects_panel_src/test.toml); `python3 scripts/compat_check.py <files>` only if _shared_src is touched (plan: it is not).
- ST3 (10-27, ad46e86d): `.venv/bin/python scripts/stats_page_mockup.py OUTDIR` renders stats_tab / stats_tab_tip_deflection / vanilla_tip_defense (+ _zoom). PARTIAL until the user OKs the look. character_page_mockup.py now reads `_shared_src/FontWidths.inc`.
- ST4 (10-27, 265ae68b): 0.3.0-probe BUILT, GATE PASS md5 b553fac0. Hook H-C5 0x006E2351 (orig 0F B6 91 C1 00 00 00): ECX=rec, [ebp-0x20]=total, [ebp-0x18]=defender, [ebp-0x34]=engine eff delta (uninit if loop skipped), [ebp-0xb8]=attacker. Asm-confirmed: JD/FR/WM/BAB use DEFENDER stats, Precise Shot (feats 0xF0..0xF4) uses ATTACKER stats; rec+0x17A/B can be stale, so the probe recomputes PS from the attacker (logs psRec as a cross-check). Needs REINSTALL (new hook) with user GO; play row ST4-P1. Ini key `deflect_check=1`.
