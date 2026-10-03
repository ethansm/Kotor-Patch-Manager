// Host test of Party.h on a fake app -> a -> b -> table chain in a MAP_32BIT arena (doc 20 conventions).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "Party.h"
#include "GuiKit.h"
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

    KT_CHECK_EQ(party::failText(party::F_None), "ok");
    KT_CHECK(party::failText(party::F_App) == party::failText(party::F_App));
    return kt::Report("party_test");
}
