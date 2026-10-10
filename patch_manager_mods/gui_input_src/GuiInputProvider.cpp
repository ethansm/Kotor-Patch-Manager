// GuiInputProvider.cpp -- Provider of the "k2.guiinput" v1 registry interface (GuiInput.h).
// Consolidates the stacked SetWindowLongPtrA GWLP_WNDPROC subclasses on the game window into a single provider.
// Owns game HWND discovery, foreground state tracking, cursor-to-engine coordinate mapping, and WM_MOUSEWHEEL subscriber dispatch.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "WindowHook.h"
#include "GuiInput.h"
#include "GiDispatch.h"

KP_LOG_PATCH("gui-input");

namespace {

KP_CHANNEL(chCore,  "core");
KP_CHANNEL(chWheel, "wheel");

// Fallback logging sink before core attaches
const bool g_fallbackSet = (KpLog_SetFallbackFile("gui-input_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

gid::Dispatcher g_disp;
HWND g_gameHwnd = nullptr;
DWORD g_lastWindowTry = 0;

struct GuiInputTag;
using Hook = kp::WindowHook<GuiInputTag>;

// ---- Window discovery (largest visible window of this pid > 320x240) ----
struct EnumCtx { DWORD pid; HWND best; long area; };

BOOL CALLBACK enumProc(HWND h, LPARAM lp) {
    EnumCtx* c = reinterpret_cast<EnumCtx*>(lp);
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid != c->pid || !IsWindowVisible(h)) return TRUE;
    RECT r;
    if (!GetClientRect(h, &r) || r.right <= 320 || r.bottom <= 240) return TRUE;
    long a = r.right * r.bottom;
    if (a > c->area) { c->area = a; c->best = h; }
    return TRUE;
}

HWND findGameWindow() {
    if (g_gameHwnd && IsWindow(g_gameHwnd)) return g_gameHwnd;
    DWORD now = GetTickCount();
    if (g_lastWindowTry && (now - g_lastWindowTry) < 1000) return g_gameHwnd;
    g_lastWindowTry = now;

    EnumCtx c = { GetCurrentProcessId(), nullptr, 0 };
    EnumWindows(enumProc, reinterpret_cast<LPARAM>(&c));
    if (c.best) {
        g_gameHwnd = c.best;
        KP_LOGV(chCore, KPLOG_INFO, "found game window %p", (void*)g_gameHwnd);
    }
    return g_gameHwnd;
}

void readScreenDims(int* w, int* h) {
    if (w) *w = 0;
    if (h) *h = 0;
    saferead::beginScope();
    int ew = 0, eh = 0;
    if (saferead::readAt(0x7101, gameaddr::ScreenW, 0, &ew) && ew >= 320 && ew <= 16384) {
        if (w) *w = ew;
    }
    if (saferead::readAt(0x7102, gameaddr::ScreenH, 0, &eh) && eh >= 200 && eh <= 16384) {
        if (h) *h = eh;
    }
}

// ---- Window proc subclass ----
LRESULT CALLBACK wheelProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_MOUSEWHEEL && Hook::active()) {
        int delta = (short)HIWORD(w);
        POINT pt = { (short)LOWORD(l), (short)HIWORD(l) }; // screen coordinates
        RECT r = { 0, 0, 0, 0 };
        bool haveClient = ScreenToClient(h, &pt) && GetClientRect(h, &r) && r.right > 0 && r.bottom > 0;
        int sw = haveClient ? r.right : 1024;
        int sh = haveClient ? r.bottom : 768;
        float ex = 0.0f, ey = 0.0f;
        if (haveClient) {
            int ew = 0, eh = 0;
            readScreenDims(&ew, &eh);
            if (ew > 0 && eh > 0) { sw = ew; sh = eh; }
            ex = static_cast<float>(pt.x) * static_cast<float>(sw) / static_cast<float>(r.right);
            ey = static_cast<float>(pt.y) * static_cast<float>(sh) / static_cast<float>(r.bottom);
        }

        K2WheelEvent ev = {
            sizeof(K2WheelEvent),
            delta,
            ex,
            ey,
            haveClient ? pt.x : 0,
            haveClient ? pt.y : 0,
            sw,
            sh
        };

        if (g_disp.dispatch(ev)) {
            KP_LOGV(chWheel, KPLOG_DEBUG, "wheel consumed delta=%d client=(%d,%d) engine=(%.1f,%.1f)",
                    delta, pt.x, pt.y, ex, ey);
            return 0; // Consumed, do not forward to engine
        }
    }
    return Hook::forward(h, m, w, l);
}

