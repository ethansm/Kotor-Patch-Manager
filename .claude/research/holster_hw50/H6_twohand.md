# H6: two-handed feasibility (HW50 hypothesis d)

Read-only recon. No source, ini or game file edited. Anchors are repo file:line.

## 1. What exists today
- `isTwoHandedStem` exists (HolsterLogic.h:95) with host tests (holster_stagger_test.cpp:189-210). No runtime caller in the repo, so it gates nothing.
- The reach code drives the left arm only for a weapon Gob placed on `lhand` (`ent->left`, Holster.cpp:2674). `lhand` is accepted only with bone_test_left=1 (HolsterDrawGate.h:58-61). A right-hand two-hander never moves the left arm. No support-hand IK exists.
- Bones written by reach code (reachWriteQ at Holster.cpp:1541; writes at 1813, 1823, 1863, 1882): `rbicep_g`, `Rforearm_g`, and the wrist = parent of the `rhand` hook (`Rhand_g`). Left equivalents (`lbicep_g`, `Lforearm_g`, `Lhand_g`) only for a left Gob (kBic/kFore/kHand, Holster.cpp:1632-1634).
- Not written by the mod: `rcollar_g`, torso, pelvis, spine, head. Reads only: `rhand` / `lhand` hooks and parent poses.
- Engine stance: heavy (weaponwield 6) maps to style 9 and rifle (5) to style 7; both ready idle is `g7r1` (10_equipment_3d_character.md:47). So the engine's two-hand clip may already pose the left arm for heavy. Unverified: SM-L8 is still `pending` (PLAY_TEST_QUEUE.md:119).
- Arm geometry, measured: shoulder `lbicep_g` (-0.164, -0.017, 1.462) m, arm length 0.63 m (.claude/research/holster_reach/arm_bones.tsv:11; r3_blend_proxies.md).

## 2. Smallest heavy-only support hand (not built)
1. Pure helper (HolsterReach.h): foregrip point = weapon world pose with model offset `c + k*len` along `axis`. Heavy rows: axis 1, len 0.75-0.85 m, c.y 0.26-0.31 (HolsterShape.inc:63-65). `k` is unmeasured (no mesh in repo).
2. World path, after the right-arm call (Holster.cpp:2674): when cat==CatHeavy, `reach_offhand_heavy=1`, no left Gob for the owner (scan like staggerFor, Holster.cpp ~2404-2411), and pathF inside the grip window: `reachArm(g, beh, foregrip, wL, left=true, cat, pathF, restMix, foregrip.pos, nullptr, false, n, carry=false)`. Passing the target as mountPos keeps u=0 (hip pole, no hinge). wL from reachWeightRamp.
3. Undo: a w=0 call beside the HW19 undo (Holster.cpp:2624) and at slide end.
4. Editor preview (Holster.cpp:2327): same guard, or stays off until HW47 parity.
5. Ini key, CONFIG line, `offhand=` field on REACH.
Size: about 35-45 lines in Holster.cpp, about 15 in HolsterReach.h, about 30 host-test lines. About 80-90 total.

Risks:
- ReachEnt is keyed (parent, left) (Holster.cpp ~1617-1620). A later real left Gob would share the entry; the owner scan must block that.
- Left elbow pole is untuned (mirrors the right-arm hip style; override via reach_pole_hip_heavy, holster.ini:212).
- From the back mount the left hand must cross the body: gate by pathF.
- If SM-L8 shows the engine already holds the weapon with the left hand, the write is redundant and can fight the engine pose. Ramp from w=0 to avoid a pop.
- Cost: one extra two-bone solve per heavy per frame inside the window (not measured).

## 3. Cheaper tricks (no second IK)
1. Ini-only: longer heavy draw / stow (`draw_slide_ms_heavy`, `stow_slide_ms_heavy`, holster.ini:108-109; try 800 / 900). Mass shows as time.
2. Ini-only: `reach_via_heavy=0` (holster.ini:188) removes the long-axis-up turn, the likeliest "swung" read (hypothesis a). Test first.
3. Code, about 10 lines: inertia on the weapon Gob pose the mod already writes (+0xA4): low-pass toward the hand (tau about 100 ms) so the heavy trails and sags. No new bone.
4. Code, about 30 lines, UNVERIFIED: torso lean (`torsoUpr_g`) or shoulder drop (`rcollar_g`). Neither is written today; the parent chain is not in the repo. Needs base / undo / persist handling like the arm bones and a write before the arm solve.
5. Code, about 6 lines: heavy-only wrist lag (use pathF^2 for the wrist turn). Wrist caps are global only (holster.ini:162, reach_wrist_max_deg=60).

## Notes
- Hypothesis (d) holds in code: no left-hand control for a right-hand heavy.
- Whether that causes the "light prop" look is unproven. SM-L8 has never been played.
- HW33 stays GATED on SM-L8 (42c_smooth_holstering_master_plan.md:69, :153).
