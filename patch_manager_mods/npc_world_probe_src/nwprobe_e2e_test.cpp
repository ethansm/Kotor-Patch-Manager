// End-to-end host test of npc-world-probe: the real nw::World (NwCore.h, the library core) behind the real k2.npcworld C table, a real
// k2nw::Client, and the real nwp::Probe, driven like the engine drives the hooks (load, creature adds, ticks). Only the engine spawn
// and the log sink are fakes. The DLL glue in NpcWorldProbe.cpp mirrors the event decoding done here (saferead reads aside).
#include "NpcWorld.h"
#include "NwCore.h"
#include "ProbeCore.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

static_assert(nwp::EV_LOAD == K2NW_AREA_LOAD && nwp::EV_ADDED == K2NW_CREATURE_ADDED && nwp::EV_STABLE == K2NW_AREA_STABLE && nwp::EV_TICK == K2NW_TICK,
              "probe event kinds must equal the library kinds");
static_assert(nwp::ORIGIN_ENGINE == K2NW_ORIGIN_ENGINE && nwp::ORIGIN_LIB == K2NW_ORIGIN_LIB, "probe origins must equal the library origins");

// ---- the library side: nw::World behind the C table -------------------------------------------------------------------------------
static nw::World g_w;
static char g_areaA[4];
static int32_t KPATCH_CALL fReg(int32_t m, K2NwCallback fn, void* u, const char* o) { return g_w.disp.add(m, fn, u, o); }
static void KPATCH_CALL fUnreg(int32_t t) { g_w.disp.remove(t); }
static uint32_t KPATCH_CALL fSalt() { return g_w.visitSalt(); }
static int32_t KPATCH_CALL fSeq() { return g_w.visitSeq(); }
static int32_t KPATCH_CALL fArea(char* b, uint32_t c) { return g_w.currentArea(b, c); }
static int32_t KPATCH_CALL fTag(const char* t) { return g_w.tagPresent(t) ? 1 : 0; }
static uint32_t KPATCH_CALL fSpawn(const char* r, const char* t, float x, float y, float z, float f) { uint32_t o = 0; g_w.spawn(r, t, x, y, z, f, &o); return o; }
static int32_t KPATCH_CALL fCount() { return g_w.spawnedCount(); }
static int32_t KPATCH_CALL fCap() { return g_w.effectiveCap(); }
static const K2NpcWorldApi g_table = { sizeof(K2NpcWorldApi), fReg, fUnreg, fSalt, fSeq, fArea, fTag, fSpawn, fCount, fCap };

static uint32_t g_nextOid = 0x7f000100;
static std::string realTag(const char* resref) {   // the tag the engine would read from the template
    if (!strcmp(resref, "c_drdmouse01")) return "MouseDroid";
    if (!strcmp(resref, "c_jawa")) return "Jawa_Tag";
    return resref;
}
static std::string g_pendingTag;
static uint32_t fakeSpawn(void*, void* area, const char* resref, float x, float y, float z, float) {
    g_pendingTag = realTag(resref);
    nw::CreatureInfo ci; ci.oid = g_nextOid; ci.tag = g_pendingTag.c_str(); ci.areaPtr = area; ci.areaRes = g_w.area(); ci.x = x; ci.y = y; ci.z = z;
    g_w.onCreatureAdded(ci);   // the native AddToArea re-enters the hook
    return g_nextOid++;
}

// ---- the probe side ----------------------------------------------------------------------------------------------------------------------
static k2nw::Client g_nw;
static nwp::Probe g_probe;
static uint32_t g_clock = 0;

struct BridgeEnv : nwp::Env {
    std::vector<std::string> lines;
    void log(nwp::Chan, int, const char* m) override { lines.push_back(m); }
    bool tagPresent(const char* t) override { return g_nw.tagPresent(t); }
    uint32_t spawn(const nwp::Row& r) override { return g_nw.spawn(r.resref, r.tag, r.x, r.y, r.z, r.facing); }
    int spawned() override { return g_nw.spawnedCount(); }
    int cap() override { return g_nw.spawnCap(); }
    uint32_t nowUs() override { return g_clock; }
    int count(const char* s) const { int n = 0; for (auto& l : lines) if (l.find(s) != std::string::npos) ++n; return n; }
    bool has(const char* s) const { return count(s) > 0; }
};
static BridgeEnv g_env;

