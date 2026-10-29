# R12 — Deep Review & Additional RE Findings: Grass and Fog Patches/Mods

Date: 2026-10-29
Target binary: KOTOR II Steam Aspyr (`6A522E71`)
Scope: Audit of `k2-grass-draw-distance` (`0.1.9-dev`), `k2-fog-distance` (`0.1.0-dev`), upstream KPM grass/fog patches (`K2GrassFogFix`, `K2AspyrFogReflectionFix`, `K2AspyrShaderFixes`, K1 `GrassRenderFix`/`GrassMemorySafety`), and static reverse-engineering of the Odyssey grass/fog pipeline for additional performance, correctness, and visual diversity improvements.

---

## 1. Audit Findings on Existing Grass & Fog Mods

### 1.1 `k2-grass-draw-distance` (`0.1.9-dev`)
- **Multi-Triangle Bin Prefix-Thinning Bias in `PF2c` (`far_lod`)**:
  - `CAurTriangleBin::CreateGrassArrays` (`0x0050EBC0`, `0050ebc0.c:148-347`) iterates sequentially over the bin's faces (`for (face = 0; face < *(int*)(bin + 0x4c); ++face)`) and appends each face's blades contiguously into `bin+0x38`.
  - `K2GrassDrawArraysA/B` (`0x0050DF0D`, `0x00436AE6`) implements `far_lod` via prefix count reduction (`glDrawArrays(GL_QUADS, first=0, count=thinQuads*4)`).
  - When a bin contains multiple triangles (`*(int*)(bin + 0x4c) > 1`), prefix thinning keeps 100% of blades on the first `keep` fraction of triangles in the bin and **0% of blades on the remaining `(1 - keep)` triangles**, creating whole-triangle bald cutouts in the mid/far band (`60..140 m`) rather than uniform thinning across all triangles in the bin.
  - **Fix**: Either (a) shuffle/interleave the emitted quads across faces at the end of `FUN_0050ebc0` (1 pass over `totalQuads` swapping quad records of `0x30` pos + `0x30` norm + `0x20` uv0 + `0x20` uv1 bytes, or permuting face blade emission order), or (b) scale blade width/alpha by `1 / sqrt(keep)` while permuting blade order so thinned far bins preserve spatial coverage across all faces.
- **Path Probe Telemetry vs Doc/RFC Assumption (`RenderGrassPolys` `0x0050DAC0`)**:
  - Live `k2-grass-draw-distance_log.txt` reports: `path probe F=0x79FEFDFF G=1 H=1 P=1 Q=1 M=1 f07c=1 bac4=0`.
  - When `binFlags & 0x08` is set (lightmapped grass), `P=1` (`0x00A7D6D8`) and `f07c=1` (`0x00A7F07C`) route execution through **Path B2** (`0050dac0.c:212-255`), not Path A. Both Path A (`0x0050DF0D`) and Path B2 (`0x00436AE6`) are hooked by `K2GrassDrawArraysA/B`, so `far_lod` covers both, but `issue_rfc_grass_draw_distance_variety.md` line 116 ("only path A runs") should be updated to reflect that Path B2 runs for lightmapped bins (`binFlags & 0x08`).
- **Config vs Mod-Menu `wind_dist` Upper Bound Mismatch**:
  - `K2GrassDrawDistanceLogic.h:90` sets `kMaxWindDist = 200.0f` in `Config::sanitize()`, whereas `kPageSchema` (`K2GrassDrawDistanceLogic.h:803`) caps the `wind_dist` slider at `max = 90`. Opening Mod Configuration clamps any INI `wind_dist > 90` down to `90`.
- **Plan Doc & RFC Draft Version Drift**:
  - `patch_manager_mods/48_k2_grass_draw_distance.md` header still reads `0.1.2-dev` (90 m default) and lists `P3`/`P4`/`P4a`/`P4b`/`P4c` as `TODO`/`BUILT`.
  - `patch_manager_mods/new_issue_drafts/issue_rfc_grass_draw_distance_variety.md` reads `0.1.8-dev` with `vary_seed=0, even_spread=0`, whereas `5ca1c2bf` (`0.1.9-dev`) enabled both by default.

