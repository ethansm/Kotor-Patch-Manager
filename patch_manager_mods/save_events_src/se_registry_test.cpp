// Host test of the save-events registry (SeDispatch.h): kind-mask filtering, token accounting, re-entrancy (a callback that
// unregisters itself / registers another / unregisters a later consumer), the slow-consumer guard with an injected clock, save-folder
// parsing fixtures (local / cloud / bad), CurrentSaveFolder buffer rules, and the K2SaveEvent / K2SaveEventsApi ABI layout.
// Doc 22 section 6.2 (the k2.saveevents bullets). The client side (k2se::Client) is saveevents_client_test.cpp.
#include "SeDispatch.h"
#include "check.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

// ---- ABI layout: the numbers a consumer DLL built separately relies on (the 32-bit game; the API table holds pointers) ----
#if UINTPTR_MAX == 0xffffffff
static_assert(sizeof(K2SaveEvent) == 16, "K2SaveEvent size");
static_assert(offsetof(K2SaveEvent, folder) == 8, "ev.folder");
static_assert(offsetof(K2SaveEvent, cloud) == 12, "ev.cloud");
static_assert(sizeof(K2SaveEventsApi) == 16, "K2SaveEventsApi size");
static_assert(offsetof(K2SaveEventsApi, Register) == 4, "api.Register");
static_assert(offsetof(K2SaveEventsApi, Unregister) == 8, "api.Unregister");
static_assert(offsetof(K2SaveEventsApi, CurrentSaveFolder) == 12, "api.CurrentSaveFolder");
#endif
static_assert(offsetof(K2SaveEvent, struct_size) == 0, "ev.struct_size");
static_assert(offsetof(K2SaveEvent, kind) == 4, "ev.kind");
static_assert(offsetof(K2SaveEventsApi, struct_size) == 0, "api.struct_size");
static_assert(K2SE_SAVED == 1 && K2SE_AUTOSAVED == 2 && K2SE_LOAD_REQUEST == 4 && K2SE_NEW_GAME == 8 && K2SE_ALL == 15, "kind bits");

// ---- recording consumers ----
struct Seen { int32_t kind; std::string folder; bool nullFolder; int32_t cloud; uint32_t size; int id; };
static std::vector<Seen> g_seen;
static void KPATCH_CALL rec(const K2SaveEvent* ev, void* user) {
    g_seen.push_back({ ev->kind, ev->folder ? ev->folder : "", ev->folder == nullptr, ev->cloud, ev->struct_size, int(reinterpret_cast<intptr_t>(user)) });
}
static std::string ids() { std::string s; for (const Seen& x : g_seen) s += char('0' + x.id); return s; }
static void clearSeen() { g_seen.clear(); }
static void KPATCH_CALL nop(const K2SaveEvent*, void*) {}

// ---- fake clock: each callback advances it by g_step[id] microseconds ----
static uint32_t g_clock = 0;
static uint32_t g_step[8];
static std::vector<std::string> g_slowLog;
static uint32_t clk(void*) { return g_clock; }
static void slowCb(void*, const se::Entry& e, uint32_t us, int32_t kind) {
    g_slowLog.push_back(std::string(e.owner) + ":" + std::to_string(us) + ":" + std::to_string(kind));
}
static void KPATCH_CALL timed(const K2SaveEvent*, void* user) { g_clock += g_step[reinterpret_cast<intptr_t>(user)]; }

// ---- re-entrancy fixtures ----
static se::Hub* g_hub = nullptr;
static int32_t g_selfTok = 0, g_lateTok = 0, g_regResult = 0, g_newTok = 0;
static int g_selfCalls = 0, g_newCalls = 0;
static void KPATCH_CALL selfRemoving(const K2SaveEvent*, void*) {
    ++g_selfCalls;
    g_hub->disp.remove(g_selfTok);
    g_newTok = g_hub->disp.add(K2SE_ALL, [](const K2SaveEvent*, void*) { ++g_newCalls; }, nullptr, "new");   // registers another
}
static void KPATCH_CALL removesLate(const K2SaveEvent*, void*) { g_hub->disp.remove(g_lateTok); }
static void KPATCH_CALL registersWhenFull(const K2SaveEvent*, void*) { g_regResult = g_hub->disp.add(K2SE_ALL, nop, nullptr, "x"); }

