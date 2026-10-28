# RFC: KotOR 2 (Steam) performance patches: texture memory release, frame pacing, shadow distance, engine call caching

**Type:** RFC / packaging question for a set of small performance patches  
**Target:** KOTOR II Steam (Aspyr), `kotor2_steam_aspyr` (`6A522E71...`) only. Developed and played under Proton on Linux  
**Patches:** `K2TextureMemoryRelease`, `K2FramePacing`, `K2ShadowDistance`, `K2EnginePerf`  
**Related Discussion:** Companion to the grass RFC (`k2-grass-draw-distance`), which came out of the same performance work. Settings pages use the Mod Configuration menu RFC where present; every patch also works from its ini alone.

---

### Status

1. **`K2TextureMemoryRelease` is ready for a PR.** Version 0.2.0 is a hooks-only patch (one replace hook, no DLL). It has been on in my live install since 2026-10-15, across many play sessions.
2. **The other three are built and installed in my live install, but are young** (0.1.x, installed 2026-10-25/26). Each logs a `PERF` line every 5 s so its effect can be measured. Their dedicated play checks are still pending, except the `K2EnginePerf` cache, which is measured below.
3. **Question for maintainers:** one bundle, or separate patches? See "Questions" at the bottom.

---

### Is there already something like this upstream?

* `FrameLimiter` caps the frame rate, but it is **K1 only** and needs V-Sync off. Nothing for K2 handles a minimized or background window.
* `SaveGameMemoryLeak`, `TextureBucketSafety` and `GrassMemorySafety` are memory fixes for K1. I found no K2 patch that reduces texture memory.
* `SoftShadowsFix` is K1 only and about enabling soft shadows, not distance.

---

### The Patches

#### 1. K2TextureMemoryRelease (ready)
* **Problem:** `BuildAndStoreTexture` (`0x004260E0`) keeps the CPU-side copy of every TPC image after uploading it to OpenGL. I measured about 195 MB of these copies. TGA textures are already freed after upload. In a 32-bit process that is address space other mods and large areas could use.
* **Fix:** free the CPU copy after upload, as the engine does for TGA. Textures driven by a texture controller (animated or procedural) keep theirs, because the controller reads them every frame. Later use goes through the engine's own reload.
* **Seen live:** one session freed 2,038 TPC copies and kept 17 for controllers, with no warnings.
* Note: this does not purge OpenGL texture handles. It frees system-memory copies only.

#### 2. K2FramePacing
* **Problem:** when the game is minimized the main loop keeps spinning with no `SwapBuffers` and no `Sleep`, so it holds a CPU core at full use. In the background it keeps rendering at full rate.
* **Fix:** a detour at the end of the `WinMain` loop (`0x0040846E`):
  * minimized (`IsIconic`): `Sleep(50)` per loop;
  * not focused (`GetForegroundWindow` != the game window): cap at 30 fps;
  * optional fps cap when focused, using `Sleep` plus a short QPC spin (about 1.5 ms) so the cap is steady.
* Three settings (ini, or a Mod Configuration page). The `PERF` line reports fps, work time, and time slept and spun.

#### 3. K2ShadowDistance
* **What it does:** a slider for how far creature shadows are drawn. Vanilla cuts them at 35 m with two separate checks:
  * `[0x009F5D50]` (writable `.data`, 35.0) in `Scene::DoGobShadows` and `DoGobShadowsForThisLight`;
  * a second squared-distance check (≤ 1100, about 33.2 m) at `0x00468CDB`, read from `.rdata`, around the shadow-volume build.
* The patch writes the first and points the second `FLD` operand at its own value, scaled to match. Lower values save shadow-volume work in crowded areas; higher values look better.
* The log counts shadow casters per second, and records the live light-cap global `[0x00A0F040]`, which it only reads.

#### 4. K2EnginePerf
* **GlobalMemoryStatus cache (kept):** the engine calls `GlobalMemoryStatus` about twice a frame (call at `0x00478A57`). Under Wine each call reads `/proc/meminfo` and costs about 40 µs. The patch caches the result for 250 ms. Measured: 600 calls per 5 s, of which 20 still reach the real function; saves about 5 ms per second. On native Windows the call is likely much cheaper, so this one may matter mainly under Proton/Wine.
* **Texture-id list reuse (dropped):** the texture-id scan (`0x00427970`) looked like it ran 3 times a frame. The cache got 0 hits in 139 log windows: it runs about once a frame (5.5 µs), so there is nothing to save. It is being removed in the next build; I'm reporting it so nobody else spends time on it.

---

<!-- SCREENSHOT PLACEHOLDER 1 -->
<!-- ![Shadow distance 35 vs 70 m](screenshots/shadow_distance_35_70.png) -->
> *Screenshot Placeholder 1: The same street with Shadow Distance 35 m (vanilla) and 70 m.*

---

### Things That Turned Out Not To Be Levers

These were checked in the Steam disassembly and dropped, so others can skip them:
* **glFlush:** the default render path issues none per frame; nothing flushes before `SwapBuffers`.
* **Camera far plane:** already 10,000 m in world scenes. Culling at the fog end would clip skyboxes, which are geometry.
* **Fog:** fixed-function GL fog per fragment, so there is nothing to save.
* **Particle emitters:** off-screen emitters already skip simulation (frustum list).
* **Room visibility:** VIS is applied every frame already; a distance cut on rooms would make geometry pop.

Ideas found but **not built**: skipping animation updates for creatures that are off-screen and far away (`Scene::Animate` `0x0046D230`; risky because of animation events and sounds), a per-bucket blend/depth state cache, and a camera-distance cut for particle emitters.

---

### Testing

* Steam (Aspyr) build `6A522E71...` only, Proton on Linux, RTX 5070. GOG, Windows native and Linux/macOS native are untested; the hooks files are Steam-only.
* All four have been installed alongside about 55 other patches. `K2TextureMemoryRelease` has the longest play history. The others still need their own play checks (minimize/background CPU use, shadow distance look, fps cap steadiness) before each PR.
* **Port needed before a PR (except texmem):** my local builds use shared headers that are not upstream yet (`CodePatch.h`, `IniKit.h`, `KpLog.h`, `FileWatch.h`, `SafeRead.h`; plus `ModConfigDecl.h` for the menu pages).

---

### Questions

1. **Bundle or separate?** I lean toward separate PRs, starting with `K2TextureMemoryRelease` because it is hooks-only and has the most play time. Would a single `K2Performance` patch with per-feature switches be preferred instead?
2. **Frame pacing vs `FrameLimiter`:** should `K2FramePacing` be a K2 port of `FrameLimiter` (same name and ini) with the minimized/background behaviour added, or a separate patch?
3. **Wine-only gains:** is a patch whose main benefit is under Proton/Wine (the `GlobalMemoryStatus` cache) welcome upstream, or should it stay local?
4. **Shadow distance:** a visual setting more than a fix. Does it belong here, or with the grass/fog distance patches?
5. **Measurements:** is there a preferred way to report performance numbers for PRs (a benchmark save and spot, frame time over N seconds)? I can follow it for all of these.
