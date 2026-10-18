# C. Axis geometry for the U4 gizmo (recon, Haiku)

Scope: frames, which axes each gesture rotates about, screen sign rule, tip length, pure helper signatures, test vectors. Read-only; no source edited. Numeric checks: `scratchpad/u4_axis_check.py` and `scratchpad/u4_sign_numeric.py` (python3 stdlib, not committed).

## 1. Frames (what GetMountWorld really returns)
- `GetMountWorld` returns the MOUNT BONE pose, not the weapon: `Holster.cpp:733` sets `g_editMount = base` (`base` = node pose, `Holster.cpp:730`). Anchor A = pos, B = q (`Holster.cpp:1002-1009`).
- Weapon origin (pivot, "+" marker): `W = A + qrotate(B, off)` (`HolsterQuat.h:47-52`, `pl.pos` rotated by node.q). `off` (metres, bone frame) comes from `GetEdit` (`HolsterApi.h:45`), not from GetMountWorld.
- Weapon orientation: `F = qmul(B, qbase(rot))` (`HolsterQuat.h:37`, `:47-52`). Equals `B * placeQ` after `overlaySync` (`HolsterPreview.h:118-122`).
- Bone axes (what NudgeLocal uses): `b_i = qrotate(B, e_i)`, e = x,y,z.
- Weapon axes: `f_i = qrotate(F, e_i)`.
- Tips: `T_i = W + L * b_i` (recommended) or `W + L * f_i`.

## 2. Which axes the gestures act on
- NudgeLocal(axis, deg): `placeQ' = qaxis(e_axis, deg) * placeQ` (`HolsterEdit.h:129-134`, `HolsterPreview.h:187-192`). So `F' = R(b_axis, deg) * F`: a right-hand rotation about the BONE axis b_axis through W (W unchanged).
- NudgeWorld(dq): `placeQ' = conj(B) dq B placeQ` (`HolsterEdit.h:116-118`, `HolsterPreview.h:168-174`), so `F' = dq * F`: rotation about a WORLD axis through W. A rotation about weapon axis f_i is `dq = qaxis(f_i, deg)`, equivalent to `F' = F * R(e_i, deg)`. No current gesture does this.
- Current gesture map (`HseView.h`): Depth = NudgeLocal axis 1 metres (`:601`); Tilt = axis 0, `deg = -fy*0.5` (`:615`); Roll = axis 1, `deg = -fy*0.5`; Turn = axis 2, `deg = +fx*0.5` (`:629-650`); Move = NudgeWorld dp `(0, dx*k, -dy*k)` (`:553-557`).
- Recommendation: draw the BONE axes `b_i` at W. Tilt, Roll, Turn and Depth then act on the drawn axis with no gesture code change. Weapon-axis markers differ from the gestures once rot != 0, so they need new NudgeWorld drag code.

## 3. Screen sign rule (pure)
- Inputs: `s = (sx, sy)` = projected tip minus projected W, UI px, y down. Mouse delta `d = (dx, dy)`, y down.
- Tangent: `t = (-sy, sx)`. Positive (right-hand) rotation about the axis iff `dot(d, t) > 0`.
- Degrees: `deg = kRotDegPerPx * dot(d, t) / |t|`, with kRotDegPerPx = 0.5 (`HseView.h:524`), x0.2 when fine. Return 0 if `|s| < 1e-6`.
- Derivation: worldToScreen has `R = R(q)X`, `U = R(q)Y`, `fwd = -R(q)Z` (`HseView.h:172-186`), so `R x U` = toward-viewer and the screen is right-handed. A near-side rim point moves with `a x B`, which projects to `(a_U, a_R) = (-sy, sx)`.
- Per-axis reading:
  - Horizontal drag (d = (dx,0)): sign = sign(dx) * sign(-sy). An axis pointing screen-up is turned by dragging right.
  - Vertical drag (d = (0,dy)): sign = sign(dy) * sign(sx). An axis pointing screen-right is turned by dragging down.
- Existing fixed signs agree with this rule only on some views: Turn (+fx) iff sy < 0 (bone z up on screen); Tilt and Roll (-fy) iff sx < 0. Otherwise the U4 change flips them. Add a play row.
- Reliability (20000 random proper cameras, depth 0.5-3 m, perspective, L 0.05-0.3 m): agreement with a numeric rim probe is 99.5% overall. By projected/orthographic length ratio: >=0.9 at 100%, 0.6-0.7 at 99.8%, 0.2-0.3 at 93-95%, <0.2 at 60-85%. Below 0.3, use the numeric probe or hide the axis.

