# Session Handoff & Plan State: .claude/plans/holster_hw_ux3_build.md

## 0. Next-Phase Direction
HW-UX3 fix round: (1) slider thumb drag follows cursor (Scrub scale 2W/trackW, hints, tests), (2) restore Reset Scope button (end of pageLayoutV3 order, two-click confirm, 70 boxes + 6 wedge labels <= 80). Then Wave D install for user play test.

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/holster_hw_ux3_build.md`
- **Branch:** `KOTOR-II` (1fbcd9f0 docs(holster): HW-UX3 build round handoff)
- **Generated:** 2026-10-18 14:02:50 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
1fbcd9f0 docs(holster): HW-UX3 build round handoff
 .claude/handoffs/holster_editor_ux_v2.handoff.md | 8 ++++++++
 1 file changed, 8 insertions(+)
```
- **Dirty Files (173):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ... +163 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/holster_hw_ux3_build.md`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
- Wave D state at hand-off: `check_shared_headers.py --sync` DONE (52 headers, 0 drifted; SliderArtTex.inc now in KPM Common/_shared). `kpm_status.py`: MIRROR for enemy-effects-strip, holster 0.2.0-dev, holster-editor 0.4.0-dev, mod-config 0.1.3-dev (so the KPM tree still needs the source mirror via --prepare). `--prepare` NOT run yet: held for the fix round (it builds from the repo sources). User gave "go Wave D" and wants it installed after the fixes for play testing.
- Trap: `agent_contract.py check` takes the reply file FIRST (`check --type worker <reply.txt> --scope <globs...>`); `check --scope ... -` as in CLAUDE.md fails (argparse eats the dash). Recon agents' own output files show as UNCLAIMED warnings; include their dir in --scope.
- Trap: re-taking `agent_contract.py snapshot` while another worker is running baselines its in-progress edits; snapshot once per launch batch before launching, not mid-run.
- Trap: PLAY_TEST_QUEUE.md has a foreign uncommitted edit; commit my rows via `git hash-object -w` + `git update-index --cacheinfo`, then `git add` my other paths and `git commit` WITHOUT pathspec (done once, worked).
- Trap: new raw engine addresses in a .cpp fail check_raw_addresses until added to tests/static/raw_addresses.toml (alphabetical by path, uppercase 0x00XXXXXX); query_ghidra first.
- Fix-round facts: Scrub (HseView.h:923) steps 1 px = kScrubOffPerPx 0.001 m / 0.5 deg while the track spans the full +-W window (kScrubWindowOff 0.30 => ~2.4 mm per track px), thumb lags ~0.4x; `inspTrackValue(row, px, trackX, trackW)` HseView.h:1111 is the jump mapping. Reset Scope: old code (git show 041aab12:patch_manager_mods/holster_editor_src/HolsterEditor.cpp lines ~481, 2159) called g_hol.h->ResetScope() (HolsterApi.h:64 deletes scope keys then saves) behind guardTransition(ACT_RESET_SCOPE); new GuardAction enum is HolsterEditor.cpp:120 without it; PageIdxV3 enum HseLayoutV3.h:280 (PI_SAVE 16, PI_REVERT 17, ... PI_HINT 67 + 2 = 69).
- Verification commands: `python3 scripts/run_tests.py hse_view hse_layout_v3 reset_icon slider_art guikit common_contract menu_ui_logic`; `python3 scripts/run_tests.py -k holster_`; `python3 scripts/check_raw_addresses.py` (FAIL=0); `python3 scripts/build_check.py --mod holster_editor_src --mod holster_src --mod mod_config_src`.

## Fix round status (10-18, after a2cfadcf)
- DONE + committed: 373643b8 (track-scaled scrub, "Reset" button Save|Revert|Reset, ACT_RESET_SCOPE two-click), 0fa157f4 (Opus fix: scrub overshoot via unclamped virt, warning "Reset? Click again"), ba3229e6 (play rows UX3/UX3b/UX3c/UX3d/CAM2/LOG1/ICON1/HW7a/HW7c restored: commit 8d90707c from another session had dropped them). Opus: APPROVE-FOR-INSTALL (research/holstered_weapons/ux_v3/OPUS_review_fix_round.txt). holster-editor 0.4.1-dev; label is "Reset" ("Reset Scope" does not fit a third-width box).
- Non-blocking: "Revert" label overflows ~0.4 px at 1280x720.
- INSTALL BLOCKED: `kpm_hotswap.py --prepare` DONE (SWAP x3, mirrored, not applied). `pre_install_gate.sh --mod mod_config_src --mod holster_src --mod holster_editor_src` = GATE FAIL 1: suite:check-hook-sources, activation_fix_src/kotor2-steam-aspyr.hooks.toml hook 0x0040D69E source "esp+0" (another session's 0be31786). User chose: wait for the activation-fix owner. Then: re-run gate, `kpm_hotswap.py --mod mod-config holster holster-editor --apply`, `kpm_status.py` LIVE x3 (game closed). Re-run `--prepare` first if KPM mirrors changed.

## INSTALLED 10-18 14:34 (supersedes "INSTALL BLOCKED" above)
- Blocker fixed by 04448fca (check_hook_sources allowlist for activation-fix 0x0040D69E esp+0, with tests). Gate PASS at head 04448fca; apply done; kpm_status LIVE x3: holster 0.2.0-dev, holster-editor 0.4.1-dev, mod-config 0.1.3-dev. Rollback: `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261018_143419`.
- Play rows pending: UX3, UX3b, UX3c, UX3d, ICON1, HW7a, HW7c, LOG1, CAM2. KPM-side commits not made (commands printed by kpm_hotswap).

## 10-18 15:24 layout-scale round (user screenshot at 3840x1600)
- Cause: real panes column=(772,357,1132,757) view=(1345,357,576,896) preview=(1953,357,1113,746); layout capped rows 22 / column 260 / inspector 460 and measured text with sy=1 while the engine draws it ~2.5x wider. Fix 823d1e21: holster-editor 0.4.2-dev, ini `text_k` (default 2.5; tune from play), caps raised (rows 34, col 440, insp 720, name 120, value 140), testReal3840x1600. LIVE 15:24; rollback `python3 scripts/kpm_hotswap.py --rollback holster-editor_swap_20261018_152410`.
- Open: confirm by play (screenshot); if text still clips/has slack adjust `text_k` in holster_editor.ini (game-dir copy) before touching code. Other Mod Configuration pages are correct (user): do not touch mod-config.
