// Host test of nw::World (NwCore.h): per-load reset and salt, the 30-tick POP-STABLE latch, seen tags, event payloads, spawn gating
// (stable, tag, cap, re-entrancy, engine failure), the ini-off passive mode and area re-arm. No engine, no Win32.
#include "NwCore.h"
#include "check.h"
#include <string.h>
#include <string>
#include <vector>

struct Seen { int32_t kind; int32_t loadSeq, visitSeq; uint32_t salt; std::string area, tag; uint32_t oid; int32_t origin; void* areaPtr; };
static std::vector<Seen> g_seen;
static void KPATCH_CALL rec(const K2NwEvent* e, void*) {
    g_seen.push_back({ e->kind, e->load_seq, e->visit_seq, e->visit_salt, e->area, e->tag, e->oid, e->origin, e->area_ptr });
}
static int count(int32_t kind) { int c = 0; for (auto& s : g_seen) if (s.kind == kind) ++c; return c; }

static char g_areaA[4], g_areaB[4];   // distinct addresses used as fake area pointers
static nw::World* g_w = nullptr;
static int g_engineCalls = 0, g_nextOid = 0x7f000100;
static bool g_engineFail = false;
static std::string g_lastResref; static float g_lastX = 0;
static std::string g_engineTagToReport;   // the tag the fake engine's AddToArea reports back (re-entry)

static uint32_t fakeSpawn(void*, void* area, const char* resref, float x, float, float, float) {
    ++g_engineCalls; g_lastResref = resref; g_lastX = x;
    if (g_engineFail) return 0;
    nw::CreatureInfo ci; ci.oid = uint32_t(g_nextOid); ci.tag = g_engineTagToReport.c_str(); ci.areaPtr = area; ci.areaRes = g_w->area();
    g_w->onCreatureAdded(ci);   // the native AddToArea re-enters the hook
    return uint32_t(g_nextOid++);
}

static void add(nw::World& w, const char* tag, void* area, const char* res, uint32_t oid = 1) {
    nw::CreatureInfo ci; ci.oid = oid; ci.tag = tag; ci.areaPtr = area; ci.areaRes = res;
    w.onCreatureAdded(ci);
}
static void ticks(nw::World& w, int n) { for (int i = 0; i < n; ++i) w.onTick(); }
static void fresh(nw::World& w) { w = nw::World(); w.eng.spawn = fakeSpawn; g_w = &w; g_seen.clear(); g_engineCalls = 0; g_engineFail = false; g_engineTagToReport.clear(); }

