// common_contract_test.cpp -- Patches/Common CONTRACT test for the Shield Bar (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on (ShieldBar.cpp call sites), not the headers' own behaviour (that is _shared_src/{k2addr,effects,gui2d,guikit,saferead}_test.cpp
// and portrait_slots_src/ps_*_test.cpp). Every message starts with "shield relies on: " so a failure names the consumer and the reason.
// Built against ../_shared_src (see test.toml), so ANY edit to SafeRead.h / K2Addr.h / Effects.h / Gui2D.h / GuiKit.h / PortraitSlots.h
// re-runs this test: a Common change that would break the shield bar fails here first.
//
// Expected values are independent literals: ShieldBar.cpp comments, ShieldLogic.h, manifest / hooks tomls, doc 21b (GOG column), the
// real GL enum values. Never read back from the header under test.
//
// Host traps (see companion_ai_src/common_contract_test.cpp): the host stub's VirtualQuery reports every address readable, so SafeRead.h's
// VirtualQuery is routed through a local wrapper that can fake a NOACCESS page; engine 32-bit pointers are written as uint32_t (the arena is
// MAP_32BIT) and every arena access goes through memcpy so UBSan sees no misaligned load.
//
// Areas: (1) K2Addr table values + selection, (2) SafeRead, (3) Effects.h pool decode, (4) Gui2D clip/draw/scale + CapGuard on a fake GL,
// (5) GuiKit imageFor, (6) PortraitSlots k2ps::Client on a fake provider, (7) StrFmt appendf, (8) k2addr::featureOk + gui2d::vfillClip(int). Host-only, deterministic, no game, no DLL.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <vector>

struct FakeRange { uintptr_t lo, hi; DWORD protect, state; };
static FakeRange g_fake[4];
static int g_fakeN = 0;
static auto g_realVQ = &VirtualQuery;
static SIZE_T fakeVQ(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) {
    SIZE_T r = g_realVQ(a, m, n);
    if (!r) return r;
    uintptr_t p = reinterpret_cast<uintptr_t>(a);
    for (int i = 0; i < g_fakeN; ++i) {
        const FakeRange& f = g_fake[i];
        if (p >= f.lo && p < f.hi) { m->BaseAddress = reinterpret_cast<void*>(f.lo); m->RegionSize = f.hi - f.lo; m->Protect = f.protect; m->State = f.state; return r; }
        if (p < f.lo && p + m->RegionSize > f.lo) m->RegionSize = f.lo - p;   // end the default region where a fake one starts
    }
    return r;
}
#define VirtualQuery fakeVQ
#include "SafeRead.h"

// Gui2D.h (no GUI2D_HOST: CapGuard / gl() are what ShieldBar uses) needs three Win32 symbols the host stub lacks. They are faked here so
// gui2d::gl() can be driven: the module lookup and each glXxx proc name are observable.
typedef void* HMODULE;
typedef void* FARPROC;
static bool g_haveGlModule = true;
static const char* g_missingProc = nullptr;
static int g_gmhCalls = 0, g_gpaCalls = 0;
static int fGetError() { return 0; }
static void fGetIntegerv(unsigned, int*) {}
static void fGetBooleanv(unsigned, unsigned char*) {}
static bool g_capOn[9];
static const unsigned kCaps[9] = { 0x0C11, 0x0B90, 0x0B71, 0x0B50, 0x0BE2, 0x0B60, 0x0DE1, 0x0B44, 0x0BC0 };   // real GL enums: scissor stencil depth lighting blend fog tex2d cull alpha-test
static int capIdx(unsigned cap) { for (int i = 0; i < 9; ++i) if (kCaps[i] == cap) return i; return -1; }
static int g_enN = 0, g_disN = 0; static unsigned g_enLog[64], g_disLog[64];
static unsigned char fIsEnabled(unsigned cap) { int i = capIdx(cap); return (i >= 0 && g_capOn[i]) ? 1 : 0; }
static void fReadPixels(int, int, int, int, unsigned, unsigned, void*) {}
static void fEnable(unsigned cap) { int i = capIdx(cap); if (i >= 0) g_capOn[i] = true; if (g_enN < 64) g_enLog[g_enN] = cap; ++g_enN; }
static void fDisable(unsigned cap) { int i = capIdx(cap); if (i >= 0) g_capOn[i] = false; if (g_disN < 64) g_disLog[g_disN] = cap; ++g_disN; }
static HMODULE GetModuleHandleA(const char* name) { ++g_gmhCalls; return (g_haveGlModule && !strcmp(name, "opengl32.dll")) ? reinterpret_cast<HMODULE>(&g_gmhCalls) : nullptr; }
static FARPROC GetProcAddress(HMODULE, const char* n) {
    ++g_gpaCalls;
    if (g_missingProc && !strcmp(n, g_missingProc)) return nullptr;
    if (!strcmp(n, "glGetError")) return reinterpret_cast<FARPROC>(&fGetError);
    if (!strcmp(n, "glGetIntegerv")) return reinterpret_cast<FARPROC>(&fGetIntegerv);
    if (!strcmp(n, "glGetBooleanv")) return reinterpret_cast<FARPROC>(&fGetBooleanv);
    if (!strcmp(n, "glIsEnabled")) return reinterpret_cast<FARPROC>(&fIsEnabled);
    if (!strcmp(n, "glReadPixels")) return reinterpret_cast<FARPROC>(&fReadPixels);
    if (!strcmp(n, "glEnable")) return reinterpret_cast<FARPROC>(&fEnable);
    if (!strcmp(n, "glDisable")) return reinterpret_cast<FARPROC>(&fDisable);
    return nullptr;
}

#include "K2Addr.h"
#include "Effects.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "PortraitSlots.h"
#include "ShieldLogic.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define SCHECK(why, expr) ::kt::Check("shield relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define SEQ(why, a, b) ::kt::CheckEq("shield relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

// ============================================================ 1. K2Addr table values (compile time)
// Every field below is read by ShieldBar.cpp through k2addr::get() / k2addr::addr(); a changed value means the bar would read the wrong
// engine offset or call the wrong function. Steam column: ShieldBar.cpp comments / log formats; GOG column: doc 21b section 4.
#define SSTEAM(f, v) static_assert(kSteam.f == (v), "shield relies on: kSteam." #f " == " #v)
#define SGOG(f, v) static_assert(kGog.f == (v), "shield relies on: kGog." #f " == " #v)
#define SBOTH(f, v) SSTEAM(f, v); SGOG(f, v)   // struct offsets are byte-identical on both exes
// ShieldBar.cpp:467-469 onSlot (flags590 vt690 vt7fc log fields), :511 / :569 Slot_VitRect reads, :556 arrow counters (log only)
SBOTH(Slot_CardRect, 0x54c); SBOTH(Slot_Flags, 0x590); SBOTH(Slot_VitCtl, 0x690); SBOTH(Slot_VitRect, 0x694);
SBOTH(Slot_ForceCtl, 0x7fc); SBOTH(Slot_ForceRect, 0x800); SBOTH(Slot_ArrowBuff, 0xddc); SBOTH(Slot_ArrowDebuff, 0xddd);
// ShieldBar.cpp:41-48 readCtlRect: control extent x,y,w,h at +4/+8/+0xC/+0x10; flags at +0x48 (bit 2 = shown); SetRect = vtable slot 1 (+4)
SBOTH(Ctl_X, 4); SBOTH(Ctl_Y, 8); SBOTH(Ctl_W, 0xC); SBOTH(Ctl_H, 0x10); SBOTH(Ctl_Flags, 0x48); SBOTH(Ctl_VtSetRect, 4);
// ShieldBar.cpp:404-428 spreadCompanions: LBL_CHARn / LEVELUPn / BACKn / BTN_CHANGEn (PortraitSlots.h: companionCtrl = panel + 0x3134 / 0x36DC)
SBOTH(Menu_Char2, 0x3134); SBOTH(Menu_Level2, 0x327C); SBOTH(Menu_Back2, 0x33C4); SBOTH(Menu_Change2, 0x350C);
SBOTH(Menu_Char3, 0x36DC); SBOTH(Menu_Level3, 0x3824); SBOTH(Menu_Back3, 0x396C); SBOTH(Menu_Change3, 0x3AB4);
// ShieldBar.cpp:372-385 readVitals: creature+0x1198 stats, stats shorts +0x130/+0x132, vtable +0x98 (max, arg 1) / +0x9c (cur, arg 0)
SBOTH(Creature_Stats, 0x1198); SBOTH(Stats_ForceA, 0x130); SBOTH(Stats_ForceB, 0x132); SBOTH(Creature_VtMaxHp, 0x98); SBOTH(Creature_VtCurHp, 0x9c);
// ShieldBar.cpp:606-624 tooltip hook (613): char button +0x1d0 slot, +0x1d4 CExoString text
SBOTH(Tip_Slot, 0x1d0); SBOTH(Tip_Text, 0x1d4);
// Effects.h decode (ShieldBar.cpp:96-150 probe, :185-188 pools): creature +0x148 array / +0x14c count, CGameEffect fields
SBOTH(Creature_EffectArray, 0x148); SBOTH(Creature_EffectCount, 0x14c);
SBOTH(Effect_Type, 0x08); SBOTH(Effect_Subtype, 0x0a); SBOTH(Effect_Duration, 0x0c); SBOTH(Effect_Creator, 0x18);
SBOTH(Effect_SpellId, 0x1c); SBOTH(Effect_IntCount, 0x30); SBOTH(Effect_Ints, 0x34);
// image wrapper vtable slot 6 = Draw (ShieldBar.cpp:258 / 393 comments "image vtable ImageVt_Draw")
SBOTH(ImageVt_Draw, 6);
// engine functions / globals (Steam: Gui2D.h + ShieldBar.cpp comments FUN_00734270 / FUN_0057eca0; GOG: doc 21b)
SSTEAM(CExoStringFormat, 0x00734270); SGOG(CExoStringFormat, 0x00606380);
SSTEAM(GetMaxForcePoints, 0x0057ECA0); SGOG(GetMaxForcePoints, 0x006977D0);
SSTEAM(PushClipRect, 0x00479460);      SGOG(PushClipRect, 0x008CFD70);
SSTEAM(PopClipRect, 0x004799C0);       SGOG(PopClipRect, 0x008D02D0);
SSTEAM(FlushGuiBatch, 0x0047EA60);     SGOG(FlushGuiBatch, 0x008D5370);
SSTEAM(FlushArg, 0x009F5F60);          SGOG(FlushArg, 0x009F7144);
SSTEAM(HudScale, 0x00479AB0);          SGOG(HudScale, 0x008D03C0);
SSTEAM(ClipDepth, 0x00A309C0);         SGOG(ClipDepth, 0x00A32B90);
SSTEAM(ClipNoFillColor, 0x00A1BA3C);   SGOG(ClipNoFillColor, 0x00A768A4);
SSTEAM(GuiLoadImage, 0x0047EB60);      SGOG(GuiLoadImage, 0x008D5470);
SSTEAM(ScreenW, 0x009F42A4);           SGOG(ScreenW, 0x009F22A4);
SSTEAM(ScreenH, 0x009F42A8);           SGOG(ScreenH, 0x009F22A8);
// ShieldBar.cpp:49 / 594-598 + the :8-12 header: priorities and phases the registrations use; shield draws before the buff HUD
static_assert(K2PS_PRIO_SHIELD == 100 && K2PS_PRIO_BUFF == 200, "shield relies on: shield prio 100 runs before buff prio 200 (the v1.1.0 draw order)");
static_assert(K2PS_SNAPSHOT == 0 && K2PS_FRAME_BEGIN == 2 && K2PS_SLOT == 3 && K2PS_MENU_OVERLAY == 4 && K2PS_HUD_DRAW == 5,
              "shield relies on: phase numbers SNAPSHOT 0 / FRAME_BEGIN 2 / SLOT 3 / MENU_OVERLAY 4 / HUD_DRAW 5 (the provider dispatches by number)");
