// HolsterReach.h -- reach math of the holstered-weapons mod (doc 42): the two-bone arm solve that places the elbow so the hand
// reaches the grip target, the quaternion helpers it needs (shortest arc, conjugate, world-delta to local), the reach ramp weight,
// the nlerp blend and the base-pose cache that lets the mod write a bone without losing the engine's sampled pose. Pure math,
// header-only, no windows.h, no engine calls. Every function returns finite output for any input. Host-tested by
// holster_reach_test.cpp.
#pragma once
#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
// `bulge` = how far the hand path bows away from the straight mount -> hand line at its middle, in arm lengths, for a high mount: { up, forward, outward }
// (outward = away from the body's midline on the arm's own side). Swords swing up and forward; heavy weapons go straight up and out so they clear the head.
struct ReachStyle { float hip[3]; float over[3]; float bulge[3]; };

inline ReachStyle reachDefaultStyle(int cat) {
    switch (cat) {
        case 1: case 2: return { { 0.60f, -0.40f, -0.40f }, { 0.50f, 0.00f, 0.85f }, { 0.20f, 0.40f, 0.10f } };   // lightsaber, melee: sheath at the hip, hilt over the shoulder
        case 3:         return { { 0.60f, -0.40f, -0.40f }, { 0.60f, -0.20f, 0.70f }, { 0.40f, 0.20f, 0.20f } };   // quarterstaff
        case 4:         return { { 0.50f, -0.70f, -0.20f }, { 0.50f, 0.00f, 0.80f }, { 0.20f, 0.40f, 0.10f } };    // pistol: straight back from the holster
        default:        return { { 0.60f, -0.50f, -0.30f }, { 0.70f, -0.10f, 0.65f }, { 0.60f, 0.10f, 0.25f } };   // rifle, heavy, unknown: straight up and over, away from the head
    }
}

