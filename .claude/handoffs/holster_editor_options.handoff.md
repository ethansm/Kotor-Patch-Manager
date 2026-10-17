# Session Handoff & Plan State: .claude/plans/holster_editor_options.md

## 0. Next-Phase Direction
D3c (holster-editor 0.2.4-dev + holster 0.1.2-dev) both hot-swapped LIVE!
- `holster-editor 0.2.4-dev` LIVE md5 `36fbb1dd` (rollback `kpm_hotswap.py --rollback holster-editor_swap_20261016_235216`).
- `holster 0.1.2-dev` LIVE md5 `73a525b0` (rollback `kpm_hotswap.py --rollback holster_swap_20261017_001501`).
- Left-hand weapons are now fully wired: `kWorldKnowsHand = true` and in-world `handLookup` active, with automatic sagittal mirroring and independent `place_*_l_*` support.
- Next: Launch game and playtest row `D3C` (both right and left weapons).

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/holster_editor_options.md`
- **Branch:** `KOTOR-II` (04f32af holster 0.1.2-dev: enable kWorldKnowsHand and in-world left-hand weapon mapping)
- **Last Landed Commits:**
```
04f32af holster 0.1.2-dev: enable kWorldKnowsHand and in-world left-hand weapon mapping
75a9f23 docs: add D3C play-test queue row for holster-editor 0.2.4-dev
7c94d35 holster-editor 0.2.4-dev: weapon edit mode, bone cycling, scopes, and mouse nudging (D3c)
```
- **Dirty Files (166):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ... +156 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/holster_editor_options.md`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
- Session 10-16 (pivot session) state:
  - Commits: 03aa85d (HE0 probe + HE1 units), 7b388aa (HW-C6 row), 4c2d7f3/85845ae (R1/R2, UX study), 7322abb/7bd5624/153fbe2/21d9be1 (plan v2 + D2 recon), 57374b9 (EditKit.h + HseLayout.h split, D2f). KPM: 24ef666 (HolsterApi.h, GearIconTex.inc), 53e5d2a (holster kpatch mirror), 9379fb0 (EditKit.h).
  - HE0: holster kpatch 19a54e80 prepared + re-gated (SWAP); `kpm_hotswap.py --mod holster --apply` NOT run, awaits user GO with the game closed.
  - Research notes: .claude/research/holster_edit/{r1,r2,ux_a..ux_d,UX_OPTIONS,d2b,d2c,d2d,d2e,d2f}*.md. D2f: memory GFF via engine Malloc 0x712F30 + CResGFF parse-in-place (vtbl+0x10, OnResourceServiced 0x717300), freed by 0x7120C0; no resource registration API.
  - Dead end: the equip-screen editor (v1 HE3/HE4/HE5) is ABANDONED by user decision; title-screen Options rejected (no party bodies).
  - Dead end: typed numbers (game also reads keyboard via DirectInput, r1).
  - Dead end: intercepting Options Back/EXIT/Esc (no whole-function skip in the detour framework); Esc closes the menu, overlay kept.
  - Trap: `git add -A` with deletions was permission-denied; stage deletions with `git rm --cached <paths>` + explicit `git add`.
  - Trap: hook 0x8A1704 is a rel32 CALL (rejected); use 0x8A16FE. Draw 0x40EC90 is hot and shared by all panels: filter on vtable 0x9A701C first.
  - Verify: `python3 scripts/run_tests.py holster editkit hse_layout equip3d`; `python3 scripts/check_shared_headers.py`; `scripts/pre_install_gate.sh --mod holster_src --skip-default`.