static_assert(K2PS_MAX_SLOTS == 4, "shield relies on: K2PS_MAX_SLOTS == 4 (ShieldBar.cpp:561/566 loop it with Pool t[8], int v[4])");
static_assert(k2fx::MaxEffects == 512 && k2fx::EffType_Pool == 0x02 && k2fx::EffSub_PoolBit == 8 && k2fx::EffType_Visual == 0x1e,
              "shield relies on: pool = type 0x02 with subtype bit 8, colour sibling = type 0x1e, list sane up to 512 (ShieldBar.cpp:182 comment)");

// ============================================================ arena + fakes
static uint8_t* g_arena = nullptr; static size_t g_used = 0x2000; constexpr size_t ArenaSize = 1u << 20, Page = 0x1000;   // pages 0 (data) and 1 (NOACCESS fake) are reserved
static uintptr_t A(size_t off) { return reinterpret_cast<uintptr_t>(g_arena) + off; }
static const size_t kBadOff = Page;     // NOACCESS page
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); }
    uintptr_t a = A(g_used); memset(reinterpret_cast<void*>(a), 0, n); g_used += n; return a;
}
template <typename T> static void put(uintptr_t b, int off, T v) { memcpy(reinterpret_cast<void*>(b + off), &v, sizeof v); }
template <typename T> static T get(uintptr_t b, int off) { T v; memcpy(&v, reinterpret_cast<const void*>(b + off), sizeof v); return v; }
// bit-exact float compare: under -m32 (x87 excess precision) `x == 0.85f` can be false for the very same float
static bool sameBits(float a, float b) { unsigned x, y; memcpy(&x, &a, 4); memcpy(&y, &b, 4); return x == y; }
static void setupFakes() {
    g_fakeN = 0;
    g_fake[g_fakeN++] = { A(kBadOff), A(kBadOff) + Page, PAGE_NOACCESS, MEM_COMMIT };
    saferead::beginScope();
}

// ============================================================ 1b. K2Addr selection + the sb:: gates built on the table fields
static void k2addr_contract() {
    const char* steamSha = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF";   // manifest.toml supported_versions / hooks tomls target_versions
    const char* gogSha = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914";
    // ShieldBar.cpp:42/98/233 `k2addr::get()`: unset KOTOR_VERSION_SHA (host, hot-swap) = the Steam table, both shipped exes resolve, anything else is inert
    SCHECK("get() with KOTOR_VERSION_SHA unset is the Steam table (main() unset it before the first call)", k2addr::get() == &kSteam);
    SCHECK("selectBySha(null / empty) is Steam", k2addr::selectBySha(nullptr) == &kSteam && k2addr::selectBySha("") == &kSteam);
    SCHECK("the Steam sha from the shield manifest selects Steam, case-insensitively", k2addr::selectBySha(steamSha) == &kSteam &&
           k2addr::selectBySha("6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef") == &kSteam);
    SCHECK("the GOG sha from the shield manifest selects GOG", k2addr::selectBySha(gogSha) == &kGog);
    SCHECK("an unknown sha selects nothing (every shield feature stays inert, never a guessed address)", k2addr::selectBySha("0000") == nullptr);
    setenv("KOTOR_VERSION_SHA", gogSha, 1);
    SCHECK("selectFromEnv reads KOTOR_VERSION_SHA (what KotorPatcher sets before the DLL loads)", k2addr::selectFromEnv() == &kGog);
    unsetenv("KOTOR_VERSION_SHA");

    // ShieldBar.cpp:347 sceneBrightness: ScreenW/ScreenH through k2addr::addr; 0 when unknown
    k2addr::setForTest(&kSteam);
    SEQ("addr(ScreenW) is the Steam framebuffer width global", (unsigned long long)k2addr::addr(&K2AddrTable::ScreenW), 0x009F42A4ull);
    SEQ("addr(ScreenH) is the Steam framebuffer height global", (unsigned long long)k2addr::addr(&K2AddrTable::ScreenH), 0x009F42A8ull);
    k2addr::setForTest(&kGog);
    SEQ("addr(ScreenW) on GOG", (unsigned long long)k2addr::addr(&K2AddrTable::ScreenW), 0x009F22A4ull);
    k2addr::setForTest(nullptr);
    SEQ("addr(ScreenW) is 0 for an unknown exe (sceneBrightness returns -1)", (unsigned long long)k2addr::addr(&K2AddrTable::ScreenW), 0ull);
    SCHECK("get() is null for an unknown exe", k2addr::get() == nullptr);
    K2AddrTable z = kSteam; z.ScreenH = 0; k2addr::setForTest(&z);
    SEQ("addr() of a zero field is 0 (ShieldBar.cpp:348 `if (!aw || !ah) return -1`)", (unsigned long long)k2addr::addr(&K2AddrTable::ScreenH), 0ull);
    k2addr::clearForTest();
    SCHECK("clearForTest returns to the environment selection (Steam)", k2addr::get() == &kSteam);

    // ShieldLogic.h gates (ShieldBar.cpp:98/233/374/389/406/464/505-506/555/611): every feature is on for both exes, off for a null / empty table, and one zero field flips it
    for (const K2AddrTable* t : { &kSteam, &kGog }) {
        SCHECK("every sb:: gate is open on a shipped table", sb::slotFieldsOk(t) && sb::vitRectOk(t) && sb::menuFieldsOk(t) && sb::vitalsFieldsOk(t) &&
               sb::drawFieldsOk(t) && sb::effectProbeFieldsOk(t) && sb::tooltipFieldsOk(t));
    }
    const K2AddrTable* none = nullptr; K2AddrTable empty{};
    SCHECK("every sb:: gate is closed on a null table (unknown exe)", !sb::slotFieldsOk(none) && !sb::vitRectOk(none) && !sb::menuFieldsOk(none) && !sb::vitalsFieldsOk(none) &&
           !sb::drawFieldsOk(none) && !sb::effectProbeFieldsOk(none) && !sb::tooltipFieldsOk(none));
    SCHECK("every sb:: gate is closed on an all-zero table", !sb::slotFieldsOk(&empty) && !sb::vitRectOk(&empty) && !sb::menuFieldsOk(&empty) && !sb::vitalsFieldsOk(&empty) &&
           !sb::drawFieldsOk(&empty) && !sb::effectProbeFieldsOk(&empty) && !sb::tooltipFieldsOk(&empty));
    K2AddrTable t1 = kSteam; t1.Slot_VitRect = 0;
    SCHECK("a zero Slot_VitRect closes the HUD / menu-bar gates (the bars need the health-bar rect)", !sb::vitRectOk(&t1) && !sb::slotFieldsOk(&t1));
    K2AddrTable t2 = kSteam; t2.ImageVt_Draw = 0;
    SCHECK("a zero ImageVt_Draw closes the draw gate", !sb::drawFieldsOk(&t2));
    K2AddrTable t3 = kSteam; t3.CExoStringFormat = 0;
    SCHECK("a zero CExoStringFormat closes the tooltip gate", !sb::tooltipFieldsOk(&t3));
    K2AddrTable t4 = kSteam; t4.GetMaxForcePoints = 0;
    SCHECK("a zero GetMaxForcePoints closes the companion-vitals gate", !sb::vitalsFieldsOk(&t4));
}

