// PreviewCanvas.h -- a pool of engine labels drawn as rectangles inside an options page's PREVIEW pane (MCF10, doc 46).
// Generic version of the Enemy Effects Strip preview (EnemyStrip.cpp buildPreview / refreshPreview): N labels cloned from LBL_TITLE are listed on the
// page with AddCustom(MUI_NOFREE) (this canvas OWNS them and frees them in destroy() once mod-config reports them unlinked), then every tick the
// page puts rectangles on them with change-only setters. A virtual screen (vw x vh) is scaled uniformly into the pane and centred, so a page can
// draw a whole HUD mockup in its own layout units.
//
//   pvc::Canvas cv;                                   // keep it static (about 8 KB)
//   build(panel, page):   cv.build(ui, panel, page, 60)           // pool of 60; then ui->SetDescSplit(page, 700) is the page's call
//   every tick:           if (cv.begin(ui, panel, 1920, 1080)) { cv.put(0, {x,y,w,h}, fillImg, tint, alpha); ... cv.end(); } else cv.hideAll();
//   destroy(panel):       cv.destroy(ui, page)
// Creation order = draw order: label i is drawn above label i-1. A label not put in a frame is hidden by end().
// Pure math (fit / map / clip) is host-tested in previewcanvas_test.cpp; engine access goes through pvc::Ops so tests inject fakes.
#pragma once
#include "GuiKit.h"
#include "ModConfigApi.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

