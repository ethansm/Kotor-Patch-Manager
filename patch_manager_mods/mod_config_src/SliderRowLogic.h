// SliderRowLogic.h -- pure maths of the look-alike SLIDER option row (doc 46 MCF5; namespace mui; ints only, no engine access, no heap).
// A slider row lives in ONE row slot (rowSlotRect): [caption "Label: value"][gap][track]. The track carries the art parts: frame (the outline,
// also the hit area), wedge (the filled part, thin left to tall right) and mark (the thumb). Parts per row: caption, frame, wedge, mark = 4 items.
// Everything is integer arithmetic (long long for the range maths): the pixel <-> value mapping must not differ between x87 and SSE builds and
// must not overflow on a wide consumer range (INT_MIN..INT_MAX).
// Wiring (ModConfig.cpp, MCF5 part 2): press on the track grabs the thumb and jumps the value to the pointer; while LMB is held the value follows
// (sliderValueAt); the change handler is called from the tick with the snapped value and its FINAL result is stored and redrawn.
#pragma once
#include "MenuUiLogic.h"

namespace mui {

enum { RK_SLIDER = 2 };                                // RowKind value (MenuUiLogic.h reserves it); MUI_ROW_SLIDER in ModConfigApi.h
enum { kSliderCaptionPct = 42, kSliderThumbW = 10, kSliderPartCount = 4 };   // caption share of the slot, thumb width px at the reference size

struct SliderGeom {
    Rect whole, caption, track;                        // track = the frame rect (hit area); wedge and mark are derived from it
};

// [caption][gap][track]: the caption takes kSliderCaptionPct of the slot, the track the rest minus the 6 px gap. Narrow slots keep a >= 0 caption.
inline SliderGeom sliderGeom(const Rect& slot) {
    SliderGeom g;
    g.whole = slot;
    int cw = (slot.w * kSliderCaptionPct) / 100;
    int tw = slot.w - cw - kRowGap;
    if (tw < 0) { tw = 0; cw = slot.w > kRowGap ? slot.w - kRowGap : 0; }
    g.caption = Rect{slot.x, slot.y, cw, slot.h};
    g.track = Rect{slot.x + cw + kRowGap, slot.y, tw, slot.h};
    return g;
}

// The usable run inside the frame art: the frame texture (512x64) draws a ~8 px stroke, so the wedge, the thumb and the pointer-to-value mapping
// live in the frame rect inset by that stroke plus a 2 px margin (fractions of the rect, so every resolution matches). The frame stays the hit area.
// A rect too small to inset is returned unchanged.
inline Rect sliderInner(const Rect& frame) {
    int ix = (frame.w * 10) / 512, iy = (frame.h * 10) / 64;
    if (ix < 2) ix = 2;
    if (iy < 2) iy = 2;
    if (frame.w <= 2 * ix || frame.h <= 2 * iy) return frame;
    return Rect{frame.x + ix, frame.y + iy, frame.w - 2 * ix, frame.h - 2 * iy};
}

inline bool sliderHit(const SliderGeom& g, int px, int py) { return rectHas(g.track, px, py); }

inline void sliderRange(const RowCfg& c, int& lo, int& hi) {
    lo = c.minV;
    hi = c.maxV;
    if (hi < lo) { const int t = lo; lo = hi; hi = t; }
}

// Thumb position 0..track.w for a value (rounded to the nearest pixel, clamped). A zero-width range or track gives 0.
inline int sliderPosOf(const RowCfg& c, int trackW, int value) {
    int lo, hi;
    sliderRange(c, lo, hi);
    if (trackW <= 0 || hi == lo) return 0;
    const long long v = (long long)rowClamp(c, value) - lo, span = (long long)hi - lo;
    return (int)((v * trackW + span / 2) / span);
}

// Pointer x -> value, snapped to the row step from min and clamped: px at or left of the track start gives min, at or right of its end gives max
// (max is reachable even when (max - min) is not a multiple of the step). Rounds to the nearest step.
inline int sliderValueAt(const RowCfg& c, const Rect& track, int px) {
    int lo, hi;
    sliderRange(c, lo, hi);
    if (track.w <= 0 || hi == lo) return lo;
    const int d = px - track.x;
    if (d <= 0) return lo;
    if (d >= track.w) return hi;
    const long long span = (long long)hi - lo;
    const long long raw = ((long long)d * span + track.w / 2) / track.w;   // nearest unit
    const long long st = c.step < 1 ? 1 : c.step;
    long long v = ((raw + st / 2) / st) * st;                                // nearest step from min
    if (v > span) v = span;
    return rowClamp(c, (int)(lo + v));
}

// The wedge fills the track from its left edge to the thumb centre: {track.x, track.y, posOf, track.h}.
inline Rect sliderWedgeRect(const RowCfg& c, const Rect& track, int value) {
    return Rect{track.x, track.y, sliderPosOf(c, track.w, value), track.h};
}

// The mark (thumb) is kSliderThumbW wide, centred on the value position; it may overhang the frame by half its width at the ends, so the rect is
// shifted inside the track. A track narrower than the thumb gets a thumb as wide as the track.
inline Rect sliderThumbRect(const RowCfg& c, const Rect& track, int value, int thumbW = kSliderThumbW) {
    int w = thumbW < track.w ? thumbW : track.w;
    if (w < 0) w = 0;
    int x = track.x + sliderPosOf(c, track.w, value) - w / 2;
    if (x < track.x) x = track.x;
    if (x > track.x + track.w - w) x = track.x + track.w - w;
    return Rect{x, track.y, w, track.h};
}

// ---- drag state (one slider can be held at a time) ----
struct SliderDrag {
    bool active = false;
    int row = -1;
    int lastValue = 0;
};

// Press edge on the track of row `row` with the current value: grab, return the jumped-to value (a click on the track jumps there). The caller
// checks RF_DISABLED / hidden rows BEFORE calling.
inline int sliderBegin(SliderDrag& d, int row, const RowCfg& c, const Rect& track, int px) {
    d.active = true;
    d.row = row;
    d.lastValue = sliderValueAt(c, track, px);
    return d.lastValue;
}

// While held: the new value for pointer x; changed = whether it differs from the last reported value (call the handler only when it does).
inline int sliderMove(SliderDrag& d, const RowCfg& c, const Rect& track, int px, bool* changed) {
    const int v = sliderValueAt(c, track, px);
    if (changed) *changed = d.active && v != d.lastValue;
    if (d.active) d.lastValue = v;
    return v;
}

// A handler that snapped or rejected the value: later moves compare against what is stored, not what was requested.
inline void sliderSetStored(SliderDrag& d, int stored) { d.lastValue = stored; }

inline void sliderEnd(SliderDrag& d) {
    d.active = false;
    d.row = -1;
}

}  // namespace mui