bool ensureSubclass() {
    if (Hook::active()) return true;
    HWND h = findGameWindow();
    if (!h) return false;
    bool ok = Hook::install(h, wheelProc);
    if (ok) {
        KP_LOGV(chCore, KPLOG_INFO, "subclassed game window %p, old proc %p", (void*)h, (void*)Hook::old);
    } else {
        KP_LOGV(chCore, KPLOG_WARN, "failed to subclass game window %p (err=%lu)", (void*)h, (unsigned long)GetLastError());
    }
    return ok;
}

// ---- Exported Provider API ----
intptr_t KPATCH_CALL apiGameHwnd() {
    HWND h = findGameWindow();
    return reinterpret_cast<intptr_t>(h);
}

int32_t KPATCH_CALL apiCursorEngine(float* x, float* y) {
    if (!x || !y) return 0;
    HWND h = findGameWindow();
    if (!h) return 0;

    POINT pt;
    RECT r = { 0, 0, 0, 0 };
    if (!GetCursorPos(&pt) || !ScreenToClient(h, &pt) || !GetClientRect(h, &r) || r.right < 1 || r.bottom < 1) {
        return 0;
    }

    int sw = r.right, sh = r.bottom;
    int ew = 0, eh = 0;
    readScreenDims(&ew, &eh);
    if (ew > 0 && eh > 0) { sw = ew; sh = eh; }

    *x = static_cast<float>(pt.x) * static_cast<float>(sw) / static_cast<float>(r.right);
    *y = static_cast<float>(pt.y) * static_cast<float>(sh) / static_cast<float>(r.bottom);
    return 1;
}

int32_t KPATCH_CALL apiWheelSubscribe(K2WheelCallback cb, void* user) {
    if (!cb) return -1;
    if (!ensureSubclass()) {
        KP_LOGV(chCore, KPLOG_WARN, "wheel subscribe cb=%p -> -1 (game window not subclassed yet)", (void*)cb);
        return -1;
    }
    int32_t tok = g_disp.subscribe(cb, user);
    KP_LOGV(chCore, tok > 0 ? KPLOG_INFO : KPLOG_WARN, "wheel subscribe cb=%p -> token %d", (void*)cb, tok);
    return tok;
}

void KPATCH_CALL apiWheelUnsubscribe(K2WheelCallback cb) {
    if (!cb) return;
    bool removed = g_disp.unsubscribe(cb);
    KP_LOGV(chCore, KPLOG_INFO, "wheel unsubscribe cb=%p -> removed %d", (void*)cb, (int)removed);
}

int32_t KPATCH_CALL apiForeground() {
    HWND fg = GetForegroundWindow();
    HWND h = findGameWindow();
    return (fg && h && fg == h) ? 1 : 0;
}

const K2GuiInputApi g_apiTable = {
    sizeof(K2GuiInputApi),
    apiGameHwnd,
    apiCursorEngine,
    apiWheelSubscribe,
    apiWheelUnsubscribe,
    apiForeground
};

} // anonymous namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) return;
    api->provide(K2GUIINPUT_IFACE, K2GUIINPUT_VERSION, &g_apiTable);
    KpLog_Init(api);
    KP_LOG(chCore, "provided %s v%d", K2GUIINPUT_IFACE, K2GUIINPUT_VERSION);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_DETACH) {
        Hook::uninstall(&wheelProc);
    }
    return TRUE;
}

} // extern "C"
