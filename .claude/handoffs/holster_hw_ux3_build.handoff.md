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

## 10-18 18:41 0.4.5-dev LIVE (holster-editor 4f47ade3, repo 08e39444, KPM 9d20477)
User feedback on 0.4.4: big improvement; selection >marks< ugly; reset icons tiny (button border DIMENSION 16 insets the fill: 40 px square = 8 px arrow); no arrows on member buttons (caption lines=0, blank); buttons cramped; default zoom too far out; wants slider description boxes; asks about overlays.
Built: selected selector = hover border tint copied onto normal border (setSelFrame, falls back to >marks< if tints equal/refused; log `STYLE frame tint`); reset squares up to 60 px, inspector rows 60, white hover palette (gen_reset_icon.py); chevron arrow textures on member prev/next (ART log member_arrow=); kGap 4, kPad 6, ColWantW 600, InspWantW 1100; ini margin_px=120 widens panes into side margins (0 = off); view_zoom 0.95.
Not done: slider description boxes (green box = mod-config LB_DESC, not clonable after panel ctor; idea: bordered hint labels); mod-config tick-gap guard: a 1.1 s game hitch (T=607312..608554, cause unknown) while scrubbing Slide made mod-config `PAGE 3->0 (tick gap)` (kTickGapMs 500, ModConfig.cpp:1263) and DISCARD why=leave dropped unsaved edits; fix idea `gap > kTickGapMs && !(flags48 & 0x40)` needs user OK (do-not-touch-mod-config). 1280x720: left column goes compact, Save label tight (test relaxed).
Play row LAY2. Rollback: python3 scripts/kpm_hotswap.py --rollback holster-editor_swap_20261018_184125

## 10-18 19:37 0.4.6-dev LIVE (LAY3)
User play of 0.4.5: flip button blank (caption "<>" renders as nothing, same as the member "<" ">"), no box on the slider hint lines. Fix: hse_flip/_hi icon (8 textures now, embtex 11/16), hint lines get the slider frame art (hintPx = labelPx-16). Commits repo 4960fbb0, KPM ce7babd. Live 974e572c. Rollback `python3 scripts/kpm_hotswap.py --rollback holster-editor_swap_20261018_193656`. Play row LAY3 (LAY2 superseded). Observed: normal-state icons look dim (border tint multiplies the fill); mod-config tick-gap kick still open.

## 10-18 21:08 0.4.7-dev LIVE (LAY4)
User play of 0.4.6: slider art pixelated, hint frames wrong, marker not matching wedge. Fix: slider frame/wedge 512x64 + marker 32x128 (gen_slider_art.py K=4), wedge rect cut at thumb centre, new hse_desc_box texture (vanilla LB_DESC colours sampled from screenshot: fill 0,17,11 / edge 0,105,81 / text 37,200,164, font fnt_d16x16b). Slider hints now go to mod-config's own green box via SetDesc; left-column button tips are blank for mod-config and shown by the editor in a left description box (hint[0] box + hint[1] text, under the status line; left rows reserve kV3DescWantH=130, so left rows are 42 at 3840x1600, 52 before). Commits repo beb36dc6, KPM b968d71; live 88135190; rollback holster-editor_swap_20261018_210830; play row LAY4.
Open requests from the user (not started): (1) hotkey to toggle holster mode, ideally in the native Options key bindings: recon = UNKNOWN (needs rea-recon on Steam 0x900d40/0x900e30/0x9012b0/0x901310 for row source + key->command dispatch; prior notes .claude/research/holstered_weapons/hotkey_keymap_menu.md, hotkey_prior_art.md; fallback = own key via GetAsyncKeyState + holster.ini); (2) shorten the ~8 s holster-after-combat delay when no enemies are near (holster.ini line 28; engine combat-flag exit timer).

## 10-18 22:10 hotkey + holster-delay research (Wave 1 done, plan pending)
User asks: (1) native Key Mapping row "Toggle holster" (configurable in Options > Key Mapping), (2) shorten the ~8 s post-combat / post-flourish holster delay when no enemies are near.
Research (static, `research/holstered_weapons/keybind/`): K1 panel rows, K2 ini table, K3 dispatch, K4 keymap.2da, K5 combat exit timer, K6 flourish state, S1_reconcile (Sonnet, authoritative; corrects K2).
- Goal A verdict FEASIBLE with no engine patch (pending in-game unknowns): rows = hard-coded id range 200..269 (0x900D40) intersect keymap.2da rows (ReMappable/ForceDisplay, Disabled!=1). Ids 266, 267, 269 are free. Override/keymap.2da (81 rows, V2.b) + holster tick polls `0x72F9D0` (thiscall ECX=[0xA1B48C], id, slot 0, RET 8, float in ST0) with rising-edge toggle. Row needs EventType=1, ICPC flag, a default key unused in ICPC, a TLK strref for the label.
  Unknowns: Override honoured?; list shows the extra row?; custom label strref; free key code. Fallback: own key in holster.ini.
- Goal B: delay is server countdown creature+0x524 (set 8000 at 0x57A64F on combat entry, decremented in AIUpdate 0x569320, expiry -> SetCombatState(0) at 0x569923), no enemy check. Flourish 0x79F430 sets server +0x520=1/+0x524=8000 with param_3=2, never client +0x138, so holster never stows (cause of "carrying the weapon"). Recommended: holster tick writes +0x524 (0 for flourish-only: +0x520==1 && client +0x138==0 && +0x11E8==2; ~1500 for real combat with no hostile near). Hostile check: companion_ai FnNearestEnemy 0x57C990.
- Next Haiku Qs: H1 0x72B300/0x72B830 + free ICPC key codes; H2 ActionStrRef -> TLK text (0x901D70, 0x83EA60); H3 server-creature recipe + 0x57C990 sentinel/range. Plan to be written in plan mode.
