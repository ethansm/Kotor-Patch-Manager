// enemy_strip_draw_test.cpp -- host fake-engine test for the BUILD + draw path of EnemyStrip.cpp (Enemy Effects Strip, doc 39).
// EnemyStrip.cpp is #included whole (like juhani_bundle_test.cpp), so the test drives the real callbacks (captured from a fake hud-host
// k2.hud@1 through the real KPatchInit) and the real private state. The engine is faked through the existing seams only:
//   GuiKit Engine pointers (alloc/ctor/initControl/exo/listAdd/loadImage/callSetRect) over a MAP_32BIT arena, party g_serverCreature,
//   k2fx g_fx (world time), K2Addr setForTest(kSteam), FakeKPatchApi (kpatch.log + k2.hud) and the stubs/ in-memory INI.
// Every GuiKit setter the mod calls is routed through guikit_rec (a recording wrapper that forwards to the real GuiKit.h over the fake
// label memory), so asserts are on a RECORDED CALL LOG (create/append/setVisible/setClickable/setRect/setFill/setAlpha/setBorderTint)
// plus the resulting label memory (flags, border fill/alpha); rects and tints are asserted from the call records.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

#include "SafeRead.h"
#include "KpLog.h"
#include "HudHost.h"
#include "K2Addr.h"
#include "Party.h"
#include "Effects.h"
#include "EffectIcons.h"
#include "GuiKit.h"
#include "ModConfigApi.h"
#include "EnemyStripLogic.h"
#include "check.h"
#include "FakeKPatchApi.h"

// ---------------------------------------------------------------- recording GuiKit wrapper
namespace real_gk = guikit;   // the real header, kept reachable after the rename below

namespace trec {
enum Op { O_Create, O_Append, O_Vis, O_Click, O_Rect, O_Fill, O_Alpha, O_Tint, O_Text };
struct Rec {
    Op op = O_Create;
    uintptr_t c = 0;        // the control (Create: the new label, 0 on failure)
    uintptr_t panel = 0;    // Create / Append
    const char* tag = "";   // Create
    bool flag = false;      // Vis / Click argument
    int r[4] = { 0, 0, 0, 0 };   // Rect
    void* img = nullptr;    // Fill
    float f = 0.f;          // Alpha
    float rgb[3] = { 0.f, 0.f, 0.f };   // Tint
    char text[32] = { 0 };  // Text
    bool ok = true;         // the real GuiKit call's result
};
inline std::vector<Rec>& log() { static std::vector<Rec> l; return l; }
inline Rec& add(Op op, uintptr_t c) { log().emplace_back(); Rec& r = log().back(); r.op = op; r.c = c; return r; }
}  // namespace trec

namespace guikit_rec {
using namespace guikit;   // everything else (imageFor, constants, CF_*) resolves to the real header
inline uintptr_t createLabelFromTemplate(uintptr_t panel, const char* tag, int* why = nullptr, uint32_t* bad = nullptr) {
    const uintptr_t c = real_gk::createLabelFromTemplate(panel, tag, why, bad);
    trec::Rec& r = trec::add(trec::O_Create, c);
    r.panel = panel; r.tag = tag; r.ok = c != 0;
    return c;
}
inline bool appendToPanel(uintptr_t panel, uintptr_t ctl) {
    const bool ok = real_gk::appendToPanel(panel, ctl);
    trec::Rec& r = trec::add(trec::O_Append, ctl);
    r.panel = panel; r.ok = ok;
    return ok;
}
inline bool setVisible(uintptr_t ctl, bool vis) { const bool ok = real_gk::setVisible(ctl, vis); trec::Rec& r = trec::add(trec::O_Vis, ctl); r.flag = vis; r.ok = ok; return ok; }
inline bool setClickable(uintptr_t ctl, bool v) { const bool ok = real_gk::setClickable(ctl, v); trec::Rec& r = trec::add(trec::O_Click, ctl); r.flag = v; r.ok = ok; return ok; }
inline bool setRect(uintptr_t ctl, int x, int y, int w, int h) {
    const bool ok = real_gk::setRect(ctl, x, y, w, h);
    trec::Rec& r = trec::add(trec::O_Rect, ctl);
    r.r[0] = x; r.r[1] = y; r.r[2] = w; r.r[3] = h; r.ok = ok;
    return ok;
}
inline bool setFill(uintptr_t ctl, void* image, const float* tint = nullptr) {
    const bool ok = real_gk::setFill(ctl, image, tint);
    trec::Rec& r = trec::add(trec::O_Fill, ctl); r.img = image; r.ok = ok;
    return ok;
}
inline bool setAlpha(uintptr_t ctl, float a) { const bool ok = real_gk::setAlpha(ctl, a); trec::Rec& r = trec::add(trec::O_Alpha, ctl); r.f = a; r.ok = ok; return ok; }
inline bool setBorderTint(uintptr_t ctl, const float* rgb) {
    const bool ok = real_gk::setBorderTint(ctl, rgb);
    trec::Rec& r = trec::add(trec::O_Tint, ctl);
    for (int k = 0; k < 3; ++k) r.rgb[k] = rgb[k];
    r.ok = ok;
    return ok;
}
inline bool setText(uintptr_t ctl, const char* text) {
    const bool ok = real_gk::setText(ctl, text);
    trec::Rec& r = trec::add(trec::O_Text, ctl);
    strncpy(r.text, text ? text : "", sizeof r.text - 1);
    r.ok = ok;
    return ok;
}
}  // namespace guikit_rec

#define guikit guikit_rec
#include "EnemyStrip.cpp"
#undef guikit

// ---------------------------------------------------------------- fake engine memory
static uint8_t* tArena = nullptr;
static size_t tUsed = 0;
constexpr size_t kArenaSize = 4u << 20;
static uintptr_t tAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (tUsed + n > kArenaSize) { std::printf("arena full\n"); exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(tArena + tUsed);
    memset(tArena + tUsed, 0, n);
    tUsed += n;
    return a;
}
template <typename T> static T& tAt(uintptr_t base, int off) { return *reinterpret_cast<T*>(base + static_cast<uintptr_t>(off)); }

// Image registry: resref -> fake image pointer (get-or-create, so the test can predict what the loader hands out). Missing = no image.
static std::vector<std::string> tImgNames;
static const char* const kMissingImg = "ip_sleep";   // the engine cannot load this one (scenario 8)
static void* tImgPtr(const char* name) {
    for (size_t i = 0; i < tImgNames.size(); ++i) if (tImgNames[i] == name) return reinterpret_cast<void*>(static_cast<uintptr_t>(0x10000u + 8u * i));
    tImgNames.push_back(name);
    return reinterpret_cast<void*>(static_cast<uintptr_t>(0x10000u + 8u * (tImgNames.size() - 1)));
}
static int tLoads = 0;
static void* fxLoadImage(const char* n) {
    ++tLoads;
    if (!strcmp(n, kMissingImg)) return nullptr;
    return tImgPtr(n);
}
static int tAllocN = 0, tAllocFailAt = -1;   // label allocations counted since the last reset; the one at index tAllocFailAt returns null
static void* fxAlloc(unsigned n) {
    if (tAllocN++ == tAllocFailAt) return nullptr;
    return reinterpret_cast<void*>(tAlloc(n));
}
static std::vector<std::pair<uintptr_t, unsigned>> tDtors;   // label destructor calls (control, flags) made through guikit::releaseUnlinkedLabel
static void fxLabelDtor(uint32_t, uintptr_t ctl, unsigned flags) { tDtors.emplace_back(ctl, flags); }
static void fxLabelCtor(void* t) {
    const uintptr_t c = reinterpret_cast<uintptr_t>(t);
    tAt<uint32_t>(c, 0) = real_gk::LabelVtable;
    tAt<int>(c, real_gk::Ctl_Flags) = real_gk::FlagVisible;   // engine ctor default: visible (createLabelFromTemplate hides it)
    tAt<uint32_t>(c + real_gk::Label_Border, 0) = real_gk::BorderVtable;
}
static void fxInit(void*, void* ctl, void*, int, int) { tAt<int>(reinterpret_cast<uintptr_t>(ctl), real_gk::Ctl_Id) = -1; }
static void* fxExoCtor(void* e, const char* s) { memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fxExoDtor(void*) {}
static void fxListAdd(void* list, void* item) {   // panel control list {ptr, count, cap} at panel + 0x24
    const uintptr_t l = reinterpret_cast<uintptr_t>(list);
    uint32_t arr = tAt<uint32_t>(l, 0);
    const int n = tAt<int>(l, 4);
    if (!arr) { arr = static_cast<uint32_t>(tAlloc(4 * 64)); tAt<uint32_t>(l, 0) = arr; }
    tAt<uint32_t>(arr, n * 4) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(item));
    tAt<int>(l, 4) = n + 1;
}
static void fxSetRect(uint32_t, uintptr_t, int*) {}   // the wrapper records the call; the engine side is a no-op

// Server creature lookup (Party.h MemberServerCreature seam): client -> server.
struct TxCre { uintptr_t client = 0, server = 0; };
static std::vector<TxCre> tCres;
static int tSrvCalls = 0;
static uintptr_t fxServerCreature(void* m) {
    ++tSrvCalls;
    for (const TxCre& c : tCres) if (c.client == reinterpret_cast<uintptr_t>(m)) return c.server;
    return 0;
}
static int tNowDay = 0, tNowMs = 1000;
static void fxGetTime(uintptr_t, int* day, int* ms) { *day = tNowDay; *ms = tNowMs; }

static TxCre makeCre() {
    TxCre c;
    c.client = tAlloc(0x100);
    tAt<uint32_t>(c.client, 0) = 0x00A00000u;   // non-zero vtable (resolveServer guard)
    c.server = tAlloc(0x1200);
    tCres.push_back(c);
    return c;
}
struct TxEff { int type; int sub; int spell; int state; float dur; int expMs; };
static void setEffects(const TxCre& c, const std::vector<TxEff>& v) {
    tAt<uint32_t>(c.server, 0x148) = 0;
    tAt<int>(c.server, 0x14c) = 0;
    if (v.empty()) return;
    const uintptr_t arr = tAlloc(4 * v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        const uintptr_t e = tAlloc(0x40);
        tAt<uint16_t>(e, 0x08) = static_cast<uint16_t>(v[i].type);
        tAt<uint16_t>(e, 0x0a) = static_cast<uint16_t>(v[i].sub);
        tAt<float>(e, 0x0c) = v[i].dur;
        tAt<int>(e, 0x10) = 0;
        tAt<int>(e, 0x14) = v[i].expMs;
        tAt<int>(e, 0x1c) = v[i].spell;
        if (v[i].state >= 0) {
            const uintptr_t ints = tAlloc(16);
            tAt<int>(ints, 0) = v[i].state;
            tAt<int>(e, 0x30) = 1;
            tAt<uint32_t>(e, 0x34) = static_cast<uint32_t>(ints);
        }
        tAt<uint32_t>(arr, static_cast<int>(i) * 4) = static_cast<uint32_t>(e);
    }
    tAt<uint32_t>(c.server, 0x148) = static_cast<uint32_t>(arr);
    tAt<int>(c.server, 0x14c) = static_cast<int>(v.size());
}

// Target menu fixture: origin (100,400), name rect (20,60,200,24) -> auto icon edge 24, row centred over the name.
static uintptr_t tMenu = 0;
constexpr int kMx = 100, kMy = 400, kNx = 20, kNy = 60, kNw = 200, kNh = 24;
static void setMenu(bool shown) {
    tAt<uint8_t>(tMenu, 0x1BAC) = shown ? 1 : 0;
    tAt<int>(tMenu, 0x1648) = kMx; tAt<int>(tMenu, 0x164C) = kMy;
    tAt<int>(tMenu, 0x166C) = kNx; tAt<int>(tMenu, 0x1670) = kNy; tAt<int>(tMenu, 0x1674) = kNw; tAt<int>(tMenu, 0x1678) = kNh;
}

