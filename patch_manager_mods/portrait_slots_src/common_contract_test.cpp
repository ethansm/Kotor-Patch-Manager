// common_contract_test.cpp -- Patches/Common CONTRACT test for portrait-slots (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on (PortraitSlotsProvider.cpp / PsDispatch.h call sites), not the headers' own behaviour (that is _shared_src/*_test.cpp).
// Every message starts with "portrait relies on: " so a failure names the consumer and the reason. Expected values are independent
// literals (doc 21b address map, the provider's comments, the hook tomls, GL constants), never read back from the header under test.
// ps_dispatch_test.cpp (gating, order) and ps_registry_test.cpp (registry edge cases) are NOT repeated here.
//
// Areas: (1) k2addr values + table selection, (2) saferead, (3) Effects.h plausibleCreature + the menu caller-frame read,
// (4) Party.h roster on a fake app -> a -> b -> table chain, (5) Gui2D.h panelRect / inMenuFrame / ClipScope / Gui2DScope / CapGuard
// (fake GL) in the exact scope order of runMenuOverlay. Host-only, deterministic, no game, no DLL.
// Host traps handled: the stub VirtualQuery says everything is readable (routed through fakeVQ below to fake holes), target
// pointers are modelled as uint32_t in a MAP_32BIT arena.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctype.h>

// ---- unreadable memory: SafeRead.h's VirtualQuery goes through this wrapper (as in companion_ai's contract test) ----
struct FakeRange { uintptr_t lo, hi; DWORD protect, state; };
static FakeRange g_fake[4];
static int g_fakeN = 0;
static auto g_realVQ = &VirtualQuery;
static SIZE_T fakeVQ(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) {
    SIZE_T r = g_realVQ(a, m, n);
    if (!r) return r;
    uintptr_t p = reinterpret_cast<uintptr_t>(a);
    if (p >= 0xFFFF0000ull) return 0;   // top of the 32-bit space is never committed (the stub would let a wrapped negative-offset read fault under -m32)
    if (p + m->RegionSize > 0xFFFF0000ull || p + m->RegionSize < p) m->RegionSize = 0xFFFF0000ull - p;   // and no default region may reach it (cached regions would answer first)
    for (int i = 0; i < g_fakeN; ++i) {
        const FakeRange& f = g_fake[i];
        if (p >= f.lo && p < f.hi) { m->BaseAddress = reinterpret_cast<void*>(f.lo); m->RegionSize = f.hi - f.lo; m->Protect = f.protect; m->State = f.state; return r; }
        if (p < f.lo && p + m->RegionSize > f.lo) m->RegionSize = f.lo - p;   // end the default region where a fake one starts
    }
    return r;
}
#define VirtualQuery fakeVQ
#include "SafeRead.h"

// ---- fake opengl32 for gui2d::CapGuard (the host stub has no module API; Gui2D.h resolves GL through these) ----
typedef void* HMODULE;
static bool g_haveGl = true, g_omitEnable = false;
static const unsigned kCaps[9] = { 0x0C11, 0x0B90, 0x0B71, 0x0B50, 0x0BE2, 0x0B60, 0x0DE1, 0x0B44, 0x0BC0 };   // GL_SCISSOR_TEST STENCIL DEPTH LIGHTING BLEND FOG TEXTURE_2D CULL_FACE ALPHA_TEST
static bool g_glOn[9];
static std::string g_ev;   // event trace: S snapshot/compare start, B begin, P push, x body, p pop, E end, e GL enable, d GL disable
static int glIdx(unsigned c) { for (int i = 0; i < 9; ++i) if (kCaps[i] == c) return i; return -1; }
static int glGetErrorF() { return 0; }
static void glGetIntegervF(unsigned, int*) {}
static void glGetBooleanvF(unsigned, unsigned char*) {}
static unsigned char glIsEnabledF(unsigned c) { if (c == kCaps[0]) g_ev += 'S'; int i = glIdx(c); return i >= 0 && g_glOn[i]; }
static void glReadPixelsF(int, int, int, int, unsigned, unsigned, void*) {}
static void glEnableF(unsigned c) { g_ev += 'e'; int i = glIdx(c); if (i >= 0) g_glOn[i] = true; }
static void glDisableF(unsigned c) { g_ev += 'd'; int i = glIdx(c); if (i >= 0) g_glOn[i] = false; }
static HMODULE GetModuleHandleA(const char* n) { return g_haveGl && !strcmp(n, "opengl32.dll") ? reinterpret_cast<HMODULE>(1) : nullptr; }
static void* GetProcAddress(HMODULE, const char* n) {
    if (!strcmp(n, "glGetError")) return reinterpret_cast<void*>(&glGetErrorF);
    if (!strcmp(n, "glGetIntegerv")) return reinterpret_cast<void*>(&glGetIntegervF);
    if (!strcmp(n, "glGetBooleanv")) return reinterpret_cast<void*>(&glGetBooleanvF);
    if (!strcmp(n, "glIsEnabled")) return reinterpret_cast<void*>(&glIsEnabledF);
    if (!strcmp(n, "glReadPixels")) return reinterpret_cast<void*>(&glReadPixelsF);
    if (!strcmp(n, "glEnable")) return g_omitEnable ? nullptr : reinterpret_cast<void*>(&glEnableF);
    if (!strcmp(n, "glDisable")) return reinterpret_cast<void*>(&glDisableF);
    return nullptr;
}