// ============================================================ 2. SafeRead
static void saferead_contract() {
    setupFakes(); g_stubVirtualQueryAll = true;
    uintptr_t buf = arenaAlloc(64);
    put<uint32_t>(buf, 8, 0xCAFEF00D); put<uint8_t>(buf, 12, 0x7B); put<uint16_t>(buf, 14, 0xFFFE); put<float>(buf, 16, 0.5f); put<uint16_t>(buf, 20, 0x0102);
    saferead::beginScope();

    // ShieldBar.cpp:37-39 readAt wrapper (site 1): value on success, *out untouched on failure -- every caller pre-seeds 0 and trusts it
    uint32_t v = 0x11111111;
    SCHECK("readAt on readable memory returns true", saferead::readAt(1u, buf, 8, &v));
    SEQ("readAt delivers the value", (unsigned long long)v, 0xCAFEF00Dull);
    // ShieldBar.cpp:377-382 Stats_ForceA/B are shorts summed as int(a)+int(b): the signed 16-bit value must come through
    short sh = 0; SCHECK("readAt<short> works (stats force halves)", saferead::readAt<short>(1u, buf, 14, &sh)); SEQ("readAt<short> is signed (0xFFFE -> -2)", (int)sh, -2);
    // ShieldBar.cpp:114-115 / Effects.h type + subtype are ushorts, arrow counters are bytes, duration is a float
    unsigned short us = 0; SCHECK("readAt<unsigned short> works (effect type/subtype)", saferead::readAt<unsigned short>(1u, buf, 20, &us)); SEQ("readAt<unsigned short> value", (int)us, 0x0102);
    unsigned char uc = 0; SCHECK("readAt<unsigned char> works (arrow counters, raw bytes)", saferead::readAt<unsigned char>(1u, buf, 12, &uc)); SEQ("readAt<unsigned char> value", (int)uc, 0x7B);
    float fl = 1.0f; SCHECK("readAt<float> works (effect duration)", saferead::readAt(1u, buf, 16, &fl)); SCHECK("readAt<float> value", fl == 0.5f);
    // ShieldBar.cpp:41-48 readCtlRect / :569 slot rect reads: a null or tiny base (slot 0, no creature) is a miss that leaves the 0 default
    v = 0x22222222;
    SCHECK("readAt with base 0 returns false", !saferead::readAt(1u, 0, 0x694, &v));
    SEQ("readAt base 0 leaves *out untouched (slot rect stays 0 -> v[2] < 1 -> skipped)", (unsigned long long)v, 0x22222222ull);
    SCHECK("readAt below 0x10000 returns false", !saferead::readAt(1u, 0x20, 0, &v));
    SEQ("readAt low address leaves *out untouched", (unsigned long long)v, 0x22222222ull);
    // unreadable (NOACCESS) page: the engine freed / never mapped it
    uintptr_t bad = A(kBadOff);
    SCHECK("readAt on a NOACCESS page returns false", !saferead::readAt(1u, bad, 0, &v));
    SEQ("readAt NOACCESS leaves *out untouched", (unsigned long long)v, 0x22222222ull);
    int ri = -7; SCHECK("readAt<int> miss reports false", !saferead::readAt(1u, bad, 8, &ri)); SEQ("readAt<int> miss keeps the pre-seeded default", ri, -7);

    // ShieldBar.cpp:607 tooltip hook entry: beginScope drops the cached regions (engine code between hooks can free memory)
    saferead::beginScope();
    SCHECK("setup: region readable and cached", saferead::probeRead(2u, reinterpret_cast<const void*>(buf), 4));
    g_stubVirtualQueryAll = false;
    SCHECK("a cached region still answers inside one scope (no VirtualQuery per read)", saferead::probeRead(2u, reinterpret_cast<const void*>(buf), 4));
    saferead::beginScope();
    SCHECK("beginScope drops cached regions (freed memory is not reported readable in the next hook)", !saferead::probeRead(2u, reinterpret_cast<const void*>(buf), 4));
    v = 0x33333333;
    SCHECK("readAt after beginScope re-queries (false once unreadable)", !saferead::readAt(1u, buf, 8, &v));
    SEQ("re-query miss leaves *out untouched", (unsigned long long)v, 0x33333333ull);
    g_stubVirtualQueryAll = true; saferead::beginScope();
    SCHECK("readable again after the stub is restored", saferead::readAt(1u, buf, 8, &v));

    // ShieldBar.cpp:630 enableRing(g_probe) + :456 dump(rb, 3000): ring off records nothing, on records each failed probe, dump stays inside a 3000-byte buffer
    auto count = [&]() -> long { char b[3000]; saferead::dump(b, sizeof b); long n = -1; sscanf(b, "saferead: %ld failed probes", &n); return n; };
    saferead::enableRing(false);
    long c0 = count();
    for (int i = 0; i < 3; ++i) saferead::readAt(1u, 0x20, 0, &v);
    SEQ("enableRing(false): failed probes are not recorded (DllMain without shieldprobe.txt)", count(), c0);
    saferead::enableRing(true);
    saferead::readAt(1u, 0x20, 0, &v);
    SEQ("enableRing(true): one failed probe is one ring entry", count(), c0 + 1);
    { char b[3000]; saferead::dump(b, sizeof b);
      SCHECK("dump names the failing site (1 = ShieldBar's readAt), address and length", strstr(b, "site=1 addr=00000020 len=4") != nullptr); }
    for (int i = 0; i < 100; ++i) saferead::readAt(1u, 0x20, 0, &v);
    char rb[3000 + 16]; memset(rb, 0x55, sizeof rb);
    int dn = saferead::dump(rb, 3000);
    SCHECK("dump into the mod's char rb[3000] returns a length below the capacity", dn > 0 && dn < 3000);
    SEQ("dump returns exactly strlen of what it wrote", (int)strlen(rb), dn);
    bool canary = true; for (int i = 3000; i < 3016; ++i) canary &= (unsigned char)rb[i] == 0x55;
    SCHECK("dump never writes past the 3000-byte buffer (a full 64-entry ring is longer than 3000)", canary);
    SEQ("dump reports the total failure count", count(), c0 + 101);
    saferead::enableRing(false);
}

