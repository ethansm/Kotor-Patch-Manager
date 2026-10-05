// Host test of k2se::Client (SaveEvents.h): inert without a provider, struct_size gating of the API table and of K2SaveEvent, poll
// cadence, init-order independence, require after close, short KPatchApi, registration accounting, release(), and the ABI layout.
// Doc 22 section 6.2. Provider-side rules (mask, re-entrancy, slow guard, folder parsing) are se_registry_test.cpp.
#include "SaveEvents.h"
#include "SeDispatch.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if UINTPTR_MAX == 0xffffffff
static_assert(sizeof(K2SaveEventsApi) == 16, "K2SaveEventsApi size (m32)");
static_assert(sizeof(K2SaveEvent) == 16, "K2SaveEvent size (m32)");
static_assert(offsetof(K2SaveEventsApi, CurrentSaveFolder) + sizeof(((K2SaveEventsApi*)0)->CurrentSaveFolder) == 16, "api end (m32)");
#endif

// ---- a fake provider: the real se::Hub behind the C table, so Client -> table -> Hub is exercised end to end ----
static se::Hub g_hub;
static int g_regCalls = 0, g_unregCalls = 0, g_refuseMask = 0;
static int32_t KPATCH_CALL fReg(int32_t m, K2SaveCallback fn, void* u, const char* o) { ++g_regCalls; return m == g_refuseMask ? 0 : g_hub.disp.add(m, fn, u, o); }
static void KPATCH_CALL fUnreg(int32_t t) { ++g_unregCalls; g_hub.disp.remove(t); }
static int32_t KPATCH_CALL fCur(char* b, uint32_t c) { return g_hub.copyCurrent(b, c); }
static const K2SaveEventsApi g_table = { sizeof(K2SaveEventsApi), fReg, fUnreg, fCur };

static int g_hits = 0; static int32_t g_lastKind = 0; static uint32_t g_lastSize = 0;
static void KPATCH_CALL cb(const K2SaveEvent* ev, void*) { ++g_hits; g_lastKind = ev->kind; g_lastSize = ev->struct_size; }

static int g_connLogs = 0; static bool g_lastOk = false; static char g_lastWhy[128];
static void onConn(bool ok, const char* why) { ++g_connLogs; g_lastOk = ok; snprintf(g_lastWhy, sizeof g_lastWhy, "%s", why ? why : "(null)"); }
static void resetAll() { fakekp::reset(); g_hub = se::Hub(); g_regCalls = g_unregCalls = g_refuseMask = 0; g_hits = 0; g_connLogs = 0; g_lastOk = false; g_lastWhy[0] = 0; }
static bool why(const char* n) { return strstr(g_lastWhy, n) != nullptr; }