// 0 = hip style, 1 = over-the-shoulder style. `dz` = height of the mount above the shoulder (body frame), `arm` = shoulder-to-hand reach.
// Smooth between -0.60 and -0.38 arm lengths below the shoulder: pelvis_g sits at -0.54 (u about 0.18, so a pelvis pistol or lightsaber keeps the hip pole),
// a mid-back rifle at -0.50 gives u about 0.43, and an upper-back mount at -0.36 or higher is fully over (u 1). The caller forces 1 for a back bone; this is the
// fallback for a custom mount height.
inline float reachOverWeight(float dz, float arm) {
    if (!std::isfinite(dz) || !(arm > 1e-4f)) return 0.f;
    return easeSmooth((dz / arm + 0.60f) / 0.22f);
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

// HW29b: bias the elbow pole toward `grip` (unit world direction the caller derives from the target hand orientation) early in the
// reach only. weight = amount * (1 - easeSmooth(f / kPoleBiasEndF)); the pole turns toward grip by min(angle, maxDeg) * weight degrees
// about cross(poleIn, grip). amount <= 0, f >= kPoleBiasEndF, maxDeg <= 0, or any non-finite input -> out = poleIn bit for bit.
constexpr float kPoleBiasEndF = 0.30f;
constexpr float kPoleBiasDeg = 20.f;     // ini reach_pole_bias_deg
inline void reachPoleBias(const float* poleIn, const float* grip, float f, float amount, float maxDeg, float* out) {
    float r[3] = { poleIn[0], poleIn[1], poleIn[2] };   // the unchanged pole
    const float a = slideClamp01(amount);
    const bool allFinite = std::isfinite(f) && std::isfinite(amount) && std::isfinite(maxDeg) && std::isfinite(poleIn[0]) && std::isfinite(poleIn[1]) &&
                           std::isfinite(poleIn[2]) && std::isfinite(grip[0]) && std::isfinite(grip[1]) && std::isfinite(grip[2]);
    const float lp = r3len(poleIn), lg = r3len(grip);
    if (allFinite && a > 0.f && maxDeg > 0.f && f < kPoleBiasEndF && lp > 1e-6f && lg > 1e-6f) {
        float p[3], g[3];
        for (int i = 0; i < 3; ++i) { p[i] = poleIn[i] / lp; g[i] = grip[i] / lg; }
        // cross(p, g) = sin(angle) * axis; a parallel or antiparallel grip has nothing to turn toward
        const float c[3] = { p[1] * g[2] - p[2] * g[1], p[2] * g[0] - p[0] * g[2], p[0] * g[1] - p[1] * g[0] };
        const float sc = r3len(c);
        if (sc >= 1e-6f) {
            const float angle = std::atan2(sc, r3dot(p, g)) * 57.2957795f;   // degrees, 0..180
            const float weight = a * (1.f - easeSmooth(f / kPoleBiasEndF));
            const float turn = (angle < maxDeg ? angle : maxDeg) * weight;
            const float axis[3] = { c[0] / sc, c[1] / sc, c[2] / sc };
            float rot[3];
            qrotate(qaxis(axis, turn), p, rot);
            const float lr = r3len(rot);
            if (lr > 1e-6f && std::isfinite(lr)) for (int i = 0; i < 3; ++i) r[i] = rot[i] / lr;
        }
    }
    for (int i = 0; i < 3; ++i) out[i] = r[i];
}

// Local rotation that makes a bone's world orientation change by deltaWorld, given its parent's world orientation and its current
// local orientation: parent * newLocal = deltaWorld * parent * curLocal. A non-finite input leaves the bone where it is.
inline Q reachDeltaToLocal(const Q& parentWorld, const Q& deltaWorld, const Q& curLocal) {
    if (!qfinite(parentWorld) || !qfinite(deltaWorld) || !qfinite(curLocal)) return qfinite(curLocal) ? qnormalize(curLocal) : Q{ 1, 0, 0, 0 };
    return qnormalize(qmul(reachConj(parentWorld), qmul(deltaWorld, qmul(parentWorld, curLocal))));
}

// Reach ramp of a Slide-timed reach: 0 outside the slide, smoothstep ramps over the first and last 20% of its length, 1 in the
// middle 60%. Uses the signed tick difference, so a counter wrap is harmless.
// The first ramp window starts at the reach weight the slide was retargeted from (w0) and joins the normal ramp at w = 1, so a reversal does not pop.
inline float reachWeightRamp(const Slide& s, uint32_t now, float ramp) {
    const int32_t d = (int32_t)(now - s.t0);
    if (d < 0 || d >= (int32_t)s.len || !(ramp > 1e-3f)) return 0.f;   // len 0: always 0
    if (ramp > 0.5f) ramp = 0.5f;
    const float p = (float)d / (float)s.len;
    const float up = p / ramp, down = (1.f - p) / ramp;
    float w = up < down ? up : down;
    if (w > 1.f) w = 1.f;
    const float g = easeSmooth(w);
    if (up < 1.f) return 1.f - (1.f - s.w0) * (1.f - g);   // retarget window: starts at w0, joins the ramp at w = 1
    return g;
}
inline float reachWeight(const Slide& s, uint32_t now) { return reachWeightRamp(s, now, 0.2f); }

// How much of the animation's own elbow plane is mixed into the pole at path position f (0 = mount, 1 = rest hand): none up to 0.45, then eased to 1 at the
// rest hand, so the elbow comes down out of the over-the-shoulder pose as the hand comes forward and down (and goes up again on the way back).
inline float reachAnimMix(float f) { return easeSmooth((f - 0.45f) / 0.5f); }

// Hand path of the carried weapon: from the mount point to the animation's hand along a straight line, bowed by `bulge` = { up, forward, outward } (arm lengths,
// see ReachStyle) so a high mount is reached up and back, then swung up, out and forward and down to the hand (instead of dragged straight down through the body or
// over the head). `up`, `fwd`, `out` are unit world axes (any may be null; `out` points away from the body's midline on this arm's side). The bow is
// arm * over * sin(pi f) * bulge. f is eased; mount at f = 0, hand at f = 1. Finite for any finite input.
inline void reachCarryTarget(const float* mount, const float* hand, const float* up, const float* fwd, const float* out3, const float* bulge, float over, float arm, float f, float* out) {
    const float e = easeSmooth(f);
    float k = 0.f;
    if (std::isfinite(arm) && std::isfinite(over) && arm > 0.f) k = arm * slideClamp01(over) * std::sin(3.14159265358979f * slideClamp01(f));
    const float bu = bulge ? k * bulge[0] : 0.f, bf = bulge ? k * bulge[1] : 0.f, bo = bulge ? k * bulge[2] : 0.f;
    for (int i = 0; i < 3; ++i)
        out[i] = (1.f - e) * mount[i] + e * hand[i] + (up ? bu * up[i] : 0.f) + (fwd ? bf * fwd[i] : 0.f) + (out3 ? bo * out3[i] : 0.f);
}

// ---- Hinged back-mount reach (HW23) ----
// A back mount is reached in three stages instead of one bowed line (the line folds the elbow across the chest). Path position f runs mount (0) -> rest hand (1):
//   f in [0, kHingeS3]       STAGE 3 (grab): the upper arm is PARKED horizontal and forward, the elbow pinned out in front of the shoulder (pole forward), and the forearm
//                            closes backward from vertical over the shoulder until the hand is on the mount. T sweeps an arc around the parked elbow E = S + L1 fwd.
//   f in [kHingeS3, kHingeS2] STAGE 2: the elbow stays locked at 90 deg while the shoulder pitches the whole arm from forward-horizontal (forearm up) down to hanging (forearm
//                            forward): T = S + L1 u(th) + L2 v(th) with u = -up cos th + fwd sin th, v = fwd cos th + up sin th, th 90 -> 0 deg; the pole turns with the upper arm.
//   f in [kHingeS2, 1]       STAGE 1: the L-shaped ready pose (upper arm down, forearm forward, elbow tucked) settles into the animation's resting hand and elbow plane.
// Axes are unit world vectors (`out` points away from the body's midline on this arm's side). `restPole` = the animation's own elbow direction (any length). T and pole
// are for solveTwoBone. Finite for any finite input; false when an input is non-finite or a segment is shorter than 1e-4.
// `outT` / `outPole` (defaults kHingeOutT / kHingeOutPole) set the outward offset of the hand and the outward share of the parked pole; per-category values
// and config keys are deferred to the caller wiring card.
constexpr float kHingeS3 = 0.30f, kHingeS2 = 0.80f;
constexpr float kHingeOutPole = 0.25f;   // outward share of the parked pole (the elbow stays out of the head's way)
constexpr float kHingeOutT = 0.10f;      // outward offset of the raised hand at the top of the pitch, in forearm lengths
constexpr float kHingeOutPoleRifle = 0.25f, kHingeOutPoleHeavy = 0.45f;   // interim per-category outward pole shares (check in play; caller wiring and config keys come in a later card)
constexpr float kHingeMinOut = 0.35f;   // stage 3: smallest outward share of the elbow bend direction (keeps the elbow off the face)
constexpr float kHingeLiftPole = 1.0f, kHingeLiftOut = 0.5f;   // stage 3: up / outward share added to the pole at the mount (the elbow rises when the mount is out of the parked reach)

inline float hingeEase(float x) {        // half linear, half smoothstep: a soft stop at the stage joints, not a robotic one
    x = slideClamp01(x);
    return 0.5f * x + 0.5f * easeSmooth(x);
}

inline void hingeNlerp(const float* a, const float* b, float x, float* out) {
    float v[3], l;
    for (int i = 0; i < 3; ++i) v[i] = (1.f - x) * a[i] + x * b[i];
    l = r3len(v);
    if (!(l > 1e-6f)) { for (int i = 0; i < 3; ++i) v[i] = b[i]; l = r3len(v); }
    if (!(l > 1e-9f)) { for (int i = 0; i < 3; ++i) out[i] = a[i]; return; }
    for (int i = 0; i < 3; ++i) out[i] = v[i] / l;
}

// The hinged path suits a mount at about shoulder height or higher (the hand goes over the shoulder); a lower back mount (rifle slung at mid back) would make the elbow
// flip from up to down as the hand passes the shoulder, so it keeps the bowed line. dz = mount height above the shoulder (body frame), arm = shoulder-to-hand reach.
constexpr float kHingeMinDz = -0.25f;   // in arm lengths
inline bool reachHingeFits(float dz, float arm) { return std::isfinite(dz) && std::isfinite(arm) && arm > 1e-4f && dz >= kHingeMinDz * arm; }

// ---- Back-mount reach topology (HW27) ----
// Which reach a mount takes. dzArm = mount height above the shoulder in arm lengths, hingeFits = reachHingeFits(dz, arm). First match wins: below kThighDz ->
// Thigh (the hip pole for now; the drop-and-piston path is deferred); hingeFits -> HingeBack; at or above kOverDz -> OverBack; otherwise Hip. A non-finite dzArm is Hip.
// There is no fy / fx input: torso_g and pelvis_g differ by only 0.01-0.02 arm in height and side offset, so a LowBack class would need a bone-name hint (the
// signature is fixed here); both resolve to Hip, and LowBack is not in the enum.
// Interim thresholds, check in play: kThighDz sits between pelvis_g (-0.54) and rthigh_g (-0.77), midpoint -0.655; kOverDz is the old over-the-shoulder band top.
constexpr float kThighDz = -0.66f;   // in arm lengths
constexpr float kOverDz = -0.45f;    // in arm lengths
enum class ReachTopo { HingeBack, OverBack, Hip, Thigh };
inline ReachTopo reachTopology(float dzArm, bool hingeFits) {
    if (!std::isfinite(dzArm)) return ReachTopo::Hip;
    if (dzArm < kThighDz) return ReachTopo::Thigh;
    if (hingeFits) return ReachTopo::HingeBack;
    if (dzArm >= kOverDz) return ReachTopo::OverBack;
    return ReachTopo::Hip;
}

// HW46: the slide's topology latch. topoSet 0 = not latched this slide: the live topo is stored as topoSet = 1 + enum (nonzero = latched, so the topoSet test in
// reachExtractPct is unchanged). Later calls return the stored topo, so u, the hinge path, the pole and the log use one topo per slide. topoBack = the back bit
// reachExtractPct reads. The three clear sites in Holster.cpp reset topoSet to 0 at slide end.
inline ReachTopo reachTopoLatch(ReachTopo liveTopo, uint8_t& topoSet, uint8_t& topoBack) {
    if (!topoSet) topoSet = static_cast<uint8_t>(1 + static_cast<int>(liveTopo));
    const ReachTopo t = static_cast<ReachTopo>(topoSet - 1);
    topoBack = (t == ReachTopo::HingeBack || t == ReachTopo::OverBack) ? 1 : 0;
    return t;
}

inline bool reachHinge(const float* S, float L1, float L2, const float* up, const float* fwd, const float* out3, const float* mount, const float* rest, const float* restPole,
                       float f, float* T, float* pole, float outT = kHingeOutT, float outPole = kHingeOutPole) {
    if (!(L1 > 1e-4f) || !(L2 > 1e-4f) || !std::isfinite(L1) || !std::isfinite(L2) || !std::isfinite(f)) return false;
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(S[i]) || !std::isfinite(up[i]) || !std::isfinite(fwd[i]) || !std::isfinite(out3[i]) || !std::isfinite(mount[i]) || !std::isfinite(rest[i]) || !std::isfinite(restPole[i])) return false;
    f = slideClamp01(f);
    float poleReady[3], poleHigh[3], tReady[3], ePark[3];
    for (int i = 0; i < 3; ++i) {
        poleReady[i] = -0.3f * fwd[i] - up[i];
        poleHigh[i] = fwd[i] + outPole * out3[i];
        tReady[i] = S[i] + L2 * fwd[i] - L1 * up[i];
        ePark[i] = S[i] + L1 * fwd[i];
    }
    float tv[3], pv[3];
    if (f <= kHingeS3) {   // stage 3: forearm closes about the parked elbow, x = 0 at the top, 1 at the mount
        const float x = 1.f - hingeEase(f / kHingeS3);
        float dA[3], dB[3], d[3], m[3], rB, r;
        for (int i = 0; i < 3; ++i) { dA[i] = up[i]; m[i] = mount[i] - ePark[i]; }
        rB = r3len(m);
        if (rB > 1e-6f) for (int i = 0; i < 3; ++i) dB[i] = m[i] / rB;
        else { for (int i = 0; i < 3; ++i) dB[i] = dA[i]; rB = L2; }
        hingeNlerp(dA, dB, x, d);
        r = (1.f - x) * L2 + x * rB;
        for (int i = 0; i < 3; ++i) tv[i] = ePark[i] + r * d[i] + (1.f - x) * outT * L2 * out3[i];
        // a mount behind the shoulder is past the parked forearm's reach: the elbow lifts up and out (never inboard across the face) as the hand closes on it
        float pr[3];
        for (int i = 0; i < 3; ++i) pr[i] = poleHigh[i] + x * (kHingeLiftPole * up[i] + kHingeLiftOut * out3[i]);
        // the solver bends the elbow toward the pole's part across the shoulder -> hand axis: for a central or low mount that part can point inboard (the elbow
        // would cross the face), so it is pushed outward until its outward share is at least kHingeMinOut of its length (fully at the mount)
        float ax[3];
        for (int i = 0; i < 3; ++i) ax[i] = tv[i] - S[i];
        const float al = r3len(ax);
        if (al > 1e-6f) {
            for (int i = 0; i < 3; ++i) ax[i] /= al;
            float pp[3], op[3];
            const float pa = r3dot(pr, ax), oa = r3dot(out3, ax);
            for (int i = 0; i < 3; ++i) { pp[i] = pr[i] - pa * ax[i]; op[i] = out3[i] - oa * ax[i]; }
            const float ol = r3len(op);
            if (ol > 1e-4f) {
                for (int i = 0; i < 3; ++i) op[i] /= ol;
                const float need = kHingeMinOut * r3len(pp) - r3dot(pp, op);
                if (need > 0.f) for (int i = 0; i < 3; ++i) pr[i] += x * need * op[i];   // faded in with the close (none at the top: no step at the stage joint)
            }
        }
        const float pl = r3len(pr);
        for (int i = 0; i < 3; ++i) pv[i] = pl > 1e-9f ? pr[i] / pl : poleHigh[i];
    } else if (f <= kHingeS2) {   // stage 2: elbow locked at 90 deg, the whole arm pitches from forward (x = 1) down to hanging (x = 0)
        const float x = 1.f - hingeEase((f - kHingeS3) / (kHingeS2 - kHingeS3));
        const float th = x * 1.57079632679f, c = std::cos(th), s = std::sin(th);
        for (int i = 0; i < 3; ++i) tv[i] = S[i] + L1 * (-up[i] * c + fwd[i] * s) + L2 * (fwd[i] * c + up[i] * s) + x * outT * L2 * out3[i];
        hingeNlerp(poleReady, poleHigh, x, pv);
    } else {   // stage 1: the ready pose settles into the animation's hand and elbow plane
        const float x = hingeEase((f - kHingeS2) / (1.f - kHingeS2));
        for (int i = 0; i < 3; ++i) tv[i] = (1.f - x) * tReady[i] + x * rest[i];
        float rp[3] = { restPole[0], restPole[1], restPole[2] };
        if (!(r3len(rp) > 1e-6f)) for (int i = 0; i < 3; ++i) rp[i] = poleReady[i];
        float prn[3];
        hingeNlerp(poleReady, rp, x, prn);
        for (int i = 0; i < 3; ++i) pv[i] = prn[i];
    }
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(tv[i]) || !std::isfinite(pv[i])) return false;
    for (int i = 0; i < 3; ++i) { T[i] = tv[i]; pole[i] = pv[i]; }
    return true;
}