## 4. Tip length and visibility
- `f_px = (ext.h / 2) / tan(fovy / 2)` (fovy vertical; `HseRect` x,y,w,h at `HseView.h:88-93`; formula from `:188-191`).
- Projected tip length ~ `L * f_px / z_W * sqrt(1 - (b . fwd)^2)`, with `z_W` = depth of W (`HseView.h:186`).
- Use pixel-constant tips: `L = clamp(56 * z_W / f_px, 0.002, 0.25)` m, giving about 56 px for a perpendicular axis.
- Hide an axis when projected length < 10 px (plan). Hover within 20 px (plan).
- Example, assuming z_W = 0.86 m (distance from the logged camera to the right-hand base mount; confirm in the U0 log), ext.h = 896: fovy 10.33 gives f_px 4953 and L 0.0097 m; fovy 21.6 gives f_px 2348 and L 0.0205 m. A fixed 0.10 m tip would be about 576 px at zoom 2.2.

## 5. Proposed pure helpers (header-only, next to worldToScreen in HseView.h)
```
void  axisTips(const float W[3], const float B[4] /*w-first*/, float L, float out[3][3]); // out[i] = W + L*qrotate(B, e_i)
int   rotSignFromScreen(float sx, float sy, float dx, float dy); // +1/-1/0; t=(-sy,sx); 0 if |s|==0 or dot==0
float rotDegFromScreen(float sx, float sy, float dx, float dy, float degPerPx); // degPerPx*dot(d,t)/|t|; 0 if |s|<1e-6
```

## 6. Test vectors (expected values from python3 stdlib, rounded to 4 dp)
- V1 axisTips(W=0, B=identity, L=0.1): x (0.1,0,0), y (0,0.1,0), z (0,0,0.1).
- V2 axisTips(W=(0.024,0.001,1.593), B=(0.70711,0,0,0.70711), L=0.05): x (0.024,0.051,1.593), y (-0.026,0.001,1.593), z (0.024,0.001,1.643).
- V3 axisTips(W=0, B=(0.70711,0.70711,0,0), L=1): x (1,0,0), y (0,0,1), z (0,-1,0).
- V4 axisTips(W=0, B=(0.5,0.5,0.5,0.5), L=2): x (0,2,0), y (0,0,2), z (2,0,0).
- V5 rotSignFromScreen(s=(1,0), d=(0,1)) = +1.
- V6 rotSignFromScreen(s=(0,-1), d=(1,0)) = +1.
- V7 rotSignFromScreen(s=(0.6,0.8), d=(0.8,-0.6)) = -1.
- V8 rotDegFromScreen(s=(3,4), d=(-4,3), degPerPx=0.5) = 2.5.
- Extra: rotSign(s=(1,0), d=(1,0)) = 0; rotSign(s=(0,0), d=(1,2)) = 0; axisTips with L=0 gives three copies of W; axisTips(W=(0.5,-0.5,1), B=qbase(30,-20,45)=(0.861642,0.299673,-0.057422,0.40555), L=0.25) gives x (0.6661,-0.3339,1.0855), y (0.3167,-0.3771,1.1175), z (0.5360,-0.6407,1.2034).

## 7. Open items and risks
- Camera basis is unresolved. `worldToScreen` (`HseView.h:172`) uses fwd = R(q)(0,0,-1). With the logged q = (0.7071,0,0,0.7071) that looks straight down -Z, which is implausible. Sign and tangent math hold for any proper basis; only that basis line changes. Confirm with a probe that projects A and W.
- `moveDp` (`HseView.h:553-557`) assumes camera right = +Y and up = +Z. With the logged q, R(q)Y = -X, so vertical Move may be mis-mapped. Check in the same probe.
- Hidden edit: when `GetEdit.hidden == 1`, off/rot are not the world placement. Skip markers then. Reading the effective pose needs a new provider slot, which changes the HolsterApi ABI pinned by `common_contract_test.cpp`.
- Probe to run first: log CAMHOOK and project A and W; pass if W lands on the drawn weapon within about 10% of view size.
