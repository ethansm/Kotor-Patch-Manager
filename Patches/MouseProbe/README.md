# Mouse Probe

> **STATUS** Track: SHIPPABLE. Version 0.4.1 (matches manifest.toml).
> **What it does / explores:** Fixes Wine/Wayland mouse-camera problems in KOTOR II (sanitizes bad pitch/yaw deltas, synthesizes double-clicks Wine drops, optional edge warp during camera drag) and logs mouse/camera telemetry for diagnosis.
> **Requires:** Steam/Aspyr KOTOR II (target hash in manifest.toml `[patch.supported_versions]`); no other provider. Installed through KPM.
> **Known limits:** Hook addresses are not all confirmed in the Ghidra knowledge DB. Host tests cover the pure heuristics only; the engine call sites are verified in game only. **Testing:** not yet played in game.
> **Install:** Install `mouse-probe` through Patch Manager (KPM). Default settings are the fix on; set `enabled = 0` in `[mouse_fix]` to get the read-only 0.1.0 behaviour.

## Features

- **Delta sanitizer** (`sanitize_norm`): overwrites the normalised camera delta at `this+0x408/+0x40C` when it is out of range.
- **Double-click synth** (`synth_dblclk`): rewrites a dropped double-click message to `WM_LBUTTONDBLCLK`. Guarded by a writable check; skipped and logged when the target is not writable.
- **Edge warp** (`edge_warp`): while dragging the camera, moves the OS cursor back from the window edge with `SetCursorPos`.
- **Telemetry**: RAW, POLLS, NORM, DRAGSTATE, CAM and CLICK channels, with WARN heuristics (MISSEDDBL, DBLGUARD, STUCKDRAG).

## Configuration

File: `mouse_probe.ini` in the game folder.

| Section | Key | Default | Meaning |
|---|---|---|---|
| `[mouse_fix]` | `enabled` | 1 | Master switch for all fixes. 0 = read-only probe. |
| `[mouse_fix]` | `sanitize_norm` | 1 | Delta sanitizer on/off. |
| `[mouse_fix]` | `synth_dblclk` | 1 | Double-click synth on/off. |
| `[mouse_fix]` | `edge_warp` | 1 | Edge warp on/off. |
| `[mouse_fix]` | `sens_scale_pct` | 5 | Sensitivity scale in percent (0 or less keeps the default). |
| `[mouse_fix]` | `edge_margin` | 50 | Edge warp margin in pixels (0 or less keeps the default). |

Kill switch: delete or rename the flag file `mouseprobe.txt` (old core only) to stop the legacy fallback logging.

## Logging

- New core (kpatch.log service): enable channels in `kplog.ini` under `[mouse-probe]`. Lines go to `mouse-probe_log.txt`.
- Old core (no service): fallback sink writes `mouseprobe_log.txt`; the `mouseprobe.txt` flag file turns on the fallback level.

## Hooks

Nine detours, all in `kotor2-steam-aspyr.hooks.toml`: ProbeMouseRaw `0x0072F14E`, ProbeMouseNorm `0x0072F2EC`, ProbeCameraFrame `0x0078C452`, ProbeMouseMsg `0x0072E039`, ProbeLButtonDown `0x007B3560`, ProbeClickInWorld `0x007B3CA0`, ProbeDefaultAction `0x007B3E3C`, ProbeMoveToPoint `0x00564770`, ProbeWalkToWaypoint `0x0065B4F0`.

## Build

From this folder run `python3 ../create-patch.py`.

## Changelog pointer

No CHANGELOG.md.
