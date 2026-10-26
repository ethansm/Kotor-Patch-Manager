// common_contract_test.cpp -- Patches/Common CONTRACT test for the Buff Duration HUD (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on (BuffDurationHUD.cpp call sites), not the headers' own behaviour (that is _shared_src/*_test.cpp). Every message starts
// with "buff relies on: " so a failure names the consumer and the reason. Built against ../_shared_src + Patches/Common (test.toml),
// so ANY edit to K2Addr.h / SafeRead.h / Gui2D.h / GuiKit.h / EffectIcons.h / Effects.h / PortraitSlots.h re-runs this test.
// Expected values are literals copied from the mod .cpp, the hooks tomls, the Gui2D.h/Effects.h engine-fact comments and doc 21b,
// never values read back from the header under test.
//
// Areas: (1) k2addr tables, (2) saferead, (3) gui2d engine calls + CapGuard on fake GL, (4) guikit::imageFor pooling,
// (5) k2fx / EffectIcons on fake creatures, (6) k2ps::Client as KPatchInit drives it. Host-only, deterministic, no game, no DLL.
// Float compares of non-binary values (0.95f, 0.35f) use near_(): under -m32 x87 a stored float can compare unequal to its own literal.
// Host traps: the stub VirtualQuery reports everything readable when g_stubVirtualQueryAll (unreadable = address < 0x10000 or the
// switch off), and host uintptr_t is 8 bytes (target pointers are modelled as uint32_t in the MAP_32BIT arena).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>   // host stub (research/companion_levelup/tools/parity/stub)
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <algorithm>
#include <string>
#include <set>
#include <vector>
#include <type_traits>
#include <sys/mman.h>

// Gui2D.h's CapGuard needs these four Win32 names; the host stub has none, so fake opengl32.dll here (before Gui2D.h).
typedef void* HMODULE;
static std::vector<std::string> g_modNames, g_procNames;
static bool g_glPresent = true, g_hideGlEnable = false;
static int g_modImage = 0;
static inline HMODULE GetModuleHandleA(const char* n) {
    g_modNames.push_back(n);
    return (g_glPresent && !strcmp(n, "opengl32.dll")) ? reinterpret_cast<HMODULE>(&g_modImage) : nullptr;
}
static unsigned char g_glCap[0x10000];
static int g_nEnable = 0, g_nDisable = 0;
static int fGetError() { return 0; }
static void fGetIntegerv(unsigned, int*) {}
static void fGetBooleanv(unsigned, unsigned char*) {}
static unsigned char fIsEnabled(unsigned c) { return g_glCap[c & 0xFFFF]; }
static void fReadPixels(int, int, int, int, unsigned, unsigned, void*) {}
static void fEnable(unsigned c) { ++g_nEnable; g_glCap[c & 0xFFFF] = 1; }
static void fDisable(unsigned c) { ++g_nDisable; g_glCap[c & 0xFFFF] = 0; }
static inline void* GetProcAddress(HMODULE, const char* n) {
    g_procNames.push_back(n);
    if (!strcmp(n, "glGetError")) return reinterpret_cast<void*>(&fGetError);
    if (!strcmp(n, "glGetIntegerv")) return reinterpret_cast<void*>(&fGetIntegerv);
    if (!strcmp(n, "glGetBooleanv")) return reinterpret_cast<void*>(&fGetBooleanv);
    if (!strcmp(n, "glIsEnabled")) return reinterpret_cast<void*>(&fIsEnabled);
    if (!strcmp(n, "glReadPixels")) return reinterpret_cast<void*>(&fReadPixels);
    if (!strcmp(n, "glEnable")) return g_hideGlEnable ? nullptr : reinterpret_cast<void*>(&fEnable);
    if (!strcmp(n, "glDisable")) return reinterpret_cast<void*>(&fDisable);
    return nullptr;
}

#include "SafeRead.h"
#include "K2Addr.h"
#include "EffectIcons.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "PortraitSlots.h"
#include "BuffHudLogic.h"   // the mod's own pure logic: its featureOk() gates (arrowFieldsOk etc.)
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("buff relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("buff relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))
static bool near_(float a, float b) { return fabsf(a - b) < 1e-5f; }

