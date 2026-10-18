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
