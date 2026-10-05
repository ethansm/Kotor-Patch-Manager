// common_contract_test.cpp -- Patches/Common + _shared_src CONTRACT test for Companion Self-Heal (doc 20 section 4.1).
// Pins the semantics THIS mod relies on in SafeRead.h, KpLog.h, K2Addr.h/GameAddr.h, Party.h, PortraitSlots.h, HealItems.h and ItemRead.h
// (CompanionSelfHeal.cpp). Every message starts with "companion-selfheal relies on:" so a failure names the consumer and the reason.
// Built against ../_shared_src and the KPM checkout's Patches/Common (see test.toml). Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SafeRead.h"
#include "KpLog.h"
#include "K2Addr.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "HudHost.h"
#include "HealItems.h"
#include "ItemRead.h"
#include "GuiKit.h"
#include "GuiTip.h"
#include "FileWatch.h"
#include "SelfHealUi.h"
#include "check.h"
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>

#define HCHECK(why, expr) ::kt::Check("companion-selfheal relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("companion-selfheal relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("companion-selfheal");
KP_CHANNEL(chHud, "hud");

// ---- fake kpatch.log + k2.portraitslots providers behind one require()
struct Sub { int ch, lvl; std::string text; };
static std::vector<Sub> g_subs; static std::vector<std::string> g_regs;
static int32_t KPATCH_CALL fReg(const char* id) { g_regs.push_back(id); return 7; }
static int32_t KPATCH_CALL fChan(int32_t, const char*) { return 200; }
static int32_t KPATCH_CALL fEn(int32_t, int32_t lvl) { return lvl <= KPLOG_INFO; }
static void KPATCH_CALL fSub(int32_t ch, int32_t lvl, const char* line, uint32_t len) { g_subs.push_back({ ch, lvl, std::string(line, len) }); }
static void KPATCH_CALL fMark(const char*) {}
static void KPATCH_CALL fTick(int32_t) {}
static uint32_t KPATCH_CALL fFrame() { return 41; }
static uint64_t KPATCH_CALL fNow() { return 0; }
static KPatchLogApi g_log = { sizeof(KPatchLogApi), fReg, fChan, fEn, fSub, fMark, fTick, fFrame, fNow };

static int g_registered; static int32_t g_regPhase = -1, g_regPrio = -1; static std::string g_regOwner;
static int32_t KPATCH_CALL psRegister(int32_t phase, int32_t prio, K2PsCallback fn, void*, const char* owner) { if (!fn) return 0; ++g_registered; g_regPhase = phase; g_regPrio = prio; g_regOwner = owner; return 1; }
static void KPATCH_CALL psUnreg(int32_t) {}
static int32_t KPATCH_CALL psCreature(int32_t) { return 0; }
static int32_t KPATCH_CALL psSlotAt(int32_t i, int32_t* slot, int32_t* cr) { if (i != 1) return 0; *slot = 0x5000; *cr = 0x6000; return 1; }
static uint32_t KPATCH_CALL psFrame() { return 9; }
static K2PortraitSlotsApi g_psApi = { sizeof(K2PortraitSlotsApi), psRegister, psUnreg, psCreature, psSlotAt, psFrame };
static int g_hudRegs; static int32_t g_hudPhase = -1, g_hudPrio = -1; static std::string g_hudOwner; static K2HudCallback g_hudFn;
static int32_t KPATCH_CALL huRegister(int32_t phase, int32_t prio, K2HudCallback fn, void*, const char* owner) { if (!fn) return 0; ++g_hudRegs; g_hudPhase = phase; g_hudPrio = prio; g_hudOwner = owner; g_hudFn = fn; return 3; }
static void KPATCH_CALL huUnreg(int32_t) {}
static int32_t KPATCH_CALL huHud() { return 0; }
static uint32_t KPATCH_CALL huFrame() { return 5; }
static K2HudApi g_huApi = { sizeof(K2HudApi), huRegister, huUnreg, huHud, huFrame };
static std::string g_reqName; static uint32_t g_reqVer;
static const void* KPATCH_CALL fRequire(const char* n, uint32_t v) {
    g_reqName = n; g_reqVer = v;
    if (strcmp(n, "kpatch.log") == 0) return &g_log;
    if (strcmp(n, K2HUD_IFACE) == 0) return &g_huApi;
    return &g_psApi;
}
static KPatchApi g_api = { sizeof(KPatchApi), 1, nullptr, fRequire };