// ============================================================ 1. K2Addr (compile time + selection)
// BuffDurationHUD.cpp:144,167,261,337 -- the mod reads every engine offset from k2addr::get(). These are the Steam values from the mod's own
// comments (lines 39-41, 264-271, 292-301), the hooks toml and the Gui2D.h / Effects.h engine-fact headers. 0 would silently disable a feature.
#define HF(name, val) static_assert(kSteam.name == (val), "buff relies on: kSteam." #name " == " #val " (validated Steam value)")
// BuffDurationHUD.cpp:148,150,157 -- arrow-hide: icon widget -> border sub-object, vtable check, width/height words.
HF(Label_Border, 0x60); HF(BorderVtable, 0x009875BC); HF(Border_Width, 0xC); HF(Border_Height, 0x10);
// BuffDurationHUD.cpp:192-196 -- column drawer: border rotation bits, alpha float, tint rgb floats.
HF(Border_Rot, 0x20); HF(Border_Alpha, 0x24); HF(Border_Tint, 0x28); HF(ImageVt_Draw, 6);
// BuffDurationHUD.cpp:264-271 -- ForceArrowCounters: slot flags word, the two arrow counters, the three overlay words (mod comment 270).
HF(Slot_Flags, 0x590); HF(Slot_ArrowBuff, 0xddc); HF(Slot_ArrowDebuff, 0xddd);
HF(Slot_Overlay0, 0x1b8); HF(Slot_Overlay1, 0x300); HF(Slot_Overlay2, 0x448);
// BuffDurationHUD.cpp:296-301 -- ICONCACHE diagnostic: creature+0xFFC cache (mod comment 250-253), entry fields.
HF(Creature_IconCacheData, 0xFFC); HF(Creature_IconCacheCount, 0x1000); HF(IconEnt_F14, 0x14); HF(IconEnt_F1c, 0x1c);
// BuffDurationHUD.cpp:339-344 -- menu companion control: flags word + extent x, y, w, h.
HF(Ctl_Flags, 0x48); HF(Ctl_X, 4); HF(Ctl_Y, 8); HF(Ctl_W, 0xC); HF(Ctl_H, 0x10);
// BuffDurationHUD.cpp:138,171,173 (through Effects.h) -- effect list on the creature + the CGameEffect fields collectBuffs reads (Effects.h header).
HF(Creature_EffectArray, 0x148); HF(Creature_EffectCount, 0x14c);
HF(Effect_SpellId, 0x1c); HF(Effect_Duration, 0xc); HF(Effect_ExpiryDay, 0x10); HF(Effect_ExpiryTime, 0x14);
HF(AppGlobal_ToWorld, 8); HF(World_ToSub, 4); HF(Sub_ToTimer, 0x10048);
HF(AppGlobal, 0x00A1B4A4); HF(WorldTimerGetCurrentTime, 0x0051AD20);
// BuffDurationHUD.cpp:176,197,202,217 (through Gui2D.h) -- the 2D engine calls (Gui2D.h header comment) and GuiLoadImage (imageFor, line 204).
HF(BeginGui2D, 0x004792C0); HF(EndGui2D, 0x004793F0); HF(PushClipRect, 0x00479460); HF(PopClipRect, 0x004799C0);
HF(FlushGuiBatch, 0x0047EA60); HF(FlushArg, 0x009F5F60); HF(HudScale, 0x00479AB0); HF(ClipDepth, 0x00A309C0);
HF(ClipNoFillColor, 0x00A1BA3C); HF(PanelRect, 0x00410020); HF(GuiLoadImage, 0x0047EB60);
// GOG column (kotor2-gog-aspyr.hooks.toml target; doc 21b section 4): struct offsets are byte-identical to Steam, addresses are the mapped twins.
#define HG(name, val) static_assert(kGog.name == (val), "buff relies on: kGog." #name " == " #val " (doc 21b mapped address)")
HG(AppGlobal, 0x00A11C04); HG(WorldTimerGetCurrentTime, 0x0080B200); HG(BeginGui2D, 0x008CFBD0); HG(EndGui2D, 0x008CFD00);
HG(PushClipRect, 0x008CFD70); HG(PopClipRect, 0x008D02D0); HG(HudScale, 0x008D03C0); HG(FlushGuiBatch, 0x008D5370);
HG(GuiLoadImage, 0x008D5470); HG(PanelRect, 0x0090E4A0); HG(FlushArg, 0x009F7144); HG(ClipDepth, 0x00A32B90);
HG(ClipNoFillColor, 0x00A768A4); HG(BorderVtable, 0x009A994C);
HG(Slot_Flags, 0x590); HG(Slot_ArrowBuff, 0xDDC); HG(Slot_ArrowDebuff, 0xDDD); HG(Slot_Overlay0, 0x1B8); HG(Slot_Overlay1, 0x300);
HG(Slot_Overlay2, 0x448); HG(Label_Border, 0x60); HG(Border_Rot, 0x20); HG(Border_Alpha, 0x24); HG(Border_Tint, 0x28);
HG(Creature_EffectArray, 0x148); HG(Creature_EffectCount, 0x14C); HG(Effect_SpellId, 0x1C); HG(ImageVt_Draw, 6);
static int g_featureLogs = 0;
static void fakeFeatureLog(const char*, ...) { ++g_featureLogs; }
static void k2addr_contract() {
    // BuffDurationHUD.cpp:144 -- unset env / known sha / unknown exe. hooks toml target_versions literals (Steam, GOG).
    const char* steam = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF";
    const char* gog = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914";
    HEQ("kSteam.sha is the Steam hooks-toml target_versions sha", kSteam.sha, steam);
    HEQ("kGog.sha is the GOG hooks-toml target_versions sha", kGog.sha, gog);
    HCHECK("unset/empty KOTOR_VERSION_SHA selects Steam (hot-swapped DLLs, host)", k2addr::selectBySha(nullptr) == &kSteam && k2addr::selectBySha("") == &kSteam);
    HCHECK("the Steam sha (either case) selects kSteam", k2addr::selectBySha(steam) == &kSteam &&
           k2addr::selectBySha("6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef") == &kSteam);
    HCHECK("the GOG sha selects kGog", k2addr::selectBySha(gog) == &kGog);
    HCHECK("an unknown exe sha selects nullptr (every mod feature must then be inert)", k2addr::selectBySha("DEADBEEF") == nullptr);
    // BuffDurationHUD.cpp:99-107,145,168,262,338 -- the featureOk() gates: a field a gate needs that reads 0 turns that feature off for the exe.
    HCHECK("every featureOk gate field is non-zero in kSteam (arrow counters/hide, columns, menu, icon cache)",
           bh::arrowFieldsOk(&kSteam) && bh::arrowHideFieldsOk(&kSteam) && bh::columnFieldsOk(&kSteam) && bh::menuFieldsOk(&kSteam) && bh::iconCacheFieldsOk(&kSteam));
    HCHECK("every featureOk gate field is non-zero in kGog (the GOG hooks toml is installed, so its features must not be off)",
           bh::arrowFieldsOk(&kGog) && bh::arrowHideFieldsOk(&kGog) && bh::columnFieldsOk(&kGog) && bh::menuFieldsOk(&kGog) && bh::iconCacheFieldsOk(&kGog));
    // BuffDurationHUD.cpp:145 -- featureOk(bh::xFieldsOk(T)) with a null table (unknown exe) and with one needed field zeroed.
    const K2AddrTable* none = nullptr;
    HCHECK("null table fails every featureOk gate", !bh::arrowFieldsOk(none) && !bh::arrowHideFieldsOk(none) && !bh::columnFieldsOk(none) && !bh::menuFieldsOk(none));
    K2AddrTable z = kSteam; z.Slot_Overlay2 = 0;
    HCHECK("a zero Slot_Overlay2 fails the arrow-counter gate (field 0 = unknown)", !bh::arrowFieldsOk(&z));
    z = kSteam; z.ImageVt_Draw = 0;
    HCHECK("a zero ImageVt_Draw fails the column gate", !bh::columnFieldsOk(&z));
    // BuffDurationHUD.cpp:99-102 -- the mod's featureOk wraps k2addr::featureOk: ok -> true silently; !ok -> false, ONE log line per noted flag.
    g_featureLogs = 0; bool noted = false;
    HCHECK("k2addr::featureOk(true) is true with no log and noted untouched", k2addr::featureOk(true, "f", fakeFeatureLog, &noted) && g_featureLogs == 0 && !noted);
    bool r1 = k2addr::featureOk(false, "f", fakeFeatureLog, &noted), r2 = k2addr::featureOk(false, "f", fakeFeatureLog, &noted);
    HCHECK("k2addr::featureOk(false) twice: false both times, exactly one log line, noted set", !r1 && !r2 && g_featureLogs == 1 && noted);
    // BuffDurationHUD.cpp:144 -- get() returns whatever setForTest forced, and the field values the mod reads through it.
    k2addr::setForTest(&kSteam);
    const K2AddrTable* T = k2addr::get();
    HCHECK("get() hands the mod the forced table", T == &kSteam);
    HEQ("T->Slot_ArrowBuff via get() (BuffDurationHUD.cpp:266)", T->Slot_ArrowBuff, 0xddc);
    HEQ("T->Border_Tint via get() (BuffDurationHUD.cpp:195)", T->Border_Tint, 0x28);
    k2addr::setForTest(nullptr);
    HCHECK("setForTest(nullptr) = unknown exe: get() is null", k2addr::get() == nullptr);
    HEQ("off() on a null table is 0 (unknown), never a guessed offset", k2addr::off(&K2AddrTable::Slot_Flags), 0);
    HEQ("addr() on a null table is 0, never a call target", (unsigned long long)k2addr::addr(&K2AddrTable::PushClipRect), 0ull);
    HCHECK("fnptr() on a null table is nullptr", k2addr::fnptr<void (*)()>(&K2AddrTable::BeginGui2D) == nullptr);
    k2addr::setForTest(&kSteam);
    HEQ("addr() on Steam gives the validated PushClipRect address", (unsigned long long)k2addr::addr(&K2AddrTable::PushClipRect), 0x00479460ull);
}

// ============================================================ arena + fakes
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 4u << 20;
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a;
}
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
static void wptr(uintptr_t b, int off, uintptr_t v) { at<uint32_t>(b, off) = static_cast<uint32_t>(v); }   // target pointers are 32-bit

// ============================================================ 2. SafeRead
static void saferead_contract() {
    uintptr_t buf = arenaAlloc(64);
    at<uint32_t>(buf, 8) = 0xCAFEF00D; at<float>(buf, 16) = 0.5f; at<uint8_t>(buf, 12) = 0x7B; at<int16_t>(buf, 20) = -3;
    // BuffDurationHUD.cpp:73-75 -- the mod's readAt<T> wrapper is saferead::readAt(site 1, base, off, out): value on success, out untouched on failure.
    saferead::beginScope();
    int vi = 0x11111111; float vf = 1.0f; unsigned char vb = 9; short vs = 5;
    HCHECK("readAt<int> reads readable memory (border vtable, width, height)", saferead::readAt(1u, buf, 8, &vi));
    HEQ("readAt<int> value", (unsigned long long)(unsigned)vi, 0xCAFEF00Dull);
    HCHECK("readAt<float> reads the border alpha (BuffDurationHUD.cpp:196)", saferead::readAt(1u, buf, 16, &vf) && vf == 0.5f);
    HCHECK("readAt<unsigned char> reads the arrow counters (BuffDurationHUD.cpp:266)", saferead::readAt(1u, buf, 12, &vb) && vb == 0x7B);
    HCHECK("readAt<short> reads the icon-cache entry id (BuffDurationHUD.cpp:301)", saferead::readAt(1u, buf, 20, &vs) && vs == -3);
    vi = 0x22222222; vf = 1.0f; vb = 9;
    HCHECK("readAt base 0 is false (creatureless slot / missing control)", !saferead::readAt(1u, 0, 0x10, &vi));
    HEQ("readAt base-0 failure leaves the int default (mod pre-seeds 0)", (unsigned long long)(unsigned)vi, 0x22222222ull);
    HCHECK("readAt below 0x10000 is false", !saferead::readAt(1u, 0x20, 4, &vi));
    HCHECK("readAt<float> failure keeps baseAlpha's default 1.0f (BuffDurationHUD.cpp:196)", !saferead::readAt(1u, 0x20, 0, &vf) && vf == 1.0f);
    HCHECK("readAt<unsigned char> failure keeps the default (BuffDurationHUD.cpp:265)", !saferead::readAt(1u, 0x20, 0, &vb) && vb == 9);
    // BuffDurationHUD.cpp:125,238 -- beginScope at every hook entry drops cached regions: freed memory is not reported readable.
    HCHECK("setup: region cached readable", saferead::readAt(1u, buf, 8, &vi));
    g_stubVirtualQueryAll = false;
    saferead::beginScope();
    vi = 0x33333333;
    HCHECK("after beginScope a freed region is re-queried and refused", !saferead::readAt(1u, buf, 8, &vi));
    HEQ("re-query failure leaves the out value untouched", (unsigned long long)(unsigned)vi, 0x33333333ull);
    g_stubVirtualQueryAll = true; saferead::beginScope();
    HCHECK("beginScope then readable again -> true", saferead::readAt(1u, buf, 8, &vi));

    // BuffDurationHUD.cpp:371 -- enableRing(g_probe): failures are recorded only while the ring is on, with the mod's site id 1 and the address.
    char d[2048];
    saferead::enableRing(false);
    saferead::readAt(7u, 0x30, 0, &vi);
    saferead::dump(d, sizeof d);
    HCHECK("ring off: a failed read is not recorded", !strstr(d, "site=7 addr=00000030"));
    saferead::enableRing(true);
    saferead::readAt(1u, 0x20, 0, &vi);
    saferead::dump(d, sizeof d);
    HCHECK("ring on: a failed readAt(site 1) is recorded as site=1 addr=00000020 len=4", strstr(d, "site=1 addr=00000020 len=4") != nullptr);
    saferead::enableRing(false);
}

