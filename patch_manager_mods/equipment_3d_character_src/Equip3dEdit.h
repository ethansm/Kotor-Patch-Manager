// Equipment 3D Character -- holster editor UI logic (pure: no Win32 types, no STL). Host-tested by equip3d_edit_test.cpp.
// Layout is integer math only (float rounding differed between x87 and SSE builds, F18); the only floats are the slider
// and thumb fractions, which are clamped before they touch a pixel. Everything here is a function of its arguments:
// the .cpp keeps the GDI calls, hit-testing state and logging.
#pragma once
#include <stdio.h>
#include <math.h>
#include "Equip3dLogic.h"   // struct Box { int x, y, w, h; }

namespace eq3d {

// ---------------------------------------------------------------- weapon row (holster view on the weapon buttons)
// Named WeaponRowLayout: EquipWeaponTable.inc already defines eq3d::WeaponRow (the baseitems row).
struct WeaponRowLayout { Box weapons, holster, gear; bool showHolster, showGear; };
// Left third of the button is the holster slot, a 2 px gap, then the weapon slot. Gear (edit holster placement) is a
// square of the button height just left of the button, shown only when the holster is shown and the weapon is holstered.
inline WeaponRowLayout weaponRow(Box btn, bool on, bool canHolster, bool holstered) {
    WeaponRowLayout r;
    r.weapons = btn;
    r.holster = Box{0, 0, 0, 0};
    r.gear = Box{0, 0, 0, 0};
    r.showHolster = false;
    r.showGear = false;
    if (!on || !canHolster) return r;
    int hw = btn.w / 3;
    r.showHolster = true;
    r.holster = Box{btn.x, btn.y, hw, btn.h};
    r.weapons = Box{btn.x + hw + 2, btn.y, btn.w - hw - 2, btn.h};
    if (holstered) {
        r.showGear = true;
        r.gear = Box{btn.x - 2 - btn.h, btn.y, btn.h, btn.h};
    }
    return r;
}

// ---------------------------------------------------------------- holster modal (pop-up pane)
constexpr int kModalRows = 6;      // slider rows (the holster placement axes)
constexpr int kModalScope = 3;     // scope boxes
constexpr int kModalLock = 3;      // X/Y/Z lock boxes
constexpr int kModalButtons = 4;   // Save Revert Reset Close

struct ModalRow { Box name, minus, track, thumb, value, plus; };
struct ModalLayout {
    Box header, handPrev, handNext;
    Box scope[kModalScope], lock[kModalLock];
    ModalRow rows[kModalRows];
    Box bonePrev, boneName, boneNext;
    Box buttons[kModalButtons];
    Box warning;
};

// Stacked from the top with a 4 px pad and 2 px gaps. Row: name 40 | minus 18 | track (flexible) | value 52 | plus 18.
// Thumb: 10 px wide, row high, centred at track.x + frac * track.w, but kept fully inside the track so it never covers
// the minus/value/plus boxes at the extremes. thumbFrac may be null (every thumb at 0.5); each entry is clamped to [0,1].
// Fits inside the pane for any pane >= 200 x 330 (the real pane is {294,134,232,336}; content ends 256 px below pane.y).
inline ModalLayout modalLayout(Box pane, const float* thumbFrac) {
    ModalLayout L = ModalLayout();
    const int pad = 4, gap = 2;
    const int left = pane.x + pad, right = pane.x + pane.w - pad;
    const int W = right - left;
    int y = pane.y + pad;

    // header row: title on the left, hand prev/next squares on the right
    L.handNext = Box{right - 18, y, 18, 18};
    L.handPrev = Box{L.handNext.x - gap - 18, y, 18, 18};
    L.header = Box{left, y, L.handPrev.x - gap - left, 18};
    y += 18 + gap;

    L.warning = Box{left, y, W, 14};
    y += 14 + gap;

    // scope (20 high) then lock (18 high): three equal columns each
    int c3 = (W - 2 * gap) / 3;
    for (int i = 0; i < kModalScope; ++i) L.scope[i] = Box{left + i * (c3 + gap), y, c3, 20};
    y += 20 + gap;
    for (int i = 0; i < kModalLock; ++i) L.lock[i] = Box{left + i * (c3 + gap), y, c3, 18};
    y += 18 + gap;

    // six slider rows, 20 high, 2 px apart
    for (int r = 0; r < kModalRows; ++r) {
        ModalRow& R = L.rows[r];
        int x = left;
        R.name = Box{x, y, 40, 20};
        x += 40 + gap;
        R.minus = Box{x, y, 18, 20};
        x += 18 + gap;
        R.plus = Box{right - 18, y, 18, 20};
        R.value = Box{R.plus.x - gap - 52, y, 52, 20};
        R.track = Box{x, y, R.value.x - gap - x, 20};

        float f = thumbFrac ? thumbFrac[r] : 0.5f;
        f = fminf(fmaxf(f, 0.f), 1.f);   // NaN -> 0
        int off = (int)(f * (float)R.track.w + 0.5f);
        int tx = R.track.x + off - 5;
        if (tx < R.track.x) tx = R.track.x;
        if (tx > R.track.x + R.track.w - 10) tx = R.track.x + R.track.w - 10;
        R.thumb = Box{tx, y, 10, 20};
        y += 20 + gap;
    }

    // bone row: prev 18, name, next 18
    L.bonePrev = Box{left, y, 18, 18};
    L.boneNext = Box{right - 18, y, 18, 18};
    L.boneName = Box{left + 18 + gap, y, W - 2 * (18 + gap), 18};
    y += 18 + gap;

    // buttons: four equal columns, 22 high
    int b4 = (W - (kModalButtons - 1) * gap) / kModalButtons;
    for (int i = 0; i < kModalButtons; ++i) L.buttons[i] = Box{left + i * (b4 + gap), y, b4, 22};
    return L;
}

// ---------------------------------------------------------------- slider <-> value
// Pixel x inside the track -> value. Clamped to the track; the ends return lo and hi exactly.
inline float sliderToValue(int px, Box track, float lo, float hi) {
    if (track.w <= 0) return lo;
    int d = px - track.x;
    if (d <= 0) return lo;
    if (d >= track.w) return hi;
    float t = (float)d / (float)track.w;
    return lo + t * (hi - lo);
}
// Value -> pixel x inside the track, rounded to the nearest pixel and clamped to [track.x, track.x + track.w].
inline int valueToSlider(float v, Box track, float lo, float hi) {
    if (track.w <= 0) return track.x;
    float t = (hi != lo) ? (v - lo) / (hi - lo) : 0.f;
    t = fminf(fmaxf(t, 0.f), 1.f);
    return track.x + (int)(t * (float)track.w + 0.5f);
}

// ---------------------------------------------------------------- hold-to-repeat (+/- buttons)
// One step on press, then (after a 400 ms hold) one step every 60 ms. Release resets. When a long stall leaves more than
// kRepeatMaxPerTick steps due, the excess is dropped and the schedule restarts at now (no replay burst).
constexpr unsigned kRepeatDelayMs = 400;
constexpr unsigned kRepeatIntervalMs = 60;
constexpr int kRepeatMaxPerTick = 8;
struct Repeat { int state; unsigned t0, last; };   // state: 0 released, 1 pressed (before first repeat), 2 repeating
// Call once per tick with the button's held state. Returns the number of steps to apply this tick.
inline int repeatTick(Repeat& r, bool held, unsigned nowMs) {
    if (!held) { r.state = 0; r.t0 = nowMs; r.last = nowMs; return 0; }
    if (r.state == 0) { r.state = 1; r.t0 = nowMs; r.last = nowMs; return 1; }
    unsigned due = (r.state == 1) ? r.t0 + kRepeatDelayMs : r.last + kRepeatIntervalMs;
    int n = 0;
    while (n < kRepeatMaxPerTick && (int)(nowMs - due) >= 0) {   // signed difference: safe across unsigned wrap
        r.state = 2;
        r.last = due;
        due += kRepeatIntervalMs;
        ++n;
    }
    if (n == kRepeatMaxPerTick) r.last = nowMs;   // drop the rest of a long stall
    return n;
}

// ---------------------------------------------------------------- drag mode from the mouse buttons and modifiers
enum DragMode { DmNone, DmRotate, DmMove, DmSpin };
struct DragSel { DragMode mode; bool fine, snap; int lockAxis; /* -1 none, 0..2 = x,y,z */ };
// Priority L > R > M. Shift = fine, Ctrl = snap. lockMask bit0 x, bit1 y, bit2 z; lockAxis is the lowest set bit.
inline DragSel dragMode(bool lmb, bool rmb, bool mmb, bool shift, bool ctrl, int lockMask) {
    DragSel s;
    s.mode = lmb ? DmRotate : rmb ? DmMove : mmb ? DmSpin : DmNone;
    s.fine = shift;
    s.snap = ctrl;
    s.lockAxis = -1;
    for (int b = 0; b < 3; ++b) {
        if (lockMask & (1 << b)) { s.lockAxis = b; break; }
    }
    return s;
}

// A press and release less than 4 px apart (Euclidean) is a click, not a drag.
inline bool midClick(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0, dy = y1 - y0;
    return dx * dx + dy * dy < 16;
}

// ---------------------------------------------------------------- text
// Angles: "+12.3" + degree sign (cp1252 0xB0 single byte). Metres: "+0.123 m". Always NUL-terminated within cap.
inline void fmtValue(char* out, int cap, float v, bool angle) {
    if (cap <= 0) return;
    if (angle) snprintf(out, (size_t)cap, "%+.1f\xB0", v);
    else snprintf(out, (size_t)cap, "%+.3f m", v);
}

inline const char* legendText() {
    return "L-drag rotate | R-drag move | Shift+wheel depth | wheel zoom | M-drag spin | X/Y/Z lock (bone axis; may change all 3 Rot) | Shift fine | Ctrl snap 5deg/0.01m";
}

enum EditCtl { EcHolster, EcGear, EcCount };
inline const char* tipFor(EditCtl c) {
    switch (c) {
        case EcHolster: return "Holster view: show weapons stowed";
        case EcGear: return "Edit holster placement";
        default: return "";
    }
}

}  // namespace eq3d
