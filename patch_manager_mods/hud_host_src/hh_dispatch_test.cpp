// Host test of the hud-host dispatcher (HhDispatch.h): priority order, stable ties, unregister, limits, snapshot iteration,
// FRAME-before-PRE sequencing, context construction (hud = menu - 0xCC, targetType mapping, struct_size) and a consumer wired
// through k2hud::Client against a fake provider (tests/include/FakeKPatchApi.h). HL1.
#include "HhDispatch.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include <string.h>
#include <string>

static std::string g_trace;
static void rec(const char* tag, const K2HudContext* c) {
    char b[64]; snprintf(b, sizeof b, "%s@p%d;", tag, c->phase);
    g_trace += b;
}
#define CB(name) static void KPATCH_CALL name(const K2HudContext* c, void*) { rec(#name, c); }
CB(a) CB(b) CB(c3) CB(dd) CB(frameCb) CB(preCb)

static int g_selfToken = 0; static hhd::Dispatcher* g_selfDisp = nullptr;
static void KPATCH_CALL selfRemover(const K2HudContext* c, void*) { rec("self", c); g_selfDisp->remove(g_selfToken); }
static void KPATCH_CALL userCb(const K2HudContext*, void* u) { ++*static_cast<int*>(u); }

static void tOrder() {
    hhd::Dispatcher d;
    int ta = d.add(K2HUD_TARGET_MENU_PRE, 200, a, nullptr, "x");
    int tb = d.add(K2HUD_TARGET_MENU_PRE, 100, b, nullptr, "x");
    int tc = d.add(K2HUD_TARGET_MENU_PRE, 200, c3, nullptr, "x");   // tie with a: after a
    int td = d.add(K2HUD_TARGET_MENU_PRE, -5, dd, nullptr, "x");
    KT_CHECK(ta > 0 && tb > ta && tc > tb && td > tc);
    K2HudContext ctx = {}; ctx.phase = K2HUD_TARGET_MENU_PRE;
    g_trace.clear(); d.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "dd@p1;b@p1;a@p1;c3@p1;");
    // other phase is empty: nothing runs
    ctx.phase = K2HUD_FRAME; g_trace.clear(); d.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "");
    // user pointer is passed through
    int hits = 0;
    hhd::Dispatcher u; u.add(K2HUD_FRAME, 0, userCb, &hits, "u");
    ctx.phase = K2HUD_FRAME; u.run(ctx); u.run(ctx);
    KT_CHECK_EQ(hits, 2);
}

static void tUnregister() {
    hhd::Dispatcher d;
    int ta = d.add(K2HUD_FRAME, 1, a, nullptr, "x");
    int tb = d.add(K2HUD_FRAME, 2, b, nullptr, "x");
    int tc = d.add(K2HUD_FRAME, 3, c3, nullptr, "x");
    KT_CHECK(d.remove(tb));
    KT_CHECK(!d.remove(tb));            // already gone
    KT_CHECK(!d.remove(9999));
    KT_CHECK(!d.remove(0)); KT_CHECK(!d.remove(-1));
    K2HudContext ctx = {}; ctx.phase = K2HUD_FRAME;
    g_trace.clear(); d.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "a@p0;c3@p0;");
    KT_CHECK(d.remove(ta)); KT_CHECK(d.remove(tc));
    KT_CHECK(d.empty(K2HUD_FRAME));
    // a token is never reused
    int te = d.add(K2HUD_FRAME, 1, a, nullptr, "x");
    KT_CHECK(te > tc);
    // a callback that unregisters itself does not skip the next one
    hhd::Dispatcher s; g_selfDisp = &s;
    g_selfToken = s.add(K2HUD_FRAME, 1, selfRemover, nullptr, "x");
    s.add(K2HUD_FRAME, 2, b, nullptr, "x");
    g_trace.clear(); s.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "self@p0;b@p0;");
    g_trace.clear(); s.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "b@p0;");
}

