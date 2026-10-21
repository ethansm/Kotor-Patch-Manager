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
struct ReachPlan { bool active; float pathF, restMix, carryW; };

constexpr float kCarryEaseLen = 0.15f;   // L: share of the carry over which the carry weight eases in/out
// Carry path position after carry fraction q (0..1 -> 0..1). Zero speed at both ends; slope v = 1/(1-L/2) in the middle.
inline float reachCarryF(float q) {
    const float L = kCarryEaseLen, v = 1.f / (1.f - L / 2.f);
    q = slideClamp01(q);
    if (q <= L) { const float x = q / L; return v * L * (x * x * x - x * x * x * x / 2.f); }
    return v * (L / 2.f + q - L);
}

// The plan starts from the slide's stored retarget state (pathFrom, carryFrom), so a reversal continues where the arm was. The retarget tick
// (d == 0) is active.
inline ReachPlan reachPlan(const Slide& s, uint32_t now, float g) {
    ReachPlan r = { false, 0.f, 0.f, 0.f };
    const int32_t d = (int32_t)(now - s.t0);
    if (d < 0 || d >= (int32_t)s.len) return r;   // was s.active(now): the retarget tick (d == 0) is inside now
    if (!(g > 0.f)) g = 0.f;
    if (g > 0.6f) g = 0.6f;
    const float p = slideClamp01((float)d / (float)s.len);
    const float from = slideClamp01(s.from);      // frac: only the fresh / partial split uses it
    r.active = true;
    if (s.dir) {                                   // draw
        if (from <= 0.001f) {                      // fresh from the mount: reach if g > 0, then carry from 0
            if (g > 0.f && p < g) { r.pathF = s.pathFrom * (1.f - easeSmooth(p / g)); return r; }
            const float q = (p - g) / (1.f - g);
            r.pathF = reachCarryF(q); r.carryW = easeSmooth(q / kCarryEaseLen); return r;
        }
        r.pathF = s.pathFrom + (1.f - s.pathFrom) * reachCarryF(p);     // partial: continues from the retarget
        r.carryW = s.carryFrom + (1.f - s.carryFrom) * easeSmooth(p / kCarryEaseLen);
        return r;
    }
    const float c = 1.f - g;                       // stow: carry share of the slide
    if (p < c) {
        const float q = p / c;
        r.pathF = s.pathFrom * reachCarryF(1.f - q);
        r.carryW = s.carryFrom * easeSmooth((1.f - q) / kCarryEaseLen);
        return r;
    }
    r.pathF = easeSmooth((p - c) / g); r.carryW = 0.f;   // release: unchanged
    return r;
}

// Retarget with continuity. g must be the grab share the reader passes to reachPlan. ramp must equal the reader's reach ramp (the Holster.cpp
// readers use g_reachCarry ? 0.04f : 0.2f); the default is the carry-mode value. Duration (Option B, this wrapper only): a reversal of a running
// slide takes ms * path distance / 2 (stow (P0 + 1) / 2, draw (1 - P0) / 2), so arm speed stays normal.
inline void reachRetarget(Slide& s, bool hand, uint32_t now, uint32_t drawMs, uint32_t stowMs, float g, float ramp = 0.04f) {
    if (s.dir == (hand ? 1 : 0)) return;                    // same direction: target() is a no-op, store nothing
    const ReachPlan old = reachPlan(s, now, g);             // read BEFORE target(), which resets t0
    const float P0 = old.active ? old.pathF : 1.f;
    const float C0 = old.active ? old.carryW : (float)s.dir;
    const float w = reachWeightRamp(s, now, ramp);          // also before target()
    s.target(hand, now, drawMs, stowMs, w);
    s.pathFrom = P0; s.carryFrom = C0;
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
inline Q reachClampRot(const Q& dq0, float maxDeg) {
    if (!qfinite(dq0)) return { 1, 0, 0, 0 };
    Q dq = qnormalize(dq0);
    if (dq.w < 0.f) { dq.w = -dq.w; dq.x = -dq.x; dq.y = -dq.y; dq.z = -dq.z; }   // shorter arc
    const float ang = 2.f * std::acos(dq.w > 1.f ? 1.f : dq.w) * 57.29577951308232f;
    if (!(maxDeg >= 0.f) || ang <= maxDeg) return dq;
    const float s2 = std::sqrt(dq.x * dq.x + dq.y * dq.y + dq.z * dq.z);
    if (!(s2 > 1e-9f)) return { 1, 0, 0, 0 };
    const float h = maxDeg * 3.14159265358979f / 360.f, k = std::sin(h) / s2;
    return { std::cos(h), dq.x * k, dq.y * k, dq.z * k };
}

// World rotation to apply to a hand so its orientation `cur` becomes the target blend(from, to, t): dq = target * conj(cur), then limited to maxDeg and
// scaled by `weight` (0 = none). The hand is turned from the weapon's mount orientation (t = 0, at the grab) to its resting orientation (t = 1) over the carry.
inline Q reachWristDelta(const Q& from, const Q& to, float t, const Q& cur, float maxDeg, float weight) {
    if (!qfinite(from) || !qfinite(to) || !qfinite(cur)) return { 1, 0, 0, 0 };
    const Q target = blendQ(from, to, easeSmooth(t));
    const Q dq = reachClampRot(qmul(target, reachConj(cur)), maxDeg);
    return blendQ(Q{ 1, 0, 0, 0 }, dq, weight);
}

}  // namespace hs