// ---- Phases of a carried draw / stow ----
// Draw (dir 1) from the mount: REACH (the first `g` of the slide: the hand goes from its resting position to the mount ALONG THE CARRY ARC, the weapon stays on the mount),
// then CARRY (the hand brings the weapon along the path, mount -> rest). Draw from a partly drawn weapon: carry only.
// Stow (dir 0): CARRY (rest -> mount, the weapon lands on the mount at the end of it), then RELEASE (the last `g`: the hand goes from the mount back to rest,
// the weapon stays). pathF = position on the carry path (0 mount .. 1 rest); restMix = how far the aimed point has been pulled toward the resting hand
// (kept for callers; 0 now: the reach and release walk the carry arc); carryW = how much the weapon follows the hand (0 = on the mount).
struct ReachPlan { bool active; float pathF, restMix, carryW; float extractK = 0.f; float oriF = 0.f; };

constexpr float kCarryEaseLen = 0.15f;   // L: share of the carry over which the carry weight eases in/out
// Carry path position after carry fraction q (0..1 -> 0..1). Zero speed at both ends; slope v = 1/(1-L/2) in the middle.
inline float reachCarryF(float q) {
    const float L = kCarryEaseLen, v = 1.f / (1.f - L / 2.f);
    q = slideClamp01(q);
    if (q <= L) { const float x = q / L; return v * L * (x * x * x - x * x * x * x / 2.f); }
    return v * (L / 2.f + q - L);
}
// reach_ease_soft_<cat> (0.21.4-dev, HW50): soft = smoothstep (easeSmooth) instead of reachCarryF, so the hand settles at the end of a draw; false = reachCarryF byte for byte.
inline float reachCarryEase(float q, bool soft) { return soft ? easeSmooth(q) : reachCarryF(q); }

