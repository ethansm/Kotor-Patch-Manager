# HW25 items 1+4 recon (r1_retarget)

Read-only, HEAD 21be17da. R = HolsterReach.h, S = HolsterSlide.h, H = Holster.cpp, T = holster_reach_test.cpp.
E(x) = easeSmooth (S:18). g = grab share (H:93, clamped to 0.6 at R:320). p = elapsed / len.

## 1. Current formulas (R:316-342)
Draw (dir 1):
- reach, from<=0.001 and g>0, p<g (R:326): pathF = 1 - E(p/g); carryW = 0
- carry after reach (R:327-328): q = (p-g)/(1-g); pathF = q; carryW = E(q/0.10)
- partial draw, from>0.001 (R:331): pathF = from + (1-from)*p; carryW = 1
Stow (dir 0), c = 1-g:
- carry (R:336-337): q = p/c; pathF = from*(1-q); carryW = E(pathF/0.10)
- release (R:340): pathF = E((p-c)/g); carryW = 0
restMix = 0 on every branch. R:337 ramps carryW on pathF, not on time: a stow from from=0.2 ramps over the last 50% of its carry.

## 2. Retarget state and the jump
- S keeps from (= frac(now), S:68, S:76), t0, len (= ms * frac-distance, S:69-79), dir. No path position is stored.
- R reads `from` as a PATH position (R:331, R:337). frac and pathF disagree in reach: frac rises 0 to 1 while pathF falls 1 to 0.
- Hand calc, g=0.3, 400 ms slides, pathF/carryW just before vs just after the reversal:

| reversal | pathF before | pathF after (now) | jump | carryW before -> after |
|---|---|---|---|---|
| draw reach f=0.02 | 0.987 | 0.020 | -0.967 | 0 -> 0.10 |
| draw reach f=0.10 | 0.741 | 0.100 | -0.641 | 0 -> 1.00 |
| draw reach f=0.15 | 0.500 | 0.150 | -0.350 | 0 -> 1.00 |
| draw carry p=0.5 | 0.286 | 0.500 | +0.214 | 1 -> 1 |

These match 42c master plan row 49. Stow release p=0.85 to draw: pathF 0.500 to 0.150, carryW 0 to 1.
Second defect: after a reach reversal the stow carry is frac-timed (len = stowMs * f), so its arm speed is (pathF before)/f times normal: 49x, 7.4x, 3.3x.

## 3. Proposed design
Storage: Slide gets `pathFrom` (default 1) and `carryFrom` (reset S:47-53 sets it to hand ? 1 : 0).
Filled by a free function in R, `reachRetarget(Slide&, hand, now, drawMs, stowMs, g)`: old = active ? reachPlan(s, now, g) : {pathF 1, carryW = dir}; then s.target(...); then store old.pathF and old.carryW. It must be a free function because reachPlan needs g and S cannot include R. H:1791 and H:2217 switch to it. Slide::target is unchanged.

Formulas. P0 = pathFrom, C0 = carryFrom, L = 0.15, v = 1/(1-L/2) = 1.081.
F(q) = v*L*(x^3 - x^4/2) with x = q/L for q<=L, else v*(L/2 + q - L). F is 0 to 1 with zero speed at q=0.
- draw reach (fresh, g>0): pathF = P0*(1-E(p/g)); carryW = 0
- draw carry: fresh q=(p-g)/(1-g) with start 0; partial q=p with start P0. pathF = start + (1-start)*F(q); carryW = C0' + (1-C0')*E(q/L), where C0' = 0 after a reach, else C0
- stow carry (q = p/c): pathF = P0*F(1-q); carryW = C0*E((1-q)/L)
- stow release: unchanged
Total len unchanged. pathF hits P0, 0, 1 and carryW hits C0, 1, 0 exactly; step at reversal = 0.
Speed at grab (draw): pathF slope is 1/(1-g) = 1.43 now (42c row 34). New slope is 0 at the grab (ratio 0 < 0.3 of linear), plateau 1.081 per q (1.54 per slide unit). Reach-to-carry join is 0 to 0.
carryW: smoothstep over 15% of carry, slope 0 at start, peak 1.5x linear.

OPEN duration: Option A keeps len frac-timed. The 0.01 step test at t+1 ms fails at f=0.02 (0.987 swing in 5.6 ms). Option B sets len = ms*(P0+1)/2 (stow) or ms*(1-P0)/2 (partial draw) in the wrapper only. Carry speed vs normal: 0.99x (f=.02), 0.85x (f=.10), 0.67x (f=.15), 0.37x (carry p=.5). Frac timing changes too.

## 4. Test expectations that change (T; retargets must go through reachRetarget)
- T:585 draw c.pathF 0.5 -> 0.460
- T:595 draw z.pathF 0.5 -> 0.460
- T:605 stow b.pathF 0.3 -> 0.243
- T:622-624 partial a: sl.target bypasses pathFrom; expect ~0.210 (old 0.40-0.50)
- T:629-630 partial b: carryW == 1 -> ~0.09 (ramp from C0 over 15%)
Unchanged: T:581, T:583, T:587, T:591-593, T:603-604, T:607, T:609, T:611-613, T:615.
New tests: reversal continuity (f .02/.10/.15, carry p .5), endpoints, speed at grab < 0.3.
