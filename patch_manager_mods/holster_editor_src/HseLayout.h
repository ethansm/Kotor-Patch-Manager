// Holster editor modal layout and legend (pure: no Win32 types, no STL). Host-tested by hse_layout_test.cpp.
// Layout is integer math only (float rounding differed between x87 and SSE builds, F18); the only floats are the thumb
// fractions, which are clamped before they touch a pixel. Everything here is a function of its arguments. The generic
// helpers (Box, slider maths, drag mode) live in _shared_src/EditKit.h.
#pragma once
#include <math.h>
#include "EditKit.h"
#include "MenuLayout.h"

namespace hse {

// ---------------------------------------------------------------- holster modal (pop-up pane)
constexpr int kModalRows = 6;      // slider rows (the holster placement axes)
constexpr int kModalScope = 3;     // scope boxes
constexpr int kModalLock = 3;      // X/Y/Z lock boxes
constexpr int kModalButtons = 4;   // Save Revert Reset Close

struct ModalRow { editkit::Box name, minus, track, thumb, value, plus; };
struct ModalLayout {
    editkit::Box header, handPrev, handNext;
    editkit::Box scope[kModalScope], lock[kModalLock];
    ModalRow rows[kModalRows];
    editkit::Box bonePrev, boneName, boneNext;
    editkit::Box buttons[kModalButtons];
    editkit::Box warning;
};

// Stacked from the top with a 4 px pad and 2 px gaps. Row: name 40 | minus 18 | track (flexible) | value 52 | plus 18.
// Thumb: 10 px wide, row high, centred at track.x + frac * track.w, but kept fully inside the track so it never covers
// the minus/value/plus boxes at the extremes. thumbFrac may be null (every thumb at 0.5); each entry is clamped to [0,1].
// Fits inside the pane for any pane >= 200 x 330 (the real pane is {294,134,232,336}; content ends 256 px below pane.y).
inline ModalLayout modalLayout(editkit::Box pane, const float* thumbFrac) {
    ModalLayout L = ModalLayout();
    const int pad = 4, gap = 2;
    const int left = pane.x + pad, right = pane.x + pane.w - pad;
    const int W = right - left;
    int y = pane.y + pad;

    // header row: title on the left, hand prev/next squares on the right
    L.handNext = editkit::Box{right - 18, y, 18, 18};
    L.handPrev = editkit::Box{L.handNext.x - gap - 18, y, 18, 18};
    L.header = editkit::Box{left, y, L.handPrev.x - gap - left, 18};
    y += 18 + gap;

    L.warning = editkit::Box{left, y, W, 14};
    y += 14 + gap;

    // scope (20 high) then lock (18 high): three equal columns each
    int c3 = (W - 2 * gap) / 3;
    for (int i = 0; i < kModalScope; ++i) L.scope[i] = editkit::Box{left + i * (c3 + gap), y, c3, 20};
    y += 20 + gap;
    for (int i = 0; i < kModalLock; ++i) L.lock[i] = editkit::Box{left + i * (c3 + gap), y, c3, 18};
    y += 18 + gap;

    // six slider rows, 20 high, 2 px apart
    for (int r = 0; r < kModalRows; ++r) {
        ModalRow& R = L.rows[r];
        int x = left;
        R.name = editkit::Box{x, y, 40, 20};
        x += 40 + gap;
        R.minus = editkit::Box{x, y, 18, 20};
        x += 18 + gap;
        R.plus = editkit::Box{right - 18, y, 18, 20};
        R.value = editkit::Box{R.plus.x - gap - 52, y, 52, 20};
        R.track = editkit::Box{x, y, R.value.x - gap - x, 20};

        float f = thumbFrac ? thumbFrac[r] : 0.5f;
        f = fminf(fmaxf(f, 0.f), 1.f);   // NaN -> 0
        int off = (int)(f * (float)R.track.w + 0.5f);
        int tx = R.track.x + off - 5;
        if (tx < R.track.x) tx = R.track.x;
        if (tx > R.track.x + R.track.w - 10) tx = R.track.x + R.track.w - 10;
        R.thumb = editkit::Box{tx, y, 10, 20};
        y += 20 + gap;
    }

    // bone row: prev 18, name, next 18
    L.bonePrev = editkit::Box{left, y, 18, 18};
    L.boneNext = editkit::Box{right - 18, y, 18, 18};
    L.boneName = editkit::Box{left + 18 + gap, y, W - 2 * (18 + gap), 18};
    y += 18 + gap;

    // buttons: four equal columns, 22 high
    int b4 = (W - (kModalButtons - 1) * gap) / kModalButtons;
    for (int i = 0; i < kModalButtons; ++i) L.buttons[i] = editkit::Box{left + i * (b4 + gap), y, b4, 22};
    return L;
}

// ---------------------------------------------------------------- editor view + status (card c13)
// The 3D view sits LEFT of the description pane (LB_DESC), so the title and status labels no longer share its rect. Layout units
// are the vanilla 800x600 GUI space: LB_DESC is 232 x 336 and the view is 216 x 336, 12 units left of it. Everything scales by
// s = desc.h / 336 (integer math, rounded to nearest): w = 216 s, gap = 12 s, x = desc.x - gap - w, y = desc.y, h = desc.h.
// A desc rect with no area gives the zero rect.
constexpr int kViewRefW = 216, kViewRefH = 336, kViewRefGap = 12;
inline menulayout::Rect viewRect(const menulayout::Rect& desc) {
    menulayout::Rect r = {0, 0, 0, 0};
    if (desc.w <= 0 || desc.h <= 0) return r;
    const int w = (kViewRefW * desc.h + kViewRefH / 2) / kViewRefH;
    const int gap = (kViewRefGap * desc.h + kViewRefH / 2) / kViewRefH;
    r.x = desc.x - gap - w;
    r.y = desc.y;
    r.w = w;
    r.h = desc.h;
    return r;
}

// Floor for menulayout::statusStrip: the top y of the nearest slider bar below the button column (strictly below quitBottom) that
// leaves at least 16 px under the view; else the nearest bar at least 16 px under the view; else viewBottom + 34 (a full 28 px strip).
inline int statusFloor(const int* barY, int n, int quitBottom, int viewBottom) {
    const int kRoom = 16;
    int best = 0;
    bool have = false;
    for (int i = 0; barY && i < n; ++i)
        if (barY[i] > quitBottom && barY[i] - viewBottom >= kRoom && (!have || barY[i] < best)) { best = barY[i]; have = true; }
    if (have) return best;
    for (int i = 0; barY && i < n; ++i)
        if (barY[i] - viewBottom >= kRoom && (!have || barY[i] < best)) { best = barY[i]; have = true; }
    return have ? best : viewBottom + 34;
}

// Legend shown while the modal is open: the drag controls.
inline const char* legendText() {
    return "L-drag rotate | R-drag move | Shift+wheel depth | wheel zoom | M-drag spin | X/Y/Z lock (bone axis; may change all 3 Rot) | Shift fine | Ctrl snap 5deg/0.01m";
}

}  // namespace hse
