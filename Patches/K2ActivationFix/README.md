# K2 Activation Fix

> **STATUS** Track: SHIPPABLE. Version 1.1.0 (matches manifest.toml).
> **What it does / explores:** Stops false app-deactivate messages from leaving the load screen stuck after a movie. Under Proton the movie thread posts a synthetic `WM_ACTIVATEAPP(0, 0)` while the game window owns the foreground; this patch neutralises it.
> **Requires:** Steam/Aspyr KOTOR II (hooks file `kotor2-steam-aspyr.hooks.toml` only), Patch Manager. No other patches (`requires = []`).
> **Known limits:** Steam/Aspyr only; GOG and other builds have no hooks file. Hook sites are byte-checked at install. **Testing:** the post-movie load-screen cases still need an in-game check.
> **Install:** Install through Patch Manager like any patch. Safe for normal play.

## Features
- Detour at `0x0040D69E` (before `CALL [PostMessageA]` in the movie thread). If the post is `WM_ACTIVATEAPP` with `wParam == 0` and `lParam == 0`, and the foreground window belongs to this process, the call becomes `PostMessageA(NULL, WM_NULL, 0, 0)`. Real alt-tabs are left alone.
- Second hook at the WndProc entry `0x0040A750` (`dropForeignDeactivate`, version 0.3.0). It drops `WM_ACTIVATEAPP(wParam == 0, any lParam)` while the foreground is ours and `IsMoviePlaying() == 1`. This covers the `lParam = 0x1ac` case the post-site gate cannot see.

## Configuration
- None. No ini keys.

## Logging
- Each dropped message is appended to `k2-activation-fix_drops.txt` in the game folder, capped at 50 lines.

## Hooks
- `0x0040D69E` (post site) and `0x0040A750` (WndProc entry), both in `kotor2-steam-aspyr.hooks.toml`. Not DB-backed.

## Build
- From this folder run `python3 ../create-patch.py`.
- Host unit tests (Common contract and drop decision logic) live in the author's development workspace and are not shipped here.

## What was learned
- The hang is intermittent (about 1 launch in 8 before the fix). The false deactivate leaves the window minimised, so no `WM_ACTIVATEAPP(TRUE)` ever arrives and the load screen never hides.

## Changelog pointer
- Version history is in the git log for this directory (0.2.0 detour port, 0.3.0 WndProc second hook).
