The 5 bones × 6 categories × 2 hands don't each need their own motion. A draw changes based on three things: where the grip sits relative to the drawing shoulder, the weapon's shape, and how many hands are involved. That gives seven draw profiles, built from about seventeen calculations. Roughly half of those calculations don't exist in the solver yet.

## 1. What makes two draws different

- **Grip position relative to the drawing shoulder, not the bone name.** What matters is height (in arm lengths from the shoulder), side (same side or across the body), and whether the grip is in front or behind. The current bone test has two problems:
  - `torso_g` sits at z 1.139, essentially pelvis height. The editor's "Back" is really a small-of-back mount, but it still gets the over-shoulder pole.
  - `pelvis_g` with +0.20 x is a right-hip mount in practice.
  
  If `chooseMount` gets wired, its "other side" spills also create cross-body reaches, which nothing handles today.
- **Shape:**
  - compact: pistols, baton, saber hilts
  - end-gripped blade: swords, axe, warblade
  - centre-gripped pole: qstaff, double swords, pike, gaffi
  - stocked long gun: rifles, bowcaster
  - heavy: the repeaters, which are long guns with more mass and bulk
- **Hands:** one hand, dual wield, or two-handed.
- **Mount motion:** torso bones are steady, the pelvis rocks, and the thighs swing a lot while walking.
- **Saber blade state,** if the blade can be visible during the slide.

## 2. The seven draw profiles

The left hand mirrors each one.

