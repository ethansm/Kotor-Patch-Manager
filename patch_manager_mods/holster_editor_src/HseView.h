// Holster editor 3D preview viewport math and strip layout helpers (pure: no Win32, no STL, no engine headers).
// Host-tested by hse_view_test.cpp (doc 20).
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>

namespace hse {

struct View {
    float yaw;
    float zoom;
    float panZ;
};

constexpr float kZoomMin = 0.8f;
constexpr float kZoomMax = 4.0f;
constexpr float kSpinDegPerPx = 0.5f;
constexpr float kZoomStep = 1.25f;

// Deg mapped to [0, 360); NaN or inf -> 0.0f.
inline float wrapYaw(float deg) {
    if (isnan(deg) || isinf(deg)) return 0.0f;
    float y = fmodf(deg, 360.0f);
    if (y < 0.0f) y += 360.0f;
    if (y >= 360.0f) y = 0.0f;
    if (y == 0.0f) y = 0.0f;
    return y;
}

inline float spin(float yaw, float dxPx) {
    return wrapYaw(yaw + dxPx * kSpinDegPerPx);
}

// [0.8, 4.0]; NaN -> 2.2f; +-inf clamp to the range ends.
inline float clampZoom(float z) {
    if (isnan(z)) return 2.2f;
    if (z < kZoomMin) return kZoomMin;
    if (z > kZoomMax) return kZoomMax;
    return z;
}

inline float zoomIn(float z) {
    return clampZoom(z * kZoomStep);
}

inline float zoomOut(float z) {
    return clampZoom(z / kZoomStep);
}

// wrapYaw, clampZoom, panZ clamped to [-1, 1] (NaN -> 0.0f; +-inf clamp to +-1).
inline View makeView(float yaw, float zoom, float panZ) {
    float p = panZ;
    if (isnan(p)) {
        p = 0.0f;
    } else if (p < -1.0f) {
        p = -1.0f;
    } else if (p > 1.0f) {
        p = 1.0f;
    }
    return View{wrapYaw(yaw), clampZoom(zoom), p};
}

// panZ + dir * 0.1, clamped to [-1, 1]; dir is +1 (Up) or -1 (Down); NaN -> 0.0f; +-inf clamp to +-1.
inline float panStep(float panZ, int dir) {
    float delta = (dir > 0) ? 0.1f : ((dir < 0) ? -0.1f : 0.0f);
    float n = panZ + delta;
    if (isnan(n)) return 0.0f;
    if (n < -1.0f) return -1.0f;
    if (n > 1.0f) return 1.0f;
    return n;
}

inline float flipYaw(float yaw) {
    return wrapYaw(yaw + 180.0f);
}

// (i + delta) wrapped into [0, n); n <= 0 -> 0.
inline int wrapIndex(int i, int delta, int n) {
    if (n <= 0) return 0;
    int r = (i + delta) % n;
    if (r < 0) r += n;
    return r;
}

// NOT named Rect: four other namespaces already have a Rect (R12d).
struct HseRect {
    int x;
    int y;
    int w;
    int h;
};

// {col.x, col.y, max(0, view.x - gap - col.x), col.h}
inline HseRect leftStrip(const HseRect& col, const HseRect& view, int gap) {
    int w = view.x - gap - col.x;
    if (w < 0) w = 0;
    return HseRect{col.x, col.y, w, col.h};
}

// vanilla ratio: slotH = strip.h * 32 / 284, pitch = strip.h * 36 / 284, y = strip.y + row * pitch, x/w = strip's
inline HseRect stripSlot(const HseRect& strip, int row) {
    int slotH = strip.h * 32 / 284;
    int pitch = strip.h * 36 / 284;
    return HseRect{strip.x, strip.y + row * pitch, strip.w, slotH};
}

// which 0 = left, 1 = right; w = (s.w - gap) / 2; right.x = s.x + w + gap
inline HseRect half(const HseRect& s, int which, int gap) {
    int w = (s.w - gap) / 2;
    if (w < 0) w = 0;
    int x = (which == 0) ? s.x : (s.x + w + gap);
    return HseRect{x, s.y, w, s.h};
}

// name when non-empty, else "Member <idx0+1>"; always NUL terminated; cap <= 0 is a no-op
inline void memberLabel(char* out, int cap, int idx0, const char* name) {
    if (cap <= 0 || !out) return;
    if (name && name[0] != '\0') {
        snprintf(out, cap, "%s", name);
    } else {
        snprintf(out, cap, "Member %d", idx0 + 1);
    }
    out[cap - 1] = '\0';
}

// Scope label: 0=Weapon, 1=Category, 2=Body
inline const char* scopeLabel(int scope) {
    switch (scope) {
        case 0: return "Scope: Weapon";
        case 1: return "Scope: Category";
        case 2: return "Scope: Body";
        default: return "Scope: Weapon";
    }
}

// Accumulate delta, return the part that is a whole multiple of step (truncated toward zero), keep the rest in acc.
// Invariant: sum(applied) + acc == sum(delta). step must be > 0.
inline float snapAccum(float& acc, float delta, float step) {
    acc += delta;
    float applied = truncf(acc / step) * step;
    acc -= applied;
    return applied;
}

// A two-click confirm is armed and still inside its window (wrap-safe unsigned time difference).
inline bool guardArmLive(bool armed, uint32_t armedAt, uint32_t now, uint32_t windowMs) {
    return armed && (uint32_t)(now - armedAt) < windowMs;
}

// A transition may proceed when nothing is unsaved, or the same action was armed and is still live.
inline bool guardProceed(bool dirty, bool armedSame, uint32_t armedAt, uint32_t now, uint32_t windowMs) {
    return !dirty || guardArmLive(armedSame, armedAt, now, windowMs);
}

} // namespace hse
