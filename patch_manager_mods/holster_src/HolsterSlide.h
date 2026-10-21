// HolsterSlide.h -- cosmetic draw/stow slide of the holstered-weapons mod (doc 42, HW14): a weapon eases between the body mount (hip)
// pose and the hand pose instead of snapping. One hs::Slide per weapon holds a time-based hand fraction (0 = hip, 1 = hand); the draw
// gate (HolsterDrawGate.h) only chooses the target. The slide never gates the hand attach, it only shapes the written pose. Pure math,
// header-only, no engine calls. Host-tested by holster_slide_test.cpp.
#pragma once
#include <cmath>
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
    float extractFrom = 0;  // HW31 D4: extractK at the last retarget (reachRetarget); reachPlan decays it to 0 over the new slide
    float oriFrom = 0;      // HW31 B1: oriF at the last retarget when the extract share was > 0; reachPlan blends oriF from it over the first 10%
    uint8_t oriOn = 0;      // 1 while oriFrom is set (reset() and target() clear it; reachRetarget sets it)
    float w0 = 0;         // reach weight at the last retarget (holdW), blended over the new slide's first ramp window
    // Hemisphere lock (blendPoseLocked). lockH is the hand quat after the sign flip of the last locked call (the sign reference);
    // lockOn is cleared only by reset(), so target() keeps both.
    Q lockH{ 1, 0, 0, 0 };
    uint8_t lockOn = 0;   // 1 once blendPoseLocked ran since the last reset()
    // HW33a pending target (targetDelayed / pump). pendAt is the tick the armed target fires; pendWant is its direction; the rest
    // are the target() arguments. pendOn is cleared by reset() and target(), so a pending target never outlives a direct retarget.
    uint32_t pendAt = 0, pendDraw = 0, pendStow = 0;
    float pendHoldW = 0;
    uint8_t pendWant = 0, pendOn = 0;

    // Silent placement: no motion, fraction is the target at once.
    void reset(bool hand, uint32_t now) {
        pendOn = 0;
        dir = hand ? 1 : 0;
        from = (float)dir;
        len = 0;
        t0 = last = now;
        init = 1;
        pathFrom = 1.f;
        carryFrom = (float)dir;
        extractFrom = 0.f;
        oriFrom = 0.f;
        oriOn = 0;
        w0 = 0.f;
        lockOn = 0;
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
        pendOn = 0;
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
        // extractFrom and oriOn are cleared here (HW31 D4, B1). pathFrom / carryFrom are left alone: reachRetarget sets them after target().
        extractFrom = 0.f;
        oriOn = 0;
    }

    // HW33a stagger: head for the target after delayMs (dual-wield left hand, party jitter). delayMs == 0 is target() itself.
    // Idempotent per want: the same want called every tick keeps the armed target (it fires at its first pendAt). A want equal to the
    // current dir cancels a pending target (reversing during the delay = stay as is). pump() fires it.
    void targetDelayed(bool hand, uint32_t now, uint32_t delayMs, uint32_t drawMs, uint32_t stowMs, float holdW = 0.f) {
        if (delayMs == 0) {
            target(hand, now, drawMs, stowMs, holdW);
            return;
        }
        const uint8_t want = hand ? 1 : 0;
        if (pendOn && pendWant == want) return;
        if (dir == want) {
            pendOn = 0;
            return;
        }
        pendOn = 1;
        pendWant = want;
        pendAt = now + delayMs;
        pendDraw = drawMs;
        pendStow = stowMs;
        pendHoldW = holdW;
    }

    // Fires the pending target once now >= pendAt (signed compare, wrap-safe). It fires at pendAt, so t0 = pendAt and the slide loses
    // no time. A pump more than 100 ms late (a stall with no touch) fires at now instead, so the slide never starts in the past.
    void pump(uint32_t now) {
        if (!pendOn || (int32_t)(now - pendAt) < 0) return;
        const uint8_t want = pendWant;
        const uint32_t at = (int32_t)(now - pendAt) > 100 ? now : pendAt;
        const uint32_t drawMs = pendDraw, stowMs = pendStow;
        const float holdW = pendHoldW;
        target(want != 0, at, drawMs, stowMs, holdW);  // clears pendOn
    }

    // True while a targetDelayed() is armed (not yet fired, cancelled, or reset).
    bool pending() const { return pendOn != 0; }

    // Gap guard, call once per tick: a stall longer than 100 ms while a slide was running shifts t0 by the stall, so the slide
    // resumes from the fraction it had at the last call instead of finishing during the pause. An armed targetDelayed() shifts its
    // pendAt by the same stall, so a pause does not fire it early. last is updated on every call.
    void touch(uint32_t now) {
        const int32_t gap = (int32_t)(now - last);
        if (gap > 100 && len > 0 && (int32_t)(last - t0) < (int32_t)len) t0 += (uint32_t)gap;
        if (gap > 100 && pendOn) pendAt += (uint32_t)gap;
        last = now;
    }

    // True strictly between the two ends of a running slide.
    bool active(uint32_t now) const {
        if (len == 0) return false;
        const int32_t d = (int32_t)(now - t0);
        return d > 0 && d < (int32_t)len;
    }
};