#include "K2Addr.h"
#include "Gui2D.h"
#include "Effects.h"
#include "Party.h"
#include "check.h"

#define HCHECK(why, expr) ::kt::Check("portrait relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("portrait relies on: " why " (line " KT_STR(__LINE__) ")", (long long)(a), (long long)(b))

// ============================================================ 1. k2addr values (compile time)
// Literals: doc 21b address map (Steam | GOG columns), provider comments (menu+0x2350, [callerEbp-0x38]), Effects/Gui2D header docs.
#define PADDR(tbl, name, val) static_assert(tbl.name == (val), "portrait relies on: " #tbl "." #name " == " #val " (doc 21b)")
// PortraitSlotsProvider.cpp:98-114 menu overlay: panel = slot - Menu_PortraitSlot, controls at panel + Menu_Char2/3, creature at [callerEbp + MenuCaller_Creature].
PADDR(kSteam, Menu_PortraitSlot, 0x2350); PADDR(kGog, Menu_PortraitSlot, 0x2350);
PADDR(kSteam, Menu_Char2, 0x3134);        PADDR(kGog, Menu_Char2, 0x3134);
PADDR(kSteam, Menu_Char3, 0x36DC);        PADDR(kGog, Menu_Char3, 0x36DC);
PADDR(kSteam, MenuCaller_Creature, -0x38); PADDR(kGog, MenuCaller_Creature, -0x38);   // negative: a frame LOCAL below the caller's ebp
// PortraitSlotsProvider.cpp:101 roster chain (Party.h) and :102 plausibleCreature (Effects.h).
PADDR(kSteam, AppGlobal, 0x00A1B4A4);              PADDR(kGog, AppGlobal, 0x00A11C04);
PADDR(kSteam, PartyGetAt, 0x007E5DA0);             PADDR(kGog, PartyGetAt, 0x004789D0);
PADDR(kSteam, MemberServerCreature, 0x0077D800);   PADDR(kGog, MemberServerCreature, 0x00429780);
PADDR(kSteam, Creature_EffectArray, 0x148);        PADDR(kGog, Creature_EffectArray, 0x148);
PADDR(kSteam, Creature_EffectCount, 0x14C);        PADDR(kGog, Creature_EffectCount, 0x14C);
// PortraitSlotsProvider.cpp:115-125 menu frame 2D calls (Gui2D.h).
PADDR(kSteam, BeginGui2D, 0x004792C0);  PADDR(kGog, BeginGui2D, 0x008CFBD0);
PADDR(kSteam, EndGui2D, 0x004793F0);    PADDR(kGog, EndGui2D, 0x008CFD00);
PADDR(kSteam, PushClipRect, 0x00479460); PADDR(kGog, PushClipRect, 0x008CFD70);
PADDR(kSteam, PopClipRect, 0x004799C0); PADDR(kGog, PopClipRect, 0x008D02D0);
PADDR(kSteam, PanelRect, 0x00410020);   PADDR(kGog, PanelRect, 0x0090E4A0);
PADDR(kSteam, ClipDepth, 0x00A309C0);   PADDR(kGog, ClipDepth, 0x00A32B90);
PADDR(kSteam, ClipNoFillColor, 0x00A1BA3C); PADDR(kGog, ClipNoFillColor, 0x00A768A4);

// ============================================================ arena + fakes
static uint8_t* g_arena = nullptr; static size_t g_used = 0x10000; constexpr size_t ArenaSize = 1u << 20, Page = 0x1000;
// layout: [0x2000,0x3000) NOACCESS, [0x3000,0x4000) not committed (both faked), bump allocator from 0x10000
static const size_t kBad = 0x2000, kFree = 0x3000;
static uintptr_t A(size_t off) { return reinterpret_cast<uintptr_t>(g_arena) + off; }
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); }
    uintptr_t a = A(g_used); memset(g_arena + g_used, 0, n); g_used += n; return a;
}
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
static void freshScope() { saferead::beginScope(); }

static const char* kSteamSha = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF";   // kotor2-steam-aspyr.hooks.toml target_versions
static const char* kGogSha = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914";     // kotor2-gog-aspyr.hooks.toml target_versions

