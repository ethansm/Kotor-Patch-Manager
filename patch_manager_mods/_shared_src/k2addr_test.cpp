// Host test of K2Addr.h (per-version engine address table) and of the fail-safe behaviour of Effects.h / Gui2D.h / GuiKit.h
// when the selected table is null or a field is zero. The Steam literals below were extracted from the headers as they were
// BEFORE the table existed (git HEAD of GameAddr.h / Effects.h / Gui2D.h / GuiKit.h), so a typo in kSteam cannot hide.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#define GUI2D_HOST 1
#include "Effects.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "check.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

static const char* const kSteamSha = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF";
static const char* const kGogSha = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914";

static uint8_t* g_arena = nullptr;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// Every kSteam field against today's constant (literal list).
static void checkSteamTable() {
    const K2AddrTable& t = kSteam;
#define STEAM(field, value) KT_CHECK_EQ((unsigned long long)t.field, (unsigned long long)(value))
    STEAM(AppGlobal, 0x00A1B4A4);
    STEAM(GuiSoundMgr, 0x00A1B49C);
    STEAM(PlayGuiSound, 0x004122A0);
    STEAM(ScreenW, 0x009F42A4);
    STEAM(ScreenH, 0x009F42A8);
    STEAM(GuiRefW, 0x009F4390);
    STEAM(GuiRefH, 0x009F4394);
    STEAM(ProjectionFlag, 0x00A1B9F0);
    STEAM(New, 0x00919723);
    STEAM(LabelCtor, 0x00419740);
    STEAM(InitControl, 0x0040F620);
    STEAM(StopLoadFromLayout, 0x0040F5A0);
    STEAM(ExoCtor, 0x00733570);
    STEAM(ExoDtor, 0x00733780);
    STEAM(TextSet, 0x00416E30);
    STEAM(SetFont, 0x00416DE0);
    STEAM(SetAlign, 0x00416FA0);
    STEAM(TextColor, 0x00417140);
    STEAM(PtrListAdd, 0x0083EA60);
    STEAM(GuiLoadImage, 0x0047EB60);
    STEAM(GetControlledCreature, 0x007E5D80);
    STEAM(PartyGetAt, 0x007E5DA0);
    STEAM(MemberServerCreature, 0x0077D800);
    STEAM(ClientPartyTable, 0x0073FB90);
    STEAM(LabelVtable, 0x009878BC);
    STEAM(BorderVtable, 0x009875BC);
    STEAM(TextVtable, 0x009876B4);
    STEAM(KeyboardDeviceGlobal, 0x0099C884);
    STEAM(TexPendingList, 0x00A1BC2C);
    STEAM(WorldTimerGetCurrentTime, 0x0051AD20);
    STEAM(Creature_EffectArray, 0x148);
    STEAM(Creature_EffectCount, 0x14c);
    STEAM(Effect_Type, 0x08);
    STEAM(Effect_Subtype, 0x0a);
    STEAM(Effect_Duration, 0x0c);
    STEAM(Effect_ExpiryDay, 0x10);
    STEAM(Effect_ExpiryTime, 0x14);
    STEAM(Effect_Creator, 0x18);
    STEAM(Effect_SpellId, 0x1c);
    STEAM(Effect_IntCount, 0x30);
    STEAM(Effect_Ints, 0x34);
    STEAM(BeginGui2D, 0x004792C0);
    STEAM(EndGui2D, 0x004793F0);
    STEAM(PushClipRect, 0x00479460);
    STEAM(PopClipRect, 0x004799C0);
    STEAM(FlushGuiBatch, 0x0047EA60);
    STEAM(FlushArg, 0x009F5F60);
    STEAM(HudScale, 0x00479AB0);
    STEAM(ClipDepth, 0x00A309C0);
    STEAM(ClipNoFillColor, 0x00A1BA3C);
    STEAM(PanelRect, 0x00410020);
    STEAM(ImageVt_Draw, 6);
    STEAM(LabelSize, 0x148);
    STEAM(Ctl_Flags, 0x48);
    STEAM(Ctl_Id, 0x54);
    STEAM(Ctl_X, 4);
    STEAM(Ctl_Y, 8);
    STEAM(Ctl_W, 0xC);
    STEAM(Ctl_H, 0x10);
    STEAM(Ctl_VtSetRect, 4);
    STEAM(Label_Border, 0x60);
    STEAM(Label_Text, 0xD8);
    STEAM(Text_Renderer, 0x14);
    STEAM(Text_StringSub, 0x18);
    STEAM(Text_Color, 0x2C);
    STEAM(Border_Alpha, 0x24);
    STEAM(Border_Tint, 0x28);
    STEAM(Border_Fill, 0x74);
    STEAM(Panel_CtlArray, 0x24);
    STEAM(Panel_CtlCount, 0x28);
    STEAM(AppGlobal_ToWorld, 8);
    STEAM(World_ToSub, 4);
    STEAM(Sub_ToTimer, 0x10048);
    // portrait-slots / buff-duration-hud / shield-bar: the literals those three sources carried before they read the table
    STEAM(CExoStringFormat, 0x00734270);
    STEAM(GetMaxForcePoints, 0x0057eca0);
    STEAM(Slot_CardRect, 0x54c);
    STEAM(Slot_Flags, 0x590);
    STEAM(Slot_VitCtl, 0x690);
    STEAM(Slot_VitRect, 0x694);
    STEAM(Slot_ForceCtl, 0x7fc);
    STEAM(Slot_ForceRect, 0x800);
    STEAM(Slot_ArrowBuff, 0xddc);
    STEAM(Slot_ArrowDebuff, 0xddd);
    STEAM(Slot_Overlay0, 0x1b8);
    STEAM(Slot_Overlay1, 0x300);
    STEAM(Slot_Overlay2, 0x448);
    STEAM(Menu_PortraitSlot, 0x2350);
    STEAM(Menu_Char2, 0x3134);
    STEAM(Menu_Char3, 0x36DC);
    // ShieldBar's MenuCharGroup was dword indices: LBL_CHARn, LEVELUPn, BACKn, BTN_CHANGEn, offset = index * 4
    STEAM(Menu_Char2, 0xc4d * 4);   STEAM(Menu_Level2, 0xc9f * 4);  STEAM(Menu_Back2, 0xcf1 * 4);   STEAM(Menu_Change2, 0xd43 * 4);
    STEAM(Menu_Char3, 0xdb7 * 4);   STEAM(Menu_Level3, 0xe09 * 4);  STEAM(Menu_Back3, 0xe5b * 4);   STEAM(Menu_Change3, 0xead * 4);
    STEAM(MenuCaller_Creature, -0x38);
    STEAM(Creature_Stats, 0x1198);
    STEAM(Stats_ForceA, 0x130);
    STEAM(Stats_ForceB, 0x132);
    STEAM(Creature_VtMaxHp, 0x98);
    STEAM(Creature_VtCurHp, 0x9c);
    STEAM(Creature_IconCacheData, 0xFFC);
    STEAM(Creature_IconCacheCount, 0x1000);
    STEAM(IconEnt_F14, 0x14);
    STEAM(IconEnt_F1c, 0x1c);
    STEAM(Border_Width, 0xc);
    STEAM(Border_Height, 0x10);
    STEAM(Border_Rot, 0x20);
    STEAM(Tip_Slot, 0x1d0);
    STEAM(Tip_Text, 0x1d4);
#undef STEAM
    KT_CHECK_EQ(t.sha, kSteamSha);
    KT_CHECK_EQ(kGog.sha, kGogSha);
    // the headers still export the Steam column under their old names
    KT_CHECK_EQ((unsigned long long)gui2d::AddrPushClipRect, 0x479460ull);
    KT_CHECK_EQ((unsigned long long)gui2d::AddrClipNoFillColor, 0xA1BA3Cull);
    KT_CHECK_EQ((long long)gui2d::ImageVtDraw, 6ll);
    KT_CHECK_EQ((unsigned long long)k2fx::AddrAppGlobal, 0xA1B4A4ull);
    KT_CHECK_EQ((unsigned long long)k2fx::AddrWorldTimerGetCurrentTime, 0x51AD20ull);
    KT_CHECK_EQ((unsigned long long)guikit::AddrNew, 0x919723ull);
    KT_CHECK_EQ((unsigned long long)guikit::LabelVtable, 0x9878BCull);
    KT_CHECK_EQ((long long)guikit::LabelSize, 0x148ll);
    KT_CHECK_EQ((unsigned long long)gameaddr::AppGlobal, 0xA1B4A4ull);
}