// ---------------------------------------------------------------- fake hud-host (k2.hud@1)
static K2HudCallback tCbBuild = nullptr, tCbPost = nullptr;
static int tPrioBuild = -1, tPrioPost = -1, tRegCalls = 0;
static int32_t KPATCH_CALL hudRegister(int32_t phase, int32_t prio, K2HudCallback fn, void*, const char*) {
    ++tRegCalls;
    if (phase == K2HUD_BUILD) { tCbBuild = fn; tPrioBuild = prio; }
    if (phase == K2HUD_TARGET_MENU_POST) { tCbPost = fn; tPrioPost = prio; }
    return tRegCalls;
}
static void KPATCH_CALL hudUnregister(int32_t) {}
static int32_t KPATCH_CALL hudHud(void) { return 0; }
static uint32_t KPATCH_CALL hudFrame(void) { return 0; }
static int32_t KPATCH_CALL hudStrip(const char*, int32_t, const K2HudRect*, int32_t, int32_t, K2HudRect*) { return 0; }
static const K2HudApi kHudTable = { sizeof(K2HudApi), hudRegister, hudUnregister, hudHud, hudFrame, hudStrip };

// ---------------------------------------------------------------- helpers over the call log
using trec::Rec;
using trec::Op;
static std::vector<Rec>& L() { return trec::log(); }
static int cnt(Op op) { int n = 0; for (const Rec& r : L()) if (r.op == op) ++n; return n; }
static int cntOn(Op op, uintptr_t c) { int n = 0; for (const Rec& r : L()) if (r.op == op && r.c == c) ++n; return n; }
static int callsOn(uintptr_t c) { int n = 0; for (const Rec& r : L()) if (r.c == c) ++n; return n; }
static const Rec* lastOn(Op op, uintptr_t c) { const Rec* p = nullptr; for (const Rec& r : L()) if (r.op == op && r.c == c) p = &r; return p; }
static bool allOk() { for (const Rec& r : L()) if (!r.ok) return false; return true; }
static bool rectEq(const Rec* r, const esl::RowRect& e) { return r && r->r[0] == e.x && r->r[1] == e.y && r->r[2] == e.w && r->r[3] == e.h; }
// Expected colours as literals (the documented look: white icon, black drain overlay, grey pill, teal item disc), NOT the mod's own constants.
static const float kRgbWhite[3] = { 1.0f, 1.0f, 1.0f };
static const float kRgbBlack[3] = { 0.0f, 0.0f, 0.0f };
static const float kRgbGrey[3] = { 0.5f, 0.5f, 0.5f };
static const float kRgbTeal[3] = { 0.035f, 0.415f, 0.34f };
static bool sameRgb(const Rec* r, const float* rgb) { return r && r->rgb[0] == rgb[0] && r->rgb[1] == rgb[1] && r->rgb[2] == rgb[2]; }
static int warnCount(const char* needle) {
    int n = 0;
    const std::string lvl = ":" + std::to_string(KPLOG_WARN) + ":";
    for (const std::string& s : fakekp::state().logLines) if (s.find(lvl) != std::string::npos && s.find(needle) != std::string::npos) ++n;
    return n;
}

// Control roles by creation order: [0] pill, then per slot k: disc 1+3k, icon 2+3k, overlay 3+3k.
static std::vector<uintptr_t> tCtl;
static uintptr_t pillC() { return tCtl[0]; }
static uintptr_t discC(int k) { return tCtl[1 + 3 * k]; }
static uintptr_t iconC(int k) { return tCtl[2 + 3 * k]; }
static uintptr_t overC(int k) { return tCtl[3 + 3 * k]; }

static uintptr_t tPanel = 0;          // the HUD panel of the current pool
static DWORD tFrameMs = 0;

static K2HudContext mkCtx(int phase) {
    K2HudContext c;
    memset(&c, 0, sizeof c);
    c.struct_size = sizeof(K2HudContext);
    c.phase = phase;
    c.targetType = -1;
    return c;
}
static void doBuild(uintptr_t panel, uint32_t structSize = sizeof(K2HudContext)) {
    K2HudContext c = mkCtx(K2HUD_BUILD);
    c.struct_size = structSize;
    c.hud = c.panel = static_cast<int32_t>(panel);
    tCbBuild(&c, nullptr);
}
// One POST frame (the clock advances by dtMs first). target 0 = none.
static K2HudContext postCtx(uintptr_t hud, const TxCre* target, int targetType = 1) {
    K2HudContext c = mkCtx(K2HUD_TARGET_MENU_POST);
    c.hud = static_cast<int32_t>(hud);
    c.menu = static_cast<int32_t>(tMenu);
    c.leader = 0;
    c.target = target ? static_cast<int32_t>(target->client) : 0;
    c.targetType = target ? targetType : -1;
    return c;
}
static void post(const K2HudContext& c, DWORD dtMs = 1) {
    tFrameMs += dtMs;
    g_stubTick = tFrameMs;
    tCbPost(&c, nullptr);
}
static void postT(const TxCre* target, DWORD dtMs = 1) { post(postCtx(tPanel, target), dtMs); }

static Config tBaseCfg;
// A clean HUD: baseline config, empty log, a new panel, one BUILD. Controls are read back from the Create records.
// The pre-0.4.3 "one click applies" behaviour: one pending step followed by Save. The staged edit / Reset / Defaults paths have their own scenario.
static int logCount(const char* needle);
// After a test pokes g_cfg directly: make the page's saved / pending copies follow it.
static void syncEdit() { g_saved = currentRaw(g_cfg.maxIcons); g_edit = g_saved; }
static void applySetting(int s, int dir) { editSetting(s, dir); saveEdits(); }

static void fresh(bool expectBuilt = true) {
    g_cfg = tBaseCfg;
    g_saved = currentRaw(g_cfg.maxIcons);
    g_edit = g_saved;
    g_nWarned = 0;
    g_stubIniWriteFails = false;
    stubIni().clear();
    fakekp::state().logLines.clear();
    setMenu(true);
    L().clear();
    tPanel = tAlloc(0x100);
    doBuild(tPanel);
    tCtl.clear();
    for (const Rec& r : L()) if (r.op == trec::O_Create && r.c) tCtl.push_back(r.c);
    if (expectBuilt) KT_CHECK_EQ(tCtl.size(), static_cast<size_t>(1 + 3 * PoolSlots));   // the pool is always built at full size (0.4.4)
    L().clear();
}

// ---------------------------------------------------------------- scenarios
static void scenario1_build() {
    fresh(false);   // (re-run BUILD below with the log kept)
    L().clear();
    const uintptr_t panel = tAlloc(0x100);
    doBuild(panel);
    const std::vector<Rec> l = L();
    std::vector<uintptr_t> ctl;
    for (const Rec& r : l) if (r.op == trec::O_Create) ctl.push_back(r.c);
    // 1 pill + PoolSlots (8) slots x (disc, icon, overlay) = 25 labels (the pool is always full size, 0.4.4), each exactly Create -> Vis(false) -> Click(false) -> Append, in that order.
    KT_CHECK_EQ(l.size(), static_cast<size_t>(100));
    bool seq = l.size() == 100;
    for (size_t i = 0; seq && i < 25; ++i) {
        const Rec& cr = l[4 * i]; const Rec& v = l[4 * i + 1]; const Rec& k = l[4 * i + 2]; const Rec& ap = l[4 * i + 3];
        seq = cr.op == trec::O_Create && cr.c && cr.panel == panel && !strcmp(cr.tag, "LBL_QUEUE1") &&
              v.op == trec::O_Vis && v.c == cr.c && !v.flag &&
              k.op == trec::O_Click && k.c == cr.c && !k.flag &&
              ap.op == trec::O_Append && ap.c == cr.c && ap.panel == panel;
    }
    KT_CHECK(seq);
    KT_CHECK(allOk());
    KT_CHECK_EQ(ctl.size(), static_cast<size_t>(25));
    // Nothing is positioned, filled or shown during BUILD.
    KT_CHECK_EQ(cnt(trec::O_Rect), 0);
    KT_CHECK_EQ(cnt(trec::O_Fill), 0);
    KT_CHECK_EQ(cnt(trec::O_Alpha), 0);
    KT_CHECK_EQ(cnt(trec::O_Tint), 0);
    // Label memory: hidden + click-through; the panel list holds the labels in creation order (draw order).
    bool hidden = true;
    for (uintptr_t c : ctl) hidden = hidden && !real_gk::isVisible(c) && (tAt<int>(c, real_gk::Ctl_Flags) & real_gk::FlagClickThrough);
    KT_CHECK(hidden);
    KT_CHECK_EQ(tAt<int>(panel, real_gk::Panel_CtlCount), 25);
    const uint32_t arr = tAt<uint32_t>(panel, real_gk::Panel_CtlArray);
    bool order = arr != 0;
    for (size_t i = 0; order && i < ctl.size(); ++i) order = tAt<uint32_t>(arr, static_cast<int>(i) * 4) == static_cast<uint32_t>(ctl[i]);
    KT_CHECK(order);
    KT_CHECK(g_built && g_nSw == PoolSlots && g_panel == static_cast<int32_t>(panel));
    KT_CHECK(g_pill.c == ctl[0] && g_sw[0].disc.c == ctl[1] && g_sw[0].icon.c == ctl[2] && g_sw[0].over.c == ctl[3] && g_sw[2].over.c == ctl[9] && g_sw[7].over.c == ctl[24]);
    // Registered through the fake hud-host with the documented priorities.
    KT_CHECK_EQ(tRegCalls, 2);
    KT_CHECK(tCbBuild == &onHudBuild && tCbPost == &onTargetMenuPost);
    KT_CHECK_EQ(tPrioBuild, 300);
    KT_CHECK_EQ(tPrioPost, 100);
    // A frame with no target right after BUILD: nothing was ever shown, so it makes no setter call.
    tPanel = panel;
    postT(nullptr);
    KT_CHECK_EQ(L().size(), l.size());   // unchanged: the POST added no record
}

static int findItemDebuffSpell() {
    for (int s = 0; s < kDebuffIconTableSize; ++s) if (kDebuffIcons[s].item && kDebuffIcons[s].resref) return s;
    return -1;
}

static void scenario2_hides_every_label() {
    // Show ALL four label kinds (pill, disc, icon, overlay) first, then lose the target: every one of them must get its own hide.
    const int itemSpell = findItemDebuffSpell();
    KT_CHECK(itemSpell >= 0);
    fresh();
    g_cfg.backing = true;
    tNowDay = 0; tNowMs = 1000;
    const TxCre t = makeCre();
    setEffects(t, { { 0x07, 0, itemSpell, -1, 10.0f, 1000 + 5000 } });   // item debuff (disc), 10 s long, half left (overlay)
    postT(&t);
    const uintptr_t all[4] = { pillC(), discC(0), iconC(0), overC(0) };
    for (uintptr_t c : all) KT_CHECK(real_gk::isVisible(c));
    L().clear();
    postT(nullptr);
    for (uintptr_t c : all) {
        KT_CHECK(!real_gk::isVisible(c));
        KT_CHECK(lastOn(trec::O_Vis, c) && !lastOn(trec::O_Vis, c)->flag);
        KT_CHECK_EQ(cntOn(trec::O_Vis, c), 1);
        KT_CHECK(tAt<int>(c, real_gk::Ctl_Flags) & real_gk::FlagClickThrough);
    }
    KT_CHECK_EQ(L().size(), static_cast<size_t>(8));   // exactly Vis(false) + Click(false) per label, nothing else
    L().clear();
    postT(nullptr);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
}