// ============================================================ 3. Gui2D
struct PushCall { int x, y, w, h; const void* color; int flag; float alpha; };
static PushCall g_last; static int g_pushes = 0, g_pops = 0, g_flushes = 0, g_draws = 0, g_pushResult = 1;
static float g_flushSeen[4]; static float g_scale = 1.0f;
static int g_order[32]; static int g_nOrder = 0;   // 3 push, 4 pop, 5 draw, 6 flush
static void note(int k) { if (g_nOrder < 32) g_order[g_nOrder++] = k; }
static void* g_drawImg; static int g_drawArgs[6]; static const float* g_drawRgb; static float g_drawAlpha;
static void g2dReset() { g_pushes = g_pops = g_flushes = g_draws = 0; g_nOrder = 0; g_pushResult = 1; g_last = {}; }
static void installG2dFakes() {
    gui2d::g_g2d.push = [](int x, int y, int w, int h, const void* c, int f, float a) { ++g_pushes; note(3); g_last = { x, y, w, h, c, f, a }; return g_pushResult; };
    gui2d::g_g2d.pop = [] { ++g_pops; note(4); };
    gui2d::g_g2d.flush = [](float a) { if (g_flushes < 4) g_flushSeen[g_flushes] = a; ++g_flushes; note(6); };
    gui2d::g_g2d.hudScale = [] { return g_scale; };
    gui2d::g_g2d.draw = [](void* img, int x, int y, int w, int h, int a5, int rot, const float* rgb, float al) {
        ++g_draws; note(5); g_drawImg = img; g_drawArgs[0] = x; g_drawArgs[1] = y; g_drawArgs[2] = w; g_drawArgs[3] = h; g_drawArgs[4] = a5; g_drawArgs[5] = rot;
        g_drawRgb = rgb; g_drawAlpha = al;
    };
}
// Fake image wrapper: vtable of 8 slots, slot 6 = the draw we expect; every other slot flags a wrong-slot call.
static int g_vtDraws = 0, g_vtWrong = 0; static void* g_vtThis; static int g_vtArgs[6]; static const float* g_vtRgb; static float g_vtAlpha;
static void vtWrong(void*, int, int, int, int, int, int, const float*, float) { ++g_vtWrong; }
static void vtDraw(void* t, int x, int y, int w, int h, int a5, int rot, const float* rgb, float a) {
    ++g_vtDraws; g_vtThis = t; g_vtArgs[0] = x; g_vtArgs[1] = y; g_vtArgs[2] = w; g_vtArgs[3] = h; g_vtArgs[4] = a5; g_vtArgs[5] = rot; g_vtRgb = rgb; g_vtAlpha = a;
}
struct FakeImg { void** vt; };

static int g_glInitCalls = 0, g_glInitOk = -1, g_glInitRead = -1, g_glInitEn = -1;
static void onGlInitFake(const gui2d::GlFns& g) { ++g_glInitCalls; g_glInitOk = g.ok; g_glInitRead = g.readPixels != nullptr; g_glInitEn = g.enable && g.disable; }
struct CapFix { unsigned cap; int was, now; };
static std::vector<CapFix> g_capFixes;
static void onCapFixFake(unsigned cap, int was, int now) { g_capFixes.push_back({ cap, was, now }); }