// ============================================================ 3. Effects.h pool decode
// A fake creature: +0x148 CGameEffect** array, +0x14c count; CGameEffect: +8 ushort type, +0xa ushort subtype, +0x1c spell, +0x30 int count, +0x34 int* ints.
static uintptr_t mkEff(unsigned short type, unsigned short sub, int spell, const std::vector<int>& ints, bool nullInts = false) {
    uintptr_t e = arenaAlloc(0x40);
    put<uint16_t>(e, 0x08, type); put<uint16_t>(e, 0x0a, sub); put<int32_t>(e, 0x1c, spell);
    uintptr_t ip = 0;
    if (!nullInts && !ints.empty()) { ip = arenaAlloc(ints.size() * 4); for (size_t i = 0; i < ints.size(); ++i) put<int32_t>(ip, int(i * 4), ints[i]); }
    put<int32_t>(e, 0x30, (int)ints.size()); put<uint32_t>(e, 0x34, (uint32_t)ip);
    return e;
}
static uintptr_t mkList(uintptr_t creature, const std::vector<uintptr_t>& effs) {
    uintptr_t arr = arenaAlloc(effs.size() * 4 + 16);
    for (size_t i = 0; i < effs.size(); ++i) put<uint32_t>(arr, int(i * 4), (uint32_t)effs[i]);
    put<uint32_t>(creature, 0x148, (uint32_t)arr); put<int32_t>(creature, 0x14c, (int)effs.size());
    return arr;
}
static void effects_contract() {
    setupFakes(); g_stubVirtualQueryAll = true; k2addr::setForTest(&kSteam); saferead::beginScope();

    // ShieldBar.cpp:185 collectPools -> effectListNonEmpty: only a non-null array with 1..512 effects is scanned (0 / negative / > 512 = a freed or corrupt creature)
    uintptr_t c = arenaAlloc(0x200); uintptr_t arr = arenaAlloc(64);
    int ra = 77, rc = 77;
    put<uint32_t>(c, 0x148, (uint32_t)arr); put<int32_t>(c, 0x14c, 1);
    SCHECK("effectListNonEmpty: array + count 1 is scanned", k2fx::effectListNonEmpty(c, &ra, &rc));
    SEQ("effectListNonEmpty delivers the array pointer", (unsigned long long)(unsigned)ra, (unsigned long long)(uint32_t)arr);
    SEQ("effectListNonEmpty delivers the count", rc, 1);
    put<int32_t>(c, 0x14c, 512); SCHECK("count 512 (the cap) is scanned", k2fx::effectListNonEmpty(c, &ra, &rc));
    put<int32_t>(c, 0x14c, 513); SCHECK("count 513 is rejected (corrupt list)", !k2fx::effectListNonEmpty(c, &ra, &rc));
    put<int32_t>(c, 0x14c, 0);   SCHECK("count 0 is rejected (no effects, nothing to draw)", !k2fx::effectListNonEmpty(c, &ra, &rc));
    put<int32_t>(c, 0x14c, -1);  SCHECK("negative count is rejected", !k2fx::effectListNonEmpty(c, &ra, &rc));
    put<int32_t>(c, 0x14c, 3); put<uint32_t>(c, 0x148, 0); SCHECK("a null array is rejected", !k2fx::effectListNonEmpty(c, &ra, &rc));
    ra = rc = 55;
    SCHECK("effectListNonEmpty false on creature 0", !k2fx::effectListNonEmpty(0, &ra, &rc));
    SCHECK("effectListNonEmpty false on an unreadable creature", !k2fx::effectListNonEmpty(A(kBadOff), &ra, &rc));
    SCHECK("effectListNonEmpty miss keeps the caller's pre-seeded 0 pattern (collectPools passes arr = count = 0)", ra == 55 && rc == 55);

    // ShieldBar.cpp:187-188 scanShieldPools(creature, raw, min(maxOut, 16)): the decode the whole mod depends on
    //   ints = [flags, resist, REMAINING, vuln, 1]; pool = type 0x02 && (sub & 8); colour = ints[0] of the type-0x1e sibling with the same spell
    uintptr_t eNotPool = mkEff(0x01, 8, 100, { 1, 2, 3 });                       // wrong type (type 1 with the pool bit)
    uintptr_t eNoBit   = mkEff(0x02, 0x07, 101, { 1, 2, 3 });                    // type 2 but subtype bit 8 clear
    uintptr_t poolA    = mkEff(0x02, 0x0008, 107, { 4224, 7, 40, 9, 1 });         // the canonical shield; remaining = ints[2] = 40
    uintptr_t sibA     = mkEff(0x1e, 0, 107, { 2040 });
    uintptr_t poolB    = mkEff(0x02, 0x0009, 108, { 5248, 1, 25, 2, 1 });         // bit 8 plus other bits; no sibling -> vfx 0
    uintptr_t sibOther = mkEff(0x1e, 0, 999, { 2045 });                          // different spell: not B's colour
    uintptr_t eTwoInts = mkEff(0x02, 8, 109, { 1, 2 });                          // < 3 ints
    uintptr_t eEmpty   = mkEff(0x02, 8, 110, { 1, 2, 0 });                       // remaining 0 = a spent pool
    uintptr_t eNeg     = mkEff(0x02, 8, 111, { 1, 2, -5 });                      // remaining < 0
    uintptr_t eNullInt = mkEff(0x02, 8, 112, { 1, 2, 3, 4, 5 }, true);           // count 5 but int* == 0
    uintptr_t poolC    = mkEff(0x02, 8, 113, { 17, 0, 3 });                      // exactly 3 ints: valid (boundary)
    uintptr_t cr1 = arenaAlloc(0x200);
    mkList(cr1, { eNotPool, eNoBit, poolA, sibA, poolB, sibOther, eTwoInts, eEmpty, eNeg, eNullInt, 0, poolC });
    k2fx::PoolRaw raw[16];
    for (auto& r : raw) r = { -1, -1, -1, -1, -1 };
    int n = k2fx::scanShieldPools(cr1, raw, 16);
    SEQ("scanShieldPools finds exactly the three live pools (type 0x02 + bit 8, >= 3 ints, remaining > 0, null entry skipped)", n, 3);
    SCHECK("pool A: eff / spell / flags = ints[0] / remaining = ints[2] / vfx = sibling ints[0]",
           raw[0].eff == (int)poolA && raw[0].spell == 107 && raw[0].flags == 4224 && raw[0].remaining == 40 && raw[0].vfx == 2040);
    SCHECK("pool B: bit 8 among other subtype bits counts; no same-spell sibling -> vfx 0 (a different spell's sibling is ignored)",
           raw[1].eff == (int)poolB && raw[1].spell == 108 && raw[1].flags == 5248 && raw[1].remaining == 25 && raw[1].vfx == 0);
    SCHECK("pool C: exactly 3 ints is enough, remaining = ints[2]", raw[2].eff == (int)poolC && raw[2].spell == 113 && raw[2].flags == 17 && raw[2].remaining == 3 && raw[2].vfx == 0);
    SCHECK("pools come back in effect-list order (the bar stacks segments bottom-up in this order)", raw[0].eff == (int)poolA && raw[1].eff == (int)poolB && raw[2].eff == (int)poolC);
    SEQ("entries past the returned count are untouched", raw[3].eff, -1);

    // a sibling whose ints pointer is null is skipped; the first sibling with readable ints wins
    uintptr_t poolD = mkEff(0x02, 8, 120, { 1, 1, 9 });
    uintptr_t cr2 = arenaAlloc(0x200);
    mkList(cr2, { poolD, mkEff(0x1e, 0, 120, { 2046 }, true), mkEff(0x1e, 0, 120, { 2047 }), mkEff(0x1e, 0, 120, { 2048 }) });
    n = k2fx::scanShieldPools(cr2, raw, 16);
    SCHECK("the first same-spell 0x1e sibling with a readable ints array provides the colour", n == 1 && raw[0].vfx == 2047);

    // type and subtype are separate 16-bit fields (a 32-bit read at +8 would see 0x00080002 and never match)
    SCHECK("type (+8) and subtype (+0xa) are decoded as separate 16-bit fields", n == 1 && raw[0].eff == (int)poolD);

    // ShieldBar.cpp:187: maxOut = min(maxOut, 16); describePools passes 8, the HUD loop too -- the scan must never write past it
    k2fx::PoolRaw out4[4]; for (auto& r : out4) r = { -77, 0, 0, 0, 0 };
    n = k2fx::scanShieldPools(cr1, out4, 2);
    SEQ("scanShieldPools stops at maxOut", n, 2);
    SEQ("scanShieldPools writes nothing past maxOut", out4[2].eff, -77);
    std::vector<uintptr_t> many; for (int i = 0; i < 20; ++i) many.push_back(mkEff(0x02, 8, 200 + i, { i, 0, 10 + i }));
    uintptr_t cr3 = arenaAlloc(0x200); mkList(cr3, many);
    n = k2fx::scanShieldPools(cr3, raw, 16);
    SEQ("20 pools with the mod's raw[16] buffer: exactly 16 come back", n, 16);
    SCHECK("the first 16 in list order (spell 200..215, remaining 10..25)", raw[0].spell == 200 && raw[15].spell == 215 && raw[15].remaining == 25);

    // unreadable / corrupt memory never crashes and never invents a pool
    SEQ("scanShieldPools on creature 0 is 0", k2fx::scanShieldPools(0, raw, 16), 0);
    SEQ("scanShieldPools on an unreadable creature is 0", k2fx::scanShieldPools(A(kBadOff), raw, 16), 0);
    uintptr_t cr4 = arenaAlloc(0x200); put<uint32_t>(cr4, 0x148, (uint32_t)A(kBadOff)); put<int32_t>(cr4, 0x14c, 3);
    SEQ("scanShieldPools with the array on an unreadable page is 0", k2fx::scanShieldPools(cr4, raw, 16), 0);
    uintptr_t good = mkEff(0x02, 8, 130, { 1, 2, 6 });
    uintptr_t cr5 = arenaAlloc(0x200); mkList(cr5, { (uintptr_t)A(kBadOff), good });
    n = k2fx::scanShieldPools(cr5, raw, 16);
    SCHECK("an effect pointer on an unreadable page is skipped and the scan continues", n == 1 && raw[0].eff == (int)good && raw[0].remaining == 6);
    uintptr_t eBadInts = mkEff(0x02, 8, 131, { 1, 2, 3 }); put<uint32_t>(eBadInts, 0x34, (uint32_t)A(kBadOff));
    uintptr_t cr6 = arenaAlloc(0x200); mkList(cr6, { eBadInts, good });
    n = k2fx::scanShieldPools(cr6, raw, 16);
    SCHECK("an ints pointer on an unreadable page is skipped and the scan continues", n == 1 && raw[0].eff == (int)good);

    // ShieldBar.cpp:34-35 / 42 "a null table or a 0 field = that feature does nothing": the scan is inert, never reads with offset 0
    K2AddrTable t = kSteam; t.Effect_Ints = 0; k2addr::setForTest(&t);
    SEQ("scanShieldPools is 0 when Effect_Ints is unknown", k2fx::scanShieldPools(cr1, raw, 16), 0);
    t = kSteam; t.Effect_SpellId = 0; k2addr::setForTest(&t);
    SEQ("scanShieldPools is 0 when Effect_SpellId is unknown", k2fx::scanShieldPools(cr1, raw, 16), 0);
    t = kSteam; t.Creature_EffectCount = 0; k2addr::setForTest(&t);
    SCHECK("effectListNonEmpty is false when Creature_EffectCount is unknown", !k2fx::effectListNonEmpty(cr1, &ra, &rc));
    k2addr::setForTest(nullptr);
    SEQ("scanShieldPools is 0 for an unknown exe", k2fx::scanShieldPools(cr1, raw, 16), 0);
    SCHECK("effectListNonEmpty is false for an unknown exe", !k2fx::effectListNonEmpty(cr1, &ra, &rc));
    k2addr::setForTest(&kSteam);
}

// ============================================================ 4. Gui2D
struct PushCall { int x, y, w, h; const void* color; int flag; float alpha; };
static PushCall g_last; static int g_pushes = 0, g_pops = 0, g_flushes = 0; static int g_pushResult = 1; static float g_flushArgSeen = -1; static float g_scale = 1.0f;
static int g_order[32], g_nOrder = 0;   // 3 = push, 4 = pop
static void note(int k) { if (g_nOrder < 32) g_order[g_nOrder++] = k; }
static void resetG2d() { g_pushes = g_pops = g_flushes = 0; g_nOrder = 0; g_pushResult = 1; g_last = {}; }
static void installG2dFakes() {
    gui2d::g_g2d = gui2d::Engine{};
    gui2d::g_g2d.push = [](int x, int y, int w, int h, const void* c, int f, float a) { ++g_pushes; note(3); g_last = { x, y, w, h, c, f, a }; return g_pushResult; };
    gui2d::g_g2d.pop = [] { ++g_pops; note(4); };
    gui2d::g_g2d.flush = [](float a) { ++g_flushes; g_flushArgSeen = a; };
    gui2d::g_g2d.hudScale = [] { return g_scale; };
}
// image vtable with a Draw at slot 6 and decoys at 5 / 7
static int g_vtDraws = 0, g_vtWrong = 0, g_vtArgs[6]; static void* g_vtImg = nullptr; static const float* g_vtRgb = nullptr; static float g_vtAlpha = 0;
static void vtDraw(void* img, int x, int y, int w, int h, int a5, int rot, const float* rgb, float a) {
    ++g_vtDraws; g_vtImg = img; g_vtArgs[0] = x; g_vtArgs[1] = y; g_vtArgs[2] = w; g_vtArgs[3] = h; g_vtArgs[4] = a5; g_vtArgs[5] = rot; g_vtRgb = rgb; g_vtAlpha = a;
}
static void vtWrong(void*, int, int, int, int, int, int, const float*, float) { ++g_vtWrong; }
struct FakeImg { void** vt; };
static void* g_vt[8];