// kGog is the unfilled table: sha set, every address / offset zero.
static void checkGogTable() {
    const K2AddrTable& g = kGog;
    KT_CHECK_EQ((unsigned long long)g.AppGlobal, 0ull);
    KT_CHECK_EQ((unsigned long long)g.New, 0ull);
    KT_CHECK_EQ((unsigned long long)g.PushClipRect, 0ull);
    KT_CHECK_EQ((long long)g.Creature_EffectArray, 0ll);
    KT_CHECK_EQ((long long)g.Ctl_Flags, 0ll);
    KT_CHECK_EQ((long long)g.LabelSize, 0ll);
    KT_CHECK_EQ((unsigned long long)g.CExoStringFormat, 0ull);
    KT_CHECK_EQ((unsigned long long)g.GetMaxForcePoints, 0ull);
    KT_CHECK_EQ((long long)g.Slot_Flags, 0ll);
    KT_CHECK_EQ((long long)g.Menu_PortraitSlot, 0ll);
    KT_CHECK_EQ((long long)g.MenuCaller_Creature, 0ll);
    KT_CHECK_EQ((long long)g.Tip_Text, 0ll);
}

static void checkSelection() {
    // pure selection
    KT_CHECK(k2addr::selectBySha(nullptr) == &kSteam);
    KT_CHECK(k2addr::selectBySha("") == &kSteam);
    KT_CHECK(k2addr::selectBySha(kSteamSha) == &kSteam);
    char lower[80]; strncpy(lower, kSteamSha, sizeof lower - 1); lower[sizeof lower - 1] = 0;
    for (char* p = lower; *p; ++p) if (*p >= 'A' && *p <= 'Z') *p = (char)(*p - 'A' + 'a');
    KT_CHECK(k2addr::selectBySha(lower) == &kSteam);
    KT_CHECK(k2addr::selectBySha(kGogSha) == &kGog);
    KT_CHECK(k2addr::selectBySha("deadbeef") == nullptr);
    KT_CHECK(k2addr::selectBySha("7751F7D8") == nullptr);                         // the 4gb-patched file hash prefix is not an env value
    char longer[96]; snprintf(longer, sizeof longer, "%sAA", kSteamSha);
    KT_CHECK(k2addr::selectBySha(longer) == nullptr);                              // a longer string is not a match
    KT_CHECK(k2addr::selectBySha("6A522E71") == nullptr);                          // neither is a prefix

    // selection from the real environment variable (uncached path)
    unsetenv("KOTOR_VERSION_SHA");
    KT_CHECK(k2addr::selectFromEnv() == &kSteam);
    setenv("KOTOR_VERSION_SHA", kSteamSha, 1);
    KT_CHECK(k2addr::selectFromEnv() == &kSteam);
    setenv("KOTOR_VERSION_SHA", lower, 1);
    KT_CHECK(k2addr::selectFromEnv() == &kSteam);
    setenv("KOTOR_VERSION_SHA", kGogSha, 1);
    KT_CHECK(k2addr::selectFromEnv() == &kGog);
    setenv("KOTOR_VERSION_SHA", "0000", 1);
    KT_CHECK(k2addr::selectFromEnv() == nullptr);
    setenv("KOTOR_VERSION_SHA", "", 1);
    KT_CHECK(k2addr::selectFromEnv() == &kSteam);
    unsetenv("KOTOR_VERSION_SHA");

    // get() is cached; with the env unset at start-up it is Steam (what the host tests and un-hooked DLLs get)
    k2addr::clearForTest();
    KT_CHECK(k2addr::get() == &kSteam);
    // test seam
    K2AddrTable custom = kSteam; custom.AppGlobal = 0x1234;
    k2addr::setForTest(&custom);
    KT_CHECK(k2addr::get() == &custom);
    KT_CHECK_EQ((unsigned long long)k2addr::addr(&K2AddrTable::AppGlobal), 0x1234ull);
    k2addr::setForTest(nullptr);
    KT_CHECK(k2addr::get() == nullptr);
    KT_CHECK_EQ((unsigned long long)k2addr::addr(&K2AddrTable::AppGlobal), 0ull);
    KT_CHECK_EQ((long long)k2addr::off(&K2AddrTable::Ctl_Flags), 0ll);
    KT_CHECK(k2addr::fnptr<void(*)()>(&K2AddrTable::BeginGui2D) == nullptr);
    k2addr::setForTest(&kGog);
    KT_CHECK(k2addr::get() == &kGog);
    k2addr::clearForTest();
    KT_CHECK(k2addr::get() == &kSteam);
    KT_CHECK(k2addr::fnptr<void(*)()>(&K2AddrTable::BeginGui2D) != nullptr);
}

