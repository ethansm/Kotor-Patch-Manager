# T2: K1 vs K2 binary MDL/MDX (recon, read-only)
Method: WebSearch standard, WebFetch, and curl of raw source (grep only, no local writes). ModdingWiki host did not resolve.
Summary: binary format differs (K2 trimesh header +8 bytes). Gemini's "facial bones / skeletal rigs differ" not supported. Raw drop-in breakage not tested by any source.

## Q1 Is binary MDL/MDX identical K1 vs K2?
VERIFIED (differs in trimesh header). xoreos readMesh: `if (ctx.kotor2)` then `skip(8)` (lines 824-825). https://github.com/xoreos/xoreos/blob/master/src/graphics/aurora/model_kotor.cpp
PyKotor io_mdl.py: `K1_SIZE = 332` (line 1103), `K2_SIZE = 340` (line 1104). K2 branch replaces 2-byte `tail_short` with dirt/hologram fields (lines 1189-1195, 1299-1315). https://raw.githubusercontent.com/OpenKotOR/PyKotor/master/Libraries/PyKotor/src/pykotor/resource/formats/mdl/io_mdl.py
DeadlyStream 4501: "2 unknown UInt32's specific to Kotor2:TSL" in common mesh header (= 8 bytes). https://deadlystream.com/topic/4501-kotortsl-model-format-mdlmdx-technical-details
Skin MDX offsets 332 vs 340 per PyKotor comments (WebFetch summary, not grepped).
Caveat: PyKotor's own field comments do not sum to the 8-byte delta. Exact K2 field layout UNVERIFIED.
Node types: no per-game branch in xoreos flags. rakata MDL page documents K1 only, no K2 comparison. https://synchro.codeberg.page/rakata/formats/models/mdl.html
Bone/rig hierarchy: no per-game branch in either source. Gemini "skeletal rigs differ" unsupported.

## Q2 Different head/neck hook names K1 vs K2?
REFUTED (code-level only). No source names "headhook" for either game (grep and search: none).
PyKotor io_mdl.py uses `neck_g` for head-model detection (lines 2394-2399), no game branch.
xoreos reparents nodes "head" and "tongue" for both games (lines 422-426).
Game data itself not inspected.

## Q3 Raw K1 model crashes or breaks in K2, or works unconverted?
UNVERIFIED. No thread tests an unconverted K1 binary in K2.
9809 Juhani: K1 model saved in K2 mode via MDLEdit was invisible. Cause traced to TSL p_juhani.utc, fixed by remaking UTC. https://deadlystream.com/topic/9809-porting-juhani-to-tsl/
9809 DarthParametric: game "typically crashes when it tries to load a model that is missing or bad".
8809 DarthParametric: engine crashes on shadow-casting mesh over ~3-3.5K tris (not K1-specific). https://deadlystream.com/topic/8809-porting-free-models-from-the-internet-into-kotortsl/
Mouth: 9809 JCarter426 says lips "would likely need adjustment, as with other K1 heads" (no broken-animation claim).

## Q4 Port threads and tools: conversion steps (one line each)
4501: format reference only, no conversion steps.
1643: TSL->K1 hilt example. MDLOps 0.5 reads to ASCII in source mode, replacer in target mode, rename outputs.
12017: MDLEdit ASCII export and compile to K1 with K1 supermodel present; or Blender TSL export checkbox.
9809: K1->K2. MDLEdit save ASCII, load in K2 mode, save binary with K2 S_Female02 present; lips need adjusting.
8809: 3DS Max/GMax or Blender with KOTORMax/KOTORBlender; MDLEdit compiles ASCII to MDL/MDX; NWMax/MDLOps tutorials outdated.
xoreos: loader only, no conversion steps. K2 handled by `ctx.kotor2` flag.
PyKotor: MDL reader/writer take `game` param (default K2). Writer checks trimesh size against target game (io_mdl.py line 1488). No K1->K2 convert function seen (io_mdl.py partially read).
ModdingWiki: UNVERIFIED. moddingwiki.shiny.ca did not resolve; no search hit.
