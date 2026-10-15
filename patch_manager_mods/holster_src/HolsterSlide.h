// HolsterSlide.h -- cosmetic draw/stow slide of the holstered-weapons mod (doc 42, P3): the weapon eases between the body mount pose
// and the hand pose while Fsm (HolsterLogic.h) reports the hand fraction. Pure math, header-only, no engine calls. The slide never
// gates the hand attach: Fsm::inHand() is already true on the combat edge; this file only shapes the pose. Host-tested by
// holster_slide_test.cpp.
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

// Slide pose at tick `now`: the mount at hand fraction 0, the hand at fraction 1, eased in between.
inline Pose slidePose(const Fsm& fsm, uint32_t now, const Pose& mount, const Pose& hand) {
    return blendPose(mount, hand, easeSmooth(fsm.handFraction(now)));
}

}  // namespace hs
