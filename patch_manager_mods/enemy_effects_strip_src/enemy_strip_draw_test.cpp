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
enum Op { O_Create, O_Append, O_Vis, O_Click, O_Rect, O_Fill, O_Alpha, O_Tint };
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
static void* fxAlloc(unsigned n) { return reinterpret_cast<void*>(tAlloc(n)); }
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
static void fresh(bool expectBuilt = true) {
    g_cfg = tBaseCfg;
    g_pendMax = g_cfg.maxIcons;
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
    if (expectBuilt) KT_CHECK_EQ(tCtl.size(), static_cast<size_t>(1 + 3 * g_cfg.maxIcons));
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
    // 1 pill + 3 slots x (disc, icon, overlay) = 10 labels, each exactly Create -> Vis(false) -> Click(false) -> Append, in that order.
    KT_CHECK_EQ(l.size(), static_cast<size_t>(40));
    bool seq = l.size() == 40;
    for (size_t i = 0; seq && i < 10; ++i) {
        const Rec& cr = l[4 * i]; const Rec& v = l[4 * i + 1]; const Rec& k = l[4 * i + 2]; const Rec& ap = l[4 * i + 3];
        seq = cr.op == trec::O_Create && cr.c && cr.panel == panel && !strcmp(cr.tag, "LBL_QUEUE1") &&
              v.op == trec::O_Vis && v.c == cr.c && !v.flag &&
              k.op == trec::O_Click && k.c == cr.c && !k.flag &&
              ap.op == trec::O_Append && ap.c == cr.c && ap.panel == panel;
    }
    KT_CHECK(seq);
    KT_CHECK(allOk());
    KT_CHECK_EQ(ctl.size(), static_cast<size_t>(10));
    // Nothing is positioned, filled or shown during BUILD.
    KT_CHECK_EQ(cnt(trec::O_Rect), 0);
    KT_CHECK_EQ(cnt(trec::O_Fill), 0);
    KT_CHECK_EQ(cnt(trec::O_Alpha), 0);
    KT_CHECK_EQ(cnt(trec::O_Tint), 0);
    // Label memory: hidden + click-through; the panel list holds the labels in creation order (draw order).
    bool hidden = true;
    for (uintptr_t c : ctl) hidden = hidden && !real_gk::isVisible(c) && (tAt<int>(c, real_gk::Ctl_Flags) & real_gk::FlagClickThrough);
    KT_CHECK(hidden);
    KT_CHECK_EQ(tAt<int>(panel, real_gk::Panel_CtlCount), 10);
    const uint32_t arr = tAt<uint32_t>(panel, real_gk::Panel_CtlArray);
    bool order = arr != 0;
    for (size_t i = 0; order && i < ctl.size(); ++i) order = tAt<uint32_t>(arr, static_cast<int>(i) * 4) == static_cast<uint32_t>(ctl[i]);
    KT_CHECK(order);
    KT_CHECK(g_built && g_nSw == 3 && g_panel == static_cast<int32_t>(panel));
    KT_CHECK(g_pill.c == ctl[0] && g_sw[0].disc.c == ctl[1] && g_sw[0].icon.c == ctl[2] && g_sw[0].over.c == ctl[3] && g_sw[2].over.c == ctl[9]);
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
    KT_CHECK_EQ(newCtl.size(), static_cast<size_t>(10));
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
    // Disabled at BUILD: no labels are created at all, and enabling later is "restart needed": a frame stays silent.
    fresh(false);
    g_cfg.enabled = false;
    L().clear();
    doBuild(tAlloc(0x100));
    KT_CHECK_EQ(cnt(trec::O_Create), 0);
    KT_CHECK(!g_built && g_skippedDisabled);
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

    // --- MaxIcons is write-only live: the built pool keeps its size, the INI and the pending value change, the frame is silent.
    fresh();
    postT(&t);
    L().clear();
    applySetting(esl::S_MaxIcons, +1);
    KT_CHECK_EQ(g_cfg.maxIcons, 3);
    KT_CHECK_EQ(g_pendMax, 4);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "MaxIcons", -1, "")), 4);
    postT(&t, 1);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
    char rowTxt[64];
    rowText(esl::S_MaxIcons, rowTxt, static_cast<int>(sizeof rowTxt));
    KT_CHECK(strstr(rowTxt, "(restart)") != nullptr);

    // --- INI write failure: the live value still applies (Gap), MaxIcons is left unchanged.
    fresh();
    postT(&t);
    g_stubIniWriteFails = true;
    applySetting(esl::S_Gap, +1);
    KT_CHECK_EQ(g_cfg.gap, 5);
    KT_CHECK_EQ(static_cast<int>(GetPrivateProfileIntA("Strip", "Gap", -1, "")), -1);
    applySetting(esl::S_MaxIcons, +1);
    KT_CHECK_EQ(g_pendMax, 3);
    g_stubIniWriteFails = false;

    // --- built-while-disabled: turning Enabled on later needs a restart; the frame stays silent and the row says so.
    fresh(false);
    g_cfg.enabled = false;
    L().clear();
    tPanel = tAlloc(0x100);
    doBuild(tPanel);
    KT_CHECK(!g_built && g_skippedDisabled);
    applySetting(esl::S_Enabled, +1);
    KT_CHECK(g_cfg.enabled);
    rowText(esl::S_Enabled, rowTxt, static_cast<int>(sizeof rowTxt));
    KT_CHECK(strstr(rowTxt, "(restart)") != nullptr);
    L().clear();
    postT(&t, 1);
    postT(&t, 300);
    KT_CHECK_EQ(L().size(), static_cast<size_t>(0));
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
    tBaseCfg = g_cfg;

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
    return kt::Report("enemy_strip_draw_test");
}