// ---- Effects.h: a zero offset / null table makes the scan helpers inert ----
static void checkEffects() {
    uint8_t* c = g_arena + 0x1000;   // fake creature: array at +0x148, count at +0x14c
    memset(c, 0, 0x200);
    uintptr_t cr = reinterpret_cast<uintptr_t>(c);
    at<int>(cr, 0x148) = 0x4000; at<int>(cr, 0x14c) = 0;
    KT_CHECK(k2fx::plausibleCreature(cr));                      // Steam: count 0 is sane
    int arr = -1, cnt = -1;
    KT_CHECK(k2fx::readEffectList(cr, &arr, &cnt) && arr == 0x4000 && cnt == 0);

    K2AddrTable noCount = kSteam; noCount.Creature_EffectCount = 0;
    k2addr::setForTest(&noCount);
    arr = cnt = -1;
    KT_CHECK(!k2fx::plausibleCreature(cr));
    KT_CHECK(!k2fx::readEffectList(cr, &arr, &cnt)); KT_CHECK_EQ((long long)arr, -1ll);
    k2fx::PoolRaw pools[2];
    KT_CHECK_EQ((long long)k2fx::scanShieldPools(cr, pools, 2), 0ll);

    // world time: a zero timer offset in the chain -> false and the engine call is never made
    static int calls = 0; calls = 0;
    auto savedGet = k2fx::g_fx.getTime; auto savedApp = k2fx::g_fx.appGlobal;
    k2fx::g_fx.getTime = [](uintptr_t, int* d, int* m) { ++calls; *d = 1; *m = 2; };
    uintptr_t cell = reinterpret_cast<uintptr_t>(g_arena + 0x2000), g = cell + 0x100, a = cell + 0x200, b = cell + 0x300;
    memset(reinterpret_cast<void*>(cell), 0, 0x400 + 0x10100);
    at<uint32_t>(cell, 0) = (uint32_t)g; at<uint32_t>(g, 8) = (uint32_t)a; at<uint32_t>(a, 4) = (uint32_t)b; at<uint32_t>(b, 0x10048) = 0x7000;
    k2fx::g_fx.appGlobal = cell;
    int day = -9, ms = -9;
    k2addr::clearForTest();
    KT_CHECK(k2fx::currentTime(&day, &ms) && day == 1 && ms == 2 && calls == 1);   // Steam: works on the fake chain
    K2AddrTable noTimer = kSteam; noTimer.Sub_ToTimer = 0;
    k2addr::setForTest(&noTimer);
    day = -9; ms = -9;
    KT_CHECK(!k2fx::currentTime(&day, &ms)); KT_CHECK_EQ((long long)calls, 1ll); KT_CHECK_EQ((long long)day, -9ll);
    k2addr::setForTest(nullptr);
    KT_CHECK(!k2fx::currentTime(&day, &ms)); KT_CHECK_EQ((long long)calls, 1ll);
    // effectRatio with a zero expiry offset: neutral 1.0 (permanent)
    K2AddrTable noExpiry = kSteam; noExpiry.Effect_ExpiryDay = 0;
    uintptr_t eff = reinterpret_cast<uintptr_t>(g_arena + 0x20000);
    memset(reinterpret_cast<void*>(eff), 0, 0x100);
    at<float>(eff, 0x0c) = 100.0f; at<int>(eff, 0x10) = 5; at<int>(eff, 0x14) = 50000;
    k2addr::clearForTest();
    float full = k2fx::effectRatio(eff, 5, 0, true);
    KT_CHECK(full > 0.0f && full < 1.0f);                       // Steam: 50 s of 100 s left
    k2addr::setForTest(&noExpiry);
    KT_CHECK(k2fx::effectRatio(eff, 5, 0, true) == 1.0f);
    k2fx::g_fx.getTime = savedGet; k2fx::g_fx.appGlobal = savedApp;
    k2addr::clearForTest();
}