- Session 10-16 (execution session, plan "HE0 -> HE2 -> D1b -> D3a") state:
  - HE0 DONE: holster LIVE 819e4462 (hot-swap 15:54, rollback `kpm_hotswap.py --rollback holster_swap_20261016_155408`). HW-C6 play row pending (user plays; then read PREVIEWNODE/PREVIEW/CAMNODE via `kotor2_logs.py check holster_log.txt --new`; PREVIEWNODE n=0 -> fallback F1 ComposePose card before D3b).
  - Wave A (Haiku x6) landed: c07 7058df9 (gen_hse_layout), c04 163568c (Preview3dApi.h + Preview3dLogic.h), c01 806b4c2 (boneName + scopeKeys), c02 30963db (HolsterPreview.h), c08 0ce2363 (EmbedGui.h; resman/CResGFF names moved into GameAddr.h). KPM Common: ed9ba5d, 19b56b7 (+ the final sync commit of this session).
  - Wave B: c03 b8dcc3f (Holster.cpp provides k2.holster@1: preview registry, overlay, Save via setKeys/removeKeys + atomicWrite + shared parseTuneText, in-world resolveFull cached per tune seq, editor= key, chEdit cap 200). c05+c06 f16d966 (preview-3d provider; Opus fix: BeforeFree fires BEFORE wpn[i] is cleared). 6c3e35d (HSE_VIEW ID -1). c09+c10 + ViewControl slot: committed at session end (see git log).
  - HE2: holster kpatch 071ec9db prepared (--commit-kpm 2f38603) + re-gated GATE PASS; verdict SWAP; `kpm_hotswap.py --mod holster --apply` awaits user GO (game closed). Play row HE2.
  - D3a: preview-3d + holster-editor GATE PASS (md5 f9dac5d7 / 012d5d97); NOT-LIVE. Next: `kpm_hotswap.py --mod preview-3d holster-editor --prepare --commit-kpm`, re-gate, `kpm_reinstall.py --mod preview-3d holster-editor --dry-run`, then real run on GO. Play row D3a.
  - REVIEW: an independent Opus 5.5 reviewer was launched on this session's commits (range 87194ab..HEAD, paths holster_src, holster_editor_src, preview_3d_src, _shared_src/{EmbedGui,Preview3dApi,HolsterApi,GameAddr}.h, scripts/gen_hse_layout.py); output `.claude/reviews/holster_editor_d1b_d3a_review.md`. Fix all valid findings, re-gate, then continue the install steps above.
  - Trap: kpm_status LIVE-OTHER (live DLL mtime newer than kpatch) refuses a swap; rebuild the kpatch (create-patch via kpm_hotswap.run_create_patch, or a MIRROR --prepare) and re-gate.
  - Trap: a wave worker editing holster_src makes kpm_status DIRTY: apply hot-swaps BEFORE launching workers on that mod.
  - Trap: sceneview::createView uses InitControl addToList=1: the embedded control ID must be -1 (else it overwrites a vanilla list slot); the editor appends the view itself via preview-3d ViewControl(token).
  - Trap: check_shared_headers.py --sync also pushed other sessions' committed-but-unsynced headers (CodePatch, EmbedTex, GuiKit, HudKit, SafeRead, SelfHealUi, WindowHook) into the KPM working tree; left uncommitted in KPM (not ours).
  - Trap: build_check compiles against KPM Patches/Common: sync headers before building a consumer of a changed _shared_src header.
  - Verify: `python3 scripts/run_tests.py preview3d preview_3d embedgui holster hse_layout gen_hse_layout editkit equip3d`; `check_provider_manifests.py` FAIL=0; `check_raw_addresses.py` FAIL=0; `scripts/pre_install_gate.sh --mod holster_src --mod preview_3d_src --mod holster_editor_src --skip-default`.
- Review DONE 10-16: Opus review `.claude/reviews/holster_editor_d1b_d3a_review.md` (0 BLOCKING, F1 MAJOR + F2-F7 MINOR); all 7 fixed in a8f9f36 (KPM Common 5f9f1fc). F1: kWorldKnowsHand=false in Holster.cpp, left preview resolved/saved as right (open: record the hand per weapon Gob at attach, then flip it). F3: Options-dtor leak of our view + 4 labels is DELIBERATE (open D3b item: check panel hover/focus pointers, then remove our list tail + vtable-guarded free + preview-3d Release(token)).
  - Re-gated after fixes: GATE PASS holster + preview-3d + holster-editor (head e0418e0+fixes), 17/17 suites, providers/raw/hooks FAIL=0. Holster re-prepared (KPM 3fe0406), verdict SWAP, re-gated.
  - Next: user GO (game closed) -> `kpm_hotswap.py --mod holster --apply` (HE2); then `kpm_hotswap.py --mod preview-3d holster-editor --prepare --commit-kpm`, re-gate, `kpm_reinstall.py --mod preview-3d holster-editor --dry-run`, real run on GO; play rows HE2 + D3a (+ HW-C6 still pending).
