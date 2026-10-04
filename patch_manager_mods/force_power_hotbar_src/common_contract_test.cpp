// common_contract_test.cpp -- Patches/Common CONTRACT test for the Force Power Hotbar (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/{saferead,guikit,party}_test.cpp). Every message starts with
// "hotbar relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to
// SafeRead.h / GuiKit.h / Party.h / GameAddr.h re-runs this test: a Common change that would break the hotbar fails here first.
//
// Areas: (1) gameaddr constants (compile time), (2) saferead, (3) guikit on fake engine objects in a MAP_32BIT arena,
// (4) party on a fake app -> a -> b -> table chain with fake vtable HP slots, (5) filewatch (INI reload poll + atomic save) in a temp dir,
// (6) KpLog.h (init fail-soft, channel names, WARN without ini, KP_LOG_EVERY, 511 cut, fallback file format + 60000-line cap). Host-only, deterministic, no game, no DLL.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SafeRead.h"
#include "GameAddr.h"
#include "GuiKit.h"
#include "Party.h"
#include "FileWatch.h"
#include "KpLog.h"
#include "check.h"
#include <sys/mman.h>
#include <type_traits>
#include <string>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <vector>

#define HCHECK(why, expr) ::kt::Check("hotbar relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("hotbar relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

// ============================================================ 1. GameAddr (compile time)
// Values validated in 07_force_power_hotbar.md / ForcePowerHotbar.cpp comments / the Steam Address DB. A changed value means the
// hotbar would patch or call the wrong address.
#define HADDR(name, val) static_assert(gameaddr::name == (val), "hotbar relies on: gameaddr::" #name " == " #val " (validated Steam address)")
HADDR(AppGlobal, 0x00A1B4A4);              // app -> +4 -> +4 -> +0x270 party table chain; also the save-request holder
HADDR(GetControlledCreature, 0x007E5D80);
HADDR(PartyGetAt, 0x007E5DA0);             // member by party POSITION (index 0 = controlled)
HADDR(MemberServerCreature, 0x0077D800);
HADDR(ClientPartyTable, 0x0073FB90);
HADDR(New, 0x00919723);
HADDR(LabelCtor, 0x00419740);
HADDR(InitControl, 0x0040F620);
HADDR(StopLoadFromLayout, 0x0040F5A0);
HADDR(ExoCtor, 0x00733570);
HADDR(ExoDtor, 0x00733780);
HADDR(TextSet, 0x00416E30);
HADDR(SetFont, 0x00416DE0);
HADDR(SetAlign, 0x00416FA0);
HADDR(TextColor, 0x00417140);
HADDR(PtrListAdd, 0x0083EA60);
HADDR(GuiLoadImage, 0x0047EB60);
HADDR(GuiSoundMgr, 0x00A1B49C);
HADDR(PlayGuiSound, 0x004122A0);
HADDR(GuiRefW, 0x009F4390);
HADDR(GuiRefH, 0x009F4394);
HADDR(ProjectionFlag, 0x00A1B9F0);
HADDR(LabelVtable, 0x009878BC);
HADDR(BorderVtable, 0x009875BC);
HADDR(TextVtable, 0x009876B4);
HADDR(KeyboardDeviceGlobal, 0x0099C884);
HADDR(TexPendingList, 0x00A1BC2C);
// GuiKit keeps its own copies of the addresses the hotbar also names through gameaddr; the two must agree.
static_assert(gameaddr::New == guikit::AddrNew && gameaddr::LabelCtor == guikit::AddrLabelCtor && gameaddr::InitControl == guikit::AddrInitControl,
              "hotbar relies on: gameaddr and guikit agree on New/LabelCtor/InitControl");
static_assert(gameaddr::ExoCtor == guikit::AddrExoCtor && gameaddr::ExoDtor == guikit::AddrExoDtor && gameaddr::TextSet == guikit::AddrTextSet &&
              gameaddr::TextColor == guikit::AddrTextColor && gameaddr::PtrListAdd == guikit::AddrPtrListAdd && gameaddr::GuiLoadImage == guikit::AddrLoadImage,
              "hotbar relies on: gameaddr and guikit agree on the text / list / image engine functions");
static_assert(gameaddr::GuiSoundMgr == guikit::AddrGuiSoundMgr && gameaddr::PlayGuiSound == guikit::AddrPlayGuiSound,
              "hotbar relies on: gameaddr and guikit agree on the GUI sound manager (hotbar logs gameaddr::GuiSoundMgr, guikit plays through its copy)");
static_assert(gameaddr::LabelVtable == guikit::LabelVtable && gameaddr::BorderVtable == guikit::BorderVtable && gameaddr::TextVtable == guikit::TextVtable,
              "hotbar relies on: gameaddr and guikit agree on the label/border/text vtables (the hotbar gates its own reads with gameaddr's)");
// The hotbar keeps local copies of the control-layout offsets (ForcePowerHotbar.cpp 52-90); GuiKit's must match what it reads/writes itself.
static_assert(guikit::LabelSize == 0x148 && guikit::Ctl_Flags == 0x48 && guikit::Ctl_Id == 0x54 && guikit::Label_Border == 0x60 &&
              guikit::Border_Alpha == 0x24 && guikit::Border_Fill == 0x74 && guikit::Ctl_W == 0xC && guikit::Ctl_H == 0x10,
              "hotbar relies on: GuiKit control/border layout offsets equal the hotbar's local Ctl_/Border_/Label_ constants");
static_assert(guikit::Panel_CtlArray == 0x24 && guikit::Panel_CtlCount == 0x28 && guikit::Label_Text == 0xD8 &&
              guikit::Text_Renderer == 0x14 && guikit::Text_StringSub == 0x18,
              "hotbar relies on: GuiKit panel/text offsets equal the hotbar's local Panel_/Label_Text/Text_ constants");
static_assert(guikit::ImagePoolSize == 256, "hotbar relies on: imageFor pool size 256 (its 'img: pool full' diagnosis compares against ImagePoolSize)");
static_assert(guikit::CF_None == 0 && guikit::CF_Alloc != guikit::CF_Vtable && guikit::CF_Vtable != guikit::CF_Template && guikit::CF_Alloc != guikit::CF_Template,
              "hotbar relies on: CF_None == 0 and CF_Alloc/CF_Vtable/CF_Template are distinct (createLabel switches on them)");
static_assert(party::SlotMaxHp == 0x98 && party::SlotCurHp == 0x9C, "hotbar relies on: HP vtable slots max +0x98, cur +0x9C");
static_assert(std::is_same<decltype(&party::callVitalSlot), int (*)(uintptr_t, uintptr_t, int)>::value,
              "hotbar relies on: callVitalSlot(fn, creature, arg) takes exactly ONE engine arg (a 2-arg call crashed the game)");

// ============================================================ arena + fakes
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { std::printf("arena full\n"); std::exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a;
}
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// ---- fake engine (guikit) ----
static int nAlloc = 0, nDtor = 0, nInit = 0, nTextSet = 0, nTextColor = 0, nFont = 0, nAlign = 0, nSetRect = 0, nSound = 0, nListAdd = 0, nLoad = 0;
static bool g_allocFails = false, g_initWritesId = true; static uint32_t g_ctorVt = guikit::LabelVtable;
static int g_initAdd = -1, g_initScale = -1; static char g_initTag[16];
static void* g_tsThis = nullptr; static char g_tsStr[64];
static void* g_tcThis = nullptr; static float g_tcRgb[3];
static void* g_fontThis = nullptr; static char g_fontStr[16];
static void* g_alThis = nullptr; static unsigned g_alVal = 0;
static void* g_sndMgr = nullptr; static int g_sndIdx = -1;
static uint32_t g_srFn = 0; static uintptr_t g_srCtl = 0; static int g_srRect[4];
static int g_listMode = 0;   // 0 normal, 1 count not bumped, 2 wrong slot stored
static void* g_loadResult = reinterpret_cast<void*>(0x5000);

