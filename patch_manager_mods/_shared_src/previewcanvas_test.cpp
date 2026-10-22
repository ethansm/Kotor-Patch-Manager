// Host test of PreviewCanvas.h (MCF10): the fit / map / clip maths with independent literals, and the label pool (all-or-nothing build, change-only
// setters, hide-unused, visibility resync, destroy) against a fake engine (pvc::Ops) and a fake k2.menuui table.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "PreviewCanvas.h"
#include "check.h"
#include <string.h>

// ---- fake engine ----
struct FakeCtl { bool vis = false; pvc::Rect r = {0, 0, 0, 0}; void* fill = nullptr; float alpha = -1.f; float tint[3] = {-1, -1, -1}; char text[32] = "OPTIONS"; bool released = false; void* fillAtRelease = nullptr; };
static int g_orig[pvc::kMaxLabels + 4];   // each fake label's OWN fill image (its address is the pointer)
static FakeCtl g_ctl[pvc::kMaxLabels + 4];
static int g_nCtl = 0, g_failCreateAt = -1;
static int g_setRect = 0, g_setFill = 0, g_setAlpha = 0, g_setTint = 0, g_setVis = 0, g_releases = 0;
static FakeCtl* C(uintptr_t c) { return &g_ctl[c - 1]; }
static uintptr_t fCreate(uintptr_t, const char* tmpl) { if (strcmp(tmpl, "LBL_TITLE")) return 0; if (g_nCtl == g_failCreateAt) return 0; g_ctl[g_nCtl] = FakeCtl(); g_ctl[g_nCtl].fill = &g_orig[g_nCtl]; return static_cast<uintptr_t>(++g_nCtl); }
static void fRelease(uintptr_t c) { C(c)->released = true; C(c)->fillAtRelease = C(c)->fill; ++g_releases; }
static bool fGetFill(uintptr_t c, void** i) { *i = C(c)->fill; return true; }
static void fVis(uintptr_t c, bool v) { C(c)->vis = v; ++g_setVis; }
static void fClick(uintptr_t, bool) {}
static void fRect(uintptr_t c, int x, int y, int w, int h) { C(c)->r = pvc::Rect{x, y, w, h}; ++g_setRect; }
static void fFill(uintptr_t c, void* i) { C(c)->fill = i; ++g_setFill; }
static void fAlpha(uintptr_t c, float a) { C(c)->alpha = a; ++g_setAlpha; }
static void fTint(uintptr_t c, const float* t) { memcpy(C(c)->tint, t, sizeof C(c)->tint); ++g_setTint; }
static void fText(uintptr_t c, const char* t) { strncpy(C(c)->text, t, 31); }
static bool fIsVis(uintptr_t c) { return C(c)->vis; }

// ---- fake k2.menuui ----
static int g_addFailAt = -1, g_nAdded = 0; static bool g_unlinked = false; static K2MuiRect g_pane = {100, 50, 600, 300}; static bool g_paneOk = true;
static int32_t KPATCH_CALL mAddCustom(uint32_t, void*, int32_t flags) { if (!(flags & MUI_NOFREE)) return -1; if (g_nAdded == g_addFailAt) return -1; return g_nAdded++; }
static int32_t KPATCH_CALL mPane(void*, int32_t which, K2MuiRect* out) { if (which != MUI_PANE_PREVIEW || !g_paneOk) return 0; *out = g_pane; return 1; }
static void* KPATCH_CALL mControl(uint32_t, int32_t item) { return g_unlinked ? nullptr : reinterpret_cast<void*>(static_cast<uintptr_t>(item + 1)); }

static void reset() {
    g_nCtl = 0; g_failCreateAt = -1; g_setRect = g_setFill = g_setAlpha = g_setTint = g_setVis = g_releases = 0;
    g_addFailAt = -1; g_nAdded = 0; g_unlinked = false; g_paneOk = true; g_pane = K2MuiRect{100, 50, 600, 300};
}

