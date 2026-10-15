## Step 1: File Verification

* **`3a_shared_headers.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).




* **`3b_shared_tests_a.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).





---

## Step 2: Code Reviews

### `3a_shared_headers.txt`

**Omitted files from `3a_shared_headers.manifest.tsv`:**

* `patch_manager_mods/_shared_src/HudKitTextures.inc` (`omitted: over max_file_tokens`, 391,334 bytes)



#### Findings for `3a_shared_headers.txt`

1. **`patch_manager_mods/_shared_src/GuiKit.h` — `guikit::detail::guikitWheelCb**`
* **Anchor:** `clientToEngineF(ev->client_x, ev->client_y, ev->screen_w, ev->screen_h,`

* **What is wrong:** `clientToEngineF(int px, int py, int cw, int ch, float sw, float sh, float* x, float* y)` takes the window's **client rect size (`cw`, `ch`)** as its 3rd and 4th parameters (`*x = px * sw / cw; *y = py * sh / ch;`), with precondition `cw >= 1 && ch >= 1`. In `guikitWheelCb`, when an INI screen override (`s.iniW > 0 || s.iniH > 0`) is set, it passes `ev->screen_w` and `ev->screen_h` (the **engine** screen resolution, which can also be 0 if unread) as `cw` and `ch` instead of the OS client window dimensions:


1. When the OS window client size differs from `ev->screen_w` / `ev->screen_h` (e.g., DPI scaling or windowed mode), `ev->client_x * sw / ev->screen_w` divides client pixels by engine pixels, producing completely wrong coordinates so `s.inside(ex, ey)` misses the wheel rect.
2. If `ev->screen_w == 0` or `ev->screen_h == 0`, `clientToEngineF` divides by zero, yielding `NaN`/`Inf`.


3. Even when `s.iniW == s.ev->screen_w`, `ev->client_x * sw / ev->screen_w` simplifies to raw `ev->client_x` instead of scaling `ev->engine_x`!


* **Suggested fix:** Scale the already-mapped `ev->engine_x` / `ev->engine_y` by `(float)sw / ev->screen_w` when `ev->screen_w > 0 && ev->screen_h > 0`, or query the client rect via `g_gk.cursorClient`:
```cpp
if ((s.iniW > 0 || s.iniH > 0) && ev->screen_w > 0 && ev->screen_h > 0) {
    ex = ev->engine_x * static_cast<float>(sw) / static_cast<float>(ev->screen_w);
    ey = ev->engine_y * static_cast<float>(sh) / static_cast<float>(ev->screen_h);
}

```


* **Confidence:** certain


2. **`patch_manager_mods/_shared_src/GuiKit.h` — `guikit::detail::wr**`
* **Anchor:** `T cur; if (!rd<T>(base, off, &cur)) return false;`

* **What is wrong:** `wr<T>` checks only whether `base + off` is **readable** (`rd<T>` calls `saferead::probeRead`) before executing `*reinterpret_cast<T*>(base + off) = v;`. If a corrupted control or border pointer points into committed read-only memory (`.rdata` or `.text`), `rd<T>` returns `true` and the write triggers an unhandled `STATUS_ACCESS_VIOLATION` crash. `SafeRead.h` provides `saferead::probeWrite` (with a separate `t_wcache` so read-only cached regions never pass write checks) specifically for this purpose.


* **Suggested fix:** Use `saferead::probeWrite` in `guikit::detail::wr`:
```cpp
template <typename T> inline bool wr(uintptr_t base, int off, T v) {
    if (base == 0 || !saferead::probeWrite(SiteBase, reinterpret_cast<void*>(base + off), sizeof(T))) return false;
    *reinterpret_cast<T*>(base + off) = v;
    return true;
}

```


* **Confidence:** certain


3. **`patch_manager_mods/_shared_src/SafeRead.h` — `saferead::readAt**`
* **Anchor:** `if (base == 0 || !probeRead(site, reinterpret_cast<const void*>(base + off), sizeof(T))) return false;`

