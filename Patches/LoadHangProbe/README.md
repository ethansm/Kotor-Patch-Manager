# Load Hang Probe

> **STATUS** Track: DIAGNOSTIC. Version 0.1.13 (matches manifest.toml).
> **What it does / explores:** Logs why the post-movie load screen hangs (frame heartbeat, load-screen watchdog, stack sampler, file-I/O ring, window/activation events). A DR0 guard also stops the engine step loop at +aa949 when it would run forever (the combat-unpause freeze).
> **Requires:** Steam/Aspyr LAA build only, exe SHA-256 `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF`; Patch Manager (KPM) with the Common SafeRead.h and CodePatch.h headers.
> **Known limits:** Steam hash above only. The guard changes engine behaviour (it stores +inf and resumes at +aa980) and is armed even without `hangprobe_debug.txt`. `fix=auto` is ON by default when `hangprobe_debug.txt` exists, and it calls engine AppActivated 0x40BCF0 (acts only when the foreground window is the game). **Testing:** the load-hang guard still needs an in-game check.
> **Install:** DIAGNOSTIC: do not install for normal play. Install only when debugging a load hang, because the guard runs without any debug file. Set `fix=off` in `hangprobe_debug.txt` to keep the activation call disabled.

## Features
- Frame heartbeat on `CClientExoAppInternal::MainLoop` (hook 0x00781BE0).
- Load-screen watchdog (250 ms poll) with stack sampling only when a hang looks likely. It never suspends the main thread while the DR0 breakpoint is armed (0.1.11).
- File-I/O ring (IAT hooks) and SetPlayerId hook (0x00798660) with a load timeline.
- Window and activation events (WndProc 0x0040A750, AppActivated 0x0040BCF0, AppDeactivated 0x0040BC40) and load-screen decision points.
- DR0 execute breakpoint on the +aa916 division, with the last 64 passes in the stall report.
- Optional guarded re-activation fix (`fix=auto`).
- Minidump on MAIN_STALL (`hangprobe_stall.dmp`).

## Configuration
Full probe runs only when `hangprobe_debug.txt` exists in the game dir. Keys in that file:
- `fix=auto` (default ON) or `fix=off`
- `fixdelay=6000` (ms, 1000 to 60000)
- `io=0` disables the file-I/O ring

Marker files in the game dir:
- `hangprobe_nofix.txt`: turns the guard off (no breakpoint).
- `hangprobe_nodr.txt`: skips the DR0 breakpoint.
- `hangprobe_nodump.txt`: skips the minidump.
- `hangprobe_activate.txt`: manual causality test (one AppActivated call).
- `hangprobe_dump.txt`: manual stall report.

## Logging
- `kplog.ini` section `[load-hang-probe]`, channel `main`, file `load-hang-probe_log.txt`. On an old core the fallback is `hangprobe_log.txt`.
- `hangprobe_report.txt`: rewritten every 5 s while a stall condition holds.
- Guard-only mode (no debug file) writes `freeze_guard_history.txt` (not rotated by kplog).

## Hooks
Addresses are in `kotor2-steam-aspyr.hooks.toml`. Not all hook addresses are backed by a Ghidra lookup on record.

## Build
- From this folder run `python3 ../create-patch.py`. The Common contract test lives in the author's development workspace and is not shipped here.

## What was learned
The hang is a `CSWGuiLoadScreen` that is never dismissed while the main thread keeps rendering.

## Changelog pointer
Version history is in `manifest.toml` (`description`). Older labels 1.2.0 to 1.4.1 in the source comments are internal phase labels, not patch versions. Patch versions stay 0.x.y.