static void* fAlloc(unsigned n) { ++nAlloc; return g_allocFails ? nullptr : reinterpret_cast<void*>(arenaAlloc(n)); }
static void fCtor(void* t) { uintptr_t c = reinterpret_cast<uintptr_t>(t); at<uint32_t>(c, 0) = g_ctorVt; at<int>(c, guikit::Ctl_Flags) = guikit::FlagVisible; }
static void* fExoCtor(void* e, const char* s) { memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fExoDtor(void*) { ++nDtor; }
static void fInit(void*, void* ctl, void* exo, int add, int scale) {
    ++nInit; g_initAdd = add; g_initScale = scale; memcpy(g_initTag, exo, 16);
    if (g_initWritesId) at<int>(reinterpret_cast<uintptr_t>(ctl), guikit::Ctl_Id) = -1;
}
static void fTextSet(void* t, void* exo) { ++nTextSet; g_tsThis = t; strncpy(g_tsStr, static_cast<char*>(exo), sizeof g_tsStr - 1); }
static void fTextColor(void* t, float* rgb) { ++nTextColor; g_tcThis = t; for (int k = 0; k < 3; ++k) g_tcRgb[k] = rgb[k]; }
static void fSetFont(void* t, const char* rr) { ++nFont; g_fontThis = t; memcpy(g_fontStr, rr, 16); }
static void fSetAlign(void* t, unsigned a) { ++nAlign; g_alThis = t; g_alVal = a; }
static void fPlaySound(void* mgr, char idx) { ++nSound; g_sndMgr = mgr; g_sndIdx = idx; }
static void* fLoadImage(const char*) { ++nLoad; return g_loadResult; }
static void fListAdd(void* list, void* v) {   // CExoArrayList<void*> {ptr @+0, count @+4}; list = panel+0x24
    ++nListAdd; uintptr_t l = reinterpret_cast<uintptr_t>(list);
    if (g_listMode == 1) return;
    int cnt = at<int>(l, 4); uint32_t arr = at<uint32_t>(l, 0);
    at<uint32_t>(arr, cnt * 4) = g_listMode == 2 ? 0xDEAD0000u : static_cast<uint32_t>(reinterpret_cast<uintptr_t>(v));
    at<int>(l, 4) = cnt + 1;
}
static void fCallSetRect(uint32_t fn, uintptr_t ctl, int* r) {
    ++nSetRect; g_srFn = fn; g_srCtl = ctl; for (int k = 0; k < 4; ++k) { g_srRect[k] = r[k]; at<int>(ctl, guikit::Ctl_X + 4 * k) = r[k]; }   // the engine's SetRect stores the extent
}

// A fake label: valid border vtable at +0x60, valid text object (vtable + non-null renderer) at +0xD8, real arena vtable (slot 1 non-zero) at +0.
static uintptr_t g_labelVt;
static uintptr_t mkLabel() {
    uintptr_t c = arenaAlloc(guikit::LabelSize);
    at<uint32_t>(c, 0) = static_cast<uint32_t>(g_labelVt);
    at<uint32_t>(c, guikit::Label_Border) = guikit::BorderVtable;
    at<uint32_t>(c, guikit::Label_Text) = guikit::TextVtable;
    at<uint32_t>(c, guikit::Label_Text + guikit::Text_Renderer) = 0x1234;
    return c;
}

// ============================================================ 2. SafeRead
static void saferead_contract() {
    uintptr_t buf = arenaAlloc(64);
    at<uint32_t>(buf, 8) = 0xCAFEF00D; at<uint8_t>(buf, 12) = 0x7B;

    // readAt: readable -> true + value; unreadable -> false and the out value is left exactly as it was (the hotbar pre-seeds defaults and
    // trusts them on failure: rdInt(..., dflt), `int fl = 0`, `float ba = 1.0f`).
    saferead::beginScope();
    uint32_t v = 0x11111111;
    HCHECK("readAt on readable memory returns true", saferead::readAt(1u, buf, 8, &v));
    HEQ("readAt on readable memory delivers the value", (unsigned long long)v, 0xCAFEF00Dull);
    uint8_t b8 = 0; HCHECK("readAt<uint8_t> works (target quirk readers)", saferead::readAt<uint8_t>(1u, buf, 12, &b8)); HEQ("readAt<uint8_t> value", b8, 0x7B);
    float f = 1.0f; at<float>(buf, 16) = 0.5f; HCHECK("readAt<float> works (border alpha)", saferead::readAt(1u, buf, 16, &f)); HCHECK("readAt<float> value", f == 0.5f);
    v = 0x22222222;
    HCHECK("readAt below 0x10000 returns false", !saferead::readAt(1u, 0x20, 0, &v));
    HEQ("readAt failure leaves the out value untouched (low address)", (unsigned long long)v, 0x22222222ull);
    HCHECK("readAt with base 0 returns false", !saferead::readAt(1u, 0, 0x30, &v));
    HEQ("readAt failure leaves the out value untouched (null base)", (unsigned long long)v, 0x22222222ull);
    HCHECK("readAt with base 0xFFFFFFF0 + off wrapping to unreadable low memory is not reported readable", !saferead::readAt<uint32_t>(1u, 0x8, 4, &v));
    HEQ("readAt failure leaves the out value untouched (base+off < 0x10000)", (unsigned long long)v, 0x22222222ull);
    int ri = -7; HCHECK("readAt<int> failure reports false", !saferead::readAt(1u, 0x1000, 0, &ri)); HEQ("readAt<int> failure keeps the default", ri, -7);

    // probeRead
    HCHECK("probeRead true on a readable range", saferead::probeRead(2u, reinterpret_cast<const void*>(buf), 16));
    HCHECK("probeRead false on an unreadable address", !saferead::probeRead(2u, reinterpret_cast<const void*>(0x20), 4));
    HCHECK("probeRead false on null", !saferead::probeRead(2u, nullptr, 1));
    HCHECK("probeRead false when the range wraps the address space", !saferead::probeRead(2u, reinterpret_cast<const void*>(~uintptr_t(0) - 2), 16));

    // beginScope: readable regions are cached per hook; beginScope must drop them, because engine code that runs between hook calls can
    // free memory (the load-hang lesson, doc 09 section 10). Every hotbar hook entry calls it.
    saferead::beginScope();
    HCHECK("setup: region is readable and now cached", saferead::probeRead(3u, reinterpret_cast<const void*>(buf), 4));
    g_stubVirtualQueryAll = false;   // the "memory" is freed behind our back
    saferead::beginScope();
    HCHECK("beginScope drops cached regions (freed memory is not reported readable in the next hook)", !saferead::probeRead(3u, reinterpret_cast<const void*>(buf), 4));
    v = 0x33333333;
    HCHECK("readAt after beginScope re-queries (false once unreadable)", !saferead::readAt(3u, buf, 8, &v));
    HEQ("readAt re-query failure leaves the out value untouched", (unsigned long long)v, 0x33333333ull);
    g_stubVirtualQueryAll = true; saferead::beginScope();
    HCHECK("beginScope then readable again -> true", saferead::readAt(3u, buf, 8, &v));

    // readExo: entry names. The hotbar calls readExo(site, addr, buf[96], 96) with the defaults maxLen 80, printableOnly.
    char out[96];
    auto mkExo = [&](const void* bytes, int n, int len) {
        uintptr_t data = arenaAlloc(n > 0 ? n : 1); if (n > 0) memcpy(reinterpret_cast<void*>(data), bytes, n);
        uintptr_t st = arenaAlloc(8); at<uint32_t>(st, 0) = (uint32_t)data; at<int>(st, 4) = len; return st;
    };
    auto fill = [&] { memset(out, 'X', sizeof out); };
    fill(); uintptr_t e = mkExo("Cure\0", 5, 5);
    HEQ("readExo strips the engine's trailing NUL and returns the length", saferead::readExo(4u, e, out, sizeof out), 4);
    HEQ("readExo gives the NUL-terminated name", (const char*)out, "Cure");
    fill(); e = mkExo("Force Push", 10, 10);
    HEQ("readExo accepts a name whose length excludes the NUL", saferead::readExo(4u, e, out, sizeof out), 10);
    HEQ("readExo name without NUL is terminated", (const char*)out, "Force Push");
    char eighty[81]; memset(eighty, 'A', 80); eighty[80] = 0;
    fill(); e = mkExo(eighty, 80, 80);
    HEQ("readExo accepts an 80-char name (maxLen)", saferead::readExo(4u, e, out, sizeof out), 80);
    HEQ("readExo 80-char name is complete and terminated", (int)strlen(out), 80);
    char big[100]; memset(big, 'B', 99); big[99] = 0;
    fill(); e = mkExo(big, 90, 90);
    HEQ("readExo REJECTS (does not truncate) a name over maxLen 80", saferead::readExo(4u, e, out, sizeof out), 0);
    HEQ("readExo rejection leaves an empty buffer", (int)out[0], 0);
    fill(); e = mkExo(eighty, 80, 80);
    HEQ("readExo rejects a name that does not fit the cap (len >= cap)", saferead::readExo(4u, e, out, 80), 0);
    HEQ("readExo cap rejection leaves an empty buffer", (int)out[0], 0);
    fill(); e = mkExo("", 0, 0);
    HEQ("readExo empty string (len 0) returns 0", saferead::readExo(4u, e, out, sizeof out), 0);
    HEQ("readExo empty string leaves an empty buffer", (int)out[0], 0);
    fill(); e = mkExo("\0\0\0", 3, 3);
    HEQ("readExo all-NUL string returns 0", saferead::readExo(4u, e, out, sizeof out), 0);
    HEQ("readExo all-NUL string leaves an empty buffer", (int)out[0], 0);
    fill(); uintptr_t st = arenaAlloc(8); at<uint32_t>(st, 0) = 0; at<int>(st, 4) = 5;
    HEQ("readExo null string pointer returns 0", saferead::readExo(4u, st, out, sizeof out), 0);
    HEQ("readExo null string pointer leaves an empty buffer", (int)out[0], 0);
    fill(); st = arenaAlloc(8); at<uint32_t>(st, 0) = 0x20; at<int>(st, 4) = 5;
    HEQ("readExo unreadable string pointer returns 0", saferead::readExo(4u, st, out, sizeof out), 0);
    fill(); HEQ("readExo unreadable CExoString address returns 0", saferead::readExo(4u, 0x20, out, sizeof out), 0);
    HEQ("readExo unreadable CExoString address leaves an empty buffer", (int)out[0], 0);
    fill(); e = mkExo("Ab\x01" "c", 4, 4);
    HEQ("readExo rejects non-printable bytes (default printableOnly)", saferead::readExo(4u, e, out, sizeof out), 0);
    HEQ("readExo non-printable rejection leaves an empty buffer", (int)out[0], 0);
    fill(); e = mkExo("Hi", 2, -2);
    HEQ("readExo negative length returns 0", saferead::readExo(4u, e, out, sizeof out), 0);
    HEQ("readExo with null buffer returns 0 without crashing", saferead::readExo(4u, e, nullptr, 10), 0);
}

// ============================================================ 3. GuiKit
static void guikit_contract() {
    using namespace guikit;
    Engine fresh;   // defaults, before the fakes go in
    HEQ("default Engine plays the hover sound through gameaddr::GuiSoundMgr", (unsigned long long)fresh.guiSoundMgrAddr, (unsigned long long)gameaddr::GuiSoundMgr);
    HEQ("default Engine screen globals are the engine framebuffer size", (unsigned long long)fresh.screenWAddr, (unsigned long long)gameaddr::ScreenW);

    Engine& e = g_gk;
    e.alloc = fAlloc; e.labelCtor = fCtor; e.initControl = fInit; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.textSet = fTextSet;
    e.textColor = fTextColor; e.setFont = fSetFont; e.setAlign = fSetAlign; e.listAdd = fListAdd; e.loadImage = fLoadImage;
    e.playSound = fPlaySound; e.callSetRect = fCallSetRect;
    g_labelVt = arenaAlloc(0x100); at<uint32_t>(g_labelVt, 4) = 0x401000;   // slot 1 = SetRect

    // ---- getTextColor / setTextColor: the gray "0" out-of-stock text reads the colour back from label+0xF0+0x2C, then sets it ----
    uintptr_t L = mkLabel();
    at<float>(L, Label_Text + Text_StringSub + Text_Color + 0) = 0.25f;
    at<float>(L, Label_Text + Text_StringSub + Text_Color + 4) = 0.5f;
    at<float>(L, Label_Text + Text_StringSub + Text_Color + 8) = 0.75f;
    float rgb[3] = { -1, -1, -1 };
    HCHECK("getTextColor succeeds on a label with a valid text object", getTextColor(L, rgb));
    HCHECK("getTextColor reads RGB at label+0xD8+0x18+0x2C (= label+0x120), 3 consecutive floats", rgb[0] == 0.25f && rgb[1] == 0.5f && rgb[2] == 0.75f);
    float untouched[3] = { -1, -1, -1 };
    uintptr_t Lbad = mkLabel(); at<uint32_t>(Lbad, Label_Text) = 0xBAD0BAD0;
    HCHECK("getTextColor returns false when the text vtable is wrong (textRgbSet stays false, no gray)", !getTextColor(Lbad, untouched));
    HCHECK("getTextColor failure leaves rgb untouched", untouched[0] == -1 && untouched[1] == -1 && untouched[2] == -1);
    uintptr_t Lnr = mkLabel(); at<uint32_t>(Lnr, Label_Text + Text_Renderer) = 0;
    HCHECK("getTextColor returns false when the text renderer is null", !getTextColor(Lnr, untouched));
    HCHECK("getTextColor false on null ctl / null out", !getTextColor(0, untouched) && !getTextColor(L, nullptr));
    HCHECK("getTextColor false on an unreadable ctl", !getTextColor(0x20, untouched));

    nTextColor = 0;
    HCHECK("setTextColor succeeds on a valid text object", setTextColor(L, 0.4f, 0.5f, 0.6f));
    HEQ("setTextColor calls the engine once", nTextColor, 1);
    HEQ("setTextColor passes label+0xF0 as the text object", (unsigned long long)reinterpret_cast<uintptr_t>(g_tcThis), (unsigned long long)(L + 0xF0));
    HCHECK("setTextColor passes r,g,b in order", g_tcRgb[0] == 0.4f && g_tcRgb[1] == 0.5f && g_tcRgb[2] == 0.6f);
    nTextColor = 0;
    HCHECK("setTextColor returns false when the text vtable is bad", !setTextColor(Lbad, 1, 1, 1));
    HEQ("setTextColor makes no engine call when the text vtable is bad", nTextColor, 0);

    // ---- setText ----
    nTextSet = 0; nDtor = 0;
    HCHECK("setText succeeds on a valid text object", setText(L, "Medpac x3"));
    HEQ("setText calls the engine text setter once", nTextSet, 1);
    HEQ("setText passes label+0xF0 as the text object", (unsigned long long)reinterpret_cast<uintptr_t>(g_tsThis), (unsigned long long)(L + 0xF0));
    HEQ("setText delivers the string through a CExoString", (const char*)g_tsStr, "Medpac x3");
    HEQ("setText destroys the temporary CExoString", nDtor, 1);
    nTextSet = 0;
    HCHECK("setText returns false when the text vtable is bad", !setText(Lbad, "x"));
    HCHECK("setText returns false when the text renderer is null", !setText(Lnr, "x"));
    HCHECK("setText returns false on null ctl / null text", !setText(0, "x") && !setText(L, nullptr));
    HEQ("setText makes no engine call when the text object is bad", nTextSet, 0);
    HCHECK("setFont / setAlign refuse a bad text object", !setFont(Lbad, "dialogfont16x16") && !setAlign(Lbad, 17));
    HEQ("setFont / setAlign make no engine call on a bad text object", nFont + nAlign, 0);
    HCHECK("setFont passes the resref through a 16-byte buffer on label+0xF0", setFont(L, "dialogfont16x16") && g_fontThis == reinterpret_cast<void*>(L + 0xF0) && !strcmp(g_fontStr, "dialogfont16x16"));
    HCHECK("setAlign passes the align value on label+0xF0", setAlign(L, 17) && g_alThis == reinterpret_cast<void*>(L + 0xF0) && g_alVal == 17);

    // ---- setFill / setAlpha: gated on the border vtable at label+0x60 ----
    void* img = reinterpret_cast<void*>(0x7000);
    HCHECK("setFill succeeds on a label with the border vtable", setFill(L, img));
    HEQ("setFill writes the image pointer at border+0x74 (label+0xD4)", (unsigned long long)at<uint32_t>(L, Label_Border + Border_Fill), 0x7000ull);
    HCHECK("setFill(nullptr) clears the fill", setFill(L, nullptr) && at<uint32_t>(L, Label_Border + Border_Fill) == 0);
    HCHECK("setAlpha succeeds on a label with the border vtable", setAlpha(L, 0.35f));
    HCHECK("setAlpha writes the float at border+0x24 (label+0x84)", at<float>(L, Label_Border + Border_Alpha) == 0.35f);
    uintptr_t Lnb = mkLabel(); at<uint32_t>(Lnb, Label_Border) = 0xBAD0BAD0;
    at<uint32_t>(Lnb, Label_Border + Border_Fill) = 0x1111; at<float>(Lnb, Label_Border + Border_Alpha) = 0.9f;
    HCHECK("setFill returns false when the border vtable is bad", !setFill(Lnb, img));
    HEQ("setFill writes nothing when the border vtable is bad", (unsigned long long)at<uint32_t>(Lnb, Label_Border + Border_Fill), 0x1111ull);
    HCHECK("setAlpha returns false when the border vtable is bad", !setAlpha(Lnb, 0.1f));
    HCHECK("setAlpha writes nothing when the border vtable is bad", at<float>(Lnb, Label_Border + Border_Alpha) == 0.9f);
    HCHECK("setFill/setAlpha false on null or unreadable ctl", !setFill(0, img) && !setAlpha(0, 1.f) && !setFill(0x20, img) && !setAlpha(0x20, 1.f));
    float tint[3] = { 0.1f, 0.2f, 0.3f };
    HCHECK("setFill with a tint writes it at border+0x28", setFill(L, img, tint) && at<float>(L, Label_Border + Border_Tint + 8) == 0.3f);

    // ---- geometry: setRect / getExtent round trip ----
    nSetRect = 0;
    HCHECK("setRect succeeds when the vtable and slot 1 are readable", setRect(L, 10, 20, 111, 112));
    HEQ("setRect calls the engine exactly once", nSetRect, 1);
    HEQ("setRect hands the vtable slot-1 function to the engine call", (unsigned long long)g_srFn, 0x401000ull);
    HCHECK("setRect passes x,y,w,h in order", g_srRect[0] == 10 && g_srRect[1] == 20 && g_srRect[2] == 111 && g_srRect[3] == 112);
    int x = -1, y = -1, w = -1, h = -1;
    HCHECK("getExtent reads the extent setRect stored (ctl+4/+8/+0xC/+0x10)", getExtent(L, &x, &y, &w, &h) && x == 10 && y == 20 && w == 111 && h == 112);
    int ox = -5; HCHECK("getExtent accepts null out-params", getExtent(L, &ox, nullptr, nullptr, nullptr) && ox == 10);
    x = y = w = h = -9;
    HCHECK("getExtent returns false on an unreadable ctl", !getExtent(0x20, &x, &y, &w, &h));
    HCHECK("getExtent leaves the callers' defaults untouched on failure", x == -9 && y == -9 && w == -9 && h == -9);
    HCHECK("getExtent returns false on null ctl", !getExtent(0, &x, &y, &w, &h));
    uintptr_t Lnv = mkLabel(); at<uint32_t>(Lnv, 0) = 0; nSetRect = 0;
    HCHECK("setRect returns false when the control vtable is null", !setRect(Lnv, 1, 2, 3, 4));
    at<uint32_t>(Lnv, 0) = static_cast<uint32_t>(g_labelVt); at<uint32_t>(g_labelVt, 4) = 0;
    HCHECK("setRect returns false when vtable slot 1 is null", !setRect(Lnv, 1, 2, 3, 4));
    at<uint32_t>(g_labelVt, 4) = 0x401000;
    HCHECK("setRect returns false on unreadable ctl", !setRect(0x20, 1, 2, 3, 4));
    HEQ("setRect makes no engine call on those failures", nSetRect, 0);

    // ---- visibility: shown clears click-through, hidden sets it ----
    at<int>(L, Ctl_Flags) = FlagVisible | FlagClickThrough;
    HCHECK("setVisible(true) succeeds", setVisible(L, true));
    HEQ("setVisible(true) sets bit1 and clears click-through bit5", at<int>(L, Ctl_Flags) & (FlagVisible | FlagClickThrough), FlagVisible);
    HCHECK("setVisible(false) succeeds", setVisible(L, false));
    HEQ("setVisible(false) clears bit1 and sets click-through bit5", at<int>(L, Ctl_Flags) & (FlagVisible | FlagClickThrough), FlagClickThrough);
    HCHECK("setVisible false on unreadable ctl", !setVisible(0x20, true));

    // ---- playHoverSound: GUI sound manager, sound index 1 ----
    uintptr_t mgrCell = arenaAlloc(16); at<uint32_t>(mgrCell, 0) = 0x5555;
    e.guiSoundMgrAddr = mgrCell; nSound = 0;
    HCHECK("playHoverSound true when the manager exists", playHoverSound());
    HEQ("playHoverSound plays sound index 1 (the native mouse-enter sound)", g_sndIdx, 1);
    HEQ("playHoverSound plays through the manager pointer", (unsigned long long)reinterpret_cast<uintptr_t>(g_sndMgr), 0x5555ull);
    HEQ("playHoverSound calls the engine once", nSound, 1);
    at<uint32_t>(mgrCell, 0) = 0; nSound = 0;
    HCHECK("playHoverSound returns false when the manager is null (hotbar logs 'manager null')", !playHoverSound());
    HEQ("playHoverSound makes no engine call when the manager is null", nSound, 0);
    e.guiSoundMgrAddr = 0x20;
    HCHECK("playHoverSound returns false when the manager global is unreadable", !playHoverSound());
    e.guiSoundMgrAddr = gameaddr::GuiSoundMgr;

    // ---- createLabelFromTemplate + appendToPanel: the HUD build appends 86 controls and relies on ids/counts ----
    const int Native = 41, Controls = 86;   // native control count (arbitrary) + ControlCount
    uintptr_t panel = arenaAlloc(0x100), arr = arenaAlloc((Native + Controls + 8) * 4);
    at<uint32_t>(panel, Panel_CtlArray) = (uint32_t)arr; at<int>(panel, Panel_CtlCount) = Native;
    bool idsOk = true, countsOk = true, slotsOk = true, hiddenOk = true, whyOk = true, listOk = true, noDouble = true;
    for (int i = 0; i < Controls; ++i) {
        int why = -1; uint32_t bad = 5;
        nListAdd = 0; g_initAdd = g_initScale = -1;
        uintptr_t c = createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &bad);
        whyOk &= c != 0 && why == CF_None && bad == 0;
        if (!c) break;
        noDouble &= g_initAdd == 0 && g_initScale == 1 && nListAdd == 0 && at<int>(panel, Panel_CtlCount) == Native + i;   // creation never adds to the panel list
        hiddenOk &= (at<int>(c, Ctl_Flags) & FlagVisible) == 0 && (at<int>(c, Ctl_Flags) & FlagClickThrough) != 0;
        listOk &= appendToPanel(panel, c);
        idsOk &= at<int>(c, Ctl_Id) == Native + i;
        countsOk &= at<int>(panel, Panel_CtlCount) == Native + i + 1;
        slotsOk &= at<uint32_t>(arr, (Native + i) * 4) == (uint32_t)c;
    }
    HCHECK("createLabelFromTemplate succeeds 86 times with why == CF_None", whyOk);
    HCHECK("created labels are not on the panel list yet (InitControl addToList=0, scale=1), so appendToPanel owns the count", noDouble);
    HCHECK("created labels are hidden and click-through until the hotbar shows them", hiddenOk);
    HCHECK("appendToPanel returns true for each of 86 appends", listOk);
    HCHECK("appendToPanel stamps id == old count == list index (ids Native..Native+85)", idsOk);
    HCHECK("appendToPanel raises the panel count by exactly 1 per append (86 total)", countsOk && at<int>(panel, Panel_CtlCount) == Native + Controls);
    HCHECK("appendToPanel stores the control at array[index] (draw order = append order)", slotsOk);
    HEQ("createLabelFromTemplate passes the template tag through the CExoString", (const char*)g_initTag, "LBL_QUEUE1");

    int why = -1; uintptr_t pc = at<int>(panel, Panel_CtlCount);
    uintptr_t cl = createLabelFromTemplate(panel, "LBL_X", &why);
    g_listMode = 1;
    HCHECK("appendToPanel returns false when the engine add did not bump the count", !appendToPanel(panel, cl));
    at<int>(panel, Panel_CtlCount) = (int)pc; g_listMode = 2;
    HCHECK("appendToPanel returns false when array[index] is not the control", !appendToPanel(panel, cl));
    g_listMode = 0; at<int>(panel, Panel_CtlCount) = (int)pc;
    HCHECK("appendToPanel false on null panel / ctl / unreadable panel", !appendToPanel(0, cl) && !appendToPanel(panel, 0) && !appendToPanel(0x10, cl));

    g_allocFails = true; why = -1;
    HCHECK("createLabelFromTemplate returns 0 on allocation failure", createLabelFromTemplate(panel, "LBL_X", &why) == 0);
    HEQ("allocation failure reports CF_Alloc (hotbar logs 'allocation failed')", why, (int)CF_Alloc);
    g_allocFails = false;
    g_ctorVt = 0xABCD0000u; why = -1; uint32_t bad = 0;
    HCHECK("createLabelFromTemplate returns 0 when the ctor leaves a wrong vtable", createLabelFromTemplate(panel, "LBL_X", &why, &bad) == 0);
    HEQ("wrong vtable reports CF_Vtable", why, (int)CF_Vtable);
    HEQ("wrong vtable reports the vtable it saw (hotbar logs it)", (unsigned long long)bad, 0xABCD0000ull);
    g_ctorVt = LabelVtable;
    g_initWritesId = false; why = -1;
    HCHECK("createLabelFromTemplate returns 0 when the template tag is not in the layout", createLabelFromTemplate(panel, "LBL_X", &why) == 0);
    HEQ("missing template reports CF_Template (hotbar logs 'template ... not in the HUD layout')", why, (int)CF_Template);
    g_initWritesId = true;

    // ---- imageFor: pool-full signalling (the pool is function-local static; this block owns it in this binary) ----
    bool loaded = false; int pool = -1;
    nLoad = 0;
    void* a = imageFor("FPH_Slot", &loaded, &pool);
    HCHECK("imageFor loads a new resref and reports loaded + pool 1", a == g_loadResult && loaded && pool == 1 && nLoad == 1);
    void* b = imageFor("fph_SLOT", &loaded, &pool);
    HCHECK("imageFor is case-insensitive and does not reload (hotbar logs only fresh loads)", b == a && !loaded && pool == 1 && nLoad == 1);
    HCHECK("imageFor on empty / null resref returns null and not loaded", imageFor("", &loaded, &pool) == nullptr && !loaded && imageFor(nullptr, &loaded, &pool) == nullptr && !loaded);
    char nm[24]; bool fillOk = true;
    for (int i = 1; i < ImagePoolSize; ++i) { snprintf(nm, sizeof nm, "fill%d", i); fillOk &= imageFor(nm, &loaded, &pool) != nullptr && loaded && pool == i + 1; }
    HCHECK("imageFor fills all 256 pool entries, each reporting loaded and the growing pool count", fillOk);
    int before = nLoad;
    void* full = imageFor("one_too_many", &loaded, &pool);
    HCHECK("pool full is signalled as: null result, loaded == false, poolCount >= ImagePoolSize (the hotbar's exact test)", full == nullptr && !loaded && pool >= ImagePoolSize);
    HEQ("a full pool never calls the engine loader", nLoad, before);
    HCHECK("a pooled resref still resolves after the pool is full", imageFor("FPH_SLOT", &loaded, &pool) == a && !loaded);
}

