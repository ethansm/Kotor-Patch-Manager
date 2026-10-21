# H4: animation style during draw/stow and the reach target (recon, read-only)

Anchors are `patch_manager_mods/holster_src/` unless noted.

## H4a Style during the slide
- Claim tested: the body's style flips at slide start/end.
- DLL never writes the style byte. +0xC5 is only read, for logging (Holster.cpp:582, ANIMRES at :603). `hs::decide()` (HolsterAnimStyle.h:27) has no caller outside tests. The HW16 detour is GATED (42_holstered_weapons_master_plan.md:38). So the DLL causes no flip.
- Engine style during the slide: UNKNOWN. Style is engine-owned, set per equipped weapon (D2_offhand_engine.md:24: right-only wield 3 gives style 3). Nothing in the repo shows whether holstering leaves the weapon equipped, or traces style across a slide.
- Probe: HW7a-P (PLAY_TEST_QUEUE.md:98) logs `ANIMRES hol=1`. Add `style=` against slide start/stop, then `python3 scripts/kotor2_logs.py check holster_log.txt --new`.

## H4b Rest-hand target
- Premise PARTIAL: the target is not a frozen end-of-draw pose. `pH` is the animated hand node, re-read on every call (Holster.cpp:1292 `reachNodePose(e->hand, e->poseFn, pH)`; reader at :1187-1190).
- Persist write before the read (Holster.cpp:1290-1291) resets bic/fore to base, so `pH` is not polluted by our own write.
- Target per frame (Holster.cpp:1331): `T = (1-restMix)*((1-pathF)*mount + pathF*pH) + restMix*pH`. Non-carry plan is `{active, slideF, 0, 1}` (Holster.cpp:1485), so restMix=0 and `T = mount + pathF*(pH - mount)`.
- Weight: `reachArm` gets `reachW` (Holster.cpp:1498). `reachWeightRamp` (HolsterReach.h:180-186) ramps 0->1->0 over the first and last 20% of the slide (0.04 in carry, Holster.cpp:1480). It is 0 at both ends.
- Result: walk/run swing moves T every frame, scaled by pathF*w. Peak mid-slide. A clip flip mid-slide steps T by up to pathF*w*delta(pose). At the ends it is zero, so no visible jump.
- Carry path restMix comes from `hs::reachPlan` (HolsterReach.h:316-321). Not read in full here.

## H4c Two-handed
- No runtime two-hand detection. Every `classify` passes weaponWield=0 (D_twohand_dual.md:11, D5). No foregrip target in holster_src.
- Off-hand is solved only for the weapon's own hand (`left` flag, Holster.cpp:1498). A rifle on the back gets no left-hand target from the DLL.
- Whether the engine two-hand clip puts the left hand on the foregrip: UNKNOWN. The w_ models expose no foregrip node (D2_offhand_engine.md:22-25).
- Probe needed: ANIMRES style for a rifle carrier, plus left hand vs weapon at slide mid-point (`HANDATTACH hand=L` lines).

## Verdict
- H4a: UNKNOWN for the engine (DLL side CONFIRMED: no write).
- H4b: PARTIAL (live read, not frozen; scaled by slide weight).
- H4c: UNKNOWN.