// ============================================================ 1b. table selection (PortraitSlotsProvider.cpp:147/162 k2addr::get)
static void k2addr_contract() {
    // :147 / :162: a null table is "unknown exe" -> no snapshot, no dispatch, one WARN. Only the two shipped hashes select a table.
    HCHECK("the Steam hook toml's target sha selects kSteam (the table the Steam hooks were written for)", k2addr::selectBySha(kSteamSha) == &kSteam);
    HCHECK("the GOG hook toml's target sha selects kGog", k2addr::selectBySha(kGogSha) == &kGog);
    std::string lower = kSteamSha; for (char& c : lower) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    HCHECK("sha matching is case-insensitive (KPM writes lowercase hex)", k2addr::selectBySha(lower.c_str()) == &kSteam);
    HCHECK("an unknown sha selects NO table (never a guessed address)", k2addr::selectBySha("0000") == nullptr && k2addr::selectBySha("6A522E71") == nullptr);
    HCHECK("no sha at all (no env var) keeps the Steam table", k2addr::selectBySha(nullptr) == &kSteam && k2addr::selectBySha("") == &kSteam);
    k2addr::setForTest(nullptr);
    HCHECK("setForTest(nullptr) makes k2addr::get() null (the provider's inert path)", k2addr::get() == nullptr);
    k2addr::setForTest(&kGog);
    HCHECK("setForTest(&kGog) makes k2addr::get() the GOG table", k2addr::get() == &kGog);
    k2addr::setForTest(&kSteam);
    HCHECK("setForTest(&kSteam) makes k2addr::get() the Steam table", k2addr::get() == &kSteam);
}

// ============================================================ 2. saferead
static void saferead_contract() {
    g_fakeN = 0;
    g_fake[g_fakeN++] = { A(kBad), A(kBad) + Page, PAGE_NOACCESS, MEM_COMMIT };
    g_fake[g_fakeN++] = { A(kFree), A(kFree) + Page, 0, 0 };
    freshScope();
    // PortraitSlotsProvider.cpp:146 / :160: beginScope at every hook entry. Engine code between hook calls can free memory (a stale
    // slot's creature pointer), so a region cached in the previous hook must not answer this one.
    uintptr_t obj = arenaAlloc(0x200);
    HCHECK("setup: readable creature memory reads true and is cached", k2fx::plausibleCreature(obj));
    g_stubVirtualQueryAll = false;   // "freed behind our back"
    HCHECK("within the same scope the cached region still answers (no VirtualQuery per read)", k2fx::plausibleCreature(obj));
    freshScope();
    HCHECK("beginScope drops cached regions: freed creature memory is NOT plausible in the next hook", !k2fx::plausibleCreature(obj));
    g_stubVirtualQueryAll = true; freshScope();
    HCHECK("beginScope then readable again -> plausible", k2fx::plausibleCreature(obj));

    // :166 k2fx::rd(callerEbp, T->MenuCaller_Creature, &c): `int c = 0` pre-seeded, offset is NEGATIVE, a failed read keeps the 0
    // (which plausibleCreature then rejects, clearing g_menuSlot). Decoys on every neighbouring word catch a wrong offset.
    uintptr_t frame = arenaAlloc(0x100) + 0x80;
    at<uint32_t>(frame, -0x3C) = 0xD0D0D0D0; at<uint32_t>(frame, -0x38) = static_cast<uint32_t>(obj); at<uint32_t>(frame, -0x34) = 0xD1D1D1D1;
    at<uint32_t>(frame, 0x38) = 0xD2D2D2D2; at<uint32_t>(frame, 0) = 0xD3D3D3D3;
    int c = 0;
    HCHECK("rd(callerEbp, -0x38) succeeds on a readable caller frame", k2fx::rd(frame, kSteam.MenuCaller_Creature, &c));
    HEQ("rd with the NEGATIVE MenuCaller_Creature offset reads [callerEbp-0x38] (not +0x38, not ebp itself)", c, static_cast<int>(obj));
    HCHECK("the read creature is then plausible (menu slot is recorded)", k2fx::plausibleCreature(static_cast<uintptr_t>(c)));
    c = 0;
    HCHECK("rd on callerEbp 0 returns false (no wrap through the negative offset)", !k2fx::rd(0, kSteam.MenuCaller_Creature, &c));
    HEQ("rd failure on callerEbp 0 leaves the seeded 0", c, 0);
    HCHECK("rd on a tiny callerEbp (0x20 - 0x38 wraps) returns false", !k2fx::rd(0x20, kSteam.MenuCaller_Creature, &c));
    HCHECK("rd on a caller frame whose local sits in an unreadable page returns false", !k2fx::rd(A(kBad) + 0x38, kSteam.MenuCaller_Creature, &c));
    HCHECK("rd on a local that straddles into an unreadable page returns false", !k2fx::rd(A(kBad) - 2 + 0x38, kSteam.MenuCaller_Creature, &c));
    HEQ("every failed rd leaves the seeded 0 (-> plausibleCreature(0) false -> g_menuSlot cleared)", c, 0);
    HCHECK("plausibleCreature(0) is false", !k2fx::plausibleCreature(0));
}