static void gui2d_contract() {
    using namespace gui2d;
    const Engine def;   // the engine defaults the mod runs with (it replaces none of g_g2d)
    k2addr::setForTest(nullptr);
    // BuffDurationHUD.cpp:176,202,217 -- unknown exe: every default engine call is a no-op / neutral value and never a call into address 0.
    {
        int r[4] = { 11, 22, 33, 44 };
        def.begin(); def.end(); def.pop(); def.flush(1.0f);
        HEQ("unknown exe: default push returns 0 (caller never pops)", def.push(1, 2, 3, 4, nullptr, 0, 1.0f), 0);
        HCHECK("unknown exe: default hudScale is the neutral 1.0f", def.hudScale() == 1.0f);
        def.panelRect(0x20, r);
        HCHECK("unknown exe: default panelRect leaves the rect untouched", r[0] == 11 && r[3] == 44);
        FakeImg img; void* vt[8]; for (auto& s : vt) s = reinterpret_cast<void*>(&vtWrong); vt[6] = reinterpret_cast<void*>(&vtDraw); img.vt = vt;
        g_vtDraws = g_vtWrong = 0;
        def.draw(&img, 1, 2, 3, 4, 0, 0, nullptr, 1.0f);
        HEQ("unknown exe: default draw does not call the image vtable", g_vtDraws + g_vtWrong, 0);
    }
    installG2dFakes();
    static const float kNoFill[4] = {};
    // BuffDurationHUD.cpp:217,346 -- pushClip with no colour needs the no-fill colour address; unknown -> returns 0 and pushes nothing (so no pop).
    g2dReset();
    HEQ("unknown exe: pushClip(no colour) returns 0", pushClip(10, 20, 30, 40), 0);
    HEQ("unknown exe: pushClip(no colour) never reaches the engine", g_pushes, 0);
    HCHECK("unknown exe: clipNoFill() is null", clipNoFill() == nullptr);
    // BuffDurationHUD.cpp:229 -- the 1 px boundary line passes its own colour: pushed even without a no-fill address, colour/alpha passed through.
    {
        const float lineColor[3] = { 0.0f, 0.16f, 0.14f };
        g2dReset();
        HCHECK("pushClip(explicit colour) pushes even when the no-fill address is unknown", pushClip(5, 6, 7, 1, lineColor, 0.95f) == 1);
        HCHECK("the line colour pointer and alpha reach the engine unchanged", g_last.color == lineColor && near_(g_last.alpha, 0.95f));
        HEQ("explicit-colour push passes flag 0", g_last.flag, 0);
        HEQ("push geometry order is x, y, w, h", g_last.x * 1000000 + g_last.y * 10000 + g_last.w * 100 + g_last.h, 5 * 1000000 + 6 * 10000 + 7 * 100 + 1);
    }
    k2addr::setForTest(&kSteam);
    // BuffDurationHUD.cpp:217,346 -- default colour is the engine's own no-fill colour (0xA1BA3C), flag 0, alpha 1.0, engine result passed through.
    g2dReset();
    HCHECK("Steam: clipNoFill() is the engine's no-fill colour 0xA1BA3C", clipNoFill() == reinterpret_cast<const void*>(static_cast<uintptr_t>(0xA1BA3C)));
    HEQ("pushClip returns the engine's nonzero result", pushClip(10, 20, 30, 40), 1);
    HCHECK("default pushClip colour is the engine no-fill constant", g_last.color == reinterpret_cast<const void*>(static_cast<uintptr_t>(0xA1BA3C)));
    HCHECK("default pushClip passes flag 0 and alpha 1.0", g_last.flag == 0 && g_last.alpha == 1.0f);
    HCHECK("pushClip passes x,y,w,h through (menu rect / bright slice, negative or zero origin included)", g_last.x == 10 && g_last.y == 20 && g_last.w == 30 && g_last.h == 40);
    g_pushResult = 0; HEQ("a refused push returns 0 (mod skips the draw and the pop)", pushClip(1, 2, 3, 4), 0);
    g_g2d.clipNoFill = kNoFill;
    g2dReset(); pushClip(1, 2, 3, 4);
    HCHECK("g_g2d.clipNoFill overrides the table address", g_last.color == kNoFill);
    g_g2d.clipNoFill = nullptr;

    // BuffDurationHUD.cpp:215-231 -- the bright-slice sequence: push -> draws -> pop only when the push succeeded; one pop per push.
    {
        FakeImg* dummy = reinterpret_cast<FakeImg*>(arenaAlloc(16)); (void)dummy;
        const float tint[3] = { 1, 1, 1 };
        g2dReset();
        if (pushClip(100, 90, 20, 15)) { drawImage(dummy, 0, -5, 20, 20, 0x3F800000, tint, 0.8f); popClip(); }
        HEQ("accepted push: exactly one pop", g_pops, 1);
        HCHECK("accepted push: order is push, draw, pop", g_nOrder == 3 && g_order[0] == 3 && g_order[1] == 5 && g_order[2] == 4);
        HCHECK("drawImage passes the image pointer through", g_drawImg == dummy);
        HCHECK("drawImage argument order x,y,w,h then a5 = 0 then rotation bits", g_drawArgs[0] == 0 && g_drawArgs[1] == -5 && g_drawArgs[2] == 20 && g_drawArgs[3] == 20 && g_drawArgs[4] == 0 && g_drawArgs[5] == 0x3F800000);
        HCHECK("drawImage passes tint pointer and alpha through", g_drawRgb == tint && near_(g_drawAlpha, 0.8f));
        g2dReset(); g_pushResult = 0;
        if (pushClip(100, 90, 20, 15)) { drawImage(dummy, 0, 0, 1, 1, 0, tint, 1.0f); popClip(); }
        HEQ("refused push: no pop", g_pops, 0); HEQ("refused push: no draw", g_draws, 0);
        g2dReset();
        if (pushClip(1, 2, 3, 4)) popClip();
        if (pushClip(5, 6, 7, 8)) popClip();
        HEQ("N accepted pushes -> N pops", g_pops, g_pushes);
    }
    // BuffDurationHUD.cpp:200 -- default draw: image vtable slot 6, args (img, x, y, w, h, a5=0, rotBits, rgb, alpha).
    {
        FakeImg img; void* vt[8]; for (auto& s : vt) s = reinterpret_cast<void*>(&vtWrong); vt[6] = reinterpret_cast<void*>(&vtDraw); img.vt = vt;
        const float tint[3] = { 0.1f, 0.2f, 0.3f };
        g_vtDraws = g_vtWrong = 0;
        def.draw(&img, 7, 8, 9, 10, 0, 0x3F000000, tint, 0.35f);
        HEQ("default draw calls vtable slot 6 exactly once", g_vtDraws, 1);
        HEQ("default draw calls no other vtable slot", g_vtWrong, 0);
        HCHECK("default draw: this = image wrapper", g_vtThis == &img);
        HCHECK("default draw: x,y,w,h, a5 = 0, rotBits in that order", g_vtArgs[0] == 7 && g_vtArgs[1] == 8 && g_vtArgs[2] == 9 && g_vtArgs[3] == 10 && g_vtArgs[4] == 0 && g_vtArgs[5] == 0x3F000000);
        HCHECK("default draw: tint pointer and alpha last", g_vtRgb == tint && near_(g_vtAlpha, 0.35f));
        def.draw(nullptr, 1, 2, 3, 4, 0, 0, tint, 1.0f);
        HEQ("default draw with a null image does nothing (imageFor failed -> mod skips, but the call must be safe)", g_vtDraws, 1);
    }
    // BuffDurationHUD.cpp:197,202,234 -- flushArg() reads the float the engine passes to FlushGuiBatch; flush(arg) forwards that exact value.
    {
        uintptr_t fa = arenaAlloc(16); at<float>(fa, 0) = 1.25f;
        g_g2d.flushArgAddr = fa;
        HCHECK("flushArg reads the float at the flush-arg address", flushArg() == 1.25f);
        g2dReset(); flush(flushArg()); flush(flushArg());
        HEQ("two flushes reach the engine (before and after the icon loop)", g_flushes, 2);
        HCHECK("each flush carries the value read from the arg address", g_flushSeen[0] == 1.25f && g_flushSeen[1] == 1.25f);
        g_g2d.flushArgAddr = 0; HCHECK("flushArg with address 0 is 0.0f (never a read of address 0)", flushArg() == 0.0f);
        g_g2d.flushArgAddr = 0x20; HCHECK("flushArg with an unreadable address is 0.0f", flushArg() == 0.0f);
        k2addr::setForTest(nullptr); g_g2d.flushArgAddr = k2addr::UseTable;
        HCHECK("unknown exe: flushArg (UseTable) is 0.0f", flushArg() == 0.0f);
        k2addr::setForTest(&kSteam); g_g2d.flushArgAddr = k2addr::UseTable;   // (reading 0x9F5F60 on the host would fault: not exercised)
        g_g2d.flushArgAddr = fa;
    }
    // BuffDurationHUD.cpp:176 -- hudScale passes sane values and falls back to 1.0 for nonsense (it scales every icon).
    g_scale = 2.5f; HCHECK("hudScale passes 2.5", hudScale() == 2.5f);
    g_scale = 0.0f; HCHECK("hudScale 0 -> 1.0", hudScale() == 1.0f);
    g_scale = -1.0f; HCHECK("hudScale negative -> 1.0", hudScale() == 1.0f);
    g_scale = 10.0f; HCHECK("hudScale 10 (exclusive bound) -> 1.0", hudScale() == 1.0f);
    g_scale = 0.05f; HCHECK("hudScale 0.05 (below the 0.1 lower bound) -> 1.0", hudScale() == 1.0f);
    g_scale = NAN; HCHECK("hudScale NaN -> 1.0", hudScale() == 1.0f);
    g_scale = 1.0f;

    // ---- CapGuard on a fake opengl32 (BuffDurationHUD.cpp:191, 113-116, 372-373) ----
    static const unsigned Caps[9] = { 0x0C11, 0x0B90, 0x0B71, 0x0B50, 0x0BE2, 0x0B60, 0x0DE1, 0x0B44, 0x0BC0 };   // scissor stencil depth LIGHTING blend fog tex2d cull alpha
    g_hooks.onGlInit = onGlInitFake; g_hooks.onCapFix = onCapFixFake;   // the same hook signatures DllMain assigns (a signature change fails to compile here)
    // no opengl32 at all: guard is inert, never faults, onGlInit is not called.
    g_gl = GlFns{}; g_glPresent = false; g_glInitCalls = 0; g_nEnable = g_nDisable = 0; g_modNames.clear(); g_procNames.clear(); g_capFixes.clear();
    { CapGuard cg; g_glCap[0x0B50] ^= 1; }
    HEQ("no opengl32: CapGuard restores nothing", g_nEnable + g_nDisable, 0);
    HEQ("no opengl32: onGlInit is not called", g_glInitCalls, 0);
    HCHECK("no opengl32: g_gl.ok is false", !g_gl.ok);
    HCHECK("the module looked up is opengl32.dll", g_modNames.size() == 1 && g_modNames[0] == "opengl32.dll");
    g_glCap[0x0B50] = 0;
    // opengl32 present but no glEnable: ok (the diagnostics' four) but the guard stays disarmed -> no restore, onGlInit says enable/disable=0.
    g_gl = GlFns{}; g_glPresent = true; g_hideGlEnable = true; g_glInitCalls = 0; g_nEnable = g_nDisable = 0; g_procNames.clear();
    { CapGuard cg; g_glCap[0x0B50] ^= 1; }
    HEQ("glEnable missing: no restore calls", g_nEnable + g_nDisable, 0);
    HCHECK("glEnable missing: onGlInit reports ok=1 enable/disable=0", g_glInitCalls == 1 && g_glInitOk == 1 && g_glInitEn == 0);
    g_glCap[0x0B50] = 0; g_hideGlEnable = false;
    // full GL: onGlInit once, with the lookup names the mod's GLINIT log line implies.
    g_gl = GlFns{}; g_glInitCalls = 0; g_nEnable = g_nDisable = 0; g_procNames.clear(); g_capFixes.clear();
    for (unsigned i = 0; i < 9; ++i) g_glCap[Caps[i]] = (i % 2) ? 1 : 0;
    unsigned char before[9]; for (int i = 0; i < 9; ++i) before[i] = g_glCap[Caps[i]];
    g_glCap[0x0BD0] = 0;   // GL_DITHER: not one of the guarded caps
    {
        CapGuard cg;
        for (int i = 0; i < 9; ++i) g_glCap[Caps[i]] ^= 1;   // the engine flush / pushClip leaks: every guarded cap flipped
        g_glCap[0x0BD0] = 1;
    }
    bool restored = true; for (int i = 0; i < 9; ++i) restored &= g_glCap[Caps[i]] == before[i];
    HCHECK("CapGuard restores all nine guarded caps (scissor, stencil, depth, LIGHTING, blend, fog, tex2d, cull, alpha test)", restored);
    HEQ("GL_LIGHTING (0x0B50, the doc 08 session 4 leak) is restored to its pre-draw state", g_glCap[0x0B50], before[3]);
    HCHECK("an unguarded cap (GL_DITHER 0x0BD0) is left as the engine set it", g_glCap[0x0BD0] == 1);
    HEQ("one enable/disable per changed cap", g_nEnable + g_nDisable, 9);
    HEQ("onGlInit fires once, on the first CapGuard", g_glInitCalls, 1);
    HCHECK("onGlInit sees ok, readPixels and enable/disable", g_glInitOk == 1 && g_glInitRead == 1 && g_glInitEn == 1);
    const char* wantProcs[] = { "glGetError", "glGetIntegerv", "glGetBooleanv", "glIsEnabled", "glReadPixels", "glEnable", "glDisable" };
    bool namesOk = g_procNames.size() == 7; for (int i = 0; namesOk && i < 7; ++i) namesOk = g_procNames[i] == wantProcs[i];
    HCHECK("GL entry points are looked up by the standard names, in this order", namesOk);
    HEQ("onCapFix fired once per restored cap", (int)g_capFixes.size(), 9);
    bool fixOk = g_capFixes.size() == 9;
    for (size_t i = 0; fixOk && i < g_capFixes.size(); ++i) {
        const CapFix& f = g_capFixes[i];
        int idx = -1; for (int k = 0; k < 9; ++k) if (Caps[k] == f.cap) idx = k;
        fixOk = idx >= 0 && f.was == before[idx] && f.now == !before[idx];   // (cap, was = pre-draw state, now = leaked state)
    }
    HCHECK("onCapFix(cap, was, now) reports the pre-draw state as 'was' and the leaked state as 'now'", fixOk);
    // an untouched scope costs nothing.
    g_nEnable = g_nDisable = 0; size_t nfix = g_capFixes.size();
    { CapGuard cg; }
    HCHECK("no cap changed: no restore call, no onCapFix", g_nEnable + g_nDisable == 0 && g_capFixes.size() == nfix);
    // diagnostics are bounded (first 20 per DLL) but restoration is not.
    g_nEnable = g_nDisable = 0;
    for (int round = 0; round < 6; ++round) { CapGuard cg; for (int i = 0; i < 9; ++i) g_glCap[Caps[i]] ^= 1; }
    HEQ("restores keep happening after the log cap (54 flips)", g_nEnable + g_nDisable, 54);
    HEQ("onCapFix is capped at 20 calls per DLL (the log cannot flood)", (int)g_capFixes.size(), 20);
    k2addr::setForTest(&kSteam);
    g_hooks = Hooks{};
}

