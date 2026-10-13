# Holster pipeline tools (web recon, 2026-10-13)
Repo not touched. [UNVERIFIED] = search snippet only, page not fetched.

## 1. Extract / pack (BIF, KEY, ERF, RIM, MOD)
- xoreos-tools: https://github.com/xoreos/xoreos-tools. GPL-3.0+, C/C++ CLI (unerf, unrim, unkeybif, unherf). CI builds Linux (autotools + CMake). No GitHub Releases shown.
- xoreos 0.0.6 release note: https://www.xoreos.org/blog/2020/08/03/xoreos-0-dot-0-6-elanee-released/. Says rim/keybif create tools are highly experimental; extract is the reliable path.
- reone toolkit: https://github.com/seedhartha/reone and https://git.wbrawner.com/wbrawner/reone/wiki/Tooling. C++. reone-tools reads BIF/ERF/RIM and builds RIM/ERF/MOD [UNVERIFIED, from snippet]. Linux CI badge. License not on page (COPYING file present). Release date not found.
- PyKotor (pykotorcli): https://github.com/OpenKotOR/PyKotor. Python. LGPL-3.0-or-later per GitHub page; PyPI snippet said MIT (conflict, check LICENSE). CLI ships with `pip install pykotor`; only `2da2csv` seen as an example. No Releases shown.
- KotOR Stuff (ApanLoon): https://deadlystream.com/topic/5544-tool-kotor-stuff/. C#, CLIs: bif, erf (ERF/MOD/SAV/HAK), tpc to png. No RIM. Mono on non-Windows. Date and license unknown.
- KotOR Tool (Fred Tetra) and ERFEdit: no repo or Linux status found. Source: https://deadlystream.com/topic/6572-tools-inventory/ [UNVERIFIED].

## 2. Batch-patch 2DA / GFF
- PyKotor library (above): reads and writes GFF, 2DA, TLK, MDL via pykotor.resource.formats. Best scriptable route.
- HoloPatcher (TSLPatcher successor, TSLPatcher-compatible changes.ini): https://deadlystream.com/topic/9807-toolholopatcher. Open source; installs via `pip install holopatcher` per PyKotor README. Standalone GitHub repo not confirmed.
- TSLPatcher on Linux: https://deadlystream.com/topic/10885-ysk-tslpatcher-has-been-rewritten-in-python-and-made-open-source/. Python rewrite reportedly runs without Wine [UNVERIFIED].
- Known Linux quirk: non-UTF-8 info.rtf shows an error window but install still completes: https://deadlystream.com/topic/10959-modding-tsl-with-kotormodsync-on-linux/ [UNVERIFIED version].
- changes.ini LookupGameFolder / registry lookup is Windows-oriented; on Linux set the game path explicitly.

## 3. Install conflicts and file priority
- Override vs .mod priority (community guide, not official): https://deadlystream.com/topic/5329-folder-priorities-where-to-put-your-mods-files/. Order for .ut*/.are: savegame, then Override, then Modules/*.mod, then vanilla. A .git in Override resets the area on each load. Confirm in game.
- Body-slot conflicts: https://deadlystream.com/blogs/entry/424-workaround-for-the-limited-number-of-body-model-slots-in-kotor-and-tsl/. Slot J (Revan/Star Forge robes) and slot K (4 flight suits) are mostly taken. Slot L freed (Dancer outfit). Slot edits add appearance.2da rows and change modell/texl columns plus disguise .uti files.

## 4. MDL batch processing
- MDLOps: https://deadlystream.com/files/file/779-mdlops/. mdlops.exe takes one model or a *.mdl wildcard. Flags: -a skip anims, -s skin to trimesh, -k1 / -k2 output game. Binary MDL needs the .mdx in the same dir. Supermodel must be in the same dir. Perl version has same switches [Linux use UNVERIFIED]. License and date unknown.
- MDL format spec (clean-room): https://synchro.codeberg.page/rakata/formats/models/mdl.html. supermodel_name at +0x88; literal "NULL" means none. supermodel_ref: writer emits 0. node_count = own nodes + supermodel node_count + 1 (resolved chain).
- rakata MDL API: https://synchro.codeberg.page/rakata/api/rakata_formats/mdl. Parser library, Rust.

## 5. Model inventory and supermodels
- TSL players use medium (M) body models only. Letter codes: A underwear, B clothes, C-H armour, I/M/N robes, J Revan/Star Forge, K flight suit: https://deadlystream.com/topic/10967-kotor2-model-editing-help/ and https://www.nexusmods.com/kotor2/mods/1185.
- Female base body: pfbam.mdl [source: DeadlyStream underwear mod thread, UNVERIFIED].
- Full count of distinct humanoid body models: NOT FOUND in any source.
- SUPERMODEL column in appearance.2da: NOT FOUND. Blog above names modell/texl only.
- Supermodel geometry sharing: NOT CONFIRMED. Spec shows the header reference and node_count chain; it does not say geometry is inherited. Forum hint that subclass models reuse supermodel animations is anecdotal.
- Test to settle it: parse a subclass with PyKotor and compare its mesh nodes against its supermodel.
