// WindowHook.h -- one game-HWND subclass per Tag, safe against out-of-order unhook (finding F28, design A, doc 20f section 3).
// install pins the DLL image (GetModuleHandleExA FROM_ADDRESS|PIN) so a proc that stays chained can never dangle. uninstall restores
// only when still on top; otherwise it sets `passthrough` and KEEPS `old`: the proc stays chained but inert and always forwards.
// A later install() on a pass-through hook just clears the flag (no second SetWindowLongPtr). Usage, per DLL:
//   struct Tag; using Hook = kp::WindowHook<Tag>;   Hook::install(hwnd, &MyProc);   ... Hook::uninstall(&MyProc);
//   LRESULT CALLBACK MyProc(HWND h, UINT m, WPARAM w, LPARAM l) { if (Hook::active()) { ...handle... } return Hook::forward(h, m, w, l); }
// Host tests: define WINDOWHOOK_HOST and supply kp::whapi (setProc/getProc/callProc/defProc/isWindow/pin/xchg) plus the Win32 type aliases.
#pragma once
#ifndef WINDOWHOOK_HOST
#include <windows.h>
namespace kp { namespace whapi {
inline WNDPROC setProc(HWND h, WNDPROC p) { return reinterpret_cast<WNDPROC>(SetWindowLongPtrA(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(p))); }
inline WNDPROC getProc(HWND h) { return reinterpret_cast<WNDPROC>(GetWindowLongPtrA(h, GWLP_WNDPROC)); }
inline LRESULT callProc(WNDPROC p, HWND h, UINT m, WPARAM w, LPARAM l) { return CallWindowProcA(p, h, m, w, l); }
inline LRESULT defProc(HWND h, UINT m, WPARAM w, LPARAM l) { return DefWindowProcA(h, m, w, l); }
inline bool isWindow(HWND h) { return IsWindow(h) != 0; }
inline void pin(WNDPROC p) { HMODULE m = nullptr; GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast<LPCSTR>(p), &m); }
inline LONG xchg(volatile LONG* t, LONG v) { return InterlockedExchange(t, v); }
} }
#endif

namespace kp {

template <class Tag>
struct WindowHook {
    static inline WNDPROC old = nullptr;
    static inline HWND hwnd = HWND();
    static inline volatile LONG passthrough = 0;
    static inline int pins = 0;

    static bool install(HWND h, WNDPROC proc) {
        if (pins == 0) { whapi::pin(proc); ++pins; }
        if (old) {
            if (passthrough) whapi::xchg(&passthrough, 0);
            return true;
        }
        old = whapi::setProc(h, proc);
        if (old) { hwnd = h; passthrough = 0; }
        return old != nullptr;
    }
    static bool active() { return old && !passthrough; }
    static LRESULT forward(HWND h, UINT m, WPARAM w, LPARAM l) { return old ? whapi::callProc(old, h, m, w, l) : whapi::defProc(h, m, w, l); }
    static void uninstall(WNDPROC proc) {
        if (!old || !hwnd || !whapi::isWindow(hwnd)) return;
        if (whapi::getProc(hwnd) == proc) {
            whapi::setProc(hwnd, old);
            old = nullptr; hwnd = HWND(); passthrough = 0;
        } else {
            whapi::xchg(&passthrough, 1);
        }
    }
};

}  // namespace kp