int main() {
    nw::World w;
    // 1. load: counter, salt, reset
    {
        fresh(w); w.disp.add(K2NW_ALL & ~K2NW_TICK, rec, nullptr, "t");
        KT_CHECK_EQ(w.visitSalt(), 0u); KT_CHECK_EQ(w.loadSeq(), 0);
        w.onLoad(111);
        KT_CHECK_EQ(w.loadSeq(), 1); KT_CHECK(w.visitSalt() != 0); KT_CHECK_EQ(count(K2NW_AREA_LOAD), 1);
        KT_CHECK_EQ(g_seen[0].salt, w.visitSalt()); KT_CHECK(g_seen[0].area.empty()); KT_CHECK(g_seen[0].areaPtr == nullptr);
        const uint32_t s1 = w.visitSalt();
        add(w, "npc_a", g_areaA, "204tel"); KT_CHECK_EQ(w.visitSalt(), s1);   // the salt is constant for the whole visit
        ticks(w, 40); KT_CHECK_EQ(w.visitSalt(), s1);
        w.onLoad(111);                                                          // same entropy, next load: new salt (load counter mixed in)
        KT_CHECK(w.visitSalt() != s1); KT_CHECK_EQ(w.loadSeq(), 2);
        KT_CHECK(!w.stable()); KT_CHECK_EQ(w.seenTags(), 0); KT_CHECK(!w.tagPresent("npc_a")); KT_CHECK_EQ(w.area()[0], 0);
        KT_CHECK_EQ(w.visitSeq(), 1);                                           // visit_seq is per process, not reset by a load
    }
    // 2. POP-STABLE latch: exactly 30 quiet ticks after the area is known, engine adds reset it
    {
        fresh(w); w.disp.add(K2NW_ALL & ~K2NW_TICK, rec, nullptr, "t");
        w.onLoad(1);
        ticks(w, 100); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 0);                 // area unknown: never stable
        add(w, "a", g_areaA, "204tel");
        ticks(w, 1); KT_CHECK_EQ(w.stableRun(), 0);                             // the add tick itself does not count
        ticks(w, 29); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 0); KT_CHECK_EQ(w.stableRun(), 29);
        add(w, "b", g_areaA, "204tel");                                         // an add at run 29 resets
        ticks(w, 1); KT_CHECK_EQ(w.stableRun(), 0);
        ticks(w, 29); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 0);
        ticks(w, 1); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 1); KT_CHECK(w.stable()); KT_CHECK_EQ(w.visitSeq(), 1);
        ticks(w, 100); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 1);                 // once per load
        add(w, "late", g_areaA, "204tel"); ticks(w, 100); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 1);   // a late add does not re-fire
        for (auto& s : g_seen) if (s.kind == K2NW_AREA_STABLE) { KT_CHECK(s.area == "204tel"); KT_CHECK(s.areaPtr == g_areaA); KT_CHECK_EQ(s.visitSeq, 1); KT_CHECK_EQ(s.loadSeq, 1); KT_CHECK_EQ(s.salt, w.visitSalt()); }
        w.onLoad(2); add(w, "a", g_areaA, "204tel"); ticks(w, 31);              // next load: latch fires again, visit_seq counts up
        KT_CHECK_EQ(count(K2NW_AREA_STABLE), 2); KT_CHECK_EQ(w.visitSeq(), 2);
    }
    // 3. creature-added payload, tags, empty/unreadable inputs
    {
        fresh(w); w.disp.add(K2NW_CREATURE_ADDED, rec, nullptr, "t");
        w.onLoad(1);
        add(w, "MouseDroid", g_areaA, "204tel", 0x7f000005u);
        KT_CHECK_EQ(g_seen.size(), size_t(1)); KT_CHECK(g_seen[0].tag == "MouseDroid"); KT_CHECK_EQ(g_seen[0].oid, 0x7f000005u);
        KT_CHECK_EQ(g_seen[0].origin, K2NW_ORIGIN_ENGINE); KT_CHECK(g_seen[0].area == "204tel");
        KT_CHECK(w.tagPresent("mousedroid")); KT_CHECK(w.tagPresent("MOUSEDROID")); KT_CHECK(!w.tagPresent("other")); KT_CHECK(!w.tagPresent("")); KT_CHECK(!w.tagPresent(nullptr));
        add(w, "mousedroid", g_areaA, "204tel"); KT_CHECK_EQ(w.seenTags(), 1);   // case-insensitive dedupe
        add(w, "", g_areaA, "204tel"); KT_CHECK_EQ(w.seenTags(), 1);             // empty tag not remembered
        nw::CreatureInfo bad;                                                    // all defaults: no area, no tag
        w.onCreatureAdded(bad); KT_CHECK_EQ(g_seen.size(), size_t(4)); KT_CHECK(g_seen.back().area == "204tel");   // area from the earlier add stays
        char b[8]; KT_CHECK_EQ(w.currentArea(b, sizeof b), 6); KT_CHECK(strcmp(b, "204tel") == 0);
        KT_CHECK_EQ(w.currentArea(b, 4), 6); KT_CHECK(strcmp(b, "204") == 0);
        char longTag[40]; memset(longTag, 'q', 39); longTag[39] = 0; add(w, longTag, g_areaA, "204tel");
        KT_CHECK(w.tagPresent(longTag)); KT_CHECK(w.tagPresent("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"));   // stored as 32 chars
    }
    // 4. tag table overflow: unknown counts as present
    {
        fresh(w); w.onLoad(1);
        for (int i = 0; i < nw::MaxSeenTags; ++i) { char t[16]; snprintf(t, sizeof t, "t%d", i); add(w, t, g_areaA, "204tel"); }
        KT_CHECK_EQ(w.seenTags(), nw::MaxSeenTags); KT_CHECK(!w.seenOverflow()); KT_CHECK(!w.tagPresent("nothere"));
        add(w, "one_too_many", g_areaA, "204tel");
        KT_CHECK(w.seenOverflow()); KT_CHECK(w.tagPresent("nothere")); KT_CHECK(w.tagPresent("t3"));
        w.onLoad(2); KT_CHECK(!w.seenOverflow()); KT_CHECK(!w.tagPresent("nothere"));   // a load clears it
    }
    // 5. TICK events only when somebody listens, once per tick
    {
        fresh(w); w.onLoad(1);
        ticks(w, 3); KT_CHECK_EQ(w.tick(), 3u);
        w.disp.add(K2NW_TICK, rec, nullptr, "t"); ticks(w, 5); KT_CHECK_EQ(count(K2NW_TICK), 5); KT_CHECK_EQ(g_seen.back().kind, K2NW_TICK);
    }
    // 6. spawn gating
    {
        fresh(w); w.disp.add(K2NW_CREATURE_ADDED | K2NW_AREA_STABLE, rec, nullptr, "t");
        uint32_t oid = 77;
        KT_CHECK(w.spawn("c_drdmouse01", "MouseDroid", 1, 2, 3, 0, &oid) == nw::SpawnResult::BadArgs); KT_CHECK_EQ(oid, 0u);   // before any load/area
        w.onLoad(1); add(w, "a", g_areaA, "204tel");
        KT_CHECK(w.spawn("c_drdmouse01", "MouseDroid", 1, 2, 3, 0, &oid) == nw::SpawnResult::NotStable);
        ticks(w, 31); KT_CHECK(w.stable());
        KT_CHECK(w.spawn(nullptr, "x", 0, 0, 0, 0, &oid) == nw::SpawnResult::BadArgs);
        KT_CHECK(w.spawn("", "x", 0, 0, 0, 0, &oid) == nw::SpawnResult::BadArgs);
        KT_CHECK_EQ(g_engineCalls, 0);
        g_engineTagToReport = "MouseDroid"; g_seen.clear();
        KT_CHECK(w.spawn("c_drdmouse01", "MouseDroid", 4.5f, 2, 3, 0, &oid) == nw::SpawnResult::Ok);
        KT_CHECK_EQ(oid, 0x7f000100u); KT_CHECK_EQ(g_engineCalls, 1); KT_CHECK(g_lastResref == "c_drdmouse01"); KT_CHECK(g_lastX == 4.5f);
        KT_CHECK_EQ(w.spawnedCount(), 1); KT_CHECK(w.tagPresent("MouseDroid"));
        KT_CHECK_EQ(g_seen.size(), size_t(1)); KT_CHECK_EQ(g_seen[0].kind, K2NW_CREATURE_ADDED); KT_CHECK_EQ(g_seen[0].origin, K2NW_ORIGIN_LIB);   // re-entry delivered as LIB
        KT_CHECK(!w.spawning());
        KT_CHECK(w.spawn("c_drdmouse01", "mousedroid", 0, 0, 0, 0, &oid) == nw::SpawnResult::TagPresent); KT_CHECK_EQ(oid, 0u); KT_CHECK_EQ(g_engineCalls, 1);
        // cap
        w.spawnCap = 3; g_engineTagToReport.clear();
        KT_CHECK(w.spawn("r1", "n1", 0, 0, 0, 0, &oid) == nw::SpawnResult::Ok);
        KT_CHECK(w.spawn("r2", "", 0, 0, 0, 0, &oid) == nw::SpawnResult::Ok);                  // empty tag: no duplicate check
        KT_CHECK(w.spawn("r3", "n3", 0, 0, 0, 0, &oid) == nw::SpawnResult::CapReached); KT_CHECK_EQ(w.spawnedCount(), 3);
        KT_CHECK(w.spawn("r2", "", 0, 0, 0, 0, &oid) == nw::SpawnResult::CapReached);
        // a failed engine call does not count and does not remember the tag
        fresh(w); w.onLoad(1); add(w, "a", g_areaA, "204tel"); ticks(w, 31);
        g_engineFail = true;
        KT_CHECK(w.spawn("bad", "badtag", 0, 0, 0, 0, &oid) == nw::SpawnResult::EngineFailed); KT_CHECK_EQ(w.spawnedCount(), 0); KT_CHECK(!w.tagPresent("badtag"));
        KT_CHECK(!w.spawning());
        g_engineFail = false;
        KT_CHECK(w.spawn("good", "badtag", 0, 0, 0, 0, &oid) == nw::SpawnResult::Ok);
        // the cap resets per visit
        w.onLoad(2); add(w, "a", g_areaA, "204tel"); ticks(w, 31); KT_CHECK_EQ(w.spawnedCount(), 0);
        KT_CHECK(w.spawn("good", "badtag", 0, 0, 0, 0, &oid) == nw::SpawnResult::Ok);          // tag list was cleared by the load
    }
    // 7. cap clamp and zero cap
    {
        fresh(w); w.spawnCap = 1000; KT_CHECK_EQ(w.effectiveCap(), nw::HardSpawnCap);
        w.spawnCap = -5; KT_CHECK_EQ(w.effectiveCap(), 0); w.spawnCap = 0;
        w.onLoad(1); add(w, "a", g_areaA, "204tel"); ticks(w, 31); uint32_t oid;
        KT_CHECK(w.spawn("x", "y", 0, 0, 0, 0, &oid) == nw::SpawnResult::CapReached); KT_CHECK_EQ(g_engineCalls, 0);
    }
    // 8. Spawn from an AREA_STABLE callback, and re-entrant Spawn from a LIB add callback is refused
    {
        fresh(w);
        struct Ctx { nw::World* w; int refused; int ok; };
        static Ctx c; c = { &w, 0, 0 };
        auto onStable = [](const K2NwEvent* e, void* u) {
            Ctx* cx = static_cast<Ctx*>(u);
            if (e->kind == K2NW_AREA_STABLE) { if (cx->w->spawn("r1", "s1", 0, 0, 0, 0, nullptr) == nw::SpawnResult::Ok) ++cx->ok; }
            else if (e->kind == K2NW_CREATURE_ADDED && e->origin == K2NW_ORIGIN_LIB) {
                if (cx->w->spawn("r2", "s2", 0, 0, 0, 0, nullptr) == nw::SpawnResult::Reentrant) ++cx->refused;
            }
        };
        w.disp.add(K2NW_AREA_STABLE | K2NW_CREATURE_ADDED, onStable, &c, "t");
        w.onLoad(1); add(w, "a", g_areaA, "204tel"); ticks(w, 31);
        KT_CHECK_EQ(c.ok, 1); KT_CHECK_EQ(c.refused, 1); KT_CHECK_EQ(w.spawnedCount(), 1);
        KT_CHECK_EQ(g_engineCalls, 1);
        KT_CHECK(w.stable());
    }
    // 9. area change without a load event re-arms the gate and the spawn counter
    {
        fresh(w); w.disp.add(K2NW_AREA_STABLE, rec, nullptr, "t");
        w.onLoad(1); add(w, "a", g_areaA, "204tel"); ticks(w, 31); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 1);
        uint32_t oid; KT_CHECK(w.spawn("r", "t1", 0, 0, 0, 0, &oid) == nw::SpawnResult::Ok);
        add(w, "b", g_areaB, "261tel");
        KT_CHECK(!w.stable()); KT_CHECK(strcmp(w.area(), "261tel") == 0); KT_CHECK_EQ(w.spawnedCount(), 0);
        ticks(w, 31); KT_CHECK_EQ(count(K2NW_AREA_STABLE), 2); KT_CHECK(g_seen.back().area == "261tel" && g_seen.back().areaPtr == g_areaB);
        add(w, "c", g_areaA, "261TEL");                                          // same resref in another case: same area, no re-arm
        KT_CHECK(w.stable());
        add(w, "d", g_areaB, "this_is_a_very_long_resref");                     // > 16 chars: stored truncated
        KT_CHECK_EQ(strlen(w.area()), size_t(16));
    }
    // 10. enabled=false keeps the hooks passive
    {
        fresh(w); w.enabled = false; w.disp.add(K2NW_ALL, rec, nullptr, "t");
        w.onLoad(1); add(w, "a", g_areaA, "204tel"); ticks(w, 50);
        KT_CHECK(g_seen.empty()); KT_CHECK_EQ(w.visitSalt(), 0u); KT_CHECK_EQ(w.loadSeq(), 0); KT_CHECK_EQ(w.tick(), 0u); KT_CHECK_EQ(w.seenTags(), 0);
        uint32_t oid = 5; KT_CHECK(w.spawn("r", "t", 0, 0, 0, 0, &oid) == nw::SpawnResult::Disabled); KT_CHECK_EQ(oid, 0u);
        w.enabled = true; w.eng.spawn = nullptr; w.onLoad(1); add(w, "a", g_areaA, "204tel"); ticks(w, 31);
        KT_CHECK(w.spawn("r", "t", 0, 0, 0, 0, &oid) == nw::SpawnResult::NoEngine);
    }
    // 11. preStable runs before the AREA_STABLE dispatch
    {
        fresh(w);
        static std::vector<int> order; order.clear();
        w.preStable = [](const nw::World&) { order.push_back(1); };
        w.disp.add(K2NW_AREA_STABLE, [](const K2NwEvent*, void*) { order.push_back(2); }, nullptr, "t");
        w.onLoad(1); add(w, "a", g_areaA, "204tel"); ticks(w, 31);
        KT_CHECK(order == (std::vector<int>{1, 2}));
    }
    return kt::Report("nw_world_test");
}
