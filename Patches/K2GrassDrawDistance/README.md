# K2 Grass Draw Distance

> **STATUS** Track: SHIPPABLE. Version 0.1.10-dev (matches manifest.toml).
> **What it does / explores:** Extends KOTOR II grass render and octree build distance from the vanilla 15-30 m fade (hard-clipped at 25.1 m by the 0.40 alpha test) to a configurable 120-150 m, and trims grass that is fully hidden.
> **Requires:** Steam/Aspyr KOTOR II build matching the hash in manifest.toml `[patch.supported_versions]`; KPM. Optional: mod-config (adds the in-game Options page).
> **Known limits:** Max draw distance 150 m; wind radius capped at 90 m. R12 review fixes (far-LOD thinning, B1 vertex pointer, fast bin diff) are not yet play-verified. **Testing:** 0.1.10-dev and the earlier grass checks still need an in-game pass.
> **Install:** Install through KPM (see `kotor2-kpm-install` skill). Normal play: install as usual; the mod is standalone (`requires = []`, `conflicts = []`).

## Features
- Grass fade start/end and draw distance (defaults 120 m / 150 m).
- Wind radius for CPU grass simulation, decoupled from draw distance (default 20 m).
- Alpha cutoff for grass edges (default 40, vanilla 0.40 behaviour).
- Trim hidden grass from collection (`trim_hidden`, default on).
- Optional grass variety: varied pattern, even spread, density patches, size range.
- Optional edge smoothing (alpha-to-coverage), far-LOD thinning, wind throttle.

## Configuration
- File: `k2_grass_draw_distance.ini`, section `[Grass]`.
- Keys: `enabled`, `fade_start_dist`, `fade_end_dist`, `wind_dist`, `alpha_cutoff`, `trim_hidden`, `fast_bin_diff`, `smooth_edges`, `wind_throttle`, `far_lod`, `vary_seed`, `even_spread`, `density_noise`, `density_amount`, `density_cell`, `size_var`, `wind_near`, `wind_every`, `lod_start`, `lod_min_keep`, `fix_b1_pointer`, `gpu_timing`, `perf_log`, `verbose`.
- Ranges shown in mod-config: fade start 15-135 m, draw distance 30-150 m, wind radius 0-90 m, alpha cutoff 5-60, size variation 40-120.
- The default ini is written on first run when missing.

## Logging
- Log file: `k2-grass-draw-distance_log.txt` (KpLog fallback, INFO level).
- Channel `perf`: grass PERF bin counts and grass-pass CPU time every 5 s (`perf_log`).

## Hooks
- Code patches (code_patches.toml): `wind_fld` 0x0050DCC8, `b1_vertex_ptr` 0x0050E483 (gated by `fix_b1_pointer`), `size_span_fmul` 0x0050FEE7, `collect_fld` 0x00499ADC, data `fade_start_sq` 0x00A0F070.
- Hook list: `kotor2-steam-aspyr.hooks.toml`, including the fast bin-diff hook at 0x00498829.
- Addresses are not yet all backed by the address DB.

## Build
- From this folder run `python3 ../create-patch.py`. Host unit tests live in the author's development workspace and are not shipped here.
- Test config: `test.toml` (uses `${KPM_COMMON_DIR}` for the KPM Common headers).

## Changelog
- No CHANGELOG.md.
