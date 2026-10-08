// gi_dispatch_test.cpp -- Unit test of gid::Dispatcher (GiDispatch.h).
// Host test, no Win32 APIs, verifies registration, capacity, FIFO dispatch, short-circuit consumption, and unsubscription.
#include "GiDispatch.h"
#include "check.h"
#include <vector>
#include <string>

#define GCHECK(why, expr) ::kt::Check(why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define GEQ(why, a, b) ::kt::CheckEq(why " (line " KT_STR(__LINE__) ")", (a), (b))

static std::vector<std::string> g_log;

static int32_t KPATCH_CALL cbLog1(const K2WheelEvent* ev, void* user) {
    g_log.push_back("cb1:" + std::to_string(ev->delta) + ":" + (user ? static_cast<const char*>(user) : "null"));
    return 0; // not consumed
}

static int32_t KPATCH_CALL cbLog2(const K2WheelEvent* ev, void* user) {
    g_log.push_back("cb2:" + std::to_string(ev->delta) + ":" + (user ? static_cast<const char*>(user) : "null"));
    return 0; // not consumed
}

static int32_t KPATCH_CALL cbConsume(const K2WheelEvent* ev, void*) {
    g_log.push_back("consume:" + std::to_string(ev->delta));
    return 1; // consumed!
}

static gid::Dispatcher* g_pDisp = nullptr;
static int32_t KPATCH_CALL cbUnsubSelf(const K2WheelEvent*, void*) {
    g_log.push_back("unsub_self");
    if (g_pDisp) g_pDisp->unsubscribe(cbUnsubSelf);
    return 0;
}

int main() {
    gid::Dispatcher d;
    GEQ("initially empty", d.empty(), true);
    GEQ("initial count is 0", d.count, 0);

    // 1. Subscribe edge cases
    GEQ("subscribe null cb fails with -1", d.subscribe(nullptr, nullptr), -1);

    int32_t t1 = d.subscribe(cbLog1, (void*)"u1", "mod1");
    GCHECK("t1 > 0", t1 > 0);
    GEQ("count is 1", d.count, 1);

    int32_t t2 = d.subscribe(cbLog2, (void*)"u2", "mod2");
    GCHECK("t2 > t1", t2 > t1);
    GEQ("count is 2", d.count, 2);

    // 2. Dispatch FIFO and non-consumption
    g_log.clear();
    K2WheelEvent ev = { sizeof(K2WheelEvent), 120, 100.0f, 200.0f, 50, 60, 1024, 768 };
    bool consumed = d.dispatch(ev);
    GEQ("dispatch returns false when unconsumed", consumed, false);
    GEQ("2 callbacks fired", (int)g_log.size(), 2);
    GCHECK("cb1 fired first", g_log[0] == "cb1:120:u1");
    GCHECK("cb2 fired second", g_log[1] == "cb2:120:u2");

    // 3. Short-circuit consumption
    d.subscribe(cbConsume, nullptr, "mod_cons");
    d.subscribe(cbLog2, (void*)"after", "mod_after");

    g_log.clear();
    consumed = d.dispatch(ev);
    GEQ("dispatch returns true when consumed", consumed, true);
    GEQ("3 callbacks fired (after skipped)", (int)g_log.size(), 3);
    GCHECK("cb1 fired", g_log[0] == "cb1:120:u1");
    GCHECK("cb2 fired", g_log[1] == "cb2:120:u2");
    GCHECK("cbConsume fired", g_log[2] == "consume:120");

    // 4. Unsubscribe by function pointer
    GCHECK("unsubscribe cbConsume returns true", d.unsubscribe(cbConsume));
    GEQ("count decreased", d.count, 3);
    GCHECK("cbConsume is gone", d.unsubscribe(cbConsume) == false);

    g_log.clear();
    consumed = d.dispatch(ev);
    GEQ("dispatch returns false without consumer", consumed, false);
    GEQ("all 3 remaining fired", (int)g_log.size(), 3);
    GCHECK("first is cb1", g_log[0] == "cb1:120:u1");
    GCHECK("second is cb2 (u2)", g_log[1] == "cb2:120:u2");
    GCHECK("third is cb2 (after)", g_log[2] == "cb2:120:after");

    // Unsubscribe cbLog2 should remove BOTH instances
    GCHECK("unsubscribe cbLog2 removes both", d.unsubscribe(cbLog2));
    GEQ("count is now 1 (only cbLog1 remains)", d.count, 1);

    // 5. Unsubscribe by token
    GCHECK("unsubscribe token t1 returns true", d.unsubscribeToken(t1));
    GEQ("empty after unsubscribing last", d.empty(), true);
    GCHECK("unsubscribe invalid token returns false", !d.unsubscribeToken(999));
    GCHECK("unsubscribe 0 token returns false", !d.unsubscribeToken(0));

    // 6. MaxSubscribers capacity limit
    for (int i = 0; i < gid::MaxSubscribers; ++i) {
        int32_t tok = d.subscribe(cbLog1, nullptr);
        GCHECK("slot subscribed", tok > 0);
    }
    GEQ("table full", d.count, gid::MaxSubscribers);
    GEQ("subscribe beyond limit returns -2", d.subscribe(cbLog2, nullptr), -2);

    // 7. Snapshot safety during callback
    d.unsubscribe(cbLog1);
    GEQ("cleared", d.count, 0);
    g_pDisp = &d;
    d.subscribe(cbUnsubSelf, nullptr);
    d.subscribe(cbLog1, (void*)"after_unsub");
    g_log.clear();
    d.dispatch(ev);
    GEQ("both callbacks in snapshot ran", (int)g_log.size(), 2);
    GCHECK("unsub_self ran", g_log[0] == "unsub_self");
    GCHECK("after_unsub ran", g_log[1] == "cb1:120:after_unsub");
    GEQ("cbUnsubSelf was removed", d.count, 1);

    return ::kt::Report("gi_dispatch_test");
}