// HW31 extract bump: 16 x^2 (1 - x)^2 on x in [0, 1] (0 at both ends, 1 at the middle); x is clamped.
inline float extractBump(float x) { x = slideClamp01(x); return 16.f * x * x * (1.f - x) * (1.f - x); }

// HW31 orientation progress (D2), applied only inside the carry window (reachPlan uses oriF = pathF in reach and release).
// e = clamped extract share. e == 0 returns pathF (today's behaviour). In the carry: 0 while pathF <= X (the extract window's
// path end), then smooth 0 -> 1 to the hand end.
inline float reachOriF(float pathF, float e, bool soft = false) {
    if (!(e > 0.f)) return pathF;
    const float X = reachCarryEase(e, soft);   // pathF at the end of the extract window
    if (pathF <= X) return 0.f;                // mount orientation held through the extract
    return easeSmooth((pathF - X) / (1.f - X));
}

// The plan starts from the slide's stored retarget state (pathFrom, carryFrom), so a reversal continues where the arm was. The retarget tick
// (d == 0) is active.
inline ReachPlan reachPlan(const Slide& s, uint32_t now, float g, float extractPct = 0.f, bool soft = false) {
    ReachPlan r = { false, 0.f, 0.f, 0.f, 0.f, 0.f };
    const int32_t d = (int32_t)(now - s.t0);
    if (d < 0 || d >= (int32_t)s.len) return r;   // was s.active(now): the retarget tick (d == 0) is inside now
    if (!(g > 0.f)) g = 0.f;
    if (g > 0.6f) g = 0.6f;
    float e = extractPct;                          // D1: NaN and negatives -> 0, at most 1 - g - 0.05
    if (!(e > 0.f)) e = 0.f;
    if (e > 1.f - g - 0.05f) e = 1.f - g - 0.05f;
    const float p = slideClamp01((float)d / (float)s.len);
    const float from = slideClamp01(s.from);      // frac: only the fresh / partial split uses it
    r.active = true;
    float bump = 0.f;                              // D3 fresh extract bump (time-based)
    bool carry = true;                             // true inside the carry window; reach and release keep oriF = pathF
    if (s.dir) {                                   // draw
        if (from <= 0.001f) {                      // fresh from the mount: reach if g > 0, then carry from 0
            if (g > 0.f && p < g) {
                r.pathF = s.pathFrom * (1.f - easeSmooth(p / g));
                carry = false;
            } else {
                const float q = (p - g) / (1.f - g);
                r.pathF = reachCarryEase(q, soft); r.carryW = easeSmooth(q / kCarryEaseLen);
                if (e > 0.f && q <= e) bump = extractBump(q / e);
            }
        } else {                                   // partial: continues from the retarget, no fresh extract
            r.pathF = s.pathFrom + (1.f - s.pathFrom) * reachCarryEase(p, soft);
            r.carryW = s.carryFrom + (1.f - s.carryFrom) * easeSmooth(p / kCarryEaseLen);
        }
    } else {                                       // stow
        const float c = 1.f - g;                   // stow: carry share of the slide
        if (p < c) {
            const float q = p / c;
            r.pathF = s.pathFrom * reachCarryEase(1.f - q, soft);
            r.carryW = s.carryFrom * easeSmooth((1.f - q) / kCarryEaseLen);
            if (e > 0.f && q >= 1.f - e) bump = extractBump((q - (1.f - e)) / e);
        } else {                                   // release: unchanged
            r.pathF = easeSmooth((p - c) / g); r.carryW = 0.f;
            carry = false;
        }
    }
    r.oriF = carry ? reachOriF(r.pathF, e, soft) : r.pathF;   // hold only in the carry window; reach and release follow pathF
    if (s.oriOn) r.oriF = s.oriFrom + (r.oriF - s.oriFrom) * easeSmooth(p / 0.1f);   // B1: the join keeps oriF continuous (blend over 10%)
    r.extractK = s.extractFrom > 0.f ? s.extractFrom * (1.f - easeSmooth(p)) : bump;   // D4
    return r;
}