static void scenario2_no_target() {
    fresh();
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 0.f, 0 } });
    postT(nullptr);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));   // pool built hidden: nothing to hide
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    L().clear();
    postT(nullptr);   // target gone: exactly the shown label is hidden (+ click-through kept)
    KT_CHECK_EQ(L().size(), static_cast<size_t>(2));
    KT_CHECK(lastOn(trec::O_Vis, iconC(0)) && !lastOn(trec::O_Vis, iconC(0))->flag);
    KT_CHECK_EQ(callsOn(iconC(0)), 2);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    bool allHidden = true;
    for (uintptr_t c : tCtl) allHidden = allHidden && !real_gk::isVisible(c);
    KT_CHECK(allHidden);
    L().clear();
    postT(nullptr);   // and stays hidden without writes
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    postT(&t);        // back: only the visibility flips (rect / fill / tint / alpha caches survive the hide)
    KT_CHECK_EQ(L().size(), static_cast<size_t>(2));
    KT_CHECK(lastOn(trec::O_Vis, iconC(0)) && lastOn(trec::O_Vis, iconC(0))->flag);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    // Other ways of "no target": targetType -1 with a pointer, target type out of range, menu flag bit0 clear, then target type valid again.
    L().clear();
    post(postCtx(tPanel, &t, -1));
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    KT_CHECK_EQ(callsOn(iconC(0)), 2);
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    L().clear();
    post(postCtx(tPanel, &t, 4));
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    L().clear();
    setMenu(false);
    postT(&t);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    KT_CHECK_EQ(L().size(), static_cast<size_t>(2));
    setMenu(true);
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
}

static void scenario3_shows_fear() {
    fresh();
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 0.f, 0 } });   // Fear: SetState (type 8), state 8, spell 16 ("ip_fear")
    postT(&t);
    const esl::Layout lay = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 1, 0, g_cfg.gap, g_cfg.spacing, g_cfg.backingPad);
    KT_CHECK(lay.ok && lay.n == 1);
    // Anchor the expected rect with hand arithmetic as well: s = 24, x0 = 100 + 20 + 100 - 12, y = 400 + 60 - 4 - 24.
    KT_CHECK(lay.icon[0].x == 208 && lay.icon[0].y == 432 && lay.icon[0].w == 24 && lay.icon[0].h == 24);
    const uintptr_t ic = iconC(0);
    // Only the icon label of slot 0 is touched: fill (ip_fear), white tint, alpha 1, the layoutRow rect, shown.
    KT_CHECK_EQ(callsOn(ic), static_cast<int>(L().size()));
    KT_CHECK(allOk());
    const Rec* f = lastOn(trec::O_Fill, ic);
    KT_CHECK(f && f->img == tImgPtr("ip_fear"));
    KT_CHECK(sameRgb(lastOn(trec::O_Tint, ic), kRgbWhite));
    KT_CHECK(lastOn(trec::O_Alpha, ic) && lastOn(trec::O_Alpha, ic)->f == 1.0f);
    KT_CHECK(rectEq(lastOn(trec::O_Rect, ic), lay.icon[0]));
    KT_CHECK(lastOn(trec::O_Vis, ic) && lastOn(trec::O_Vis, ic)->flag);
    KT_CHECK(lastOn(trec::O_Click, ic) && !lastOn(trec::O_Click, ic)->flag);
    KT_CHECK_EQ(cntOn(trec::O_Rect, ic), 1);
    // Label memory agrees: visible, still click-through, border image = ip_fear's image, alpha 1.
    KT_CHECK(real_gk::isVisible(ic));
    KT_CHECK(tAt<int>(ic, real_gk::Ctl_Flags) & real_gk::FlagClickThrough);
    KT_CHECK(tAt<uint32_t>(ic + real_gk::Label_Border, real_gk::Border_Fill) == static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tImgPtr("ip_fear"))));
    float a = 0.f;
    KT_CHECK(real_gk::getAlpha(ic, &a) && a == 1.0f);
    // No other label was shown: pill, disc, overlay and slots 1-2 stay hidden.
    int shownCount = 0;
    for (uintptr_t c : tCtl) if (real_gk::isVisible(c)) ++shownCount;
    KT_CHECK_EQ(shownCount, 1);
    KT_CHECK(g_nSlots == 1 && g_tier == 3);
}

static void scenario3b_roles_disc_pill() {
    // Role proof for the BUILD order: control 0 is the pill, 1 the disc, 2 the icon, 3 the overlay. A debuff whose table row is an ITEM
    // ability draws the teal disc behind its icon; Backing=1 draws the pill.
    int itemSpell = -1;
    for (int s = 0; s < kDebuffIconTableSize && itemSpell < 0; ++s) if (kDebuffIcons[s].item && kDebuffIcons[s].resref) itemSpell = s;
    KT_CHECK(itemSpell >= 0);
    fresh();
    g_cfg.backing = true;
    g_cfg.backingAlpha = 0.5f;
    const TxCre t = makeCre();
    setEffects(t, { { 0x07, 0, itemSpell, -1, 0.f, 0 } });
    postT(&t);
    const esl::Layout lay = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 1, 0, g_cfg.gap, g_cfg.spacing, g_cfg.backingPad);
    const char* res = kDebuffIcons[itemSpell].resref;
    const Rec* df = lastOn(trec::O_Fill, discC(0));
    KT_CHECK(df && df->img == tImgPtr("buffhud_circle"));
    KT_CHECK(sameRgb(lastOn(trec::O_Tint, discC(0)), kRgbTeal));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, discC(0)), lay.icon[0]));
    KT_CHECK(real_gk::isVisible(discC(0)));
    const Rec* inf = lastOn(trec::O_Fill, iconC(0));
    KT_CHECK(inf && inf->img == tImgPtr(res));
    KT_CHECK(real_gk::isVisible(iconC(0)));
    const Rec* pf = lastOn(trec::O_Fill, pillC());
    KT_CHECK(pf && pf->img == tImgPtr("lbl_miscroll_f"));
    KT_CHECK(sameRgb(lastOn(trec::O_Tint, pillC()), kRgbGrey));
    KT_CHECK(lastOn(trec::O_Alpha, pillC()) && lastOn(trec::O_Alpha, pillC())->f == 0.5f);
    KT_CHECK(rectEq(lastOn(trec::O_Rect, pillC()), lay.pill));
    KT_CHECK(real_gk::isVisible(pillC()));
    KT_CHECK(!real_gk::isVisible(overC(0)));   // ratio unknown: no drain overlay
    KT_CHECK_EQ(callsOn(overC(0)), 0);
    // Draw order: pill (behind) is created before the disc, which is created before the icon.
    const uint32_t arr = tAt<uint32_t>(tPanel, real_gk::Panel_CtlArray);
    KT_CHECK(tAt<uint32_t>(arr, 0) == static_cast<uint32_t>(pillC()) && tAt<uint32_t>(arr, 4) == static_cast<uint32_t>(discC(0)) &&
             tAt<uint32_t>(arr, 8) == static_cast<uint32_t>(iconC(0)));
    // Backing off again: the pill is hidden with one Vis(false)/Click(false) pair, nothing else changes.
    L().clear();
    g_cfg.backing = false;
    postT(&t);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(2));
    KT_CHECK(lastOn(trec::O_Vis, pillC()) && !lastOn(trec::O_Vis, pillC())->flag);
}

static void scenario4_change_only() {
    fresh();
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 0.f, 0 } });
    postT(&t);
    KT_CHECK(L().size() > 0);
    const int srvBefore = tSrvCalls;
    L().clear();
    postT(&t, 1);   // inside the 250 ms refresh window
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    KT_CHECK_EQ(tSrvCalls, srvBefore);   // and no effect re-read either
    postT(&t, 300); // refresh due: the effect list IS re-read, the result is identical, still no setter call
    KT_CHECK(tSrvCalls > srvBefore);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    postT(&t, 300);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
}

static void scenario5_drain() {
    fresh();
    const TxCre t = makeCre();
    // 10 s duration, 7.5 s left at now = 1000 ms -> ratio 0.75 -> overlay height 24 - round(24 * 0.75) = 6.
    tNowDay = 0; tNowMs = 1000;
    setEffects(t, { { 0x08, 0, 16, 8, 10.0f, 1000 + 7500 } });
    postT(&t);
    const esl::Layout lay = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 1, 0, g_cfg.gap, g_cfg.spacing, g_cfg.backingPad);
    const uintptr_t ov = overC(0);
    const Rec* r1 = lastOn(trec::O_Rect, ov);
    KT_CHECK(r1 && r1->r[0] == lay.icon[0].x && r1->r[1] == lay.icon[0].y && r1->r[2] == lay.icon[0].w);
    KT_CHECK(r1 && r1->r[3] == 6);
    KT_CHECK(lastOn(trec::O_Fill, ov) && lastOn(trec::O_Fill, ov)->img == tImgPtr("uibit_fill_2bt"));
    KT_CHECK(sameRgb(lastOn(trec::O_Tint, ov), kRgbBlack));
    KT_CHECK(lastOn(trec::O_Alpha, ov) && lastOn(trec::O_Alpha, ov)->f > 0.8333f - 1e-4f && lastOn(trec::O_Alpha, ov)->f < 0.8333f + 1e-4f);   // 0.65 / 0.78
    KT_CHECK(real_gk::isVisible(ov));
    // The ratio drops to 0.25 (2.5 s left): ONLY the overlay rect changes (height 24 - 6 = 18); the icon is not touched.
    L().clear();
    const uintptr_t e0 = tAt<uint32_t>(tAt<uint32_t>(t.server, 0x148), 0);   // first (only) effect block
    tAt<int>(e0, 0x14) = 1000 + 2500;
    postT(&t, 300);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(1));
    const Rec* r2 = lastOn(trec::O_Rect, ov);
    KT_CHECK(r2 && r2->r[3] == 18 && r2->r[0] == lay.icon[0].x && r2->r[1] == lay.icon[0].y);
    KT_CHECK_EQ(callsOn(iconC(0)), 0);
    // Different ratios give different heights (a constant height would pass either single check above).
    KT_CHECK(r1->r[3] != r2->r[3]);
    // Duration over (ratio back to 1.0 = nothing drained): the overlay is hidden.
    L().clear();
    tAt<int>(e0, 0x14) = 1000 + 10000;
    postT(&t, 300);
    KT_CHECK(lastOn(trec::O_Vis, ov) && !lastOn(trec::O_Vis, ov)->flag);
    KT_CHECK(!real_gk::isVisible(ov));
    KT_CHECK_EQ(callsOn(iconC(0)), 0);
    // Below the full tier (Awareness tier 1, forced) there is no drain overlay at all, even with time left.
    fresh();
    g_cfg.forceTier = 1;
    setEffects(t, { { 0x08, 0, 16, 8, 10.0f, 1000 + 7500 } });
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    KT_CHECK_EQ(callsOn(overC(0)), 0);
    KT_CHECK(!real_gk::isVisible(overC(0)));
}