// ============================================================ 3. Effects.h plausibleCreature (:102, :167)
static void effects_contract() {
    freshScope();
    k2addr::setForTest(&kSteam);
    // The check reads count at creature+0x14C and array at +0x148 and trusts the creature when the count is 0..512 (the array is
    // not dereferenced). A fresh creature with no effects must still be accepted: the menu / roster must keep its overlay.
    auto mk = [&](int arr, int cnt) { uintptr_t c = arenaAlloc(0x200); at<int>(c, 0x148) = arr; at<int>(c, 0x14C) = cnt; return c; };
    HCHECK("a creature with 0 effects (array null) is plausible", k2fx::plausibleCreature(mk(0, 0)));
    HCHECK("a creature with 3 effects is plausible", k2fx::plausibleCreature(mk(0x12345678, 3)));
    HCHECK("count 512 (the maximum) is plausible", k2fx::plausibleCreature(mk(0, 512)));
    HCHECK("count 513 is NOT plausible (freed / wrong object)", !k2fx::plausibleCreature(mk(0, 513)));
    HCHECK("a negative count is NOT plausible", !k2fx::plausibleCreature(mk(0, -1)));
    HCHECK("the COUNT is read at +0x14C, not +0x148 (a big value in the array slot does not reject)", k2fx::plausibleCreature(mk(600, 3)));
    HCHECK("the COUNT is read at +0x14C, not +0x148 (a small array value does not accept a bad count)", !k2fx::plausibleCreature(mk(3, 600)));
    HCHECK("a null creature is not plausible", !k2fx::plausibleCreature(0));
    HCHECK("a creature in an unreadable page is not plausible", !k2fx::plausibleCreature(A(kBad)));
    HCHECK("a creature whose +0x14C lies in an unreadable page is not plausible (both fields are read)", !k2fx::plausibleCreature(A(kBad) - 0x14C));
    HCHECK("a creature in an uncommitted page is not plausible", !k2fx::plausibleCreature(A(kFree) + 0x10));
    HCHECK("a tiny non-null pointer (0x20) is not plausible", !k2fx::plausibleCreature(0x20));
    K2AddrTable noOffs = kSteam; noOffs.Creature_EffectArray = 0;
    static K2AddrTable s_noOffs; s_noOffs = noOffs; k2addr::setForTest(&s_noOffs);
    HCHECK("a table without the effect-array offset (unknown field) rejects every creature (inert, no guess)", !k2fx::plausibleCreature(mk(0, 0)));
    s_noOffs = kSteam; s_noOffs.Creature_EffectCount = 0;
    HCHECK("a table without the effect-count offset rejects every creature", !k2fx::plausibleCreature(mk(0, 0)));
    k2addr::setForTest(nullptr);
    HCHECK("an unknown exe (null table) rejects every creature", !k2fx::plausibleCreature(mk(0, 0)));
    k2addr::setForTest(&kSteam);
}

// ============================================================ 4. Party.h roster (:100-103 rosterCreature)
static uintptr_t g_cl[4], g_sv[4];
static int g_mCalls = 0, g_mIdx[8], g_sCalls = 0; static void* g_mTbl = nullptr; static uintptr_t g_sArg[8];
static uintptr_t __thiscall fakeMember(void* t, int i) { g_mTbl = t; if (g_mCalls < 8) g_mIdx[g_mCalls] = i; ++g_mCalls; return g_cl[i]; }
static uintptr_t __thiscall fakeServer(void* m) {
    if (g_sCalls < 8) g_sArg[g_sCalls] = reinterpret_cast<uintptr_t>(m);
    ++g_sCalls;
    for (int i = 0; i < 4; ++i) if (g_cl[i] == reinterpret_cast<uintptr_t>(m)) return g_sv[i];
    return 0;
}
static uintptr_t cell, app, pa, pb, tbl;
static void build(int count) {
    at<uint32_t>(cell, 0) = static_cast<uint32_t>(app); at<uint32_t>(app, 4) = static_cast<uint32_t>(pa); at<uint32_t>(pa, 4) = static_cast<uint32_t>(pb);
    at<uint32_t>(pb, 0x270) = static_cast<uint32_t>(tbl); at<int>(tbl, 0) = count;
    g_mCalls = 0; g_sCalls = 0; g_mTbl = nullptr;
}
static void mkMember(int i) { g_cl[i] = 0x1000 + 0x10 * (i + 1); g_sv[i] = arenaAlloc(0x200); }
// PortraitSlotsProvider.cpp:101-102, verbatim shape.
static int roster(int idx) {
    uintptr_t c = party::serverCreature(party::member(party::table(), idx));
    return k2fx::plausibleCreature(c) ? static_cast<int>(c) : 0;
}

