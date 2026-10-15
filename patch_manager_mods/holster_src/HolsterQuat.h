// HolsterQuat.h -- pure placement math of the holstered-weapons mod (doc 42): quaternions and the node-to-weapon pose composition.
// Quaternion core copied from inventory_3d_viewport_src/Inv3dQuat.h (w-first, Hamilton qmul, angles in degrees). Header-only, no
// windows.h, no engine calls. Placement (hs::Placement) comes from HolsterLogic.h. Host-tested by holster_quat_test.cpp.
#pragma once
#include <cmath>
#include "HolsterLogic.h"

namespace hs {

struct Q { float w, x, y, z; };

inline Q qmul(const Q& a, const Q& b) {
    return { a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
             a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
             a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
             a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w };
}
inline Q qnormalize(Q q) {
    float n = std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (n < 1e-12f) return { 1, 0, 0, 0 };
    return { q.w / n, q.x / n, q.y / n, q.z / n };
}
inline Q qaxis(const float* axis, float deg) {
    float l = std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (l < 1e-12f) return { 1, 0, 0, 0 };
    float h = deg * 3.14159265358979f / 360.0f, s = std::sin(h) / l;
    return { std::cos(h), axis[0] * s, axis[1] * s, axis[2] * s };
}
// v' = v + w*t + q.xyz x t, t = 2 * (q.xyz x v): the active rotation q v q*.
inline void qrotate(const Q& q, const float* v, float* out) {
    float tx = 2 * (q.y * v[2] - q.z * v[1]), ty = 2 * (q.z * v[0] - q.x * v[2]), tz = 2 * (q.x * v[1] - q.y * v[0]);
    out[0] = v[0] + q.w * tx + (q.y * tz - q.z * ty);
    out[1] = v[1] + q.w * ty + (q.z * tx - q.x * tz);
    out[2] = v[2] + q.w * tz + (q.x * ty - q.y * tx);
}
// qBase = qZ(rz) * qY(ry) * qX(rx), degrees. Rotates a column vector like euler_matrix (R = Rz @ Ry @ Rx), see u05.md.
inline Q qbase(float rx, float ry, float rz) {
    const float X[3] = {1, 0, 0}, Y[3] = {0, 1, 0}, Z[3] = {0, 0, 1};
    return qmul(qaxis(Z, rz), qmul(qaxis(Y, ry), qaxis(X, rx)));
}

// Pose of a scene node: position (Gob+0xA4 vec3) and orientation (Gob+0xB0 quat, w-first).
struct Pose { float pos[3]; Q q; };

// Child pose = node pose composed with a local placement. The offset is rotated by the node's orientation; the local Euler
// rotation is applied after the node's, so the output orientation is node.q * qbase(rotDeg), normalized.
inline Pose applyPlacement(const Pose& node, const Placement& pl) {
    Pose o;
    float r[3];
    qrotate(node.q, pl.pos, r);
    for (int i = 0; i < 3; ++i) o.pos[i] = node.pos[i] + r[i];
    o.q = qnormalize(qmul(node.q, qbase(pl.rotDeg[0], pl.rotDeg[1], pl.rotDeg[2])));
    return o;
}

// True when all 7 floats are finite and the orientation length is within 1e-3 of 1.
inline bool poseSane(const Pose& p) {
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(p.pos[i])) return false;
    const float c[4] = {p.q.w, p.q.x, p.q.y, p.q.z};
    float n2 = 0.f;
    for (int i = 0; i < 4; ++i) {
        if (!std::isfinite(c[i])) return false;
        n2 += c[i] * c[i];
    }
    return std::fabs(std::sqrt(n2) - 1.f) <= 1e-3f;
}

}  // namespace hs