static void gui2d_contract() {
    using namespace gui2d;
    setupFakes(); g_stubVirtualQueryAll = true; saferead::beginScope(); k2addr::setForTest(&kSteam);
    installG2dFakes();

    // ShieldBar.cpp:262/274/303/304: pushClip(x, y, w, h) with no colour = the engine's own no-fill colour (0xA1BA3C), flag 0, alpha 1:
    // a pure clip, never a fill (the fill variant would paint rectangles over the HUD)
    resetG2d();
    int r = pushClip(10, 20, 30, 40);
    SEQ("pushClip passes the engine's nonzero result through (the caller then pops)", r, 1);
    SCHECK("pushClip passes x, y, w, h in order", g_last.x == 10 && g_last.y == 20 && g_last.w == 30 && g_last.h == 40);
    SEQ("pushClip's default colour is the table's ClipNoFillColor address (0xA1BA3C on Steam)", (unsigned long long)reinterpret_cast<uintptr_t>(g_last.color), 0x00A1BA3Cull);
    SCHECK("pushClip passes flag 0 and alpha 1.0 by default", g_last.flag == 0 && g_last.alpha == 1.0f);
    // ShieldBar.cpp:307 (DrawSeam, compiled out) would pass a colour + alpha: the colour must go through as given
    const float seam[3] = { 0.0f, 0.16f, 0.14f };
    pushClip(1, 2, 3, 4, seam, 0.85f);
    SCHECK("pushClip with an explicit colour passes it and the alpha through (the 1 px seam fill)", g_last.color == seam && sameBits(g_last.alpha, 0.85f));
    // clip balance: `if (pushClip(...)) { draw; popClip(); }` -- a zero result means nothing was pushed, so no pop
    resetG2d();
    const int results[5] = { 1, 0, 1, 1, 0 }; int nonzero = 0, depth = 0, minDepth = 0;
    for (int res : results) {
        g_pushResult = res;
        if (pushClip(0, 0, 5, 5)) { ++depth; ++nonzero; popClip(); --depth; if (depth < minDepth) minDepth = depth; }
    }
    SEQ("5 push attempts reach the engine", g_pushes, 5);
    SEQ("exactly one pop per successful push (clip stack balanced)", g_pops, nonzero);
    SCHECK("a refused push is never followed by a pop (order is push,pop,push,push,pop... only after success)", g_order[0] == 3 && g_order[1] == 4 && g_order[2] == 3 && g_order[3] == 3 && g_order[4] == 4 && minDepth == 0);
    resetG2d(); popClip(); SEQ("popClip makes exactly one engine pop call", g_pops, 1);

    // ShieldBar.cpp:257-259 / 308: flushArg() read from the table's FlushArg global and handed to flush()
    uintptr_t cell = arenaAlloc(16); put<float>(cell, 0, 0.375f);
    g_g2d.flushArgAddr = cell;
    SCHECK("flushArg reads the float at the flush-arg global", flushArg() == 0.375f);
    resetG2d(); flush(flushArg());
    SCHECK("flush passes that float to the engine's FlushGuiBatch exactly once", g_flushes == 1 && g_flushArgSeen == 0.375f);
    g_g2d.flushArgAddr = 0x20;
    SCHECK("flushArg is 0.0 when the global is unreadable (neutral value, never a crash)", flushArg() == 0.0f);
    g_g2d.flushArgAddr = k2addr::UseTable;
    g_stubVirtualQueryAll = false; saferead::beginScope();
    SCHECK("flushArg through the table's own address is 0.0 when that address is not readable", flushArg() == 0.0f);
    g_stubVirtualQueryAll = true; saferead::beginScope();

    // ShieldBar.cpp:335 / 433 clipDepthRaw: the RAW word (MENUSTATE / GL logs print it with %x; 0 = HUD frame, 0xFFFF = menu frame)
    uintptr_t cd = arenaAlloc(16);
    g_g2d.clipDepthAddr = cd;
    put<uint32_t>(cd, 0, 0xFFFF);     SEQ("clipDepthRaw returns 0xFFFF in a menu frame", clipDepthRaw(), 0xFFFF);
    put<uint32_t>(cd, 0, 0);          SEQ("clipDepthRaw returns 0 in the HUD frame", clipDepthRaw(), 0);
    put<uint32_t>(cd, 0, 0xABCD0003u); SEQ("clipDepthRaw is NOT masked (the log shows the raw word)", (unsigned long long)(unsigned)clipDepthRaw(), 0xABCD0003ull);
    g_g2d.clipDepthAddr = 0x20;       SEQ("clipDepthRaw is 0 when the global is unreadable", clipDepthRaw(), 0);
    g_g2d.clipDepthAddr = k2addr::UseTable;

    // ShieldBar.cpp:485 / 509 / 557 hudScale: a sane scale passes, nonsense is 1.0
    g_scale = 1.5f;  SCHECK("hudScale passes a sane scale", hudScale() == 1.5f);
    g_scale = 0.0f;  SCHECK("hudScale clamps 0 to 1.0", hudScale() == 1.0f);
    g_scale = -2.0f; SCHECK("hudScale clamps a negative scale to 1.0", hudScale() == 1.0f);
    g_scale = 25.0f; SCHECK("hudScale clamps a huge scale to 1.0", hudScale() == 1.0f);
    g_scale = __builtin_nanf(""); SCHECK("hudScale clamps NaN to 1.0", hudScale() == 1.0f);
    g_scale = 1.0f;

    // ShieldBar.cpp:258 / 393 drawImage(img, x, y, w, h, 0, rgb, a): rotBits 0 and the fixed a5 = 0; arg order is x,y,w,h
    // The default engine draw dispatches through the image's vtable slot ImageVt_Draw (6)
    g_g2d = Engine{};
    memset(g_vt, 0, sizeof g_vt); g_vt[5] = reinterpret_cast<void*>(&vtWrong); g_vt[6] = reinterpret_cast<void*>(&vtDraw); g_vt[7] = reinterpret_cast<void*>(&vtWrong);
    FakeImg img{ g_vt }; const float tint[3] = { 0.3f, 0.4f, 0.5f };
    g_vtDraws = g_vtWrong = 0;
    drawImage(&img, 7, 8, 9, 10, 0, tint, 0.75f);
    SEQ("drawImage calls vtable slot 6 exactly once", g_vtDraws, 1);
    SEQ("drawImage never calls the neighbouring slots", g_vtWrong, 0);
    SCHECK("drawImage passes the image as this", g_vtImg == &img);
    SCHECK("drawImage passes x, y, w, h in order", g_vtArgs[0] == 7 && g_vtArgs[1] == 8 && g_vtArgs[2] == 9 && g_vtArgs[3] == 10);
    SCHECK("drawImage passes a5 = 0 and the caller's rotBits (0) in that order", g_vtArgs[4] == 0 && g_vtArgs[5] == 0);
    SCHECK("drawImage passes the tint pointer and the alpha", g_vtRgb == tint && g_vtAlpha == 0.75f);
    g_vtDraws = 0; drawImage(&img, 1, 2, 3, 4, 5, tint, 1.0f);
    SEQ("a non-zero rotBits reaches the engine in the rot slot, a5 stays 0", g_vtArgs[5] * 10 + g_vtArgs[4], 50);
    g_vtDraws = 0; drawImage(nullptr, 1, 2, 3, 4, 0, tint, 1.0f);
    SEQ("drawImage on a null image makes no engine call (imageFor can return null)", g_vtDraws, 0);
    { K2AddrTable t = kSteam; t.ImageVt_Draw = 0; k2addr::setForTest(&t);
      drawImage(&img, 1, 2, 3, 4, 0, tint, 1.0f);
      SEQ("drawImage makes no engine call when the Draw slot is unknown", g_vtDraws, 0);
      k2addr::setForTest(&kSteam); }

    // ShieldBar.cpp:34-35 / 42 "unknown exe / 0 address = does nothing": every default engine call is inert, neutral values come back
    // (default engine functions only; a real address would be called, so only a null / zeroed table is used here)
    g_g2d = Engine{};
    k2addr::setForTest(nullptr);
    g_pushes = 0;
    SEQ("pushClip is 0 on an unknown exe (no push, so the caller never pops)", pushClip(1, 2, 3, 4), 0);
    popClip(); flush(1.0f);   // must not crash (address unknown = no call)
    SCHECK("hudScale is 1.0 on an unknown exe", hudScale() == 1.0f);
    SCHECK("flushArg / clipDepthRaw are 0 on an unknown exe", flushArg() == 0.0f && clipDepthRaw() == 0);
    drawImage(&img, 1, 2, 3, 4, 0, tint, 1.0f);
    SEQ("drawImage on an unknown exe draws nothing", g_vtDraws, 0);
    { K2AddrTable t = kSteam; t.ClipNoFillColor = 0; t.PushClipRect = 0; t.PopClipRect = 0; t.FlushGuiBatch = 0; t.HudScale = 0; t.FlushArg = 0; t.ClipDepth = 0;
      k2addr::setForTest(&t);
      SEQ("pushClip is 0 when PushClipRect / the no-fill colour are unknown", pushClip(1, 2, 3, 4), 0);
      popClip(); flush(1.0f);
      SCHECK("hudScale is 1.0 when HudScale is unknown", hudScale() == 1.0f);
      SCHECK("flushArg / clipDepthRaw are 0 when their globals are unknown", flushArg() == 0.0f && clipDepthRaw() == 0);
      installG2dFakes();
      resetG2d();
      SEQ("pushClip is 0 and never reaches the engine when the no-fill colour address is 0", pushClip(1, 2, 3, 4), 0);
      SEQ("... the engine push was not called", g_pushes, 0);
      k2addr::setForTest(&kSteam); }
    installG2dFakes();
}

