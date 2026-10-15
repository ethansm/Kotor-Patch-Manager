// Host test of WindowHook.h (finding F28, design A): kp::WindowHook<Tag> over tests/include/fake_wndproc.h.
// Mirrors SubPass/runPassScenario of tests/wndproc_chain/wndproc_chain_test.cpp, but drives the production header.
#include "check.h"
#include "fake_wndproc.h"

#include <algorithm>
#include <array>
#include <string>

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

using namespace fakewnd;

template <int N> struct T;

template <int N>
struct Sub {
    using H = kp::WindowHook<T<N>>;
    inline static int handled = 0, handledAfterUnhook = 0;
    inline static bool unhooked = false;  // test bookkeeping
    static intptr_t proc(HWND h, unsigned m, intptr_t w, intptr_t l) {
        if (H::active()) { ++handled; if (unhooked) ++handledAfterUnhook; }
        return H::forward(h, m, w, l);
    }
    static void install() { H::install(kHwnd, &proc); unhooked = false; }
    static void uninstall() { H::uninstall(&proc); unhooked = true; }
    static void reset() { H::old = nullptr; H::hwnd = HWND(); H::passthrough = 0; H::pins = 0; handled = handledAfterUnhook = 0; unhooked = false; }
    static Proc procPtr() { return &proc; }
};

struct Ops {
    void (*install)(); void (*uninstall)(); void (*reset)();
    int (*handled)(); int (*afterUnhook)(); int (*pins)(); bool (*passthrough)(); Proc (*procPtr)();
};
template <class S>
static Ops opsFor() {
    return {&S::install, &S::uninstall, &S::reset, [] { return S::handled; }, [] { return S::handledAfterUnhook; },
            [] { return S::H::pins; }, [] { return S::H::passthrough != 0; }, &S::procPtr};
}
static std::array<Ops, 3> allOps() { return {opsFor<Sub<0>>(), opsFor<Sub<1>>(), opsFor<Sub<2>>()}; }

static void resetWorld() {
    Reset();
    kp::whapi::pinCalls = kp::whapi::setCalls = kp::whapi::defHits = 0;
    for (auto& o : allOps()) o.reset();
}

static std::string tag(const char* what, const std::array<int, 3>& ins, const std::array<int, 3>& un, int step) {
    std::string s = what;
    s += " ins=";
    for (int i : ins) s += char('A' + i);
    s += " un=";
    for (int i : un) s += char('A' + i);
    s += " step=" + std::to_string(step);
    return s;
}

// Returns true when the window's top proc is the base game proc after every DLL unhooked.
static bool runScenario(const std::array<int, 3>& ins, const std::array<int, 3>& un) {
    auto ops = allOps();
    resetWorld();
    bool live[3] = {false, false, false};
    int liveMsgs[3] = {0, 0, 0};
    int sent = 0, step = 0;
    auto verifyStep = [&](const char* what) {
        ++step;
        std::string t = tag(what, ins, un, step);
        for (int rep = 0; rep < 2; ++rep) {
            int before[3];
            for (int i = 0; i < 3; ++i) before[i] = ops[i].handled();
            int baseBefore = base().hits;
            intptr_t r = FakeSendMessage(0x20A, 7, 41);
            ++sent;
            ::kt::Check((t + ": base proc received the message exactly once, return == l+1").c_str(), base().hits == baseBefore + 1 && r == 42);
            for (int i = 0; i < 3; ++i) {
                int d = ops[i].handled() - before[i];
                if (live[i]) { ++liveMsgs[i]; ::kt::Check((t + ": live proc handled the message exactly once").c_str(), d == 1); }
                else ::kt::Check((t + ": non-live proc did not handle the message").c_str(), d == 0);
            }
        }
    };
    for (int i : ins) {
        ops[i].install();
        live[i] = true;
        ::kt::Check((tag("install", ins, un, 0) + ": exactly one pin per tag").c_str(), ops[i].pins() == 1);
        verifyStep("after-install");
    }
    for (int i : un) { ops[i].uninstall(); live[i] = false; verifyStep("after-unhook"); }
    std::string ft = tag("final", ins, un, 0);
    ::kt::Check((ft + ": base proc got every message exactly once overall").c_str(), base().hits == sent);
    ::kt::CheckEq((ft + ": pin calls total").c_str(), kp::whapi::pinCalls, 3);
    for (int i = 0; i < 3; ++i) {
        ::kt::Check((ft + ": no handling after unhook").c_str(), ops[i].afterUnhook() == 0);
        ::kt::Check((ft + ": handled == messages sent while live").c_str(), ops[i].handled() == liveMsgs[i]);
        ::kt::Check((ft + ": pins still 1").c_str(), ops[i].pins() == 1);
    }
    return FakeGetWindowLongPtr(kHwnd) == &BaseGameProc;
}

