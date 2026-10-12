# Session Handoff & Plan State: patch_manager_mods/02_buff_duration_hud.md

## 0. Next-Phase Direction
Debuff fix landed and verified in game (EffectIcon type 0x43 rows, 0.2.1 live c65dc679). Next: record, decide Task 4 force-power icon gaps, optional DBGMISS cleanup.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/02_buff_duration_hud.md`
- **Branch:** `KOTOR-II` (01a0e46 docs(review): track the 09-29 and 09-30 code review feedback docs)
- **Generated:** 2026-10-12 14:21:07 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
01a0e46 docs(review): track the 09-29 and 09-30 code review feedback docs
 .../00_code_review_feedback_2026-09-29.md          | 142 +++++++++++++++++++++
 .../00_code_review_feedback_2026-09-30.md          | 109 ++++++++++++++++
 2 files changed, 251 insertions(+)
```
- **Dirty Files (113):**
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M scripts/kpm_hotswap.py`
  - ` M scripts/kpm_status.py`
  - ` M scripts/pre_install_gate.py`
  - ... +103 more (run `git status --short`)

## 2. Immediate Objective (Phase 2)
**findings (2026-09-29)**

- `CUIHUD`/`CGameEffect` (the spec's original SDK-style guesses) are
- Separately, `FUN_006a5110` (a ~7.7KB AC-bonus recompute function, found
- **Continuation session (2026-09-29): Apply/Remove dispatch architecture
- **3rd session (2026-09-29): TARGET 1 RESOLVED.** A pre-session query of
- `0x005e4e60` = `GetInteger(index)` — `*(int*)(this+0x34)[index]`
- `0x005e4e80` = `SetInteger(index,value)` — same `+0x34` array, write
- `0x005e4ea0` = `GetFloat(index)` — `*(float*)(this+0x38)[index]`
- `0x005e4ec0` = `SetFloat(index,value)` — same `+0x38` array, write

## 2b. Upcoming Batches in Pipeline
- **Phase 3** [Parity Verification] : findings (2026-09-29, planning session)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/02_buff_duration_hud.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate Phase 2` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
### Debuff-icon investigation (2026-10-12)
State: live Buff HUD md5 3c132c47 (repo f13c4f5 = DBGMISS diag; 524a18a = DebuffIconTable iw_ fix; KPM eb17f03). Rollback: `kpm_hotswap.py --rollback buff-duration-hud_swap_20261011_230858`.
Symptom: save loaded with Slow on; Character Effects Panel shows it, Buff HUD portrait/menu show only the engine red arrow.
Findings:
- Debuff arrow = engine byte raw debuff>0 while the HUD's spell-id walk tracks 0 (`ARROWS ... raw=(N,1) tracked=(N,0)`). Never a debuff tracked in 18 launches.
- DBGMISS fired (buffhud_log.txt lines 3945-4027, creature 0bf44d08, effect count 52 -> 47 -> 37): the dump is CAPPED AT 12 LINES, all of them equipped (sub=3) / spell 42 / spell 138 duplicates, so the Slow effect is beyond i=11 and NOT yet seen.
- Seen so far: many spell=-1 effects (type 2/10/13, sub=3 equipped) and duplicate spell=42 (type 2, sub=9).
- The Effects Panel finds Slow through an EffectIcon effect: type 0x43 (67), ints[0]=iconRow into kEffIconRows (CharEffectsPanel.cpp:61,428,455-470; EffectNameTable.inc:324 `/*28*/ { "Slow", "ip_slow", 0 }`, good=0 harmful). The Buff HUD only walks Effect_SpellId, so a Slow carried by a type-0x43 effect (spell -1 or a spell with no debuff icon) is invisible to it. HYPOTHESIS, not yet seen in the log.
- Spell-source facts: spells.2da `usertype` 1 = Force power (48/48 hostile), 2 = monster, 4 = item/droid/grenade/plot; mines are NOT in spells.2da (traps.2da, no spell column). Effect subtype&7: 3 equipped, 4 innate, 1 temporary.
- Force-power gaps in DebuffIconTable: rows 141-143 (BAT_MED_ENEMY, hostile, no iconresref; ip_battlemed01-03 exist), 269 DROID_CONFUSION and 201 DROID_TRICK (hs=0, fh empty -> generator skips), 14 DOMINATE / 6 AFFECT_MIND in EXCLUDE (by design). Slow (spell 45) IS in the table.
Dead ends / traps:
- NEVER cat/tail/grep raw logs; use `python3 scripts/kotor2_logs.py check buffhud_log.txt --launch N` or the scratch filter `scratchpad/dbf.py '<regex>' <lo> <hi> <max>` (prints line numbers; scratchpad is under /tmp/claude-1000/.../scratchpad, recreate if gone).
- Screenshots in ~/Documents (18:12-18:15) showed no party HUD; newest save (cloudsaves/62703561/000008 - Game7) had no Slow effect: neither proves anything.
- `kpm_hotswap` cannot see _shared_src changes: copy the one header into KPM Patches/Common by hand, commit there by explicit path, then call `kpm_hotswap.prepare()` to force the rebuild (done once for DebuffIconTable.inc).
- Gate `pre_install_gate.sh`: 2 failures not from this work (test_agent_contract: AGENTS.md vs CLAUDE.md drift from another session; test_compat_breakage: 15/16 inner checks fail, not isolated).
- Haiku recon agents that read whole save files or research docs peak >100k tokens (C, D did): give them one file and a line window.
Verification commands:
- `python3 scripts/build_check.py --mod buff_duration_hud_src` and `python3 scripts/run_tests.py buff`
- after play: `python3 scripts/kotor2_logs.py check buffhud_log.txt --new` and the scratch filter with regex `DBGMISS`

