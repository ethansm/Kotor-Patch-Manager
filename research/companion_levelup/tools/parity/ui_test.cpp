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
static int g_fail = 0, g_checks = 0;
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fail; fprintf(stderr, "FAIL line %d: ", __LINE__); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)
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
    at<float>(c, Label_Text + Text_StringSub + Text_Color) = 0.f; at<float>(c, Label_Text + Text_StringSub + Text_Color + 4) = 1.f; at<float>(c, Label_Text + Text_StringSub + Text_Color + 8) = 0.f;
}
// CExoString fake: the first dword of the 16-byte buffer GuiKit passes holds a pointer to a copy (ring of 64 arena slots).
static uintptr_t g_exoSlots[64]; static int g_exoNext = 0;
static void* fExoCtor(void* e, const char* s) { uintptr_t sl = g_exoSlots[g_exoNext++ & 63]; strncpy(reinterpret_cast<char*>(sl), s, 255); memset(e, 0, 16); *reinterpret_cast<uint32_t*>(e) = (uint32_t)sl; return e; }
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
static void* fLoadImage(const char*) { return reinterpret_cast<void*>(0x5000); }
static void fSound(void*, char) { ++nSound; }
static void fSetRect(uint32_t, uintptr_t ctl, int* r) { ++nSetRect; for (int k = 0; k < 4; ++k) at<int>(ctl, Ctl_X + 4 * k) = r[k]; }
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
    if (!getExtent(ctl(panel, idx), &x, &y, &w, &h)) { ++g_fail; fprintf(stderr, "FAIL clickCtl: no extent\n"); return; }
    curX = x + w / 2; curY = y + h / 2;
    lmb = false; tick(panel, who, real);
    lmb = true; tick(panel, who, real);
    lmb = false; tick(panel, who, real);
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
    at<uint32_t>(snd, 0) = (uint32_t)arenaAlloc(16);
    guikit::Engine& e = g_gk;
    e.alloc = fAlloc; e.labelCtor = fCtor; e.initControl = fInit; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.textSet = fTextSet; e.setFont = fFont; e.setAlign = fAlign;
    e.textColor = fColor; e.listAdd = fList; e.loadImage = fLoadImage; e.playSound = fSound; e.callSetRect = fSetRect;
    e.screenWAddr = screen; e.screenHAddr = screen + 4; e.guiSoundMgrAddr = snd;
    e.cursorClient = fCursor; e.gameForeground = fFg; e.lbuttonDown = fLmb;

    const char* PRESET = "atton__melee_dual_light_early";
    Fake atton = makeCreature("Atton", {{2, 7}}, 0);
    Fake attonXp1 = makeCreature("Atton", {{2, 7}}, 30000);   // L7: tbl[7] = 28000 <= XP < 36000 -> 1 banked
    Fake sentinel = makeCreature("atton", {{2, 7}, {5, 0}}, 30000);
    Fake pc = makeCreature("", {{0, 7}}, 0);
    Fake nobody = makeCreature("zzz-nobody", {{6, 7}}, 0);
    const uintptr_t liveTbl = intl + 0x100FC;
    (void)liveTbl;

    // 1. panel ctor: creates (addToList 0), does not append; once per panel
    setIni("mode=rewrite", table);
    clearLog();
    uintptr_t pA = makePanel();
    LuoUiPanelCtor((int)pA);
    CHECK(rec(pA) && rec(pA)->nCreated == C_N && nCtor == C_N && nInit == C_N, "ctor created %d (nCtor %d nInit %d)", rec(pA) ? rec(pA)->nCreated : -1, nCtor, nInit);
    CHECK(nInitFramed == 16 && nInitPlain == 3, "templates framed %d plain %d (want 16/3)", nInitFramed, nInitPlain);
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
    CHECK(vis(pA, C_CFG) && txt(pA, C_CFG) == "Configure" && !vis(pA, C_STATUS) && !vis(pA, C_BG0) && !vis(pA, C_TITLE) && !vis(pA, C_APPLY) && !vis(pA, C_CANCEL), "state: cfg=%d '%s'", vis(pA, C_CFG), txt(pA, C_CFG).c_str());
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
    // PC and a tag without presets: everything hidden
    tick(pA, pc, 1);
    CHECK(!anyVisible(pA), "PC leader: all hidden");
    tick(pA, nobody, 1);
    CHECK(!anyVisible(pA), "tag without presets: all hidden");
    Fake garbage = makeCreature("x", {{0, 1}});
    tick(pA, atton, 0);
    CHECK(vis(pA, C_CFG), "atton shown again");
    CHECK(LuoHold_84F221(P(0x20), P(g_frames[pA])) == 0 && !anyVisible(pA), "unreadable stats: hidden, no crash");
    CHECK(LuoHold_84F221(P(atton.stats), nullptr) == 0 && LuoHold_84F221(P(atton.stats), P(arenaAlloc(0x400) + 0x300)) == 0, "no frame / empty panel slot: no tick, no crash");
    (void)garbage;

    // 3. a new panel pointer creates anew; first tick appends its own controls
    const int ctorsBefore = nCtor, listBefore = nList;
    uintptr_t pB = makePanel();
    LuoUiPanelCtor((int)pB);
    CHECK(nCtor == ctorsBefore + C_N && rec(pB) && rec(pB) != rec(pA), "new panel creates anew");
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
    g_missingTag.clear();
    // mode=off hides everything
    uintptr_t pG = makePanel();
    LuoUiPanelCtor((int)pG);
    tick(pG, atton, 0);
    CHECK(vis(pG, C_CFG), "pG shown");
    setIni("mode=off", table);
    tick(pG, atton, 0);
    CHECK(!anyVisible(pG), "mode=off: hidden");
    setIni("mode=rewrite", table);


    // 4. options page through simulated clicks on panel pE (fresh: first tick appends)
    uintptr_t pW = pE;
    tick(pW, atton, 0);
    CHECK(vis(pW, C_CFG), "pW cfg visible");
    const int sounds = nSound;
    clearLog();
    clickCtl(pW, atton, 0, C_CFG);
    CHECK(nSound == sounds + 1, "hover sound once per enter (%d)", nSound - sounds);
    CHECK(rec(pW)->open && logHas("UI click cfg tag=Atton row=-") && logHas("UI page open tag=Atton start=atton__melee_dual_light_early (recommended)"), "cfg click opens the page:\n%s", readLog().c_str());
    {
        UiPanel* r = rec(pW);
        const Preset* rp = findPreset(PRESET);
        CHECK(r->nItems == 7 && !strcmp(r->items[r->os.cur].id, PRESET) && r->recItem == r->os.cur, "starts on the recommended preset (%d items, cur %d)", r->nItems, r->os.cur);
        CHECK(!vis(pW, C_CFG) && !vis(pW, C_STATUS) && vis(pW, C_BG0) && vis(pW, C_BG1) && vis(pW, C_BG2) && vis(pW, C_TITLE) && vis(pW, C_BUILD) && vis(pW, C_INSTANT) && vis(pW, C_BANK) && vis(pW, C_APPLY) && vis(pW, C_CANCEL),
              "page visibility");
        CHECK(!vis(pW, C_REC) && !vis(pW, C_VANILLA), "no 'Use recommended' on the recommended preset, no Vanilla while unconfigured");
        CHECK(txt(pW, C_TITLE) == "Atton Rand - recommended build", "title '%s'", txt(pW, C_TITLE).c_str());
        CHECK(txt(pW, C_BUILD) == rp->shortName && txt(pW, C_INSTANT) == "Instant: Off" && txt(pW, C_BANK) == "Pause at level 7" && txt(pW, C_APPLY) == "Apply" && txt(pW, C_CANCEL) == "Cancel",
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
                    CHECK(txt(pW, C_ROW0 + q) == uiRowText(cp, q, b, sizeof b) && y == 2 * (162 + 30 * k) && h == 52, "row %d text '%s' y %d (slot %d)", q, txt(pW, C_ROW0 + q).c_str(), y, k);
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
        // 0.3.1 bank cycler: default (convertAt-1) -> convertAt-2 -> ... -> minPause -> Off -> default (Atton early: 7,6,...,min,Off)
        {
            const int def = rp->convertAt - 1, mn = uiMinPause(*rp);
            CHECK(rp->convertAt == 8 && def == 7 && mn >= 1 && mn <= 6, "atton early preset: convertAt %d minPause %d", rp->convertAt, mn);
            CHECK(r->bank && r->pause == 0 && txt(pW, C_BANK) == "Pause at level 7", "page opens on bank on / default");
            for (int v = def - 1; v >= mn; --v) {
                clickCtl(pW, atton, 0, C_BANK);
                char want[24]; snprintf(want, sizeof want, "Pause at level %d", v);
                CHECK(r->bank && r->pause == v && txt(pW, C_BANK) == want, "cycle -> %d: '%s' pause %d", v, txt(pW, C_BANK).c_str(), r->pause);
                CHECK(txt(pW, C_BANK).size() <= 18, "bank text <= 18 chars");
            }
            clickCtl(pW, atton, 0, C_BANK);
            CHECK(!r->bank && r->pause == 0 && txt(pW, C_BANK) == "Bank for Jedi: Off" && txt(pW, C_BANK).size() <= 18, "cycle -> Off '%s'", txt(pW, C_BANK).c_str());
            clickCtl(pW, atton, 0, C_BANK);
            CHECK(r->bank && r->pause == 0 && txt(pW, C_BANK) == "Pause at level 7", "Off -> default again '%s'", txt(pW, C_BANK).c_str());
            clickCtl(pW, atton, 0, C_BANK);   // leave on pause 6, then Off, for the checks below
            clickCtl(pW, atton, 0, C_BANK);
            while (r->bank) clickCtl(pW, atton, 0, C_BANK);
            CHECK(!r->bank && txt(pW, C_BANK) == "Bank for Jedi: Off", "bank off");
        }
        // Cancel closes
        clickCtl(pW, atton, 0, C_CANCEL);
        CHECK(!r->open && !vis(pW, C_BG0) && !vis(pW, C_APPLY) && vis(pW, C_CFG), "cancel closes");
        // reopen -> toggles reset; pick a non-recommended preset, Apply writes the line with the toggles
        clickCtl(pW, atton, 0, C_CFG);
        CHECK(r->open && r->instant == false && r->bank == true && r->pause == 0, "reopen resets toggles for an unconfigured tag");
        clickCtl(pW, atton, 0, C_ROW0 + 1);   // Role cycles (atton has several roles)
        CHECK(r->os.cur != r->recItem, "moved off the recommended preset");
        clickCtl(pW, atton, 0, C_INSTANT);
        for (int i = 0; i < 20 && r->bank; ++i) clickCtl(pW, atton, 0, C_BANK);   // cycle down to Off
        const std::string id = r->items[r->os.cur].id;
        remove("levelup_playthroughs.ini");
        CHECK(tagConfig("atton") == nullptr, "unconfigured before Apply");
        clearLog();
        clickCtl(pW, atton, 0, C_APPLY);
        CHECK(!r->open && !vis(pW, C_BG0), "page closed after Apply");
        CHECK(ptGuid() != 0 && logHas("UI apply tag=Atton preset=") && logHas("instant=1 hold=0 pause=0 ok=1"), "apply log:\n%s", readLog().c_str());
        const std::string ini = fileText("levelup_playthroughs.ini");
        CHECK(ini.find("atton=" + id + "|instant=1|hold=0") != std::string::npos, "ini line:\n%s", ini.c_str());
        { const TagCfg* c = tagConfig("atton"); CHECK(c && c->instant && !c->hold && c->preset >= 0 && g_presets[c->preset].id == id, "tagConfig switched"); }
        tick(pW, atton, 0);
        CHECK(txt(pW, C_CFG) == "Change path" && vis(pW, C_CFG) && vis(pW, C_STATUS), "cfg text after apply '%s'", txt(pW, C_CFG).c_str());
        // configured: opens on the configured preset with the config's toggles; Vanilla and Use recommended are offered
        clearLog();
        clickCtl(pW, atton, 0, C_CFG);
        CHECK(r->open && r->items[r->os.cur].id == id && logHas(("UI page open tag=Atton start=" + id + " (configured)").c_str()), "reopen on the configured preset:\n%s", readLog().c_str());
        CHECK(r->instant && !r->bank && txt(pW, C_INSTANT) == "Instant: On" && txt(pW, C_BANK) == "Bank for Jedi: Off", "toggles from the config");
        CHECK(vis(pW, C_VANILLA) && txt(pW, C_VANILLA) == "Vanilla" && vis(pW, C_REC) && txt(pW, C_TITLE).find("level-up path") != std::string::npos, "vanilla + recommended offered");
        // a preset without a hold class: bank toggle hidden, Apply writes hold=1 whatever the (hidden) toggle says
        for (int i = 0; i < 4 && g_presets[r->itemPreset[r->os.cur]].holdClass != 0; ++i) clickCtl(pW, atton, 0, C_ROW0 + 0);
        CHECK(g_presets[r->itemPreset[r->os.cur]].holdClass == 0 && !vis(pW, C_BANK) && !r->bank, "convert row reached a no-Jedi preset (bank hidden, toggle still Off)");
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
            const int mn = uiMinPause(*ep);
            clickCtl(pW, atton, 0, C_CFG);   // opens on the recommended (early) preset: bank on, pause 0
            CHECK(r->open && r->bank && r->pause == 0 && r->itemPreset[r->os.cur] == (int)(ep - g_presets), "pause block: page on the early preset");
            clickCtl(pW, atton, 0, C_BANK); clickCtl(pW, atton, 0, C_BANK);   // default -> 6 -> 5
            CHECK(r->bank && r->pause == 5 && txt(pW, C_BANK) == "Pause at level 5", "pause 5 '%s'", txt(pW, C_BANK).c_str());
            clearLog();
            clickCtl(pW, atton, 0, C_APPLY);
            CHECK(!r->open && logHas("UI apply tag=Atton preset=atton__melee_dual_light_early instant=0 hold=1 pause=5 ok=1"), "apply pause=5 log:\n%s", readLog().c_str());
            CHECK(fileText("levelup_playthroughs.ini").find("atton=atton__melee_dual_light_early|instant=0|hold=1|pause=5\n") != std::string::npos, "ini |pause=5:\n%s", fileText("levelup_playthroughs.ini").c_str());
            { const TagCfg* c = tagConfig("atton"); CHECK(c && c->pause == 5 && c->hold, "tagConfig pause 5"); }
            // held-by-pause status line: Scoundrel 5 -> level 6 banked once past pause 5 (Scoundrel 5 = key 6 > 5)
            clickCtl(pW, atton, 0, C_CFG);
            CHECK(r->open && r->bank && r->pause == 5 && txt(pW, C_BANK) == "Pause at level 5", "reopen from config shows pause 5 '%s'", txt(pW, C_BANK).c_str());
            // cycle the Convert row (early -> late -> never ...): pause is clamped into each preset's range, 0 without a hold class
            int expP = 5;   // once clamped to 0 (different range) the value stays 0
            for (int i = 0; i < 4; ++i) {
                clickCtl(pW, atton, 0, C_ROW0 + 0);
                const Preset& cp = g_presets[r->itemPreset[r->os.cur]];
                expP = uiClampPause(cp, expP);
                CHECK(r->pause == expP && (cp.holdClass == 0 ? r->pause == 0 : true) && (r->pause == 0 || (r->pause >= uiMinPause(cp) && r->pause <= cp.convertAt - 2)),
                      "row cycle %d: pause %d on %s (convertAt %d hold %d)", i, r->pause, cp.id, cp.convertAt, cp.holdClass);
                if (cp.holdClass && cp.convertAt > 0) { char want[24]; snprintf(want, sizeof want, r->bank ? "Pause at level %d" : "Bank for Jedi: Off", r->pause > 0 ? r->pause : cp.convertAt - 1);
                    CHECK(txt(pW, C_BANK) == want, "bank text '%s' vs '%s'", txt(pW, C_BANK).c_str(), want); }
            }
            // forced clamp: a preset with convertAt 5 cannot keep pause 5 (range ends at 3)
            { Preset fake = *ep; fake.convertAt = 5;
              CHECK(uiClampPause(fake, 5) == 0 && uiClampPause(fake, 4) == 0 && uiClampPause(fake, 3) == (3 >= uiMinPause(fake) ? 3 : 0), "uiClampPause range end convertAt-2");
              CHECK(uiClampPause(*ep, 0) == 0 && uiClampPause(*ep, -1) == 0 && uiClampPause(*ep, ep->convertAt - 1) == 0 && uiClampPause(*ep, ep->convertAt) == 0 && uiClampPause(*ep, mn - 1) == 0 && uiClampPause(*ep, mn) == mn && uiClampPause(*ep, ep->convertAt - 2) == ep->convertAt - 2, "uiClampPause bounds");
              fake.holdClass = 0; CHECK(uiClampPause(fake, 4) == 0, "no hold class -> 0");
              bool bk = true; int pz = 0; Preset tiny = *ep; tiny.convertAt = mn + 1;   // range empty: default -> Off -> default
              uiBankCycle(tiny, &bk, &pz); CHECK(!bk && pz == 0, "empty range: default -> Off");
              uiBankCycle(tiny, &bk, &pz); CHECK(bk && pz == 0, "Off -> default");
              char tb[24]; CHECK(!strcmp(uiBankText(tiny, true, 0, tb, sizeof tb), "Pause at level 1") || tiny.convertAt - 1 != 1, "default text"); }
            // A3 mutation gaps: absolute (non-tautological) expectations on synthetic presets
            { Preset sp = *ep; for (int cl = 0; cl <= MaxLevel; ++cl) sp.lv[cl].present = false;
              sp.holdClass = 5; sp.convertAt = 10;
              sp.lv[4].present = true; CHECK(uiMinPause(sp) == 3, "uiMinPause first record at char level 4 -> 3 (got %d)", uiMinPause(sp));
              sp.lv[4].present = false; sp.lv[2].present = true; CHECK(uiMinPause(sp) == 1, "uiMinPause first record at level 2 -> 1 (got %d)", uiMinPause(sp));
              sp.lv[2].present = false; sp.lv[1].present = true; CHECK(uiMinPause(sp) == 1, "uiMinPause first record at level 1 -> 1");
              CHECK(uiCanPause(sp), "holdClass + convertAt -> can pause");
              Preset a = sp; a.holdClass = 0; CHECK(!uiCanPause(a), "no hold class -> cannot pause (convertAt set)");
              Preset b = sp; b.convertAt = 0; CHECK(!uiCanPause(b), "no convertAt -> cannot pause (hold class set)");
              bool bk = true; int pz = 6; uiBankCycle(a, &bk, &pz); CHECK(!bk && pz == 0, "non-pausable toggle: On -> Off, stale pause cleared (pause %d)", pz);
              bk = false; pz = 6; uiBankCycle(a, &bk, &pz); CHECK(bk && pz == 0, "non-pausable toggle: Off -> On, stale pause cleared (pause %d)", pz);
              bk = false; pz = 4; uiBankCycle(sp, &bk, &pz); CHECK(bk && pz == 0, "Off -> default clears the pause"); }
            // Use recommended keeps the toggles and clamps
            clickCtl(pW, atton, 0, C_CANCEL);
            clickCtl(pW, atton, 0, C_CFG);
            CHECK(r->pause == 5, "reopen: pause 5 again");
            for (int i = 0; i < 4 && r->os.cur == r->recItem; ++i) clickCtl(pW, atton, 0, C_ROW0 + 0);
            clickCtl(pW, atton, 0, C_REC);
            CHECK(r->os.cur == r->recItem && r->bank && r->pause == uiClampPause(*ep, r->pause), "Use recommended keeps the bank state (pause %d)", r->pause);
            // Off applies hold=0 pause=0 and drops the field
            while (r->bank) clickCtl(pW, atton, 0, C_BANK);
            clearLog();
            clickCtl(pW, atton, 0, C_APPLY);
            CHECK(logHas("hold=0 pause=0 ok=1") && fileText("levelup_playthroughs.ini").find("pause") == std::string::npos, "bank off: pause dropped:\n%s", fileText("levelup_playthroughs.ini").c_str());
            // Apply with bank OFF never stores a pause, even when a stale config row (hold=0, pause=N) opened the page with one
            { CHECK(ptApply("atton", ep->id, false, false, mn), "ptApply hold=0 with a pause");
              clickCtl(pW, atton, 0, C_CFG);
              CHECK(r->open && !r->bank && r->pause == mn, "stale hold=0 pause=%d opens bank off with the pause (bank %d pause %d)", mn, r->bank, r->pause);
              clearLog(); clickCtl(pW, atton, 0, C_APPLY);
              CHECK(logHas("hold=0 pause=0 ok=1") && fileText("levelup_playthroughs.ini").find("pause") == std::string::npos, "Apply with bank off drops the stale pause:\n%s", readLog().c_str());
              }
            // stale / out-of-range config (hand-edited |pause=99) is clamped on page open and never applied raw
            CHECK(ptApply("atton", PRESET, false, true, 99), "ptApply pause 99");
            clickCtl(pW, atton, 0, C_CFG);
            CHECK(r->bank && r->pause == 0 && txt(pW, C_BANK) == "Pause at level 7", "pause 99 clamped on open '%s'", txt(pW, C_BANK).c_str());
            clickCtl(pW, atton, 0, C_CANCEL);
            CHECK(ptRemove("atton") && tagConfig("atton") == nullptr, "pause block cleanup");
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
        { int first = -1; for (int q = 0; q < NumRows && first < 0; ++q) if (vis(pW, C_ROW0 + q)) first = q;
          int x, y, w, h; getExtent(ctl(pW, C_ROW0 + first), &x, &y, &w, &h);
          CHECK(first > 0 && y == 2 * 162, "kreia: first shown row %d sits in slot 0 (y %d)", first, y); }
        { const int before = logCount("UI click"); clickCtl(pW, other, 0, C_BANK); CHECK(logCount("UI click") == before && !r->instant, "hidden bank toggle ignored"); }
        { int x, y, w, h; getExtent(ctl(pW, C_ROW0 + 0), &x, &y, &w, &h); (void)x; (void)y; (void)w; (void)h; }
        // PC leader hides + resets the page
        tick(pW, pc, 0);
        CHECK(!r->open && !anyVisible(pW), "PC leader hides + closes the page");
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
            CHECK(y == 2 * (162 + 30 * slot) && txt(pW, C_ROW0 + opt::VariantRow) == std::string("Variant: ") + r->items[before].shortName, "variant row last slot (y %d slot %d) '%s'", y, slot, txt(pW, C_ROW0 + opt::VariantRow).c_str());
            clickCtl(pW, g0t0, 0, C_ROW0 + opt::VariantRow);
            CHECK(r->os.cur != before && txt(pW, C_BUILD) == r->items[r->os.cur].shortName && txt(pW, C_ROW0 + opt::VariantRow) == std::string("Variant: ") + r->items[r->os.cur].shortName, "variant click cycles to %d", r->os.cur);
        }
        clickCtl(pW, g0t0, 0, C_CANCEL);
        // a table change (stamp) closes the page
        tick(pW, atton, 0);
        clickCtl(pW, atton, 0, C_CFG);
        CHECK(r->open, "open before the table change");
        g_tableStamp ^= 1;
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

    printf("ui_test checks %d failures %d\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
