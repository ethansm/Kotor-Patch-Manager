# Session Handoff & Plan State: patch_manager_mods/46_mod_config_declarative_layer.md

## 0. Next-Phase Direction
Execute MCF1: [Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/46_mod_config_declarative_layer.md`
- **Waves:** .claude/waves/WI-46-a1/
- **Waves:** .claude/waves/WI-46-a2/
- **Waves:** .claude/waves/WI-46-a3/
- **Waves:** .claude/waves/w1-46-prev/
- **Branch:** `KOTOR-II` (bcff4263 play queue: CH-PV1 retest after mod-config 0.3.4-dev (gear toggles the modal) + combat-hotbar 0.11.1-dev (thin hugging frame))
- **Generated:** 2026-10-20 22:24:30 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
bcff4263 play queue: CH-PV1 retest after mod-config 0.3.4-dev (gear toggles the modal) + combat-hotbar 0.11.1-dev (thin hugging frame)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```
- **Dirty Files (185):**
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
  - ... +175 more (run `git status --short`)

## 2. Immediate Objective (MCF1)
**[Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests**

Status: BUILT
Task: [Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests

## 2b. Upcoming Batches in Pipeline
- **MCF5** [Meta / Planning Setup] : [Mod-config] Look-alike slider row (SliderArt extracted; part 1 pure maths 09fbf5d7, part 2 provider wiring)
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

## Update 10-19 01:25
combat-hotbar 0.10.1-dev (d9e96ba7): same early-register fix (`pagePoll()` in KPatchInit; it loads at 173 ms, mod-config at 71 ms). Gate PASS (both), prepare SWAP for combat-hotbar + companion-selfheal 0.9.3-dev, NOT installed. Install both with one GO: `python3 scripts/kpm_hotswap.py --mod combat-hotbar companion-selfheal --apply`.

## Update 10-19 01:13
INSTALLED combat-hotbar 0.10.1-dev (8e05cb4e) + companion-selfheal 0.9.3-dev (0435be5f), both register in KPatchInit. Rollback: `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261019_011326`. Play check: Options -> Mod Configuration must list Companion Self-Heal and Combat Hotbar; logs: `page REGISTER ok` early in both mods' logs (kplog_parse). MCF5 part 2 (slider wiring) is being built by a Sonnet worker in worktree .claude/worktrees/mcf5 (branch mcf5-slider-part2, on e05391f1); rebase onto 0369b41a, review, then merge. Build in worktrees needs a scratch KPM dir (SP/kpm copy of Patches/Common + lib; refresh Common from _shared_src) because the real mirror lacks new headers.

## Update 10-19 01:50 (usage limit reached here)
MCF5 part 2 (slider row, kind MUI_ROW_SLIDER 2, mod-config 0.3.1-dev) is BUILT and Opus-reviewed PASS on branch `mcf5-slider-part2` (worktree .claude/worktrees/mcf5, commit c2357b08, rebased on 9226ec1a). NOT merged, NOT gated, NOT installed.
Next steps: (1) from the main tree `git merge --ff-only mcf5-slider-part2` (or cherry-pick c2357b08); (2) `python3 scripts/check_shared_headers.py --sync`; (3) `bash scripts/pre_install_gate.sh --skip-default --mod mod_config_src --mod companion_selfheal_src --mod combat_hotbar_src`; (4) `kpm_hotswap.py --prepare` (must be SWAP; import table was identical in review); (5) install on user GO, game closed; (6) play rows: selftest=1/2 slider rows, with holster-editor loaded.
Review non-blocking findings: holster-editor and mod-config now both register the hse_sld_* texture names from separate DLLs (needs a play check with both loaded); ModConfigDecl::change rewrites the ini on every slider value change (defer write to release); ROW/SLIDER log cap 200 shared; hardware look/drag feel unverified.
Owner decisions: keep all shared helpers (EditKit/SliderArt/ResetIcon) until MCF5 is played; Holster session told. Live now: combat-hotbar 0.10.1-dev + companion-selfheal 0.9.3-dev (rollback batch_swap_20261019_011326); play check pending that both pages show in Options -> Mod Configuration.

## Update 10-19 02:05
MCF5 slider merged to KOTOR-II (ff, now repo ~7ca9fdfe), shared headers synced (63, 0 drift), gate PASS (mod-config, companion-selfheal, combat-hotbar), `--prepare` mod-config 0.3.1-dev = SWAP, NOT installed (needs user GO, game closed): `python3 scripts/kpm_hotswap.py --mod mod-config --apply`. After install: set `selftest=1` (6 rows + 2 sliders) or `2` in the game-dir mod_config.ini, play the slider rows with holster-editor loaded (shared hse_sld_* texture names), then set it back to 0.

## Update 10-20 13:16 - MCF5 INSTALLED
- `kpm_hotswap.py --mod mod-config --apply`: mod-config 0.3.1-dev LIVE (658ea991). Rollback: `python3 scripts/kpm_hotswap.py --rollback mod-config_swap_20261020_131556`.
- Play row MCR3 added to PLAY_TEST_QUEUE.md (selftest=1 then 2). Pending user play: slider art/drag feel, wheel delivery, hold-repeat, shared hse_sld_* names with holster-editor, SH-PG1 (Self-Heal + Combat Hotbar pages), MCR1/MCR2.
- Shared-helper prune stays ON HOLD until MCR3 is played. Next rows: MCF6, MCF7 (need direction).

## Update 10-20 20:40 - MCR3 first play + slider inset fix
- MCR3 played (selftest=1): mod-config log CLEAN (no WARN/ERR). `SLIDER art applied parts=3/3 after 1 tries` for both rows (no text-mark fallback), 126 SLIDER drag commits (Volume row), art path = fill. The Snap slider (5..95 step 5) saw only 2 changes, so its handler snap/reject case is NOT exercised yet.
- User screenshot: wedge and thumb overhang the frame border (wedge ran to the right frame edge, thumb arrows stuck out above and below). Fix: `mui::sliderInner(frame)` (inset ~10/512 of width, 10/64 of height); wedge x-run, thumb rect and the pointer->value mapping use it; frame stays the hit area. Tests added (test_inner_inside_frame). Repo 4279c35a (mod-config 0.3.2-dev), KPM side committed, `--prepare` = SWAP, gate PASS (md5 f008e6c9). NOT installed: needs the user's GO, game closed.
- Re-check after install: both sliders sit inside the border at 0 and 100; drag the Snap slider across the range and hit the value the handler rejects.

## Update 10-20 20:35 - mod-config 0.3.2-dev INSTALLED
- `kpm_hotswap.py --mod mod-config --apply`: LIVE 4399c8b3. Rollback: `python3 scripts/kpm_hotswap.py --rollback mod-config_swap_20261020_203430`. `selftest=1` is still set in the game-dir mod_config.ini (set to 0 after the play check).
- Play check: sliders stay inside the border at 0 and 100; Snap slider across its whole range incl. the rejected value.

## Update 10-20 21:20 - slider wedge/thumb mismatch fixed (0.3.3-dev prepared)
- User screenshot of 0.3.2-dev: inside the border now, but at 100% the wedge ran a little past the thumb arrow (wedge ended at the inner edge, thumb box stopped half a thumb short). Fix: thumb centre travels the inner run minus half a thumb (`sliderTravel`); wedge ends at the thumb centre (`sliderWedgeFor`), thumb via `sliderThumbFor`; mapping uses `sliderTravelOf`. Test `test_wedge_ends_at_thumb_centre` checks wedge end == thumb centre for every value 0..100.
- Repo 9902c7f9 (mod-config 0.3.3-dev), KPM committed, `--prepare` SWAP, gate PASS. NOT installed (needs GO, game closed). Rollback of the current live 0.3.2-dev: `kpm_hotswap.py --rollback mod-config_swap_20261020_203430`.

## Update 10-20 21:20 - slider wedge/thumb fix INSTALLED (as mod-config 0.3.4-dev)
- The combat-hud session committed mod-config 0.3.4-dev (32d1e9dc: Combat Hotbar test modal gear) on top of my 0.3.3-dev (9902c7f9) before my apply, so `kpm_hotswap.py --mod mod-config --apply` installed 0.3.4-dev (f64b39b6), which contains the slider fix (sliderWedgeFor/sliderThumbFor present) plus their gear change. Rollback: `python3 scripts/kpm_hotswap.py --rollback mod-config_swap_20261020_212011`.
- combat-hotbar 0.11.1-dev is still SWAP (not installed): that session owns its install.
- Play check: both sliders stay inside the border and the wedge end meets the thumb arrow at 0, mid and max; Snap slider full range incl. the rejected value.

## Update 10-20 21:22 - combat-hotbar 0.11.1-dev INSTALLED (user "install", only staged item)
- The slider fix was already live (mod-config 0.3.4-dev). The only SWAP left in `kpm_status` was combat-hotbar 0.11.1-dev (combat-hud session's build, pairs with mod-config 0.3.4-dev). Gate PASS, `kpm_hotswap.py --mod combat-hotbar --apply`: LIVE e5afadac. Rollback: `python3 scripts/kpm_hotswap.py --rollback combat-hotbar_swap_20261020_212120`.
