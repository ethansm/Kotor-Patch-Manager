# H3 timing and easing: heavy draw / stow (HW50)

Recon, read-only. Assumes built-in heavy 570 / 700 ms, grab 45%, slow_holster_x=1. Numbers come from a Python replica of reachPlan + carryShownBase (scratchpad h3_timing*.py), not a host test run.

## 1. Easing functions (every category shares all of them)

| Function | Formula | Defined | Used by |
|---|---|---|---|
| easeSmooth | 3t^2 - 2t^3, zero slope both ends | HolsterSlide.h:19 | reach pathF (HolsterReach.h:425), carryW (:429, :441), oriF join (:449), extract decay (:450), residual (:646), stow k (Holster.cpp:1972), plain blend (Holster.cpp:2318, :2656) |
| reachCarryF | q<=0.15: v*L*(x^3 - x^4/2), x=q/L; q>0.15: v*(L/2+q-L); L=0.15, v=1.081 | HolsterReach.h:386 | draw pathF (:429, :433), stow pathF (:440), oriF hold X (:401) |
| reachOriF | 0 while pathF <= X=reachCarryF(e); then easeSmooth((pathF-X)/(1-X)) | HolsterReach.h:399 | oriF (:448) |
| extractBump | 16x^2(1-x)^2 over q in [0,e] | HolsterReach.h:394 | draw :430, stow :442 |
| residual unwind | blend(identity, Rres, 1-easeSmooth(t/ms)); ms = min(slide time left, reach_resid_ms) | HolsterReach.h:644; Holster.cpp:1953 | fresh-draw carry (ResidDraw) |
| stow landing | k = 1-easeSmooth(pathF/0.15) | HolsterSlide.h:288; Holster.cpp:1963-1964 | stow carry |
| slide clock | linear, no ease | HolsterSlide.h:80 | every reader |
| via bump | 16s^2(1-s)^2, linear s | HolsterReach.h:666 | reach_via_heavy=1 (holster.ini:188) |

Only these are per category: draw / stow ms and grab % (HolsterTune.h:715-722, slideResolve :750), plus exit, hinge, bulge, via-out and pole. The curves above are global.

Note: HolsterReach.h:385 says the carry has "zero speed at both ends". Only the mount end (q=0) is zero. At the hand end the slope is v = 1.081, so the draw stops dead.

## 2. Heavy draw velocity (570 ms; reach 0-256 ms, carry 256-570 ms; residual 240 ms = 77% of carry)

- 0-256 ms: the hand travels rest -> mount on easeSmooth. The weapon stays on the mount (carryW = 0). Hand moves first.
- Hand path pathF 10/50/90% at 310 / 426 / 542 ms. Hand speed: 0 at 256 ms, plateau 0.00345/ms from about 303 ms to 570 ms, then stops dead.
- oriF 10/50/90% at 369 / 445 / 521 ms (back mount, extract e=0.20). Hip (e=0.10): 345 / 430 / 516 ms. Rotation median trails translation median by about 20 ms only.
- Extract lift peak: 288 ms (back) / 272 ms (hip).
- Weapon catches the hand (residual 50% gone): 377 ms, which is 66% of the draw. Pistol (300 ms, residual 210 ms = whole carry): 195 ms, which is 65%. Same proportion.
- The visible weapon rotation and offset both follow the 240 ms residual, not oriF. oriF mostly moves the hand wrist.

Heavy and pistol use identical curves. The differences are draw duration (1.9x), grab (45% vs 30%, i.e. 256 ms vs 90 ms of hand-only reach) and the residual cap.

## 3. Heavy stow (700 ms; carry 0-385 ms, release 385-700 ms)

- Hand speed at t=0 is full cruise (0.0028/ms). No slow-in at the hand end.
- Turn starts at 143 ms (37%) while the weapon is still in the hand (reach_stow_rot_f=0.6).
- Weapon leaves the hand and lands 303-385 ms. The hand arrives with slow-out (zero speed at the mount).

## 4. Changes that convey weight, heavy only (ranked)

1. CODE, about 12 lines. Use easeSmooth(q) for heavy in place of reachCarryF(q) at HolsterReach.h:429, :433, :440 and in the X of :401. Result: zero speed at both ends, mid-draw speed +39% (0.00345 -> 0.00478/ms), path median 426 -> 414 ms. Needs a per-slide flag, because reachPlan has no category. Set it at Holster.cpp:2543, :2584, :3112 and read it in reachPlan.
2. INI-ONLY: stow_slide_ms_heavy=900 (built-in 700; max 1000 at HolsterSlide.h:226). Lowering is 29% longer and 22% slower, same shape. The draw cannot exceed 570: draw_slide_ms_heavy=700 is clamped to 570 with a cfgWarn (HolsterSlide.h:220).
3. INI-ONLY: reach_exit_len_heavy=0.25 (range 0-0.5 m). The heavy lifts straight out of the mount before the turn, which reads as anticipation. Needs reach_test=1, reach_carry=1 and an extract share above 0 (default). Table rows: carbine 0.25, pistol 0.12; heavy-model rows not checked.
4. INI-ONLY: reach_resid_ms=300 (global, default 240, max 600 at Holster.cpp:599). Heavy catch-up moves 377 -> 407 ms, so the weapon trails the hand longer. Side effect: rifle, melee and qstaff draws too. Pistol is unchanged (carry 210 ms). Draw only.
5. INI-ONLY: reach_stow_rot_f=0.3 (global, default 0.6). Heavy stow turn starts at 250 ms instead of 143 ms and lasts 135 ms instead of 242 ms. The weapon stays level in the hand and turns near the mount. Side effect: every category's stow turn.

Not proposed: overshoot or follow-through. Heavy objects overshoot less, and the code has none. Keep it that way.

## 5. Open items

- The template ships reach_test=0 (holster.ini:143). Retarget with grab, extract and residual needs reach_test=1 and reach_carry=1 (Holster.cpp:2584). If the live config is reach_test=0, heavy is just easeSmooth over a linear clock: the duration is the only difference. Live value is not in the repo, so confirm it.
- Back vs hip topology is decided at runtime (HolsterReach.h:292-298). The numbers above give both.