// CapGuard on the fake GL (ShieldBar.cpp:517 onMenuOverlay, :558 onHudDraw: one guard per draw block, every frame)
static int g_fixN = 0, g_fixAll = 0; static unsigned g_fixCap[64]; static int g_fixWas[64], g_fixNow[64];
static void onFix(unsigned cap, int was, int now) { ++g_fixAll; if (g_fixN < 64) { g_fixCap[g_fixN] = cap; g_fixWas[g_fixN] = was; g_fixNow[g_fixN] = now; } ++g_fixN; }
static int g_glInitN = 0; static bool g_glInitOk = false, g_glInitReadPixels = false, g_glInitEnDis = false;
static void onGlInit(const gui2d::GlFns& g) { ++g_glInitN; g_glInitOk = g.ok; g_glInitReadPixels = g.readPixels != nullptr; g_glInitEnDis = g.enable && g.disable; }
static void resetGl() { gui2d::g_gl = gui2d::GlFns{}; gui2d::g_hooks = {}; g_gmhCalls = g_gpaCalls = 0; g_haveGlModule = true; g_missingProc = nullptr; g_glInitN = 0; }
static void glState(bool on) { for (bool& b : g_capOn) b = on; g_enN = g_disN = 0; }

static void capguard_contract() {
    using namespace gui2d;
    SCHECK("CapGuard snapshots exactly the nine GL caps the engine's GUI draws can toggle, in the documented order", [&] {
        for (int i = 0; i < 9; ++i) if (CapGuard::Caps[i] != kCaps[i]) return false;
        return true; }());

    // ShieldBar.cpp:320-326 / 345 gl(): module + proc lookups once, and the .ok / .readPixels the diagnostics test
    resetGl(); g_haveGlModule = false; g_hooks.onGlInit = onGlInit;
    SCHECK("gl() without opengl32.dll is not ok (glDump / sceneBrightness / CapGuard all bail)", !gl().ok);
    SEQ("gl() without opengl32.dll does not call the onGlInit hook (no GLINIT line)", g_glInitN, 0);
    SEQ("gl() is cached: a second call does not look the module up again", (gl(), g_gmhCalls), 1);
    resetGl(); g_hooks.onGlInit = onGlInit;
    SCHECK("gl() with all procs present is ok", gl().ok);
    SEQ("gl() calls onGlInit exactly once with the final state (ShieldBar logs GLINIT ok= readPixels= enable/disable=)", g_glInitN, 1);
    SCHECK("onGlInit sees ok, readPixels and enable/disable all set", g_glInitOk && g_glInitReadPixels && g_glInitEnDis);
    SEQ("gl() looks up exactly seven procs", g_gpaCalls, 7);
    gl(); SEQ("gl() is cached after success too", g_gmhCalls, 1);
    const char* required[4] = { "glGetError", "glGetIntegerv", "glGetBooleanv", "glIsEnabled" };
    for (const char* miss : required) {
        resetGl(); g_missingProc = miss;
        char w[96]; snprintf(w, sizeof w, "shield relies on: gl().ok needs %s (glDump calls it unguarded once ok)", miss);
        kt::Check(w, !gl().ok);
    }
    resetGl(); g_missingProc = "glReadPixels"; g_hooks.onGlInit = onGlInit;
    SCHECK("a missing glReadPixels leaves gl().ok true but readPixels null (sceneBrightness returns -1)", gl().ok && gl().readPixels == nullptr && g_glInitReadPixels == false);
    resetGl(); g_missingProc = "glEnable";
    SCHECK("a missing glEnable keeps gl().ok true", gl().ok && gl().enable == nullptr);

    // restore: changed caps are put back, unchanged caps are left alone
    resetGl(); gl(); g_hooks.onCapFix = onFix; g_fixN = 0;
    glState(false); g_capOn[3] = true;   // lighting starts ON
    { CapGuard guard; g_capOn[3] = false; g_capOn[2] = true; g_capOn[5] = true; }   // the draw turned lighting off, depth and fog on
    SCHECK("CapGuard turns a cap the draw switched ON back off (depth, fog)", !g_capOn[2] && !g_capOn[5]);
    SCHECK("CapGuard turns a cap the draw switched OFF back on (lighting)", g_capOn[3]);
    SEQ("CapGuard calls disable twice and enable once for those three changes", g_disN * 10 + g_enN, 21);
    SCHECK("disable was called with depth (0x0B71) and fog (0x0B60), enable with lighting (0x0B50)",
           ((g_disLog[0] == 0x0B71 && g_disLog[1] == 0x0B60) || (g_disLog[0] == 0x0B60 && g_disLog[1] == 0x0B71)) && g_enLog[0] == 0x0B50);
    SEQ("onCapFix fires once per restored cap (ShieldBar logs CAPFIX)", g_fixN, 3);
    bool lightFix = false; for (int i = 0; i < g_fixN && i < 64; ++i) if (g_fixCap[i] == 0x0B50) lightFix = (g_fixWas[i] == 1 && g_fixNow[i] == 0);
    SCHECK("onCapFix(cap, was, now) reports lighting was 1, now 0", lightFix);
    // untouched state: the destructor makes no GL call at all (it runs every frame)
    glState(true); g_capOn[4] = false; g_fixN = 0;
    { CapGuard guard; }
    SEQ("an unchanged GL state makes no enable/disable call", g_enN + g_disN, 0);
    SEQ("an unchanged GL state fires no onCapFix", g_fixN, 0);
    // the snapshot is taken at construction, not at the first draw
    glState(false);
    { CapGuard guard; g_capOn[8] = true; }
    SCHECK("the baseline is the state at construction (alpha test switched on inside is switched off)", !g_capOn[8] && g_disN == 1 && g_disLog[0] == 0x0BC0);

    // unarmed guards never touch GL
    resetGl(); g_haveGlModule = false; gl(); glState(false);
    { CapGuard guard; g_capOn[3] = true; }
    SCHECK("without opengl32 the guard is inert (no restore, no crash)", g_capOn[3] && g_enN + g_disN == 0);
    resetGl(); g_missingProc = "glDisable"; gl(); glState(false);
    { CapGuard guard; g_capOn[3] = true; }
    SCHECK("without glDisable the guard is inert", g_capOn[3] && g_enN + g_disN == 0);
    resetGl(); g_missingProc = "glEnable"; gl(); glState(false);
    { CapGuard guard; g_capOn[3] = true; }
    SCHECK("without glEnable the guard is inert", g_capOn[3] && g_enN + g_disN == 0);

    // onCapFix is capped at 20 lines per DLL (the flag log would flood otherwise); restores continue past the cap. This block owns the
    // static counter in this binary: count every fix made so far.
    resetGl(); gl(); g_hooks.onCapFix = onFix;
    glState(false);
    for (int i = 0; i < 30; ++i) { CapGuard guard; g_capOn[i % 9] = !g_capOn[i % 9]; }
    SEQ("every one of 30 changes is restored", g_enN + g_disN, 30);
    SEQ("onCapFix stops after 20 reports per DLL in total, restores keep going", g_fixAll, 20);
    resetGl();
}

