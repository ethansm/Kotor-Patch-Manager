// w-first quaternions/vectors for Inventory 3D Viewport v2 (engine convention: identity = (1,0,0,0), index 0 = w).
// qmul is standard Hamilton(a,b); NOTE the engine's inline product in FUN_008cce70 is Hamilton(b,a) - we never call it. Host-testable, no windows.h.
#pragma once
#include <math.h>

struct Q { float w, x, y, z; };

inline Q qmul(const Q& a, const Q& b) {
    return { a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
             a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
             a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
             a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w };
}
inline Q qnormalize(Q q) {
    float n = sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (n < 1e-12f) return { 1, 0, 0, 0 };
    return { q.w / n, q.x / n, q.y / n, q.z / n };
}
inline Q qaxis(const float* axis, float deg) {
    float l = sqrtf(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (l < 1e-12f) return { 1, 0, 0, 0 };
    float h = deg * 3.14159265358979f / 360.0f, s = sinf(h) / l;
    return { cosf(h), axis[0] * s, axis[1] * s, axis[2] * s };
}
inline void qrotate(const Q& q, const float* v, float* out) {
    // v' = v + w*t + q.xyz x t, t = 2 * (q.xyz x v)
    float tx = 2 * (q.y * v[2] - q.z * v[1]), ty = 2 * (q.z * v[0] - q.x * v[2]), tz = 2 * (q.x * v[1] - q.y * v[0]);
    out[0] = v[0] + q.w * tx + (q.y * tz - q.z * ty);
    out[1] = v[1] + q.w * ty + (q.z * tx - q.x * tz);
    out[2] = v[2] + q.w * tz + (q.x * ty - q.y * tx);
}
inline void vcross(const float* a, const float* b, float* o) {
    o[0] = a[1] * b[2] - a[2] * b[1]; o[1] = a[2] * b[0] - a[0] * b[2]; o[2] = a[0] * b[1] - a[1] * b[0];
}
// qBase = qZ(rz) * qY(ry) * qX(rx), degrees
inline Q qbase(float rx, float ry, float rz) {
    const float X[3] = {1, 0, 0}, Y[3] = {0, 1, 0}, Z[3] = {0, 0, 1};
    return qmul(qaxis(Z, rz), qmul(qaxis(Y, ry), qaxis(X, rx)));
}