static void scenario6_stale_hud() {
    fresh();
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 0.f, 0 } });
    postT(&t);
    const uintptr_t ic = iconC(0);
    KT_CHECK(real_gk::isVisible(ic));
    // A different HUD object: the old labels may be freed, so NOT ONE write may go to them; all pointers are dropped.
    L().clear();
    const uintptr_t otherHud = tAlloc(0x100);
    post(postCtx(otherHud, &t));
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    KT_CHECK(real_gk::isVisible(ic));   // untouched
    KT_CHECK(!g_built && g_panel == 0 && g_nSw == 0 && g_pill.c == 0 && g_sw[0].icon.c == 0 && g_nSlots == 0 && g_lastTarget == 0);
    // Even the original hud pointer is now ignored (no pool): still no writes.
    postT(&t, 400);
    postT(nullptr, 400);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));

    // The just-fixed bug: Enabled=0 AND a stale HUD must not run hideAll on the old labels.
    fresh();
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    const uintptr_t ic2 = iconC(0);
    g_cfg.enabled = false;
    L().clear();
    post(postCtx(tAlloc(0x100), &t));
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    KT_CHECK(real_gk::isVisible(ic2));   // proof the old label really was visible and was not hidden by hideAll
    KT_CHECK(!g_built && g_nSw == 0 && g_pill.c == 0);
    // Contrast: with the same HUD (not stale) the disabled strip IS hidden (see scenario 11), so the zero above is the stale-guard.
    fresh();
    postT(&t);
    g_cfg.enabled = false;
    L().clear();
    postT(&t);
    KT_CHECK(lastOn(trec::O_Vis, iconC(0)) && !lastOn(trec::O_Vis, iconC(0))->flag);
}

static void scenario7_build_resets() {
    fresh();
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 0.f, 0 } });
    postT(&t);
    const std::vector<uintptr_t> oldCtl = tCtl;
    KT_CHECK(real_gk::isVisible(iconC(0)));
    const int srvBefore = tSrvCalls;
    // A new HUD layout loads: BUILD again with a NEW panel while the old pool is still referenced by the mod's state.
    L().clear();
    const uintptr_t panel2 = tAlloc(0x100);
    doBuild(panel2);
    std::vector<uintptr_t> newCtl;
    for (const Rec& r : L()) if (r.op == trec::O_Create) newCtl.push_back(r.c);
    KT_CHECK_EQ(newCtl.size(), static_cast<size_t>(25));
    bool touchedOld = false;
    for (const Rec& r : L()) for (uintptr_t o : oldCtl) if (r.c == o) touchedOld = true;
    KT_CHECK(!touchedOld);   // BUILD never writes to the previous pool's labels
    KT_CHECK(g_built && g_panel == static_cast<int32_t>(panel2) && g_nSlots == 0 && g_lastTarget == 0 && g_nImgs == 0);
    KT_CHECK(g_sw[0].icon.c == newCtl[2] && g_sw[0].icon.vis == 0 && !g_sw[0].icon.rectSet && !g_sw[0].icon.fillSet && !g_sw[0].icon.tintSet);
    // The same target at the SAME tick still draws into the new pool completely (no stale vis/rect/fill caches, refresh forced).
    L().clear();
    tPanel = panel2;
    tCtl = newCtl;
    post(postCtx(panel2, &t), 0);
    KT_CHECK(tSrvCalls > srvBefore);   // g_lastTarget was reset, so the effect list was re-read at once
    const uintptr_t ic = iconC(0);
    KT_CHECK_EQ(callsOn(ic), static_cast<int>(L().size()));
    KT_CHECK_EQ(cntOn(trec::O_Fill, ic), 1);
    KT_CHECK_EQ(cntOn(trec::O_Tint, ic), 1);
    KT_CHECK_EQ(cntOn(trec::O_Alpha, ic), 1);
    KT_CHECK_EQ(cntOn(trec::O_Rect, ic), 1);
    KT_CHECK(lastOn(trec::O_Vis, ic) && lastOn(trec::O_Vis, ic)->flag);
    KT_CHECK(real_gk::isVisible(ic));
    KT_CHECK(allOk());
    // The old pool still shows what it showed (the mod does not touch it any more).
    KT_CHECK(real_gk::isVisible(oldCtl[2]));
    // Disabled at BUILD (0.4.4): the full pool is still created (hidden), so enabling later works live; a frame draws nothing.
    fresh(false);
    g_cfg.enabled = false;
    syncEdit();
    L().clear();
    doBuild(tAlloc(0x100));
    KT_CHECK_EQ(cnt(trec::O_Create), 1 + 3 * PoolSlots);
    KT_CHECK(g_built);
}

static void scenario8_missing_image() {
    fresh();
    const TxCre t = makeCre();
    // Slot 0: Sleep (type 8, state 6, no spell row) -> fallback icon "ip_sleep" the engine cannot load. Slot 1: Hold (spell 29, "ip_hold").
    setEffects(t, { { 0x08, 0, -1, 6, 0.f, 0 }, { 0x08, 0, 29, 5, 0.f, 0 } });
    postT(&t);
    KT_CHECK(allOk());
    KT_CHECK_EQ(g_nSlots, 2);
    const esl::Layout lay = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 2, 0, g_cfg.gap, g_cfg.spacing, g_cfg.backingPad);
    // The missing icon's slot stays hidden with no write at all; the next slot still draws at ITS layout rect (slot index is kept).
    KT_CHECK_EQ(callsOn(discC(0)) + callsOn(iconC(0)) + callsOn(overC(0)), 0);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    KT_CHECK(lastOn(trec::O_Fill, iconC(1)) && lastOn(trec::O_Fill, iconC(1))->img == tImgPtr("ip_hold"));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, iconC(1)), lay.icon[1]));
    KT_CHECK(real_gk::isVisible(iconC(1)));
    // Exactly one warn, naming the resref, however many frames and refreshes follow.
    KT_CHECK_EQ(warnCount("image 'ip_sleep' not found"), 1);
    postT(&t, 1);
    postT(&t, 300);
    postT(&t, 300);
    KT_CHECK_EQ(warnCount("image 'ip_sleep' not found"), 1);
    // Only this resref warned (ip_hold and the overlay/disc textures all load).
    int allWarns = 0;
    const std::string lvl = ":" + std::to_string(KPLOG_WARN) + ":";
    for (const std::string& s : fakekp::state().logLines) if (s.find(lvl) != std::string::npos) ++allWarns;
    KT_CHECK_EQ(allWarns, 1);
    // After a BUILD (image cache reset) the warn is NOT repeated for the same resref (the warned list survives).
    const uintptr_t panel2 = tAlloc(0x100);
    doBuild(panel2);
    tPanel = panel2;
    tCtl.clear();
    for (const Rec& r : L()) if (r.op == trec::O_Create && r.c) tCtl.push_back(r.c);
    postT(&t);
    KT_CHECK_EQ(warnCount("image 'ip_sleep' not found"), 1);
}

static void scenario9_target_change() {
    fresh();
    const TxCre a = makeCre(), b = makeCre(), c = makeCre();
    setEffects(a, { { 0x08, 0, 16, 8, 0.f, 0 } });                                  // Fear
    setEffects(b, { { 0x08, 0, 29, 5, 0.f, 0 }, { 0x08, 0, 16, 8, 0.f, 0 } });      // Hold + Fear
    setEffects(c, {});                                                               // clean
    postT(&a);
    KT_CHECK(lastOn(trec::O_Fill, iconC(0)) && lastOn(trec::O_Fill, iconC(0))->img == tImgPtr("ip_fear"));
    // Target B one millisecond later (far inside the 250 ms refresh window): same frame, the row switches and re-centres for 2 icons.
    L().clear();
    postT(&b, 1);
    const esl::Layout l2 = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 2, 0, g_cfg.gap, g_cfg.spacing, g_cfg.backingPad);
    KT_CHECK(lastOn(trec::O_Fill, iconC(0)) && lastOn(trec::O_Fill, iconC(0))->img == tImgPtr("ip_hold"));
    KT_CHECK(lastOn(trec::O_Fill, iconC(1)) && lastOn(trec::O_Fill, iconC(1))->img == tImgPtr("ip_fear"));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, iconC(0)), l2.icon[0]));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, iconC(1)), l2.icon[1]));
    KT_CHECK(l2.icon[0].x != 208);   // 2 icons are centred differently from 1 (so the rect really moved)
    KT_CHECK(real_gk::isVisible(iconC(0)) && real_gk::isVisible(iconC(1)));
    // Back to A: slot 1 goes away within the same frame.
    L().clear();
    postT(&a, 1);
    KT_CHECK(lastOn(trec::O_Fill, iconC(0)) && lastOn(trec::O_Fill, iconC(0))->img == tImgPtr("ip_fear"));
    KT_CHECK(!real_gk::isVisible(iconC(1)));
    KT_CHECK(real_gk::isVisible(iconC(0)));
    // To a target without effects: the row hides at once.
    L().clear();
    postT(&c, 1);
    KT_CHECK(!real_gk::isVisible(iconC(0)) && !real_gk::isVisible(iconC(1)));
    // An unreadable client creature (no vtable) is no target: the row stays hidden, no crash.
    // It HAS a Fear effect on its server side: only the vtable guard in resolveServer keeps the row hidden.
    const TxCre bad = makeCre();
    setEffects(bad, { { 0x08, 0, 16, 8, 0.f, 0 } });
    postT(&a, 1);
    KT_CHECK(real_gk::isVisible(iconC(0)));   // the same effects on a healthy creature do show
    tAt<uint32_t>(bad.client, 0) = 0;
    postT(&bad, 1);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
}

static void scenario10_short_ctx() {
    fresh();
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 0.f, 0 } });
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    const int srvBefore = tSrvCalls;
    // struct_size one byte short of targetType's end: the mod cannot read the target, so a built pool is hidden and nothing else happens.
    L().clear();
    K2HudContext c = postCtx(tPanel, &t);
    c.struct_size = static_cast<uint32_t>(offsetof(K2HudContext, targetType) + sizeof(c.targetType) - 1);
    post(c, 400);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    KT_CHECK_EQ(L().size(), static_cast<size_t>(2));
    KT_CHECK_EQ(callsOn(iconC(0)), 2);
    KT_CHECK_EQ(tSrvCalls, srvBefore);   // no refresh on a short context
    L().clear();
    post(c, 400);   // repeated: already hidden, silent
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    // Exactly targetType's end is enough: it draws again.
    c.struct_size = static_cast<uint32_t>(offsetof(K2HudContext, targetType) + sizeof(c.targetType));
    post(c, 1);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    // A short context with NO pool built writes nothing.
    fresh(false);
    g_cfg.enabled = false;
    doBuild(tAlloc(0x100));
    L().clear();
    K2HudContext c2 = postCtx(tPanel, &t);
    c2.struct_size = 8;
    post(c2, 1);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    // BUILD with a short context or panel 0: no labels, one warn each, no half-built pool.
    fresh(false);
    L().clear();
    doBuild(tAlloc(0x100), static_cast<uint32_t>(offsetof(K2HudContext, panel)));
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    KT_CHECK(!g_built);
    KT_CHECK_EQ(warnCount("no panel in the context"), 1);
    doBuild(0);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    KT_CHECK(!g_built);
    KT_CHECK_EQ(warnCount("no panel in the context"), 2);
}