static void tLimits() {
    hhd::Dispatcher d;
    for (int i = 0; i < hhd::MaxPerPhase; ++i) KT_CHECK(d.add(K2HUD_FRAME, i, a, nullptr, "x") > 0);
    KT_CHECK(d.add(K2HUD_FRAME, 0, a, nullptr, "x") <= 0);                       // table full
    KT_CHECK(d.add(K2HUD_TARGET_MENU_PRE, 0, a, nullptr, "x") > 0);              // other phase has its own table
    KT_CHECK(d.add(K2HUD_FRAME, 0, nullptr, nullptr, "x") <= 0);                 // null fn
    hhd::Dispatcher e;
    KT_CHECK(e.add(K2HUD_FRAME, 0, nullptr, nullptr, "x") <= 0);
    KT_CHECK(e.add(-1, 0, a, nullptr, "x") <= 0);                                // bad phase
    KT_CHECK(e.add(K2HUD_PHASE_COUNT, 0, a, nullptr, "x") <= 0);
    KT_CHECK(e.add(99, 0, a, nullptr, "x") <= 0);
    KT_CHECK(e.empty(K2HUD_FRAME) && e.empty(K2HUD_BUILD));                      // refused adds left nothing behind
    KT_CHECK(e.empty(-1) && e.empty(K2HUD_PHASE_COUNT));
    // reserved phases accept registrations (they just never fire yet)
    KT_CHECK(e.add(K2HUD_TARGET_MENU_POST, 0, a, nullptr, "x") > 0);
    KT_CHECK(e.add(K2HUD_BUILD, 0, a, nullptr, "x") > 0);
    KT_CHECK(e.add(K2HUD_FRAME, 0, a, nullptr, nullptr) > 0);                    // null owner is tolerated
}

static void tPopulate() {
    hhd::Dispatcher d;
    // registration order deliberately reversed: PRE first, FRAME second; phases decide, not registration order
    d.add(K2HUD_TARGET_MENU_PRE, 0, preCb, nullptr, "x");
    d.add(K2HUD_FRAME, 0, frameCb, nullptr, "x");
    d.add(K2HUD_TARGET_MENU_POST, 0, a, nullptr, "x");   // reserved: must not fire
    d.add(K2HUD_BUILD, 0, b, nullptr, "x");              // reserved: must not fire
    K2HudContext ctx = hhd::makeContext(0x1000 + 0xCC, 0x2000, 1, 0x3000, 2, 5);
    g_trace.clear(); hhd::runPopulate(d, ctx);
    KT_CHECK_EQ(g_trace.c_str(), "frameCb@p0;preCb@p1;");
    // with no FRAME consumers the PRE one still runs
    hhd::Dispatcher p; p.add(K2HUD_TARGET_MENU_PRE, 0, preCb, nullptr, "x");
    g_trace.clear(); hhd::runPopulate(p, ctx);
    KT_CHECK_EQ(g_trace.c_str(), "preCb@p1;");
    hhd::Dispatcher none; g_trace.clear(); hhd::runPopulate(none, ctx);
    KT_CHECK_EQ(g_trace.c_str(), "");
}