static void KPATCH_CALL onEvent(const K2NwEvent* ev, void*) {   // same decoding as NpcWorldProbe.cpp
    nwp::Ev e;
    e.kind = ev->kind; e.loadSeq = ev->load_seq; e.visitSeq = ev->visit_seq; e.salt = ev->visit_salt; e.area = ev->area ? ev->area : "";
    if (ev->kind == K2NW_CREATURE_ADDED) {
        e.tag = ev->tag ? ev->tag : ""; e.oid = ev->oid; e.x = ev->x; e.y = ev->y; e.z = ev->z; e.origin = ev->origin;
        e.fieldsOk = true; e.f11ac = 0; e.f11fc = 0; e.areaId = 0x55;
    }
    switch (ev->kind) {
        case K2NW_AREA_LOAD: g_probe.onLoad(e); break;
        case K2NW_CREATURE_ADDED: g_probe.onAdded(e); break;
        case K2NW_AREA_STABLE: g_probe.onStable(e); break;
        case K2NW_TICK: g_probe.onTick(e); break;
    }
}

static void setup(const nwp::Config& c, bool provider = true) {
    fakekp::reset();
    g_w = nw::World(); g_w.eng.spawn = fakeSpawn; g_nextOid = 0x7f000100; g_env.lines.clear(); g_nw = k2nw::Client(); g_clock = 0;
    if (provider) fakekp::provide(K2NW_IFACE, K2NW_VERSION, &g_table);
    g_probe = nwp::Probe(); g_probe.configure(c, &g_env);
    g_nw.init(fakekp::api(), "npc-world-probe"); g_nw.add(g_probe.mask(), onEvent); g_nw.connect();
}
// One area visit as the engine does it: load, population adds (restored creatures among them), then quiet ticks until the latch.
static void visit(const std::vector<const char*>& engineTags, int quietTicks = 31) {
    g_w.onLoad(1234);
    int n = 1;
    nw::CreatureInfo ci; ci.areaPtr = g_areaA; ci.areaRes = "204tel";
    ci.oid = 0x7f000001u; ci.tag = "nwp_bystander"; g_w.onCreatureAdded(ci);   // the area is only known once a creature is added to it
    for (const char* t : engineTags) { ci.oid = 0x7f000001u + n++; ci.tag = t; g_w.onCreatureAdded(ci); }
    for (int i = 0; i < quietTicks; ++i) { g_clock += 16000; g_w.onTick(); }
}

