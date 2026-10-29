// Host test of k2hud::Client registry edge cases (HudHost.h): inert-without-provider, no-init, poll cadence, require-after-close,
// short KPatchApi struct_size, registration accounting, and the K2HudContext / K2HudApi ABI layout. HL1.
// Complements hh_dispatch_test.cpp (order / limits / FRAME-before-PRE / late provider via poll).
#include "HudHost.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ---- ABI layout: the numbers a consumer DLL built separately relies on ----
#if UINTPTR_MAX == 0xffffffff
static_assert(sizeof(K2HudApi) == 24, "K2HudApi size");
static_assert(offsetof(K2HudApi, struct_size) == 0, "api.struct_size");
static_assert(offsetof(K2HudApi, Register) == 4, "api.Register");
static_assert(offsetof(K2HudApi, Unregister) == 8, "api.Unregister");
static_assert(offsetof(K2HudApi, Hud) == 12, "api.Hud");
static_assert(offsetof(K2HudApi, Frame) == 16, "api.Frame");
static_assert(offsetof(K2HudApi, StripSlot) == 20, "api.StripSlot (appended after Frame)");
static_assert(sizeof(K2HudRect) == 16 && offsetof(K2HudRect, y) == 4 && offsetof(K2HudRect, w) == 8 && offsetof(K2HudRect, h) == 12, "K2HudRect layout");
#endif
static_assert(offsetof(K2HudContext, struct_size) == 0, "ctx.struct_size");
static_assert(offsetof(K2HudContext, phase) == 4, "ctx.phase");
static_assert(offsetof(K2HudContext, frame) == 8, "ctx.frame");
static_assert(offsetof(K2HudContext, hud) == 12, "ctx.hud");
static_assert(offsetof(K2HudContext, menu) == 16, "ctx.menu");
static_assert(offsetof(K2HudContext, leader) == 20, "ctx.leader");
static_assert(offsetof(K2HudContext, forceUnusable) == 24, "ctx.forceUnusable");
static_assert(offsetof(K2HudContext, target) == 28, "ctx.target");
static_assert(offsetof(K2HudContext, targetType) == 32, "ctx.targetType");
static_assert(offsetof(K2HudContext, panel) == 36, "ctx.panel");
static_assert(offsetof(K2HudContext, panelVtbl) == offsetof(K2HudContext, panel) + 4, "ctx.panelVtbl (appended after panel)");
static_assert(sizeof(K2HudContext) == 44, "K2HudContext size");
static_assert(K2HUD_PHASE_COUNT == 5 && K2HUD_FRAME == 0 && K2HUD_TARGET_MENU_PRE == 1 && K2HUD_PANEL_BUILD == 4, "phase ids");

// ---- fake provider whose Register can refuse one phase ----
static int g_refusePhase = -1;
static int g_regCalls = 0, g_nextToken = 1;
static int32_t KPATCH_CALL fReg(int32_t ph, int32_t, K2HudCallback, void*, const char*) {
    ++g_regCalls;
    return ph == g_refusePhase ? 0 : g_nextToken++;
}
static void KPATCH_CALL fUnreg(int32_t) {}
static int32_t KPATCH_CALL fHud() { return 0x1000; }
static uint32_t KPATCH_CALL fFrame() { return 7; }
static const K2HudApi g_table = { sizeof(K2HudApi), fReg, fUnreg, fHud, fFrame, nullptr };

static int g_stripCalls = 0;
static int32_t KPATCH_CALL fStrip(const char*, int32_t order, const K2HudRect* vit, int32_t width, int32_t height, K2HudRect* out) {
    ++g_stripCalls;
    if (order < 0 || order > 1) return 0;
    *out = K2HudRect{ vit->x - width, vit->y + (vit->h - height) / 2, width, height };
    return 1;
}
static const K2HudApi g_tableStrip = { sizeof(K2HudApi), fReg, fUnreg, fHud, fFrame, fStrip };

static void KPATCH_CALL cb(const K2HudContext*, void*) {}

static int g_connLogs = 0; static bool g_lastOk = false; static char g_lastWhy[128];
static void onConn(bool ok, const char* why) { ++g_connLogs; g_lastOk = ok; snprintf(g_lastWhy, sizeof g_lastWhy, "%s", why ? why : "(null)"); }
static void resetConn() { g_connLogs = 0; g_lastOk = false; g_lastWhy[0] = 0; g_regCalls = 0; g_nextToken = 1; g_refusePhase = -1; }
static bool why(const char* needle) { return strstr(g_lastWhy, needle) != nullptr; }

