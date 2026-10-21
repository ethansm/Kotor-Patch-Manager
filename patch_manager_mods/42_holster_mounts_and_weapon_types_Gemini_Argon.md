Looking at the live data in your snapshot, a single "Hip vs. Back" binary switch isn't enough. Between the 5 mount bones, 6 weapon categories, two-handed vs. dual-wield states, and weapon local axes (`+Z` blades vs. `+Y` barrels), there are **five distinct mechanical problems** that need dedicated movement calculations and configuration parameters.

---

## 1. The 4 Mount-Zone Topologies (Fixing the `torso_g` Trap)

Right now, your code treats both `torsoUpr_g` and `torso_g` as "back mounts" that force an over-the-shoulder pole vector (`HolsterReach.h`).

Look closely at the bone heights in Section 1:

* `torsoUpr_g` is at **$z = 1.244\text{ m}$** (between the shoulder blades).
* `torso_g` is at **$z = 1.139\text{ m}$**—only **$1.3\text{ cm}$ above `pelvis_g` ($1.126\text{ m}$)**.

`torso_g` is the **small of the back (lumbar)**, not the upper back. If a player puts a weapon on `torso_g`, `reachHingeFits` will disable the hinged path (because it is more than 0.25 arm lengths below the shoulder), **but Section 4 states:** *"Any weapon on a back bone always uses over-the-shoulder [pole]."* Sending the hand down to waist height ($1.139\text{ m}$) while forcing an upward over-the-shoulder elbow pole ($+Z$) twists the bicep inside-out through the ribcage.

You actually need **4 distinct kinematic path topologies** based on mount bone and height relative to the shoulder ($S_z$):