// ============================================================ 4. Party
struct FakeC { int hp, maxhp; };
static FakeC g_fc[4]; static uintptr_t g_clients[4], g_servers[4];
static uintptr_t g_slotMax, g_slotCur;
static int g_vcalls = 0, g_vArg[16]; static uintptr_t g_vFn[16], g_vC[16];
static int g_mIdx[16], g_mn = 0; static void* g_mTable = nullptr; static const uintptr_t Creature = 0xC0FFEE; static int g_gcCalls = 0; static void* g_gcTable = nullptr;
static uintptr_t __thiscall fakeGetControlled(void* t) { ++g_gcCalls; g_gcTable = t; return Creature; }
static uintptr_t __thiscall fakeMember(void* t, int i) { g_mTable = t; if (g_mn < 16) g_mIdx[g_mn] = i; ++g_mn; return g_clients[i]; }
static uintptr_t __thiscall fakeServer(void* m) { for (int i = 0; i < 4; ++i) if (g_clients[i] == (uintptr_t)m) return g_servers[i]; return 0; }
static int fakeVital(uintptr_t fn, uintptr_t c, int arg) {
    if (g_vcalls < 16) { g_vFn[g_vcalls] = fn; g_vC[g_vcalls] = c; g_vArg[g_vcalls] = arg; }
    ++g_vcalls;
    for (int i = 0; i < 4; ++i) if (g_servers[i] == c) return arg == 1 ? g_fc[i].maxhp : g_fc[i].hp;
    return -999;
}
static uintptr_t cell, app, pa, pb, tbl;
static void build(int count) {
    at<uint32_t>(cell, 0) = (uint32_t)app; at<uint32_t>(app, 4) = (uint32_t)pa; at<uint32_t>(pa, 4) = (uint32_t)pb;
    at<uint32_t>(pb, 0x270) = (uint32_t)tbl; at<int>(tbl, 0) = count;
    g_gcCalls = 0; g_mn = 0; g_vcalls = 0;
}
static void mkCreature(int i, int hp, int maxhp) {
    uintptr_t vt = arenaAlloc(0x100), c = arenaAlloc(16);
    at<uint32_t>(vt, party::SlotMaxHp) = (uint32_t)g_slotMax; at<uint32_t>(vt, party::SlotCurHp) = (uint32_t)g_slotCur; at<uint32_t>(c, 0) = (uint32_t)vt;
    g_servers[i] = c; g_clients[i] = 0x1000 + 0x10 * (i + 1); g_fc[i] = { hp, maxhp };
}

