// Host test of k2ps::Client registry edge cases (PortraitSlots.h): inert-without-provider, poll cadence, require-after-close,
// short KPatchApi struct_size, registration accounting, and the K2PsContext / K2PortraitSlotsApi ABI layout. Doc 24 T3.
// Complements ps_dispatch_test.cpp (which owns: late provider via poll() after pollCount=0, log-once, usable(tiny)/usable(nullptr)).
#include "PortraitSlots.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ---- ABI layout: the numbers a consumer DLL built separately relies on ----
#if UINTPTR_MAX == 0xffffffff
#define PS_CTX_SIZE 72
static_assert(sizeof(K2PortraitSlotsApi) == 24, "K2PortraitSlotsApi size");
static_assert(offsetof(K2PortraitSlotsApi, struct_size) == 0, "api.struct_size");
static_assert(offsetof(K2PortraitSlotsApi, Register) == 4, "api.Register");
static_assert(offsetof(K2PortraitSlotsApi, Unregister) == 8, "api.Unregister");
static_assert(offsetof(K2PortraitSlotsApi, CreatureForSlot) == 12, "api.CreatureForSlot");
static_assert(offsetof(K2PortraitSlotsApi, SlotAt) == 16, "api.SlotAt");
static_assert(offsetof(K2PortraitSlotsApi, Frame) == 20, "api.Frame");
#else
#define PS_CTX_SIZE 72   // K2PsContext is all 4-byte ints: same layout on a 64-bit host (the API table holds pointers, so it is not pinned here)
#endif
static_assert(offsetof(K2PsContext, struct_size) == 0, "ctx.struct_size");
static_assert(offsetof(K2PsContext, phase) == 4, "ctx.phase");
static_assert(offsetof(K2PsContext, slot) == 8, "ctx.slot");
static_assert(offsetof(K2PsContext, creature) == 12, "ctx.creature");
static_assert(offsetof(K2PsContext, isLarge) == 16, "ctx.isLarge");
static_assert(offsetof(K2PsContext, callerEbp) == 20, "ctx.callerEbp");
static_assert(offsetof(K2PsContext, frame) == 24, "ctx.frame");
static_assert(offsetof(K2PsContext, inMenu) == 28, "ctx.inMenu");
static_assert(offsetof(K2PsContext, panel) == 32, "ctx.panel");
static_assert(offsetof(K2PsContext, panelRect) == 36, "ctx.panelRect");
static_assert(offsetof(K2PsContext, menuCreature) == 52, "ctx.menuCreature");
static_assert(offsetof(K2PsContext, companions) == 56, "ctx.companions");
static_assert(offsetof(K2PsContext, companionCtrl) == 64, "ctx.companionCtrl");
static_assert(sizeof(K2PsContext) == PS_CTX_SIZE, "K2PsContext size");

// ---- fake provider whose Register can refuse one phase ----
static int g_refusePhase = -1;      // Register returns 0 (refused) for this phase; -1 = accept all
static int g_regCalls = 0, g_nextToken = 1;
static int32_t KPATCH_CALL fReg(int32_t ph, int32_t, K2PsCallback, void*, const char*) {
    ++g_regCalls;
    return ph == g_refusePhase ? 0 : g_nextToken++;
}
static void KPATCH_CALL fUnreg(int32_t) {}
static int32_t KPATCH_CALL fCreature(int32_t s) { return s == 0x10 ? 0x1000 : 0; }
static int32_t KPATCH_CALL fSlotAt(int32_t i, int32_t* s, int32_t* c) { if (i) return 0; *s = 0x10; *c = 0x1000; return 1; }
static uint32_t KPATCH_CALL fFrame() { return 7; }
static const K2PortraitSlotsApi g_table = { sizeof(K2PortraitSlotsApi), fReg, fUnreg, fCreature, fSlotAt, fFrame };

static void KPATCH_CALL cb(const K2PsContext*, void*) {}

