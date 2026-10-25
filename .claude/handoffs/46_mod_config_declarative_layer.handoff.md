# Session Handoff & Plan State: patch_manager_mods/46_mod_config_declarative_layer.md

## 0. Next-Phase Direction
RF1/RF2 BUILT 61131905 (mod-config 0.4.7-dev, combat-hotbar 0.11.5-dev), KPM ee8a3e8/685487c, both SWAP, GATE PASS head ea2e954e. Next: user GO (game closed) -> kpm_hotswap.py --mod mod-config --mod combat-hotbar --apply, then mark-done 46:RF1/RF2 INSTALLED and set MC-TT1 LIVE. RF3 PARTIAL: optional follow-up guikit::imageResolved via ResMan probe 0x711ED0 (see .claude/research/pv_missing_tex/README.md). MCF7 waits on peer EnemyStrip.cpp.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/46_mod_config_declarative_layer.md`
- **Waves:** .claude/waves/WI-46-a1/
- **Waves:** .claude/waves/WI-46-a2/
- **Waves:** .claude/waves/WI-46-a3/
- **Waves:** .claude/waves/w1-46-prev/
- **Branch:** `KOTOR-II` (4a5e60c3 doc 46: MCF6 DROPPED (superseded by MCF5); RF3 recon DONE (ResMan pre-check 0x711ED0, no signal on image) + REA ledger)
- **Generated:** 2026-10-25 19:43:39 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
4a5e60c3 doc 46: MCF6 DROPPED (superseded by MCF5); RF3 recon DONE (ResMan pre-check 0x711ED0, no signal on image) + REA ledger
 .../46_mod_config_declarative_layer.md             |  4 ++--
 .../20261025_pv-missing-tex-resref.tsv             | 26 ++++++++++++++++++++++
 2 files changed, 28 insertions(+), 2 deletions(-)
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

## 2. Immediate Objective (RF1)
**[Mod-config] `dropRefs` also clears the GUI manager tooltip owner `M+0x40` (and owner flags48 bit 0x40) before a label is freed; mod-config 0.4.7-dev; play row MC-TT1**

Status: BUILT
Task: [Mod-config] `dropRefs` also clears the GUI manager tooltip owner `M+0x40` (and owner flags48 bit 0x40) before a label is freed; mod-config 0.4.7-dev; play row MC-TT1

## 2b. Upcoming Batches in Pipeline
- **MCF6** [Meta / Planning Setup] : [Mod-config] Native slider embed probe (gated by MCF0)
- **MCF7** [Meta / Planning Setup] : [Enemy strip] Migrate its page to the declarative layer
- **RF2** [Research & Probes] : [Combat hotbar] Preview icons that never resolved get a fresh 20 tries per Options open (`pvOnBuilt`); combat-hotbar 0.11.5-dev

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/46_mod_config_declarative_layer.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate RF1` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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