| Profile | Who lands here | What's distinct |
|---|---|---|
| P1 Same-side hip | pistols, all saber hilts (locked to hip); baton and one-hand swords if moved off the back | Short lift along the holster axis, then the weapon turns to face forward. The turn should come from the forearm swinging up, not from the wrist. Saber hilts flip about 180°, which needs a fixed rotation direction (#6). |
| P2 Cross-draw | right hand to left hip or front waist (from a spill, or configured) | The hand crosses in front of the belly, with the elbow forward, out and down. It needs a forward bow at hip height. The current bow scales with mount height, so it is zero here and the straight path cuts through the abdomen. |
| P3 Thigh | the pistol thigh seed | P1 with a moving target. The mount swings with the leg, so the grip orientation drifts during the reach and at landing. |
| P4 Small of back | `torso_g` with a low offset | The hand goes behind the back palm-out, elbow back and up, and the weapon exits horizontally. This is the hardest wrist case; I'd consider not offering it. |
| P5 Over-shoulder scabbard | long and vibro swords, warblade, and the live one-hand melee on the back | The blade slides out along its axis and pivots at the scabbard mouth, and the tip arcs up and outside the head. The hinge path is the hand half of this motion; the weapon half is missing. |
| P6 Over-shoulder sling | rifles, bowcaster; repeaters as a heavier variant | Lift a few cm off the back, then swing the muzzle up and out over the shoulder. The off-hand catches the foregrip at the end. Heavy weapons should be slower with more lift than rifles; today they share one bulge row. |
| P7 Back pole | qstaff, double swords, force pike, gaffi | Both ends sweep up to about 1 m from the grip. The pole rotates in the back plane to vertical beside the shoulder, then forward, and the off-hand catches it. |

Two cases fall outside the profiles:
- **Creature weapons** (`w_crslash`, `w_crpierce`, `w_crbite`) sit in the melee category, so the live melee key puts them on the upper back. They should never holster or reach.
- **`CatUnknown`** should stay in hand or fall back to P1.

## 3. Calculations to build

### Setup (once per weapon and body)

1. **Mount classifier from geometry.** Put the mounted grip point in the drawing shoulder's frame and classify it by height (in arm lengths), side and depth. Drive the profile, the pole and the hinge from that, instead of "torso bone means back." This fixes the `torso_g` case and the cross-draw case above.
2. **Weapon shape from the model's bounding box at load.** Extract:
   - main axis and length
   - grip point (the model origin)
   - far-end points (tip, butt, muzzle, stock)
   - exit length (the blade length past the guard)
   - a support-grip point for two-handers
   
   Per-model keys can override any of these. This replaces the unmeasured-extents list and gives you the planned 0.6 m "long" threshold directly.
3. **Body proxies from bones.** A head sphere on `head_g`, a neck/torso capsule and thigh capsules, sized from measured segment lengths. Then the female body and per-body variants need no separate tuning.

### Hand path

4. **Piecewise path** with these segments in order:
   - reach arc
   - grab dwell
   - extract: straight along the exit axis, length from #2, capped by arm reach
   - swing arc
   - rest
   
   Keep velocity continuous (Hermite) from extract into swing. Bring it nearly to zero at the grab so the grip reads. Stow runs the same waypoints backwards.
5. **Path in the torso frame, recomputed every frame.** Low-pass the rest-hand target while walking or running; otherwise the animation's arm swing wobbles the end of the path. Track the mount bone per frame for P3.

### Weapon orientation (what the wrist aims for)

6. **Orientation schedule,** replacing the single slerp from mount orientation to rest orientation:
   - **Extraction:** hold the mount orientation.
   - **P5 mouth pivot:** while the tip is still in the scabbard, the blade axis is the direction from the grip to the mouth point M, which is fixed in the mount bone's frame. The blade then pivots at M as the hand moves. It goes free once |M − grip| exceeds the blade length.
   - **P6/P7 via-orientation:** add one waypoint between mount and rest, with the barrel or pole near vertical outside the shoulder, and slerp twice.
   - **Explicit rotation direction for turns past about 150°.** Near 180° the shortest-arc choice is unstable and can flip between frames as the bones move. That affects the live saber placement (180,0,0).
7. **Sweep check.** Sample the far-end points along the whole slide against the body proxies. Report the minimum clearance per body, weapon, mount and hand.
   - In preview-3d, use it to pick the rotation direction and tune the bulges.
   - Optionally at runtime, push the hand target outward by the penetration depth.
   
   That turns head clipping from "tuned" into "checked" without building real collision.

### Arm solve

8. **Elbow from the hand's target orientation at the grab.**
   - The elbow lies on a circle around the shoulder→hand line.
   - The target hand orientation implies a forearm direction d. Project H − L_forearm·d onto the circle and use that swivel at the grab.
   - Over the swing, blend to the pole and then to the animation's elbow.
   
   Why it matters: a pistol pointing down on the hip is gripped with the forearm hanging along it. The barrel comes forward because the forearm comes up, not because the wrist bends. Today the pole sets the forearm and the wrist absorbs the rest. A barrel-down-to-forward turn is roughly 90–100°, at or past the 90° cap. The per-category poles stay, but as a bias.
9. **Swing/twist split.** Decompose the remaining hand rotation into two parts with separate limits, replacing `reach_wrist_max_deg`:
   - twist about the forearm axis, written on `Rforearm_g`, roughly ±80°
   - swing at the wrist, roughly ±60° of flex and less sideways
10. **Head tilt (optional new handle).** A few degrees of neck/head tilt away from the drawing side during P5–P7. It uses rotations only, through the same safe-write path, and is a cheap way to sell head clearance.

### Hand-off

11. **Attach-switch residual.** At the grab and at the landing:
    - compute the difference between the hand-attached pose and the mount pose;
    - store it as an offset in the hand's frame and decay it over about 100 ms, so it rides the moving hand (a world-space blend lags behind a fast hand);
    - log its size.
    
    The position and angle residual is the best single smoothness number to tune against, and #8–9 should push it toward zero.

### Timing and state

12. **Per-profile duration, phase shares and easing.** The phases are reach, dwell, extract and swing.
    - The 570 ms draw clamp is tight for P5–P7, which now have four phases. Find out whether the clamp comes from the engine before squeezing them.
    - Starting points: pistol about 300 ms, saber 350, hip sword 400, back sword or rifle 520, staff and heavy at the cap.
    - Stow can run longer.
13. **Interrupts and reversal.** Keep one progress value per hand plus a record of who holds the weapon. A reversal continues from the current hand position in the mirror phase:
    - a draw interrupted during the reach becomes a stow release from that point;
    - a draw interrupted during the carry becomes a stow carry.
    
    A weapon swap, death or cutscene mid-slide snaps to a safe end state.

### Two hands and special cases

14. **Off-hand for two-handers.** IK the off-hand to the support grip from #2, with its own pole. It joins during a catch window at the end of the draw and leaves during a release window at the start of the stow.
    - Check which animation style plays during the slide. If the two-handed stance is already active, the off-hand holds air at the foregrip while the rifle is still on the back.
    - Whichever end of the slide the style flips at, the rest-hand target jumps there. That needs a cross-fade from the cached pose.
15. **Dual wield.** Run mirrored profiles with a 60–120 ms stagger, with both hands finishing inside the draw clamp. Crossed back mounts need a depth order so the blades don't pass through each other during extraction.
16. **Saber blade gate,** if the blade can show during the slide. Ignite only after the swing clears the body (using the sweep check), and douse before the stow carry starts. The double saber needs both ends clear.
17. **Party start jitter.** A small random offset per actor, so `delay_party` doesn't move the whole party in lockstep.

## 4. Configuration

Most of this should be derived automatically, with keys as overrides:

| Key (suggested) | Default |
|---|---|
| `reach_profile_<cat>` / `reach_profile_w_<model>` | auto from #1 and #2 |
| `reach_exit_<cat>` / `reach_exit_w_<model>` (axis, length) | from the bounding box |
| `reach_via_<profile>`, `reach_spin_<cat>` (+1 / −1 / 0 = auto) | per profile / chosen by the sweep check |
| `reach_twist_max_deg`, `reach_swing_max_deg` | replace `reach_wrist_max_deg` |
| `reach_dwell_ms`, `reach_extract_pct`, `draw_slide_ms_<cat>`, `stow_slide_ms_<cat>` | per profile |
| `offhand_grip_w_<model>`, `offhand_catch_pct` | from the bounding box / about 25 |
| `dual_stagger_ms`, `party_jitter_ms` | about 80 / about 100 |
| `saber_ignite_pct`, `saber_douse_pct` | gated by the sweep check |
| `clearance_min_m` | editor warning threshold |

`reach_pole_*` and `reach_bulge_*` stay, as biases on top of #8 and the per-profile bow.

## 5. Build order

1. **Classifier and creature-weapon exclusion.** It's small, and it fixes the wrong pole for `torso_g` and for any cross reach.
2. **Residual readout (#11),** so every later step has a number to tune against.
3. **Elbow swivel and swing/twist split (#8, #9).** These help every profile and shrink the grab/landing pop.
4. **Shape data, extraction, orientation schedule and sweep check in preview (#2, #4, #6, #7).** This is what makes the back profiles work.
5. **Per-profile timing and interrupts (#12, #13).**
6. **Off-hand, dual stagger, saber gate, thigh tracking (#14–16, #5).**

Four open questions could change the plan:
- Which animation style is active during the slide?
- Does the 570 ms clamp come from the engine?
- Do saber blades render while sliding?
- Will `chooseMount` be wired? If not, P2 doesn't exist.

I can write this up as a design doc next to `42_holstered_weapons.md` if that's useful.