### Debuff (Slow) fix: LANDED + verified in game (2026-10-12)
- Root cause: script `SetEffectIcon` effects (type 0x43, `ints[0]` = effecticon.2da row, row 28 = `ip_slow`, good=0) have spell -1 / no debuff-table spell, so the spell-id walk tracked 0 while the engine's arrow counter was (N,1).
- Fix: `EffectIcons.h` `iconForEffectIconEffect` + `collectBuffs(debuff=true)` fallback; generated `_shared_src/EffectIconRowTable.inc` (gen_debuff_icon_table.py writes it next to the DebuffIconTable out path). Buff column untouched. Dedupe by icon (row path) and by spell (+ icon seen via rows).
- Commits: repo `4261c1b`, KPM Common `c05cbd1`, KPM mod `e686e77`. Live `buff-duration-hud` 0.2.1 md5 `c65dc679`. Rollback `python3 scripts/kpm_hotswap.py --rollback buff-duration-hud_swap_20261012_005844` (to 0.2.0 `3c132c47`).
- Play log (launch 20 of buffhud_log.txt): `ARROWS raw=(5,1) tracked=(5,1)`, `DEBUFF ... tracked=1 draw=1`, 0 DBGMISS lines. User confirmed the icon shows. The log never names the resref; icon identity is the user's visual check.
- Shared-header workflow used: hand-copy to KPM `Patches/Common/` + forwarder via `check_shared_headers.forwarder()` (no --sync), `check_shared_headers.py` => 0 drifted at that time.
- Gate 10-12: failing = test_agent_contract, test_compat_breakage (known), check-shared-headers (another session's uncommitted `SelfHealUi.h` drift), test_kpm_hotswap (21 fails; other session's uncommitted `scripts/kpm_hotswap.py`; cause not confirmed). None from this work.
- OPEN: (1) Task 4 force-power gaps in gen_debuff_icon_table.py hostile rule (rows 141-143 ip_battlemed01-03, 201/269 ip_scram01/03; 14 DOMINATE / 6 AFFECT_MIND stay out) only on user "cover the 4 gaps"; (2) DBGMISS diag can be removed or kept (capped, debug-gated); (3) Effects Panel not rebuilt (unaffected functionally).
