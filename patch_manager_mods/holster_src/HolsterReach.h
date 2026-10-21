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
inline bool solveTwoBone(const float* S, const float* E, const float* H, const float* T, float* Enew, float* reach) {
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(S[i]) || !std::isfinite(E[i]) || !std::isfinite(H[i]) || !std::isfinite(T[i])) return false;
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
    const float ad = r3dot(es, axis);
    float perp[3];
    for (int i = 0; i < 3; ++i) perp[i] = es[i] - ad * axis[i];
    const float lp = r3len(perp);
    if (lp > 1e-6f * len1) {
        for (int i = 0; i < 3; ++i) perp[i] /= lp;
    } else {
        r3anyPerp(axis, perp);
    }

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

// Local rotation that makes a bone's world orientation change by deltaWorld, given its parent's world orientation and its current
// local orientation: parent * newLocal = deltaWorld * parent * curLocal. A non-finite input leaves the bone where it is.
inline Q reachDeltaToLocal(const Q& parentWorld, const Q& deltaWorld, const Q& curLocal) {
    if (!qfinite(parentWorld) || !qfinite(deltaWorld) || !qfinite(curLocal)) return qfinite(curLocal) ? qnormalize(curLocal) : Q{ 1, 0, 0, 0 };
    return qnormalize(qmul(reachConj(parentWorld), qmul(deltaWorld, qmul(parentWorld, curLocal))));
}

// Reach ramp of a Slide-timed reach: 0 outside the slide, smoothstep ramps over the first and last 20% of its length, 1 in the
// middle 60%. Uses the signed tick difference, so a counter wrap is harmless.
inline float reachWeight(const Slide& s, uint32_t now) {
    if (!s.active(now)) return 0.f;
    const float p = (float)(int32_t)(now - s.t0) / (float)s.len;
    const float up = p / 0.2f, down = (1.f - p) / 0.2f;
    float w = up < down ? up : down;
    if (w > 1.f) w = 1.f;
    return easeSmooth(w);
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