* **What is wrong:** On 32-bit builds (`uintptr_t` is 32-bit unsigned), when `base` is a corrupted high address (e.g. `0xFFFFF000`) and `off` is a positive offset such as `PauseInternal = 0x10078` in `ActQueue.h` or `Sub_ToTimer = 0x10048` in `Effects.h`, `base + off` wraps around 32-bit unsigned arithmetic to `0x0000F078` (or `>= 0x10000` for `base >= 0xFFFFFEFC`), bypassing `probeRead`'s `a < 0x10000` and `a + len < a` checks and reading from an unrelated low memory address. Similarly, `sceneview::gobSetScale` computes `static_cast<unsigned char*>(gob) + o` without checking for pointer wrap-around before calling `probeWrite`.


* **Suggested fix:** Check for signed/unsigned offset wrap-around in `readAt` before calling `probeRead`:
```cpp
template <typename T> bool readAt(unsigned site, uintptr_t base, int off, T* out) {
    uintptr_t addr = base + static_cast<intptr_t>(off);
    if (base == 0 || (off > 0 && addr < base) || (off < 0 && addr > base)) return false;
    if (!probeRead(site, reinterpret_cast<const void*>(addr), sizeof(T))) return false;
    *out = *reinterpret_cast<const T*>(addr);
    return true;
}

```


* **Confidence:** certain


4. **`patch_manager_mods/_shared_src/CodePatch.h` — `codepatch::writeChecked**`
* **Anchor:** `memcpy(seen, p, n);`

* **What is wrong:** `writeChecked` unconditionally reads `n` bytes from `at` (`memcpy(seen, p, n);`) **before** calling `ops().protect` or checking `committed(at, n)`. If the caller does not call `committed(at, n)` first, or if the target page is committed with `PAGE_NOACCESS`, `PAGE_GUARD`, or `PAGE_EXECUTE` (execute-only without read permission), `memcpy(seen, p, n)` faults immediately instead of returning `-2`. Additionally, if `n <= 0`, `write` and `writeChecked` pass `n` converted to `SIZE_T` (wrapping negative `n` to a huge `SIZE_T`) into `VirtualProtect` and `memcpy`.


* **Suggested fix:** Guard `n <= 0`, check `committed(at, n)` or run `ops().protect(p, n, PAGE_EXECUTE_READWRITE, &old)` before reading `p`, or probe readability before `memcpy(seen, p, n)`.


* **Confidence:** certain


5. **`patch_manager_mods/_shared_src/HudKit.h` — `hudkit::image**`
* **Anchor:** `if (!cache[id]) cache[id] = guikit::imageFor(detail::kHudKitTextures[id].resref);`

* **What is wrong:** As documented in `HudKit.h`'s own comment (`guikit::imageFor pools a failed lookup too`) and implemented in `GuiKit.h` (`imageFor` appends `{ buf, im }` to its static `pool` even when `im == nullptr` and returns `pool[i].im` on every subsequent call for that `resref`), calling `hudkit::image(id)` (or `guikit::imageFor(resref)`) once when `g_gk.loadImage` returns `nullptr` permanently caches `nullptr` in `guikit::imageFor`'s pool. Even though `hudkit::image` checks `if (!cache[id])` to retry when `cache[id]` is null, every retry calls `guikit::imageFor`, which hits the cached `nullptr` entry in `pool` and never invokes `g_gk.loadImage` again for the lifetime of the process!


* **Suggested fix:** Either do not cache failed (`nullptr`) lookups in `guikit::imageFor` (or add a `retryIfNull` parameter to `guikit::imageFor`), so `hudkit::image` can recover once `hudkit::ensure()` / `embtex::heal()` has registered the texture.


* **Confidence:** certain


6. **`patch_manager_mods/_shared_src/EmbedTex.h` — `embtex::heal**`
* **Anchor:** `if (s.heals == MaxHeals) EMBTEX_LOG("tex %s: giving up after %d failed refills (built=%d missing=%d pixels=%p); art missing, labels still work",`

