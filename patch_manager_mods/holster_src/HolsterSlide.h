// HolsterSlide.h -- cosmetic draw/stow slide of the holstered-weapons mod (doc 42, HW14): a weapon eases between the body mount (hip)
// pose and the hand pose instead of snapping. One hs::Slide per weapon holds a time-based hand fraction (0 = hip, 1 = hand); the draw
// gate (HolsterDrawGate.h) only chooses the target. The slide never gates the hand attach, it only shapes the written pose. Pure math,
// header-only, no engine calls. Host-tested by holster_slide_test.cpp.
#pragma once
#include <stdint.h>
#include "HolsterQuat.h"

namespace hs {

// Clamp to [0, 1]. NaN maps to 0, so a bad fraction can never produce a non-finite pose.
inline float slideClamp01(float f) {
    if (!(f > 0.f)) return 0.f;
    return f > 1.f ? 1.f : f;
}

// Smoothstep ease: 0 at 0, 1 at 1, zero slope at both ends (the weapon starts and lands gently).
inline float easeSmooth(float f) {
    f = slideClamp01(f);
    return f * f * (3.f - 2.f * f);
}

// Position lerp; orientation nlerp along the shorter arc (the hand quat is negated when it lies more than 90 deg from the mount,
// which is the same rotation). f is clamped to [0, 1]; the output orientation is a unit quaternion.
inline Pose blendPose(const Pose& mount, const Pose& hand, float f) {
    f = slideClamp01(f);
    Pose o;
    for (int i = 0; i < 3; ++i) o.pos[i] = (1.f - f) * mount.pos[i] + f * hand.pos[i];
    Q h = hand.q;
    const Q& m = mount.q;
    if (m.w * h.w + m.x * h.x + m.y * h.y + m.z * h.z < 0.f) { h.w = -h.w; h.x = -h.x; h.y = -h.y; h.z = -h.z; }
    o.q = qnormalize({ (1.f - f) * m.w + f * h.w, (1.f - f) * m.x + f * h.x, (1.f - f) * m.y + f * h.y, (1.f - f) * m.z + f * h.z });
    return o;
}

// ---- Per-weapon time-based slide ----
// One Slide per weapon (left/right independent). `frac` is the hand fraction at a tick: 0 = hip/body mount, 1 = hand. A retarget
// starts from the current fraction (no pop) and takes ms * remaining distance, so reversing half way takes half the time.
// All tick compares are signed 32-bit differences, so the counter wrapping past 0xFFFFFFFF is harmless.
struct Slide {
    float from = 0;
    uint32_t t0 = 0, len = 0, last = 0;
    uint8_t dir = 0;  // 0 = heading to the hip, 1 = heading to the hand
    uint8_t init = 0;
    float pathFrom = 1;   // path position (0 mount .. 1 rest) at the last retarget; reachPlan starts from it
    float carryFrom = 0;  // carry weight at the last retarget
    float w0 = 0;         // reach weight at the last retarget (holdW), blended over the new slide's first ramp window

    // Silent placement: no motion, fraction is the target at once.
    void reset(bool hand, uint32_t now) {
        dir = hand ? 1 : 0;
        from = (float)dir;
        len = 0;
        t0 = last = now;
        init = 1;
        pathFrom = 1.f;
        carryFrom = (float)dir;
        w0 = 0.f;
    }

    // Hand fraction in [0, 1] at tick `now`. A tick before t0 (stale) reports `from`; endpoints are exact.
    float frac(uint32_t now) const {
        if (len == 0) return (float)dir;
        const int32_t d = (int32_t)(now - t0);
        if (d <= 0) return from;
        if (d >= (int32_t)len) return (float)dir;
        return from + ((float)dir - from) * (float)d / (float)len;
    }

    // Head for the hand (true) or the hip (false). No-op when already heading there. ms == 0 moves instantly. holdW = the reach weight at
    // this retarget (NaN -> 0); the new slide's reach weight blends from it (reachWeightRamp). Timing is unchanged by holdW.
    void target(bool hand, uint32_t now, uint32_t drawMs, uint32_t stowMs, float holdW = 0.f) {
        const uint8_t want = hand ? 1 : 0;
        if (dir == want) return;
        const float f = frac(now);  // evaluated before dir changes
        const float dist = (float)want > f ? (float)want - f : f - (float)want;
        const uint32_t ms = hand ? drawMs : stowMs;
        uint32_t l = 0;
        if (ms > 0) {
            l = (uint32_t)((float)ms * dist + 0.5f);
            if (l == 0 && dist > 0.f) l = 1;
        }
        from = f;
        dir = want;
        t0 = last = now;
        len = l;
        w0 = slideClamp01(holdW);
    }

    // Gap guard, call once per tick: a stall longer than 100 ms while a slide was running shifts t0 by the stall, so the slide
    // resumes from the fraction it had at the last call instead of finishing during the pause.
    void touch(uint32_t now) {
        const int32_t gap = (int32_t)(now - last);
        if (gap > 100 && len > 0 && (int32_t)(last - t0) < (int32_t)len) t0 += (uint32_t)gap;
        last = now;
    }

    // True strictly between the two ends of a running slide.
    bool active(uint32_t now) const {
        if (len == 0) return false;
        const int32_t d = (int32_t)(now - t0);
        return d > 0 && d < (int32_t)len;
    }
};

// Shown pose of one weapon: eased blend of its hip pose and its hand pose. Exactly hip / hand at the slide ends.
inline Pose slideShown(const Slide& s, uint32_t now, const Pose& hip, const Pose& hand) {
    return blendPose(hip, hand, easeSmooth(s.frac(now)));
}

inline uint32_t clampDrawSlideMs(long v) {
    if (v < 0) return 0;
    if (v > 570) return 570;
    return (uint32_t)v;
}

inline uint32_t clampStowSlideMs(long v) {
    if (v < 0) return 0;
    if (v > 1000) return 1000;
    return (uint32_t)v;
}

}  // namespace hs
