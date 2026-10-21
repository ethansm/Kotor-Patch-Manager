# D2: engine and data feasibility of a left-hand support reach (two-handed weapons)

Static recon only. Source: patch_manager_mods/holster_src (read-only), hw18/hw19 notes, 10_equipment_3d_character.md. Line numbers are Holster.cpp unless noted.

## 1. Write point, same-bone writers, per-frame re-apply (PARTIAL)
- Arm writes come from HolsterPlace (Holster.cpp:1547). Its comment (1547-1549) says it runs on every per-frame refresh of an attach behavior, right after the engine copies node N's world pose into gob+0xA4/+0xB0. Arm writes are local quats (part+0x14) via reachWriteQ (~1190).
- Re-apply: the mod re-solves and rewrites every call. If the bone was not resampled (persist=1), it restores the cached base first (1279-1287). A left-arm write must therefore be re-applied each frame the reach is active. CONFIRMED by code.
- Engine resample: the controller write chain (0x4DEA20 per Gob::Animate pass) is CONFIRMED static (hw18 sec 1). hw19 REFUTES the AnimateExternal vt+0x14 entry. NPC gating is UNSURE (hw18 OPEN-2). Live answer is the RCH1 persist=0/1 signal (PLAY_TEST_QUEUE.md:104), not yet recorded.
- Two writers, same bone, same frame: last write wins, no blending. The second writer sees the first write as persist=1 and restores its cached base (1279-1287), which clobbers the first. Right and left arm bones are disjoint (rbicep_g/lbicep_g, Rforearm_g/Lforearm_g; 1247-1248), so a left reach is safe only if it never shares a ReachEnt with a left-hand weapon.
- Child follow: 0x457510 recomputes world from locals on each call (hw18 sec 2, CONFIRMED static). Writing the bicep moves forearm and hand.

## 2. Arm cache scope (CONFIRMED)
- g_reach[64] (~1181). ReachEnt is keyed by (parent = [beh+0x14], left) at Holster.cpp:1235. One entry per owner per side, so two entries per owner are possible.
- An entry is created only by a reachArm call with w > 0 for that side (1237). A right-hand weapon never creates the left entry, so only the weapon's own arm is cached today.
- Capacity 64, round-robin overwrite (g_reachNext), no LRU.

## 3. Two-handed knowledge at runtime (CONFIRMED: NOT available)
- classify(stem, weaponWield) is called with weaponWield = 0 at Holster.cpp:1093 (log), 1453, 1609, 1862. Category comes from the model stem table only (HolsterLogic.h:59-70). Unknown stem gives CatUnknown (fromWeaponWield(0), HolsterLogic.h:76-81).
- isLong (HolsterLogic.h:94), preferredZone (:108) and chooseMount (:135) are called only from holster_logic_test.cpp. The mount bone comes from config (resolveFull, HolsterTune.h:344; boneIdx at Holster.cpp:1455), not from wield.
- Effect: w_Dblswrd, w_ForcePike, w_gaffi, w_warblade and w_Qtrstaff are category-only at runtime. Nothing marks a melee weapon as two-handed.

## 4. Vanilla left hand on a two-handed weapon (UNKNOWN)
- The lhand attach (0x763E7B, Holster.cpp:1011) runs only for an equipped left-hand item. A right-only weapon attaches to rhand alone, so the left hand has no weapon attach.
- Engine style rule (10_equipment_3d_character.md:47, R7f): right-only weaponwield 3 gives style 3, ready idle g3r1 (looping). That clip is the only candidate that places the left hand. Its left-hand pose against the weapon is not measured. The w_ models expose no foregrip node (only rhand/lhand/rhand_g/lhand_g on bodies).
- So the drawn rest pose is probably the left hand posed against the body in the clip, not against the weapon. Whether it touches the foregrip: UNKNOWN.

## Required new code (feasible, PARTIAL)
1. HolsterLogic.h: a stem-based two-handed predicate (e.g. isTwoHandedStem) for the melee stems in kStems (w_Dblswrd, w_VbrDblswd, w_ForcePike, w_gaffi, w_warblade) plus CatQStaff. Test in holster_logic_test.cpp. Needed because weaponWield never reaches runtime.
2. Holster.cpp reachArm (1229): a support target branch. The target is the foregrip point, taken from the weapon Gob world (gob+0xA4/+0xB0) shifted along its long axis by a new config key support_off_<cat> (parser near 359-375). The current signature assumes the rhand path, so it needs a target parameter, not just a flag.
3. Holster.cpp HolsterPlace (call site near 1756): a second reachArm with left=true and w = support weight, only when the rhand weapon is two-handed and plan.active. Skip when a left-hand weapon Gob exists for the same parent (shared ReachEnt).
4. Holster.cpp REACH log line: add side=L and support=1 so the RCH1 play check can read the new path.
5. No new hook: HolsterPlace already runs per frame for every weapon Gob.

## Open
- Does g3r1 already place the left hand on the staff? Needs a live probe that logs lforearm world against the weapon axis in ready pose.
- NPC per-frame resample (RCH1 persist=0 vs 1) decides whether the support write needs the same restore logic as the right arm. It does by default.