static void party_contract() {
    cell = arenaAlloc(16); app = arenaAlloc(16); pa = arenaAlloc(16); pb = arenaAlloc(0x300); tbl = arenaAlloc(16);
    party::g_appGlobal = cell; party::g_getControlled = fakeGetControlled;
    party::g_member = fakeMember; party::g_serverCreature = fakeServer; party::g_callVital = fakeVital;
    g_slotMax = 0x401000; g_slotCur = 0x402000;

    // ---- chain: app -> +4 -> +4 -> +0x270 -> count; failure -> controlledCreature 0 + a non-null reason ----
    build(3);
    { party::Fail w = party::F_App; int n = 77;
      HEQ("controlledCreature returns the engine's controlled creature", (unsigned long long)party::controlledCreature(&w, &n), (unsigned long long)Creature);
      HEQ("controlledCreature reports F_None on success", (int)w, (int)party::F_None);
      HEQ("controlledCreature writes the party count (hotbar's g_partyCount)", n, 3);
      HEQ("the engine call receives the party table pointer", (unsigned long long)reinterpret_cast<uintptr_t>(g_gcTable), (unsigned long long)tbl); }
    struct Break { const char* what; int level; party::Fail want; };
    const Break breaks[] = { { "app global cell -> 0", 0, party::F_App }, { "app+4 -> 0", 1, party::F_App4 }, { "a+4 -> 0", 2, party::F_A4 }, { "b+0x270 -> 0", 3, party::F_Table } };
    for (const Break& bk : breaks) {
        build(3);
        if (bk.level == 0) at<uint32_t>(cell, 0) = 0; else if (bk.level == 1) at<uint32_t>(app, 4) = 0; else if (bk.level == 2) at<uint32_t>(pa, 4) = 0; else at<uint32_t>(pb, 0x270) = 0;
        party::Fail w = party::F_None; int n = 77;
        HEQ("a broken chain link makes controlledCreature return 0", (unsigned long long)party::controlledCreature(&w, &n), 0ull);
        HEQ("a broken chain link names itself (the hotbar logs failText once per reason)", (int)w, (int)bk.want);
        HEQ("a broken chain link makes no engine call", g_gcCalls, 0);
        HCHECK("a broken chain link keeps the count untouched (table not found)", n == 77);
        party::Vitals out[3]; HEQ("a broken chain link makes members() return 0 (target probe uses 'no members')", party::members(out, 3), 0);
    }
    build(0);
    { party::Fail w = party::F_None; int n = 77;
      HEQ("count 0 makes controlledCreature return 0", (unsigned long long)party::controlledCreature(&w, &n), 0ull);
      HEQ("count 0 reports F_Count", (int)w, (int)party::F_Count); HEQ("count 0 is still reported to the caller", n, 0); }
    // failText: non-null for every Fail value, distinct text per reason (the hotbar dedupes its log by string pointer)
    bool nonNull = true, distinct = true;
    for (int f = party::F_None; f <= party::F_Count; ++f) {
        const char* t = party::failText((party::Fail)f); nonNull &= t != nullptr && *t;
        for (int g = f + 1; g <= party::F_Count; ++g) distinct &= strcmp(t, party::failText((party::Fail)g)) != 0;
    }
    HCHECK("failText gives a non-empty string for every Fail value", nonNull);
    HCHECK("failText gives a different string for every Fail value", distinct);
    HCHECK("failText returns a stable pointer per value (pointer-compared by the hotbar's log dedupe)", party::failText(party::F_Table) == party::failText(party::F_Table));

    // ---- members(): index == party position, 0 = controlled ----
    build(4); mkCreature(0, 50, 100); mkCreature(1, 0, 80); mkCreature(2, -3, 7); mkCreature(3, 0, 0);
    party::Vitals v[4];
    HEQ("members returns the party count (capped by the buffer)", party::members(v, 4), 4);
    bool idxOk = g_mn == 4; for (int i = 0; i < 4 && i < 16; ++i) idxOk &= g_mIdx[i] == i;
    HCHECK("members asks the engine for positions 0,1,2,3 in order (index 0 = controlled)", idxOk);
    HEQ("members asks PartyGetAt on the party table", (unsigned long long)reinterpret_cast<uintptr_t>(g_mTable), (unsigned long long)tbl);
    HCHECK("members[i].client is the client creature at position i", v[0].client == g_clients[0] && v[1].client == g_clients[1] && v[2].client == g_clients[2] && v[3].client == g_clients[3]);
    HCHECK("members[i].server is that member's server creature", v[0].server == g_servers[0] && v[2].server == g_servers[2]);
    HEQ("members cap 3 returns 3 (hotbar passes Vitals[3])", party::members(v, 3), 3);
    HEQ("members with cap 0 / null buffer returns 0", party::members(v, 0) + party::members(nullptr, 3), 0);

    // ---- vitals: max slot +0x98 with arg 1, cur slot +0x9C with arg 0, EXACTLY two calls ----
    party::Vitals o; g_vcalls = 0;
    HCHECK("vitals succeeds on a readable server creature", party::vitals(g_servers[0], &o));
    HEQ("vitals makes exactly two engine calls (one per slot, one arg each)", g_vcalls, 2);
    HEQ("vitals calls the max-HP slot (+0x98) first", (unsigned long long)g_vFn[0], (unsigned long long)g_slotMax);
    HEQ("vitals calls max-HP with arg 1", g_vArg[0], 1);
    HEQ("vitals calls the cur-HP slot (+0x9C) second", (unsigned long long)g_vFn[1], (unsigned long long)g_slotCur);
    HEQ("vitals calls cur-HP with arg 0", g_vArg[1], 0);
    HCHECK("vitals calls both slots on the server creature", g_vC[0] == g_servers[0] && g_vC[1] == g_servers[0]);
    HCHECK("vitals fills cur/max/ok/server", o.cur == 50 && o.max == 100 && o.ok && !o.down && o.server == g_servers[0]);
    // down iff ok && cur <= 0
    party::vitals(g_servers[1], &o); HCHECK("cur == 0 with max > 0 is down", o.ok && o.down && o.cur == 0);
    party::vitals(g_servers[2], &o); HCHECK("cur < 0 with max > 0 is down", o.ok && o.down && o.cur == -3);
    party::vitals(g_servers[0], &o); HCHECK("cur > 0 is not down", o.ok && !o.down);
    HCHECK("max <= 0 is not ok and not down (no false 'dead' icon)", !party::vitals(g_servers[3], &o) && !o.ok && !o.down);
    // unreadable -> ok=false and no engine call
    uintptr_t vt0 = at<uint32_t>(g_servers[0], 0); g_vcalls = 0;
    at<uint32_t>(g_servers[0], 0) = 0x20;
    HCHECK("an unreadable vtable gives ok=false", !party::vitals(g_servers[0], &o) && !o.ok && !o.down);
    at<uint32_t>(g_servers[0], 0) = (uint32_t)vt0; at<uint32_t>(vt0, party::SlotMaxHp) = 0;
    HCHECK("a null max-HP slot gives ok=false", !party::vitals(g_servers[0], &o) && !o.ok);
    at<uint32_t>(vt0, party::SlotMaxHp) = (uint32_t)g_slotMax; at<uint32_t>(vt0, party::SlotCurHp) = 0;
    HCHECK("a null cur-HP slot gives ok=false", !party::vitals(g_servers[0], &o) && !o.ok);
    at<uint32_t>(vt0, party::SlotCurHp) = (uint32_t)g_slotCur;
    HCHECK("a null server gives ok=false", !party::vitals(0, &o) && !o.ok);
    HEQ("unreadable members make NO engine call (a bad call crashes the game)", g_vcalls, 0);
    // an unreadable middle member inside members(): alignment kept, others read, no call for it
    at<uint32_t>(g_servers[1], 0) = 0; g_vcalls = 0;
    HEQ("members still returns the full count with an unreadable member", party::members(v, 4), 4);
    HCHECK("the unreadable member is ok=false/down=false and its neighbours are read", v[0].ok && !v[1].ok && !v[1].down && v[2].ok && v[2].down && v[1].server == g_servers[1]);
    HEQ("only readable members trigger engine calls (idx 0, 2, 3 x 2 calls; the unreadable idx 1 none)", g_vcalls, 6);
    HCHECK("members keeps client at the unreadable member's slot", v[1].client == g_clients[1]);

    // ---- a one-member party (the Exile alone; the hotbar's characterKeyFor special-cases partyCount == 1) ----
    build(1); mkCreature(0, 50, 100);
    { party::Fail w = party::F_App; int n = 77;
      HEQ("hotbar relies on: a one-member party still has a controlled creature", (unsigned long long)party::controlledCreature(&w, &n), (unsigned long long)Creature);
      HEQ("hotbar relies on: a one-member party reports F_None", (int)w, (int)party::F_None);
      HEQ("hotbar relies on: a one-member party reports count 1", n, 1); }
    { party::Vitals one[3];
      HEQ("hotbar relies on: members() of a one-member party returns 1", party::members(one, 3), 1);
      HCHECK("hotbar relies on: members() fills entry 0 of a one-member party", one[0].client == g_clients[0] && one[0].ok && one[0].cur == 50 && one[0].max == 100); }
    // ---- a corrupt (negative) count is "no members", never a negative loop bound ----
    build(-1);
    { party::Vitals neg[3]; HEQ("hotbar relies on: a negative party count makes members() return 0 (the hotbar loops i < n)", party::members(neg, 3), 0);
      party::Fail w = party::F_None; int n = 77;
      HEQ("a negative party count makes controlledCreature return 0", (unsigned long long)party::controlledCreature(&w, &n), 0ull);
      HEQ("a negative party count reports F_Count", (int)w, (int)party::F_Count); }
    // ---- Vitals default: unread (the hotbar's Vitals[3] entries past the party size stay default) ----
    { party::Vitals z;
      HCHECK("hotbar relies on: a default Vitals is all zero / not ok / not down", z.client == 0 && z.server == 0 && z.cur == 0 && z.max == 0 && !z.ok && !z.down); }
}

