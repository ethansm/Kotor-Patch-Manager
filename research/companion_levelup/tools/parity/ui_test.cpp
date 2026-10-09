// Host test of the M4 character-screen UI (LevelUpUi.h) and the R11 banked message on fake engine structs in a MAP_32BIT arena.
// GuiKit's engine calls are fakes (as guikit_test.cpp), the SetStats hook is driven through LuoHold_84F221(stats, ebp) with a fake frame
// ([ebp-0x288] = panel), clicks are simulated through the fake cursor / left button / foreground. Config = temp ini in a temp cwd
// (table = argv[1], the real runtime table). Run via parity.py.
#include "../../../../patch_manager_mods/level_up_override_src/LevelUpOverride.cpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cmath>
#include <map>
#include <string>
#include <vector>

static uint8_t* g_arena = nullptr;
static size_t g_used = 0;
constexpr size_t ArenaSize = 16u << 20;
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { fprintf(stderr, "arena full\n"); exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used);
    memset(g_arena + g_used, 0, n);
    g_used += n;
    return a;
}
#include "TestHarness.h"
#define CHECK(c, ...) do { \
    char _detail[512]; \
    std::snprintf(_detail, sizeof(_detail), __VA_ARGS__); \
    ::kt::Check(__FILE__ ":" KT_STR(__LINE__) ": " #c, static_cast<bool>(c), _detail); \
} while (0)
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
static void* P(uintptr_t a) { return reinterpret_cast<void*>(a); }

// ---- fake engine: gates ----
static int g_canRet = 1;
static int fakeCanLevelUp(void*) { return g_canRet; }

// ---- fake engine: CExoString / globals table (M3 for ptApply) ----
static std::map<std::pair<void*, std::string>, int> g_vars;
static uintptr_t g_nameSlots[64];
static int g_ctors = 0;
static std::string nameOf(void* n) { return reinterpret_cast<const char*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(n))); }
static void* fakeStrCtor(void* buf, const char* s) {
    uintptr_t slot = g_nameSlots[g_ctors++ & 63];
    strncpy(reinterpret_cast<char*>(slot), s, 255);
    reinterpret_cast<uint32_t*>(buf)[0] = (uint32_t)slot;
    reinterpret_cast<uint32_t*>(buf)[1] = (uint32_t)strlen(s) + 1;
    return buf;
}
static void fakeStrDtor(void*) {}
static int fakeGFind(void* t, void* n) { return g_vars.count({t, nameOf(n)}) ? 0 : -1; }
static int fakeGAdd(void* t, void* n) { g_vars[{t, nameOf(n)}] = 0; return 1; }
static void fakeGSet(void* t, void* n, uint32_t v) { if (g_vars.count({t, nameOf(n)})) g_vars[{t, nameOf(n)}] = (int)v; }
static void fakeGGet(void* t, void* n, int* out) { auto it = g_vars.find({t, nameOf(n)}); *out = it == g_vars.end() ? 0 : it->second; }
static int fakeServerInit(void*) { return 0; }

// ---- fake engine: banked message ----
static std::vector<std::string> g_banked;
static void* g_bankedGui = nullptr;
static int g_bankedFlags = -1, g_bankedByte = -1;
static void fakeBankedAdd(void* gui, void* exo, int flags, uint8_t b) { g_bankedGui = gui; g_bankedFlags = flags; g_bankedByte = b; g_banked.push_back(nameOf(exo)); }

// ---- fake engine: GuiKit ----
using namespace guikit;
static int g_addToListSeen = 0;
static int nCtor = 0, nInit = 0, nInitPlain = 0, nInitFramed = 0, nSetRect = 0, nSound = 0, nColor = 0, nList = 0;
static float lastColor[3];
static std::map<uintptr_t, std::string> g_text;
static std::map<std::string, int> g_missingTag;   // template tags the fake layout lacks
static void* fAlloc(unsigned n) { return reinterpret_cast<void*>(arenaAlloc(n)); }
static void fCtor(void* t) {
    ++nCtor; uintptr_t c = reinterpret_cast<uintptr_t>(t);
    at<uint32_t>(c, 0) = LabelVtable; at<uint32_t>(c, Label_Border) = BorderVtable; at<uint32_t>(c, Label_Text) = TextVtable;
    at<uint32_t>(c, Label_Text + Text_Renderer) = 0x1000; at<int>(c, Ctl_Flags) = 0x2;
    at<int>(c, 0x28) = 77;   // stale tooltip strref: GuiTip::setNative must zero it on tip controls only
    at<float>(c, Label_Text + Text_StringSub + Text_Color) = 0.f; at<float>(c, Label_Text + Text_StringSub + Text_Color + 4) = 1.f; at<float>(c, Label_Text + Text_StringSub + Text_Color + 8) = 0.f;
}
// CExoString fake: the first dword of the 16-byte buffer GuiKit passes holds a pointer to a copy (ring of 64 arena slots).
static uintptr_t g_exoSlots[64]; static int g_exoNext = 0;
static std::map<uintptr_t, std::string> g_exoDst;   // destination buffer -> last string constructed there (the native tooltip field ctl+0x2C is one)
static void* fExoCtor(void* e, const char* s) { g_exoDst[reinterpret_cast<uintptr_t>(e)] = s; uintptr_t sl = g_exoSlots[g_exoNext++ & 63]; strncpy(reinterpret_cast<char*>(sl), s, 255); memset(e, 0, 16); *reinterpret_cast<uint32_t*>(e) = (uint32_t)sl; return e; }
static const char* exoStr(void* e) { return reinterpret_cast<const char*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(e))); }
static void fExoDtor(void*) {}
static void fInit(void* panel, void* ctl, void* exo, int addToList, int scale) {
    (void)panel; (void)scale;
    ++nInit;
    uintptr_t c = reinterpret_cast<uintptr_t>(ctl);
    const char* tag = exoStr(exo);
    if (addToList != 0) ++g_addToListSeen;   // we must create with addToList = 0 (Mod 14's ids 69..150 stay intact)
    if (g_missingTag.count(tag)) return;   // sentinel stays: createLabelFromTemplate returns 0
    at<int>(c, Ctl_Id) = -1;
    const bool plain = !strcmp(tag, "LBL_EXPERIENCE_STAT");
    plain ? ++nInitPlain : ++nInitFramed;
    const int u[4] = {plain ? 60 : 54, plain ? 449 : 431, plain ? 108 : 223, plain ? 18 : 39};   // template unit rect; scale 2, origin 0
    for (int k = 0; k < 4; ++k) at<int>(c, Ctl_X + 4 * k) = u[k] * 2;
}
static void fTextSet(void* sub, void* exo) { g_text[reinterpret_cast<uintptr_t>(sub) - (Label_Text + Text_StringSub)] = exoStr(exo); }
static void fFont(void*, const char*) {}
static void fAlign(void*, unsigned) {}
static void fColor(void*, float* v) { ++nColor; memcpy(lastColor, v, 12); }
static void fList(void* list, void* v) { ++nList; uintptr_t l = reinterpret_cast<uintptr_t>(list); uint32_t arr = at<uint32_t>(l, 0); int cnt = at<int>(l, 4); at<uint32_t>(arr, cnt * 4) = (uint32_t)reinterpret_cast<uintptr_t>(v); at<int>(l, 4) = cnt + 1; }
static std::map<std::string, void*> g_img;   // resref -> image handle (absent: 0x5000; nullptr = does not load). imageFor pools: set before the first load
static void* fLoadImage(const char* n) { auto it = g_img.find(n); return it == g_img.end() ? reinterpret_cast<void*>(0x5000) : it->second; }
// ---- fake engine: EmbedTex (batch 3c; installed in section 7 only, so earlier panels take the lbl_icn_opt2 fallback) ----
static int eNew = 0, eCtor = 0, eAdd = 0;
static void* efNew(unsigned n) { ++eNew; return reinterpret_cast<void*>(arenaAlloc(n)); }
static void* efNewArr(unsigned n) { return reinterpret_cast<void*>(arenaAlloc(n)); }
static void* efCtor(void* t, const char*, const char*) { ++eCtor; return t; }
static void efAdd(void*, void*) { ++eAdd; }
static void efReset(void*) {}
static void fSound(void*, char) { ++nSound; }
static void fSetRect(uint32_t, uintptr_t ctl, int* r) { ++nSetRect; for (int k = 0; k < 4; ++k) at<int>(ctl, Ctl_X + 4 * k) = r[k]; }
static uintptr_t g_tipCell = 0, g_tipMgr = 0;
static int curX = 0, curY = 0; static bool fg = true, lmb = false;
static bool fCursor(int* x, int* y, int* w, int* h) { *x = curX; *y = curY; *w = 1920; *h = 800; return true; }
static bool fFg() { return fg; }
static bool fLmb() { return lmb; }

// ---- fake character screen panels ----
static std::map<uintptr_t, uintptr_t> g_frames;   // panel -> ebp
// Panel with 70 controls (vanilla ids 0..69), BTN_LEVELUP at index 66 with the extent given (default 2x: 724,650,188,80).
static uintptr_t makePanel(int btnW = 188, int btnH = 80, bool withBtn = true) {
    uintptr_t panel = arenaAlloc(0x200), arr = arenaAlloc(512 * 4);
    at<uint32_t>(panel, Panel_CtlArray) = (uint32_t)arr; at<int>(panel, Panel_CtlCount) = 70;
    if (withBtn) {
        uintptr_t btn = arenaAlloc(0x150); at<uint32_t>(btn, 0) = LabelVtable;
        at<int>(btn, Ctl_X) = 724; at<int>(btn, Ctl_Y) = 650; at<int>(btn, Ctl_W) = btnW; at<int>(btn, Ctl_H) = btnH;
        at<uint32_t>(arr, 66 * 4) = (uint32_t)btn;
    }
    uintptr_t frame = arenaAlloc(0x400), ebp = frame + 0x300;
    at<uint32_t>(ebp, -0x288) = (uint32_t)panel;
    g_frames[panel] = ebp;
    return panel;
}

// ---- fake creatures ----
struct Fake { uintptr_t stats, creature; };
static Fake makeCreature(const char* tag, std::vector<std::pair<int, int>> classes, uint32_t xp = 0) {
    Fake f;
    f.stats = arenaAlloc(0x400);
    f.creature = arenaAlloc(0x1200);
    uint8_t* st = reinterpret_cast<uint8_t*>(f.stats);
    st[St_ClassCount] = (uint8_t)classes.size();
    for (size_t i = 0; i < classes.size(); ++i) { st[St_ClassId + i * St_ClassStride] = (uint8_t)classes[i].first; st[St_ClassLevel + i * St_ClassStride] = (uint8_t)classes[i].second; }
    *reinterpret_cast<uint32_t*>(st + St_Creature) = (uint32_t)f.creature;
    *reinterpret_cast<uint32_t*>(st + St_XP) = xp;
    uint8_t* cr = reinterpret_cast<uint8_t*>(f.creature);
    uintptr_t str = arenaAlloc(64);
    memcpy(reinterpret_cast<void*>(str), tag, strlen(tag));
    *reinterpret_cast<uint32_t*>(cr + Cr_Tag) = (uint32_t)str;
    *reinterpret_cast<uint32_t*>(cr + Cr_Tag + 4) = (uint32_t)strlen(tag);
    *reinterpret_cast<uint32_t*>(cr + Cr_Stats) = (uint32_t)f.stats;
    return f;
}

static void setIni(const char* body, const char* table) {
    FILE* f = fopen("levelup_override.ini", "w");
    fprintf(f, "dev=1\n%s\ntable=%s\n", body, table);   // dev=1: the dev keys are ignored without it (1.0.0)
    fclose(f);
    g_stubTick += 5000;
}
static std::string readLog() { std::string s; if (FILE* f = fopen("levelup_override_log.txt", "r")) { char b[512]; while (fgets(b, sizeof b, f)) s += b; fclose(f); } return s; }
static void clearLog() { remove("levelup_override_log.txt"); }
static bool logHas(const char* x) { return readLog().find(x) != std::string::npos; }
static int logCount(const char* x) { std::string l = readLog(); int n = 0; size_t p = 0; while ((p = l.find(x, p)) != std::string::npos) { ++n; ++p; } return n; }
static std::string fileText(const char* name) { std::string s; if (FILE* f = fopen(name, "r")) { char b[512]; while (fgets(b, sizeof b, f)) s += b; fclose(f); } return s; }

