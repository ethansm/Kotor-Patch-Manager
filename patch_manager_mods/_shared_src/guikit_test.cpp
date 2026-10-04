// Host test of the GuiKit.h additions: imageFor out-params / 256 pool, createLabelFromTemplate failure reason.
// Fake engine in a MAP_32BIT arena (see research/companion_levelup/tools/parity/guikit_test.cpp for the full parity test).
#define GUIKIT_HOST 1
#include "GuiKit.h"
#include "check.h"
#include <sys/mman.h>
#include <stdio.h>

using namespace guikit;
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); } uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static int nLoad = 0, nAlloc = 0; static void* loadResult = reinterpret_cast<void*>(0x5000);
static bool allocFails = false; static uint32_t ctorVtable = LabelVtable; static bool initWritesId = true;
static void* fLoadImage(const char*) { ++nLoad; return loadResult; }
static void* fAlloc(unsigned n) { ++nAlloc; return allocFails ? nullptr : reinterpret_cast<void*>(arenaAlloc(n)); }
static void fCtor(void* t) { uintptr_t c = reinterpret_cast<uintptr_t>(t); at<uint32_t>(c, 0) = ctorVtable; at<int>(c, Ctl_Flags) = 0x2; }
static void* fExoCtor(void* e, const char* s) { memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fExoDtor(void*) {}
static void fInit(void*, void* ctl, void*, int, int) { if (initWritesId) at<int>(reinterpret_cast<uintptr_t>(ctl), Ctl_Id) = -1; }

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    void* vp = mmap(reinterpret_cast<void*>(0x00987000), 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (vp == MAP_FAILED) { perror("mmap 0x987000"); return 5; }
    Engine& e = g_gk;
    e.alloc = fAlloc; e.labelCtor = fCtor; e.initControl = fInit; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.loadImage = fLoadImage;

    // ---- imageFor (the pool is a function-local static: order matters) ----
    bool loaded = true; int pc = -1;
    KT_CHECK_EQ(reinterpret_cast<uintptr_t>(imageFor("", &loaded, &pc)), 0ull);
    KT_CHECK(!loaded); KT_CHECK_EQ(pc, 0);
    loaded = true; KT_CHECK(imageFor(nullptr, &loaded, &pc) == nullptr && !loaded && pc == 0);
    void* a = imageFor("FPH_Slot", &loaded, &pc);
    KT_CHECK(loaded); KT_CHECK_EQ(pc, 1);
    KT_CHECK(a == loadResult); KT_CHECK_EQ(nLoad, 1);
    void* b = imageFor("FPH_Slot", &loaded, &pc);
    KT_CHECK(!loaded); KT_CHECK_EQ(pc, 1);
    KT_CHECK(b == a);
    void* c = imageFor("fph_slot", &loaded, &pc);
    KT_CHECK(!loaded && pc == 1 && c == a); KT_CHECK_EQ(nLoad, 1);
    KT_CHECK(imageFor("fph_slot") == a);
    // null result is cached
    loadResult = nullptr;
    KT_CHECK(imageFor("nullimg", &loaded, &pc) == nullptr && loaded && pc == 2);
    KT_CHECK(imageFor("nullimg", &loaded, &pc) == nullptr && !loaded && pc == 2);
    KT_CHECK_EQ(nLoad, 2);
    loadResult = reinterpret_cast<void*>(0x5000);
    KT_CHECK(imageFor("plain_new") == loadResult);   // pool = 3
    // fill to ImagePoolSize
    char nm[24]; bool allOk = true;
    for (int i = 3; i < ImagePoolSize; ++i) { snprintf(nm, sizeof nm, "fill%d", i); allOk &= imageFor(nm, &loaded, &pc) != nullptr && loaded && pc == i + 1; }
    KT_CHECK(allOk); KT_CHECK_EQ(ImagePoolSize, 256);
    int before = nLoad;
    void* full = imageFor("one_too_many", &loaded, &pc);
    KT_CHECK(full == nullptr); KT_CHECK(!loaded);
    KT_CHECK_EQ(pc, ImagePoolSize); KT_CHECK_EQ(nLoad, before);
    KT_CHECK(imageFor("fph_slot", &loaded, &pc) == a && !loaded && pc == ImagePoolSize);

    // ---- createLabelFromTemplate ----
    uintptr_t panel = arenaAlloc(0x100); at<int>(panel, Panel_CtlCount) = 7;
    int why = -1; uint32_t bad = 123;
    uintptr_t lbl = createLabelFromTemplate(panel, "LBL_X", &why, &bad);
    KT_CHECK(lbl != 0); KT_CHECK_EQ(why, (int)CF_None);
    KT_CHECK(lbl && (at<int>(lbl, Ctl_Flags) & FlagVisible) == 0 && (at<int>(lbl, Ctl_Flags) & FlagClickThrough));
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X") != 0);

    allocFails = true; why = -1;
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Alloc);
    allocFails = false;
    why = -1; KT_CHECK(createLabelFromTemplate(0, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Alloc);

    ctorVtable = 0xABCD0000u; why = -1; bad = 0;
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why, &bad) == 0);
    KT_CHECK_EQ(why, (int)CF_Vtable); KT_CHECK_EQ((unsigned long long)bad, 0xABCD0000ull);
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0 && why == CF_Vtable);
    ctorVtable = LabelVtable;

    initWritesId = false; why = -1;
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Template);
    initWritesId = true;

    why = -1;   // panel whose count field (0x38) sits in the unreadable low page
    KT_CHECK(createLabelFromTemplate(0x10, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Count);

    why = -1; KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) != 0 && why == CF_None);

    // ---- flags, border readback, units, cursor, hit, clickEdge ----
    {
        uintptr_t k = arenaAlloc(0x200);
        at<int>(k, Ctl_Flags) = FlagVisible | FlagClickThrough;
        KT_CHECK(setClickable(k, true)); KT_CHECK_EQ(at<int>(k, Ctl_Flags), (int)FlagVisible);       // clears bit5 (write path)
        KT_CHECK(setClickable(k, true)); KT_CHECK_EQ(at<int>(k, Ctl_Flags), (int)FlagVisible);       // no-op path returns true
        KT_CHECK(setClickable(k, false)); KT_CHECK_EQ(at<int>(k, Ctl_Flags), (int)(FlagVisible | FlagClickThrough));
        KT_CHECK(!setClickable(0, true));

        // border tint / alpha: gated on the border vtable at ctl+Label_Border
        float rgb[3] = { -1.f, -1.f, -1.f }, al = -1.f;
        KT_CHECK(!getBorderTint(k, rgb)); KT_CHECK(!getAlpha(k, &al));                                // vtable not set
        at<uint32_t>(k, Label_Border) = 0x9875BC;
        at<float>(k, Label_Border + Border_Tint) = 0.25f; at<float>(k, Label_Border + Border_Tint + 4) = 0.5f; at<float>(k, Label_Border + Border_Tint + 8) = 0.75f;
        at<float>(k, Label_Border + Border_Alpha) = 0.125f;
        KT_CHECK(getBorderTint(k, rgb)); KT_CHECK(rgb[0] == 0.25f && rgb[1] == 0.5f && rgb[2] == 0.75f);
        KT_CHECK(getAlpha(k, &al)); KT_CHECK(al == 0.125f);
        KT_CHECK(!getBorderTint(k, nullptr)); KT_CHECK(!getBorderTint(0, rgb)); KT_CHECK(!getAlpha(k, nullptr)); KT_CHECK(!getAlpha(0, &al));

        // scaleFromRef: ref ctl px rect (100,50,800,600) vs layout units (10,20,400,300) -> s = 2, origin (100,50)
        at<int>(k, Ctl_X) = 100; at<int>(k, Ctl_Y) = 50; at<int>(k, Ctl_W) = 800; at<int>(k, Ctl_H) = 600;
        Scale s = scaleFromRef(k, 10, 20, 400, 300);
        KT_CHECK(s.ok); KT_CHECK(s.sx == 2.f && s.sy == 2.f && s.ox == 100.f && s.oy == 50.f); KT_CHECK(s.refX == 10 && s.refY == 20);
        KT_CHECK_EQ(unitX(s, 30.f), 140);   // 100 + (30-10)*2
        KT_CHECK_EQ(unitY(s, 120.f), 250);  // 50 + (120-20)*2
        int rx = -1, ry = -1, rw = -1, rh = -1;
        unitRect(s, 20, 30, 5, 3, &rx, &ry, &rw, &rh);
        KT_CHECK(rx == 120 && ry == 70 && rw == 10 && rh == 6);   // x0=120, x1=unitX(25)=130; y0=70, y1=unitY(33)=76
        unitRect(s, 20, 30, 5, 3, nullptr, nullptr, nullptr, nullptr);   // null out-params are skipped
        rw = -1; unitRect(s, 20, 30, 5, 3, nullptr, nullptr, &rw, nullptr); KT_CHECK_EQ(rw, 10);
        // fractional scale: edges rounded first, size = difference of rounded edges
        at<int>(k, Ctl_X) = 0; at<int>(k, Ctl_Y) = 0; at<int>(k, Ctl_W) = 10; at<int>(k, Ctl_H) = 7;
        Scale f = scaleFromRef(k, 0, 0, 3, 4);   // sx = 10/3, sy = 1.75
        KT_CHECK(f.ok);
        unitRect(f, 1, 1, 1, 1, &rx, &ry, &rw, &rh);
        KT_CHECK(rx == 3 && rw == 4 && ry == 2 && rh == 2);   // x: lround(3.33)=3, lround(6.67)=7; y: lround(1.75)=2, lround(3.5)=4
        // failure paths: identity scale, ok=false, ref unit carried through
        Scale b1 = scaleFromRef(k, 5, 6, 0, 4), b2 = scaleFromRef(k, 5, 6, 4, -1), b3 = scaleFromRef(0, 5, 6, 4, 4);
        KT_CHECK(!b1.ok && !b2.ok && !b3.ok);
        KT_CHECK(b1.sx == 1.f && b1.sy == 1.f && b1.ox == 0.f && b1.oy == 0.f && b1.refX == 5 && b1.refY == 6);
        KT_CHECK_EQ(unitX(b1, 9.f), 4);   // identity: 0 + (9-5)*1
        at<int>(k, Ctl_W) = 0; KT_CHECK(!scaleFromRef(k, 0, 0, 3, 4).ok);
        at<int>(k, Ctl_W) = 10; at<int>(k, Ctl_H) = -2; KT_CHECK(!scaleFromRef(k, 0, 0, 3, 4).ok);

        // hit / isVisible
        at<int>(k, Ctl_X) = 10; at<int>(k, Ctl_Y) = 20; at<int>(k, Ctl_W) = 30; at<int>(k, Ctl_H) = 40; at<int>(k, Ctl_Flags) = FlagVisible;
        KT_CHECK(hit(k, 10, 20)); KT_CHECK(hit(k, 39, 59)); KT_CHECK(hit(k, 25, 30));
        KT_CHECK(!hit(k, 40, 30)); KT_CHECK(!hit(k, 25, 60));   // x == ex+ew, y == ey+eh are exclusive
        KT_CHECK(!hit(k, 9, 30)); KT_CHECK(!hit(k, 25, 19));
        at<int>(k, Ctl_Flags) = 0; KT_CHECK(!hit(k, 25, 30));    // invisible
        KT_CHECK(!hit(0, 25, 30));

        // cursorEngine
        Engine saved = g_gk;
        static int cCx = 100, cCy = 50, cCw = 800, cCh = 600; static bool cOk = true;
        g_gk.cursorClient = [](int* x, int* y, int* w, int* h) { *x = cCx; *y = cCy; *w = cCw; *h = cCh; return cOk; };
        uintptr_t gw = arenaAlloc(16), gh = gw + 4;
        g_gk.screenWAddr = gw; g_gk.screenHAddr = gh;
        int ox = -1, oy = -1;
        at<int>(gw, 0) = 1600; at<int>(gh, 0) = 1200;
        KT_CHECK(cursorEngine(&ox, &oy)); KT_CHECK(ox == 200 && oy == 100);   // 100*1600/800, 50*1200/600
        at<int>(gw, 0) = 0;                                                    // out of range -> client px
        KT_CHECK(cursorEngine(&ox, &oy)); KT_CHECK(ox == 100 && oy == 50);
        at<int>(gw, 0) = 1600; at<int>(gh, 0) = 199;
        KT_CHECK(cursorEngine(&ox, &oy)); KT_CHECK(ox == 100 && oy == 50);
        at<int>(gh, 0) = 1200; at<int>(gw, 0) = 20000;
        KT_CHECK(cursorEngine(&ox, &oy)); KT_CHECK(ox == 100 && oy == 50);
        g_gk.screenWAddr = 0; g_gk.screenHAddr = 0;                            // unknown addresses -> client px
        KT_CHECK(cursorEngine(&ox, &oy)); KT_CHECK(ox == 100 && oy == 50);
        g_gk.screenWAddr = gw; g_gk.screenHAddr = gh; at<int>(gw, 0) = 1600;
        cCw = 0; KT_CHECK(!cursorEngine(&ox, &oy)); cCw = 800;
        cCh = 0; KT_CHECK(!cursorEngine(&ox, &oy)); cCh = 600;
        cOk = false; KT_CHECK(!cursorEngine(&ox, &oy)); cOk = true;
        KT_CHECK(!cursorEngine(nullptr, &oy)); KT_CHECK(!cursorEngine(&ox, nullptr));
        g_gk.cursorClient = nullptr; KT_CHECK(!cursorEngine(&ox, &oy));

        // clickEdge
        static bool fg = true, dn = false;
        g_gk.gameForeground = [] { return fg; }; g_gk.lbuttonDown = [] { return dn; };
        ClickEdge ce;
        KT_CHECK(!clickEdge(ce));
        dn = true; KT_CHECK(clickEdge(ce));
        KT_CHECK(!clickEdge(ce)); KT_CHECK(!clickEdge(ce));         // held: fires once
        dn = false; KT_CHECK(!clickEdge(ce));
        dn = true; KT_CHECK(clickEdge(ce));                          // new press fires again
        fg = false; KT_CHECK(!clickEdge(ce)); KT_CHECK(!ce.prev);    // lost focus counts as up
        fg = true; KT_CHECK(clickEdge(ce));                          // press that resumes in foreground re-fires
        g_gk.gameForeground = nullptr; KT_CHECK(!clickEdge(ce));
        g_gk.gameForeground = [] { return true; }; g_gk.lbuttonDown = nullptr;
        KT_CHECK(!clickEdge(ce));
        g_gk = saved;
    }
    return kt::Report("guikit_test");
}
