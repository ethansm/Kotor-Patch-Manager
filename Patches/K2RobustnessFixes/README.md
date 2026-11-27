# Engine Robustness Fixes (k2-robustness-fixes)

> **STATUS** Track: SHIPPABLE. Version 0.1.3 (matches manifest.toml).
> **What it does / explores:** Thirteen small, independently switchable fixes for crashes and leaks in the Steam/Aspyr KOTOR II engine (resource teardown, script dispatch, network send ring, GUI parsing, save thumbnails, movie loop, script situations).
> **Requires:** Steam/Aspyr KOTOR II build (`supported_versions` hash in manifest.toml), Patch Manager (KPM). No other patch required (`requires = []`, `conflicts = []`).
> **Known limits:** `fs=2` and `dclk=2` (free mode) are not the default until real drop volume is logged. `movspin=1` may shift the load-hang H1 race. Every site is byte-verified, so a different build refuses the affected group instead of writing.
> **Testing:** live-tested on Steam since 10-02/10-03; `fs` and `dclk` run count-only.
> **Install:** Install through KPM like any other patch. Defaults are safe for normal play. `movspin`, `fs=2` and `dclk=2` are opt-in toggles: leave them off unless you are testing them.

## Features

Each group is one engine fix. A group is written only if every one of its sites matches the vanilla bytes.

| Key | Fix |
|---|---|
| `kt` | Resource key-table teardown leaves a dangling key-entry pointer that is later written to freed heap. |
| `vmid` | Negative NWScript command ids bypass the dispatch bounds check. |
| `sndri` | Options > Sound apply with Force Software leaves Miles shut down until a second apply. |
| `netovf` | Oversized network message overruns the 2x64 KB send ring. |
| `guiid` | A huge control id in a `.gui` file pads the control list until allocation fails. |
| `abbound` | Action-bar list shrink copies past the new buffer. |
| `fs` | A failed resource Service leaves a stale buffer that Malloc overwrites. |
| `ssb` | Save-thumbnail capture dereferences a missing camera. |
| `ssc` | Save-thumbnail capture uses uninitialised dimensions in a square or portrait window. |
| `td` | A 2DA token over 255 characters smashes the stack cookie. |
| `vmlk` | ActionDoCommand leaks the script situation when OBJECT_SELF is gone. |
| `movspin` | Movie wait loop busy-spins (one Sleep(1) per message pump). |
| `dclk` | DelayCommand/AssignCommand drop the script situation's code copy and stack. |

## Configuration

File: `robustnessfixes.txt` in the game folder. One `key=value` per line. Lines longer than 127 characters are cut off, so keep each line short.

| Key | Default | Values |
|---|---|---|
| `enabled` | on | `enabled=0` disables all groups, even if a per-key line says on. |
| `kt`, `vmid`, `sndri`, `netovf`, `guiid`, `abbound`, `ssb`, `ssc`, `td`, `vmlk` | 1 (on) | `0` disables the group. |
| `fs` | 1 (count only) | `0` off, `1` count only, `2` count and free. |
| `dclk` | 1 (count only) | `0` off, `1` count only, `2` count and free. Not the default: the drop frequency is still unmeasured. |
| `movspin` | 0 (off) | `1` enables. |

Example file with the defaults written out:

```
enabled=1
kt=1
vmid=1
sndri=1
netovf=1
guiid=1
abbound=1
fs=1
ssb=1
ssc=1
td=1
vmlk=1
movspin=0
dclk=1
```

## Logging

Log file: `k2-robustness-fixes_log.txt` in the game folder (shared KpLog, channel `main`). Only if the old core is loaded does it fall back to `robustnessfixes_log.txt`.

- **Load line** (`k2-robustness-fixes 0.1.3 loaded: N/13 groups patched: ...`): per-group state. `ON` = patched, `off` = disabled by config, `REFUSED` = bytes did not match, so the group was skipped. `fs=ON(count|free)` and `dclk` modes are shown too.
- **REFUSED continuation lines**: `REFUSED: bytes at 0x... are ... (group untouched)` means the build differs from the vanilla bytes. `REFUSED(protect)` means VirtualProtect failed and the group may be PARTIAL. `REFUSED(unmapped)` and `REFUSED(cave)` mean the target memory or the code cave could not be used.
- **COUNTERS lines**: written every 5 seconds, three lines (21 values in total), only when a counter changed.
- **FIRED lines**: one per group, written the first time that fix's counter moves. A FIRED line means the fix really triggered in play. For example, `FIRED fs stale=... bytes=... mode=count|free`.

## Hooks

- The DLL writes byte patches into the game (not KPM hooks). Every site is listed with its original bytes in `code_patches.toml`, and `RobustnessFixes.cpp` memcmps each group before writing.
- `kotor2-steam-aspyr.hooks.toml` lists the sites and the adjacency notes (perf-lab detour and script_events sites do not collide).

## Build

- From this folder run `python3 ../create-patch.py`.

## What was learned

- `fs` and `dclk` were added as count-only first. Free mode is kept opt-in until the logs show real drop volume.

## Changelog

No CHANGELOG.md.