// ============================================================ 2b. saferead::probeWrite (write guard before engine-memory stores)
static void saferead_write_contract() {
    uintptr_t w = arenaAlloc(64);
    auto W = [&](uintptr_t p, size_t n) { return saferead::probeWrite(4u, reinterpret_cast<void*>(p), n); };
    saferead::beginScope();
    HCHECK("probeWrite true on a writable arena range", W(w, 64));
    HCHECK("probeWrite true on a 4-byte field inside it (the typical store)", W(w + 8, 4));
    HCHECK("probeWrite len 0 is true (probeRead shape)", W(w, 0));
    HCHECK("probeWrite false on null", !W(0, 4));
    HCHECK("probeWrite false below 0x10000", !W(0x20, 4));
    HCHECK("probeWrite false at the 0xFFFF boundary", !W(0xFFFF, 4));
    HCHECK("probeWrite false when p+len wraps the address space", !W(~uintptr_t(0) - 3, 16));
    HCHECK("probeWrite false when len is huge", !W(w, ~size_t(0)));
    g_stubVirtualQueryAll = false; saferead::beginScope();
    HCHECK("probeWrite false when VirtualQuery fails (unreadable)", !W(w, 4));
    g_stubVirtualQueryAll = true; saferead::beginScope();
    HCHECK("probeWrite true again after the stub is restored", W(w, 4));
    // read-only / guard / reserved pages are not fakeable here (the stub reports one PAGE_READWRITE region); the protection verdicts
    // are pinned in _shared_src/saferead_test.cpp, which routes VirtualQuery through a fake.
}

