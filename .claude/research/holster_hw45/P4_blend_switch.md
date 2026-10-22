# P4 blend switch (cross-fade) on the HW45 wrist pick

Probe (scratch, repo untouched): `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/P4/p4.cpp`; output `.../P4/p4_results_0-360.txt`.
Baseline reproduced: axis0 E 149.61, E0 119.96. Method: 3 axes, raw roll swept at frame step d, dt 16.7 ms. Span = sum over axes of (steps with E > E0+1) x d, raw deg. The task window 120-260 misses the largest baseline step (267-283), so tables use 0-360.

## 1. Baseline (no fade), 0-360
| d deg/frame | max step sw | max step comb | max E | span E>E0+1 (deg) |
|---|---|---|---|---|
| 0.1 | 97.8 | 175.6 | 180.0 (279) | 110.6 |
| 1 | 97.6 | 175.1 | 179.9 | 110.0 |
| 3 | 96.7 | 173.7 | 179.9 | 111.0 |
| 6 | 95.3 | 176.4 | 175.4 | 114.0 |
- The error persists at every d; it is not a sampling artefact.
- Over 120-260, comb excess over the input step is 0.00 at every d. A fixed 5 deg/call gate is not frame-rate free (d = 6 input is 6 deg/call).

## 2. Cross-fade (kSwitch 5, twist and swing faded separately, caps 80/60)
| variant | d | max step sw | max step comb | max E (deg) | span E>E0+1 | span E>E0+10 |
|---|---|---|---|---|---|---|
| fade N=5 | 0.1 | 21.2 | 52.2 | 160.0 (183) | 1.2 | 1.2 |
| fade N=5 | 3 | 21.2 | 52.2 | 165.3 | 45 | 36 |
| fade N=5 | 6 | 21.1 | 52.1 | 168.1 | 84 | 54 |
| fade N=20 | 0.1 | 5.3 | 13.1 | 176.6 (184) | 5.5 | 5.1 |
| fade N=20 | 1 | 5.3 | 13.1 | 179.4 | 64 | 44 |
| fade N=20 | 3 | 119.9 | 171.0 | 178.6 | 300 | 255 |
| fade N=20 | 6 | 78.0 | 136.6 | 179.7 | 534 | 480 |
- At 0.1 deg the N=20 span looks 20x better than baseline. That is sampling: the bad window lasts N calls, so its raw width is N x d. At d >= 3, N=20 is worse than baseline.
- Per-call step: twist 160/N, swing ~100/N, independent of d. The 5 deg/call gate needs N ~ 52.
- d = 0.1, N=40: comb step 6.5, E 178.7, span 11.1. N=80: step 3.3, E 179.8, span 24.6.
- Other (d = 0.1): hold cand 0 gives E = E0 (span 0) but a 130.3 step at 233.5. Comb-space fade N=20 keeps E near E0 (118.7) but its landing jumps 120. Fade only on cand-0 jump: E 178.5 at 234.

## 3. Mechanism
- Cand 0 (twist +80) and cand 2 (twist -80) differ by a twist sign flip. Any fade passes zero twist, where the comb is the swing alone: E ~ dq + swing = 176-180 at raw 183-234 (dq 126-177 deg).
- The E-min switch (0 to 2) fires at 181-229 for kSwitch 1-80. Threshold is not the lever.
- Cand 0's own chain snaps at 233.5 (comb step 130). The canonical split jumps 97-120 deg even with no fade (N=1).

## 4. Time-based gate and acceptance (proposal)
- Use deg/ms with dt = elapsed ms: 5 deg/call = 0.30 deg/ms at 60 fps, 0.15 at 30 fps. Fade rates at 60 fps: N=20 0.78 deg/ms, N=40 0.39, N ~ 52 needed for 0.30.
- Gate 1 (E): E <= E0 + 1 on every sample, raw 0-360, at d = 0.1 / 1 / 3 / 6. Baseline FAIL (span 110, peak 180).
- Gate 2 (window): contiguous E > E0+10 <= 2 deg raw. Baseline FAIL. Fade N=20 at d >= 1 FAIL.
- Gate 3 (rate): max |dsw|/dt and |dcomb|/dt <= max(0.30 deg/ms, input rate + 0.06 deg/ms) at dt 16.7 and 33.3 ms. Baseline PASS (excess 0.00). Fade N <= 40 FAIL.
- A fix must pass gates 1 and 3. Gate 2 is the closure test for the far-side snap.

## 5. Best code (cross-fade core, probe only)
```cpp
// candidates use cand-0 history as hints; cur = candidate index, re-derived each call
if (!fading && c[cur].e - c[best].e > kSwitch) { cur = best; fading = true; j = 0; fromTw = pTw; fromSw = pSw; }
if (fading) {
    const float f = std::min(1.f, ++j / float(N));
    tw = reachClampRot(slerp(fromTw, c[cur].t, f), 80.f, have ? &pTw : nullptr);
    sw = reachClampRot(slerp(fromSw, c[cur].s, f), 60.f, nullptr);
    if (f >= 1.f) fading = false;
} else { tw = c[cur].t; sw = c[cur].s; }
```

## Verdict
REFUTED. The cross-fade fails gate 1 at every N and gate 3 for N <= 40. At 0.1 deg it looks partial, but that is sampling.

## Open
Does a continuous comb-space writer (twist chained from cand 0) keep mid-fade E near E0? Untested: the canonical writer alone jumps 97-120 deg.
