// wndproc_chain_test.cpp - WndProc subclass chain over one HWND (doc 20 T3).
//
// Several mod DLLs subclass the game window with the same pattern (tests/static/subclassers.toml):
//   install:  if (!g_oldProc) g_oldProc = SetWindowLongPtr(hwnd, GWLP_WNDPROC, proc);
//   proc:     ... return CallWindowProc(g_oldProc, hwnd, msg, w, l);
//   unload:   if (g_oldProc && IsWindow(hwnd) && GetWindowLongPtr(hwnd, GWLP_WNDPROC) == proc) {
//                 SetWindowLongPtr(hwnd, GWLP_WNDPROC, g_oldProc); g_oldProc = nullptr; }
// Sub<N,true> mirrors that exactly; Sub<N,false> omits the still-ours check (the computed negative).
//
// KNOWN HAZARD F28: unhooking a subclasser that is NOT on top skips the restore, so its proc stays chained after the
// DLL considers itself unhooked (a WndProc leak, and a dangling pointer once that DLL unloads). It is also never
// retried (g_oldProc stays set), and when the subclasser above it later restores, the leaked proc becomes the
// window's top proc again. Only unhooking in exact reverse install order is leak-free. These tests characterise that
// behaviour (assert the leak where the model says it exists) and assert the safe invariants: the base proc gets every
// message, no proc sees a message twice, every still-installed proc sees every message.
#include "check.h"
#include "fake_wndproc.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

using namespace fakewnd;

template <int N, bool CheckOurs = true>
struct Sub {
    inline static Proc g_oldProc = nullptr;
    inline static int hits = 0;
    inline static int postUnhookHits = 0;  // messages received after the DLL considered itself unhooked (test bookkeeping)
    inline static bool unhooked = false;   // test bookkeeping, not part of the real pattern

    static intptr_t proc(HWND h, unsigned m, intptr_t w, intptr_t l) {
        ++hits;
        if (unhooked) ++postUnhookHits;
        return FakeCallWindowProc(g_oldProc, h, m, w, l);
    }
    static void install() {
        if (!g_oldProc) {
            g_oldProc = FakeSetWindowLongPtr(kHwnd, &proc);
            unhooked = false;
        }
    }
    static void uninstall() {
        if (g_oldProc && FakeIsWindow(kHwnd) && (!CheckOurs || FakeGetWindowLongPtr(kHwnd) == &proc)) {
            FakeSetWindowLongPtr(kHwnd, g_oldProc);
            g_oldProc = nullptr;
        }
        unhooked = true;
    }
    static void reset() { g_oldProc = nullptr; hits = postUnhookHits = 0; unhooked = false; }
    static bool installedFlag() { return g_oldProc != nullptr; }
    static Proc procPtr() { return &proc; }
};

struct Ops {
    void (*install)();
    void (*uninstall)();
    void (*reset)();
    int (*hits)();
    int (*postUnhook)();
    bool (*unhooked)();
    Proc (*procPtr)();
};
template <class S>
Ops opsFor() {
    return {&S::install, &S::uninstall, &S::reset, [] { return S::hits; }, [] { return S::postUnhookHits; },
            [] { return S::unhooked; }, &S::procPtr};
}
static std::array<Ops, 3> checkedOps() { return {opsFor<Sub<0, true>>(), opsFor<Sub<1, true>>(), opsFor<Sub<2, true>>()}; }
static std::array<Ops, 3> uncheckedOps() { return {opsFor<Sub<0, false>>(), opsFor<Sub<1, false>>(), opsFor<Sub<2, false>>()}; }

// Independent model of the intended semantics (a linked list from the window's top proc down to the base).
struct Model {
    static constexpr int kBase = -1;
    bool check;
    int top = kBase;
    int below[3] = {kBase, kBase, kBase};
    bool held[3] = {false, false, false};  // g_oldProc != nullptr
    explicit Model(bool c) : check(c) {}
    void install(int i) { if (!held[i]) { below[i] = top; top = i; held[i] = true; } }
    void uninstall(int i) {
        if (held[i] && (!check || top == i)) { top = below[i]; held[i] = false; }
    }
    // Procs a message visits. A proc whose g_oldProc was cleared forwards to nullptr (a crash in the real game), so the
    // walk ends there and the base proc is not reached.
    std::vector<int> chain() const {
        std::vector<int> c;
        for (int t = top; t != kBase; t = below[t]) { c.push_back(t); if (!held[t]) break; }
        return c;
    }
};

