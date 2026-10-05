# 20f — WindowHook design note (F28, BF4)

Design only. No mod code changes. Implementing `_shared_src/WindowHook.h` needs a KPM `Patches/Common` sync and user approval.

## 1. Problem (F28)

All 5 registered subclassers (`tests/static/subclassers.toml`) use restore-if-still-ours:

```cpp
install: if (!g_oldProc) g_oldProc = SetWindowLongPtrA(h, GWLP_WNDPROC, (LONG_PTR)wheelProc);
proc:    ... return CallWindowProcA(g_oldProc, h, m, w, l);
unload:  if (g_oldProc && IsWindow(h) && GetWindowLongPtrA(h, GWLP_WNDPROC) == wheelProc) {
             SetWindowLongPtrA(h, GWLP_WNDPROC, g_oldProc); g_oldProc = nullptr; }
```

When a DLL unhooks while it is not on top, the restore is skipped. Its proc stays chained **and keeps handling messages** (wheel, keys) after its owner said "stop". A later restore by the DLL above can also put the stale proc back on top. `tests/wndproc_chain` shows this in 10 of 12 install × unhook orders (only the two strict-reverse orders are clean).

If the DLL's code were unmapped while still chained, the next message would jump into freed memory. Today this is latent: KPM DLLs detach only at process exit, and no mod unhooks at run time.

## 2. Sites

| Mod | Install | Restore |
|---|---|---|
| `inventory_3d_viewport_src/Inventory3DViewport.cpp` | :341 | :596 (DllMain DETACH) |
| `equipment_3d_character_src/EquipCharPreview.cpp` | :292 | :1015 |
| `active_effects_panel_src/CharEffectsPanel.cpp` | :254 | :1029 |
| `action_hotbar_src/ActionHotbar.cpp` | :1035 | :1864 |
| `_shared_src/GuiKit.h` `wheelInstall` / `wheelUninstall` | :623 | :632 (no mod calls it yet; doc 32 C8) |

## 3. Options

### A — per-DLL pass-through + module pin (RECOMMENDED)

No cross-DLL state. Each DLL keeps its own `oldProc`.

- **Install:** as today, plus `GetModuleHandleExA(FROM_ADDRESS | PIN, (LPCSTR)&proc, &mod)`. The proc's code is then never unmapped, even if KPM or anyone calls `FreeLibrary`.
- **Uninstall:** restore if ours is on top (as today). Otherwise set `passthrough = true` and **keep** `oldProc`. A chained proc's `oldProc` is never nulled.
- **Proc:** `if (!passthrough) handle(...)`; then always `CallWindowProcA(oldProc, ...)`.
- **Re-install while passthrough:** clear `passthrough`; no second `SetWindowLongPtr` (the proc is still in the chain).

Result: no handling after unhook, no dangling code, and a stale proc put back on top is harmless (it only forwards).

Sketch (`kp::WindowHook`, ≤25 lines):

```cpp
namespace kp {
template <class Tag> struct WindowHook {           // one per subclassing site
    static inline WNDPROC old = nullptr;
    static inline HWND hwnd = nullptr;
    static inline volatile LONG passthrough = 0;   // set off the window thread
    static bool install(HWND h, WNDPROC proc) {
        if (old) { InterlockedExchange(&passthrough, 0); return true; }
        HMODULE m; GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast<LPCSTR>(proc), &m);
        old = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(h, GWLP_WNDPROC,
                                                          reinterpret_cast<LONG_PTR>(proc)));
        if (old) hwnd = h;
        return old != nullptr;
    }
    static bool active() { return old && !passthrough; }   // proc: if (active()) handle
    static LRESULT forward(HWND h, UINT m, WPARAM w, LPARAM l) { return CallWindowProcA(old, h, m, w, l); }
    static void uninstall(WNDPROC proc) {
        if (!old || !hwnd || !IsWindow(hwnd)) return;
        if (reinterpret_cast<WNDPROC>(GetWindowLongPtrA(hwnd, GWLP_WNDPROC)) == proc) {
            SetWindowLongPtrA(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(old));
            old = nullptr; hwnd = nullptr;
        } else InterlockedExchange(&passthrough, 1);  // stay chained, forward only
    }
};
}
```

### B — one shared dispatcher (REJECTED)

One proc owned by "whoever came first", with others registering handlers through a window property or a named shared registry.

- Needs a cross-DLL ABI (handler struct layout, calling convention) and versioning across mods built at different times.
- If the owner unloads first, every registered handler loses its dispatcher. The fix is pinning, which A already has, at a much lower cost.
- More code in every mod, and a single point of failure in the hottest path (every window message).

## 4. Model evidence

`tests/wndproc_chain/wndproc_chain_test.cpp` now has `SubPass<N>` beside `Sub<N,…>`, run over the same 12 orders:

- 0 messages handled after unhook, for every proc and every order.
- The base proc gets every message exactly once; no proc handles a message twice; every live proc handles every message.
- Reverse order still ends with `BaseGameProc` on top.
- CHARACTERIZATION (the cost of A): in non-reverse orders, forwarding procs stay on the chain.

The existing F28 CHARACTERIZATION asserts for `Sub` are unchanged; they document today's pattern and flip only when the mods migrate.

## 5. Migration (later, after approval)

1. Add `_shared_src/WindowHook.h` + colocated test + Common contract test; KPM `Patches/Common` sync.
2. Migrate the 5 sites (one commit per mod; GuiKit `wheelInstall`/`wheelUninstall` become thin wrappers).
3. `tests/static/subclassers.toml`: patterns for the new API.
4. `check_subclassers.py`: hint text points at `kp::WindowHook`.
5. Flip the F28 model asserts from CHARACTERIZATION to "fixed".

## 6. Risks

- **Pin keeps the DLL mapped until process exit.** That matches today's real lifetime (KPM never unloads), so no behaviour change.
- **Pin defeats run-time unload.** If KPM ever wants hot unload, a pinned DLL silently stays. Acceptable: hot unload of a subclassing DLL is unsafe without a pin anyway.
- **Unhook in `DllMain` DETACH** runs under the loader lock at exit; `SetWindowLongPtr` there is what we do today. A pinned DLL only sees DETACH at process exit, where the window may already be gone (`IsWindow` guard stays).
- **Thread visibility:** `passthrough` may be set off the window thread; use an interlocked write (sketch does).
- **Chain growth** if a mod installs/uninstalls repeatedly while not on top: avoided, since re-install just clears `passthrough`.

## 7. Open questions for the user

1. Approve implementing `WindowHook.h` (design A) + the 5-site migration? Needs a Common sync.
2. Migrate all 5 sites in one batch, or one mod per install cycle?
3. Should `subclassers.toml` / `check_subclassers.py` start rejecting the raw pattern once `WindowHook.h` exists?