static void KPATCH_CALL dummyCb(const K2PsContext*, void*) {}
static void KPATCH_CALL dummyHudCb(const K2HudContext*, void*) {}

static uint8_t* g_a;
static uintptr_t A(size_t off) { return reinterpret_cast<uintptr_t>(g_a + off); }
template <typename T> static void put(size_t off, T v) { memcpy(g_a + off, &v, sizeof v); }
static int fakeVital(uintptr_t fn, uintptr_t c, int arg) { (void)fn; (void)c; return arg ? 100 : 40; }

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_32BIT
    flags |= MAP_32BIT;
#endif
    void* m = mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    memset(m, 0, 0x10000);
    g_a = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;

    // ===== 1. saferead as rd<T> uses it
    saferead::beginScope();
    put<uint32_t>(0x100, 0x0099DCC4u); put<uint8_t>(0x174 + 7, 9); put<uint32_t>(0x200, 0xFFFF0000u);
    uint32_t vt = 0; uint8_t b = 0; uint32_t mask = 0;
    HCHECK("rd<uint32_t>(panel, 0) reads the vtable word", saferead::readAt(0x6E00u, A(0x100), 0, &vt) && vt == 0x0099DCC4u);
    HCHECK("rd<uint8_t> reads a skill byte at base+off", saferead::readAt(0x6E00u, A(0), 0x174 + 7, &b) && b == 9);
    HCHECK("rd<uint32_t>(global, 0) reads the id mask global", saferead::readAt(0x6E00u, A(0x200), 0, &mask) && mask == 0xFFFF0000u);
    uint32_t keep = 77;
    HCHECK("rd with base 0 fails (a null panel/creature must not fault)", !saferead::readAt(0x6E00u, 0, 0x30, &keep));
    HEQ("failed rd leaves *out untouched", keep, 77u);

    // ===== 2. KpLog_Init + channels
    HCHECK("HasCore() false before init", !KpLog_HasCore());
    HEQ("KpLog_Init(api) returns 1 on a complete kpatch.log service (KPatchInit)", KpLog_Init(&g_api), 1);
    HCHECK("registered under the manifest id \"companion-selfheal\"", g_regs.size() == 1 && g_regs[0] == "companion-selfheal");
    HEQ("kpatch.log version 1 requested", (unsigned long long)g_reqVer, 1ULL);
    KP_LOG(chHud, "P1 hud built panel=%08x", 0x1234u);
    HCHECK("an INFO KP_LOG with %08x is submitted verbatim", g_subs.size() == 1 && g_subs[0].text == "P1 hud built panel=00001234");

    // ===== 3. K2Addr / GameAddr
    HEQ("gameaddr::AppGlobal is the Steam app global (ItemRead g_appGlobal default)", (unsigned long long)gameaddr::AppGlobal, 0x00A1B4A4ULL);

    HEQ("gameaddr::CanUseItem is the Steam CanUseItem (thiscall, RET 0xC, called with itemPtr,0,1)", (unsigned long long)gameaddr::CanUseItem, 0x005D2F80ULL);
    HEQ("gameaddr::UseItemRouteA is the Steam route A (thiscall, RET 0x20, 8 stack args)", (unsigned long long)gameaddr::UseItemRouteA, 0x0057D8B0ULL);
    HEQ("gameaddr::ProjectionFlag is cleared/restored around the label create", (unsigned long long)gameaddr::ProjectionFlag, 0x00A1B9F0ULL);
    HEQ("gameaddr::RmbHoldFlag / RButtonDownHoldSite are the R6 RMB swallow flag and hook site", (unsigned long long)gameaddr::RmbHoldFlag * 0 + gameaddr::RmbHoldFlag + gameaddr::RButtonDownHoldSite, 0x00A7FCBCULL + 0x007B3817ULL);
    HCHECK("guikit::createLabelFromTemplate(panel, tag, int* why, uint32_t* badVt) -> uintptr_t", static_cast<uintptr_t(*)(uintptr_t, const char*, int*, uint32_t*)>(&guikit::createLabelFromTemplate) != nullptr);
    HCHECK("guikit::appendToPanel(panel, ctl) -> bool", static_cast<bool(*)(uintptr_t, uintptr_t)>(&guikit::appendToPanel) != nullptr);
    HCHECK("guikit::setText(ctl, const char*) -> bool", static_cast<bool(*)(uintptr_t, const char*)>(&guikit::setText) != nullptr);
    HCHECK("guikit::setVisible(ctl, bool) / getExtent(ctl, x,y,w,h) keep their signatures", static_cast<bool(*)(uintptr_t, bool)>(&guikit::setVisible) != nullptr && static_cast<bool(*)(uintptr_t, int*, int*, int*, int*)>(&guikit::getExtent) != nullptr);
    HEQ("guikit CF_Count is a CreateFail reason (label P2 log)", (int)guikit::CF_Count, 4);
    HEQ("K2PsContext carries inMenu (the call's menu refusal)", (long long)offsetof(K2PsContext, inMenu), 28LL);

    // ===== 4. Party: Vitals defaults + vitals() through the vtable slots
    party::Vitals def;
    HCHECK("a default Vitals is zeroed and not ok (members() leaves unreadable members like this)", def.client == 0 && def.server == 0 && def.cur == 0 && def.max == 0 && !def.ok && !def.down);
    put<uint32_t>(0x300, (uint32_t)A(0x400));                       // server creature +0 = vtable
    put<uint32_t>(0x400 + party::SlotMaxHp, 0x1111); put<uint32_t>(0x400 + party::SlotCurHp, 0x2222);
    party::g_callVital = fakeVital;
    party::Vitals v;
    HCHECK("party::vitals(server) reads max via slot 0x98 (arg 1) and cur via slot 0x9C (arg 0)", party::vitals(A(0x300), &v) && v.max == 100 && v.cur == 40 && v.ok && !v.down);
    HEQ("vitals() records the server pointer it was given", (unsigned long long)v.server, (unsigned long long)A(0x300));
    party::g_appGlobal = A(0x500);   // cell holds 0 -> chain fails
    party::Vitals ms[3];
    HEQ("members() returns 0 when the app chain is unreadable (no engine call)", party::members(ms, 3), 0);
    HEQ("members(nullptr, 3) returns 0", party::members(nullptr, 3), 0);

    // ===== 5. PortraitSlots client
    k2ps::Client c;
    c.init(&g_api, "companion-selfheal");
    HCHECK("add() accepts the FRAME_BEGIN callback at prio 300", c.add(K2PS_FRAME_BEGIN, 300, dummyCb));
    HCHECK("connect() succeeds against a complete v1 table", c.connect());
    HEQ("it required \"k2.portraitslots\"", g_reqName == K2PS_IFACE, 1);
    HEQ("version 1", (unsigned long long)g_reqVer, (unsigned long long)K2PS_VERSION);
    HEQ("one registration", g_registered, 1);
    HEQ("phase FRAME_BEGIN", (long long)g_regPhase, (long long)K2PS_FRAME_BEGIN);
    HEQ("priority 300", (long long)g_regPrio, 300LL);
    HCHECK("owner is the manifest id", g_regOwner == "companion-selfheal");
    int32_t slot = 0, cr = 0;
    HCHECK("slotAt(1) returns the slot/creature pair", c.slotAt(1, &slot, &cr) && slot == 0x5000 && cr == 0x6000);
    HCHECK("slotAt on an empty entry returns false", !c.slotAt(0, &slot, &cr));
    HEQ("K2PS_MAX_SLOTS bounds the P3 walk", K2PS_MAX_SLOTS, 4);
    k2ps::Client c2; c2.poll();   // poll() before init/connect must not crash
    HCHECK("poll() on an unconnected, uninitialised client stays unconnected", !c2.connected);

    // ===== 5b. hud-host client (K2HUD_BUILD, 0.4.2-dev: the HUD labels are built in hud-host's BUILD phase, no hook of ours)
    HEQ("K2HUD_BUILD is phase 3 (onHudBuild registration)", K2HUD_BUILD, 3);
    HCHECK("K2HudContext has a panel field, covered by struct_size, after the older fields", offsetof(K2HudContext, panel) + sizeof(int32_t) <= sizeof(K2HudContext) && offsetof(K2HudContext, panel) > offsetof(K2HudContext, targetType));
    HCHECK("K2HudContext struct_size gate: offsetof(panel)+sizeof(panel) == sizeof(K2HudContext) on v1 (the onHudBuild guard)", offsetof(K2HudContext, panel) + sizeof(K2HudContext::panel) == sizeof(K2HudContext));
    {
        k2hud::Client hc;
        hc.init(&g_api, "companion-selfheal");
        HCHECK("k2hud::Client::add() accepts the BUILD callback at prio 200", hc.add(K2HUD_BUILD, 200, dummyHudCb));
        HCHECK("k2hud::Client::connect() succeeds against a complete v1 K2HudApi", hc.connect());
        HCHECK("it required \"k2.hud\" v1", g_reqName == K2HUD_IFACE && g_reqVer == (uint32_t)K2HUD_VERSION);
        HEQ("one hud registration", g_hudRegs, 1);
        HEQ("phase K2HUD_BUILD", (long long)g_hudPhase, (long long)K2HUD_BUILD);
        HEQ("priority 200", (long long)g_hudPrio, 200LL);
        HCHECK("hud owner is the manifest id and the callback is the one added", g_hudOwner == "companion-selfheal" && g_hudFn == dummyHudCb);
        HCHECK("manifest requires_interfaces: the iface id string is \"k2.hud\"", strcmp(K2HUD_IFACE, "k2.hud") == 0 && K2HUD_VERSION == 1);
    }

    // ===== 6. HealItems
    heal::AutoCfg cfg;
    HCHECK("default AutoCfg is off, 35 percent, keep 2 (the probe sets on=true)", !cfg.on && cfg.hpPct == 35 && cfg.keep == 2);
    cfg.on = true; heal::AutoState st;
    HCHECK("shouldAutoUse: 20/100 alive commandable gameplay, fresh state -> true", heal::shouldAutoUse(cfg, 20, 100, true, true, true, st, 1000));
    HCHECK("shouldAutoUse: exactly 35/100 is NOT below the threshold", !heal::shouldAutoUse(cfg, 35, 100, true, true, true, st, 1000));
    HCHECK("shouldAutoUse: not commandable -> false", !heal::shouldAutoUse(cfg, 20, 100, true, false, true, st, 1000));
    HEQ("healEstimate(64, TI 10, repair 0, WIS 14, INT 0) = 10 + 10 + 2", heal::healEstimate(64, 10, 0, 14, 0), 22);
    HEQ("healEstimate of an unknown spells.2da row is -1", heal::healEstimate(5, 10, 10, 10, 10), -1);
    heal::HealCand cands[2] = { { 1, 64, 22, 3, 0, true }, { 2, 65, 42, 1, 0, true } };
    HEQ("pickHealItem(missing 30) takes the smallest covering heal (index 1)", heal::pickHealItem(30, cands, 2, 1), 1);
    HEQ("pickHealItem with missing 0 picks nothing", heal::pickHealItem(0, cands, 2, 1), -1);
    HEQ("HealCand brace-init order is key,row,heal,count,cost,usable", cands[1].count, 1);

    // ===== 7. ItemRead
    using namespace itemread;
    HEQ("item key flag", (long long)ItemKeyFlag, 0x40000000LL);
    HEQ("stackCount parses a trailing (N)", stackCount(0x40000001, "Medpac (3)"), 3);
    HEQ("stackCount: no suffix means 0 (the probe treats that as count 1)", stackCount(0x40000001, "Medpac"), 0);
    const size_t hud = 0x1000, ents = 0x1800;
    put<int32_t>(hud + Hud_GroupLists + 12, (int32_t)A(ents)); put<int32_t>(hud + Hud_GroupLists + 12 + 4, 1);
    put<uint32_t>(ents, (uint32_t)A(0x1C00)); put<int32_t>(ents + 4, 10); memcpy(g_a + 0x1C00, "Medpac (3)", 11);
    put<int32_t>(ents + Entry_Key, 0x40000123);
    std::vector<Entry> items;
    HEQ("readGroup(hud, 1) reads group 1's list (ItemGroup)", readGroup(A(hud), 1, &items), 1);
    HCHECK("entry key/name round-trip", items.size() == 1 && items[0].key == 0x40000123 && items[0].name == "Medpac (3)");
    put<uint32_t>(0x80, (uint32_t)A(0x90)); put<uint32_t>(0x90 + 8, (uint32_t)A(0xA00));
    g_appGlobal = A(0x80);
    g_clientToServer = [](uintptr_t, uint32_t id) -> uint32_t { return id + 1; };
    g_getItem = [](uintptr_t, uint32_t) -> uint32_t { return (uint32_t)A(0x2000); };
    uint32_t srv = 0;
    HCHECK("serverItemFor reports the server id through its out param and returns the item pointer", serverItemFor(0x123, &srv) == A(0x2000) && srv == 0x124);
    put<int32_t>(0x2000 + Item_PropCount, 1); put<uint32_t>(0x2000 + Item_PropArray, (uint32_t)A(0x2100));
    put<int16_t>(0x2100, 10); put<int16_t>(0x2102, 64); put<uint8_t>(0x2100 + 0xC, 1);
    ItemProp p = readItemProp(A(0x2000));
    HCHECK("readItemProp: type 10 (CastSpell), row at +2, usable at +0xC", p.count == 1 && p.type == 10 && p.row == 64 && p.usable == 1);

    // ===== 8. 0.3.0-dev UI APIs (CS4 + CS5)
    HCHECK("guikit::setClickable(ctl, bool) / hit(ctl, x, y) / cursorEngine(int*, int*) keep their signatures", static_cast<bool(*)(uintptr_t, bool)>(&guikit::setClickable) != nullptr && static_cast<bool(*)(uintptr_t, int, int)>(&guikit::hit) != nullptr && static_cast<bool(*)(int*, int*)>(&guikit::cursorEngine) != nullptr);
    HCHECK("guikit::setRect/setFont/setAlign/setAlpha/setTextColor/getTextColor/playHoverSound keep their signatures", static_cast<bool(*)(uintptr_t, int, int, int, int)>(&guikit::setRect) != nullptr && static_cast<bool(*)(uintptr_t, const char*)>(&guikit::setFont) != nullptr && static_cast<bool(*)(uintptr_t, unsigned)>(&guikit::setAlign) != nullptr && static_cast<bool(*)(uintptr_t, float)>(&guikit::setAlpha) != nullptr && static_cast<bool(*)(uintptr_t, float, float, float)>(&guikit::setTextColor) != nullptr && static_cast<bool(*)(uintptr_t, float*)>(&guikit::getTextColor) != nullptr && static_cast<bool(*)()>(&guikit::playHoverSound) != nullptr);
    HCHECK("guikit::clickEdge/rclickEdge(ClickEdge&) -> bool with a default-constructed (prev=false) edge", static_cast<bool(*)(guikit::ClickEdge&)>(&guikit::clickEdge) != nullptr && static_cast<bool(*)(guikit::ClickEdge&)>(&guikit::rclickEdge) != nullptr && !guikit::ClickEdge().prev);
    HCHECK("guitip::setNative(ctl, text) -> bool and nativeSupported() -> bool", static_cast<bool(*)(uintptr_t, const char*)>(&guitip::setNative) != nullptr && static_cast<bool(*)()>(&guitip::nativeSupported) != nullptr);
    HEQ("Slot_VitRect is +0x694 on Steam (the card vit rect read per frame)", (long long)k2addr::get()->Slot_VitRect, 0x694LL);

    using namespace shui;
    Rect vit{ 3671, 1285, 47, 96 };   // live small (companion) card vit, 3840x1600 (doc 08 probe 1)
    const Rect none{ 0, 0, 0, 0 }, leaderVit{ 3511, 1386, 103, 208 };   // live leader vit
    Rect br = buttonRect(vit);
    HCHECK("shui::buttonRect(live companion vit 47x96) is a 16..24 px square left of the bar", br.w == br.h && br.w >= 16 && br.w <= 24 && br.x + br.w < vit.x);
    HCHECK("shui::buttonRect of the leader bar (w >= 70) is the empty rect; isLeaderVit splits 103 vs 47", buttonRect(leaderVit) == none && isLeaderVit(leaderVit) && !isLeaderVit(vit));
    HCHECK("shui::resolveState priority: hidden > full > noitem > pending > auto > normal", resolveState(true, false, 10, 10, true, false, false) == BtnState::FullHp && resolveState(true, false, 5, 10, false, false, false) == BtnState::NoItem && resolveState(true, false, 5, 10, true, true, true) == BtnState::Pending && resolveState(true, false, 5, 10, true, false, true) == BtnState::AutoOn && resolveState(true, true, 5, 10, true, false, false) == BtnState::Hidden);
    HCHECK("shui::clickAllowed false for Hidden/FullHp/Pending, true for NoItem (it logs) and Normal; clickMinCount is 1", !clickAllowed(BtnState::Pending) && !clickAllowed(BtnState::FullHp) && clickAllowed(BtnState::NoItem) && clickAllowed(BtnState::Normal) && clickMinCount(BtnState::AutoOn) == 1);
    PopLayout L = popoverLayout(br, 3840);
    HCHECK("shui::popoverLayout: bg first, items inside the bg, left of the button", L.r[0].x + L.r[0].w <= br.x && L.r[1].y == L.r[0].y && L.r[5].x + L.r[5].w == L.r[0].x + L.r[0].w);
    HEQ("shui::popoverHit on item 3 (HpPlus) centre", (int)popoverHit(L, L.r[3].x + 1, L.r[3].y + 1), (int)PopItem::HpPlus);
    HEQ("shui::popoverHit outside the items", (int)popoverHit(L, L.r[0].x - 5, L.r[0].y), (int)PopItem::None);
    HCHECK("shui::popValueRect rows 2/3 are between the steppers, other rows empty", popValueRect(L, 2).w > 0 && popValueRect(L, 3).y == L.r[4].y && popValueRect(L, 1) == none);
    CompCfg cc;
    HCHECK("shui::applyPop: toggle auto, hp +5, keep -1 (defaults 35/2)", applyPop(cc, PopItem::AutoToggle).autoOn && applyPop(cc, PopItem::HpPlus).hpPct == 40 && applyPop(cc, PopItem::KeepMinus).keep == 1 && applyPop(cc, PopItem::None) == cc);
    HCHECK("shui::popLabel / popValueLabel texts", popLabel(PopItem::AutoToggle, cc) == "Auto: OFF" && popLabel(PopItem::HpMinus, cc) == "-" && popLabel(PopItem::KeepPlus, cc) == "+" && popValueLabel(2, cc) == "Use below 35%" && popValueLabel(3, cc) == "Keep 2");
    CfgTable tb = parseFile("[General]\nLog=1\n[defaults]\ndefault=auto:0,hp:35,keep:2\n[companions]\nbao_dur=auto:1,hp:50,keep:3\n");
    HCHECK("shui::parseFile + lookup: known tag row, unknown tag -> defaults", lookup(tb, "bao_dur").autoOn && lookup(tb, "bao_dur").hpPct == 50 && lookup(tb, "nobody") == tb.def);
    upsert(tb, "bao_dur", CompCfg{ false, 60, 4 }); upsert(tb, "t3m4", CompCfg{ true, 20, 0 });
    std::string out = serializeFile(tb, "[General]\nLog=1\n[defaults]\ndefault=auto:0,hp:35,keep:2\n[companions]\nbao_dur=auto:1,hp:50,keep:3\n");
    HCHECK("shui::serializeFile keeps foreign sections and round-trips through parseFile", out.find("[General]\nLog=1\n") == 0 && parseFile(out) == tb);
    Pending pe; pe.on = true; pe.since = 1000; pe.preHp = 10; pe.preStack = 3;
    HCHECK("shui::pendingDone: HP up, stack down, item gone, timeout; otherwise false", pendingDone(pe, 1100, 11, 3) && pendingDone(pe, 1100, 10, 2) && pendingDone(pe, 1100, 10, -1) && pendingDone(pe, 1000 + kPendingTimeoutMs, 10, 3) && !pendingDone(pe, 1100, 10, 3));
    HEQ("shui::kRingFrames is 8 (csh_ring0..7) and kRingFrameMs is 90", (long long)kRingFrames * 1000 + (long long)kRingFrameMs, 8090LL);
    {
        Look g = lookFor(BtnState::Normal, true, 0), n = lookFor(BtnState::Normal, false, 0), d = lookFor(BtnState::FullHp, false, 0), h = lookFor(BtnState::Hidden, true, 0);
        HCHECK("shui::lookFor: ring only when autoOn and not Hidden; Hidden alpha 0; FullHp/NoItem grey+dim", g.ring && !n.ring && !h.ring && h.alpha == 0.0f && d.alpha < 1.0f && d.r == d.g && d.g == d.b);
        HCHECK("shui::lookFor: gold (r>g>b) when auto, ringFrame in [0,kRingFrames) and advances every kRingFrameMs", g.r > g.g && g.g > g.b && lookFor(BtnState::Normal, true, 90 * 3 + 5).ringFrame == 3 && lookFor(BtnState::Normal, true, 0xFFFFFFF0u).ringFrame < kRingFrames);
        Rect rr = ringRect(br);
        HCHECK("shui::ringRect is a square larger than the button and centred on it; empty for h<=0", rr.w == rr.h && rr.w > br.w && rr.x + rr.w / 2 == br.x + br.w / 2 && ringRect(none) == none);
    }
    HCHECK("guikit::setFill(ctl, image, tint) -> bool and guikit::imageFor(resref) -> void*", static_cast<bool(*)(uintptr_t, void*, const float*)>(&guikit::setFill) != nullptr && static_cast<void*(*)(const char*, bool*, int*)>(&guikit::imageFor) != nullptr);
    HEQ("gameaddr::TexBasicCtor/EngineNewArr/TexListAdd/TexReset (embedded-texture recipe)", (long long)gameaddr::TexBasicCtor + gameaddr::EngineNewArr + gameaddr::TexListAdd + gameaddr::TexReset, 0x423000LL + 0x91D288LL + 0x4491C0LL + 0x423990LL);
    HEQ("gameaddr::MessagesPanelVtable is 0x0099E5EC (gate whitelist)", (long long)gameaddr::MessagesPanelVtable, 0x99E5ECLL);
    HEQ("kPendingTimeoutMs is 6000", (long long)kPendingTimeoutMs, 6000LL);

    // filewatch: atomicWrite + changed/accept (the INI save + hot reload)
    char tmpl[] = "/tmp/cs_contract_XXXXXX";
    int fd = mkstemp(tmpl);
    HCHECK("mkstemp for the filewatch checks", fd >= 0);
    if (fd >= 0) {
        close(fd);
        std::string path = tmpl;
        HCHECK("filewatch::atomicWrite(path, text, makeBakOnce) -> true", filewatch::atomicWrite(path.c_str(), "a=1\n", true));
        filewatch::ReloadPoll rp;
        HCHECK("filewatch::changed: the first call reports a change", filewatch::changed(rp, path.c_str(), 5000));
        HCHECK("filewatch::changed is throttled (1 s) and quiet when unchanged", !filewatch::changed(rp, path.c_str(), 5100) && !filewatch::changed(rp, path.c_str(), 7000));
        HCHECK("filewatch::atomicWrite again (bak exists) works; accept() makes our own write quiet", filewatch::atomicWrite(path.c_str(), "a=2\n", true) && (filewatch::accept(rp, path.c_str()), !filewatch::changed(rp, path.c_str(), 9000)));
        remove(path.c_str()); remove((path + ".bak").c_str());
    }
    return kt::Report("common_contract_test (companion_selfheal)");
}