- HE2 LIVE 10-16 16:48: holster 9196b748 hot-swapped (rollback `kpm_hotswap.py --rollback holster_swap_20261016_164843`); queue 125dce5.
  - Dead end: `kpm_hotswap.py --prepare` REFUSES NOT-LIVE ids and `kpm_reinstall.py` refuses ids with no kpatch. For a new id call `kpm_hotswap.prepare(pid, row|verdict=MIRROR, src, KPM, tag)` + `commit_kpm` from Python (scripts/ cwd).
  - Trap: create-patch.py needs a `*hooks.toml` even for a hookless provider: preview-3d got a metadata-only `kotor2-steam-aspyr.hooks.toml` (`hooks = []`, like gui-input), 7f3eb36.
  - D3a kpatches built: KPM 157eaca (Preview3D) + e09a9e3 (HolsterEditor); GATE PASS head 7f3eb36; `kpm_reinstall.py --mod preview-3d holster-editor --dry-run` clean (48 ids, no peer restore); queue 1c8dd34.
  - Next: user GO (game closed) -> `python3 scripts/kpm_reinstall.py --mod preview-3d holster-editor`; then play rows HW-C6, HE2, D3a.
- D3a LIVE 10-16 16:57: holster-editor 6368a82a + preview-3d 7384ae1a (48 patches applied, 0 peer restores; rollback `kpm_reinstall.py --rollback batch_reinstall_20261016_165749`). Next: play rows HW-C6, HE2, D3a; then `kotor2_logs.py check <log> --new` and D3b cards.
- PLAYED 10-16 17:30 (first D3a session). D3a PASS: EMBED open/parse/swap ok, view token=1 id=-1, restore ok, append view/btn/back/header/status ok (ids 16-20), button visible, 2x enter/leave restored=16 of 16, Esc then DTOR with no crash, preview-3d CreateView ready + Destroy done. HE0/HW-C6: PREVIEWNODE n=5 (no ComposePose fallback needed), camhook non-zero on all 3 parents. HE2 boot: `TUNE boot ... bootbone=3 editor=1`; live edit-reload (RELOAD) NOT exercised yet.
  - Needed fix (D3b): header + status labels are placed at modalLayout rects INSIDE the description pane, under the 3D view, so editor mode shows a blank title bar. Move the header to the vanilla LBL_TITLE rect (listed ctl) and the status below the view.
  - kplog.ini needs a section per new patch: added [holster-editor] + [preview-3d] (level=debug); without it no log is written.
  - Recon: r1 hand per weapon -> NEEDS-LIVE-PROBE (hook 0x763B4C rhand / 0x763E7E lhand in FUN_007637E0); r2 Options dtor cleanup -> labels SAFE-RECIPE, view free needs probe (hover mgr+0x08, count P+0x28, list P+0x24); r3 input -> GAPS small (no held/move events, no point-in-box, thumb inset 5px, no camera basis, wheel unwired). Reports in .claude/research/holster_edit/r1..r3.
- USER FEEDBACK 10-16 (after D3a play), DESIGN CHANGE for D3b:
  1. The 'Holstered weapons' button is a plain cloned label: needs the real button look (colours, outline, hover state).
  2. On hover, the right-hand pane (LB_DESC) must show a text description of the entry.
  3. Placement: NOT last in the menu. Nest it under a new 'Mod Configuration' Options entry placed directly UNDER Gameplay (Feedback/Auto-Pause/Graphics/Sound/Exit shift down one slot). Holstered weapons becomes an entry on that sub-page.
  4. 'Mod Configuration' becomes its own shared provider patch (working name mod-config, k2.modconfig@1): other mods register entries (name, description, enter/leave callbacks); holster-editor becomes a client.
  Recon launched: r4 button style + hover (.claude/research/holster_edit/r4_button_style_hover.md), r5 reflow + provider sketch (r5_modconfig_provider.md). r2 already found hover pointer = [P+0x1C]+0x08 (poll per Draw tick to drive the description).
  New order: D3a2 (mod-config provider + styled entry button + hover description + header fix) BEFORE D3b viewport. Cards after r4/r5 land.
