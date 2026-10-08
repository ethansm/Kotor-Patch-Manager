// Host test of patch_manager_mods/_shared_src/GuiKit.h on fake engine structs in a MAP_32BIT arena (no -m32 on this host).
// Engine calls and platform queries are fakes recorded in g_calls; the label vtable (0x9878BC) is a real low-address page so
// the slot-1 read works. Run via guikit_test.py.
#define GUIKIT_HOST 1
#include "GuiKit.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 4u << 20;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); if (g_used + n > ArenaSize) { fprintf(stderr, "arena full\n"); exit(4); } uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
#include "TestHarness.h"
#define CHECK(c, ...) do { \
    char _detail[512]; \
    std::snprintf(_detail, sizeof(_detail), __VA_ARGS__); \
    ::kt::Check(__FILE__ ":" KT_STR(__LINE__) ": " #c, static_cast<bool>(c), _detail); \
} while (0)
using namespace guikit;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// ---- fakes ----
static int nAlloc = 0, nCtor = 0, nInit = 0, nSetRect = 0, nTextSet = 0, nFont = 0, nAlign = 0, nColor = 0, nList = 0, nSound = 0;
static char lastTag[16], lastText[64], lastFont[16]; static int lastAddToList = -1, lastInitScale = -1, lastRect[4], lastAlign;
static float lastColor[3]; static void* lastInitPanel; static uintptr_t lastSetRectCtl; static uint32_t lastSetRectFn; static int lastSoundIdx; static void* lastSoundMgr;
static void* fAlloc(unsigned n) { ++nAlloc; return reinterpret_cast<void*>(arenaAlloc(n)); }
static void fCtor(void* t) { ++nCtor; uintptr_t c = reinterpret_cast<uintptr_t>(t); at<uint32_t>(c, 0) = LabelVtable; at<uint32_t>(c, Label_Border) = BorderVtable; at<uint32_t>(c, Label_Text) = TextVtable; at<uint32_t>(c, Label_Text + Text_Renderer) = 0x1000; at<int>(c, Ctl_Flags) = 0x2; }
static void* fExoCtor(void* e, const char* s) { memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fExoDtor(void*) {}
static void fInit(void* panel, void* ctl, void* exo, int addToList, int scale) {
    ++nInit; lastInitPanel = panel; lastAddToList = addToList; lastInitScale = scale; strncpy(lastTag, static_cast<char*>(exo), 15);
    if (strcmp(lastTag, "LBL_MISSING")) at<int>(reinterpret_cast<uintptr_t>(ctl), Ctl_Id) = -1;   // Load overwrote the sentinel
}
static void fTextSet(void* sub, void* exo) { ++nTextSet; strncpy(lastText, static_cast<char*>(exo), 63); (void)sub; }
static void fFont(void*, const char* r) { ++nFont; strncpy(lastFont, r, 15); }
static void fAlign(void*, unsigned a) { ++nAlign; lastAlign = (int)a; }
static void fColor(void*, float* v) { ++nColor; memcpy(lastColor, v, 12); }
static void fList(void* list, void* v) { ++nList; uintptr_t l = reinterpret_cast<uintptr_t>(list); uint32_t arr = at<uint32_t>(l, 0); int cnt = at<int>(l, 4); at<uint32_t>(arr, cnt * 4) = (uint32_t)reinterpret_cast<uintptr_t>(v); at<int>(l, 4) = cnt + 1; }
static void* fLoadImage(const char*) { return reinterpret_cast<void*>(0x5000); }
static void fSound(void* m, char i) { ++nSound; lastSoundMgr = m; lastSoundIdx = i; }
static void fSetRect(uint32_t fn, uintptr_t ctl, int* r) { ++nSetRect; lastSetRectFn = fn; lastSetRectCtl = ctl; memcpy(lastRect, r, 16); for (int k = 0; k < 4; ++k) at<int>(ctl, Ctl_X + 4 * k) = r[k]; }
static int curX = 0, curY = 0, curW = 1920, curH = 800; static bool fg = true, lmb = false, haveCur = true;
static bool fCursor(int* x, int* y, int* w, int* h) { if (!haveCur) return false; *x = curX; *y = curY; *w = curW; *h = curH; return true; }
static bool fFg() { return fg; }
static bool fLmb() { return lmb; }

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    void* vp = mmap(reinterpret_cast<void*>(0x00987000), 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (vp == MAP_FAILED) { perror("mmap 0x987000"); return 5; }
    at<uint32_t>(LabelVtable, 4) = 0x1234;   // vtable slot 1 = fake SetRect id

    uintptr_t screen = arenaAlloc(16); at<int>(screen, 0) = 3840; at<int>(screen, 4) = 1600;
    uintptr_t snd = arenaAlloc(16);
    Engine& e = g_gk;
    e.alloc = fAlloc; e.labelCtor = fCtor; e.initControl = fInit; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.textSet = fTextSet; e.setFont = fFont; e.setAlign = fAlign;
    e.textColor = fColor; e.listAdd = fList; e.loadImage = fLoadImage; e.playSound = fSound; e.callSetRect = fSetRect;
    e.screenWAddr = screen; e.screenHAddr = screen + 4; e.guiSoundMgrAddr = snd;
    e.cursorClient = fCursor; e.gameForeground = fFg; e.lbuttonDown = fLmb;

    // panel with an empty list (array room for 16)
    uintptr_t panel = arenaAlloc(0x100), arr = arenaAlloc(64);
    at<uint32_t>(panel, Panel_CtlArray) = (uint32_t)arr; at<int>(panel, Panel_CtlCount) = 7;

    // ---- create ----
    uintptr_t a = createLabelFromTemplate(panel, "LBL_QUEUE1");
    CHECK(a != 0, "create returned 0");
    CHECK(nAlloc == 1 && nCtor == 1 && nInit == 1, "alloc/ctor/init counts %d %d %d", nAlloc, nCtor, nInit);
    CHECK(!strcmp(lastTag, "LBL_QUEUE1") && lastInitPanel == reinterpret_cast<void*>(panel) && lastAddToList == 0 && lastInitScale == 1, "init args tag=%s addToList=%d scale=%d", lastTag, lastAddToList, lastInitScale);
    CHECK(at<int>(panel, Panel_CtlCount) == 7, "create must not append");
    CHECK((at<int>(a, Ctl_Flags) & 0x2) == 0 && (at<int>(a, Ctl_Flags) & 0x20), "created hidden + click-through, flags %x", at<int>(a, Ctl_Flags));
    CHECK(createLabelFromTemplate(panel, "LBL_MISSING") == 0, "missing template must return 0");
    CHECK(createLabelFromTemplate(0, "X") == 0 && createLabelFromTemplate(panel, nullptr) == 0, "null args");

    // ---- append ----
    CHECK(appendToPanel(panel, a), "append failed");
    CHECK(at<int>(panel, Panel_CtlCount) == 8 && at<uint32_t>(arr, 7 * 4) == (uint32_t)a && at<int>(a, Ctl_Id) == 7, "append state count=%d id=%d", at<int>(panel, Panel_CtlCount), at<int>(a, Ctl_Id));
    CHECK(!appendToPanel(0, a) && !appendToPanel(panel, 0), "append null");

    // ---- visibility / clickable ----
    CHECK(setVisible(a, true), "setVisible true");
    CHECK((at<int>(a, Ctl_Flags) & 0x2) && !(at<int>(a, Ctl_Flags) & 0x20), "shown flags %x", at<int>(a, Ctl_Flags));
    CHECK(isVisible(a), "isVisible");
    CHECK(setVisible(a, false) && !(at<int>(a, Ctl_Flags) & 0x2) && (at<int>(a, Ctl_Flags) & 0x20), "hidden flags %x", at<int>(a, Ctl_Flags));
    CHECK(setClickable(a, true) && !(at<int>(a, Ctl_Flags) & 0x20) && setClickable(a, false) && (at<int>(a, Ctl_Flags) & 0x20), "clickable toggle");

    // ---- rect / extent ----
    CHECK(setRect(a, 10, 20, 300, 40), "setRect");
    CHECK(nSetRect == 1 && lastSetRectFn == 0x1234 && lastSetRectCtl == a && lastRect[0] == 10 && lastRect[1] == 20 && lastRect[2] == 300 && lastRect[3] == 40, "setRect args");
    int x, y, w, h; CHECK(getExtent(a, &x, &y, &w, &h) && x == 10 && y == 20 && w == 300 && h == 40, "getExtent %d %d %d %d", x, y, w, h);

    // ---- text / font / align / colour ----
    CHECK(setText(a, "hello") && nTextSet == 1 && !strcmp(lastText, "hello"), "setText");
    CHECK(setFont(a, "fnt_d16x16") && !strcmp(lastFont, "fnt_d16x16"), "setFont");
    CHECK(setAlign(a, 18) && lastAlign == 18, "setAlign");
    CHECK(setTextColor(a, 0.25f, 0.5f, 0.75f) && lastColor[0] == 0.25f && lastColor[2] == 0.75f, "setTextColor");
    at<float>(a, Label_Text + Text_StringSub + Text_Color) = 1.f; at<float>(a, Label_Text + Text_StringSub + Text_Color + 8) = 0.5f;
    float tc[3]; CHECK(getTextColor(a, tc) && tc[0] == 1.f && tc[2] == 0.5f, "getTextColor readback");
    at<uint32_t>(a, Label_Text) = 0;   // bad text object: every text call refuses
    int before = nTextSet + nFont + nAlign + nColor;
    CHECK(!setText(a, "x") && !setFont(a, "f") && !setAlign(a, 1) && !setTextColor(a, 1, 1, 1) && !getTextColor(a, tc) && nTextSet + nFont + nAlign + nColor == before, "bad text vtable must refuse");
    at<uint32_t>(a, Label_Text) = TextVtable;

    // ---- fill / alpha / tint ----
    void* im = imageFor("FPH_Slot"); CHECK(im == reinterpret_cast<void*>(0x5000) && imageFor("fph_slot") == im, "imageFor pool");
    float tint[3] = {0.1f, 0.2f, 0.3f};
    CHECK(setFill(a, im, tint) && at<uint32_t>(a, Label_Border + Border_Fill) == 0x5000 && at<float>(a, Label_Border + Border_Tint + 4) == 0.2f, "setFill");
    float bt[3]; CHECK(getBorderTint(a, bt) && bt[2] == 0.3f, "getBorderTint");
    CHECK(setAlpha(a, 0.5f) && at<float>(a, Label_Border + Border_Alpha) == 0.5f, "setAlpha");
    float al = 0; CHECK(getAlpha(a, &al) && al == 0.5f, "getAlpha");
    at<uint32_t>(a, Label_Border) = 0;
    CHECK(!setFill(a, im) && !setAlpha(a, 1.f) && !getAlpha(a, &al) && at<float>(a, Label_Border + Border_Alpha) == 0.5f, "bad border vtable must refuse");
    at<uint32_t>(a, Label_Border) = BorderVtable;

    // ---- scale ----
    uintptr_t ref = arenaAlloc(0x150); at<uint32_t>(ref, 0) = LabelVtable;
    at<int>(ref, Ctl_X) = 1000; at<int>(ref, Ctl_Y) = 400; at<int>(ref, Ctl_W) = 760; at<int>(ref, Ctl_H) = 800;   // unit size 380x400 -> 2.0
    Scale s = scaleFromRef(ref, 362, 325, 380, 400);
    CHECK(s.ok && s.sx == 2.f && s.sy == 2.f && s.ox == 1000.f && s.oy == 400.f, "scale %.2f %.2f %.0f %.0f", s.sx, s.sy, s.ox, s.oy);
    unitRect(s, 362, 325, 10, 5, &x, &y, &w, &h);   CHECK(x == 1000 && y == 400 && w == 20 && h == 10, "unitRect origin %d %d %d %d", x, y, w, h);
    unitRect(s, 372, 335, 100, 20, &x, &y, &w, &h); CHECK(x == 1020 && y == 420 && w == 200 && h == 40, "unitRect offset %d %d %d %d", x, y, w, h);
    CHECK(!scaleFromRef(0, 362, 325, 380, 400).ok && !scaleFromRef(ref, 362, 325, 0, 400).ok, "scale null/zero");
    at<int>(ref, Ctl_W) = 0; CHECK(!scaleFromRef(ref, 362, 325, 380, 400).ok, "scale zero-width ref");

    // ---- hit ----
    setRect(a, 100, 100, 50, 20); setVisible(a, true);
    CHECK(hit(a, 100, 100) && hit(a, 149, 119) && !hit(a, 150, 100) && !hit(a, 100, 120) && !hit(a, 99, 100), "hit bounds");
    setVisible(a, false); CHECK(!hit(a, 110, 110), "hit false when hidden");

    // ---- cursor ----
    curX = 960; curY = 400; int cx, cy;
    CHECK(cursorEngine(&cx, &cy) && cx == 1920 && cy == 800, "cursor scaled to framebuffer %d,%d", cx, cy);
    at<int>(screen, 0) = 0; CHECK(cursorEngine(&cx, &cy) && cx == 960 && cy == 400, "cursor fallback client px %d,%d", cx, cy);
    haveCur = false; CHECK(!cursorEngine(&cx, &cy), "no cursor"); haveCur = true;

    // ---- click edge ----
    ClickEdge ce; int clicks = 0;
    bool seq[] = {false, true, true, true, false, true, false};
    for (bool d : seq) { lmb = d; clicks += clickEdge(ce); }
    CHECK(clicks == 2, "rising edges %d (want 2)", clicks);
    ClickEdge c2; lmb = true; fg = false; CHECK(!clickEdge(c2) && !c2.prev, "not foreground -> no click");
    fg = true; CHECK(clickEdge(c2), "press after regaining focus fires"); CHECK(!clickEdge(c2), "held does not refire");

    // ---- sound ----
    CHECK(!playHoverSound(), "null manager -> false");
    at<uint32_t>(snd, 0) = (uint32_t)arenaAlloc(16);
    CHECK(playHoverSound() && nSound == 1 && lastSoundIdx == 1 && lastSoundMgr == reinterpret_cast<void*>(at<uint32_t>(snd, 0)), "hover sound");

    // ---- null ctl ----
    CHECK(!setRect(0, 1, 2, 3, 4) && !setVisible(0, true) && !setClickable(0, true) && !setText(0, "x") && !setTextColor(0, 1, 1, 1) && !setFill(0, nullptr)
          && !setAlpha(0, 1) && !getExtent(0, &x, &y, &w, &h) && !hit(0, 1, 1) && !setFont(0, "f") && !setAlign(0, 1) && !getTextColor(0, tc) && !isVisible(0), "null ctl");
    CHECK(!setText(a, nullptr), "null text");
    // unreadable ctl (low page): guarded, no crash
    CHECK(!setVisible(0x10, true) && !setText(0x10, "x") && !getExtent(0x10, &x, &y, &w, &h), "garbage ctl");

    return kt::Report("guikit_test");
}
