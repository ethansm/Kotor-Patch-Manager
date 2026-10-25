# Doc 48: K2 Grass Draw Distance

**Mod Directory:** `patch_manager_mods/k2_grass_draw_distance_src/`  
**Mod ID:** `k2-grass-draw-distance` (`0.1.2-dev`)  
**Status:** Built & Verified (Ready for KPM Prepare / Install)  
**Owning Mod Scope:** `patch_manager_mods/k2_grass_draw_distance_src/` (Strict boundary: do not touch other mods)

---

## 1. Objective & Architecture

Extends the grass draw distance and octree build/collect radius in KOTOR II (`swkotor2.exe` Steam Aspyr `6A522E71...`):

1. **Octree Build & Fade-End Distance (`0x00A0F074`, `_DAT_00a0f074`)**:
   - Vanilla value: `900.0f` ($30.0^2\text{ m}^2 \implies 30\text{ m}$).
   - Used at `0x00499ADC` in `Scene::CollectAndTraverse` (`0x00499610`) to query room octrees (`0x00519A50` -> `0x004986B0`) and build/destroy grass bin quad vertex buffers (`0x004987E0` -> `0x0050D960` / `0x0050DA00`).
   - Also used at `0x0050DB4C` in `CAurTriangleBin::RenderGrassPolys` (`0x0050DAC0`) as the outer distance squared (`local_44`) where grass alpha fades to `0.0`.
   - Extended default: `90.0m` (`8100.0f`).
2. **Fade-Start Distance (`0x00A0F070`, `_DAT_00a0f070`) & CPU Wind Radius (operand at `0x0050DCC8`)**:
   - Vanilla shares `0x00A0F070` (`225.0f` = $15.0^2\text{ m}^2$) between fade-start (`0x0050DB43`) and CPU wind vertex deformation (`0x0050DCC8` -> `CAurTriangleBin::UpdateGrassForWind` `0x00510F70`).
   - This mod decouples them:
     - `0x00A0F070` keeps its vanilla role, fade start: the DLL writes `fade_start_dist^2` (default `120.0m` = `14400.0f` since 0.1.3; was `45.0m`), read at `0x0050DB43`.
     - Wind (0.1.1): `KPatchInit` rewrites the imm32 of the wind gate `FLD [0x00A0F070]` at `0x0050DCC8` (`D9 05 70 F0 A0 00`, byte-verified with `codepatch::writeChecked`, listed in `code_patches.toml`) to the DLL's own float `g_windDistSq` = `wind_dist^2` (default `20.0m` = `400.0f`, `0` = no wind). The vanilla operand is restored on FreeLibrary. On a byte mismatch nothing is written and wind follows fade start (WARN logged). 0.1.0 used a per-bin detour at `0x0050DB55` writing `[EBP-0x28]` instead (up to ~3,700 calls a frame at 90 m); removed.
3. **OpenGL Alpha-Test Threshold (`0x009F5D38`, `_DAT_009f5d38`)**:
   - Vanilla value: `0.40f` (read at `0x004665F6` in `Scene::DoBinBucket` `0x004664E0` for `glAlphaFunc(GL_GREATER, ref)`), which prematurely hard-clips fading grass at $\approx 25.1\text{ m}$.
   - Default stays vanilla `0.40f` (0.1.1): lowering it also draws soft edge texels on near grass. With the default window 120..150 m (0.1.3; was 45..90 m = ~75.3 m), opaque texels clip at $\approx 138.8\text{ m}$. `0.20` on 60..90 m would reach $\approx 84.9\text{ m}$ at the cost of fuzzier blades.
4. **Clamps (0.1.1):** `fade_end_dist` 10..150 m (150 m = 25x vanilla grass area; raise only after P3 numbers), `fade_start_dist` 5..145 m and always >= 5 m below the end. A start at or past the end pulls the start down; it never raises the draw distance (the menu would show a stale value).
5. **PERF telemetry (0.1.1, `perf_log = 1`, channel `perf`):** every 5 s a `PERF fps= frames= bins_avg= bins_max= end= start= wind= alpha=` line. `bins` = previous frame's in-range bin list count `[0x00A73304]` (list `{0x00A73300 ptr, +4 count}`, copied from the current list `0x00A7330C` by `0x00495760` at `0x00499775` before hook 1); `fps` = grass frames per second. `0x00A73694` was rejected: it is a shared polygon counter written by about 10 renderers.
6. **Configuration & Live Reload (`k2_grass_draw_distance.ini` + `ModConfigDecl.h`)**:
   - Hot-reloads `k2_grass_draw_distance.ini` via `FileWatch.h` (1s throttle) inside `K2GrassOnSceneCollect` (`0x00499ABC`).
   - Lazily registers an in-game `"K2 Grass Draw Distance"` options page with `mod-config` (`k2.modconfig@1` / `k2.menuui@1`) when present.

---

## 2. Master Plan Phasing (7-Column Schema)

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **P0** | Baseline & Scaffolding | - | - | DONE | 17db2e05 | `build_check.py --mod k2_grass_draw_distance_src` |
| **P1** | Core Detour Implementation | P0 | - | DONE | 17db2e05 | `build_check.py` + host unit tests |
| **P2** | Common Contract Test Verification | P1 | - | DONE | 17db2e05 | `run_tests.py -k k2_grass_draw_distance` |
| **P2b** | Review fixes 0.1.1-dev: vanilla alpha default + 45..90 m, 150 m cap, start-pulls-down sanitize, PERF telemetry, hook 2 -> wind operand rewrite, docs | P2 | - | DONE | ab47500d | 0.1.1-dev review fixes; KPM ab63e85; GATE PASS (mod steps; default tier fails only on peer texmem 6ee4936c) |
| **P2c** | 0.1.2-dev menu slider link: Fade Start capped at menuFadeStartMax(Draw Distance) (15 m grid, >= 5 m below); lowering Draw Distance moves the Fade Start slider and writes its ini key; page build repairs a hand-edited ini | P2b | - | DONE | 53b9703b | 0.1.2-dev linked sliders; KPM 0c7d9a3; LIVE 10-25 10:21 kpatch 0675daed, rollback k2-grass-draw-distance_reinstall_20261025_102121; play GR1-GR3 |
| **P3a** | 0.1.3-dev from GR1/GR2 play: defaults 120..150 m (user: 150 m smooth, visible gain; 895 bins at 57-59 fps vs 42 bins at 51-55 fps for 30 m), PERF drops a window after a >1 s grass gap (menu/load samples read 0.1-23 fps) | P2c | - | TODO | | hot-swap (no hook change) |
| **P3** | Live In-Game Probing & Calibration | P2c | user GO | TODO | | `kpm_reinstall.py --mod k2-grass-draw-distance` (new hooks: full reinstall), play rows GR1-GR3 |
| **P4** | Log Tightening & Packaging | P3 | - | TODO | | Clean startup log verification |

---

## 3. Verification Gates

```bash
# Build gate
python3 scripts/build_check.py --mod k2_grass_draw_distance_src

# Unit and contract tests
python3 scripts/run_tests.py -k k2_grass_draw_distance
python3 scripts/run_tests.py -k grass_draw_distance_test

# Pre-install gate
./scripts/pre_install_gate.sh --mod k2_grass_draw_distance_src
```

---

## 5. Open design note (not scheduled)

A native slider in the vanilla Graphics panel (`CSWGuiOptionsGraphics` 0x007D4620, free space y~330-382 under
CB_FORCESPEED, clone SLI_GAMMA) is possible as a later phase; it needs a ctor hook site, the slider change event id and a
dtor hook (none mapped yet). 0.1.x uses the Mod Configuration page.