static void tContext() {
    K2HudContext c = hhd::makeContext(0x1000 + 0xCC, 0x2000, 1, 0x3000, 2, 5);
    KT_CHECK_EQ(c.struct_size, uint32_t(sizeof(K2HudContext)));
    KT_CHECK_EQ(c.hud, 0x1000); KT_CHECK_EQ(c.menu, 0x10CC);
    KT_CHECK_EQ(c.leader, 0x2000); KT_CHECK_EQ(c.forceUnusable, 1); KT_CHECK_EQ(c.target, 0x3000);
    KT_CHECK_EQ(c.targetType, 2); KT_CHECK_EQ(c.frame, 5u); KT_CHECK_EQ(c.panel, 0);
    KT_CHECK_EQ(hhd::makeContext(0, 0, 0, 0, 0, 1).hud, 0);                  // null menu -> unknown hud
    KT_CHECK_EQ(hhd::makeContext(0xCC, 0, 0, 0, 0, 1).hud, 0);               // menu == 0xCC -> hud 0 stays "unknown"
    // targetType: 0..3 pass through, 0xFF / negative / read failure (-1) -> -1
    for (int t = 0; t <= 3; ++t) KT_CHECK_EQ(hhd::makeContext(0x1000, 0, 0, 0, t, 1).targetType, t);
    KT_CHECK_EQ(hhd::makeContext(0x1000, 0, 0, 0, -1, 1).targetType, -1);
    KT_CHECK_EQ(hhd::makeContext(0x1000, 0, 0, 0, 0xFF, 1).targetType, -1);
    KT_CHECK_EQ(hhd::makeContext(0x1000, 0, 0, 0, -128, 1).targetType, -1);
    // a consumer-side struct_size gate sees the full v1 size
    KT_CHECK(c.struct_size >= offsetof(K2HudContext, panel) + sizeof(c.panel));
}

// ---- a consumer wired through k2hud::Client against a fake provider ----
static hhd::Dispatcher g_disp;
static int32_t g_hudSeen = 0x1000; static uint32_t g_frameSeen = 7;
static int32_t KPATCH_CALL fReg(int32_t ph, int32_t pr, K2HudCallback fn, void* u, const char* o) { return g_disp.add(ph, pr, fn, u, o); }
static void KPATCH_CALL fUnreg(int32_t t) { g_disp.remove(t); }
static int32_t KPATCH_CALL fHud() { return g_hudSeen; }
static uint32_t KPATCH_CALL fFrame() { return g_frameSeen; }
static const K2HudApi g_fakeHud = { sizeof(K2HudApi), fReg, fUnreg, fHud, fFrame };
static int g_connectLogs = 0;
static void onConn(bool, const char*) { ++g_connectLogs; }

static void tClient() {
    fakekp::reset();
    k2hud::Client hotbar, other;
    hotbar.init(fakekp::api(), "combat-hotbar"); hotbar.onConnect = onConn;
    hotbar.add(K2HUD_TARGET_MENU_PRE, 100, preCb);
    other.init(fakekp::api(), "other"); other.onConnect = onConn;
    other.add(K2HUD_FRAME, 100, frameCb);
    other.add(K2HUD_TARGET_MENU_PRE, 50, a);
    KT_CHECK(!hotbar.connect()); KT_CHECK(!other.connect());
    KT_CHECK(!hotbar.connect());                  // second failure: no second log line
    KT_CHECK_EQ(g_connectLogs, 2);
    KT_CHECK_EQ(hotbar.hud(), 0); KT_CHECK_EQ(hotbar.frame(), 0u);   // inert without provider
    KT_CHECK_EQ(fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &g_fakeHud), 0);
    other.pollCount = 0; other.poll(); hotbar.pollCount = 0; hotbar.poll();
    KT_CHECK(other.connected && hotbar.connected);
    KT_CHECK_EQ(hotbar.hud(), 0x1000); KT_CHECK_EQ(hotbar.frame(), 7u);
    K2HudContext ctx = hhd::makeContext(0x10CC, 1, 0, 2, 1, 8);
    g_trace.clear(); hhd::runPopulate(g_disp, ctx);
    KT_CHECK_EQ(g_trace.c_str(), "frameCb@p0;a@p1;preCb@p1;");   // priority 50 < 100 within PRE
    static const K2HudApi tiny = { 8, fReg, fUnreg, fHud, fFrame };
    KT_CHECK(!k2hud::Client::usable(&tiny)); KT_CHECK(!k2hud::Client::usable(nullptr)); KT_CHECK(k2hud::Client::usable(&g_fakeHud));
}

int main() {
    tOrder(); tUnregister(); tLimits(); tPopulate(); tContext(); tClient();
    return kt::Report("hh_dispatch_test");
}
