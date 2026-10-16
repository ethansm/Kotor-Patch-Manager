# Session Handoff & Plan State: .claude/plans/holster_editor_options.md

## 0. Next-Phase Direction
Fix all valid findings from the Opus review (.claude/reviews/holster_editor_d1b_d3a_review.md), re-gate, then HE2 hot-swap + D3a reinstall on user GO; after the D3a play row, write D3b cards

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/holster_editor_options.md`
- **Branch:** `KOTOR-II` (c94b332 holster-editor D3a-T9/T10: scaffold (ctor 0x8A16FE [ebp-0xAC], Draw 0x40EC90 vtable-filtered, dtor 0x8A1930) + skeleton (embedded HSE_VIEW via EmbedGui, BTN_HOLSTER/Back/pane labels, editor mode with raw vanilla flags restore, EMBED/INPUTCLASS/HIDDENDRAW probes); preview-3d ViewControl slot; OptionsInGameVtable in GameAddr.h; play rows HE2 + D3a)
- **Generated:** 2026-10-16 16:16:38 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
c94b332 holster-editor D3a-T9/T10: scaffold (ctor 0x8A16FE [ebp-0xAC], Draw 0x40EC90 vtable-filtered, dtor 0x8A1930) + skeleton (embedded HSE_VIEW via EmbedGui, BTN_HOLSTER/Back/pane labels, editor mode with raw vanilla flags restore, EMBED/INPUTCLASS/HIDDENDRAW probes); preview-3d ViewControl slot; OptionsInGameVtable in GameAddr.h; play rows HE2 + D3a
 patch_manager_mods/PLAY_TEST_QUEUE.md              |   4 +-
 patch_manager_mods/_shared_src/GameAddr.h          |   2 +
 patch_manager_mods/_shared_src/Preview3dApi.h      |   8 +-
 .../holster_editor_src/HolsterEditor.cpp           | 341 +++++++++++++++++++++
 patch_manager_mods/holster_editor_src/exports.def  |   6 +
 .../holster_editor_src/holster_editor.ini          |   5 +
 .../kotor2-steam-aspyr.hooks.toml                  |  47 +++
 .../holster_editor_src/manifest.toml               |  17 +
 .../preview_3d_src/Preview3dProvider.cpp           |   9 +-
 9 files changed, 435 insertions(+), 4 deletions(-)
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