// ============================================================ 4. GuiKit::imageFor
static int nLoad = 0; static char g_loadName[24];
static void* fLoadImage(const char* n) {
    ++nLoad; snprintf(g_loadName, sizeof g_loadName, "%s", n);
    if (!strcmp(n, "failnull_img")) return nullptr;   // the engine could not load the TGA
    return reinterpret_cast<void*>(0x7000u + 8u * (unsigned)nLoad);
}
static void guikit_contract() {
    using namespace guikit;
    // BuffDurationHUD.cpp:204,211 -- imageFor(resref): nullptr until a loader exists; then pooled one per resref.
    Engine& e = g_gk;
    LoadImageFn saved = e.loadImage;
    e.loadImage = nullptr;
    HCHECK("no loadImage (unknown exe): imageFor returns nullptr, not a call into address 0", imageFor("ip_armor") == nullptr);
    e.loadImage = fLoadImage; (void)saved;
    HCHECK("null resref -> nullptr", imageFor(nullptr) == nullptr);
    HCHECK("empty resref -> nullptr", imageFor("") == nullptr);
    HEQ("null/empty resrefs never load", nLoad, 0);
    // The engine's GuiLoadImage returns a NEW wrapper each call; the mod calls imageFor every frame per icon, so the pool must load once.
    void* a = imageFor("ip_armor");
    HCHECK("first imageFor loads and returns the engine wrapper", a != nullptr && nLoad == 1);
    HCHECK("repeat imageFor returns the same pointer without reloading (called every frame per icon)", imageFor("ip_armor") == a && nLoad == 1);
    HCHECK("resref is case-insensitive (pooled lowercase)", imageFor("IP_Armor") == a && nLoad == 1);
    void* b = imageFor("ip_aura");
    HCHECK("a different resref is a different pool entry", b != nullptr && b != a && nLoad == 2);
    // BuffDurationHUD.cpp:50,211 -- the disc image resref (lowercase, 14 chars, shipped in override/).
    void* disc = imageFor("buffhud_circle");
    HCHECK("the disc resref \"buffhud_circle\" pools like any icon", disc != nullptr && imageFor("buffhud_circle") == disc && !strcmp(g_loadName, "buffhud_circle"));
    // item-ability debuff icons are exactly 16 chars: the full name must reach the loader and _003/_004 must not collide.
    void* d3 = imageFor("ii_drdutldev_003");
    HCHECK("a 16-char resref reaches the loader untruncated", d3 != nullptr && !strcmp(g_loadName, "ii_drdutldev_003"));
    void* d4 = imageFor("ii_drdutldev_004");
    HCHECK("16-char resrefs differing in the last char are distinct entries", d4 != nullptr && d4 != d3);
    // BuffDurationHUD.cpp:205 -- load failure: nullptr (mod logs 'no image' and skips) and the failure is pooled, not retried every frame.
    int before = nLoad;
    HCHECK("engine load failure yields nullptr", imageFor("failnull_img") == nullptr && nLoad == before + 1);
    HCHECK("a failed load is pooled: not retried next frame", imageFor("failnull_img") == nullptr && nLoad == before + 1);
    // Pool capacity 256 vs the resrefs the mod can ask for: tables (checked in section 5) + the disc must fit with the entries above.
    int pc = 0; bool loaded = false; imageFor("ip_armor", &loaded, &pc);
    HCHECK("pool already holds the entries made above", pc >= 6 && !loaded);
    char nm[24]; bool allOk = true;
    while (pc < 256) { snprintf(nm, sizeof nm, "zfill%d", pc); bool l = false; int p2 = 0; void* r = imageFor(nm, &l, &p2); allOk &= r != nullptr && l && p2 == pc + 1; pc = p2; }
    HCHECK("pool fills to exactly 256 entries", allOk && pc == 256);
    int nl = nLoad;
    HCHECK("a full pool refuses new resrefs without loading (icon silently missing, no crash)", imageFor("one_too_many") == nullptr && nLoad == nl);
    HCHECK("a full pool still serves already-pooled resrefs", imageFor("ip_armor") == a && imageFor("buffhud_circle") == disc);
}

