# HW45 P1: look-ahead twist ramp (recon, no repo edit)

Harness (scratch): `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/P1/la/la.cpp`
Build: `g++ -std=c++17 -O1 -I <scratch>/P1/la -I patch_manager_mods/holster_src -I patch_manager_mods/_shared_src -I tests/include -o la la.cpp`; run `./la <S_deg> <E_deg> <swMode>`.
Baseline mode 3 (repo `reachSplitWrist`) reproduces the card: axis0 max E 149.61, E0 119.96, steps 0.06/0.09.

## Idea tested
Twist target T(U), U = unwrapped roll (deg): T = U up to 80; 80 until S; linear 80 to -80 on [S, E]; -80 on [E, 280]; U-360 after.
E = 260 (the baseline flip). Swing = clamp60(dqn * conj(T)). Stateless in U. Ramp slope = 160/(E-S) twist deg per roll deg.

## Table (axes 0-2, raw 120 to 260, 0.1 deg; E0 = candidate 0 chained on its own twist)
| variant | max E (all axes) | max step sw / comb | steps E>E0+1 (axis0) | steps E>E0+10 (axis0) | gate step | gate E |
|---|---|---|---|---|---|---|
| baseline (repo) | 149.61 | 0.06 / 0.09 | 266 | 266 | PASS | FAIL |
| ramp S=180 | 179.94 | 0.26 / 0.42 | 795 | 745 | PASS | FAIL |
| ramp S=200 | 179.98 | 0.32 / 0.53 | 676 | 640 | PASS | FAIL |
| ramp S=220 | 179.98 | 0.45 / 0.77 | 551 | 528 | PASS | FAIL |
| ramp S=240 | 179.98 | 0.84 / 1.46 | 417 | 405 | PASS | FAIL |
| ramp S=250 | 179.93 | 1.60 / 2.83 | 344 | 339 | PASS | FAIL |
| ramp S=255 | 179.97 | 3.10 / 5.55 | 306 | 304 | FAIL | FAIL |
| no ramp (flip at 260, continuous swing) | 170.02 | 65.3 / 154.7 | 267 | 267 | FAIL | FAIL |

Swing hint: hinting the clamp with the previous OUTPUT (prev sw) aligns the input to the wrong side once the clamp binds. It gave a spurious 120 deg step at raw 227.7 (S=240). Hinting with the previous aligned UNCLAMPED input removes it; all ramp rows use that.

Relaxed gate (max E, steps with E > E0+10): best look-ahead 179.9 with 339 steps (S=250). Baseline 149.6 with 266 steps. Look-ahead is worse on both.

## Best code change (for the record, not recommended)
```cpp
// reachSplitWrist: replaces the twistOut clamp + pick for the ramp. kRampStartDeg = S, kRampEndDeg = E (roll units).
const float U = 2.f * std::atan2(twistRaw.z, twistRaw.w) * 57.29577951308232f;   // twistRaw.w >= 0; axis = foreAxis
const float Un = U < 0.f ? U + 360.f : U;
const float T = Un <= 80.f ? Un : Un <= kRampStartDeg ? 80.f : Un <= kRampEndDeg ? 80.f - 160.f * (Un - kRampStartDeg) / (kRampEndDeg - kRampStartDeg) : Un <= 280.f ? -80.f : Un - 360.f;
twistOut = (cos(T/2), sin(T/2) * unit(foreAxis));                // quaternion about foreAxis
swingOut = reachClampRot(qmul(dqn, reachConj(twistOut)), swingMaxDeg, &prevUnclampedAligned);
```

## Why no ramp passes both gates
1. The clamped swing must sit on one side: side +n while w > 0, side -n once w < 0, where w = cos(th/2)cos(T/2) + (a.z) sin(th/2) sin(T/2) for the unfolded dq * conj(twist).
2. Gate E needs +n at raw 120 (candidate 0 E = 28; the -n side is about 150) and -n at raw 233.5 (candidate 0 E = 33; the +n side is about 150).
3. Axis0: w(120,+80) = +0.72 and w(233.5,-80) = -0.69; axis1 and axis2 have the same signs. Any twist path from one to the other crosses w = 0.
4. The two sides are 120 deg apart. At w = 0 only caps of about 10 deg around +-n meet E <= E0+1, so no continuous switch exists.
5. Result: a 120 deg swing step (gate step fails), or a continuous swing that keeps the -n side too early so E runs to 180 (gate E fails). The ramp only moves the jump and trades it for E.

## Verdict
REFUTED. No look-ahead variant passes both gates. Best look-ahead (S=240 or S=250) passes the step gate at max E 180 versus baseline 149.6. Do not implement. HW45 stays OPEN.
Context: `.claude/waves/w2-42c-HW45b/A_probe_a.md` already showed that the swing cap is the only blocker for a2 (target dqn, uncapped swing: max E 8-28, steps 1.0).

OPEN: Is dropping the 60 deg swing cap acceptable (A probe a2 meets both literal gates), or must the cap stay and a one-frame ~120 deg wrist step near raw 233.5 be accepted?
