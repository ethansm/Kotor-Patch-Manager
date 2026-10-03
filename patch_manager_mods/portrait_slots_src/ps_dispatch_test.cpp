// Host test of the portrait-slots dispatcher (PsDispatch.h) and the consumer helper (PortraitSlots.h k2ps::Client). Doc 21 / doc 20 T1+T2.
// T1: priority order, stable ties, unregister, limits, snapshot iteration. T2: two fake consumers wired like buff-duration-hud 2.0.0 and
// shield-bar 1.2.0, driven through runSlotDraw for a HUD frame (leader + 2 small cards) and a menu frame; the recorded call order must
// equal the order BuffDurationHUD v1.1.0 did that work in (ForceArrowCounters before the engine read on every slot, flag polling, FORCEMOVE,
// shield bars before buff columns in the menu, shield HUD loop from the leader call only).
#include "PsDispatch.h"
#include "K2Addr.h"
#include "check.h"
#include <string.h>
#include <string>

static std::string g_trace;
static void rec(const char* tag, const K2PsContext* c) {
    char b[64]; snprintf(b, sizeof b, "%s@%x%s;", tag, unsigned(c->slot), c->isLarge ? "L" : "");
    g_trace += b;
}
#define CB(name) static void KPATCH_CALL name(const K2PsContext* c, void*) { rec(#name, c); }
CB(a) CB(b) CB(c3) CB(dd)
// buff-duration-hud 2.0.0
CB(buffArrows) CB(buffPoll) CB(buffMenuCols)
// shield-bar 1.2.0
CB(shieldPoll) CB(shieldSlot) CB(shieldMenuBars) CB(shieldHud)

static int g_selfToken = 0; static psd::Dispatcher* g_selfDisp = nullptr;
static void KPATCH_CALL selfRemover(const K2PsContext* c, void*) { rec("self", c); g_selfDisp->remove(g_selfToken); }

static void t1() {
    psd::Dispatcher d;
    int ta = d.add(K2PS_HUD_DRAW, 200, a, nullptr, "x");
    int tb = d.add(K2PS_HUD_DRAW, 100, b, nullptr, "x");
    int tc = d.add(K2PS_HUD_DRAW, 200, c3, nullptr, "x");   // tie with a: after a
    int td = d.add(K2PS_HUD_DRAW, -5, dd, nullptr, "x");
    KT_CHECK(ta > 0 && tb > ta && tc > tb && td > tc);
    K2PsContext ctx = {}; ctx.phase = K2PS_HUD_DRAW; ctx.slot = 1;
    g_trace.clear(); d.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "dd@1;b@1;a@1;c3@1;");
    KT_CHECK(d.remove(tb)); KT_CHECK(!d.remove(tb)); KT_CHECK(!d.remove(9999));
    g_trace.clear(); d.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "dd@1;a@1;c3@1;");
    ctx.phase = K2PS_SLOT; g_trace.clear(); d.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "");
    KT_CHECK(d.empty(K2PS_SLOT)); KT_CHECK(!d.empty(K2PS_HUD_DRAW)); KT_CHECK(d.empty(-1)); KT_CHECK(d.empty(K2PS_PHASE_COUNT));
    KT_CHECK_EQ(d.add(-1, 0, a, nullptr, "x"), -1);
    KT_CHECK_EQ(d.add(K2PS_PHASE_COUNT, 0, a, nullptr, "x"), -1);
    KT_CHECK_EQ(d.add(K2PS_SLOT, 0, nullptr, nullptr, "x"), -1);
    psd::Dispatcher f;
    for (int i = 0; i < psd::MaxPerPhase; ++i) KT_CHECK(f.add(K2PS_SLOT, i, a, nullptr, nullptr) > 0);
    KT_CHECK_EQ(f.add(K2PS_SLOT, 0, a, nullptr, nullptr), -2);
    KT_CHECK(f.add(K2PS_HUD_DRAW, 0, a, nullptr, nullptr) > 0);           // other phases unaffected
    // a callback that unregisters itself during run: the others still run exactly once, and it is gone next time
    psd::Dispatcher s; g_selfDisp = &s;
    s.add(K2PS_SLOT, 1, a, nullptr, "x"); g_selfToken = s.add(K2PS_SLOT, 2, selfRemover, nullptr, "x"); s.add(K2PS_SLOT, 3, b, nullptr, "x");
    ctx.phase = K2PS_SLOT; g_trace.clear(); s.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "a@1;self@1;b@1;");
    g_trace.clear(); s.run(ctx);
    KT_CHECK_EQ(g_trace.c_str(), "a@1;b@1;");
}