// ============================================================ 5. FileWatch (config reload + atomic save)
// The hotbar polls force_power_hotbar.ini once a second and saves it from the editor; see checkConfig / assignSlot / replaceLiveProfiles.
static std::string fwRead(const std::string& p) {
    std::string o; FILE* f = fopen(p.c_str(), "rb"); if (!f) return "<missing>";
    char b[256]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) o.append(b, n);
    fclose(f); return o;
}
static void fwPut(const std::string& p, const std::string& s) { FILE* f = fopen(p.c_str(), "wb"); if (f) { fwrite(s.data(), 1, s.size(), f); fclose(f); } }
static bool fwExists(const std::string& p) { struct stat st; return lstat(p.c_str(), &st) == 0; }

static void filewatch_contract() {
    const char* t = std::getenv("TMPDIR");
    std::string tmpl = std::string(t && *t ? t : "/tmp") + "/hotbar_fw.XXXXXX";
    char* d = mkdtemp(&tmpl[0]);
    if (!d) { HCHECK("mkdtemp works", false); return; }
    std::string dir = d, ini = dir + "/force_power_hotbar.ini";

    // A hand edit within the same second as the previous write must be seen (the old 1 s stat mtime missed it).
    fwPut(ini, "[General]\nInvoke=1\n");
    filewatch::ReloadPoll poll; poll.throttleMs = 1000;
    HCHECK("first poll reports a change (initial load) and records the stamp", filewatch::changed(poll, ini.c_str(), 0));
    HCHECK("an untouched ini is not reloaded", !filewatch::changed(poll, ini.c_str(), 1000));
    timespec ts[2] = { {1700000000, 0}, {1700000000, 0} };
    HCHECK("utimensat works", utimensat(AT_FDCWD, ini.c_str(), ts, 0) == 0);   // pin the mtime so both writes share a second on any filesystem
    filewatch::changed(poll, ini.c_str(), 2000);
    fwPut(ini, "[General]\nInvoke=0\nVerbose=1\n"); HCHECK("utimensat works again", utimensat(AT_FDCWD, ini.c_str(), ts, 0) == 0);
    HCHECK("a same-second rewrite with a different size is detected", filewatch::changed(poll, ini.c_str(), 3000));
    HCHECK("the hotbar's 1000 ms throttle: no stamp inside the window", !filewatch::changed(poll, ini.c_str(), 3500));
    fwPut(ini, "[General]\nInvoke=0\nVerbose=1\nAutoFill=1\n");
    HCHECK("force (checkConfig(true) before a save) bypasses the throttle", filewatch::changed(poll, ini.c_str(), 3600, true));
    filewatch::accept(poll, ini.c_str());
    HCHECK("accept() after our own Save: the write is not reloaded as a change", !filewatch::changed(poll, ini.c_str(), 9000));
    remove(ini.c_str());
    HCHECK("a deleted ini is a change (template rewrite path)", filewatch::changed(poll, ini.c_str(), 11000));

    // Save must never lose the user's file: failures keep the old content and leave no .tmp.
    fwPut(ini, "old content");
    std::filesystem::create_directory(dir + "/asdir");
    HCHECK("atomicWrite returns false when the replace fails (target is a directory)", !filewatch::atomicWrite((dir + "/asdir").c_str(), "x", false));
    HCHECK("atomicWrite failure leaves no .tmp behind", !fwExists(dir + "/asdir.tmp"));
    HCHECK("atomicWrite failure leaves the target untouched", std::filesystem::is_directory(dir + "/asdir"));
    HCHECK("atomicWrite to a missing directory fails", !filewatch::atomicWrite((dir + "/nodir/f.ini").c_str(), "x", false));
    if (geteuid() != 0) {
        std::filesystem::create_directory(dir + "/ro");
        fwPut(dir + "/ro/f.ini", "keep me");
        chmod((dir + "/ro").c_str(), 0555);
        bool ok = filewatch::atomicWrite((dir + "/ro/f.ini").c_str(), "new", false);
        chmod((dir + "/ro").c_str(), 0755);
        HCHECK("atomicWrite fails in an unwritable directory", !ok);
        HCHECK("atomicWrite keeps the old file on failure", fwRead(dir + "/ro/f.ini") == "keep me");
        HCHECK("atomicWrite leaves no .tmp after an unwritable-dir failure", !fwExists(dir + "/ro/f.ini.tmp"));
    }
    HCHECK("atomicWrite replaces the content", filewatch::atomicWrite(ini.c_str(), "new content", false) && fwRead(ini) == "new content");
    HCHECK("atomicWrite leaves no .tmp on success", !fwExists(ini + ".tmp"));
    HCHECK("makeBakOnce=false never writes a .bak (hotbar Save makes its own backups)", !fwExists(ini + ".bak"));

    // .bak is written once: the first managed save keeps the pre-edit file.
    std::string f2 = dir + "/b.ini";
    fwPut(f2, "original");
    HCHECK("atomicWrite(makeBakOnce) writes", filewatch::atomicWrite(f2.c_str(), "v2", true));
    HCHECK("the .bak holds the original", fwRead(f2 + ".bak") == "original");
    HCHECK("a second atomicWrite writes", filewatch::atomicWrite(f2.c_str(), "v3", true) && fwRead(f2) == "v3");
    HCHECK("the .bak is NOT overwritten by later saves", fwRead(f2 + ".bak") == "original");

    std::error_code ec; std::filesystem::remove_all(dir, ec);
}