* **What is wrong:** In `embtex::heal()`, `++s.heals` is incremented *before* `detail::fill(s, ...)` is called, and `if (s.heals == MaxHeals)` is checked immediately at the bottom of the same loop iteration. On the 20th refill (`s.heals` transitions from 19 to 20), even when `detail::fill` succeeds (`b[Tex_Constructed] == 1` and `b[Tex_Pixels] != nullptr`, queued for upload on the next engine texture tick), `heal()` immediately logs `"giving up after 20 failed refills ... art missing"`. Furthermore, if a GL reset occurs *after* a texture has been healed 20 times across the entire play session (wait: `s.heals = 0` resets on healthy upload, except if the engine resets 20 times in a row before the texture tick runs—e.g. 20 UI ticks during a loading/alt-tab stall before the texture upload tick drains `pendingList`—`heal()` will enqueue the same `CAurTextureBasic*` 20 times into `pendingList` because `!built && !px` is false once filled, wait: for `built && !px && (b[Tex_Missing] || w != s.w)`, `fill` sets `px != nullptr`, so next tick `px` is non-null until consumed; if consumed and reset 20 times, the 20th successful refill still logs the false give-up message).


* **Suggested fix:** Only log the give-up message when `detail::fill` fails on the `MaxHeals`-th attempt, or log when `s.heals == MaxHeals` is reached on a subsequent tick while the texture is still unhealthy (incrementing `s.heals` to `MaxHeals + 1` so it logs only once).


* **Confidence:** certain


7. **`patch_manager_mods/_shared_src/GuiKit.h` — `guikit::createLabelFromTemplate` and `guikit::bindByTag**`
* **Anchor:** `if (!rdInt(c, oId, &id) || id == IdSentinel) { if (why) *why = CF_Template; return 0; }   // template not in the layout`

* **What is wrong:** In `createLabelFromTemplate` (`addToList = 0`), `g_gk.alloc(labelSize)` allocates 328 bytes (`0x148`) on the engine heap and calls `g_gk.labelCtor(mem)`. If the vtable check (`CF_Vtable`), template tag load (`CF_Template`), panel count read (`CF_Count`), or flags read (`CF_Flags`) fails, `createLabelFromTemplate` returns `0` without destroying or freeing `mem` (and since `addToList = 0`, the panel does not own it either). Similarly, in `bindByTag`, if `CF_Vtable` fails before `initControl`, `mem` is leaked.


* **Suggested fix:** Destroy and free `mem` when `createLabelFromTemplate` fails (or when `bindByTag` fails before `initControl`).


* **Confidence:** certain


8. **`patch_manager_mods/_shared_src/WindowHook.h` — `kp::WindowHook::install` and `patch_manager_mods/_shared_src/GuiKit.h` — `guikit::wheelInstall**`
* **Anchor:** `if (detail::WheelWin::old) { detail::WheelWin::install(detail::WheelWin::hwnd, detail::wheelProc); return true; }`

* **What is wrong:** Once `WindowHook::install` succeeds for an initial `HWND`, `WindowHook::old` and `WindowHook::hwnd` remain non-null unless `uninstall` is called while our proc is still on top of the chain. If the game recreates its main window (e.g., on fullscreen/resolution change), the old `hwnd` is destroyed (`!whapi::isWindow(hwnd)`), yet `wheelInstall()` sees `if (detail::WheelWin::old)` and re-passes the stale `detail::WheelWin::hwnd` to `WindowHook::install`, which sees `if (old)` and returns `true` without ever checking `whapi::isWindow(hwnd)` or subclassing the new window.


* **Suggested fix:** In `WindowHook::install(HWND h, WNDPROC proc)`, if `old` is set but `!whapi::isWindow(hwnd)` or `h != hwnd`, reset `old = nullptr; hwnd = HWND(); passthrough = 0;` and subclass the new `h`; in `guikit::wheelInstall()`, check `detail::WheelWin::old && IsWindow(detail::WheelWin::hwnd)` before taking the fast path.


