OUTPUT: .claude/review/k1_import/T12e_engine_loader_v2.md
EVIDENCE: 0x00a050b8 (skinned VP, reads v[TEX4-6], no NRML, ev_d963adb5 xrefs 0x477290/0x477db4) ; 0x00476be0, 0x00477990, 0x00478300 (VP loaders into DAT_00a7bae0/ae4, pseudo-code) ; 0x0048caf0 (299 B, ASCII skin-field dispatcher "weights"/"qbone_ref_inv"/"tbone_ref_inv"/"boneconstantindices", not binary)
FINDING: Steam twins NOT found for CreateSkinNodeList, ExecuteSkinTransforms, HasTangentBases, CalculateTangentSpaceBasis, or binary MDL skin/trimesh parsers; Q1-Q5 all UNSURE; Steam VP anchors confirmed.
OPEN: Which function draws with DAT_00a7bae0 (0x004c5196, callees incl. 0x004c31a0, too large to decompile in this run) and where is the 29-byte bit-0x80 test?
VERDICT: PARTIAL