int main() {
    // 1. inert without a provider: no callback ever fires, outputs are safe, one connect log
    {
        resetAll();
        k2se::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2SE_ALL, cb);
        KT_CHECK(!c.connect()); KT_CHECK(!c.connected);
        char b[8]; memset(b, 'x', sizeof b);
        KT_CHECK_EQ(c.currentSaveFolder(b, sizeof b), 0); KT_CHECK_EQ(b[0], 0);
        KT_CHECK_EQ(c.currentSaveFolder(nullptr, 0), 0); KT_CHECK_EQ(c.currentSaveFolder(nullptr, 5), 0);
        KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("not provided"));
        g_hub.disp.add(K2SE_ALL, cb, nullptr, "other");              // events happen with nobody connected: nothing of ours fires
        g_hub.emit(K2SE_SAVED, "saves/000012 - G", 0); KT_CHECK_EQ(g_hits, 1);   // only the "other" consumer
        KT_CHECK(!c.connect()); KT_CHECK_EQ(g_connLogs, 1);          // repeated: still one log
        c.release();                                                 // safe when not connected
        k2se::Client n; n.onConnect = onConn;                        // no init: api == nullptr
        KT_CHECK(!n.connect()); KT_CHECK(why("no KPatchApi")); KT_CHECK_EQ(g_connLogs, 2);
        for (int i = 0; i < 600; ++i) n.poll();
        KT_CHECK_EQ(g_connLogs, 2); KT_CHECK_EQ(fakekp::state().requireCalls, 2);   // c.connect() twice; n never reaches require
    }
    // 2. struct_size gating of the API table: a table too short to hold CurrentSaveFolder is treated as absent
    {
        resetAll();
        static K2SaveEventsApi shortT = g_table;
        shortT.struct_size = offsetof(K2SaveEventsApi, CurrentSaveFolder);               // ends before the last field
        fakekp::provide(K2SE_IFACE, K2SE_VERSION, &shortT);
        k2se::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2SE_SAVED, cb);
        KT_CHECK(!c.connect()); KT_CHECK(why("too small")); KT_CHECK_EQ(g_regCalls, 0);
        KT_CHECK(!k2se::Client::usable(nullptr));
        K2SaveEventsApi t = g_table;
        KT_CHECK(k2se::Client::usable(&t));
        t.struct_size = sizeof(K2SaveEventsApi) + 8; KT_CHECK(k2se::Client::usable(&t));   // a newer, larger provider is fine
        t.struct_size = 0; KT_CHECK(!k2se::Client::usable(&t));
        t = g_table; t.Register = nullptr; KT_CHECK(!k2se::Client::usable(&t));
        t = g_table; t.Unregister = nullptr; KT_CHECK(!k2se::Client::usable(&t));
        t = g_table; t.CurrentSaveFolder = nullptr; KT_CHECK(!k2se::Client::usable(&t));
        // the event side: a consumer gates a field on ev->struct_size
        K2SaveEvent ev = {}; ev.struct_size = offsetof(K2SaveEvent, folder) + sizeof(ev.folder);   // an old provider without `cloud`
        KT_CHECK(K2SE_EVENT_HAS(&ev, folder)); KT_CHECK(!K2SE_EVENT_HAS(&ev, cloud));
        ev.struct_size = sizeof ev; KT_CHECK(K2SE_EVENT_HAS(&ev, cloud));
        KT_CHECK(!k2se::eventHas(nullptr, 1));
    }
    // 3. poll cadence + init-order independence (the provider appears after the consumer's init)
    {
        resetAll();
        k2se::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn; c.add(K2SE_ALL, cb);
        for (int i = 0; i < 512; ++i) c.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 2);                // pollCount 0 and 256
        fakekp::provide(K2SE_IFACE, K2SE_VERSION, &g_table);
        KT_CHECK(!c.connected);
        c.poll();                                                    // 513th call: boundary
        KT_CHECK(c.connected); KT_CHECK_EQ(fakekp::state().requireCalls, 3); KT_CHECK(g_lastOk);
        for (int i = 0; i < 1000; ++i) c.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 3);                // connected: poll never requires again
        g_hub.emit(K2SE_LOAD_REQUEST, "saves/000012 - G", 0); KT_CHECK_EQ(g_hits, 1); KT_CHECK_EQ(g_lastKind, K2SE_LOAD_REQUEST);
        KT_CHECK_EQ(g_lastSize, (unsigned)sizeof(K2SaveEvent));
        char b[64]; KT_CHECK_EQ(c.currentSaveFolder(b, sizeof b), 16); KT_CHECK_EQ(b, "saves/000012 - G");
        KT_CHECK_EQ(c.currentSaveFolder(b, 5), 16); KT_CHECK_EQ(b, "save");   // truncated through the table, NUL kept
    }
    // 4. require after close: a consumer that connected before keeps working, a late one stays inert
    {
        resetAll();
        fakekp::provide(K2SE_IFACE, K2SE_VERSION, &g_table);
        k2se::Client early; early.init(fakekp::api(), "early"); early.onConnect = onConn; early.add(K2SE_ALL, cb);
        KT_CHECK(early.connect());
        fakekp::close();
        k2se::Client late; late.init(fakekp::api(), "late"); late.onConnect = onConn; late.add(K2SE_ALL, cb);
        KT_CHECK(!late.connect()); KT_CHECK(why("not provided"));
        g_hub.emit(K2SE_NEW_GAME, nullptr, 0); KT_CHECK_EQ(g_hits, 1);             // only `early`
        int req = fakekp::state().requireCalls; KT_CHECK(early.connect()); KT_CHECK_EQ(fakekp::state().requireCalls, req);
    }
    // 5. KPatchApi too small to contain `require`; exactly large enough
    {
        resetAll(); fakekp::reset(offsetof(KPatchApi, require));
        fakekp::provide(K2SE_IFACE, K2SE_VERSION, &g_table);
        k2se::Client c; c.init(fakekp::api(), "t"); c.onConnect = onConn;
        KT_CHECK(!c.connect()); KT_CHECK(why("no KPatchApi")); KT_CHECK_EQ(fakekp::state().requireCalls, 0);
        fakekp::reset(offsetof(KPatchApi, require) + sizeof(((KPatchApi*)0)->require));
        fakekp::provide(K2SE_IFACE, K2SE_VERSION, &g_table);
        k2se::Client d; d.init(fakekp::api(), "t"); d.onConnect = onConn;
        KT_CHECK(d.connect()); KT_CHECK(g_lastOk);
        // a different major is not our interface
        fakekp::reset(); fakekp::provide(K2SE_IFACE, 2, &g_table);
        k2se::Client e; e.init(fakekp::api(), "t"); e.onConnect = onConn; KT_CHECK(!e.connect());
    }
    // 6. registration accounting, add() limits, release()
    {
        resetAll();
        fakekp::provide(K2SE_IFACE, K2SE_VERSION, &g_table);
        k2se::Client c; c.init(fakekp::api(), "me"); c.onConnect = onConn;
        KT_CHECK(!c.add(K2SE_SAVED, nullptr)); KT_CHECK(!c.add(0, cb)); KT_CHECK(!c.add(0x10, cb)); KT_CHECK_EQ(c.nRegs, 0);
        c.add(K2SE_SAVED, cb); c.add(K2SE_NEW_GAME, cb); c.add(K2SE_LOAD_REQUEST, cb);
        g_refuseMask = K2SE_NEW_GAME;
        KT_CHECK(c.connect());                                       // a refusal does not undo the connection
        KT_CHECK_EQ(g_regCalls, 3); KT_CHECK_EQ(g_connLogs, 1); KT_CHECK(!g_lastOk); KT_CHECK(why("some registrations refused"));
        KT_CHECK(c.connect()); KT_CHECK_EQ(g_regCalls, 3); KT_CHECK_EQ(g_connLogs, 1);   // idempotent
        g_hub.emit(K2SE_NEW_GAME, nullptr, 0); KT_CHECK_EQ(g_hits, 0);                    // the refused kind never fires
        g_hub.emit(K2SE_SAVED, "saves/000012 - G", 0); KT_CHECK_EQ(g_hits, 1);
        KT_CHECK_EQ(g_hub.disp.e[0].owner, "me");                                          // owner id reaches the provider
        c.release();                                                                       // unregisters only the accepted ones
        KT_CHECK_EQ(g_unregCalls, 2); KT_CHECK(!c.connected); KT_CHECK_EQ(g_hub.disp.n, 0);
        g_hub.emit(K2SE_SAVED, "saves/000012 - G", 0); KT_CHECK_EQ(g_hits, 1);
        char b[8]; KT_CHECK_EQ(c.currentSaveFolder(b, sizeof b), 0);                       // released = inert again
        g_refuseMask = 0; KT_CHECK(c.connect()); KT_CHECK_EQ(g_hub.disp.n, 3);             // and can reconnect
        // capacity
        k2se::Client f; f.init(fakekp::api(), "t");
        for (int i = 0; i < k2se::MaxRegs; ++i) KT_CHECK(f.add(K2SE_ALL, cb));
        KT_CHECK(!f.add(K2SE_ALL, cb)); KT_CHECK_EQ(f.nRegs, k2se::MaxRegs);
        // zero registrations: connected, "registered"
        resetAll(); fakekp::provide(K2SE_IFACE, K2SE_VERSION, &g_table);
        k2se::Client z; z.init(fakekp::api(), "z"); z.onConnect = onConn; KT_CHECK(z.connect()); KT_CHECK(g_lastOk); KT_CHECK(why("registered"));
    }
    return kt::Report("saveevents_client_test");
}
