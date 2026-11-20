// Pure decision logic for k2-activation-fix (no windows.h, host-testable).
#pragma once
#include <stdint.h>

namespace actfix {

constexpr uint32_t kWmActivateApp = 0x001C;

// A WM_ACTIVATEAPP(wParam==0) means "your app lost activation". Wine also sends it from the movie thread while
// focus stays inside our own process (observed: lParam = movie thread id, e.g. 0x1ac), which latches the engine's
// DAT_00a1b4e0 to 0 and strands the load screen. Drop it only when the foreground window belongs to our process.
// A null foreground (transient) or another process's window is a real deactivate and passes through.
// moviePlaying: the engine's own latch condition (client exists and IsMoviePlaying()==1). Outside a movie the engine's
// wParam==0 handling (alt-tab, Alt+Enter, pause) is left exactly as it was.
inline bool shouldDropDeactivate(uint32_t msg, uintptr_t wParam, uint32_t fgPid, uint32_t ourPid, bool moviePlaying) {
    if (msg != kWmActivateApp || wParam != 0 || !moviePlaying) return false;
    return fgPid != 0 && fgPid == ourPid;
}

// The 0x40D69E synthetic PostMessageA(hwnd, WM_ACTIVATEAPP, 0, 0): same rule, plus lParam must be 0.
inline bool shouldDropSyntheticPost(uint32_t msg, uintptr_t wParam, intptr_t lParam, uint32_t fgPid, uint32_t ourPid) {
    return lParam == 0 && shouldDropDeactivate(msg, wParam, fgPid, ourPid, true);   // the post only happens at a movie end
}

}  // namespace actfix
