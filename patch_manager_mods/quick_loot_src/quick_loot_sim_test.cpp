// quick_loot_sim_test.cpp -- frame-by-frame host simulation of quick-loot's peek / take state machine (QuickLootMachine.h, doc 52 PV1 step 3).
// A fake world (leader, area list, containers, engine side effects with scripted Show / OnOpen timing) implements qlogic::Env and records
// the action stream (OpenInventory, SendClose, ServerClose, label, toast, log). Scenarios a..i drive it over many 16 ms frames and assert
// the stream; invariant j (every accepted OpenInventory is followed by exactly one Close before the next one, never two Closes for one
// open) is checked live and at the end of every scenario.
#include "QuickLootMachine.h"
#include "check.h"
#include <stdlib.h>
#include <string>
#include <vector>

using namespace qlogic;

namespace {

enum Kind { A_OPEN, A_SEND, A_SRV, A_LSHOW, A_LHIDE, A_TOAST, A_LOG };
struct Act { Kind k; uintptr_t plc; uint32_t obj; int flag; std::string text; uint32_t frame; };
struct Item { std::string name; int stack; };
struct Obj {
    uint32_t sid = 0;
    uintptr_t plc = 0;
    bool isPlc = true, present = true;
    float x = 0, y = 0;
    bool hasInv = true, locked = false, trap = false, useable = true;
    int open = 0;
    std::vector<Item> items, loot;   // loot = what the OnOpen script adds (once)
    int showDelay = 3;               // frames from an accepted OpenInventory to its ShowContainerGui; 0 = synchronous, -1 = never
    int onOpenDelay = 5;             // frames from open to the OnOpen event (0x16); -1 = never
    int autoCloseDelay = -1;         // frames after open the container's open flag drops by itself (-1 never)
    bool refuse = false;             // the engine refuses OpenInventory
    bool outstanding = false;        // opened and not yet closed
    bool excused = false;            // container gone / abandoned: no Close is expected
};
struct Ev { uint32_t frame; int kind; int idx; };   // kind 0 show, 1 OnOpen, 2 auto close

constexpr uint32_t ObjBase = 0x7F000000u;

struct Sim : Env {
    Machine m;
    uint32_t now = 100000, frame = 5000;
    // world
    bool worldOk = true, haveSrv = true, menu = false, dialog = false, paused = false;
    int combat = 0;
    uintptr_t area = 0xA0000;
    float lx = 0, ly = 0;
    bool key = false, hud = true, shift = false, msgOk = true;
    std::vector<Obj> objs;
    std::vector<Ev> evs;
    std::vector<Act> acts;
    std::vector<std::string> bad;
    std::vector<std::vector<Item>> taken;
    int vanilla = 0;
    std::string label;   // last label text shown ("" = hidden)

    int add(float x, float y, std::vector<Item> items = {}) {
        Obj o;
        const int i = static_cast<int>(objs.size());
        o.sid = 0x1000u + static_cast<unsigned>(i);
        o.plc = 0x50000u + static_cast<unsigned>(i) * 0x400u;
        o.x = x; o.y = y; o.items = items;
        objs.push_back(o);
        return i;
    }
    uint32_t objId(int i) const { return ObjBase | objs[static_cast<size_t>(i)].sid; }
    int byPlc(uintptr_t p) const { for (size_t i = 0; i < objs.size(); ++i) if (objs[i].plc == p && objs[i].present) return static_cast<int>(i); return -1; }
    int bySid(uint32_t s) const { for (size_t i = 0; i < objs.size(); ++i) if (objs[i].sid == s && objs[i].present) return static_cast<int>(i); return -1; }
    int byObj(uint32_t o) const { return bySid(o & 0xFFFFFFu); }

    void rec(Kind k, uintptr_t plc, uint32_t obj, int flag, const char* text) { acts.push_back({k, plc, obj, flag, text ? text : "", frame}); }
    void violate(const std::string& s) { bad.push_back(s); }

    // ---- driver
    void deliverShow(int idx, int flagOverride = -1) {
        Obj& o = objs[static_cast<size_t>(idx)];
        const int n = static_cast<int>(o.items.size());
        const int flag = flagOverride >= 0 ? flagOverride : (n > 0 ? 1 : 0);
        if (m.onShow(*this, objId(idx), o.plc, flag, n, now) == 0) ++vanilla;
    }
    void deliverOnOpen(int idx) {
        Obj& o = objs[static_cast<size_t>(idx)];
        for (const Item& it : o.loot) o.items.push_back(it);
        o.loot.clear();
        if (m.wantsEvent(o.plc)) m.onEvent(o.plc, EvtOnOpenLoot);
    }
    void step() {
        now += 16; ++frame;
        for (size_t i = 0; i < evs.size();) {
            if (evs[i].frame <= frame) {
                const Ev e = evs[i];
                evs.erase(evs.begin() + static_cast<long>(i));
                if (!objs[static_cast<size_t>(e.idx)].present) continue;
                if (e.kind == 0) deliverShow(e.idx);
                else if (e.kind == 1) deliverOnOpen(e.idx);
                else objs[static_cast<size_t>(e.idx)].open = 0;
                i = 0;   // handlers may queue events
                continue;
            }
            ++i;
        }
        m.onFrame(*this, frame, now);
    }
    void run(int n) { for (int i = 0; i < n; ++i) step(); }
    void setLeader(float x, float y = 0) { lx = x; ly = y; }
    void remove(int idx) { objs[static_cast<size_t>(idx)].present = false; objs[static_cast<size_t>(idx)].excused = true; }
    // The player clicks a container (engine opens it, vanilla Show). Show is delivered now; the OnOpen follows after onOpenDelay frames.
    void click(int idx) {
        Obj& o = objs[static_cast<size_t>(idx)];
        o.open = 1; o.outstanding = true;
        deliverShow(idx);
        if (o.onOpenDelay >= 0) evs.push_back({frame + static_cast<uint32_t>(o.onOpenDelay), 1, idx});
    }