// Retarget with continuity. g must be the grab share the reader passes to reachPlan. ramp must equal the reader's reach ramp (the Holster.cpp
// readers use g_reachCarry ? 0.04f : 0.2f); the default is the carry-mode value. Duration (Option B, this wrapper only): a reversal of a running
// slide takes ms * path distance / 2 (stow (P0 + 1) / 2, draw (1 - P0) / 2), so arm speed stays normal.
inline void reachRetarget(Slide& s, bool hand, uint32_t now, uint32_t drawMs, uint32_t stowMs, float g, float ramp = 0.04f, float extractPct = 0.f, bool soft = false) {
    if (s.dir == (hand ? 1 : 0)) return;                    // same direction: target() is a no-op, store nothing
    const ReachPlan old = reachPlan(s, now, g, extractPct, soft); // read BEFORE target(), which resets t0; extractPct = the reader's extract share; soft = the reader's reach_ease_soft_<cat>
    const float P0 = old.active ? old.pathF : 1.f;
    const float C0 = old.active ? old.carryW : (float)s.dir;
    const float w = reachWeightRamp(s, now, ramp);          // also before target()
    s.target(hand, now, drawMs, stowMs, w);
    s.pathFrom = P0; s.carryFrom = C0; s.extractFrom = old.extractK;   // HW31 D4: target() cleared extractFrom, so this comes after it
    // HW31 B1: only with an extract share (e > 0 is the only case where the carry target differs from pathF); extractPct 0 leaves oriOn 0.
    s.oriFrom = old.oriF; s.oriOn = (old.active && extractPct > 0.f) ? 1 : 0;
    if (old.active) {                                       // reversal of a running slide only; fresh start keeps frac length
        const uint32_t ms = hand ? drawMs : stowMs;
        const float half = 0.5f * (hand ? 1.f - P0 : P0 + 1.f);
        uint32_t l = 0;
        if (ms > 0) { l = (uint32_t)((float)ms * half + 0.5f); if (l == 0 && half > 0.f) l = 1; }
        s.len = l;
    }
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

// HW31 item 3: mount a -> via -> hand b. Continuous at t = 0.5 and sign-aligned (see slideViaQ in HolsterSlide.h).
// Non-finite a is taken as identity and a non-finite b keeps a (as blendQ). via must be finite (blendPoseLocked2 checks it).
inline Q reachViaQ(const Q& a0, const Q& b, const Q& via, float t) {
    const Q a = qfinite(a0) ? a0 : Q{ 1, 0, 0, 0 };
    if (!qfinite(b)) return a;
    return slideViaQ(a, via, b, t);
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

// ---- Wrist ----
// Rotation angle of a unit quaternion in degrees (0..180, sign-agnostic). Non-finite -> 0.
inline float reachQAngleDeg(const Q& q) {
    if (!qfinite(q)) return 0.f;
    float w = std::fabs(q.w);
    const float n = std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (!(n > 1e-12f)) return 0.f;
    w /= n;
    if (w > 1.f) w = 1.f;
    return 2.f * std::acos(w) * 57.29577951308232f;
}

// Limit a rotation to `maxDeg` (same axis, shorter angle). Within the limit it is returned normalized, unchanged. Non-finite -> identity.
// hint == nullptr: the sign is the shorter arc (w >= 0). With a hint (the previous output of this wrist) the sign follows the hint, so
// the axis stays put while the input passes 180 deg. The hint snaps the sign once the continued input passes 180 + maxDeg (~300 deg at
// the 120 deg limit); nothing caps the input. With a hint, w < 0 reads above 180 deg, so the clamp branch always takes it (maxDeg <= 180).
inline Q reachClampRot(const Q& dq0, float maxDeg, const Q* hint = nullptr) {
    if (!qfinite(dq0)) return { 1, 0, 0, 0 };
    Q dq = qnormalize(dq0);
    if (hint && qfinite(*hint)) {
        if (qdot(*hint, dq) < 0.f) { dq.w = -dq.w; dq.x = -dq.x; dq.y = -dq.y; dq.z = -dq.z; }
    } else if (dq.w < 0.f) { dq.w = -dq.w; dq.x = -dq.x; dq.y = -dq.y; dq.z = -dq.z; }   // shorter arc
    const float ang = 2.f * std::acos(dq.w > 1.f ? 1.f : dq.w) * 57.29577951308232f;
    if (!(maxDeg >= 0.f) || ang <= maxDeg) return dq;
    const float s2 = std::sqrt(dq.x * dq.x + dq.y * dq.y + dq.z * dq.z);
    if (!(s2 > 1e-9f)) return { 1, 0, 0, 0 };
    const float h = maxDeg * 3.14159265358979f / 360.f, k = std::sin(h) / s2;
    return { std::cos(h), dq.x * k, dq.y * k, dq.z * k };
}

// HW44 continuity pick for the forearm roll and the wrist swing (q11 candidate A, W2 variant). On entry twist = the clamped, hint-aligned roll; on exit twist and swing
// are the chosen pair with dqn ~= swing * twist. Four candidates: (twist, s), (twist, conj s), (conj twist, s'), (conj twist, conj s'), where s / s' is the shorter-arc clamp
// of dqn * conj(twist) / dqn * twist. Each is scored by |qdot(prev, swing * twist)| against the previous combined rotation of this slide (nullptr or non-finite = first
// frame: candidate 0). Candidate 0 keeps unless another beats it by more than 1e-4, so unclamped leftovers (both roll signs give the same combined rotation) stay put.
// Returns true when a candidate other than 0 wins (pk=1). A jump comes from the swing's shorter-arc sign (natural w of the leftover crossing 0 while the clamp binds)
// or from the roll's 4-D sign snap against its hint (the roll and the swing then flip together); the pick takes the continuous one.
inline bool reachPickContinuous(const Q& dqn, Q& twist, Q& swing, float swingMaxDeg, const Q* prev) {
    const Q t[2] = { twist, reachConj(twist) };
    Q cT[4], cS[4];
    for (int k = 0; k < 2; ++k) {
        const Q s = reachClampRot(qmul(dqn, reachConj(t[k])), swingMaxDeg, nullptr);
        cT[2 * k] = cT[2 * k + 1] = t[k];
        cS[2 * k] = s;
        cS[2 * k + 1] = reachConj(s);
    }
    int best = 0;
    if (prev && qfinite(*prev)) {
        float bestSc = std::fabs(qdot(*prev, qmul(cS[0], cT[0])));
        for (int i = 1; i < 4; ++i) {
            const float sc = std::fabs(qdot(*prev, qmul(cS[i], cT[i])));
            if (sc > bestSc + 1e-4f) { bestSc = sc; best = i; }
        }
    }
    twist = cT[best];
    swing = cS[best];
    return best != 0;
}

// HW29 wrist split (recon r1 step 3): dq = swingOut * twistOut. twistOut = the part of dq about foreAxis (world frame; need not be unit), clamped to
// twistMaxDeg by reachClampRot with twistHint (the previous twistOut), so its sign follows the hint across the 180 deg axis flip. swingOut = dq * conj(twistOut),
// clamped to swingMaxDeg (shorter arc), then reachPickContinuous against combPrev (HW44; nullptr = first frame = shorter arc). leftW = natural w of the leftover, overrode = the pick took the
// opposite sign (diagnostics). twistMaxDeg <= 0 or NaN: twistOut identity and swingOut = reachClampRot(dq, swingMaxDeg, nullptr), the pre-HW29 path.
// That path ignores foreAxis (may be null or NaN). Otherwise non-finite dq or a null/non-finite foreAxis: both identity. Zero foreAxis: twistOut identity and the whole dq goes to swing.
inline void reachSplitWrist(const Q& dq, const float* foreAxis, float twistMaxDeg, float swingMaxDeg, const Q* twistHint, Q& twistOut, Q& swingOut,
                            const Q* combPrev = nullptr, bool* overrode = nullptr, float* leftW = nullptr) {
    if (overrode) *overrode = false;
    if (leftW) *leftW = 0.f;
    if (!(twistMaxDeg > 0.f)) { twistOut = { 1, 0, 0, 0 }; swingOut = reachClampRot(dq, swingMaxDeg, nullptr); return; }   // before the axis checks
    const bool axisOk = foreAxis && std::isfinite(foreAxis[0]) && std::isfinite(foreAxis[1]) && std::isfinite(foreAxis[2]);
    if (!qfinite(dq) || !axisOk) { twistOut = { 1, 0, 0, 0 }; swingOut = { 1, 0, 0, 0 }; return; }
    const Q dqn = qnormalize(dq);
    Q twistRaw, swingDiscard;                         // the swing from qSwingTwist is unused: swingOut is re-derived from the clamped twist
    qSwingTwist(dqn, foreAxis, swingDiscard, twistRaw);
    twistOut = reachClampRot(twistRaw, twistMaxDeg, twistHint);
    if (leftW) *leftW = qmul(dqn, { twistOut.w, -twistOut.x, -twistOut.y, -twistOut.z }).w;
    // HW44c release: candidate 0 is exact when neither cap binds (the roll is the raw roll, as a rotation, and the natural swing is within its cap): then the split recomposes
    // to dq and there is nothing to keep continuous, so a stale previous rotation (a one-frame step in the target) can never pin the wrist on a wrong candidate.
    const Q twN = qnormalize(twistRaw);
    const Q swNat = qmul(dqn, reachConj(twistOut));
    const bool exact = qfinite(twN) && std::fabs(qdot(twistOut, twN)) > 0.99999f && reachQAngleDeg(swNat) <= swingMaxDeg;
    if (exact) { swingOut = reachClampRot(swNat, swingMaxDeg, nullptr); return; }
    const bool picked = reachPickContinuous(dqn, twistOut, swingOut, swingMaxDeg, combPrev);
    if (overrode) *overrode = picked;
}

// HW44 blend hemisphere (candidate B of q11). blendQ(from, to, easeSmooth(t)) aligns `to` to `from`, so the nlerp path changes when dot(from, to) crosses 0 (the hand
// passes 180 deg from the mount): the target jumps by up to ~180 deg. Here the two signs of `to` give two candidate blends (sum with +to, sum with -to); each is normalized
// and the one nearer prev (the previous blended target of this slide, same sign convention) is taken, so the path keeps its side. prev == nullptr (first frame), a
// non-finite input or a degenerate sum: blendQ, bit for bit.
inline Q reachTargetQ(const Q& from, const Q& to, float t, const Q* prev) {
    const float s = easeSmooth(t);
    if (!prev || !qfinite(*prev) || !qfinite(from) || !qfinite(to) || !(s > 0.f)) return blendQ(from, to, s);
    const Q sp = { (1.f - s) * from.w + s * to.w, (1.f - s) * from.x + s * to.x, (1.f - s) * from.y + s * to.y, (1.f - s) * from.z + s * to.z };
    const Q sm = { (1.f - s) * from.w - s * to.w, (1.f - s) * from.x - s * to.x, (1.f - s) * from.y - s * to.y, (1.f - s) * from.z - s * to.z };
    const bool okP = qdot(sp, sp) > 1e-4f, okM = qdot(sm, sm) > 1e-4f;   // a near-zero sum has no usable direction
    if (!okP && !okM) return blendQ(from, to, s);
    const Q pn = qnormalize(*prev);
    const float dp = okP ? qdot(pn, qnormalize(sp)) : -2.f;
    const float dm = okM ? qdot(pn, qnormalize(sm)) : -2.f;
    return dm > dp ? qnormalize(sm) : qnormalize(sp);
}

// World rotation to apply to a hand so its orientation `cur` becomes the target blend(from, to, t): dq = target * conj(cur), then limited to maxDeg and
// scaled by `weight` (0 = none). The hand is turned from the weapon's mount orientation (t = 0, at the grab) to its resting orientation (t = 1) over the carry.
// hint: the previous return value, passed to reachClampRot. The sign guarantee needs maxDeg in [0, 180]: the clamped dq then has w >= 0, so blendQ keeps it.
// HW44: the target is reachTargetQ(from, to, t, prevTgt) (prevTgt null = blendQ, the old path); tgtOut receives it for the caller's next frame.
inline Q reachWristDelta(const Q& from, const Q& to, float t, const Q& cur, float maxDeg, float weight, const Q* hint = nullptr, const Q* prevTgt = nullptr, Q* tgtOut = nullptr) {
    if (!qfinite(from) || !qfinite(to) || !qfinite(cur)) return { 1, 0, 0, 0 };
    const Q target = reachTargetQ(from, to, t, prevTgt);
    if (tgtOut) *tgtOut = target;
    const Q dq = reachClampRot(qmul(target, reachConj(cur)), maxDeg, hint);
    return blendQ(Q{ 1, 0, 0, 0 }, dq, weight);
}

// HW44 diagnostics: appends " lw=%.2f md=%d pk=%d" to a REACH piece already in buf (used = its length). Skipped when the result would pass 200 bytes, so a piece
// stays <= 200. lw outside [-9.99, 9.99] or NaN prints 0. Returns the new length.
inline int reachDiagTail(char* buf, size_t cap, int used, float lw, int md, int pk) {
    if (!buf || used < 0 || static_cast<size_t>(used) >= cap) return used;
    const float lwC = !(std::fabs(lw) <= 9.99f) ? 0.f : lw;
    char tail[40];
    const int n = snprintf(tail, sizeof tail, " lw=%.2f md=%d pk=%d", static_cast<double>(lwC), md, pk);
    if (n <= 0 || used + n > 200 || static_cast<size_t>(used + n) >= cap) return used;
    memcpy(buf + used, tail, static_cast<size_t>(n) + 1);
    return used + n;
}

// HW45 R2 diagnostics: appends " dq=%.0f dj=%.0f" (deg) to a REACH piece 3; skipped, never truncated, past 200 bytes or cap; NaN or out of [0, 360] prints 0.
// dq is the capped turn when reach_twist_deg = 0 (cap = reach_wrist_max_deg) and the whole turn when twist is on; dj = jump from the previous solve of this slide.
// The caller passes dqTot, which is already scaled by the arm-weight blend (w * reach_wrist_pct), so dq reads low while the weight ramps; at full weight it is the whole turn (does it cross 180?).
inline int reachDqTail(char* buf, size_t cap, int used, float dqDeg, float djDeg) {
    if (!buf || used < 0 || static_cast<size_t>(used) >= cap) return used;
    const float dqC = (dqDeg >= 0.f && dqDeg <= 360.f) ? dqDeg : 0.f;
    const float djC = (djDeg >= 0.f && djDeg <= 360.f) ? djDeg : 0.f;
    char tail[40];
    const int n = snprintf(tail, sizeof tail, " dq=%.0f dj=%.0f", static_cast<double>(dqC), static_cast<double>(djC));
    if (n <= 0 || used + n > 200 || static_cast<size_t>(used + n) >= cap) return used;
    memcpy(buf + used, tail, static_cast<size_t>(n) + 1);
    return used + n;
}

// Residual of a retarget (HW26): the rotation still to undo t ms after the retarget. Rres at t = 0, identity at t = ms, eased by
// easeSmooth. A later card multiplies it onto the carried rotation in place of the 10% world carry blend. Non-positive or non-finite ms,
// or a non-finite Rres, give identity. nlerp (blendQ) is deliberate: plan 42c 4.3 wrote slerp, but the step numbers were taken with nlerp.
// kReachResidMs = 240: at 60 fps the worst step over grabs 60..180 is 23.6 deg (180 deg grab); 230 ms gives 24.5. The 1.4 deg margin
// keeps the 25 deg bound.
constexpr uint32_t kReachResidMs = 240;
inline Q reachResidual(const Q& Rres, float t, float ms) {
    if (!(ms > 0.f) || !std::isfinite(ms)) return { 1, 0, 0, 0 };
    const float s = easeSmooth(t / ms);                 // clamps to [0, 1]; NaN -> 0, t < 0 -> Rres
    return blendQ(Q{ 1, 0, 0, 0 }, Rres, 1.f - s);     // w <= 0 returns identity bit for bit
}

// HW31b (doc 42c 4.14): reachResidual routed through a via quat. Rres -> viaH -> identity as t runs 0 -> ms, eased by easeSmooth. The
// via is sign-aligned to Rres (hemisphere lock), so each leg turns <= 90 deg and the join at s = 0.5 is continuous. viaH null or
// non-finite, a bad ms or a non-finite Rres give reachResidual bit for bit.
inline Q reachResidualVia(const Q& Rres, const Q* viaH, float t, float ms) {
    if (!viaH || !qfinite(*viaH) || !qfinite(Rres) || !(ms > 0.f) || !std::isfinite(ms)) return reachResidual(Rres, t, ms);
    const Q via = qdot(Rres, *viaH) < 0.f ? Q{ -viaH->w, -viaH->x, -viaH->y, -viaH->z } : *viaH;
    const float s = easeSmooth(t / ms);                 // clamps to [0, 1]; NaN -> 0, t < 0 -> Rres
    if (s < 0.5f) return blendQ(Rres, via, 2.f * s);
    return blendQ(via, Q{ 1, 0, 0, 0 }, 2.f * s - 1.f);   // lands on -identity (same rotation) when via.w < 0
}

// Outside-shoulder lateral offset of the via pose, metres (default for ini reach_via_out_m).
constexpr float kReachViaOutM = 0.12f;
// HW44d: reach_via_out_m_<cat> (0..0.30 m, per category). Parse: clamp to 0..0.30 (NaN -> 0). Use: the category value when set (>= 0; -1 = unset), else the global reach_via_out_m.
inline float reachViaOutClamp(float v) { return !(v >= 0.f) ? 0.f : (v > 0.30f ? 0.30f : v); }
inline float reachViaOutFor(float catOut, float globalOut) { return catOut >= 0.f ? catOut : globalOut; }
// HW50: reach_via_tilt_deg_<cat> (0..90, default 0): the via's target up tilts toward body forward. NaN or < 0 -> 0, > 90 -> 90.
constexpr float kReachViaTiltMax = 90.f;
inline float reachViaTiltClamp(float v) { return !(v >= 0.f) ? 0.f : (v > kReachViaTiltMax ? kReachViaTiltMax : v); }
// HW50: tilt <= 0 -> {0, 0, 1}, bodyQ untouched. Else {0, sin t, cos t} (body frame x right, y forward, z up), rotated by a finite *bodyQ, else unrotated.
inline void reachViaUp(float tiltDeg, const Q* bodyQ, float out[3]) {
    const float t = reachViaTiltClamp(tiltDeg);
    if (!(t > 0.f)) { out[0] = 0.f; out[1] = 0.f; out[2] = 1.f; return; }
    const float a = t * 3.14159265358979f / 180.f;
    const float v[3] = { 0.f, std::sin(a), std::cos(a) };
    if (bodyQ && qfinite(*bodyQ)) qrotate(qnormalize(*bodyQ), v, out);
    else { out[0] = v[0]; out[1] = v[1]; out[2] = v[2]; }
}
// HW31b: lateral bump of the via pose, outM * 16 s^2 (1 - s)^2 with s = clamp01(t / ms) (linear, not eased). 0 at both ends, outM at
// s = 0.5. Bad ms, non-finite values or outM <= 0 give 0.
inline float reachViaOffset(float t, float ms, float outM) {
    if (!(ms > 0.f) || !std::isfinite(ms) || !(outM > 0.f) || !std::isfinite(outM)) return 0.f;
    const float s = slideClamp01(t / ms);               // NaN -> 0
    return outM * 16.f * s * s * (1.f - s) * (1.f - s);
}

}  // namespace hs
