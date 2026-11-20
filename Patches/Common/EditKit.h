// EditKit -- generic UI helpers for an in-game editor (pure: no Win32 types, no STL, no game headers). Shared by the
// holster-editor patch (doc 42 HW-EDIT v2). Host-tested by editkit_test.cpp.
// Pixel maths is integer; the only floats are the slider fractions, clamped before they touch a pixel (float rounding
// differed between x87 and SSE builds, F18). Everything here is a function of its arguments: the .cpp keeps the GDI
// calls, hit-testing state and logging.
#pragma once
#include <stdio.h>
#include <math.h>

namespace editkit {

struct Box { int x, y, w, h; };

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

}  // namespace editkit