static void scenario11_enabled_and_live_setter() {
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 0.f, 0 } });
    // --- Enabled=0 live (config toggled directly): the shown row hides, then stays silent.
    fresh();
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    g_cfg.enabled = false;
    L().clear();
    postT(&t, 1);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    KT_CHECK_EQ(L().size(), static_cast<size_t>(2));
    L().clear();
    postT(&t, 300);
    postT(nullptr, 1);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));

    // --- the live setter (the mod-config page calls applySetting; here it is called directly, no mod-config needed)
    fresh();
    postT(&t);
    const esl::Layout base = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 1, 0, 4, g_cfg.spacing, g_cfg.backingPad);
    KT_CHECK(rectEq(lastOn(trec::O_Rect, iconC(0)), base.icon[0]));
    // Gap 4 -> 5: INI written, the row moves up one pixel on the next frame (only the rect call; everything else is cached).
    applySetting(esl::S_Gap, +1);
    KT_CHECK_EQ(g_cfg.gap, 5);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Gap", -1, "")), 5);
    L().clear();
    const int srv0 = tSrvCalls;
    postT(&t, 1);
    const esl::Layout gap5 = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 1, 0, 5, g_cfg.spacing, g_cfg.backingPad);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(1));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, iconC(0)), gap5.icon[0]));
    KT_CHECK_EQ(gap5.icon[0].y, base.icon[0].y - 1);
    KT_CHECK(tSrvCalls > srv0);   // the setter forced an immediate effect re-read (g_lastRefresh = 0)
    // IconSize Auto -> 16: INI written, icon edge 16, re-centred.
    applySetting(esl::S_IconSize, +1);
    KT_CHECK_EQ(g_cfg.iconSize, 16);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "IconSize", -1, "")), 16);
    L().clear();
    postT(&t, 1);
    const esl::Layout s16 = esl::layoutRow(kMx, kMy, kNx, kNy, kNw, kNh, 1, 16, 5, g_cfg.spacing, g_cfg.backingPad);
    KT_CHECK(s16.icon[0].w == 16 && s16.icon[0].x == 212);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(1));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, iconC(0)), s16.icon[0]));
    // Enabled toggled off through the setter: the row hides; toggled on again: it shows again (only visibility flips).
    applySetting(esl::S_Enabled, +1);
    KT_CHECK(!g_cfg.enabled);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Enabled", -1, "")), 0);
    L().clear();
    postT(&t, 1);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    KT_CHECK_EQ(L().size(), static_cast<size_t>(2));
    L().clear();
    postT(&t, 1);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    applySetting(esl::S_Enabled, +1);
    KT_CHECK(g_cfg.enabled);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Enabled", -1, "")), 1);
    L().clear();
    postT(&t, 1);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    KT_CHECK(lastOn(trec::O_Vis, iconC(0)) && lastOn(trec::O_Vis, iconC(0))->flag);
    KT_CHECK_EQ(cntOn(trec::O_Rect, iconC(0)), 0);   // the rect cache already holds the 16 px rect
    KT_CHECK_EQ(cntOn(trec::O_Fill, iconC(0)), 0);

    // --- MaxIcons is live too (0.4.4): the pool is always full size, so a Save caps the shown icons at once, with no restart.
    fresh();
    postT(&t);
    L().clear();
    applySetting(esl::S_MaxIcons, +1);
    KT_CHECK_EQ(g_cfg.maxIcons, 4);
    KT_CHECK_EQ(g_saved.maxIcons, 4);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "MaxIcons", -1, "")), 4);
    char rowTxt[64];
    rowText(esl::S_MaxIcons, rowTxt, static_cast<int>(sizeof rowTxt));
    KT_CHECK(strstr(rowTxt, "(restart)") == nullptr);

    // --- INI write failure (0.4.3): nothing is applied and the edit stays pending ("Save *"); a later Save with a working INI applies it.
    fresh();
    postT(&t);
    g_stubIniWriteFails = true;
    applySetting(esl::S_Gap, +1);
    KT_CHECK_EQ(g_cfg.gap, 4);
    KT_CHECK_EQ(g_saved.gap, 4);
    KT_CHECK_EQ(g_edit.gap, 5);
    KT_CHECK(esl::pageDiffers(g_saved, g_edit));
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Gap", -1, "")), -1);
    KT_CHECK_EQ(logCount("INI write FAILED for Gap=5"), 1);
    g_stubIniWriteFails = false;
    saveEdits();
    KT_CHECK_EQ(g_cfg.gap, 5);
    KT_CHECK(!esl::pageDiffers(g_saved, g_edit));
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Gap", -1, "")), 5);

    // --- built while disabled (0.4.4): enabling on the page works live; nothing was visible before, the row is shown after, no "(restart)".
    fresh();
    g_cfg.enabled = false;
    syncEdit();
    postT(&t);
    KT_CHECK(!real_gk::isVisible(iconC(0)));
    applySetting(esl::S_Enabled, +1);
    KT_CHECK(g_cfg.enabled);
    rowText(esl::S_Enabled, rowTxt, static_cast<int>(sizeof rowTxt));
    KT_CHECK(strstr(rowTxt, "(restart)") == nullptr);
    L().clear();
    postT(&t, 1);
    KT_CHECK(real_gk::isVisible(iconC(0)));
}