// ============================================================ 5. Effects / EffectIcons on fake creatures
static int g_nowDay = 100, g_nowMs = 50000; static uintptr_t g_timerSeen = 0; static int g_timeCalls = 0;
static void fGetTime(uintptr_t timer, int* day, int* ms) { ++g_timeCalls; g_timerSeen = timer; *day = g_nowDay; *ms = g_nowMs; }
struct Eff { int spell; float dur; int day, ms; };
static uintptr_t mkCreature(const Eff* e, int n) {
    uintptr_t c = arenaAlloc(0x200), arr = arenaAlloc(n * 4 + 16);
    for (int i = 0; i < n; ++i) {
        uintptr_t ef = arenaAlloc(0x40);
        at<int>(ef, 0x1c) = e[i].spell; at<float>(ef, 0x0c) = e[i].dur; at<int>(ef, 0x10) = e[i].day; at<int>(ef, 0x14) = e[i].ms;
        wptr(arr, i * 4, ef);
    }
    wptr(c, 0x148, n ? arr : 0); at<int>(c, 0x14c) = n;
    return c;
}
static void effects_contract() {
    using namespace k2fx;
    k2addr::setForTest(&kSteam);
    // BuffDurationHUD.cpp:138,171,276 -- currentTime: AppGlobal -> +8 -> +4 -> +0x10048 timer chain (Effects.h engine facts), then the engine time call.
    uintptr_t ag = arenaAlloc(16), g = arenaAlloc(16), a = arenaAlloc(16), b = arenaAlloc(0x10100), tm = arenaAlloc(16);
    wptr(ag, 0, g); wptr(g, 8, a); wptr(a, 4, b); wptr(b, 0x10048, tm);
    g_fx.appGlobal = ag; g_fx.getTime = fGetTime;
    int day = -1, ms = -1;
    HCHECK("currentTime true with a world timer", currentTime(&day, &ms));
    HCHECK("currentTime delivers the engine's (day, ms)", day == 100 && ms == 50000);
    HCHECK("the timer handed to the engine is *(*(*(*(app)+8)+4)+0x10048)", g_timerSeen == tm);
    wptr(b, 0x10048, 0); day = -1; ms = -1; int calls = g_timeCalls;
    HCHECK("timer == 0 (main menu / load) -> false, engine not called", !currentTime(&day, &ms) && g_timeCalls == calls);
    HCHECK("currentTime false leaves day/ms untouched (the mod pre-seeds 0 and passes haveTime)", day == -1 && ms == -1);
    wptr(b, 0x10048, tm); wptr(a, 4, 0);
    HCHECK("broken chain (null sub object) -> false", !currentTime(&day, &ms));
    wptr(a, 4, b);
    g_fx.appGlobal = 0;
    HCHECK("appGlobal 0 -> false (never a read of address 0)", !currentTime(&day, &ms));
    g_fx.appGlobal = ag;
    k2addr::setForTest(nullptr);
    HCHECK("unknown exe -> currentTime false", !currentTime(&day, &ms));
    k2addr::setForTest(&kSteam);

    // BuffDurationHUD.cpp:140,173,174,278,279 -- collectBuffs(creature, day, ms, haveTime, out[16], 16, debuff).
    Buff out[16];
    HEQ("creature 0 -> 0 buffs", collectBuffs(0, 100, 50000, true, out, 16, false), 0);
    HEQ("unreadable creature (0x20) -> 0 buffs", collectBuffs(0x20, 100, 50000, true, out, 16, false), 0);
    uintptr_t none = mkCreature(nullptr, 0);
    HEQ("empty effect list -> 0 buffs", collectBuffs(none, 100, 50000, true, out, 16, false), 0);
    uintptr_t big = arenaAlloc(0x200); wptr(big, 0x148, arenaAlloc(64)); at<int>(big, 0x14c) = 513;
    HEQ("effect count 513 (> 512 sanity cap) -> 0 buffs", collectBuffs(big, 100, 50000, true, out, 16, false), 0);
    at<int>(big, 0x14c) = -1;
    HEQ("negative effect count -> 0 buffs", collectBuffs(big, 100, 50000, true, out, 16, false), 0);
    uintptr_t nullarr = mkCreature(nullptr, 0); at<int>(nullarr, 0x14c) = 3;
    HEQ("null effect array with count 3 -> 0 buffs", collectBuffs(nullarr, 100, 50000, true, out, 16, false), 0);

    // buff vs debuff selection by spells.2da row (literals from EffectIconTable.inc / DebuffIconTable.inc generator output)
    const Eff mix[] = { { 8, 10.0f, 100, 55000 }, { 4, 10.0f, 100, 55000 }, { 17, 0.0f, 0, 0 } };
    uintptr_t cm = mkCreature(mix, 3);
    int n = collectBuffs(cm, 100, 50000, true, out, 16, false);
    HEQ("buff mode: rows 8 and 17 are buffs, row 4 (a debuff) is skipped", n, 2);
    HEQ("buff mode: application order, first icon", out[0].resref, "ip_speedburst");
    HEQ("buff mode: second icon", out[1].resref, "ip_armor");
    n = collectBuffs(cm, 100, 50000, true, out, 16, true);
    HEQ("debuff mode: only row 4 (ip_advlsthrow)", n, 1);
    HEQ("debuff mode: icon", out[0].resref, "ip_advlsthrow");
    // ratio: 5 s left of 10 s -> 0.5; permanent (duration 0) -> 1.0 (draws fully bright)
    n = collectBuffs(cm, 100, 50000, true, out, 16, false);
    HCHECK("ratio = remaining / duration (5000 ms of 10 s = 0.5)", near_(out[0].ratio, 0.5f));
    HCHECK("duration 0 (permanent) -> ratio 1.0", near_(out[1].ratio, 1.0f));
    n = collectBuffs(cm, 100, 50000, false, out, 16, false);
    HCHECK("haveTime false -> every ratio 1.0 (no drain without a clock)", n == 2 && near_(out[0].ratio, 1.0f));
    n = collectBuffs(cm, 100, 61000, true, out, 16, false);
    HCHECK("elapsed effect clamps to ratio 0.0, never negative (bright = 0)", n == 2 && out[0].ratio == 0.0f);
    n = collectBuffs(cm, 100, 40000, true, out, 16, false);
    HCHECK("remaining beyond the duration clamps to 1.0", n == 2 && out[0].ratio == 1.0f);
    const Eff roll[] = { { 8, 10.0f, 101, 4000 } };
    n = collectBuffs(mkCreature(roll, 1), 100, 86400000 - 1000, true, out, 16, false);
    HCHECK("day rollover: 1000 ms to midnight + 4000 ms = 5 s of 10 s", n == 1 && near_(out[0].ratio, 0.5f));
    // dedupe by spell id, item flag
    const Eff dup[] = { { 8, 10.0f, 100, 55000 }, { 8, 10.0f, 100, 52000 }, { 108, 10.0f, 100, 55000 }, { 24, 10.0f, 100, 55000 } };
    n = collectBuffs(mkCreature(dup, 4), 100, 50000, true, out, 16, false);
    HEQ("the same spell id twice yields one icon (the first)", n, 3);
    HCHECK("dedupe keeps the first application's ratio", near_(out[0].ratio, 0.5f));
    HCHECK("row 108 is an item ability: icon ip_shield on the round disc", !strcmp(out[1].resref, "ip_shield") && out[1].item);
    HCHECK("row 24 is a force power: the same icon ip_shield but no disc", !strcmp(out[2].resref, "ip_shield") && !out[2].item);
    const Eff itm[] = { { 69, 10.0f, 100, 55000 }, { 4, 10.0f, 100, 55000 }, { 116, 10.0f, 100, 55000 } };
    uintptr_t ci = mkCreature(itm, 3);
    n = collectBuffs(ci, 100, 50000, true, out, 16, false);
    HCHECK("row 69 (adrenaline stim) is a buff on the item disc: ii_adrnaline_001", n == 1 && !strcmp(out[0].resref, "ii_adrnaline_001") && out[0].item);
    n = collectBuffs(ci, 100, 50000, true, out, 16, true);
    HCHECK("debuff rows: 4 = force power (no disc), 116 = device on the item disc ii_drdutldev_003", n == 2 && !strcmp(out[0].resref, "ip_advlsthrow") && !out[0].item &&
           !strcmp(out[1].resref, "ii_drdutldev_003") && out[1].item);
    // unreadable effect pointer: skipped, the scan continues.
    uintptr_t cb = mkCreature(mix, 3); wptr(at<uint32_t>(cb, 0x148), 0, 0x20);   // first effect pointer now unreadable
    n = collectBuffs(cb, 100, 50000, true, out, 16, false);
    HCHECK("an unreadable effect is skipped and later effects still count", n == 1 && !strcmp(out[0].resref, "ip_armor"));
    // maxOut: the mod passes 16 into Buff[16]; more tracked effects than that must not overrun.
    static const int ids[] = { 8, 17, 18, 20, 22, 24, 33, 34, 36, 37, 40, 41, 42, 52, 53, 69, 70, 71, 72, 73 };   // 20 distinct tracked buff rows
    Eff many[20]; for (int i = 0; i < 20; ++i) many[i] = { ids[i], 10.0f, 100, 55000 };
    Buff guardOut[20]; for (auto& x : guardOut) x = { "sentinel", -1.0f, true };
    n = collectBuffs(mkCreature(many, 20), 100, 50000, true, guardOut, 16, false);
    HEQ("more tracked effects than maxOut: exactly maxOut (16) returned", n, 16);
    bool guardIntact = true; for (int i = 16; i < 20; ++i) guardIntact &= !strcmp(guardOut[i].resref, "sentinel");
    HCHECK("collectBuffs never writes past out[maxOut-1]", guardIntact);
    // poison (BuffDurationHUD.cpp onFrameBegin): effecticon row 110 TRAP_EFFECT_POISON has no art in the 2da table (placeholder resref) and
    // stays untracked (engine arrow) until the mod sets g_trapPoisonIcon; then it is a debuff icon, never a buff, one per creature.
    {
        const Eff p[] = { { -1, 10.0f, 100, 55000 }, { -1, 10.0f, 100, 55000 } };
        uintptr_t cp = mkCreature(p, 2);
        for (int i = 0; i < 2; ++i) {
            uintptr_t ef = at<uint32_t>(at<uint32_t>(cp, 0x148), i * 4), ints = arenaAlloc(8);
            at<unsigned short>(ef, 0x08) = EffType_Icon; at<int>(ef, 0x30) = 1; wptr(ef, 0x34, ints); at<int>(ints, 0) = EffIconRow_TrapPoison;
        }
        HCHECK("row 110 carries no table icon (placeholder ip_resistenergy)", kEffectIconRows[EffIconRow_TrapPoison].resref == nullptr);
        HEQ("poison row without the override: untracked (engine arrow)", collectBuffs(cp, 100, 50000, true, out, 16, true), 0);
        g_trapPoisonIcon = "bh_poison";
        n = collectBuffs(cp, 100, 50000, true, out, 16, true);
        HCHECK("poison row with the override: one debuff icon bh_poison (two effects deduped), no disc", n == 1 && !strcmp(out[0].resref, "bh_poison") && !out[0].item);
        HCHECK("poison override draws with the effect's drain ratio", near_(out[0].ratio, 0.5f));
        HEQ("poison row is never a buff", collectBuffs(cp, 100, 50000, true, out, 16, false), 0);
        g_trapPoisonIcon = nullptr;
    }
    k2addr::setForTest(nullptr);
    HEQ("unknown exe: collectBuffs finds nothing (creature offsets unknown)", collectBuffs(cm, 100, 50000, true, out, 16, false), 0);
    k2addr::setForTest(&kSteam);
    g_fx.appGlobal = k2addr::UseTable; g_fx.getTime = &defaultGetTime;

    // ---- EffectIcons tables (BuffDurationHUD.cpp:140,173,174 reach them through collectBuffs) ----
    bool item = true;
    HCHECK("iconForSpell row < 0 -> nullptr, item false", iconForSpell(-1, false, &item) == nullptr && !item);
    item = true;
    HCHECK("iconForSpell row 287 (== table size) -> nullptr, item false", iconForSpell(287, true, &item) == nullptr && !item);
    HEQ("buff table size is 287 spells.2da rows", kEffectIconTableSize, 287);
    HEQ("debuff table size is 287 spells.2da rows", kDebuffIconTableSize, 287);
    HCHECK("iconForSpell row 286 (last) is in range and untracked", iconForSpell(286, false, &item) == nullptr);
    HCHECK("row 8 -> ip_speedburst", !strcmp(iconForSpell(8, false, &item), "ip_speedburst") && !item);
    HCHECK("row 8 has no debuff icon", iconForSpell(8, true, &item) == nullptr);
    HCHECK("row 4 has no buff icon", iconForSpell(4, false, &item) == nullptr);
    HCHECK("row 4 debuff -> ip_advlsthrow", !strcmp(iconForSpell(4, true, &item), "ip_advlsthrow") && !item);
    HCHECK("row 257 buff -> ip_shield on the disc", !strcmp(iconForSpell(257, false, &item), "ip_shield") && item);
    HCHECK("row 129 buff -> ii_medeqpmnt_005 on the disc", !strcmp(iconForSpell(129, false, &item), "ii_medeqpmnt_005") && item);
    HCHECK("row 270 debuff -> ip_breath (last debuff row)", !strcmp(iconForSpell(270, true, &item), "ip_breath") && !item);
    // item-ability rows (disc) boundaries: 69-77, 99-108, 110-115, 129, 132, 257
    HCHECK("item rows 69 and 77 are item abilities; 78 is not", isItemAbility(69) && isItemAbility(77) && !isItemAbility(78) && !isItemAbility(68));
    HCHECK("item rows 99 and 108 are item abilities; 98 and 109 are not", isItemAbility(99) && isItemAbility(108) && !isItemAbility(98) && !isItemAbility(109));
    HCHECK("item rows 110 and 115 are item abilities; 116 is not", isItemAbility(110) && isItemAbility(115) && !isItemAbility(116));
    HCHECK("item rows 129, 132, 257 are item abilities; 130, 131, 256 are not", isItemAbility(129) && isItemAbility(132) && isItemAbility(257) &&
           !isItemAbility(130) && !isItemAbility(131) && !isItemAbility(256));
    HCHECK("force-power rows 8 and 24 are not item abilities", !isItemAbility(8) && !isItemAbility(24));
    // every resref imageFor will see is <= 16 chars (imageFor truncates at 16) and the distinct set + the disc fits the 256-entry pool.
    std::set<std::string> distinct; size_t maxLen = 0; bool lower = true; int nBuff = 0, nDebuff = 0;
    for (int i = 0; i < 287; ++i) {
        if (kEffectIcons[i]) { ++nBuff; distinct.insert(kEffectIcons[i]); maxLen = std::max(maxLen, strlen(kEffectIcons[i])); }
        if (kDebuffIcons[i].resref) { ++nDebuff; distinct.insert(kDebuffIcons[i].resref); maxLen = std::max(maxLen, strlen(kDebuffIcons[i].resref)); }
    }
    // effecticon.2da rows (script EffectIcon, type 0x43) feed the same imageFor pool via iconForEffectIconEffect (R2-BH-10).
    int nRows = 0;
    for (int i = 0; i < kEffectIconRowCount; ++i) {
        if (!kEffectIconRows[i].resref) continue;
        ++nRows; distinct.insert(kEffectIconRows[i].resref); maxLen = std::max(maxLen, strlen(kEffectIconRows[i].resref));
    }
    for (const std::string& s : distinct) for (char ch : s) lower &= !(ch >= 'A' && ch <= 'Z');
    HCHECK("effecticon row table carries resrefs (row 0 is the none row)", nRows >= 40 && kEffectIconRows[0].resref == nullptr);
    HCHECK("tables track a non-trivial number of buff and debuff rows", nBuff >= 40 && nDebuff >= 40);
    HCHECK("no icon resref exceeds 16 chars (imageFor would truncate and collide)", maxLen <= 16);
    HCHECK("every icon resref is lowercase (engine resrefs, matches imageFor's pooling key)", lower);
    HCHECK("distinct icon resrefs + the disc fit imageFor's 256-entry pool (otherwise icons silently vanish)", distinct.size() + 1 <= 256);
    HCHECK("the disc resref is not also an effect icon", distinct.count("buffhud_circle") == 0);
}

