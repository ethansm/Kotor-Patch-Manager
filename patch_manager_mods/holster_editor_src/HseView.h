// Holster editor 3D preview viewport math and strip layout helpers (pure: no Win32, no STL, no engine headers).
// Host-tested by hse_view_test.cpp (doc 20).
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

// ---- CAMENG probe maths (U4 Q-L; pure, mirrors Camera::MapToScreen 0x4809D0 so the engine answer can be checked) ----
// Eye-space point of the world point P for a camera at camPos with orientation camQ (w,x,y,z; normalised here): eye = conj(q) * (P - pos).
// The camera looks down eye -Z, so a point in front has eye[2] < 0. False (eye zeroed) on a non-finite input or a quaternion shorter than 1e-6.
inline bool eyeOf(const float camPos[3], const float camQ[4], const float P[3], float eye[3]) {
    if (eye) eye[0] = eye[1] = eye[2] = 0.0f;
    if (!camPos || !camQ || !P || !eye) return false;
    for (int i = 0; i < 3; ++i) if (!finiteF(camPos[i]) || !finiteF(P[i])) return false;
    for (int i = 0; i < 4; ++i) if (!finiteF(camQ[i])) return false;
    const float qn = sqrtf(camQ[0] * camQ[0] + camQ[1] * camQ[1] + camQ[2] * camQ[2] + camQ[3] * camQ[3]);
    if (!(qn > 1e-6f)) return false;
    const float w = camQ[0] / qn;
    const float ux = -camQ[1] / qn, uy = -camQ[2] / qn, uz = -camQ[3] / qn;   // conjugate: negated vector part
    const float v[3] = {P[0] - camPos[0], P[1] - camPos[1], P[2] - camPos[2]};
    const float tx = 2.0f * (uy * v[2] - uz * v[1]);   // t = 2 * (u x v)
    const float ty = 2.0f * (uz * v[0] - ux * v[2]);
    const float tz = 2.0f * (ux * v[1] - uy * v[0]);
    eye[0] = v[0] + w * tx + (uy * tz - uz * ty);      // v' = v + w*t + u x t
    eye[1] = v[1] + w * ty + (uz * tx - ux * tz);
    eye[2] = v[2] + w * tz + (ux * ty - uy * tx);
    return finiteF(eye[0]) && finiteF(eye[1]) && finiteF(eye[2]);
}

// A point is in front of the camera when its eye-space z is negative (the engine does no behind-camera clip: z >= 0 mirrors silently).
inline bool eyeInFront(const float eye[3]) { return eye && finiteF(eye[2]) && eye[2] < 0.0f; }

// The engine's raw MapToScreen answer (normalized = 0) predicted from the eye-space point: GL pixels, origin bottom-left, y up, no aspect term.
//   t = tan(fovy * pi / 360); X = vx + W/2 + (H/2)/t * (-ex/ez); Yg = vy + H/2 + (H/2)/t * (-ey/ez).  vp = {x, y, w, h}.
// False (out zeroed) when the viewport is empty, fovy is outside (0, 179), or eye is non-finite or has ez == 0. A point behind the camera still
// returns true (mirrored, like the engine); the caller tests eyeInFront.
inline bool rawPredict(const float eye[3], const int vp[4], float fovyDeg, float out[2]) {
    if (out) out[0] = out[1] = 0.0f;
    if (!eye || !vp || !out) return false;
    if (vp[2] <= 0 || vp[3] <= 0) return false;
    if (!finiteF(fovyDeg) || !(fovyDeg > 0.0f) || !(fovyDeg < 179.0f)) return false;
    for (int i = 0; i < 3; ++i) if (!finiteF(eye[i])) return false;
    if (eye[2] == 0.0f) return false;
    const float t = tanf(fovyDeg * 3.14159265358979f / 360.0f);
    const float hh = static_cast<float>(vp[3]) * 0.5f;
    out[0] = static_cast<float>(vp[0]) + static_cast<float>(vp[2]) * 0.5f + hh / t * (-eye[0] / eye[2]);
    out[1] = static_cast<float>(vp[1]) + hh + hh / t * (-eye[1] / eye[2]);
    return finiteF(out[0]) && finiteF(out[1]);
}

// The engine's normalized answer (normalized = 1) from a raw answer: nx = X / W, ny = 1 - Yg / H (top-left, unclipped; vx / vy are not subtracted).
inline bool rawToNorm(const float raw[2], const int vp[4], float out[2]) {
    if (out) out[0] = out[1] = 0.0f;
    if (!raw || !vp || !out || vp[2] <= 0 || vp[3] <= 0) return false;
    out[0] = raw[0] / static_cast<float>(vp[2]);
    out[1] = 1.0f - raw[1] / static_cast<float>(vp[3]);
    return finiteF(out[0]) && finiteF(out[1]);
}

