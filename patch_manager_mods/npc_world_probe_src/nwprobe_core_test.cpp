// Host test of the npc-world-probe pure logic (ProbeCore.h): stage list and ini row parsing, Config::set, the capacity grid, frame
// statistics and every Probe stage against a fake Env. The wiring to the real library is nwprobe_e2e_test.cpp. No engine, no Win32.
#include "ProbeCore.h"
#include "check.h"
#include <string.h>
#include <string>
#include <vector>

using namespace nwp;

struct FakeEnv : Env {
    std::vector<std::string> lines;
    std::vector<int> levels;
    std::vector<std::string> present;     // tags the library reports as present
    std::vector<Row> spawned_rows;
    int capValue = 4, refuseAfter = 1000;
    uint32_t clock = 0;
    uint32_t nextOid = 0x7f000100;
    void log(Chan, int level, const char* msg) override { lines.push_back(msg); levels.push_back(level); }
    bool tagPresent(const char* tag) override { for (auto& p : present) if (ciEq(p.c_str(), tag)) return true; return false; }
    uint32_t spawn(const Row& r) override {
        if (static_cast<int>(spawned_rows.size()) >= refuseAfter || static_cast<int>(spawned_rows.size()) >= capValue) return 0;
        spawned_rows.push_back(r);
        return nextOid++;
    }
    int spawned() override { return static_cast<int>(spawned_rows.size()); }
    int cap() override { return capValue; }
    uint32_t nowUs() override { return clock; }
    int count(const char* sub) const { int n = 0; for (auto& l : lines) if (l.find(sub) != std::string::npos) ++n; return n; }
    bool has(const char* sub) const { return count(sub) > 0; }
};

static Ev ev(int kind) { Ev e; e.kind = kind; e.area = "204tel"; e.loadSeq = 1; e.visitSeq = 1; e.salt = 0xABCD1234u; return e; }
static Ev added(const char* tag, int origin, uint32_t oid = 0x7f000001u) {
    Ev e = ev(EV_ADDED); e.tag = tag; e.origin = origin; e.oid = oid; e.fieldsOk = true; e.f11ac = 0x11; e.f11fc = 0x22; e.areaId = 0x33; return e;
}

