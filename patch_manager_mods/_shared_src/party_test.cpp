// Host test of Party.h on a fake app -> a -> b -> table chain in a MAP_32BIT arena (doc 20 conventions).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "Party.h"
#include "GuiKit.h"
#include "K2Addr.h"
#include "check.h"
#include <sys/mman.h>
#include <initializer_list>

// gameaddr must agree with GuiKit's constants for every shared address.
static_assert(gameaddr::New == guikit::AddrNew, "New");
static_assert(gameaddr::LabelCtor == guikit::AddrLabelCtor, "LabelCtor");
static_assert(gameaddr::InitControl == guikit::AddrInitControl, "InitControl");
static_assert(gameaddr::StopLoadFromLayout == guikit::AddrStopLoad, "StopLoadFromLayout");
static_assert(gameaddr::ExoCtor == guikit::AddrExoCtor, "ExoCtor");
static_assert(gameaddr::ExoDtor == guikit::AddrExoDtor, "ExoDtor");
static_assert(gameaddr::TextSet == guikit::AddrTextSet, "TextSet");
static_assert(gameaddr::SetFont == guikit::AddrSetFont, "SetFont");
static_assert(gameaddr::SetAlign == guikit::AddrSetAlign, "SetAlign");
static_assert(gameaddr::TextColor == guikit::AddrTextColor, "TextColor");
static_assert(gameaddr::PtrListAdd == guikit::AddrPtrListAdd, "PtrListAdd");
static_assert(gameaddr::GuiLoadImage == guikit::AddrLoadImage, "LoadImage");
static_assert(gameaddr::GuiSoundMgr == guikit::AddrGuiSoundMgr, "GuiSoundMgr");
static_assert(gameaddr::PlayGuiSound == guikit::AddrPlayGuiSound, "PlayGuiSound");
static_assert(gameaddr::ScreenW == guikit::AddrScreenW, "ScreenW");
static_assert(gameaddr::ScreenH == guikit::AddrScreenH, "ScreenH");
static_assert(gameaddr::LabelVtable == uintptr_t(guikit::LabelVtable), "LabelVtable");
static_assert(gameaddr::BorderVtable == uintptr_t(guikit::BorderVtable), "BorderVtable");
static_assert(gameaddr::TextVtable == uintptr_t(guikit::TextVtable), "TextVtable");

// Every address / offset Party.h uses, against the validated Steam values. Party.h reads these from gameaddr:: (compile-time), not through
// the K2Addr table; kSteam is built from the same constants, so both columns must agree.
static_assert(gameaddr::AppGlobal == 0x00A1B4A4u, "AppGlobal");
static_assert(gameaddr::GetControlledCreature == 0x007E5D80u, "GetControlledCreature");
static_assert(gameaddr::PartyGetAt == 0x007E5DA0u, "PartyGetAt");
static_assert(gameaddr::MemberServerCreature == 0x0077D800u, "MemberServerCreature");
static_assert(kSteam.AppGlobal == gameaddr::AppGlobal, "kSteam.AppGlobal");
static_assert(kSteam.GetControlledCreature == gameaddr::GetControlledCreature, "kSteam.GetControlledCreature");
static_assert(kSteam.PartyGetAt == gameaddr::PartyGetAt, "kSteam.PartyGetAt");
static_assert(kSteam.MemberServerCreature == gameaddr::MemberServerCreature, "kSteam.MemberServerCreature");
// GOG column (doc 21b section 4)
static_assert(kGog.AppGlobal == 0x00A11C04u, "kGog.AppGlobal");
static_assert(kGog.GetControlledCreature == 0x004789B0u, "kGog.GetControlledCreature");
static_assert(kGog.PartyGetAt == 0x004789D0u, "kGog.PartyGetAt");
static_assert(kGog.MemberServerCreature == 0x00429780u, "kGog.MemberServerCreature");
static_assert(kGog.ClientPartyTable == 0x0040D070u, "kGog.ClientPartyTable");
static_assert(party::SlotMaxHp == 0x98 && party::SlotCurHp == 0x9C, "vtable HP slots");
static_assert(party::SiteBase == 0x6C00, "saferead site id");