// Viewport fractions (0..1, origin top-left) of an engine raw answer, for comparing with a hse::worldToScreen result whose ext is the same viewport:
// fx = (X - vx) / W, fy = 1 - (Yg - vy) / H.
inline void rawFrac(const float raw[2], const int vp[4], float frac[2]) {
    frac[0] = (raw[0] - static_cast<float>(vp[0])) / static_cast<float>(vp[2]);
    frac[1] = 1.0f - (raw[1] - static_cast<float>(vp[1])) / static_cast<float>(vp[3]);
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

// ---- U0b calibration prompt hint ----
// Appended to the calibration prompt when it fits the status label (the wheel is the only way to zoom out of a tight framing).
constexpr const char* kCalibHint = " (wheel = zoom out)";

// ---- U2 controls: mouse chord, weapon-mode gestures, labels ----
// Weapon edit mode input model (plan HW-UX2 §2.3; research H5 §5):
//   LMB alone         orbit the character (same as NAV)
//   RMB + LMB drag    act on the weapon per the mode bar (Move / Depth / Tilt / Roll+Turn)
//   RMB + wheel       depth in any mode      RMB alone  nothing (the old trackball is gone)
//   MMB drag          spin the character yaw
//   Shift = x0.2 (fine), Ctrl = snap (1 cm / 5 deg, carried in a SnapAcc)
// Chord is a pure per-tick step function: the editor feeds the button levels and press edges, the chord answers what the tick means.

enum ChordState { CS_IDLE = 0, CS_ORBIT, CS_WEAPON, CS_DEAD };
enum ChordAct { CA_NONE = 0, CA_ORBIT, CA_WEAPON, CA_DEPTH, CA_SPIN, CA_ZOOM };
// Why a state changed / a gesture began or ended (why), or why a press was dropped (drop). Names via chordWhyName.
enum ChordWhy {
    CW_NONE = 0,
    CW_LMB_DOWN,          // L pressed in the view: orbit begins
    CW_CHORD_RL,          // RMB held first, then LMB: weapon gesture begins
    CW_CHORD_SAME,        // both pressed in the same tick: weapon gesture begins (weapon wins)
    CW_LMB_UP,            // L released
    CW_RMB_UP,            // R released mid weapon gesture while L is still held: DEAD (no orbit jump)
    CW_ALL_UP,            // every button released
    CW_MMB_DOWN,          // M pressed in the view: spin begins
    CW_MMB_UP,            // M released: spin ends
    CW_FG_LOST,           // the game lost foreground: everything cancelled
    CW_FG_RETURN,         // foreground back with a button still held: DEAD until released
    CW_PRESS_OUTSIDE,     // a press began outside the view or on a button: never a gesture (DEAD until all released)
    CW_RMB_DURING_ORBIT,  // R pressed while an orbit is running: ignored until L is up
    CW_MMB_BUSY,          // M pressed while another gesture runs: ignored
    CW_MMB_ACTIVE,        // L or R pressed while the spin runs: DEAD until released
    CW_STALE_HELD,        // a button that was already held (never a valid press) met a new press: DEAD
    CW_COUNT
};

inline const char* chordWhyName(int w) {
    static const char* const kNames[CW_COUNT] = {"none", "lmb_down", "chord_rl", "chord_same", "lmb_up", "rmb_up", "all_up", "mmb_down", "mmb_up",
                                                 "fg_lost", "fg_return", "press_outside", "rmb_during_orbit", "mmb_busy", "mmb_active", "stale_held"};
    return (w >= 0 && w < CW_COUNT) ? kNames[w] : "?";
}
inline const char* chordStateName(int s) {
    static const char* const kNames[4] = {"IDLE", "ORBIT", "WEAPON", "DEAD"};
    return (s >= 0 && s < 4) ? kNames[s] : "?";
}
inline const char* chordActName(int a) {
    static const char* const kNames[6] = {"none", "orbit", "weapon", "depth", "spin", "zoom"};
    return (a >= 0 && a < 6) ? kNames[a] : "?";
}

struct ChordIn {
    bool l, r, m;            // button levels (the caller already forces them false while the game is in the background)
    bool lEdge, rEdge, mEdge;   // press edges this tick (level now, not the level on the previous tick)
    bool inView;             // the cursor is inside the 3D view and not over one of the editor's buttons
    bool fg;                 // the game window has foreground
    bool shift, ctrl;
    int x, y;                // cursor in engine px
    int wheel;               // wheel units this tick (120 per notch; 0 = none); the caller only counts notches inside the view
};

struct ChordOut {
    int prevState, state;
    int act;        // drag action this tick: CA_ORBIT / CA_WEAPON while a gesture runs, CA_SPIN while MMB spins, else CA_NONE
    int began;      // act that began this tick, else CA_NONE
    int ended;      // act that ended this tick (release, fg loss, DEAD), else CA_NONE
    int why;        // reason for a state change, gesture begin or end this tick, else CW_NONE
    int drop;       // reason a press was dropped this tick (no gesture), else CW_NONE
    int dx, dy;     // cursor movement since the previous tick of this gesture (0,0 on the begin tick)
    int wheelAct;   // CA_DEPTH (R held, weapon side), CA_ZOOM, or CA_NONE when wheel == 0
    int wheel;
    bool fine, snap;   // Shift, Ctrl this tick (mid-drag changes only change speed and snap, never jump)
};

struct Chord {
    int state;
    bool spinning;   // an MMB spin is running (state is IDLE, or DEAD when L/R were pressed meanwhile)
    bool rArmed;     // R was pressed validly (in the view) and is still held, with no gesture: the next L press makes a weapon gesture
    bool fgLost;     // foreground was lost: the first foreground tick with a held button goes DEAD
    bool mDead;      // M was pressed where it cannot start a spin: ignored until released
    int lastX, lastY;

    Chord() : state(CS_IDLE), spinning(false), rArmed(false), fgLost(false), mDead(false), lastX(0), lastY(0) {}

    void reset() { *this = Chord(); }
    // A gesture (or a latch that changes what the next press means) is in flight.
    bool busy() const { return state != CS_IDLE || spinning || rArmed; }
    int activeAct() const {
        if (state == CS_ORBIT) return CA_ORBIT;
        if (state == CS_WEAPON) return CA_WEAPON;
        if (spinning) return CA_SPIN;
        return CA_NONE;
    }

    ChordOut step(const ChordIn& in) {
        ChordOut o = {};
        o.prevState = state;
        o.fine = in.shift;
        o.snap = in.ctrl;
        o.wheel = in.wheel;

        // Foreground: the gesture is cancelled; a button still held on return is dead until it is released and pressed again.
        if (!in.fg) {
            if (busy()) {
                o.ended = activeAct();
                o.why = CW_FG_LOST;
            }
            state = CS_IDLE;
            spinning = false;
            rArmed = false;
            mDead = false;
            fgLost = true;
            o.state = state;
            return o;
        }
        if (fgLost) {
            fgLost = false;
            if (in.l || in.r || in.m) {
                state = CS_DEAD;
                o.why = CW_FG_RETURN;
            }
        }

        // Releases.
        if (!in.m) {
            mDead = false;
            if (spinning) {
                spinning = false;
                o.ended = CA_SPIN;
                o.why = CW_MMB_UP;
            }
        }
        if (!in.r) rArmed = false;
        switch (state) {
            case CS_DEAD:
                if (!in.l && !in.r && !in.m) {
                    state = CS_IDLE;
                    o.why = CW_ALL_UP;
                }
                break;
            case CS_ORBIT:
                if (!in.l) {
                    state = CS_IDLE;
                    o.ended = CA_ORBIT;
                    o.why = CW_LMB_UP;
                } else if (in.rEdge) {
                    o.drop = CW_RMB_DURING_ORBIT;
                }
                break;
            case CS_WEAPON:
                if (!in.l && !in.r) {
                    state = CS_IDLE;
                    o.ended = CA_WEAPON;
                    o.why = CW_ALL_UP;
                } else if (!in.l) {   // L lifted, R still down: the next L press in the view starts a new weapon gesture
                    state = CS_IDLE;
                    rArmed = true;
                    o.ended = CA_WEAPON;
                    o.why = CW_LMB_UP;
                } else if (!in.r) {   // R lifted with L down: dead, never an orbit jump
                    state = CS_DEAD;
                    o.ended = CA_WEAPON;
                    o.why = CW_RMB_UP;
                }
                break;
            default: break;
        }

        // Presses (R is read before L; a same-tick pair is a weapon gesture).
        if (state == CS_IDLE && (in.lEdge || in.rEdge)) {
            if (spinning) {
                state = CS_DEAD;
                o.why = CW_MMB_ACTIVE;
                o.drop = CW_MMB_ACTIVE;
            } else if (!in.inView) {
                state = CS_DEAD;
                o.why = CW_PRESS_OUTSIDE;
                o.drop = CW_PRESS_OUTSIDE;
            } else if (in.lEdge && in.rEdge) {
                state = CS_WEAPON;
                o.began = CA_WEAPON;
                o.why = CW_CHORD_SAME;
            } else if (in.rEdge) {
                if (in.l) {   // L is down but never started a gesture: do not guess
                    state = CS_DEAD;
                    o.why = CW_STALE_HELD;
                    o.drop = CW_STALE_HELD;
                } else {
                    rArmed = true;   // RMB alone does nothing
                }
            } else {   // lEdge only
                if (in.r && rArmed) {
                    state = CS_WEAPON;
                    o.began = CA_WEAPON;
                    o.why = CW_CHORD_RL;
                } else if (in.r) {
                    state = CS_DEAD;
                    o.why = CW_STALE_HELD;
                    o.drop = CW_STALE_HELD;
                } else {
                    state = CS_ORBIT;
                    o.began = CA_ORBIT;
                    o.why = CW_LMB_DOWN;
                }
            }
        }
        if (in.mEdge && !spinning) {
            if (!in.inView) {
                mDead = true;
                o.drop = CW_PRESS_OUTSIDE;
            } else if (state != CS_IDLE || in.l || in.r) {
                mDead = true;
                o.drop = CW_MMB_BUSY;
            } else if (!mDead) {
                spinning = true;
                o.began = CA_SPIN;
                o.why = CW_MMB_DOWN;
            }
        }

        // Action and cursor delta.
        o.act = activeAct();
        if (o.act != CA_NONE) {
            if (o.began == o.act) {
                lastX = in.x;
                lastY = in.y;
            } else {
                o.dx = in.x - lastX;
                o.dy = in.y - lastY;
                lastX = in.x;
                lastY = in.y;
            }
        }

        // Wheel: depth while R is down on the weapon side (weapon gesture, or R armed); an orbit or a dead press zooms.
        if (in.wheel != 0) o.wheelAct = (in.r && (state == CS_WEAPON || (state == CS_IDLE && rArmed))) ? CA_DEPTH : CA_ZOOM;

        o.state = state;
        return o;
    }
};

// Ctrl-snap remainders for a weapon gesture (a[0], a[1]) and for the wheel (a[2]).
struct SnapAcc {
    float a[3];
    SnapAcc() { a[0] = a[1] = a[2] = 0.0f; }
    bool nonzero() const { return a[0] != 0.0f || a[1] != 0.0f || a[2] != 0.0f; }
    void reset() { a[0] = a[1] = a[2] = 0.0f; }
};

enum WeaponMode { WPM_MOVE = 0, WPM_DEPTH, WPM_TILT, WPM_ROLLTURN, WPM_COUNT };
enum WeaponCallKind { WK_NONE = 0, WK_WORLD, WK_LOCAL };

constexpr float kMovePerPx = 0.0025f;     // metres per px at zoom 1 (divided by the zoom)
constexpr float kRotDegPerPx = 0.5f;
constexpr float kFineScale = 0.2f;
constexpr float kSnapMetres = 0.01f;
constexpr float kSnapDeg = 5.0f;
constexpr float kWheelDepthMetres = 0.02f;   // per notch

struct WeaponCall {
    int kind;       // WeaponCallKind
    int axis;       // WK_LOCAL: bone axis 0 x right, 1 y forward, 2 z up
    float deg;      // WK_LOCAL rotation
    float metres;   // WK_LOCAL translation along the axis
    float dp[3];    // WK_WORLD translation (world frame)
};

// Up to two provider calls per tick (Roll/Turn can move on both axes). a, b are the applied amounts for the GESTURE log line:
// Move (dp y, dp z) metres, Depth (metres, 0), Tilt (deg, 0), Roll/Turn (roll deg, turn deg).
struct WeaponStep {
    int n;
    WeaponCall c[2];
    float a, b;
};

inline const char* weaponModeName(int m) {
    static const char* const kNames[WPM_COUNT] = {"Move", "Depth", "Tilt", "Roll/Turn"};
    return (m >= 0 && m < WPM_COUNT) ? kNames[m] : "?";
}

// The one place the Move drag maps pixels to a world translation (camera fixed; U4 swaps this for the projected axes).
// dx right on screen -> world y, dy down on screen -> -world z.
inline void moveDp(float dx, float dy, float k, float dp[3]) {
    dp[0] = 0.0f;
    dp[1] = dx * k;
    dp[2] = -dy * k;
}

inline WeaponCall noCall() {
    WeaponCall c = {};
    c.kind = WK_NONE;
    return c;
}

inline float zoomK(float zoom, bool fine) {
    float k = kMovePerPx / (zoom > 0.1f ? zoom : 1.0f);
    return fine ? k * kFineScale : k;
}

// One tick of a weapon drag (dx, dy cursor px) for the mode latched at the gesture start.
//   Move       world dp = moveDp, Ctrl snaps y and z to 1 cm (acc.a[0], acc.a[1])
//   Depth      bone axis 1 (forward), metres = -dy * k, Ctrl 1 cm (acc.a[0])
//   Tilt       bone axis 0, deg = -dy * 0.5, Ctrl 5 deg (acc.a[0])
//   Roll/Turn  Turn = bone axis 2, deg = dx * 0.5 (acc.a[0]); Roll = bone axis 1, deg = -dy * 0.5 (acc.a[1]); Ctrl 5 deg each
// Shift scales by 0.2. Without Ctrl the used remainders are zeroed (a slow drag after releasing Ctrl starts clean).
inline WeaponStep weaponGesture(int mode, int dx, int dy, float zoom, bool fine, bool snap, SnapAcc& acc) {
    WeaponStep st = {};
    const float fx = static_cast<float>(dx), fy = static_cast<float>(dy);
    switch (mode) {
        case WPM_MOVE: {
            WeaponCall c = noCall();
            float dp[3];
            moveDp(fx, fy, zoomK(zoom, fine), dp);
            if (snap) {
                dp[1] = snapAccum(acc.a[0], dp[1], kSnapMetres);
                dp[2] = snapAccum(acc.a[1], dp[2], kSnapMetres);
            } else {
                acc.a[0] = acc.a[1] = 0.0f;
            }
            if (dp[1] != 0.0f || dp[2] != 0.0f) {
                c.kind = WK_WORLD;
                c.dp[0] = dp[0];
                c.dp[1] = dp[1];
                c.dp[2] = dp[2];
                st.c[st.n++] = c;
            }
            st.a = dp[1];
            st.b = dp[2];
            break;
        }
        case WPM_DEPTH: {
            float m = -fy * zoomK(zoom, fine);
            if (snap) m = snapAccum(acc.a[0], m, kSnapMetres);
            else acc.a[0] = 0.0f;
            if (m != 0.0f) {
                WeaponCall c = noCall();
                c.kind = WK_LOCAL;
                c.axis = 1;
                c.metres = m;
                st.c[st.n++] = c;
            }
            st.a = m;
            break;
        }
        case WPM_TILT: {
            float d = -fy * kRotDegPerPx * (fine ? kFineScale : 1.0f);
            if (snap) d = snapAccum(acc.a[0], d, kSnapDeg);
            else acc.a[0] = 0.0f;
            if (d != 0.0f) {
                WeaponCall c = noCall();
                c.kind = WK_LOCAL;
                c.axis = 0;
                c.deg = d;
                st.c[st.n++] = c;
            }
            st.a = d;
            break;
        }
        case WPM_ROLLTURN: {
            const float s = kRotDegPerPx * (fine ? kFineScale : 1.0f);
            float turn = fx * s, roll = -fy * s;
            if (snap) {
                turn = snapAccum(acc.a[0], turn, kSnapDeg);
                roll = snapAccum(acc.a[1], roll, kSnapDeg);
            } else {
                acc.a[0] = acc.a[1] = 0.0f;
            }
            if (turn != 0.0f) {
                WeaponCall c = noCall();
                c.kind = WK_LOCAL;
                c.axis = 2;
                c.deg = turn;
                st.c[st.n++] = c;
            }
            if (roll != 0.0f) {
                WeaponCall c = noCall();
                c.kind = WK_LOCAL;
                c.axis = 1;
                c.deg = roll;
                st.c[st.n++] = c;
            }
            st.a = roll;
            st.b = turn;
            break;
        }
        default: break;
    }
    return st;
}

// RMB + wheel: depth along bone axis 1, 2 cm per notch (wheel is 120 units per notch), Shift x0.2, Ctrl 1 cm steps (acc.a[2]). kind WK_NONE when
// nothing is applied this tick (zero wheel, or the snap remainder has not reached a step).
inline WeaponCall wheelDepth(int wheel, bool fine, bool snap, SnapAcc& acc) {
    WeaponCall c = noCall();
    float m = (static_cast<float>(wheel) / 120.0f) * kWheelDepthMetres * (fine ? kFineScale : 1.0f);
    if (snap) m = snapAccum(acc.a[2], m, kSnapMetres);
    else acc.a[2] = 0.0f;
    if (m != 0.0f) {
        c.kind = WK_LOCAL;
        c.axis = 1;
        c.metres = m;
    }
    return c;
}

// ---- U2 labels (pure text; the editor fits them with textfit::fitText) ----

// ">Move<" for the active mode, "Move" otherwise; always NUL terminated.
inline void modeLabel(char* out, int cap, int mode, bool active) {
    if (!out || cap <= 0) return;
    if (active) snprintf(out, cap, ">%s<", weaponModeName(mode));
    else snprintf(out, cap, "%s", weaponModeName(mode));
    out[cap - 1] = '\0';
}

// Plural of the GetEdit type for the scope button; an unknown or empty type has no category word.
inline const char* categoryPlural(const char* type) {
    if (!type || !type[0]) return "of this type";
    if (!strcmp(type, "lightsaber")) return "lightsabers";
    if (!strcmp(type, "melee")) return "melee weapons";
    if (!strcmp(type, "qstaff")) return "quarterstaffs";
    if (!strcmp(type, "pistol")) return "pistols";
    if (!strcmp(type, "rifle")) return "rifles";
    if (!strcmp(type, "heavy")) return "heavy weapons";
    return "of this type";
}

// Scope button text: 0 "For: this weapon", 1 "For: all <type plural>", 2 "For: this body"; any other scope reads as 0.
inline void scopeButtonText(char* out, int cap, int scope, const char* type) {
    if (!out || cap <= 0) return;
    if (scope == 1) snprintf(out, cap, "For: all %s", categoryPlural(type));
    else if (scope == 2) snprintf(out, cap, "For: this body");
    else snprintf(out, cap, "For: this weapon");
    out[cap - 1] = '\0';
}

// Hover text of the scope button (the mod-config description pane). ASCII, under 190 characters.
inline const char* scopeTip() {
    return "Where Save applies. This weapon: only this item model. All <type>: every weapon of this type. This body: every weapon on this character body.";
}

inline const char* helpLine() { return "LMB turn . RMB+LMB move weapon . RMB+wheel depth . Shift fine . Ctrl snap"; }

// Weapon button text for nav mode: "Right: Vibrosword" (shortForm: "R: Vibrosword"). The name is the item display name, else the model resref, else
// "Equipped" (a weapon is registered) or "None".
inline void handButtonText(char* out, int cap, int hand, bool shortForm, const char* display, const char* resref, bool registered) {
    if (!out || cap <= 0) return;
    const char* who = hand ? (shortForm ? "L" : "Left") : (shortForm ? "R" : "Right");
    const char* name = (display && display[0]) ? display : ((resref && resref[0]) ? resref : (registered ? "Equipped" : "None"));
    snprintf(out, cap, "%s: %s", who, name);
    out[cap - 1] = '\0';
}

// ---- U3 inspector rows (plan HW-UX2 U3; research H6): six scrub rows (a scrub button with the live value + a small reset button) ----
// Slide / Depth / Height are the offset x / y / z in the BONE frame (metres in the provider, shown in cm); Tilt / Roll / Turn are the
// rotation x / y / z (degrees). The provider's SetValues takes ABSOLUTE rot[3] and off[3], so every scrub tick sends all six values with
// only the scrubbed one changed. Typed input is deferred (needs a CSWGuiEditbox RE).

enum InspRow { IR_SLIDE = 0, IR_DEPTH, IR_HEIGHT, IR_TILT, IR_ROLL, IR_TURN, IR_COUNT };

constexpr int kInspItems = 2 * IR_COUNT;     // page items: one scrub button + one reset button per row
constexpr int kPageItemsBase = 26;           // items cbBuild creates before the inspector: view, status, 2 name/picker rows, hands, stance, emote, play,
                                             // bone, scope, save, revert, reset scope, done, reset/flip view, 4 mode buttons
constexpr int kPageItemCap = 40;             // mod-config mui::kMaxItems (0.1.1+): AddButton/AddLabel/AddCustom return -1 beyond it
static_assert(kPageItemsBase + kInspItems <= kPageItemCap, "the editor page (base items + 12 inspector items) must fit mod-config's per-page item cap");
constexpr int kStripRowsBase = 9;            // weapon mode strip rows without the inspector: name, hands, bone, scope, save/revert, reset scope/done, view, 2 mode rows
constexpr int kStripRowsInsp = kStripRowsBase + IR_COUNT;
constexpr int kScopeBodyId = 2;              // edit scope BODY (K2HOLSTER_SCOPE_BODY): the offset only, rotation is ignored

constexpr float kScrubOffPerPx = 0.001f;     // 1 px = 1 mm
constexpr float kScrubRotPerPx = 0.5f;       // 1 px = 0.5 deg
constexpr float kScrubSnapOff = 0.01f;       // Ctrl: 1 cm
constexpr float kScrubSnapRot = 5.0f;        // Ctrl: 5 deg
constexpr float kScrubWindowOff = 0.30f;     // a scrub moves a value at most this far from where the scrub began
constexpr float kScrubWindowRot = 180.0f;
constexpr float kScrubLimitOff = 3.0f;       // holster.ini parser limits (HolsterTune.h): |offset| <= 3 m, |rotation| <= 720 deg
constexpr float kScrubLimitRot = 720.0f;
constexpr float kInspDiffOff = 0.0005f;      // a row differs from its open value from half a display step on (0.05 cm / 0.05 deg)
constexpr float kInspDiffRot = 0.05f;
constexpr uint8_t kSrcMirrorBit = 8;         // K2HolsterEditInfo.src[0] / src[1] bit: the shown left value is the right one mirrored (reserved; holster has not set it yet)

inline const char* inspRowName(int r) {
    static const char* const kNames[IR_COUNT] = {"Slide", "Depth", "Height", "Tilt", "Roll", "Turn"};
    return (r >= 0 && r < IR_COUNT) ? kNames[r] : "?";
}
inline bool inspIsRot(int r) { return r >= IR_TILT; }
inline int inspAxis(int r) { return r % 3; }
inline const char* inspUnit(int r) { return inspIsRot(r) ? "deg" : "cm"; }

// Hover text of the scrub button (ASCII, under 190 characters).
inline const char* inspTip(int r) {
    static const char* const kTips[IR_COUNT] = {
        "Slide: the weapon's side-to-side offset on the bone. Hold the left button and drag sideways: 1 px = 1 mm. Shift is fine, Ctrl snaps to 1 cm.",
        "Depth: the weapon's forward (+) and back (-) offset on the bone. Hold the left button and drag sideways: 1 px = 1 mm. Shift is fine, Ctrl snaps to 1 cm.",
        "Height: the weapon's up (+) and down (-) offset on the bone. Hold the left button and drag sideways: 1 px = 1 mm. Shift is fine, Ctrl snaps to 1 cm.",
        "Tilt: rotation about the bone's side axis. Hold the left button and drag sideways: 1 px = 0.5 degrees. Shift is fine, Ctrl snaps to 5 degrees.",
        "Roll: rotation about the bone's forward axis. Hold the left button and drag sideways: 1 px = 0.5 degrees. Shift is fine, Ctrl snaps to 5 degrees.",
        "Turn: rotation about the bone's up axis. Hold the left button and drag sideways: 1 px = 0.5 degrees. Shift is fine, Ctrl snaps to 5 degrees.",
    };
    return (r >= 0 && r < IR_COUNT) ? kTips[r] : "";
}
inline const char* inspResetTip() { return "Put this value back to what it was when the edit opened (or after the last Save)."; }

// Body scope edits the offset only: its rotation rows have nothing to scrub.
inline bool inspRowEnabled(int row, int scope) { return !(scope == kScopeBodyId && inspIsRot(row)); }

// The value of a row in provider units (metres or degrees); NaN for a bad row.
inline float inspGet(int row, const float rot[3], const float off[3]) {
    if (row < 0 || row >= IR_COUNT || !rot || !off) return NAN;
    return inspIsRot(row) ? rot[inspAxis(row)] : off[inspAxis(row)];
}
inline void inspSet(int row, float rot[3], float off[3], float v) {
    if (row < 0 || row >= IR_COUNT || !rot || !off) return;
    if (inspIsRot(row)) rot[inspAxis(row)] = v;
    else off[inspAxis(row)] = v;
}

// A row differs from its open value (half a display step or more). Non-finite values never count as a difference.
inline bool inspDiffers(int row, float v, float open) {
    if (!finiteF(v) || !finiteF(open)) return false;
    return fabsf(v - open) >= (inspIsRot(row) ? kInspDiffRot : kInspDiffOff);
}

// A value read back from the provider is the value that was sent (rotations compare modulo 360: SetValues(450) reads back as 90).
inline bool inspMatches(int row, float got, float want) {
    if (!finiteF(got) || !finiteF(want)) return false;
    if (!inspIsRot(row)) return fabsf(got - want) < 1e-4f;
    float d = fmodf(got - want, 360.0f);
    if (d > 180.0f) d -= 360.0f;
    if (d < -180.0f) d += 360.0f;
    return fabsf(d) < 0.05f;
}

// "Depth  -11.0 cm" / "Tilt  30.5 deg"; a leading "* " marks a row that differs from its open value. A disabled row reads "Tilt  (body: n/a)".
// Never "-0.0". Always NUL terminated; null/cap<=0 is a no-op.
inline void inspRowText(char* out, int cap, int row, float value, bool changed, bool enabled) {
    if (!out || cap <= 0) return;
    const char* name = inspRowName(row);
    if (!enabled) {
        snprintf(out, cap, "%s  (body: n/a)", name);
    } else if (!finiteF(value)) {
        snprintf(out, cap, "%s  n/a", name);
    } else {
        float shown = inspIsRot(row) ? value : value * 100.0f;
        shown = roundf(shown * 10.0f) / 10.0f;
        if (shown == 0.0f) shown = 0.0f;   // folds -0.0 into +0.0
        snprintf(out, cap, "%s%s  %.1f %s", changed ? "* " : "", name, static_cast<double>(shown), inspUnit(row));
    }
    out[cap - 1] = '\0';
}

// Copies cur to out, then puts ONE row back to its open value (the other five stay as they are). True when that row actually changes.
inline bool inspRevert(int row, const float curRot[3], const float curOff[3], const float openRot[3], const float openOff[3], float outRot[3], float outOff[3]) {
    if (!curRot || !curOff || !openRot || !openOff || !outRot || !outOff) return false;
    for (int i = 0; i < 3; ++i) {
        outRot[i] = curRot[i];
        outOff[i] = curOff[i];
    }
    if (row < 0 || row >= IR_COUNT) return false;
    const float was = inspGet(row, outRot, outOff);
    const float to = inspGet(row, openRot, openOff);
    inspSet(row, outRot, outOff, to);
    return was != to;
}

// What SetText was last given per row: update() is true only when the text changed (the editor skips SetText otherwise).
struct TextCache {
    char t[IR_COUNT][48];
    TextCache() { clear(); }
    void clear() { memset(t, 0, sizeof t); }
    bool update(int i, const char* s) {
        if (i < 0 || i >= IR_COUNT || !s) return false;
        char cut[sizeof t[0]];
        snprintf(cut, sizeof cut, "%s", s);   // compared as stored, so an over-long text cannot look "changed" every tick
        if (!strcmp(t[i], cut)) return false;
        memcpy(t[i], cut, sizeof cut);
        return true;
    }
};

struct ScrubStep {
    bool changed;      // the value moved this tick
    float value;       // the scrubbed value now
    bool clampedNow;   // the FIRST clamp of this scrub (the editor WARNs once)
    int clampSide;     // -1 pinned at the low end, +1 at the high end, 0 not pinned this tick
    bool hardLimit;    // the end hit is the holster.ini limit (3 m / 720 deg), not the +-window
};

// One scrub of one row: begin() on the LMB press over the row button, step() once per tick with the cursor x while LMB is down, cancel() on
// release or any loss of the capture. 1 px = 1 mm or 0.5 deg, Shift x0.2, Ctrl snaps to 1 cm / 5 deg (carried in acc). The value moves by the
// per-tick delta (a Shift or Ctrl toggle mid drag never jumps) and stays inside [open - window, open + window] cut to the hard limits.
struct Scrub {
    bool active;
    int row;
    float open, value, lo, hi;
    bool loHard, hiHard;
    int pressX, lastX, frames;
    float acc;
    bool clampWarned;

    Scrub() : active(false), row(-1), open(0.0f), value(0.0f), lo(0.0f), hi(0.0f), loHard(false), hiHard(false), pressX(0), lastX(0), frames(0), acc(0.0f), clampWarned(false) {}

    // False (and nothing starts) for a bad row, a non-finite open value, or a scrub already running.
    bool begin(int r, int x, float openValue) {
        if (active || r < 0 || r >= IR_COUNT || !finiteF(openValue)) return false;
        const bool rot = inspIsRot(r);
        const float W = rot ? kScrubWindowRot : kScrubWindowOff;
        const float L = rot ? kScrubLimitRot : kScrubLimitOff;
        float l = openValue - W, h = openValue + W;
        loHard = l <= -L;
        hiHard = h >= L;
        if (l < -L) l = -L;
        if (l > L) { l = L; loHard = true; }
        if (h > L) h = L;
        if (h < -L) { h = -L; hiHard = true; }
        active = true;
        row = r;
        open = value = openValue;
        lo = l;
        hi = h;
        pressX = lastX = x;
        frames = 0;
        acc = 0.0f;
        clampWarned = false;
        return true;
    }

    ScrubStep step(int x, bool shift, bool ctrl) {
        ScrubStep s = {false, value, false, 0, false};
        if (!active) return s;
        const int dx = x - lastX;
        lastX = x;
        ++frames;
        const bool rot = inspIsRot(row);
        float d = static_cast<float>(dx) * (rot ? kScrubRotPerPx : kScrubOffPerPx) * (shift ? kFineScale : 1.0f);
        if (ctrl) {
            // float sums of 0.001 land a hair under a whole step; the tiny bias keeps "10 px = 1 cm" exact (negligible against the step)
            const float step = rot ? kScrubSnapRot : kScrubSnapOff;
            if (d != 0.0f) d += (d > 0.0f ? 1.0f : -1.0f) * step * 1e-5f;
            d = snapAccum(acc, d, step);
        } else {
            acc = 0.0f;
        }
        if (d == 0.0f || !finiteF(d)) return s;
        float nv = value + d;
        if (!finiteF(nv)) return s;
        if (nv < lo) {
            nv = lo;
            s.clampSide = -1;
            s.hardLimit = loHard;
        } else if (nv > hi) {
            nv = hi;
            s.clampSide = 1;
            s.hardLimit = hiHard;
        }
        if (s.clampSide != 0) {
            acc = 0.0f;
            if (!clampWarned) {
                clampWarned = true;
                s.clampedNow = true;
            }
        }
        if (nv != value) {
            value = nv;
            s.changed = true;
            s.value = nv;
        }
        return s;
    }

    void cancel() { *this = Scrub(); }
};

// Strip row `row` of `rows` equal rows that all fit in availH px below strip.y. The vanilla pitch (strip.h * 36 / 284) is kept while the rows fit,
// otherwise the pitch shrinks to availH / rows (slot height scales with it). With rows <= availH / pitch this is exactly stripSlot().
inline HseRect compactSlot(const HseRect& strip, int availH, int row, int rows) {
    const int basePitch = strip.h * 36 / 284;
    const int baseH = strip.h * 32 / 284;
    int pitch = basePitch;
    if (rows > 0 && availH > 0 && availH / rows < pitch) pitch = availH / rows;
    int h = basePitch > 0 ? baseH * pitch / basePitch : baseH;
    if (h < 1) h = 1;
    if (h > pitch && pitch > 0) h = pitch;
    return HseRect{strip.x, strip.y + row * pitch, strip.w, h};
}

// Splits an inspector slot into the scrub button (left, wide) and the reset button (right, at least 18 px or the slot height, never over a third of it).
inline void inspRects(const HseRect& slot, HseRect* scrub, HseRect* reset) {
    if (!scrub || !reset) return;
    int rw = slot.h < 18 ? 18 : slot.h;
    if (rw > slot.w / 3) rw = slot.w / 3;
    const int gap = 2;
    int sw = slot.w - rw - gap;
    if (sw < 0) sw = 0;
    *scrub = HseRect{slot.x, slot.y, sw, slot.h};
    *reset = HseRect{slot.x + slot.w - rw, slot.y, rw, slot.h};
}

// The left hand's shown values come from the right hand's mirror (K2HolsterEditInfo.src bit kSrcMirrorBit on rot or off).
inline bool leftMirrored(bool left, const uint8_t src[3]) {
    return left && src && ((src[0] | src[1]) & kSrcMirrorBit) != 0;
}

// Weapon mode status line at rest: an optional "* " (unsaved), the mirrored-left note, then the help line.
inline void idleStatusText(char* out, int cap, bool dirty, bool mirroredLeft) {
    if (!out || cap <= 0) return;
    snprintf(out, cap, "%s%s%s", dirty ? "* " : "", mirroredLeft ? "Left: mirrored from right . " : "", helpLine());
    out[cap - 1] = '\0';
}

} // namespace hse
