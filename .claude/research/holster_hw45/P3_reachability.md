# P3 reachability: does a real draw reach the HW45 raw turn (233 deg)?

Verdict: no. Production never hands reachSplitWrist more than 180 deg. The 233 deg failure is a sweep-only regime. A separate issue sits at the 180 clamp (section 3) and is not HW45 as written.

## 1. Bound at the split (code)
- Holster.cpp:1855 calls `hs::reachWristDelta(..., 180.f, w * g_reachWristPct, ...)`, the only caller of the dq at Holster.cpp:1859 (`reachSplitWrist`).
- reachWristDelta (HolsterReach.h:618-624): `dq = reachClampRot(target * conj(cur), maxDeg=180, hint)`, then `blendQ(identity, dq, weight)`.
- reachClampRot (HolsterReach.h:528-540): output angle <= 180 (clamp branch returns 180 about the hint-aligned axis).
- blendQ (HolsterReach.h:478-485): flips to w >= 0 and caps w <= 1. So the split always gets w >= 0, raw angle <= 180.
- The 233 regime (w < 0 quaternion, raw 2*acos(w) > 180) is not reachable from the wrist block.
- The HW45 probe fed raw 120-260 straight into the split (`.claude/waves/w2-42c-HW45b/A_probe_a.md:6`): baseline first failure at 233.5 deg, E = E0 below it.
- `holster_sweep_test.cpp` never calls reachSplitWrist or reachWristDelta. `run_tests.py holster_sweep` cannot show this either way; not run.

## 2. Max natural turn per category (frozen REACH logs)
- Sources (frozen copies, repo only): `.claude/waves/hw44slow/holster_log_snap3.txt` (1594 joined REACH lines), `holster_log_snap2.txt` (1356), `holster_log_snap.txt` (162). Joined REACH+ piece 2 (cat=) and piece 3 (g=, tw=, sw=, f=, topo=, bk=).
- Field g = reachQAngleDeg(mountQ * conj(post-IK hand)), always 0..180. At f=0 it equals the dq angle at the grab. Mid-path dq is not logged.
- Tool: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/P3/reach_angles.py`.

| category | topology | n | g max (f at max) | g >= 140 | g >= 179.5 |
|---|---|---|---|---|---|
| melee | hinge+bk | 2486 | 180 (0.71) | 1793 | 47 |
| rifle | hinge+bk | 522 | 179 (0.17) | 39 | 0 |
| pistol | hip | 99 | 179 (0.80) | 14 | 0 |
| heavy | hinge+bk | 5 | 112 (0.66) | 0 | 0 |
| lightsaber, qstaff, thigh, over | - | 0 | not sampled | - | - |

- Caps seen in the same logs: tw max 80, sw max 60 (cap = 140).
- Melee g >= 170 runs: up to 56 consecutive frames, peak 180, at f ~0.52.
- No category exceeds 180 in g, and none reaches 233.

## 3. Real risk at the 180 clamp (not in HW45 as written)
- Hint continuity can pick the w < 0 representation for a continuous turn past 180. The clamp then returns 180 about that axis, and the wrist holds there.
- Replica: `.../scratchpad/P3/clamp_chain_sim.py` (pure Python port of reachClampRot and blendQ, no float32, no weight ramp).
- Monotone turn 0 -> 300 deg about a fixed axis: output error 20 deg at 200, 60 at 240, 120 at 300.
- Out-and-back 0 -> 200 -> 0: error peaks at 20 and returns to exact.
- Exposure: 47 melee frames at g >= 179.5, and sustained g >= 170 runs near f 0.5. Whether the continuous target-to-hand turn crosses 180 is not logged, so this stays a hypothesis.

## 4. Answer
- Max raw dq at reachSplitWrist: 180 deg, for every category. None exceeds 233. None exceeds 180.
- HW45 (233 deg split failure) is a sweep-only artifact in production.
- The 180 truncation (section 3) is the only over-180 mechanism the production path can produce, and it has not been probed.

## 5. Next checks (not run)
1. Add a pre-clamp diagnostic in reachWristDelta: count and max of `ang` when `ang > 180`. Read it from the SM-L18 play log (copy the file first; never the live game-dir file).
2. Host test: drive reachWristDelta with a physical turn that crosses 180 about one axis. Expect error = theta - 180.
3. Do not spend on the split (HW45 pick) until check 1 shows whether `ang > 180` occurs in real draws.

Scratch: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/P3/`