// ============================================================ 6. k2ps::Client as KPatchInit drives it
// BuffDurationHUD.cpp:356-363 -- init, onConnect, add x3 (phase, K2PS_PRIO_BUFF), connect. Only what buff's call sites need; the registry edge
// cases are portrait_slots_src/ps_registry_test.cpp and ps_dispatch_test.cpp.
static_assert(K2PS_PRE_ENGINE_READ == 1 && K2PS_FRAME_BEGIN == 2 && K2PS_MENU_OVERLAY == 4,
              "buff relies on: phase ids PRE_ENGINE_READ 1, FRAME_BEGIN 2, MENU_OVERLAY 4 (the three phases KPatchInit registers)");
static_assert(K2PS_PRIO_BUFF == 200 && K2PS_PRIO_SHIELD == 100 && K2PS_PRIO_BUFF > K2PS_PRIO_SHIELD,
              "buff relies on: K2PS_PRIO_BUFF == 200, after the shield bar's 100 (BuffDurationHUD.cpp:11, 333)");
static_assert(K2PS_VERSION == 1 && K2PS_PHASE_COUNT == 6, "buff relies on: k2.portraitslots version 1");
static_assert(std::is_same<K2PsCallback, void (KPATCH_CALL*)(const K2PsContext*, void*)>::value,
              "buff relies on: K2PsCallback is void(const K2PsContext*, void*) (onPreEngineRead/onFrameBegin/onMenuOverlay, BuffDurationHUD.cpp:317-334)");
// BuffDurationHUD.cpp:318,323,335,342-344 -- the K2PsContext fields the callbacks read (frame, slot, companions, companionCtrl).
static_assert(offsetof(K2PsContext, slot) == 8 && offsetof(K2PsContext, frame) == 24 && offsetof(K2PsContext, companions) == 56 &&
              offsetof(K2PsContext, companionCtrl) == 64, "buff relies on: K2PsContext slot @8, frame @24, companions @56, companionCtrl @64");

struct RegRec { int32_t phase, prio; K2PsCallback fn; void* user; std::string owner; };
static std::vector<RegRec> g_regs; static int g_refusePhase = -1, g_nextToken = 1;
static int32_t KPATCH_CALL pReg(int32_t ph, int32_t prio, K2PsCallback fn, void* user, const char* owner) {
    g_regs.push_back({ ph, prio, fn, user, owner ? owner : "(null)" });
    return ph == g_refusePhase ? 0 : g_nextToken++;
}
static void KPATCH_CALL pUnreg(int32_t) {}
static int32_t KPATCH_CALL pCreature(int32_t s) { return s == 0x10 ? 0x1000 : 0; }
static int32_t KPATCH_CALL pSlotAt(int32_t, int32_t*, int32_t*) { return 0; }
static uint32_t KPATCH_CALL pFrame() { return 7; }
static const K2PortraitSlotsApi g_goodTable = { sizeof(K2PortraitSlotsApi), pReg, pUnreg, pCreature, pSlotAt, pFrame };
static const K2PortraitSlotsApi g_tinyTable = { 4, pReg, pUnreg, pCreature, pSlotAt, pFrame };   // struct_size too small to reach Frame