// One SetStats tick (what the hooked 0x84F221 does every frame).
static int tick(uintptr_t panel, const Fake& who, int realCan) {
    g_canRet = realCan;
    return LuoHold_84F221(P(who.stats), P(g_frames[panel]));
}
static UiPanel* rec(uintptr_t panel) { return findUi(panel); }
static uintptr_t ctl(uintptr_t panel, int i) { UiPanel* r = rec(panel); return r ? r->c[i] : 0; }
static bool vis(uintptr_t panel, int i) { return isVisible(ctl(panel, i)); }
static std::string txt(uintptr_t panel, int i) { return g_text[ctl(panel, i)]; }
static bool anyVisible(uintptr_t panel) { for (int i = 0; i < C_N; ++i) if (vis(panel, i)) return true; return false; }
// Hover tick, press tick, release tick over the centre of control idx.
static void clickCtl(uintptr_t panel, const Fake& who, int real, int idx) {
    int x, y, w, h;
    if (idx == C_CFG && !vis(panel, C_CFG) && vis(panel, C_ICON)) idx = C_ICON;   // batch 3a: no level-up buttons -> the corner icon opens the same page
    if (!getExtent(ctl(panel, idx), &x, &y, &w, &h)) { CHECK(false, "clickCtl: no extent"); return; }
    curX = x + w / 2; curY = y + h / 2;
    lmb = false; tick(panel, who, real);
    lmb = true; tick(panel, who, real);
    lmb = false; tick(panel, who, real);
}
// ---- hover tooltip helpers (section 5) ----
static unsigned g_tipCovered = 0; static int g_tipHovers = 0;
static bool boxShown(uintptr_t panel) { return vis(panel, C_TIPBG) || vis(panel, C_TIP); }
static const uintptr_t kTipDelay = 500;   // fake manager appear time 0.5 s
// Hover the centre of control idx from a fresh hover, step the clock around delay + grace (500 + 250 ms), check the box, then move away.
static void tipHover(uintptr_t panel, const Fake& who, int idx, int real = 0) {   // real = the SetStats CanLevelUp value (1: Configure shown, 0: icon)
    UiPanel* r = rec(panel);
    int x, y, w, h;
    if (!r || !getExtent(ctl(panel, idx), &x, &y, &w, &h)) { CHECK(false, "tipHover: no extent for %d", idx); return; }
    at<uint32_t>(g_tipMgr, 0x40) = 0; at<uint32_t>(g_tipMgr, 0x08) = (uint32_t)ctl(panel, idx); at<uint32_t>(g_tipMgr, 0x1C) = 0;
    curX = 0; curY = 0; lmb = false; tick(panel, who, real);   // start from no hover
    curX = x + w / 2; curY = y + h / 2;
    g_stubTick += 20000;
    const DWORD t0 = g_stubTick;
    ++g_tipHovers;
    tick(panel, who, real);
    CHECK(!boxShown(panel), "%s: no box on the first hover tick", kUiName[idx]);
    g_stubTick = t0 + kTipDelay + 249; tick(panel, who, real);
    CHECK(!boxShown(panel), "%s: no box before delay + 250 ms grace", kUiName[idx]);
    g_stubTick = t0 + kTipDelay + 250; tick(panel, who, real);
    CHECK(boxShown(panel) && vis(panel, C_TIPBG) && vis(panel, C_TIP) && txt(panel, C_TIP) == uiTipText(idx), "%s: custom box at delay + grace (tipbg %d tip %d '%s')", kUiName[idx], vis(panel, C_TIPBG), vis(panel, C_TIP), txt(panel, C_TIP).c_str());
    int bx, by, bw, bh, tx, ty, tw, th;
    getExtent(ctl(panel, C_TIPBG), &bx, &by, &bw, &bh); getExtent(ctl(panel, C_TIP), &tx, &ty, &tw, &th);
    const int yEnd = r->open ? 940 : 744;   // page column y 134..470, or above the Configure button (y 372) when the page is closed; scale 2, origin 0
    if (idx == C_ICON)   // batch 3a: the icon's box stays inside the 3D view right of SLD_ALIGN and above BTN_AUTO (units 350..525 x 134..255)
        CHECK(bx >= 700 && bx + bw <= 1050 && by >= 268 && by + bh <= 510 && bw > 0 && bh > 0, "%s: box px rect %d,%d,%d,%d outside the 3D-view region", kUiName[idx], bx, by, bw, bh);
    else CHECK(bx >= 108 && bx + bw <= 600 && by >= 268 && by + bh <= yEnd && bw > 0 && bh > 0, "%s: box px rect %d,%d,%d,%d outside the column (y end %d)", kUiName[idx], bx, by, bw, bh, yEnd);
    CHECK(tx >= bx && ty >= by && tx + tw <= bx + bw && ty + th <= by + bh, "%s: text rect %d,%d,%d,%d not inside the box", kUiName[idx], tx, ty, tw, th);
    CHECK(!(at<int>(ctl(panel, C_TIPBG), Ctl_Flags) & 0x2) == false && (at<int>(ctl(panel, C_TIPBG), Ctl_Flags) & FlagClickThrough) && (at<int>(ctl(panel, C_TIP), Ctl_Flags) & FlagClickThrough), "%s: box click-through", kUiName[idx]);
    CHECK(bx + bw <= x || x + w <= bx || by + bh <= y || y + h <= by, "%s: box %d,%d,%d,%d must not cover the control %d,%d,%d,%d", kUiName[idx], bx, by, bw, bh, x, y, w, h);
    { const int sr = nSetRect; tick(panel, who, real); CHECK(nSetRect == sr && boxShown(panel), "%s: a steady box costs no setRect (%d)", kUiName[idx], nSetRect - sr); }
    g_tipCovered |= 1u << idx;
    curX = 0; curY = 0; tick(panel, who, real);   // moving away hides it
    CHECK(!boxShown(panel), "%s: box hidden on move", kUiName[idx]);
}
static void tipCoverShown(uintptr_t panel, const Fake& who) {
    for (int i = 0; i < C_TIPBG; ++i) if (uiTipText(i) && vis(panel, i) && !(g_tipCovered & (1u << i))) tipHover(panel, who, i);
}