static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 16;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static int g_calls = 0; static void* g_lastTable = nullptr;
static const uintptr_t Creature = 0xC0FFEE;
static uintptr_t __thiscall fakeGetControlled(void* t) { ++g_calls; g_lastTable = t; return Creature; }

static uintptr_t cell, app, a, b, tbl;
static void build(int count) {
    at<uint32_t>(cell, 0) = (uint32_t)app;
    at<uint32_t>(app, 4) = (uint32_t)a;
    at<uint32_t>(a, 4) = (uint32_t)b;
    at<uint32_t>(b, 0x270) = (uint32_t)tbl;
    at<int>(tbl, 0) = count;
    g_calls = 0; g_lastTable = nullptr;
}

// A broken link must give exactly `want`, 0, no engine call and an untouched count.
static void broken(const char* name, party::Fail want, const char* text) {
    int before = kt::g_failures;
    int n = 77; party::Fail w = party::F_None;
    uintptr_t r = party::controlledCreature(&w, &n);
    KT_CHECK_EQ((unsigned long long)r, 0ull);
    KT_CHECK_EQ((long long)w, (long long)want);
    KT_CHECK_EQ(party::failText(w), text);
    KT_CHECK_EQ((long long)g_calls, 0ll);
    KT_CHECK_EQ((long long)n, 77ll);
    party::Fail tw = party::F_None;
    KT_CHECK_EQ((unsigned long long)party::table(&tw), 0ull);
    KT_CHECK_EQ((long long)tw, (long long)want);
    if (kt::g_failures != before) std::printf("  ^ in broken-link case: %s\n", name);
}


// ---- members / vitals ----
struct FakeC { uintptr_t addr; int hp, maxhp; };
static FakeC g_fc[4];
static uintptr_t g_slotMax, g_slotCur;           // distinct non-zero fake "function addresses"
static int g_vcalls = 0; static uintptr_t g_vFn[16], g_vC[16]; static int g_vArg[16];
static uintptr_t g_clients[4], g_servers[4];
static uintptr_t __thiscall fakeMember(void* t, int i) { (void)t; return g_clients[i]; }
static uintptr_t __thiscall fakeServer(void* m) { for (int i = 0; i < 4; ++i) if (g_clients[i] == (uintptr_t)m) return g_servers[i]; return 0; }
static int fakeVital(uintptr_t fn, uintptr_t c, int arg) {
    if (g_vcalls < 16) { g_vFn[g_vcalls] = fn; g_vC[g_vcalls] = c; g_vArg[g_vcalls] = arg; }
    ++g_vcalls;
    for (int i = 0; i < 4; ++i) if (g_servers[i] == c) return arg == 1 ? g_fc[i].maxhp : g_fc[i].hp;
    return -999;
}
// server creature i: vtable in the arena with slots +0x98/+0x9C
static uintptr_t mkCreature(int i, int hp, int maxhp) {
    uintptr_t vt = arenaAlloc(0x100), c = arenaAlloc(16);
    at<uint32_t>(vt, 0x98) = (uint32_t)g_slotMax; at<uint32_t>(vt, 0x9C) = (uint32_t)g_slotCur;
    at<uint32_t>(c, 0) = (uint32_t)vt;
    g_servers[i] = c; g_clients[i] = 0x1000 + 0x10 * (i + 1); g_fc[i] = { c, hp, maxhp };
    return vt;
}

// counting wrappers for the edge-case tests
static int g_memberCalls = 0, g_serverCalls = 0; static uintptr_t g_serverArgs[8];
static uintptr_t __thiscall countMember(void* t, int i) { (void)t; ++g_memberCalls; return g_clients[i]; }
static uintptr_t __thiscall countServer(void* m) { if (g_serverCalls < 8) g_serverArgs[g_serverCalls] = (uintptr_t)m; ++g_serverCalls; return fakeServer(m); }
// shrinks the party table to 2 after the first PartyGetAt (member 2 "leaves" between partyCount and the per-member reads)
static uintptr_t __thiscall shrinkMember(void* t, int i) { ++g_memberCalls; if (i == 2) return g_clients[i]; at<int>((uintptr_t)t, 0) = 2; return g_clients[i]; }

