# T3 verification: K1 supermodels and textures (Gemini claims A, B)
Key: VERIFIED | REFUTED | UNVERIFIED. ModdingWiki MDL page blocked (Anubis), not used.
Q1 supermodel is a name ref, needed at load time: VERIFIED | https://synchro.codeberg.page/rakata/internals/mdl_deep_dive.html ; https://raw.githubusercontent.com/xoreos/xoreos/master/src/graphics/aurora/model_kotor.cpp | header +0x88 char[32] name; "triggers a recursive FindModel"; xoreos loadSuperModel runs inside load(), throws if missing (reimplementation, not original binary).
Q1a same-directory rule: VERIFIED | https://deadlystream.com/files/file/779-mdlops/ | note 5, import only: supermodel or original "must be in the same directory".
Q1b example S_Female02 holds base anims for p_*/c_*: UNVERIFIED | none | no source names it; rakata says parent animations are available to child.
Q2 TSL uses same mechanism: VERIFIED | https://raw.githubusercontent.com/xoreos/xoreos/master/src/graphics/aurora/model_kotor.cpp | same loadSuperModel; kotor2 flag adds skip(8) in readMesh before offNodeData and is passed to supermodel. Rakata deep dive is K1-only.
Q3 TPC identical K1 vs K2: VERIFIED | https://raw.githubusercontent.com/xoreos/xoreos/master/src/graphics/images/tpc.cpp | 128-byte header, no game flag; encodings 0x01/0x02/0x04/0x0C; TXI footer read after payload.
Q3b TGA identical K1 vs K2: UNVERIFIED | https://www.nexusmods.com/kotor/mods/1565 | modder page says TGA uncompressed, TPC compressed; no K1-vs-K2 source found.
Q4 Override searched before vanilla: VERIFIED | https://raw.githubusercontent.com/OpenKotOR/PyKotor/master/Libraries/PyKotor/src/pykotor/extract/installation.py | resource() docstring: Override ranks above Chitin; first match wins.
Q4b same-name override replaces vanilla: VERIFIED | installation.py (same) | by precedence, not a direct statement; a K1 texture in K2 override shadows the K2 vanilla file for every model using that name (Gemini's collision).
Q4c TPC beats TGA same name: VERIFIED | https://www.nexusmods.com/kotor/mods/1565 ; installation.py | texture_types = (TPC, TGA), TPC checked first; Nexus says TPC wins.
Q5 texture name field size: VERIFIED | https://synchro.codeberg.page/rakata/internals/mdl_deep_dive.html ; model_kotor.cpp | texture_0/1 inline char[32], null-terminated; xoreos reads 32 bytes fixed. Max 31 chars plus NUL.
Q5b in-place hex edit keeps offsets: VERIFIED | mdl_deep_dive | inline fixed field, no offset table for trimesh textures (light texturenames is the pointer exception). Same-length or shorter NUL-padded edit is offset-neutral; longer names cannot fit.
Q5c ASCII round trip safer than hex: UNVERIFIED | https://deadlystream.com/topic/11879-porting-heads-from-k1-to-k2/ (search snippet only) | forum says set MDLedit to KOTOR2 before binary save; wrong game target breaks the K2 mesh skip(8). Round-trip correctness not tested.
Note: mdlops has a binary texture-rename function (search snippet only, UNVERIFIED).
Note: .tpc/.tga file rename and any .txi sidecar handling not checked.