### 1.2 `k2-fog-distance` (`0.1.0-dev`) & Upstream Fog Patches
- **Same-Range Area Transition Collision in `FogTracker::update`**:
  - `FogTracker` (`K2FogDistanceLogic.h:168-212`) detects external updates to `Scene+0x64` (`fogStart`) and `Scene+0x68` (`fogEnd`) by checking `curStart != s->lastStart || curEnd != s->lastEnd`.
  - If Area A has vanilla fog `20..100 m` scaled at `200%` to `40..200 m` (`lastStart = 40, lastEnd = 200`), and Area B loads into the same `Scene*` with vanilla fog `40..200 m`, `curStart == s->lastStart && curEnd == s->lastEnd` evaluates true, so Area B's fog is mistaken for Area A's already-scaled value and stays at `40..200 m` instead of scaling to `80..400 m`.
  - **Fix**: Hook `Scene::SetFogRange` (`0x00464C80`) to record `origStart`/`origEnd` directly when the engine sets them, or invalidate the tracker on area change.
- **Fog-Off Area Clobber in `Scene::DoMeshBuckets` (`0x00466700`, `00466700.c:114-121, 163-170`)**:
  - KPM's `K2AspyrShaderFixes` (`0x0046AADD` in `Scene::DoFog`) sets `glFogf(GL_FOG_END, 1e6f)` when `Scene+0x60 == 0` (fog disabled) without modifying `*(float*)(Scene + 0x68)`, because Aspyr's ARB fragment shaders (`OPTION ARB_fog_linear`) ignore `glDisable(GL_FOG)` and still apply linear fog using `GL_FOG_START`/`GL_FOG_END`.
  - However, in `Scene::DoMeshBuckets` (`00466700.c:101-121` and `150-170`), whenever any mesh has a local visibility/fade multiplier `*(float*)(mesh + 0x94) < 1.0f`, the engine enables fog to fade that mesh and then restores `glFogf(GL_FOG_START, *(Scene + 0x64))` and `glFogf(GL_FOG_END, *(Scene + 0x68))`!
  - In a fog-off area (`Scene+0x60 == 0`), that restore overwrites `GL_FOG_END = 1e6f` with the raw `*(Scene + 0x68)` value for all remaining meshes in `DoMeshBuckets`, re-triggering the ARB shader fog bug on subsequent bumpmapped/envmapped meshes!
  - **Fix**: When `Scene+0x60 == 0`, either write `1e6f` into `Scene+0x68` (or restore `GL_FOG_END` to `1e6f` at `00466700.c:119, 168`).
- **`fog_end_pct = 0` ("No Fog" `0.99e6..1.0e6`) Interaction with Per-Mesh Fog Fade (`00466700.c:109-112`)**:
  - `DoMeshBuckets` fades meshes with `mult = *(float*)(mesh + 0x94) < 1.0f` by setting `GL_FOG_START = *(Scene+0x64) * mult` and `GL_FOG_END = *(Scene+0x68) * mult + 0.001f`.
  - When `k2-fog-distance` writes `990000..1000000` into `Scene+0x64/+0x68`, `Scene*mult` stays at hundreds of thousands of meters, neutralizing per-mesh fog fade for `0 < mult < 1`. Keeping the unscaled vanilla fog range for the `DoMeshBuckets` local fade multiplier (or clamping the base range used at `00466700.c:109-112` to `origStart`/`origEnd`) preserves mesh fade-outs while keeping scene fog disabled.

---

## 2. New Performance Optimizations Discovered in Engine RE