    // ---- counters / queries
    int count(Kind k, size_t from = 0) const { int n = 0; for (size_t i = from; i < acts.size(); ++i) if (acts[i].k == k) ++n; return n; }
    size_t mark() const { return acts.size(); }
    const Act* nth(Kind k, int n, size_t from = 0) const {
        for (size_t i = from; i < acts.size(); ++i) if (acts[i].k == k && n-- == 0) return &acts[i];
        return nullptr;
    }
    bool logged(const char* sub, size_t from = 0) const {
        for (size_t i = from; i < acts.size(); ++i) if (acts[i].k == A_LOG && acts[i].text.find(sub) != std::string::npos) return true;
        return false;
    }
    int logCount(const char* sub, size_t from = 0) const {
        int n = 0;
        for (size_t i = from; i < acts.size(); ++i) if (acts[i].k == A_LOG && acts[i].text.find(sub) != std::string::npos) ++n;
        return n;
    }
    // Invariant j at the end of a scenario: nothing left open (except gone / abandoned containers), nothing violated live.
    void finishCheck(const char* scn) {
        for (size_t i = 0; i < objs.size(); ++i)   // a peek still up at the end legitimately holds its container open
            if (objs[i].outstanding && !objs[i].excused && !(m.peek.busy() && m.peek.plc == objs[i].plc)) violate("unclosed open at end: obj " + std::to_string(i));
        if (getenv("QL_SIM_DUMP"))
            for (const Act& a : acts) printf("[%s] f=%u k=%d plc=%x obj=%x flag=%d %s\n", scn, a.frame, static_cast<int>(a.k), static_cast<unsigned>(a.plc), a.obj, a.flag, a.text.c_str());
        std::string all;
        for (const std::string& b : bad) all += b + "; ";
        kt::Check((std::string(scn) + ": invariant j (one Close per open, none before the next open)").c_str(), bad.empty(), all.c_str());
    }