* **Confidence:** certain


9. **`patch_manager_mods/_shared_src/GuiInput.h` — `k2input::Client::connect` and `patch_manager_mods/_shared_src/GuiKit.h` — `guikit::wheelInstall**`
* **Anchor:** `if (!subOk) { int32_t tok = g_gkInput.wheel_subscribe(detail::guikitWheelCb, nullptr); if (tok > 0) subOk = true; }`

* **What is wrong:** Two issues in the `GuiInput.h` / `GuiKit.h` integration:
1. In `GuiKit.h`, `wheelInstall()` stores `static bool subOk = false;`, which is never reset in `wheelUninstall()`. If `wheelUninstall()` is called (which calls `g_gkInput.wheel_unsubscribe(detail::guikitWheelCb)`) and `wheelInstall()` is called again later, `subOk` is still `true`, so `wheelInstall()` skips `wheel_subscribe` and returns `true` without re-subscribing!


2. In `GuiInput.h`, `k2input::Client::connect()` calls `onConnect(false, ...)` on **every** failed `poll()` attempt (every 256 calls) because `k2input::Client` lacks the `bool reportedMissing = false;` guard that `k2hud::Client`, `k2nw::Client`, `k2ps::Client`, `k2rai::Client`, and `k2se::Client` all have. Any consumer that sets `g_input.onConnect` will spam its log every 256 ticks when `gui-input` is not installed.




* **Suggested fix:** Make `subOk` a non-local variable (or member) that `wheelUninstall()` resets to `false`, and add `bool reportedMissing = false;` to `k2input::Client` matching the other registry clients.


* **Confidence:** certain


10. **`patch_manager_mods/_shared_src/EffectIcons.h` — `k2fx::collectBuffs` and `k2fx::iconForEffectIconEffect**`
* **Anchor:** `if (!rd(arr, i * 4, &eff) || !rd(eff, Effect_SpellId, &spell)) continue;`

* **What is wrong:** While `Effects.h` resolves every struct offset through `k2addr::off(&K2AddrTable::...)` and checks that each offset is non-zero before reading engine memory, `EffectIcons.h` bypasses `k2addr::off(...)` and passes the compile-time constants `Effect_SpellId`, `Effect_Type`, `Effect_IntCount`, and `Effect_Ints` directly to `rd(...)`. If a test or future table sets `Effect_SpellId = 0` or `Effect_Type = 0` in `K2AddrTable` to mark those offsets unknown (expecting `collectBuffs` / `iconForEffectIconEffect` to be inert like `scanShieldPools`), `EffectIcons.h` ignores the table and reads using the Steam constants anyway. Additionally, if `count` on a modified creature were to exceed `MaxEffects` (currently gated by `effectListNonEmpty`), `seen[nSeen++] = spell;` has no `nSeen < MaxEffects` bounds check unlike `if (nIcon < MaxEffects) iconSeen[nIcon++] = resref;` on the same line.


* **Suggested fix:** Read `Effect_SpellId`, `Effect_Type`, `Effect_IntCount`, and `Effect_Ints` via `k2addr::off(&K2AddrTable::...)` in `iconForEffectIconEffect` and `collectBuffs` (returning `nullptr` / `0` if any needed offset is 0), matching `scanShieldPools` in `Effects.h`.


* **Confidence:** certain


11. **`patch_manager_mods/_shared_src/Party.h` — `party::table`, `party::partyCount`, and `party::vitals**`
* **Anchor:** `saferead::readAt<uint32_t>(SiteBase, vt, SlotMaxHp, &fMax) && fMax &&`