// ---- Gui2D.h: default engine calls never call address 0; pushClip refuses without the no-fill colour ----
static int g_pushCalls = 0;
static void checkGui2D() {
    // default Engine under a table that knows nothing: every call is a no-op / neutral (Steam would jump into the real exe)
    k2addr::setForTest(&kGog);
    gui2d::Engine def;
    def.begin(); def.end(); def.pop(); def.flush(1.0f);
    KT_CHECK_EQ((long long)def.push(1, 2, 3, 4, nullptr, 0, 1.0f), 0ll);
    KT_CHECK(def.hudScale() == 1.0f);
    int r[4] = { 9, 9, 9, 9 }; def.panelRect(0x1000, r);
    KT_CHECK_EQ((long long)r[0], 9ll);                           // untouched
    void* vt[8] = {}; void* pimg = vt;
    def.draw(&pimg, 0, 0, 1, 1, 0, 0, nullptr, 1.0f);            // slot offset unknown: no call through the vtable
    KT_CHECK(gui2d::clipNoFill() == nullptr);
    KT_CHECK_EQ((long long)gui2d::clipDepthRaw(), 0ll);
    KT_CHECK(!gui2d::inMenuFrame());
    KT_CHECK(gui2d::flushArg() == 0.0f);

    // pushClip with a substituted push fake but no no-fill colour known: refused, push never reached
    auto savedPush = gui2d::g_g2d.push;
    gui2d::g_g2d.push = [](int, int, int, int, const void*, int, float) { ++g_pushCalls; return 1; };
    g_pushCalls = 0;
    KT_CHECK_EQ((long long)gui2d::pushClip(0, 0, 4, 4), 0ll);
    KT_CHECK_EQ((long long)g_pushCalls, 0ll);
    static const float rgb[3] = { 1, 1, 1 };
    KT_CHECK_EQ((long long)gui2d::pushClip(0, 0, 4, 4, rgb), 1ll);   // an explicit colour does not need the table
    KT_CHECK_EQ((long long)g_pushCalls, 1ll);
    gui2d::g_g2d.push = savedPush;

    k2addr::clearForTest();
    KT_CHECK(gui2d::clipNoFill() == reinterpret_cast<const void*>(0xA1BA3Cu));
}

