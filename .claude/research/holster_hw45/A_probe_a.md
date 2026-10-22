# Task A probe, mechanism (a): glide toward candidate 0 (and a2: toward dqn)

Scope: Steps 1-2 only (probe + acceptance). No repo file edited. Step 3 not run.
Scratch: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/A_probe_a/` (`HolsterReach.h` probe copy with macros `HW45_MODE`, `HW45_GLIDE_DEG`, `HW45_TWIST_CONT`; `measure2.cpp`; `diag.cpp`, `diag2.cpp`).
Build: `g++ -std=c++17 -O1 -I <scratch> -I patch_manager_mods/holster_src -I patch_manager_mods/_shared_src -I tests/include measure2.cpp`.
Baseline reproduced exactly (axis0 max E 149.61, max E0 119.96, 266 steps, first 233.5 deg).

## Results (guarded sweep, raw 120 -> 260, 0.1 deg; E = off(chosen), E0 = off(cand 0))

Columns: max step sw / max step comb (gate i needs both < 5), max E, max E0, steps with E > E0+1 (gate ii needs 0), max swing angle (cap 60).

| variant | axis | step sw | step comb | max E | max E0 | steps E>E0+1 | max swing deg |
|---|---|---|---|---|---|---|---|
| baseline (current) | 0 | 0.06 | 0.09 | 149.61 | 119.96 | 266 | 60.00 |
| baseline (current) | 1 | 0.06 | 0.09 | 144.69 | 119.94 | 236 | 60.00 |
| baseline (current) | 2 | 0.06 | 0.07 | 131.30 | 119.96 | 123 | 60.00 |
| a, k=1 | 0 | 160.85 | 1.00 | 119.96 | 119.96 | 120 | 156.74 |
| a, k=1 | 1 | 160.86 | 1.00 | 119.94 | 119.94 | 122 | 154.65 |
| a, k=1 | 2 | 160.88 | 1.00 | 119.96 | 119.96 | 123 | 147.02 |
| a, k=2 | 0 | 161.71 | 2.00 | 119.96 | 119.96 | 55 | 155.86 |
| a, k=2 | 1 | 161.72 | 2.00 | 119.94 | 119.94 | 57 | 153.77 |
| a, k=2 | 2 | 161.76 | 2.00 | 119.96 | 119.96 | 62 | 146.15 |
| a, k=4 | 0 | 163.42 | 4.00 | 119.96 | 119.96 | 26 | 154.10 |
| a, k=4 | 1 | 163.44 | 4.00 | 119.94 | 119.94 | 27 | 152.01 |
| a, k=4 | 2 | 163.51 | 4.00 | 119.96 | 119.96 | 30 | 144.41 |
| a2 (toward dqn), k=1 | 0 | 160.06 | 1.00 | 28.38 | 119.96 | 0 | 179.96 |
| a2, k=1 | 1 | 160.06 | 1.00 | 24.66 | 119.94 | 0 | 179.94 |
| a2, k=1 | 2 | 160.08 | 1.00 | 8.14 | 119.96 | 0 | 179.96 |
| a2, k=2 / k=4 | 0-2 | 160.06-160.08 | 2.00 / 4.00 | same as k=1 | same | 0 | 179.94-179.96 |
| DIAG a2 + roll continuity, k=1 | 0 | 1.00 | 1.00 | 28.38 | 119.96 | 0 | 179.96 |
| DIAG a2 + roll continuity, k=1 | 1 | 1.00 | 1.00 | 24.66 | 119.94 | 0 | 179.98 |
| DIAG a2 + roll continuity, k=1 | 2 | 1.00 | 1.00 | 8.14 | 119.96 | 0 | 179.96 |
| DIAG a2 + roll continuity, k=2 / k=4 | 0-2 | 2.00 / 4.00 | 2.00 / 4.00 | same | same | 0 | ~180 |

The DIAG rows are not a card candidate. They add one line that picks +roll or conj(roll), whichever is nearer the previous roll (`twistHint`).

## Why mechanism (a) fails

1. The roll jumps at the flip. Candidate 0's own roll (`twistOut`) jumps 160.00 deg between raw 233.4 and 233.5 on axis 0. The combined rotation stays continuous (comb step 2.00 at k=2), so the swing `comb * conj(roll)` takes the whole jump: sw goes 60 -> 156 in one step. This is the sw gate failure, and it does not depend on k.
2. The E gate is impossible for (a) at any k < 86 deg. Pre-flip, the glide tracks candidate 0, so E(233.4) = E0(233.4) = 119.96. At 233.5, E0 drops to 32.87 (candidate 0's own sign flip). Gate (ii) then needs E(233.5) <= 33.9, but the comb moves at most k per step, so E(233.5) >= 119.96 - k >= 115.
3. a2 (target dqn) gives E near 0 with zero gate-ii violations, but it fails gate (i) on sw (160 deg). Its swing is unclamped and reaches about 180 deg against the 60 deg cap.
4. a2 with roll continuity passes both literal gates (sw and comb steps = k <= 4 < 5; zero E violations; max E 28.38 / 24.66 / 8.14, which is the first-step candidate-0 value). It fails the swing cap (about 180 deg vs 60). The cap is not in the card's gate list.

## Best variant code (a2 + roll continuity; lowest max E, passes gates (i) and (ii) literally, violates the swing cap)

```cpp
// reachSplitWrist, just before `const bool picked = reachPickContinuous(...)`:
if (twistHint && qfinite(*twistHint) && std::fabs(qdot(*twistHint, reachConj(twistOut))) > std::fabs(qdot(*twistHint, twistOut))) twistOut = reachConj(twistOut);

// reachPickContinuous, after the cT/cS build (prev non-null branch only; kGlideDeg = 1, 2 or 4):
constexpr float kGlideDeg = 1.f;
const Q rel = qmul(dqn, reachConj(*prev));                       // a: rel = qmul(cS[0] * cT[0], reachConj(*prev))
const Q written = qnormalize(qmul(reachClampRot(rel, kGlideDeg, nullptr), *prev));
twist = cT[0];
swing = qnormalize(qmul(written, reachConj(twist)));
return false;
```

## Verdict

- Mechanism (a) as specified: fails gate (i) on sw at every k (160-163 deg step) and gate (ii) at every k (26-123 steps). REFUTED.
- a2 as specified: fails gate (i) on sw (160 deg). REFUTED.
- a2 + roll continuity (diagnostic): passes gates (i) and (ii) literally, but the swing reaches about 180 deg against the 60 deg cap. Not a valid winner without a cap-respecting clamp, which is a new mechanism and was not tested.
- Target choice matters: dqn gives low E; candidate 0 cannot satisfy E <= E0+1 under any budget.
- Gate (i) passes for no variant as specified, so the card's winner rule (lowest max E among gate-(i) passes) selects nothing. Step 3 not run, no repo change.

OPEN: must the 60 deg swing cap hold as a gate? If yes, a2 is out and a cap-respecting mechanism (b or c) is needed. If no, a2 + roll continuity is the lowest-E candidate (max E 8-28 deg).