// ============================================================ 6. KpLog.h (logging; map 07b sections 7-8)
// The hotbar logs through KpLog.h only (channels build cfg input edit slots target tip char gate main). Pinned here: init fails soft,
// the channel names are accepted by the core's name rule, WARN needs no ini, KP_LOG_EVERY gates, the header's own 511-byte cut,
// the fallback file line format and its 60000-line cap. KpLog state is process-global: failed inits, then fallback, then the real init.
KP_LOG_PATCH("force-power-hotbar");   // == manifest id (one per test TU)

namespace klt {
struct Cap { std::string chan; int lvl; std::string text; };
std::vector<Cap> cap; std::vector<std::string> chans; int threshold = KPLOG_WARN; int regRet = 3;
const char* reqName = nullptr; unsigned reqVer = 0; int reqCalls = 0; const void* reqIface = nullptr;
int32_t KPATCH_CALL fReg(const char*) { return regRet; }
int32_t KPATCH_CALL fChan(int32_t, const char* n) {   // the core's rule: 1..63 chars of [A-Za-z0-9_.-]
    size_t len = n ? strlen(n) : 0; if (len < 1 || len > 63) return -1;
    for (size_t i = 0; i < len; ++i) { char c = n[i]; if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-')) return -1; }
    for (size_t i = 0; i < chans.size(); ++i) if (chans[i] == n) return (int32_t)i;   // idempotent per name
    chans.push_back(n); return (int32_t)chans.size() - 1;
}
int32_t KPATCH_CALL fEnabled(int32_t, int32_t lvl) { return lvl <= threshold; }   // like the core: no ini -> WARN and ERR only
void KPATCH_CALL fSubmit(int32_t c, int32_t l, const char* t, uint32_t n) { cap.push_back({chans[(size_t)c], l, std::string(t, n)}); }
void KPATCH_CALL fMark(const char*) {}
void KPATCH_CALL fTick(int32_t) {}
uint32_t KPATCH_CALL fFrame() { return 0; }
uint64_t KPATCH_CALL fNow() { return 0; }
KPatchLogApi mk() {
    KPatchLogApi l; memset(&l, 0, sizeof l); l.struct_size = sizeof l;
    l.Register = fReg; l.Channel = fChan; l.Enabled = fEnabled; l.Submit = fSubmit; l.Mark = fMark; l.FrameTick = fTick; l.Frame = fFrame; l.NowUs = fNow;
    return l;
}
KPatchLogApi g_api;
const void* KPATCH_CALL fRequire(const char* n, uint32_t v) { ++reqCalls; reqName = n; reqVer = v; return reqIface; }
KPatchApi mkReg() { KPatchApi a; memset(&a, 0, sizeof a); a.struct_size = sizeof a; a.abi_version = KPATCH_ABI_VERSION; a.require = fRequire; return a; }

std::vector<std::string> fileLines(const std::string& path) {
    std::vector<std::string> v; FILE* f = fopen(path.c_str(), "rb"); if (!f) return v;
    std::string cur; int c; while ((c = fgetc(f)) != EOF) { if (c == '\n') { v.push_back(cur); cur.clear(); } else cur += (char)c; }
    if (!cur.empty()) v.push_back(cur);
    fclose(f); return v;
}
// "<ms> force-power-hotbar/<chan> <rest>" -> rest (empty + ok=false when the shape is wrong); the ms stamp is all digits.
bool parseFb(const std::string& line, const std::string& chan, std::string* rest) {
    size_t sp = line.find(' '); if (sp == std::string::npos || sp == 0) return false;
    for (size_t i = 0; i < sp; ++i) if (line[i] < '0' || line[i] > '9') return false;
    std::string pre = " force-power-hotbar/" + chan + " ";
    if (line.compare(sp, pre.size(), pre) != 0) return false;
    *rest = line.substr(sp + pre.size()); return true;
}
}  // namespace klt

static void kplog_contract() {
    using namespace klt;
    const char* hotbarChans[] = {"build", "cfg", "input", "edit", "slots", "target", "tip", "char", "gate", "main"};

    // ---- init fails soft: the patch stays on the fallback and nothing crashes
    HCHECK("KpLog_HasCore is false before KpLog_Init (DllMain/static-init logging is fallback only)", !KpLog_HasCore());
    HCHECK("KpLog_Init(nullptr) == 0", KpLog_Init(nullptr) == 0);
    KPatchApi reg = mkReg(); reg.require = nullptr;
    HCHECK("KpLog_Init with null require == 0", KpLog_Init(&reg) == 0);
    reg = mkReg(); reqIface = nullptr; reqCalls = 0;
    HCHECK("KpLog_Init with no kpatch.log service (old core) == 0", KpLog_Init(&reg) == 0);
    HCHECK("the service is asked as (\"kpatch.log\", 1)", reqCalls == 1 && reqName && strcmp(reqName, "kpatch.log") == 0 && reqVer == 1u);
    g_api = mk(); g_api.struct_size = sizeof(KPatchLogApi) - 1; reqIface = &g_api;
    HCHECK("KpLog_Init with a too-short struct == 0", KpLog_Init(&reg) == 0);
    g_api.struct_size = 0; HCHECK("KpLog_Init with struct_size 0 == 0", KpLog_Init(&reg) == 0);
    g_api = mk(); g_api.Submit = nullptr; HCHECK("KpLog_Init with a missing Submit == 0", KpLog_Init(&reg) == 0);
    g_api = mk(); g_api.Enabled = nullptr; HCHECK("KpLog_Init with a missing Enabled == 0", KpLog_Init(&reg) == 0);
    g_api = mk(); regRet = -1; HCHECK("KpLog_Init when Register rejects the id == 0", KpLog_Init(&reg) == 0); regRet = 3;
    HCHECK("no failed init leaves the header thinking a core exists", !KpLog_HasCore());

    // ---- fallback file mode (old core): documented line format, level gate, 511 cut, 60000-line cap
    const char* t = std::getenv("TMPDIR");
    std::string tmpl = std::string(t && *t ? t : "/tmp") + "/hotbar_kplog." + std::to_string((long)getpid()) + ".XXXXXX";
    char* d = mkdtemp(&tmpl[0]);
    if (!d) { HCHECK("mkdtemp works", false); return; }
    std::string dir = d, path = dir + "/force_power_hotbar_log." + std::to_string((long)getpid()) + ".txt";
    KpLog_SetFallbackFile(std::string(260, 'x').c_str());   // 260+ chars ignored
    KpLog_SetFallbackFile(path.c_str());
    KpLog_SetFallbackLevel(KPLOG_INFO);                      // what startOnce() sets on an old core
    {
        kplog::Channel cw{"cfg"};
        KP_WARN("warn %d", 7);
        KP_ERR("err %s", "x");
        KP_LOGV(cw, KPLOG_INFO, "info line");
        KP_LOGV(cw, KPLOG_DEBUG, "debug dropped");
        int sideEffects = 0; KP_LOGV(cw, KPLOG_TRACE, "trace dropped %d", ++sideEffects);
        HCHECK("a line above the fallback level is not even formatted", sideEffects == 0);
        KP_LOGV(cw, KPLOG_WARN, "two\nlines");
        for (const char* n : hotbarChans) { kplog::Channel c{n}; KP_LOGV(c, KPLOG_INFO, "hello-%s", n); }
        std::string longmsg(600, 'L');
        kplog::Channel ct{"tip"}; KP_LOGV(ct, KPLOG_INFO, "%s", longmsg.c_str());
        for (int i = 0; i < 3; ++i) KP_LOG_EVERY(cw, 60000, "every-%d", i);
        std::vector<std::string> L = fileLines(path); std::string r;
        HCHECK("the fallback file is created at the configured path (and the 260-char path was ignored)", !L.empty());
        size_t k = 0;
        HCHECK("WARN line shape: '<ms> force-power-hotbar/main WARN text'", k < L.size() && parseFb(L[k], "main", &r) && r == "WARN warn 7"); ++k;
        HCHECK("ERR line shape: 'ERR text'", k < L.size() && parseFb(L[k], "main", &r) && r == "ERR err x"); ++k;
        HCHECK("INFO line has no level tag (level is implied by the fallback level)", k < L.size() && parseFb(L[k], "cfg", &r) && r == "info line"); ++k;
        HCHECK("DEBUG and TRACE are dropped at fallback level INFO", k < L.size() && L[k].find("dropped") == std::string::npos);
        HCHECK("an embedded newline becomes two tagged lines", k + 1 < L.size() && parseFb(L[k], "cfg", &r) && r == "WARN two" && parseFb(L[k + 1], "cfg", &r) && r == "WARN lines"); k += 2;
        for (const char* n : hotbarChans) { std::string want = std::string("hello-") + n; HCHECK("every hotbar channel name writes in fallback mode", k < L.size() && parseFb(L[k], n, &r) && r == want); ++k; }
        HCHECK("a >511-byte message is cut at 511 bytes ending with [...]", k < L.size() && parseFb(L[k], "tip", &r) && r.size() == 511 && r.compare(506, 5, "[...]") == 0 && r.compare(0, 506, std::string(506, 'L')) == 0); ++k;
        HCHECK("KP_LOG_EVERY logs the first call only inside the window", k < L.size() && parseFb(L[k], "cfg", &r) && r == "every-0" && k + 1 == L.size());
    }
    {   // cap: line 60000 is "--- log cap reached ---", nothing after it
        const unsigned cap = KP_LOG_FALLBACK_MAX_LINES;
        HCHECK("the cap is 60000 lines (KpLog.h default)", cap == 60000u);
        for (unsigned i = 0; i < cap + 50; ++i) KP_WARN("fill");
        std::vector<std::string> L = fileLines(path);
        HCHECK("the file holds exactly 60000 lines plus the cap line", L.size() == (size_t)cap + 1);
        HCHECK("the last line is the cap marker", !L.empty() && L.back() == "--- log cap reached ---");
        size_t n = 0; for (const std::string& l : L) if (l == "--- log cap reached ---") ++n;
        HCHECK("the cap marker is written once", n == 1);
    }
    KpLog_SetFallbackFile(nullptr);
    { std::error_code ec; std::filesystem::remove_all(dir, ec); }

    // ---- with the core: real init, channel names, thresholds
    g_api = mk(); reqIface = &g_api;
    HCHECK("KpLog_Init with a valid api == 1", KpLog_Init(&reg) == 1);
    HCHECK("KpLog_HasCore is true after a successful init", KpLog_HasCore());
    cap.clear();
    for (const char* n : hotbarChans) { kplog::Channel c{n}; KP_LOGV(c, KPLOG_WARN, "w-%s", n); }
    HCHECK("all ten hotbar channels are accepted by the core's name rule and deliver", cap.size() == 10);
    for (size_t i = 0; i < cap.size() && i < 10; ++i) HCHECK("each line arrives on its own channel at WARN with its text", cap[i].chan == hotbarChans[i] && cap[i].lvl == KPLOG_WARN && cap[i].text == std::string("w-") + hotbarChans[i]);
    { kplog::Channel bad{"bad name"}; size_t before = cap.size(); KP_LOGV(bad, KPLOG_WARN, "x"); HCHECK("sanity: the fake core rejects a bad channel name (so acceptance above means something)", cap.size() == before); }

    cap.clear(); threshold = KPLOG_WARN;   // no kplog.ini: the core records WARN and ERR only
    KP_WARN("no ini %d", 1); KP_ERR("no ini err");
    HCHECK("KP_WARN and KP_ERR pass with no ini/threshold on channel main", cap.size() == 2 && cap[0].chan == "main" && cap[0].lvl == KPLOG_WARN && cap[0].text == "no ini 1" && cap[1].lvl == KPLOG_ERR);
    cap.clear();
    { kplog::Channel c{"slots"}; int se = 0; KP_LOG(c, "info %d", ++se); KP_LOGV(c, KPLOG_DEBUG, "dbg %d", ++se);
      HCHECK("INFO/DEBUG are dropped without a threshold and their arguments are not evaluated", cap.empty() && se == 0 && !KP_ENABLED(c, KPLOG_INFO) && KP_ENABLED(c, KPLOG_WARN)); }

    threshold = KPLOG_TRACE; cap.clear();
    {
        kplog::Channel c{"gate"};
        for (int i = 0; i < 3; ++i) KP_LOG_EVERY(c, 60000, "tick %d", i);
        HCHECK("KP_LOG_EVERY gates a second call inside the window (first logs, rest dropped)", cap.size() == 1 && cap[0].text == "tick 0" && cap[0].lvl == KPLOG_INFO);
        cap.clear();
        KP_LOGV(c, KPLOG_TRACE, "t"); KP_LOG(c, "i");
        HCHECK("explicit levels are passed through unchanged (TRACE, INFO)", cap.size() == 2 && cap[0].lvl == KPLOG_TRACE && cap[1].lvl == KPLOG_INFO);
        cap.clear();
        std::string longmsg(600, 'M'); KP_LOGV(c, KPLOG_INFO, "%s", longmsg.c_str());
        HCHECK("the header cuts a >511-byte message at 511 bytes ending with [...]", cap.size() == 1 && cap[0].text.size() == 511 && cap[0].text.compare(506, 5, "[...]") == 0 && cap[0].text.compare(0, 506, std::string(506, 'M')) == 0);
        cap.clear();
        std::string exact(511, 'E'); KP_LOGV(c, KPLOG_INFO, "%s", exact.c_str());
        HCHECK("a 511-byte message is not cut", cap.size() == 1 && cap[0].text == exact);
        cap.clear();
        KP_LOGV(c, KPLOG_INFO, "a\nb\n\nc");
        HCHECK("an embedded newline arrives as separate lines (formatters must not emit one), empties skipped", cap.size() == 3 && cap[0].text == "a" && cap[1].text == "b" && cap[2].text == "c");
    }
    threshold = KPLOG_WARN;
}

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { std::perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;   // every arena address reads as committed readable memory; < 0x10000 and 0 stay unreadable (probeRead rule)
    saferead_contract();
    saferead_write_contract();
    guikit_contract();
    party_contract();
    filewatch_contract();
    kplog_contract();
    return kt::Report("common_contract_test (force_power_hotbar)");
}
