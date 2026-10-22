# A probe, mechanism (c): one shared roll/pick state

Scope: Steps 1-2 only (probe and acceptance). No repo file edited. Step 3 not done.
Scratch: /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/A_probe_c/ (HolsterReach.h variant copy behind HW45C_MODE / HW45C_ROLL_STEP_DEG; measure.cpp, window.cpp, cut.cpp, diag.cpp).

## Baseline (reproduced, current code)

Guarded sweep, raw 120 to 260 deg, 0.1 deg steps. Matches the card.

| variant | axis | max step sw | max step comb | max E | max E0 | steps E > E0+1 |
|---|---|---|---|---|---|---|
| current | 0 | 0.06 | 0.09 | 149.61 | 119.96 | 266 |
| current | 1 | 0.06 | 0.09 | 144.69 | 119.94 | 236 |
| current | 2 | 0.06 | 0.07 | 131.30 | 119.96 | 123 |

## Mechanism (c) variants

- Mode 1 = literal. Roll moves from prevTw toward the hint-aligned target by at most kRollStepDeg (rolled with reachClampRot on T * conj(P)). Swing = reachClampRot(dqn * conj(rolled twist), 60, nullptr).
- Mode 2 = mode 1, with the swing sign hinted by prev comb * conj(prevTw).
- The exact branch (Release B1) is unchanged in both modes.

| mode | budget deg | axis | max step sw | max step comb | max E | max E0 | steps E > E0+1 |
|---|---|---|---|---|---|---|---|
| 1 | 1 | 0 | 120.00 | 120.70 | 119.96 | 119.96 | 148 |
| 1 | 1 | 1 | 120.00 | 120.74 | 119.94 | 119.94 | 149 |
| 1 | 1 | 2 | 120.00 | 120.87 | 119.96 | 119.96 | 123 |
| 1 | 2 | 0 | 120.00 | 121.41 | 119.96 | 119.96 | 76 |
| 1 | 2 | 1 | 120.00 | 121.48 | 119.94 | 119.94 | 76 |
| 1 | 2 | 2 | 120.00 | 121.74 | 119.96 | 119.96 | 74 |
| 1 | 4 | 0 | 119.99 | 122.83 | 119.96 | 119.96 | 38 |
| 1 | 4 | 1 | 119.99 | 122.97 | 119.94 | 119.94 | 38 |
| 1 | 4 | 2 | 120.00 | 123.48 | 119.96 | 119.96 | 39 |
| 2 | 1 | 0 | 120.00 | 120.61 | 179.99 | 119.96 | 148 |
| 2 | 1 | 1 | 120.00 | 120.66 | 179.90 | 119.94 | 149 |
| 2 | 1 | 2 | 120.00 | 120.83 | 179.17 | 119.96 | 123 |
| 2 | 2 | 0 | 120.00 | 121.18 | 179.97 | 119.96 | 76 |
| 2 | 2 | 1 | 120.00 | 121.29 | 179.01 | 119.94 | 76 |
| 2 | 2 | 2 | 120.00 | 121.65 | 179.60 | 119.96 | 74 |
| 2 | 4 | 0 | 119.98 | 122.36 | 177.88 | 119.96 | 38 |
| 2 | 4 | 1 | 119.98 | 122.57 | 178.40 | 119.94 | 38 |
| 2 | 4 | 2 | 119.99 | 123.29 | 179.66 | 119.96 | 39 |

Max E for mode 1 ties the E0 floor only because the peak sits where E0 itself peaks. The gap is where E0 drops.

## Best variant: mode 1, kRollStepDeg = 4 deg. Fails both gates.

Fewest gap steps (38/38/39) and the lowest max E of the set. Code change:

```cpp
inline constexpr float kRollStepDeg = 4.0f;
inline void reachOneState(const Q& dqn, Q& twist, Q& swing, float swingMaxDeg, const Q* twistHint) {
    Q tw = twist;   // on entry: dq-faithful target (reachClampRot with hint)
    if (twistHint && qfinite(*twistHint) && qfinite(twist)) {
        const Q d = reachClampRot(qmul(twist, reachConj(*twistHint)), kRollStepDeg, nullptr);
        tw = qnormalize(qmul(d, *twistHint));
    }
    twist = tw;
    swing = reachClampRot(qmul(dqn, reachConj(tw)), swingMaxDeg, nullptr);
}
// reachSplitWrist, after the unchanged exact branch, replaces the reachPickContinuous call:
//     reachOneState(dqn, twistOut, swingOut, swingMaxDeg, twistHint);
//     if (overrode) *overrode = false;
```

## Gate results

- **Gate (i), step < 5 deg on sw and comb: FAIL.** Every mode-1 budget has one 120.00 deg swing step per axis, at axis0 233.5, axis1 236.5, axis2 247.8 (budgets 1 and 4 both). Mechanism: at that step the rolled twist is still on the +80 side, and its natural swing's w changes sign at geometric 180. The shorter-arc clamp then flips the swing. Candidate 0 avoids this only because its own target twist flips from +80 to -80 deg about z in the same step (comb0 step 130.26 deg at axis0 233.5, measured in cut.cpp).
- **Gate (ii), E <= E0+1 at every step: FAIL.** E0 drops from about 117 to about 33 at the target flip. The rolled twist must cover about 160 deg to reach -80. Gap steps equal about 152/k (148, 76, 38 at k = 1, 2, 4 on axis0; the gap window is [233.5, 248.2] at k=1 and [233.5, 237.2] at k=4). Zero gap steps needs k of about 150 deg per call, which is itself a 150 deg change in twist and swing. That fails gate (i), so no budget in this family passes both gates.
- **Mode 2 (swing sign hinted from prev comb):** moves the sign flip (to 235.6 / 238.5 / 249.5 at k=4) and makes max E worse (177.9 to 179.7). It does not remove the jump.

## Notes on the card

- The card's 120 deg step at 180.7 deg came from a 2-deg roll variant that kept the discrete pick. This probe removes the pick, so the step lands at 233.5 for axis0. The two runs are different designs, so the locations are not comparable.
- The baseline pick avoids the step only by switching twist sides, which mechanism (c) cannot do with a rate limit.

## Verdict

REFUTED for mechanism (c) as specified (rate-limited roll toward the hint-aligned dq-faithful twist, swing from the rolled twist). No budget in {1, 2, 4} passes both gates. Open for the Opus decision: a target that starts leaving the +80 side before the raw roll passes 180 deg (while E0 is still high) is untested and is a different design.
