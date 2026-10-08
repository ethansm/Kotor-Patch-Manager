// common_contract_test.cpp -- Patches/Common CONTRACT test for gui-input.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins semantics relied on by GuiInputProvider.cpp in WindowHook.h, KpLog.h, and GuiInput.h.

#define WINDOWHOOK_HOST 1
#include "check.h"
#include "fake_wndproc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using HWND = fakewnd::HWND;
using WNDPROC = fakewnd::Proc;
using LRESULT = intptr_t;
using UINT = unsigned;
using WPARAM = intptr_t;
using LPARAM = intptr_t;
using LONG = long;

namespace kp { namespace whapi {
inline int pinCalls = 0, setCalls = 0, defHits = 0;
constexpr LRESULT kDef = 0x5EED;
inline WNDPROC setProc(HWND h, WNDPROC p) { ++setCalls; return fakewnd::FakeSetWindowLongPtr(h, p); }
inline WNDPROC getProc(HWND h) { return fakewnd::FakeGetWindowLongPtr(h); }
inline LRESULT callProc(WNDPROC p, HWND h, UINT m, WPARAM w, LPARAM l) { return fakewnd::FakeCallWindowProc(p, h, m, w, l); }
inline LRESULT defProc(HWND, UINT, WPARAM, LPARAM) { ++defHits; return kDef; }
inline bool isWindow(HWND h) { return fakewnd::FakeIsWindow(h); }
inline void pin(WNDPROC) { ++pinCalls; }
inline LONG xchg(volatile LONG* t, LONG v) { return __atomic_exchange_n(t, v, __ATOMIC_SEQ_CST); }
} }

#include "WindowHook.h"
#include "KpLog.h"
#include "GuiInput.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("gui-input relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("gui-input relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("gui-input");
KP_CHANNEL(chCore, "core");

struct HookTag;
using Hook = kp::WindowHook<HookTag>;

static int g_handled = 0;
static intptr_t wheelProc(HWND h, unsigned m, intptr_t w, intptr_t l) {
    if (Hook::active() && m == 0x20A) { ++g_handled; return 0; }
    return Hook::forward(h, m, w, l);
}

static intptr_t otherProc(HWND h, unsigned m, intptr_t w, intptr_t l) {
    return Hook::forward(h, m, w, l);
}

static void resetWorld() {
    fakewnd::Reset();
    kp::whapi::pinCalls = kp::whapi::setCalls = kp::whapi::defHits = 0;
    Hook::old = nullptr; Hook::hwnd = HWND(); Hook::passthrough = 0; Hook::pins = 0; g_handled = 0;
}

int main() {
    // 1. WindowHook contract
    resetWorld();
    using namespace fakewnd;
    HCHECK("fresh hook: active() is false", !Hook::active());
    HCHECK("fresh hook: old is null", Hook::old == nullptr);

    HCHECK("install on game window succeeds", Hook::install(kHwnd, &wheelProc));
    HCHECK("install records previous proc", Hook::old == &BaseGameProc);
    HCHECK("active() is true after install", Hook::active());
    HEQ("install pins DLL once", kp::whapi::pinCalls, 1);
    HEQ("install calls setProc once", kp::whapi::setCalls, 1);

    // Second install while old set does not re-subclass
    Hook::install(kHwnd, &wheelProc);
    HEQ("second install makes no extra setProc", kp::whapi::setCalls, 1);

    // Uninstall while on top restores old proc
    Hook::uninstall(&wheelProc);
    HCHECK("top uninstall restores previous proc", FakeGetWindowLongPtr(kHwnd) == &BaseGameProc);
    HCHECK("top uninstall nulls old", Hook::old == nullptr);
    HCHECK("top uninstall: active() is false", !Hook::active());

    // Buried uninstall sets passthrough
    resetWorld();
    Hook::install(kHwnd, &wheelProc);
    WNDPROC below = FakeSetWindowLongPtr(kHwnd, &otherProc);
    HCHECK("setup other subclass chains over ours", below == &wheelProc);
    Hook::uninstall(&wheelProc);
    HCHECK("buried uninstall: active() is false", !Hook::active());
    HEQ("buried uninstall sets passthrough", (long long)Hook::passthrough, 1ll);
    HCHECK("buried uninstall keeps old so chain stays intact", Hook::old == &BaseGameProc);

    // 2. KpLog contract
    HEQ("KpLog_Init(nullptr) = 0", KpLog_Init(nullptr), 0);
    fakekp::reset();
    HEQ("KpLog_Init without kpatch.log service = 0", KpLog_Init(fakekp::api()), 0);
    KP_LOG(chCore, "no core yet %d", 1);
    fakekp::withLog();
    HEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
    KP_LOG(chCore, "provided %s v%d", K2GUIINPUT_IFACE, K2GUIINPUT_VERSION);
    HCHECK("KP_LOG reaches core service", !fakekp::state().logLines.empty());

    // 3. GuiInput.h client contract
    fakekp::reset();
    k2input::Client c; c.init(fakekp::api(), "t");
    HCHECK("connect() without provider is false", !c.connect());
    HEQ("game_hwnd() without provider = 0", c.game_hwnd(), 0);
    HEQ("foreground() without provider = false", c.foreground(), false);

    return ::kt::Report("common_contract_test (gui-input)");
}