int main() {
    // fresh statics (before any reset)
    KT_CHECK(Sub<0>::H::old == nullptr && Sub<0>::H::hwnd == HWND());
    KT_CHECK_EQ(Sub<0>::H::passthrough, 0);
    KT_CHECK_EQ(Sub<0>::H::pins, 0);

    // (a) all 12 scenarios
    const std::array<std::array<int, 3>, 2> orders = {{{0, 1, 2}, {2, 0, 1}}};
    int scenarios = 0;
    for (const auto& ins : orders) {
        std::array<int, 3> un = {0, 1, 2};
        do {
            ++scenarios;
            bool baseOnTop = runScenario(ins, un);
            std::array<int, 3> rev = {ins[2], ins[1], ins[0]};
            if (un == rev) ::kt::Check((tag("final", ins, un, 0) + ": reverse-order unhook ends with the base proc on top").c_str(), baseOnTop);
        } while (std::next_permutation(un.begin(), un.end()));
    }
    KT_CHECK_EQ(scenarios, 12);

    // (b) pin exactly once per tag across repeated install
    {
        resetWorld();
        using H = Sub<0>::H;
        KT_CHECK(H::install(kHwnd, &Sub<0>::proc));
        KT_CHECK(H::install(kHwnd, &Sub<0>::proc));
        KT_CHECK(H::install(kHwnd, &Sub<0>::proc));
        KT_CHECK_EQ(kp::whapi::pinCalls, 1);
        KT_CHECK_EQ(H::pins, 1);
        KT_CHECK_EQ(kp::whapi::setCalls, 1);  // old already set: no second setProc
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == &Sub<0>::proc);
        KT_CHECK(H::old == &BaseGameProc);
        KT_CHECK(H::hwnd == kHwnd);
        // a different tag pins separately
        KT_CHECK(Sub<1>::H::install(kHwnd, &Sub<1>::proc));
        KT_CHECK_EQ(kp::whapi::pinCalls, 2);
        // install on an unknown window fails, keeps old null
        KT_CHECK(!Sub<2>::H::install(0x7777, &Sub<2>::proc));
        KT_CHECK(Sub<2>::H::old == nullptr);
        KT_CHECK(Sub<2>::H::hwnd == HWND());
        KT_CHECK(!Sub<2>::H::active());
    }

    // (c) A, then B on top, uninstall A -> pass-through; re-install A clears it without a second setProc
    {
        resetWorld();
        Sub<0>::install();
        Sub<1>::install();
        KT_CHECK_EQ(kp::whapi::setCalls, 2);
        Sub<0>::uninstall();
        KT_CHECK_EQ(Sub<0>::H::passthrough, 1);
        KT_CHECK(!Sub<0>::H::active());
        KT_CHECK(Sub<0>::H::old == &BaseGameProc);  // old kept: still chained
        KT_CHECK(Sub<0>::H::hwnd == kHwnd);
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == &Sub<1>::proc);
        int h0 = Sub<0>::handled;
        FakeSendMessage(1, 0, 0);
        KT_CHECK_EQ(Sub<0>::handled, h0);
        Sub<0>::install();
        KT_CHECK_EQ(Sub<0>::H::passthrough, 0);
        KT_CHECK(Sub<0>::H::active());
        KT_CHECK_EQ(kp::whapi::setCalls, 2);
        KT_CHECK_EQ(kp::whapi::pinCalls, 2);
        KT_CHECK_EQ(Sub<0>::H::pins, 1);
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == &Sub<1>::proc);
        int b = base().hits;
        intptr_t r = FakeSendMessage(1, 0, 10);
        KT_CHECK_EQ(Sub<0>::handled, h0 + 1);
        KT_CHECK_EQ(base().hits, b + 1);
        KT_CHECK_EQ(r, 11);
    }

    // (d) uninstall when the window is gone is a no-op
    {
        resetWorld();
        using H = Sub<0>::H;
        Sub<0>::install();
        int sets = kp::whapi::setCalls;
        Proc oldBefore = H::old;
        H::hwnd = 0x9999;  // bogus: !isWindow
        H::uninstall(&Sub<0>::proc);
        KT_CHECK(H::old == oldBefore);
        KT_CHECK_EQ(H::passthrough, 0);
        KT_CHECK(H::hwnd == 0x9999);
        KT_CHECK_EQ(kp::whapi::setCalls, sets);
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == &Sub<0>::proc);
        // never installed: also a no-op
        Sub<1>::H::uninstall(&Sub<1>::proc);
        KT_CHECK_EQ(Sub<1>::H::passthrough, 0);
        KT_CHECK_EQ(kp::whapi::setCalls, sets);
        // old set but hwnd null: no-op
        H::hwnd = HWND();
        H::uninstall(&Sub<0>::proc);
        KT_CHECK(H::old == oldBefore);
        KT_CHECK_EQ(H::passthrough, 0);
        KT_CHECK_EQ(kp::whapi::setCalls, sets);
    }

    // (e) forward with no old proc hits defProc; with one, it calls through
    {
        resetWorld();
        using H = Sub<0>::H;
        KT_CHECK(!H::active());
        KT_CHECK_EQ(H::forward(kHwnd, 1, 2, 3), kp::whapi::kDef);
        KT_CHECK_EQ(kp::whapi::defHits, 1);
        KT_CHECK_EQ(base().hits, 0);
        Sub<0>::install();
        KT_CHECK_EQ(H::forward(kHwnd, 1, 2, 3), 4);
        KT_CHECK_EQ(kp::whapi::defHits, 1);
        KT_CHECK_EQ(base().hits, 1);
        // top uninstall restores and clears everything
        Sub<0>::uninstall();
        KT_CHECK(H::old == nullptr);
        KT_CHECK(H::hwnd == HWND());
        KT_CHECK_EQ(H::passthrough, 0);
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == &BaseGameProc);
    }
    // (f) hooked window destroyed and recreated under a new HWND: install() drops the dead chain and subclasses the new window
    {
        resetWorld();
        using H = Sub<0>::H;
        const HWND kNew = 0x2222;
        Sub<0>::install();
        KT_CHECK(H::hwnd == kHwnd);
        KT_CHECK_EQ(kp::whapi::setCalls, 1);
        window().h = kNew;               // old window destroyed, a new one exists: FakeIsWindow(kHwnd) is now false
        window().proc = &BaseGameProc;   // the new window starts on the class proc
        KT_CHECK(!kp::whapi::isWindow(kHwnd));
        KT_CHECK(H::install(kNew, &Sub<0>::proc));
        KT_CHECK_EQ(kp::whapi::setCalls, 2);  // exactly one setProc, on the new window; none on the dead one
        KT_CHECK(H::hwnd == kNew);
        KT_CHECK(H::old == &BaseGameProc);
        KT_CHECK_EQ(H::passthrough, 0);
        KT_CHECK(H::active());
        KT_CHECK_EQ(H::pins, 1);
        KT_CHECK_EQ(kp::whapi::pinCalls, 1);
        KT_CHECK(FakeGetWindowLongPtr(kNew) == &Sub<0>::proc);
        int b = base().hits;
        KT_CHECK_EQ(FakeSendMessage(1, 0, 10), 11);  // new chain: Sub<0>::proc -> new old proc (base)
        KT_CHECK_EQ(Sub<0>::handled, 1);
        KT_CHECK_EQ(base().hits, b + 1);
        KT_CHECK(H::install(kNew, &Sub<0>::proc));   // repeat on the live window: no second setProc
        KT_CHECK_EQ(kp::whapi::setCalls, 2);
        Sub<0>::uninstall();                          // restores the NEW window's proc
        KT_CHECK(H::old == nullptr);
        KT_CHECK(H::hwnd == HWND());
        KT_CHECK(FakeGetWindowLongPtr(kNew) == &BaseGameProc);
    }

    // (g) stale pass-through state on a dead window is dropped too: the re-install is a real subclass, not just "clear the flag"
    {
        resetWorld();
        using H = Sub<0>::H;
        Sub<0>::install();
        Sub<1>::install();
        Sub<0>::uninstall();              // not on top: pass-through, old kept
        KT_CHECK_EQ(H::passthrough, 1);
        window().h = 0x2222;              // window recreated
        window().proc = &BaseGameProc;
        int sets = kp::whapi::setCalls;
        KT_CHECK(H::install(0x2222, &Sub<0>::proc));
        KT_CHECK_EQ(kp::whapi::setCalls, sets + 1);
        KT_CHECK_EQ(H::passthrough, 0);
        KT_CHECK(H::hwnd == 0x2222);
        KT_CHECK(H::old == &BaseGameProc);
        KT_CHECK(FakeGetWindowLongPtr(0x2222) == &Sub<0>::proc);
    }

    // (h) live stored window: install() with another HWND keeps the chain (no resubclass of a window that still exists)
    {
        resetWorld();
        using H = Sub<0>::H;
        Sub<0>::install();
        KT_CHECK(H::install(0x2222, &Sub<0>::proc));
        KT_CHECK(H::hwnd == kHwnd);
        KT_CHECK(H::old == &BaseGameProc);
        KT_CHECK_EQ(kp::whapi::setCalls, 1);
    }
    // (i) the providers' re-arm gate (GuiInputProvider rearmWheelHook / ActionHotbar ensureWheelHook): `active() && isWindow(hwnd)` means healthy.
    // Healthy polls never touch the window; after the window is recreated the first re-arm re-subclasses it exactly once, later polls are healthy again.
    {
        resetWorld();
        using H = Sub<0>::H;
        auto rearm = [](HWND cur) { if (H::active() && kp::whapi::isWindow(H::hwnd)) return false; H::install(cur, &Sub<0>::proc); return true; };
        Sub<0>::install();
        for (int i = 0; i < 100; ++i) KT_CHECK(!rearm(kHwnd));   // healthy: no install call at all
        KT_CHECK_EQ(kp::whapi::setCalls, 1);
        window().h = 0x2222; window().proc = &BaseGameProc;       // game recreated its window
        KT_CHECK(rearm(0x2222));
        KT_CHECK_EQ(kp::whapi::setCalls, 2);
        for (int i = 0; i < 100; ++i) KT_CHECK(!rearm(0x2222));
        KT_CHECK_EQ(kp::whapi::setCalls, 2);
        KT_CHECK(FakeGetWindowLongPtr(0x2222) == &Sub<0>::proc);
    }
    return kt::Report("windowhook_test");
}