### 2.1 $O(N^2) \to O(N)$ Per-Frame Grass Bin Membership Diff (`0x004987E0`)
- **Location**: `FUN_004987e0` (`004987e0.c:21-50`, `004987e0.asm:0x00498806..0x004988A6`), called every frame from `Scene::CollectAndTraverse` (`0x00499610`).
- **Root Cause**:
  - `DAT_00a7330c` (`curBins`, count `DAT_00a73310`) holds the grass bins inside `g_collectSq` this frame; `DAT_00a73300` (`prevBins`, count `DAT_00a73304`) holds last frame's bins.
  - Lines 33–45 loop over every `prevBins[i]` (`0..prevCount`) and run an **unindexed inner linear scan** (`0x0049886B..0x00498893`) over all `curBins[0..curCount]` to check whether `prevBins[i]` is still in range, calling `CAurTriangleBin::DestroyGrassPolys` (`0x0050DA00`) if not found.
  - At vanilla 30 m ($N \approx 42$), $N^2 \approx 1,764$ iterations/frame. At 140–150 m ($N \approx 900\text{–}1,200$ bins), this becomes **$810,000\text{–}1,440,000$ pointer comparisons every single frame** on the CPU render thread.
- **Fix ($O(N)$ via unused flag bit in `bin+0x02`)**:
  - `CAurTriangleBin` flag byte at `bin+0x02` only uses bits `0x01` (built), `0x02` (AABB init), `0x04` (queued), `0x08` (lightmapped), and `0x10` (vertex-colored). Bit `0x80` is completely unused across the binary.
  - Pass 1 (`i = 0..curCount`): set `*(uint8_t*)(curBins[i] + 2) |= 0x80` and call `BuildGrassPolys(curBins[i])` (`0x0050D960`, which already early-outs in 3 instructions if `bit 0x01` is set).
  - Pass 2 (`i = 0..prevCount`): check `if ((*(uint8_t*)(prevBins[i] + 2) & 0x80) == 0) DestroyGrassPolys(prevBins[i]);`.
  - Pass 3 (`i = 0..curCount`): clear `*(uint8_t*)(curBins[i] + 2) &= ~0x80` and copy `curBins[i]` to `prevBins[i]`.
  - Eliminates ~1M loop iterations per frame on Dantooine/Dxun/Onderon.

### 2.2 Collect/Destroy Radius Hysteresis (`0x004986B0` / `0x004987E0`)
- **Location**: `FUN_004986b0` (`004986b0.c:21-26`) and `FUN_004987e0` (`004987e0.c:33-45`).
- **Root Cause**:
  - A single threshold `g_collectSq` both builds (`distSq < g_collectSq`) and immediately frees (`distSq >= g_collectSq` -> `DestroyGrassPolys` `0x0050DA00` -> `_free` + `FUN_00474030`) a bin's vertex arrays.
  - At 140 m radius, the perimeter circumference is $2\pi \times 140 \approx 880\text{ m}$ ($4.7\times$ larger than vanilla 30 m). Small camera movements or combat strafing step dozens of perimeter bins across the boundary repeatedly, triggering expensive `malloc` + `FUN_0050ebc0` blade generation + `free` churn.
- **Fix**:
  - In `FUN_004987e0` Pass 2, before calling `DestroyGrassPolys(prevBin)`, check the bin's squared distance to camera (`prevBin+0x2C..0x34` vs camera position) against a hysteresis destroy threshold `destroySq = (collectRadius + 8.0f)^2` (or retain built bins in `prevBins` until `distSq > destroySq`).

