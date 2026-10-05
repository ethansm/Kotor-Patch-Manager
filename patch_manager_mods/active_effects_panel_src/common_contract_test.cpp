// common_contract_test.cpp -- Patches/Common CONTRACT test for the Character Effects Panel (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/{saferead,windowhook}_test.cpp). Every message starts with
// "chareffects relies on: " so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY
// edit to SafeRead.h / WindowHook.h / GameAddr.h re-runs this test: a Common change that would break the panel fails here first.
//
// Areas: (1) gameaddr constants (compile time), (2) saferead via the mod's readAt wrapper call shape (site 0), (3) WindowHook<WheelTag>
// lifecycle with a proc that mirrors the mod's wheelProc (handle when active, else forward). Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "check.h"
#include "fake_wndproc.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using HWND = fakewnd::HWND;
using WNDPROC = fakewnd::Proc;
using LRESULT = intptr_t;
using UINT = unsigned;
using WPARAM = intptr_t;
using LPARAM = intptr_t;

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

#define HCHECK(why, expr) ::kt::Check("chareffects relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("chareffects relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))
#define U(x) static_cast<unsigned long long>(x)

// ============================================================ 1. GameAddr (compile time)
#define HADDR(name, val) static_assert(gameaddr::name == (val), "chareffects relies on: gameaddr::" #name " == " #val " (validated Steam address)")
HADDR(ClientCreatureVtable, 0x0099EE14); HADDR(WorldTimerGetCurrentTime, 0x0051AD20);

// ============================================================ 2. SafeRead (the mod's readAt wrapper shape, site 0)
// The mod's wrapper truncates the base to 32 bits (engine pointers); on the 64-bit host the buffer therefore lives in a MAP_32BIT arena.
static uint8_t* g_arena = nullptr;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
template <typename T> static bool modRead(const void* base, int off, T* out) {   // exact call shape of CharEffectsPanel readAt<T>
    return saferead::readAt<T>(0, static_cast<uintptr_t>(static_cast<unsigned>(reinterpret_cast<uintptr_t>(base))), off, out);
}
static void saferead_contract() {
    memset(g_arena, 0, 64);
    at<int>(reinterpret_cast<uintptr_t>(g_arena), 8) = 1920;
    at<short>(reinterpret_cast<uintptr_t>(g_arena), 12) = -7;
    at<float>(reinterpret_cast<uintptr_t>(g_arena), 16) = 0.5f;
    saferead::beginScope();
    int i = 0x11111111; short s = 99; float f = 1.f;
    HCHECK("readAt<int> on readable memory returns true (framebuffer size, effect counts)", modRead(g_arena, 8, &i));
    HEQ("readAt<int> delivers the value", (long long)i, 1920ll);
    HCHECK("readAt<short> works (effect fields)", modRead(g_arena, 12, &s) && s == -7);
    HCHECK("readAt<float> works (durations)", modRead(g_arena, 16, &f) && f == 0.5f);
    i = 0x22222222;
    HCHECK("readAt with base 0 returns false", !modRead(nullptr, 8, &i));
    HEQ("readAt failure leaves *out untouched (base 0; the mod pre-seeds defaults)", (long long)i, 0x22222222ll);
    HCHECK("readAt below 0x10000 returns false", !modRead(reinterpret_cast<void*>(0x20), 0, &i));
    HEQ("readAt failure leaves *out untouched (low address)", (long long)i, 0x22222222ll);
    s = 55; f = 2.f;
    HCHECK("readAt<short>/<float> failures return false", !modRead(reinterpret_cast<void*>(0x30), 4, &s) && !modRead(nullptr, 0, &f));
    HCHECK("readAt<short>/<float> failures leave *out untouched", s == 55 && f == 2.f);
    bool ok = true;
    for (int n = 0; n < 5; ++n) { saferead::beginScope(); i = 0; ok = ok && modRead(g_arena, 8, &i) && i == 1920; }
    HCHECK("beginScope is callable repeatedly (every poll) and reads still work", ok);
}

