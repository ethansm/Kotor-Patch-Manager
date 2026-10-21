# B: carry and wrist claims (holster draw / stow)

Scope: `patch_manager_mods/holster_src/HolsterReach.h`, `Holster.cpp` (reachArm, HolsterPlace), `HolsterSlide.h`. Read-only.

Gate: `holster.ini:113` ships `reach_test=0`, so the whole carry/wrist path is off by default. Doc 42 (`patch_manager_mods/42_holster_mounts_and_weapon_types.md:123`) says live is `reach_test=1`. Findings below assume that.

Defaults checked: `reach_wrist_pct` 1.0 and `reach_wrist_max_deg` 90 (`Holster.cpp:91`); `holster.ini:128-129` = 100 / 90. `draw_slide_ms` = `stow_slide_ms` = 350 (`Holster.cpp:72-73`). `reach_grab_pct` = 30 (`Holster.cpp:88`).

## Claims

| claim | verdict | anchor | what the code does |
|---|---|---|---|
| B1 carry walks Reach's path in reverse, same T(f), no extraction segment | PARTIAL | `HolsterReach.h:326-340`; `:199-206` (reachCarryTarget), `:242` (reachHinge); `Holster.cpp:1318-1325` | Geometry is shared: reach is pathF 1→0 and draw carry is 0→1 through the same `T(f)` / hinge. No extraction: during reach `carryW=0` (`:326`), so the weapon stays on the mount. Timing is not mirrored: reach uses `easeSmooth(p/g)` (`:326`), draw carry is linear in q (`:327-328`, `:331`), stow carry is linear (`:337`), stow release is eased (`:340`). pathF also has a velocity kink at the grab. |
| B2 during carry the weapon rides the solved hand, grab switches mount→hand | TRUE | `Holster.cpp:1757-1759` (world), `:1498-1500` (preview), `:1369-1372` (carried); `HolsterReach.h:328` | Weapon = `blendPose(hipPose, carried, carryW)`, where `carried` = attach node (`n`, the hand) pose read after the arm writes. The switch is not a hard step: `carryW = easeSmooth(q/0.1)` is 0 at the grab and 1 at 10% of carry. |
| B3 wrist default 90 / 100%; delta vs which rotations; after shoulder/elbow; wrote==3, order | TRUE | `Holster.cpp:91`; `:1339` (bicep, `wrote|=1`); `:1348` (forearm, `|=2`); `:1358` (gate `wrote==3`); `:1362` (delta); `:1364` (write, `|=4`); `HolsterReach.h:399-404` | Order: bicep → forearm → wrist (3 writes). `from` = mount quat (`hipPose.q` in world, `out.q` in preview). `to` = pH.q, the hand read before this frame's arm writes (`:1292`). `cur` = pHn.q, the hand after bicep and forearm writes (`:1361`). `dq = clamp(blend(from,to,easeSmooth(pathF)) * conj(cur), max)`, applied relative to the post-IK hand via `reachDeltaToLocal` (`HolsterReach.h:173-176`). Wrist also needs `e->wrist` (`:1261-1263`) and `w*pct > 0`. |
| B4 >90 deg mount-vs-hand gap snaps across the remaining gap at the grab | PARTIAL | `Holster.cpp:1362`, `:1757-1759`; `HolsterSlide.h:25-33`; `HolsterReach.h:328` | Weapon does follow the capped hand during carry, so the wrist cap is real. At the grab itself there is no value jump: `carryW=0` writes `hipPose.q` exactly. But the excess `G-90` (G = angle(hipPose.q, hand at grab)) swings in the first 10% of carry, about 24.5 ms at 350 ms (`0.7*350*0.1`), roughly 1-2 frames at 60 fps. The hand also turns during reach (wrist gate has no reach exclusion, `:1358`) while the weapon sits on the mount, so the hand/weapon mismatch is visible in reach. |
| B5 wrist slerp is linear in pathF across the carry | FALSE | `HolsterReach.h:401` (`blendQ(..., easeSmooth(t))`); `:346-353` (nlerp, not slerp); `HolsterSlide.h:18-21` | Wrist blend parameter is smoothstep of pathF (slow at both ends, fast middle). The blend is normalized lerp, not slerp. Only pathF is linear in the carry (`:328`, `:337`); the wrist is not. |

## Snap sources (pose can jump between frames)

1. `HolsterReach.h:331-332` + `Holster.cpp:1753-1756`: a draw that starts from a partial fraction (`from > 0.001`, e.g. reversing a stow mid-release) sets `carryW=1` and `pathF = from + (1-from)*p` in one frame. The weapon jumps from mount to hand, and the arm target jumps from the eased pathF to the linear slide fraction.
2. `Holster.cpp:1757-1759` vs `:1747/1750`: whenever `reachArm` returns false for a frame, the weapon falls back from `rid` to `out = blend(hip, in, easeSmooth(slideF))`. Causes: `w==0` on the first tick after a retarget (`HolsterReach.h:183-187`), the undo branch (`Holster.cpp:1280-1284`), refusals (`:1251-1293`), or `carriedOk` false (`:1369-1372`).
3. `HolsterReach.h:328` and `:337`: `carryW` ramps 0→1 over the first 10% of draw carry (and 1→0 over the last 10% of stow carry). This is the `G-90` swing in about 24.5 ms. Fast, but continuous.
4. `Holster.cpp:1281-1284`: the w==0 undo restores the cached bone base for one frame when `persist && dirty`, so the arm flickers to the animation pose at each w==0 frame.

## Gaps to build

1. Extend `holster_src/holster_reach_test.cpp` (reachPlan tests at ~`:574-590` only check pathF/carryW). Add a continuity test: step the written weapon quat across the grab and across a reversal, and assert a max per-frame angle.
2. Fix the reversal snap: carry the current pathF and carryW into the new slide (`HolsterReach.h:331-332`) instead of `from = slideF`.
3. Remove the in-carry jump of the weapon: keep carryW at 0 until the wrist has taken the excess (or allow wrist > 90 during reach), so `G-90` is closed on the hand before the weapon follows.
4. Replace the w==0 fallback to `out` with holding the last `rid` for that frame (`Holster.cpp:1500`, `:1757-1759`).
5. Update the RCH1 pass signal in `patch_manager_mods/PLAY_TEST_QUEUE.md`: the log line prints `wrote` after bit 4 is set (`Holster.cpp:1364`, `:1374-1379`), so expect `wrote=7`, not `wrote=3`, once the wrist applies.

## OPEN

Does the attach node's world pose (`carried`) equal the engine's native in-hand pose (`in`) at draw end? `Holster.cpp:1757` assumes it ("the carried hand IS the engine hand"). If not, the weapon steps at slide end.