// ---------------------------------------------------------------- settings-page preview (0.4.1): fake k2.modconfig@1 / k2.menuui@1
// The provider is faked at the K2MenuUiApi table level (the mod's own callbacks are the real ones: cbBuild / cbEnter / cbTick / cbDestroy).
struct FakeMui {
    int newPage = 0, addButton = 0, addLabel = 0, addCustom = 0, setDesc = 0, splitCalls = 0, lastSplit = -1, previewPaneCalls = 0;
    int items = 0;                         // item ids handed out on the page (the real provider caps a page at 40)
    std::vector<void*> customCtl;
    std::vector<int> customFlags, customItem;
    int failAddCustomAt = -1;              // 0-based AddCustom call that answers -1
    int splitRet = 1;
    K2MuiRect preview = { 500, 100, 300, 220 }, column = { 20, 60, 260, 300 };
    bool previewOk = true;
    bool unlinked = true;                  // Control() answers nullptr for listed custom items (mod-config unlinked them)
    std::string desc;
    std::vector<int> clicks;               // item ids Clicked() answers 1 for, once each
    std::vector<std::pair<int, void*>> itemCtl;   // engine controls Control() answers for NON-custom items (the slider track / fill / marker)
    void reset() { *this = FakeMui(); }
} tm;
static K2ModConfigEntry tRegEntry;
static int tRegCount = 0;
static int32_t KPATCH_CALL mRegister(const K2ModConfigEntry* e) { ++tRegCount; if (e) tRegEntry = *e; return 9; }
static void KPATCH_CALL mUnregister(int32_t) {}
static uint32_t KPATCH_CALL mNewPage(void*, const char*) { ++tm.newPage; return 2; }
static int32_t KPATCH_CALL mAddButton(uint32_t, const char*, const char*) { ++tm.addButton; return tm.items >= 40 ? -1 : tm.items++; }
static int32_t KPATCH_CALL mAddLabel(uint32_t, const char*) { ++tm.addLabel; return tm.items >= 40 ? -1 : tm.items++; }
static int32_t KPATCH_CALL mAddCustom(uint32_t, void* c, int32_t f) {
    const int idx = tm.addCustom++;
    if (idx == tm.failAddCustomAt || tm.items >= 40) return -1;
    tm.customCtl.push_back(c); tm.customFlags.push_back(f); tm.customItem.push_back(tm.items);
    return tm.items++;
}
static int32_t KPATCH_CALL mClicked(uint32_t, int32_t item) {
    for (size_t i = 0; i < tm.clicks.size(); ++i) if (tm.clicks[i] == item) { tm.clicks.erase(tm.clicks.begin() + static_cast<long>(i)); return 1; }
    return 0;
}
static void KPATCH_CALL mSetText(uint32_t, int32_t, const char*) {}
static void KPATCH_CALL mSetRect(uint32_t, int32_t, const K2MuiRect*) {}
static void KPATCH_CALL mSetDesc(uint32_t, const char* t) { ++tm.setDesc; tm.desc = t ? t : ""; }
static int32_t KPATCH_CALL mPaneRect(void*, int32_t which, K2MuiRect* out) {
    if (which == MUI_PANE_COLUMN) { *out = tm.column; return 1; }
    if (which == MUI_PANE_PREVIEW) { ++tm.previewPaneCalls; if (!tm.previewOk) return 0; *out = tm.preview; return 1; }
    return 0;
}
static void* KPATCH_CALL mControl(uint32_t, int32_t item) {
    for (const auto& pr : tm.itemCtl) if (pr.first == item) return pr.second;
    for (size_t i = 0; i < tm.customItem.size(); ++i) if (tm.customItem[i] == item) return tm.unlinked ? nullptr : tm.customCtl[i];
    return nullptr;
}
static int32_t KPATCH_CALL mSetDescSplit(uint32_t, int32_t permille) { ++tm.splitCalls; tm.lastSplit = permille; return tm.splitRet; }
static K2ModConfigApi tCfgTable = { sizeof(K2ModConfigApi), mRegister, mUnregister };
static K2MenuUiApi tMui = { sizeof(K2MenuUiApi), mNewPage, mAddButton, mAddLabel, mAddCustom, mClicked, mSetText, mSetRect, mSetDesc, mPaneRect, mControl, mSetDescSplit, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

static int logCount(const char* needle) {
    int n = 0;
    for (const std::string& s : fakekp::state().logLines) if (s.find(needle) != std::string::npos) ++n;
    return n;
}
// What mod-config does on entry: show every listed page item (setVisible(true) also clears click-through).
static void modconfigShowAll() { for (void* c : tm.customCtl) real_gk::setVisible(reinterpret_cast<uintptr_t>(c), true); }
static bool clickThrough(uintptr_t c) { return (tAt<int>(c, real_gk::Ctl_Flags) & real_gk::FlagClickThrough) != 0; }
static bool vis(uintptr_t c) { return real_gk::isVisible(c); }
static uintptr_t pv(int i) { return g_pv.w[i].c; }

// A clean Options panel build with the fake provider connected: returns the page's panel. withSplit = false makes it an 11-slot (older) table.
static uintptr_t pageBuild(bool withSplit = true) {
    fresh();
    tm.reset();
    tDtors.clear();
    tAllocN = 0; tAllocFailAt = -1;
    tMui.struct_size = static_cast<uint32_t>(withSplit ? sizeof(K2MenuUiApi) : offsetof(K2MenuUiApi, SetDescSplit));
    tMui.SetDescSplit = withSplit ? mSetDescSplit : nullptr;
    g_mcMenu.p = &tMui;
    g_mcMenu.connected = true;
    g_pg.reset(); g_pv.reset();
    const uintptr_t panel = tAlloc(0x100);
    cbBuild(reinterpret_cast<void*>(panel), nullptr);
    return panel;
}
static int createsOf(const char* tag) { int n = 0; for (const Rec& r : L()) if (r.op == trec::O_Create && !strcmp(r.tag, tag)) ++n; return n; }

static void scenario12_page_preview() {
    // --- registration through the real tryRegisterPage over the fake registry (k2.modconfig + k2.menuui provided, first poll connects)
    fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &tCfgTable);
    fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &tMui);
    g_mcCfg.init(fakekp::api()); g_mcMenu.init(fakekp::api());
    g_mcRegistered = false;
    KT_CHECK(tryRegisterPage());
    KT_CHECK_EQ(tRegCount, 1);
    KT_CHECK(tRegEntry.build == cbBuild && tRegEntry.enter == cbEnter && tRegEntry.tick == cbTick && tRegEntry.destroy == cbDestroy && tRegEntry.leave == nullptr);

    // --- BUILD: rows first (25 items: 2 toggles, 4 number rows of 3, 2 slider rows of 4, Save/Reset/Defaults), then 12 preview labels from LBL_TITLE, listed MUI_NOFREE and NOT appended by the mod; then the split.
    const uintptr_t panel = pageBuild();
    KT_CHECK(g_pg.panel == reinterpret_cast<void*>(panel) && g_pg.page == 2);
    KT_CHECK_EQ(tm.addButton + tm.addLabel, 25);
    KT_CHECK_EQ(createsOf("LBL_TITLE"), 12);
    KT_CHECK_EQ(cnt(trec::O_Append), 0);
    KT_CHECK_EQ(tAt<int>(panel, real_gk::Panel_CtlCount), 0);
    KT_CHECK_EQ(tm.addCustom, 12);
    KT_CHECK_EQ(tm.items, 37);   // 25 rows + 12 preview <= the 40-item page cap
    bool flagsOk = tm.customFlags.size() == 12;
    for (int f : tm.customFlags) flagsOk = flagsOk && f == MUI_NOFREE;
    KT_CHECK(flagsOk);
    KT_CHECK_EQ(tm.splitCalls, 1);
    KT_CHECK_EQ(tm.lastSplit, 750);
    KT_CHECK(g_pv.on && g_pv.n == 12);
    KT_CHECK_EQ(logCount("preview BUILD ok"), 1);
    bool made = true, hid = true, style = true, listedSame = true;
    const int oStyle = k2addr::off(&K2AddrTable::Border_FillStyle);
    for (int i = 0; i < 12; ++i) {
        const uintptr_t c = pv(i);
        made = made && c != 0;
        hid = hid && !vis(c) && clickThrough(c);
        style = style && (tAt<uint32_t>(c + real_gk::Label_Border, oStyle) & 3u) == 2u;
        listedSame = listedSame && reinterpret_cast<uintptr_t>(tm.customCtl[static_cast<size_t>(i)]) == c;
    }
    KT_CHECK(made && hid && style && listedSame);
    KT_CHECK(lastOn(trec::O_Text, pv(0)) && !strcmp(lastOn(trec::O_Text, pv(0))->text, "Sith Assassin"));
    // The other 11 labels are clones of the Options title: their inherited text ("OPTIONS") must be blanked, or it draws over the icons.
    bool blank = true;
    for (int i = 1; i < 12; ++i) blank = blank && lastOn(trec::O_Text, pv(i)) && !strcmp(lastOn(trec::O_Text, pv(i))->text, "");
    KT_CHECK(blank);

    // --- ENTER: mod-config shows every item; the preview then draws the layout previewLayout gives (literals: pane 500,100,300,220, 3 icons).
    modconfigShowAll();
    L().clear();
    cbEnter(2, nullptr);
    KT_CHECK_EQ(tm.setDesc, 1);
    KT_CHECK(tm.desc.find("Example") != std::string::npos);
    KT_CHECK(vis(pv(0)) && rectEq(lastOn(trec::O_Rect, pv(0)), { 550, 212, 200, 24 }));
    KT_CHECK(sameRgb(lastOn(trec::O_Tint, pv(0)), kRgbBlack));
    KT_CHECK(lastOn(trec::O_Fill, pv(0)) && lastOn(trec::O_Fill, pv(0))->img == tImgPtr("uibit_fill_2bt"));
    KT_CHECK(!vis(pv(1)));   // backing is off: no pill
    const char* const rs[3] = { "ip_fear", "ip_stun", "ip_slow" };
    const int oh[3] = { 18, 12, 6 };   // overlay heights for ratios 0.25 / 0.5 / 0.75 on a 24 px icon
    for (int k = 0; k < 3; ++k) {
        const uintptr_t ic = pv(2 + 2 * k), ov = pv(3 + 2 * k);
        KT_CHECK(vis(ic) && rectEq(lastOn(trec::O_Rect, ic), { 610 + 28 * k, 184, 24, 24 }));
        KT_CHECK(lastOn(trec::O_Fill, ic) && lastOn(trec::O_Fill, ic)->img == tImgPtr(rs[k]));
        KT_CHECK(sameRgb(lastOn(trec::O_Tint, ic), kRgbWhite));
        KT_CHECK(vis(ov) && rectEq(lastOn(trec::O_Rect, ov), { 610 + 28 * k, 184, 24, oh[k] }));
        KT_CHECK(sameRgb(lastOn(trec::O_Tint, ov), kRgbBlack));
        KT_CHECK(lastOn(trec::O_Alpha, ov) && lastOn(trec::O_Alpha, ov)->f == esl::overlayAlpha(0.65f));
    }
    KT_CHECK(!vis(pv(8)) && !vis(pv(9)) && !vis(pv(10)) && !vis(pv(11)));   // icons 4 and 5 are beyond MaxIcons 3
    bool ct = true;
    for (int i = 0; i < 12; ++i) ct = ct && clickThrough(pv(i));
    KT_CHECK(ct);
    KT_CHECK(allOk());

    // --- an idle tick writes nothing but the (change-only) click-through keep-alive
    L().clear();
    cbTick(2, nullptr);
    bool onlyClick = true;
    for (const Rec& r : L()) onlyClick = onlyClick && r.op == trec::O_Click && !r.flag;
    KT_CHECK(onlyClick);
    KT_CHECK(tm.previewPaneCalls >= 2);

    // --- staged edit through the page: Gap 4 -> 5 (click on its '>'): the same tick redraws the example; the game's g_cfg is untouched until Save; bh 53 -> block y 183.
    tm.clicks.push_back(g_pg.next[esl::S_Gap]);
    L().clear();
    cbTick(2, nullptr);
    KT_CHECK_EQ(g_edit.gap, 5);
    KT_CHECK_EQ(g_cfg.gap, 4);
    KT_CHECK_EQ(g_pg.dirtyShown, 1);
    KT_CHECK_EQ(cntOn(trec::O_Rect, pv(0)), 0);   // the name bar does not move (still y 212): cached
    KT_CHECK(rectEq(lastOn(trec::O_Rect, pv(2)), { 610, 183, 24, 24 }) && rectEq(lastOn(trec::O_Rect, pv(6)), { 666, 183, 24, 24 }));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, pv(3)), { 610, 183, 24, 18 }));
    // DrainAlpha 0.65 -> 0.70 (a slider value): the overlays take the new alpha.
    g_edit = esl::setSliderValue(g_edit, esl::S_DrainAlpha, 70);
    L().clear();
    cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 0.70f && g_cfg.drainAlpha == 0.65f);
    KT_CHECK(lastOn(trec::O_Alpha, pv(3)) && lastOn(trec::O_Alpha, pv(3))->f == esl::overlayAlpha(0.70f));
    KT_CHECK(lastOn(trec::O_Alpha, pv(7)) && lastOn(trec::O_Alpha, pv(7))->f == esl::overlayAlpha(0.70f));
    // Backing on: pill (lbl_miscroll_f, grey, BackingAlpha) behind the row; pad 4 -> bh 57, block y 181, name y 214, row y 185, pill {606,181,88,32}.
    tm.clicks.push_back(g_pg.text[esl::S_Backing]);
    L().clear();
    cbTick(2, nullptr);
    KT_CHECK(g_edit.backing && !g_cfg.backing);
    KT_CHECK(vis(pv(1)) && rectEq(lastOn(trec::O_Rect, pv(1)), { 606, 181, 88, 32 }));
    KT_CHECK(lastOn(trec::O_Fill, pv(1)) && lastOn(trec::O_Fill, pv(1))->img == tImgPtr("lbl_miscroll_f"));
    KT_CHECK(sameRgb(lastOn(trec::O_Tint, pv(1)), kRgbGrey));
    KT_CHECK(lastOn(trec::O_Alpha, pv(1)) && lastOn(trec::O_Alpha, pv(1))->f == g_edit.backingAlpha);
    KT_CHECK(rectEq(lastOn(trec::O_Rect, pv(0)), { 550, 214, 200, 24 }) && rectEq(lastOn(trec::O_Rect, pv(2)), { 610, 185, 24, 24 }));
    // Enabled off: row, pill and overlays hide; the name bar stays, re-centred (by = 100 + (220-24)/2 = 198). On again: they come back.
    tm.clicks.push_back(g_pg.text[esl::S_Enabled]);
    L().clear();
    cbTick(2, nullptr);
    KT_CHECK(!g_edit.enabled && g_cfg.enabled);
    bool rowHidden = true;
    for (int i = 1; i < 12; ++i) rowHidden = rowHidden && !vis(pv(i));
    KT_CHECK(rowHidden);
    KT_CHECK(vis(pv(0)) && rectEq(lastOn(trec::O_Rect, pv(0)), { 550, 198, 200, 24 }));
    tm.clicks.push_back(g_pg.text[esl::S_Enabled]);
    cbTick(2, nullptr);
    KT_CHECK(g_edit.enabled && vis(pv(2)) && vis(pv(3)) && vis(pv(1)) && vis(pv(6)));
    // MaxIcons is restart-only for the GAME (g_cfg keeps the built pool size) but the example follows the pending value: 4 icons.
    tm.clicks.push_back(g_pg.next[esl::S_MaxIcons]);
    L().clear();
    cbTick(2, nullptr);
    KT_CHECK(g_edit.maxIcons == 4 && g_saved.maxIcons == 3 && g_cfg.maxIcons == 3 && vis(pv(8)) && !vis(pv(10)));

    // --- mod-config re-showing hidden preview items (page re-entry, or a late show): the next refresh hides them again, all click-through.
    modconfigShowAll();
    cbEnter(2, nullptr);   // also drops every unsaved edit (back to the saved values: Gap 4, no backing, MaxIcons 3)
    KT_CHECK(g_edit.gap == 4 && g_edit.maxIcons == 3 && !esl::pageDiffers(g_saved, g_edit));
    KT_CHECK(!vis(pv(8)) && !vis(pv(11)) && vis(pv(6)));
    ct = true;
    for (int i = 0; i < 12; ++i) ct = ct && clickThrough(pv(i));
    KT_CHECK(ct);
    real_gk::setVisible(pv(9), true);   // a late show, no enter
    cbTick(2, nullptr);
    KT_CHECK(!vis(pv(9)) && clickThrough(pv(9)));

    // --- the pane moves (resolution change): the next tick re-lays out; a failing PaneRect hides the preview with ONE warning, then it recovers.
    tm.preview = { 0, 0, 100, 200 };   // 100 wide: 3 icons of 24 + 2*4 = 80 fit; name bar narrows to 100
    cbTick(2, nullptr);
    KT_CHECK(rectEq(lastOn(trec::O_Rect, pv(0)), { 0, 102, 100, 24 }));   // saved values: bh = 24+4+24 = 52 (no backing), by = 74, name y = 74+52-24
    tm.previewOk = false;
    cbTick(2, nullptr);
    cbTick(2, nullptr);
    bool allHid = true;
    for (int i = 0; i < 12; ++i) allHid = allHid && !vis(pv(i));
    KT_CHECK(allHid);
    KT_CHECK_EQ(warnCount("no PREVIEW pane rect"), 1);
    tm.previewOk = true;
    cbTick(2, nullptr);
    KT_CHECK(vis(pv(0)) && vis(pv(2)));

    // --- DESTROY: mod-config unlinked the items (Control answers nullptr): the 12 labels are freed (slot-0 dtor, flags 1), then NO more writes.
    cbDestroy(reinterpret_cast<void*>(panel + 8), nullptr);   // another panel: ignored
    KT_CHECK(g_pv.n == 12 && tDtors.empty());
    cbDestroy(reinterpret_cast<void*>(panel), nullptr);
    KT_CHECK_EQ(tDtors.size(), static_cast<size_t>(12));
    bool dtorOk = tDtors.size() == 12;
    for (size_t i = 0; dtorOk && i < 12; ++i) dtorOk = tDtors[i].second == 1u && tDtors[i].first == reinterpret_cast<uintptr_t>(tm.customCtl[i]);
    KT_CHECK(dtorOk);
    KT_CHECK(g_pv.n == 0 && !g_pv.on && g_pg.panel == nullptr && g_pg.page == 0);
    KT_CHECK_EQ(logCount("preview DESTROY: freed=12 kept=0"), 1);
    L().clear();
    cbTick(2, nullptr);
    cbEnter(2, nullptr);
    applySetting(esl::S_Spacing, +1);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));

    // --- DESTROY with the unlink refused (Control still answers): the labels are leaked, never freed.
    const uintptr_t panel2 = pageBuild();
    tm.unlinked = false;
    cbDestroy(reinterpret_cast<void*>(panel2), nullptr);
    KT_CHECK(tDtors.empty());
    KT_CHECK_EQ(logCount("freed=0 kept=12"), 1);
    KT_CHECK_EQ(g_pv.n, 0);
}