| Topology | Mount Bone(s) | Height Zone | Arm & Elbow Mechanics | Pole Vector Trajectory (Right Arm: $+X$ right, $+Y$ fwd, $+Z$ up) |
| --- | --- | --- | --- | --- |
| **1. High-Back Hinge** | `torsoUpr_g` (when $M_z > S_z - 0.25 L_{\text{arm}}$) | Shoulder blades ($z \approx 1.24\text{ m}$) | **4-Stage Sagittal Hinge:** Set 90° L-pose $\rightarrow$ Pitch upper arm forward to horizontal `(0, +1, 0)` $\rightarrow$ Park elbow in front of shoulder & fold forearm back over trap $\rightarrow$ Reverse. | Starts down `(0, -0.3, -1)`, pitches forward to `(0.25, +1.0, 0.2)` at apex, stays pinned forward while forearm folds back. |
| **2. Lumbar / Small-of-Back Under-Reach** | `torso_g` (or low-offset `torsoUpr_g`) | Lower back / waist behind spine ($z \approx 1.14\text{ m}$) | **Behind-the-Back Chicken-Wing:** Upper arm pitches slightly *backward* and flares outward; elbow bends ~90° behind the torso; forearm sweeps horizontally across the beltline. | Must use a **Low-Back Pole**, NEVER over-the-shoulder: `(0.70, -0.70, -0.20)` (outward, backward, slightly down). |
| **3. Pelvis / Belt Lateral Drop** | `pelvis_g` | Waist side/front ($z \approx 1.13\text{ m}$) | **Short Lateral Arc:** Upper arm stays nearly vertical at the side; forearm opens from rest down to belt height with a slight outward bow (`+X`) to clear the hip bone, then lifts straight forward. | Hip pole: `(0.60, -0.40, -0.40)` (elbow flares slightly out and back so the wrist doesn't jam into the iliac crest). |
| **4. Thigh Holster Vertical Draw** | `rthigh_g`, `lthigh_g` | Mid-thigh ($z \approx 0.98\text{ m}$) | **Drop & Piston Pull:** Hand drops nearly to full arm extension ($L_1 + L_2$) along the outside of the thigh, grabs, pulls *straight up* $+Z$ for ~15 cm to clear the holster/leg, then bends the elbow forward. | Deep hip pole: `(0.50, -0.70, -0.20)` transitioning to forward-down during the upward pull. |

---

## 2. Weapon-Specific Extraction Vectors (Clearing the Head & Body)

In your current setup, the `Carry` phase simply walks the exact same path as `Reach` in reverse. That works for a bare hand, but once a $1.5\text{ m}$ sword or $0.7\text{ m}$ rifle is rigidly attached to the wrist, immediately rotating the wrist toward the `Rest` orientation swings the blade or barrel right through the character's cranium.

Each weapon category needs an **Extraction Tangent (Un-sheathe Vector)** during the first 15%–20% of the Carry phase (and last 15%–20% of Holster carry) before the wrist is allowed to slerp toward the rest pose:

### A. Upper-Back Long Blades (`melee` on `torsoUpr_g`, rot `-150, 7, 94`)

* **The Geometry Problem:** At `-150°` X and `94°` Z, the blade (`+Z` model axis) points diagonally down across the back, with the hilt angled over the right shoulder.
* **The Calculation:** When the hand grabs the hilt at `Grab`, do **not** immediately unfold the forearm straight up to `(0, 0, +1)` while rotating the wrist to rest. First, pull the hand **along the weapon's local $+Z$ (hilt-to-tip) axis** (up, forward, and slightly right) while **locking the wrist to the mount orientation** for the first ~25% of the carry phase. Once the blade clears the scabbard/shoulder line (at `T_high`), unlock the wrist slerp as the upper arm pitches down.

### B. Upper-Back Rifles & Bowcasters (`rifle` on `torsoUpr_g`, rot `-60, 4, 93`)

* **The Geometry Problem:** Rifles have their barrel along model `+Y` and grip along `-Z`. Unlike a sword where you grab the very top tip on your shoulder, a rifle's grip sits closer to the middle/stock, meaning part of the receiver extends *above* the hand when slung.
* **The Calculation:** The hand target `T_high` needs a wider outward lateral clearance (`+X` offset of `0.25 * L_forearm` instead of `0.10`) so the stock and scope pass *around* the ear in the coronal plane rather than directly over the top of the head in the sagittal plane.

### C. Heavy Repeaters (`heavy` on `torsoUpr_g`, rot `-90, -3, -90`)

* **The Geometry Problem:** Heavy blasters (`w_RptnBlstr`, `w_HvRptBltr`) are massive, held low at the hip/abdomen when drawn, and carried with a top/side handle. Using the high sagittal sword hinge brings a giant minigun directly through the character's face.
* **The Calculation:** Heavy weapons on `torsoUpr_g` should bypass the pure sagittal hinge and use a **Wide Lateral Sweep (Coronal Arc)**:
* During Carry, instead of keeping the elbow pinned straight forward `(0, +1, 0)`, flare the elbow and hand target wide to the right (`+X = 0.45` arm length), sweeping the repeater out and around the right shoulder down to the low hip-fire `Rest` position.



### D. Double-Bladed Sabers & Quarterstaves (`w_Dblsbr`, `w_Qtrstaff`)

* **The Geometry Problem:** Look at Section 4: `w_dblsbr` has its model origin at the **lower hilt end**, with the actual hand grip center at **`+0.21 m`**. Furthermore, `w_Qtrstaff` is a symmetrical long rod with no scabbard.
* **The Calculation:**
1. **Grip-to-Origin Offset:** If `HolsterPlace` places the weapon on the mount using the model origin, but the reach targets the mount bone position directly, the hand will grab the *bottom tip* of the double-saber hilt instead of the center grip, and the saber will teleport `0.21 m` along its axis on attach. The IK target `T_mount` must be computed as:

$$T_{\text{mount}} = \text{MountWorldPos} + (\text{MountWorldRot} \times \vec{O}_{\text{grip}})$$


2. **Side-Break Extraction:** Because a quarterstaff or double-hilt extends in *both* directions (`-Z` and `+Z`), you don't pull it "up and out" of a scabbard. You unclip it **perpendicular** to the back (`-Y` backward / `+X` outward) by ~10 cm before pitching it forward over the shoulder.



---

## 3. Wrist Rotation Math & The 90° Cap Bug

Check Section 3 and Section 5 of your doc:

* `lightsaber` mount rotation on `pelvis_g` is **`180, 0, 0`** (upside-down relative to drawn hand).
* `melee` mount rotation on `torsoUpr_g` is **`-150, 7, 94`**.
* `pistol` mount rotation on `pelvis_g` is **`-100, 0, 0`**.
* Yet `reach_wrist_max_deg` defaults to **`90` degrees**!

### Why this causes visible snapping:

If the weapon is mounted at `150°` or `180°` relative to the rest hand, clamping the wrist bone rotation to `90°` means the hand stops turning halfway there. During the `Carry` phase (where "the weapon follows the hand pose we just solved"), the weapon will instantly snap across the remaining `60°–90°` gap during the brief grab/land blend window!

### The Fix:

1. **Natural Arm Roll from the Hinge:** When the arm folds back over the shoulder in the 4-stage hinge, the forearm (`Rforearm_g`) has already rotated ~`135°–150°` in world space relative to the rest pose. Make sure the wrist delta quaternion is computed **after** the shoulder and forearm IK writes (`Step 7` in your doc), measuring the delta between the **newly solved forearm/hand world rotation** and the **weapon's world mount rotation**—*not* the delta from the cached rest hand!
2. **Raise `reach_wrist_max_deg` to `135°–160°` for Hip Mounts:** On a hip mount (`pelvis_g` at `180, 0, 0`), the forearm doesn't flip upside down, so the wrist/forearm pronation must absorb more of that rotation, or the saber hilt should be mounted with the emitter pointing down via a Z/Y rotation that aligns closer to natural wrist pronation.
3. **Delayed Wrist Slerp Curve:** Do not slerp the wrist linearly (`0.0 -> 1.0`) across the entire carry phase. Hold the wrist near the mount alignment ($t \in [0, 0.25]$) while the weapon clears the body, then slerp the wrist rapidly during the mid-swing ($t \in [0.25, 0.85]$) when the weapon is safely out in front of the character in open space.

---

## 4. Two-Handed Off-Hand & Dual-Wielding Calculations

Right now, 15 of your 32 weapon stems are two-handed, and 15 are dual-wield eligible, yet both have edge cases:

### A. The Off-Hand "Support Meet" Solver (15 Two-Handed Stems)

In Section 7, Claude notes: *"Two-handed weapons still move only the main hand. The off-hand support hand isn't built yet."*
When drawing a rifle, repeater, quarterstaff, or double-bladed weapon, having the left arm swing idly at the character's side until the very last millisecond—and then instantly teleport up to the rifle barrel—breaks the illusion of weight.

* **The Calculation:** You do *not* want the left hand reaching up to the back mount. Instead, add a secondary procedural track for `lbicep_g` / `Lforearm_g` whenever `isTwoHanded(category, model)` is true:
* **During Reach (`0% -> Grab`):** Left arm stays at `100%` animation pose (`Rest_L`).
* **During Carry (`Grab -> 100%`):** Compute a dynamic `ForegripTarget` in world space along the weapon's barrel/shaft axis (e.g., `HandPos_R + WeaponWorldRot * (0, +0.25m, 0)` for rifles/heavy, or `(0, 0, +0.20m)` for two-handed melee/staves).
* Blend the left hand target $T_L$ from `Rest_L` (at `Grab`) to `Rest_L_Drawn` / `ForegripTarget` (at `70% -> 100%` of the slide) so both hands arrive on the weapon simultaneously as it settles in front of the chest.



### B. Dual-Wielding Back Mount De-Confliction

When dual-wielding two vibroswords (`w_Vbroswrd`), both weapons map to `torsoUpr_g` with mirrored X offsets (`+0.12` and `-0.12`) and negated Y/Z rotations.

* **The Collision Problem:** If both arms run the exact same symmetric hinge calculation at the exact same millisecond, both swords will cross at the midline during the apex of Stage 3/4, clipping through each other.
* **The Calculation:** When both main hand and off-hand are drawing/stowing simultaneously from `torsoUpr_g`:
1. **Lateral Separation:** Increase the off-hand's outward X-clearance (`X_clear`) by `+0.08 m` so the left arm's arc passes slightly wider than the right arm's arc.
2. **Phase Stagger (`35–50 ms`):** Delay the off-hand's normalized time $t_{\text{left}} = \text{clamp}((t - 0.08) / 0.92, 0, 1)$ so the right hand leads the grab/draw by a split second. Not only does this prevent the hilts from occupying the same space above the head, it makes dual-wield draws look noticeably more natural and organic instead of mirrored robot arms.



---

## 5. Timing & Easing Curves: Fixing the "171 ms Twitch"

Look at the math in Section 5 of your snapshot:

* `draw_slide_ms` is clamped to a hard ceiling of **`570 ms`**.
* `reach_grab_pct` defaults to **`30%`**.
* That means the **entire 3-stage reach to the back** happens in **$570 \times 0.30 = 171\text{ ms}$** (barely 10 frames at 60 FPS), while the return carry gets $399\text{ ms}$.

At `171 ms`, our multi-stage hinge (drop to L-pose $\rightarrow$ pitch shoulder 90° forward $\rightarrow$ fold forearm back to shoulder blade) will move so fast it will look like a single-frame glitch!

### Timing & Curve Calculations Needed:

1. **Per-Topology `reach_grab_pct` and Duration Scaling:**
* A **hip pistol or lightsaber draw** (`pelvis_g` / `rthigh_g`) only travels ~25 cm to the mount; `30%` grab (`~150–170 ms`) is snappy and appropriate.
* An **upper-back hinge draw** (`torsoUpr_g`) travels nearly $1.2\text{ m}$ along a 2-part arc before touching the weapon. It needs **`reach_grab_pct = 45% to 50%`** and, ideally, raising the internal clamp on `draw_slide_ms` from `570 ms` to **`750–850 ms`** for back-mounted weapons (or scaling the slide duration by a per-category multiplier like `back_slide_scale = 1.35`).


2. **Asymmetric Easing Curve (Ease-Out into Grab, Pause, Ease-In-Out Carry):**
* If `t` advances linearly through `reach_grab_pct`, the hand slams into the weapon at full speed and instantly bounces back in the opposite direction on the very next frame.
* Apply a smoothstep / cubic ease-out as $t \to t_{\text{grab}}$ and ease-in as $t$ leaves $t_{\text{grab}}$, with a **2-to-3 frame "grip dwell" plateau** ($\sim 30\text{ ms}$ where $T = T_{\text{mount}}$) right across the grab/release boundary so the player's eye actually registers the hand grasping the hilt before the weapon begins moving.



---

## Summary Configuration Matrix to Build

Here is the complete matrix of movement rules your system needs to cover every weapon and mount smoothly:

| Archetype | Categories / Stems | Mount Bone | Path Solver | Extraction Tangent (First 20% of Carry) | Off-Hand (Left Arm) | Ideal Timing (`slide_ms` / `grab_pct`) |
| --- | --- | --- | --- | --- | --- | --- |
| **1. Hip Saber / Short** | `lightsaber` (all 3), hip-mounted short melee | `pelvis_g` | **Topology 3:** Short Lateral Belt Drop | Outward (`+X`) 8 cm, then straight forward (`+Y`) | Mirrored if dual-wield; Meet-curve if `w_Dblsbr` | `400 ms` / `30%` |
| **2. Thigh / Hip Pistol** | `pistol` (all 6) | `pelvis_g` or `rthigh_g` | **Topology 3 or 4:** Vertical Holster Drop | Straight up (`+Z`) 12 cm along hip/thigh before pitching barrel forward | Mirrored with `40 ms` stagger if dual-wielding | `350 ms` / `30%` |
| **3. Back 1H / 2H Blade** | `melee` (swords, axes, pike, warblade, gaffi) | `torsoUpr_g` | **Topology 1:** 4-Stage Sagittal Hinge (`X_clear = 0.12`) | Pull along blade axis (`+Z` local: up & forward) with locked wrist | Dual-wield: staggered `45 ms` + wider `X_clear`. 2H: Meet-curve at 65% | `700 ms` / `45%` |
| **4. Back Staff / Double** | `qstaff`, back-mounted `w_Dblsbr` | `torsoUpr_g` | **Topology 1:** 4-Stage Sagittal Hinge + **Grip Offset ($\vec{O}_{\text{grip}}$)** | Unclip perpendicular to back (`-Y` / `+X`) 10 cm before pitching forward | 2H Support Meet-curve to center shaft at 65% | `650 ms` / `45%` |
| **5. Back Rifle / Carbine** | `rifle` (all 6, incl. bowcaster) | `torsoUpr_g` | **Topology 1 (Wide):** Hinge with wider lateral clearance (`X_clear = 0.25`) | Up and outward (`+Z, +X`) to clear stock/scope around ear | 2H Support Meet-curve to foregrip (`+0.25m Y`) at 60% | `700 ms` / `45%` |
| **6. Back Heavy Repeater** | `heavy` (`w_RptnBlstr`, `w_HvRptBltr`) | `torsoUpr_g` | **Wide Coronal Sweep:** High-outward arc (`+X = 0.45`) down to low hip rest | Outward (`+X`) and up to clear wide receiver from shoulder | 2H Support Meet-curve to top/front carry handle at 55% | `800 ms` / `45%` |
| **7. Small-of-Back** | Any weapon assigned to `torso_g` | `torso_g` | **Topology 2:** Lumbar Under-Reach (Elbow back & out, NEVER over-shoulder) | Pull backward (`-Y`) and outward (`+X`) around waist before coming forward | Standard (idle or mirrored) | `500 ms` / `35%` |