// ============================================================ 3. WindowHook<WheelTag>
struct WheelTag {};
using WheelHook = kp::WindowHook<WheelTag>;
static int g_handled = 0;
static intptr_t wheelProc(HWND h, unsigned m, intptr_t w, intptr_t l) {   // mirrors the mod's wheelProc: handle when active(), else forward
    if (WheelHook::active() && m == 0x20A) { ++g_handled; return 0; }
    return WheelHook::forward(h, m, w, l);
}
// a second DLL's subclass stacked above ours
static int g_otherHits = 0; static fakewnd::Proc g_otherPrev = nullptr;
static intptr_t otherProc(HWND h, unsigned m, intptr_t w, intptr_t l) { ++g_otherHits; return fakewnd::FakeCallWindowProc(g_otherPrev, h, m, w, l); }
static void resetWorld() {
    fakewnd::Reset();
    kp::whapi::pinCalls = kp::whapi::setCalls = kp::whapi::defHits = 0;
    WheelHook::old = nullptr; WheelHook::hwnd = HWND(); WheelHook::passthrough = 0; WheelHook::pins = 0;
    g_handled = g_otherHits = 0; g_otherPrev = nullptr;
}
static void windowhook_contract() {
    using namespace fakewnd;
    resetWorld();
    HCHECK("fresh hook is not active() and old is null (the mod installs only when !active())", !WheelHook::active() && WheelHook::old == nullptr);

    HCHECK("install on the game window succeeds", WheelHook::install(kHwnd, &wheelProc));
    HCHECK("install sets old to the previous proc (the mod logs it) and active() becomes true", WheelHook::old == &BaseGameProc && WheelHook::active());
    HEQ("install pins the DLL image once", kp::whapi::pinCalls, 1);
    HEQ("install subclasses with exactly one setProc", kp::whapi::setCalls, 1);
    HCHECK("our proc is on top of the window", FakeGetWindowLongPtr(kHwnd) == &wheelProc);

    WheelHook::install(kHwnd, &wheelProc);
    HEQ("second install while old is set makes no extra setProc", kp::whapi::setCalls, 1);
    HEQ("second install does not pin again", kp::whapi::pinCalls, 1);

    int b0 = base().hits;
    intptr_t r = WheelHook::forward(kHwnd, 0x200, 0, 41);
    HCHECK("forward reaches the previous proc (non-wheel messages keep working)", base().hits == b0 + 1 && r == 42);
    HCHECK("forward does not touch defProc while old is set", kp::whapi::defHits == 0);
    FakeSendMessage(0x20A, 0, 0);
    HEQ("active proc handles wheel messages itself", g_handled, 1);

    WheelHook::uninstall(&wheelProc);
    HCHECK("uninstall while on top restores the previous proc", FakeGetWindowLongPtr(kHwnd) == &BaseGameProc);
    HCHECK("uninstall while on top nulls old and hwnd", WheelHook::old == nullptr && WheelHook::hwnd == HWND());
    HCHECK("after top uninstall active() is false", !WheelHook::active());

    // 6: another subclass on top at detach -> passthrough
    resetWorld();
    WheelHook::install(kHwnd, &wheelProc);
    g_otherPrev = FakeSetWindowLongPtr(kHwnd, &otherProc);
    WheelHook::uninstall(&wheelProc);
    HCHECK("uninstall under another subclass leaves that subclass on top", FakeGetWindowLongPtr(kHwnd) == &otherProc);
    HCHECK("uninstall under another subclass sets passthrough: active() false", !WheelHook::active() && WheelHook::passthrough == 1);
    HCHECK("passthrough keeps old so the chain stays intact", WheelHook::old == &BaseGameProc);
    int bb = base().hits, hh = g_handled;
    intptr_t r2 = FakeSendMessage(0x20A, 0, 5);
    HCHECK("passthrough forward still chains to the previous proc and handles nothing", base().hits == bb + 1 && r2 == 6 && g_handled == hh && g_otherHits == 1);

    // 7: re-install clears passthrough
    int sets = kp::whapi::setCalls;
    WheelHook::install(kHwnd, &wheelProc);
    HCHECK("re-install after passthrough makes active() true again", WheelHook::active() && WheelHook::passthrough == 0);
    HCHECK("re-install after passthrough makes no extra setProc", kp::whapi::setCalls == sets);
    FakeSendMessage(0x20A, 0, 5);
    HEQ("re-installed proc handles wheel messages again", g_handled, hh + 1);
}

int main() {
    void* m = mmap(nullptr, 1u << 16, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    saferead_contract();
    windowhook_contract();
    return kt::Report("common_contract_test (active_effects_panel)");
}