namespace pvc {

struct Rect { int x, y, w, h; };

constexpr int kMaxLabels = 128;

// ---- pure math ---------------------------------------------------------------------------------------------------------------------------
// Uniform scale-to-fit of a vw x vh virtual screen into `pane`: scale = min(pane.w / vw, pane.h / vh), centred. ok = false for an empty screen or pane.
struct Fit {
    float scale = 0.f;
    int ox = 0, oy = 0;      // engine px of the virtual origin
    int w = 0, h = 0;        // engine px size of the scaled virtual screen
    bool ok = false;
};

inline Fit fitScreen(int vw, int vh, const Rect& pane) {
    Fit f;
    if (vw <= 0 || vh <= 0 || pane.w <= 0 || pane.h <= 0) return f;
    const float sx = static_cast<float>(pane.w) / static_cast<float>(vw), sy = static_cast<float>(pane.h) / static_cast<float>(vh);
    f.scale = sx < sy ? sx : sy;
    f.w = static_cast<int>(lroundf(vw * f.scale));
    f.h = static_cast<int>(lroundf(vh * f.scale));
    if (f.w > pane.w) f.w = pane.w;
    if (f.h > pane.h) f.h = pane.h;
    f.ox = pane.x + (pane.w - f.w) / 2;
    f.oy = pane.y + (pane.h - f.h) / 2;
    f.ok = true;
    return f;
}

// Virtual rect -> engine px. Width / height are differences of rounded edges (neighbouring virtual rects never gain a gap or an overlap).
inline Rect mapRect(const Fit& f, const Rect& v) {
    const int x0 = f.ox + static_cast<int>(lroundf(v.x * f.scale)), y0 = f.oy + static_cast<int>(lroundf(v.y * f.scale));
    const int x1 = f.ox + static_cast<int>(lroundf((v.x + v.w) * f.scale)), y1 = f.oy + static_cast<int>(lroundf((v.y + v.h) * f.scale));
    return Rect{x0, y0, x1 - x0, y1 - y0};
}

// Intersection of r with clip. false (and *out zeroed) when the result has no area.
inline bool clipRect(const Rect& r, const Rect& clip, Rect* out) {
    const int x0 = r.x > clip.x ? r.x : clip.x, y0 = r.y > clip.y ? r.y : clip.y;
    const int x1 = (r.x + r.w) < (clip.x + clip.w) ? (r.x + r.w) : (clip.x + clip.w);
    const int y1 = (r.y + r.h) < (clip.y + clip.h) ? (r.y + r.h) : (clip.y + clip.h);
    if (x1 <= x0 || y1 <= y0 || r.w <= 0 || r.h <= 0) { if (out) *out = Rect{0, 0, 0, 0}; return false; }
    if (out) *out = Rect{x0, y0, x1 - x0, y1 - y0};
    return true;
}

// ---- engine access (injectable) ------------------------------------------------------------------------------------------------------------
struct Ops {
    uintptr_t (*create)(uintptr_t panel, const char* tmpl);              // a hidden, click-through label cloned from tmpl; 0 = failed
    void (*release)(uintptr_t c);                                        // free a created label that was never listed (or is unlinked)
    void (*setVisible)(uintptr_t c, bool v);
    void (*setClickable)(uintptr_t c, bool v);
    void (*setRect)(uintptr_t c, int x, int y, int w, int h);
    void (*setFill)(uintptr_t c, void* img);
    void (*setAlpha)(uintptr_t c, float a);
    void (*setTint)(uintptr_t c, const float* rgb);
    void (*setText)(uintptr_t c, const char* text);
    bool (*isVisible)(uintptr_t c);
    bool (*getFill)(uintptr_t c, void** img);                            // the label's own fill image (read once at create, restored before release)
};

namespace detail {
inline uintptr_t defCreate(uintptr_t panel, const char* tmpl) {
    int why = guikit::CF_None; uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, tmpl, &why, &bv);
    if (!c) return 0;
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    guikit::setFillStyle(c, 2);   // LBL_TITLE has no stretch fill style
    return c;
}
inline void defRelease(uintptr_t c) { guikit::releaseUnlinkedLabel(c, guikit::LabelVtable); }
inline void defVisible(uintptr_t c, bool v) { guikit::setVisible(c, v); guikit::setClickable(c, false); }   // shown labels must stay click-through
inline void defClickable(uintptr_t c, bool v) { guikit::setClickable(c, v); }
inline void defRect(uintptr_t c, int x, int y, int w, int h) { guikit::setRect(c, x, y, w, h); }
inline void defFill(uintptr_t c, void* img) { guikit::setFill(c, img); }
inline void defAlpha(uintptr_t c, float a) { guikit::setAlpha(c, a); }
inline void defTint(uintptr_t c, const float* rgb) { guikit::setBorderTint(c, rgb); }
inline void defText(uintptr_t c, const char* t) { guikit::setText(c, t); }
inline bool defIsVisible(uintptr_t c) { return guikit::isVisible(c); }
inline bool defGetFill(uintptr_t c, void** img) { return guikit::getFill(c, img); }
}  // namespace detail

inline Ops& ops() {
    static Ops o = { detail::defCreate, detail::defRelease, detail::defVisible, detail::defClickable, detail::defRect, detail::defFill,
                     detail::defAlpha, detail::defTint, detail::defText, detail::defIsVisible, detail::defGetFill };
    return o;
}

// ---- the canvas --------------------------------------------------------------------------------------------------------------------------
// One label with change-only setters (cache = the last value written), the same pattern as EnemyStrip.cpp's Wid.
struct Wid {
    uintptr_t c = 0;
    int vis = -1;                 // -1 unknown, 0 hidden, 1 shown
    Rect rect = {0, 0, 0, 0};
    bool rectSet = false;
    void* fill = nullptr;
    bool fillSet = false;
    void* origFill = nullptr;     // the label's own fill image at create: the border dtor deletes what sits in border+0x74, so it goes back before release
    bool origSet = false;
    float alpha = -1.f;
    float tint[3] = {0.f, 0.f, 0.f};
    bool tintSet = false;
};

struct Canvas {
    Wid w[kMaxLabels];
    int32_t item[kMaxLabels];     // page item id per label (AddCustom)
    bool used[kMaxLabels];        // put() this frame
    int n = 0;                    // labels made AND listed (the first n of w[])
    Fit fit;
    int viewX = 0, viewY = 0;     // virtual origin of the current frame's view
    Rect pane = {0, 0, 0, 0};
    bool inFrame = false;
    bool paneWarned = false;