int main() {
    using namespace pvc;
    // ---- fit: the scale is the tighter axis, centred ----
    { Fit f = fitScreen(1920, 1080, Rect{100, 50, 600, 300});          // 600/1920 = 0.3125, 300/1080 = 0.2778: height limits
      KT_CHECK(f.ok); KT_CHECK(f.scale > 0.2777f && f.scale < 0.2778f);
      KT_CHECK_EQ(f.w, 533); KT_CHECK_EQ(f.h, 300); KT_CHECK_EQ(f.ox, 100 + (600 - 533) / 2); KT_CHECK_EQ(f.oy, 50);
      f = fitScreen(3440, 1440, Rect{0, 0, 600, 500});                  // width limits: 600/3440
      KT_CHECK_EQ(f.w, 600); KT_CHECK_EQ(f.h, 251); KT_CHECK_EQ(f.ox, 0); KT_CHECK_EQ(f.oy, (500 - 251) / 2);
      KT_CHECK(!fitScreen(0, 1080, Rect{0, 0, 10, 10}).ok); KT_CHECK(!fitScreen(10, 10, Rect{0, 0, 0, 10}).ok); KT_CHECK(!fitScreen(10, -1, Rect{0, 0, 10, 10}).ok); }
    // ---- map: edges are rounded, never gaps or overlaps between neighbours ----
    { Fit f = fitScreen(1000, 1000, Rect{10, 20, 333, 333});            // scale 0.333
      const Rect a = mapRect(f, Rect{0, 0, 100, 100}), b = mapRect(f, Rect{100, 0, 100, 100}), c = mapRect(f, Rect{200, 0, 100, 100});
      KT_CHECK_EQ(a.x, 10); KT_CHECK_EQ(a.y, 20); KT_CHECK_EQ(a.x + a.w, b.x); KT_CHECK_EQ(b.x + b.w, c.x);
      KT_CHECK_EQ(c.x + c.w, 10 + 100);                                 // 300 * 0.333 = 99.9 -> 100
      const Rect whole = mapRect(f, Rect{0, 0, 1000, 1000}); KT_CHECK_EQ(whole.w, 333); KT_CHECK_EQ(whole.h, 333); }
    // ---- clip ----
    { Rect o; const Rect clip = {10, 10, 100, 100};
      KT_CHECK(clipRect(Rect{0, 0, 50, 50}, clip, &o)); KT_CHECK_EQ(o.x, 10); KT_CHECK_EQ(o.y, 10); KT_CHECK_EQ(o.w, 40); KT_CHECK_EQ(o.h, 40);
      KT_CHECK(clipRect(Rect{20, 20, 10, 10}, clip, &o)); KT_CHECK_EQ(o.w, 10);
      KT_CHECK(!clipRect(Rect{0, 0, 10, 10}, clip, &o)); KT_CHECK_EQ(o.w, 0);       // touching edge only: no area
      KT_CHECK(!clipRect(Rect{200, 200, 5, 5}, clip, &o));
      KT_CHECK(!clipRect(Rect{20, 20, 0, 5}, clip, &o)); KT_CHECK(!clipRect(Rect{20, 20, 5, -3}, clip, nullptr)); }

    // ---- canvas ----
    Ops& o = ops();
    o.create = fCreate; o.release = fRelease; o.setVisible = fVis; o.setClickable = fClick; o.setRect = fRect; o.setFill = fFill; o.setAlpha = fAlpha;
    o.setTint = fTint; o.setText = fText; o.isVisible = fIsVis; o.getFill = fGetFill;
    K2MenuUiApi ui = {};
    ui.struct_size = sizeof ui; ui.AddCustom = mAddCustom; ui.PaneRect = mPane; ui.Control = mControl;
    static Canvas cv;
    int img1, img2;
    float tintA[3] = {0.1f, 0.2f, 0.3f};

    // build all listed
    reset();
    KT_CHECK(cv.build(&ui, reinterpret_cast<void*>(1), 7, 5));
    KT_CHECK_EQ(cv.n, 5); KT_CHECK_EQ(g_nCtl, 5); KT_CHECK_EQ(g_nAdded, 5);
    KT_CHECK(!strcmp(C(1)->text, "")); KT_CHECK(!C(1)->vis);              // the inherited title text is blanked, labels start hidden
    // bad args
    reset(); KT_CHECK(!cv.build(&ui, reinterpret_cast<void*>(1), 7, 0)); KT_CHECK(!cv.build(&ui, reinterpret_cast<void*>(1), 7, kMaxLabels + 1)); KT_CHECK(!cv.build(&ui, nullptr, 7, 3)); KT_CHECK_EQ(g_nCtl, 0);
    // a clone fails: every created label is released, nothing listed, not ok
    reset(); g_failCreateAt = 3;
    KT_CHECK(!cv.build(&ui, reinterpret_cast<void*>(1), 7, 5)); KT_CHECK_EQ(g_nCtl, 3); KT_CHECK_EQ(g_releases, 3); KT_CHECK_EQ(g_nAdded, 0); KT_CHECK(!cv.ok());
    KT_CHECK(!cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080));
    // AddCustom fails after 2: the 2 listed stay (hidden), the other 3 are released, not ok
    reset(); g_addFailAt = 2;
    KT_CHECK(!cv.build(&ui, reinterpret_cast<void*>(1), 7, 5)); KT_CHECK_EQ(g_nAdded, 2); KT_CHECK_EQ(g_releases, 3); KT_CHECK_EQ(cv.n, 2); KT_CHECK(!cv.ok());
    KT_CHECK(!C(1)->released && !C(2)->released && C(3)->released && C(5)->released);

    // frame: put / end
    reset();
    KT_CHECK(cv.build(&ui, reinterpret_cast<void*>(1), 7, 4));
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080));      // pane {100,50,600,300}: fit w 533 h 300 ox 133 oy 50
    cv.put(0, Rect{0, 0, 1920, 1080}, &img1, tintA, 0.5f);
    KT_CHECK(C(1)->vis); KT_CHECK_EQ(C(1)->r.x, 133); KT_CHECK_EQ(C(1)->r.y, 50); KT_CHECK_EQ(C(1)->r.w, 533); KT_CHECK_EQ(C(1)->r.h, 300);
    KT_CHECK(C(1)->fill == &img1); KT_CHECK(C(1)->alpha == 0.5f); KT_CHECK(C(1)->tint[2] == 0.3f);
    cv.put(2, Rect{960, 540, 960, 540}, &img2, nullptr, 1.f);             // lower-right quarter
    KT_CHECK(C(3)->vis); KT_CHECK_EQ(C(3)->r.x, 133 + 267); KT_CHECK_EQ(C(3)->r.w, 266); KT_CHECK_EQ(C(3)->r.y, 200); KT_CHECK_EQ(C(3)->r.h, 150);
    KT_CHECK(C(3)->tint[0] == -1.f);                                       // null tint leaves the engine value alone
    cv.put(3, Rect{5000, 5000, 10, 10}, &img1, nullptr, 1.f);              // entirely outside the virtual screen -> clipped away
    cv.end();
    KT_CHECK(C(1)->vis && !C(2)->vis && C(3)->vis && !C(4)->vis);          // 2 and 4 were never put: hidden
    // second identical frame: change-only setters write nothing
    const int r0 = g_setRect, f0 = g_setFill, a0 = g_setAlpha, t0 = g_setTint, v0 = g_setVis;
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080));
    cv.put(0, Rect{0, 0, 1920, 1080}, &img1, tintA, 0.5f); cv.put(2, Rect{960, 540, 960, 540}, &img2, nullptr, 1.f);
    cv.end();
    KT_CHECK_EQ(g_setRect, r0); KT_CHECK_EQ(g_setFill, f0); KT_CHECK_EQ(g_setAlpha, a0); KT_CHECK_EQ(g_setTint, t0); KT_CHECK_EQ(g_setVis, v0);
    // a changed value is written once
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080));
    cv.put(0, Rect{0, 0, 1920, 1080}, &img1, tintA, 0.25f); cv.put(2, Rect{960, 540, 960, 540}, &img2, nullptr, 1.f); cv.end();
    KT_CHECK_EQ(g_setAlpha, a0 + 1); KT_CHECK_EQ(g_setRect, r0);
    // a label put last frame but not this one hides; put outside a frame does nothing
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080)); cv.put(0, Rect{0, 0, 10, 10}, &img1, tintA, 0.25f); cv.end();
    KT_CHECK(C(1)->vis && !C(3)->vis);
    { const int s = g_setRect; cv.put(1, Rect{0, 0, 10, 10}, &img1, nullptr, 1.f); KT_CHECK_EQ(g_setRect, s); KT_CHECK(!C(2)->vis); }
    // text only inside a frame
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080)); cv.text(1, "Hello"); KT_CHECK(!strcmp(C(2)->text, "Hello")); cv.end();
    // resync: mod-config showed every page item on entry; the cache is dropped and the next frame hides the unused ones again
    for (int i = 1; i <= 4; ++i) C(i)->vis = true;
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080)); cv.put(0, Rect{0, 0, 10, 10}, &img1, tintA, 0.25f); cv.end();
    KT_CHECK(C(1)->vis && !C(2)->vis && !C(3)->vis && !C(4)->vis);
    // zoomed view: only a 960x540 part of the virtual screen, starting at (900, 400), fills the pane; what lies outside is clipped away
    KT_CHECK(cv.beginView(&ui, reinterpret_cast<void*>(1), 900, 400, 960, 540));
    cv.put(0, Rect{900, 400, 960, 540}, &img1, tintA, 1.f);
    KT_CHECK_EQ(C(1)->r.x, 133); KT_CHECK_EQ(C(1)->r.y, 50); KT_CHECK_EQ(C(1)->r.w, 533); KT_CHECK_EQ(C(1)->r.h, 300);
    cv.put(2, Rect{1380, 670, 480, 270}, &img2, nullptr, 1.f);             // lower-right quarter of the view
    KT_CHECK_EQ(C(3)->r.x, 400); KT_CHECK_EQ(C(3)->r.y, 200); KT_CHECK_EQ(C(3)->r.w, 266); KT_CHECK_EQ(C(3)->r.h, 150);
    cv.put(1, Rect{0, 0, 100, 100}, &img1, nullptr, 1.f);                  // left of / above the view: clipped away, stays hidden
    cv.end();
    KT_CHECK(!C(2)->vis);
    // putPx: engine px, clipped to the pane, shown like put()
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080));
    cv.putPx(1, Rect{90, 330, 200, 40}, &img1, tintA, 1.f);                // sticks out left of the pane {100,50,600,300}: x clipped to 100, y to 350
    KT_CHECK(C(2)->vis); KT_CHECK_EQ(C(2)->r.x, 100); KT_CHECK_EQ(C(2)->r.y, 330); KT_CHECK_EQ(C(2)->r.w, 190); KT_CHECK_EQ(C(2)->r.h, 20);
    cv.putPx(2, Rect{0, 0, 50, 50}, &img1, tintA, 1.f);                    // entirely outside the pane
    cv.end();
    KT_CHECK(C(2)->vis && !C(3)->vis);
    // style: writes rect / fill / tint / alpha change-only and never shows or hides, outside a frame and without a pane
    { const bool v3 = C(4)->vis; const int r1 = g_setRect, v1 = g_setVis;
      cv.style(3, Rect{7, 8, 90, 40}, &img1, tintA, 0.5f);
      KT_CHECK_EQ(C(4)->r.x, 7); KT_CHECK_EQ(C(4)->r.w, 90); KT_CHECK(C(4)->fill == &img1); KT_CHECK(C(4)->alpha == 0.5f);
      KT_CHECK_EQ(g_setRect, r1 + 1); KT_CHECK_EQ(g_setVis, v1); KT_CHECK(C(4)->vis == v3);
      cv.style(3, Rect{7, 8, 90, 40}, &img1, tintA, 0.5f); KT_CHECK_EQ(g_setRect, r1 + 1);   // unchanged: nothing written
      cv.style(9, Rect{0, 0, 1, 1}, &img1, tintA, 1.f); KT_CHECK_EQ(g_setRect, r1 + 1); }    // out of range: ignored
    // begin failures: no pane, empty pane, bad virtual size; hideAll hides everything
    g_paneOk = false; KT_CHECK(!cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080));
    g_paneOk = true; g_pane = K2MuiRect{0, 0, 0, 10}; KT_CHECK(!cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080));
    g_pane = K2MuiRect{100, 50, 600, 300}; KT_CHECK(!cv.begin(&ui, reinterpret_cast<void*>(1), 0, 1080));
    KT_CHECK(cv.begin(&ui, reinterpret_cast<void*>(1), 1920, 1080)); cv.put(0, Rect{0, 0, 10, 10}, &img1, tintA, 1.f); cv.end();
    cv.hideAll(); KT_CHECK(!C(1)->vis);
    // destroy: frees only labels whose Control() answers null (unlinked); a still-listed label is leaked
    g_releases = 0; g_unlinked = false;
    { Canvas& c2 = cv; KT_CHECK_EQ(c2.destroy(&ui, 7), 0); KT_CHECK_EQ(g_releases, 0); }
    reset(); KT_CHECK(cv.build(&ui, reinterpret_cast<void*>(1), 7, 3)); g_unlinked = true;
    KT_CHECK_EQ(cv.destroy(&ui, 7), 3); KT_CHECK_EQ(g_releases, 3); KT_CHECK_EQ(cv.n, 0);
    KT_CHECK_EQ(cv.destroy(&ui, 7), 0);                                    // already reset
    // a pooled image shared by several labels must never reach a label dtor: the label's own fill goes back before release (crash 10-21 23:26)
    reset(); g_unlinked = false; KT_CHECK(cv.build(&ui, reinterpret_cast<void*>(1), 7, 4)); g_unlinked = true;
    cv.style(0, Rect{0, 0, 10, 10}, &img1, tintA, 1.f); cv.style(1, Rect{0, 0, 10, 10}, &img1, tintA, 1.f); cv.style(2, Rect{0, 0, 10, 10}, &img2, tintA, 1.f);   // label 3 never styled
    KT_CHECK(C(1)->fill == &img1 && C(2)->fill == &img1);
    const int fillsBefore = g_setFill;
    KT_CHECK_EQ(cv.destroy(&ui, 7), 4); KT_CHECK_EQ(g_releases, 4);
    for (int i = 1; i <= 4; ++i) KT_CHECK(C(i)->released && C(i)->fillAtRelease == &g_orig[i - 1]);   // every label died holding its own image
    KT_CHECK_EQ(g_setFill - fillsBefore, 3);                               // restored only where style() had replaced it
    return kt::Report("previewcanvas_test");
}
