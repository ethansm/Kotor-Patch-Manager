# RFC: Extended Grass Draw Distance, Grass Cost Controls and Grass Variety for KotOR 2 (`k2-grass-draw-distance`)

**Type:** RFC / Feature Proposal  
**Target Component:** new `Patches/K2GrassDrawDistance`; optional menu page through `Patches/ModConfig` (Mod Configuration menu RFC)  
**Current Status:** `k2-grass-draw-distance` `0.1.8-dev` is running in my live install. Draw distance, fade and wind settings have been played and checked in the logs (150 m chosen as the default after a cost run). The cost controls (0.1.4-0.1.5) and the variety options (0.1.6-0.1.8) are installed but still waiting for a play pass in a grassy area  
**Platform:** Steam/Aspyr (`kotor2_steam_aspyr`, `6A522E71...`) only, played under Proton on Linux. No GOG, Linux-native or macOS hooks yet  
**Related Discussion:** Sits next to `K2GrassFogFix` (no shared bytes, see Co-Existence). A separate RFC covers the other render and memory performance patches found during the same work (shadow distance, frame pacing, engine call caching, `K2TextureMemoryRelease`).

---

### Motivation & Background

KotOR 2 grass stops about 25 m from the camera. The engine fades it between 15 m and 30 m, but the alpha test (`glAlphaFunc(GL_GREATER, 0.40)`) clips the fading blades before the fade ends. On Dantooine and Onderon there is a visible line where the grass ends. On a modern GPU the grass pass costs very little at that range.

The grass that is drawn also looks uniform. The same clumps repeat across neighbouring triangles, and blades bunch toward the middle of each triangle.

`k2-grass-draw-distance` does three things:
1. Moves the grass draw distance and fade (default 120 to 150 m), with wind kept to a short radius.
2. Adds switches that lower the cost of far grass, each one measured in the log.
3. Adds optional variety: a better per-triangle seed, an even blade spread, density patches and a wider size range.

Every new behaviour beyond the distance itself is **off by default** (or set to the vanilla value), so the patch with default settings only changes how far grass reaches.

---

<!-- SCREENSHOT PLACEHOLDER 1 -->
<!-- ![Vanilla vs 150 m grass](screenshots/grass_vanilla_vs_150.png) -->
> *Screenshot Placeholder 1: The same Dantooine view, vanilla (grass ends about 25 m out) vs this patch at 150 m.*

---

### Key Features & Design

#### Distance and fade
* **Draw distance (fade end), 10 to 150 m, default 150 m.** The engine uses one squared distance (`[0x00A0F074]`, vanilla 900 = 30 m) for two things: the octree query radius in `Scene::CollectAndTraverse` (`0x00499ADC`), which decides which grass bins get built, and the fade end in `CAurTriangleBin::RenderGrassPolys` (`0x0050DB4C`). The patch writes this value.
* **Fade start, default 120 m,** always at least 5 m below the draw distance. Vanilla `[0x00A0F070]` (225 = 15 m) is shared by the fade start and the CPU wind gate.
* **Wind radius, default 20 m, decoupled from the fade.** The wind gate's `FLD [0x00A0F070]` at `0x0050DCC8` gets its operand rewritten to the patch's own float, so wind keeps a short radius while the fade moves out. Without this, every bin out to the fade start would run the CPU wind update every frame.
* **Alpha cutoff** stays at the vanilla 0.40 by default; a lower value reaches further but makes near blades look fuzzy.

#### Cost controls (each one a switch, measured in the log)
* **Trim Hidden Grass:** with the alpha test, opaque texels clip at `sqrt(start² + (1 - cutoff)(end² - start²))`, which is 138.8 m for 120..150 m at 0.40. Bins past that point are collected, built and drawn but cannot show a pixel. This option sets the collect radius to that distance plus 2 m.
* **Wind Throttle:** bins past a near radius update their wind every k-th frame.
* **Far Grass Thinning:** far bins draw a prefix of their blades. Only the count pushed to `glDrawArrays` is changed; the bin's own count field (read elsewhere for the normals offset) is left alone.
* **Smooth Grass Edges:** alpha-to-coverage around the grass bin loop, only when the context has multisample buffers.
* **Timing:** CPU time of the grass bin loop (QPC) and GPU time (`GL_TIME_ELAPSED`, if the extension is there) go into a `PERF` line every 5 s, so each switch's cost or gain can be compared in the log.

