# K2 Shadow Distance

> **STATUS** Track: SHIPPABLE. Version 0.1.0-dev (matches manifest.toml).
> **What it does / explores:** Raises the distance at which KOTOR II objects cast stencil shadows, from the vanilla 35 m up to 100 m, with 35 m as the default so vanilla behaviour is kept until changed.
> **Requires:** Steam/Aspyr KOTOR II build (supported_versions hash in manifest.toml), Kotor Patch Manager (KPM). No required providers. Optional: `k2.modconfig@1` and `k2.menuui@1` for the in-game page.
> **Known limits:** Play test SH1 not yet run, so the visual result and frame cost are unverified. Shadow cost per distance is unmeasured. A light-cap setting is not implemented (0x00A0F040 is read only). **Play-test status:** SH1 pending in the project play-test queue.
> **Install:** Install through KPM like other patches, then start the game once so the default ini is written.

## Features
- Object shadow distance gate raised from 35 m (vanilla) to a value chosen by the user, 10-100 m.
- Optional in-game page "K2 Shadow Distance" under Options -> Mod Configuration (rows: Extended Shadow Distance toggle, Shadow Distance slider 10-100 m, step 5).
- Ini changes are picked up while the game runs (polled about once per second); no restart needed.
- A `PERF` line is logged every 5 seconds with the number of shadow-casting lights per second and the active distance.

## Configuration
File: `k2_shadow_distance.ini` in the game folder (relative path; written with defaults if missing).

Section `[Shadows]`:

| Key | Default | Range / values | Meaning |
|-----|---------|----------------|---------|
| `enabled` | `1` | `0` / `1` | `0` restores the vanilla 35 m engine value. |
| `shadow_dist` | `35` | 10-100 (out-of-range values are clamped; alias `shadowdist`) | Shadow distance in metres. `35` = vanilla. |
| `perf_log` | `1` | `0` / `1` | Emit the `PERF` line every 5 s. |
| `verbose` | `0` | `0` / `1` | Extra logging of config and engine state. |

Log file: `k2-shadow-distance_log.txt` (KpLog fallback file in the game folder). Use `python3 scripts/kotor2_logs.py check k2-shadow-distance_log.txt --new` to read it.

## Hooks and patched addresses
Steam/Aspyr build (`kotor2-steam-aspyr.hooks.toml`, `code_patches.toml`):

- **Detour** at `0x00468B50` (`Scene::DoGobShadowsForThisLight` prologue, 9 bytes). Handler `K2ShadowOnLight` polls the ini, registers the config page, re-writes the engine globals and counts calls for the PERF line.
- **Engine float** `0x009F5D50` (vanilla 35.0) is written with `shadow_dist` on every shadow-light call. Vanilla value is written back when `enabled = 0`.
- **Code patch** `0x00468CDB`: the `FLD` view-reference gate (vanilla `[0x0098BA98]` = 1100.0) is re-pointed at the DLL's `g_shadowRefSq` (= dist^2 * 1100/1225). Applied at KPatchInit, restored on FreeLibrary.
- **Read only:** `0x00A0F040` (light cap), `0x009F5D54` (distance-cut flag), `0x009F5D30` (shadows on), `0x009F5DA8` (soft shadows).

## Build and test
- Scoped tests: `python3 scripts/run_tests.py k2_shadow_distance`
- Fork lint: `python3 scripts/fork_lint.py k2_shadow_distance`

## Known gaps (tracked in PR_READINESS.md)
- Play row SH1 (creature shadows at 35 / 70 / 15 m) is pending.
- The ini mtime reload path is not yet covered by a test (the contract test uses a nonexistent path).
- `clampf(+inf)` returns the lower bound (10); the intended upper handling is still to be decided.

## Changelog
No CHANGELOG.md yet. See `PR_READINESS.md` for the current audit state.