static const char* kName[3] = {"A", "B", "C"};

struct StepResult {
    bool baseGotIt = false;      // base proc received the message exactly once
    int maxHits = 0;             // max hits any one proc got for this message
    int hits[3] = {0, 0, 0};
};

static StepResult sendOne(const std::array<Ops, 3>& ops) {
    int before[3];
    for (int i = 0; i < 3; ++i) before[i] = ops[i].hits();
    int baseBefore = base().hits;
    intptr_t r = FakeSendMessage(0x20A, 7, 41);
    StepResult s;
    s.baseGotIt = (base().hits == baseBefore + 1) && r == 42;
    for (int i = 0; i < 3; ++i) { s.hits[i] = ops[i].hits() - before[i]; s.maxHits = std::max(s.maxHits, s.hits[i]); }
    return s;
}

static std::string tag(const char* what, const std::array<int, 3>& ins, const std::array<int, 3>& un, int step) {
    std::string s = what;
    s += " install=";
    for (int i : ins) s += kName[i];
    s += " unhook=";
    for (int i : un) s += kName[i];
    s += " step=" + std::to_string(step);
    return s;
}

struct ScenarioOut {
    bool leak = false;          // a proc still chained after every DLL unhooked
    bool staleOnTop = false;    // at some step an already-unhooked proc was the window's top proc
    int liveLost = 0;           // live (not unhooked) subclassers that missed a message at some step
    bool modelMatch = true;
};

// Runs one install order + unhook order; sends two messages after every step; compares with the model.
static ScenarioOut runScenario(bool check, const std::array<int, 3>& ins, const std::array<int, 3>& un, bool report) {
    auto ops = check ? checkedOps() : uncheckedOps();
    Reset();
    for (auto& o : ops) o.reset();
    Model m(check);
    ScenarioOut out;
    bool live[3] = {false, false, false};
    int step = 0;
    auto verifyStep = [&](const char* what) {
        ++step;
        for (int rep = 0; rep < 2; ++rep) {
            StepResult s = sendOne(ops);
            std::vector<int> chain = m.chain();
            bool inChain[3] = {false, false, false};
            for (int c : chain) inChain[c] = true;
            bool match = true;
            for (int i = 0; i < 3; ++i) match = match && (s.hits[i] == (inChain[i] ? 1 : 0));
            if (!match) out.modelMatch = false;
            if (report) {
                std::string t = tag(what, ins, un, step);
                ::kt::Check((t + ": hits match model").c_str(), match);
                if (check) {
                    ::kt::Check((t + ": base proc received the message exactly once (none lost)").c_str(), s.baseGotIt);
                    ::kt::Check((t + ": no proc saw the message twice").c_str(), s.maxHits <= 1);
                }
            }
            for (int i = 0; i < 3; ++i)
                if (live[i] && s.hits[i] != 1) { ++out.liveLost; if (report && check) ::kt::Check((tag("live subclasser lost", ins, un, step)).c_str(), false); }
        }
        Proc top = FakeGetWindowLongPtr(kHwnd);
        for (int i = 0; i < 3; ++i)
            if (ops[i].unhooked() && top == ops[i].procPtr()) out.staleOnTop = true;
    };
    for (int i : ins) { ops[i].install(); m.install(i); live[i] = true; verifyStep("after-install"); }
    for (int i : un)  { ops[i].uninstall(); m.uninstall(i); live[i] = false; verifyStep("after-unhook"); }
    out.leak = FakeGetWindowLongPtr(kHwnd) != &BaseGameProc;
    return out;
}

