# Holster/draw animation generation: web recon (2026-10-13)

## Ranked routes (effort vs quality)

1. Procedural FK keys in headless bpy, then KotorBlender export (low-medium effort, clean licence)
   - https://github.com/seedhartha/KotorBlender : GPL-3.0; README lists MDL animations, "Editing Animations" with events, armature-based edits (unapply to objects before export). No transtime mention.
   - https://projects.blender.org/blender/blender/issues/82267 : IK chain_count not animatable since Blender 3.2; key bone rotations or a target empty instead.
   - https://skills.sh/terminalskills/skills/blender-animation : community page, shows `blender --background --python` and pose-bone keying (not authoritative).

2. CMU mocap BVH, then retarget to KOTOR skeleton (medium effort, licence likely OK, unverified)
   - https://paperswithcode.com/dataset/cmu-motion-capture : secondhand mirror of CMU terms: free for research and commercial products, no direct resale. Lab page fetch failed (TLS error).
   - https://mocaponline.com/blogs/mocap-news/bvh-files-guide-convert-import : Blender imports BVH natively; CMU data often needs scale 0.01.
   - https://blenderartists.org/t/bvh-retargeter-the-add-on-formerly-known-as-makewalk/1216579 : BVH Retargeter, licence not stated; user reports of no-op retarget in T-pose.

3. Text-to-motion (NVIDIA Kimodo) plus retarget (high effort: GPU, bone mapping)
   - https://huggingface.co/nvidia/Kimodo-SOMA-SEED-v1/blob/main/README.md : model card says NVIDIA Open Model License, commercial use cleared; 30-joint SOMA skeleton; pose/waypoint constraints.
   - https://github.com/matbeedotcom/kimodo : non-NVIDIA mirror; official nv-tlabs/kimodo LICENSE not verified (Apache-2.0 claim unconfirmed).

4. MDM / MotionGPT (high effort, SMPL/T5 dependencies; skip unless SMPL terms cleared)
   - https://www.sourcepulse.org/projects/528812 : MDM index listing, MIT (aggregator, unverified against repo).
   - https://www.sourcepulse.org/projects/1594954 : MotionGPT index listing, MIT; needs SMPL and T5.

5. Skip
   - https://amass.is.tue.mpg.de/license.html : AMASS non-commercial only ("any use for commercial purposes, is prohibited"); derived data needs written permission.
   - Auto-Rig Pro (paid) and RetargetKit (paid, licence conflicting between sellers) excluded per brief.
   - https://huggingface.co/Vanth-Labs/hannah-motion : SMPL-X weights, non-commercial.

## KOTOR MDL ASCII constraints

- https://xoreos.org/pipermail/xoreos-devel/2015-August/000028.html : positionkey/orientationkey rows are time then xyz or axis-angle (NWN-style); placeable rows vary in value count, unresolved.
- https://synchro.codeberg.page/rakata/api/rakata_formats/mdl/ascii_writer/index.html : writer emits geometry, DFS node tree, then animations; quaternions become axis-angle. Keyword syntax not shown (page fetch confirmed no newanim/transtime/event detail).
- https://deadlystream.com/topic/7475-editing-animations-and-rigging/ : KOTORmax puts all animations on one timeline with named ranges; transtime and events not discussed.
- Not verified by any source: newanim/doneanim, length, transtime, event line syntax.

## Recommendation

- Start with route 1: key rest and holster poses on the imported KotorBlender armature, slerp in Python, export via KotorBlender.
- Confirm transtime and event round-trip by diffing an exported MDL against a vanilla model before any in-game test.

## Caveats

- No source confirmed a holster or draw generator end to end.
- Blender data-API keying is preferred over bpy.ops in background mode (inference, not sourced).
- Licence notes are not legal advice; confirm before any commercial distribution.