## Update 10-20 23:05 - ESCPROBE crash fix prepared (mod-config 0.3.7-dev)
- Peer kotor-ii-e6 reported a fresh-load crash (page fault in the code cave at 0x09BA0000) from the Esc probe stubs added in 0.3.5/0.3.6-dev (other session's commits 2e2fe1d6, 381c60ea). Verified: 48-byte stub at a 40-byte stride overlapped its neighbours and the cave was 8 bytes short.
- Fix: `mod_config_src/EscProbeStub.h` (kStubLen/kStride, static_assert, emitStub, caveBytes) + `escprobe_stub_test.cpp`; `installInputProbe` uses it. Repo 86d455bc, KPM cf0306c, `--prepare` SWAP, GATE PASS.
- LIVE mod-config is still 0.3.6-dev (afa73345) and contains ESCPROBE: do NOT launch the game until 0.3.7-dev is installed (`python3 scripts/kpm_hotswap.py --mod mod-config --apply`, needs user GO) or the probe is rolled back (`kpm_hotswap.py --rollback mod-config_swap_20261020_212011` = 0.3.4-dev, no probe, loses nothing of the slider/gear work).
- Unexplained: the earlier 21:47 crash (peer's open item); at that time the live build was 0.3.4-dev (no probe).

## Update 10-20 22:55 - mod-config 0.3.7-dev INSTALLED (Esc probe crash fixed)
- `kpm_hotswap.py --mod mod-config --apply`: LIVE cca903f7. Rollback to 0.3.6-dev (crashes): `kpm_hotswap.py --rollback mod-config_swap_20261020_225536`; no-probe 0.3.4-dev: `--rollback mod-config_swap_20261020_212011`.
- First launch check: game reaches the world; `kotor2_logs.py check mod-config_log.txt --new` shows `ESCPROBE stubs on N vtable slots` (N ~25) and no crash. Open Options once, press Esc, then read `ESCPROBE dump` rows.

## Update 10-20 23:45 - Esc back handled by the Esc-probe session
- User play result on 0.3.7-dev: everything good except Esc (it closed the whole GUI from an entry page). The other session (ESCPROBE owner) implemented it as mod-config 0.4.0-dev (02cffa1f: Options key-handler slot 14 hooked via vtable data patch, event 0x2a swallowed on non-MAIN pages, probe removed) and it is LIVE (0396f7c4). Not played by me; the probe/EscProbeStub.h work is superseded (header may be dead code now).
- Still to play-verify with 0.4.0-dev: Esc from an entry page goes to the mod list, from the list to the Options root, from root closes; Esc with the combat-hotbar test modal open closes the modal first; sliders inside the border (0.3.2/0.3.3 fixes); Snap slider full range incl. the rejected value.

## Update 10-21 Esc step-back VERIFIED in game (mod-config 0.4.5-dev, 734f29bc, live 351333c2)
- 0.4.0-dev never patched: the key handler 0x8A2070 is vtable slot **15** (stored at 0x9A7058), not 14 (slot 14 = 0x40E770 per-frame update). Fixed 0.4.1-dev. The REA ledger row and the `GameAddr.h` comment ("slot 14") are still wrong.
- Real Esc arrives as event **0x28 state=1** (then 0xdf state 0), not 0x2a. Swallow 0x28/0x2a when Esc is held and the page is not MAIN (0.4.2-dev).
- The tick-side Esc-edge reset raced ahead of the hook on some presses (hook then saw MAIN, vanilla closed the menu). It is now a fallback only when the hook is not installed (0.4.3-dev).
- Vanilla Esc plays GUI sound 5 via FUN_007CA060 = the item-equip sound, wrong for menus. The menu boop is guisounds row 0 (same as the Back button); Esc step-back plays row 0 (0.4.5-dev). REA ledger: `ghidra_knowledge/rea_findings/20261021_options-esc-sound.tsv`. 0x4122A0 is PlayGuiSound, not a close function.
- Play-verified: Esc steps back one page on every press, boop sound correct.
- Still unplayed: sliders inside border / wedge meets thumb, Snap slider range + rejected value, SH-PG1, MCR1-3, hold-repeat, modal-closes-first on Esc. `selftest=1` is still set in the game-dir mod_config.ini.
- KPM-side commit for mod-config not yet made (kpm_hotswap prints the command).

## Update 10-25 18:20 - pooled-fill free guard LIVE (mod-config 0.4.6-dev, e432a5d1; combat-hotbar 0.11.4-dev unchanged)
- Root cause class (static, V1 + own read of decomp/41/00414c60.asm): the border dtor deletes +0x6C/+0x70/+0x74 children unconditionally (`CMP [ECX+0x74],0; JZ; vt[1](1)`). Any control freed while holding a `guikit::imageFor/imageForRetry` POOLED image deletes it for every holder and leaves the per-DLL pool dangling.
- Fix: `guikit::isPooledImage` + `detachPooledFills(ctl)` (label border or both button borders), called in `releaseUnlinkedLabel`, `releaseUnlinkedButton` (GuiButton.h) and mod-config `freeCtl` (logs `pooled=N`). PreviewCanvas::destroy still restores each label's own fill first (lets the own image be freed). Host test in guikit_test.cpp. KPM Common cc840f0, ModConfig 0dd33c4. Rollback `kpm_hotswap.py --rollback mod-config_swap_20261025_181514`.
- Exposure covered by rebuilds only: enemy-effects-strip (EnemyStrip.cpp:1123/1150 use releaseUnlinkedLabel) gets the guard at its owner's next build (peer has uncommitted EnemyStrip.cpp; do not build it from here). Holster-editor FREEVIEW (HolsterEditor.cpp:3087) frees a scene view, not a border label: not affected.
- Logs 10-25 17:18 launch: CLEAN; Options opened once, no page entered, so 0.11.4's preview-then-close path is still unplayed. Play row MC-PF1 (grass sliders + combat page, close, reopen, close).
- Review leftovers (not fixed): `iconFallbacks` cannot see a missing resref (engine returns a blank white texture, not null: needs a resource-exists check, no helper in _shared_src); `pvImage` tries (max 20) persist across Options opens (CombatHotbar.cpp:936-950); dropRefs never clears tooltip owner M+0x40 (V1 fix text: `if (rd(M,0x40)==L){ wr(L,0x48, rd(L,0x48)&~0x40u); wr(M,0x40,0); }`, consumer RemoveToolTip 0x413F00); per-tick model rebuild + setClickable writes are cheap, leave.
- Verification used: `run_tests.py mod_config previewcanvas guikit common_contract combat_` (44/0), `compat_check.py _shared_src/GuiKit.h _shared_src/GuiButton.h` (43/0, after `check_shared_headers.py --sync`: build_check compiles against KPM Common, so sync BEFORE build_check), `GATE PASS mods=mod-config head=b4515f02405d`.