int main() {
    // 1. stage list
    {
        int unk = -1;
        KT_CHECK_EQ(parseStages("", &unk), DefaultStages); KT_CHECK_EQ(unk, 0);
        KT_CHECK_EQ(parseStages(nullptr), DefaultStages);
        KT_CHECK_EQ(parseStages("   "), DefaultStages);
        KT_CHECK_EQ(parseStages("T26"), ST_T26);
        KT_CHECK_EQ(parseStages("t26, Persist|frame;capacity pool"), ST_T26 | ST_PERSIST | ST_FRAME | ST_CAPACITY | ST_POOL);
        KT_CHECK_EQ(parseStages("all"), ST_T26 | ST_POOL | ST_CAPACITY | ST_PERSIST | ST_FRAME);
        KT_CHECK_EQ(parseStages("none"), 0u);                         // explicit empty list is not the default
        KT_CHECK_EQ(parseStages("t26,bogus,zzz", &unk), ST_T26); KT_CHECK_EQ(unk, 2);
        KT_CHECK_EQ(parseStages("bogus", &unk), 0u); KT_CHECK_EQ(unk, 1);   // only unknown tokens: nothing enabled, not the default
        KT_CHECK_EQ(parseStages("averyveryverylongtokenname,t26"), ST_T26);   // long unknown token is cut, not a buffer overflow
    }
    // 2. rows
    {
        Row r;
        KT_CHECK(parseRow("c_drdmouse01, MouseDroid ,204tel,-39.0,-63.1,0,1.5", &r));
        KT_CHECK_EQ(r.resref, "c_drdmouse01"); KT_CHECK_EQ(r.tag, "MouseDroid"); KT_CHECK_EQ(r.area, "204tel");
        KT_CHECK(r.x == -39.0f && r.y == -63.1f && r.z == 0.0f && r.facing == 1.5f);
        KT_CHECK(parseRow("a,b,c,1,2,3", &r)); KT_CHECK(r.facing == 0.0f);   // facing optional
        KT_CHECK(!parseRow("a,b,c,1,2", &r));                                  // too few
        KT_CHECK(!parseRow("a,b,c,1,2,3,4,5", &r));                            // too many
        KT_CHECK(!parseRow(",b,c,1,2,3", &r));                                 // empty resref
        KT_CHECK(!parseRow("a,,c,1,2,3", &r));
        KT_CHECK(!parseRow("a,b,,1,2,3", &r));
        KT_CHECK(!parseRow("a,b,c,x,2,3", &r));                                // not a number
        KT_CHECK(!parseRow("a,b,c,1,2,3z", &r));                               // trailing junk
        KT_CHECK(!parseRow("a,b,c,nan,2,3", &r));
        KT_CHECK(!parseRow("a,b,c,inf,2,3", &r));
        KT_CHECK(!parseRow("abcdefghijklmnopq,b,c,1,2,3", &r));                // resref 17 chars
        KT_CHECK(parseRow("abcdefghijklmnop,b,c,1,2,3", &r));                  // resref 16 chars
        KT_CHECK(!parseRow("a,123456789012345678901234567890123,c,1,2,3", &r)); // tag 33 chars
        KT_CHECK(parseRow("a,12345678901234567890123456789012,c,1,2,3", &r));   // tag 32 chars
        KT_CHECK(!parseRow("a,b,12345678901234567,1,2,3", &r));                // area 17 chars
        KT_CHECK(!parseRow(nullptr, &r)); KT_CHECK(!parseRow("a,b,c,1,2,3", nullptr));
        Row keep; copyStr(keep.resref, sizeof keep.resref, "keep");
        KT_CHECK(!parseRow("bad", &keep)); KT_CHECK_EQ(keep.resref, "keep");   // a failed parse leaves the target alone
    }
    // 3. config keys
    {
        Config c;
        KT_CHECK_EQ(c.stages, DefaultStages); KT_CHECK_EQ(c.capN, 0); KT_CHECK_EQ(c.nPool, 0);
        KT_CHECK(c.set("stages", "t26,capacity,frame")); KT_CHECK_EQ(c.stages, ST_T26 | ST_CAPACITY | ST_FRAME);
        KT_CHECK(!c.set("stages", "t26,oops")); KT_CHECK_EQ(c.badKeys, 1);       // unknown stage still applies the known ones
        KT_CHECK(c.set("CAPACITY_N", "18")); KT_CHECK_EQ(c.capN, 18);             // keys are case-insensitive
        KT_CHECK(!c.set("capacity_n", "33")); KT_CHECK_EQ(c.capN, 18);            // above the library hard cap
        KT_CHECK(!c.set("capacity_n", "-1")); KT_CHECK(!c.set("capacity_n", "x")); KT_CHECK(!c.set("capacity_n", "5x"));
        KT_CHECK(c.set("capacity_resref", "c_jawa")); KT_CHECK_EQ(c.capResref, "c_jawa");
        KT_CHECK(!c.set("capacity_resref", "")); KT_CHECK_EQ(c.capResref, "c_jawa");
        KT_CHECK(!c.set("capacity_resref", "abcdefghijklmnopq"));
        KT_CHECK(c.set("capacity_realtag", "Jawa_Tag")); KT_CHECK_EQ(c.capRealTag, "Jawa_Tag");
        KT_CHECK(c.set("capacity_realtag", "")); KT_CHECK_EQ(c.capRealTag, "");
        KT_CHECK(c.set("capacity_x", "1.5") && c.set("capacity_y", "-2") && c.set("capacity_z", "0.25") && c.set("capacity_step", "1.2"));
        KT_CHECK(c.capX == 1.5f && c.capY == -2.0f && c.capZ == 0.25f && c.capStep == 1.2f);
        KT_CHECK(c.set("capacity_cols", "3")); KT_CHECK_EQ(c.capCols, 3); KT_CHECK(!c.set("capacity_cols", "0"));
        KT_CHECK(c.set("frame_window", "60")); KT_CHECK_EQ(c.frameWindow, 60); KT_CHECK(!c.set("frame_window", "1"));
        KT_CHECK(c.set("frame_pre_windows", "2") && c.set("frame_post_windows", "5")); KT_CHECK_EQ(c.framePreWindows, 2); KT_CHECK_EQ(c.framePostWindows, 5);
        KT_CHECK(c.set("row1", "c_jawa,JawaA,204tel,1,2,3")); KT_CHECK(c.set("Row2", "c_ithorian,IthA,204tel,4,5,6,0.5")); KT_CHECK_EQ(c.nPool, 2);
        KT_CHECK(!c.set("row3", "garbage")); KT_CHECK_EQ(c.nPool, 2);
        KT_CHECK(!c.set("unknown_key", "1")); KT_CHECK(!c.set("row", "a,b,c,1,2,3")); KT_CHECK_EQ(c.nPool, 2);   // "row" alone is not a row key
        KT_CHECK(!c.set(nullptr, "1")); KT_CHECK(!c.set("stages", nullptr));
        for (int i = 3; i <= MaxPool + 2; ++i) c.set("row9", "a,b,c,1,2,3");
        KT_CHECK_EQ(c.nPool, MaxPool);                                            // table full: refused, not overflowed
        KT_CHECK(!c.set("row9", "a,b,c,1,2,3"));
    }
    // 4. capacity grid
    {
        Config c; c.capResref[0] = 0; copyStr(c.capResref, sizeof c.capResref, "c_jawa");
        c.capX = 10; c.capY = 20; c.capZ = 0; c.capStep = 2; c.capCols = 3;
        Row r0 = capacityRow(c, 0), r4 = capacityRow(c, 4), r17 = capacityRow(c, 17);
        KT_CHECK_EQ(r0.resref, "c_jawa"); KT_CHECK_EQ(r0.tag, "nwp_cap_00"); KT_CHECK_EQ(r0.area, "204tel");
        KT_CHECK(r0.x == 10 && r0.y == 20);
        KT_CHECK(r4.x == 12 && r4.y == 22);                                       // i=4: col 1, row 1
        KT_CHECK(r17.x == 14 && r17.y == 30); KT_CHECK_EQ(r17.tag, "nwp_cap_17");
        std::string seen;
        bool unique = true;
        for (int i = 0; i < MaxCapacity; ++i) { Row r = capacityRow(c, i); if (seen.find(std::string("|") + r.tag + "|") != std::string::npos) unique = false; seen += std::string("|") + r.tag + "|"; }
        KT_CHECK(unique);                                                         // each grid cell has its own duplicate key
    }
    // 5. frame statistics
    {
        FrameStats f;
        KT_CHECK_EQ(f.avgUs(), 0u);
        f.push(16000); f.push(33333); f.push(50000); f.push(8000);
        KT_CHECK_EQ(f.n, 4); KT_CHECK_EQ(f.maxUs, 50000u); KT_CHECK_EQ(f.ge33, 2); KT_CHECK_EQ(f.ge50, 1); KT_CHECK_EQ(f.avgUs(), 26833u);
        f.clear(); KT_CHECK_EQ(f.n, 0); KT_CHECK_EQ(f.maxUs, 0u);
    }
    // 6. t26 stage: spawn once per stable, skip present tags, only the matching area
    {
        FakeEnv env; Probe p; Config c; c.stages = ST_T26; p.configure(c, &env);
        p.onLoad(ev(EV_LOAD));
        p.onStable(ev(EV_STABLE));
        KT_CHECK_EQ(env.spawned(), 2); KT_CHECK_EQ(p.spawnedVisit(), 2);
        KT_CHECK_EQ(env.spawned_rows[0].resref, "c_drdmouse01"); KT_CHECK_EQ(env.spawned_rows[1].resref, "k1p_native_npc");
        KT_CHECK(env.spawned_rows[0].x == -39.0f && env.spawned_rows[0].y == -63.1f && env.spawned_rows[1].x == -21.1f);
        KT_CHECK(env.has("SPAWNED resref=c_drdmouse01")); KT_CHECK(env.has("SPAWNED resref=k1p_native_npc"));
        FakeEnv env2; Probe p2; env2.present = {"mousedroid"}; p2.configure(c, &env2);   // case-insensitive presence
        p2.onLoad(ev(EV_LOAD)); p2.onStable(ev(EV_STABLE));
        KT_CHECK_EQ(env2.spawned(), 1); KT_CHECK(env2.has("SKIP stage=t26 resref=c_drdmouse01"));
        FakeEnv env3; Probe p3; p3.configure(c, &env3);
        Ev other = ev(EV_STABLE); other.area = "301nar";
        p3.onLoad(ev(EV_LOAD)); p3.onStable(other);
        KT_CHECK_EQ(env3.spawned(), 0);                                           // wrong area: nothing, and no crash
    }
    // 7. stages off = no spawns; none = fully inert except logging
    {
        FakeEnv env; Probe p; Config c; c.stages = 0; p.configure(c, &env);
        KT_CHECK_EQ(p.mask(), EV_LOAD | EV_ADDED | EV_STABLE);
        p.onLoad(ev(EV_LOAD)); p.onStable(ev(EV_STABLE)); p.onAdded(added("MouseDroid", ORIGIN_ENGINE));
        KT_CHECK_EQ(env.spawned(), 0); KT_CHECK_EQ(p.restored(), 0); KT_CHECK(!env.has("RESTORED"));
        Config f; f.stages = ST_FRAME; Probe pf; pf.configure(f, &env); KT_CHECK_EQ(pf.mask(), EV_LOAD | EV_ADDED | EV_STABLE | EV_TICK);
    }
    // 8. pool rows (area filter) and the watch list
    {
        FakeEnv env; Probe p; Config c; c.stages = ST_POOL | ST_PERSIST;
        c.set("row1", "c_jawa,JawaA,204tel,1,2,0"); c.set("row2", "c_ithorian,IthA,301nar,4,5,0");
        p.configure(c, &env);
        KT_CHECK_EQ(p.nWatch(), 2);
        p.onLoad(ev(EV_LOAD)); p.onStable(ev(EV_STABLE));
        KT_CHECK_EQ(env.spawned(), 1); KT_CHECK_EQ(env.spawned_rows[0].tag, "JawaA");
        Config d; d.stages = ST_T26 | ST_POOL | ST_CAPACITY; d.set("row1", "c_jawa,MouseDroid,204tel,1,2,0"); d.set("capacity_realtag", "CapTag");
        Probe pd; pd.configure(d, &env); KT_CHECK_EQ(pd.nWatch(), 3);              // t26 has 2, pool dup of MouseDroid dedupes, + CapTag
    }
    // 9. persist: lib adds logged with fields, restored engine copies counted, previous-visit verdict
    {
        FakeEnv env; Probe p; Config c; c.stages = ST_T26 | ST_PERSIST; p.configure(c, &env);
        p.onLoad(ev(EV_LOAD)); p.onAdded(added("Bystander", ORIGIN_ENGINE)); p.onStable(ev(EV_STABLE));   // visit 1: nothing restored
        KT_CHECK_EQ(p.engineAdds(), 1); KT_CHECK_EQ(p.restored(), 0); KT_CHECK_EQ(p.spawnedVisit(), 2); KT_CHECK_EQ(p.prevSpawned(), 2);
        KT_CHECK(!env.has("PERSIST-CHECK"));                                       // first visit has no previous verdict
        p.onAdded(added("MouseDroid", ORIGIN_LIB, 0x7f000100u));
        KT_CHECK_EQ(p.libAdds(), 1); KT_CHECK(env.has("LIBADD oid=7f000100 tag=\"MouseDroid\"")); KT_CHECK(env.has("f11ac=00000011 f11fc=00000022 areaId=00000033"));
        // visit 2, save reloaded: the engine restored both
        env.spawned_rows.clear();
        p.onLoad(ev(EV_LOAD));
        KT_CHECK_EQ(p.restored(), 0); KT_CHECK_EQ(p.spawnedVisit(), 0); KT_CHECK_EQ(p.libAdds(), 0);   // per-visit counters reset
        p.onAdded(added("mousedroid", ORIGIN_ENGINE)); p.onAdded(added("K1P_NATIVE_NPC", ORIGIN_ENGINE));
        KT_CHECK_EQ(p.restored(), 2); KT_CHECK_EQ(env.count("RESTORED oid="), 2);
        env.present = {"MouseDroid", "k1p_native_npc"};
        p.onStable(ev(EV_STABLE));
        KT_CHECK(env.has("PERSIST-CHECK area=204tel prev_spawned=2 restored_now=2 verdict=all-persisted"));
        KT_CHECK_EQ(env.spawned(), 0);                                             // nothing doubled
        // visit 3: nothing restored, spawn again
        env.present.clear(); env.lines.clear();
        p.onLoad(ev(EV_LOAD)); p.onStable(ev(EV_STABLE));
        KT_CHECK(env.has("verdict=none-persisted")); KT_CHECK_EQ(env.spawned(), 2);
        // visit 4: partial
        env.spawned_rows.clear(); env.present = {"MouseDroid"}; env.lines.clear();
        p.onLoad(ev(EV_LOAD)); p.onAdded(added("MouseDroid", ORIGIN_ENGINE)); p.onStable(ev(EV_STABLE));
        KT_CHECK(env.has("restored_now=1 verdict=partial"));
        // a lib add whose fields were unreadable still logs, without invented values
        FakeEnv env2; Probe p2; p2.configure(c, &env2);
        Ev u = added("MouseDroid", ORIGIN_LIB); u.fieldsOk = false; p2.onAdded(u);
        KT_CHECK(env2.has("fields unreadable")); KT_CHECK(!env2.has("f11ac"));
        // persist stage off: no persist output at all
        FakeEnv env3; Probe p3; Config q; q.stages = ST_T26; p3.configure(q, &env3);
        p3.onAdded(added("MouseDroid", ORIGIN_ENGINE)); p3.onAdded(added("MouseDroid", ORIGIN_LIB));
        KT_CHECK(!env3.has("RESTORED")); KT_CHECK(!env3.has("LIBADD")); KT_CHECK_EQ(p3.restored(), 0);
    }
    // 10. capacity: N rows on the grid, cap warning, stop at the first refusal, realtag skip, area check
    {
        Config c; c.stages = ST_CAPACITY; c.set("capacity_n", "6"); c.set("capacity_resref", "c_jawa");
        FakeEnv env; env.capValue = 32; Probe p; p.configure(c, &env);
        p.onLoad(ev(EV_LOAD)); p.onStable(ev(EV_STABLE));
        KT_CHECK_EQ(env.spawned(), 6); KT_CHECK(env.has("CAPACITY requested=6 spawned=6 cap=32")); KT_CHECK(!env.has("raise npc_world.ini"));
        KT_CHECK_EQ(env.spawned_rows[5].tag, "nwp_cap_05"); KT_CHECK_EQ(env.spawned_rows[0].resref, "c_jawa");
        FakeEnv e2; e2.capValue = 4; Probe p2; Config c18 = c; c18.set("capacity_n", "18"); p2.configure(c18, &e2);
        p2.onLoad(ev(EV_LOAD)); p2.onStable(ev(EV_STABLE));
        KT_CHECK_EQ(e2.spawned(), 4); KT_CHECK(e2.has("exceeds the library spawn cap 4: raise npc_world.ini")); KT_CHECK(e2.has("REFUSED resref=c_jawa tag=nwp_cap_04"));
        KT_CHECK(e2.has("CAPACITY requested=18 spawned=4 cap=4"));
        bool warn = false; for (size_t i = 0; i < e2.lines.size(); ++i) if (e2.levels[i] == 1) warn = true;
        KT_CHECK(warn);                                                            // cap warning and refusal are WARN level
        FakeEnv e3; e3.present = {"Jawa_Tag"}; Probe p3; Config cr = c; cr.set("capacity_realtag", "Jawa_Tag"); p3.configure(cr, &e3);
        p3.onLoad(ev(EV_LOAD)); p3.onStable(ev(EV_STABLE));
        KT_CHECK_EQ(e3.spawned(), 0); KT_CHECK(e3.has("already present (restored from the save)"));
        FakeEnv e4; Probe p4; p4.configure(c, &e4); Ev wrong = ev(EV_STABLE); wrong.area = "301nar";
        p4.onLoad(ev(EV_LOAD)); p4.onStable(wrong);
        KT_CHECK_EQ(e4.spawned(), 0); KT_CHECK(e4.has("CAPACITY skipped: area 301nar"));
        FakeEnv e5; Probe p5; Config c0; c0.stages = ST_CAPACITY; p5.configure(c0, &e5); p5.onLoad(ev(EV_LOAD)); p5.onStable(ev(EV_STABLE));
        KT_CHECK_EQ(e5.spawned(), 0); KT_CHECK(e5.has("CAPACITY nothing to do"));   // capacity_n = 0
    }
    // 11. frame windows: pre/post phase, budgets, load-stall gap ignored, uint32 wrap
    {
        Config c; c.stages = ST_T26 | ST_FRAME; c.frameWindow = 10; c.framePreWindows = 2; c.framePostWindows = 1;
        FakeEnv env; Probe p; p.configure(c, &env);
        p.onLoad(ev(EV_LOAD));
        auto tick = [&](uint32_t step) { env.clock += step; p.onTick(ev(EV_TICK)); };
        env.clock = 1000;
        for (int i = 0; i < 11; ++i) tick(16000);          // the first tick only primes the clock: 10 intervals = 1 window
        KT_CHECK_EQ(env.count("FRAME phase=pre"), 1); KT_CHECK(env.has("avg=16.00ms max=16.00ms ge33=0 ge50=0"));
        for (int i = 0; i < 40; ++i) tick(16000);
        KT_CHECK_EQ(env.count("FRAME phase=pre"), 2);       // budget of 2 pre windows
        p.onStable(ev(EV_STABLE)); KT_CHECK(p.post());
        for (int i = 0; i < 10; ++i) tick(40000);
        KT_CHECK_EQ(env.count("FRAME phase=post spawned=2"), 1); KT_CHECK(env.has("avg=40.00ms max=40.00ms ge33=10 ge50=0"));
        for (int i = 0; i < 30; ++i) tick(40000);
        KT_CHECK_EQ(env.count("FRAME phase=post"), 1);      // budget of 1 post window
        // a gap above 2 s (a load) is not frame time and does not poison the window
        FakeEnv e2; Probe p2; Config c2 = c; c2.framePreWindows = 5; p2.configure(c2, &e2);
        p2.onLoad(ev(EV_LOAD)); e2.clock = 5; p2.onTick(ev(EV_TICK));
        for (int i = 0; i < 4; ++i) { e2.clock += 16000; p2.onTick(ev(EV_TICK)); }
        e2.clock += 3000000; p2.onTick(ev(EV_TICK));        // 3 s stall
        for (int i = 0; i < 6; ++i) { e2.clock += 16000; p2.onTick(ev(EV_TICK)); }
        KT_CHECK_EQ(e2.count("FRAME"), 1); KT_CHECK(e2.has("max=16.00ms"));
        // a new load re-primes: the first tick after a load never produces an interval across the load
        FakeEnv e3; Probe p3; p3.configure(c2, &e3);
        p3.onLoad(ev(EV_LOAD)); e3.clock = 100; p3.onTick(ev(EV_TICK)); e3.clock += 16000; p3.onTick(ev(EV_TICK));
        p3.onLoad(ev(EV_LOAD)); e3.clock += 1500000; p3.onTick(ev(EV_TICK));
        for (int i = 0; i < 10; ++i) { e3.clock += 16000; p3.onTick(ev(EV_TICK)); }
        KT_CHECK_EQ(e3.count("FRAME"), 1); KT_CHECK(e3.has("max=16.00ms"));
        // uint32 microsecond clock wrap
        FakeEnv e4; Probe p4; p4.configure(c2, &e4); p4.onLoad(ev(EV_LOAD));
        e4.clock = 0xFFFFFFFFu - 40000u; p4.onTick(ev(EV_TICK));
        for (int i = 0; i < 10; ++i) { e4.clock += 16000; p4.onTick(ev(EV_TICK)); }
        KT_CHECK_EQ(e4.count("FRAME"), 1); KT_CHECK(e4.has("max=16.00ms"));
        // frame stage off: ticks do nothing
        FakeEnv e5; Probe p5; Config cn; cn.stages = ST_T26; p5.configure(cn, &e5);
        for (int i = 0; i < 50; ++i) { e5.clock += 1000; p5.onTick(ev(EV_TICK)); }
        KT_CHECK(e5.lines.empty());
    }
    // 12. every log line fits the KpLog cut (232 bytes) even with maximum-length fields
    {
        FakeEnv env; Probe p; Config c; c.stages = ST_POOL | ST_PERSIST | ST_CAPACITY;
        c.set("row1", "abcdefghijklmnop,12345678901234567890123456789012,abcdefghijklmnop,-99999.9,-99999.9,-99999.9,3.1");
        c.set("capacity_n", "2"); c.set("capacity_resref", "abcdefghijklmnop"); c.set("capacity_area", "abcdefghijklmnop");
        p.configure(c, &env);
        Ev l = ev(EV_LOAD); l.area = "abcdefghijklmnop"; p.onLoad(l);
        Ev a = added("12345678901234567890123456789012", ORIGIN_ENGINE, 0xFFFFFFFFu); a.area = "abcdefghijklmnop"; a.x = a.y = a.z = -99999.9f; a.f11ac = a.f11fc = a.areaId = 0xFFFFFFFFu;
        p.onAdded(a);
        Ev s = ev(EV_STABLE); s.area = "abcdefghijklmnop"; s.visitSeq = 99999; s.loadSeq = 99999; p.onStable(s);
        Ev la = a; la.origin = ORIGIN_LIB; p.onAdded(la);
        size_t longest = 0; for (auto& ln : env.lines) if (ln.size() > longest) longest = ln.size();
        KT_CHECK(env.lines.size() >= 5); KT_CHECK(longest < 232); KT_CHECK(longest > 100);
    }
    // 13. null env: configure/handlers never crash
    {
        Probe p; Config c; c.stages = ST_T26 | ST_FRAME | ST_PERSIST | ST_CAPACITY | ST_POOL; p.configure(c, nullptr);
        p.onLoad(ev(EV_LOAD)); p.onAdded(added("x", ORIGIN_ENGINE)); p.onAdded(added("x", ORIGIN_LIB)); p.onStable(ev(EV_STABLE)); p.onTick(ev(EV_TICK));
        KT_CHECK_EQ(p.spawnedVisit(), 0);
    }
    return kt::Report("nwprobe_core_test");
}
