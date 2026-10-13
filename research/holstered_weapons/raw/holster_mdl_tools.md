# Holster/hook MDL tooling recon (2026-10-13)
Scope: web only. ~17 searches, 10 fetches. Repo not touched. Last-commit dates not visible on any GitHub page fetched.

## Tools
- MDLOps (Perl; ndix UR, lineage cchargin/JdNoa/bead-v): https://deadlystream.com/files/file/779-mdlops/
  GPL per page; `perl mdlops.pl [-a] [-s] [-k1|-k2] model.mdl`; binary<->ASCII both ways; nodes: trimesh, danglymesh, lightsaber, skin, emitter, light, reference, dummy; supermodel must sit in same dir; no source repo found; 1.0.0 dated 2017-12-23.
- MDLOps bug reports: https://deadlystream.com/topic/12232-mdlops-vs-newer-tools/
  2026 thread: MDLOps 1.0.2 binary output reported mangled in-game (head models) even with no manual edits; unresolved.
- MDLedit (bead-v): https://deadlystream.com/topic/7590-creating-custom-head/
  Windows GUI, MDL+MDX <-> ASCII; bone-limit warning wrong (says 16; ~17 real); no public source found; not CLI.
- KOTORMax (MAXScript, 3ds Max/gmax): https://github.com/OpenKotOR/KOTORMax
  Writes ASCII MDL only; needs MDLOps to compile; nodes: trimesh, skin, danglymesh, AABB, emitters, lights, dummy/pivot, reference; license not named on page (LICENSE file exists); 51 commits, no date; do not load with NWmax.
- KotorBlender (Python, Blender add-on): https://github.com/seedhartha/KotorBlender
  GPL-3.0 (README); GUI-only, no CLI; binary MDL import/export with animations and walkmesh; ASCII NOT supported; tested Blender 3.6/4.2/4.4; apply object keyframes before export; no dummy or supermodel notes.
- PyKotor (Python, 3.8+): https://github.com/OpenKotOR/PyKotor
  LGPL-3.0-or-later on GitHub (PyPI says MIT: conflict); CLI `pykotor`/`pykotorcli`; MDL package has `io_mdl.py` and `io_mdl_ascii.py` (https://github.com/OpenKotOR/PyKotor/tree/master/Libraries/PyKotor/src/pykotor/resource/formats/mdl); round-trip, supermodel, dummy handling NOT verified; 7,287 commits, no date.
- reone (C++ engine): https://github.com/seedhartha/reone
  Not a converter; CLI `reone --game GAME_DIR`; visible model reader is binary only (`mdlmdxreader.cpp`); license not named (COPYING file). Not usable for round-trip.
- rakata (Rust, not in your list): https://synchro.codeberg.page/rakata/api/rakata_formats/mdl/ascii_writer/index.html
  Has ASCII MDL reader and writer, claims binary<->ASCII; reverse-engineered from swkotor.exe; license, CLI, commit date unchecked.
- xoreos / xoreos-tools: https://www.xoreos.org/doxygen/df/de8/model__kotor_8cpp_source.html
  KotOR model loader only (read); xoreos-tools has no MDL converter (https://wiki.xoreos.org/index.php/Running_xoreos-tools).
- NWN side: https://unrealarchive.org/wikis/unreal-wiki/Legacy:NeverWinter_Nights_To_UT2003.html
  nwnmdlcomp decompiles NWN MDL to .mdl.ascii; reported working on GOG, not Steam NWN:EE (https://forums.beamdog.com/discussion/comment/1145293/). nwnrs (Rust CLI, NWN, KotOR untested): https://docs.rs/crate/nwnrs/0.0.1

## Format docs
- Node header 80 bytes: node_type u16 @+0x00, name_index @+0x04, off_parent @+0x0C (content-relative); dummy = type 0x0001, bare header + controllers. Other codes: reference 0x0011, trimesh 0x0021, skin 0x0061, dangly 0x0121, AABB 0x0221, saber 0x0821. https://synchro.codeberg.page/rakata/internals/mdl_deep_dive.html ; https://synchro.codeberg.page/rakata/formats/models/mdl.html
- Supermodel: binary supermodel_name char[32] @+0x88, literal "NULL" = none; supermodel_ref @+0x64 is a leaked pointer (write 0); node_count counts the supermodel chain. Same deep-dive URL.
- ASCII writer keywords: `newmodel`, `setsupermodel <name> <super|NULL>`, `beginmodelgeom`, `node <type> <name>`, `parent <name|NULL>`, `endnode`, `donemodel`; nodes in DFS order; `node dummy` confirmed; `position`/`orientation` (axis-angle) only if no controller. https://synchro.codeberg.page/rakata/api/src/rakata_formats/mdl/ascii_writer.rs.html
- MDLOps: supermodel file must be in the same directory as the imported model. https://deadlystream.com/files/file/779-mdlops/

## Attach/hook node names
- CONFIRMED in sources: head_g, neck_g, necklwr_g (https://deadlystream.com/topic/7590-creating-custom-head/); "Mask hook", "google hook" (same thread, unnamed); lshandle01, lshandle07 (lightsaber hilt, https://deadlystream.com/topic/1643-how-to-convert-a-hilt-model-between-kotor-and-tsl/).
- headhook: xoreos dev says heads follow it (search snippet only): https://xoreos.org/pipermail/xoreos-devel/2017-August/000047.html
- NOT FOUND in any source (about 8 searches): rhand, lhand, torso_g, headconjure, handconjure. Treat as unverified.

## Caveats
- No GitHub repo found for MDLOps or MDLedit. No commit dates captured for any repo.
- No vanilla humanoid ASCII dump was fetched, so real attach-node names and parent chains are unconfirmed.