### 2.3 `UpdateGrassForWind` (`0x00510F70`) Per-Bin / Cell-Cached Wind Matrix & Point-Gust Short-Circuit
- **Location**: `CAurTriangleBin::UpdateGrassForWind` (`0x00510F70`, `00510f70.c:66-146`), `FUN_00507250` (`00507250.c:50-121`), and `FUN_005076c0` (`005076c0.c:32-71`).
- **Root Cause**:
  1. **Per-quad trig & $3\times 3$ rotation matrix**: For *every single grass quad* (`local_18 = 0..quadCount`), `00510f70` calls `FUN_00507250(&DAT_00a7d6fc, ...)` passing the quad's `(x, y)`. Inside `FUN_00507250`, `(x, y)` is quantized to a $16\times 16$ grid at 25 cm resolution (`(int)(x * 4.0f) & 0xF`, `(int)(y * 4.0f) & 0xF`), and then `00510f70` calls **4 trig functions** (`FUN_00513ce0` `cos`, `FUN_00513d20` `sin` twice at lines 92–95) and constructs a full **$3\times 3$ quaternion rotation matrix** (`lines 101–126`) for every blade!
  2. **Per-quad point-gust loop (`FUN_00507680` -> `FUN_005076c0`)**: Line 81 calls `FUN_00507680(&DAT_00a7d6fc, local_3c)` on every blade even when `*(int*)(0x00A7D70C) == 0` (`DAT_00a7d6fc + 0x10`, number of active local wind point gusts, which is `0` in almost all scenes).
- **Fix**:
  - Evaluated once per bin (using bin center `bin+0x2C..0x34`) or cached per quantized `(cellX, cellY)` (`((int)(x*4)&0xF) | (((int)(y*4)&0xF)<<4)` — only 256 possible values across the entire world!): compute `(windVec, rotMatrix[3][3])` at most 256 times per frame (or once per bin), and reduce the per-blade loop in `00510f70` to a simple 9-multiply matrix-vector product on the top 2 vertices.
  - Cuts CPU wind update cost by **10x–25x** while keeping **every-frame smooth 60+ Hz wind sway** (no `wind_throttle` frame-skipping stutter needed).

### 2.4 Hoisting Per-Face Lightmap / Vertex-Color Buffer Locks Out of the Per-Blade Loop (`0x0050EBC0`)
- **Location**: `CAurTriangleBin::CreateGrassArrays` (`0x0050EBC0`, `0050ebc0.c:219-330`).
- **Root Cause**:
  - Inside the inner per-blade loop (`for (blade = 0; blade < faceBladeCount; ++blade)`), lines 220 & 307 (`binFlags & 0x10` vertex color path) and lines 311–330 (`binFlags & 0x08` lightmap UV path) call `FUN_00432ee0` (lock/get mesh attribute pointer) and `FUN_00432f10` (unlock) and recompute the triangle's 3 vertex attribute pointers (`*(short*)(face + 0x1a/0x1c/0x1e) * 8 + base`) **on every single blade** on the same triangle face!
- **Fix**:
  - Hoist the 3 vertex color / lightmap UV lookups out of the `local_f8` blade loop into the `local_28` face loop. Speeds up initial area load and boundary bin builds.

### 2.5 Per-Bucket GL State Hoist in `Scene::DoBinBucket` (`0x004664E0` / `0x0050DAC0`)
- **Location**: `CAurTriangleBin::RenderGrassPolys` (`0x0050DAC0`, `0050dac0.c:101-139`).
- **Root Cause**:
  - Live log confirms `G=1 H=1 P=1 Q=1 M=1 f07c=1 bac4=0`. For every visible bin (~400–600 bins/frame at 150 m), `RenderGrassPolys` redundantly calls `glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ...)` (`0x0050DC2B`), `glMaterialfv(..., GL_DIFFUSE, ...)` (`0x0050DC3C`), `glMaterialfv(..., GL_SPECULAR, ...)` (`0x0050DC62`), and `glEnableClientState` x3 (`0x0050DBFA..0x0050DC10`) with identical constants.
  - Furthermore, `RenderGrassPolys` never calls `glDisableClientState` for `GL_VERTEX_ARRAY`, `GL_NORMAL_ARRAY`, or unit-0 `GL_TEXTURE_COORD_ARRAY` on exit (only unit-1 `GL_TEXTURE_COORD_ARRAY` is disabled at `0050dac0.c:204, 249`).

---

## 3. New Bug Fixes & Visual Diversity Improvements Discovered

