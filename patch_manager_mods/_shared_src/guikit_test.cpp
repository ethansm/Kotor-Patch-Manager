// Host test of the GuiKit.h additions: imageFor out-params / 256 pool, createLabelFromTemplate failure reason.
// Fake engine in a MAP_32BIT arena (see research/companion_levelup/tools/parity/guikit_test.cpp for the full parity test).
#define GUIKIT_HOST 1
#include "GuiKit.h"
#include "check.h"
#include <sys/mman.h>
#include <stdio.h>
#include <new>

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
// label-dtor recorder (the g_gkCallLabelDtor seam): the header hands (vtable slot 0 value, label, flags) here instead of calling it
static int dtorCalls = 0; static uint32_t dtorFn = 0; static uintptr_t dtorSelf = 0; static unsigned dtorFlags = 0;
static void recordDtor(uint32_t fn, uintptr_t self, unsigned flags) { ++dtorCalls; dtorFn = fn; dtorSelf = self; dtorFlags = flags; }

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

    // WI-87 (BG40): a create that fails AFTER the vtable check releases its label once through the label's OWN vtable slot 0 (flags = 1);
    // an unknown vtable is never called; a label that succeeds is never released.
    const uint32_t slot0Saved = at<uint32_t>(LabelVtable, 0); auto* const dtorSaved = g_gkCallLabelDtor;
    at<uint32_t>(LabelVtable, 0) = 0x7510B0u; g_gkCallLabelDtor = recordDtor;
    dtorCalls = 0; why = -1; initWritesId = false;                       // InitControl leaves the id sentinel: template not in the layout
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Template);
    KT_CHECK_EQ(dtorCalls, 1); KT_CHECK_EQ((unsigned long long)dtorFn, 0x7510B0ull); KT_CHECK_EQ(dtorFlags, 1u);
    KT_CHECK(dtorSelf >= reinterpret_cast<uintptr_t>(g_arena) && dtorSelf < reinterpret_cast<uintptr_t>(g_arena) + g_used);   // the allocated label
    KT_CHECK(at<uint32_t>(dtorSelf, 0) == LabelVtable);
    initWritesId = true;
    dtorCalls = 0; why = -1;                                             // panel count unreadable: CF_Count, still after InitControl
    KT_CHECK(createLabelFromTemplate(0x10, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Count); KT_CHECK_EQ(dtorCalls, 1);
    KT_CHECK_EQ(dtorFlags, 1u);
    dtorCalls = 0; why = -1;                                             // success: the label is kept
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) != 0); KT_CHECK_EQ(why, (int)CF_None); KT_CHECK_EQ(dtorCalls, 0);
    dtorCalls = 0; why = -1; ctorVtable = 0x00987800u; at<uint32_t>(0x00987800u, 0) = 0x7510B0u;   // readable foreign vtable with a live slot 0
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Vtable); KT_CHECK_EQ(dtorCalls, 0);   // never called
    at<uint32_t>(0x00987800u, 0) = 0; ctorVtable = LabelVtable;
    at<uint32_t>(LabelVtable, 0) = 0; initWritesId = false; dtorCalls = 0; why = -1;   // our vtable but slot 0 empty: no call
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Template); KT_CHECK_EQ(dtorCalls, 0);
    initWritesId = true;
    g_gkCallLabelDtor = nullptr; at<uint32_t>(LabelVtable, 0) = 0x7510B0u; why = -1; initWritesId = false;   // no seam: refuses, no crash
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Template);
    initWritesId = true; g_gkCallLabelDtor = recordDtor;

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
        // setBorderTint: writes the 3 floats at border+0x28; gated like getBorderTint
        const float hi[3] = { 0.8f, 0.8f, 0.69804f };
        KT_CHECK(setBorderTint(k, hi)); KT_CHECK(getBorderTint(k, rgb)); KT_CHECK(rgb[0] == 0.8f && rgb[1] == 0.8f && rgb[2] == 0.69804f);
        KT_CHECK(!setBorderTint(k, nullptr)); KT_CHECK(!setBorderTint(0, hi));
        at<uint32_t>(k, Label_Border) = 0; KT_CHECK(!setBorderTint(k, rgb)); at<uint32_t>(k, Label_Border) = 0x9875BC;
        KT_CHECK(at<float>(k, Label_Border + Border_Tint) == 0.8f);
        // setFillStyle: low 2 bits only, other bits kept; gated like the other border setters
        at<uint32_t>(k, Label_Border + Border_FillStyle) = 0xF0u | 1u;
        KT_CHECK(setFillStyle(k, 2)); KT_CHECK_EQ((unsigned long long)at<uint32_t>(k, Label_Border + Border_FillStyle), 0xF2ull);
        KT_CHECK(setFillStyle(k, 0)); KT_CHECK_EQ((unsigned long long)at<uint32_t>(k, Label_Border + Border_FillStyle), 0xF0ull);
        KT_CHECK(!setFillStyle(k, 4)); KT_CHECK(!setFillStyle(k, -1)); KT_CHECK(!setFillStyle(0, 2));
        at<uint32_t>(k, Label_Border) = 0; KT_CHECK(!setFillStyle(k, 2)); at<uint32_t>(k, Label_Border) = 0x9875BC;

        // setAlpha: writes the float at border+0x24 only through the border-vtable gate; the tint neighbour is untouched
        at<float>(k, Label_Border + Border_Alpha) = 0.125f;
        KT_CHECK(setAlpha(k, 0.5f)); KT_CHECK(at<float>(k, Label_Border + Border_Alpha) == 0.5f);
        KT_CHECK(getAlpha(k, &al) && al == 0.5f); KT_CHECK(at<float>(k, Label_Border + Border_Tint) == 0.8f);
        KT_CHECK(!setAlpha(0, 0.25f));
        at<uint32_t>(k, Label_Border) = 0; KT_CHECK(!setAlpha(k, 0.25f)); KT_CHECK(at<float>(k, Label_Border + Border_Alpha) == 0.5f);
        at<uint32_t>(k, Label_Border) = 0x9875BC;
        // setFill: image pointer -> border+Border_Fill (nullptr clears), optional tint -> border+Border_Tint; gated like the other border setters
        void* const img = reinterpret_cast<void*>(0x5000);
        const float t2[3] = { 0.1f, 0.2f, 0.3f };
        KT_CHECK(setFill(k, img)); KT_CHECK_EQ((unsigned long long)at<uint32_t>(k, Label_Border + Border_Fill), 0x5000ull);
        KT_CHECK(at<float>(k, Label_Border + Border_Tint) == 0.8f);                                   // no tint argument: tint untouched
        KT_CHECK(setFill(k, nullptr, t2)); KT_CHECK_EQ((unsigned long long)at<uint32_t>(k, Label_Border + Border_Fill), 0ull);
        KT_CHECK(at<float>(k, Label_Border + Border_Tint) == 0.1f && at<float>(k, Label_Border + Border_Tint + 4) == 0.2f && at<float>(k, Label_Border + Border_Tint + 8) == 0.3f);
        KT_CHECK(!setFill(0, img));
        at<uint32_t>(k, Label_Border) = 0; KT_CHECK(!setFill(k, img, hi)); at<uint32_t>(k, Label_Border) = 0x9875BC;
        KT_CHECK_EQ((unsigned long long)at<uint32_t>(k, Label_Border + Border_Fill), 0ull); KT_CHECK(at<float>(k, Label_Border + Border_Tint) == 0.1f);
        KT_CHECK(setBorderTint(k, hi));                                                               // restore the tint other blocks expect

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

        // rclickEdge (same cases; independent of the left button)
        static bool rdn = false;
        g_gk.gameForeground = [] { return fg; }; g_gk.lbuttonDown = [] { return dn; }; g_gk.rbuttonDown = [] { return rdn; };
        fg = true; dn = false; ClickEdge re;
        KT_CHECK(!rclickEdge(re));
        dn = true; KT_CHECK(!rclickEdge(re));                         // left button does not fire right edge
        rdn = true; KT_CHECK(rclickEdge(re));
        KT_CHECK(!rclickEdge(re)); KT_CHECK(!rclickEdge(re));         // held: fires once
        rdn = false; KT_CHECK(!rclickEdge(re));
        rdn = true; KT_CHECK(rclickEdge(re));                         // new press fires again
        fg = false; KT_CHECK(!rclickEdge(re)); KT_CHECK(!re.prev);    // lost focus counts as up
        fg = true; KT_CHECK(rclickEdge(re));                          // press that resumes in foreground re-fires
        g_gk.gameForeground = nullptr; KT_CHECK(!rclickEdge(re));
        g_gk.gameForeground = [] { return true; }; g_gk.rbuttonDown = nullptr;
        KT_CHECK(!rclickEdge(re));
        g_gk = saved;
    }

    // ---- screen size / cursorEngineF / WheelHook / bindByTag (additive block) ----
    {
        // screenValid bounds
        KT_CHECK(screenValid(320, 200)); KT_CHECK(screenValid(16384, 16384)); KT_CHECK(screenValid(3840, 1600));
        KT_CHECK(!screenValid(319, 200)); KT_CHECK(!screenValid(320, 199)); KT_CHECK(!screenValid(16385, 200)); KT_CHECK(!screenValid(320, 16385));
        KT_CHECK(!screenValid(0, 0)); KT_CHECK(!screenValid(-1, 600));
        // pickScreen: INI wins (either key), engine only when valid, else untouched
        int w = 111, h = 222;
        KT_CHECK_EQ(pickScreen(0, 0, 1920, 1080, &w, &h), (int)SS_Engine); KT_CHECK(w == 1920 && h == 1080);
        w = 111; h = 222; KT_CHECK_EQ(pickScreen(0, 0, 100, 100, &w, &h), (int)SS_None); KT_CHECK(w == 111 && h == 222);
        KT_CHECK_EQ(pickScreen(2560, 1440, 1920, 1080, &w, &h), (int)SS_Ini); KT_CHECK(w == 2560 && h == 1440);
        w = 111; h = 222; KT_CHECK_EQ(pickScreen(2560, 0, 1920, 1080, &w, &h), (int)SS_Ini); KT_CHECK(w == 2560 && h == 222);   // one key: other keeps current
        w = 111; h = 222; KT_CHECK_EQ(pickScreen(0, 900, 1920, 1080, &w, &h), (int)SS_Ini); KT_CHECK(w == 111 && h == 900);
        w = 111; h = 222; KT_CHECK_EQ(pickScreen(-5, -5, 1920, 1080, &w, &h), (int)SS_Engine); KT_CHECK(w == 1920 && h == 1080);   // negative = no override
        KT_CHECK_EQ((int)SS_None, 0); KT_CHECK_EQ((int)SS_Ini, 1); KT_CHECK_EQ((int)SS_Engine, 2);
        w = 111; h = 222; KT_CHECK_EQ(pickScreen(1, 0, 1920, 1080, &w, &h), (int)SS_Ini); KT_CHECK(w == 1 && h == 222);   // smallest positive override counts
        w = 111; h = 222; KT_CHECK_EQ(pickScreen(0, 1, 1920, 1080, &w, &h), (int)SS_Ini); KT_CHECK(w == 111 && h == 1);
        KT_CHECK_EQ(pickScreen(0, 0, 1920, 1080, nullptr, &h), (int)SS_None); KT_CHECK_EQ(pickScreen(0, 0, 1920, 1080, &w, nullptr), (int)SS_None);
        // syncScreen against fake engine globals
        Engine saved = g_gk;
        uintptr_t gw = arenaAlloc(16), gh = gw + 4;
        g_gk.screenWAddr = gw; g_gk.screenHAddr = gh; at<int>(gw, 0) = 1600; at<int>(gh, 0) = 1200;
        w = 1; h = 2;
        KT_CHECK(syncScreen(0, 0, &w, &h)); KT_CHECK(w == 1600 && h == 1200);
        at<int>(gw, 0) = 10; w = 1; h = 2; KT_CHECK(!syncScreen(0, 0, &w, &h)); KT_CHECK(w == 1 && h == 2);   // engine invalid: untouched
        at<int>(gw, 0) = 1600;
        KT_CHECK(syncScreen(2560, 1440, &w, &h)); KT_CHECK(w == 2560 && h == 1440);                            // INI beats a valid engine size
        KT_CHECK(!syncScreen(0, 0, nullptr, &h)); KT_CHECK(!syncScreen(0, 0, &w, nullptr));
        g_gk.screenWAddr = 0; g_gk.screenHAddr = 0; w = 1; h = 2; KT_CHECK(!syncScreen(0, 0, &w, &h)); KT_CHECK(w == 1 && h == 2);   // unknown addresses
        g_gk.screenWAddr = gw; g_gk.screenHAddr = gh;

        // clientToEngineF / cursorEngineF: no integer truncation
        float fx = 0, fy = 0;
        clientToEngineF(1, 1, 3, 3, 10.f, 10.f, &fx, &fy); KT_CHECK(fabsf(fx - 10.f / 3) < 1e-6f && fabsf(fy - 10.f / 3) < 1e-6f);
        static int cCx = 1, cCy = 1, cCw = 3, cCh = 3; static bool cOk = true;
        g_gk.cursorClient = [](int* x, int* y, int* ww, int* hh) { *x = cCx; *y = cCy; *ww = cCw; *hh = cCh; return cOk; };
        at<int>(gw, 0) = 1600; at<int>(gh, 0) = 1200;
        float ex = 0, ey = 0; int ix = 0, iy = 0;
        KT_CHECK(cursorEngineF(&ex, &ey)); KT_CHECK(fabsf(ex - 1600.f / 3) < 1e-3f && fabsf(ey - 400.f) < 1e-3f);
        KT_CHECK(cursorEngine(&ix, &iy)); KT_CHECK(ix == 533 && iy == 400); KT_CHECK(ex > (float)ix);   // int variant truncates, float keeps .33
        KT_CHECK(cursorEngineF(&ex, &ey, 3000, 900)); KT_CHECK(fabsf(ex - 1000.f) < 1e-3f && fabsf(ey - 300.f) < 1e-3f);   // INI override beats engine
        KT_CHECK(cursorEngineF(&ex, &ey, 3000, 0)); KT_CHECK(ex == 1.f && ey == 1.f);                                         // half override: client px
        KT_CHECK(cursorEngineF(&ex, &ey, 0, 900)); KT_CHECK(ex == 1.f && ey == 1.f);                                          // half override (H only): client px
        KT_CHECK(cursorEngineF(&ex, &ey, 1, 1)); KT_CHECK(ex == 1.f / 3 && ey == 1.f / 3);                                    // minimal override maps (1*1/3)
        cCx = 5; cCy = 7; cCw = 1; cCh = 1; KT_CHECK(cursorEngineF(&ex, &ey, 100, 200)); KT_CHECK(ex == 500.f && ey == 1400.f);   // 1x1 client is valid
        cCx = 1; cCy = 1; cCw = 3; cCh = 3;
        at<int>(gw, 0) = 0; KT_CHECK(cursorEngineF(&ex, &ey)); KT_CHECK(ex == 1.f && ey == 1.f);                              // invalid engine: client px
        // wheelIniScaleF: client 1000x500, engine 1920x1080, INI 2560x1440; provider engine px 960,540 = client 500,250
        {
            float wx = 0, wy = 0;
            wheelIniScaleF(960.f, 540.f, 1920, 1080, 2560.f, 1440.f, &wx, &wy);
            KT_CHECK(fabsf(wx - 1280.f) < 1e-3f && fabsf(wy - 720.f) < 1e-3f);   // client 500,250 * INI / client 1000x500
            wheelIniScaleF(960.f, 540.f, 1920, 1080, 1920.f, 1080.f, &wx, &wy);
            KT_CHECK(wx == 960.f && wy == 540.f);                                 // INI == engine: unchanged
            wheelIniScaleF(960.f, 540.f, 0, 0, 2560.f, 1440.f, &wx, &wy);
            KT_CHECK(wx == 960.f && wy == 540.f);                                 // engine size unknown: unchanged, no divide by 0
            wheelIniScaleF(960.f, 540.f, 1920, 1080, 2560.f, 1080.f, &wx, &wy);
            KT_CHECK(fabsf(wx - 1280.f) < 1e-3f && wy == 540.f);                 // half override (W only): y keeps engine scale
        }
        at<int>(gw, 0) = 1600;
        cCw = 0; KT_CHECK(!cursorEngineF(&ex, &ey)); cCw = 3; cCh = 0; KT_CHECK(!cursorEngineF(&ex, &ey)); cCh = 3;
        cOk = false; KT_CHECK(!cursorEngineF(&ex, &ey)); cOk = true;
        KT_CHECK(!cursorEngineF(nullptr, &ey)); KT_CHECK(!cursorEngineF(&ex, nullptr));
        g_gk.cursorClient = nullptr; KT_CHECK(!cursorEngineF(&ex, &ey));
        g_gk = saved;

        // wheel pure parts
        KT_CHECK_EQ(wheelDelta(static_cast<uintptr_t>(120u << 16)), 120); KT_CHECK_EQ(wheelDelta(static_cast<uintptr_t>(0xFF88u << 16)), -120);
        KT_CHECK_EQ(WheelAccumMax, 1 << 20);
        KT_CHECK_EQ(wheelDelta(static_cast<uintptr_t>(121u << 16)), 121); KT_CHECK_EQ(wheelDelta(static_cast<uintptr_t>(0xFF89u << 16)), -119);   // low bit of the delta
        KT_CHECK_EQ(wheelPtX(0x0001u), 1); KT_CHECK_EQ(wheelPtY(static_cast<uintptr_t>(1u << 16)), 1); KT_CHECK_EQ(wheelPtX(static_cast<uintptr_t>(0xFFFFu)), -1);
        KT_CHECK_EQ(wheelPtY(static_cast<uintptr_t>(0xFFFFu << 16)), -1);
        KT_CHECK_EQ(wheelDelta(0x0000FFFFu), 0);   // low word (modifier keys) ignored
        KT_CHECK_EQ(wheelPtX(static_cast<uintptr_t>((200u << 16) | 100u)), 100); KT_CHECK_EQ(wheelPtY(static_cast<uintptr_t>((200u << 16) | 100u)), 200);
        KT_CHECK_EQ(wheelPtX(static_cast<uintptr_t>(0x0000FFF6u)), -10); KT_CHECK_EQ(wheelPtY(static_cast<uintptr_t>(0xFFF60000u)), -10);   // multi-monitor negatives
        KT_CHECK_EQ(wheelAddClamped(0, 120), 120); KT_CHECK_EQ(wheelAddClamped(120, -240), -120);
        KT_CHECK_EQ(wheelAddClamped(WheelAccumMax - 10, 120), WheelAccumMax); KT_CHECK_EQ(wheelAddClamped(WheelAccumMax, 1), WheelAccumMax);
        KT_CHECK_EQ(wheelAddClamped(-WheelAccumMax + 10, -120), -WheelAccumMax); KT_CHECK_EQ(wheelAddClamped(WheelAccumMax, -1), WheelAccumMax - 1);
        KT_CHECK(rectContainsF(10.f, 20.f, 10, 20, 30, 40)); KT_CHECK(rectContainsF(39.9f, 59.9f, 10, 20, 30, 40));
        KT_CHECK(!rectContainsF(40.f, 30.f, 10, 20, 30, 40)); KT_CHECK(!rectContainsF(25.f, 60.f, 10, 20, 30, 40));   // right/bottom exclusive
        KT_CHECK(!rectContainsF(9.9f, 30.f, 10, 20, 30, 40)); KT_CHECK(!rectContainsF(25.f, 19.9f, 10, 20, 30, 40));
        KT_CHECK(!rectContainsF(10.f, 20.f, 10, 20, 0, 40)); KT_CHECK(!rectContainsF(10.f, 20.f, 10, 20, 30, 0));      // empty rect

        // WheelHook core: gate (enabled / fresh / any rect / drag guard), inside, accumulate, take, chain
        {   // constructor must clear every rect even when the storage is garbage
            alignas(WheelHook) static unsigned char raw[sizeof(WheelHook)]; memset(raw, 0xAA, sizeof raw);
            WheelHook* g = new (raw) WheelHook;
            KT_CHECK(!g->anyRect()); KT_CHECK(!g->inside(0.f, 0.f)); KT_CHECK_EQ(g->take(), 0);
            KT_CHECK(g->fresh(0) && g->fresh(299) && !g->fresh(300));   // lastTickMs starts at 0
            KT_CHECK(g->iniW == 0 && g->iniH == 0 && g->screenW == 3840 && g->screenH == 1600 && g->eventLogs == 0);
        }
        WheelHook wh; wh.screenW = 1000; wh.screenH = 500;   // client 500x250 -> scale 2
        wh.iniW = 5;                                         // sentinel: an out-of-range rect index must not touch the fields after rects[]
        KT_CHECK(!wh.anyRect()); KT_CHECK(!wh.onWheel(100, 120, true, 50, 50, 500, 250));            // no rect: not consumed
        wh.setRect(0, 100, 100, 200, 100);                                                           // engine px; client (50..150, 50..100)
        KT_CHECK(wh.anyRect()); wh.touch(1000);
        KT_CHECK(wh.fresh(1000)); KT_CHECK(wh.fresh(1299)); KT_CHECK(!wh.fresh(1300)); KT_CHECK(!wh.fresh(999 + 0x80000000u));
        KT_CHECK(!wh.onWheel(1300, 120, true, 100, 75, 500, 250));                                   // stale tick: passes through
        KT_CHECK_EQ(wh.take(), 0);
        KT_CHECK(wh.onWheel(1100, 120, true, 100, 75, 500, 250)); KT_CHECK(wh.onWheel(1100, 120, true, 100, 75, 500, 250));
        KT_CHECK(wh.onWheel(1100, -60, true, 50, 50, 500, 250));                                     // (100,100) engine = left/top edge inclusive
        KT_CHECK_EQ(wh.take(), 180); KT_CHECK_EQ(wh.take(), 0);                                      // take drains
        KT_CHECK(!wh.onWheel(1100, 120, true, 150, 75, 500, 250));                                   // x=300 = right edge exclusive
        KT_CHECK(!wh.onWheel(1100, 120, true, 49, 75, 500, 250)); KT_CHECK(!wh.onWheel(1100, 120, true, 100, 100, 500, 250));
        KT_CHECK(!wh.onWheel(1100, 120, false, 100, 75, 500, 250));                                  // no client mapping: pass through
        KT_CHECK(!wh.onWheel(1100, 120, true, 100, 75, 0, 250)); KT_CHECK(!wh.onWheel(1100, 120, true, 100, 75, 500, 0));
        KT_CHECK_EQ(wh.take(), 0);
        wh.enabled = false; KT_CHECK(!wh.onWheel(1100, 120, true, 100, 75, 500, 250)); wh.enabled = true;
        // second rect, clearRect, out-of-range index
        wh.setRect(1, 600, 300, 100, 100); wh.setRect(WheelMaxRects, 0, 0, 5000, 5000); wh.setRect(-1, 0, 0, 5000, 5000); wh.clearRect(WheelMaxRects); wh.clearRect(-1);
        KT_CHECK(wh.onWheel(1100, 120, true, 325, 160, 500, 250)); KT_CHECK_EQ(wh.take(), 120);     // (650,320) in rect 1
        KT_CHECK(wh.onWheel(1100, 120, true, 100, 75, 500, 250)); KT_CHECK_EQ(wh.take(), 120);      // rect 0 still active
        wh.onWheel(1100, 120, true, 100, 75, 500, 250); wh.clearRect(-1); wh.clearRect(WheelMaxRects); KT_CHECK_EQ(wh.iniW, 5);   // OOB clear is a no-op
        KT_CHECK_EQ(wh.take(), 120);
        wh.clearRect(0); KT_CHECK(!wh.onWheel(1100, 120, true, 100, 75, 500, 250)); KT_CHECK(wh.anyRect());
        wh.clearRect(1); KT_CHECK(!wh.anyRect()); KT_CHECK(!wh.onWheel(1100, 120, true, 325, 160, 500, 250));
        // 1-px rect and 1x1 client are valid; w == 1 counts as active
        wh.setRect(0, 10, 10, 1, 1); wh.setRect(1, 0, 0, 0, 0);
        KT_CHECK(wh.anyRect()); KT_CHECK(wh.inside(10.f, 10.f)); KT_CHECK(!wh.inside(11.f, 10.f)); KT_CHECK(!wh.inside(10.f, 11.f));
        wh.screenW = 1; wh.screenH = 1; KT_CHECK(wh.onWheel(1100, 120, true, 10, 10, 1, 1)); KT_CHECK_EQ(wh.take(), 120); wh.screenW = 1000; wh.screenH = 500;
        wh.setRect(0, 0, 0, 0, 0); KT_CHECK(!wh.anyRect());
        wh.setRect(1, 100, 100, 200, 100);   // keep the rest of the tests' rect geometry (the rect lives at index 1 now)
        wh.setRect(0, 100, 100, 200, 100); wh.clearRect(1);
        wh.clearRect(0); wh.setRect(WheelMaxRects - 1, 400, 200, 50, 50);   // the last slot is usable
        KT_CHECK_EQ(WheelMaxRects, 4); KT_CHECK(wh.anyRect()); KT_CHECK(wh.inside(420.f, 220.f)); KT_CHECK(!wh.inside(450.f, 220.f));
        wh.clearRect(WheelMaxRects - 1); KT_CHECK(!wh.anyRect()); wh.setRect(0, 100, 100, 200, 100);
        // drag guard: guard true -> not consumed, nothing accumulated; guard false -> normal
        static bool dragging = false; wh.dragGuard = [] { return dragging; };
        wh.setRect(0, 100, 100, 200, 100);
        dragging = true; KT_CHECK(!wh.gate(1100)); KT_CHECK(!wh.onWheel(1100, 120, true, 100, 75, 500, 250)); KT_CHECK_EQ(wh.take(), 0);
        dragging = false; KT_CHECK(wh.gate(1100)); KT_CHECK(wh.onWheel(1100, 120, true, 100, 75, 500, 250)); KT_CHECK_EQ(wh.take(), 120);
        // saturation through the core
        for (int i = 0; i < 12000; ++i) wh.onWheel(1100, 120, true, 100, 75, 500, 250);
        KT_CHECK_EQ(wh.take(), WheelAccumMax);
        // diagnostic callback: first eventLogMax gated events only, reports inside flag
        static int logN = 0, logIn = 0, logDelta = 0; wh.onEvent = [](int d, int, int, bool in) { ++logN; logIn += in; logDelta = d; };
        wh.eventLogMax = 3; wh.eventLogs = 0;
        wh.onWheel(1100, 7, true, 100, 75, 500, 250); wh.onWheel(1100, 8, true, 0, 0, 500, 250); wh.onWheel(1100, 9, true, 100, 75, 500, 250);
        wh.onWheel(1100, 10, true, 100, 75, 500, 250);
        KT_CHECK_EQ(logN, 3); KT_CHECK_EQ(logIn, 2); KT_CHECK_EQ(logDelta, 9);
        wh.onWheel(1300 + 5000, 11, true, 100, 75, 500, 250); KT_CHECK_EQ(logN, 3);                 // gated-out events are not logged
        // default g_wheel exists with the copies' defaults
        KT_CHECK(g_wheel.enabled && g_wheel.freshMs == 300 && g_wheel.eventLogMax == 20 && !g_wheel.scoped && !g_wheel.dragGuard);

        // bindFlags / bindMatches (id-mismatch decision)
        KT_CHECK(bindMatches(5, 5)); KT_CHECK(!bindMatches(4, 5));
        KT_CHECK_EQ(bindFlags(5, 5, 0x2), 0x22);                // match: visibility kept, click-through set
        KT_CHECK_EQ(bindFlags(5, 5, 0x102), 0x122);             // other bits kept
        KT_CHECK_EQ(bindFlags(4, 5, 0x2), 0x20);                // mismatch: hidden + click-through
        KT_CHECK_EQ(bindFlags(4, 5, 0x102), 0x120);
        KT_CHECK_EQ(bindFlags(5, 5, 0x20), 0x20); KT_CHECK_EQ(bindFlags(-999, 5, 0x0), 0x20);

        // bindByTag with fakes
        static int fakeId = 7, initCalls = 0, lastAddToList = -1, lastScale = -1; static void* lastPanel = nullptr; static char lastTag[16];
        g_gk.initControl = [](void* pnl, void* ctl, void* exo, int add, int scale) {
            uintptr_t k = reinterpret_cast<uintptr_t>(ctl); ++initCalls; lastAddToList = add; lastScale = scale; lastPanel = pnl; strncpy(lastTag, static_cast<char*>(exo), 15);
            at<int>(k, Ctl_Id) = fakeId; at<int>(k, Ctl_X) = 11; at<int>(k, Ctl_Y) = 22; at<int>(k, Ctl_W) = 33; at<int>(k, Ctl_H) = 44; at<int>(k, Ctl_Flags) = 0x102;
        };
        uintptr_t bp = arenaAlloc(0x100);
        BindInfo bi; int a0 = nAlloc;
        uintptr_t bc = bindByTag(bp, "LBL_FOO", 7, 0, &bi);
        KT_CHECK(bc != 0); KT_CHECK_EQ(nAlloc, a0 + 1); KT_CHECK(bi.ctl == bc && bi.why == CF_None && bi.id == 7);
        KT_CHECK(bi.flagsBefore == 0x102 && bi.flagsAfter == 0x122 && at<int>(bc, Ctl_Flags) == 0x122);
        KT_CHECK(bi.ext[0] == 11 && bi.ext[1] == 22 && bi.ext[2] == 33 && bi.ext[3] == 44);
        KT_CHECK(initCalls == 1 && lastAddToList == 1 && lastScale == 1 && lastPanel == reinterpret_cast<void*>(bp) && !strcmp(lastTag, "LBL_FOO"));   // addToList = 1
        KT_CHECK(at<uint32_t>(bc, 0) == LabelVtable);
        KT_CHECK(bindByTag(bp, "LBL_FOO", 7) != 0);                       // info optional
        // explicit label size is what is allocated; tag mismatch -> hidden, returns 0, control still reported
        static unsigned lastSize = 0; g_gk.alloc = [](unsigned n) -> void* { lastSize = n; return reinterpret_cast<void*>(arenaAlloc(n)); };
        KT_CHECK(bindByTag(bp, "LBL_FOO", 7, 0x200) != 0); KT_CHECK_EQ(lastSize, 0x200u);
        KT_CHECK(bindByTag(bp, "LBL_FOO", 7) != 0); KT_CHECK_EQ(lastSize, (unsigned)LabelSize);          // 0 = table size
        g_gk.alloc = fAlloc;
        dtorCalls = 0;
        fakeId = 9; bc = bindByTag(bp, "LBL_BAR", 7, 0, &bi);
        KT_CHECK(bc == 0); KT_CHECK(bi.ctl != 0 && bi.id == 9 && bi.why == CF_Template);
        KT_CHECK_EQ(dtorCalls, 0);                                        // addToList = 1: the control is in the panel list, never released
        KT_CHECK(bi.flagsAfter == 0x120 && at<int>(bi.ctl, Ctl_Flags) == 0x120);                          // hidden (bit1 clear) + click-through
        fakeId = 7;
        // failure paths
        allocFails = true; KT_CHECK(bindByTag(bp, "LBL_FOO", 7, 0, &bi) == 0 && bi.why == CF_Alloc && bi.ctl == 0); allocFails = false;
        KT_CHECK(bi.id == -999 && bi.flagsBefore == 0 && bi.flagsAfter == 0 && bi.badVtable == 0 && !bi.ext[0] && !bi.ext[1] && !bi.ext[2] && !bi.ext[3]);   // defaults on failure
        bi.id = 1; bi.flagsAfter = 9; bi.ext[2] = 9; bi.badVtable = 9; KT_CHECK(bindByTag(0, "LBL_FOO", 7, 0, &bi) == 0);
        KT_CHECK(bi.id == -999 && bi.flagsAfter == 0 && bi.ext[2] == 0 && bi.badVtable == 0);                                                            // info reset per call
        g_gk.alloc = [](unsigned n) -> void* { void* m = reinterpret_cast<void*>(arenaAlloc(n)); memset(m, 0xCC, n); return m; };
        bc = bindByTag(bp, "LBL_FOO", 7); KT_CHECK(bc != 0); KT_CHECK_EQ(at<int>(bc, 0x100), 0); KT_CHECK_EQ(at<int>(bc, LabelSize - 4), 0);        // zero-filled before the ctor
        g_gk.alloc = fAlloc;
        int ic = initCalls;
        ctorVtable = 0xABCD0000u; KT_CHECK(bindByTag(bp, "LBL_FOO", 7, 0, &bi) == 0 && bi.why == CF_Vtable && bi.badVtable == 0xABCD0000u); ctorVtable = LabelVtable;
        KT_CHECK_EQ(initCalls, ic);                                       // never reached InitControl on a bad vtable
        ctorVtable = 0x00987800u; at<uint32_t>(0x00987800u, 0) = 0x7510B0u;   // readable foreign vtable with a live slot 0
        KT_CHECK(bindByTag(bp, "LBL_FOO", 7, 0, &bi) == 0 && bi.why == CF_Vtable); KT_CHECK_EQ(dtorCalls, 0);   // the CF_Vtable leak stays: never called
        at<uint32_t>(0x00987800u, 0) = 0; ctorVtable = LabelVtable;
        KT_CHECK(bindByTag(0, "LBL_FOO", 7, 0, &bi) == 0 && bi.why == CF_Alloc);
        KT_CHECK(bindByTag(bp, nullptr, 7, 0, &bi) == 0 && bi.why == CF_Alloc);
        g_gk.initControl = nullptr; KT_CHECK(bindByTag(bp, "LBL_FOO", 7, 0, &bi) == 0 && bi.why == CF_Alloc);
        g_gk = saved;
    }
    g_gkCallLabelDtor = dtorSaved; at<uint32_t>(LabelVtable, 0) = slot0Saved;
    return kt::Report("guikit_test");
}