- USER DIRECTION 10-16: put much of the menu code in a SHARED DLL provider so other mods can build menu screens from shared components that keep vanilla styling (styled button with hover + LB_DESC description, header, Back, status, list rows, slider/value rows, toggle, 3D view slot, sub-page enter/leave with automatic vanilla hide/restore). Supersedes the narrower mod-config idea: provider scope = menu UI toolkit + Mod Configuration page registry. r4/r5 were told to cover it; r6 (hand hook), r7 (free safety), r8 (D3b call sequence) running. Planning starts when all of r4-r8 are in.
- RECON r4-r8 DONE 10-16 (reports .claude/research/holster_edit/r4..r8):
  - r4 CLONE-BUTTON: vanilla option buttons are CSWGuiButton (vt 0x987A1C, ctor 0x41A510, 0x1D0 bytes, inline panel+0x68+k*0x1D0; borders +0x70 normal/+0xE8 highlight; text +0x160). Hover desc is NOT mgr+0x08: event 0 -> 0x8A2210 (k -> TLK strref) -> 0x8A2320 writes label panel+0x1030; a mod button gets no description, so the provider writes LB_DESC itself. Clonable too: slider 0x41AB60, toggle 0x41A330, listbox 0x41CE90. Gate with one live probe (vtable, size, event 0).
  - r5 GAPS: one DLL mod-config, two interfaces k2.menuui@1 (components + page enter/leave) + k2.modconfig@1 (registry: K2ModConfigEntry{struct_size,name,desc,enter,leave,tick}). Mod Configuration = clone BTN_GAMEPLAY at y 242, FEEDBACK..QUIT shift +36 each; rects set once per panel so Layout must re-apply each tick; mod-config must own hooks 0x8A16FE/0x40EC90/0x8A1930 and holster-editor drops them; new id = full kpm_reinstall.
  - r6 NEEDS-PROBE-BUILD: hand hooks 0x763B49 (right) / 0x763E7B (left), stolen bytes 8B 42 7C FF D0, resume 0x763B4E/0x763E80; at site [ESP+8]=flag (1 right), [EBP-0x2C]=weapon Gob, W=item+0x14. vtbl+0x7C is a RET stub (r1 wrong). Check hooks against live build 6a522e71 vs old 7751f7d8 cites; confirm Gob == HolsterPlace Gob live.
  - r7: labels SAFE-RECIPE (at OptDestroy: focus P+0x20, hover M+0x08 via 0x413C50 writer, pressed M+0x10, M+0x14, then P+0x28=count, vt[0](1) only if vtable 0x9878BC); 3D view NEEDS-PROBE (Scene* view+0x74 dtor unidentified, double-free risk).
  - r8 GAPS: Bind(party::member(party::table(),N)) -> Rebuild -> SetWeapons(token,1) -> RegisterPreview(WeaponGob(token,h),NULL,h==LEFT) -> BeginEdit a frame later; SetView(token,yaw,zoom,panZ). Gaps: bind+register (G1), framing (G2), party picker + names (G3, no accessor; 0x5632F0 unverified), left-hand decision (G4).
  - NEXT: user go to plan next round (D3a2 mod-config/menuui provider + probes, then D3b).

## Session context 10-16 (round 3 plan)
- D3a play result (10-16): button visible, enter/leave restore OK, EMBED/INPUTCLASS/DTOR lines clean, no crash. Two faults on the user's screenshots: (1) the "Holstered weapons" label does not look like a vanilla button (no outline, colours, hover); (2) the editor title bar is blank: header and status labels sit inside the LB_DESC pane (hse::modalLayout on the desc box, HolsterEditor.cpp:169-175) and the 3D view drawn on the same rect covers them. Fix = header on the LBL_TITLE rect, status in a strip under the view.
- User feedback verbatim: "options button isn't properly styled, colors, outline, hover state. On hover the menu on the right side should have a text description of what it is. This should be nested in the options menu under Gameplay or Graphics or a new Mod Configuration option that this would sit underneath. Mod Configuration menu could be its own provider dll patch for shared leverage. We wouldn't want it last in the menu list, instead move it up under the Gameplay option." and "we should put a lot of this into a shared DLL provider for other mods to create menu screens with shared components that maintain vanilla styling".
- Design decisions:
  - Menu nesting: Options -> Mod Configuration (under Gameplay) -> list -> mod page. The list page exists even with one mod.
  - Pages live inside the Options panel (hide/restore vanilla controls), not in a new engine panel.
  - On mod pages BTN_EXIT and LBL_TITLE are hidden; our Back = BTN_EXIT clone on its rect, title = LBL_TITLE clone. Esc still closes the whole GUI.
  - Hover is detected by polling the cursor each tick (the M+0x08 writer runs on clicks only). Event 0 / 0x8A2210 not used.
  - Button classes are cloned in `_shared_src/GuiButton.h`, not GuiKit.h.
  - mod-config v1 = buttons, labels, custom controls, pages only (sliders/toggles in D3c).
  - Probes ship first (Install 1).