    Canvas() { reset(); }
    void reset() {
        for (int i = 0; i < kMaxLabels; ++i) { w[i] = Wid(); item[i] = -1; used[i] = false; }
        n = 0; fit = Fit(); viewX = viewY = 0; pane = Rect{0, 0, 0, 0}; inFrame = false; paneWarned = false;
    }

    // Options ctor window, after the rows: clone `want` labels (1..kMaxLabels) and list them on the page. All or nothing: when any clone or AddCustom
    // fails, the created-but-unlisted ones are freed, the listed ones stay hidden (they cannot be taken back) and ok() is false, so nothing draws.
    // Each clone's inherited "OPTIONS" title text is blanked.
    bool build(const K2MenuUiApi* ui, void* panel, uint32_t page, int want) {
        reset();
        if (!ui || !ui->AddCustom || !panel || want < 1 || want > kMaxLabels) return false;
        Ops& o = ops();
        const uintptr_t pn = reinterpret_cast<uintptr_t>(panel);
        Wid tmp[kMaxLabels];
        int made = 0;
        for (; made < want; ++made) {
            const uintptr_t c = o.create(pn, "LBL_TITLE");
            if (!c) break;
            tmp[made] = Wid();
            tmp[made].c = c; tmp[made].vis = 0;
            tmp[made].origSet = o.getFill && o.getFill(c, &tmp[made].origFill);
            o.setText(c, "");
        }
        int listed = 0;
        if (made == want) {
            for (; listed < want; ++listed) {
                const int id = ui->AddCustom(page, reinterpret_cast<void*>(tmp[listed].c), MUI_NOFREE);
                if (id < 0) break;
                w[listed] = tmp[listed];
                item[listed] = id;
            }
            n = listed;
        }
        for (int j = n; j < made; ++j) o.release(tmp[j].c);   // created but never listed
        want_ = want;
        return ok();
    }
    bool ok() const { return n > 0 && n == want_; }

    // Start of a frame: read the preview pane and fit the virtual screen into it. false = nothing to draw (no pane, bad size, canvas not built):
    // the caller then calls hideAll(). Every label is click-through; mod-config shows page items on entry, so a stale visibility cache is resynced.
    bool begin(const K2MenuUiApi* ui, void* panel, int vw, int vh) { return beginView(ui, panel, 0, 0, vw, vh); }
    // Same, but only the part of the virtual screen starting at (vx, vy) and vw x vh big is fitted into the pane (a zoomed view; put() takes the
    // virtual screen's own coordinates and clips what falls outside the view).
    bool beginView(const K2MenuUiApi* ui, void* panel, int vx, int vy, int vw, int vh) {
        if (!ok() || !ui || !ui->PaneRect) return false;
        resync();
        K2MuiRect r = {0, 0, 0, 0};
        if (!ui->PaneRect(panel, MUI_PANE_PREVIEW, &r) || r.w <= 0 || r.h <= 0) { paneWarned = true; return false; }
        pane = Rect{r.x, r.y, r.w, r.h};
        fit = fitScreen(vw, vh, pane);
        if (!fit.ok) return false;
        viewX = vx; viewY = vy;
        for (int i = 0; i < n; ++i) used[i] = false;
        inFrame = true;
        return true;
    }

