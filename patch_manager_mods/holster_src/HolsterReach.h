// HolsterReach.h -- reach math of the holstered-weapons mod (doc 42): the two-bone arm solve that places the elbow so the hand
// reaches the grip target, the quaternion helpers it needs (shortest arc, conjugate, world-delta to local), the reach ramp weight,
// the nlerp blend and the base-pose cache that lets the mod write a bone without losing the engine's sampled pose. Pure math,
// header-only, no windows.h, no engine calls. Every function returns finite output for any input. Host-tested by
// holster_reach_test.cpp.
#pragma once
#include <cmath>
#include <stdint.h>
#include "HolsterSlide.h"

namespace hs {

inline bool qfinite(const Q& q) {
    return std::isfinite(q.w) && std::isfinite(q.x) && std::isfinite(q.y) && std::isfinite(q.z);
}
inline float qdot(const Q& a, const Q& b) { return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z; }
inline Q reachConj(const Q& q) { return { q.w, -q.x, -q.y, -q.z }; }

// 3-vector helpers (world units, float[3]).
inline float r3len(const float* v) { return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
inline float r3dot(const float* a, const float* b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

// Unit vector perpendicular to the unit `axis`: the longer of axis x X and axis x Y (never degenerate for a unit axis).
inline void r3anyPerp(const float* axis, float* out) {
    const float cx[3] = { 0.f, axis[2], -axis[1] };   // axis x X
    const float cy[3] = { -axis[2], 0.f, axis[0] };   // axis x Y
    const bool useX = r3len(cx) >= r3len(cy);
    const float* c = useX ? cx : cy;
    const float n = r3len(c);
    for (int i = 0; i < 3; ++i) out[i] = c[i] / n;
}

// Shortest-arc rotation taking the direction `from` onto the direction `to` (qrotate(q, from) points along `to`). Either direction
// zero-length or non-finite: identity. Parallel: identity. Anti-parallel: 180 deg about a perpendicular axis.
inline Q arcQuat(const float* from, const float* to) {
    const float lf = r3len(from), lt = r3len(to);
    if (!(lf > 1e-6f) || !(lt > 1e-6f) || !std::isfinite(lf) || !std::isfinite(lt)) return { 1, 0, 0, 0 };
    const float f[3] = { from[0] / lf, from[1] / lf, from[2] / lf };
    const float t[3] = { to[0] / lt, to[1] / lt, to[2] / lt };
    const float d = r3dot(f, t);
    if (1.f + d < 1e-5f) {
        float a[3];
        r3anyPerp(f, a);
        return { 0.f, a[0], a[1], a[2] };  // 180 deg about a: f -> -f
    }
    // (1 + d, f x t) is 2 cos(th/2) (cos(th/2), sin(th/2) axis): the half-angle quaternion of the arc, unnormalized.
    return qnormalize({ 1.f + d, f[1] * t[2] - f[2] * t[1], f[2] * t[0] - f[0] * t[2], f[0] * t[1] - f[1] * t[0] });
}

// Two-bone solve. S shoulder, E elbow, H end effector (grip hook), T target, world 3-vectors. len1 = |E-S| and len2 = |H-E| come
// from the live pose. The target distance d = |T-S| is clamped to the reachable band [|len1-len2|+eps, len1+len2-eps]; the elbow
// is placed by the law of cosines in the plane of S, T and the current elbow (perp hint from E-S; any perpendicular when E-S is
// collinear with the axis). On success Enew satisfies |Enew-S| = len1 and |T'-Enew| = len2 for the clamped target T'. reach = 1
// when T was reachable without clamping, else d_clamped / d_target (1 when d_target is ~0). Returns false, leaving Enew and reach
// untouched, when any input is non-finite, a segment is shorter than 1e-4, or the result is not finite.
inline bool solveTwoBone(const float* S, const float* E, const float* H, const float* T, float* Enew, float* reach, const float* pole = nullptr) {
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(S[i]) || !std::isfinite(E[i]) || !std::isfinite(H[i]) || !std::isfinite(T[i])) return false;
    if (pole && !(std::isfinite(pole[0]) && std::isfinite(pole[1]) && std::isfinite(pole[2]))) pole = nullptr;
    float es[3], he[3], ts[3];
    for (int i = 0; i < 3; ++i) {
        es[i] = E[i] - S[i];
        he[i] = H[i] - E[i];
        ts[i] = T[i] - S[i];
    }
    const float len1 = r3len(es), len2 = r3len(he);
    if (!(len1 >= 1e-4f) || !(len2 >= 1e-4f)) return false;
    const float dT = r3len(ts);
    const float shorter = len1 < len2 ? len1 : len2;
    const float eps = 1e-4f * shorter;  // keeps lo < hi for every pair of bone lengths
    const float lo = std::fabs(len1 - len2) + eps, hi = len1 + len2 - eps;
    float d = dT;
    bool clamped = false;
    if (d < lo) { d = lo; clamped = true; }
    else if (d > hi) { d = hi; clamped = true; }

    // Axis: shoulder to target, or shoulder to elbow when the target sits on the shoulder.
    const bool degenerate = !(dT > 1e-6f);
    float axis[3];
    for (int i = 0; i < 3; ++i) axis[i] = degenerate ? es[i] / len1 : ts[i] / dT;

    // Elbow projection on the axis (law of cosines), clamped to the bone length, and its height off the axis.
    float a = (len1 * len1 - len2 * len2 + d * d) / (2.f * d);
    if (a > len1) a = len1;
    else if (a < -len1) a = -len1;
    const float h2 = len1 * len1 - a * a;
    const float h = std::sqrt(h2 > 0.f ? h2 : 0.f);

    // Side hint: the part of E-S across the axis keeps the elbow on its current side. Collinear: any perpendicular.
    // A caller pole (world direction the elbow should bend toward) replaces the current-elbow hint; a pole parallel to the axis falls back.
    float perp[3];
    bool havePerp = false;
    for (int pass = 0; pass < 2 && !havePerp; ++pass) {
        const float* hint = (pass == 0 && pole) ? pole : es;
        const float hl = r3len(hint);
        if (pass == 0 && !pole) continue;
        const float ad = r3dot(hint, axis);
        for (int i = 0; i < 3; ++i) perp[i] = hint[i] - ad * axis[i];
        const float lp = r3len(perp);
        if (lp > 1e-6f * hl && lp > 1e-9f) {
            for (int i = 0; i < 3; ++i) perp[i] /= lp;
            havePerp = true;
        }
    }
    if (!havePerp) r3anyPerp(axis, perp);

    float en[3];
    for (int i = 0; i < 3; ++i) en[i] = S[i] + axis[i] * a + perp[i] * h;
    float rr = 1.f;
    if (!degenerate && clamped) rr = d / dT;
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(en[i])) return false;
    if (!std::isfinite(rr)) return false;
    for (int i = 0; i < 3; ++i) Enew[i] = en[i];
    if (reach) *reach = rr;
    return true;
}

// ---- Elbow style: which way the elbow bends, by weapon category and where the weapon is mounted ----
// Directions are in the BODY frame of the owner (x right, y forward, z up) and are for the RIGHT arm; the left arm mirrors x. `hip`
// is the bend for a mount at or below the waist (draw back and out), `over` for a mount at or above the shoulder (elbow up and out,
// the hand goes up and over the shoulder to the hilt). The solver blends the two by how high the mount sits (reachOverWeight).
struct ReachStyle { float hip[3]; float over[3]; };

inline ReachStyle reachDefaultStyle(int cat) {
    switch (cat) {
        case 1: case 2: return { { 0.60f, -0.40f, -0.40f }, { 0.50f, 0.00f, 0.85f } };   // lightsaber, melee: sheath at the hip, hilt over the shoulder
        case 3:         return { { 0.60f, -0.40f, -0.40f }, { 0.60f, -0.20f, 0.70f } };  // quarterstaff
        case 4:         return { { 0.50f, -0.70f, -0.20f }, { 0.50f, 0.00f, 0.80f } };   // pistol: straight back from the holster
        default:        return { { 0.60f, -0.50f, -0.30f }, { 0.70f, -0.10f, 0.65f } };  // rifle, heavy, unknown
    }
}

// 0 = hip style, 1 = over-the-shoulder style. `dz` = height of the mount above the shoulder (body frame), `arm` = shoulder-to-hand reach.
// Smooth between -0.75 and -0.45 arm lengths below the shoulder (a hip mount sits ~0.8 or more below, an upper-back mount ~0.1 below, a mid-back
// rifle ~0.5); the caller forces 1 for a back bone, this is the fallback for a custom mount height.
inline float reachOverWeight(float dz, float arm) {
    if (!std::isfinite(dz) || !(arm > 1e-4f)) return 0.f;
    return easeSmooth((dz / arm + 0.75f) / 0.3f);
}

// World-space pole for the elbow: blends the style's hip / over directions by u, mirrors x for the left arm, rotates by the body
// orientation, then blends toward the animation's own elbow direction `anim` (world, any length, may be null) by animMix in [0,1]
// (1 = the animation's elbow plane, used as the weapon reaches the hand so the plane does not pop). Output is a unit vector, or
// false when it cannot be formed.
inline bool reachPole(const ReachStyle& st, float u, bool left, const Q& bodyQ, const float* anim, float animMix, float* out) {
    if (!qfinite(bodyQ)) return false;
    u = slideClamp01(u);
    animMix = slideClamp01(animMix);
    float loc[3];
    for (int i = 0; i < 3; ++i) loc[i] = (1.f - u) * st.hip[i] + u * st.over[i];
    if (left) loc[0] = -loc[0];
    float w[3];
    qrotate(qnormalize(bodyQ), loc, w);
    const float lw = r3len(w);
    if (!(lw > 1e-6f)) return false;
    for (int i = 0; i < 3; ++i) w[i] /= lw;
    if (anim && animMix > 0.f) {
        const float la = r3len(anim);
        if (la > 1e-6f && std::isfinite(la))
            for (int i = 0; i < 3; ++i) w[i] = (1.f - animMix) * w[i] + animMix * anim[i] / la;
    }
    const float l = r3len(w);
    if (!(l > 1e-6f) || !std::isfinite(l)) return false;
    for (int i = 0; i < 3; ++i) out[i] = w[i] / l;
    return true;
}

// Local rotation that makes a bone's world orientation change by deltaWorld, given its parent's world orientation and its current
// local orientation: parent * newLocal = deltaWorld * parent * curLocal. A non-finite input leaves the bone where it is.
inline Q reachDeltaToLocal(const Q& parentWorld, const Q& deltaWorld, const Q& curLocal) {
    if (!qfinite(parentWorld) || !qfinite(deltaWorld) || !qfinite(curLocal)) return qfinite(curLocal) ? qnormalize(curLocal) : Q{ 1, 0, 0, 0 };
    return qnormalize(qmul(reachConj(parentWorld), qmul(deltaWorld, qmul(parentWorld, curLocal))));
}

// Reach ramp of a Slide-timed reach: 0 outside the slide, smoothstep ramps over the first and last 20% of its length, 1 in the
// middle 60%. Uses the signed tick difference, so a counter wrap is harmless.
inline float reachWeightRamp(const Slide& s, uint32_t now, float ramp) {
    if (!s.active(now) || !(ramp > 1e-3f)) return 0.f;
    if (ramp > 0.5f) ramp = 0.5f;
    const float p = (float)(int32_t)(now - s.t0) / (float)s.len;
    const float up = p / ramp, down = (1.f - p) / ramp;
    float w = up < down ? up : down;
    if (w > 1.f) w = 1.f;
    return easeSmooth(w);
}
inline float reachWeight(const Slide& s, uint32_t now) { return reachWeightRamp(s, now, 0.2f); }

// How much of the animation's own elbow plane is mixed into the pole at hand fraction f (0 = mount, 1 = hand): none up to 0.8, then eased to 1 at the
// hand. The mount style therefore holds for the whole swing in BOTH directions (draw and stow), so a sword drawn over the shoulder is also stowed over it.
inline float reachAnimMix(float f) { return easeSmooth((f - 0.8f) / 0.2f); }

// Hand path of the carried weapon: from the mount point to the animation's hand along a straight line, bulged by an arc so a high mount is
// reached up and back, then swung up, forward and down to the hand (instead of dragged straight down through the body). `up` and `fwd` are unit
// body axes (either may be null). bulge = arm * over * sin(pi f) * (0.15 up + 0.45 fwd), over in 0..1. f is eased; mount at f = 0, hand at f = 1.
// Finite for any finite input.
inline void reachCarryTarget(const float* mount, const float* hand, const float* up, const float* fwd, float over, float arm, float f, float* out) {
    const float e = easeSmooth(f);
    float bu = 0.f, bf = 0.f;
    if (std::isfinite(arm) && std::isfinite(over) && arm > 0.f) {
        const float k = arm * slideClamp01(over) * std::sin(3.14159265358979f * slideClamp01(f));
        bu = 0.15f * k;
        bf = 0.45f * k;
    }
    for (int i = 0; i < 3; ++i) out[i] = (1.f - e) * mount[i] + e * hand[i] + (up ? bu * up[i] : 0.f) + (fwd ? bf * fwd[i] : 0.f);
}

// ---- Phases of a carried draw / stow ----
// Draw (dir 1) from the mount: REACH (the first `g` of the slide: the hand goes from its resting position to the mount, the weapon stays on the mount),
// then CARRY (the hand brings the weapon along the path, mount -> rest). Draw from a partly drawn weapon: carry only.
// Stow (dir 0): CARRY (rest -> mount, the weapon lands on the mount at the end of it), then RELEASE (the last `g`: the hand goes from the mount back to rest,
// the weapon stays). pathF = position on the carry path (0 mount .. 1 rest); restMix = how far the aimed point has been pulled toward the resting hand
// (1 at the slide ends in the reach / release phases); carryW = how much the weapon follows the hand (0 = on the mount).
struct ReachPlan { bool active; float pathF, restMix, carryW; };

inline ReachPlan reachPlan(const Slide& s, uint32_t now, float g) {
    ReachPlan r = { false, 0.f, 0.f, 0.f };
    if (!s.active(now)) return r;
    if (!(g > 0.f)) g = 0.f;
    if (g > 0.6f) g = 0.6f;
    const float p = slideClamp01((float)(int32_t)(now - s.t0) / (float)s.len);
    const float from = slideClamp01(s.from);
    r.active = true;
    if (s.dir) {                                   // draw
        if (from <= 0.001f && g > 0.f) {
            if (p < g) { r.pathF = 0.f; r.restMix = 1.f - easeSmooth(p / g); r.carryW = 0.f; return r; }
            const float q = (p - g) / (1.f - g);
            r.pathF = q; r.restMix = 0.f; r.carryW = easeSmooth(q / 0.1f);
            return r;
        }
        r.pathF = from + (1.f - from) * p; r.restMix = 0.f; r.carryW = 1.f;
        return r;
    }
    const float c = 1.f - g;                       // stow: carry share of the slide
    if (p < c) {
        const float q = p / c;
        r.pathF = from * (1.f - q); r.restMix = 0.f; r.carryW = easeSmooth(r.pathF / 0.1f);
        return r;
    }
    r.pathF = 0.f; r.restMix = easeSmooth((p - c) / g); r.carryW = 0.f;
    return r;
}

// Nlerp along the shorter arc. w is clamped to [0, 1] (NaN -> 0). w == 0 returns a bit for bit (no normalize). A non-finite a is
// taken as identity; a non-finite b keeps a.
inline Q blendQ(const Q& a0, const Q& b0, float w) {
    const Q a = qfinite(a0) ? a0 : Q{ 1, 0, 0, 0 };
    if (!qfinite(b0) || !(w > 0.f)) return a;
    if (w > 1.f) w = 1.f;
    Q b = b0;
    if (qdot(a, b) < 0.f) { b.w = -b.w; b.x = -b.x; b.y = -b.y; b.z = -b.z; }
    return qnormalize({ (1.f - w) * a.w + w * b.w, (1.f - w) * a.x + w * b.x, (1.f - w) * a.y + w * b.y, (1.f - w) * a.z + w * b.z });
}

// Cache of a bone's sampled base orientation. The engine resamples the bone each frame; when it did not (the read still matches
// what the mod last wrote), the cached base is returned so the mod does not compound its own write.
struct Base {
    Q q{ 1, 0, 0, 0 };      // last base orientation sampled from the engine
    Q wrote{ 1, 0, 0, 0 };  // last orientation the mod wrote to the bone
    bool have = false;
};

inline Q reachBase(Base& b, const Q& read) {
    if (!qfinite(read)) return b.q;
    if (b.have && std::fabs(qdot(read, b.wrote)) > 0.99999f) return b.q;
    b.q = qnormalize(read);
    b.have = true;
    return b.q;
}
inline void reachNoteWrite(Base& b, const Q& w) { b.wrote = w; }

}  // namespace hs
