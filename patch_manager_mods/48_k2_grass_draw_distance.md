# Doc 48: K2 Grass Draw Distance

**Mod Directory:** `patch_manager_mods/k2_grass_draw_distance_src/`  
**Mod ID:** `k2-grass-draw-distance` (`0.1.0-dev`)  
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
2. **Fade-Start Distance (`[EBP-0x28]` at `0x0050DB55`) & CPU Wind Radius (`0x00A0F070`, `_DAT_00a0f070`)**:
   - Vanilla shares `0x00A0F070` (`225.0f` = $15.0^2\text{ m}^2$) between fade-start (`0x0050DB43`) and CPU wind vertex deformation (`0x0050DCC8` -> `CAurTriangleBin::UpdateGrassForWind` `0x00510F70`).
   - This mod decouples them:
     - `0x00A0F070` holds `wind_dist^2` (default `20.0m` = `400.0f`), read at `0x0050DCC8` so distant grass bins do not incur CPU wind vertex math.
     - Detour `K2GrassOnBinFadeInit` at `0x0050DB55` (`83 3D 90 D6 A7 00 00`, 7 bytes) writes `fade_start_dist^2` (default `60.0m` = `3600.0f`) into `[EBP-0x28]` right after `0x0050DB49` initializes it.
3. **OpenGL Alpha-Test Threshold (`0x009F5D38`, `_DAT_009f5d38`)**:
   - Vanilla value: `0.40f` (read at `0x004665F6` in `Scene::DoBinBucket` `0x004664E0` for `glAlphaFunc(GL_GREATER, ref)`), which prematurely hard-clips fading grass at $\approx 25.1\text{ m}$.
   - Extended default: `0.20f` (extends visible smooth fade to $\approx 84.9\text{ m}$ for a $60\text{ m}..90\text{ m}$ fade window).
4. **Configuration & Live Reload (`k2_grass_draw_distance.ini` + `ModConfigDecl.h`)**:
   - Hot-reloads `k2_grass_draw_distance.ini` via `FileWatch.h` (1s throttle) inside `K2GrassOnSceneCollect` (`0x00499ABC`).
   - Lazily registers an in-game `"K2 Grass Draw Distance"` options page with `mod-config` (`k2.modconfig@1` / `k2.menuui@1`) when present.

---

## 2. Master Plan Phasing (7-Column Schema)

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **P0** | Baseline & Scaffolding | - | - | DONE | 64efbd9a | `build_check.py --mod k2_grass_draw_distance_src` |
| **P1** | Core Detour Implementation | P0 | - | DONE | 64efbd9a | `build_check.py` + host unit tests |
| **P2** | Common Contract Test Verification | P1 | - | DONE | 64efbd9a | `run_tests.py -k k2_grass_draw_distance` |
| **P3** | Live In-Game Probing & Calibration | P2 | user GO | TODO | | Live KPM install & validation |
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
./scripts/pre_install_gate.sh --skip-default --mod k2_grass_draw_distance_src
```
