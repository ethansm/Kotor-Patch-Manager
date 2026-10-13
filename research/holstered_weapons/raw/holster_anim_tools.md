# KOTOR holster / ready animation tools (web research, 2026-10-13)
Web-only. Sources are mostly 2016-2019 forum posts. No tool was run.

## Tools (CLI / scriptable)
- https://deadlystream.com/files/file/779-mdlops/ : MDLOps CLI; -a skips anims, -s skin to trimesh, -k1/-k2 sets game. KTool's bundled build is ancient.
- https://deadlystream.com/topic/7475-editing-animations-and-rigging/ : MDLEdit 1.0.104b and MDLOps 1.0.2 cited as latest (2022 reply). Binary<->ASCII round trip is the editing path.
- https://deadlystream.com/files/file/889-kotorblender/ : KotorBlender Blender add-on, MDL import/export with anims; 4.0.3 fixes Blender 4.2/4.4. GUI-driven, no headless path found.
- https://deadlystream.com/files/file/1853-kotorblender-for-blender-33/ : Blender 3.3+ fork (3.10.3); reported export failures (duplicate nodes, empty file) on Blender 3.6 and 4.1.
- https://deadlystream.com/files/file/1151-kotormax/ : KOTORmax, 3ds Max/gmax MAXScript I/O with an animation editor. Max-only, not headless.
- https://pypi.org/project/PyKotor/ : PyKotor Python library for KotOR formats incl. MDL. Anim-specific API not verified; PyPI says MIT, the wiki says LGPL.
- https://synchro.codeberg.page/rakata/api/rakata_formats/mdl : rakata (Rust) MDL types incl. MdlAnimEvent and MdlAnimation; its ASCII writer targets mdledit/kotorblender.
- Blender headless (bpy): no KOTOR-specific example found. bpy imports once per process (blenderless note).

## Adding / reusing animations
- https://deadlystream.com/topic/6606-filling-in-in-animations-with-existing-animations/ : title match, not fetched. Duplicate an ASCII anim block (newanim...doneanim), rename, recompile.
- https://deadlystream.com/topic/7475-editing-animations-and-rigging/ : K2 bead-v (2019): list the new anim in animations.2da and call it from script. K1 unclear. Whole-model export with all anims is unconfirmed.
- https://deadlystream.com/topic/12309-modplayable-ewok-with-2-hand-animations/ : real custom combat-anim mod; its download has a tutorial.txt (not read).
- https://deadlystream.com/topic/6798-porting-animations-from-tsl-to-k1/ : TSL to K1 anim porting thread, title match only, not fetched.

## Engine facts
- https://www.xoreos.org/doxygen/df/de8/model__kotor_8cpp_source.html : readAnim offsets: name@8 (32B), nodes@40/44, type@76, animLength f32@82, transTime f32@86, animRoot@90 (read, not stored), event array offset@122 count@126. Event entries are never read.
- https://synchro.codeberg.page/rakata/internals/mdl_deep_dive.html : anim array ptr +0x58, count +0x5C; supermodel name char[32] @+0x88; ResetAnimation @0x004a0fb0. No events, animroot, or weapon-visibility content.
- https://synchro.codeberg.page/rakata/api/rakata_formats/mdl : defines an event as firing at a specific time in an animation. No field layout on the page.
- Supermodel lookup (forum, not engine-verified): model first, then supermodel chain, first match wins. Player chain Body > S_Female03 > S_Female02 > S_Female01 > S_Male02 > S_Male01 (7475 thread, 2019-12-05).

## Animation naming
- https://deadlystream.com/topic/4505-analysis-of-the-combat-animations/ : 4-part codes. Category b/c/f/g/m; weapon digit (2 single saber, 3 two-handed, 4 dual sabers); action a/d/g/n/p/r/w (r = ready, w = wield). K2 has ready anims per saber form. Posted 2016-06-18.
- Inference, unsourced: g1w1 parses as generic, stun-baton, wield. cwalk/cpause1/cready/cdisarm do not fit this scheme and are unconfirmed.

## Weapon visibility
- No source shows the engine hiding or showing a weapon from animation events. Blade visuals are baked into the model and animated (https://deadlystream.com/files/file/1847-new_lightsaber_blade_model_tsl).
- TSL floating saber is an animated rig that needs an equipped saber on the paperdoll (https://deadlystream.com/topic/6525-porting-lightsaber_floating-from-tsl-to-k1).
- Event-driven vs hardcoded: UNSURE.

## Gaps / next checks
- https://home.comcast.net/~cchargin/kotor/mdl_info.html : cchargin KotOR MDL spec, the likely source for event layout. Fetch refused (connection refused); archive.org also blocked.
- Decompile a vanilla humanoid MDL with MDLOps, then grep its ASCII for event and anim blocks. No web needed.
- Check weapon show/hide in swkotor.exe via rea-recon rather than guessing.
