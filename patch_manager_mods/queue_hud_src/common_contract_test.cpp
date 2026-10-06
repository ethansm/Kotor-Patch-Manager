// common_contract_test.cpp -- Patches/Common + _shared_src CONTRACT test for queue-hud (doc 20 section 4.1).
// Pins the semantics QueueHud.cpp relies on in SafeRead.h, ActQueue.h, KpLog.h, Party.h, PortraitSlots.h, Gui2D.h, GuiKit.h, GuiTip.h, HudHost.h, EmbedTex.h and GameAddr.h.
// Every message starts with "queue-hud relies on:" so a failure names the consumer and the reason. Built against ../_shared_src and
// the KPM checkout's Patches/Common (see test.toml). Host-only, deterministic; expected values are independent literals.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#ifndef GUI2D_HOST
#define GUI2D_HOST 1
#endif
#include "SafeRead.h"
#include "ActQueue.h"
#include "KpLog.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "GuiTip.h"
#include "HudHost.h"
#include "SelfHealUi.h"
#include "EmbedTex.h"
#include "HudKit.h"
#include "GameAddr.h"
#include "check.h"
#include <sys/mman.h>
#include <stddef.h>
#include <string.h>
#include <string>
#include <vector>

#define QCHECK(why, expr) ::kt::Check("queue-hud relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define QEQ(why, a, b) ::kt::CheckEq("queue-hud relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("queue-hud");
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
static int32_t KPATCH_CALL psSlotAt(int32_t i, int32_t* slot, int32_t* cr) { if (i != 2) return 0; *slot = 0x1234; *cr = 0x5678; return 1; }
static uint32_t KPATCH_CALL psFrame() { return 9; }
static K2PortraitSlotsApi g_psApi = { sizeof(K2PortraitSlotsApi), psRegister, psUnreg, psCreature, psSlotAt, psFrame };
// ---- fake k2.hud provider (the registry call behind k2hud::Client)
static int g_hudRegs; static int32_t g_hudPhase = -1, g_hudPrio = -1; static std::string g_hudOwner;
static int32_t KPATCH_CALL huRegister(int32_t phase, int32_t prio, K2HudCallback fn, void*, const char* owner) { if (!fn) return 0; ++g_hudRegs; g_hudPhase = phase; g_hudPrio = prio; g_hudOwner = owner; return 1; }
static void KPATCH_CALL huUnreg(int32_t) {}
static int32_t KPATCH_CALL huHud() { return 0; }
static uint32_t KPATCH_CALL huFrame() { return 3; }
static int g_stripCalls; static std::string g_stripOwner; static int32_t g_stripOrder = -1, g_stripW = -1, g_stripH = -1;
static int32_t KPATCH_CALL huStrip(const char* owner, int32_t order, const K2HudRect* vit, int32_t width, int32_t height, K2HudRect* out) { ++g_stripCalls; g_stripOwner = owner; g_stripOrder = order; g_stripW = width; g_stripH = height; *out = K2HudRect{ vit->x - width, vit->y + (vit->h - height) / 2, width, height }; return 1; }
static K2HudApi g_huApi = { sizeof(K2HudApi), huRegister, huUnreg, huHud, huFrame, huStrip };
static std::string g_reqName; static uint32_t g_reqVer;
static const void* KPATCH_CALL fRequire(const char* n, uint32_t v) { g_reqName = n; g_reqVer = v; if (strcmp(n, K2HUD_IFACE) == 0) return &g_huApi;
    return strcmp(n, "kpatch.log") == 0 ? (const void*)&g_log : (const void*)&g_psApi; }
static KPatchApi g_api = { sizeof(KPatchApi), 1, nullptr, fRequire };

static void KPATCH_CALL hudCb(const K2PsContext*, void*) {}
static void KPATCH_CALL hudBuildCb(const K2HudContext*, void*) {}
static constexpr int AppToServerForTest = 8;