#### Variety (all off by default)
* **Varied Grass Pattern:** the engine seeds each triangle with `srand(round(x0 + y1 + z2))`. Many triangles share the same rounded sum, so the same clump layout repeats. The patch replaces the seed (stack slot before `srand`, hook at `0x0050EF89`) with a hash of the 9 vertex floats. It is deterministic: a triangle rebuilds the same way every time.
* **Even Grass Spread:** the blade position weights come from two independent `rand()` values, which bunches blades toward the triangle's centre. The patch reflects the pair into a uniform barycentric sample (hook at `0x0050F177`). The weights still lie in [0, 1] and sum to 1, because they also drive vertex colour and lightmap UV.
* **Grass Density Patches:** the blade count per triangle is scaled by a smooth value noise over world position and then rounded stochastically, so the average density stays the same but thick and thin patches appear (hook at `0x0050D35C` in `CAurTriangleBin::AddTriangle`, on the count before the engine's own `FRNDINT`).
* **Grass Size Variation:** the engine's blade scale jitter is `1 + (r - 0.5) * 0.4`. The patch points that `FMUL` operand (`0x0050FEE7`) at its own value, from 0.4 (vanilla) up to 1.2.
* The variety options change how grass is built, not how it is drawn, so they apply to grass built after the change (reload the save, or leave the area and come back).

#### Configuration
* `k2_grass_draw_distance.ini`, hot-reloaded about once a second.
* If `mod-config` is installed, a 13-row "K2 Grass Draw Distance" page appears under Options -> Mod Configuration. Lowering the draw distance moves the fade start slider down with it.
* Every byte change goes through a checked write: the expected bytes are compared first, and a mismatch logs a warning and leaves that feature off. All operand rewrites are restored on unload.

---

<!-- SCREENSHOT PLACEHOLDER 2 -->
<!-- ![Grass variety options](screenshots/grass_variety_on_off.png) -->
> *Screenshot Placeholder 2: The same field with the variety options off and on (repeated clumps vs patchy, varied grass).*

<!-- SCREENSHOT PLACEHOLDER 3 -->
<!-- ![Mod Configuration page](screenshots/grass_modconfig_page.png) -->
> *Screenshot Placeholder 3: The K2 Grass Draw Distance page in Options -> Mod Configuration.*

---

### Measurements So Far

The cost run at one spot on Dantooine (RTX 5070, 3840x1600, V-Sync on, Proton):

| Draw distance | Grass bins in range | fps |
|---|---|---|
| 30 m | 42 | 51-55 |
| 90 m | 441 | 60 |
| 120 m | 676 | 53-57 |
| 150 m | 895 | 57-59 |

The fps barely followed the bin count (correlation -0.36 across the session). The same bin counts ran at 58-67 fps early in one session and 37-45 fps later, so grass is not the main frame cost on this machine. That is why every cost control is measured on its own before any becomes a default.

A later session at 150 m with all variety options on (314-492 bins, V-Sync 60) measured the grass pass at 0.45-1.8 ms CPU and 0.04-0.14 ms GPU per frame, at 52-60 fps. Density Patches changed the total blade count of a load by +3.9% (13,660 -> 14,189), close to the "same average" design.

---

### What Didn't Work: Dead Ends & Engine Traps

#### 1. Per-bin wind detour
* 0.1.0 decoupled wind with a detour inside the per-bin render call. At 90 m that is up to about 3,700 calls a frame.
* **The Fix:** rewrite the `FLD` operand at `0x0050DCC8` once at load instead. No per-frame cost.

#### 2. The "polygon count" global is shared
* `[0x00A73694]` looked like a grass bin counter, but about 10 renderers write it.
* **The Fix:** the log reads the previous frame's in-range bin list count (`[0x00A73304]`) instead.

#### 3. Mirroring blades is redundant
* A planned "flip half the blades" option does nothing visible: each blade already gets a uniform random yaw over 0 to 2π, face culling is off for grass, and the normals are not rotated, so a flip at yaw t looks the same as yaw t+π.
* It was replaced by the wider size range.

#### 4. Thinning must not touch the bin's count field
* The bin's quad count is read for both the draw and the normals offset. Changing it breaks the normals.
* **The Fix:** Far Grass Thinning changes only the count pushed to `glDrawArrays`.

#### 5. Menu time inflates the numbers
* While a menu is open the grass pass does not run, so a 5 s window that spans a menu reads 0.1 to 23 fps.
* **The Fix:** the log drops a window after a grass gap of more than 1 s.

---

### Co-Existence With Existing Patches

* **`K2GrassFogFix`:** owns `0x004665D8..0x004665F5` in `Scene::DoBinBucket`. This patch does not touch that range; its bin-loop timing detours are at `0x0046661B` and `0x00466698`.
* **KPM shader fixes:** own `0x0046AADD`; not touched.
* **K1 `GrassRenderFix` / `GrassMemorySafety`:** both are K1-only today. KotOR 2's `RenderGrassPolys` also has three draw paths. In the Steam K2 exe, path B1 has the same wrong vertex pointer as K1 (`ADD ECX,0x40` at `0x0050E483` pushes the address of field `+0x40`; paths A and B2 push the buffer at `+0x38`). The double free does not carry over: the bin's built flag is set only after the alias at `+0x3C` is cleared. This patch logs which path runs live. On my machine (NVIDIA, Proton) only path A runs (the backend-dispatcher path); B1 and B2 drew 0 times, so the B1 bug is real in the bytes but I have not seen it on screen.

---

### Verification Status

* **Played and passed:** banner and patch lines clean; grass to 150 m with a smooth fade; the distance/cost run above; the linked fade start slider.
* **Variety, played with an on/off comparison:** the grass looked better with the options on, at no measurable cost (grass pass 1.29-1.42 ms CPU with them off vs 1.31-1.32 ms on, at 492 bins). Varied Grass Pattern and Even Grass Spread stayed on in my install; Density Patches I left off.
* **Running in play, logs clean:** Trim Hidden Grass, Smooth Grass Edges, GPU timing.
* **Installed, not yet played:** Wind Throttle, Far Grass Thinning.
* **Not started:** no GOG or native Linux/macOS hooks.
* **Needs a port before a PR:** my local build uses shared headers that are not upstream yet (`CodePatch.h`, `IniKit.h`, `KpLog.h`, `FileWatch.h`, `SafeRead.h`, `ModConfigDecl.h`). The patch works without `mod-config` (ini only).

---

### Community Feedback & Discussion

1. **Default distance:** is 150 m too far for older GPUs? Should the default be lower (90 m) with 150 m as an option?
2. **Variety defaults:** if the play pass shows no artefacts, should Varied Grass Pattern and Even Grass Spread be on by default? They do not change the number of blades.
3. **Scope:** one grass patch with switches, or split it into "draw distance" and "grass variety"?
4. **K2 grass bugs:** the K1 `GrassRenderFix` vertex-pointer bug is present in K2's path B1 (see Co-Existence). Does anyone with a non-shader GPU path (older hardware) see stretched grass in KotOR 2? Should the one-instruction fix go into this patch or a separate `K2GrassRenderFix`?
5. **Native slider:** would a slider in the vanilla Graphics options be preferred over the Mod Configuration page? It is possible (`CSWGuiOptionsGraphics` has room under the Force speed checkbox), but not mapped yet.