static void party_contract() {
    freshScope();
    k2addr::setForTest(&kSteam);
    cell = arenaAlloc(16); app = arenaAlloc(16); pa = arenaAlloc(16); pb = arenaAlloc(0x300); tbl = arenaAlloc(16);
    party::g_appGlobal = cell; party::g_member = fakeMember; party::g_serverCreature = fakeServer;
    for (int i = 0; i < 3; ++i) mkMember(i);

    // :111-112 companions[0] / [1] = party positions 1 / 2 (the menu fills LBL_CHAR2/3 from them; position 0 is the controlled creature).
    build(3);
    HEQ("roster(1) is the server creature of party position 1", roster(1), static_cast<int>(g_sv[1]));
    HEQ("roster(2) is the server creature of party position 2", roster(2), static_cast<int>(g_sv[2]));
    HCHECK("the two companions are distinct creatures", g_sv[1] != g_sv[2] && roster(1) != roster(2));
    build(3); roster(1); roster(2);
    HCHECK("the engine accessor is asked for positions 1 then 2 (not 0 and 1)", g_mCalls == 2 && g_mIdx[0] == 1 && g_mIdx[1] == 2);
    HEQ("PartyGetAt is called on the party table found through app -> +4 -> +4 -> +0x270", reinterpret_cast<uintptr_t>(g_mTbl), tbl);
    HCHECK("the server-creature accessor gets the CLIENT member returned for that position", g_sCalls == 2 && g_sArg[0] == g_cl[1] && g_sArg[1] == g_cl[2]);

    // A smaller party: positions past the count resolve to 0 WITHOUT an engine call (a bad PartyGetAt index would crash the game).
    build(2);
    HEQ("a 2-member party: position 1 resolves", roster(1), static_cast<int>(g_sv[1]));
    g_mCalls = g_sCalls = 0;
    HEQ("a 2-member party: position 2 resolves to 0", roster(2), 0);
    HCHECK("a 2-member party: position 2 makes no engine call at all", g_mCalls == 0 && g_sCalls == 0);
    build(1);
    HCHECK("the Exile alone: companions 1 and 2 are both 0", roster(1) == 0 && roster(2) == 0);
    HCHECK("the Exile alone: no engine call", g_mCalls == 0 && g_sCalls == 0);
    build(0);
    HCHECK("an empty party count (between areas) resolves to 0, no engine call", roster(1) == 0 && roster(2) == 0 && g_mCalls == 0 && g_sCalls == 0);
    build(-1);
    HCHECK("a corrupt negative count resolves to 0, no engine call", roster(1) == 0 && roster(2) == 0 && g_mCalls == 0 && g_sCalls == 0);

    // A broken chain link (HUD torn down / load): table() is 0 -> member() and serverCreature() are 0 and nothing is called.
    for (int level = 0; level < 4; ++level) {
        build(3);
        if (level == 0) at<uint32_t>(cell, 0) = 0; else if (level == 1) at<uint32_t>(app, 4) = 0; else if (level == 2) at<uint32_t>(pa, 4) = 0; else at<uint32_t>(pb, 0x270) = 0;
        HCHECK("a broken app-chain link: table() is 0", party::table() == 0);
        HCHECK("a broken app-chain link: roster is empty and no engine call is made", roster(1) == 0 && roster(2) == 0 && g_mCalls == 0 && g_sCalls == 0);
    }
    build(3);
    HEQ("an intact chain: table() is the party table", party::table(), tbl);
    HEQ("member() on a null table is 0", party::member(0, 1), 0);
    g_mCalls = g_sCalls = 0;
    HCHECK("serverCreature(0) is 0 and never calls the engine (a null client member)", party::serverCreature(0) == 0 && g_sCalls == 0);
    HCHECK("member() on a null table never calls the engine", g_mCalls == 0);

    // The accessors return a value the plausibility check then judges: a stale / freed / foreign server pointer yields 0.
    build(3);
    at<int>(g_sv[1], 0x14C) = 600;
    HEQ("a server creature with an implausible effect count is dropped (roster 0)", roster(1), 0);
    HEQ("... while its sibling still resolves", roster(2), static_cast<int>(g_sv[2]));
    at<int>(g_sv[1], 0x14C) = 0;
    uintptr_t keep = g_sv[1]; g_sv[1] = A(kBad);
    HEQ("a server pointer into unreadable memory is dropped (roster 0)", roster(1), 0);
    g_sv[1] = 0;
    HEQ("a member whose server creature is null is dropped (roster 0)", roster(1), 0);
    g_sv[1] = keep;
    HEQ("restored: position 1 resolves again", roster(1), static_cast<int>(keep));

    // Unknown exe: every table-resolved engine function is inert (null), so nothing is ever called at address 0.
    k2addr::setForTest(nullptr);
    party::g_appGlobal = k2addr::UseTable;
    party::g_member = party::useTableFn<party::MemberFn>(); party::g_serverCreature = party::useTableFn<party::ServerCreatureFn>();
    party::Fail why = party::F_None;
    HCHECK("unknown exe: table() is 0 (app global unknown)", party::table(&why) == 0 && why == party::F_App);
    HEQ("unknown exe: member() is 0 even for a readable table", party::member(tbl, 1), 0);
    HEQ("unknown exe: serverCreature() is 0 for a non-null member", party::serverCreature(0x1010), 0);
    HCHECK("unknown exe: the roster is empty", roster(1) == 0 && roster(2) == 0);
    k2addr::setForTest(&kSteam);
}

