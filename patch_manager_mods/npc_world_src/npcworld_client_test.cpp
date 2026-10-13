// Host test of k2nw::Client (NpcWorld.h): inert without a provider, struct_size gating of the API table and of K2NwEvent, poll cadence,
// init-order independence, registration accounting, release(), the service wrappers end to end against a real nw::World, and the ABI
// layout. Provider-side rules are nw_registry_test.cpp / nw_world_test.cpp.
#include "NpcWorld.h"
#include "NwCore.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ---- a fake provider: the real nw::World behind the C table, so Client -> table -> World is exercised end to end ----
static nw::World g_w;
static char g_area[4];
static int g_regCalls = 0, g_unregCalls = 0, g_refuseMask = 0, g_engineCalls = 0;
static int32_t KPATCH_CALL fReg(int32_t m, K2NwCallback fn, void* u, const char* o) { ++g_regCalls; return m == g_refuseMask ? 0 : g_w.disp.add(m, fn, u, o); }
static void KPATCH_CALL fUnreg(int32_t t) { ++g_unregCalls; g_w.disp.remove(t); }
static uint32_t KPATCH_CALL fSalt() { return g_w.visitSalt(); }
static int32_t KPATCH_CALL fSeq() { return g_w.visitSeq(); }
static int32_t KPATCH_CALL fArea(char* b, uint32_t c) { return g_w.currentArea(b, c); }
static int32_t KPATCH_CALL fTag(const char* t) { return g_w.tagPresent(t) ? 1 : 0; }
static uint32_t KPATCH_CALL fSpawn(const char* r, const char* t, float x, float y, float z, float f) { uint32_t o = 0; g_w.spawn(r, t, x, y, z, f, &o); return o; }
static int32_t KPATCH_CALL fCount() { return g_w.spawnedCount(); }
static int32_t KPATCH_CALL fCap() { return g_w.effectiveCap(); }
static const K2NpcWorldApi g_table = { sizeof(K2NpcWorldApi), fReg, fUnreg, fSalt, fSeq, fArea, fTag, fSpawn, fCount, fCap };
static uint32_t fakeEngine(void*, void*, const char*, float, float, float, float) { ++g_engineCalls; return 0x7f000001u; }

static int g_hits = 0; static int32_t g_lastKind = 0; static uint32_t g_lastSize = 0, g_lastSalt = 0;
static void KPATCH_CALL cb(const K2NwEvent* ev, void*) { ++g_hits; g_lastKind = ev->kind; g_lastSize = ev->struct_size; g_lastSalt = ev->visit_salt; }

static int g_connLogs = 0; static bool g_lastOk = false; static char g_lastWhy[128];
static void onConn(bool ok, const char* why) { ++g_connLogs; g_lastOk = ok; snprintf(g_lastWhy, sizeof g_lastWhy, "%s", why ? why : "(null)"); }
static void resetAll() {
    fakekp::reset(); g_w = nw::World(); g_w.eng.spawn = fakeEngine; g_regCalls = g_unregCalls = g_refuseMask = g_engineCalls = 0;
    g_hits = 0; g_connLogs = 0; g_lastOk = false; g_lastWhy[0] = 0;
}
static bool why(const char* n) { return strstr(g_lastWhy, n) != nullptr; }
static void toStable() { g_w.onLoad(7); nw::CreatureInfo ci; ci.areaPtr = g_area; ci.areaRes = "204tel"; g_w.onCreatureAdded(ci); for (int i = 0; i < 31; ++i) g_w.onTick(); }