// ---- Design A model (test-only, finding F28): per-DLL pass-through + module pin ----
// install:   same as Sub, plus ++pins (stands in for GetModuleHandleExA(FROM_ADDRESS | PIN): the DLL image can never unload,
//            so a proc that stays chained never dangles).
// proc:      handles the message only while !passthrough, then ALWAYS forwards to oldProc.
// uninstall: top == ours -> restore oldProc and clear it; otherwise go pass-through and KEEP oldProc (stay chained, inert).
template <int N>
struct SubPass {
    inline static Proc oldProc = nullptr;
    inline static bool passthrough = false;
    inline static int handled = 0;
    inline static int handledAfterUnhook = 0;  // any handling after the DLL considered itself unhooked
    inline static int pins = 0;
    inline static bool unhooked = false;       // test bookkeeping

    static intptr_t proc(HWND h, unsigned m, intptr_t w, intptr_t l) {
        if (!passthrough) {
            ++handled;
            if (unhooked) ++handledAfterUnhook;
        }
        return FakeCallWindowProc(oldProc, h, m, w, l);
    }
    static void install() {
        if (!oldProc) {
            oldProc = FakeSetWindowLongPtr(kHwnd, &proc);
            unhooked = false;
            passthrough = false;
            ++pins;
        }
    }
    static void uninstall() {
        if (oldProc && FakeIsWindow(kHwnd) && FakeGetWindowLongPtr(kHwnd) == &proc) {
            FakeSetWindowLongPtr(kHwnd, oldProc);
            oldProc = nullptr;
        } else {
            passthrough = true;  // not on top: stay chained, keep oldProc, stop handling
        }
        unhooked = true;
    }
    static void reset() { oldProc = nullptr; passthrough = false; handled = handledAfterUnhook = pins = 0; unhooked = false; }
    static Proc procPtr() { return &proc; }
};

struct PassOps {
    void (*install)();
    void (*uninstall)();
    void (*reset)();
    int (*handled)();
    int (*afterUnhook)();
    int (*pins)();
    bool (*passthrough)();
    Proc (*procPtr)();
};
template <class S>
PassOps passOpsFor() {
    return {&S::install, &S::uninstall, &S::reset, [] { return S::handled; }, [] { return S::handledAfterUnhook; },
            [] { return S::pins; }, [] { return S::passthrough; }, &S::procPtr};
}
static std::array<PassOps, 3> passOps() { return {passOpsFor<SubPass<0>>(), passOpsFor<SubPass<1>>(), passOpsFor<SubPass<2>>()}; }

struct PassOut {
    bool baseOnTop = false;       // window's top proc is BaseGameProc after every DLL unhooked
    bool passChained = false;     // at least one pass-through proc is still chained (it kept its oldProc)
};

// One install order + unhook order with SubPass; two messages after every step; asserts the design-A invariants.
static PassOut runPassScenario(const std::array<int, 3>& ins, const std::array<int, 3>& un) {
    auto ops = passOps();
    Reset();
    for (auto& o : ops) o.reset();
    bool live[3] = {false, false, false};
    int liveMsgs[3] = {0, 0, 0};  // messages sent while that proc was installed and not yet unhooked
    int sent = 0;
    int step = 0;
    auto verifyStep = [&](const char* what) {
        ++step;
        std::string t = tag(what, ins, un, step);
        for (int rep = 0; rep < 2; ++rep) {
            int before[3];
            for (int i = 0; i < 3; ++i) before[i] = ops[i].handled();
            int baseBefore = base().hits;
            intptr_t r = FakeSendMessage(0x20A, 7, 41);
            ++sent;
            ::kt::Check((t + ": base proc received the message exactly once").c_str(), base().hits == baseBefore + 1 && r == 42);
            for (int i = 0; i < 3; ++i) {
                int d = ops[i].handled() - before[i];
                ::kt::Check((t + ": no proc handled a message twice").c_str(), d <= 1);
                if (live[i]) {
                    ++liveMsgs[i];
                    ::kt::Check((t + ": live proc handled the message").c_str(), d == 1);
                } else {
                    ::kt::Check((t + ": non-live proc did not handle the message").c_str(), d == 0);
                }
            }
        }
    };
    for (int i : ins) {
        ops[i].install();
        live[i] = true;
        ::kt::Check((tag("pass-install", ins, un, 0) + ": exactly one pin per proc after install").c_str(), ops[i].pins() == 1);
        verifyStep("pass-after-install");
    }
    for (int i : un) { ops[i].uninstall(); live[i] = false; verifyStep("pass-after-unhook"); }
    std::string ft = tag("pass-final", ins, un, 0);
    ::kt::Check((ft + ": base proc got every message exactly once overall").c_str(), base().hits == sent);
    PassOut out;
    for (int i = 0; i < 3; ++i) {
        ::kt::Check((ft + ": no handling after unhook").c_str(), ops[i].afterUnhook() == 0);
        ::kt::Check((ft + ": handled <= messages sent while live").c_str(), ops[i].handled() <= liveMsgs[i]);
        ::kt::Check((ft + ": pins still 1 at end").c_str(), ops[i].pins() == 1);
        if (ops[i].passthrough()) out.passChained = true;
    }
    out.baseOnTop = FakeGetWindowLongPtr(kHwnd) == &BaseGameProc;
    return out;
}

