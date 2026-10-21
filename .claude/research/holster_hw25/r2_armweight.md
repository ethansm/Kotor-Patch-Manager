# HW25 item 2: arm weight across a Slide retarget (recon r2)

Read-only. Anchors are line numbers in patch_manager_mods/holster_src/ at HEAD.

## 1. Formula and the zero frame
- reachWeightRamp(s, now, ramp) (HolsterReach.h:180-188): r = min(ramp, 0.5); p = (now - t0) / len (signed diff); w = min(p/r, (1-p)/r), capped at 1; return easeSmooth(w). Returns 0 if `!s.active(now)` or ramp <= 1e-3.
- Inputs: Slide (t0, len), now, ramp = 0.04 in carry mode, 0.2 otherwise. g_reachCarry defaults to true (Holster.cpp:88).
- Read sites: Holster.cpp:1581 (editor), Holster.cpp:1855 (world).
- Zero cause: target() sets t0 = now (HolsterSlide.h:78). active() needs d > 0 (HolsterSlide.h:94). At the retarget tick d = 0, so active is false and HolsterReach.h:181 returns 0. The ramp restarts from 0 whatever the old weight was.
- Effect: blendQ(a, b, 0) returns the animation base (HolsterReach.h:348), so reachArm writes the animation pose for that tick.
- Same gate in reachPlan (HolsterReach.h:318) returns carryW = 0 and active = false. In carry mode that also switches reachArm to non-carry (Holster.cpp:1857, `g_reachCarry && pl.active`) and drops the weapon to mount (Holster.cpp:1873, carryW 0).
- Caveat: retarget (Holster.cpp:1791) and the weight read (1855) call slideNowMs() separately. The zero is exact only when both fall in the same ms.

## 2. Stored state
- struct ReachEnt (Holster.cpp:1269-1274): parent, owner, bic, fore, hand, wrist, poseFn, bb/fb/wb (hs::Base: q, wrote, have, HolsterReach.h:357-361), dirty, refused, left, refusedAt.
- No weight field. The Base caches hold orientations, not blend weight. No last weight is stored anywhere.
- ReachEnt is keyed by (parent, left) (Holster.cpp:1326-1329), not by slide.
- Time state lives in hs::Slide (HolsterSlide.h:40-44): Ent::slide (Holster.cpp:1662, per weapon slot) and g_prevSlide (Holster.cpp:1490, editor).
- Recommend w0 in Slide: the weight depends on that slide's timing, and reachArm already takes w as an argument.

## 3. Minimal design
- Slide: add `float w0 = 0;`. reset() sets w0 = 0 (HolsterSlide.h:47-53).
- target(hand, now, drawMs, stowMs, float holdW = 0.f): after the no-op early return (HolsterSlide.h:67), set w0 = slideClamp01(holdW). Default keeps existing callers valid.
- reachWeightRamp keeps its signature. Gate becomes d < 0 || d >= len. In the carry window (up < 1, i.e. d < r*len) return 1 - (1 - s.w0) * (1 - g), where g is the existing value. Else return g.
- Why it is continuous: in the window g = easeSmooth(d/(r*len)) and g reaches 1 at the window end. The blend starts at w0 (d = 0, no pop), stays <= 1, and joins the current curve at d = r*len. w0 = 0 reproduces the current curve.
- Normal end: window is at most len/2, so carry is gone before the slide ends. d >= len still returns 0. Stale w0 is never read and is overwritten by the next target().
- Call sites that must change:
  1. Holster.cpp:1791 (world retarget): pass hs::reachWeightRamp(ent->slide, now, g_reachCarry ? 0.04f : 0.2f) as holdW. Arguments evaluate before the body, so this reads the old slide. touch() at 1790 already runs first.
  2. Holster.cpp:2217 (apiSetPreviewDraw, g_prevSlide): same pattern. No touch runs before it today (pre-existing, Holster.cpp:2212-2217).
  - Unchanged: reads at Holster.cpp:1581 and 1855 (signature kept).
  - reachPlan (HolsterReach.h:318) is the open item below.
- Example: draw 1000-1400, stow retarget at 1200 (w = 1, frac 0.5). New len 200, window 40 ms. Weight stays 1.0 over 1200-1240, since g equals the window fraction and 1-(1-1)(1-g) = 1.

## 4. Tests
- New, in holster_reach_test.cpp near testReachWeightRamp (534-544):
  1. Reversal at plateau: draw 1000+400, retarget stow at 1200 with holdW 1.0. Expect 1.0 at 1200, 1210, 1220, 1240; 0 at 1400.
  2. Mid-ramp: draw, w(1020) = 0.15625 (ramp 0.2). Retarget stow at 1020 (len 20). Expect w(1020) = 0.15625, w(1021) = 0.2881, all <= 1.
  3. Fresh start: holdW 0. Expect w(t0) = 0, w(t0+200) = 1.
  4. Bounds: w0 = 1, ramp 0.5. Every tick in [t0, t0+len) lies in [0, 1].
  5. NaN holdW gives w0 = 0 and weight 0 at t0.
  6. Same-direction target is a no-op: weight unchanged.
  7. reset() after a reversal: weight 0 on the reset tick.
  8. ms = 0 retarget (len 0): weight 0 at every tick.
  9. Wrap: t0 = 0xFFFFFF00, holdW 0.8. Expect w(t0) = 0.8.
- Existing tests that change: none by assertion.
  - holster_reach_test.cpp:326-364: no target() call, w0 = 0.
  - holster_reach_test.cpp:538-543: reset + target from rest, holdW 0, unchanged.
  - holster_slide_test.cpp target() calls: default argument.
  - holster_reach_test.cpp:579 (`!reachPlan(sl, 1000).active`) changes only if the reachPlan gate changes.

## 5. Notes
- Weight drop alone does not fix carry mode. reachPlan's gate also drops carryW to 0 and carry to false for that tick (see FINDING). Fixing only the weight leaves a weapon and arm-mode pop.