int main() {
    // 1. inert without a provider: no callback ever fires, getters are safe, one connect log
    {
        resetAll();
        k2nw::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2NW_ALL, cb);
        KT_CHECK(!c.connect()); KT_CHECK(!c.connected);
        char b[8]; memset(b, 'x', sizeof b);
        KT_CHECK_EQ(c.currentArea(b, sizeof b), 0); KT_CHECK_EQ(b[0], 0);
        KT_CHECK_EQ(c.currentArea(nullptr, 0), 0); KT_CHECK_EQ(c.currentArea(nullptr, 5), 0);
        KT_CHECK_EQ(c.visitSalt(), 0u); KT_CHECK_EQ(c.visitSeq(), 0); KT_CHECK(!c.tagPresent("x")); KT_CHECK_EQ(c.spawn("r", "t", 0, 0, 0, 0), 0u);
        KT_CHECK_EQ(c.spawnedCount(), 0); KT_CHECK_EQ(c.spawnCap(), 0);
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("not provided"));
        KT_CHECK(!c.connect()); KT_CHECK_EQ(g_connLogs, 1);
        c.release();
        k2nw::Client n; n.onConnect = onConn;                                  // no init: api == nullptr
        KT_CHECK(!n.connect()); KT_CHECK(why("no KPatchApi")); KT_CHECK_EQ(g_connLogs, 2);
        for (int i = 0; i < 600; ++i) n.poll();
        KT_CHECK_EQ(g_connLogs, 2); KT_CHECK_EQ(fakekp::state().requireCalls, 2);
    }
    // 2. struct_size gating of the API table
    {
        resetAll();
        static K2NpcWorldApi shortT = g_table;
        shortT.struct_size = offsetof(K2NpcWorldApi, SpawnCap);                // ends before the last field
        fakekp::provide(K2NW_IFACE, K2NW_VERSION, &shortT);
        k2nw::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2NW_AREA_STABLE, cb);
        KT_CHECK(!c.connect()); KT_CHECK(why("too small")); KT_CHECK_EQ(g_regCalls, 0);
        resetAll();
        static K2NpcWorldApi nullFn = g_table; nullFn.Spawn = nullptr;          // full size but a hole
        fakekp::provide(K2NW_IFACE, K2NW_VERSION, &nullFn);
        k2nw::Client d; d.init(fakekp::api(), "t"); d.onConnect = onConn; d.add(K2NW_AREA_STABLE, cb);
        KT_CHECK(!d.connect()); KT_CHECK(why("too small"));
        resetAll();
        static K2NpcWorldApi bigger[2]; memcpy(&bigger[0], &g_table, sizeof g_table); bigger[0].struct_size = sizeof g_table + 16;   // a newer provider
        fakekp::provide(K2NW_IFACE, K2NW_VERSION, &bigger[0]);
        k2nw::Client e; e.init(fakekp::api(), "t"); e.add(K2NW_AREA_STABLE, cb);
        KT_CHECK(e.connect());
    }
    // 3. connect + registration accounting + service wrappers + events
    {
        resetAll(); fakekp::provide(K2NW_IFACE, K2NW_VERSION, &g_table);
        k2nw::Client c; c.init(fakekp::api(), "walkers"); c.onConnect = onConn;
        KT_CHECK(c.add(K2NW_AREA_STABLE | K2NW_CREATURE_ADDED, cb)); KT_CHECK(!c.add(0, cb)); KT_CHECK(!c.add(K2NW_ALL, nullptr));
        KT_CHECK(c.connect()); KT_CHECK(g_lastOk); KT_CHECK(why("registered")); KT_CHECK_EQ(g_regCalls, 1); KT_CHECK_EQ(g_w.disp.n, 1);
        KT_CHECK(strcmp(g_w.disp.e[0].owner, "walkers") == 0);
        KT_CHECK_EQ(c.spawnCap(), nw::DefaultSpawnCap); KT_CHECK_EQ(c.visitSalt(), 0u);
        KT_CHECK_EQ(c.spawn("r", "t", 0, 0, 0, 0), 0u);                         // not stable yet: refused through the table
        toStable();
        KT_CHECK_EQ(g_lastKind, K2NW_AREA_STABLE);                              // CREATURE_ADDED then AREA_STABLE reached the consumer
        KT_CHECK_EQ(g_lastSize, uint32_t(sizeof(K2NwEvent))); KT_CHECK(g_lastSalt != 0); KT_CHECK_EQ(g_lastSalt, c.visitSalt());
        KT_CHECK_EQ(c.visitSeq(), 1);
        char b[16]; KT_CHECK_EQ(c.currentArea(b, sizeof b), 6); KT_CHECK(strcmp(b, "204tel") == 0);
        KT_CHECK_EQ(c.spawn("r", "k1p", 1, 2, 3, 0), 0x7f000001u); KT_CHECK_EQ(g_engineCalls, 1);
        KT_CHECK(c.tagPresent("k1p")); KT_CHECK_EQ(c.spawnedCount(), 1);
        KT_CHECK(c.connect());                                                  // idempotent
        KT_CHECK_EQ(g_regCalls, 1);
        const int before = g_hits;
        c.release(); KT_CHECK_EQ(g_unregCalls, 1); KT_CHECK(!c.connected); KT_CHECK_EQ(g_w.disp.n, 0);
        g_w.onLoad(9); KT_CHECK_EQ(g_hits, before);                              // no longer called
        KT_CHECK_EQ(c.visitSalt(), 0u);                                         // wrappers inert after release
        c.release();                                                            // twice is safe
        KT_CHECK(c.connect()); KT_CHECK_EQ(g_regCalls, 2);                      // reconnect registers again
    }
    // 4. refused registrations, MaxRegs, poll cadence and late provider
    {
        resetAll(); fakekp::provide(K2NW_IFACE, K2NW_VERSION, &g_table); g_refuseMask = K2NW_TICK;
        k2nw::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2NW_TICK, cb); c.add(K2NW_AREA_LOAD, cb);
        KT_CHECK(c.connect()); KT_CHECK(!g_lastOk); KT_CHECK(why("refused")); KT_CHECK_EQ(g_regCalls, 2);
        k2nw::Client m; m.init(fakekp::api(), "t");
        for (int i = 0; i < k2nw::MaxRegs; ++i) KT_CHECK(m.add(K2NW_TICK, cb));
        KT_CHECK(!m.add(K2NW_TICK, cb));
        resetAll();                                                             // provider arrives after the first connect attempt
        k2nw::Client p; p.init(fakekp::api(), "t"); p.add(K2NW_AREA_STABLE, cb);
        p.poll(); KT_CHECK_EQ(fakekp::state().requireCalls, 1);                  // first poll tries immediately
        for (int i = 0; i < 255; ++i) p.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 1);                           // then every 256th call
        fakekp::provide(K2NW_IFACE, K2NW_VERSION, &g_table);
        p.poll(); KT_CHECK_EQ(fakekp::state().requireCalls, 2); KT_CHECK(p.connected);
        KT_CHECK_EQ(g_w.disp.n, 1);
    }
    // 5. short KPatchApi (patcher predates the registry), provider after close
    {
        resetAll(); fakekp::reset(offsetof(KPatchApi, require));                // struct ends before require
        k2nw::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2NW_ALL, cb);
        KT_CHECK(!c.connect()); KT_CHECK(why("no KPatchApi"));
        resetAll(); fakekp::close();
        fakekp::provide(K2NW_IFACE, K2NW_VERSION, &g_table);                    // refused: registry closed
        k2nw::Client d; d.init(fakekp::api(), "t"); d.onConnect = onConn; d.add(K2NW_ALL, cb);
        KT_CHECK(!d.connect()); KT_CHECK(why("not provided"));
    }
    // 6. event struct_size gating helper
    {
        K2NwEvent e = {}; e.struct_size = offsetof(K2NwEvent, origin);           // an older provider that stops before `origin`
        KT_CHECK(!K2NW_EVENT_HAS(&e, origin)); KT_CHECK(K2NW_EVENT_HAS(&e, z));
        e.struct_size = sizeof e; KT_CHECK(K2NW_EVENT_HAS(&e, origin));
        KT_CHECK(!k2nw::eventHas(nullptr, 4));
    }
#if UINTPTR_MAX == 0xffffffff
    static_assert(sizeof(K2NwEvent) == 60 && sizeof(K2NpcWorldApi) == 40, "ABI size (m32)");
    static_assert(offsetof(K2NpcWorldApi, SpawnCap) + sizeof(((K2NpcWorldApi*)0)->SpawnCap) == 40, "api end (m32)");
#endif
    return kt::Report("npcworld_client_test");
}