int main() {
    const std::array<std::array<int, 3>, 2> orders = {{{0, 1, 2}, {2, 0, 1}}};  // ABC, CAB
    int leakScenarios = 0, leakFree = 0, scenarios = 0;
    int staleScenarios = 0;
    int uncheckedLiveLost = 0, uncheckedScenariosWithLoss = 0, checkedLiveLost = 0;
    int modelMismatch = 0;

    for (const auto& ins : orders) {
        std::array<int, 3> un = {0, 1, 2};
        do {
            ++scenarios;
            ScenarioOut c = runScenario(true, ins, un, true);
            std::array<int, 3> rev = {ins[2], ins[1], ins[0]};
            bool expectLeakFree = (un == rev);
            std::string t = tag("final", ins, un, 0);
            ::kt::Check((t + ": leak iff unhook order is not exact reverse of install order").c_str(), c.leak == !expectLeakFree);
            if (expectLeakFree) ::kt::Check((t + ": reverse-order unhook ends with the base proc back on the window").c_str(), !c.leak);
            if (expectLeakFree) ::kt::Check((t + ": reverse-order unhook never puts a stale proc on top").c_str(), !c.staleOnTop);
            if (c.leak) ++leakScenarios; else ++leakFree;
            if (c.staleOnTop) ++staleScenarios;
            checkedLiveLost += c.liveLost;
            if (!c.modelMatch) ++modelMismatch;

            ScenarioOut u = runScenario(false, ins, un, true);
            uncheckedLiveLost += u.liveLost;
            if (u.liveLost) ++uncheckedScenariosWithLoss;
            if (!u.modelMatch) ++modelMismatch;
        } while (std::next_permutation(un.begin(), un.end()));
    }

    KT_CHECK_EQ(scenarios, 12);
    KT_CHECK_EQ(modelMismatch, 0);
    // Safe invariant with the still-ours check: no live subclasser is ever dropped from the chain.
    KT_CHECK_EQ(checkedLiveLost, 0);
    // KNOWN HAZARD F28: 10 of 12 (install order x unhook order) scenarios leave a proc chained; only reverse order is clean.
    ::kt::CheckEq("KNOWN HAZARD F28: scenarios that leak a WndProc (middle/bottom unhook skips restore)", leakScenarios, 10);
    ::kt::CheckEq("exactly the 2 reverse-order scenarios are leak-free", leakFree, 2);
    ::kt::Check("KNOWN HAZARD F28: a leaked proc can become the window's top proc again after a later restore", staleScenarios > 0);

    // ---- named hazard walk-through: install A,B,C then unhook B (middle) ----
    {
        Reset();
        using SA = Sub<0, true>; using SB = Sub<1, true>; using SC = Sub<2, true>;
        SA::reset(); SB::reset(); SC::reset();
        SA::install(); SB::install(); SC::install();
        SB::uninstall();
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == SC::procPtr());                       // restore skipped: not ours
        KT_CHECK(SB::g_oldProc != nullptr);                                           // still holds its old proc
        int b0 = SB::postUnhookHits;
        FakeSendMessage(0x20A, 0, 0);
        ::kt::Check("KNOWN HAZARD F28: unhooked middle subclasser B still receives messages (WndProc leak)", SB::postUnhookHits == b0 + 1);
        int bBefore = SB::hits;
        SB::install();                                                                // a second install is refused: g_oldProc still set
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == SC::procPtr());
        KT_CHECK_EQ(SB::hits, bBefore);
        SC::uninstall();                                                              // C is on top: restores to B (leaked, stale)
        ::kt::Check("KNOWN HAZARD F28: C's restore puts the already-unhooked B proc back on top", FakeGetWindowLongPtr(kHwnd) == SB::procPtr());
        SA::uninstall();                                                              // A not top (B is): skipped
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == SB::procPtr());
        base().hits = 0;
        FakeSendMessage(0x20A, 0, 0);
        KT_CHECK_EQ(base().hits, 1);                                                  // base still gets the message: nothing is lost, only leaked
    }

    // ---- computed negative: without the still-ours check a stale restore unhooks later (live) subclassers ----
    ::kt::Check("negative: without the still-ours check, live subclassers get dropped in some scenarios", uncheckedLiveLost > 0);
    ::kt::Check("negative: ... in more than one scenario", uncheckedScenariosWithLoss > 1);
    {
        Reset();
        using SA = Sub<0, false>; using SB = Sub<1, false>;
        SA::reset(); SB::reset();
        SA::install(); SB::install();
        SA::uninstall();  // unchecked: stale restore of A's old proc (the base) over live B
        KT_CHECK(FakeGetWindowLongPtr(kHwnd) == &BaseGameProc);
        int b0 = SB::hits;
        FakeSendMessage(0x20A, 0, 0);
        ::kt::Check("negative: unchecked A unhook silently unhooks live B (B receives nothing)", SB::hits == b0);
    }
    {
        Reset();
        using SA = Sub<0, true>; using SB = Sub<1, true>;
        SA::reset(); SB::reset();
        SA::install(); SB::install();
        SA::uninstall();  // checked: skipped, B stays live
        int b0 = SB::hits;
        FakeSendMessage(0x20A, 0, 0);
        ::kt::Check("positive: checked A unhook leaves live B receiving messages", SB::hits == b0 + 1);
    }

    // ---- design A (per-DLL pass-through + module pin): same 2 install orders x 6 unhook orders ----
    {
        int passScenarios = 0, passLeaveChained = 0, passTopMismatch = 0;
        for (const auto& ins : orders) {
            std::array<int, 3> un = {0, 1, 2};
            do {
                ++passScenarios;
                PassOut p = runPassScenario(ins, un);
                std::array<int, 3> rev = {ins[2], ins[1], ins[0]};
                if (un == rev) {
                    ::kt::Check((tag("pass-final", ins, un, 0) + ": reverse-order unhook ends with BaseGameProc on top").c_str(), p.baseOnTop);
                    ::kt::Check((tag("pass-final", ins, un, 0) + ": reverse-order unhook leaves no pass-through proc chained").c_str(), !p.passChained);
                }
                if (p.passChained) ++passLeaveChained;
                if (p.passChained == p.baseOnTop) ++passTopMismatch;  // chained pass-through <=> base not on top
            } while (std::next_permutation(un.begin(), un.end()));
        }
        KT_CHECK_EQ(passScenarios, 12);
        KT_CHECK_EQ(passTopMismatch, 0);
        // CHARACTERIZATION (F28, design A): pass-through makes a leftover proc inert (handledAfterUnhook == 0 above) and the
        // module pin keeps it from dangling, but it does NOT unlink it: in every non-reverse order (10 of 12) at least one
        // forwarding pass-through proc stays on the chain. Only exact reverse order ends with BaseGameProc on top.
        ::kt::CheckEq("design A CHARACTERIZATION: scenarios that leave a pass-through proc chained", passLeaveChained, 10);
    }

    return kt::Report("wndproc_chain_test");
}