    // Label i draws `v` (virtual units; clipped to the pane) with this fill, tint (3 floats, null = keep) and alpha. A rect with no area hides it.
    void put(int i, const Rect& v, void* img, const float* tint, float alpha) {
        if (!inFrame || i < 0 || i >= n || !w[i].c) return;
        putPx(i, mapRect(fit, Rect{v.x - viewX, v.y - viewY, v.w, v.h}), img, tint, alpha);
    }
    // Same in engine px (not scaled, not offset; still clipped to the pane): captions and frames that sit on the pane, not on the virtual screen.
    void putPx(int i, const Rect& px, void* img, const float* tint, float alpha) {
        if (!inFrame || i < 0 || i >= n || !w[i].c) return;
        Rect r;
        if (!clipRect(px, pane, &r)) return;   // stays unused: end() hides it
        style(i, r, img, tint, alpha);
        show(i);
        used[i] = true;
    }
    // Rect, fill, tint and alpha of label i in engine px with change-only setters; never touches visibility or the frame (no clip, no pane needed). For a
    // canvas whose labels something else shows and hides (a frame listed on a modal page: the provider shows it with the modal).
    void style(int i, const Rect& r, void* img, const float* tint, float alpha) {
        if (i < 0 || i >= n || !w[i].c) return;
        Wid& x = w[i];
        Ops& o = ops();
        if (!x.rectSet || x.rect.x != r.x || x.rect.y != r.y || x.rect.w != r.w || x.rect.h != r.h) { o.setRect(x.c, r.x, r.y, r.w, r.h); x.rect = r; x.rectSet = true; }
        if (!x.fillSet || x.fill != img) { o.setFill(x.c, img); x.fill = img; x.fillSet = true; }
        if (tint && (!x.tintSet || x.tint[0] != tint[0] || x.tint[1] != tint[1] || x.tint[2] != tint[2])) {
            o.setTint(x.c, tint); x.tint[0] = tint[0]; x.tint[1] = tint[1]; x.tint[2] = tint[2]; x.tintSet = true;
        }
        if (x.alpha != alpha) { o.setAlpha(x.c, alpha); x.alpha = alpha; }
    }
    // Text on a label already put this frame (centred captions: the caller sets the alignment once at build through the engine).
    void text(int i, const char* t) { if (inFrame && i >= 0 && i < n && w[i].c) ops().setText(w[i].c, t ? t : ""); }

    // End of a frame: every label not put is hidden.
    void end() {
        if (!inFrame) return;
        for (int i = 0; i < n; ++i) if (!used[i]) hide(i);
        inFrame = false;
    }
    void hideAll() { for (int i = 0; i < n; ++i) hide(i); inFrame = false; }

    void show(int i) { Wid& x = w[i]; if (x.c && x.vis != 1) { ops().setVisible(x.c, true); x.vis = 1; } }
    void hide(int i) { Wid& x = w[i]; if (x.c && x.vis != 0) { ops().setVisible(x.c, false); x.vis = 0; } }

    // mod-config shows every page item on entry: forget a visibility cache that no longer matches.
    void resync() {
        Ops& o = ops();
        for (int i = 0; i < n; ++i) {
            Wid& x = w[i];
            if (x.c && x.vis >= 0 && o.isVisible(x.c) != (x.vis == 1)) x.vis = -1;
        }
    }

    // Options dtor, after mod-config unlinked its controls: free the labels it listed MUI_NOFREE. A label whose Control() still answers is still in
    // the panel list (the unlink was refused): it is leaked, never freed. Returns the number freed.
    int destroy(const K2MenuUiApi* ui, uint32_t page) {
        int freed = 0;
        if (n && ui && ui->Control && page) {
            for (int i = 0; i < n; ++i) {
                if (!w[i].c) continue;
                if (ui->Control(page, item[i])) continue;
                // style() put a POOLED image (shared by many labels, owned by no label) into border+0x74, and the label dtor deletes that image: the
                // first label freed would delete it for all (crash 10-21 23:26: second label's border dtor called a deleted image). Give the label
                // its own fill back first (null when it could not be read: a small leak, never a crash).
                if (w[i].fillSet) ops().setFill(w[i].c, w[i].origSet ? w[i].origFill : nullptr);
                ops().release(w[i].c);
                ++freed;
            }
        }
        reset();
        return freed;
    }

private:
    int want_ = 0;
};

}  // namespace pvc
