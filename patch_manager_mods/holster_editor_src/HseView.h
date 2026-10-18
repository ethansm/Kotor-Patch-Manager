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

// World point -> view screen position for the preview camera (U0 probe; S1 arbitration). Pure, no engine calls.
// The camera node pose is (camPos, camQ w-first = w,x,y,z). It looks down node-local -Z, up +Y, right +X. fovyDeg is the VERTICAL fov
// (gluPerspective); aspect is ext.w / ext.h. Screen = top-left UI units: sx = ext.x + (ndcX + 1) / 2 * w, sy = ext.y + (1 - ndcY) / 2 * h.
struct WorldScreen {
    bool ok;
    float sx;
    float sy;
    float depth;   // distance along the camera forward axis (0 when not computed)
};

constexpr float kMinProjDepth = 0.05f;

inline bool finiteF(float v) { return !(isnan(v) || isinf(v)); }

inline WorldScreen worldToScreen(const float camPos[3], const float camQ[4], float fovyDeg, const HseRect& ext, const float P[3]) {
    WorldScreen bad = {false, 0.0f, 0.0f, 0.0f};
    if (!camPos || !camQ || !P) return bad;
    for (int i = 0; i < 3; ++i) if (!finiteF(camPos[i]) || !finiteF(P[i])) return bad;
    for (int i = 0; i < 4; ++i) if (!finiteF(camQ[i])) return bad;
    if (!finiteF(fovyDeg) || fovyDeg <= 0.0f || fovyDeg >= 179.0f) return bad;
    if (ext.w <= 0 || ext.h <= 0) return bad;
    float qn = sqrtf(camQ[0] * camQ[0] + camQ[1] * camQ[1] + camQ[2] * camQ[2] + camQ[3] * camQ[3]);
    if (!(qn > 1e-6f)) return bad;
    const float qw = camQ[0] / qn, qx = camQ[1] / qn, qy = camQ[2] / qn, qz = camQ[3] / qn;
    // camera axes in world space: R(q) * (1,0,0), (0,1,0), (0,0,-1); v' = v + w*t + q.xyz x t, t = 2 * (q.xyz x v)
    const float basis[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}};
    float ax[3][3];
    for (int k = 0; k < 3; ++k) {
        const float* v = basis[k];
        float tx = 2.0f * (qy * v[2] - qz * v[1]);
        float ty = 2.0f * (qz * v[0] - qx * v[2]);
        float tz = 2.0f * (qx * v[1] - qy * v[0]);
        ax[k][0] = v[0] + qw * tx + (qy * tz - qz * ty);
        ax[k][1] = v[1] + qw * ty + (qz * tx - qx * tz);
        ax[k][2] = v[2] + qw * tz + (qx * ty - qy * tx);
    }
    const float d[3] = {P[0] - camPos[0], P[1] - camPos[1], P[2] - camPos[2]};
    const float xc = d[0] * ax[0][0] + d[1] * ax[0][1] + d[2] * ax[0][2];
    const float yc = d[0] * ax[1][0] + d[1] * ax[1][1] + d[2] * ax[1][2];
    const float depth = d[0] * ax[2][0] + d[1] * ax[2][1] + d[2] * ax[2][2];
    if (!finiteF(depth) || depth <= kMinProjDepth) return bad;
    const float t = tanf(fovyDeg * 3.14159265358979f / 360.0f);
    const float aspect = static_cast<float>(ext.w) / static_cast<float>(ext.h);
    const float ndcX = xc / (depth * t * aspect);
    const float ndcY = yc / (depth * t);
    WorldScreen r;
    r.sx = static_cast<float>(ext.x) + (ndcX + 1.0f) * 0.5f * static_cast<float>(ext.w);
    r.sy = static_cast<float>(ext.y) + (1.0f - ndcY) * 0.5f * static_cast<float>(ext.h);
    r.depth = depth;
    r.ok = finiteF(r.sx) && finiteF(r.sy);
    return r;
}

// A two-click confirm is armed and still inside its window (wrap-safe unsigned time difference).
inline bool guardArmLive(bool armed, uint32_t armedAt, uint32_t now, uint32_t windowMs) {
    return armed && (uint32_t)(now - armedAt) < windowMs;
}

// A transition may proceed when nothing is unsaved, or the same action was armed and is still live.
inline bool guardProceed(bool dirty, bool armedSame, uint32_t armedAt, uint32_t now, uint32_t windowMs) {
    return !dirty || guardArmLive(armedSame, armedAt, now, windowMs);
}

// ---- U0b calibration click probe (pure; the editor owns the clicks and the logging) ----
// A fixed ordered list of body targets the user clicks one at a time; the clicks are fitted offline against the node world positions.
// "right"/"left" are the CHARACTER's own sides. step is 0-based, round is 1-based; advance() wraps after the last target and bumps round.
struct CalibScript {
    static constexpr int kCount = 6;
    int step;     // 0 .. kCount-1
    int round;    // 1-based
    int clicks;   // advance() calls so far (clicks recorded)

    CalibScript() : step(0), round(1), clicks(0) {}

    static const char* targetName(int i) {
        static const char* const kNames[kCount] = {"head centre", "belt", "right hand", "left hand", "right foot", "left foot"};
        return (i >= 0 && i < kCount) ? kNames[i] : "";
    }
    const char* current() const { return targetName(step); }
    // Moves to the next target. True when it wrapped past the last one (a round just finished and round was bumped).
    bool advance() {
        ++clicks;
        if (step < 0 || step >= kCount - 1) {
            step = 0;
            ++round;
            return true;
        }
        ++step;
        return false;
    }
};

// "Calib 3/6 (round 1): click the right hand"; always NUL terminated; null/cap<=0 is a no-op.
inline void calibPrompt(char* out, int cap, const CalibScript* s) {
    if (!out || cap <= 0) return;
    out[0] = '\0';
    if (!s) return;
    snprintf(out, cap, "Calib %d/%d (round %d): click the %s", s->step + 1, CalibScript::kCount, s->round, s->current());
    out[cap - 1] = '\0';
}

// Item names carry a leading "{NN}" tag (e.g. "{06}Vibrosword"). Copies in to out without ONE leading "{digits}" group. A name with no
// tag, an unterminated or empty tag ("{", "{06", "{}", "{ab}"), or an empty string is copied untouched. out is NUL terminated and capped
// at cap-1 chars. Returns true when a tag was removed. null/cap<=0 is a no-op (returns false).
inline bool stripNameTag(const char* in, char* out, int cap) {
    if (!out || cap <= 0) return false;
    out[0] = '\0';
    if (!in) return false;
    const char* src = in;
    bool stripped = false;
    if (in[0] == '{') {
        int i = 1;
        while (in[i] >= '0' && in[i] <= '9') ++i;
        if (i > 1 && in[i] == '}') {
            src = in + i + 1;
            stripped = true;
        }
    }
    int k = 0;
    for (; src[k] && k < cap - 1; ++k) out[k] = src[k];
    out[k] = '\0';
    return stripped;
}

} // namespace hse