// ---- T2: fake provider + registry, consumers connect through k2ps::Client ----
static psd::Dispatcher g_disp;
static int32_t KPATCH_CALL fReg(int32_t ph, int32_t pr, K2PsCallback fn, void* u, const char* o) { return g_disp.add(ph, pr, fn, u, o); }
static void KPATCH_CALL fUnreg(int32_t t) { g_disp.remove(t); }
static int32_t KPATCH_CALL fCreature(int32_t s) { return s == 0x10 ? 0x1000 : 0; }
static int32_t KPATCH_CALL fSlotAt(int32_t i, int32_t* s, int32_t* c) { if (i) return 0; *s = 0x10; *c = 0x1000; return 1; }
static uint32_t KPATCH_CALL fFrame() { return 7; }
static const K2PortraitSlotsApi g_fakePs = { sizeof(K2PortraitSlotsApi), fReg, fUnreg, fCreature, fSlotAt, fFrame };
static bool g_provided = false;
static const void* KPATCH_CALL fRequire(const char* n, uint32_t v) {
    return (g_provided && v == K2PS_VERSION && !strcmp(n, K2PS_IFACE)) ? &g_fakePs : nullptr;
}
static int32_t KPATCH_CALL fProvide(const char*, uint32_t, const void*) { return 0; }
static const KPatchApi g_api = { sizeof(KPatchApi), KPATCH_ABI_VERSION, fProvide, fRequire };
static int g_connectLogs = 0;
static void onConn(bool, const char*) { ++g_connectLogs; }