### 3.1 K2 Port of K1 `GrassRenderFix` Path B1 Vertex Pointer Bug (`0x0050E47E` in `0050dac0.c:176`)
- **Location**: `CAurTriangleBin::RenderGrassPolys` (`0x0050DAC0`, `0050dac0.c:176`).
- **Root Cause**:
  - In Path A (`0050dac0.c:94`) and Path B2 (`0050dac0.c:234`), `FUN_004369a0` is passed `*(undefined4*)(param_1 + 0x1c)` (`MOV EAX, [ESI+0x38]`, the heap-allocated grass vertex buffer).
  - In Path B1 (`0050dac0.c:176`, taken when `binFlags & 0x08` is set and `FUN_00484480() == 0`), the compiler emitted `param_1 + 0x20` (`LEA EAX, [ESI+0x40]`, the address of the struct field `+0x40` inside `CAurTriangleBin`!) instead of `*(undefined4*)(param_1 + 0x1c)` (`MOV EAX, [ESI+0x38]`).
  - This is the exact K2 counterpart of K1's `GrassRenderFix` (`0x004A7114`: `8D 45 40` -> `8B 45 38`).

### 3.2 K2 `DestroyGrassPolys` (`0x0050DA00`) Double-Free Safety Guard (K1 `GrassMemorySafety` Counterpart)
- **Location**: `CAurTriangleBin::DestroyGrassPolys` (`0x0050DA00`, `0050da00.c:8-11`).
- **Root Cause**:
  - `0050ebc0.c:121` and `0050dac0.c:78` alias `*(void**)(bin + 0x3c) = *(void**)(bin + 0x38)` as a write cursor during vertex generation and zero `+0x3c` on completion (`0050ebc0.c:151`, `0050dac0.c:81`). If an exception or interrupted build leaves `+0x3c` non-null (pointing into `+0x38`), `DestroyGrassPolys` calls `_free(*(void**)(bin + 0x3c))` followed by `_free(*(void**)(bin + 0x38))`, causing heap corruption/crash.

### 3.3 Stochastic Fractional Blade Rounding (`0x0050D35C` in `CAurTriangleBin::AddTriangle`)
- **Location**: `CAurTriangleBin::AddTriangle` (`0x0050C8A0`, `0050c8a0.c:355-358`, `FRNDINT` at `0x0050D35C`).
- **Root Cause**:
  - Vanilla computes `bladeCount = (int)ROUND(triangleArea * density)` (`FRNDINT`).
  - On finely tessellated terrain meshes (slopes, pathways, area seams) where `triangleArea * density < 0.5`, `ROUND` deterministically produces `0` for every triangle in the strip (`if (local_18 != 0)` skips adding the triangle to the grass bin completely!), creating artificial geometric bald strips on high-detail terrain even when `density > 0`.
  - Conversely, triangles with `0.51 <= area * density < 1.0` deterministically round up to `1` (up to $2\times$ over-dense).
- **Fix (Unbiased Stochastic Rounding)**:
  - Even when `density_noise` (`PV3`, 6 m blotchy value noise) is OFF, replacing banker's `FRNDINT` at `0x0050D35C` with deterministic hash-dithered stochastic rounding `floor(x + u01_from_centroid_hash)` guarantees $\mathbb{E}[\text{blades}] = \text{area} \times \text{density}$ across all triangle sizes, eliminating small-triangle bald seams without altering macro grass density.