int main(int argc, char** argv) {
    if (argc < 2) return 2;
    const char* table = argv[1];
    char tmpl[] = "/tmp/ui_test_XXXXXX";
    if (!mkdtemp(tmpl) || chdir(tmpl)) { perror("mkdtemp"); return 5; }
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    void* vp = mmap(reinterpret_cast<void*>(0x00987000), 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (vp == MAP_FAILED) { perror("mmap 0x987000"); return 5; }
    at<uint32_t>(LabelVtable, 4) = 0x1234;

    // pure text builders and banked arithmetic (no engine)
    {
        char b[160]; char cb[16];
        CHECK(buildStatus(true, true, true, 1, 5, "Dual sabers", false, b, sizeof b) && !strcmp(b, "Holding 1 level for Jedi Sentinel - Dual sabers"), "held N=1: %s", b);
        CHECK(buildStatus(true, true, true, 3, 5, "Dual sabers", false, b, sizeof b) && !strcmp(b, "Holding 3 levels for Jedi Sentinel - Dual sabers"), "held N=3: %s", b);
        CHECK(buildStatus(true, true, true, 0, 3, "X", false, b, sizeof b) && !strcmp(b, "Holding the next level for Jedi Guardian - X"), "held N=0: %s", b);
        CHECK(buildStatus(true, true, false, 0, 5, "Dual sabers", false, b, sizeof b) && !strcmp(b, "Auto path: Dual sabers"), "configured: %s", b);
        CHECK(buildStatus(true, true, false, 0, 5, "Dual sabers", true, b, sizeof b) && !strcmp(b, "Auto path: Dual sabers (instant)"), "instant: %s", b);
        CHECK(!buildStatus(true, false, false, 0, 5, "Dual sabers", false, b, sizeof b) && !b[0], "unconfigured must be hidden");
        CHECK(!buildStatus(false, true, true, 2, 5, "Dual sabers", true, b, sizeof b) && !b[0], "PC must be hidden");
        CHECK(buildStatus(true, true, true, 2, 42, "S", false, b, sizeof b) && !strcmp(b, "Holding 2 levels for class 42 - S"), "unknown class: %s", b);
        CHECK(!strcmp(className(0, cb, sizeof cb), "Soldier") && !strcmp(className(9, cb, sizeof cb), "Tech Specialist") && !strcmp(className(10, cb, sizeof cb), "class 10") && !strcmp(className(-1, cb, sizeof cb), "class -1"), "class names");
        bool hl;
        CHECK(!strcmp(cfgButtonText(false, false, &hl), "Configure") && !hl, "cfg unconfigured");
        CHECK(!strcmp(cfgButtonText(false, true, &hl), "Plan level-ups") && hl, "cfg nudge");
        CHECK(!strcmp(cfgButtonText(true, true, &hl), "Change path") && !hl && !strcmp(cfgButtonText(true, false, &hl), "Change path") && !hl, "cfg configured");
        uint32_t tbl[51];
        for (int i = 0; i < 50; ++i) tbl[i] = 500u * i * (i + 1);
        tbl[50] = 0xFFFFFFFFu;
        tbl[0] = 0; tbl[1] = 1000; tbl[2] = 3000;   // tbl[i] = XP to reach level i+1
        CHECK(bankedFromTable(tbl, 1, 1000) == 1, "L1 XP1000");
        CHECK(bankedFromTable(tbl, 1, 3000) == 2, "L1 XP3000");
        CHECK(bankedFromTable(tbl, 3, 5999) == 0, "L3 XP5999");
        CHECK(bankedFromTable(tbl, 8, 100000) == 6, "L8 XP100000 -> %d", bankedFromTable(tbl, 8, 100000));
        CHECK(bankedFromTable(tbl, 50, 0xFFFFFFFEu) == 0 && bankedFromTable(tbl, 0, 99999) == 0 && bankedFromTable(nullptr, 3, 9) == 0, "L50 / bad input");
        buildBankedText(b, sizeof b, "Atton", 1, "Jedi Sentinel");
        CHECK(!strcmp(b, "Atton: level banked - waiting for Jedi Sentinel (level-up path)"), "banked text 1: %s", b);
        buildBankedText(b, sizeof b, "Atton", 3, "Jedi Sentinel");
        CHECK(!strcmp(b, "Atton: 3 levels banked - waiting for Jedi Sentinel (level-up path)"), "banked text 3: %s", b);
    }

    // engine fakes
    g_eng.checkCode = false;
    g_eng.canLevelUp = reinterpret_cast<uintptr_t>(&fakeCanLevelUp);
    g_eng.strCtor = reinterpret_cast<uintptr_t>(&fakeStrCtor);
    g_eng.strDtor = reinterpret_cast<uintptr_t>(&fakeStrDtor);
    g_eng.guidFind = reinterpret_cast<uintptr_t>(&fakeGFind);
    g_eng.guidAdd = reinterpret_cast<uintptr_t>(&fakeGAdd);
    g_eng.guidSet = reinterpret_cast<uintptr_t>(&fakeGSet);
    g_eng.guidGet = reinterpret_cast<uintptr_t>(&fakeGGet);
    g_eng.serverInit = reinterpret_cast<uintptr_t>(&fakeServerInit);
    g_eng.bankedAdd = reinterpret_cast<uintptr_t>(&fakeBankedAdd);
    for (auto& sl : g_nameSlots) sl = arenaAlloc(256);
    for (auto& sl : g_exoSlots) sl = arenaAlloc(256);
    // app chain: [appGlobal] = app; [app+4] = client; [client+4] = clientInternal; [ci+0x40] = GuiInGame; [app+8] = srv; [srv+4] = intl
    uintptr_t cell = arenaAlloc(16), app = arenaAlloc(32), client = arenaAlloc(32), ci = arenaAlloc(0x100), gui = arenaAlloc(32), srv = arenaAlloc(32), intl = arenaAlloc(0x10200);
    at<uint32_t>(cell, 0) = (uint32_t)app; at<uint32_t>(app, 4) = (uint32_t)client; at<uint32_t>(client, 4) = (uint32_t)ci; at<uint32_t>(ci, 0x40) = (uint32_t)gui;
    at<uint32_t>(app, 8) = (uint32_t)srv; at<uint32_t>(srv, 4) = (uint32_t)intl;
    g_eng.appGlobal = cell;
    // rules object: XP table at +0x38, tbl[i] = XP to reach level i+1 = 500*i*(i+1)
    uintptr_t rulesCell = arenaAlloc(16), rules = arenaAlloc(0x200);
    at<uint32_t>(rulesCell, 0) = (uint32_t)rules;
    for (int i = 0; i < 50; ++i) at<uint32_t>(rules, 0x38 + 4 * i) = 500u * i * (i + 1);
    at<uint32_t>(rules, 0x38 + 4 * 50) = 0xFFFFFFFFu;
    g_eng.rulesGlobal = rulesCell;
    // GuiKit fakes
    uintptr_t screen = arenaAlloc(16), snd = arenaAlloc(16);   // screen size 0 -> cursor is engine px 1:1
    g_tipCell = snd; g_tipMgr = arenaAlloc(0x100);   // fake tooltip manager (GuiTip.h reads it through the GuiSoundMgr cell)
    at<uint32_t>(snd, 0) = (uint32_t)g_tipMgr; at<float>(g_tipMgr, 0x48) = 0.5f;
    guikit::Engine& e = g_gk;
    e.alloc = fAlloc; e.labelCtor = fCtor; e.initControl = fInit; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.textSet = fTextSet; e.setFont = fFont; e.setAlign = fAlign;
    e.textColor = fColor; e.listAdd = fList; e.loadImage = fLoadImage; e.playSound = fSound; e.callSetRect = fSetRect;
    e.screenWAddr = screen; e.screenHAddr = screen + 4; e.guiSoundMgrAddr = snd;
    e.cursorClient = fCursor; e.gameForeground = fFg; e.lbuttonDown = fLmb;

    const char* PRESET = "atton__melee_dual_light_early";
    Fake atton = makeCreature("Atton", {{2, 7}}, 0);
    Fake attonLo = makeCreature("Atton", {{2, 3}}, 0);      // Scoundrel 3: below the early preset's default pause (7)
    Fake attonNone = makeCreature("Atton", {}, 0);          // class list unreadable (count 0) -> curLevel -1
    Fake attonXp1 = makeCreature("Atton", {{2, 7}}, 30000);   // L7: tbl[7] = 28000 <= XP < 36000 -> 1 banked
    Fake sentinel = makeCreature("atton", {{2, 7}, {5, 0}}, 30000);
    Fake pc = makeCreature("", {{0, 7}}, 0);
    Fake nobody = makeCreature("zzz-nobody", {{6, 7}}, 0);
    const uintptr_t liveTbl = intl + 0x100FC;
    (void)liveTbl;

    g_missingTag["LBL_EFX_BACK"] = 1;   // Mod 14 absent unless a case says otherwise (batch 3c: the page back is then the 3-stack)
    // 1. panel ctor: creates (addToList 0), does not append; once per panel
    setIni("mode=rewrite", table);
    clearLog();
    uintptr_t pA = makePanel();
    LuoUiPanelCtor((int)pA);
    CHECK(rec(pA) && rec(pA)->nCreated == C_N && nCtor == C_N + 1 && nInit == C_N + 1, "(+1: the LBL_EFX_BACK window try) ctor created %d (nCtor %d nInit %d)", rec(pA) ? rec(pA)->nCreated : -1, nCtor, nInit);
    CHECK(nInitFramed == 20 && nInitPlain == 6, "templates framed %d plain %d (want 20/6: batch 3d C_GEAR is plain)", nInitFramed, nInitPlain);
    CHECK(g_addToListSeen == 0, "InitControl addToList must be 0");
    CHECK(at<int>(pA, Panel_CtlCount) == 70 && nList == 0, "ctor must not append (count %d, listAdd %d)", at<int>(pA, Panel_CtlCount), nList);
    { bool allHidden = true; for (int i = 0; i < C_N; ++i) if (isVisible(ctl(pA, i))) allHidden = false; CHECK(allHidden, "created hidden"); }
    CHECK(rec(pA)->fb.ok && rec(pA)->fb.sx == 2.f && rec(pA)->fb.sy == 2.f, "fallback scale from the ctor-time extent");
    CHECK(logHas("UI ctor panel="), "ctor log:\n%s", readLog().c_str());
    CHECK(rec(0x1234) == nullptr, "unknown panel has no record");

    // 2. first tick appends all controls once, in order; unconfigured atton: Configure button, no status
    CHECK(tick(pA, atton, 0) == 0, "no XP: real result passes through");
    CHECK(at<int>(pA, Panel_CtlCount) == 70 + C_N && nList == C_N, "first tick appends %d (count %d listAdd %d)", (int)C_N, at<int>(pA, Panel_CtlCount), nList);
    { bool ordered = true; uint32_t arr = at<uint32_t>(pA, Panel_CtlArray); for (int i = 0; i < C_N; ++i) if (at<uint32_t>(arr, (70 + i) * 4) != (uint32_t)ctl(pA, i) || at<int>(ctl(pA, i), Ctl_Id) != 70 + i) ordered = false; CHECK(ordered, "z-order / ids"); }
    CHECK(logCount("UI create panel=") == 1, "UI create logged %d times", logCount("UI create panel="));
    CHECK(!vis(pA, C_CFG) && vis(pA, C_ICON) && txt(pA, C_CFG) == "Configure" && !vis(pA, C_STATUS) && !vis(pA, C_BG0) && !vis(pA, C_TITLE) && !vis(pA, C_APPLY) && !vis(pA, C_CANCEL), "state: cfg=%d '%s'", vis(pA, C_CFG), txt(pA, C_CFG).c_str());
    for (int i = 0; i < 3; ++i) tick(pA, atton, 0);
    CHECK(at<int>(pA, Panel_CtlCount) == 70 + C_N && nList == C_N && logCount("UI create panel=") == 1, "no re-append on later ticks");
    { int x, y, w, h; getExtent(ctl(pA, C_CFG), &x, &y, &w, &h); CHECK(x == 724 && y == 744 && w == 188 && h == 52, "cfg rect %d,%d,%d,%d (want 724,744,188,52)", x, y, w, h);
      getExtent(ctl(pA, C_STATUS), &x, &y, &w, &h); CHECK(x == 108 && y == 948 && w == 700 && h == 40, "status rect %d,%d,%d,%d", x, y, w, h); }
    const int rects = nSetRect;
    tick(pA, atton, 0);
    CHECK(nSetRect == rects, "setRect only when the rect changed");
    // R9 nudge: unconfigured + real CanLevelUp -> text + highlight colour; back to normal when it cannot level
    tick(pA, atton, 1);
    CHECK(txt(pA, C_CFG) == "Plan level-ups" && nColor == 1 && lastColor[0] == 1.0f && std::fabs(lastColor[1] - 0.85f) < 1e-6f, "nudge text '%s' colour writes %d", txt(pA, C_CFG).c_str(), nColor);
    tick(pA, atton, 1);
    CHECK(nColor == 1, "colour written only on change");
    tick(pA, atton, 0);
    CHECK(txt(pA, C_CFG) == "Configure" && nColor == 2 && lastColor[0] == 0.f && lastColor[1] == 1.f, "nudge off restores the base colour");
    // batch 3a: level-up buttons shown (held result 1) -> Configure, no icon; the PC gets the filter button; a tag without presets: everything hidden
    tick(pA, atton, 1);
    CHECK(vis(pA, C_CFG) && !vis(pA, C_ICON), "held result 1: cfg %d icon %d", vis(pA, C_CFG), vis(pA, C_ICON));
    tick(pA, pc, 1);
    CHECK(vis(pA, C_CFG) && !vis(pA, C_ICON) && txt(pA, C_CFG) == "Level-up filter" && !vis(pA, C_STATUS) && !vis(pA, C_BG0), "PC leader: filter button '%s'", txt(pA, C_CFG).c_str());
    tick(pA, pc, 0);
    CHECK(!vis(pA, C_CFG) && vis(pA, C_ICON), "PC, no level-up: icon only");
    tick(pA, nobody, 1);
    CHECK(!anyVisible(pA), "tag without presets: all hidden");
    Fake garbage = makeCreature("x", {{0, 1}});
    tick(pA, atton, 0);
    CHECK(vis(pA, C_ICON) && !vis(pA, C_CFG), "atton shown again");
    CHECK(LuoHold_84F221(P(0x20), P(g_frames[pA])) == 0 && !anyVisible(pA), "unreadable stats: hidden, no crash");
    CHECK(LuoHold_84F221(P(atton.stats), nullptr) == 0 && LuoHold_84F221(P(atton.stats), P(arenaAlloc(0x400) + 0x300)) == 0, "no frame / empty panel slot: no tick, no crash");
    (void)garbage;

    // 3. a new panel pointer creates anew; first tick appends its own controls
    const int ctorsBefore = nCtor, listBefore = nList;
    uintptr_t pB = makePanel();
    LuoUiPanelCtor((int)pB);
    CHECK(nCtor == ctorsBefore + C_N + 1 && rec(pB) && rec(pB) != rec(pA), "new panel creates anew");
    CHECK(nList == listBefore, "ctor of the second panel does not append");
    tick(pB, atton, 0);
    CHECK(nList == listBefore + C_N && at<int>(pB, Panel_CtlCount) == 70 + C_N && at<int>(pA, Panel_CtlCount) == 70 + C_N, "second panel appended once, first untouched");
    CHECK(logCount("UI create panel=") == 2, "UI create once per panel");
    // fallback scale when BTN_LEVELUP is missing / implausible
    uintptr_t pC = makePanel(188, 80, false), pD = makePanel(100, 100, true);
    LuoUiPanelCtor((int)pC); LuoUiPanelCtor((int)pD);
    tick(pC, atton, 0); tick(pD, atton, 0);
    { int x, y, w, h; bool ok = getExtent(ctl(pC, C_CFG), &x, &y, &w, &h) && x == 724 && y == 744 && w == 188 && h == 52; ok = ok && getExtent(ctl(pD, C_CFG), &x, &y, &w, &h) && x == 724 && y == 744 && w == 188 && h == 52;
      CHECK(ok, "fallback scale placement (no BTN / bad aspect) last %d,%d,%d,%d", x, y, w, h); }
    // a 5th panel recycles the oldest record (4 slots)
    uintptr_t pE = makePanel();
    LuoUiPanelCtor((int)pE);
    CHECK(rec(pE) && !rec(pA), "4 records round robin: the oldest is recycled");
    // a missing framed template falls back to the plain one
    g_missingTag["LBL_XP_BACK"] = 1;
    uintptr_t pF = makePanel();
    const int plainBefore = nInitPlain;
    LuoUiPanelCtor((int)pF);
    CHECK(rec(pF) && rec(pF)->nCreated == C_N && nInitPlain == plainBefore + C_N, "framed -> plain fallback (%d created)", rec(pF) ? rec(pF)->nCreated : -1);
    g_missingTag.erase("LBL_XP_BACK");
    // mode=off hides everything
    uintptr_t pG = makePanel();
    LuoUiPanelCtor((int)pG);
    tick(pG, atton, 0);
    CHECK(vis(pG, C_ICON), "pG shown");
    setIni("mode=off", table);
    tick(pG, atton, 0);
    CHECK(!anyVisible(pG), "mode=off: hidden");
    setIni("mode=rewrite", table);


    // 4. options page through simulated clicks on panel pE (fresh: first tick appends)
    uintptr_t pW = pE;
    tick(pW, atton, 0);
    CHECK(vis(pW, C_ICON) && !vis(pW, C_CFG), "pW icon visible (no level-up: no Configure button)");
    const int sounds = nSound;
    clearLog();
    clickCtl(pW, atton, 0, C_CFG);
    CHECK(nSound == sounds + 1, "hover sound once per enter (%d)", nSound - sounds);
    CHECK(rec(pW)->open && logHas("UI click icon tag=Atton row=-") && logHas("UI page open tag=Atton start=atton__melee_dual_light_early (recommended)"), "cfg click opens the page:\n%s", readLog().c_str());
    {
        UiPanel* r = rec(pW);
        const Preset* rp = findPreset(PRESET);
        CHECK(r->nItems == 7 && !strcmp(r->items[r->os.cur].id, PRESET) && r->recItem == r->os.cur, "starts on the recommended preset (%d items, cur %d)", r->nItems, r->os.cur);
        CHECK(!vis(pW, C_CFG) && !vis(pW, C_STATUS) && vis(pW, C_BG0) && vis(pW, C_BG1) && vis(pW, C_BG2) && vis(pW, C_TITLE) && vis(pW, C_BUILD) && vis(pW, C_INSTANT) && vis(pW, C_BANK) && vis(pW, C_PAUSE) && vis(pW, C_APPLY) && vis(pW, C_CANCEL),
              "page visibility");
        CHECK(!vis(pW, C_PMINUS) && !vis(pW, C_PPLUS), "Atton is level 7 = the default pause: no explicit range, both buttons hidden");
        CHECK(!vis(pW, C_REC) && !vis(pW, C_VANILLA), "no 'Use recommended' on the recommended preset, no Vanilla while unconfigured");
        CHECK(txt(pW, C_TITLE) == "Atton Rand - recommended build", "title '%s'", txt(pW, C_TITLE).c_str());
        CHECK(txt(pW, C_BUILD) == rp->shortName && txt(pW, C_INSTANT) == "Instant: Off" && txt(pW, C_BANK) == "Bank for Jedi: On" && txt(pW, C_PAUSE) == "Pause at level 7" && txt(pW, C_APPLY) == "Apply" && txt(pW, C_CANCEL) == "Cancel",
              "build '%s' instant '%s' bank '%s'", txt(pW, C_BUILD).c_str(), txt(pW, C_INSTANT).c_str(), txt(pW, C_BANK).c_str());
        CHECK(txt(pW, C_ROW0 + 0) == "Jedi: Early - Jedi Sentinel at 8", "convert row '%s'", txt(pW, C_ROW0 + 0).c_str());
        CHECK(txt(pW, C_ROW0 + 1) == std::string("Role: ") + wiz::answerLabel(wiz::Role, rp->role) && txt(pW, C_ROW0 + 2) == std::string("Weapons: ") + wiz::answerLabel(wiz::Style, rp->style) &&
              txt(pW, C_ROW0 + 3) == std::string("Powers: ") + wiz::answerLabel(wiz::Alignment, rp->alignment) && txt(pW, C_ROW0 + 4) == std::string("Skills: ") + wiz::answerLabel(wiz::Skills, rp->skills),
              "row texts '%s' '%s' '%s' '%s'", txt(pW, C_ROW0 + 1).c_str(), txt(pW, C_ROW0 + 2).c_str(), txt(pW, C_ROW0 + 3).c_str(), txt(pW, C_ROW0 + 4).c_str());
        // geometry: every page control inside the stats column (units x 54..300, y 134..470 -> px at scale 2); bg labels carry the opaque fill
        for (int i = C_BG0; i <= C_CANCEL; ++i) {
            int x, y, w, h;
            getExtent(ctl(pW, i), &x, &y, &w, &h);
            CHECK(x >= 108 && x + w <= 600 && y >= 268 && y + h <= 940, "ctl %s px rect %d,%d,%d,%d outside x 108..600 y 268..940", kUiName[i], x, y, w, h);
        }
        for (int b = 0; b < NumBg; ++b) {
            CHECK(at<uint32_t>(ctl(pW, C_BG0 + b), Label_Border + Border_Fill) != 0 && at<float>(ctl(pW, C_BG0 + b), Label_Border + Border_Alpha) == 1.0f, "bg%d fill + alpha 1", b);
            int x, y, w, h; getExtent(ctl(pW, C_BG0 + b), &x, &y, &w, &h);
            CHECK(x == 108 && y == 268 && w == 492 && h == 672, "bg%d rect %d,%d,%d,%d", b, x, y, w, h);
        }
        // info labels are not clickable
        { int x, y, w, h; getExtent(ctl(pW, C_TITLE), &x, &y, &w, &h); curX = x + 5; curY = y + 5; lmb = false; tick(pW, atton, 0); lmb = true; tick(pW, atton, 0); lmb = false; tick(pW, atton, 0);
          CHECK(r->open && logCount("UI click") == 1, "title click ignored"); }
        // press from elsewhere dragged onto a button does not fire (no rising edge over it)
        { int x, y, w, h; getExtent(ctl(pW, C_INSTANT), &x, &y, &w, &h); curX = 0; curY = 0; lmb = true; tick(pW, atton, 0); curX = x + 5; curY = y + 5; tick(pW, atton, 0); lmb = false; tick(pW, atton, 0);
          CHECK(!r->instant, "drag-in press must not click"); }
        { fg = false; clickCtl(pW, atton, 0, C_INSTANT); fg = true; CHECK(!r->instant, "no click while the game is not foreground"); }
        // rows cycle exactly like opt::cycle (parallel state), twice around; texts, build line and compact positions follow
        opt::State par; opt::init(par, r->items, r->nItems, r->os.cur);
        int clicks = 0;
        for (int round = 0; round < 2; ++round)
            for (int row = 0; row < NumRows; ++row) {
                if (!opt::shown(r->os, row)) continue;
                clickCtl(pW, atton, 0, C_ROW0 + row);
                opt::cycle(par, row, +1);
                ++clicks;
                const Preset& cp = g_presets[r->itemPreset[r->os.cur]];
                CHECK(r->os.cur == par.cur, "row %s click -> cur %d, opt::cycle -> %d", opt::rowName(row), r->os.cur, par.cur);
                CHECK(txt(pW, C_BUILD) == cp.shortName, "build line '%s' vs '%s'", txt(pW, C_BUILD).c_str(), cp.shortName);
                char b[UiText];
                int k = 0;
                for (int q = 0; q < NumRows; ++q) {
                    const bool sh = opt::shown(par, q);
                    CHECK(vis(pW, C_ROW0 + q) == sh, "row %d visibility %d vs %d", q, vis(pW, C_ROW0 + q), sh);
                    if (!sh) continue;
                    int x, y, w, h; getExtent(ctl(pW, C_ROW0 + q), &x, &y, &w, &h);
                    CHECK(txt(pW, C_ROW0 + q) == uiRowText(cp, q, b, sizeof b) && y == 2 * (162 + 28 * k) && h == 48, "row %d text '%s' y %d (slot %d)", q, txt(pW, C_ROW0 + q).c_str(), y, k);
                    ++k;
                }
                CHECK(vis(pW, C_REC) == (r->os.cur != r->recItem), "recommended button visibility");
                CHECK(vis(pW, C_BANK) == (cp.holdClass != 0), "bank visibility");
                CHECK(txt(pW, C_TITLE) == std::string(cp.name) + (cp.recommended ? " - recommended build" : " - level-up path"), "title '%s'", txt(pW, C_TITLE).c_str());
            }
        CHECK(clicks >= 8, "cycled %d rows", clicks);
        // "Use recommended": jumps back, keeps toggles
        if (r->os.cur == r->recItem) clickCtl(pW, atton, 0, C_ROW0 + 0 + (opt::shown(r->os, 0) ? 0 : 1));
        CHECK(r->os.cur != r->recItem && vis(pW, C_REC), "off the recommended preset: button shown");
        clickCtl(pW, atton, 0, C_INSTANT);
        CHECK(r->instant && txt(pW, C_INSTANT) == "Instant: On", "instant toggled '%s'", txt(pW, C_INSTANT).c_str());
        clickCtl(pW, atton, 0, C_REC);
        CHECK(r->os.cur == r->recItem && !vis(pW, C_REC) && r->instant && txt(pW, C_TITLE) == "Atton Rand - recommended build", "Use recommended jumps back (cur %d), toggles kept", r->os.cur);
        // bank toggle
        // Bank is a plain On/Off toggle; the pause row (label + "-" / "+") shows while it is On
        {
            const int def = rp->convertAt - 1, mn = uiMinPause(*rp);
            CHECK(rp->convertAt == 8 && def == 7 && mn >= 1 && mn <= 6, "atton early preset: convertAt %d minPause %d", rp->convertAt, mn);
            CHECK(r->bank && r->pause == 0 && txt(pW, C_BANK) == "Bank for Jedi: On" && txt(pW, C_PAUSE) == "Pause at level 7" && vis(pW, C_PAUSE), "page opens on bank on / default (level 7 = hi)");
            CHECK(txt(pW, C_BANK).size() <= 18 && txt(pW, C_PAUSE).size() <= 18, "texts <= 18 chars");
            { const int before = logCount("UI click"); clickCtl(pW, atton, 0, C_PMINUS); clickCtl(pW, atton, 0, C_PPLUS); CHECK(logCount("UI click") == before && r->pause == 0, "hidden -/+ ignored"); }
            clickCtl(pW, atton, 0, C_BANK);
            CHECK(!r->bank && r->pause == 0 && txt(pW, C_BANK) == "Bank for Jedi: Off" && !vis(pW, C_PAUSE) && !vis(pW, C_PMINUS) && !vis(pW, C_PPLUS), "toggle -> Off hides the pause row '%s'", txt(pW, C_BANK).c_str());
            clickCtl(pW, atton, 0, C_BANK);
            CHECK(r->bank && r->pause == 0 && txt(pW, C_BANK) == "Bank for Jedi: On" && vis(pW, C_PAUSE), "Off -> On (current level 7 = hi -> default)");
            clickCtl(pW, atton, 0, C_BANK);
            CHECK(!r->bank && txt(pW, C_BANK) == "Bank for Jedi: Off", "bank off");
        }
        // Cancel closes
        clickCtl(pW, atton, 0, C_CANCEL);
        CHECK(!r->open && !vis(pW, C_BG0) && !vis(pW, C_APPLY) && vis(pW, C_ICON), "cancel closes");
        // reopen -> toggles reset; pick a non-recommended preset, Apply writes the line with the toggles
        clickCtl(pW, atton, 0, C_CFG);
        CHECK(r->open && r->instant == false && r->bank == true && r->pause == 0, "reopen resets toggles for an unconfigured tag");
        clickCtl(pW, atton, 0, C_ROW0 + 1);   // Role cycles (atton has several roles)
        CHECK(r->os.cur != r->recItem, "moved off the recommended preset");
        clickCtl(pW, atton, 0, C_INSTANT);
        for (int i = 0; i < 2 && r->bank; ++i) clickCtl(pW, atton, 0, C_BANK);   // toggle Off
        const std::string id = r->items[r->os.cur].id;
        remove("levelup_playthroughs.ini");
        CHECK(tagConfig("atton") == nullptr, "unconfigured before Apply");
        clearLog();
        clickCtl(pW, atton, 0, C_APPLY);
        CHECK(!r->open && !vis(pW, C_BG0), "page closed after Apply");
        CHECK(ptGuid() != 0 && logHas("UI apply tag=Atton preset=") && logHas("instant=1 hold=0 pause=0 ok=1"), "apply log:\n%s", readLog().c_str());
        CHECK(g_applyNow.pending && !_stricmp(g_applyNow.tag, "Atton"), "Apply with instant=1 records the apply-now request (consumed later, outside SetStats)");
        const std::string ini = fileText("levelup_playthroughs.ini");
        CHECK(ini.find("atton=" + id + "|instant=1|hold=0") != std::string::npos, "ini line:\n%s", ini.c_str());
        { const TagCfg* c = tagConfig("atton"); CHECK(c && c->instant && !c->hold && c->preset >= 0 && g_presets[c->preset].id == id, "tagConfig switched"); }
        tick(pW, atton, 0);
        CHECK(txt(pW, C_CFG) == "Change path" && !vis(pW, C_CFG) && vis(pW, C_ICON) && vis(pW, C_STATUS), "cfg text after apply '%s'", txt(pW, C_CFG).c_str());
        // configured: opens on the configured preset with the config's toggles; Vanilla and Use recommended are offered
        clearLog();
        clickCtl(pW, atton, 0, C_CFG);
        CHECK(r->open && r->items[r->os.cur].id == id && logHas(("UI page open tag=Atton start=" + id + " (configured)").c_str()), "reopen on the configured preset:\n%s", readLog().c_str());
        CHECK(r->instant && !r->bank && txt(pW, C_INSTANT) == "Instant: On" && txt(pW, C_BANK) == "Bank for Jedi: Off" && !vis(pW, C_PAUSE), "toggles from the config");
        CHECK(vis(pW, C_VANILLA) && txt(pW, C_VANILLA) == "Vanilla" && vis(pW, C_REC) && txt(pW, C_TITLE).find("level-up path") != std::string::npos, "vanilla + recommended offered");
        // a preset without a hold class: bank toggle hidden, Apply writes hold=1 whatever the (hidden) toggle says
        for (int i = 0; i < 4 && g_presets[r->itemPreset[r->os.cur]].holdClass != 0; ++i) clickCtl(pW, atton, 0, C_ROW0 + 0);
        CHECK(g_presets[r->itemPreset[r->os.cur]].holdClass == 0 && !vis(pW, C_BANK) && !vis(pW, C_PAUSE) && !vis(pW, C_PMINUS) && !vis(pW, C_PPLUS) && !r->bank, "convert row reached a no-Jedi preset (bank + pause row hidden, toggle still Off)");
        clearLog();
        clickCtl(pW, atton, 0, C_APPLY);
        CHECK(!r->open && logHas("hold=1 pause=0 ok=1") && fileText("levelup_playthroughs.ini").find("|hold=1") != std::string::npos, "hold-less preset applies hold=1:\n%s", readLog().c_str());
        clickCtl(pW, atton, 0, C_CFG);
        clickCtl(pW, atton, 0, C_ROW0 + 1);
        clickCtl(pW, atton, 0, C_CANCEL);
        CHECK(!r->open && tagConfig("atton") != nullptr, "cancel leaves the config alone");
        clickCtl(pW, atton, 0, C_CFG);
        clearLog();
        clickCtl(pW, atton, 0, C_VANILLA);
        CHECK(!r->open && logHas("UI click vanilla tag=Atton row=-") && logHas("UI vanilla tag=Atton ok=1"), "vanilla log:\n%s", readLog().c_str());
        CHECK(fileText("levelup_playthroughs.ini").find("atton=") == std::string::npos && tagConfig("atton") == nullptr, "line removed, tag unconfigured");
        tick(pW, atton, 0);
        CHECK(txt(pW, C_CFG) == "Configure" && !vis(pW, C_STATUS), "cfg back to Configure, status hidden");
        // 0.3.1 pause: apply payload, reopen from config, clamps on row cycle / Use recommended / stale config
        {
            const Preset* ep = findPreset(PRESET);
            const int mn = uiMinPause(*ep), lo = uiPauseLo(*ep, 3);   // attonLo is level 3
            CHECK(lo >= 3 && lo <= 5, "lo %d", lo);
            clickCtl(pW, attonLo, 0, C_CFG);   // opens on the recommended (early) preset: bank on, pause = the current level (lo)
            CHECK(r->open && r->bank && r->pause == lo && r->itemPreset[r->os.cur] == (int)(ep - g_presets), "pause block: page on the early preset");
            for (int k = lo; k < 5; ++k) clickCtl(pW, attonLo, 0, C_PPLUS);   // lo -> 5
            CHECK(r->bank && r->pause == 5 && txt(pW, C_PAUSE) == "Pause at level 5", "pause 5 '%s'", txt(pW, C_PAUSE).c_str());
            clearLog(); g_applyNow.pending = false;
            clickCtl(pW, attonLo, 0, C_APPLY);
            CHECK(!r->open && logHas("UI apply tag=Atton preset=atton__melee_dual_light_early instant=0 hold=1 pause=5 ok=1"), "apply pause=5 log:\n%s", readLog().c_str());
            CHECK(!g_applyNow.pending, "Apply with instant=0 records no apply-now request");
            CHECK(fileText("levelup_playthroughs.ini").find("atton=atton__melee_dual_light_early|instant=0|hold=1|pause=5\n") != std::string::npos, "ini |pause=5:\n%s", fileText("levelup_playthroughs.ini").c_str());
            { const TagCfg* c = tagConfig("atton"); CHECK(c && c->pause == 5 && c->hold, "tagConfig pause 5"); }
            // held-by-pause status line: Scoundrel 5 -> level 6 banked once past pause 5 (Scoundrel 5 = key 6 > 5)
            clickCtl(pW, attonLo, 0, C_CFG);
            CHECK(r->open && r->bank && r->pause == 5 && txt(pW, C_PAUSE) == "Pause at level 5", "reopen from config shows pause 5 '%s'", txt(pW, C_PAUSE).c_str());
            // cycle the Convert row (early -> late -> never ...): pause is clamped into each preset's range, 0 without a hold class
            int expP = 5;   // once clamped to 0 (different range) the value stays 0
            for (int i = 0; i < 4; ++i) {
                clickCtl(pW, attonLo, 0, C_ROW0 + 0);
                const Preset& cp = g_presets[r->itemPreset[r->os.cur]];
                expP = uiNormPause(cp, expP, 3);
                CHECK(r->pause == expP && (cp.holdClass == 0 ? r->pause == 0 : true) && (r->pause == 0 || (r->pause >= uiPauseLo(cp, 3) && r->pause <= cp.convertAt - 2)),
                      "row cycle %d: pause %d on %s (convertAt %d hold %d)", i, r->pause, cp.id, cp.convertAt, cp.holdClass);
                { const bool pz = cp.holdClass && cp.convertAt > 0 && r->bank;
                  CHECK(vis(pW, C_PAUSE) == pz, "pause row visibility on %s", cp.id);
                  if (pz) { char want[24]; snprintf(want, sizeof want, "Pause at level %d", r->pause > 0 ? r->pause : cp.convertAt - 1);
                    CHECK(txt(pW, C_PAUSE) == want, "pause text '%s' vs '%s'", txt(pW, C_PAUSE).c_str(), want); } }
            }
            // forced clamp: a preset with convertAt 5 cannot keep pause 5 (range ends at 3)
            { Preset fake = *ep; fake.convertAt = 5;
              CHECK(uiClampPause(fake, 5) == 0 && uiClampPause(fake, 4) == 0 && uiClampPause(fake, 3) == (3 >= uiMinPause(fake) ? 3 : 0), "uiClampPause range end convertAt-2");
              CHECK(uiClampPause(*ep, 0) == 0 && uiClampPause(*ep, -1) == 0 && uiClampPause(*ep, ep->convertAt - 1) == 0 && uiClampPause(*ep, ep->convertAt) == 0 && uiClampPause(*ep, mn - 1) == 0 && uiClampPause(*ep, mn) == mn && uiClampPause(*ep, ep->convertAt - 2) == ep->convertAt - 2, "uiClampPause bounds");
              fake.holdClass = 0; CHECK(uiClampPause(fake, 4) == 0, "no hold class -> 0");
              bool bk = true; int pz = 0; Preset tiny = *ep; tiny.convertAt = mn + 1;   // range empty: default -> Off -> default
              uiBankToggle(tiny, &bk, &pz, 3); CHECK(!bk && pz == 0, "empty range: On -> Off");
              uiBankToggle(tiny, &bk, &pz, 3); CHECK(bk && pz == 0, "Off -> On: lo >= hi -> default");
              char tb[24]; snprintf(tb, sizeof tb, "Pause at level %d", tiny.convertAt - 1); char tb2[24]; CHECK(!strcmp(uiPauseText(tiny, 0, tb2, sizeof tb2), tb), "default text"); }
            // A3 mutation gaps: absolute (non-tautological) expectations on synthetic presets
            { Preset sp = *ep; for (int cl = 0; cl <= MaxLevel; ++cl) sp.lv[cl].present = false;
              sp.holdClass = 5; sp.convertAt = 10;
              sp.lv[4].present = true; CHECK(uiMinPause(sp) == 3, "uiMinPause first record at char level 4 -> 3 (got %d)", uiMinPause(sp));
              sp.lv[4].present = false; sp.lv[2].present = true; CHECK(uiMinPause(sp) == 1, "uiMinPause first record at level 2 -> 1 (got %d)", uiMinPause(sp));
              sp.lv[2].present = false; sp.lv[1].present = true; CHECK(uiMinPause(sp) == 1, "uiMinPause first record at level 1 -> 1");
              CHECK(uiCanPause(sp), "holdClass + convertAt -> can pause");
              Preset a = sp; a.holdClass = 0; CHECK(!uiCanPause(a), "no hold class -> cannot pause (convertAt set)");
              Preset b = sp; b.convertAt = 0; CHECK(!uiCanPause(b), "no convertAt -> cannot pause (hold class set)");
              bool bk = true; int pz = 6; uiBankToggle(a, &bk, &pz, -1); CHECK(!bk && pz == 0, "non-pausable toggle: On -> Off, stale pause cleared (pause %d)", pz);
              bk = false; pz = 6; uiBankToggle(a, &bk, &pz, -1); CHECK(bk && pz == 0, "non-pausable toggle: Off -> On, stale pause cleared (pause %d)", pz);
              bk = false; pz = 4; uiBankToggle(sp, &bk, &pz, -1); CHECK(bk && pz == 0, "Off -> On with an unknown level clears the pause");
              bk = false; pz = 4; uiBankToggle(sp, &bk, &pz, 5); CHECK(bk && pz == 5, "Off -> On starts at the current level (pause %d)", pz);
              bk = false; pz = 4; uiBankToggle(sp, &bk, &pz, 9); CHECK(bk && pz == 0, "Off -> On at the default level: no explicit pause (pause %d)", pz);
              bk = true; pz = 5; uiBankToggle(sp, &bk, &pz, 5); CHECK(!bk && pz == 0, "On -> Off clears the pause");
              CHECK(uiPauseLo(sp, 5) == 5 && uiPauseLo(sp, -1) == 1 && uiPauseLo(sp, 0) == 1 && uiPauseHi(sp) == 9, "uiPauseLo/Hi absolute");
              CHECK(uiClampPause(sp, 4, 5) == 0 && uiClampPause(sp, 5, 5) == 5 && uiClampPause(sp, 8, 5) == 8 && uiClampPause(sp, 9, 5) == 0 && uiClampPause(sp, 4, -1) == 4, "uiClampPause lower bound = lo");
              CHECK(uiNormPause(sp, 4, 6) == 6 && uiNormPause(sp, 0, 6) == 0 && uiNormPause(sp, 9, 6) == 0 && uiNormPause(sp, 7, 6) == 7 && uiNormPause(sp, 4, 9) == 0 && uiNormPause(sp, 4, 12) == 0, "uiNormPause raises below lo, drops at hi");
              int pp = 0; uiPauseStep(sp, &pp, +1, 5); CHECK(pp == 0, "+ at the default: nothing");
              uiPauseStep(sp, &pp, -1, 5); CHECK(pp == 8, "- from the default -> hi-1");
              uiPauseStep(sp, &pp, +1, 5); CHECK(pp == 0, "+ reaching hi stores 0");
              pp = 5; uiPauseStep(sp, &pp, -1, 5); CHECK(pp == 5, "- stops at lo");
              pp = 5; uiPauseStep(sp, &pp, +1, 5); CHECK(pp == 6, "+ raises by one");
              CHECK(!uiPauseMinusShown(sp, 5, 5) && uiPauseMinusShown(sp, 6, 5) && uiPauseMinusShown(sp, 0, 5) && uiPausePlusShown(sp, 5) && !uiPausePlusShown(sp, 0) && !uiPauseMinusShown(sp, 0, 9) && !uiPausePlusShown(sp, 0), "-/+ visibility at the limits"); }
            // Use recommended keeps the toggles and clamps
            clickCtl(pW, attonLo, 0, C_CANCEL);
            clickCtl(pW, attonLo, 0, C_CFG);
            CHECK(r->pause == 5, "reopen: pause 5 again");
            for (int i = 0; i < 4 && r->os.cur == r->recItem; ++i) clickCtl(pW, attonLo, 0, C_ROW0 + 0);
            clickCtl(pW, attonLo, 0, C_REC);
            CHECK(r->os.cur == r->recItem && r->bank && r->pause == uiClampPause(*ep, r->pause, 3), "Use recommended keeps the bank state (pause %d)", r->pause);
            // Off applies hold=0 pause=0 and drops the field
            while (r->bank) clickCtl(pW, attonLo, 0, C_BANK);
            clearLog();
            clickCtl(pW, attonLo, 0, C_APPLY);
            CHECK(logHas("hold=0 pause=0 ok=1") && fileText("levelup_playthroughs.ini").find("pause") == std::string::npos, "bank off: pause dropped:\n%s", fileText("levelup_playthroughs.ini").c_str());
            // Apply with bank OFF never stores a pause, even when a stale config row (hold=0, pause=N) opened the page with one
            { CHECK(ptApply("atton", ep->id, false, false, mn), "ptApply hold=0 with a pause");
              clickCtl(pW, attonLo, 0, C_CFG);
              CHECK(r->open && !r->bank && r->pause == uiNormPause(*ep, mn, 3), "stale hold=0 pause=%d opens bank off with the pause (bank %d pause %d)", uiNormPause(*ep, mn, 3), r->bank, r->pause);
              clearLog(); clickCtl(pW, attonLo, 0, C_APPLY);
              CHECK(logHas("hold=0 pause=0 ok=1") && fileText("levelup_playthroughs.ini").find("pause") == std::string::npos, "Apply with bank off drops the stale pause:\n%s", readLog().c_str());
              }
            // stale / out-of-range config (hand-edited |pause=99) is clamped on page open and never applied raw
            CHECK(ptApply("atton", PRESET, false, true, 99), "ptApply pause 99");
            clickCtl(pW, attonLo, 0, C_CFG);
            CHECK(r->bank && r->pause == 0 && txt(pW, C_PAUSE) == "Pause at level 7", "pause 99 clamped on open '%s'", txt(pW, C_PAUSE).c_str());
            clickCtl(pW, attonLo, 0, C_CANCEL);
            CHECK(ptRemove("atton") && tagConfig("atton") == nullptr, "pause block cleanup");
            tick(pW, atton, 0);
        }
        // Task 1: the pause range starts at the creature's current level
        {
            const Preset* ep = findPreset(PRESET);
            const int hi = ep->convertAt - 1, mn = uiMinPause(*ep), lo = uiPauseLo(*ep, 3);
            CHECK(hi == 7 && lo == (mn > 3 ? mn : 3) && uiPauseLo(*ep, -1) == mn && uiPauseLo(*ep, 99) == 99 && uiPauseHi(*ep) == 7, "pure range helpers (mn %d lo %d)", mn, lo);
            // default = current level; "-" hidden at lo
            clearLog();
            clickCtl(pW, attonLo, 0, C_CFG);
            { char want[40]; snprintf(want, sizeof want, "UI pause lo=%d hi=7 cur=3", lo);
              CHECK(r->open && r->bank && r->pause == lo && vis(pW, C_PAUSE) && vis(pW, C_PPLUS) && !vis(pW, C_PMINUS) && logHas(want), "unconfigured: bank on, pause = current level %d (pause %d)\n%s", lo, r->pause, readLog().c_str()); }
            // "+" up to hi: reaching hi stores 0
            for (int v = lo; v < hi; ++v) {
                clickCtl(pW, attonLo, 0, C_PPLUS);
                const int want = v + 1 >= hi ? 0 : v + 1;
                CHECK(r->pause == want, "+ from %d -> pause %d (want %d)", v, r->pause, want);
            }
            CHECK(r->pause == 0 && txt(pW, C_PAUSE) == "Pause at level 7" && !vis(pW, C_PPLUS) && vis(pW, C_PMINUS), "at hi: stored 0, '+' hidden");
            // "-" down to lo, then it stops
            for (int v = hi - 1; v >= lo; --v) {
                clickCtl(pW, attonLo, 0, C_PMINUS);
                char want[24]; snprintf(want, sizeof want, "Pause at level %d", v);
                CHECK(r->pause == v && txt(pW, C_PAUSE) == want, "- -> %d: '%s' pause %d", v, txt(pW, C_PAUSE).c_str(), r->pause);
            }
            CHECK(!vis(pW, C_PMINUS) && r->pause == lo, "'-' hidden at lo");
            { const int before = logCount("UI click"); clickCtl(pW, attonLo, 0, C_PMINUS); CHECK(logCount("UI click") == before && r->pause == lo, "hidden '-' ignored"); }
            // Bank Off -> On restarts at the current level
            clickCtl(pW, attonLo, 0, C_PPLUS);
            clickCtl(pW, attonLo, 0, C_BANK); CHECK(!r->bank && !vis(pW, C_PAUSE) && !vis(pW, C_PMINUS) && !vis(pW, C_PPLUS), "Off hides the pause row");
            clickCtl(pW, attonLo, 0, C_BANK); CHECK(r->bank && r->pause == lo, "On again -> current level (pause %d)", r->pause);
            clickCtl(pW, attonLo, 0, C_CANCEL);
            // configured below the current level: raised to lo; a configured pause above lo is kept
            CHECK(ptApply("atton", PRESET, false, true, lo - 1), "ptApply pause below lo");
            clickCtl(pW, attonLo, 0, C_CFG);
            CHECK(r->open && r->bank && r->pause == lo && txt(pW, C_PAUSE) == "Pause at level " + std::to_string(lo), "configured pause %d raised to lo %d (pause %d)", lo - 1, lo, r->pause);
            clickCtl(pW, attonLo, 0, C_CANCEL);
            CHECK(ptApply("atton", PRESET, false, true, 6), "ptApply pause 6");
            clickCtl(pW, attonLo, 0, C_CFG);
            CHECK(r->pause == 6, "configured pause 6 kept (pause %d)", r->pause);
            // preset switch re-clamps into the NEW range (never silently zeroed while it still fits)
            { int kept = 0, expect = 6;
              for (int i = 0; i < 4; ++i) {
                  clickCtl(pW, attonLo, 0, C_ROW0 + 0);
                  const Preset& cp = g_presets[r->itemPreset[r->os.cur]];
                  const int w = expect = uiNormPause(cp, expect, 3);   // once dropped to 0 (default) it stays 0
                  CHECK(r->pause == w, "switch %d: pause %d want %d on %s", i, r->pause, w, cp.id);
                  if (r->pause > 0) { ++kept; CHECK(r->pause >= uiPauseLo(cp, 3) && r->pause < uiPauseHi(cp), "re-clamped into [lo,hi)"); }
                  if (!uiCanPause(cp)) CHECK(r->pause == 0 && !vis(pW, C_PAUSE), "no pause range -> 0, row hidden");
              }
              CHECK(kept >= 1, "some other preset kept a non-zero pause"); }
            clickCtl(pW, attonLo, 0, C_CANCEL);
            CHECK(ptRemove("atton"), "task1 cleanup");
            // current level unreadable (-1) = old behaviour: default pause (0), range down to minPause
            tick(pW, attonNone, 0);
            clickCtl(pW, attonNone, 0, C_CFG);
            CHECK(r->open && r->curLevel == -1 && r->bank && r->pause == 0 && txt(pW, C_PAUSE) == "Pause at level 7" && vis(pW, C_PMINUS) && !vis(pW, C_PPLUS), "cur -1: default pause, '-' available (pause %d)", r->pause);
            for (int v = hi - 1; v >= mn; --v) clickCtl(pW, attonNone, 0, C_PMINUS);
            CHECK(r->pause == mn && !vis(pW, C_PMINUS), "cur -1: '-' reaches minPause %d (pause %d)", mn, r->pause);
            // unconfigured non-Jedi preset: no pause row
            for (int i = 0; i < 4 && uiCanPause(g_presets[r->itemPreset[r->os.cur]]); ++i) clickCtl(pW, attonLo, 0, C_ROW0 + 0);
            CHECK(!uiCanPause(g_presets[r->itemPreset[r->os.cur]]) && !vis(pW, C_PAUSE) && !vis(pW, C_PMINUS) && !vis(pW, C_PPLUS), "non-Jedi preset: no pause row");
            clickCtl(pW, attonLo, 0, C_CANCEL);
            tick(pW, atton, 0);
        }
        // leader change closes the page
        clickCtl(pW, atton, 0, C_CFG);
        CHECK(r->open, "open again");
        Fake other = makeCreature("kreia", {{4, 3}});
        tick(pW, other, 0);
        CHECK(!r->open && !vis(pW, C_BG0) && txt(pW, C_CFG) == "Configure", "leader change closes the page");
        // Kreia: no Convert row (compact), no bank toggle
        clearLog();
        clickCtl(pW, other, 0, C_CFG);
        CHECK(r->open && !strcmp(r->pageTag, "kreia") && r->nItems == 7 && logHas("start=kreia__support_neutral (recommended)"), "kreia page: 7 selectable presets (not the __live one): %d\n%s", r->nItems, readLog().c_str());
        CHECK(!opt::shown(r->os, 0) && !vis(pW, C_ROW0 + 0) && !vis(pW, C_BANK) && vis(pW, C_INSTANT), "kreia: no Convert row, no bank toggle");
        CHECK(!vis(pW, C_PAUSE) && !vis(pW, C_PMINUS) && !vis(pW, C_PPLUS), "kreia: no pause row");
        { int first = -1; for (int q = 0; q < NumRows && first < 0; ++q) if (vis(pW, C_ROW0 + q)) first = q;
          int x, y, w, h; getExtent(ctl(pW, C_ROW0 + first), &x, &y, &w, &h);
          CHECK(first > 0 && y == 2 * 162, "kreia: first shown row %d sits in slot 0 (y %d)", first, y); }
        { const int before = logCount("UI click"); clickCtl(pW, other, 0, C_BANK); CHECK(logCount("UI click") == before && !r->instant, "hidden bank toggle ignored"); }
        { int x, y, w, h; getExtent(ctl(pW, C_ROW0 + 0), &x, &y, &w, &h); (void)x; (void)y; (void)w; (void)h; }
        // PC leader hides + resets the page
        tick(pW, pc, 0);
        CHECK(!r->open && !vis(pW, C_BG0) && !vis(pW, C_STATUS) && vis(pW, C_ICON) && !vis(pW, C_CFG), "PC leader closes the page, shows its own icon");
        // batch 3a: the PC's R10 filter page (icon -> page -> rows -> Apply writes the pc line; Cancel writes nothing; reopen loads it)
        {
            const std::string before = fileText("levelup_playthroughs.ini");
            clearLog();
            clickCtl(pW, pc, 0, C_ICON);
            CHECK(r->open && r->pcPage && logHas("UI click icon tag= row=-") && logHas("UI r10 page open start=default on=0"), "PC icon opens the R10 page:\n%s", readLog().c_str());
            CHECK(vis(pW, C_BG0) && txt(pW, C_TITLE) == "Level-up filter" && vis(pW, C_APPLY) && vis(pW, C_CANCEL) && !vis(pW, C_INSTANT) && !vis(pW, C_BANK) && !vis(pW, C_PAUSE) &&
                  !vis(pW, C_BUILD) && !vis(pW, C_VANILLA) && !vis(pW, C_REC) && vis(pW, C_ICON) && !vis(pW, C_CFG) && !vis(pW, C_STATUS), "R10 page controls (batch 3d: the icon stays)");
            { bool rows = true; for (int i = 0; i < NumRows; ++i) if (!vis(pW, C_ROW0 + i)) rows = false; CHECK(rows, "all six R10 rows shown"); }
            CHECK(txt(pW, C_ROW0) == "Filter: off" && txt(pW, C_ROW0 + 1) == "Role: melee" && txt(pW, C_ROW0 + 4) == "Feats: build_dim" && txt(pW, C_ROW0 + 5) == "Powers: build_dim",
                  "R10 rows '%s' '%s' '%s'", txt(pW, C_ROW0).c_str(), txt(pW, C_ROW0 + 1).c_str(), txt(pW, C_ROW0 + 4).c_str());
            clickCtl(pW, pc, 0, C_CANCEL);
            CHECK(!r->open && !r->pcPage && fileText("levelup_playthroughs.ini") == before && vis(pW, C_ICON), "Cancel closes and writes nothing");
            clickCtl(pW, pc, 0, C_ICON);
            clearLog();
            clickCtl(pW, pc, 0, C_ROW0);       // on
            clickCtl(pW, pc, 0, C_ROW0 + 2);   // style none -> single
            clickCtl(pW, pc, 0, C_ROW0 + 3);   // alignment none -> light
            CHECK(txt(pW, C_ROW0) == "Filter: on" && txt(pW, C_ROW0 + 2) == "Style: single" && txt(pW, C_ROW0 + 3) == "Alignment: light" && logHas("UI click row0 tag= row=on") && logHas("UI click row3 tag= row=align"),
                  "R10 rows cycle '%s' '%s' '%s':\n%s", txt(pW, C_ROW0).c_str(), txt(pW, C_ROW0 + 2).c_str(), txt(pW, C_ROW0 + 3).c_str(), readLog().c_str());
            clearLog();
            clickCtl(pW, pc, 0, C_APPLY);
            std::string ini = fileText("levelup_playthroughs.ini");
            CHECK(!r->open && ini.find("pc=r10|instant=0|hold=1|r10=1|pcfilter=melee/single/light|ffilter=build_dim|pfilter=build_dim") != std::string::npos &&
                  logHas("UI apply pc r10=1 pcfilter=melee/single/light ffilter=build_dim pfilter=build_dim ok=1"), "Apply writes the pc line:\n%s\n%s", ini.c_str(), readLog().c_str());
            CHECK(tagConfig("pc") == nullptr && !logHas(":r10/"), "the pc line is not a companion tag:\n%s", readLog().c_str());
            tick(pW, pc, 0);
            CHECK(vis(pW, C_STATUS) && txt(pW, C_STATUS) == "Level-up filter: on - feats build_dim, powers build_dim", "PC status '%s'", txt(pW, C_STATUS).c_str());
            clearLog();
            clickCtl(pW, pc, 0, C_ICON);
            CHECK(r->pcPage && txt(pW, C_ROW0) == "Filter: on" && txt(pW, C_ROW0 + 3) == "Alignment: light" && logHas("UI r10 page open start=playthrough on=1"), "reopen loads the saved line:\n%s", readLog().c_str());
            clickCtl(pW, pc, 0, C_ROW0);   // off
            clickCtl(pW, pc, 0, C_APPLY);
            ini = fileText("levelup_playthroughs.ini");
            CHECK(ini.find("pc=r10|instant=0|hold=1|pcfilter=melee/single/light|ffilter=build_dim|pfilter=build_dim") != std::string::npos, "off: the line stays, no r10 field:\n%s", ini.c_str());
            tick(pW, pc, 0);
            CHECK(!vis(pW, C_STATUS), "filter off: no PC status");
            tick(pW, pc, 1);
            CHECK(vis(pW, C_CFG) && !vis(pW, C_ICON) && txt(pW, C_CFG) == "Level-up filter", "PC with a level-up: the big button '%s'", txt(pW, C_CFG).c_str());
            CHECK(g_exoDst[ctl(pW, C_ROW0) + 0x2C] == uiTipText(C_ROW0, true) && g_exoDst[ctl(pW, C_CFG) + 0x2C] == uiTipText(C_CFG, true), "PC native tips swapped");
            tick(pW, other, 0);
            CHECK(logHas("UI tips kind=companion native=") && g_exoDst[ctl(pW, C_ROW0) + 0x2C] == uiTipText(C_ROW0) && !r->pcPage, "companion native tips restored");
            for (int i = 0; i < C_N; ++i) if (uiTipText(i, true)) { int ln = 0, mx = 0; guitip::measure(uiTipText(i, true), &ln, &mx); CHECK(ln <= TipMaxLines && mx <= TipMaxChars, "PC tip %s: %d lines, %d chars", kUiName[i], ln, mx); }
        }
        // Apply failure (no live table) -> title says so and the page stays open
        tick(pW, other, 0);
        clickCtl(pW, other, 0, C_CFG);
        at<uint32_t>(app, 8) = 0;
        const uint32_t g0 = ptGuid();
        remove("levelup_playthroughs.ini");
        LuoGuidNewServer(P(1));   // GUID forgotten: ptApply needs the live table again
        clickCtl(pW, other, 0, C_APPLY);
        CHECK(r->open && txt(pW, C_TITLE) == "Could not save - see log" && tagConfig("kreia") == nullptr, "save failure keeps the page open: '%s'", txt(pW, C_TITLE).c_str());
        at<uint32_t>(app, 8) = (uint32_t)srv;
        clickCtl(pW, other, 0, C_APPLY);
        CHECK(!r->open && tagConfig("kreia") != nullptr && ptGuid() != 0 && ptGuid() != g0, "second attempt succeeds");
        ptRemove("kreia");
        // g0t0: the Variant row appears on the identical-answer pair, cycles, and sits in the last slot
        Fake g0t0 = makeCreature("g0t0", {{0, 3}});
        tick(pW, g0t0, 0);
        clickCtl(pW, g0t0, 0, C_CFG);
        CHECK(r->open && !strcmp(r->pageTag, "g0t0") && !opt::shown(r->os, opt::VariantRow) && !vis(pW, C_ROW0 + opt::VariantRow), "g0t0 opens without a Variant row");
        for (int i = 0; i < 6 && !opt::shown(r->os, opt::VariantRow); ++i) clickCtl(pW, g0t0, 0, C_ROW0 + 1);   // Role cycles onto the ranged/pistols/none/n/a/combat pair
        CHECK(opt::shown(r->os, opt::VariantRow) && vis(pW, C_ROW0 + opt::VariantRow), "g0t0: Variant row appears on the identical-answer pair");
        if (opt::shown(r->os, opt::VariantRow)) {
            const int before = r->os.cur;
            int slot = 0; for (int q = 0; q < opt::VariantRow; ++q) if (opt::shown(r->os, q)) ++slot;
            int x, y, w, h; getExtent(ctl(pW, C_ROW0 + opt::VariantRow), &x, &y, &w, &h);
            CHECK(y == 2 * (162 + 28 * slot) && txt(pW, C_ROW0 + opt::VariantRow) == std::string("Variant: ") + r->items[before].shortName, "variant row last slot (y %d slot %d) '%s'", y, slot, txt(pW, C_ROW0 + opt::VariantRow).c_str());
            clickCtl(pW, g0t0, 0, C_ROW0 + opt::VariantRow);
            CHECK(r->os.cur != before && txt(pW, C_BUILD) == r->items[r->os.cur].shortName && txt(pW, C_ROW0 + opt::VariantRow) == std::string("Variant: ") + r->items[r->os.cur].shortName, "variant click cycles to %d", r->os.cur);
        }
        clickCtl(pW, g0t0, 0, C_CANCEL);
        // a table change (stamp) closes the page
        tick(pW, atton, 0);
        clickCtl(pW, atton, 0, C_CFG);
        CHECK(r->open, "open before the table change");
        g_tableStamp.mtime ^= 1;
        tick(pW, atton, 0);
        CHECK(!r->open && !vis(pW, C_BG0), "table change closes the page");
        (void)rp;
    }

    // 5. held status + R11 banked message
    {
        remove("levelup_playthroughs.ini");
        LuoGuidNewServer(P(1));
        char ini[256];
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ninstant.atton=0", PRESET);
        setIni(ini, table);
        const Preset* Pr = findPreset(PRESET);
        CHECK(Pr != nullptr && Pr->holdClass == 5, "preset for the held tests");
        uintptr_t pH = makePanel();
        LuoUiPanelCtor((int)pH);
        clearLog(); g_banked.clear(); g_stubTick += 6000;
        // no XP: held but nothing banked -> status says the next level waits; no message
        tick(pH, atton, 0);
        CHECK(txt(pH, C_CFG) == "Change path" && vis(pH, C_STATUS) && txt(pH, C_STATUS).rfind("Holding the next level for Jedi Sentinel - ", 0) == 0, "held, N=0: '%s'", txt(pH, C_STATUS).c_str());
        CHECK(g_banked.empty(), "no message without banked levels");
        // N=1: status + one message
        int r = tick(pH, attonXp1, 1);
        CHECK(r == 0, "hold gate result 0 while held");
        CHECK(txt(pH, C_STATUS).rfind("Holding 1 level for Jedi Sentinel - ", 0) == 0, "held N=1: '%s'", txt(pH, C_STATUS).c_str());
        CHECK(txt(pH, C_CFG) == "Change path" && nColor >= 0, "configured text");
        CHECK(g_banked.size() == 1 && g_bankedGui == P(gui) && g_bankedFlags == 0x80 && g_bankedByte == 0, "banked message called once (%zu)", g_banked.size());
        CHECK(!g_banked.empty() && g_banked[0].find(": level banked - waiting for Jedi Sentinel (level-up path)") != std::string::npos && g_banked[0].rfind(Pr->name[0] ? Pr->name : "Atton", 0) == 0, "banked text '%s'", g_banked.empty() ? "" : g_banked[0].c_str());
        CHECK(logHas("BANKED tag=Atton n=1 guid=00000000 shown=1"), "banked log:\n%s", readLog().c_str());
        for (int i = 0; i < 5; ++i) { tick(pH, attonXp1, 1); LuoHold_74F3C7(P(attonXp1.stats)); }
        CHECK(g_banked.size() == 1, "not again for N=1 (%zu)", g_banked.size());
        // N=2 inside the 5 s window: not yet; after the window: once
        Fake attonXp2 = makeCreature("Atton", {{2, 7}}, 40000);
        g_stubTick += 1000;
        LuoHold_74F3C7(P(attonXp2.stats));
        CHECK(g_banked.size() == 1, "N=2 inside the 5 s window waits");
        g_stubTick += 5000;
        LuoHold_5FD7CE(P(attonXp2.stats));
        CHECK(g_banked.size() == 2 && g_banked[1].find(": 2 levels banked - waiting for Jedi Sentinel") != std::string::npos, "N=2 after the window: %zu '%s'", g_banked.size(), g_banked.size() > 1 ? g_banked[1].c_str() : "");
        tick(pH, attonXp2, 1);
        CHECK(txt(pH, C_STATUS).rfind("Holding 2 levels for Jedi Sentinel - ", 0) == 0, "status N=2 '%s'", txt(pH, C_STATUS).c_str());
        // broken chain: nothing shown, retried when the chain is back
        Fake attonXp3 = makeCreature("Atton", {{2, 7}}, 50000);
        g_stubTick += 6000;
        at<uint32_t>(ci, 0x40) = 0;
        LuoHold_74F3C7(P(attonXp3.stats));
        CHECK(g_banked.size() == 2, "never when the chain is null");
        at<uint32_t>(client, 4) = 0; LuoHold_74F3C7(P(attonXp3.stats)); at<uint32_t>(app, 4) = 0; LuoHold_74F3C7(P(attonXp3.stats));
        CHECK(g_banked.size() == 2, "never when any link is null");
        at<uint32_t>(app, 4) = (uint32_t)client; at<uint32_t>(client, 4) = (uint32_t)ci; at<uint32_t>(ci, 0x40) = (uint32_t)gui;
        LuoHold_74F3C7(P(attonXp3.stats));
        CHECK(g_banked.size() == 3 && g_banked[2].find(": 3 levels banked") != std::string::npos, "retried once the chain is back (%zu)", g_banked.size());
        // never for non-held / unconfigured / log mode
        g_stubTick += 6000;
        Fake sentinelXp = makeCreature("atton", {{2, 7}, {5, 0}}, 90000);
        LuoHold_74F3C7(P(sentinelXp.stats));
        CHECK(g_banked.size() == 3, "unheld creature: no message");
        tick(pH, sentinelXp, 1);
        CHECK(vis(pH, C_STATUS) && txt(pH, C_STATUS).rfind("Auto path: ", 0) == 0, "configured not held: '%s'", txt(pH, C_STATUS).c_str());
        // GUID change resets the per-tag memory
        g_stubTick += 6000;
        LuoHold_74F3C7(P(attonXp3.stats));
        CHECK(g_banked.size() == 3, "same GUID: N=3 not repeated");
        char want[64];
        snprintf(want, sizeof want, "mode=rewrite\npreset.atton=%s\nhold.atton=1", PRESET);
        ptApply("hk47", PRESET, false, true, 0);   // creates a GUID: the playthrough section now applies, dev keys do not -> atton unconfigured
        CHECK(ptGuid() != 0 && tagConfig("atton") == nullptr, "GUID created");
        ptApply("atton", PRESET, false, true, 0);
        LuoHold_74F3C7(P(attonXp3.stats));
        CHECK(g_banked.size() == 4, "new GUID resets lastShown: shown again (%zu)", g_banked.size());
        // log mode: no message, no hold
        g_stubTick += 6000;
        setIni("mode=log", table);
        Fake attonXp4 = makeCreature("Atton", {{2, 7}}, 70000);
        LuoHold_74F3C7(P(attonXp4.stats));
        CHECK(g_banked.size() == 4, "log mode: no message");
        // engine mismatch: message disabled, logged once
        setIni("mode=rewrite", table);
        g_stubTick += 6000;
        g_eng.checkCode = true; g_bankedEngState = 0; g_engState = 1;   // gates already verified; only the banked-message functions mismatch
        clearLog();
        Fake attonXp5 = makeCreature("Atton", {{2, 7}}, 99000);
        LuoHold_74F3C7(P(attonXp5.stats)); LuoHold_74F3C7(P(attonXp5.stats));
        g_eng.checkCode = false; g_engState = 0;
        CHECK(g_banked.size() == 4 && logCount("BANKED engine function bytes differ") == 1, "engine mismatch: disabled, logged once (%d)", logCount("BANKED engine function bytes differ"));
        // A3: levels taken (banked count drops) -> a later increase announces again
        g_stubTick += 6000;
        LuoHold_74F3C7(P(attonXp5.stats));
        const size_t bAfter = g_banked.size();
        CHECK(bAfter == 5, "announced once the engine check is off (%zu)", bAfter);
        g_stubTick += 6000;
        LuoHold_74F3C7(P(attonXp1.stats));
        CHECK(g_banked.size() == bAfter, "fewer banked levels: no message");
        g_stubTick += 6000;
        LuoHold_74F3C7(P(attonXp5.stats));
        CHECK(g_banked.size() == bAfter + 1, "banked count dropped then rose again -> announced again (%zu)", g_banked.size());
    }

    // 5. hover tooltips (GuiTip.h): native strings set once in uiAppend, custom fallback box, probe log
    {
        unsigned wantMask = 0;
        for (int i = 0; i < C_N; ++i) {
            const char* t = uiTipText(i);
            const bool want = i == C_CFG || i == C_ICON || (i >= C_ROW0 && i < C_ROW0 + NumRows) || i == C_INSTANT || i == C_BANK || i == C_PAUSE || i == C_PMINUS || i == C_PPLUS || i == C_REC || i == C_APPLY || i == C_VANILLA || i == C_CANCEL;
            CHECK((t != nullptr) == want, "tip text for %s: %s (want %d)", kUiName[i], t ? "set" : "null", want);
            if (!t) continue;
            wantMask |= 1u << i;
            int lines = 0, maxLen = 0;
            guitip::measure(t, &lines, &maxLen);
            CHECK(lines >= 1 && lines <= TipMaxLines && maxLen <= TipMaxChars, "tip %s: %d lines, longest %d chars (max %d x %d)", kUiName[i], lines, maxLen, TipMaxLines, TipMaxChars);
        }
        CHECK(strstr(uiTipText(C_ROW0 + 1), "Role") && strstr(uiTipText(C_ROW0 + 2), "Weapons") && strstr(uiTipText(C_ROW0 + 3), "Powers") && strstr(uiTipText(C_ROW0 + 4), "Skills") && strstr(uiTipText(C_ROW0 + 5), "Variant"), "row tips name their row");
        CHECK(uiTipText(C_PAUSE) == uiTipText(C_PMINUS) && uiTipText(C_PMINUS) == uiTipText(C_PPLUS), "pause label, - and + share one tip");
        ptApply("atton", PRESET, false, true, 0);   // configured -> Vanilla offered
        CHECK(tagConfig("atton") != nullptr, "atton configured for the tip tests");
        setIni("mode=rewrite", table);
        clearLog();
        g_exoDst.clear();
        uintptr_t pT = makePanel();
        LuoUiPanelCtor((int)pT);
        tick(pT, attonLo, 0);
        CHECK(logCount("UI tips native=") == 1 && logHas(("UI tips native=" + std::to_string(__builtin_popcount(wantMask))).c_str()), "native tips logged once with the tip-control count:\n%s", readLog().c_str());
        for (int i = 0; i < C_N; ++i) {
            const uintptr_t c = ctl(pT, i);
            if (uiTipText(i)) CHECK(at<int>(c, 0x28) == 0 && g_exoDst.count(c + 0x2C) && g_exoDst[c + 0x2C] == uiTipText(i), "native tip of %s: strref %d text '%s'", kUiName[i], at<int>(c, 0x28), g_exoDst.count(c + 0x2C) ? g_exoDst[c + 0x2C].c_str() : "(none)");
            else CHECK(at<int>(c, 0x28) == 77 && !g_exoDst.count(c + 0x2C), "no native tip on %s", kUiName[i]);
        }
        // closed page: Configure button tip (box goes above y 372, inside x 54..300)
        tipCoverShown(pT, attonLo);
        CHECK(g_tipCovered == (1u << C_ICON), "closed page, no level-up: only the icon has a tip (%x)", g_tipCovered);
        clickCtl(pT, attonLo, 0, C_CFG);
        CHECK(rec(pT)->open && !boxShown(pT), "page open, box hidden after the click");
        tipCoverShown(pT, attonLo);
        clickCtl(pT, attonLo, 0, C_ROW0 + 1); tipCoverShown(pT, attonLo);      // off the recommended preset: 'Use recommended' appears
        clickCtl(pT, attonLo, 0, C_REC); tipCoverShown(pT, attonLo);
        if (vis(pT, C_PMINUS)) clickCtl(pT, attonLo, 0, C_PMINUS); else if (vis(pT, C_PPLUS)) clickCtl(pT, attonLo, 0, C_PPLUS);
        tipCoverShown(pT, attonLo);
        if (vis(pT, C_PPLUS)) clickCtl(pT, attonLo, 0, C_PPLUS);
        if (vis(pT, C_PMINUS)) clickCtl(pT, attonLo, 0, C_PMINUS);
        tipCoverShown(pT, attonLo);
        {   // the Variant row only shows for an identical-answer pair (g0t0, as the variant test above)
            const Fake g0 = makeCreature("g0t0", {{0, 3}});
            uintptr_t pV = makePanel();
            LuoUiPanelCtor((int)pV);
            tick(pV, g0, 0);
            clickCtl(pV, g0, 0, C_CFG);
            for (int i = 0; i < 6 && !vis(pV, C_ROW0 + opt::VariantRow); ++i) clickCtl(pV, g0, 0, C_ROW0 + 1);
            CHECK(vis(pV, C_ROW0 + opt::VariantRow), "variant row shown for the tip test");
            if (vis(pV, C_ROW0 + opt::VariantRow)) { tipHover(pV, g0, C_ROW0 + opt::VariantRow); }
        }
        {   // batch 3a: the Configure button exists only next to the engine's level-up buttons (held result 1; g0t0 is unconfigured here)
            const Fake g1 = makeCreature("g0t0", {{0, 3}});
            uintptr_t pC = makePanel();
            LuoUiPanelCtor((int)pC);
            tick(pC, g1, 1);
            CHECK(vis(pC, C_CFG) && !vis(pC, C_ICON), "level-up shown: Configure, no icon");
            if (vis(pC, C_CFG)) tipHover(pC, g1, C_CFG, 1);
        }
        CHECK(g_tipCovered == wantMask, "every tip control shown + hovered once (covered %06x, want %06x)", g_tipCovered, wantMask);
        CHECK(logCount("UI tips native=") == 3, "native tips set once per appended panel (pT, pV, pC), not from clicks (%d)", logCount("UI tips native="));
        { bool same = true; for (int i = 0; i < C_N; ++i) if (uiTipText(i) && g_exoDst[ctl(pT, i) + 0x2C] != uiTipText(i)) same = false; CHECK(same, "native strings unchanged after the clicks"); }
        CHECK(logCount("TIPPROBE over_is_ours=1 owner_is_ours=0 shown=0 ctl40=0 timer=0.00 appear=0.50 dis=0 ctl=") >= g_tipHovers && logCount("UI tip custom ctl=") == g_tipHovers,
              "one probe line + one custom-box line per hover (%d hovers):\n%s", g_tipHovers, readLog().c_str());
        // manager unreadable: engine delay unknown -> 400 ms default (+ 250 grace)
        {
            at<uint32_t>(g_tipCell, 0) = 0;
            int x, y, w, h; getExtent(ctl(pT, C_CANCEL), &x, &y, &w, &h);
            curX = x + w / 2; curY = y + h / 2; g_stubTick += 20000; const DWORD t0 = g_stubTick;
            tick(pT, attonLo, 0); g_stubTick = t0 + 649; tick(pT, attonLo, 0);
            CHECK(!boxShown(pT), "default delay: hidden at 649 ms");
            g_stubTick = t0 + 650; tick(pT, attonLo, 0);
            CHECK(boxShown(pT), "default delay 400 + 250 grace: shown at 650 ms");
            at<uint32_t>(g_tipCell, 0) = (uint32_t)g_tipMgr;
            curX = 0; curY = 0; tick(pT, attonLo, 0);
        }
        // a click hides the box (and restarts the hover clock); the toggle still works
        {
            int x, y, w, h; getExtent(ctl(pT, C_INSTANT), &x, &y, &w, &h);
            at<uint32_t>(g_tipMgr, 0x40) = 0;
            curX = x + w / 2; curY = y + h / 2; lmb = false; g_stubTick += 20000; const DWORD t0 = g_stubTick;
            tick(pT, attonLo, 0); g_stubTick = t0 + 750; tick(pT, attonLo, 0);
            CHECK(boxShown(pT), "box up before the click");
            const bool before = rec(pT)->instant;
            lmb = true; tick(pT, attonLo, 0);
            CHECK(!boxShown(pT) && rec(pT)->instant != before, "press hides the box, click handling unchanged");
            lmb = false; g_stubTick += 100; tick(pT, attonLo, 0);
            CHECK(!boxShown(pT), "box stays hidden after the click until the hover clock runs out again");
            g_stubTick = t0 + 750 + 100 + 750; tick(pT, attonLo, 0);
            CHECK(boxShown(pT), "box comes back after a fresh delay");
            // gate inactive / other character: hidden
            const Fake hk = makeCreature("zzz-nobody", {{6, 7}}, 0);
            tick(pT, hk, 0);
            CHECK(!boxShown(pT) && !anyVisible(pT), "tag without presets: box hidden with everything else");
            curX = 0; curY = 0;
        }
        // native box seen: custom box never shows, even after the owner is gone, for any control (fresh panel: sticky is per record)
        {
            uintptr_t pN = makePanel();
            LuoUiPanelCtor((int)pN);
            tick(pN, attonLo, 0);
            const uintptr_t c = ctl(pN, C_ICON);   // batch 3a: no level-up -> the icon is the closed-page control
            int x, y, w, h; getExtent(c, &x, &y, &w, &h);
            at<uint32_t>(g_tipMgr, 0x08) = (uint32_t)c; at<uint32_t>(g_tipMgr, 0x40) = (uint32_t)c; at<uint32_t>(g_tipMgr, 0x1C) = 8; at<int>(c, Ctl_Flags) |= 0x40;
            curX = x + w / 2; curY = y + h / 2; g_stubTick += 20000; const DWORD t0 = g_stubTick;
            clearLog();
            tick(pN, attonLo, 0); g_stubTick = t0 + 100; tick(pN, attonLo, 0);   // the engine owns the tooltip from here on
            g_stubTick = t0 + 900; tick(pN, attonLo, 0);
            CHECK(!boxShown(pN), "engine owns the hovered control: no custom box");
            CHECK(logHas("TIPPROBE over_is_ours=1 owner_is_ours=1 shown=1 ctl40=1 "), "probe sees the native box:\n%s", readLog().c_str());
            at<uint32_t>(g_tipMgr, 0x40) = 0; at<uint32_t>(g_tipMgr, 0x1C) = 0; at<int>(c, Ctl_Flags) &= ~0x40;
            g_stubTick = t0 + 30000; tick(pN, attonLo, 0);
            CHECK(!boxShown(pN) && rec(pN)->tip.nativeSeen, "owner gone later: still no custom box (sticky)");
            curX = 0; curY = 0; tick(pN, attonLo, 0);
            clickCtl(pN, attonLo, 0, C_CFG);   // open the page and hover another control
            int ax, ay, aw, ah; getExtent(ctl(pN, C_CANCEL), &ax, &ay, &aw, &ah);
            curX = ax + aw / 2; curY = ay + ah / 2; g_stubTick += 20000; const DWORD t1 = g_stubTick;
            tick(pN, attonLo, 0); g_stubTick = t1 + 5000; tick(pN, attonLo, 0);
            CHECK(!boxShown(pN), "sticky: no custom box on other controls either");
        }
    }

    // 7. batch 3c: square icon, embedded gear + hover swap, fallbacks, one framed window, lvlbtn sampled a tick late
    {
        const guikit::Scale ws = {4.8f, 2.667f, 100.f, 50.f, 362, 325, true};   // wide screen: sx != sy
        int x, y, w, h;
        uiIconRect(ws, &x, &y, &w, &h);
        CHECK(w == h && w == (int)lroundf(28 * 2.667f) && x + w == unitX(ws, 520.f) && y == unitY(ws, 172.f), "icon rect %d,%d,%d,%d not square / not pinned at unit 520", x, y, w, h);
        static UiPanel tmpR;
        const guitip::Rect q = uiRectUnits(&tmpR, C_ICON, &ws), q0 = uiRectUnits(&tmpR, C_ICON);
        CHECK(q.x + q.w == 520 && q.w == (int)lroundf(28 * 2.667f / 4.8f) && q.y == 172 && q.h == 28, "tip anchor units %d,%d,%d,%d", q.x, q.y, q.w, q.h);
        CHECK(q0.x == 492 && q0.w == 28, "no scale: spec rect");

        // placed on a panel whose BTN_LEVELUP gives sx 2.298 / sy 2 (aspect 2.7, accepted): square, right edge = unitX(520)
        embtex::g_et = {efNew, efNewArr, efCtor, efAdd, efReset, arenaAlloc(16)};
        g_img["luo_gear"] = reinterpret_cast<void*>(0x5200); g_img["luo_gear_hi"] = reinterpret_cast<void*>(0x5100); g_img["luo_fill"] = reinterpret_cast<void*>(0x5300);
        uintptr_t pI = makePanel(216, 80);
        LuoUiPanelCtor((int)pI);
        clearLog();
        tick(pI, atton, 0);
        const uintptr_t ic = ctl(pI, C_ICON), gc = ctl(pI, C_GEAR);
        getExtent(ic, &x, &y, &w, &h);
        CHECK(w == h && w == 56 && x + w == 724 + (int)lroundf((520 - 362) * (216.f / 94.f)), "placed icon %d,%d,%d,%d", x, y, w, h);
        CHECK(logHas("tex luo_gear: created") && logHas("tex luo_gear_hi: created") && logHas("tex luo_fill: created") && eCtor == 3 && eAdd == 3, "embedded textures created (ctor %d add %d):\n%s", eCtor, eAdd, readLog().c_str());
        CHECK(logHas("UI icon fill=luo_gear ok=1") && logHas("UI icon emb=1 hi=1 style=1"), "gear fill log:\n%s", readLog().c_str());
        CHECK(at<uint32_t>(gc, Label_Border + Border_Fill) == 0x5200 && (at<uint32_t>(gc, Label_Border + Border_FillStyle) & 3) == 2, "gear fill / stretch style (on C_GEAR)");
        CHECK(at<uint32_t>(ic, Label_Border + Border_Fill) == 0, "batch 3d: the icon button keeps its template fill");
        // hover: the gold fill once, back on leave
        curX = x + w / 2; curY = y + h / 2; tick(pI, atton, 0);
        CHECK(at<uint32_t>(gc, Label_Border + Border_Fill) == 0x5100, "hover -> luo_gear_hi");
        at<uint32_t>(gc, Label_Border + Border_Fill) = 0xDEAD; tick(pI, atton, 0);
        CHECK(at<uint32_t>(gc, Label_Border + Border_Fill) == 0xDEAD, "hover fill written only on a change");
        curX = 0; curY = 0; tick(pI, atton, 0);
        CHECK(at<uint32_t>(gc, Label_Border + Border_Fill) == 0x5200, "leave -> luo_gear");
        for (int k = 0; k < embtex::slotCount(); ++k) at<uint32_t>(reinterpret_cast<uintptr_t>(embtex::slots()[k].basic), embtex::Tex_Uploaded) = 1;   // the texture tick uploaded both
        auto glReset = [](unsigned char* b) { b[embtex::Tex_Constructed] = 0; *reinterpret_cast<void**>(b + embtex::Tex_Pixels) = nullptr; };   // what an engine GL reset leaves
        tick(pI, atton, 0);
        CHECK(eCtor == 3 && eAdd == 3, "heal: healthy textures are not refilled (add %d)", eAdd);
        // GL reset -> heal() refills on the next UI tick; a texture that never comes back is given up after exactly 20 refills
        unsigned char* gb = embtex::slots()[0].basic;
        clearLog(); glReset(gb); tick(pI, atton, 0);
        CHECK(eAdd == 4 && logHas("tex luo_gear: refilled after a texture reset"), "heal refills after a reset (add %d):\n%s", eAdd, readLog().c_str());
        at<uint32_t>(reinterpret_cast<uintptr_t>(gb), embtex::Tex_Uploaded) = 0;
        clearLog();
        for (int k = 0; k < 30; ++k) { glReset(gb); tick(pI, atton, 0); }
        CHECK(eAdd == 3 + 20 && logCount("tex luo_gear: giving up after 20") == 1, "give-up after 20 refills in all (the first one above counts; adds %d)", eAdd - 3);

        // a second panel reuses the created textures (ensure is idempotent)
        uintptr_t pI2 = makePanel(); LuoUiPanelCtor((int)pI2); clearLog(); tick(pI2, atton, 0);
        CHECK(eCtor == 3 && logHas("UI icon emb=1 hi=1 style=1"), "second panel: no new textures");
        // embedded fill fails (engine table unusable) -> lbl_icn_opt2, no hover swap
        const embtex::Engine saved = embtex::g_et; embtex::g_et.ctorFn = nullptr;
        uintptr_t pJ = makePanel(); LuoUiPanelCtor((int)pJ); clearLog(); tick(pJ, atton, 0);
        CHECK(logHas("UI icon fill=lbl_icn_opt2 ok=1") && logHas("UI icon emb=0 hi=0 style=1"), "embedded failure falls back:\n%s", readLog().c_str());
        embtex::g_et = saved;
        // ui.icon= names a resref that does not load -> lbl_icn_opt2
        g_img["zz_noicon"] = nullptr;
        setIni("mode=rewrite\nui.icon=zz_noicon", table);
        tick(pI, atton, 0);   // the ini reloads in the gate, after a new panel's first-tick uiAppend: load it on an existing panel first
        uintptr_t pK = makePanel(); LuoUiPanelCtor((int)pK); clearLog(); tick(pK, atton, 0);
        CHECK(logHas("UI icon fill=lbl_icn_opt2 ok=1") && logHas("UI icon emb=0 hi=0"), "missing ui.icon falls back:\n%s", readLog().c_str());
        setIni("mode=rewrite", table);

        // Mod 14's LBL_EFX_BACK present: one window (its own fill kept, alpha 1), BG1/BG2 hidden
        g_missingTag.erase("LBL_EFX_BACK");
        clearLog();
        uintptr_t pX = makePanel(); LuoUiPanelCtor((int)pX);
        CHECK(rec(pX) && rec(pX)->win && logHas("UI page window=efx"), "window template used");
        tick(pX, atton, 0);
        CHECK(logHas("UI page window back alpha ok=1") && !logHas("UI page back0"), "window: no fill swap:\n%s", readLog().c_str());
        clickCtl(pX, atton, 0, C_CFG);
        CHECK(rec(pX)->open && vis(pX, C_BG0) && !vis(pX, C_BG1) && !vis(pX, C_BG2), "one window: bg0 %d bg1 %d bg2 %d", vis(pX, C_BG0), vis(pX, C_BG1), vis(pX, C_BG2));
        CHECK(at<uint32_t>(ctl(pX, C_BG0), Label_Border + Border_Fill) == 0x5300 && (at<uint32_t>(ctl(pX, C_BG0), Label_Border + Border_FillStyle) & 3) == 2 &&
              at<float>(ctl(pX, C_BG0), Label_Border + Border_Alpha) == 1.0f, "batch 3d: the window gets the opaque luo_fill (stretch), alpha 1");
        getExtent(ctl(pX, C_BG0), &x, &y, &w, &h);
        CHECK(x == 108 && y == 268 && w == 492 && h == 672, "window rect %d,%d,%d,%d (stats column, x <= unit 300)", x, y, w, h);
        clickCtl(pX, atton, 0, C_CANCEL);
        CHECK(!rec(pX)->open && !vis(pX, C_BG0), "cancel hides the window");
        g_missingTag["LBL_EFX_BACK"] = 1;
        uintptr_t pS = makePanel(); clearLog(); LuoUiPanelCtor((int)pS);
        CHECK(rec(pS) && !rec(pS)->win && logHas("UI page window=stack"), "no template: the 3-stack");

        // UI lvlbtn: logged on the tick AFTER the change (the engine writes BTN_LEVELUP bit1 after the hook)
        clearLog();
        tick(pS, atton, 1);
        CHECK(!logHas("UI lvlbtn shown="), "not on the change tick:\n%s", readLog().c_str());
        tick(pS, atton, 1);
        CHECK(logCount("UI lvlbtn shown=") == 1 && logCount(" isVisible=") == 1, "next tick:\n%s", readLog().c_str());
        tick(pS, atton, 1);
        CHECK(logCount("UI lvlbtn shown=") == 1, "once per change");
    }

    // 8. batch 3d: gear label on a framed button, vanilla hover tint, the icon closes an open page, one opaque luo_fill back (embedded engine still installed)
    {
        CHECK(C_N == 26 && C_GEAR == C_ICON + 1 && !strcmp(kUiName[C_GEAR], "gear"), "C_N %d (gui_ids count 26)", (int)C_N);
        // 8.1 gear rect: square, strictly inside the icon rect, inset round(side * 0.18) per side
        const guikit::Scale ws = {4.8f, 2.667f, 100.f, 50.f, 362, 325, true};
        int ix, iy, iw, ih, gx, gy, gw, gh;
        uiIconRect(ws, &ix, &iy, &iw, &ih); uiGearRect(ws, &gx, &gy, &gw, &gh);
        const int in = (int)lroundf(iw * 0.18f);
        CHECK(in > 0 && gw == gh && gx == ix + in && gy == iy + in && gw == iw - 2 * in, "gear rect %d,%d,%d,%d in icon %d,%d,%d,%d", gx, gy, gw, gh, ix, iy, iw, ih);
        uintptr_t p8 = makePanel();
        LuoUiPanelCtor((int)p8);
        const float base[3] = {0.05098f, 0.34902f, 0.27059f};
        for (int i = 0; i < C_N; ++i) for (int k = 0; k < 3; ++k) at<float>(ctl(p8, i), Label_Border + Border_Tint + 4 * k) = base[k];
        clearLog();
        tick(p8, atton, 0);
        const uintptr_t ic = ctl(p8, C_ICON), gc = ctl(p8, C_GEAR);
        getExtent(ic, &ix, &iy, &iw, &ih); getExtent(gc, &gx, &gy, &gw, &gh);
        CHECK(vis(p8, C_ICON) && vis(p8, C_GEAR) && gw == gh && gx > ix && gy > iy && gx + gw < ix + iw && gy + gh < iy + ih, "placed gear %d,%d,%d,%d inside icon %d,%d,%d,%d", gx, gy, gw, gh, ix, iy, iw, ih);
        CHECK(at<uint32_t>(ic, Label_Border + Border_Fill) == 0 && at<uint32_t>(gc, Label_Border + Border_Fill) == 0x5200 && (at<int>(gc, Ctl_Flags) & FlagClickThrough), "icon keeps the template fill, gear = luo_gear, click-through");
        CHECK(logHas("UI hover tint base_ok=1") && logHas("UI page fill=luo_fill ok=1 style=1 one=1"), "3d logs:\n%s", readLog().c_str());
        // 8.2 hover tint: cream on enter, written once, base on leave; follows the cursor; a hidden control is restored
        auto tintIs = [&](uintptr_t c, const float* v) { for (int k = 0; k < 3; ++k) if (at<float>(c, Label_Border + Border_Tint + 4 * k) != v[k]) return false; return true; };
        auto hoverAt = [&](int idx) { int x, y, w, h; getExtent(ctl(p8, idx), &x, &y, &w, &h); curX = x + w / 2; curY = y + h / 2; lmb = false; tick(p8, atton, 0); };
        hoverAt(C_ICON);
        CHECK(tintIs(ic, kHiTint) && rec(p8)->tinted == C_ICON, "icon hover -> HILIGHT tint");
        at<float>(ic, Label_Border + Border_Tint) = 0.5f; tick(p8, atton, 0);
        CHECK(at<float>(ic, Label_Border + Border_Tint) == 0.5f, "tint written only on a change");
        curX = 0; curY = 0; tick(p8, atton, 0);
        CHECK(tintIs(ic, base) && rec(p8)->tinted == -1, "leave -> base tint");
        // 8.3 the page stays reachable through the icon: open, icon + gear visible, icon click closes like Cancel
        const std::string before = fileText("levelup_playthroughs.ini");
        clickCtl(p8, atton, 0, C_ICON);
        CHECK(rec(p8)->open && vis(p8, C_ICON) && vis(p8, C_GEAR) && vis(p8, C_BG0), "page open: icon %d gear %d", vis(p8, C_ICON), vis(p8, C_GEAR));
        hoverAt(C_APPLY);
        CHECK(tintIs(ctl(p8, C_APPLY), kHiTint) && tintIs(ic, base), "Apply hover tinted, icon restored");
        hoverAt(C_ROW0);
        CHECK(tintIs(ctl(p8, C_ROW0), kHiTint) && tintIs(ctl(p8, C_APPLY), base), "row hover tinted, Apply restored");
        clearLog();
        clickCtl(p8, atton, 0, C_ICON);
        CHECK(!rec(p8)->open && !vis(p8, C_BG0) && !vis(p8, C_APPLY) && vis(p8, C_ICON) && vis(p8, C_GEAR) && logHas("UI gear close tag=Atton") &&
              fileText("levelup_playthroughs.ini") == before, "icon click closes the page, writes nothing:\n%s", readLog().c_str());
        CHECK(tintIs(ctl(p8, C_ROW0), base), "rows restored");
        // level-up buttons shown: the open page still shows the icon (it closes it), Configure hidden
        clickCtl(p8, atton, 1, C_CFG);
        CHECK(rec(p8)->open && vis(p8, C_ICON) && !vis(p8, C_CFG), "open with lvl: icon %d cfg %d", vis(p8, C_ICON), vis(p8, C_CFG));
        hoverAt(C_CANCEL);
        CHECK(tintIs(ctl(p8, C_CANCEL), kHiTint), "Cancel hover tinted");
        clickCtl(p8, atton, 0, C_CANCEL);   // Cancel hides itself while hovered: its tint is restored
        CHECK(!rec(p8)->open && !vis(p8, C_CANCEL) && tintIs(ctl(p8, C_CANCEL), base) && rec(p8)->tinted != C_CANCEL, "hidden control restored");
        // PC page: the icon closes the R10 page too
        clickCtl(p8, pc, 0, C_ICON);
        CHECK(rec(p8)->open && rec(p8)->pcPage && vis(p8, C_ICON), "PC page open, icon shown");
        clickCtl(p8, pc, 0, C_ICON);
        CHECK(!rec(p8)->open && logHas("UI gear close tag="), "PC page: icon closes");
        // 8.4 one opaque back on the 3-stack template: BG0 = luo_fill stretched, BG1/BG2 hidden
        clickCtl(p8, atton, 0, C_ICON);
        CHECK(rec(p8)->oneBack && !rec(p8)->win && vis(p8, C_BG0) && !vis(p8, C_BG1) && !vis(p8, C_BG2), "one back: bg0 %d bg1 %d bg2 %d", vis(p8, C_BG0), vis(p8, C_BG1), vis(p8, C_BG2));
        CHECK(at<uint32_t>(ctl(p8, C_BG0), Label_Border + Border_Fill) == 0x5300 && (at<uint32_t>(ctl(p8, C_BG0), Label_Border + Border_FillStyle) & 3) == 2 &&
              at<float>(ctl(p8, C_BG0), Label_Border + Border_Alpha) == 1.0f, "BG0 luo_fill, stretch, alpha 1");
        clickCtl(p8, atton, 0, C_ICON);
        // fill failure (embedded engine unusable) -> the 3-stack, gear hidden, fallback icon fill on the button
        const embtex::Engine saved = embtex::g_et; embtex::g_et.ctorFn = nullptr;
        uintptr_t p9 = makePanel(); LuoUiPanelCtor((int)p9); clearLog(); tick(p9, atton, 0);
        CHECK(logHas("UI page fill=luo_fill ok=0 style=0 one=0") && !rec(p9)->oneBack && logHas("UI page back0 fill="), "fill failure log:\n%s", readLog().c_str());
        CHECK(vis(p9, C_ICON) && !vis(p9, C_GEAR) && at<uint32_t>(ctl(p9, C_ICON), Label_Border + Border_Fill) == 0x5000, "fallback icon on the button, gear hidden");
        clickCtl(p9, atton, 0, C_ICON);
        CHECK(rec(p9)->open && vis(p9, C_BG0) && vis(p9, C_BG1) && vis(p9, C_BG2) && !vis(p9, C_GEAR), "3-stack after a fill failure");
        embtex::g_et = saved;
        curX = 0; curY = 0;
    }

    return kt::Report("ui_test");
}