// ============================================================ 5. GuiKit imageFor
static int g_nLoad = 0; static char g_loadName[8][20]; static uintptr_t g_loadSerial = 0x1000;
static void* fLoadImage(const char* name) { if (g_nLoad < 8) snprintf(g_loadName[g_nLoad], 20, "%s", name); ++g_nLoad; g_loadSerial += 8; return reinterpret_cast<void*>(g_loadSerial); }
static void imagefor_contract() {
    using namespace guikit;
    // default binding: the engine's NewCAurGUIImage address from the table (never called here), null for an unknown exe
    { k2addr::setForTest(&kSteam); Engine fresh; SEQ("the default image loader is the table's GuiLoadImage (0x47EB60 on Steam)", (unsigned long long)reinterpret_cast<uintptr_t>(fresh.loadImage), 0x0047EB60ull);
      k2addr::setForTest(nullptr); Engine none; SCHECK("the default image loader is null on an unknown exe", none.loadImage == nullptr);
      k2addr::setForTest(&kSteam); }
    g_gk.loadImage = fLoadImage;

    // ShieldBar.cpp:237 / 294 / 390: imageFor(texture) is called EVERY FRAME for 4 textures; the engine's loader makes a new wrapper per call,
    // so imageFor must pool: one load per resref, identical pointer afterwards, case-insensitive
    const char* tex[4] = { "uibit_bar_sh_p", "uibit_bar_sc_p", "uibit_bar_vp_p", "uibit_bar_fp_p" };   // ShieldBarTexture, CornerTexture, HealthBarTexture, ForceBarTexture
    void* first[4]; bool loaded = false; int pool = -1;
    for (int i = 0; i < 4; ++i) {
        first[i] = imageFor(tex[i], &loaded, &pool);
        char w[96]; snprintf(w, sizeof w, "shield relies on: imageFor('%s') loads once and reports loaded + a growing pool", tex[i]);
        kt::Check(w, first[i] != nullptr && loaded && pool == i + 1);
    }
    SEQ("four textures cost exactly four engine loads", g_nLoad, 4);
    SCHECK("the engine loader receives the lowercase resref unchanged for a lowercase texture", !strcmp(g_loadName[0], "uibit_bar_sh_p") && !strcmp(g_loadName[3], "uibit_bar_fp_p"));
    SCHECK("the four wrappers are distinct objects", first[0] != first[1] && first[1] != first[2] && first[2] != first[3]);
    bool same = true; bool noReload = true;
    for (int frame = 0; frame < 300; ++frame) for (int i = 0; i < 4; ++i) { same &= imageFor(tex[i], &loaded, &pool) == first[i]; noReload &= !loaded && pool == 4; }
    SCHECK("300 frames x 4 textures return the identical pointer (no per-frame wrapper leak)", same);
    SCHECK("... and never report loaded again, pool stays at 4", noReload);
    SEQ("... and never call the engine loader again", g_nLoad, 4);
    SCHECK("imageFor is case-insensitive (same pooled wrapper)", imageFor("UIBIT_BAR_SH_P") == first[0] && g_nLoad == 4);
    // a texture that failed to load: result null, the mod tests `if (!image) return -1` / `if (!img ...) return`
    g_gk.loadImage = [](const char*) -> void* { ++g_nLoad; return nullptr; };
    int nb = g_nLoad;
    void* miss = imageFor("uibit_bar_missing", &loaded, &pool);
    SCHECK("a texture the engine fails to load returns null (DrawShieldPools returns -1, gauges skip)", miss == nullptr);
    SCHECK("... but it was a real load attempt (loaded true, pool grew to 5)", loaded && pool == 5 && g_nLoad == nb + 1);
    // PINNED-AS-IS: the null is pooled, so a failed load is not retried (no engine call per frame for a missing texture)
    SCHECK("a failed load is pooled as null: the next call returns null without calling the engine again", imageFor("uibit_bar_missing", &loaded, &pool) == nullptr && !loaded && g_nLoad == nb + 1);
    // unknown exe: no loader at all -> null, nothing pooled (even for a texture that is already pooled)
    g_gk.loadImage = nullptr;
    SCHECK("imageFor without an engine loader (unknown exe) returns null, not loaded", imageFor("uibit_bar_never", &loaded, &pool) == nullptr && !loaded);
    SEQ("... and pools nothing", pool, 5);
    SCHECK("... and refuses even an already pooled texture (the loader gate comes first)", imageFor(tex[0]) == nullptr);
    g_gk.loadImage = fLoadImage;
    SCHECK("restoring the loader restores the pooled wrappers", imageFor(tex[0]) == first[0]);
    SCHECK("imageFor on null / empty resref returns null", imageFor(nullptr) == nullptr && imageFor("") == nullptr);
}

// ============================================================ 6. PortraitSlots k2ps::Client at the shield call sites
// Fake provider (fixture trick from portrait_slots_src/ps_registry_test.cpp): records every Register, can refuse one phase.
struct RegRec { int phase, prio; K2PsCallback fn; void* user; char owner[32]; };
static RegRec g_reg[16]; static int g_nReg = 0, g_refusePhase = -1, g_nextToken = 1;
static int32_t KPATCH_CALL fReg(int32_t ph, int32_t pr, K2PsCallback fn, void* u, const char* owner) {
    if (g_nReg < 16) { g_reg[g_nReg] = { ph, pr, fn, u, {} }; snprintf(g_reg[g_nReg].owner, sizeof g_reg[g_nReg].owner, "%s", owner ? owner : "(null)"); }
    ++g_nReg;
    return ph == g_refusePhase ? 0 : g_nextToken++;
}
static void KPATCH_CALL fUnreg(int32_t) {}
static int32_t g_creatureSlotSeen = -1;
static int32_t KPATCH_CALL fCreature(int32_t s) { g_creatureSlotSeen = s; return s == 0x10 ? 0x1000 : 0; }
static int32_t g_slotAtRet = 1, g_slotAtIdx = -1;
static int32_t KPATCH_CALL fSlotAt(int32_t i, int32_t* s, int32_t* c) { g_slotAtIdx = i; *s = 0x10; *c = 0x1000; return g_slotAtRet; }
static uint32_t KPATCH_CALL fFrame() { return 7; }
static const K2PortraitSlotsApi g_table = { sizeof(K2PortraitSlotsApi), fReg, fUnreg, fCreature, fSlotAt, fFrame };

static void KPATCH_CALL cbSnap(const K2PsContext*, void*) {}
static void KPATCH_CALL cbFrame(const K2PsContext*, void*) {}
static void KPATCH_CALL cbSlot(const K2PsContext*, void*) {}
static void KPATCH_CALL cbMenu(const K2PsContext*, void*) {}
static void KPATCH_CALL cbHud(const K2PsContext*, void*) {}

static int g_connN = 0; static bool g_connOk = false; static char g_connWhy[128];
static void onConn(bool ok, const char* why) { ++g_connN; g_connOk = ok; snprintf(g_connWhy, sizeof g_connWhy, "%s", why ? why : "(null)"); }
static void resetPs() { g_nReg = 0; g_refusePhase = -1; g_nextToken = 1; g_connN = 0; g_connOk = false; g_connWhy[0] = 0; g_slotAtRet = 1; g_slotAtIdx = -1; g_creatureSlotSeen = -1; }
static bool whyHas(const char* s) { return strstr(g_connWhy, s) != nullptr; }
// ShieldBar.cpp:590-599 KPatchInit, verbatim: init, onConnect, five phase registrations at K2PS_PRIO_SHIELD, connect
static void shieldInit(k2ps::Client& c, const KPatchApi* api) {
    c.init(api, "shield-bar");
    c.onConnect = onConn;
    c.add(K2PS_SNAPSHOT, K2PS_PRIO_SHIELD, cbSnap);
    c.add(K2PS_FRAME_BEGIN, K2PS_PRIO_SHIELD, cbFrame);
    c.add(K2PS_SLOT, K2PS_PRIO_SHIELD, cbSlot);
    c.add(K2PS_MENU_OVERLAY, K2PS_PRIO_SHIELD, cbMenu);
    c.add(K2PS_HUD_DRAW, K2PS_PRIO_SHIELD, cbHud);
    c.connect();
}