// ---- GuiKit.h: engine pointers bind from the table; unknown fields make helpers refuse ----
static int g_allocCalls = 0;
static void checkGuiKit() {
    using namespace guikit;
    k2addr::clearForTest();
    Engine steam;
    KT_CHECK_EQ((unsigned long long)reinterpret_cast<uintptr_t>(steam.alloc), 0x919723ull);
    KT_CHECK_EQ((unsigned long long)reinterpret_cast<uintptr_t>(steam.labelCtor), 0x419740ull);
    KT_CHECK_EQ((unsigned long long)reinterpret_cast<uintptr_t>(steam.playSound), 0x4122A0ull);
    KT_CHECK_EQ((unsigned long long)steam.guiSoundMgrAddr, 0xA1B49Cull);
    KT_CHECK_EQ((unsigned long long)steam.screenWAddr, 0x9F42A4ull);

    k2addr::setForTest(&kGog);
    Engine gog;
    KT_CHECK(!gog.alloc && !gog.labelCtor && !gog.initControl && !gog.exoCtor && !gog.exoDtor && !gog.textSet);
    KT_CHECK(!gog.setFont && !gog.setAlign && !gog.textColor && !gog.listAdd && !gog.loadImage && !gog.playSound);
    KT_CHECK(gog.guiSoundMgrAddr == 0 && gog.screenWAddr == 0 && gog.screenHAddr == 0);
    k2addr::setForTest(nullptr);
    Engine unknown;
    KT_CHECK(!unknown.alloc && !unknown.listAdd && unknown.guiSoundMgrAddr == 0);

    // g_gk with working fakes, but a table whose LabelSize is 0: createLabelFromTemplate refuses before allocating
    Engine saved = g_gk;
    g_gk.alloc = [](unsigned) -> void* { ++g_allocCalls; return nullptr; };
    g_gk.labelCtor = [](void*) {};
    g_gk.initControl = [](void*, void*, void*, int, int) {};
    g_gk.exoCtor = [](void* e, const char*) -> void* { return e; };
    g_gk.exoDtor = [](void*) {};
    g_allocCalls = 0;
    K2AddrTable noSize = kSteam; noSize.LabelSize = 0;
    k2addr::setForTest(&noSize);
    int why = -1;
    KT_CHECK_EQ((unsigned long long)createLabelFromTemplate(0x1000, "LBL_X", &why), 0ull);
    KT_CHECK_EQ((long long)why, (long long)CF_Alloc); KT_CHECK_EQ((long long)g_allocCalls, 0ll);
    k2addr::clearForTest();                                      // Steam: reaches the allocator (which fails here)
    why = -1;
    KT_CHECK_EQ((unsigned long long)createLabelFromTemplate(0x1000, "LBL_X", &why), 0ull);
    KT_CHECK_EQ((long long)why, (long long)CF_Alloc); KT_CHECK_EQ((long long)g_allocCalls, 1ll);

    // flag helpers on a fake control: Steam works, zero Ctl_Flags -> refuses and leaves memory alone
    uintptr_t ctl = reinterpret_cast<uintptr_t>(g_arena + 0x30000);
    memset(reinterpret_cast<void*>(ctl), 0, 0x200);
    at<int>(ctl, 0x48) = 0x20;
    KT_CHECK(setVisible(ctl, true) && isVisible(ctl) && at<int>(ctl, 0x48) == 0x2);
    K2AddrTable noFlags = kSteam; noFlags.Ctl_Flags = 0;
    k2addr::setForTest(&noFlags);
    KT_CHECK(!setVisible(ctl, false)); KT_CHECK(!isVisible(ctl)); KT_CHECK(!setClickable(ctl, false));
    KT_CHECK_EQ((long long)at<int>(ctl, 0x48), 2ll);
    // zero extent offset -> getExtent false; null table -> everything false
    K2AddrTable noExt = kSteam; noExt.Ctl_W = 0;
    k2addr::setForTest(&noExt);
    int x, y, w, h; KT_CHECK(!getExtent(ctl, &x, &y, &w, &h));
    k2addr::setForTest(nullptr);
    KT_CHECK(!getExtent(ctl, &x, &y, &w, &h)); KT_CHECK(!setRect(ctl, 0, 0, 1, 1)); KT_CHECK(!setAlpha(ctl, 1.0f));
    KT_CHECK(!setText(ctl, "x"));
    g_gk.rebind();                                               // engine-call / global fields follow the table only on rebind()
    KT_CHECK(!g_gk.alloc && g_gk.guiSoundMgrAddr == 0);
    KT_CHECK(!playHoverSound());
    g_gk = saved;
    k2addr::clearForTest();
}

int main() {
    void* m = mmap(nullptr, 1u << 20, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    checkSteamTable();
    checkGogTable();
    checkSelection();
    checkEffects();
    checkGui2D();
    checkGuiKit();
    return kt::Report("k2addr_test");
}
