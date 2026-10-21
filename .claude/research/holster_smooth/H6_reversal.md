# H6: what a mid-slide reversal does today

Read-only recon. Claim tested: a reversal continues from the current hand pose in the mirror phase, and swap/death/cutscene snap to a safe end.

## Mechanics
- `Slide` (HolsterSlide.h:40-80) is one progress value per weapon entry (`ent->slide`, Holster.cpp:1685-1690). `target()` sets `from = frac(now)`, `t0 = now`, `len = ms * dist`, so fraction-space continuity holds and a half-way reversal takes half the time.
- `from` is a slide fraction (hip 0, hand 1). It is not a hand path position.
- `reachPlan` (HolsterReach.h:316-342) maps the slide to `pathF` (0 = mount, 1 = rest). `pathF` drives the arm (Holster.cpp:1753-1756).
- Stow branch reads `from` as if it were `pathF` (HolsterReach.h:337: `pathF = from * (1 - q)`). The two agree only when `g = 0`. Default `g_reachGrab = 0.30` (Holster.cpp:88).

## Per-case jump (g = 0.30, drawMs = stowMs = 400)
| Reversal | pathF just before | pathF first frame after | jump |
|---|---|---|---|
| draw reach, f = 0.02 | 0.99 (rest) | 0.02 (mount side) | ~0.97 (whole arc) |
| draw reach, f = 0.10 | 0.74 | 0.10 | ~0.64 |
| draw reach, f = 0.15 | 0.50 | 0.15 | ~0.35 |
| draw carry, p = 0.50 | 0.28 (`q = (p-g)/(1-g)`, line 328) | 0.50 | ~0.21 |
| stow carry, p = 0.49, then draw | 0.075 (carryW 0.84) | 0.125 (carryW 1.0) | ~0.05 |
| stow release, p = 0.85, then draw | 0.50 (carryW 0) | 0.15 (carryW 1) | ~0.35 |

- Max carry-phase jump is `g` (0.30) at p = g, shrinking to 0 at p = 1.
- Reach reversal: the hand does not restart at 0. It jumps to `pathF = f`, the slide fraction. Early reversals jump nearly the full arc.

## Answers
1. Slide: yes, fraction continues from the current value and time is proportional to remaining distance.
2. reachPlan: reach reversal enters stow carry at `pathF = f` (line 337), not at `1 - E(f/g)` (line 326). Carry reversal enters at `pathF = p`, not `q`.
3. Tests: no reach-phase reversal test exists. `testReachPlanPartialDraw` (holster_reach_test.cpp:618-631) reverses at t=1200, which is p = 0.5, i.e. carry, not reach. Its assertion (pathF in 0.4-0.5 at t=1210) accepts the ~0.18 jump from 0.28. Its comment says "continues from where the hand was". `testSlideRetargetContinuity` (holster_slide_test.cpp:142-156) checks fraction only.

## Swap / death / cutscene
- Already implemented. Owner change or `!readable` calls `slide.reset()` (Holster.cpp:1684-1686): silent snap to the target endpoint, `len = 0`, reachPlan inactive, no reach carry.

## Minimal fix
1. Pass the current path position into the retarget. At Holster.cpp:1690, compute `x = reachPlan(slide, now, g_reachGrab).pathF` when active, else the fraction. Store it as `pathFrom` in `Slide::target`. Stow and draw branches use `pathFrom` instead of `from`.
2. Reach reversal (draw, p < g) enters release from x: `pathF = x + (1 - x) * easeSmooth(p')`, `carryW = 0`. This mirrors the reach (carryW 0, weapon stays on the mount). Duration is the remaining path, not `g * len`.
3. Stow to draw: ramp `carryW` from `easeSmooth(x / 0.1)`, or accept a ~0.16 weight step at x = 0.075. Changing this breaks the `carryW == 1` assertion at holster_reach_test.cpp:630.
4. Tests: assert `|pathF(new, t_rev + 1) - pathF(old, t_rev)| < 0.01` and carryW step < 0.05 for reversals in reach, carry and release. Fix the `testReachPlanPartialDraw` expectation from 0.4-0.5 to about 0.28 before the reversal.
5. Nothing to add for swap/death/cutscene.

## Notes
- The claim's "one progress value per hand" is already true: `Slide` is one value per weapon entry. "Who holds the weapon" is implicit in `carryW` (no explicit state).
