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
static void checkSteamFields(const K2AddrTable& t) {
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
}
static void checkSteamTable() {
    checkSteamFields(kSteam);
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

// kGog against the literals of 21b_gog_address_map.md section 4 (static-verified from the Steam bodies; untested live). Every
// non-zero field is listed; fields 21b leaves out (KeyboardDeviceGlobal + the inventory-3D Scene/Gob/View/GuiGate block) stay zero.
static void checkGogFields(const K2AddrTable& g) {
#define GOG(field, value) KT_CHECK_EQ((long long)g.field, (long long)(value))
    GOG(AppGlobal, 0x00A11C04);
    GOG(GuiSoundMgr, 0x00A11BFC);
    GOG(PlayGuiSound, 0x00910720);
    GOG(ScreenW, 0x009F22A4);
    GOG(ScreenH, 0x009F22A8);
    GOG(GuiRefW, 0x00A0FFE8);
    GOG(GuiRefH, 0x00A0FFEC);
    GOG(ProjectionFlag, 0x00A76858);
    GOG(New, 0x00921347);
    GOG(LabelCtor, 0x009179E0);
    GOG(InitControl, 0x0090DAA0);
    GOG(StopLoadFromLayout, 0x0090DA20);
    GOG(ExoCtor, 0x00605680);
    GOG(ExoDtor, 0x00605890);
    GOG(TextSet, 0x00915200);
    GOG(SetFont, 0x009151B0);
    GOG(SetAlign, 0x00915370);
    GOG(TextColor, 0x00915510);
    GOG(PtrListAdd, 0x00534170);
    GOG(GuiLoadImage, 0x008D5470);
    GOG(GetControlledCreature, 0x004789B0);
    GOG(PartyGetAt, 0x004789D0);
    GOG(MemberServerCreature, 0x00429780);
    GOG(ClientPartyTable, 0x0040D070);
    GOG(LabelVtable, 0x0098998C);
    GOG(BorderVtable, 0x009A994C);
    GOG(TextVtable, 0x009A9A28);
    GOG(TexPendingList, 0x00A14CD4);
    GOG(WorldTimerGetCurrentTime, 0x0080B200);
    GOG(AppGlobal_ToWorld, 0x8);
    GOG(World_ToSub, 0x4);
    GOG(Sub_ToTimer, 0x10048);
    GOG(Creature_EffectArray, 0x148);
    GOG(Creature_EffectCount, 0x14C);
    GOG(Effect_Type, 0x8);
    GOG(Effect_Subtype, 0xA);
    GOG(Effect_Duration, 0xC);
    GOG(Effect_ExpiryDay, 0x10);
    GOG(Effect_ExpiryTime, 0x14);
    GOG(Effect_Creator, 0x18);
    GOG(Effect_SpellId, 0x1C);
    GOG(Effect_IntCount, 0x30);
    GOG(Effect_Ints, 0x34);
    GOG(BeginGui2D, 0x008CFBD0);
    GOG(EndGui2D, 0x008CFD00);
    GOG(PushClipRect, 0x008CFD70);
    GOG(PopClipRect, 0x008D02D0);
    GOG(FlushGuiBatch, 0x008D5370);
    GOG(FlushArg, 0x009F7144);
    GOG(HudScale, 0x008D03C0);
    GOG(ClipDepth, 0x00A32B90);
    GOG(ClipNoFillColor, 0x00A768A4);
    GOG(PanelRect, 0x0090E4A0);
    GOG(ImageVt_Draw, 0x6);
    GOG(LabelSize, 0x148);
    GOG(Ctl_Flags, 0x48);
    GOG(Ctl_Id, 0x54);
    GOG(Ctl_X, 0x4);
    GOG(Ctl_Y, 0x8);
    GOG(Ctl_W, 0xC);
    GOG(Ctl_H, 0x10);
    GOG(Ctl_VtSetRect, 0x4);
    GOG(Label_Border, 0x60);
    GOG(Label_Text, 0xD8);
    GOG(Text_Renderer, 0x14);
    GOG(Text_StringSub, 0x18);
    GOG(Text_Color, 0x2C);
    GOG(Border_Alpha, 0x24);
    GOG(Border_Tint, 0x28);
    GOG(Border_Fill, 0x74);
    GOG(Panel_CtlArray, 0x24);
    GOG(Panel_CtlCount, 0x28);
    GOG(CExoStringFormat, 0x00606380);
    GOG(GetMaxForcePoints, 0x006977D0);
    GOG(Slot_CardRect, 0x54C);
    GOG(Slot_Flags, 0x590);
    GOG(Slot_VitCtl, 0x690);
    GOG(Slot_VitRect, 0x694);
    GOG(Slot_ForceCtl, 0x7FC);
    GOG(Slot_ForceRect, 0x800);
    GOG(Slot_ArrowBuff, 0xDDC);
    GOG(Slot_ArrowDebuff, 0xDDD);
    GOG(Slot_Overlay0, 0x1B8);
    GOG(Slot_Overlay1, 0x300);
    GOG(Slot_Overlay2, 0x448);
    GOG(Menu_PortraitSlot, 0x2350);
    GOG(Menu_Char2, 0x3134);
    GOG(Menu_Level2, 0x327C);
    GOG(Menu_Back2, 0x33C4);
    GOG(Menu_Change2, 0x350C);
    GOG(Menu_Char3, 0x36DC);
    GOG(Menu_Level3, 0x3824);
    GOG(Menu_Back3, 0x396C);
    GOG(Menu_Change3, 0x3AB4);
    GOG(MenuCaller_Creature, -0x38);
    GOG(Creature_Stats, 0x1198);
    GOG(Stats_ForceA, 0x130);
    GOG(Stats_ForceB, 0x132);
    GOG(Creature_VtMaxHp, 0x98);
    GOG(Creature_VtCurHp, 0x9C);
    GOG(Creature_IconCacheData, 0xFFC);
    GOG(Creature_IconCacheCount, 0x1000);
    GOG(IconEnt_F14, 0x14);
    GOG(IconEnt_F1c, 0x1C);
    GOG(Border_Width, 0xC);
    GOG(Border_Height, 0x10);
    GOG(Border_Rot, 0x20);
    GOG(Tip_Slot, 0x1D0);
    GOG(Tip_Text, 0x1D4);
#undef GOG
    // unknown / out-of-scope fields stay zero (helper inert)
    KT_CHECK_EQ((unsigned long long)g.KeyboardDeviceGlobal, 0ull);
    KT_CHECK_EQ((unsigned long long)g.GuiGateGlobal, 0ull);
    KT_CHECK_EQ((unsigned long long)g.SceneAddModel, 0ull);
    KT_CHECK_EQ((unsigned long long)g.GobVtable, 0ull);
    KT_CHECK_EQ((long long)g.View_Scene, 0ll);
    KT_CHECK_EQ((long long)g.Gob_Scale, 0ll);
    // GOG really differs from Steam for the functions the three mods call
    KT_CHECK(g.GetMaxForcePoints != kSteam.GetMaxForcePoints && g.PushClipRect != kSteam.PushClipRect);
    // every function field the three mods call is non-zero on GOG
    KT_CHECK(g.GetMaxForcePoints && g.CExoStringFormat && g.WorldTimerGetCurrentTime && g.BeginGui2D && g.EndGui2D && g.PushClipRect &&
             g.PopClipRect && g.FlushGuiBatch && g.HudScale && g.PanelRect && g.GuiLoadImage && g.New && g.LabelCtor && g.InitControl &&
             g.StopLoadFromLayout && g.ExoCtor && g.ExoDtor && g.TextSet && g.SetFont && g.SetAlign && g.TextColor && g.PtrListAdd &&
             g.PlayGuiSound && g.GetControlledCreature && g.PartyGetAt && g.MemberServerCreature && g.ClientPartyTable && g.AppGlobal);
}
static void checkGogTable() { checkGogFields(kGog); }

// makeSteam()/makeGog() are constexpr and normally only evaluated at compile time (kSteam/kGog). Call them at RUNTIME through
// volatile function pointers so their bodies really execute, then check the result field by field against the same literals.
static void checkRuntimeTables() {
    K2AddrTable (*volatile mkSteam)() = &k2addr::detail::makeSteam;
    K2AddrTable (*volatile mkGog)() = &k2addr::detail::makeGog;
    const K2AddrTable s = mkSteam();
    const K2AddrTable g = mkGog();
    checkSteamFields(s);
    checkGogFields(g);
    KT_CHECK(k2addr::detail::ieq(s.sha, kSteam.sha));
    KT_CHECK(k2addr::detail::ieq(g.sha, kGog.sha));
    KT_CHECK(!k2addr::detail::ieq(s.sha, g.sha));
    // the runtime table is a drop-in for the constexpr one: selection by its sha lands on the compile-time tables
    KT_CHECK(k2addr::selectBySha(s.sha) == &kSteam);
    KT_CHECK(k2addr::selectBySha(g.sha) == &kGog);
    // runtime tables are independent copies: mutating one leaves the other and the constexpr table untouched
    K2AddrTable s2 = mkSteam(); s2.AppGlobal = 0x1234;
    KT_CHECK_EQ((unsigned long long)s2.AppGlobal, 0x1234ull);
    KT_CHECK_EQ((unsigned long long)s.AppGlobal, (unsigned long long)kSteam.AppGlobal);
    KT_CHECK_EQ((unsigned long long)mkSteam().AppGlobal, 0xA1B4A4ull);
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
    char gl[80]; strncpy(gl, kGogSha, sizeof gl - 1); gl[sizeof gl - 1] = 0;
    for (char* p = gl; *p; ++p) if (*p >= 'A' && *p <= 'Z') *p = (char)(*p - 'A' + 'a');
    KT_CHECK(k2addr::selectBySha(gl) == &kGog);                                    // GOG sha, lower case
    KT_CHECK(k2addr::selectBySha("777BEE23") == nullptr);                          // GOG prefix is not a match
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
    setenv("KOTOR_VERSION_SHA", gl, 1);
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
    // default Engine under a table that knows nothing: every call is a no-op / neutral (Steam / GOG would jump into the real exe)
    static const K2AddrTable kEmpty{};
    k2addr::setForTest(&kEmpty);
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

    k2addr::setForTest(&kGog);                                   // GOG binds its own addresses (never called here)
    Engine gog;
    KT_CHECK_EQ((unsigned long long)reinterpret_cast<uintptr_t>(gog.alloc), 0x921347ull);
    KT_CHECK_EQ((unsigned long long)reinterpret_cast<uintptr_t>(gog.labelCtor), 0x9179E0ull);
    KT_CHECK_EQ((unsigned long long)reinterpret_cast<uintptr_t>(gog.playSound), 0x910720ull);
    KT_CHECK_EQ((unsigned long long)gog.guiSoundMgrAddr, 0xA11BFCull);
    KT_CHECK_EQ((unsigned long long)gog.screenWAddr, 0x9F22A4ull);
    KT_CHECK_EQ((unsigned long long)gog.screenHAddr, 0x9F22A8ull);
    static const K2AddrTable kEmptyGk{};
    k2addr::setForTest(&kEmptyGk);
    Engine none;
    KT_CHECK(!none.alloc && !none.labelCtor && !none.initControl && !none.exoCtor && !none.exoDtor && !none.textSet);
    KT_CHECK(!none.setFont && !none.setAlign && !none.textColor && !none.listAdd && !none.loadImage && !none.playSound);
    KT_CHECK(none.guiSoundMgrAddr == 0 && none.screenWAddr == 0 && none.screenHAddr == 0);
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
    checkRuntimeTables();
    checkSelection();
    checkEffects();
    checkGui2D();
    checkGuiKit();
    return kt::Report("k2addr_test");
}