* **What is wrong:** `K2AddrTable` in `K2Addr.h` defines `Creature_VtMaxHp = 0x98` and `Creature_VtCurHp = 0x9c` specifically for the server creature HP vtable slots, but `party::vitals` ignores `k2addr::off(&K2AddrTable::Creature_VtMaxHp)` / `Creature_VtCurHp` and uses hardcoded `SlotMaxHp = 0x98` and `SlotCurHp = 0x9C`. Similarly, `party::table` and `party::partyCount` do not check `k2addr::get() != nullptr` when `g_appGlobal` is overridden in tests, nor do they read the vtable offsets from `K2AddrTable`, so zeroing `Creature_VtMaxHp` or `Creature_VtCurHp` in `K2AddrTable` does not make `party::vitals` inert.


* **Suggested fix:** Read `const int oMax = k2addr::off(&K2AddrTable::Creature_VtMaxHp), oCur = k2addr::off(&K2AddrTable::Creature_VtCurHp);` in `party::vitals` and require `oMax && oCur` before reading the vtable slots.


* **Confidence:** certain



#### Files or Areas Not Assessed for `3a_shared_headers.txt`

* `patch_manager_mods/_shared_src/HudKitTextures.inc`: Omitted from the bundle (`over max_file_tokens`, 391,334 bytes).


* `kpatch_api.h`: Included by `GuiInput.h`, `HudHost.h`, `NpcWorld.h`, `PortraitSlots.h`, `RangedAI.h`, and `SaveEvents.h`, but not included in the bundle.


* `../../research/companion_levelup/tools/parity/stub/windows.h`: Host stub header referenced in `test.toml` and `CodePatch.h`, not included in the bundle.



---

### `3b_shared_tests_a.txt`

**Omitted files from `3b_shared_tests_a.manifest.tsv`:**

* `patch_manager_mods/_shared_src/GuiKit.h` (`manifest-omit`, 0 bytes; present in `3a_shared_headers.txt`)


* `patch_manager_mods/_shared_src/K2Addr.h` (`manifest-omit`, 0 bytes; present in `3a_shared_headers.txt`)



#### Findings for `3b_shared_tests_a.txt`

1. **`patch_manager_mods/_shared_src/guikit_test.cpp` — `main**`
* **Anchor:** `// Host test of the GuiKit.h additions: imageFor out-params / 256 pool, createLabelFromTemplate failure reason.`

* **What is wrong:** Nine public functions in `GuiKit.h` (`appendToPanel`, `setRect`, `setFill`, `setAlpha`, `setText`, `setFont`, `setAlign`, `getTextColor`, ` setTextColor`, and `playHoverSound` when `guiSoundMgrAddr != 0`) are **never called on a success path (or at all)** in `guikit_test.cpp` (nor in `k2addr_test.cpp`, which only calls `setRect`, `setAlpha`, and `setText` with a `nullptr` table where they immediately return `false`). If `appendToPanel` wrote the wrong control ID or array offset, if `setFill` wrote to the wrong border offset, if `setText`/`setFont`/`setAlign`/`setTextColor`/`getTextColor` computed the wrong `textSub(ctl)` offset, or if `playHoverSound` passed the wrong sound index, `guikit_test.cpp` and `k2addr_test.cpp` would still pass 100%.


* **Suggested fix:** Add host test cases in `guikit_test.cpp` exercising the happy and failure paths of `appendToPanel`, `setRect`, `setFill` (with and without `tint`), `setAlpha`, `setText`, `setFont`, `setAlign`, `getTextColor`, `setTextColor`, and `playHoverSound` against the fake arena.


* **Confidence:** certain


2. **`patch_manager_mods/_shared_src/k2addr_test.cpp` — `checkSteamFields` and `checkGogFields**`
* **Anchor:** `static void checkSteamFields(const K2AddrTable& t) {`

* **What is wrong:** `checkSteamFields` claims *"Every kSteam field against today's constant (literal list)"*, but it completely omits **25 fields** of `K2AddrTable`:


* `Border_FillStyle` (`0x34` on both `kSteam` and `kGog`) is omitted from both `checkSteamFields` and `checkGogFields`.


