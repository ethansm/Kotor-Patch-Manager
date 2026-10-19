# Session Handoff & Plan State: patch_manager_mods/46_mod_config_declarative_layer.md

## 0. Next-Phase Direction
Combat HUD preview mockup with gear button + test-state modal, and move those components into the mod-config provider

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/46_mod_config_declarative_layer.md`
- **Waves:** .claude/waves/WI-46-a1/
- **Waves:** .claude/waves/WI-46-a2/
- **Waves:** .claude/waves/WI-46-a3/
- **Branch:** `KOTOR-II` (f32ccef7 PLAY_TEST_QUEUE: CH-PG1 combat-hotbar 0.10.0-dev LIVE 10-19 00:45)
- **Generated:** 2026-10-19 00:47:41 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
f32ccef7 PLAY_TEST_QUEUE: CH-PG1 combat-hotbar 0.10.0-dev LIVE 10-19 00:45
 patch_manager_mods/PLAY_TEST_QUEUE.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```
- **Dirty Files (179):**
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
  - ... +169 more (run `git status --short`)

## 2. Immediate Objective (MCF1)
**[Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests**

Status: BUILT
Task: [Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests

## 2b. Upcoming Batches in Pipeline
- **MCF5** [Meta / Planning Setup] : [Mod-config] Look-alike slider row (needs Holster-session SliderArt extraction)
- **MCF6** [Meta / Planning Setup] : [Mod-config] Native slider embed probe (gated by MCF0)
- **MCF7** [Meta / Planning Setup] : [Enemy strip] Migrate its page to the declarative layer

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/46_mod_config_declarative_layer.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate MCF1` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## Session note 10-18 night (read first after /clear)
Full build log, deviations, review findings and open items: `patch_manager_mods/46_mod_config_declarative_layer.md` section 17. State: MCF1-MCF4 + MCF2b BUILT and INSTALLED (LIVE 10-18 23:32, rollback `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261018_234815`); nothing played yet (PLAY_TEST_QUEUE rows MCR1, MCR2, SH-PG1 pending). Opus review 2 (MCF2b/MCF3/MCF4) was still running: check `git log --oneline -10` for a commit starting `mod-config/selfheal review fixes (MCF2b-MCF4):`; if present, re-gate, `--prepare` (must be SWAP), and re-swap with user GO. Next phases MCF5-MCF7 are later rows; play-test results decide the scroll bar look and wheel delivery first. Read logs only through `kotor2_logs.py check mod-config_log.txt --new` and `selfheal_log_check.py`.

## Update 10-18 23:48
Review 2 landed (2026b001) + selfheal 0.9.1-dev bump (2a35790f). Re-swapped LIVE: mod-config 0.2.0-dev (0e3c78c3), companion-selfheal 0.9.1-dev (655fc04a). Rollback: `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261018_234815`. Pending: user plays MCR1/MCR2/SH-PG1. Review-2 unfixed minors: untagged-follower slotN fallback, stale hover text after scroll, TomlMini BOM/bracket spaces, kpm_hotswap misses shared-header changes (bump version to force).

## Update 10-18 23:58 (first play: Self-Heal page missing)
Cause: `PAGE schema rejected: line 5: string too long (max 95)` - 5 schema descs exceeded TomlMini's 95-char limit, so the page never registered. Fix 96e23ede (companion-selfheal 0.9.2-dev): schema moved to `shpage::kPageSchema` in SelfHealPage.h, descs shortened, `test_shipped_schema` parses it. Gate PASS, prepare = SWAP, NOT YET INSTALLED (needs user GO, game closed). Live is still 0.9.1-dev. Rollback for the live batch: batch_swap_20261018_234815. After install: play SH-PG1; check `selfheal_log_check.py` for no `PAGE schema rejected` and a `REGISTER companion-selfheal` line in the mod-config log (via kplog_parse). Lesson: any shipped TOML schema needs a host test that parses it (combat_hotbar already has one).

## Update 10-18 23:57
INSTALLED companion-selfheal 0.9.2-dev (2eb7fd17), schema fix live. Rollback: `python3 scripts/kpm_hotswap.py --rollback companion-selfheal_swap_20261018_235711`. Pending: user plays SH-PG1 (page must now appear).

## Update 10-19 00:55 (combat-hotbar config page session; read before the preview/gear plan)
- DONE + LIVE: combat-hotbar 0.10.0-dev (c30c557f, KPM f94deec), hot-swapped 10-19 00:45 (rollback `combat-hotbar_swap_20261019_004552`), play row CH-PG1. Page = `combat_hotbar_src/CombatPage.h` (4 ini-bound stepper rows via `mcdecl::Page` + `mcdecl::fileIo()` with `INIW_FILE_API`/`MCDECL_FILE_API`), registered from `K2HUD_FRAME` (only after a save is loaded). All `[Buttons]` keys live (BUILD always creates MaxBtn=96 slots).
- OWNER ASKS (not started, plan mode next): (1) offline Combat HUD preview mockup (placeholder values) with a gear button opening a modal to configure demo states (no force powers, 1 vs 10 items, empty groups, edge cases) so the owner can push/test layout code; model on `scripts/holster_editor_mockup.py` (+ `test_holster_editor_mockup.py`); (2) move the preview/gear/modal pieces into the mod-config provider (`K2MenuUiApi`/ModConfigApi.h, 19 slots) so other config pages can opt in. Owner wording: "we should add these components to the mod config provider so other config pages could use the gear button etc".
- Facts: TomlMini string values max 95 chars (schema desc must be <=95). stepper `zero_text` gives "Label: Auto". `kpm_status` SWAP even though objdump import set grew: trust it. Combat layout is pure in `combat_hotbar_src/CombatLogic.h` (layoutBannerPills/Groups, fillSlots, orderGroup), host tests in `combat_logic_test.cpp`; a preview can reuse those pure functions via a host/py bridge. Recon notes: `.claude/waves/combat_cfg/A.md..D.md,V1-V3` (gitignored).
- Traps: the pre-install gate runs on the SHARED tree, so a peer session's half-edited files fail it with changing sets (holster, k1_port). Verify in a clean `git worktree` needs `ln -s <main>/.venv .venv` and still differs on gitignored data. `k1_port` test_single_body_model fails from commit 709ff749 (owner waived 10-19). Do not run a long foreground gate in one call the owner may interrupt: use run_in_background.
- Do NOT edit ModConfigApi.h/ModConfig.cpp/GuiKit.h without announcing: doc 46 session owns them; holster session owns HseParty/SliderArt/ResetIcon.

## Update 10-19 01:10 (page still missing after 0.9.2)
Cause (from kplog_parse of mod-config + selfheal logs): selfheal DID register (token 4) but at T=25.5 s, after mod-config built its Options panel at T=17.3 s. mod-config snapshots the registry once per panel (CTOR/McBind clones controls only while the vanilla layout is open) and the panel is reused for the whole session, so late registrations never appear. enemy-strip registers in KPatchInit (T=0.4 s) and works. Fix 267febee (selfheal 0.9.3-dev): `pagePoll()` at the end of KPatchInit (mod-config loads first, 71 ms vs 179 ms), frame poll kept as fallback; plus connect-result logging (`PAGE k2.modconfig ok`, one-shot `PAGE not registered after 600 polls`). Gate PASS, prepare = SWAP, NOT installed (needs user GO). Live is 0.9.2-dev.
Same class: combat-hotbar registers from its HUD frame callback (token 3 at 21 s) so its page is also missing; not ours (owner: combat-hotbar session). Provider-side hardening (open item): rebuild the panel or log "late REGISTER ignored until the panel is recreated" in mod-config's Register path.