// ---- Gui2D fakes
struct PushRec { int x, y, w, h; const void* color; int flag; float alpha; };
static int g_pushes, g_pops, g_pushResult = 1;
static PushRec g_lastPush;
static int fakePush(int x, int y, int w, int h, const void* c, int fl, float a) { ++g_pushes; g_lastPush = { x, y, w, h, c, fl, a }; return g_pushResult; }
static void fakePop() { ++g_pops; }
struct DrawRec { void* img; int x, y, w, h, a5, rot; const float* rgb; float alpha; };
static int g_draws; static DrawRec g_lastDraw;
static void fakeDraw(void* img, int x, int y, int w, int h, int a5, int rot, const float* rgb, float a) { ++g_draws; g_lastDraw = { img, x, y, w, h, a5, rot, rgb, a }; }

// ---- GuiKit fake image loader
static std::vector<std::string> g_loaded;
static void* g_imgToken = reinterpret_cast<void*>(0x1234);
static void* fakeLoadImage(const char* name) { g_loaded.push_back(name); return g_imgToken; }

static uint8_t* g_a;
static uintptr_t A(size_t off) { return reinterpret_cast<uintptr_t>(g_a + off); }
template <typename T> static void put(size_t off, T v) { memcpy(g_a + off, &v, sizeof v); }

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

    // ===== 1. SafeRead readAt: how the DLL walks creature +0x100 / +0x10DC and the round list
    saferead::beginScope();
    put<uint32_t>(0x100, 0x00ABCDEFu); put<uint8_t>(0x200 + 0x10, 9); put<uint32_t>(0x300 + 0x38, 0x7F000001u);
    uint32_t w = 0; uint8_t b = 0; uint32_t p0 = 0;
    QCHECK("readAt<uint32_t>(creature, 0x100) reads the queue-list internal pointer cell", saferead::readAt(0x7100u, A(0), 0x100, &w) && w == 0x00ABCDEFu);
    QCHECK("readAt<uint8_t>(node, 0x10) reads the round node sub-type byte", saferead::readAt(0x7100u, A(0x200), 0x10, &b) && b == 9);
    QCHECK("readAt<uint32_t>(action, 0x38) reads the first param", saferead::readAt(0x7100u, A(0x300), 0x38, &p0) && p0 == 0x7F000001u);
    uint32_t keep = 77;
    QCHECK("readAt with base 0 fails (a null creature/round must not fault)", !saferead::readAt(0x7100u, 0, 0x10DC, &keep));
    QEQ("failed readAt leaves *out untouched", keep, 77u);
    QCHECK("readAt below the 0x10000 floor fails (a small garbage pointer)", !saferead::readAt(0x7100u, 0x1000, 0x10, &keep));
    QEQ("a failed low-address readAt leaves *out untouched", keep, 77u);

    // ===== 2. KpLog_Init + channels
    QCHECK("HasCore() false before init", !KpLog_HasCore());
    QEQ("KpLog_Init(api) returns 1 on a complete kpatch.log service (KPatchInit)", KpLog_Init(&g_api), 1);
    QCHECK("registered under the manifest id \"queue-hud\"", g_regs.size() == 1 && g_regs[0] == "queue-hud");
    QEQ("kpatch.log version 1 requested", (unsigned long long)g_reqVer, 1ULL);
    QCHECK("HasCore() true after init", KpLog_HasCore());
    KP_LOG(chHud, "Q1 slots=%d key=%08x", 3, 0xBEEFu);
    QCHECK("an INFO KP_LOG with %d/%08x is submitted verbatim on the registered channel", g_subs.size() == 1 && g_subs[0].text == "Q1 slots=3 key=0000beef" && g_subs[0].ch == 200);

    // ===== 3. Party chain: party::table / member / serverCreature (member 0 -> client -> server creature)
    QCHECK("party::table(Fail*) -> uintptr_t", static_cast<uintptr_t(*)(party::Fail*)>(&party::table) != nullptr);
    QCHECK("party::member(table, idx) -> uintptr_t", static_cast<uintptr_t(*)(uintptr_t, int)>(&party::member) != nullptr);
    QCHECK("party::serverCreature(member) -> uintptr_t", static_cast<uintptr_t(*)(uintptr_t)>(&party::serverCreature) != nullptr);
    party::g_appGlobal = A(0x500);   // the cell holds 0 -> the chain fails at the first link
    party::Fail why = party::F_None;
    QEQ("table() is 0 when the app global cell is empty (HUD before the app exists)", (unsigned long long)party::table(&why), 0ULL);
    QEQ("table() reports F_App for an empty app global", (int)why, (int)party::F_App);
    // full chain: app cell -> app -> +4 -> +4 -> +0x270 = table
    put<uint32_t>(0x500, (uint32_t)A(0x600));             // *appGlobal = app
    put<uint32_t>(0x600 + 4, (uint32_t)A(0x700));         // *(app+4)
    put<uint32_t>(0x700 + 4, (uint32_t)A(0x800));         // *(a+4)
    put<uint32_t>(0x800 + 0x270, (uint32_t)A(0x900));     // *(b+0x270) = party table
    put<int32_t>(0x900, 2);                               // party count
    QEQ("table() walks app -> +4 -> +4 -> +0x270 (RE.md 0x74DCB1..)", (unsigned long long)party::table(&why), (unsigned long long)A(0x900));
    QEQ("table() success clears the failure reason", (int)why, (int)party::F_None);
    static uintptr_t s_tableSeen; static int s_idxSeen = -9;
    party::g_member = [](void* t, int i) -> uintptr_t { s_tableSeen = (uintptr_t)t; s_idxSeen = i; return A(0xA00); };
    QEQ("member(table, 0) returns the engine's client creature", (unsigned long long)party::member(A(0x900), 0), (unsigned long long)A(0xA00));
    QCHECK("member() passes (table, 0) through unchanged", s_tableSeen == A(0x900) && s_idxSeen == 0);
    s_idxSeen = -9;
    QEQ("member(table, 2) with count 2 is 0 (index out of range, no engine call)", (unsigned long long)party::member(A(0x900), 2), 0ULL);
    QEQ("member(table, -1) is 0", (unsigned long long)party::member(A(0x900), -1), 0ULL);
    QEQ("member(0, 0) is 0", (unsigned long long)party::member(0, 0), 0ULL);
    QEQ("an out-of-range member() never called the engine", s_idxSeen, -9);
    static uintptr_t s_memSeen;
    party::g_serverCreature = [](void* mm) -> uintptr_t { s_memSeen = (uintptr_t)mm; return A(0xB00); };
    QEQ("serverCreature(client) returns the server creature", (unsigned long long)party::serverCreature(A(0xA00)), (unsigned long long)A(0xB00));
    QEQ("serverCreature() passes the client pointer through", (unsigned long long)s_memSeen, (unsigned long long)A(0xA00));
    s_memSeen = 0;
    QEQ("serverCreature(0) is 0 and makes no engine call", (unsigned long long)party::serverCreature(0), 0ULL);
    QEQ("no engine call for a null member", (unsigned long long)s_memSeen, 0ULL);
    party::g_serverCreature = nullptr;
    QEQ("serverCreature() is 0 when the engine function is unavailable (nullptr override)", (unsigned long long)party::serverCreature(A(0xA00)), 0ULL);

    // ===== 4. PortraitSlots: HUD_DRAW registration + context fields
    QEQ("K2PS_HUD_DRAW is phase 5 (the leader call in the HUD frame, clip stack empty)", K2PS_HUD_DRAW, 5);
    QEQ("K2PS_PHASE_COUNT is 6", K2PS_PHASE_COUNT, 6);
    QEQ("K2PsContext.frame is at +24 (provider frame counter: snapshot age)", (long long)offsetof(K2PsContext, frame), 24LL);
    QEQ("K2PsContext.inMenu is at +28 (menu frames are refused)", (long long)offsetof(K2PsContext, inMenu), 28LL);
    QCHECK("K2PsContext.frame is uint32_t, inMenu is int32_t", sizeof(((K2PsContext*)0)->frame) == 4 && sizeof(((K2PsContext*)0)->inMenu) == 4);
    k2ps::Client c;
    c.init(&g_api, "queue-hud");
    QCHECK("add() accepts a HUD_DRAW callback", c.add(K2PS_HUD_DRAW, 300, hudCb));
    QCHECK("connect() succeeds against a complete v1 table", c.connect());
    QEQ("it required \"k2.portraitslots\"", g_reqName == K2PS_IFACE, 1);
    QEQ("version 1", (unsigned long long)g_reqVer, (unsigned long long)K2PS_VERSION);
    QEQ("one registration", g_registered, 1);
    QEQ("phase HUD_DRAW", (long long)g_regPhase, (long long)K2PS_HUD_DRAW);
    QEQ("priority 300", (long long)g_regPrio, 300LL);
    QCHECK("owner is the manifest id", g_regOwner == "queue-hud");
    k2ps::Client c2; c2.poll();
    QCHECK("poll() on an unconnected, uninitialised client stays unconnected (no crash)", !c2.connected);

    // ===== 4b. 0.2.0 companion strips: slotAt + slot geometry offsets + party members 1-2
    { int32_t sl = 0, cr = 0;
      QCHECK("k2ps::Client::slotAt(i, int32_t* slot, int32_t* creature) -> bool fills both for a live entry and fails for others", c.slotAt(2, &sl, &cr) && sl == 0x1234 && cr == 0x5678 && !c.slotAt(0, &sl, &cr));
      QEQ("K2PS_MAX_SLOTS is 4 (the strips loop bounds)", K2PS_MAX_SLOTS, 4); }
    static_assert(kSteam.Slot_CardRect == 0x54c && kSteam.Slot_VitRect == 0x694, "queue-hud relies on: Slot_CardRect 0x54c / Slot_VitRect 0x694 (strip anchors, 4 ints x,y,w,h)");
    static_assert(kSteam.ScreenW == 0x009F42A4, "queue-hud relies on: ScreenW 0x9F42A4 (strip right-edge clamp)");
    QEQ("Steam table is the default for this host test: Slot_VitRect", k2addr::get()->Slot_VitRect, 0x694);
    party::g_member = [](void*, int i) -> uintptr_t { return i == 1 ? A(0xC00) : i == 2 ? A(0xC40) : 0; };
    QEQ("member(table, 1) is the first companion (client creature)", (unsigned long long)party::member(A(0x900), 1), (unsigned long long)A(0xC00));
    put<int32_t>(0x900, 3);
    QEQ("member(table, 2) with count 3 is the second companion", (unsigned long long)party::member(A(0x900), 2), (unsigned long long)A(0xC40));

    // ===== 5. Gui2D: fillRect / drawImage forward to the engine hooks (compile-time signatures + behaviour on fakes)
    QCHECK("gui2d::fillRect(x, y, w, h, const float* rgb, float alpha) keeps its signature", static_cast<void(*)(int, int, int, int, const float*, float)>(&gui2d::fillRect) != nullptr);
    QCHECK("gui2d::drawImage(void* img, x, y, w, h, int rotBits, const float* rgb, float alpha) keeps its signature", static_cast<void(*)(void*, int, int, int, int, int, const float*, float)>(&gui2d::drawImage) != nullptr);
    gui2d::g_g2d.push = fakePush; gui2d::g_g2d.pop = fakePop; gui2d::g_g2d.draw = fakeDraw;
    const float green[3] = { 0.10f, 0.55f, 0.20f };
    gui2d::fillRect(10, 20, 30, 40, green, 0.45f);
    QCHECK("fillRect pushes (x, y, w, h) with the colour pointer and alpha, flag 0", g_pushes == 1 && g_lastPush.x == 10 && g_lastPush.y == 20 && g_lastPush.w == 30 && g_lastPush.h == 40 && g_lastPush.color == green && g_lastPush.flag == 0 && g_lastPush.alpha == 0.45f);
    QEQ("fillRect pops once after a successful push", g_pops, 1);
    g_pushResult = 0;
    gui2d::fillRect(1, 2, 3, 4, green, 1.0f);
    QCHECK("fillRect does not pop when the push was refused (clip stack stays balanced)", g_pushes == 2 && g_pops == 1);
    int img = 0;
    gui2d::drawImage(&img, 5, 6, 7, 8, 3, green, 0.6f);
    QCHECK("drawImage forwards (img, x, y, w, h, 0, rotBits, rgb, alpha) to the Draw hook", g_draws == 1 && g_lastDraw.img == &img && g_lastDraw.x == 5 && g_lastDraw.y == 6 && g_lastDraw.w == 7 && g_lastDraw.h == 8 && g_lastDraw.a5 == 0 && g_lastDraw.rot == 3 && g_lastDraw.rgb == green && g_lastDraw.alpha == 0.6f);

    // ===== 6. GuiKit: getExtent / imageFor
    QCHECK("guikit::getExtent(ctl, int* x, int* y, int* w, int* h) -> bool keeps its signature", static_cast<bool(*)(uintptr_t, int*, int*, int*, int*)>(&guikit::getExtent) != nullptr);
    QCHECK("guikit::imageFor(const char* resref, bool* loaded, int* poolCount) -> void* keeps its signature", static_cast<void*(*)(const char*, bool*, int*)>(&guikit::imageFor) != nullptr);
    int gx = 7, gy = 7, gw = 7, gh = 7;
    QCHECK("getExtent(0, ...) fails without touching the outputs (a missing control)", !guikit::getExtent(0, &gx, &gy, &gw, &gh) && gx == 7 && gy == 7 && gw == 7 && gh == 7);
    guikit::g_gk.loadImage = fakeLoadImage;
    bool loaded = false; int pool = -1;
    void* im = guikit::imageFor("I_Attack", &loaded, &pool);
    QCHECK("imageFor lowercases the resref, loads once and reports it", im == g_imgToken && loaded && pool == 1 && g_loaded.size() == 1 && g_loaded[0] == "i_attack");
    void* im2 = guikit::imageFor("i_attack", &loaded, &pool);
    QCHECK("imageFor caches per resref (case-insensitive): second lookup does not reload", im2 == g_imgToken && !loaded && pool == 1 && g_loaded.size() == 1);
    QCHECK("imageFor(\"\") and imageFor(nullptr) return null", guikit::imageFor("") == nullptr && guikit::imageFor(nullptr) == nullptr);
    QEQ("GuiKit ImagePoolSize bounds the per-resref cache (blank/unique icons must not grow it unbounded)", guikit::ImagePoolSize, 256);

    // ===== 7. 0.3.0 kit + Cancel: hud-host BUILD, EmbedTex, GuiKit label/click API, GuiTip, the Cancel route address
    QEQ("K2HUD_BUILD is phase 3 (onHudBuild registration)", K2HUD_BUILD, 3);
    QEQ("K2HudContext.panel is the last field (BUILD ctx->panel; struct_size gate offsetof+sizeof)", (long long)offsetof(K2HudContext, panel) + (long long)sizeof(((K2HudContext*)0)->panel), (long long)sizeof(K2HudContext));
    QCHECK("the hud-host iface id is \"k2.hud\" v1 (manifest requires_interfaces k2.hud@1)", strcmp(K2HUD_IFACE, "k2.hud") == 0 && K2HUD_VERSION == 1);
    { g_hudRegs = 0;
      k2hud::Client hc; hc.init(&g_api, "queue-hud");
      QCHECK("k2hud::Client::add() accepts a BUILD callback", hc.add(K2HUD_BUILD, 300, hudBuildCb));
      QCHECK("k2hud::Client::connect() succeeds against a complete v1 K2HudApi", hc.connect());
      QCHECK("it required \"k2.hud\" v1", g_reqName == K2HUD_IFACE && g_reqVer == (uint32_t)K2HUD_VERSION);
      QCHECK("one BUILD registration at prio 300 under the manifest id", g_hudRegs == 1 && g_hudPhase == K2HUD_BUILD && g_hudPrio == 300 && g_hudOwner == "queue-hud");
      k2hud::Client hn; hn.init(&g_api, "queue-hud");
      QCHECK("k2hud::Client::poll() (the capture-hook retry) connects on its first call and registers nothing when nothing was added", (hn.poll(), hn.connected) && g_hudRegs == 1); }
    QEQ("K2PS_FRAME_BEGIN is phase 2 (the Cancel click / texture heal tick)", K2PS_FRAME_BEGIN, 2);
    QCHECK("embtex::ensure(const Tex*, int) -> bool and embtex::heal() keep their signatures", static_cast<bool(*)(const embtex::Tex*, int)>(&embtex::ensure) != nullptr && static_cast<void(*)()>(&embtex::heal) != nullptr);
    QCHECK("embtex::Tex is {resref, data, size} (HudKitTextures.inc initialisers)", offsetof(embtex::Tex, resref) == 0 && sizeof(embtex::Tex) == sizeof(void*) * 2 + sizeof(unsigned) + (sizeof(void*) == 8 ? 4 : 0));
    QCHECK("hudkit::ensure() -> bool, hudkit::image(int) / frame(bool) / cancel(bool) -> void*, hudkit::resref(int) -> const char*",
           static_cast<bool(*)()>(&hudkit::ensure) != nullptr && static_cast<void*(*)(int)>(&hudkit::image) != nullptr && static_cast<void*(*)(bool)>(&hudkit::frame) != nullptr &&
           static_cast<void*(*)(bool)>(&hudkit::cancel) != nullptr && static_cast<const char*(*)(int)>(&hudkit::resref) != nullptr);
    QCHECK("hudkit ids Frame/FrameHi/Cancel/CancelHi = 0..3 (tex::Frame.. aliases), 5 kit textures, kit_frame / kit_cancel_hi resrefs",
           hudkit::Frame == 0 && hudkit::FrameHi == 1 && hudkit::Cancel == 2 && hudkit::CancelHi == 3 && hudkit::Count == 5 && !strcmp(hudkit::resref(hudkit::Frame), "kit_frame") && !strcmp(hudkit::resref(hudkit::CancelHi), "kit_cancel_hi"));
    QCHECK("the 5 kit textures fit the embtex slot table", embtex::MaxTex >= hudkit::Count);
    QCHECK("shui::buttonRect(Rect) -> Rect: the slim strip icon size is its width (24 on the live 47x96 vit)", shui::buttonRect(shui::Rect{ 3671, 1285, 47, 96 }).w == 24);
    QCHECK("embtex::g_log is a printf-style sink pointer (tex::logLine)", static_cast<void (**)(const char*, ...)>(&embtex::g_log) != nullptr);
    QCHECK("guikit::createLabelFromTemplate(panel, template, int* why, uint32_t* badVt) -> uintptr_t", static_cast<uintptr_t(*)(uintptr_t, const char*, int*, uint32_t*)>(&guikit::createLabelFromTemplate) != nullptr);
    QCHECK("guikit::appendToPanel(panel, ctl) -> bool", static_cast<bool(*)(uintptr_t, uintptr_t)>(&guikit::appendToPanel) != nullptr);
    QCHECK("guikit::setRect / setVisible / setClickable / setFill / setText keep their signatures",
           static_cast<bool(*)(uintptr_t, int, int, int, int)>(&guikit::setRect) != nullptr && static_cast<bool(*)(uintptr_t, bool)>(&guikit::setVisible) != nullptr &&
           static_cast<bool(*)(uintptr_t, bool)>(&guikit::setClickable) != nullptr && static_cast<bool(*)(uintptr_t, void*, const float*)>(&guikit::setFill) != nullptr &&
           static_cast<bool(*)(uintptr_t, const char*)>(&guikit::setText) != nullptr);
    QCHECK("guikit::hit(ctl, x, y) / cursorEngine(int*, int*) / clickEdge(ClickEdge&) / playHoverSound() keep their signatures",
           static_cast<bool(*)(uintptr_t, int, int)>(&guikit::hit) != nullptr && static_cast<bool(*)(int*, int*)>(&guikit::cursorEngine) != nullptr &&
           static_cast<bool(*)(guikit::ClickEdge&)>(&guikit::clickEdge) != nullptr && static_cast<bool(*)()>(&guikit::playHoverSound) != nullptr);
    { guikit::ClickEdge e; QCHECK("clickEdge is false without the host callbacks (no foreground / button source) and leaves e.prev false", !guikit::clickEdge(e) && !e.prev); }
    { const uintptr_t lbl = A(0xD00);
      QCHECK("setVisible / setClickable refuse a null control (a failed label build leaves w.c = 0)", !guikit::setVisible(0, true) && !guikit::setClickable(0, true));
      QCHECK("hit(0, ...) is false for a missing control", !guikit::hit(0, 1, 1)); (void)lbl; }
    QCHECK("guitip::setNative(ctl, text) -> bool and nativeSupported() -> bool", static_cast<bool(*)(uintptr_t, const char*)>(&guitip::setNative) != nullptr && static_cast<bool(*)()>(&guitip::nativeSupported) != nullptr);
    QCHECK("guitip::setNative(0, text) refuses a null control", !guitip::setNative(0, "Cancel queued actions"));
    QEQ("gameaddr::ServerClearAllActions is CServerExoApp::ClearAllActions 0x0051CB10 (doc 30 R7, verified B9)", (unsigned long long)gameaddr::ServerClearAllActions, 0x0051CB10ULL);
    QEQ("gameaddr::HudVtable is the CSWGuiInGame vtable 0x0099DCC4 (BUILD panel check)", (unsigned long long)gameaddr::HudVtable, 0x0099DCC4ULL);
    QCHECK("gameaddr::ProjectionFlag is 0x00A1B9F0 (the label-bind projection guard)", gameaddr::ProjectionFlag == 0x00A1B9F0u);
    // the Cancel route reads: *(AppGlobal)+8 = server app, server creature +4 = object id (both saferead cells, planted here)
    { party::g_appGlobal = A(0x1000);
      put<uint32_t>(0x1000, (uint32_t)A(0x1100));          // *appGlobal = app
      put<uint32_t>(0x1100 + 8, (uint32_t)A(0x1200));      // *(app+8) = CServerExoApp
      put<uint32_t>(0x1300 + 4, 0x7F001234u);              // server creature +4 = object id
      uint32_t app = 0, server = 0, id = 0;
      QCHECK("readAt(*AppGlobal, +8) resolves the server app (ECX of ClearAllActions)", saferead::readAt(0x7160u, party::appGlobalAddr(), 0, &app) && app == (uint32_t)A(0x1100) && saferead::readAt(0x7161u, app, 8, &server) && server == (uint32_t)A(0x1200));
      QCHECK("readAt(serverCreature, +4) resolves the server object id (the stack argument)", saferead::readAt(0x7162u, A(0x1300), 4, &id) && id == 0x7F001234u);
      QCHECK("an empty app cell fails the chain instead of calling the engine", !saferead::readAt(0x7163u, 0, AppToServerForTest, &server)); }

    // ---- ActQueue.h: the reader QueueHud's gatherFor wraps (offsets, caps, site-id parameters, entry/round shapes)
    QEQ("actq creature queue / round offsets", actq::CrQueue * 0x10000 + actq::CrRound, 0x100 * 0x10000 + 0x10DC);
    QEQ("actq round list/cur/ab8/aa8 offsets", (actq::RdList << 16) ^ actq::RdCur ^ (actq::RdAB8 << 4) ^ actq::RdAA8, (0xB00 << 16) ^ 0xB18 ^ (0xAB8 << 4) ^ 0xAA8);
    QEQ("actq entry cap (the slot builder's 16)", actq::kMaxEntries, 16);
    QEQ("actq round cap (the slot builder's 4)", actq::kMaxRound, 4);
    QEQ("actq round-node item id offset", actq::RnItem, 0x64);
    {
        // cr @0xE000 -> list @0xE200 -> node @0xE210 -> action @0xE220 {type 0x3F, p0 9}
        const size_t cr = 0xE000;
        put<uint32_t>(cr + actq::CrQueue, (uint32_t)A(0xE200)); put<uint32_t>(0xE200, (uint32_t)A(0xE210));
        put<uint32_t>(0xE210 + actq::NodeData, (uint32_t)A(0xE220)); put<uint32_t>(0xE220 + actq::ActType, 0x3F); put<uint32_t>(0xE220 + actq::ActParam0, 9);
        actq::Snapshot s;
        const bool ok = actq::read(A(0xE000), &s, 0x7110, 0x7120);
        QCHECK("actq::read(cr, snap, siteQueue, siteRound) returns true for a live creature pointer", ok);
        QEQ("actq::read fills entry count/type/param0", s.ne * 1000 + s.e[0].type, 1 * 1000 + 0x3F);
        QEQ("actq::read keeps param0 and the action data pointer", (int)s.e[0].param0 * 2 + (s.eData[0] == A(0xE220) ? 1 : 0), 19);
        QCHECK("actq::read with creature 0 returns false", !actq::read(0, &s, 0x7110, 0x7120));
        QCHECK("actq::hasType / headType read the snapshot", ([&] { actq::Snapshot t; t.e[0].type = 0x3F; t.ne = 1; return actq::hasType(t, 0x3F) && actq::headType(t) == 0x3F && actq::headType(actq::Snapshot()) == -1; })());
    }
    return kt::Report("common_contract_test (queue_hud)");
}