* Almost the entire `SceneView.h` block in `kSteam` (`GuiGateGlobal`, `GuiControlCtor`, `GuiSceneCtor`, `SceneAddModel`, `SceneRemoveModel`, `SceneGetModel`, `SceneViewVtable`, `SceneCameraVtable`, `GobVtable`, `GobSetPosition`, `GobSetOrientation`, `GobGetPosition`, `GobGetOrientation`, `GobGetParent`, `SceneViewSize`, `View_Scene`, `View_SceneObj`, `View_Camera`, `View_ModelCount`, `SceneVt_Init`, `GobVt_SetPosition`, `GobVt_SetOrientation`, `GobVt_GetPosition`, `GobVt_GetOrientation`, `GobVt_GetParent`, `GobVtReadable`, `GobReadable`, `Gob_Scale`) is omitted from `checkSteamFields`, and several of those (`GuiControlCtor`, `GuiSceneCtor`, `SceneRemoveModel`, `SceneGetModel`, `SceneViewVtable`, `SceneCameraVtable`, `GobSetPosition`, `GobSetOrientation`, `GobGetPosition`, `GobGetOrientation`, `GobGetParent`, `SceneViewSize`, `View_SceneObj`, `View_Camera`, `View_ModelCount`, `SceneVt_Init`, `GobVt_*`, `GobVtReadable`, `GobReadable`) are also omitted from the zero-checks in `checkGogFields`.


* Conversely, `STEAM(Menu_Char2, ...)` and `STEAM(Menu_Char3, ...)` are checked twice in `checkSteamFields`.
If `kGog.Border_FillStyle` or `kGog.SceneViewSize` were accidentally given a wrong value in `makeGog()`, `k2addr_test.cpp` could not fail.




* **Suggested fix:** Add `STEAM(Border_FillStyle, 0x34)` and the missing `SceneView.h` fields to `checkSteamFields`, and add `GOG(Border_FillStyle, 0x34)` plus zero-checks for the remaining `SceneView.h` fields to `checkGogFields`.


* **Confidence:** certain


3. **`patch_manager_mods/_shared_src/k2addr_test.cpp` — `checkEffects**`
* **Anchor:** `KT_CHECK_EQ((long long)k2fx::scanShieldPools(cr, pools, 2), 0ll);`