- Agent routing: Haiku 5.5 for recon, boilerplate, pure headers and docs; Sonnet 5.5 for engine-glue cards (c13, c14, c18, c19, c21) and failed Haiku retries; orchestrator only reviews on escalation.
- Repo state at plan start: head a154fa2; live holster 9196b748, holster-editor 6368a82a, preview-3d 7384ae1a; kplog.ini has [holster-editor] and [preview-3d] (backup kplog.ini.bak_hse_20261016); mod-config needs a [mod-config] kplog.ini section at its install.
- Resume: `python3 scripts/plan_manager.py resume .claude/plans/holster_editor_options.md`.

## Session context 10-16 evening (PRB1 played, W2 c15-c17)
- **PRB1 PASS** (8c4abc9; reports `.claude/research/holster_edit/prb1_{editor_log,hand_log,sweep}.md`): user saw styling, hover highlight, description and hover sound all correct. Logs: BTNCLONE ok canary=1 x2, HOVER hl=1 on 7/7 ours=1 hovers (engine highlights a listed mod button itself: no manual vt[0x0C] needed), DESC ok via=8A2320 7/7, REFLOW on moved=5, RELAYOUT 0, EDITOR enter focusCleared=1, leave restored=16 of 16, no WARN/ERR, no crash, preview-3d token create/destroy balanced. Hand probe: 11 HANDATTACH / 11 HANDMAP (R7/L4), each gob one hand, names match; L gobs logged `nottarget` (lhand target=0) so off-hand placement needs kWorldKnowsHand / c22. free_view=1 run NOT done.
- Open from PRB1: HOVER line showed hl=1 with m08=0 twice (pointer over no control?); mouse-probe has no screen tag.
- **User design decision (10-16, after PRB1), verbatim:** "Instead of adding our holster config to the root of the menu under gameplay, could we add a 'Mod Configuration' button, with basic description. Clicking it opens another menu of the same layout, buttons on left description on right when hovered. The only item would be our holster config to start. This new mod config screen would hold the config buttons for my patches and is where they would get merged together by the provider DLL". This is the plan's MAIN -> LIST -> ENTRY model; the LIST page must use the vanilla column rects (left) + LB_DESC hover description (right), and the root button needs its own description text.
- **W2 progress:** c15 ModConfigApi.h + contract test (49 checks) and c16 MenuUiLogic.h + tests (241 checks) committed 487d7f7; KPM Common ModConfigApi.h synced + committed in KPM 9ac4fd9. c17 scaffold (manifest/exports.def/mod_config.ini/hooks.toml, hooks McBind/McTick/McDestroy = OptBindView/OptTick/OptDestroy addresses) is on disk UNCOMMITTED by design: it lands in ONE commit with c18+c19 (hook move). check_provider_manifests currently FAIL=2 = missing provide() calls (c18 adds them); the duplicate-hook FAIL appears once holster-editor requires mod-config, so c19 must drop its hooks in the same commit.
- **User rule (10-16 evening):** after investigation, persist context, then plan mode for a detailed next-phase plan; Haiku 5.5 swarms for research in planning and in the plan; Sonnet 5.5 for the code cards so the Opus orchestrator keeps a minimal context.
- Next: plan mode -> plan for c18 (mod-config engine glue) + c19 (holster-editor 0.2.0-dev port) + Install 2 + MC1, then W3.
- **W3 (D3b) DONE 10-16**:
  - Wave 1 landed: c20 `HseView.h` + `hse_view_test.cpp` (view maths, rect helpers, labels); c21a `HseParty.h` (saferead memberCount/memberAt/memberName); c23 housekeeping (version bump 0.2.1-dev, `common_contract_test.cpp`); R13a (`r13a_menu_attach_refresh.md` confirms 504ef5 runs per frame for preview-3d weapons); R13c (`r13c_c22_left_hand.md` confirms c22 ready for build).
  - Wave 2+3 landed: c21b + c21c in `HolsterEditor.cpp` (re-reads ini at every build, creates 8 buttons + name label, layouts strip, drives Bind/Rebuild/SetWeapons/RegisterPreview/SetView/BeforeFree lifecycle, button clicks, mouse spin drag).
  - Tests: `run_tests.py hse_view hse_layout holster_editor` 3 passed; `check_raw_addresses.py` FAIL=0; `check_provider_manifests.py` FAIL=0; `check_shared_headers.py` 43 headers 0 drifted; `compat_check.py` 6 passed.
  - Gate: full `pre_install_gate.sh --mod holster_editor_src` GATE PASS (331 suites pass); `kpm_hotswap.py --mod holster-editor --prepare` rebuilt KPM side (committed c2a22c7); verdict SWAP.
