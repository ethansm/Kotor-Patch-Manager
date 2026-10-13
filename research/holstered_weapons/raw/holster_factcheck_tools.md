# KOTOR tool claims fact-check (recon, 2026-10-13)

## 1. xoreos-tools 'nwnbif' / 'nwnerf'
- FALSE: no such tools. README lists unkeybif, keybif (KEY/BIF), unerf, erf (ERF), unrim, rim (RIM), unherf, unobb, untws, tws, ncsdecomp and others. https://github.com/xoreos/xoreos-tools
- Per-tool README names no game (KotOR only in TLK section and repo topics). BIF/KEY/ERF/RIM covered by name.
- MOD: UNVERIFIED. No MOD tool listed; .mod is ERF-family per rakata docs (https://synchro.codeberg.page/rakata/formats/archives/erf.html).

## 2. MDLedit (github bead-v/mdledit) batch/CLI, binary<->ASCII
- FALSE on repo: github.com/bead-v/mdledit returns HTTP 404. https://github.com/bead-v/mdledit
- Real listing: DeadlyStream file 1150, v1.0.3 (2017-11-02), author bead-v, self-described "(de)compiler for KOTOR and KOTOR2 model files". https://deadlystream.com/files/file/1150-mdledit/
- CLI/batch mode: UNVERIFIED. Page describes a GUI with an About help window; no flags documented.
- Platform: Windows XP+ build "through Wine" on MacOS; Linux not named. License: not stated. Source/GitHub: not mentioned.

## 3. KAurora / nwnmdlcomp / MDLOps
- KAurora decompile: FALSE. Supports KotOR I and II, but "The ascii export is disabled" (no binary->ASCII). ASCII import and binary export only for dummy, trimesh, walkmesh, light. Alpha, v13 (2019-08-14). https://deadlystream.com/files/file/703-kaurora/
- nwnmdlcomp for KotOR: UNVERIFIED. All sources cover Neverwinter Nights only. https://forum.neverwintervault.org/t/compiling-models-w-nwnmdlcomp/6079
- MDLOps: TRUE. Perl script, KotOR 1 and 2 binary<->ASCII, auto-detect, -k1/-k2 output, MDL+MDX must share a directory. v1.0.0 (2017-12-23), ndix UR. A KOTORmax listing mentions a "new version of MDLOps", so 1.0.0 is not confirmed latest. https://deadlystream.com/files/file/779-mdlops/
- github.com/ndix-ur/mdlops: HTTP 404 (repo URL not confirmed).

## 4. KotorBlender
- Two repos exist: TRUE. seedhartha/kotorblender (older) and OpenKotOR/kotorblender (maintained).
- seedhartha README: Blender 3.6 and 4.2, tested 4.4; "replacing ASCII model import/export with binary format"; animations and walkmeshes. https://github.com/seedhartha/kotorblender
- OpenKotOR README: Blender LTS 5.2, 4.5, 4.2, 3.6, 3.3, 2.93, 2.83; binary MDL import/export with animations and walkmeshes. https://github.com/OpenKotOR/kotorblender
- Binary MDL: TRUE. Animations: TRUE.
- Dummy/empty nodes: UNVERIFIED. Neither README mentions them. Unofficial forum post advises selecting the OdysseyBase root before export.
- Headless (blender -b --python): UNVERIFIED. Docs describe GUI menu steps only.

## 5. HoloPatcher (OldRepublicDevs/PyKotor)
- Open-source (LGPL-3.0-or-later), cross-platform, CLI: TRUE. https://pypi.org/project/holopatcher/
- CLI flags on PyPI page: --install ("without a UI window"), --validate, --console, --uninstall, --game-dir. Linux: "Windows/Mac/Linux will all produce the same patch results." Headless Linux: TRUE on docs, not run.
- Repo: fetched https://github.com/OpenKotOR/PyKotor ("Maintained by OldRepublicDevs organization"; README lists Linux). OldRepublicDevs/PyKotor URL itself not fetched. A search result names oldrepublicwizard/PyKotor as canonical (conflict, unresolved).
- changes.ini-driven install: UNVERIFIED. PyPI says only "INI files". Forum says TSLPatcher-compatible. https://deadlystream.com/topic/9807-toolholopatcher
- Latest: 1.5.3 (2025-12-02).

## 6. Supermodels and dummy nodes
- Names s_female01, s_female02, s_female03, s_male01, s_male02: TRUE (community, K1-leaning thread). Chain S_Female03 -> S_Female02 -> S_Female01 -> S_Male02 -> S_Male01; female bodies use S_Female03, male bodies use S_Female02. https://deadlystream.com/topic/7475-editing-animations-and-rigging/
- K2-specific names: UNVERIFIED.
- Supermodel header field: TRUE. Read as 32-byte _superModelName; "NULL" skipped. https://www.xoreos.org/doxygen/df/de8/model__kotor_8cpp_source.html
- rhand, lhand, headconjure, handconjure, rootdummy, torso_g, hook: UNVERIFIED. No source lists them. xoreos reads node names from the runtime name table.
- dummy_rhand prefix: UNVERIFIED, no evidence found. Treat as suspect.
- Settling path: MDLOps-decompile a copy of s_female02.mdl from the K2 install (game files needed, not done).

## Summary
- Other AI wrong: nwnbif/nwnerf names; KAurora decompile; MDLedit CLI and GitHub URL; MDLOps GitHub URL; nwnmdlcomp for KotOR (no evidence).