### 3.4 Per-Blade Normal Tilt & Independent Height/Width Jitter (`0x0050EBC0`)
- **Location**: `CAurTriangleBin::CreateGrassArrays` (`0x0050EBC0`, `0050ebc0.c:195-210, 349-369`).
- **Root Cause**:
  1. **Uniform X/Y/Z scaling (`local_108` at `0050ebc0.c:195-210`)**: `PV4` (`size_var`) scales `local_108 = (r - 0.5f) * span + 1.0f`, which multiplies `local_64` (blade width vector X,Y) AND `local_54` (blade height Z) by the exact same factor, and only on the **top two vertices** of the quad (`v1, v2`), while the bottom two vertices (`v0, v3`) use unscaled `local_20..local_14`! When `size_var` is large (`> 0.6`), scaling `local_64` on the top two vertices while leaving the bottom two vertices unscaled turns rectangular grass quads into **trapezoids** (fanning outward when `local_108 > 1` or pinching into triangles when `local_108 < 1`). Scaling height (`local_58..local_50`) independently from width (or applying width scale to both top and bottom vertex pairs `local_20` and `local_c0`) avoids trapezoidal distortion and allows tall/slender vs short/wide blade variety.
  2. **Identical Flat Face Normals (`0050ebc0.c:349-369`)**: All 4 vertices of every grass blade on a triangle copy the exact same underlying terrain face normal (`*local_c4`). Under directional area lighting (`GL_LIGHTING` enabled in `DoBinBucket`), every blade on a flat triangle has identical diffuse shading, flattening depth perception in dense fields. Adding a small deterministic per-blade normal tilt (`±0.15` XY from blade orientation `local_64`) gives individual blades subtle sunlit/shaded contrast at zero runtime cost (computed once at bin build).

---

## Verification (2026-10-29, Opus + 3 Sonnet skeptics, dumps steam_6a522e71/decomp)

| # | Claim | Verdict | Evidence / impact |
|---|---|---|---|
| 1.2 | far_lod prefix drops whole faces | CONFIRMED | buffer is face-major (0050ebc0.c:128-153, 541/560); already noted at K2GrassDrawDistance.cpp:557; off by default, unplayed |
| 1.3 | size_var trapezoids | REFUTED as bug | vanilla shape (top 2 verts scaled, 0050ebc0.c:403/448 vs 493/541); we only widen the range |
| 1.4a | wind_dist ini 200 vs menu 90 | CONFIRMED | Logic.h:89 vs :803; minor |
| 1.4b | path B2 runs live | REFUTED | 10-28 log PERF2 `pathA=65442 pathB1=0 pathB2=0` every window |
| 1.5a | fog same-range collision | PLAUSIBLE, low | K2FogDistanceLogic.h:100-122 keys on Scene* + last written values; needs exact float match; Scene* reuse unchecked |
| 1.5b | fog-off END clobber | PLAUSIBLE, not our mod | restore is unconditional (00466700.c:114-120); vanilla x KPM shader fixes; needs live probe |
| 2.1 | O(N^2) bin diff 004987e0 | CONFIRMED, overstated | N^2/2 with early break, every frame; ~0.05 ms at 300 bins, ~0.45 ms at 900 |
| 2.2 | wind cell cache 10-25x | REFUTED | per-blade input + time blend; cache changes visuals; wind already limited to 20 m |
| 2.3 | collect/destroy hysteresis | CONFIRMED, spikes only | 004987e0.c:19-26 destroys same frame; ~0.03-0.08 ms avg |
| 2.4 | per-blade lock/unlock | CONFIRMED, build-time only | 0050ebc0.c:220/307/312/329 |
| 2.5 | identical glMaterialfv per bin | REFUTED | alpha + material index vary per bin (0050dac0.c:66-75); client-state part ~0.05-0.15 ms |
| 3.1 | B1 vertex pointer bug (K1 GrassRenderFix in K2) | CONFIRMED bytes, unreached | 0050dac0.asm 0x0050E483 `ADD ECX,0x40`; live pathB1=0 |
| 3.2 | DestroyGrassPolys double free | REFUTED | bit 4 set only after +0x3c zeroed (0050ebc0.c:150-153, 0050da00.c:6-17) |
| 3.3 | FRNDINT bald strips | REFUTED as bug | normal vanilla rounding; density_noise stochastic rounding covers it when on |
| 3.4 | independent jitter + normal tilt | idea only | not verified; taste |
