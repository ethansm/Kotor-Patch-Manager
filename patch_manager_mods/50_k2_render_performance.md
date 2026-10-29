# Doc 50: K2 Render Performance (grass, shadows, fog) — master plan

**Mods:** `k2-grass-draw-distance` (doc 48), `k2-fog-distance` (doc 49), new `k2-shadow-distance`
**Status:** planned 2026-10-25; Wave 1 next
**Goal:** every optimisation the engine allows for the grass, shadow and fog passes, each one switchable and measured,
so the gains add up and each change's cost or gain is visible in the logs.

---

## 1. Facts (Steam 6A522E71, all checked in the static dumps `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`)

### 1.1 Grass pass
- `Scene::CollectAndTraverse` 0x00499610: octree query radius `FLD float ptr [0x00A0F074]` @0x00499ADC
  (`D9 05 74 F0 A0 00`) builds/collects in-range grass bins. The same global is the fade end in
  `CAurTriangleBin::RenderGrassPolys` 0x0050DAC0 @0x0050DB4C; fade start `[0x00A0F070]` @0x0050DB43; wind gate
  @0x0050DCC8 (already rewritten to the DLL's `g_windDistSq` by grass 0.1.1).
- With the alpha test, opaque texels clip at `sqrt(start² + (1-cutoff)(end²-start²))` = 138.8 m at 120..150 m / 0.40.
  Bins between the clip distance and the fade end are collected, built and drawn but cannot show a pixel
  (inferred from the fade formula, to be confirmed by the PF1a log + screenshot).
- `Scene::DoBinBucket` 0x004664E0 (grass draw): enables state, `glAlphaFunc(GL_GREATER, [0x009F5D38])` @0x00466615,
  then the bin loop: @0x0046661B `MOV dword ptr [EBP-0x4],0` (7 bytes `C7 45 FC 00 00 00 00`) starts it, inner loop
  over `[0x00A7335C]` list / `[0x00A73360]` count calls RenderGrassPolys @0x0046668F; the loop exits to
  @0x00466698 `FLD float ptr [0x00A20040]` (6 bytes `D9 05 40 00 A2 00`), which restores the alpha ref.
  K2 Grass Fog Fix owns 0x004665D8..0x004665F5 (30 bytes); KPM Shader Fixes owns 0x0046AADD. Neither touches 0x0046661B or 0x00466698.
- RenderGrassPolys issues per bin: glTexEnvi, glColor4f, glMaterialfv, glEnable/DisableClientState, client active
  texture `[0x00A32C38]`/`[0x00A32DA8]`, program env `[0x00A32DEC]`, and backend calls `[0x00A1BCB4]`; CPU wind
  `UpdateGrassForWind` 0x00510F70 inside the wind radius. One draw per bin (~1,000 bins at 150 m in dense areas).
- PERF history (doc 48): fps correlates only -0.36 with bins; the same bin counts ran 58-67 fps early and 37-45 fps later
  in one session, so grass is not the only cost; measure each change.

### 1.2 Shadows (`.claude/research/render_distance/R3_shadows.md`, gates re-checked in asm)
- Object cut 35.0 m: `FLD float ptr [0x009F5D50]` (.data, writable float 35.0) @0x00468984 in `Scene::DoGobShadows`
  0x004687E0 and @0x00468D76 in `DoGobShadowsForThisLight` 0x00468B50, both behind `CMP [0x009F5D54],0` (.data 1).
- Second cut dist² <= 1100 (~33.2 m): `FLD float ptr [0x0098BA98]` (.rdata) @0x00468CDB (`D9 05 98 BA 98 00`),
  around the shadow-volume build `CALL 0x00465C90` @0x00468DC1. Needs an operand rewrite like the grass wind FLD.
- Light candidate cap `[0x00A0F040]` (.data, static 1) in 0x0050A630 @0x0050A74B/0x0050A881/0x0050A950/0x0050A95B;
  live value and meaning are unverified, so read and log it first and do not write it.
- 0x00468B50 entry: `55 8B EC 81 EC 8C 00 00 00` (9 bytes; PUSH EBP / MOV EBP,ESP / SUB ESP,0x8C), ECX = Scene, no relative branch.

### 1.3 Fog
- GL fixed-function fog per fragment: no cost to cut. Doc 49 `k2-fog-distance` only moves it. Culling at the fog end
  (camera far plane = fog end) is **rejected**: skyboxes are geometry and would be clipped.

### 1.4 Not levers
- Camera far plane 10,000 m in world scenes (R2). Particle emitters: frustum only, no distance cut (R1).
- Max Lights 3 -> 7 is already installed (KPM `Patches/Max Lights`).

---

## 2. Logging standard (every phase)

1. **Patch log at init**, one line per site: `patch <name> @0x<addr> expect=<bytes> seen=<bytes> rc=<1|0|-1>`
   (`codepatch::writeChecked`). A mismatch writes nothing, logs WARN, and the feature stays off.
2. **Settings line** on load and on every change: all effective values, including which optimisations are on.
3. **PERF line every 5 s** (existing grass window, `kPerfGapMs` drop): fps plus per-feature counters, e.g.
   `grass_ms_avg/max` (CPU time of the DoBinBucket bin loop, QPC), `bins` (drawn), `collected`, `trimmed`, `a2c`,
   shadow `casters/s` and `dist`. Each optimisation has an ini/menu toggle so A/B runs show its delta.
4. **Once-only capability lines:** GL facts the feature depends on (e.g. `GL_SAMPLE_BUFFERS`, `GL_SAMPLES` for A2C).
5. Logs stay under ~50 startup lines; per-frame detail only with `verbose = 1`. `kotor2_logs.py check` must read CLEAN.

---

## 3. Phases

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **PF0** | Recon + plan (this doc); fog mod built (doc 49 F1) | - | - | DONE | c5a6e142 | doc 50 master plan; recon R1-R3; fog built (doc 49 F1) |
| **PF1a** | grass 0.1.4-dev: collect radius = clip + 2 m (operand rewrite 0x00499ADC -> `g_collectSq`, toggle `trim_hidden`), bin-loop CPU timing (detours 0x0046661B / 0x00466698, QPC), alpha-to-coverage toggle `smooth_edges` (glEnable/glDisable `GL_SAMPLE_ALPHA_TO_COVERAGE` 0x809E in the same detours, gated on GL_SAMPLE_BUFFERS), PERF + 2 menu rows | PF0 | - | INSTALLED | 3467b4ec | grass 0.1.4-dev; KPM dae7f2b; GATE PASS; play GR5 |
| **PF1b** | new `k2-shadow-distance` 0.1.0-dev: Shadow Distance slider (vanilla 35 m; writes 0x009F5D50 and rewrites 0x00468CDB operand to `g_shadowRefSq` = dist² x 1100/1225), entry hook 0x00468B50 (ini poll, casters counter), logs live `[0x00A0F040]`/`[0x009F5D54]`, menu page | PF0 | - | INSTALLED | 3467b4ec | k2-shadow-distance 0.1.0-dev; KPM dae7f2b; GATE PASS; play SH1 |
| **PF1c** | Recon for Wave 2 (read-only): RenderGrassPolys per-bin state and draw path, `[0x00A1BCB4]` backend, wind update 0x00510F70 data flow, quad/index layout for LOD | PF0 | - | DONE | 95171d08 | R4 hoist candidates (path mask 0xA32DF8 runtime-only), R5 thin glDrawArrays count; both PARTIAL |
| **PF1d** | Install wave: fog F2 + grass 0.1.4 + shadow 0.1.0 in one `kpm_reinstall.py` (new hooks/ids) | PF1a, PF1b | user GO | INSTALLED | | 18:16 10-25 full reinstall (55 patches); rollback batch_reinstall_20261025_181615; play FG1/GR5/SH1 |
| **PF2a** | Redundant per-bin state hoist (set texenv/material/client state once per bucket) | PF1c, PF1d | - | TODO | | probe in 0.1.5 (path probe + pathA/B1/B2 PERF); build after GR6 logs |
| **PF2b** | Wind throttle: update wind for bins past N m every k-th frame | PF1c, PF1d | - | INSTALLED | 5c23ddf7 | grass 0.1.5-dev LIVE 10-25 20:12, play GR6 |
| **PF2c** | Far-grass LOD: draw a fraction of a far bin's quads (index count), density compensation by alpha | PF1c, PF1d | - | INSTALLED | 5c23ddf7 | grass 0.1.5-dev LIVE 10-25 20:12, play GR6 |
| **PF2d** | GPU timing of the grass pass (`GL_ARB_timer_query` if present) into PERF | PF1d | - | INSTALLED | 5c23ddf7 | grass 0.1.5-dev LIVE 10-25 20:12, play GR6 |
| **PF3** | Defaults from A/B logs; log tightening (`perf_log` default off, verbose gating) | PF2* | - | TODO | | |
| **PV1** | grass 0.1.6-dev variety: per-face RNG seed from a float-bit hash of the face verts (replaces `srand(round(x0+y1+z2))` at 0x50EF90; kills repeated clumps); toggle `vary_seed` | PF2b-d | - | INSTALLED | 5f34be01 | R6 #1 (`.claude/research/render_perf/R6_grass_variety.md`) |
| **PV2** | grass 0.1.6-dev variety: uniform barycentric blade positions (rewrite the 3 weights at 0x50F177 before the normalise call; no centre bunching); toggle `even_spread` | PF2b-d | - | INSTALLED | 5f34be01 | R6 #2 |
| **PV3** | grass 0.1.6-dev variety: density noise + stochastic count rounding at 0x50D35C in CAurTriangleBin::AddTriangle 0x0050C8A0 (deterministic world-position value noise); toggle `density_noise` + amount | PF2b-d | - | INSTALLED | 5f34be01 | R6 #3 |
| **PV4** | grass 0.1.6-dev variety: wider blade size range (operand rewrite of the scale-jitter FMUL 0x0050FEE7 `[0x009921C8]` (0.4) to the DLL's own double; s = 1 + (r-0.5)*span); slider `size_var`. Replaces R6 #4 UV flip: REDUNDANT (flip at yaw t == yaw t+pi; yaw uniform 0..2pi, culling off at 0x466568, u04) | PF2b-d | - | INSTALLED | 5f34be01 | R6 #5; recon `.claude/waves/w1-50-PV/` |
| **EP1** | new `k2-engine-perf` 0.1.0-dev (R11 levers 6-7, `.claude/research/render_perf/R11_synthesis.md`): reuse the texture-id list across the 3 bucket drains of a scene (3 `CALL 0x00427970` sites -> DLL, key = TexMgr tick + sheet count + no pending, verify every 240th hit, trips off on a mismatch) + GlobalMemoryStatus cache at 0x00478A57 (250 ms); ini `k2_engine_perf.ini`, PERF with measured `saved_us_s` | R11 | user GO | INSTALLED | 9eb531de | EP1 played 10-26: gms cache saves ~5 ms/s (40 us x ~100 calls/s); tex cache 0 hits (scan 5.5 us, ~1x/frame) = no gain; dropped in 0.1.2-dev (3 call sites, `TexScanCache.h`, `tex_scan_cache`/`tex_verify_every` keys removed) |
| **FP1** | new `k2-frame-pacing` 0.1.0-dev (R11 levers 3-4): detour 0x0040846E end of WinMain loop; Sleep 50 ms while minimized, background FPS cap 30, optional FPS cap (Sleep + 1.5 ms QPC spin); mode via IsIconic/GetForegroundWindow on HWND [0x00A1B484]; Mod Configuration page (3 rows); also ships k2-engine-perf 0.1.1-dev (verify mismatch at ERR) | R11 | user GO | INSTALLED | df0c9b09 | 00:17 10-26 full reinstall (57) + engine-perf 0.1.1 hot-swap; rollback k2-frame-pacing_reinstall_20261026_001714; play FP1 |

Wave 1 (PF1a, PF1b, PF1c) runs in parallel: disjoint write scopes, no shared header edits (`_shared_src` read-only).

## 4. Wave 2 batching (decided 2026-10-25 after PF1d)
- **Batch W2-1 = grass 0.1.5-dev: PF2d + PF2b + PF2c + PF2a probe**, one worker (all touch the same grass files, so no parallel workers), one gate, one full reinstall (new hooks), one play session.
  - PF2d GPU timing (measurement, on by default).
  - PF2b wind throttle and PF2c far LOD each behind a new ini/menu toggle, **default off**, so one play session A/Bs them from the menu.
  - PF2a probe: log `[0x00A32DF8]` (render-path mask) and the path taken once per area; no hoist yet.
- **Batch W2-2 = PF2a hoist + PF3**, after the W2-1 play logs.
- Play GR5/FG1/SH1 first if possible: they are the timing baseline (grass_ms, a2c capability, shadow per_s).

### 4.1 PF1c findings that steer Wave 2 (`.claude/research/render_perf/R4_grass_state.md`, `R5_grass_wind_lod.md`, both PARTIAL)
- R4: bins are the inner loop at 0x0046664B, `this = [0x00A7335C][j]`. Hoist candidates: bucket-level glMaterial/env-param and sub-bin texenv/enable calls. `[0x00A1BCB4]` is a dispatcher set in FUN_004317D0 (0x00432AD0 / 0x00432BB1) to FUN_004B7F40 or FUN_0042ABC0. **Open:** which of the 3 RenderGrassPolys paths (A, B1, B2) runs live depends on `[0x00A32DF8]`; not in the static dumps -> PF2a probe.
- R5: wind runs per near bin inside RenderGrassPolys and writes CPU arrays (no GL upload). The bin count (bin+0x0C, read at 0x0050E65D) feeds both the draw and the normals offset, so PF2c must thin only the `glDrawArrays` count (0x00436A8B, 0x00436B7C), not the field. **Open:** whether the shader-path base (bin+0x40) overlaps the wind-write target [bin+0x38]; whether the group path (sites A/B) is live.
