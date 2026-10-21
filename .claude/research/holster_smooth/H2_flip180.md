# H2: 180 deg shortest-arc flip in holster slide and wrist

Verdict: PARTIAL. The flip mechanism is confirmed. The place_rot link is structural; the live hand-to-mount angle is unmeasured.

## Anchors
- `patch_manager_mods/holster_src/HolsterSlide.h:25-34` blendPose: sign test `dot(m,h) < 0` on every call (line 31). No state carried between frames.
- `patch_manager_mods/holster_src/HolsterReach.h:346-353` blendQ: same test (line 351). Used by reachWristDelta (line 401) and Holster.cpp:1337, 1347, 1362.
- `patch_manager_mods/holster_src/HolsterReach.h:385-395` reachClampRot: `dq.w < 0` negation (line 388). Its axis flips when target and current hand are about 180 deg apart.
- `patch_manager_mods/holster_src/Holster.cpp:1745` hipPose = applyPlacement(base, pl): place_rot is applied to the MOUNT side only. `in` (Holster.cpp:1722-1724, engine pose at Gob+0xA4) is not composed with place_rot.
- `patch_manager_mods/holster_src/HolsterQuat.h:47-52` applyPlacement: q = node.q * qbase(rot).

## Analysis
- The sign test is on 4D dot. dot = 0 is a relative rotation of exactly 180 deg. The comment at HolsterSlide.h:23 ("more than 90 deg") is the 4D angle, which is 180 deg of rotation.
- With m = B*P (P = qbase(place_rot)) and h = in: dot(m,h) = <P, B^-1 * in>. For P = R_x(180) and B^-1*in = R_x(d), the relative angle is 180 - d. So place_rot 180,0,0 puts the sign boundary exactly where the hand and mount bone frames agree (d = 0).
- The angle is not fixed at 180 because d is animated. The boundary is crossed whenever d changes sign.
- Second boundary: reachClampRot at dq.w = 0 flips the wrist-turn axis, a jump of about 2*maxDeg. Not measured.
- Only the mid-slide and carry (0 < f < 1) is affected. At f = 0 or 1 both candidates give the same result.

## Experiment
Scratch: `/tmp/claude-1000/h2/flip_check.cpp`, built with `g++ -std=c++17 -I patch_manager_mods/holster_src`.
- Sweep relative angle 178 to 182 deg about +X, f = 0.5. blendPose midpoint: +89.75 at 179.5, -90.00 at 180.0, -89.75 at 180.5. A jump of about 180 deg.
- Hemisphere-locked variant (compare to the previous locked hand quat): +89.75, +90.00, +90.25. Continuous.
- Jitter +-0.5 deg around 180 for 400 frames: blendPose 46 midpoint flips, locked 0.

## Minimal fix
- Carry a hemisphere lock per weapon Slide. Negate the fresh hand quat when dot(last_locked_hand, fresh) < 0, store it, and pass it to blendPose without the internal sign test.
- Apply at Holster.cpp:1494, 1500, 1747, 1758 (blendPose) and 1337, 1347, 1362 (blendQ). Keep the previous wrist dq sign in reachClampRot instead of forcing w >= 0.
- Add a jitter-around-180 sequence to holster_slide_test.cpp expecting 0 flips.

## Open
- Live dot(hipPose.q, in.q) during a slide has not been logged. Need one slide log to confirm the live angle reaches about 180.