static int g_connLogs = 0; static bool g_lastOk = false; static char g_lastWhy[128];
static void onConn(bool ok, const char* why) { ++g_connLogs; g_lastOk = ok; snprintf(g_lastWhy, sizeof g_lastWhy, "%s", why ? why : "(null)"); }
static void resetConn() { g_connLogs = 0; g_lastOk = false; g_lastWhy[0] = 0; g_regCalls = 0; g_nextToken = 1; g_refusePhase = -1; }
static bool why(const char* needle) { return strstr(g_lastWhy, needle) != nullptr; }

int main() {
    // 1. inert without a provider
    {
        fakekp::reset(); resetConn();
        k2ps::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        c.add(K2PS_SLOT, 1, cb);
        KT_CHECK(!c.connect());
        KT_CHECK(!c.connected);
        KT_CHECK_EQ(c.creatureForSlot(0x10), 0);
        int32_t s = 5, cr = 6; KT_CHECK(!c.slotAt(0, &s, &cr));
        KT_CHECK(s == 5 && cr == 6);                    // outputs untouched
        KT_CHECK_EQ(c.frame(), 0u);
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("not provided"));

        // no init at all: api == nullptr
        resetConn();
        k2ps::Client n; n.onConnect = onConn; n.add(K2PS_SLOT, 1, cb);
        KT_CHECK(n.api == nullptr);
        KT_CHECK(!n.connect());
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("no KPatchApi"));
        KT_CHECK(!n.connect());                         // repeated: still one log
        for (int i = 0; i < 600; ++i) n.poll();         // poll is safe and stays silent
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!n.connected);
        KT_CHECK_EQ(n.creatureForSlot(0x10), 0);
        KT_CHECK(!n.slotAt(0, &s, &cr)); KT_CHECK_EQ(n.frame(), 0u);
        KT_CHECK_EQ(fakekp::state().requireCalls, 1);   // only c's single attempt above (n never reaches require)
    }

    // 2. poll cadence: `(pollCount++ & 255) == 0` -> one require on calls 1, 257, 513, ... (pollCount 0, 256, 512)
    {
        fakekp::reset(); resetConn();
        k2ps::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        for (int i = 0; i < 512; ++i) c.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 2);   // pollCount 0 and 256
        KT_CHECK_EQ(c.pollCount, 512u);
        KT_CHECK_EQ(fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table), 0);
        KT_CHECK(!c.connected);                         // provider appeared, but no boundary yet
        c.poll();                                       // 513th call: pollCount 512 is a boundary
        KT_CHECK(c.connected);
        KT_CHECK_EQ(fakekp::state().requireCalls, 3);
        KT_CHECK_EQ(c.pollCount, 513u);
        for (int i = 0; i < 1000; ++i) c.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 3);   // connected: poll never requires again
        KT_CHECK_EQ(c.pollCount, 513u);                 // and does not even advance

        // mid-cycle start: pollCount 300 -> boundary at 512, i.e. 212 silent calls then the 213th connects
        fakekp::reset(); resetConn();
        k2ps::Client m; m.init(fakekp::api(), "t"); m.onConnect = onConn;
        m.pollCount = 300;
        fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table);
        for (int i = 0; i < 212; ++i) m.poll();
        KT_CHECK(!m.connected);
        KT_CHECK_EQ(fakekp::state().requireCalls, 0);
        KT_CHECK_EQ(m.pollCount, 512u);
        m.poll();
        KT_CHECK(m.connected);
        KT_CHECK_EQ(fakekp::state().requireCalls, 1);
    }

    // 3. require after close. Mirrors KPM registry.cpp: close() ends the require window, so a consumer must connect in
    //    KPatchInit (or before close); a late connect gets NULL and stays inert.
    {
        fakekp::reset(); resetConn();
        fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table);
        k2ps::Client early; early.init(fakekp::api(), "early"); early.onConnect = onConn;
        early.add(K2PS_SLOT, 1, cb);
        KT_CHECK(early.connect());
        fakekp::close();
        k2ps::Client late; late.init(fakekp::api(), "late"); late.onConnect = onConn;
        late.add(K2PS_SLOT, 1, cb);
        int before = g_connLogs, req = fakekp::state().requireCalls;
        KT_CHECK(!late.connect());
        KT_CHECK(!late.connected);
        KT_CHECK_EQ(fakekp::state().requireCalls, req + 1);   // it did try
        KT_CHECK_EQ(g_connLogs, before + 1); KT_CHECK(!g_lastOk); KT_CHECK(why("not provided"));
        KT_CHECK_EQ(late.creatureForSlot(0x10), 0); KT_CHECK_EQ(late.frame(), 0u);
        // the client that connected before close keeps working and does not re-require
        req = fakekp::state().requireCalls;
        KT_CHECK(early.connect());
        KT_CHECK_EQ(fakekp::state().requireCalls, req);
        KT_CHECK_EQ(early.creatureForSlot(0x10), 0x1000);
        int32_t s = 0, cr = 0; KT_CHECK(early.slotAt(0, &s, &cr) && s == 0x10 && cr == 0x1000);
        KT_CHECK_EQ(early.frame(), 7u);
    }

    // 4. KPatchApi struct_size too small to contain `require`
    {
        fakekp::reset(offsetof(KPatchApi, require)); resetConn();
        fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table);
        k2ps::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        KT_CHECK(!c.connect());
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("no KPatchApi"));
        KT_CHECK(!c.connect()); KT_CHECK(!c.connect());
        for (int i = 0; i < 600; ++i) c.poll();
        KT_CHECK_EQ(g_connLogs, 1);
        KT_CHECK_EQ(fakekp::state().requireCalls, 0);
        KT_CHECK(!c.connected);
        // exactly large enough
        fakekp::reset(offsetof(KPatchApi, require) + sizeof(((KPatchApi*)0)->require)); resetConn();
        fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table);
        k2ps::Client d; d.init(fakekp::api(), "t"); d.onConnect = onConn;
        KT_CHECK(d.connect());
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(g_lastOk); KT_CHECK(why("registered"));
        KT_CHECK_EQ(fakekp::state().requireCalls, 1);
    }

    // 5. registration accounting
    {
        fakekp::reset(); resetConn();
        fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table);
        k2ps::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        c.add(K2PS_FRAME_BEGIN, 1, cb); c.add(K2PS_SLOT, 2, cb); c.add(K2PS_HUD_DRAW, 3, cb);
        g_refusePhase = K2PS_SLOT;
        KT_CHECK(c.connect());                          // refusals do not undo the connection
        KT_CHECK(c.connected);
        KT_CHECK_EQ(g_regCalls, 3);
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("some registrations refused"));

        resetConn();
        k2ps::Client a; a.init(fakekp::api(), "t"); a.onConnect = onConn;
        a.add(K2PS_FRAME_BEGIN, 1, cb); a.add(K2PS_SLOT, 2, cb);
        KT_CHECK(a.connect());
        KT_CHECK_EQ(g_regCalls, 2);
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(g_lastOk); KT_CHECK(why("registered"));
        KT_CHECK(a.connect());                          // idempotent: no second registration, no second log
        KT_CHECK_EQ(g_regCalls, 2); KT_CHECK_EQ(g_connLogs, 1);

        // zero registrations: ok == nRegs == 0 -> "registered"
        resetConn();
        k2ps::Client z; z.init(fakekp::api(), "t"); z.onConnect = onConn;
        KT_CHECK(z.connect());
        KT_CHECK_EQ(g_regCalls, 0); KT_CHECK(g_lastOk); KT_CHECK(why("registered"));

        // add(): null fn refused, capacity K2PS_PHASE_COUNT*2 then refused
        k2ps::Client f; f.init(fakekp::api(), "t");
        KT_CHECK(!f.add(K2PS_SLOT, 0, nullptr));
        KT_CHECK_EQ(f.nRegs, 0);
        const int cap = K2PS_PHASE_COUNT * 2;
        for (int i = 0; i < cap; ++i) KT_CHECK(f.add(i % K2PS_PHASE_COUNT, i, cb));
        KT_CHECK_EQ(f.nRegs, cap);
        KT_CHECK(!f.add(K2PS_SLOT, 99, cb));
        KT_CHECK_EQ(f.nRegs, cap);
        KT_CHECK_EQ(f.regs[cap - 1].prio, cap - 1);     // the refused add did not overwrite the last entry
    }

    return kt::Report("ps_registry_test");
}
