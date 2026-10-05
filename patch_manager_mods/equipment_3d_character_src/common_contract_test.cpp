// common_contract_test.cpp -- Patches/Common CONTRACT test for the Equipment 3D Character preview (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/windowhook_test.cpp). Every message starts with "equip3d relies on: "
// so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to GameAddr.h /
// WindowHook.h re-runs this test: a Common change that would break the preview fails here first.
//
// Areas: (1) gameaddr constants (compile time), (2) WindowHook as the mouse-wheel subclass: install only while old is null, handle
// only while active, forward chains to the previous proc, top uninstall restores, buried uninstall goes pass-through. Host-only.
#include "check.h"
#include "fake_wndproc.h"

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
#include "GameAddr.h"

#define HCHECK(why, expr) ::kt::Check("equip3d relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("equip3d relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

// ============================================================ 1. GameAddr (compile time)
#define HADDR(name, val) static_assert(gameaddr::name == (val), "equip3d relies on: gameaddr::" #name " == " #val " (validated Steam address)")
HADDR(ClientCreatureVtable, 0x0099EE14);

// ============================================================ 2. WindowHook as the wheel subclass
struct EquipTestTag {};
using Hook = kp::WindowHook<EquipTestTag>;
static int g_handled = 0;

// Mirrors wheelProc: handle only while active(), always forward the rest.
static intptr_t wheelProc(HWND h, unsigned m, intptr_t w, intptr_t l) {
    if (Hook::active()) ++g_handled;
    return Hook::forward(h, m, w, l);
}
// A second subclass of the game window that sits on top of ours.
static intptr_t otherProc(HWND, unsigned, intptr_t, intptr_t l) { return l + 100; }

static void resetWorld() {
    fakewnd::Reset();
    kp::whapi::pinCalls = kp::whapi::setCalls = kp::whapi::defHits = 0;
    Hook::old = nullptr; Hook::hwnd = HWND(); Hook::passthrough = 0; Hook::pins = 0; g_handled = 0;
}

static void windowhook_contract() {
    using namespace fakewnd;
    resetWorld();
    // (1) fresh
    HCHECK("fresh hook: active() is false", !Hook::active());
    HCHECK("fresh hook: old is null (the install guard is !old)", Hook::old == nullptr);

    // (2) install
    HCHECK("install on the game window succeeds", Hook::install(kHwnd, &wheelProc));
    HCHECK("install records the previous proc in old", Hook::old == &BaseGameProc);
    HCHECK("active() is true after install", Hook::active());
    HEQ("install pins the DLL image once", kp::whapi::pinCalls, 1);
    HEQ("install calls setProc once", kp::whapi::setCalls, 1);
    HCHECK("our proc is the top proc after install", FakeGetWindowLongPtr(kHwnd) == &wheelProc);

    // (3) MANDATORY: a second install while old is set must not subclass again (equip3d re-installs only when old is null)
    Hook::install(kHwnd, &wheelProc);
    HEQ("second install while old set makes no extra setProc", kp::whapi::setCalls, 1);
    HCHECK("second install keeps old unchanged", Hook::old == &BaseGameProc);
    HEQ("second install does not pin again", kp::whapi::pinCalls, 1);

    // (4) forward reaches the previous proc
    int hitsBefore = base().hits;
    intptr_t r = FakeSendMessage(0x20A, 7, 41);
    HCHECK("wheel proc handles the message while active", g_handled == 1);
    HCHECK("forward reaches the previous proc exactly once", base().hits == hitsBefore + 1);
    HEQ("forward returns the previous proc result", (long long)r, 42ll);

    // (5) uninstall while on top restores the previous proc and nulls old
    Hook::uninstall(&wheelProc);
    HCHECK("top uninstall restores the previous proc", FakeGetWindowLongPtr(kHwnd) == &BaseGameProc);
    HCHECK("top uninstall nulls old (so a later install can subclass again)", Hook::old == nullptr);
    HCHECK("top uninstall: active() is false", !Hook::active());
    HEQ("top uninstall leaves passthrough clear", (long long)Hook::passthrough, 0ll);

    // (6) uninstall while another subclass is on top -> pass-through, still chained
    resetWorld();
    Hook::install(kHwnd, &wheelProc);
    WNDPROC below = FakeSetWindowLongPtr(kHwnd, &otherProc);
    HCHECK("setup: other subclass chains over ours", below == &wheelProc);
    Hook::uninstall(&wheelProc);
    HCHECK("buried uninstall: active() is false", !Hook::active());
    HEQ("buried uninstall sets passthrough", (long long)Hook::passthrough, 1ll);
    HCHECK("buried uninstall keeps old so the chain stays intact", Hook::old == &BaseGameProc);
    HCHECK("buried uninstall leaves the other proc on top", FakeGetWindowLongPtr(kHwnd) == &otherProc);
    // the other proc forwards to ours (as a chained subclass would): we must not handle, only chain to the base
    int baseBefore = base().hits, handledBefore = g_handled;
    intptr_t r2 = wheelProc(kHwnd, 0x20A, 7, 41);
    HCHECK("pass-through: our proc does not handle", g_handled == handledBefore);
    HCHECK("pass-through: forward still chains to the previous proc", base().hits == baseBefore + 1);
    HEQ("pass-through: forward result is the previous proc result", (long long)r2, 42ll);
    HEQ("pass-through: no default-proc fallback", kp::whapi::defHits, 0);
}

int main() {
    windowhook_contract();
    return kt::Report("common_contract_test (equipment_3d_character)");
}