int main() {
    // 1. inert without a provider
    {
        fakekp::reset(); resetConn();
        k2hud::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        c.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(!c.connect());
        KT_CHECK(!c.connected);
        KT_CHECK_EQ(c.hud(), 0); KT_CHECK_EQ(c.frame(), 0u);
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("not provided"));

        resetConn();
        k2hud::Client n; n.onConnect = onConn; n.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(n.api == nullptr);
        KT_CHECK(!n.connect());
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("no KPatchApi"));
        KT_CHECK(!n.connect());                         // repeated: still one log
        for (int i = 0; i < 600; ++i) n.poll();         // poll is safe and stays silent
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!n.connected);
        KT_CHECK_EQ(n.hud(), 0); KT_CHECK_EQ(n.frame(), 0u);
        KT_CHECK_EQ(fakekp::state().requireCalls, 1);   // only c's single attempt above (n never reaches require)
        KT_CHECK_EQ(g_regCalls, 0);
    }

    // 2. poll cadence: one require on calls 1, 257, 513, ...
    {
        fakekp::reset(); resetConn();
        k2hud::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        for (int i = 0; i < 512; ++i) c.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 2);
        KT_CHECK_EQ(c.pollCount, 512u);
        KT_CHECK_EQ(fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &g_table), 0);
        KT_CHECK(!c.connected);
        c.poll();
        KT_CHECK(c.connected);
        KT_CHECK_EQ(fakekp::state().requireCalls, 3);
        for (int i = 0; i < 1000; ++i) c.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 3);   // connected: poll never requires again
    }

    // 3. require after close: a late connect gets NULL and stays inert; an early one keeps working
    {
        fakekp::reset(); resetConn();
        fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &g_table);
        k2hud::Client early; early.init(fakekp::api(), "early"); early.onConnect = onConn;
        early.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(early.connect());
        fakekp::close();
        k2hud::Client late; late.init(fakekp::api(), "late"); late.onConnect = onConn;
        late.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(!late.connect());
        KT_CHECK(!late.connected); KT_CHECK(!g_lastOk); KT_CHECK(why("not provided"));
        KT_CHECK_EQ(late.hud(), 0); KT_CHECK_EQ(late.frame(), 0u);
        int req = fakekp::state().requireCalls;
        KT_CHECK(early.connect());
        KT_CHECK_EQ(fakekp::state().requireCalls, req);
        KT_CHECK_EQ(early.hud(), 0x1000); KT_CHECK_EQ(early.frame(), 7u);
    }

    // 4. KPatchApi too small to hold require(): never calls it, reports "no KPatchApi"
    {
        fakekp::reset(8); resetConn();
        k2hud::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(!c.connect());
        KT_CHECK(why("no KPatchApi")); KT_CHECK_EQ(fakekp::state().requireCalls, 0);
    }

    // 5. registration accounting: a refused phase is reported, the rest still registered; add() limits
    {
        fakekp::reset(); resetConn();
        fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &g_table);
        g_refusePhase = K2HUD_TARGET_MENU_PRE;
        k2hud::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        c.add(K2HUD_FRAME, 1, cb); c.add(K2HUD_TARGET_MENU_PRE, 1, cb);
        KT_CHECK(c.connect());
        KT_CHECK_EQ(g_regCalls, 2); KT_CHECK(!g_lastOk); KT_CHECK(why("refused"));
        KT_CHECK(c.connected);                          // connected despite the refusal (idempotent afterwards)
        KT_CHECK(c.connect()); KT_CHECK_EQ(g_regCalls, 2);

        k2hud::Client f; KT_CHECK(!f.add(K2HUD_FRAME, 1, nullptr));        // null fn rejected client-side
        for (int i = 0; i < K2HUD_PHASE_COUNT * 2; ++i) KT_CHECK(f.add(K2HUD_FRAME, i, cb));
        KT_CHECK(!f.add(K2HUD_FRAME, 99, cb));                              // client table full
    }

    // 6. a v1 table without Frame() (too short) or without Register is not usable
    {
        static const K2HudApi noReg = { sizeof(K2HudApi), nullptr, fUnreg, fHud, fFrame, nullptr };
        static const K2HudApi noHud = { sizeof(K2HudApi), fReg, fUnreg, nullptr, fFrame, nullptr };
        KT_CHECK(!k2hud::Client::usable(&noReg)); KT_CHECK(!k2hud::Client::usable(&noHud));
        KT_CHECK(k2hud::Client::usable(&g_table));
    }

    // 7. stripSlot (CS10): false without a provider, with a table that predates StripSlot (struct_size 20 or null fn), or when refused;
    //    true and filled through a table that has it
    {
        const K2HudRect vit{ 3671, 1285, 47, 96 };
        K2HudRect r{ -1, -1, -1, -1 };
        k2hud::Client none;
        KT_CHECK(!none.stripSlot("t", 0, vit, 38, 38, &r)); KT_CHECK_EQ(r.x, -1);
        fakekp::reset(); resetConn(); g_stripCalls = 0;
        fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &g_tableStrip);
        k2hud::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(!c.stripSlot("t", 0, vit, 38, 38, &r));   // not connected yet: hu == null
        KT_CHECK(c.connect());
        KT_CHECK(c.stripSlot("t", 0, vit, 38, 38, &r));
        KT_CHECK_EQ(r.x, 3671 - 38); KT_CHECK_EQ(r.y, 1285 + 29); KT_CHECK_EQ(r.w, 38); KT_CHECK_EQ(r.h, 38);
        KT_CHECK(!c.stripSlot("t", 5, vit, 38, 38, &r));   // provider refuses
        KT_CHECK(!c.stripSlot("t", 0, vit, 38, 38, nullptr));
        KT_CHECK_EQ(g_stripCalls, 2);                       // null out never reaches the provider

        // v1.0 provider: struct_size covers Frame only
        static K2HudApi oldT = g_tableStrip; oldT.struct_size = uint32_t(offsetof(K2HudApi, Frame) + sizeof(oldT.Frame));
        fakekp::reset(); resetConn(); g_stripCalls = 0;
        fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &oldT);
        k2hud::Client o; o.init(fakekp::api(), "t"); o.onConnect = onConn; o.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(o.connect());
        KT_CHECK(!o.stripSlot("t", 0, vit, 38, 38, &r)); KT_CHECK_EQ(g_stripCalls, 0);
        // full-size struct but null StripSlot
        static K2HudApi nullT = g_tableStrip; nullT.StripSlot = nullptr;
        fakekp::reset(); resetConn();
        fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &nullT);
        k2hud::Client n; n.init(fakekp::api(), "t"); n.onConnect = onConn; n.add(K2HUD_FRAME, 1, cb);
        KT_CHECK(n.connect()); KT_CHECK(!n.stripSlot("t", 0, vit, 38, 38, &r)); KT_CHECK_EQ(g_stripCalls, 0);
    }
    return kt::Report("hh_registry_test");
}
