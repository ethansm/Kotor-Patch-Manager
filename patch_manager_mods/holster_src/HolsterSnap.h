// HolsterSnap.h -- pure SNAP detector of the holstered-weapons mod (no Win32, host-testable; tests in holster_snap_test.cpp). Log only (0.21.1-dev): it never changes a pose.
// Diagnoses the slow-holster draw snap (slow_holster_x 6..8: a smooth draw, then ONE frame to the engine's wielded pose). Fed with the weapon's world position at the end of
// every placement call of a Gob while its slide runs (and the one frame after the slide's end), whichever path wrote it (our blend / carry write, or the engine pose that
// stays when our write was skipped).
//   A snap = this frame's jump > threshold (ini snap_log_m, default 0.05 m, 0 = off) AND > 4x the previous frame's step (slow smooth motion never triggers).
//   The first sample after a reset has no step, and the second sample only establishes one: neither can report (no false positive after a reset / a new slide).
#pragma once
#include <math.h>
#include <stdlib.h>

namespace hs {

const float kSnapLogDefaultM = 0.05f;   // snap_log_m default (m)
const float kSnapLogMaxM = 2.0f;        // upper clamp of snap_log_m
const float kSnapStepFactor = 4.0f;     // a jump must also exceed this multiple of the previous step

struct SnapState {
    float pos[3] = {0.f, 0.f, 0.f};   // last sample
    float step = 0.f;                 // distance of the last sample from the one before it
    int n = 0;                        // samples since the reset (0, 1, 2+)
};

inline void snapReset(SnapState& s) { s.pos[0] = s.pos[1] = s.pos[2] = 0.f; s.step = 0.f; s.n = 0; }

// snap_log_m text -> metres. Blank, junk or nan = the default; below 0 = 0 (off); above kSnapLogMaxM is clamped. Returns false when the text was not a number (out = default).
inline bool snapParse(const char* text, float& out) {
    out = kSnapLogDefaultM;
    if (!text) return false;
    char* end = nullptr;
    const double v = strtod(text, &end);
    if (end == text || !(v == v) || v > 1e30 || v < -1e30) return false;
    out = v <= 0.0 ? 0.f : (v > static_cast<double>(kSnapLogMaxM) ? kSnapLogMaxM : static_cast<float>(v));
    return true;
}

// One sample. Returns true when it is a snap (jump = distance from the previous sample, prevStep = the step before it). A non-finite sample is ignored (no update).
// threshM <= 0 (or not finite) = off: the state still follows, nothing reports.
inline bool snapStep(SnapState& s, const float cur[3], float threshM, float* jumpOut = nullptr, float* prevStepOut = nullptr) {
    if (!(cur[0] == cur[0]) || !(cur[1] == cur[1]) || !(cur[2] == cur[2]) || fabsf(cur[0]) > 1e30f || fabsf(cur[1]) > 1e30f || fabsf(cur[2]) > 1e30f) return false;
    bool snap = false;
    float jump = 0.f;
    const float prevStep = s.step;
    if (s.n >= 1) {
        const float dx = cur[0] - s.pos[0], dy = cur[1] - s.pos[1], dz = cur[2] - s.pos[2];
        jump = sqrtf(dx * dx + dy * dy + dz * dz);
        if (s.n >= 2 && threshM > 0.f && jump > threshM && jump > kSnapStepFactor * prevStep) snap = true;
        s.step = snap ? prevStep : jump;   // a snap does not become the new "normal" step: a second jump right after it is still judged against the smooth motion
    }
    s.pos[0] = cur[0]; s.pos[1] = cur[1]; s.pos[2] = cur[2];
    if (s.n < 2) ++s.n;
    if (jumpOut) *jumpOut = jump;
    if (prevStepOut) *prevStepOut = prevStep;
    return snap;
}

}  // namespace hs
