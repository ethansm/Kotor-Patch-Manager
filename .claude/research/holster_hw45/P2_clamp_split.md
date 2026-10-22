# P2 (HW45): clamp once, then split

Scope: recon only. No repo file edited. Scratch: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/P2/` (`HolsterReach.h` copy with macros `HW45P2_MODE` 0-3 and `HW45P2_LIMIT`; `measure_p2.cpp`).
Build: `g++ -std=c++17 -O1 -DHW45P2_MODE=<m> -I <scratch>/P2 -I patch_manager_mods/holster_src -I patch_manager_mods/_shared_src -I tests/include measure_p2.cpp`
Metrics: raw roll 120 -> 260, 0.1 deg. E = off(chosen) vs dqn, E0 = off(cand 0). Gate i: step sw and comb < 5. Gate ii: 0 steps with E > E0+1.

## Baseline (mode 0), reproduced

| axis | step sw | step comb | max E | max E0 | E>E0+1 | E>E0+10 |
|---|---|---|---|---|---|---|
| 0 | 0.06 | 0.09 | 149.61 | 119.96 | 266 | 266 |
| 1 | 0.06 | 0.09 | 144.69 | 119.94 | 236 | 236 |
| 2 | 0.06 | 0.07 | 131.30 | 119.96 | 123 | 123 |

## Variants (limit 140 = 80 + 60)

| variant | axis | step sw | step comb | max E | max E0 | E>E0+1 | E>E0+10 |
|---|---|---|---|---|---|---|---|
| 1 plain clamp (shorter arc) | 0 | 25.02 | 25.02 | 149.61 | 119.96 | 524 | 266 |
| 1 | 1 | 24.71 | 24.71 | 144.69 | 119.94 | 495 | 236 |
| 1 | 2 | 22.30 | 22.30 | 131.30 | 119.96 | 366 | 123 |
| 2 hemisphere clamp (sign from combPrev) | 0 | 0.06 | 0.06 | 153.62 | 119.96 | 986 | 266 |
| 2 | 1 | 0.06 | 0.06 | 150.99 | 119.94 | 992 | 236 |
| 2 | 2 | 0.06 | 0.07 | 139.68 | 119.96 | 1010 | 123 |
| 3 clamp only above 140 (hint) | 0-2 | 25.02 / 24.71 / 22.30 | same | 149.61 / 144.69 / 131.30 | same | 851 / 827 / 732 | same as 1 |

Max swing 60.00 and max twist 80.00 in every variant (caps hold).
## Limit sweep (axis 0 only; no limit passes both gates)

| limit deg | mode 1 step sw | mode 1 max E | mode 2 step sw | mode 2 max E |
|---|---|---|---|---|
| 60 | 88.83 | 120.00 | 88.83 | 179.90 |
| 80 | 51.81 | 163.28 | 51.81 | 179.95 |
| 100 | 49.41 | 149.61 | 0.00 (frozen) | 170.23 |
| 120 | 39.16 | 149.61 | 0.00 (frozen) | 159.72 |
| 130 | 31.89 | 149.61 | 0.06 | 156.35 |
| 140 | 25.02 | 149.61 | 0.06 | 153.62 |

Every variant with step < 5 has max E above baseline 149.61.

## Why it fails

1. Plain clamp (mode 1, and mode 3 above 140): the clamp sign follows the shorter arc, which flips at 180 deg. The comb jumps 22-25 deg at the clamp onset (about 161 deg), where the unclamped split inside the caps is replaced by a 140 target.
2. Hemisphere clamp (mode 2): the clamp output then depends only on the axis, not on the angle. Once the input passes the limit, the target saturates. At limit 100-120 it is frozen (step 0.00) and E grows toward 260 deg. At limit 140 it tracks, but E > E0+1 holds for 986 steps, against 266 at baseline.
3. The 140 ball is loose. For axis 0 (z-component 0.6), the twist cap alone limits a pure rotation to 2*atan(tan40/0.6), about 109 deg. The clamp target is therefore not reachable, and the split still clips to the caps.

## Best code (passes gate i only, for reference)

```cpp
// reachSplitWrist: replace `const Q dqn = qnormalize(dq);`
const Q dqn = reachClampRot(dq, 140.f, combPrev);   // clamp to 80 + 60, sign follows previous combined rotation
```

Not an improvement: max E 153.62 vs baseline 149.61, and 986 steps with E > E0+1 vs 266.

## Verdict

REFUTED. No clamp-once variant passes both gates. Plain clamp fails gate i (25 deg step). Hemisphere clamp passes gate i but fails gate ii and raises max E. Drop clamp-once.

OPEN: Does a per-frame projection onto the true reachable set (twist <= 80 and swing <= 60, not an angle clamp) keep both gates? Not tested.
