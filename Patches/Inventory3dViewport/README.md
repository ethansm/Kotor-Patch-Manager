# Inventory 3D Viewport

> **STATUS** Track: SHIPPABLE. Version 0.3.0-dev (matches manifest.toml).
> **What it does / explores:** Shows a live 3D model of the inventory item under the cursor in the ultrawide margin of the in-game inventory screen, with orbit, roll and zoom controls.
> **Requires:** Steam/Aspyr KOTOR II (hash in manifest.toml `supported_versions`); the `gui-input` provider (`k2.guiinput@1`); the ultrawide GUI layout (the viewport lives in the margin); Patch Manager (KPM).
> **Known limits:** Needs free screen space to the right of the inventory list (the ultrawide margin). Model framing is per base-item category (`ItemFitTable.inc`); unlisted items use the generic fit. Per-tick detail is off by default. **Testing:** live-accepted 2026-09-29/30.
> **Install:** Install through KPM together with the `gui-input` provider (required by manifest.toml). Do not install on other builds; the hook addresses only match the Steam/Aspyr hash above.

## Features
- Live `CONTROL_3DVIEW` injected into the inventory screen (three hooks: ctor bind, item select, per-frame tick).
- Quaternion orbit camera with yaw/pitch inertia; idle auto-spin resumes after drag (`spin_deg_s`, `idle_resume_s`).
- Per-category auto-fit (`fit_margin`, `armor_box`, `fill_ref`, `fill_exp`, `fill_min`).
- Mouse wheel zoom, only over the 3D view while the inventory ticks.

## Controls
- **Left mouse drag:** orbit (yaw and pitch).
- **Shift + left mouse drag:** roll (left/right).
- **Right mouse drag:** zoom (up/down).
- **Middle mouse click:** reset view (yaw, pitch, roll, zoom).
- **Mouse wheel:** zoom in/out (disable with `wheel_zoom = 0`).

## Configuration (`$GAME/inv3d.ini`)
Missing or garbage file means defaults. Every key and every rejected line is logged.

| Key | Default | Meaning |
|---|---|---|
| `enabled` | 1 | Turn the viewport on or off |
| `wheel_step` | 0.9 | Zoom factor per wheel notch (clamped if out of range) |
| `wheel_zoom` | 1 | Mouse wheel zoom on or off |
| `zoom_min` / `zoom_max` | 0.3 / 3.0 | Zoom limits |
| `screen_w` / `screen_h` | engine framebuffer | Override the screen size used for hit-testing (clamped to 3840x1600 if invalid) |
| `fit_margin` | 1.15 | Framing margin around the model (clamped if out of range) |
| `drag_deg_per_px` / `roll_deg_per_px` | 0.5 / 0.5 | Mouse sensitivity |
| `spin_deg_s` / `idle_resume_s` | 70 / 1.5 | Idle spin speed and resume delay |
| `only_weapons_armor` | 0 | Show only weapons and armor |

Other keys (`rig`, `fov_deg`, `fov_axis`, `dist_min`, `dist_max`, `armor_spin`, `hit_flip_y`, `bevels`, `zoom_drag`, `fill_*`, `itype.<n>`, `row.<n>`) are documented in `Inv3dLogic.h`.

## Logging
- Patch log: `inventory-3d-viewport_log.txt` in the game folder (KpLog, `KP_LOG_PATCH("inventory-3d-viewport")`).
- Per-tick detail: create an empty `inv3d_debug.txt` in the game folder, then delete it when done.

## Hooks
Steam/Aspyr addresses in `kotor2-steam-aspyr.hooks.toml`: `0x008A69AD` (BindInventoryView), `0x008A8172` (ShowItem), `0x008A7350` (InventoryTick).

## Build
- From this folder run `python3 ../create-patch.py`. Host unit tests live in the author's development workspace and are not shipped here.

## Known issues
- The `research/` folder holds raw Ghidra logs with local paths. Keep it out of the pushed tree.
- Test gaps: `fitScale` with `d0 <= 0`, lone-CR and CR-then-EOF line reads in `drainLongLine`.

## Changelog
No CHANGELOG.md.