    // ---- Env: reads
    void enter() override {}
    bool readWorld(World& w) override {
        w = World();
        w.haveSrv = haveSrv;
        if (!worldOk) return false;
        w.ctrl = 0x7F0000FFu; w.area = area; w.x = lx; w.y = ly; w.ok = true;
        return true;
    }
    Sup suppress(bool ok) override { return suppressReason(ok, menu, dialog, paused, combat); }
    bool keyDown() override { return key; }
    bool hudOk() override { return hud; }
    bool shiftDown() override { return shift; }
    uintptr_t resolvePlc(uint32_t sid) override { const int i = bySid(sid); return i < 0 ? 0 : objs[static_cast<size_t>(i)].plc; }
    bool plcPos(uintptr_t plc, float* x, float* y) override {
        const int i = byPlc(plc);
        if (i < 0) return false;
        *x = objs[static_cast<size_t>(i)].x; *y = objs[static_cast<size_t>(i)].y;
        return true;
    }
    int itemCount(uintptr_t plc) override { const int i = byPlc(plc); return i < 0 ? -1 : static_cast<int>(objs[static_cast<size_t>(i)].items.size()); }
    int openFlag(uintptr_t plc) override { const int i = byPlc(plc); return i < 0 ? -1 : objs[static_cast<size_t>(i)].open; }
    uintptr_t msgObject() override { return msgOk ? 0xABC0u : 0u; }
    void capture(uintptr_t plc, Captured& c) override {
        c.count = 0; c.total = 0;
        const int i = byPlc(plc);
        if (i < 0) return;
        const std::vector<Item>& it = objs[static_cast<size_t>(i)].items;
        c.total = static_cast<unsigned>(it.size());
        for (size_t k = 0; k < it.size() && k < ToastMaxItems; ++k) {
            c.ids[k] = 0x9000u + static_cast<unsigned>(k);
            c.stacks[k] = it[k].stack;
            snprintf(c.names[k], NameCap, "%s", it[k].name.c_str());
            ++c.count;
        }
    }
    bool areaList(const World&, int* count) override { *count = static_cast<int>(objs.size()); return true; }
    bool areaRawCount(const World&, int* count) override { *count = static_cast<int>(objs.size()); return true; }
    bool areaObject(const World&, int i, uint32_t* id, float* x, float* y) override {
        const Obj& o = objs[static_cast<size_t>(i)];
        if (!o.present || !o.isPlc) return false;
        *id = o.sid; *x = o.x; *y = o.y;
        return true;
    }
    bool candFlags(uint32_t id, uintptr_t* plc, CandFlags* f) override {
        const int i = bySid(id);
        if (i < 0) return false;
        const Obj& o = objs[static_cast<size_t>(i)];
        *plc = o.plc;
        f->hasInv = o.hasInv; f->locked = o.locked; f->trap = o.trap; f->useable = o.useable; f->open = o.open != 0;
        return true;
    }
    // ---- Env: actions
    void openInventory(uintptr_t plc, uint32_t opener) override {
        (void)opener;
        rec(A_OPEN, plc, 0, 1, "");
        const int i = byPlc(plc);
        if (i < 0) { violate("OpenInventory on a missing container"); return; }
        Obj& o = objs[static_cast<size_t>(i)];
        if (o.refuse || o.open || o.locked || !o.hasInv) return;   // engine gates failed: nothing opened
        for (size_t k = 0; k < objs.size(); ++k)
            if (objs[k].outstanding && !objs[k].excused) violate("OpenInventory while obj " + std::to_string(k) + " is still open");
        o.open = 1; o.outstanding = true;
        if (o.onOpenDelay >= 0) evs.push_back({frame + static_cast<uint32_t>(o.onOpenDelay), 1, i});
        if (o.autoCloseDelay >= 0) evs.push_back({frame + static_cast<uint32_t>(o.autoCloseDelay), 2, i});
        if (o.showDelay == 0) deliverShow(i);   // synchronous ShowContainerGui inside OpenInventory
        else if (o.showDelay > 0) evs.push_back({frame + static_cast<uint32_t>(o.showDelay), 0, i});
    }
    void sendClose(uintptr_t msg, uint32_t obj, int flag) override {
        (void)msg;
        rec(A_SEND, 0, obj, flag, "");
        const int i = byObj(obj);
        if (i < 0) { violate("SendClose for an unknown container"); return; }
        Obj& o = objs[static_cast<size_t>(i)];
        if (!o.outstanding) violate("SendClose without an open container (double Close)");
        o.outstanding = false; o.open = 0;
        if (flag == 1) { taken.push_back(o.items); o.items.clear(); }
    }
    void serverClose(uintptr_t plc, uint32_t opener) override {
        (void)opener;
        rec(A_SRV, plc, 0, 0, "");
        const int i = byPlc(plc);
        if (i < 0) { violate("ServerClose on a missing container"); return; }
        Obj& o = objs[static_cast<size_t>(i)];
        if (!o.outstanding) violate("ServerClose without an open container (double Close)");
        o.outstanding = false; o.open = 0;
    }
    void labelShow(const char* t) override { rec(A_LSHOW, 0, 0, 0, t); label = t; }
    void labelHide() override { rec(A_LHIDE, 0, 0, 0, ""); label.clear(); }
    void toast(const char* t) override { rec(A_TOAST, 0, 0, 0, t); }
    void log(const char* l) override { rec(A_LOG, 0, 0, 0, l); }
};

#define CHK(name, cond) kt::Check((std::string(scn) + ": " + (name)).c_str(), (cond))
#define CHK_EQ(name, a, b) kt::CheckEq((std::string(scn) + ": " + (name)).c_str(), (a), (b))

const Item Medpac2 = {"Medpac", 2}, Pistol = {"Blaster Pistol", 1}, Credits = {"Credits", 1};

// ---- a: click take
void scenarioA() {
    {   // a1: Show flag 1 -> 0x16 two frames later -> exactly one SendClose(1) + Toast
        const char* scn = "a1 click take, flag 1";
        Sim s; s.m.cfg.proxOn = false;
        const int A = s.add(1, 0, {Medpac2, Pistol});
        s.objs[static_cast<size_t>(A)].onOpenDelay = 2;
        s.run(3);
        s.click(A);
        const uint32_t showFrame = s.frame;
        CHK("Show consumed (take armed)", s.m.take.active && s.vanilla == 0);
        CHK("TAKE-ARM logged", s.logged("TAKE-ARM"));
        s.run(2);   // up to and including the OnOpen frame: still waiting
        CHK("no Close before the OnOpen settles", s.count(A_SEND) == 0);
        s.run(40);
        CHK_EQ("one SendClose", s.count(A_SEND), 1);
        const Act* c = s.nth(A_SEND, 0);
        CHK("flag 1, right object", c && c->flag == 1 && c->obj == s.objId(A));
        CHK("sent shortly after the OnOpen (<= 4 frames after Show)", c && c->frame <= showFrame + 4 + 2);
        CHK("one Toast", s.count(A_TOAST) == 1);
        const Act* t = s.nth(A_TOAST, 0);
        CHK("toast text", t && t->text == "Took: Medpac x2, Blaster Pistol");
        CHK("TOOK lines logged", s.logCount("TOOK ") == 2);
        CHK("items taken, take cleared", s.taken.size() == 1 && s.taken[0].size() == 2 && !s.m.take.active);
        s.run(80);
        CHK("nothing more happens", s.count(A_SEND) == 1 && s.count(A_OPEN) == 0 && s.count(A_TOAST) == 1);
        s.finishCheck(scn);
    }
    {   // a2: Show flag 0 (empty at Show), the OnOpen script adds loot -> SendClose(1)
        const char* scn = "a2 click take, empty at Show, OnOpen loot";
        Sim s; s.m.cfg.proxOn = false;
        const int A = s.add(1, 0, {});
        s.objs[static_cast<size_t>(A)].loot = {Pistol, Credits};
        s.objs[static_cast<size_t>(A)].onOpenDelay = 3;
        s.run(3);
        s.click(A);
        CHK("Show consumed with flag 0 (empty container)", s.m.take.active && s.vanilla == 0 && s.logged("TAKE-ARM obj=7f001000 plc=00050000 n=0"));
        s.run(40);
        CHK_EQ("one SendClose", s.count(A_SEND), 1);
        const Act* c = s.nth(A_SEND, 0);
        CHK("flag 1 (loot present at send time)", c && c->flag == 1);
        CHK("toast lists the OnOpen loot", s.count(A_TOAST) == 1 && s.nth(A_TOAST, 0)->text == "Took: Blaster Pistol, Credits");
        s.finishCheck(scn);
    }
    {   // a3: still empty after the OnOpen -> SendClose(0), no toast
        const char* scn = "a3 click take, stays empty";
        Sim s; s.m.cfg.proxOn = false;
        const int A = s.add(1, 0, {});
        s.run(3);
        s.click(A);
        s.run(40);
        CHK_EQ("one SendClose", s.count(A_SEND), 1);
        CHK("flag 0", s.nth(A_SEND, 0) && s.nth(A_SEND, 0)->flag == 0);
        CHK("no toast", s.count(A_TOAST) == 0);
        CHK("TAKE-EMPTY logged", s.logged("TAKE-EMPTY"));
        s.finishCheck(scn);
    }
    {   // a4: no OnOpen event at all -> sent SendTimeoutMs (1000 ms) after Show
        const char* scn = "a4 click take, no OnOpen";
        Sim s; s.m.cfg.proxOn = false;
        const int A = s.add(1, 0, {Medpac2});
        s.objs[static_cast<size_t>(A)].onOpenDelay = -1;
        s.run(3);
        const uint32_t t0 = s.now;
        s.click(A);
        s.run(50);   // 800 ms
        CHK("not before the timeout", s.count(A_SEND) == 0);
        s.run(30);
        CHK_EQ("one SendClose", s.count(A_SEND), 1);
        CHK("sent at >= 1000 ms", s.nth(A_SEND, 0) && (s.nth(A_SEND, 0)->frame - 5003) * 16 + 16 >= SendTimeoutMs && s.now - t0 > 1000);
        s.finishCheck(scn);
    }
}

// ---- b: no consume
void scenarioB() {
    {
        const char* scn = "b1 shift held";
        Sim s; s.m.cfg.proxOn = false; s.shift = true;
        const int A = s.add(1, 0, {Medpac2});
        s.run(3);
        s.click(A);
        s.run(60);
        CHK("not consumed", s.vanilla == 1 && !s.m.take.active);
        CHK("VANILLA reason=shift", s.logged("VANILLA reason=shift"));
        CHK("no actions at all", s.count(A_SEND) == 0 && s.count(A_OPEN) == 0 && s.count(A_SRV) == 0 && s.count(A_TOAST) == 0 && s.count(A_LSHOW) == 0);
        s.objs[static_cast<size_t>(A)].outstanding = false;   // the vanilla menu owns this open
        s.finishCheck(scn);
    }
    {
        const char* scn = "b2 disabled";
        Sim s; s.m.cfg.enabled = false;
        const int A = s.add(1, 0, {Medpac2});
        s.run(3);
        s.click(A);
        s.run(60);
        CHK("not consumed, VANILLA reason=disabled", s.vanilla == 1 && s.logged("VANILLA reason=disabled"));
        CHK("no actions (also no proximity peek)", s.count(A_SEND) == 0 && s.count(A_OPEN) == 0 && s.count(A_TOAST) == 0);
        s.objs[static_cast<size_t>(A)].outstanding = false;
        s.finishCheck(scn);
    }
    {
        const char* scn = "b3 no hud-host";
        Sim s; s.m.cfg.proxOn = false; s.hud = false;
        const int A = s.add(1, 0, {Medpac2});
        s.run(3);
        s.click(A);
        s.run(60);
        CHK("not consumed, VANILLA reason=nohud", s.vanilla == 1 && s.logged("VANILLA reason=nohud"));
        CHK("no actions", s.count(A_SEND) == 0 && s.count(A_OPEN) == 0 && s.count(A_TOAST) == 0);
        s.objs[static_cast<size_t>(A)].outstanding = false;
        s.finishCheck(scn);
    }
    {
        const char* scn = "b4 unresolved container / second Show while a take is pending";
        Sim s; s.m.cfg.proxOn = false;
        const int A = s.add(1, 0, {Medpac2}), B = s.add(1.5f, 0, {Pistol});
        s.run(3);
        CHK_EQ("plc 0 -> vanilla", s.m.onShow(s, 0x7F00DEADu, 0, 1, 1, s.now), 0);
        CHK("VANILLA reason=noplc", s.logged("VANILLA reason=noplc"));
        s.click(A);
        const int v0 = s.vanilla;
        s.deliverShow(B);   // a second Show while the first take is pending
        CHK("second Show goes vanilla (busy)", s.vanilla == v0 + 1 && s.logged("VANILLA reason=busy"));
        s.objs[static_cast<size_t>(B)].outstanding = false;
        s.run(40);
        CHK_EQ("the first take still closes once", s.count(A_SEND), 1);
        s.finishCheck(scn);
    }
}

// ---- c: the full peek
void scenarioC() {
    const char* scn = "c peek approach / take / cooldown";
    Sim s;
    const int A = s.add(10, 0, {Medpac2});
    s.objs[static_cast<size_t>(A)].loot = {Pistol};
    s.objs[static_cast<size_t>(A)].onOpenDelay = 30;   // later than the first poll after Show, so the label lists the pre-OnOpen contents first
    s.setLeader(0, 0);
    s.run(40);
    CHK("far away: nothing opened", s.count(A_OPEN) == 0);
    s.setLeader(7.1f, 0);   // 2.9 m
    s.run(70);
    CHK_EQ("one OpenInventory", s.count(A_OPEN), 1);
    CHK("PEEK-OPEN logged once", s.logCount("PEEK-OPEN") == 1);
    CHK("Show consumed as a peek", s.vanilla == 0 && s.m.peek.active && s.logged("PEEK-SHOW"));
    CHK("label shown after the OnOpen loot arrived", s.label == "2 items  [F] take all: Medpac x2, Blaster Pistol");
    bool sawFirst = false;
    for (const Act& a : s.acts) if (a.k == A_LSHOW && a.text == "1 item  [F] take all: Medpac x2") sawFirst = true;
    CHK("label first listed the pre-OnOpen contents", sawFirst);
    CHK("no Close yet", s.count(A_SEND) == 0 && s.count(A_SRV) == 0);
    const size_t mk = s.mark();
    s.key = true;
    s.run(1);
    s.run(10);   // key held: one edge only
    s.key = false;
    CHK_EQ("F edge: exactly one SendClose", s.count(A_SEND, mk), 1);
    CHK("flag 1 and a Toast", s.nth(A_SEND, 0, mk) && s.nth(A_SEND, 0, mk)->flag == 1 && s.count(A_TOAST, mk) == 1);
    CHK("toast text", s.nth(A_TOAST, 0, mk) && s.nth(A_TOAST, 0, mk)->text == "Took: Medpac x2, Blaster Pistol");
    CHK("label hidden, peek over, cooldown holds the container", s.label.empty() && !s.m.peek.active && s.m.cool.contains(s.objs[static_cast<size_t>(A)].sid));
    s.run(80);   // still standing there
    CHK("cooldown blocks the reopen", s.count(A_OPEN) == 1);
    s.setLeader(4.9f, 0);   // 5.1 m: > CloseM
    s.run(40);
    CHK("cooldown released past CloseM", !s.m.cool.contains(s.objs[static_cast<size_t>(A)].sid) && s.logged("COOLDOWN release"));
    CHK("still no reopen while away", s.count(A_OPEN) == 1);
    s.setLeader(7.1f, 0);
    s.run(100);
    CHK_EQ("re-approach reopens", s.count(A_OPEN), 2);
    CHK("second peek: empty container closes quietly (Close 0, no toast)", s.count(A_SEND) == 2 && s.nth(A_SEND, 1)->flag == 0 && s.count(A_TOAST) == 1 && s.logged("PEEK-EMPTY"));
    s.finishCheck(scn);
}

void scenarioC2() {
    {   // key already held when the peek opens never fires
        const char* scn = "c2 key held through the open";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.key = true;
        s.run(60);
        CHK("opened, but a held key is not an edge", s.count(A_OPEN) == 1 && s.count(A_SEND) == 0 && s.m.peek.active);
        s.key = false; s.run(2); s.key = true; s.run(2);
        CHK("a fresh press takes", s.count(A_SEND) == 1 && s.nth(A_SEND, 0)->flag == 1);
        (void)A;
        s.finishCheck(scn);
    }
    {   // synchronous Show inside OpenInventory
        const char* scn = "c3 synchronous Show";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.objs[static_cast<size_t>(A)].showDelay = 0;
        s.setLeader(7.1f, 0);
        s.run(30);
        CHK("peek active, no NOSHOW, no refusal", s.count(A_OPEN) == 1 && s.m.peek.active && !s.logged("PEEK-NOSHOW") && !s.logged("PEEK-REFUSED"));
        s.key = true; s.run(2);
        CHK("take works", s.count(A_SEND) == 1);
        s.finishCheck(scn);
    }
    {   // empty at Show, loot from the OnOpen: label appears with the loot, no early empty-close
        const char* scn = "c4 empty at Show, OnOpen loot";
        Sim s;
        const int A = s.add(10, 0, {});
        s.objs[static_cast<size_t>(A)].loot = {Credits};
        s.objs[static_cast<size_t>(A)].onOpenDelay = 8;
        s.setLeader(7.1f, 0);
        s.run(60);
        CHK("opened, Show consumed with flag 0", s.count(A_OPEN) == 1 && s.m.peek.active && s.logged("PEEK-SHOW obj=7f001000 plc=00050000 flag=0"));
        CHK("not closed as empty", s.count(A_SEND) == 0 && !s.logged("PEEK-EMPTY"));
        CHK("label has the loot", s.label == "1 item  [F] take all: Credits");
        s.finishCheck(scn);
    }
}

// ---- d: walk away
void scenarioD() {
    const char* scn = "d walk away without F";
    Sim s;
    const int A = s.add(10, 0, {Medpac2});
    s.setLeader(7.1f, 0);
    s.run(40);
    CHK("peek up with a label", s.m.peek.active && !s.label.empty());
    s.setLeader(5.5f, 0);   // 4.5 m: inside the hysteresis band
    s.run(40);
    CHK("inside CloseM the peek stays", s.m.peek.active && s.count(A_SEND) == 0);
    s.setLeader(4.9f, 0);   // 5.1 m
    s.run(40);
    CHK_EQ("exactly one SendClose", s.count(A_SEND), 1);
    CHK("flag 0, no toast, items stay", s.nth(A_SEND, 0)->flag == 0 && s.count(A_TOAST) == 0 && s.objs[static_cast<size_t>(A)].items.size() == 1);
    CHK("label hidden, PEEK-CLOSE reason=far", s.label.empty() && s.logged("PEEK-CLOSE obj=7f001000 reason=far sent=1"));
    CHK("exactly one OpenInventory", s.count(A_OPEN) == 1);
    s.finishCheck(scn);
}

// ---- e: suppression
struct SupCase { const char* name; void (*on)(Sim&); void (*off)(Sim&); const char* reason; };

void scenarioE() {
    const SupCase cases[] = {
        {"combat", [](Sim& s) { s.combat = 1; }, [](Sim& s) { s.combat = 0; }, "combat"},
        {"combat unreadable", [](Sim& s) { s.combat = -1; }, [](Sim& s) { s.combat = 0; }, "combat?"},
        {"dialog", [](Sim& s) { s.dialog = true; }, [](Sim& s) { s.dialog = false; }, "dialog"},
        {"menu", [](Sim& s) { s.menu = true; }, [](Sim& s) { s.menu = false; }, "menu"},
        {"pause", [](Sim& s) { s.paused = true; }, [](Sim& s) { s.paused = false; }, "paused"},
        {"load (world gone)", [](Sim& s) { s.worldOk = false; }, [](Sim& s) { s.worldOk = true; }, "noworld"},
    };
    for (const SupCase& c : cases) {
        const std::string name = std::string("e ") + c.name + " mid-peek";
        const char* scn = name.c_str();
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        CHK("peek up", s.m.peek.active && s.count(A_OPEN) == 1);
        c.on(s);
        s.run(40);
        CHK_EQ("one SendClose", s.count(A_SEND), 1);
        CHK("flag 0, label hidden", s.nth(A_SEND, 0)->flag == 0 && s.label.empty() && s.count(A_TOAST) == 0);
        CHK("PEEK-CLOSE carries the suppress reason", s.logCount((std::string("reason=") + c.reason).c_str()) == 1);
        s.run(80);
        CHK("no new OpenInventory while suppressed", s.count(A_OPEN) == 1);
        c.off(s);
        s.run(80);
        CHK("none after the state ends either (cooldown until the leader leaves)", s.count(A_OPEN) == 1 && s.count(A_SEND) == 1);
        (void)A;
        s.finishCheck(scn);
    }
    for (const SupCase& c : cases) {   // suppression BEFORE the approach: no open until it ends
        const std::string name = std::string("e ") + c.name + " before approach";
        const char* scn = name.c_str();
        Sim s;
        s.add(10, 0, {Medpac2});
        c.on(s);
        s.setLeader(7.1f, 0);
        s.run(60);
        CHK("no OpenInventory while suppressed", s.count(A_OPEN) == 0);
        c.off(s);
        s.run(40);
        CHK("opens once it ends", s.count(A_OPEN) == 1 && s.m.peek.active);
        s.finishCheck(scn);
    }
}

// ---- f: area change
void scenarioF() {
    {
        const char* scn = "f1 area change mid-peek (container still resolves)";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        CHK("peek up", s.m.peek.active);
        s.area = 0xB0000;   // load into another area; the old container is out of range of the leader there
        s.setLeader(500, 500);
        s.run(40);
        CHK_EQ("one close", s.count(A_SEND), 1);
        CHK("flag 0, reason=area or far, label hidden", s.nth(A_SEND, 0)->flag == 0 && s.label.empty() && !s.m.peek.active);
        CHK("AREA logged", s.logged("AREA area=000b0000"));
        CHK("cooldown cleared (no stale old-area id)", s.m.cool.n == 0);
        CHK("no reopen", s.count(A_OPEN) == 1);
        (void)A;
        s.finishCheck(scn);
    }
    {
        const char* scn = "f2 area change mid-peek, leader stays in range of the same coordinates";
        Sim s;
        s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        s.area = 0xB0000;   // same leader position: the distance alone would not end the peek
        s.run(40);
        CHK_EQ("one close", s.count(A_SEND), 1);
        CHK("reason=area", s.logged("reason=area sent=1"));
        CHK("cooldown cleared", s.m.cool.n == 0);
        s.finishCheck(scn);
    }
    {
        const char* scn = "f3 area change mid-peek, container gone";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        s.area = 0xB0000;
        s.remove(A);
        s.run(40);
        CHK("no Close for a vanished container, peek over, cooldown clear", s.count(A_SEND) == 0 && !s.m.peek.active && s.m.cool.n == 0);
        CHK("PEEK-CLOSE reason=gone sent=0", s.logged("reason=gone sent=0"));
        s.finishCheck(scn);
    }
}

// ---- g: OpenInventory but no Show
void scenarioG() {
    {
        const char* scn = "g1 no Show, open flag set";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.objs[static_cast<size_t>(A)].showDelay = -1;
        s.setLeader(7.1f, 0);
        s.run(60);   // ~960 ms
        CHK("still expecting before the timeout", s.count(A_SRV) == 0 && s.m.peek.expect != 0);
        s.run(60);
        CHK_EQ("one ServerClose", s.count(A_SRV), 1);
        CHK("PEEK-NOSHOW open=1 + server close logged", s.logged("PEEK-NOSHOW obj=00001000") && s.logged("PEEK-NOSHOW server close sent"));
        CHK("no client Close", s.count(A_SEND) == 0);
        CHK("closed server-side (open flag cleared)", s.objs[static_cast<size_t>(A)].open == 0);
        s.run(100);
        CHK("no reopen (cooldown)", s.count(A_OPEN) == 1 && s.count(A_SRV) == 1);
        s.finishCheck(scn);
    }
    {
        const char* scn = "g2 no Show, open flag dropped by itself";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.objs[static_cast<size_t>(A)].showDelay = -1;
        s.objs[static_cast<size_t>(A)].autoCloseDelay = 10;
        s.objs[static_cast<size_t>(A)].onOpenDelay = -1;
        s.setLeader(7.1f, 0);
        s.run(120);
        CHK("NOSHOW logged, no ServerClose (flag not set)", s.logged("PEEK-NOSHOW") && s.count(A_SRV) == 0 && s.count(A_SEND) == 0);
        s.objs[static_cast<size_t>(A)].outstanding = false;   // the container closed itself
        s.finishCheck(scn);
    }
    {
        const char* scn = "g3 engine refuses OpenInventory";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.objs[static_cast<size_t>(A)].refuse = true;
        s.setLeader(7.1f, 0);
        s.run(120);
        CHK("PEEK-REFUSED, no ServerClose, no retry", s.logged("PEEK-REFUSED") && s.count(A_OPEN) == 1 && s.count(A_SRV) == 0 && s.count(A_SEND) == 0 && !s.m.peek.busy());
        s.finishCheck(scn);
    }
}

// ---- h: message object unavailable
void scenarioH() {
    {
        const char* scn = "h1 peek leave, message object back in time";
        Sim s;
        s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        s.msgOk = false;
        s.setLeader(4.9f, 0);
        s.run(120);   // ~1.9 s of retries
        CHK("no Close while unavailable, still active", s.count(A_SEND) == 0 && s.m.peek.active);
        CHK("ERR peek-msgobj logged exactly once", s.logCount("ERR peek-msgobj") == 1);
        s.msgOk = true;
        s.run(40);
        CHK_EQ("one Close once available", s.count(A_SEND), 1);
        CHK("flag 0, peek ended", s.nth(A_SEND, 0)->flag == 0 && !s.m.peek.active);
        s.run(80);
        CHK("no duplicate", s.count(A_SEND) == 1);
        s.finishCheck("h1 peek leave, message object back in time");
    }
    {
        const char* scn = "h2 peek leave, message object never comes (GiveUpMs)";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        s.msgOk = false;
        s.setLeader(4.9f, 0);
        s.run(250);   // ~4 s: not yet
        CHK("retrying before GiveUpMs", s.m.peek.active && s.count(A_SEND) == 0);
        s.run(150);   // past 5 s
        CHK("gave up: peek ended, no Close", !s.m.peek.active && s.count(A_SEND) == 0 && s.logged("PEEK-CLOSE obj=7f001000 reason=far sent=0"));
        s.msgOk = true;
        s.run(120);
        CHK("no late duplicate Close", s.count(A_SEND) == 0 && s.count(A_SRV) == 0);
        s.objs[static_cast<size_t>(A)].excused = true;   // known gap: the container stays open server-side
        s.finishCheck(scn);
    }
    {
        const char* scn = "h3 F pressed while the message object is missing";
        Sim s;
        s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        s.msgOk = false;
        s.key = true; s.run(2); s.key = false; s.run(20);
        CHK("ERR peek-take-msgobj, nothing sent, peek stays", s.logged("ERR peek-take-msgobj") && s.count(A_SEND) == 0 && s.m.peek.active);
        s.msgOk = true;
        s.key = true; s.run(2); s.key = false; s.run(20);
        CHK("second press takes", s.count(A_SEND) == 1 && s.nth(A_SEND, 0)->flag == 1);
        s.finishCheck(scn);
    }
    {
        const char* scn = "h4 click take: message object back after N frames";
        Sim s; s.m.cfg.proxOn = false;
        const int A = s.add(1, 0, {Medpac2});
        s.run(3);
        s.msgOk = false;
        s.click(A);
        s.run(150);   // 2.4 s
        CHK("no Close, retrying, ERR logged once", s.count(A_SEND) == 0 && s.m.take.active && s.logCount("ERR msgobj") == 1);
        s.msgOk = true;
        s.run(10);
        CHK_EQ("one Close", s.count(A_SEND), 1);
        CHK("flag 1 + toast", s.nth(A_SEND, 0)->flag == 1 && s.count(A_TOAST) == 1);
        s.run(60);
        CHK("no duplicate", s.count(A_SEND) == 1 && s.count(A_TOAST) == 1);
        s.finishCheck(scn);
    }
    {
        const char* scn = "h5 click take: give up";
        Sim s; s.m.cfg.proxOn = false;
        const int A = s.add(1, 0, {Medpac2});
        s.run(3);
        s.msgOk = false;
        s.click(A);
        s.run(400);   // 6.4 s
        CHK("ERR giveup logged, take cleared, no Close", s.logged("ERR giveup") && !s.m.take.active && s.count(A_SEND) == 0);
        s.msgOk = true;
        s.run(80);
        CHK("no late Close", s.count(A_SEND) == 0);
        s.objs[static_cast<size_t>(A)].excused = true;   // known gap: the container stays open server-side
        s.finishCheck(scn);
    }
}

// ---- i: candidate choice
void scenarioI() {
    {
        const char* scn = "i1 nearest only";
        Sim s;
        const int A = s.add(2.0f, 0, {Medpac2}), B = s.add(1.0f, 0, {Pistol});
        s.run(60);
        CHK("only the nearest was opened", s.count(A_OPEN) == 1 && s.nth(A_OPEN, 0)->plc == s.objs[static_cast<size_t>(B)].plc);
        CHK("the farther one waits while the peek is up", s.m.peek.active && s.m.peek.plc == s.objs[static_cast<size_t>(B)].plc);
        s.key = true; s.run(2); s.key = false; s.run(60);
        CHK("after B's take, A is next (one at a time)", s.count(A_OPEN) == 2 && s.nth(A_OPEN, 1)->plc == s.objs[static_cast<size_t>(A)].plc);
        s.finishCheck(scn);
    }
    {
        const char* scn = "i2 ineligible containers are never opened";
        Sim s;
        const int L = s.add(0.5f, 0, {Medpac2}), T = s.add(0.6f, 0, {Medpac2}), N = s.add(0.7f, 0, {Medpac2}), U = s.add(0.8f, 0, {Medpac2});
        const int O = s.add(0.9f, 0, {Medpac2}), D = s.add(0.2f, 0, {Medpac2}), F = s.add(4.0f, 0, {Medpac2});
        s.objs[static_cast<size_t>(L)].locked = true;
        s.objs[static_cast<size_t>(T)].trap = true;
        s.objs[static_cast<size_t>(N)].hasInv = false;
        s.objs[static_cast<size_t>(U)].useable = false;
        s.objs[static_cast<size_t>(O)].open = 1;       // already open (someone else's menu)
        s.objs[static_cast<size_t>(D)].isPlc = false;  // not a placeable
        (void)F;                                       // 4.0 m: inside CloseM, outside OpenM
        s.run(100);
        CHK("nothing opened", s.count(A_OPEN) == 0 && !s.m.peek.busy());
        CHK("SCAN names the nearest in-range candidate once, with its verdict", s.logCount("SCAN nearest obj=00001000") == 1 && s.logged("why=locked"));
        const int E = s.add(2.5f, 0, {Pistol});
        s.run(60);
        CHK("only the eligible one opens", s.count(A_OPEN) == 1 && s.nth(A_OPEN, 0)->plc == s.objs[static_cast<size_t>(E)].plc);
        s.finishCheck(scn);
    }
    {
        const char* scn = "i3 beyond OpenM is not opened, inside it is";
        Sim s;
        s.add(3.2f, 0, {Medpac2});
        s.run(60);
        CHK("3.2 m: far", s.count(A_OPEN) == 0);
        s.setLeader(0.3f, 0);   // 2.9 m
        s.run(40);
        CHK("2.9 m: opens", s.count(A_OPEN) == 1);
        s.finishCheck(scn);
    }
    {
        const char* scn = "i4 a pending click take blocks the proximity scan";
        Sim s;
        const int A = s.add(6, 0, {Medpac2});   // clicked from afar: out of OpenM, no peek of its own
        const int B = s.add(2, 0, {Pistol});
        s.objs[static_cast<size_t>(A)].onOpenDelay = -1;   // no OnOpen: the take waits SendTimeoutMs
        s.click(A);
        s.run(40);   // > 2 scan intervals, take still pending
        CHK("take pending, the neighbour is not opened meanwhile", s.m.take.active && s.count(A_OPEN) == 0);
        s.run(60);
        CHK("take sent once", s.count(A_SEND) == 1 && s.nth(A_SEND, 0)->flag == 1);
        s.run(40);
        CHK("then the neighbour peeks", s.count(A_OPEN) == 1 && s.nth(A_OPEN, 0)->plc == s.objs[static_cast<size_t>(B)].plc);
        s.finishCheck(scn);
    }
    {
        const char* scn = "i5 click on a container that is being peeked: one Close";
        Sim s;
        const int A = s.add(1, 0, {Medpac2});
        s.run(30);
        CHK("peek up", s.m.peek.active);
        s.click(A);   // the engine shows the container the leader already has peek-open (a second Show for the same open)
        s.run(120);
        CHK_EQ("exactly one SendClose (no double Close)", s.count(A_SEND), 1);
        CHK("it is the take-all, with the toast", s.nth(A_SEND, 0)->flag == 1 && s.count(A_TOAST) == 1);
        CHK("peek superseded", s.logged("PEEK-SUPERSEDED") && !s.m.peek.busy() && s.label.empty());
        s.finishCheck(scn);
    }
}

// ---- j extra: container removed mid-peek, wrap-around clock
void scenarioJ() {
    {
        const char* scn = "j1 container gone mid-peek";
        Sim s;
        const int A = s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(40);
        s.remove(A);
        s.run(40);
        CHK("no Close for a missing container, peek ended", s.count(A_SEND) == 0 && !s.m.peek.active && s.logged("reason=gone sent=0"));
        s.finishCheck(scn);
    }
    {
        const char* scn = "j2 32-bit ms wrap during a peek";
        Sim s;
        s.now = 0xFFFFFFFFu - 600;
        s.add(10, 0, {Medpac2});
        s.setLeader(7.1f, 0);
        s.run(60);   // crosses the wrap
        CHK("peek up across the wrap, one open", s.count(A_OPEN) == 1 && s.m.peek.active && s.now < 5000u);
        s.key = true; s.run(2);
        CHK("take works after the wrap", s.count(A_SEND) == 1);
        s.finishCheck(scn);
    }
}

}   // namespace

int main() {
    scenarioA();
    scenarioB();
    scenarioC();
    scenarioC2();
    scenarioD();
    scenarioE();
    scenarioF();
    scenarioG();
    scenarioH();
    scenarioI();
    scenarioJ();
    return kt::Report("quick_loot_sim_test");
}