* **What is wrong:** `checkEffects` in `k2addr_test.cpp` tests `scanShieldPools` with `noCount.Creature_EffectCount = 0`, where `cr` already had `cnt == 0` (`at<int>(cr, 0x14c) = 0;`)! Because `scanShieldPools(cr, pools, 2)` already returns `0` on `cr` even with the full `kSteam` table (since `cnt == 0` fails `effectListNonEmpty`), that assertion cannot fail even if `scanShieldPools` ignored `K2AddrTable` completely. Furthermore, `checkEffects` never tests `k2fx::collectBuffs` or `k2fx::iconForEffectIconEffect` with a zeroed `K2AddrTable` field — which is why the bug in `EffectIcons.h` (Finding #10 in `3a_shared_headers.txt`, where `EffectIcons.h` hardcodes `Effect_SpellId`, `Effect_Type`, `Effect_IntCount`, and `Effect_Ints` instead of calling `k2addr::off`) went undetected!


* **Suggested fix:** Populate `cr` with a valid shield pool and buff effect first, verify `scanShieldPools(cr, ...)` and `collectBuffs(cr, ...)` return `1` under `kSteam`, and then verify both return `0` when individual `Effect_*` offsets in `K2AddrTable` are zeroed.


* **Confidence:** certain


4. **`patch_manager_mods/_shared_src/effects_test.cpp` — `main` (`iconForEffectIconEffect` item prefix check)**
* **Anchor:** `uintptr_t slowIcon = iconFx(28, -1, 1);`

* **What is wrong:** `iconForEffectIconEffect` in `EffectIcons.h` sets `*item = !strncmp(r, "ii_", 3) || !strncmp(r, "iw_", 3);` so that item/weapon-sourced type-`0x43` debuff icons (such as row 41 `"iw_stungren_001"` or row 55 `"ii_drdutldev_001"` in `EffectIconRowTable.inc`) are drawn on the round item-ability disc (`item == true`). However, `effects_test.cpp` only tests `iconFx` with row 28 (`"ip_slow"`, `item == false`) and row 5 (`"ip_droiddestroy"`, `item == false`), and never tests any `"ii_"` or `"iw_"` row in `EffectIconRowTable.inc`, nor a row where `kEffectIconRows[row].good == 0` and `resref == nullptr` (such as row 56 or 105). If the `*item = !strncmp(r, "ii_", 3) || !strncmp(r, "iw_", 3);` line or the `!kEffectIconRows[row].resref` null check in `iconForEffectIconEffect` were deleted, `effects_test.cpp` could not fail.


* **Suggested fix:** Add checks in `effects_test.cpp` for `iconFx(41, -1, 1)` (`"iw_stungren_001"`, `o[0].item == true`), `iconFx(55, -1, 1)` (`"ii_drdutldev_001"`, `o[0].item == true`), and `iconFx(56, -1, 1)` (`good == 0, resref == nullptr` $\to$ skipped, returns 0).


* **Confidence:** certain


5. **`patch_manager_mods/_shared_src/selfheal_ui_test.cpp` — `main` (`pendingDone2` with `now == 0`)**
* **Anchor:** `if (*unqueuedSince == 0) *unqueuedSince = now ? now : 1u;   // 0 is the "not running" sentinel`

* **What is wrong:** `pendingDone2` in `SelfHealUi.h` contains a special branch `now ? now : 1u` so that if a non-queued stretch begins at `now == 0`, `*unqueuedSince` is set to `1u` instead of remaining at the `0` sentinel. In `selfheal_ui_test.cpp`, `pendingDone2` is only ever called with `now >= 1100` (`1100`, `6999`, `7000`, `10000`, etc.) and never with `now == 0` when `*unqueuedSince == 0` and `queued == false`. If `now ? now : 1u` were replaced with `now`, the timer would never start at `now == 0` and `selfheal_ui_test.cpp` would still pass.


* **Suggested fix:** Add a test case calling `pendingDone2(p, 0, 10, 3, false, &u)` when `u == 0` and assert `KT_CHECK_EQ(u, 1u);`.


* **Confidence:** certain


6. **`patch_manager_mods/_shared_src/sceneview_test.cpp` — `main` (Default vtable-slot lambdas in `sceneview::Engine`)**
* **Anchor:** `void (*callSetPos)(uint32_t fn, void* gob, float x, float y, float z) = [](uint32_t fn, void* gob, float x, float y, float z) {`

* **What is wrong:** `sceneview_test.cpp` immediately replaces `g_sv.callSceneInit`, `g_sv.callSetPos`, `g_sv.callSetOri`, `g_sv.callGetVec`, `g_sv.callGetParent`, `g_sv.callCamAttach`, and `g_sv.callCamFov` in `installFakes()` before any scene/Gob/camera call is made, and never tests the default lambdas in `sceneview::Engine` (not even checking that they are non-null in `testRebind()`). If a default lambda in `SceneView.h` were initialized to `nullptr` or passed arguments in the wrong order, `sceneview_test.cpp` could not fail.


* **Suggested fix:** In `testRebind()`, assert that `e.callSceneInit`, `e.callSetPos`, `e.callSetOri`, `e.callGetVec`, `e.callGetParent`, `e.callCamAttach`, and `e.callCamFov` are non-null on a default-constructed `sceneview::Engine`.


* **Confidence:** certain



#### Files or Areas Not Assessed for `3b_shared_tests_a.txt`

* `patch_manager_mods/_shared_src/GuiKit.h` and `patch_manager_mods/_shared_src/K2Addr.h` were omitted from `3b_shared_tests_a.txt` (`manifest-omit`, 0 bytes), though both were available in `3a_shared_headers.txt`.


* `../../research/companion_levelup/tools/parity/stub/windows.h`: The host stub header included by `SafeRead.h` under `-DGUIKIT_HOST` is not included in the bundle.