static void scenario12b_older_provider() {
    // A provider without SetDescSplit (11-slot table): no label is created, no split call, one log line; rows and clicks still work.
    const uintptr_t panel = pageBuild(false);
    KT_CHECK(g_pg.panel == reinterpret_cast<void*>(panel));
    KT_CHECK_EQ(createsOf("LBL_TITLE"), 0);
    KT_CHECK_EQ(tm.addCustom, 0);
    KT_CHECK_EQ(tm.splitCalls, 0);
    KT_CHECK_EQ(g_pv.n, 0);
    KT_CHECK_EQ(logCount("preview skipped: provider has no SetDescSplit"), 1);
    KT_CHECK_EQ(tm.addButton + tm.addLabel, 25);
    L().clear();
    cbEnter(2, nullptr);
    KT_CHECK(tm.desc.find("Example") == std::string::npos && !tm.desc.empty());
    KT_CHECK_EQ(tm.previewPaneCalls, 0);
    tm.clicks.push_back(g_pg.next[esl::S_Gap]);
    cbTick(2, nullptr);
    KT_CHECK_EQ(g_edit.gap, 5);
    KT_CHECK_EQ(g_cfg.gap, 4);
    tm.clicks.push_back(g_pg.save);
    L().clear();
    cbTick(2, nullptr);
    KT_CHECK_EQ(g_cfg.gap, 5);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Gap", -1, "")), 5);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    cbDestroy(reinterpret_cast<void*>(panel), nullptr);
    KT_CHECK(tDtors.empty() && g_pg.panel == nullptr);
    // A newer-size table whose SetDescSplit is null behaves the same.
    tMui.struct_size = sizeof(K2MenuUiApi);
    fresh(); tm.reset(); tMui.SetDescSplit = nullptr; g_mcMenu.p = &tMui; g_mcMenu.connected = true; g_pg.reset(); g_pv.reset();
    cbBuild(reinterpret_cast<void*>(tAlloc(0x100)), nullptr);
    KT_CHECK(g_pv.n == 0 && createsOf("LBL_TITLE") == 0 && tm.splitCalls == 0);
    tMui.SetDescSplit = mSetDescSplit;
}

static void scenario12c_split_refused_and_failures() {
    // SetDescSplit refuses: the labels are already listed, so they stay hidden every tick, no PREVIEW pane query, one warning.
    pageBuild();   // clean baseline first
    fresh(); tm.reset(); tDtors.clear(); tm.splitRet = 0;
    g_pg.reset(); g_pv.reset();
    const uintptr_t panel = tAlloc(0x100);
    cbBuild(reinterpret_cast<void*>(panel), nullptr);
    KT_CHECK(g_pv.n == 12 && !g_pv.on);
    KT_CHECK_EQ(warnCount("preview skipped: SetDescSplit"), 1);
    modconfigShowAll();
    cbEnter(2, nullptr);
    cbTick(2, nullptr);
    bool hid = true;
    for (int i = 0; i < 12; ++i) hid = hid && !vis(pv(i)) && clickThrough(pv(i));
    KT_CHECK(hid);
    KT_CHECK_EQ(tm.previewPaneCalls, 0);
    KT_CHECK(tm.desc.find("Example") == std::string::npos);
    cbDestroy(reinterpret_cast<void*>(panel), nullptr);
    KT_CHECK_EQ(tDtors.size(), static_cast<size_t>(12));

    // AddCustom fails on the 5th label: the 4 listed stay hidden; the 8 never listed are released at once; the split is never requested.
    fresh(); tm.reset(); tDtors.clear(); tm.failAddCustomAt = 4;
    g_pg.reset(); g_pv.reset();
    const uintptr_t panel2 = tAlloc(0x100);
    L().clear();
    cbBuild(reinterpret_cast<void*>(panel2), nullptr);
    KT_CHECK(g_pv.n == 4 && !g_pv.on && tm.splitCalls == 0);
    KT_CHECK_EQ(tDtors.size(), static_cast<size_t>(8));
    KT_CHECK_EQ(warnCount("preview skipped: AddCustom listed 4/12"), 1);
    modconfigShowAll();
    cbEnter(2, nullptr);
    hid = true;
    for (int i = 0; i < 4; ++i) hid = hid && !vis(pv(i));
    KT_CHECK(hid);
    cbDestroy(reinterpret_cast<void*>(panel2), nullptr);
    KT_CHECK_EQ(tDtors.size(), static_cast<size_t>(12));   // 8 + the 4 that were listed

    // A label allocation fails on the 7th label: the 6 created are released, nothing is listed, no split, the rows are intact.
    fresh(); tm.reset(); tDtors.clear();
    g_pg.reset(); g_pv.reset();
    tAllocN = 0; tAllocFailAt = 6;
    const uintptr_t panel3 = tAlloc(0x100);
    cbBuild(reinterpret_cast<void*>(panel3), nullptr);
    tAllocFailAt = -1;
    KT_CHECK(g_pv.n == 0 && tm.addCustom == 0 && tm.splitCalls == 0);
    KT_CHECK_EQ(tDtors.size(), static_cast<size_t>(6));
    KT_CHECK_EQ(warnCount("preview: "), 1);
    KT_CHECK_EQ(tm.addButton + tm.addLabel, 25);
    KT_CHECK(g_pg.panel == reinterpret_cast<void*>(panel3));
    L().clear();
    cbEnter(2, nullptr);
    cbTick(2, nullptr);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
}

static void scenario13_drain_off_default() {
    // HUD: drainAlpha 0 (the default) at forced tier 3, ratio 0.3: the icon shows, the overlay is never touched (no setter call at all).
    fresh();
    g_cfg.drainAlpha = 0.0f;
    tNowDay = 0; tNowMs = 1000;
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, 16, 8, 10.0f, 1000 + 3000 } });   // 3 s left of 10 s: ratio 0.3
    postT(&t);
    KT_CHECK(real_gk::isVisible(iconC(0)));
    KT_CHECK_EQ(callsOn(overC(0)), 0);
    postT(&t, 300);
    postT(&t, 300);
    KT_CHECK_EQ(callsOn(overC(0)), 0);
    KT_CHECK(!real_gk::isVisible(overC(0)));
    // 0.65: the overlay appears as before (height 24 - int(24*0.3+0.5) = 17), tinted black, alpha 0.65/0.78.
    g_cfg.drainAlpha = 0.65f;
    L().clear();
    postT(&t, 1);
    KT_CHECK(real_gk::isVisible(overC(0)));
    const Rec* r = lastOn(trec::O_Rect, overC(0));
    KT_CHECK(r && r->r[3] == 17);
    KT_CHECK(lastOn(trec::O_Alpha, overC(0)) && lastOn(trec::O_Alpha, overC(0))->f == esl::overlayAlpha(0.65f));
    // Back to 0: the overlay hides again (visibility flip only), and further frames are silent.
    g_cfg.drainAlpha = 0.0f;
    L().clear();
    postT(&t, 1);
    KT_CHECK(!real_gk::isVisible(overC(0)));
    KT_CHECK_EQ(callsOn(overC(0)), 2);   // Vis(false) + Click(false)
    L().clear();
    postT(&t, 1);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    // Through the live setter: 0 -> 0.05 shows the overlay with alpha 0.05/0.78; stepping back down to 0 hides it.
    syncEdit();
    applySetting(esl::S_DrainAlpha, +1);
    KT_CHECK(g_cfg.drainAlpha == 0.05f);
    postT(&t, 1);
    KT_CHECK(real_gk::isVisible(overC(0)));
    KT_CHECK(lastOn(trec::O_Alpha, overC(0)) && lastOn(trec::O_Alpha, overC(0))->f == esl::overlayAlpha(0.05f));
    applySetting(esl::S_DrainAlpha, -1);
    KT_CHECK(g_cfg.drainAlpha == 0.0f);
    postT(&t, 1);
    KT_CHECK(!real_gk::isVisible(overC(0)));
}

static void scenario13b_preview_drain_off() {
    // Settings-page preview with DrainAlpha 0: icons show, no overlay control is shown or written; the row's '>' brings the shading back.
    pageBuild();
    g_cfg.drainAlpha = 0.0f;
    syncEdit();
    modconfigShowAll();
    L().clear();
    cbEnter(2, nullptr);
    bool overOff = true, overQuiet = true;
    for (int k = 0; k < 3; ++k) {
        KT_CHECK(vis(pv(2 + 2 * k)));
        overOff = overOff && !vis(pv(3 + 2 * k));
    }
    for (const Rec& r : L()) for (int k = 0; k < 5; ++k) if (r.c == pv(3 + 2 * k) && r.op != trec::O_Click && r.op != trec::O_Vis) overQuiet = false;
    KT_CHECK(overOff);
    KT_CHECK(overQuiet);   // hidden overlays only get the hide/click-through calls, never a rect, fill, tint or alpha
    for (int k = 0; k < 5; ++k) KT_CHECK(!lastOn(trec::O_Alpha, pv(3 + 2 * k)) && !lastOn(trec::O_Rect, pv(3 + 2 * k)));
    g_edit = esl::setSliderValue(g_edit, esl::S_DrainAlpha, 5);   // the slider's value
    cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 0.05f && g_cfg.drainAlpha == 0.0f);
    KT_CHECK(vis(pv(3)) && vis(pv(5)) && vis(pv(7)));
    KT_CHECK(lastOn(trec::O_Alpha, pv(3)) && lastOn(trec::O_Alpha, pv(3))->f == esl::overlayAlpha(0.05f));
    g_edit = esl::setSliderValue(g_edit, esl::S_DrainAlpha, 0);
    cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 0.0f && !vis(pv(3)) && !vis(pv(5)) && !vis(pv(7)));
    cbDestroy(reinterpret_cast<void*>(g_pg.panel), nullptr);
}

// ---------------------------------------------------------------- staged edits, Reset / Defaults / Save, alpha sliders (0.4.3), live MaxIcons (0.4.4)
static int tCurX = 0, tCurY = 0;
static bool tMouseDown = false;
static bool tCursorClient(int* cx, int* cy, int* cw, int* ch) { *cx = tCurX; *cy = tCurY; *cw = 3840; *ch = 1600; return true; }
static bool tForeground() { return true; }
static bool tLeftDown() { return tMouseDown; }

static uintptr_t mkItemCtl(int item, int x, int y, int w, int h) {
    int why = 0; uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(tPanel, "LBL_TITLE", &why, &bv);
    real_gk::setRect(c, x, y, w, h);
    real_gk::setVisible(c, true);
    // The host engine fake does not store the extent the way the real SetRect does: write the four fields hit() reads.
    tAt<int>(c, k2addr::off(&K2AddrTable::Ctl_X)) = x; tAt<int>(c, k2addr::off(&K2AddrTable::Ctl_Y)) = y;
    tAt<int>(c, k2addr::off(&K2AddrTable::Ctl_W)) = w; tAt<int>(c, k2addr::off(&K2AddrTable::Ctl_H)) = h;
    tm.itemCtl.push_back({ item, reinterpret_cast<void*>(c) });
    return c;
}

