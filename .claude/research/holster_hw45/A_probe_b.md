# A probe (b): hysteresis band on the wrist pick

Verdict: REFUTED. No kBandDeg in {5, 10, 20} passes both gates. Probe only: no repo file changed, Step 3 not run.

Scratch: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/A_probe_b/` (harness `measure.cpp`, per-variant `HolsterReach.h` in `base/ b5/ b10/ b20/ bn5/ bn10/ bn20/`, diagnostics `diag.cpp`, `diag2.cpp`).

## Setup
- Sweep: guarded (hint = prevTw, guard = prevComb), raw 120 to 260 deg, 0.1 deg step, axes 0-2, caps 80 twist / 60 swing, foreAxis z.
- E = off(chosen) = angle(comb * conj(dqn)). E0 = off(candidate 0) at the same hint. "E>E0+1" counts steps where E > E0 + 1.
- Baseline reproduced exactly (1e-4 margin): axis0 E 149.61, E0 119.96, 266 steps, first 233.5 deg; axis1 144.69 / 119.94 / 236 / 236.5; axis2 131.30 / 119.96 / 123 / 247.8.

## Variants
- **Literal**: incumbent = candidate 0. score_i = angle(prev, comb_i) + angle(comb_i, dqn), in degrees. Candidate i replaces the incumbent only if score_i < best - kBandDeg. This replaces the `1e-4f` |dot| margin.
- **Nearest-incumbent (bn)**: incumbent = the candidate nearest prev, same band rule.

## Results
Gate (i) = max step < 5 deg on sw and comb. Gate E = 0 steps with E > E0+1.

| variant | kBandDeg | axis | max step sw | max step comb | max E | max E0 | steps E>E0+1 | gate (i) | gate E |
|---|---|---|---|---|---|---|---|---|---|
| baseline (1e-4) | - | 0 | 0.06 | 0.09 | 149.61 | 119.96 | 266 | PASS | FAIL |
| baseline (1e-4) | - | 1 | 0.06 | 0.09 | 144.69 | 119.94 | 236 | PASS | FAIL |
| baseline (1e-4) | - | 2 | 0.06 | 0.07 | 131.30 | 119.96 | 123 | PASS | FAIL |
| literal | 5 | 0 | 0.06 | 0.09 | 149.61 | 119.96 | 266 | PASS | FAIL |
| literal | 5 | 1 | 0.06 | 0.09 | 144.69 | 119.94 | 236 | PASS | FAIL |
| literal | 5 | 2 | 96.37 | 120.96 | 119.96 | 119.96 | 0 | FAIL | PASS |
| literal | 10 | 0-2 | same as literal 5 | | | | | FAIL | |
| literal | 20 | 0 | 90.72 | 150.42 | 146.85 | 119.96 | 249 | FAIL | FAIL |
| literal | 20 | 1 | 88.51 | 136.07 | 133.14 | 119.94 | 152 | FAIL | FAIL |
| literal | 20 | 2 | 96.37 | 120.96 | 119.96 | 119.96 | 0 | FAIL | PASS |
| nearest (bn) | 5, 10, 20 | 0 | 0.06 | 0.09 | 149.61 | 119.96 | 266 | PASS | FAIL |
| nearest (bn) | 5, 10, 20 | 1 | 0.06 | 0.09 | 144.69 | 119.94 | 236 | PASS | FAIL |
| nearest (bn) | 5, 10, 20 | 2 | 0.06 | 0.07 | 131.30 | 119.96 | 123 | PASS | FAIL |

- Lowest max E among variants that pass gate (i): 149.61 (axis0). That is the baseline value, so no gain.
- Lowest max E overall: 146.85 (literal, kBandDeg 20, axis0), but it fails gate (i) with a 90.72 deg sw step.
- The literal variant's step is a discrete switch between candidates about 120 deg apart, as the brief anticipated.

## Why (from diag.cpp, candidates given as prevDist / off)
1. **Literal band holds the wrong incumbent.** Axis2 at 247.8 deg: candidate 0 = 120.96 / 2.68 (score 123.64). The continuous candidate = 0.00 / 120.05 (score 120.05). The gap is 3.6 deg, below the band of 5, so candidate 0 is kept. That gives the 120.96 deg one-frame jump.
2. **Nearest-incumbent band never switches to the dq-faithful candidate.** Axis0 at 245.2 deg: incumbent = 0.00 / 130.08 (score 130.08). Faithful candidate = 135.29 / 24.94 (score 160.23). The prev-distance term dominates, so no band in {5, 10, 20} switches.
3. **Sum score is degenerate here.** Axis2 at 258.6 deg: faithful = 130.01 / 0.00 and continuous = 0.00 / 130.01, both scoring 130.01. The faithful candidate lies on the geodesic from prev to dqn, so angle(prev,c) + angle(c,dqn) equals angle(prev,dqn) for both. No band separates them.

Structural point: in the failure region candidates 0 and 2 sit about 120 to 135 deg from prev. Any switch to the faithful candidate is therefore a 120+ deg step. A band delays the switch but cannot make it continuous. The only way to pass gate (i) is to never switch, which leaves E at baseline. Mechanism (b) cannot pass both gates. Mechanism (a), a rate-limited move, or (c), one shared state, is needed.

## Best variant change (nearest-incumbent, passes gate (i), no E gain)
Replaces the pick block in `reachPickContinuous` (`patch_manager_mods/holster_src/HolsterReach.h:547-557`). Scratch copy: `bn5/HolsterReach.h:549-568`.

```cpp
constexpr float kBandDeg = 5.f;   // probe constant
...
    int best = 0;
    if (prev && qfinite(*prev)) {
        Q cb[4];
        float pd[4];
        for (int i = 0; i < 4; ++i) {
            cb[i] = qmul(cS[i], cT[i]);
            pd[i] = reachQAngleDeg(qmul(cb[i], reachConj(*prev)));
        }
        int inc = 0;
        for (int i = 1; i < 4; ++i) if (pd[i] < pd[inc]) inc = i;
        best = inc;
        float bestSc = pd[inc] + reachQAngleDeg(qmul(cb[inc], reachConj(dqn)));
        for (int i = 0; i < 4; ++i) {
            if (i == inc) continue;
            const float sc = pd[i] + reachQAngleDeg(qmul(cb[i], reachConj(dqn)));
            if (sc < bestSc - kBandDeg) { bestSc = sc; best = i; }
        }
    }
```

## Verdict
REFUTED. Mechanism (b) as specified fails both gates at every band tried. The literal form makes a 96 to 121 deg discrete jump on axis2 at 5 and 10 deg, and 90 to 96 deg jumps at 20 deg. The nearest-incumbent form passes gate (i) only by never leaving the baseline pick. Mechanism (a) or (c) is the next probe.