- **W4a (D3b2) DONE 10-16**:
  - Research subagents explored equip-screen controls, animations, mouse input, and weapon mode design (.claude/brain/.../w4_mouse_anim_plan.md).
  - Created `_shared_src/ViewDrag.h` (pure camera maths & drag state machine) + `view_drag_test.cpp` (PASS).
  - Updated `_shared_src/GuiKit.h` with `mbuttonDown`, `shiftDown`, `ctrlDown` wrappers; synced all 44 shared headers with KPM.
  - Updated `preview_3d_src/Preview3dProvider.cpp`: `chooseIdle` explicitly plays in-game `pause1` on body and head, eliminating posed hands; `PlayAnim` with empty or `"idle"` returns to `pause1`.
  - Updated `holster_editor_src/HolsterEditor.cpp` (0.2.2-dev): removed 6 obsolete camera buttons, installed `wheelInstall` for wheel zoom, added LMB drag with 4px axis-lock (yaw / panZ), MMB click to reset view, RMB live probe, animation picker (`<`, anim label, `>`) with `Play` and `Idle` buttons, and automatic return to `pause1` idle after timed emotes.
  - Verification: `compat_check.py` PASS 10/10; full `pre_install_gate.sh --mod holster_editor_src --mod preview_3d_src` GATE PASS (332 suites pass); `kpm_status.py` confirms SWAP on both mods.
  - Play row D3B updated in `patch_manager_mods/PLAY_TEST_QUEUE.md`. Next: await user GO (game closed) to run `--apply`.

## 10-17 FW-H1 (fix wave on review 537af56)
- 3 Sonnet workers (disjoint scopes), all PASS; combined gates: run_tests 46/46 (holster hse_view view_drag common_contract holster_editor preview3d_logic preview_3d mod_config), build_check x3 FAIL=0, check_raw_addresses FAIL=0, check_provider_manifests FAIL=0 (1 WARN npc-diversity, not ours), check_shared_headers 0 drifted, compat_check 20/20.
- Repo commits: holster 0.1.3-dev 68284d1, preview-3d 0.1.1-dev a9504b5 (incl. ViewDrag.h/Preview3dApi.h, view_drag_test), holster-editor 0.2.5-dev d0110c8. KPM: Common d0b777a + 977bae9 (ViewDrag.h was never committed in KPM before), mirrors 03c5912/239f831/70a529a.
- `kpm_hotswap.py --prepare` -> SWAP x3. Full pre_install_gate: see Saved State of the session reply. `--apply` awaits user GO (game closed); rollback `--rollback latest`.
- Decisions applied: two-click discard confirm (2 s) on scope/hand/Done/prev/next/Reset scope; Back/Esc/close discard + `DISCARD why=`; bone_test_left code default stays 0 and effLeft = left && g_boneLeft; preview Rebuild plays nothing (idleFor restored), editor plays kStanceList[stanceIdx].anim after Rebuild; g_debug removed.
- K2HolsterEditInfo pins (108/64/80/92/104) matched the compiler. RMB Ctrl snap left as round-to-5 deg (already accumulates).
- Play rows FW-H1 + HW-LH1 pending (D3B/D3C superseded). HW-LH1 needs the user's manual game-dir holster.ini edit (bone_test_left=1, gob_every=1). After pass: flip the bone_test_left default with a new GO.
- Holster banner still reads "(P0 probe, log-only)": scripts/test_holster_place_verdict.py + test_holster_log_check.py match that text; change both together later.
