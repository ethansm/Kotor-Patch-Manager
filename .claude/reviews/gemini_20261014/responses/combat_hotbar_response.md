1. **File path and function name:** `patch_manager_mods/_shared_src/GuiKit.h` — `guikit::detail::wr`

* **Anchor:** `T cur; if (!rd<T>(base, off, &cur)) return false;`

* **What is wrong:** `wr<T>` checks only whether the destination address is readable (`rd<T>` calls `saferead::probeRead`) before performing a direct memory store (`*reinterpret_cast<T*>(base + off) = v;`). If a corrupted or uninitialized pointer points into read-only committed memory (such as `.rdata` or `.text`), `rd<T>` succeeds and the subsequent store triggers an unhandled `STATUS_ACCESS_VIOLATION` crash. `SafeRead.h` explicitly provides `saferead::probeWrite` with a separate writable-region cache (`t_wcache`) for this exact reason.


* **Suggested fix:** Probe writability instead of readability:
```cpp
template <typename T> inline bool wr(uintptr_t base, int off, T v) {
    if (base == 0 || !saferead::probeWrite(SiteBase, reinterpret_cast<void*>(base + off), sizeof(T))) return false;
    *reinterpret_cast<T*>(base + off) = v;
    return true;
}

```


* **Confidence:** certain


2. **File path and function name:** `patch_manager_mods/_shared_src/GuiKit.h` — `guikit::createLabelFromTemplate` and `guikit::bindByTag`

* **Anchor:** `if (!rd32(c, 0, &vt) || vt != labelVt) { if (why) *why = CF_Vtable; if (badVtable) *badVtable = vt; return 0; }`

* **What is wrong:** In `createLabelFromTemplate` (before `initControl` is called with `addToList = 0`) and in `bindByTag` (before `initControl` is called with `addToList = 1`), `g_gk.alloc(labelSize)` allocates 328 bytes (`0x148`) on the engine heap and runs `g_gk.labelCtor(mem)`. If any of the subsequent checks (`CF_Vtable`, `CF_Template`, `CF_Count`, `CF_Flags`) fail and return `0`, the allocated `CSWGuiLabel` and its sub-objects (such as `CExoString` fields) are never destroyed or freed, nor added to `panel`'s control list (`appendToPanel` is skipped by the caller when `createLabelFromTemplate` returns `0`). Every HUD rebuild where a template tag is missing or fails leaks engine heap memory.


* **Suggested fix:** If `createLabelFromTemplate` fails after `labelCtor` (when the control has not been added to the panel list), invoke the control's virtual destructor (or `exoDtor` + engine `operator delete`) before returning `0`.
* **Confidence:** certain


3. **File path and function name:** `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp` — `updateButtons`

* **Anchor:** `if (name[0]) guitip::setNative(b.bg.c, name);`

* **What is wrong:** When a slot `b` was previously assigned a valid tooltip string (e.g., `"Master Flurry"`) and the slot is subsequently reassigned to an entry whose `readExo` name read fails or is empty (`name[0] == '\0'`), `guitip::setNative` is skipped, yet `b.tipKey = e.key; b.tipSet = true;` still executes. The control `b.bg.c` retains the stale `CExoString` tooltip from the previous action at `ctl+0x2C`, displaying the wrong ability name when hovered.


* **Suggested fix:** Always call `guitip::setNative(b.bg.c, name)` (or `guitip::clearNative(b.bg.c)` when `name[0] == '\0'`) whenever `!b.tipSet || b.tipKey != e.key`:
```cpp
if (!b.tipSet || b.tipKey != e.key) {
    guitip::setNative(b.bg.c, g_names[g][ord[g][k]]);
    b.tipKey = e.key; b.tipSet = true;
}

```


* **Confidence:** certain


4. **File path and function name:** `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp` — `updateButtons`

* **Anchor:** `const uint32_t ns = ch::orderSig(gv[g], idn, c);`

* **What is wrong:** `g_names[g]` caches entry names across frames and only re-reads them from engine memory when `ns != g_nameSig[g]`. However, `ch::orderSig` only hashes the entry count `c` and the 32-bit `key` of each entry. For grenade items (group 2), the design doc (§2H) notes that keys are `item object id | 0x40000000`. Across a save/load, module transition, or inventory churn where a new item is allocated the same object ID slot (or if a target swaps to another creature with identical feat/power keys before `onHudBuild` runs), `ns` can match `g_nameSig[g]` while `g_nameValid[g]` is still `true` (`onHudBuild` does not reset `g_nameValid`). This causes `g_names` to serve stale names for tooltips and profile matching.


* **Suggested fix:** Reset `g_nameValid[0..2] = false` inside `onHudBuild` and `hideAllButtons`, and include the entry's `CExoString` pointer (`strPtr` at `+0x00`) or target/leader identity in the name-cache signature.
* **Confidence:** certain


