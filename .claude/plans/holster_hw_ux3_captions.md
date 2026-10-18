# Implementation Plan: HW-UX3 captions — button text visible at any row height (probe + fix + offline mockup), then install for play test

Status: INSTALLED 10-18 16:30 (3b9c239e; holster-editor 0.4.3-dev LIVE 5f5897c0; play row CAP1). Parent plan `.claude/plans/holster_hw_ux3_build.md`. Handoff `.claude/handoffs/holster_hw_ux3_build.handoff.md` (section "10-18 16:10 button-caption debug" holds the evidence). Resume: `python3 scripts/plan_manager.py resume .claude/plans/holster_hw_ux3_captions.md`.

## 0. Preserved Context & Anti-Patterns
- User direction: Haiku agents in parallel for investigation, research, testing and validation; escalate findings to Sonnet; Sonnet for simple implementations, reviewed by Opus. Render the menu offline into an image for analysis BEFORE installing. Install at the end is pre-authorised (game must be closed).
- Shared worktree: commit only explicit paths; PLAY_TEST_QUEUE.md rows via index-blob trick.
- Live state at start: holster 0.2.0-dev, holster-editor 0.4.2-dev (823d1e21), mod-config 0.1.3-dev. Rollback editor: `python3 scripts/kpm_hotswap.py --rollback holster-editor_swap_20261018_152410`.
- Do not touch mod-config.
- No new raw addresses: call the caption text object's own vtable slot 1 via `guikit::setRect`.

## 1. Context
3840x1600, 0.4.2-dev: at row height 34 every BUTTON caption is absent or drawn on the box seam / below the box (+15..+24 units). LABELS render correctly. At 22-unit rows (0.4.1) captions sat inside the box. Text width is right.

## 2. Verified engine facts
- Button vftable 0x987A1C slot 1 = 0x419ED0 `CSWGuiButton::SetExtent`: writes ctl+4..+0x10; vt[1] on border +0x70 and hilight +0xE8 with the raw rect; caption text object +0x160 gets `FUN_0040e360(0x414d80(this+0x70), 0x414d80(this+0xe8))` (intersection of border-derived inner rects), not the raw rect.
- Text vtable 0x9876B4 slot 1 = 0x417780: stores rect at text+4..+0x10, string vt+0x64(x,y), vt+0x74(w,h), Relayout 0x417800.
- Draw 0x47B500: line height `h = font+4 * scale(+0x54) * 100`; centre align offset `(B - n*h)/(2D)`; a centred single line is dropped when B < h; top align never drops.
- Labels resized with the same SetRect render centred at 34, so `h <= 34` for a full-height box.
- GuiKit text helpers are label-only; button helpers in `_shared_src/GuiButton.h` (text +0x160, string +0x178). `guikit::setRect` calls slot 1 of the given object.

Hypothesis: 0x414d80 insets the caption rect by border edge/corner sizes; at 34 px the inset rect is shifted/short so the centred caption moves or drops. Fix: after each button SetRect, set the caption rect ourselves (engine x/w, button y/h), gated on `*(btn+0x160) == TextVtable`.

## 3. Design decisions
- D1 probe (ini `probe_caption=1`): per layout change per button (cap 40 lines) log `CAPTION i= btn=(x,y,w,h) text=(x,y,w,h) box=(w,h) scale= align= lines=`, before and after the fix write.
- D2 fix (ini `caption_fix=1`; 0 = engine default): in applyLayout, for changed button boxes, write `guikit::setRect(textObj, tx, by, tw, bh)`; fallback x+4/w-8 when the engine rect is zero; double vtable gate; fail closed with one WARN; labels untouched; re-apply on engine reset (cap 8).
- D3 row heights stay 34; follow-up raises button rows only if probe shows h > 34.
- D4 offline mockup `scripts/holster_editor_mockup.py` from a host JSON layout dump of the real `HseLayoutV3.h`; modes `--engine-model` / `--ideal`, `--caption-fix 0|1`. Calibration: engine-model fix 0 must reproduce the 0.4.2 screenshot symptoms (`research/holstered_weapons/ux_v3/F2_H3_screens.txt`).
- Version holster-editor 0.4.3-dev.

## 4. Waves
- W0 plan copy + snapshot.
- W1 Haiku recon R1 (0x414d80 formula + pointer chain), R2 (button border images/sizes), R3 (mockup reuse map), R4 (font line height h). Outputs `research/holstered_weapons/ux_v3/F3_R*.txt`.
- W2 Sonnet S1 mockup (+ host dump tool, tests), S2 DLL probe + fix (HolsterEditor.cpp, HseView.h captionRect + tests, manifest, ini).
- W3a Haiku render + image review RV1-RV4; W3 Haiku validation V1 calibration, V2 fix check, V3 gates.
- W4 commit, PLAY_TEST_QUEUE row CAP1, hot-swap install (gate PASS), handoff + memory.

## 5. Edge cases
Zero engine rect -> fallback; parked (-9999) -> skip; vtable mismatch -> no write + WARN; engine re-SetExtent -> compare current text rect with written, re-apply (cap 8); reset icons with empty caption excluded.

## 6. Verification
- [ ] Mockup V1 reproduces 0.4.2 symptoms (or noted uncalibrated); V2 all captions centred
- [ ] scoped run_tests + `-k holster_` pass; check_raw_addresses FAIL=0; build_check PASS
- [ ] Gate PASS; kpm_status LIVE holster-editor 0.4.3-dev
- [ ] After play: screenshot + CAPTION log lines confirm before/after rects