static void scenario14_staged_edits_and_sliders() {
    // --- Reset / Defaults / Save: edits are pending until Save; Save writes only the changed keys and applies them (MaxIcons included).
    pageBuild();
    cbEnter(2, nullptr);
    KT_CHECK(g_cfg.maxIcons == 3 && g_cfg.drainAlpha == 0.65f && g_cfg.gap == 4 && !esl::pageDiffers(g_saved, g_edit));
    tm.clicks.push_back(g_pg.next[esl::S_Gap]);
    cbTick(2, nullptr);
    KT_CHECK(g_edit.gap == 5 && g_cfg.gap == 4 && g_pg.dirtyShown == 1);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Gap", -1, "")), -1);   // nothing written yet
    tm.clicks.push_back(g_pg.rst);
    cbTick(2, nullptr);
    KT_CHECK(g_edit.gap == 4 && g_pg.dirtyShown == 0 && !esl::pageDiffers(g_saved, g_edit));
    tm.clicks.push_back(g_pg.next[esl::S_Gap]);
    cbTick(2, nullptr);
    tm.clicks.push_back(g_pg.defs);
    cbTick(2, nullptr);   // Defaults: every page setting at its default (gap 4 is the default, so the earlier step is gone too)
    KT_CHECK(g_edit.gap == 4 && g_edit.maxIcons == 5 && g_edit.drainAlpha == 0.0f);
    KT_CHECK(g_cfg.maxIcons == 3 && g_cfg.drainAlpha == 0.65f && g_pg.dirtyShown == 1);   // still only pending
    tm.clicks.push_back(g_pg.save);
    cbTick(2, nullptr);
    KT_CHECK(g_cfg.maxIcons == 5 && g_cfg.drainAlpha == 0.0f && g_pg.dirtyShown == 0);   // applied live, MaxIcons too
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "MaxIcons", -1, "")), 5);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Gap", -1, "")), -1);   // unchanged keys are not written
    KT_CHECK_EQ(logCount("cfg: SAVE wrote="), 1);
    tm.clicks.push_back(g_pg.save);
    cbTick(2, nullptr);   // nothing pending: Save writes nothing
    KT_CHECK_EQ(logCount("cfg: SAVE wrote=0 failed=0"), 1);
    // Leaving and re-entering the page drops unsaved edits.
    tm.clicks.push_back(g_pg.next[esl::S_Spacing]);
    cbTick(2, nullptr);
    KT_CHECK(g_edit.spacing == 5 && g_pg.dirtyShown == 1);
    cbEnter(2, nullptr);
    KT_CHECK(g_edit.spacing == 4 && g_pg.dirtyShown == 0);

    // --- MaxIcons applies live to the HUD row: 4 obvious effects, cap 3 -> 3 slots; Save with MaxIcons 4 -> 4 slots on the next refresh, no new labels.
    fresh();
    tNowDay = 0; tNowMs = 1000;
    const TxCre t = makeCre();
    setEffects(t, { { 0x08, 0, -1, 4, 0.f, 0 }, { 0x08, 0, -1, 5, 0.f, 0 }, { 0x08, 0, -1, 6, 0.f, 0 }, { 0x08, 0, 16, 8, 0.f, 0 } });
    postT(&t);
    KT_CHECK_EQ(g_nSlots, 3);
    applySetting(esl::S_MaxIcons, +1);
    L().clear();
    postT(&t, 1);
    KT_CHECK_EQ(g_nSlots, 4);
    KT_CHECK_EQ(cnt(trec::O_Create), 0);

    // --- alpha sliders: a press edge over the track starts a drag, the cursor x sets the value on the 0.05 grid, release ends it.
    pageBuild();
    guikit::g_gk.cursorClient = tCursorClient; guikit::g_gk.gameForeground = tForeground; guikit::g_gk.lbuttonDown = tLeftDown;
    const uintptr_t savedSw = guikit::g_gk.screenWAddr, savedSh = guikit::g_gk.screenHAddr;
    guikit::g_gk.screenWAddr = 0; guikit::g_gk.screenHAddr = 0;   // unknown screen size: cursorEngine falls back to client px (1:1)
    const int dr = esl::S_DrainAlpha;
    // Layout of cbEnter on the fake column {20,60,260,300}: slot h 33, pitch 38, bw 52, lw 144, gap 6 -> row 7 track {170, 326, 110, 33}, inner {176, 339, 98, 7}.
    const uintptr_t track = mkItemCtl(g_pg.prev[dr], 170, 326, 110, 33);
    const uintptr_t fillC = mkItemCtl(g_pg.fill[dr], 0, 0, 1, 1);
    const uintptr_t thumbC = mkItemCtl(g_pg.thumb[dr], 0, 0, 1, 1);
    (void)track;
    cbEnter(2, nullptr);
    KT_CHECK(g_pg.track[dr].x == 170 && g_pg.track[dr].y == 326 && g_pg.track[dr].w == 110 && g_pg.track[dr].h == 33);
    KT_CHECK(g_edit.drainAlpha == 0.65f);
    // 0.65 of the inner 98 px = 64 px of fill (int(0.65*98+0.5) = 64); marker 6 px wide centred on its end: x 176+64-3 = 237, 18 px tall at y 333.
    KT_CHECK(rectEq(lastOn(trec::O_Rect, fillC), { 176, 339, 64, 7 }));
    KT_CHECK(rectEq(lastOn(trec::O_Rect, thumbC), { 237, 333, 6, 18 }));
    KT_CHECK(clickThrough(fillC) && clickThrough(thumbC));
    // Not pressed: moving the cursor over the track does nothing.
    tCurX = 225; tCurY = 340; tMouseDown = false;
    cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 0.65f && g_pg.drag == -1);
    // Press on the track at x 225 (49 px into the 98 px inner span = 0.50), still unsaved.
    tMouseDown = true;
    cbTick(2, nullptr);
    KT_CHECK(g_pg.drag == dr && g_edit.drainAlpha == 0.50f && g_cfg.drainAlpha == 0.65f);
    KT_CHECK(rectEq(lastOn(trec::O_Rect, fillC), { 176, 339, 49, 7 }));
    // Drag past both ends: clamped 1.00 / 0.00; 0.05 grid in between (x 190 = 14 px = 14.3 % -> 0.15).
    tCurX = 400; cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 1.0f);
    tCurX = 190; cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 0.15f);
    tCurX = 100; cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 0.0f);
    // Release ends the drag; moving afterwards does nothing.
    tMouseDown = false; cbTick(2, nullptr);
    KT_CHECK(g_pg.drag == -1);
    tCurX = 250; cbTick(2, nullptr);
    KT_CHECK(g_edit.drainAlpha == 0.0f);
    // A press OUTSIDE every track (the Gap row) starts nothing, and dragging into a track from outside does nothing.
    tCurX = 250; tCurY = 100; tMouseDown = true;
    cbTick(2, nullptr);
    tCurY = 340; cbTick(2, nullptr);
    KT_CHECK(g_pg.drag == -1 && g_edit.drainAlpha == 0.0f);
    tMouseDown = false; cbTick(2, nullptr);
    // The drag is still only pending: Save applies it.
    g_edit = esl::setSliderValue(g_edit, dr, 35);
    tm.clicks.push_back(g_pg.save);
    cbTick(2, nullptr);
    KT_CHECK(g_cfg.drainAlpha == 0.35f);
    cbDestroy(reinterpret_cast<void*>(g_pg.panel), nullptr);
    guikit::g_gk.cursorClient = nullptr; guikit::g_gk.gameForeground = nullptr; guikit::g_gk.lbuttonDown = nullptr;
    guikit::g_gk.screenWAddr = savedSw; guikit::g_gk.screenHAddr = savedSh;
}

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    void* m = mmap(nullptr, kArenaSize, PROT_READ | PROT_WRITE, flags, -1, 0);
    if (m == MAP_FAILED) { perror("mmap arena"); return 5; }
    tArena = static_cast<uint8_t*>(m);
    // The control vtable page (slot 1 = SetRect must read back non-zero for guikit::setRect).
    void* vp = mmap(reinterpret_cast<void*>(0x00987000), 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (vp == MAP_FAILED) { perror("mmap 0x987000"); return 5; }
    tAt<uint32_t>(real_gk::LabelVtable, k2addr::off(&K2AddrTable::Ctl_VtSetRect) ? k2addr::off(&K2AddrTable::Ctl_VtSetRect) : 4) = 0x00402000u;

    g_stubVirtualQueryAll = true;
    k2addr::setForTest(&kSteam);
    saferead::beginScope();

    // Engine seams.
    real_gk::Engine& e = real_gk::g_gk;
    real_gk::g_gkCallLabelDtor = fxLabelDtor;
    tAt<uint32_t>(real_gk::LabelVtable, 0) = 0x00402100u;   // vtable slot 0 (scalar deleting dtor) must read back non-zero
    e.alloc = fxAlloc; e.labelCtor = fxLabelCtor; e.initControl = fxInit; e.exoCtor = fxExoCtor; e.exoDtor = fxExoDtor;
    e.listAdd = fxListAdd; e.loadImage = fxLoadImage; e.callSetRect = fxSetRect;
    party::g_serverCreature = fxServerCreature;
    party::g_appGlobal = 0;   // no party table (only the "own debuffs" tier would walk it)
    // World time chain for k2fx::currentTime: appGlobal cell -> g -> +8 -> a -> +4 -> b -> +0x10048 = timer (non-zero).
    const uintptr_t agCell = tAlloc(16), g = tAlloc(0x40), a = tAlloc(0x40), b = tAlloc(0x10100);
    tAt<uint32_t>(agCell, 0) = static_cast<uint32_t>(g);
    tAt<uint32_t>(g, 8) = static_cast<uint32_t>(a);
    tAt<uint32_t>(a, 4) = static_cast<uint32_t>(b);
    tAt<uint32_t>(b, 0x10048) = 0x1234;
    k2fx::g_fx.appGlobal = agCell;
    k2fx::g_fx.getTime = fxGetTime;
    tMenu = tAlloc(0x1C00);
    setMenu(true);

    // INI the real loadIni reads: a 3-label pool, forced Awareness tier 3 (everything visible, drain on).
    stubIniPut("Strip", "MaxIcons", "3");
    stubIniPut("Strip", "ForceTier", "3");
    fakekp::reset();
    fakekp::withLog();
    fakekp::provide(K2HUD_IFACE, K2HUD_VERSION, &kHudTable);
    KPatchInit(fakekp::api());   // the real export: KpLog_Init, loadIni, registers BUILD(300) and TARGET_MENU_POST(100) on the fake hud-host
    KT_CHECK(tCbBuild && tCbPost);
    if (!tCbBuild || !tCbPost) return kt::Report("enemy_strip_draw_test");
    KT_CHECK(g_cfg.enabled && g_cfg.maxIcons == 3 && g_cfg.forceTier == 3 && g_cfg.gap == 4 && g_cfg.spacing == 4 && !g_cfg.backing);
    KT_CHECK(g_cfg.drainAlpha == 0.0f);   // the shipped default: no drain shading
    tBaseCfg = g_cfg;
    tBaseCfg.drainAlpha = 0.65f;          // the drain scenarios below run with the shading ON; scenario13 covers the default

    scenario1_build();
    scenario2_hides_every_label();
    scenario2_no_target();
    scenario3_shows_fear();
    scenario3b_roles_disc_pill();
    scenario4_change_only();
    scenario5_drain();
    scenario6_stale_hud();
    scenario7_build_resets();
    scenario8_missing_image();
    scenario9_target_change();
    scenario10_short_ctx();
    scenario11_enabled_and_live_setter();
    scenario12_page_preview();
    scenario12b_older_provider();
    scenario12c_split_refused_and_failures();
    scenario13_drain_off_default();
    scenario13b_preview_drain_off();
    scenario14_staged_edits_and_sliders();
    return kt::Report("enemy_strip_draw_test");
}
