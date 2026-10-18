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

## 10-18 16:10 button-caption debug (screenshot ~/Documents/Screenshot_2026-10-18_15-52-06.png, 0.4.2-dev LIVE)
- Symptom: at row height 34 every BUTTON caption (weapons, bones, scopes, Save/Revert/Reset, mode bar, stances) is absent or drawn on the box seam / below the box (+15..+24 units); LABELS (name, status, slider names/units, group titles) render correct. OLD 0.4.1 (rows 22): captions inside the box, offset -3. Screenshot scale: 1 screenshot px (2700 wide) = 1 layout unit = 1 engine px.
- Text width is NOT the problem: measured 1.98 px per kFontTexW texel (text_k ~2.64; 2.5 is fine). The "R...", "M...", "..." cuts are a symptom only if the box measured small; captions that fit are simply invisible.
- Recon (Haiku, all PARTIAL): research/holstered_weapons/ux_v3/F2_H1_text_engine.txt (engine: text SetRect 0x417780 -> renderer +0x74(w,h) -> Relayout 0x417800; Draw 0x47B500 centre branch drops a single centred line when box B < line h, V3.md:101-109; button SetRect 0x419ED0 forwarding to text is a doc claim only, 08_shield_bar.md:500; button text sub-object at +0x160, GuiButton.h), F2_H2_modconfig_buttons.txt (mod-config has no button-only rect/text logic; buttons and labels share GuiKit setRect), F2_H3_screens.txt (measured table: caption glyph ~14 rows in both builds, no real squash; offsets), F2_H4_history.txt (823d1e21 raised rows 22/24 -> 34; vanilla BTN_EXIT 28 virtual units = ~75 px at 3840x1600; no earlier caption issue recorded).
- Leading hypothesis (unverified): the button's caption text object keeps a stale/other rect or box height after SetRect (or 0x419ED0 does not forward), so the centred line is offset or dropped. Both 22 px and vanilla ~75 px buttons render; 34 px does not.
- Next: plan holster_hw_ux3_captions.md (Haiku RE + live probe log of extent vs text-object rect/box/scale/align, then Sonnet fix: re-sync the caption rect/alignment after placeBox, or button heights the engine handles). Do not touch mod-config (user: other mod pages and combat config are correct).

## 10-18 16:30 caption fix LIVE (plan .claude/plans/holster_hw_ux3_captions.md)
- Cause (static RE, F3_R1 + F3_R4): CSWGuiButton::SetExtent 0x419ED0 gives the caption text btn+0x160 intersect(inner(border), inner(hilight)); inner = rect inset by border dimension G=16 (BTN_GAMEPLAY: corner/edge 32x32, DIMENSION 16, INNEROFFSET 9, F3_R2). At H=34 the inner height collapses to 0 and is rebuilt as 2*T around the centre; line height h = 0.16*2.0*100 = 32 px at 3840, so B < h.
- Fix 3b9c239e: holster-editor 0.4.3-dev applyCaptions() sets the caption rect to engine x/w + button y/h (hse::captionRect) via the text object's own vtable slot 1; gates ButtonVtable + TextVtable; row reset icons skipped; re-apply cap 8; ini caption_fix (1) / probe_caption (1) with CAPTION before/after lines (cap 40).
- Offline mockup: scripts/holster_editor_mockup.py (+ tools/hse_layout_dump.cpp, test 9/9). fix 1: all text captions centred at 3840/3440/1920 (F3_V2). Engine model half-calibrated (F3_V1): reproduces the 4 absent groups (drop), not the 4 groups drawn +15..+24 below; calibrate from live CAPTION lines.
- Follow-ups (pre-existing text fit, F3_RV1/RV4/V2): two scope buttons both fit to "This..."; 6 captions "..." at 3840 (Upper back, This body, inspector values); reset squares 30 px < 32 line (art only, OK).
- LIVE 10-18 16:30 holster-editor 5f5897c0 (KPM c465912), rollback `kpm_hotswap.py --rollback holster-editor_swap_20261018_163010`. Play row CAP1.
- Warning: the working-tree PLAY_TEST_QUEUE.md is a stale foreign copy missing the UX3/UX3b/UX3c/UX3d/CAM2/LOG1/ICON1/HW7a/HW7c rows (HEAD has them + CAP1); whoever commits that file must merge, not overwrite.

## 10-18 17:45 0.4.4-dev LIVE (fill panes + captions fitted to engine text box)
- User feedback on 0.4.3: squished, not filling the space, captions truncated. Live CAPTION log: engine word-wraps text wider than btn w-14, so wrapped lines landed on row edges.
- 0.4.4: rows up to 52 px (bar 44), roomy form (bones 2 cols, scopes 1/row, mode bar 2x2, stances 3+2), inspector widens; button fit = w-20, label fit = w-8, text_k 2.65, selection marks `><` counted. Offline mockup at 3840x1600: 0 button wraps/cuts; only long help lines cut.
- Install: gate PASS after a race fixed by another session (test_compat_check temporarily rewrites K2Addr.h; two concurrent gates left it dirty). A later install by another session rewrote the live DLL (older mtime logic -> LIVE-OTHER), so the kpatch was rebuilt with create-patch and applied. Live 96176a50, rollback `python3 scripts/kpm_hotswap.py --rollback holster-editor_swap_20261018_174509`. KPM commit done.
- Play row LAY1 (CAP1 superseded). Follow-ups: text_k should scale with UI scale (over-cut at 1920x1080); 3440 cuts "Upper b...", "Reve...", "30.5 d..."; mockup does not reproduce Upper back wrap or the +15..+24 drawn offset; working-tree PLAY_TEST_QUEUE.md is a stale foreign copy.