// ============================================================ 5. Gui2D.h (:115-125 runMenuOverlay)
struct PushCall { int x, y, w, h; const void* color; int flag; float alpha; };
static PushCall g_pc; static int g_pushes = 0, g_pops = 0, g_pushRet = 1, g_panelCalls = 0, g_panelOut[4]; static uintptr_t g_panelSeen = 0;
static bool g_endToggles = false;
static void glReset() {   // scissor + texture2d on, everything else off: a plausible "before the overlay" state
    for (int i = 0; i < 9; ++i) g_glOn[i] = false;
    g_glOn[0] = true; g_glOn[6] = true;
}
static void installFakes() {
    gui2d::g_g2d.begin = [] { g_ev += 'B'; };
    gui2d::g_g2d.end = [] {
        g_ev += 'E';
        if (g_endToggles) { g_glOn[0] = false; g_glOn[3] = true; g_glOn[4] = true; }   // what EndGui2D / our flushes leave behind: scissor off, GL_LIGHTING + GL_BLEND on
    };
    gui2d::g_g2d.push = [](int x, int y, int w, int h, const void* c, int f, float a) { ++g_pushes; g_pc = { x, y, w, h, c, f, a }; g_ev += 'P'; return g_pushRet; };
    gui2d::g_g2d.pop = [] { ++g_pops; g_ev += 'p'; };
    gui2d::g_g2d.panelRect = [](uintptr_t p, int* r) { ++g_panelCalls; g_panelSeen = p; for (int i = 0; i < 4; ++i) r[i] = g_panelOut[i]; };
}
// PortraitSlotsProvider.cpp:121-124, same declaration order: CapGuard, Gui2DScope, ClipScope, then the dispatch under `if (clip)`.
static void overlayShape(const int r[4]) {
    gui2d::CapGuard capGuard;
    gui2d::Gui2DScope batch;
    gui2d::ClipScope clip(r[0], r[1], r[2], r[3]);
    if (clip) g_ev += 'x';
}
static void resetTrace() { g_ev.clear(); g_pushes = g_pops = g_panelCalls = 0; g_pushRet = 1; g_endToggles = false; g_pc = {}; glReset(); gui2d::g_gl = gui2d::GlFns{}; g_haveGl = true; g_omitEnable = false; }