int main() {
    void* m = mmap(nullptr, ArenaSize,PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    cell = arenaAlloc(16); app = arenaAlloc(16); a = arenaAlloc(16); b = arenaAlloc(0x300); tbl = arenaAlloc(16);
    party::g_appGlobal = cell;
    party::g_getControlled = fakeGetControlled;

    // success
    build(3);
    {
        int n = 77; party::Fail w = party::F_App;
        uintptr_t r = party::controlledCreature(&w, &n);
        KT_CHECK_EQ((unsigned long long)r, (unsigned long long)Creature);
        KT_CHECK_EQ((long long)w, (long long)party::F_None);
        KT_CHECK_EQ((long long)n, 3ll);
        KT_CHECK_EQ((long long)g_calls, 1ll);
        KT_CHECK_EQ((unsigned long long)reinterpret_cast<uintptr_t>(g_lastTable), (unsigned long long)tbl);
        KT_CHECK_EQ((unsigned long long)party::table(), (unsigned long long)tbl);
        KT_CHECK_EQ((long long)party::partyCount(tbl), 3ll);
        KT_CHECK_EQ((long long)party::partyCount(0), 0ll);
        KT_CHECK_EQ((long long)party::partyCount(0x20), 0ll);
    }
    // null out-params
    build(2);
    KT_CHECK_EQ((unsigned long long)party::controlledCreature(), (unsigned long long)Creature);
    KT_CHECK_EQ((long long)g_calls, 1ll);
    build(2);
    KT_CHECK_EQ((unsigned long long)party::controlledCreature(nullptr, nullptr), (unsigned long long)Creature);
    build(0);
    KT_CHECK_EQ((unsigned long long)party::controlledCreature(), 0ull);
    build(2);
    { int n = 0; KT_CHECK_EQ((unsigned long long)party::controlledCreature(nullptr, &n), (unsigned long long)Creature); KT_CHECK_EQ((long long)n, 2ll); }
    build(2); at<uint32_t>(a, 4) = 0;
    KT_CHECK_EQ((unsigned long long)party::controlledCreature(), 0ull);

    // zero links
    build(3); at<uint32_t>(cell, 0) = 0;  broken("app cell 0", party::F_App, "app global null");
    build(3); at<uint32_t>(app, 4) = 0;   broken("app+4 0", party::F_App4, "app+4 null");
    build(3); at<uint32_t>(a, 4) = 0;     broken("a+4 0", party::F_A4, "a+4 null");
    build(3); at<uint32_t>(b, 0x270) = 0; broken("b+0x270 0", party::F_Table, "party table null");
    // unreadable links (below 0x10000)
    build(3); at<uint32_t>(cell, 0) = 0x20; broken("app 0x20", party::F_App4, "app+4 null");
    build(3); at<uint32_t>(app, 4) = 0x20;  broken("a 0x20", party::F_A4, "a+4 null");
    build(3); at<uint32_t>(a, 4) = 0x20;    broken("b 0x20", party::F_Table, "party table null");
    build(3); at<uint32_t>(b, 0x270) = 0x20;
    {   // table pointer itself unreadable: table() returns it, count read fails -> count 0
        int n = 77; party::Fail w = party::F_None;
        KT_CHECK_EQ((unsigned long long)party::controlledCreature(&w, &n), 0ull);
        KT_CHECK_EQ((long long)w, (long long)party::F_Count);
        KT_CHECK_EQ((long long)n, 0ll);
        KT_CHECK_EQ((long long)g_calls, 0ll);
    }
    // unreadable app global address itself
    build(3); party::g_appGlobal = 0x20; broken("app global addr unreadable", party::F_App, "app global null"); party::g_appGlobal = cell;

    // bad counts
    for (int c : {0, -1}) {
        build(c);
        int n = 77; party::Fail w = party::F_None;
        KT_CHECK_EQ((unsigned long long)party::controlledCreature(&w, &n), 0ull);
        KT_CHECK_EQ((long long)w, (long long)party::F_Count);
        KT_CHECK_EQ((long long)n, (long long)c);
        KT_CHECK_EQ((long long)g_calls, 0ll);
        KT_CHECK_EQ(party::failText(w), "party count 0");
    }
    // missing engine fn
    build(2); party::g_getControlled = nullptr;
    KT_CHECK_EQ((unsigned long long)party::controlledCreature(), 0ull);
    party::g_getControlled = fakeGetControlled;


    // ---- SH-3c-08 (WI-99): the REAL callVitalSlot through vitals() (every other vitals case swaps in the int-returning fakeVital) ----
    // The engine HP slots return a short, so only AX is the value; the upper half of EAX is junk (lesson 355 family). x86-64 host stubs
    // `mov eax, imm32 ; ret` sit in a MAP_32BIT RWX page (vtable slots are 32-bit fields; a PIE test binary is above 4 GB). The
    // one-stack-arg RET 4 half of the contract cannot be observed on the x86-64 host (no callee-pops), only the short result can.
    {
        void* page = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
        KT_CHECK(page != MAP_FAILED);
        if (page != MAP_FAILED) {
            unsigned char* code = static_cast<unsigned char*>(page);
            auto putStub = [&](int slot, uint32_t eax) {
                unsigned char* s = code + 16 * slot;
                s[0] = 0xB8; memcpy(s + 1, &eax, 4); s[5] = 0xC3;
                return reinterpret_cast<uintptr_t>(s);
            };
            struct Case { uint32_t maxEax, curEax; int max, cur; bool down; };
            const Case cases[] = {
                { 0x12340064u, 0x1234FFFFu, 100, -1,     true  },   // AX = -1, junk above: must sign-extend (an int return reads +305463295)
                { 0x12340064u, 0xABCD0032u, 100, 50,     false },   // AX = 50, EAX negative as an int: must not read as down
                { 0x56780064u, 0x56780000u, 100, 0,      true  },   // AX = 0, junk above: down
                { 0x00010064u, 0x00018000u, 100, -32768, true  },   // AX = 0x8000: short minimum
            };
            auto sVc = party::g_callVital; auto sMx = g_slotMax; auto sCx = g_slotCur;
            party::g_callVital = party::callVitalSlot;
            for (const Case& cs : cases) {
                g_slotMax = putStub(0, cs.maxEax); g_slotCur = putStub(1, cs.curEax);
                KT_CHECK(g_slotMax <= 0xFFFFFFFFull && g_slotCur <= 0xFFFFFFFFull);
                build(1); mkCreature(0, 0, 0);
                party::Vitals v;
                KT_CHECK(party::vitals(g_servers[0], &v));
                KT_CHECK_EQ((long long)v.max, (long long)cs.max);
                KT_CHECK_EQ((long long)v.cur, (long long)cs.cur);
                KT_CHECK(v.ok); KT_CHECK_EQ((long long)v.down, (long long)cs.down);
            }
            party::g_callVital = sVc; g_slotMax = sMx; g_slotCur = sCx;
            munmap(page, 4096);
        }
    }

    // ---- member / serverCreature / vitals / members ----
    {
        auto sM = party::g_member; auto sS = party::g_serverCreature; auto sV = party::g_callVital;
        party::g_member = fakeMember; party::g_serverCreature = fakeServer; party::g_callVital = fakeVital;
        g_slotMax = 0x401000; g_slotCur = 0x402000;

        // member() bounds
        build(3); mkCreature(0, 50, 100); mkCreature(1, 0, 80); mkCreature(2, 7, 7);
        KT_CHECK_EQ((unsigned long long)party::member(tbl, 0), (unsigned long long)g_clients[0]);
        KT_CHECK_EQ((unsigned long long)party::member(tbl, 2), (unsigned long long)g_clients[2]);
        KT_CHECK_EQ((unsigned long long)party::member(tbl, -1), 0ull);
        KT_CHECK_EQ((unsigned long long)party::member(tbl, 3), 0ull);
        KT_CHECK_EQ((unsigned long long)party::member(0, 0), 0ull);
        party::g_member = nullptr; KT_CHECK_EQ((unsigned long long)party::member(tbl, 0), 0ull); party::g_member = fakeMember;
        KT_CHECK_EQ((unsigned long long)party::serverCreature(0), 0ull);
        KT_CHECK_EQ((unsigned long long)party::serverCreature(g_clients[1]), (unsigned long long)g_servers[1]);

        // vitals(): slots and exactly-one-arg ABI (max: +0x98 arg 1, cur: +0x9C arg 0)
        g_vcalls = 0; party::Vitals v;
        KT_CHECK(party::vitals(g_servers[0], &v));
        KT_CHECK_EQ((long long)g_vcalls, 2ll);
        KT_CHECK_EQ((unsigned long long)g_vFn[0], (unsigned long long)g_slotMax);
        KT_CHECK_EQ((long long)g_vArg[0], 1ll);
        KT_CHECK_EQ((unsigned long long)g_vFn[1], (unsigned long long)g_slotCur);
        KT_CHECK_EQ((long long)g_vArg[1], 0ll);
        KT_CHECK_EQ((unsigned long long)g_vC[0], (unsigned long long)g_servers[0]);
        KT_CHECK_EQ((unsigned long long)g_vC[1], (unsigned long long)g_servers[0]);
        KT_CHECK_EQ((long long)v.max, 100ll); KT_CHECK_EQ((long long)v.cur, 50ll);
        KT_CHECK(v.ok); KT_CHECK(!v.down);
        KT_CHECK_EQ((unsigned long long)v.server, (unsigned long long)g_servers[0]);
        // cur 0 -> down; cur > 0 -> !down
        KT_CHECK(party::vitals(g_servers[1], &v)); KT_CHECK(v.down); KT_CHECK_EQ((long long)v.cur, 0ll);
        KT_CHECK(party::vitals(g_servers[2], &v)); KT_CHECK(!v.down);
        // null out-param / null server
        g_vcalls = 0;
        KT_CHECK(party::vitals(g_servers[0], nullptr));
        KT_CHECK(!party::vitals(0, &v)); KT_CHECK(!v.ok); KT_CHECK_EQ((long long)g_vcalls, 2ll);
        // max <= 0 -> not ok, not down
        g_fc[0].maxhp = 0; KT_CHECK(!party::vitals(g_servers[0], &v)); KT_CHECK(!v.ok); KT_CHECK(!v.down);
        g_fc[0].maxhp = -5; KT_CHECK(!party::vitals(g_servers[0], &v)); KT_CHECK(!v.ok);
        g_fc[0].maxhp = 100;
        // unreadable: vtable 0 / slot 0 / vtable below 0x10000 -> no call
        uintptr_t vt0 = at<uint32_t>(g_servers[0], 0);
        g_vcalls = 0;
        at<uint32_t>(g_servers[0], 0) = 0;
        KT_CHECK(!party::vitals(g_servers[0], &v)); KT_CHECK(!v.ok);
        at<uint32_t>(g_servers[0], 0) = 0x20;
        KT_CHECK(!party::vitals(g_servers[0], &v));
        at<uint32_t>(g_servers[0], 0) = (uint32_t)vt0;
        at<uint32_t>(vt0, 0x98) = 0; KT_CHECK(!party::vitals(g_servers[0], &v)); KT_CHECK(!v.ok);
        at<uint32_t>(vt0, 0x98) = (uint32_t)g_slotMax; at<uint32_t>(vt0, 0x9C) = 0;
        KT_CHECK(!party::vitals(g_servers[0], &v)); KT_CHECK(!v.ok);
        at<uint32_t>(vt0, 0x9C) = (uint32_t)g_slotCur;
        KT_CHECK_EQ((long long)g_vcalls, 0ll);

        // members(): count, aligned indices
        party::Vitals out[4];
        g_vcalls = 0;
        KT_CHECK_EQ((long long)party::members(out, 4), 3ll);
        KT_CHECK_EQ((long long)out[0].max, 100ll); KT_CHECK(out[0].ok && !out[0].down);
        KT_CHECK(out[1].ok && out[1].down); KT_CHECK_EQ((long long)out[1].max, 80ll);
        KT_CHECK(out[2].ok && !out[2].down); KT_CHECK_EQ((long long)out[2].cur, 7ll);
        KT_CHECK_EQ((unsigned long long)out[1].client, (unsigned long long)g_clients[1]);
        KT_CHECK_EQ((unsigned long long)out[1].server, (unsigned long long)g_servers[1]);
        KT_CHECK_EQ((long long)g_vcalls, 6ll);
        // cap smaller than count
        KT_CHECK_EQ((long long)party::members(out, 2), 2ll);
        KT_CHECK_EQ((long long)party::members(out, 0), 0ll);
        KT_CHECK_EQ((long long)party::members(nullptr, 4), 0ll);
        // unreadable middle member keeps alignment, others still read
        at<uint32_t>(g_servers[1], 0) = 0; g_vcalls = 0;
        KT_CHECK_EQ((long long)party::members(out, 4), 3ll);
        KT_CHECK(out[0].ok); KT_CHECK(!out[1].ok); KT_CHECK(!out[1].down); KT_CHECK(out[2].ok);
        KT_CHECK_EQ((unsigned long long)out[1].server, (unsigned long long)g_servers[1]);
        KT_CHECK_EQ((long long)g_vcalls, 4ll);
        // member with no server creature
        g_servers[1] = 0; g_clients[1] = 0x1099;
        KT_CHECK_EQ((long long)party::members(out, 4), 3ll);
        KT_CHECK(!out[1].ok); KT_CHECK_EQ((unsigned long long)out[1].server, 0ull); KT_CHECK_EQ((unsigned long long)out[1].client, 0x1099ull);
        // chain failures -> 0
        build(3); at<uint32_t>(b, 0x270) = 0; KT_CHECK_EQ((long long)party::members(out, 4), 0ll);
        build(0); KT_CHECK_EQ((long long)party::members(out, 4), 0ll);
        build(3); at<uint32_t>(cell, 0) = 0; KT_CHECK_EQ((long long)party::members(out, 4), 0ll);

        // ---- edge cases ----
        // (1) PartyGetAt returns 0 for the middle index: entry stays in place, ok=false, client 0, NO serverCreature call for it, neighbours read.
        party::g_member = countMember; party::g_serverCreature = countServer;
        build(3); mkCreature(0, 50, 100); mkCreature(1, 10, 80); mkCreature(2, 7, 7);
        g_clients[1] = 0; g_memberCalls = g_serverCalls = g_vcalls = 0;
        for (auto& o : out) o = party::Vitals();
        out[1].cur = 1234; out[1].server = 0xDEAD;   // stale data must be overwritten
        KT_CHECK_EQ((long long)party::members(out, 4), 3ll);          // full count kept, list not truncated
        KT_CHECK_EQ((long long)g_memberCalls, 3ll);
        KT_CHECK_EQ((long long)g_serverCalls, 2ll);                    // only members 0 and 2
        KT_CHECK(g_serverArgs[0] != 0 && g_serverArgs[1] != 0);
        KT_CHECK_EQ((unsigned long long)g_serverArgs[0], (unsigned long long)(0x1000 + 0x10 * 1));
        KT_CHECK_EQ((unsigned long long)g_serverArgs[1], (unsigned long long)(0x1000 + 0x10 * 3));
        KT_CHECK_EQ((long long)g_vcalls, 4ll);                         // no vital call for the missing member
        KT_CHECK(!out[1].ok); KT_CHECK(!out[1].down);
        KT_CHECK_EQ((unsigned long long)out[1].client, 0ull); KT_CHECK_EQ((unsigned long long)out[1].server, 0ull);
        KT_CHECK_EQ((long long)out[1].cur, 0ll); KT_CHECK_EQ((long long)out[1].max, 0ll);
        KT_CHECK(out[0].ok && out[2].ok); KT_CHECK_EQ((long long)out[2].cur, 7ll);
        // missing FIRST and LAST member too
        g_clients[1] = 0x1020; g_clients[0] = 0; g_clients[2] = 0; g_memberCalls = g_serverCalls = g_vcalls = 0;
        KT_CHECK_EQ((long long)party::members(out, 4), 3ll);
        KT_CHECK(!out[0].ok && !out[2].ok && out[1].ok);
        KT_CHECK_EQ((long long)g_serverCalls, 1ll); KT_CHECK_EQ((long long)g_vcalls, 2ll);

        // (2) cur < 0 / == 0 / == 1 through vitals()
        party::g_member = fakeMember; party::g_serverCreature = fakeServer;
        g_clients[0] = 0x1010; g_clients[2] = 0x1030;
        { party::Vitals w;
          g_fc[0].hp = -1;       KT_CHECK(party::vitals(g_servers[0], &w)); KT_CHECK(w.down); KT_CHECK_EQ((long long)w.cur, -1ll);
          g_fc[0].hp = -32768;   KT_CHECK(party::vitals(g_servers[0], &w)); KT_CHECK(w.down);
          g_fc[0].hp = 0;        KT_CHECK(party::vitals(g_servers[0], &w)); KT_CHECK(w.down);
          g_fc[0].hp = 1;        KT_CHECK(party::vitals(g_servers[0], &w)); KT_CHECK(!w.down);
          g_fc[0].hp = 100;      KT_CHECK(party::vitals(g_servers[0], &w)); KT_CHECK(!w.down);   // cur == max
          g_fc[0].hp = 500;      KT_CHECK(party::vitals(g_servers[0], &w)); KT_CHECK(!w.down);   // cur > max is not clamped
          g_fc[0].hp = -1; g_fc[0].maxhp = 0; KT_CHECK(!party::vitals(g_servers[0], &w)); KT_CHECK(!w.down);   // down needs ok
          g_fc[0].hp = 50; g_fc[0].maxhp = 100; }

        // (3) party shrinks between partyCount and the per-member reads: member() re-reads the count, so no PartyGetAt past the end
        party::g_member = shrinkMember;
        build(3); g_memberCalls = 0;
        KT_CHECK_EQ((long long)party::members(out, 4), 3ll);           // n was captured before the shrink (pinned)
        KT_CHECK_EQ((long long)at<int>(tbl, 0), 2ll);
        KT_CHECK_EQ((long long)g_memberCalls, 2ll);                    // member(2) never reached the engine
        KT_CHECK(out[0].ok); KT_CHECK(out[1].ok);
        KT_CHECK(!out[2].ok); KT_CHECK_EQ((unsigned long long)out[2].client, 0ull); KT_CHECK_EQ((unsigned long long)out[2].server, 0ull);
        // count drops to 0 / negative mid-flight: nothing reaches the engine
        party::g_member = countMember;
        for (int c : {0, -4}) {
            build(3); at<int>(tbl, 0) = c; g_memberCalls = 0;
            KT_CHECK_EQ((unsigned long long)party::member(tbl, 0), 0ull);
            KT_CHECK_EQ((unsigned long long)party::member(tbl, 2), 0ull);
            KT_CHECK_EQ((long long)g_memberCalls, 0ll);
        }
        // table pointer goes unreadable after the chain walk: count reads 0, no engine call
        build(3); g_memberCalls = 0; KT_CHECK_EQ((unsigned long long)party::member(0x20, 0), 0ull); KT_CHECK_EQ((long long)g_memberCalls, 0ll);

        party::g_member = sM; party::g_serverCreature = sS; party::g_callVital = sV;
    }

    // ---- addresses come from the K2Addr table (default g_* = "use the table") ----
    {
        auto sA = party::g_appGlobal; auto sG = party::g_getControlled; auto sM = party::g_member; auto sS = party::g_serverCreature;
        party::g_appGlobal = k2addr::UseTable;
        party::g_getControlled = party::useTableFn<party::GetControlledFn>();
        party::g_member = party::useTableFn<party::MemberFn>();
        party::g_serverCreature = party::useTableFn<party::ServerCreatureFn>();
        g_slotMax = 0x401000; g_slotCur = 0x402000;
        build(3); mkCreature(0, 50, 100);

        K2AddrTable t = kSteam;                       // a table pointing every Party address at the fakes / the arena
        t.AppGlobal = cell;
        t.GetControlledCreature = reinterpret_cast<uintptr_t>(&fakeGetControlled);
        t.PartyGetAt = reinterpret_cast<uintptr_t>(&fakeMember);
        t.MemberServerCreature = reinterpret_cast<uintptr_t>(&fakeServer);
        k2addr::setForTest(&t);
        party::Fail w = party::F_App; int n = 0;
        KT_CHECK_EQ((unsigned long long)party::controlledCreature(&w, &n), (unsigned long long)Creature);
        KT_CHECK_EQ((long long)w, (long long)party::F_None); KT_CHECK_EQ((long long)n, 3ll);
        KT_CHECK_EQ((unsigned long long)party::table(), (unsigned long long)tbl);
        KT_CHECK_EQ((unsigned long long)party::member(tbl, 0), (unsigned long long)g_clients[0]);
        KT_CHECK_EQ((unsigned long long)party::serverCreature(g_clients[0]), (unsigned long long)g_servers[0]);

        // a zero field = unknown: that helper returns 0 and calls nothing
        g_calls = 0;
        K2AddrTable noCtl = t; noCtl.GetControlledCreature = 0;
        k2addr::setForTest(&noCtl);
        KT_CHECK_EQ((unsigned long long)party::controlledCreature(), 0ull); KT_CHECK_EQ((long long)g_calls, 0ll);
        K2AddrTable noMem = t; noMem.PartyGetAt = 0; noMem.MemberServerCreature = 0;
        k2addr::setForTest(&noMem);
        KT_CHECK_EQ((unsigned long long)party::member(tbl, 0), 0ull);
        KT_CHECK_EQ((unsigned long long)party::serverCreature(g_clients[0]), 0ull);
        K2AddrTable noApp = t; noApp.AppGlobal = 0;
        k2addr::setForTest(&noApp);
        { party::Fail f = party::F_None; KT_CHECK_EQ((unsigned long long)party::table(&f), 0ull); KT_CHECK_EQ((long long)f, (long long)party::F_App); }
        KT_CHECK_EQ((long long)party::members(nullptr, 4), 0ll);

        // unknown exe (null table): inert end to end
        k2addr::setForTest(nullptr);
        { party::Fail f = party::F_None; int c = 55; KT_CHECK_EQ((unsigned long long)party::controlledCreature(&f, &c), 0ull);
          KT_CHECK_EQ((long long)f, (long long)party::F_App); KT_CHECK_EQ((long long)c, 55ll); }
        KT_CHECK_EQ((unsigned long long)party::member(tbl, 0), 0ull);
        KT_CHECK_EQ((unsigned long long)party::serverCreature(g_clients[0]), 0ull);
        party::Vitals vs[2]; KT_CHECK_EQ((long long)party::members(vs, 2), 0ll);
        KT_CHECK_EQ((long long)g_calls, 0ll);
        k2addr::clearForTest();

        party::g_appGlobal = sA; party::g_getControlled = sG; party::g_member = sM; party::g_serverCreature = sS;
    }

    KT_CHECK_EQ(party::failText(party::F_None), "ok");
    // (4) failText for out-of-range Fail values: always a non-null, non-empty string ("ok" default)
    {
        volatile int vals[] = { -1, -2147483647 - 1, 6, 7, 99, 2147483647 };
        for (int iv : vals) {
            const char* t = party::failText(static_cast<party::Fail>(iv));
            KT_CHECK(t != nullptr); KT_CHECK(t && t[0] != 0);
            KT_CHECK_EQ(t, "ok");
        }
        for (int i = party::F_None; i <= party::F_Count; ++i) { const char* t = party::failText(static_cast<party::Fail>(i)); KT_CHECK(t != nullptr && t[0]); }
        KT_CHECK_EQ(party::failText(party::F_Count), "party count 0");
    }
    KT_CHECK(party::failText(party::F_App) == party::failText(party::F_App));
    return kt::Report("party_test");
}