static void ps_contract() {
    // ShieldBar.cpp:594-599: all five registrations fit the client's table and reach the provider in this order, prio 100, owner "shield-bar"
    fakekp::reset(); resetPs();
    fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table);
    k2ps::Client c;
    shieldInit(c, fakekp::api());
    SEQ("KPatchInit's five add() calls all fit the Client's registration table", c.nRegs, 5);
    SCHECK("KPatchInit connects to a provider that is already there", c.connected);
    SEQ("the provider receives five Register calls", g_nReg, 5);
    const int wantPhase[5] = { K2PS_SNAPSHOT, K2PS_FRAME_BEGIN, K2PS_SLOT, K2PS_MENU_OVERLAY, K2PS_HUD_DRAW };
    K2PsCallback wantFn[5] = { cbSnap, cbFrame, cbSlot, cbMenu, cbHud };
    bool order = true, prio = true, owner = true, fns = true;
    for (int i = 0; i < 5 && i < g_nReg; ++i) { order &= g_reg[i].phase == wantPhase[i]; prio &= g_reg[i].prio == 100; owner &= !strcmp(g_reg[i].owner, "shield-bar"); fns &= g_reg[i].fn == wantFn[i]; }
    SCHECK("registrations arrive in phase order SNAPSHOT, FRAME_BEGIN, SLOT, MENU_OVERLAY, HUD_DRAW", order);
    SCHECK("every registration carries priority 100 (K2PS_PRIO_SHIELD)", prio);
    SCHECK("every registration is owned by \"shield-bar\" (the provider logs the owner)", owner);
    SCHECK("each phase gets its own callback", fns);
    SEQ("onConnect fires once on success", g_connN, 1);
    SCHECK("onConnect(true, \"registered\"): ShieldBar logs 'PORTRAITSLOTS ok'", g_connOk && whyHas("registered"));

    // ShieldBar.cpp:561/567 HUD loops slotAt over K2PS_MAX_SLOTS with s = cr = 0 pre-seeded; `continue` unless it returns true;
    // ShieldBar.cpp:616 creatureForSlot(slot) with the slot read from the char button
    g_slotAtRet = 1; int32_t s = 0, cr = 0;
    SCHECK("slotAt is true when the provider returns 1 and fills slot / creature", c.slotAt(2, &s, &cr) && s == 0x10 && cr == 0x1000 && g_slotAtIdx == 2);
    for (int32_t bad : { 0, 2, -1 }) {
        g_slotAtRet = bad; s = cr = 0;
        char w[96]; snprintf(w, sizeof w, "shield relies on: slotAt is false when the provider returns %d (only exactly 1 means live)", bad);
        kt::Check(w, !c.slotAt(0, &s, &cr));
    }
    SEQ("creatureForSlot passes the slot through and returns the provider's creature", c.creatureForSlot(0x10), 0x1000);
    SEQ("creatureForSlot hands the slot to the provider unchanged", g_creatureSlotSeen, 0x10);
    SEQ("an unknown slot yields creature 0 (the tooltip then does nothing)", c.creatureForSlot(0x99), 0);
    // no provider: shield is inert, not broken
    { fakekp::reset(); resetPs();
      k2ps::Client n; shieldInit(n, fakekp::api());
      int32_t s2 = 5, c2 = 6;
      SCHECK("without a provider the HUD loop gets slotAt == false with the pre-seeded values untouched", !n.slotAt(0, &s2, &c2) && s2 == 5 && c2 == 6);
      SEQ("without a provider creatureForSlot is 0 (describePools is skipped)", n.creatureForSlot(0x10), 0); }

    // ---- PINNED-AS-IS, UNDECIDED (doc 21 backlog, commit 2296ab1: "PortraitSlots.h client gaps"). Do not read these as endorsed behaviour: they are
    // what ShieldBar.cpp lives with today. If the Client is changed (retry refused registrations / report each failure), update these deliberately.
    // (1) ShieldBar.cpp:594-599 KPatchInit + :608 tooltip-hook g_ps.poll(): the provider refuses one registration (say the HUD draw phase).
    //     The Client still reports connected, onConnect(false, "some registrations refused") fires once (ShieldBar logs 'PORTRAITSLOTS MISSING'),
    //     and nothing ever retries the refused phase: no HUD bars for the rest of the session, while the other four phases keep running.
    fakekp::reset(); resetPs(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table);
    g_refusePhase = K2PS_HUD_DRAW;
    k2ps::Client rf; shieldInit(rf, fakekp::api());
    SCHECK("refused registration: the Client still reports connected", rf.connected);
    SEQ("refused registration: all five were offered once", g_nReg, 5);
    SCHECK("refused registration: onConnect(false, 'some registrations refused') fires once", g_connN == 1 && !g_connOk && whyHas("some registrations refused"));
    g_refusePhase = -1;   // the provider would accept the HUD phase now
    for (int i = 0; i < 1000; ++i) { rf.poll(); rf.connect(); }
    SEQ("refused registration: poll() / connect() never re-offer the refused phase (Register calls stay at 5)", g_nReg, 5);
    SEQ("refused registration: no further onConnect report", g_connN, 1);
    SEQ("refused registration: the registry is not asked again either (one require total)", fakekp::state().requireCalls, 1);
    SCHECK("refused registration: slotAt still works through the connection", rf.slotAt(0, &s, &cr));

    // (2) the onConnect FAILURE path fires exactly once whatever the cause, however often KPatchInit's connect() / the tooltip hook's poll() retry.
    //     Causes: no KPatchApi at all, KPatchApi too small to hold require, provider absent, provider table too small / missing functions.
    struct Cause { const char* name; const char* why; };
    auto hammer = [&](k2ps::Client& k) { for (int i = 0; i < 3; ++i) k.connect(); for (int i = 0; i < 600; ++i) k.poll(); };
    { resetPs(); k2ps::Client k; k.init(nullptr, "shield-bar"); k.onConnect = onConn; k.add(K2PS_HUD_DRAW, K2PS_PRIO_SHIELD, cbHud);
      SCHECK("cause 'no KPatchApi': connect fails", !k.connect()); hammer(k);
      SCHECK("cause 'no KPatchApi': exactly one failure report", g_connN == 1 && !g_connOk && whyHas("no KPatchApi")); }
    { fakekp::reset(offsetof(KPatchApi, require)); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_table); resetPs();
      k2ps::Client k; shieldInit(k, fakekp::api()); hammer(k);
      SCHECK("cause 'KPatchApi too small for require': exactly one failure report, never connected", g_connN == 1 && !g_connOk && whyHas("no KPatchApi") && !k.connected);
      SEQ("... and the registry was never asked", fakekp::state().requireCalls, 0); }
    { fakekp::reset(); resetPs();
      k2ps::Client k; shieldInit(k, fakekp::api()); hammer(k);
      SCHECK("cause 'provider absent': exactly one failure report", g_connN == 1 && !g_connOk && whyHas("not provided") && !k.connected); }
    { K2PortraitSlotsApi small = g_table; small.struct_size = offsetof(K2PortraitSlotsApi, Frame);
      fakekp::reset(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &small); resetPs();
      k2ps::Client k; shieldInit(k, fakekp::api()); hammer(k);
      SCHECK("cause 'provider table too small': exactly one failure report", g_connN == 1 && !g_connOk && whyHas("too small") && !k.connected);
      SEQ("... and nothing was registered", g_nReg, 0); }
    { K2PortraitSlotsApi noSlot = g_table; noSlot.SlotAt = nullptr;
      fakekp::reset(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &noSlot); resetPs();
      k2ps::Client k; shieldInit(k, fakekp::api()); hammer(k);
      SCHECK("cause 'provider without SlotAt': exactly one failure report", g_connN == 1 && !g_connOk && !k.connected); }
    // the report is shared across causes: a different failure after the first one is NOT reported again, and a later success IS reported
    { K2PortraitSlotsApi small = g_table; small.struct_size = offsetof(K2PortraitSlotsApi, Frame);
      fakekp::reset(); resetPs();
      k2ps::Client k; shieldInit(k, fakekp::api());
      SEQ("first failure (provider absent) is reported", g_connN, 1);
      fakekp::provide(K2PS_IFACE, K2PS_VERSION, &small);
      SCHECK("second failure (table too small) is not connected", !k.connect());
      SEQ("second failure with a different cause is NOT reported again", g_connN, 1);
      SCHECK("the reported cause is still the first one", whyHas("not provided"));
      fakekp::state().entries[0].iface = &g_table;   // the provider's table becomes usable
      SCHECK("a later successful connect works", k.connect() && k.connected);
      SCHECK("... and is reported once as success (ShieldBar logs 'PORTRAITSLOTS ok')", g_connN == 2 && g_connOk && whyHas("registered"));
      SEQ("... and registers all five phases", g_nReg, 5); }
}

// ============================================================ 7. StrFmt (strfmt::appendf, re-exported as sb::appendf)
// ShieldBar.cpp builds its log lines with sb::appendf (ShieldLogic.h: `using strfmt::appendf`); the shared helper must keep the bounded,
// always-terminated, clamped-length semantics the probe / effect-dump lines depend on.
static void strfmt_contract() {
    {   // appends at len, returns the new length
        char b[32] = "ab";
        int n = sb::appendf(b, (int)sizeof b, 2, " %d/%d", 3, 7);
        SEQ("sb::appendf appends at buf[len] and returns the new length", n, 6);
        SEQ("sb::appendf leaves the appended text in the buffer", strcmp(b, "ab 3/7"), 0);
    }
    {   // truncation: clamped to cap-1, NUL-terminated, never past the buffer (guard bytes intact)
        char g[16]; memset(g, 'g', sizeof g);
        char* b = g; int cap = 8;
        int n = sb::appendf(b, cap, 0, "%s", "0123456789abcdef");
        SEQ("sb::appendf clamps the returned length to cap-1 on truncation", n, cap - 1);
        SEQ("sb::appendf keeps the buffer NUL-terminated on truncation", (int)b[cap - 1], 0);
        SEQ("sb::appendf never writes past cap (guard byte intact)", (int)g[cap], (int)'g');
        int n2 = sb::appendf(b, cap, n, " more %d", 1);
        SEQ("sb::appendf on a full buffer is a no-op (length stays cap-1)", n2, cap - 1);
    }
    {   // cap <= 0 touches nothing and returns len
        char c = 'x';
        SEQ("sb::appendf with cap <= 0 returns len and writes nothing", sb::appendf(&c, 0, 3, "zzz") + (c == 'x' ? 0 : 100), 3);
    }
}

// ============================================================ 8. k2addr::featureOk + gui2d::vfillClip(Rect,int,int)
// shield-bar relies on: k2addr::featureOk (one line, once per flag), gui2d::vfillClip(Rect,int,int). ShieldBar.cpp featureOk() forwards to the
// first with probeLog and g_tableNoted; drawGauge() forwards its cur / max fill to the second (double-rounded, half up).
static int g_foLines = 0;
static char g_foLast[160];
static void foLog(const char* fmt, ...) {
    ++g_foLines;
    va_list ap; va_start(ap, fmt); vsnprintf(g_foLast, sizeof g_foLast, fmt, ap); va_end(ap);
}
static void featureok_vfill_contract() {
    {   bool noted = false; g_foLines = 0; g_foLast[0] = 0;
        SCHECK("shield relies on: featureOk(ok=true) is true", k2addr::featureOk(true, "gauge draw", foLog, &noted));
        SEQ("shield relies on: featureOk(ok=true) logs nothing", g_foLines, 0);
        SCHECK("shield relies on: featureOk(ok=true) leaves the noted flag clear", !noted);
        SCHECK("shield relies on: featureOk(ok=false) is false", !k2addr::featureOk(false, "gauge draw", foLog, &noted));
        SCHECK("shield relies on: featureOk(ok=false) sets the noted flag", noted);
        SCHECK("shield relies on: a second featureOk(ok=false) is still false", !k2addr::featureOk(false, "companion vitals", foLog, &noted));
        SEQ("shield relies on: two failures write exactly one log line in total", g_foLines, 1);
        SEQ("shield relies on: the one line names the first feature", strncmp(g_foLast, "K2ADDR gauge draw disabled", 26), 0);
    }
    {   const gui2d::Rect r{10, 20, 6, 10};
        gui2d::Rect f = gui2d::vfillClip(r, 10, 10);
        SCHECK("shield relies on: full gauge (cur==mx) fills the whole height from the rect top", f.h == 10 && f.y == 20 && f.x == 10 && f.w == 6);
        f = gui2d::vfillClip(r, 5, 10);
        SCHECK("shield relies on: half gauge (h=10, 5/10) is 5 px tall and bottom-anchored (y=25)", f.h == 5 && f.y == 25);
        f = gui2d::vfillClip(r, 0, 10);
        SEQ("shield relies on: empty gauge (cur=0) fills 0 px", f.h, 0);
        f = gui2d::vfillClip(gui2d::Rect{10, 20, 6, 11}, 13, 22);
        SCHECK("shield relies on: h=11, 13/22 = 6.5 rounds half up to 7 px (bottom edge stays 31)", f.h == 7 && f.y + f.h == 31);
    }
}

int main() {
    unsetenv("KOTOR_VERSION_SHA");   // k2addr::get() caches the environment choice on first use: make it Steam, as on every host test
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    k2addr_contract();
    saferead_contract();
    effects_contract();
    gui2d_contract();
    capguard_contract();
    imagefor_contract();
    ps_contract();
    strfmt_contract();
    featureok_vfill_contract();
    return kt::Report("common_contract_test (shield_bar)");
}
