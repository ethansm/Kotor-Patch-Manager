# Holster Editor

> **STATUS** Track: SHIPPABLE. Version 0.7.3-dev (matches manifest.toml).
> **What it does / explores:** Adds the Holstered Weapons page to Options -> Mod Configuration: a 3D holster placement editor with stance bars, a Draw / Holster toggle and a Speed slider.
> **Requires:** holster (k2.holster@1), preview-3d (k2.preview3d@1), mod-config (k2.modconfig@1, k2.menuui@1); Steam/Aspyr build (kotor2_steam_aspyr); KPM Patch Manager.
> **Known limits:** Not standalone: the page cannot run without its three dependencies. Game-linked code (HolsterEditor.cpp) has no host unit tests and is covered by play rows only. **Play-test status:** pending rows UX6, MC-PF3, SM-L17 and SM-L18 in patch_manager_mods/PLAY_TEST_QUEUE.md.
> **Install:** Install with KPM after holster, preview-3d and mod-config; ships together with holster.

## Features
- Holstered Weapons page with a 3D preview of the whole character.
- Direct buttons for hand, bone, scope, stance and party member.
- Move / Depth / Tilt / Roll-Turn mode bar and an inspector of six sliders (Slide, Depth, Height, Tilt, Roll, Turn).
- Draw / Holster toggle: preview weapons slide to the hand and back.
- Speed slider (1x to 8x, whole steps) that writes `slow_holster_x` to the game-dir `holster.ini`.

## Configuration
- `holster_editor.ini` (read at every Options open). Each key is documented inline in the file: `enabled`, `view_yaw`, `view_zoom`, `margin_px`, `free_view`, `text_k`, `caption_fix`, and the `probe_*` keys.
- `slow_holster_x` in the game-dir `holster.ini` is written by the Speed slider and polled by holster 0.21.0-dev.

## Logging
- Log file: `holster-editor_log.txt` in the game dir.
- Probe lines: CAMENG / CAM2 (`probe_cam`), CAPTION (`probe_caption`), and one SPEED line per Speed change.

## Hooks
- None. `hooks = []` in `kotor2-steam-aspyr.hooks.toml`; the mod is hookless.

## Build and test
- `python3 scripts/run_tests.py holster_editor`
- `common_contract_test.cpp` pins the Common APIs the mod uses.

## Changelog
- See [CHANGELOG.md](CHANGELOG.md).
