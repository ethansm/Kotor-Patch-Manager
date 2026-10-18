# E: camera model inferred from existing holster-editor logs (U4)

Sources: `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/holster-editor_log.txt.1` (26 CAM/CAMHOOK/PROJ lines, 01:23 session) and `holster-editor_log.txt` (10 lines, 01:58). Parsed with `scripts/kplog_parse.py --bare | rg`. Scratch scripts only in the session scratchpad.

Coverage: yaw 180 (zoom 2.2 and 1.052, panZ 0), yaw 179 (zoom 1.052, panZ 0 and -0.18). No yaw 179 at zoom 2.2. Only one 1-degree yaw step exists.

Tag caveat: `live` = first LIVE tick after entry; `settled` = logged after the view is steady for 500 ms (HolsterEditor.cpp:2042, 2049-2051, 2080-2081). Hand (PROJ) nodes differ between tags at an identical camera state (R mount y 0.443 live vs 0.331 settled at yaw 180, zoom 2.2). Hand pose is therefore state- and time-dependent, not static.

## 1. What yaw and panZ do to the camera node (hook)

- Hook angle theta = 2*atan2(qz, qw) = yaw - 90.00 deg at all 10 CAMHOOK samples. Orientation turns 1:1 with creature facing.
- Hook position per yaw degree: dx = 0, dy = -0.001 m (rounding limit). Orbit radius about the origin must be <= 0.057 m. The hook sits at xy radius 0.024 m, so the node turns in place. It does not orbit.
- panZ -0.18: hook z 1.593 -> 1.411 (dz = -0.182). Hand dz: R -0.183, L -0.182. Hook xy unchanged.
- Zoom 2.2 -> 1.052: hook pos and q identical. Only fovy changes: fovy*zoom = 22.726 vs 22.720, so fovy ~ 22.726/zoom (Preview3dProvider.cpp:69, 381).

Verdict (1): the camera node is rigidly carried by the creature transform. Yaw rotates it about world Z by the same angle, with no movement. Pan translates it with the body. It is not fixed in the world (S1.md Q1 claim refuted) and it does not orbit.

## 2. Same frame as the hand nodes?

- Live pair, hand minus hook: yaw 180 R (0.008, 0.442, -0.744), L (0.007, -0.448, -0.722). Yaw 179 with panZ -0.18: R (0.016, 0.443, -0.745), L (-0.001, -0.448, -0.722).
- A rigid -1 deg turn of the yaw-180 vector predicts R (0.0157, 0.4418, -0.744) and L (-0.0008, -0.4481, -0.722). Residuals <= 0.0012 m.
- Settled samples drift 0.02-0.08 m across states and do not fit a rigid turn, so they are not used for this test.

Verdict (2): yes. Hook and hand nodes share one frame and move as one rigid body under yaw and pan. The body mesh's parent is not in the logs.

## 3. Projection model and camera distance

- The logged PROJ values come from camera at hook with fwd = R(q)*(0,0,-1) (HseView.h:171-172; HolsterEditor.cpp:661, 670). A re-implementation reproduces all 9 non-mount PROJ samples within 5.5 px (s) and exactly on depth.
- Because q is a pure Z rotation, node -Z stays world -Z at every yaw. The model therefore points the camera straight down, with right = +Y and up = -X. That is why the hands land at ndc ~ +/-10 (s.x 4579, -1443 at zoom 2.2). The first model is wrong by construction, independent of the numbers.
- Hook-to-hand distance: 0.81-0.92 m across all samples. A camera at the hook would show a visible height of 0.13-0.17 m at zoom 2.2, not the 0.5-0.6 m reported. The camera is not at the hook.
- Camera distance from the reported extent (fovy vertical, tan(fovy/2) = 0.09039 at zoom 2.2; D = (H/2)/tan(fovy/2)):
  - H = 0.50 m gives D = 2.77 m
  - H = 0.55 m gives D = 3.04 m
  - H = 0.60 m gives D = 3.32 m
  - Range: D = 2.8-3.3 m. This is the camera-to-subject distance. Its offset from the hook is not logged.
- Same D at zoom 1.052 (tan(10.80 deg) = 0.19073): visible H = 1.05-1.27 m, ratio 2.11 to zoom 2.2.
- Width at zoom 2.2: 0.32-0.39 m (aspect 576/896 = 0.643). Half-width 0.16-0.19 m. Hands at lateral +/-0.44 m are off-frame at zoom 2.2. At zoom 1.052 (half-width 0.34-0.41 m) they are still off-frame if centred on the body axis.
- Vertical check: a horizontal view centred on the hook (z = 1.593) gives a zoom-2.2 window of z 1.29-1.89 m. That excludes the hands (z 0.67-0.87) and the hips to upper-back band reported by the user (about 0.9-1.45 m for a standing human, assumed). Either the view centre sits about 0.4 m below the hook (a camera-internal offset or pitch not in the logged node pose, cf. S1.md residual (c)) or the band estimate is off. Logs cannot decide.

## Screenshot-independent constraints on D

- Zoom does not move the camera. D is not a function of zoom and is not in the hook pose.
- The camera origin is not the hook. Hook-to-hand is 0.8-0.9 m, while the 0.5-0.6 m extent needs 2.8-3.3 m.
- The logged hook quaternion has no pitch.

## Upright-camera candidates (not resolved)

- Node +Y = creature facing; node +X = creature right (checked against q at yaw 180).
- Upright camera requires up = node +Z.
- Candidate A: fwd = +Y (looks along facing, sees the back). Candidate B: fwd = -Y (sees the front). Logs cannot choose.

## Contradictions with S1.md (for the lead)

- S1.md:4-7 "camera is fixed, creature turns": refuted by the hook q and pos (sections 1-2).
- S1.md:7 "right = +Y, up = +Z": under S1's own formula (S1.md:14) the logged q gives up = -X.
- S1.md:8 "same family as inv3d": inv3d fwd = -X needs a Y tilt under that formula. The holster hook is a pure Z rotation.
- HolsterEditor.cpp:2080 comment "the camera itself is not yawed" is contradicted by the log.

## Implications for U4

- Markers must use the live hook pose every tick. A constant camerahook pose is wrong.
- Because camera and body share a transform, yaw and pan may leave the on-screen body unchanged. This is a prediction, not a result.
- A camera model needs: camera origin = hook + unlogged offset (~3 m along the view axis) and a chosen view axis (candidate A or B).

## OPEN (one question)

Does the on-screen body move when yaw changes? One play row at zoom 2.2, yaw 180 vs 170, decides whether the camera model or the mesh parent is wrong.
