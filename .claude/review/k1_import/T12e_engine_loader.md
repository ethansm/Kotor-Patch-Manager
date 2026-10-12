# T12e engine loader recon (swkotor2.exe)

Status: all five questions UNSURE. The brief's named anchors do not match this Ghidra database. No question could be answered from the named functions.

## Binary and session
- Opened: /home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe (open_binary, provider ghidra). A first guess under the repo directory returned ENOENT.
- sha256 7751f7d8b06c63a21eacbef6e5ded6cb4f497fa960cca6a6cf645552701dc04f, PE x86, Ghidra 12.1.4.
- No symbol names: search_procedures for "Skin", "Tangent", "Demand", "MdlNode", "ParseField" each returned 0 hits. Functions are FUN_xxxxxxxx.
- Linux-build names not checked. This session has no shell, so `scripts/query_ghidra.py` was not run.
- The brief's addresses may belong to a different swkotor2.exe build. Confirm the build before relying on them.

## Anchor check (resolve_containing_procedure, procedure_pseudo_code)

| Brief name | Brief addr | Containing entry (body) | What the decompile shows | Result |
|---|---|---|---|---|
| CreateSkinNodeList | 0x0083bb70 | FUN_0083baf0 (0x83baf0-0x83bbaf, 192 B) | Loops a list at this+0x44, count at +0x48; calls FUN_00832d90(...,3), FUN_00838bd0, FUN_009196fd | Not confirmed |
| ExecuteSkinTransforms | 0x008767f0 | FUN_008767a0 (0x8767a0-0x876900, 353 B) | Gated on param_1 == 0x5dc; vtable calls; no per-bone loop seen | Not confirmed |
| RenderSkinned | 0x00877a20 | FUN_008778f0 (0x8778f0-0x877b0a, 539 B) | Effect-duration lookup ("v_lightns_dur" ...), "fxbeam" | Mismatch |
| RenderSkinnedVP | 0x00878e50 | FUN_008789f0 (0x8789f0-0x878f0c, 1309 B) | "_head_hit", "_ground", "headconjure" effect routine | Mismatch |
| HasTangentBases | 0x00846f70 | FUN_00846f00 (0x846f00-0x84700a, 267 B) | Tests shorts at +0x38/+0x3a and five at +0x3c..+0x44 via FUN_00848c30; returns 1 if no slot used or any slot passes | Not confirmed as a tangent check |
| CalculateTangentSpaceBasis | 0x0083f540 | FUN_0083eb30 (0x83eb30-0x83faf2, 4035 B) | "fire%d", "bullethook%d", "start fire": FireGunCallback | Mismatch |
| CExoResMan::Demand | 0x0061b580 | FUN_0061a210 (0x61a210-0x61bea5, 7318 B) | Not decompiled | Unconfirmed |
| MdlNode ParseFieldDispatch | 0x00846160 | FUN_00846140 (0x846140-0x846199, 90 B) | Not decompiled | Unconfirmed |

## Questions

**Q1 (UV-less textured mesh): UNSURE.**
- Render side: the skinned VP text at 0x00a050b8 and 0x00a08cb0 contains `MOV o[TEX0], v[TEX0]`. The shader always reads texcoord 0. Found by xrefs on 0x00a050b8 (callers 0x477290 = FUN_00476be0, 0x477db4).
- Not determined: whether the engine binds a UV array for a UV-less mesh, and the CPU parse side.

**Q2 (tangent flag cleared): UNSURE.**
- Two skinned VP families exist. 0x00a08cb0 reads v[NRML] only. 0x00a050b8 reads v[TEX4..6] (tangent inputs).
- Both are created in FUN_00476be0 (0x477290) and FUN_00477990 (0x477e02) and stored in DAT_00a7bae0 and DAT_00a7bae4.
- The per-draw selector was not found. HasTangentBases and CalculateTangentSpaceBasis are not identified, so the gate is unknown.

**Q3 (unknown0 vs bonemap count): UNSURE.** No comparison of the skin unknown0 count with the bonemap count was found. The skin parser was not located.

**Q4 (weights-first MDX rows): UNSURE for the CPU offsets.**
- GPU side, ARB VP text at 0x009fb620: bone indices from vertex.attrib[4] (ARL A0.x), weights from vertex.attrib[1] (vWeight).
- GPU side, VP1.0 text at 0x00a050b8: indices from v[COL1], weights from v[WGHT].
- Parse_SkinVertexWeight and ExecuteSkinTransforms were not identified. The CPU use of MDX offsets 32 and 48 is unknown.

**Q5 (59 vs 60 bonemap entries): UNSURE.** The loop bound was not found.

## Unnamed parser in 0x008c6ff0..0x008ce2e0: not identified.
- Related: FUN_0048caf0 (0x48cb13, reached from xrefs on "weights" at 0x0098d5c0) is a stricmp field-name dispatcher. "weights" calls FUN_0048ef40 with this+0x1a4. "qbone_ref_inv" calls FUN_0048f0e0 (+0x1c0). "tbone_ref_inv" calls FUN_00429010 (+0x1cc). "boneconstantindices" calls FUN_0048ea70 (+0x1d8). It is gated on DAT_00a309bd == 0.
- This is outside the requested range. Its name-keyed dispatch suggests ASCII MDL text parsing. It is not confirmed as the binary MDX reader.

## Tool calls (this session)
- open_binary x2 (same Steam path, same session); close_binary at end.
- resolve_containing_procedure x8 (brief anchors).
- procedure_pseudo_code x7 (anchors) plus 0x48cb13, 0x477290, 0x478311, 0x477e02 (the last two are teardown and init, not per-draw).
- search_procedures x5 (0 hits each).
- search_strings x5 (skin, tangent, trimesh, weight, bonemap).
- xrefs x4 (0x00a050b8, 0x0098d5c0, 0x00a7bae0, 0x00a7bae4).

## Open
- Which per-draw code binds the TEX4..6 skinned VP (0x00a050b8) versus the NRML variant (0x00a08cb0). Candidate readers of DAT_00a7bae4 include 0x4c5310.
- The binary MDL skin parser in 0x008c6ff0..0x008ce2e0.