5. **File path and function name:** `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp` — `onTargetMenuPost`

* **Anchor:** `if (g_cfg.hideArrows) {`

* **What is wrong:** When `g_cfg.hideArrows` is `true` (`[Arrows] Hide=1`) and `[Buttons] Replace=0`, `onTargetMenuPost` clears bit 1 (`~2u`) on all 6 arrow sub-controls (`menu+0x43C+i*0x750` and `menu+0x60C+i*0x750`). If the user edits `combat_hotbar.ini` to set `[Arrows] Hide=0` and hot-reloads within 1s via `filewatch::changed`, nothing restores bit 1 on the arrows for groups with `count <= 1` or until the engine repopulates them, and `g_arrowState` is never reset to `-1`, so re-enabling `Hide=1` later will not log `"arrows: hide applied"`. Furthermore, when `g_cfg.btnEnabled && g_cfg.replace` is active, `hideVanilla` already clears the arrow bits every frame, and when `replace` is toggled off, `restoreVanilla` only restores `VanillaOffs[0..1]` (`BTN` and `LBL`), leaving `g_arrowState` stale.


* **Suggested fix:** Reset `g_arrowState = -1;` inside the `filewatch::changed` block in `onTargetMenuPost`.


* **Confidence:** certain


6. **File path and function name:** `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp` — `updateButtons`

* **Anchor:** `tOk[g] = guikit::getBorderTint(btnCtl, tint[g]) && (tint[g][0] > 0.01f || tint[g][1] > 0.01f || tint[g][2] > 0.01f);`

* **What is wrong:** `updateButtons` calls `guikit::getBorderTint(btnCtl, tint[g])` to read the vanilla `BTN_TARGETn` border tint into `tint[g]` and sets `tOk[g]`, but two lines later unconditionally overwrites `tint[g]` and `tOk[g]`:
```cpp
tint[g][0] = 0.85f; tint[g][1] = 0.15f; tint[g][2] = 0.15f;
tOk[g] = true;
if (!aOk[g]) { alp[g] = 0.95f; aOk[g] = true; }
if (!tOk[g]) { if (g_contState[g] != 0) { ... } }

```


Because `tOk[g]` and `aOk[g]` are unconditionally forced to `true` before the `if (!tOk[g])` check and before `aOk[g] ? alp[g] : 0.85f`, the failure log branch (`"container g%d tint read failed or rejected (black)"`) and the `0.85f` alpha fallback are unreachable dead code, and the vanilla tint read is completely discarded contrary to the CH4d comment.


* **Suggested fix:** Either only apply the fallback `(0.85f, 0.15f, 0.15f)` when `!tOk[g]` (after logging the read failure), or remove the dead read/branches if the constant red theme is intended.
* **Confidence:** certain


7. **File path and function name:** `patch_manager_mods/_shared_src/WindowHook.h` — `kp::WindowHook::install`

* **Anchor:** `if (old) {`

* **What is wrong:** If the game recreates its main `HWND` (for example, on resolution/fullscreen toggle) without calling `uninstall`, `WindowHook::old` is still non-null from the previous window handle (`WindowHook::hwnd`). Calling `install(newHwnd, proc)` sees `if (old)` and immediately returns `true` without checking whether `h == hwnd` or `whapi::isWindow(hwnd)`, silently failing to subclass the new window. And in `GuiKit.h`, `wheelInstall()` calls `detail::WheelWin::install(detail::WheelWin::hwnd, detail::wheelProc)` whenever `detail::WheelWin::old` is non-null, never re-checking `detail::gameWindow()`.


* **Suggested fix:** In `WindowHook::install(HWND h, WNDPROC proc)` and `guikit::wheelInstall()`, check if `old != nullptr && (h != hwnd || !whapi::isWindow(hwnd))`; if the old window is gone or changed, reset `old = nullptr; hwnd = HWND(); passthrough = 0;` and subclass the new `h`.
* **Confidence:** likely



---

### Files or Areas Not Assessed

* **`patch_manager_mods/combat_hotbar_src/common_contract_test.cpp`:** Omitted from the bundle (`manifest-omit, 0 bytes`).


* **`kpatch_api.h` and `KpLog.h`:** Listed as external includes not resolvable inside the repo (not provided in `<documents>`); their exact struct definitions and logging macro implementations could not be verified directly.


* **`hud_host_src/` implementation:** `combat-hotbar` relies on `hud-host` (`k2.hud@1`) to hook `0x00743540`, `0x00743847`, and `0x0040F5A0` and populate `K2HudContext`. Only the consumer header `HudHost.h` was included in this bundle, so the provider's hook assembly, register preservation, and phase dispatch implementation could not be assessed.