// Dot of two quats. Named slideDot: HolsterReach.h already defines hs::qdot with the same meaning.
inline float slideDot(const Q& a, const Q& b) { return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z; }

// Nlerp helper (no sign flip inside: callers align the signs). The output is normalized.
inline Q slideNlerp(const Q& a, const Q& b, float w) {
    return qnormalize({ (1.f - w) * a.w + w * b.w, (1.f - w) * a.x + w * b.x, (1.f - w) * a.y + w * b.y, (1.f - w) * a.z + w * b.z });
}

// HW31 item 3, two legs: t < 0.5 -> a to via over 2t; t >= 0.5 -> via to b over 2t - 1. via is sign-aligned to a and b to via,
// so each leg takes the short arc. t is clamped (NaN -> 0). At t = 0.5 both legs give via (normalized).
inline Q slideViaQ(const Q& a, const Q& via0, const Q& b0, float t) {
    t = slideClamp01(t);
    const Q via = slideDot(a, via0) < 0.f ? Q{ -via0.w, -via0.x, -via0.y, -via0.z } : via0;
    if (t < 0.5f) return slideNlerp(a, via, 2.f * t);
    const Q b = slideDot(via, b0) < 0.f ? Q{ -b0.w, -b0.x, -b0.y, -b0.z } : b0;
    return slideNlerp(via, b, 2.f * t - 1.f);
}

// HW31 item 4. fPos drives the position, fRot the orientation. via (optional, must be finite) routes the orientation through the via quat.
// Hemisphere lock as blendPoseLocked. A non-finite via is treated as absent.
inline Pose blendPoseLocked2(Slide& s, const Pose& mount, const Pose& hand, float fPos, float fRot, const Q* via = nullptr) {
    const float fp = slideClamp01(fPos);
    const float fr = slideClamp01(fRot);
    Pose o;
    for (int i = 0; i < 3; ++i) o.pos[i] = (1.f - fp) * mount.pos[i] + fp * hand.pos[i];
    Q h = hand.q;
    const Q& m = mount.q;
    const bool flip = s.lockOn ? slideDot(s.lockH, h) < 0.f : m.w * h.w + m.x * h.x + m.y * h.y + m.z * h.z < 0.f;
    if (flip) { h.w = -h.w; h.x = -h.x; h.y = -h.y; h.z = -h.z; }
    if (std::isfinite(h.w) && std::isfinite(h.x) && std::isfinite(h.y) && std::isfinite(h.z)) {
        s.lockH = h;
        s.lockOn = 1;
    }
    o.q = (via && std::isfinite(via->w) && std::isfinite(via->x) && std::isfinite(via->y) && std::isfinite(via->z))
        ? slideViaQ(m, *via, h, fr) : slideNlerp(m, h, fr);
    return o;
}

// Locked blend (HW26): blendPose with the hand quat's sign tied to the previous call of the slide instead of the mount. Position lerp,
// f clamp and nlerp are those of blendPose. The first call after reset() picks the hemisphere nearest the mount, as blendPose does.
// Every later call negates h when slideDot(lockH, h) < 0, so the sign stays continuous while the hand turns past 180 deg from the
// mount. After the first call the blend may therefore take the long arc from the mount (dot(mount, h) < 0): that is the continuous
// path, not an error. The nlerp norm at f = 0.5 is |cos(turn/4)|, so the output degenerates as the hand nears 360 deg of turn.
// lockH is the post-flip h of every call with a finite h; a non-finite h leaves lockH and lockOn as they were.
// target() leaves lockOn and lockH alone, so a reversal keeps the hand's sign.
inline Pose blendPoseLocked(Slide& s, const Pose& mount, const Pose& hand, float f) {
    return blendPoseLocked2(s, mount, hand, f, f, nullptr);
}

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