int main() {
    // 1. folder parsing fixtures (copied from HotbarLogic.h's, so the hotbar migration cannot change behaviour)
    {
        using se::normalizeSaveFolder;
        KT_CHECK_EQ(normalizeSaveFolder("SAVES:000012 - Game11").c_str(), "000012 - Game11");
        KT_CHECK_EQ(normalizeSaveFolder("saves:000012 - Game11").c_str(), "000012 - Game11");     // alias is case-insensitive
        KT_CHECK_EQ(normalizeSaveFolder("SaVeS:000012 - Game11").c_str(), "000012 - Game11");
        KT_CHECK_EQ(normalizeSaveFolder("000012 - Game11").c_str(), "000012 - Game11");
        KT_CHECK_EQ(normalizeSaveFolder("SAVES:/000012 - Game11").c_str(), "000012 - Game11");
        KT_CHECK_EQ(normalizeSaveFolder("SAVES:\\\\000012 - Game11").c_str(), "000012 - Game11");
        KT_CHECK_EQ(normalizeSaveFolder("/000001 - AUTOSAVE").c_str(), "000001 - AUTOSAVE");
        KT_CHECK_EQ(normalizeSaveFolder("SAVES:").c_str(), "");
        KT_CHECK_EQ(normalizeSaveFolder("").c_str(), "");
        KT_CHECK_EQ(normalizeSaveFolder("12 - Game").c_str(), "");                                 // too short
        KT_CHECK_EQ(normalizeSaveFolder("000012 - ").c_str(), "");                                 // no name
        KT_CHECK_EQ(normalizeSaveFolder("000012 - G").c_str(), "000012 - G");                      // 10 chars is the minimum
        KT_CHECK_EQ(normalizeSaveFolder("00001x - Game11").c_str(), "");                           // non-digit
        KT_CHECK_EQ(normalizeSaveFolder("A00012 - Game11").c_str(), "");                           // first char is checked
        KT_CHECK_EQ(normalizeSaveFolder("000012-Game11").c_str(), "");                             // separator
        KT_CHECK_EQ(normalizeSaveFolder("000012  -Game11").c_str(), "");
        KT_CHECK_EQ(normalizeSaveFolder("SAVES:SAVES:000012 - Game11").c_str(), "");               // one alias only
        KT_CHECK_EQ(normalizeSaveFolder("999999 - Game11").c_str(), "999999 - Game11");
        KT_CHECK_EQ(normalizeSaveFolder("SAVEX:000012 - Game11").c_str(), "");                     // not the alias
        KT_CHECK_EQ(normalizeSaveFolder("XAVES:000012 - Game11").c_str(), "");                     // every alias char is compared (first)
        KT_CHECK_EQ(normalizeSaveFolder("SAVESX000012 - Game11").c_str(), "");                     // ... and the colon (last)
        KT_CHECK_EQ(se::lowerAscii('Z'), 'z'); KT_CHECK_EQ(se::lowerAscii('A'), 'a'); KT_CHECK_EQ(se::lowerAscii('M'), 'm');
        KT_CHECK_EQ(se::lowerAscii('@'), '@'); KT_CHECK_EQ(se::lowerAscii('['), '['); KT_CHECK_EQ(se::lowerAscii('z'), 'z'); KT_CHECK_EQ(se::lowerAscii(':'), ':');
        KT_CHECK_EQ(se::MaxConsumers, 16); KT_CHECK_EQ(se::SlowThresholdUs, 5000u);                 // documented limits (doc 22 4.1a)
        KT_CHECK_EQ(se::joinSaveDir("saves/", "000012 - G").c_str(), "saves/000012 - G");
        KT_CHECK_EQ(se::joinSaveDir("cloudsaves/123/", "000012 - G").c_str(), "cloudsaves/123/000012 - G");
        KT_CHECK_EQ(se::joinSaveDir("cloudsaves/123", "000012 - G").c_str(), "cloudsaves/123/000012 - G");   // missing slash added
        KT_CHECK_EQ(se::joinSaveDir("cloudsaves\\123\\", "000012 - G").c_str(), "cloudsaves\\123\\000012 - G");
        KT_CHECK_EQ(se::joinSaveDir("", "000012 - G").c_str(), "saves/000012 - G");                // empty root -> saves/

        // absFolder: local / cloud / cloud with an unreadable root / bad
        int32_t cl = 7;
        KT_CHECK_EQ(se::absFolder(false, "", "SAVES:000012 - Game11", &cl).c_str(), "saves/000012 - Game11");
        KT_CHECK_EQ(cl, 0);
        KT_CHECK_EQ(se::absFolder(false, "cloudsaves/9/", "SAVES:000012 - Game11", &cl).c_str(), "saves/000012 - Game11");   // flag clear: root ignored
        KT_CHECK_EQ(cl, 0);
        KT_CHECK_EQ(se::absFolder(true, "cloudsaves/76561198000000001/", "SAVES:000012 - Game11", &cl).c_str(), "cloudsaves/76561198000000001/000012 - Game11");
        KT_CHECK_EQ(cl, 1);
        KT_CHECK_EQ(se::absFolder(true, "cloudsaves/9", "000001 - AUTOSAVE", &cl).c_str(), "cloudsaves/9/000001 - AUTOSAVE");
        KT_CHECK_EQ(cl, 1);
        KT_CHECK_EQ(se::absFolder(true, "", "SAVES:000012 - Game11", &cl).c_str(), "saves/000012 - Game11");   // cloud byte set but root unreadable
        KT_CHECK_EQ(cl, 0);                                                                                      // ... so not reported as cloud
        cl = 7;
        KT_CHECK_EQ(se::absFolder(true, "cloudsaves/9/", "garbage", &cl).c_str(), "");
        KT_CHECK_EQ(cl, 0);
        KT_CHECK_EQ(se::absFolder(false, "", "", nullptr).c_str(), "");                           // null cloudOut is fine
        KT_CHECK_EQ(se::absFolder(true, "cloudsaves/9/", "SAVES:000012 - G").c_str(), "cloudsaves/9/000012 - G");
    }

    // 2. mask filtering + registration order + event fields
    {
        se::Hub h; clearSeen();
        KT_CHECK(h.disp.add(K2SE_SAVED, rec, (void*)1, "a") > 0);
        KT_CHECK(h.disp.add(K2SE_LOAD_REQUEST | K2SE_NEW_GAME, rec, (void*)2, "b") > 0);
        KT_CHECK(h.disp.add(K2SE_ALL, rec, (void*)3, "c") > 0);
        KT_CHECK(h.disp.add(K2SE_AUTOSAVED, rec, (void*)4, "d") > 0);
        KT_CHECK_EQ(h.emit(K2SE_SAVED, "saves/000012 - G", 0), 2); KT_CHECK_EQ(ids().c_str(), "13"); clearSeen();
        KT_CHECK_EQ(h.emit(K2SE_AUTOSAVED, "saves/000001 - AUTOSAVE", 0), 2); KT_CHECK_EQ(ids().c_str(), "34"); clearSeen();
        KT_CHECK_EQ(h.emit(K2SE_LOAD_REQUEST, "cloudsaves/9/000012 - G", 1), 2); KT_CHECK_EQ(ids().c_str(), "23");
        KT_CHECK(g_seen[0].folder == "cloudsaves/9/000012 - G" && g_seen[0].cloud == 1 && !g_seen[0].nullFolder);
        KT_CHECK_EQ(g_seen[0].kind, K2SE_LOAD_REQUEST);
        KT_CHECK_EQ(g_seen[0].size, (unsigned)sizeof(K2SaveEvent));                                   // struct_size is filled
        clearSeen();
        KT_CHECK_EQ(h.emit(K2SE_NEW_GAME, "ignored/for/new/game", 1), 2); KT_CHECK_EQ(ids().c_str(), "23");
        KT_CHECK(g_seen[0].nullFolder);                                                               // NEW_GAME: folder NULL, cloud 0
        KT_CHECK_EQ(g_seen[0].cloud, 0);
        clearSeen();
        KT_CHECK_EQ(h.emit(K2SE_SAVED, "saves/000012 - G", 5), 2); KT_CHECK_EQ(g_seen[0].cloud, 1);  // any non-zero cloud reads as 1
        clearSeen(); KT_CHECK_EQ(h.emit(K2SE_SAVED, "saves/000012 - G", 0), 2); KT_CHECK_EQ(g_seen[0].cloud, 0);   // and zero stays 0
        clearSeen(); KT_CHECK_EQ(h.emit(K2SE_SAVED, "saves/000012 - G", 5), 2);
        KT_CHECK_EQ(h.disp.count(K2SE_SAVED), 2); KT_CHECK_EQ(h.disp.count(K2SE_NEW_GAME), 2);

        // bad kinds / bad folders dispatch nothing and leave `current` alone
        clearSeen(); h.current = "keep";
        KT_CHECK_EQ(h.emit(0, "saves/000012 - G", 0), -1);
        KT_CHECK_EQ(h.emit(3, "saves/000012 - G", 0), -1);                                           // two bits is not one event
        KT_CHECK_EQ(h.emit(16, "saves/000012 - G", 0), -1);
        KT_CHECK_EQ(h.emit(-1, "saves/000012 - G", 0), -1);
        KT_CHECK_EQ(h.emit(K2SE_SAVED, nullptr, 0), -1);
        KT_CHECK_EQ(h.emit(K2SE_LOAD_REQUEST, "", 0), -1);
        KT_CHECK(g_seen.empty()); KT_CHECK(h.current == "keep");
        K2SaveEvent bad = {}; bad.struct_size = sizeof bad; bad.kind = 6;
        KT_CHECK_EQ(h.disp.dispatch(bad), 0); KT_CHECK(g_seen.empty());

        // nothing registered for a kind: 0 called, but `current` still tracks it
        se::Hub e; KT_CHECK_EQ(e.emit(K2SE_SAVED, "saves/000003 - Q", 0), 0); KT_CHECK(e.current == "saves/000003 - Q");
    }

    // 3. Register / Unregister accounting
    {
        se::Dispatcher d;
        KT_CHECK_EQ(d.add(K2SE_SAVED, nullptr, nullptr, "x"), -1);                                   // null fn
        KT_CHECK_EQ(d.add(0, nop, nullptr, "x"), -1);                                                // empty mask
        KT_CHECK_EQ(d.add(0x10, nop, nullptr, "x"), -1);                                             // only an unknown kind bit
        int32_t t1 = d.add(K2SE_SAVED | 0x10, nop, nullptr, nullptr);                                // unknown bits ignored, null owner ok
        KT_CHECK(t1 > 0); KT_CHECK_EQ(d.e[0].mask, K2SE_SAVED); KT_CHECK(d.e[0].owner && !*d.e[0].owner);
        int32_t t2 = d.add(K2SE_ALL, nop, nullptr, "b");
        KT_CHECK(t2 > t1);                                                                           // tokens increase, never 0
        KT_CHECK(d.remove(t1)); KT_CHECK(!d.remove(t1)); KT_CHECK(!d.remove(0)); KT_CHECK(!d.remove(-3));
        int32_t t3 = d.add(K2SE_ALL, nop, nullptr, "c");
        KT_CHECK(t3 > t2);                                                                           // a removed token is not reused
        KT_CHECK(!d.live(t1) && d.live(t2) && d.live(t3));
        // removing the LAST entry must leave no stale copy behind (live / remove / count never see past n)
        KT_CHECK(d.remove(t3)); KT_CHECK(!d.live(t3)); KT_CHECK(!d.remove(t3)); KT_CHECK_EQ(d.n, 1); KT_CHECK_EQ(d.count(K2SE_SAVED), 1);
        KT_CHECK_EQ(d.count(K2SE_NEW_GAME), 1); KT_CHECK(d.live(t2));
        t3 = d.add(K2SE_ALL, nop, nullptr, "c"); KT_CHECK(t3 > t2); KT_CHECK_EQ(d.n, 2);
        for (int i = d.n; i < se::MaxConsumers; ++i) KT_CHECK(d.add(K2SE_ALL, nop, nullptr, "f") > 0);
        KT_CHECK_EQ(d.n, se::MaxConsumers);
        KT_CHECK_EQ(d.add(K2SE_ALL, nop, nullptr, "over"), -2);                                      // full
        KT_CHECK(d.remove(t2));
        KT_CHECK(d.add(K2SE_ALL, nop, nullptr, "again") > 0);                                        // room again after a remove
    }

    // 4. re-entrancy
    {
        // (a) a callback unregisters itself and registers another: the new one waits for the next event
        se::Hub h; g_hub = &h; g_selfCalls = g_newCalls = 0; g_newTok = 0;
        g_selfTok = h.disp.add(K2SE_ALL, selfRemoving, nullptr, "self");
        clearSeen(); h.disp.add(K2SE_ALL, rec, (void*)7, "tail");
        KT_CHECK_EQ(h.emit(K2SE_SAVED, "saves/000012 - G", 0), 2);                                    // self + tail; the new one is NOT in this pass
        KT_CHECK_EQ(g_selfCalls, 1); KT_CHECK_EQ(g_newCalls, 0); KT_CHECK_EQ(ids().c_str(), "7");
        KT_CHECK(g_newTok > 0); KT_CHECK(!h.disp.live(g_selfTok)); KT_CHECK(h.disp.live(g_newTok));
        KT_CHECK_EQ(h.disp.n, 2);
        KT_CHECK_EQ(h.emit(K2SE_SAVED, "saves/000012 - G", 0), 2);                                    // tail + new
        KT_CHECK_EQ(g_selfCalls, 1); KT_CHECK_EQ(g_newCalls, 1);

        // (b) a callback unregisters a LATER consumer: that one is skipped in the same pass, the one after it still runs
        se::Hub h2; g_hub = &h2; clearSeen();
        h2.disp.add(K2SE_ALL, removesLate, nullptr, "killer");
        g_lateTok = h2.disp.add(K2SE_ALL, rec, (void*)5, "late");
        h2.disp.add(K2SE_ALL, rec, (void*)6, "after");
        KT_CHECK_EQ(h2.emit(K2SE_LOAD_REQUEST, "saves/000012 - G", 0), 2);
        KT_CHECK_EQ(ids().c_str(), "6");

        // (c) registering when the table is full from inside a callback is refused cleanly
        se::Hub h3; g_hub = &h3; g_regResult = 0;
        h3.disp.add(K2SE_ALL, registersWhenFull, nullptr, "full");
        while (h3.disp.n < se::MaxConsumers) h3.disp.add(K2SE_ALL, nop, nullptr, "f");
        KT_CHECK_EQ(h3.emit(K2SE_NEW_GAME, nullptr, 0), se::MaxConsumers);
        KT_CHECK_EQ(g_regResult, -2); KT_CHECK_EQ(h3.disp.n, se::MaxConsumers);
    }

    // 5. slow-consumer guard (injected microsecond clock): > 5000 us is logged, == 5000 is not, dispatching continues
    {
        se::Hub h; clearSeen();
        h.disp.timing.now = clk; h.disp.timing.slow = slowCb;
        g_slowLog.clear(); g_clock = 0;
        g_step[1] = 5000; g_step[2] = 5001; g_step[3] = 9000000; g_step[4] = 0;
        h.disp.add(K2SE_ALL, timed, (void*)1, "edge");
        h.disp.add(K2SE_ALL, timed, (void*)2, "just-over");
        h.disp.add(K2SE_ALL, timed, (void*)3, "very-slow");
        h.disp.add(K2SE_ALL, rec, (void*)4, "after");
        KT_CHECK_EQ(h.emit(K2SE_AUTOSAVED, "saves/000001 - AUTOSAVE", 0), 4);                        // the slow ones did not stop the pass
        KT_CHECK_EQ(ids().c_str(), "4");
        KT_CHECK_EQ((long long)g_slowLog.size(), 2LL);
        KT_CHECK(g_slowLog.size() == 2 && g_slowLog[0] == "just-over:5001:2" && g_slowLog[1] == "very-slow:9000000:2");
        KT_CHECK_EQ(h.disp.slowCalls, 2u);
        // clock wrap: t0 near 2^32, the callback crosses the wrap
        g_slowLog.clear(); g_clock = 0xFFFFF000u; g_step[2] = 0x2000;
        se::Hub w; w.disp.timing = h.disp.timing; w.disp.add(K2SE_ALL, timed, (void*)2, "wrap");
        KT_CHECK_EQ(w.emit(K2SE_SAVED, "saves/000001 - A", 0), 1);
        KT_CHECK(g_slowLog.size() == 1 && g_slowLog[0] == "wrap:8192:1");
        // custom threshold; no clock = guard off (never reports)
        g_slowLog.clear(); g_clock = 0; g_step[1] = 100;
        se::Hub t; t.disp.timing = h.disp.timing; t.disp.timing.thresholdUs = 99; t.disp.add(K2SE_ALL, timed, (void*)1, "t");
        t.emit(K2SE_SAVED, "saves/000001 - A", 0); KT_CHECK_EQ((long long)g_slowLog.size(), 1LL);
        g_slowLog.clear(); se::Hub o; o.disp.add(K2SE_ALL, timed, (void*)1, "off"); g_step[1] = 99999999;
        KT_CHECK_EQ(o.emit(K2SE_SAVED, "saves/000001 - A", 0), 1); KT_CHECK(g_slowLog.empty()); KT_CHECK_EQ(o.disp.slowCalls, 0u);
        // a slow callback with no slow() sink is only counted
        se::Hub q; q.disp.timing.now = clk; q.disp.add(K2SE_ALL, timed, (void*)1, "q"); g_step[1] = 6000;
        KT_CHECK_EQ(q.emit(K2SE_SAVED, "saves/000001 - A", 0), 1); KT_CHECK_EQ(q.disp.slowCalls, 1u);
        // the guard times only consumers that were called (a masked-out slow consumer costs nothing and logs nothing)
        g_slowLog.clear(); se::Hub m; m.disp.timing = h.disp.timing; g_step[1] = 9000;
        m.disp.add(K2SE_NEW_GAME, timed, (void*)1, "ng");
        KT_CHECK_EQ(m.emit(K2SE_SAVED, "saves/000001 - A", 0), 0); KT_CHECK(g_slowLog.empty());
    }

    // 6. CurrentSaveFolder: tracked, set before callbacks, cleared by NEW_GAME, buffer rules
    {
        se::Hub h;
        char buf[64]; memset(buf, 'x', sizeof buf);
        KT_CHECK_EQ(h.copyCurrent(buf, sizeof buf), 0); KT_CHECK_EQ(buf[0], 0);                      // none: 0 and an empty string
        static se::Hub* hh; static std::string seenInCb; hh = &h;
        h.disp.add(K2SE_ALL, [](const K2SaveEvent*, void*) { char b[64]; hh->copyCurrent(b, sizeof b); seenInCb = b; }, nullptr, "peek");
        h.emit(K2SE_SAVED, "saves/000012 - Game11", 0);
        KT_CHECK(seenInCb == "saves/000012 - Game11");                                               // updated BEFORE the callbacks
        const int32_t full = int32_t(strlen("saves/000012 - Game11"));
        KT_CHECK_EQ(h.copyCurrent(buf, sizeof buf), full); KT_CHECK_EQ(buf, "saves/000012 - Game11");
        h.emit(K2SE_LOAD_REQUEST, "cloudsaves/9/000013 - X", 1);
        KT_CHECK_EQ(h.copyCurrent(buf, sizeof buf), 23); KT_CHECK_EQ(buf, "cloudsaves/9/000013 - X");
        h.emit(K2SE_AUTOSAVED, "saves/000001 - AUTOSAVE", 0);
        KT_CHECK_EQ(h.copyCurrent(buf, sizeof buf), 23); KT_CHECK_EQ(buf, "saves/000001 - AUTOSAVE");
        // truncation: always NUL-terminated, return = full length
        memset(buf, 'x', sizeof buf);
        KT_CHECK_EQ(h.copyCurrent(buf, 8), 23); KT_CHECK_EQ(buf, "saves/0"); KT_CHECK_EQ(buf[7], 0); KT_CHECK_EQ(buf[8], 'x');   // 7 chars + NUL
        memset(buf, 'x', sizeof buf);
        KT_CHECK_EQ(h.copyCurrent(buf, 1), 23); KT_CHECK_EQ(buf[0], 0); KT_CHECK_EQ(buf[1], 'x');   // cap 1: just the NUL
        memset(buf, 'x', sizeof buf);
        KT_CHECK_EQ(h.copyCurrent(buf, 0), 23); KT_CHECK_EQ(buf[0], 'x');                            // cap 0: nothing written
        KT_CHECK_EQ(h.copyCurrent(nullptr, 0), 23); KT_CHECK_EQ(h.copyCurrent(nullptr, 10), 23);     // length query
        memset(buf, 'x', sizeof buf);
        KT_CHECK_EQ(h.copyCurrent(buf, 24), 23); KT_CHECK_EQ(buf, "saves/000001 - AUTOSAVE");        // cap == len + 1: exact fit
        memset(buf, 'x', sizeof buf);
        KT_CHECK_EQ(h.copyCurrent(buf, 23), 23); KT_CHECK_EQ(buf[22], 0); KT_CHECK_EQ(strlen(buf), 22u);   // cap == len: last char cut
        // a rejected event does not change it; NEW_GAME clears it
        h.emit(K2SE_SAVED, "", 0); KT_CHECK_EQ(h.copyCurrent(nullptr, 0), 23);
        h.emit(K2SE_NEW_GAME, nullptr, 0);
        memset(buf, 'x', sizeof buf);
        KT_CHECK_EQ(h.copyCurrent(buf, sizeof buf), 0); KT_CHECK_EQ(buf[0], 0);
        KT_CHECK(seenInCb.empty());                                                                  // NEW_GAME callback saw "none"
    }

    return kt::Report("se_registry_test");
}