static void KPATCH_CALL cbPre(const K2PsContext*, void*) {}
static void KPATCH_CALL cbFrame(const K2PsContext*, void*) {}
static void KPATCH_CALL cbMenu(const K2PsContext*, void*) {}

// The mod's connect state (BuffDurationHUD.cpp:67-69): last outcome wins.
static int g_connCalls = 0; static bool g_connOk = false; static std::string g_connWhy;
static void onConnect(bool ok, const char* why) { ++g_connCalls; g_connOk = ok; g_connWhy = why ? why : "(null)"; }
static void resetConn() { g_regs.clear(); g_refusePhase = -1; g_nextToken = 1; g_connCalls = 0; g_connOk = false; g_connWhy.clear(); }
static bool whyHas(const char* s) { return g_connWhy.find(s) != std::string::npos; }
// KPatchInit, verbatim from the mod (BuffDurationHUD.cpp:357-362).
static void modInit(k2ps::Client& ps, const KPatchApi* api) {
    ps.init(api, "buff-duration-hud");
    ps.onConnect = onConnect;
    ps.add(K2PS_PRE_ENGINE_READ, K2PS_PRIO_BUFF, cbPre);
    ps.add(K2PS_FRAME_BEGIN, K2PS_PRIO_BUFF, cbFrame);
    ps.add(K2PS_MENU_OVERLAY, K2PS_PRIO_BUFF, cbMenu);
    ps.connect();
}
// Drive a client the way the hooks do: many poll() calls (BuffDurationHUD.cpp:126,239) and a few direct connect()s.
static void hammer(k2ps::Client& ps) { for (int i = 0; i < 600; ++i) ps.poll(); for (int i = 0; i < 3; ++i) ps.connect(); }

static void k2ps_contract() {
    // BuffDurationHUD.cpp:357-362 -- registration shape: three Register calls in add() order, owner = manifest id, prio 200, user pointer null.
    {
        fakekp::reset(); resetConn(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_goodTable);
        k2ps::Client ps; modInit(ps, fakekp::api());
        HCHECK("provider present at init: connected", ps.connected);
        HEQ("three registrations", (int)g_regs.size(), 3);
        bool shape = g_regs.size() == 3 &&
            g_regs[0].phase == 1 && g_regs[0].prio == 200 && g_regs[0].fn == cbPre &&
            g_regs[1].phase == 2 && g_regs[1].prio == 200 && g_regs[1].fn == cbFrame &&
            g_regs[2].phase == 4 && g_regs[2].prio == 200 && g_regs[2].fn == cbMenu;
        HCHECK("registered in add() order: (1,200,onPreEngineRead) (2,200,onFrameBegin) (4,200,onMenuOverlay)", shape);
        bool owners = g_regs.size() == 3; for (const RegRec& r : g_regs) owners &= r.owner == "buff-duration-hud" && r.user == nullptr;
        HCHECK("owner is \"buff-duration-hud\" and the user pointer is null (the callbacks ignore it)", owners);
        HCHECK("one success report: onConnect(true, \"registered\")", g_connCalls == 1 && g_connOk && whyHas("registered"));
        HEQ("the interface name asked of the registry is k2.portraitslots", K2PS_IFACE, "k2.portraitslots");
        // BuffDurationHUD.cpp:70,133,243,267 -- creatureForSlot goes through the provider once connected, and is 0 before.
        HEQ("connected: creatureForSlot asks the provider", ps.creatureForSlot(0x10), 0x1000);
        HEQ("connected: unknown slot -> 0 (hooks return, engine arrows draw)", ps.creatureForSlot(0x99), 0);
        k2ps::Client lone; lone.init(fakekp::api(), "x");
        HEQ("never connected: creatureForSlot is 0", lone.creatureForSlot(0x10), 0);
    }
    // ---- pinned-as-is, undecided (doc 21 backlog, commit 2296ab1): do not "fix" without deciding in doc 21 ----
    // BuffDurationHUD.cpp:69,310-314,322-330 -- the provider refuses SOME registrations (e.g. the FRAME_BEGIN kill-switch phase).
    {
        fakekp::reset(); resetConn(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_goodTable);
        g_refusePhase = K2PS_FRAME_BEGIN;
        k2ps::Client ps; modInit(ps, fakekp::api());
        HCHECK("partial refusal: the client still reports connected (the mod keeps drawing via the phases that registered)", ps.connected);
        HEQ("all three registrations were attempted", (int)g_regs.size(), 3);
        HCHECK("partial refusal: onConnect(false, \"some registrations refused\") once", g_connCalls == 1 && !g_connOk && whyHas("some registrations refused"));
        hammer(ps);
        HEQ("refused registrations are never retried (poll/connect are no-ops once connected)", (int)g_regs.size(), 3);
        HEQ("no second onConnect after the partial refusal", g_connCalls, 1);
        HEQ("no second require() either: connected clients stop asking the registry", fakekp::state().requireCalls, 1);
        HEQ("the provider is still used for slot lookups after a partial refusal", ps.creatureForSlot(0x10), 0x1000);
    }
    // BuffDurationHUD.cpp:69,310-314 -- the failure path reports exactly once, whatever the cause. One block per cause.
    {   // (a) no KPatchApi at all (patcher predates KPatchInit)
        resetConn(); k2ps::Client ps; modInit(ps, nullptr); hammer(ps);
        HCHECK("cause: null api -> one failure report, \"no KPatchApi\"", g_connCalls == 1 && !g_connOk && whyHas("no KPatchApi"));
        HCHECK("cause: null api -> never connected, nothing registered", !ps.connected && g_regs.empty());
    }
    {   // (b) KPatchApi too small to carry require()
        fakekp::reset(8); resetConn(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_goodTable);
        k2ps::Client ps; modInit(ps, fakekp::api()); hammer(ps);
        HCHECK("cause: short KPatchApi -> one failure report, \"no KPatchApi\"", g_connCalls == 1 && !g_connOk && whyHas("no KPatchApi"));
        HCHECK("cause: short KPatchApi -> require() never called", fakekp::state().requireCalls == 0 && !ps.connected && g_regs.empty());
    }
    {   // (c) require is a null function pointer
        fakekp::reset(); fakekp::state().api.require = nullptr; resetConn();
        k2ps::Client ps; modInit(ps, fakekp::api()); hammer(ps);
        HCHECK("cause: null require pointer -> one failure report", g_connCalls == 1 && !g_connOk && whyHas("no KPatchApi") && !ps.connected);
    }
    {   // (d) provider not installed / loaded later
        fakekp::reset(); resetConn();
        k2ps::Client ps; modInit(ps, fakekp::api()); hammer(ps);
        HCHECK("cause: provider missing -> one failure report, \"not provided\"", g_connCalls == 1 && !g_connOk && whyHas("not provided"));
        HCHECK("cause: provider missing -> keeps polling the registry but never reports again", !ps.connected && fakekp::state().requireCalls >= 3 && g_regs.empty());
    }
    {   // (e) provider table too small
        fakekp::reset(); resetConn(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_tinyTable);
        k2ps::Client ps; modInit(ps, fakekp::api()); hammer(ps);
        HCHECK("cause: provider table too small -> one failure report, \"table too small\"", g_connCalls == 1 && !g_connOk && whyHas("table too small"));
        HCHECK("cause: provider table too small -> nothing registered, still unconnected", !ps.connected && g_regs.empty());
    }
    {   // (f) the cause CHANGES between attempts: still exactly one failure report in total, and a later success is reported.
        fakekp::reset(); resetConn();
        k2ps::Client ps; modInit(ps, fakekp::api());
        HCHECK("changing cause: first attempt (provider missing) reports once", g_connCalls == 1 && whyHas("not provided"));
        fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_tinyTable);
        ps.connect(); hammer(ps);
        HCHECK("changing cause: a different failure (table too small) is NOT reported again", g_connCalls == 1 && whyHas("not provided") && !ps.connected);
        fakekp::reset(); fakekp::provide(K2PS_IFACE, K2PS_VERSION, &g_goodTable);   // same api pointer, now a good provider
        HCHECK("changing cause: connect() succeeds once a good provider appears", ps.connect() && ps.connected);
        HCHECK("changing cause: the success is reported (g_connectOk flips true)", g_connCalls == 2 && g_connOk && whyHas("registered"));
        HEQ("changing cause: the three phases registered on the late connect", (int)g_regs.size(), 3);
    }
}

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    k2addr::setForTest(&kSteam);
    k2addr_contract();
    saferead_contract();
    gui2d_contract();
    guikit_contract();
    effects_contract();
    k2ps_contract();
    return kt::Report("common_contract_test");
}
