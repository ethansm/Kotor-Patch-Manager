# K2 Frame Pacing

> **STATUS** Track: SHIPPABLE. Version 0.1.0-dev (matches manifest.toml).
> **What it does / explores:** Stops KOTOR II from spinning a full CPU core while the window is minimized, caps the frame rate while the game is in the background, and offers an optional FPS cap that sleeps instead of busy-waiting.
> **Requires:** Kotor Patch Manager (KPM) and the Steam/Aspyr KOTOR II build (hash in manifest.toml). Optional: k2.modconfig for the in-game Options page; without it the ini file is the only config.
> **Known limits:** Background and FPS-cap behaviour are only partly play-tested (see below). Fullscreen alt-tab may minimize the window instead of backgrounding it, so the background cap may not trigger there. The engine's own limiter at 0x0040847F is left dormant and untouched.
> **Testing:** live-tested on Steam: minimized and active modes pass; background mode and the 45 FPS cap were not seen yet and still need an in-game check.
> **Install:** Install through KPM like any other patch. Normal play is fine; it only changes idle behaviour.

## Features
- **Minimized sleep:** while the window is minimized, the main loop sleeps each iteration instead of spinning a core.
- **Background FPS cap:** while the game is in the background, frames are capped (default 30 fps).
- **Optional FPS cap:** when set, the active window is capped too. The wait is a Sleep plus a short QPC spin, so the cap does not overshoot.
- **PERF log:** one `PERF` line every 5 s with fps, mode frame counts, work time and slept/spun time.

## Configuration (`k2_frame_pacing.ini`, section `[FramePacing]`)

| Key | Default | Range | Meaning |
|---|---|---|---|
| `enabled` | 1 | 0/1 | Master switch. 0 turns the mod off for the session. |
| `minimized_sleep` | 1 | 0/1 | Sleep while minimized. |
| `minimized_sleep_ms` | 50 | 5..200 | Milliseconds slept per loop while minimized. |
| `background_fps` | 30 | 0 or 5..120 | Background FPS cap. 0 = no cap. |
| `fps_cap` | 0 | 0 or 20..360 | FPS cap while active. 0 = off (V-Sync paces). |
| `spin_us` | 1500 | 0..5000 | Microseconds of spin at the end of a capped wait. |
| `perf_log` | 1 | 0/1 | Write the 5 s PERF line. |

Edits to the ini are picked up while the game runs (reload poll).

With mod-config installed, the same keys appear under Options -> Mod Configuration -> K2 Frame Pacing (rows: minimized sleep, background FPS, FPS cap).

## Logging
Log file: `k2-frame-pacing_log.txt` in the game folder (KpLog, fallback file). Look for `loaded:` at start, `INI reload #` on config changes, `mode <a> -> <b>` on transitions, and `PERF` every 5 s.

## Hooks
- Hook 1: WinMain loop end, address 0x0040846E (detour, 7 bytes `CMP dword ptr [0x00A1B704],0`). Defined in `kotor2-steam-aspyr.hooks.toml`.
- The WinMain parent 0x00407920 resolves in the address DB. The 0x0040846E site and the data addresses 0x00A1B704 / 0x00A1B710 are not yet backed by address-DB rows.

## Build
- From this folder run `python3 ../create-patch.py`.

## What was learned
- The vanilla main loop has no sleep, so a minimized window burns a full core. Sleeping at the loop end fixes that without touching rendering.
- The engine limiter after the hook (0x0040847F, `[0x00A1B710] = 0.0`) is dormant. This mod does not use it.

## Changelog
See `CHANGELOG.md`.