static void gui2d_contract() {
    freshScope();
    k2addr::setForTest(&kSteam);

    // ---- default engine behaviour on an unknown exe / unknown field: nothing is called, neutral results (never a call into address 0) ----
    {
        gui2d::Engine fresh;
        static K2AddrTable s_t;
        k2addr::setForTest(nullptr);
        int r[4] = { 7, 7, 7, 7 };
        fresh.begin(); fresh.end(); fresh.pop();   // would crash the host if they called address 0
        HEQ("unknown exe: default push returns 0 (nothing pushed)", fresh.push(1, 2, 3, 4, nullptr, 0, 1.0f), 0);
        fresh.panelRect(0x1234, r);
        HCHECK("unknown exe: default panelRect writes nothing", r[0] == 7 && r[1] == 7 && r[2] == 7 && r[3] == 7);
        HCHECK("unknown exe: panelRect reports a bad rect (overlay skipped, not drawn at 0,0)", !gui2d::panelRect(0x1234, r) && r[0] == 0 && r[1] == 0 && r[2] == 0 && r[3] == 0);
        HCHECK("unknown exe: inMenuFrame is false (treated as the HUD frame)", !gui2d::inMenuFrame());
        s_t = kSteam; s_t.PanelRect = 0; k2addr::setForTest(&s_t);
        fresh.panelRect(0x1234, r);
        HCHECK("a table with PanelRect == 0: default panelRect does not call address 0", gui2d::panelRect(0x1234, r) == false);
        s_t = kSteam; s_t.PushClipRect = 0;
        HEQ("a table with PushClipRect == 0: default push returns 0", fresh.push(1, 2, 3, 4, nullptr, 0, 1.0f), 0);
        s_t = kSteam; s_t.ClipDepth = 0;
        HCHECK("a table with ClipDepth == 0: inMenuFrame is false", !gui2d::inMenuFrame());
        k2addr::setForTest(&kSteam);
    }
    installFakes();
    gui2d::g_g2d.clipDepthAddr = k2addr::UseTable; gui2d::g_g2d.clipNoFill = nullptr;

    // ---- :163 inMenuFrame: low 16 bits of the clip stack index; 0 = HUD frame, 0xFFFF = menu frame after the menu closed its batch ----
    {
        uintptr_t cd = arenaAlloc(16);
        gui2d::g_g2d.clipDepthAddr = cd;
        at<uint32_t>(cd, 0) = 0;           HCHECK("clip index 0 is the HUD frame", !gui2d::inMenuFrame());
        at<uint32_t>(cd, 0) = 0xFFFF;      HCHECK("clip index 0xFFFF is a menu frame", gui2d::inMenuFrame());
        at<uint32_t>(cd, 0) = 1;           HCHECK("any non-zero low 16 bits is a menu frame", gui2d::inMenuFrame());
        at<uint32_t>(cd, 0) = 0x10000;     HCHECK("only the LOW 16 bits count (0x10000 is a HUD frame)", !gui2d::inMenuFrame());
        at<uint32_t>(cd, 0) = 0xABCD0000u; HCHECK("garbage in the high half does not make a menu frame", !gui2d::inMenuFrame());
        at<uint32_t>(cd, 0) = 0xABCDFFFFu; HCHECK("garbage in the high half does not hide a menu frame", gui2d::inMenuFrame());
        gui2d::g_g2d.clipDepthAddr = A(kBad);
        HCHECK("an unreadable clip-index address reads as the HUD frame (no overlay, no crash)", !gui2d::inMenuFrame());
        gui2d::g_g2d.clipDepthAddr = 0;
        HCHECK("a zero clip-index address reads as the HUD frame", !gui2d::inMenuFrame());
        gui2d::g_g2d.clipDepthAddr = k2addr::UseTable;
    }

    // ---- :115 panelRect: out array zeroed first; false on a degenerate rect (0, negative, > 8192); the engine's values are still left in r for the WARN ----
    {
        int r[4] = { 9, 9, 9, 9 };
        g_panelOut[0] = 10; g_panelOut[1] = 20; g_panelOut[2] = 300; g_panelOut[3] = 200; g_panelCalls = 0;
        HCHECK("a sane panel rect returns true", gui2d::panelRect(0x5000, r));
        HCHECK("panelRect returns x,y,w,h in that order", r[0] == 10 && r[1] == 20 && r[2] == 300 && r[3] == 200);
        HEQ("panelRect asks the engine once", g_panelCalls, 1);
        HEQ("panelRect hands the panel pointer to the engine", g_panelSeen, 0x5000);
        g_panelOut[0] = -10; g_panelOut[1] = -20;
        HCHECK("a negative origin is fine (panels are placed off-center)", gui2d::panelRect(0x5000, r) && r[0] == -10 && r[1] == -20);
        g_panelOut[0] = 0; g_panelOut[1] = 0; g_panelOut[2] = 1; g_panelOut[3] = 1;
        HCHECK("a 1x1 rect is valid (lower bound)", gui2d::panelRect(0x5000, r));
        g_panelOut[2] = 8192; g_panelOut[3] = 8192;
        HCHECK("an 8192x8192 rect is valid (upper bound)", gui2d::panelRect(0x5000, r));
        g_panelOut[2] = 8193; g_panelOut[3] = 100;
        HCHECK("width 8193 is rejected", !gui2d::panelRect(0x5000, r));
        HCHECK("a rejected rect keeps the engine's w,h for the provider's log line (:116-118 sig, WARN text)", r[2] == 8193 && r[3] == 100);
        g_panelOut[2] = 100; g_panelOut[3] = 8193; HCHECK("height 8193 is rejected", !gui2d::panelRect(0x5000, r));
        g_panelOut[2] = 0; g_panelOut[3] = 100;     HCHECK("width 0 is rejected", !gui2d::panelRect(0x5000, r));
        g_panelOut[2] = 100; g_panelOut[3] = 0;     HCHECK("height 0 is rejected", !gui2d::panelRect(0x5000, r));
        g_panelOut[2] = -5; g_panelOut[3] = 100;    HCHECK("a negative width is rejected", !gui2d::panelRect(0x5000, r));
        g_panelOut[2] = 100; g_panelOut[3] = -5;    HCHECK("a negative height is rejected", !gui2d::panelRect(0x5000, r));
        int pre[4] = { 9, 9, 9, 9 };
        gui2d::g_g2d.panelRect = [](uintptr_t, int*) {};   // an engine call that writes nothing
        HCHECK("an engine that writes nothing yields false", !gui2d::panelRect(0x5000, pre));
        HCHECK("panelRect zeroes the out array BEFORE the call (the provider logs stale garbage otherwise)", pre[0] == 0 && pre[1] == 0 && pre[2] == 0 && pre[3] == 0);
        installFakes();
    }

    // ---- :123-124 ClipScope: pushes (x,y,w,h) with the engine no-fill colour, flag 0, alpha 1; pops exactly once and only if pushed ----
    {
        gui2d::g_g2d.clipNoFill = nullptr; k2addr::setForTest(&kSteam);
        resetTrace();
        { gui2d::ClipScope c(10, 20, 300, 200); HCHECK("ClipScope is true when the engine pushed", static_cast<bool>(c)); HEQ("no pop while the scope is open", g_pops, 0); }
        HCHECK("ClipScope pushes x,y,w,h in order", g_pc.x == 10 && g_pc.y == 20 && g_pc.w == 300 && g_pc.h == 200);
        HEQ("ClipScope pushes with the engine no-fill colour address (clip only, no fill)", reinterpret_cast<uintptr_t>(g_pc.color), 0xA1BA3C);
        HCHECK("ClipScope pushes flag 0 and alpha 1", g_pc.flag == 0 && g_pc.alpha == 1.0f);
        HCHECK("ClipScope pops exactly once when the push succeeded", g_pushes == 1 && g_pops == 1);
        resetTrace(); g_pushRet = 0;
        { gui2d::ClipScope c(10, 20, 300, 200); HCHECK("ClipScope is false when the engine refused the push (dispatch skipped)", !static_cast<bool>(c)); }
        HCHECK("ClipScope never pops a push that failed (the clip stack would underflow)", g_pushes == 1 && g_pops == 0);
        resetTrace();
        static K2AddrTable s_t; s_t = kSteam; s_t.ClipNoFillColor = 0; k2addr::setForTest(&s_t);
        { gui2d::ClipScope c(10, 20, 300, 200); HCHECK("an unknown no-fill colour address: ClipScope is false", !static_cast<bool>(c)); }
        HCHECK("an unknown no-fill colour address: the engine push is never called, nothing is popped", g_pushes == 0 && g_pops == 0);
        k2addr::setForTest(nullptr);
        { gui2d::ClipScope c(1, 2, 3, 4); HCHECK("an unknown exe: ClipScope is false", !static_cast<bool>(c)); }
        HCHECK("an unknown exe: no push, no pop", g_pushes == 0 && g_pops == 0);
        k2addr::setForTest(&kSteam);
    }

    // ---- :121-124 the whole overlay scope: GL snapshot -> BeginGui2D -> clip -> body -> pop -> EndGui2D -> GL caps restored ----
    {
        const int rect[4] = { 5, 6, 700, 400 };
        resetTrace(); g_endToggles = true;
        overlayShape(rect);
        HCHECK("order: snapshot, begin, push, body, pop, end, THEN the cap restore (disable LIGHTING, disable BLEND, re-enable SCISSOR)", g_ev == "SBPxpESedd");
        HCHECK("CapGuard restores every cap the batch toggled (scissor on, lighting off, blend off)", g_glOn[0] && !g_glOn[3] && !g_glOn[4]);
        bool rest = true; for (int i = 0; i < 9; ++i) rest &= g_glOn[i] == (i == 0 || i == 6);
        HCHECK("after the scope the GL state equals the state before it (all 9 caps)", rest);
        HCHECK("the pushed clip is the panel rect (x,y,w,h)", g_pc.x == 5 && g_pc.y == 6 && g_pc.w == 700 && g_pc.h == 400);

        resetTrace(); g_endToggles = false;
        overlayShape(rect);
        HCHECK("untouched caps are not 'restored' (no GL enable/disable call when nothing changed)", g_ev == "SBPxpES");

        resetTrace(); g_pushRet = 0; g_endToggles = true;
        overlayShape(rect);
        HCHECK("a refused clip skips the body, never pops, but still ends the batch and restores GL", g_ev == "SBPESedd");
        HCHECK("the refused-clip path leaves GL as it was before", g_glOn[0] && !g_glOn[3] && !g_glOn[4] && g_glOn[6]);

        // Gui2DScope: exactly one begin and one end, both always (batch must be balanced even when the body is skipped).
        resetTrace();
        { gui2d::Gui2DScope b; HCHECK("Gui2DScope begins on construction", g_ev == "B"); }
        HCHECK("Gui2DScope ends on destruction", g_ev == "BE");

        // CapGuard without a usable GL (Wine without opengl32 loaded yet, or a driver missing glEnable): inert, never crashes, restores nothing.
        resetTrace(); g_haveGl = false; g_endToggles = true;
        overlayShape(rect);
        HCHECK("no opengl32 module: CapGuard is inert and the overlay still runs", g_ev == "BPxpE");
        HCHECK("no opengl32 module: caps are left as the batch set them (nothing to restore with)", !g_glOn[0] && g_glOn[3]);
        resetTrace(); g_omitEnable = true; g_endToggles = true;
        overlayShape(rect);
        HCHECK("glEnable missing: CapGuard is inert (no half restore) and the overlay still runs", g_ev == "BPxpE");
        resetTrace();
    }
}

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;   // every arena address reads as committed readable memory; the faked holes and < 0x10000 stay unreadable
    k2addr::setForTest(&kSteam);    // independent of KOTOR_VERSION_SHA in the environment
    k2addr_contract();
    saferead_contract();
    effects_contract();
    party_contract();
    gui2d_contract();
    return kt::Report("common_contract_test (portrait_slots)");
}
