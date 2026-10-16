// HolsterEdit.h -- pure maths of the holstered-weapons equip-screen editor (doc 42, HW-EDIT): Euler extraction from a placement
// quaternion (ZYX, same convention as qbase), a continuity-preserving Euler choice, world-drag to bone-local deltas, local axis
// nudges, screen drags and snapping. Header-only, no windows.h, no engine calls, no STL containers. Host-tested by
// holster_edit_test.cpp. Angles are degrees; quaternions are w-first (HolsterQuat.h).
#pragma once
#include <cmath>
#include "HolsterQuat.h"

namespace hs {

namespace detail {
constexpr float kRad2Deg = 57.29577951308232f;
constexpr float kDeg2Rad = 0.01745329251994330f;

// Rotation matrix of a unit quaternion, row-major m[row][col]. Active rotation, the same convention as qrotate.
inline void qmat(const Q& q, float m[3][3]) {
    const float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    const float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
    const float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
    m[0][0] = 1 - 2 * (yy + zz);
    m[0][1] = 2 * (xy - wz);
    m[0][2] = 2 * (xz + wy);
    m[1][0] = 2 * (xy + wz);
    m[1][1] = 1 - 2 * (xx + zz);
    m[1][2] = 2 * (yz - wx);
    m[2][0] = 2 * (xz - wy);
    m[2][1] = 2 * (yz + wx);
    m[2][2] = 1 - 2 * (xx + yy);
}

inline void mat3mul(const float a[3][3], const float b[3][3], float o[3][3]) {
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) o[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j];
}

// Squared distance between two Euler triples, each component measured as a wrapped difference.
inline float eulerDist2(const float a[3], const float b[3]) {
    float s = 0.f;
    for (int i = 0; i < 3; ++i) {
        float d = std::fmod(a[i] - b[i], 360.f);
        if (d > 180.f) d -= 360.f;
        else if (d <= -180.f) d += 360.f;
        s += d * d;
    }
    return s;
}
}  // namespace detail

inline Q qconj(const Q& q) { return { q.w, -q.x, -q.y, -q.z }; }

// Wrap an angle to (-180, 180].
inline float wrap180(float v) {
    float w = std::fmod(v, 360.f);
    if (w > 180.f) w -= 360.f;
    else if (w <= -180.f) w += 360.f;
    return w;
}

// Round to the nearest multiple of step. step <= 0 returns v unchanged.
inline float snapDeg(float v, float step) { return step > 0.f ? std::round(v / step) * step : v; }
inline float snapOff(float v, float step) { return step > 0.f ? std::round(v / step) * step : v; }

// Inverse of qbase: out = {rx, ry, rz} in degrees with qbase(out) = +-q. ry is in [-90, 90]. At the gimbal lock (|ry| = 90 within
// about 0.006 deg) rx is set to 0 and rz absorbs the combined angle.
inline void eulerFromQ(const Q& q0, float out[3]) {
    const Q q = qnormalize(q0);
    float m[3][3];
    detail::qmat(q, m);
    const float sb = std::fmax(-1.f, std::fmin(1.f, -m[2][0]));  // sin(ry)
    const float cb = std::sqrt(m[2][1] * m[2][1] + m[2][2] * m[2][2]);  // |cos(ry)|, never negative
    if (cb > 1e-4f) {
        out[0] = std::atan2(m[2][1], m[2][2]) * detail::kRad2Deg;
        out[1] = std::atan2(sb, cb) * detail::kRad2Deg;
        out[2] = std::atan2(m[1][0], m[0][0]) * detail::kRad2Deg;
    } else {
        out[0] = 0.f;
        out[1] = sb >= 0.f ? 90.f : -90.f;
        out[2] = std::atan2(-m[0][1], m[1][1]) * detail::kRad2Deg;
    }
}

// Continuity-preserving inverse: of the two equivalent triples {rx, ry, rz} and {rx+180, 180-ry, rz+180} (each component wrapped),
// out is the one closest to prev. Near the singularity (|ry| >= 80) rx is kept at prev[0], ry comes from q, and rz is solved so
// that Rz(rz) * Ry(ry) * Rx(rx) fits q. That fit is exact only when prev[0] equals the true rx, or when |ry| = 90 exactly.
inline void eulerNear(const Q& q, const float prev[3], float out[3]) {
    float p[3];
    eulerFromQ(q, p);
    if (std::fabs(p[1]) >= 80.f) {
        const float a = wrap180(prev[0]);
        const float b = p[1] * detail::kDeg2Rad;
        const float ca = std::cos(a * detail::kDeg2Rad), sa = std::sin(a * detail::kDeg2Rad);
        const float cb = std::cos(b), sb = std::sin(b);
        float R[3][3], M[3][3], Mt[3][3], T[3][3];
        detail::qmat(qnormalize(q), R);
        const float Ry[3][3] = {{cb, 0.f, sb}, {0.f, 1.f, 0.f}, {-sb, 0.f, cb}};
        const float Rx[3][3] = {{1.f, 0.f, 0.f}, {0.f, ca, -sa}, {0.f, sa, ca}};
        detail::mat3mul(Ry, Rx, M);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) Mt[i][j] = M[j][i];
        detail::mat3mul(R, Mt, T);  // T = R * M^T = Rz(rz) when the fit is exact; the Procrustes angle of its xy block is rz
        out[0] = a;
        out[1] = p[1];
        out[2] = std::atan2(T[1][0] - T[0][1], T[0][0] + T[1][1]) * detail::kRad2Deg;
        return;
    }
    float alt[3] = {wrap180(p[0] + 180.f), wrap180(180.f - p[1]), wrap180(p[2] + 180.f)};
    float pri[3] = {wrap180(p[0]), wrap180(p[1]), wrap180(p[2])};
    const bool useAlt = detail::eulerDist2(alt, prev) < detail::eulerDist2(pri, prev);
    for (int i = 0; i < 3; ++i) out[i] = useAlt ? alt[i] : pri[i];
}

// Local delta for a world-space orientation drag. The bone's world orientation is boneQ * placeQ, so the new local placement is
// conj(boneQ) * dq * boneQ * placeQ, normalized.
inline Q worldDeltaToLocal(const Q& boneQ, const Q& placeQ, const Q& dqWorld) {
    return qnormalize(qmul(qmul(qmul(qconj(boneQ), dqWorld), boneQ), placeQ));
}

// Offset change for a world-space translation drag: the offset is in the bone's frame, so out = off + rotate(conj(boneQ), dp).
inline void worldMoveToLocal(const Q& boneQ, const float off[3], const float dpWorld[3], float out[3]) {
    float r[3];
    qrotate(qconj(boneQ), dpWorld, r);
    for (int i = 0; i < 3; ++i) out[i] = off[i] + r[i];
}

// Rotate the placement about one bone axis (0 x, 1 y, 2 z) by deg, pre-multiplied so the axis is the bone's own. Invalid axis
// returns placeQ unchanged (normalized).
inline Q localAxisNudge(const Q& placeQ, int axis, float deg) {
    if (axis < 0 || axis > 2) return qnormalize(placeQ);
    float e[3] = {0.f, 0.f, 0.f};
    e[axis] = 1.f;
    return qnormalize(qmul(qaxis(e, deg), placeQ));
}

// Orientation change for a screen drag: horizontal (dx) turns about screen up, vertical (dy) about screen right.
inline Q screenDrag(float dxPx, float dyPx, const float camRight[3], const float camUp[3], float degPerPx) {
    return qnormalize(qmul(qaxis(camUp, dxPx * degPerPx), qaxis(camRight, dyPx * degPerPx)));
}

}  // namespace hs