static void t2() {
    k2ps::Client buff, shield;
    // consumers init BEFORE the provider exists (worst-case config order), then connect lazily via poll()
    buff.init(&g_api, "buff-duration-hud"); buff.onConnect = onConn;
    buff.add(K2PS_PRE_ENGINE_READ, K2PS_PRIO_BUFF, buffArrows);
    buff.add(K2PS_FRAME_BEGIN, K2PS_PRIO_BUFF, buffPoll);
    buff.add(K2PS_MENU_OVERLAY, K2PS_PRIO_BUFF, buffMenuCols);
    shield.init(&g_api, "shield-bar"); shield.onConnect = onConn;
    shield.add(K2PS_FRAME_BEGIN, K2PS_PRIO_SHIELD, shieldPoll);
    shield.add(K2PS_SLOT, K2PS_PRIO_SHIELD, shieldSlot);
    shield.add(K2PS_MENU_OVERLAY, K2PS_PRIO_SHIELD, shieldMenuBars);
    shield.add(K2PS_HUD_DRAW, K2PS_PRIO_SHIELD, shieldHud);
    KT_CHECK(!buff.connect()); KT_CHECK(!shield.connect());
    KT_CHECK(!buff.connect());                    // second failure: no second log line
    KT_CHECK_EQ(g_connectLogs, 2);
    KT_CHECK_EQ(buff.creatureForSlot(0x10), 0);   // inert without provider
    int32_t s = 0, cr = 0; KT_CHECK(!buff.slotAt(0, &s, &cr)); KT_CHECK_EQ(buff.frame(), 0u);
    g_provided = true;
    // shield's hook runs first this time: registration order must not matter, priorities decide
    shield.pollCount = 0; shield.poll(); buff.pollCount = 0; buff.poll();
    KT_CHECK(shield.connected && buff.connected);
    KT_CHECK_EQ(g_connectLogs, 4);
    KT_CHECK_EQ(buff.creatureForSlot(0x10), 0x1000);
    KT_CHECK(buff.slotAt(0, &s, &cr) && s == 0x10 && cr == 0x1000);
    KT_CHECK_EQ(buff.frame(), 7u);

    auto menuWrap = [](K2PsContext& c) { g_trace += "BEGIN2D;"; c.phase = K2PS_MENU_OVERLAY; g_disp.run(c); g_trace += "END2D;"; };
    // HUD frame: leader 0x10, small cards 0x20, 0x30
    g_trace.clear();
    uint32_t frame = 41;
    for (int slot : { 0x10, 0x20, 0x30 }) {
        K2PsContext c = {}; c.struct_size = sizeof c; c.slot = slot; c.isLarge = slot == 0x10;
        frame = psd::runSlotDraw(g_disp, c, frame, menuWrap);
    }
    KT_CHECK_EQ(frame, 42u);
    KT_CHECK_EQ(g_trace.c_str(),
        "buffArrows@10L;shieldPoll@10L;buffPoll@10L;shieldSlot@10L;shieldHud@10L;"
        "buffArrows@20;shieldSlot@20;"
        "buffArrows@30;shieldSlot@30;");
    // menu frame: top-bar slot only
    g_trace.clear();
    { K2PsContext c = {}; c.struct_size = sizeof c; c.slot = 0x40; c.isLarge = 1; c.inMenu = 1;
      frame = psd::runSlotDraw(g_disp, c, frame, menuWrap); }
    KT_CHECK_EQ(g_trace.c_str(), "buffArrows@40L;shieldPoll@40L;buffPoll@40L;shieldSlot@40L;BEGIN2D;shieldMenuBars@40L;buffMenuCols@40L;END2D;");
    // provider only (no consumer): menu wrapper (Begin/EndGui2D) never runs, nothing is called
    psd::Dispatcher none; g_trace.clear();
    { K2PsContext c = {}; c.slot = 0x40; c.isLarge = 1; c.inMenu = 1; psd::runSlotDraw(none, c, 0, menuWrap);
      c.inMenu = 0; psd::runSlotDraw(none, c, 0, menuWrap); }
    KT_CHECK_EQ(g_trace.c_str(), "");
    // too-small interface table is refused
    static const K2PortraitSlotsApi tiny = { 8, fReg, fUnreg, fCreature, fSlotAt, fFrame };
    KT_CHECK(!k2ps::Client::usable(&tiny)); KT_CHECK(!k2ps::Client::usable(nullptr)); KT_CHECK(k2ps::Client::usable(&g_fakePs));
}

// ---- T3: the provider's menu overlay is gated on the address table (null table / any zero menu field = off) ----
static void t3() {
    KT_CHECK(psd::menuFieldsOk(&kSteam));
    KT_CHECK(!psd::menuFieldsOk(&kGog));                         // GOG column is all zeros until it is mapped
    KT_CHECK(!psd::menuFieldsOk(static_cast<const K2AddrTable*>(nullptr)));
    K2AddrTable t = kSteam; t.Menu_PortraitSlot = 0; KT_CHECK(!psd::menuFieldsOk(&t));
    t = kSteam; t.Menu_Char2 = 0; KT_CHECK(!psd::menuFieldsOk(&t));
    t = kSteam; t.Menu_Char3 = 0; KT_CHECK(!psd::menuFieldsOk(&t));
    t = kSteam; t.MenuCaller_Creature = 0; KT_CHECK(!psd::menuFieldsOk(&t));
    k2addr::setForTest(nullptr); KT_CHECK(!psd::menuFieldsOk(k2addr::get()));   // unknown exe
    k2addr::setForTest(&kSteam); KT_CHECK(psd::menuFieldsOk(k2addr::get()));
    k2addr::clearForTest();
}

int main() {
    t1();
    t2();
    t3();
    return kt::Report("ps_dispatch_test");
}
