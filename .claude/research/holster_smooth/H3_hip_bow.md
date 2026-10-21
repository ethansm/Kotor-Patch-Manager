# H3 hip bow (recon, read-only)

## Verdict per claim
- H3a bow zero at hip: CONFIRMED. "Scales with height": PARTIAL, it is a smoothstep ramp, not linear. "Cuts through abdomen": UNSURE, no torso clearance or collision code exists; visual result not checked live.
- H3b pelvis_g +0.20 x = right hip: PARTIAL. Mirror logic supports +x = right, but bone-local to body-x alignment is unverified. The ini `bone_name=pelvis_g` (ini line 10) sits under `bone_test=1`, so it may be a probe key, not the production mount bone.
- H3c chooseMount unwired: CONFIRMED (conditional). No production caller; test-only (holster_logic_test.cpp:87-106). Arm side comes from the weapon's hand node, not the mount side.

## Bow formula
- dz = mount z minus shoulder z, body frame (Holster.cpp:1303-1306). arm = |E-S| + |H-E| (Holster.cpp:1308).
- over = clamp01(easeSmooth((dz/arm + 0.75) / 0.3)): 0 at dz/arm <= -0.75, 1 at dz/arm >= -0.45 (HolsterReach.h:139-142).
- Back mount forces over = 1 at any height (Holster.cpp:1310).
- Bow offset = arm * over * sin(pi*f) * bulge{up,fwd,out}, bulge per category (HolsterReach.h:127-134, 199-206).
- Doc anchors (HolsterReach.h:137-138): hip ~0.8 arm below shoulder (over 0, straight line), upper back ~0.1 (full), mid-back rifle ~0.5 (near full).

## Code facts
- chooseMount order: same zone same side, other zone same side, same zone other side, other zone other side (HolsterLogic.h:135-142). Cross-body spill only when both same-side mounts are taken.
- mirrorOff negates x; mirrorBone swaps only rthigh_g and lthigh_g; pelvis_g is not swapped (HolsterTune.h:282-283, 55).
- HolsterLogic.h:122-125 labels pelvis_g the "waist-level alternative" for a hip mount; pelvis is shared by both sides.
- reachArm `left` comes from the hand node (Holster.cpp:1623, 1244), not the mount side. No cross-body branch in HolsterReach.h or Holster.cpp.

OPEN: Does the production hip mount use pelvis_g or rthigh_g/lthigh_g, and does ini `bone_name=pelvis_g` set it?
