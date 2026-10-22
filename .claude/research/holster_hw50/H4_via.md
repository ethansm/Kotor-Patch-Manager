# H4: via quantification, heavy blaster, back mount (HW50)

Read-only recon. Scratch harness only: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/H4/` (`sweep_trace.cpp` = copy of `holster_sweep_test.cpp` with heavy timings; `main_trace{,2,3}.cpp`; binaries `trace`, `trace2`, `trace3`). No repo file edited.

## Method
- Carry model from `holster_sweep_test.cpp` (reachResidualVia / slideViaProgress / carryBlend, the same calls as Holster.cpp).
- Family `w_hvrptbltr_` (and `w_rptnblstr_`, identical numbers): shape axis y, exitSign 0 (no extract lift).
- Heavy timings: draw 570 ms, stow 700 ms, grab 0.45 (runtime built-ins). Right hand, back mount (kBack).
- Back mount quat is the GUESS (long axis at `deg` from +z toward +x). The live heavy quat is unmeasured.
- Clearance = sweep GUESS proxies (head sphere, torso capsule). Hinge = sweep's generic reachHinge (no heavy outT).

## Mechanism (code)
- `reachViaFor` (Holster.cpp:1564-1572): via = arcQuat(mount-rotated long axis, world up) * mount quat.
- `reachResidualVia` (HolsterReach.h:653-660): Rres -> via -> identity in two legs. Leg 1 is s < 0.5, leg 2 is s >= 0.5.
- The residual window is `kReachResidMs` = 240 ms (HolsterReach.h:643), not draw_slide_ms. The turn lasts 240 ms whatever the draw length.
- Bump = outM * 16 s^2 (1-s)^2 (HolsterReach.h:668-672). It peaks at s = 0.5, when the weapon is at the via (long axis vertical).
- reach_via_out_m_<cat> = 0 only removes the bump. The via rotation stays (viaS = 0, Holster.cpp:2323-2331).

## Via on vs off, draw, back mount (net rotation is 90 deg in every case)
| back deg | off: path / peak deg/s | on: path / peak deg/s | extra path |
|---|---|---|---|
| 0  | 89.9 / 593 | 90.0 / 1107 | +0   |
| 30 | 89.8 / 593 | 123.7 / 1157 | +34 |
| 45 (GUESS) | 90.0 / 593 | 143.3 / 1217 | +53 |
| 60 | 89.9 / 593 | 164.4 / 1298 | +74 |
| 90 | 89.9 / 593 | 210.0 / 1515 | +120 |

- Via adds rotation at every angle. It detours through "long axis up" (at mid-turn the weapon sits on the via orientation).
- Peak turn speed roughly doubles (x1.9 to x2.6). The via splits the turn into two 120 ms legs instead of one 240 ms leg.

## Outward offset curve (deg 45, draw, sim clock from slide start)
- 274 ms: 0.008 m. 343 ms: 0.102 m. 377 ms: 0.120 m (peak, path 45 deg = via). 411 ms: 0.102 m. 480 ms: 0.008 m. 514 ms: 0.
- Weapon lateral shift equals the bump (matches to 4 dp).

## Ini variants (heavy, deg 45 GUESS, draw)
| variant | path | peak deg/s | head min | torso min |
|---|---|---|---|---|
| baseline (via on, outM 0.12) | 143.3 | 1217 | 0.098 | 0.051 |
| reach_via_heavy=0 | 90.0 | 593 | 0.075 | 0.046 |
| outM 0 (via on) | 143.3 | 1217 | 0.112 | 0.051 |
| outM 0.04 (via on) | 143.3 | 1217 | 0.115 | 0.051 |
Stow (min head / torso): baseline -0.047 / -0.082; via off -0.057 / -0.087; outM 0 -0.088 / -0.117; outM 0.04 -0.079 / -0.105.
Stow at deg 0: via on +0.003 / -0.025, via off -0.088 / -0.090.
Stow length 900 ms (via off): peak still 593. draw_slide_ms clamps to 0..570 (holster.ini:99), so a longer draw is not reachable.

## Proposed ini settings (ranked)
1. `reach_via_heavy=0` (holster.ini:188). Expect path 143 -> 90 deg, peak 1217 -> 593 deg/s, bump gone. Cost: stow clips more at deg 0 (+0.003 -> -0.088). Gate: SM-L15 plus a sweep on the measured mount.
2. `reach_via_out_m_heavy=0.04` (new per-cat key, pattern of holster.ini:193). Expect lateral peak 0.12 -> 0.04 m, rotation unchanged. Not a swing fix. Costs stow head clearance (-0.047 -> -0.079).
3. `stow_slide_ms_heavy` up to 1000 (range from holster.ini:100). Expect no swing change (peak stays 593). Timing knobs do not shorten the turn.

## Open / caveats
- Magnitudes depend on the GUESS back quat. Sign and direction of the effect do not. Re-run with the live quat via `holster_log_check.py --mounts --json` and `holster_sweep_test --mounts-json` (not run here).
- Hinge outT for heavy (hypothesis c) is not modelled here.
- Samples are 200 per slide (about 2.9 ms per step). Peaks are approximate; the ratios are robust.
