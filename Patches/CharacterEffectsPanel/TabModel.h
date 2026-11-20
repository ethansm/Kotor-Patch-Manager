// TabModel.h: pure parts of the Effects | Stats tab UX (doc 51 / UX-TAB-W1 plan). Tabs in the header band (hit zones) +
// rich text A (per-value colour). No engine access, no windows.h, no STL: host-tested by tab_model_test.cpp.
// Units are GUI units (800x600), the same as effects_stats_mockup.py.
//
// Stats rows are not selectable (user feedback 2026-11-04): the selection and detail-box helpers were removed.
#pragma once

namespace tabm {

enum Tab { TabEffects = 0, TabStats = 1, TabNone = -1 };

struct Rect { int x, y, w, h; };

inline bool inRect(const Rect& r, int x, int y) {
  return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

// Which tab a click at (x, y) hits, using the two hit zones (plan option i: header split). TabNone = outside both.
inline int tabAt(int x, int y, const Rect& effects, const Rect& stats) {
  if (inRect(effects, x, y)) return TabEffects;
  if (inRect(stats, x, y)) return TabStats;
  return TabNone;
}

// True when both zones sit inside the header and do not overlap each other (a host check of the layout constants).
inline bool zonesValid(const Rect& header, const Rect& effects, const Rect& stats) {
  // long long: x + w can overflow int for absurd inputs, which must read as "invalid", not wrap around.
  auto inside = [](const Rect& outer, const Rect& in) {
    long long ix1 = (long long)in.x + in.w, iy1 = (long long)in.y + in.h;
    long long ox1 = (long long)outer.x + outer.w, oy1 = (long long)outer.y + outer.h;
    return in.w > 0 && in.h > 0 && in.x >= outer.x && in.y >= outer.y && ix1 <= ox1 && iy1 <= oy1;
  };
  if (!inside(header, effects) || !inside(header, stats)) return false;
  long long ex1 = (long long)effects.x + effects.w, ey1 = (long long)effects.y + effects.h;
  long long sx1 = (long long)stats.x + stats.w, sy1 = (long long)stats.y + stats.h;
  bool overlap = effects.x < sx1 && stats.x < ex1 && effects.y < sy1 && stats.y < ey1;
  return !overlap;
}

// Index of the row whose rect holds (x, y), or -1.
inline int rowAt(int x, int y, const Rect* rows, int n) {
  for (int i = 0; i < n; ++i) {
    if (inRect(rows[i], x, y)) return i;
  }
  return -1;
}

// Rich text A: colour class of a value string. "+N" = Gain (amber), "-N" = Loss (red), anything else Plain (cream).
// The sign must be followed by a digit, so "-" alone and "+x" stay Plain.
enum class ValueClass { Plain, Gain, Loss };

inline ValueClass classify(const char* val) {
  if (!val) return ValueClass::Plain;
  while (*val == ' ') ++val;
  if ((val[0] == '+' || val[0] == '-') && val[1] >= '0' && val[1] <= '9') {
    return val[0] == '+' ? ValueClass::Gain : ValueClass::Loss;
  }
  return ValueClass::Plain;
}

}  // namespace tabm