int main() {
    // 1. default stages (t26 + persist): connect registers the probe, the stable latch triggers the roster, lib adds are seen with origin lib
    {
        nwp::Config c;
        setup(c);
        KT_CHECK(g_nw.connected); KT_CHECK_EQ(g_w.disp.count(K2NW_ALL), 1);
        visit({"npc_commoner_a", "npc_commoner_b"});
        KT_CHECK_EQ(g_w.spawnedCount(), 2); KT_CHECK_EQ(g_probe.spawnedVisit(), 2);
        KT_CHECK(g_env.has("LOAD n=1")); KT_CHECK(g_env.has("STABLE visit=1 load=1 area=204tel"));
        KT_CHECK(g_env.has("SPAWNED resref=c_drdmouse01 tag=MouseDroid oid=7f000100")); KT_CHECK(g_env.has("SPAWNED resref=k1p_native_npc tag=k1p_native_npc oid=7f000101"));
        KT_CHECK_EQ(g_probe.libAdds(), 2); KT_CHECK_EQ(g_env.count("LIBADD"), 2); KT_CHECK_EQ(g_probe.restored(), 0);
        KT_CHECK(g_w.tagPresent("MouseDroid"));   // the lib adds are tracked as present by the library
        // visit 2 after a save reload: the engine restores both spawned creatures -> nothing is spawned twice, verdict all-persisted
        g_env.lines.clear();
        visit({"npc_commoner_a", "MouseDroid", "k1p_native_npc"});
        KT_CHECK_EQ(g_probe.restored(), 2); KT_CHECK_EQ(g_env.count("RESTORED"), 2);
        KT_CHECK_EQ(g_w.spawnedCount(), 0); KT_CHECK_EQ(g_env.count("SKIP stage=t26"), 2); KT_CHECK(!g_env.has("SPAWNED"));
        KT_CHECK(g_env.has("PERSIST-CHECK area=204tel prev_spawned=2 restored_now=2 verdict=all-persisted"));
        // visit 3: the spawns did not survive -> spawn again, verdict none-persisted shows up at the next stable
        g_env.lines.clear();
        visit({"npc_commoner_a"});
        KT_CHECK_EQ(g_w.spawnedCount(), 2);
        KT_CHECK(g_env.has("PERSIST-CHECK area=204tel prev_spawned=2 restored_now=0 verdict=none-persisted"));   // the check runs before the respawn
    }
    // 2. capacity N=6 against the default library cap 4: the library refuses, the probe warns, stops, and reports
    {
        nwp::Config c; c.stages = nwp::ST_CAPACITY; c.set("capacity_n", "6"); c.set("capacity_resref", "c_jawa");
        setup(c);
        visit({"npc_commoner_a"});
        KT_CHECK_EQ(g_w.spawnedCount(), 4); KT_CHECK(g_env.has("exceeds the library spawn cap 4")); KT_CHECK(g_env.has("REFUSED resref=c_jawa tag=nwp_cap_04 "));
        KT_CHECK(g_env.has("CAPACITY requested=6 spawned=4 cap=4"));
        // library cap raised (npc_world.ini spawn_cap=18): all 18 grid cells spawn once
        nwp::Config c18; c18.stages = nwp::ST_CAPACITY; c18.set("capacity_n", "18"); c18.set("capacity_resref", "c_jawa");
        setup(c18); g_w.spawnCap = 18;
        visit({});
        KT_CHECK_EQ(g_w.spawnedCount(), 18); KT_CHECK(g_env.has("CAPACITY requested=18 spawned=18 cap=18")); KT_CHECK(!g_env.has("REFUSED"));
        // restored copies are detected through capacity_realtag on the next visit
        nwp::Config cr = c18; cr.set("capacity_realtag", "Jawa_Tag");
        setup(cr); g_w.spawnCap = 18;
        visit({"Jawa_Tag", "Jawa_Tag"});
        KT_CHECK_EQ(g_w.spawnedCount(), 0); KT_CHECK(g_env.has("already present (restored from the save)"));
    }
    // 3. frame stage: TICK is registered only when asked for; pre and post windows around the spawns
    {
        nwp::Config c; c.stages = nwp::ST_T26 | nwp::ST_FRAME; c.frameWindow = 10; c.framePreWindows = 1; c.framePostWindows = 1;
        setup(c);
        KT_CHECK_EQ(g_probe.mask(), K2NW_AREA_LOAD | K2NW_CREATURE_ADDED | K2NW_AREA_STABLE | K2NW_TICK);
        visit({"npc_commoner_a"}, 25);   // 25 quiet ticks: latch not reached yet, two pre windows would fit, budget logs one
        KT_CHECK_EQ(g_env.count("FRAME phase=pre"), 1); KT_CHECK(!g_env.has("STABLE"));
        for (int i = 0; i < 6; ++i) { g_clock += 16000; g_w.onTick(); }   // ticks 26-31: the latch fires on the 30th quiet tick
        KT_CHECK(g_env.has("STABLE")); KT_CHECK_EQ(g_w.spawnedCount(), 2);
        for (int i = 0; i < 30; ++i) { g_clock += 25000; g_w.onTick(); }
        KT_CHECK_EQ(g_env.count("FRAME phase=post spawned=2"), 1);
        nwp::Config nf; nf.stages = nwp::ST_T26;   // no frame stage: TICK never registered, so ticks cost the probe nothing
        setup(nf); KT_CHECK_EQ(g_probe.mask() & K2NW_TICK, 0); visit({});
        KT_CHECK_EQ(g_env.count("FRAME"), 0);
    }
    // 4. pool rows through the library: wrong area skipped, duplicate tag refused by the library (not by the probe), disabled provider
    {
        nwp::Config c; c.stages = nwp::ST_POOL; c.set("row1", "c_jawa,Jawa_Tag,204tel,1,2,0"); c.set("row2", "c_jawa,Jawa_Tag,204tel,3,4,0"); c.set("row3", "c_ithorian,Ith,301nar,5,6,0");
        setup(c);
        visit({});
        KT_CHECK_EQ(g_w.spawnedCount(), 1);   // row2 reuses the tag: the first spawn made it present, the probe skips it
        KT_CHECK_EQ(g_env.count("SKIP stage=pool"), 1);
        setup(c); g_w.enabled = false;
        visit({});
        KT_CHECK_EQ(g_w.spawnedCount(), 0);   // disabled library: no events, nothing happens, no crash
    }
    // 5. no provider (npc-world missing): the probe stays silent and inert
    {
        nwp::Config c;
        setup(c, false);
        KT_CHECK(!g_nw.connected);
        visit({"MouseDroid"});
        KT_CHECK(g_env.lines.empty()); KT_CHECK_EQ(g_probe.spawnedVisit(), 0);
    }
    return kt::Report("nwprobe_e2e_test");
}
