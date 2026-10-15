# G2.3 skeptic input: k1_a (17 fids)

## K1-T01 (P2): Does DXT1 splice_colour (dxt.py:229) drop an RGB edit on any changed block whose parent has c0<=c1, given encode_rgb/_fit always emits c0>c1, so apply_rgb_edit leaves those blocks unedited?
- **K1-5a-01** v2=CONFIRMED (G1 CONFIRMED) | claimed high logic | DXT1 splice skips RGB edits on parents with c0<=c1 since encode_rgb is always 4-colour | file patch_manager_mods/k1_port/k1_port_tex_dxt.py fn splice_colour
  - G1 evidence: patch_manager_mods/k1_port/k1_port_tex_dxt.py:229 | G1 note: Real drop, deliberate alpha guard; removing it makes transparent index 3 opaque
  - v2 evidence: patch_manager_mods/k1_port/k1_port_tex_dxt.py:229 | v2 notes: 
- **K1-5c-02** v2=UNSURE (G1 PARTIAL) | claimed high test-gap | Red-tint test passes on mean while half of DXT1 blocks stay unedited | file patch_manager_mods/k1_port/test_k1_port_tex_dxt.py fn splice_dxt1_keeps_parent_block_when_colour_mode_would_flip / test_red_tint_decode
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_tex_tpc.py:255 | G1 note: Mean-only check is true; half-unedited and pass status unverified (make_parent, apply_rgb_edit)
  - v2 evidence: patch_manager_mods/k1_port/k1_port_tex_tpc.py:132-133; patch_manager_mods/k1_port/k1_port_tex_tpc.py:105-106 | v2 notes: [gem2 UNSURE] u018: In-range wiring and random endpoints hold; splice_colour and red-tint test are out of range

## K1-T02 (P2): Do tpc.parse (line 40) and gate._parse_parent (line 87) accept a mips byte larger than log2(max(w,h))+1 so that _layout counts trailing TXI bytes as 1x1 mip blocks?
- **K1-5a-02** v2=CONFIRMED (G1 PARTIAL) | claimed high logic | mips byte not capped to pyramid depth; TXI trailer bytes treated as 1x1 mips and overwritten | file patch_manager_mods/k1_port/k1_port_tex_tpc.py fn parse / gate._parse_parent
  - G1 evidence: patch_manager_mods/k1_port/k1_port_tex_tpc.py:34,40,44-53,61; patch_manager_mods/k1_port/k1_port_tex_gate.py:72-75,86-91 | G1 note: No pyramid-depth cap in either parser; overwrite step is downstream, unverified.
  - v2 evidence: patch_manager_mods/k1_port/k1_port_tex_gate.py:311 | v2 notes: [det CONFIRMED] d024: Only PNG bit-depth checks at gate.py:311-312; no pyramid-depth cap in tpc.py or gate.py || [gem2 PARTIAL] u019: No depth cap in edit loop or gate; overwrite hinges on parse() offsets/trailer, outside range

## K1-T03 (P2): In preview load_texture, are an uncompressed TPC (stored order) and a TGA (flipped by head[17]&0x20) given inconsistent row order, or is the horizontal-origin bit (0x10) ignored?
- **K1-5a-03** v2=PARTIAL (G1 PARTIAL) | claimed medium logic | Preview TGA/TPC row-order and horizontal-origin handling inconsistent | file patch_manager_mods/k1_port/k1_port_tex_preview.py fn load_texture
  - G1 evidence: patch_manager_mods/k1_port/k1_port_tex_preview.py:308, :318-319 | G1 note: Row order consistent (both stored order); 0x10 horizontal bit not checked here, relies on PIL
  - v2 evidence: patch_manager_mods/k1_port/k1_port_tex_tpc.py:35-36, :27-34, :67-70 | v2 notes: [gem2 PARTIAL] u020: Uncompressed TPC rejected at parse (uncompressed mismatch refuted); byte 17 never read (true); effect unverified

## K1-T04 (P3): Does gate.read_mask accept a PNG with decompressed IDAT longer than h*(stride+1) or an IHDR that is not the first chunk?
- **K1-5a-04** v2=CONFIRMED (G1 CONFIRMED) | claimed low logic | read_mask accepts PNG with extra IDAT bytes and non-first or multiple IHDR | file patch_manager_mods/k1_port/k1_port_tex_gate.py fn read_mask
  - G1 evidence: patch_manager_mods/k1_port/k1_port_tex_gate.py:318; patch_manager_mods/k1_port/k1_port_tex_gate.py:302-303 | G1 note: Both parts true. Extra bytes ignored, pixels unaffected. Exact-length and single-leading-IHDR checks match PNG spec.
  - v2 evidence: patch_manager_mods/k1_port/k1_port_tex_gate.py:318; patch_manager_mods/k1_port/k1_port_tex_gate.py:302-303 | v2 notes: 

## K1-T05 (P2): Can _child_name return a name longer than 16 chars for any atlas in ATLAS_FOR_MODEL with FIRST_NN=36 allocation, with no length check anywhere in bake?
- **K1-5a-05** v2=CONFIRMED (G1 PARTIAL) | claimed medium logic | Child texture resref not length-checked against 16-char limit | file patch_manager_mods/k1_port/k1_port_tex_bake.py fn _child_name
  - G1 evidence: patch_manager_mods/k1_port/k1_port_tex_bake.py:173-183 | G1 note: No length check in _child_name; visible atlases give <=15-char names at nn 36-99
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d025: No 16-character check or 16 literal in k1_port_tex_bake.py: child-name length unguarded || [gem2 PARTIAL] u021: Pre-write cap or length guard before :211 is correct; a cap in cmd_bake validation keeps nothing-written semantics.

## K1-T06 (P2): Does build_masks include UVs of nodes whose texture_1 differs from the target atlas, with no filter on texture name?
- **K1-5a-06** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | build_masks merges UVs of all textured nodes regardless of target atlas | file patch_manager_mods/k1_port/k1_port_uvmask.py fn build_masks
  - G1 evidence: patch_manager_mods/k1_port/k1_port_uvmask.py:131, :166-170 | G1 note: Right idea; needs atlas argument and normalised name compare as at :167
  - v2 evidence: patch_manager_mods/k1_port/k1_port_uvmask.py:131, :166-170 | v2 notes: 

## K1-T07 (P2): Can pad_skin_bones zero a bones[16] slot that a vertex weight references, when bonemap has non-contiguous bone serials so real_bone_count < max serial+1?
- **K1-5a-07** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | pad_skin_bones may zero referenced bone slots when bonemap is sparse | file patch_manager_mods/k1_port/k1_port_model.py fn pad_skin_bones
  - G1 evidence: patch_manager_mods/k1_port/k1_port_model.py:262-269 | G1 note: Code zeroes slots n..15 with no max-serial guard; triggers only if serials sparse.
  - v2 evidence: patch_manager_mods/k1_port/k1_port_model.py:262-269 | v2 notes: 

## K1-T08 (P3): Do real_k1p_commm04_vs_pykotor and the random-noise round-trip contain no assertion on MAE/pixel values (only shape checks and INFO prints)?
- **K1-5c-01** v2=PARTIAL (G1 PARTIAL) | claimed medium test-gap | DXT decode tests print MAE without any threshold assertion | file patch_manager_mods/k1_port/test_k1_port_tex_dxt.py fn real_k1p_commm04_vs_pykotor / gradient_and_noise_round_trip
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_tex_dxt.py:257; :96; :92 | G1 note: Prints at 257 and 96 unthresholded; line 92 asserts gradient MAE<8, so blanket claim overstated
  - v2 evidence: patch_manager_mods/k1_port/test_k1_port_tex_dxt.py:96 | v2 notes: [det PARTIAL] d026: Lines 96 and 257 are INFO prints with no threshold; checks at :92, :107 and :239 do threshold MAE: blanket claim overstated

## K1-T09 (P3): Does test_k1_port_tex_gate.py lack any test that makes the header or length gate check fail, so forcing _check_header/_check_length to ok would pass all tests?
- **K1-5c-03** v2=CONFIRMED (G1 UNSURE) | claimed medium test-gap | No failing-case tests for gate header and length checks | file patch_manager_mods/k1_port/test_k1_port_tex_gate.py fn ALL_IDS checks
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_tex_gate.py:25 (anchor exact); patch_manager_mods/k1_port/k1_port_tex_gate.py:110-125 | G1 note: Failing-case tests sit past test line 80, outside read range; coverage unverified.
  - v2 evidence: patch_manager_mods/k1_port/test_k1_port_tex_gate.py:25 | v2 notes: [det CONFIRMED] d027: header and length appear only in ALL_IDS (:25); no test makes either fail

## K1-T10 (P3): Does test_k1_port_model.py lack any synthetic test where node.skin is non-None through MDLBinaryWriter/Reader, with the golden skin test skipping when installs are absent?
- **K1-5c-04** v2=UNSURE (G1 PARTIAL) | claimed medium test-gap | No synthetic end-to-end test for skin MDL/MDX read-write patch hunks | file patch_manager_mods/k1_port/test_k1_port_model.py fn make_mdl / skin read-write
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_model.py:46-76; :877-883 | G1 note: in-range make_mdl never sets skin; golden skin test is install-gated; file-wide absence unverified
  - v2 evidence: patch_manager_mods/k1_port/test_k1_port_model.py:78-85; :805; :812-814; :823-826; :96-112 | v2 notes: [gem2 UNSURE] u022: Writer build() gets its model from make_mdl (out of range); in-range skin only in writer-free trees.

## K1-T11 (P3): Does test_k1_anim_pose.py never call cmd_pose, cmd_table, _rot_angle_deg or _wrap180?
- **K1-5c-05** v2=CONFIRMED (G1 PARTIAL) | claimed low test-gap | cmd_pose, cmd_table, _wrap180 and _rot_angle_deg are untested | file patch_manager_mods/k1_port/test_k1_anim_pose.py fn cmd_pose/cmd_table/_rot_angle_deg/_wrap180
  - G1 evidence: patch_manager_mods/k1_port/test_k1_anim_pose.py:1-175; patch_manager_mods/k1_port/k1_anim_pose.py:73, :79 | G1 note: Direct helper tests safe (_wrap180 :79, _rot_angle_deg :73); cmd_* unverified.
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d028: No test references cmd_pose, cmd_table, _rot_angle_deg or _wrap180: helpers untested

## K1-T12 (P3): Does test_k1_port_tex_audit.py never assert gap_texture output colours (red/yellow/cyan/dim)?
- **K1-5c-06** v2=CONFIRMED (G1 CONFIRMED) | claimed low test-gap | gap_texture colour classification branches not asserted | file patch_manager_mods/k1_port/test_k1_port_tex_audit.py fn gap_texture
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_tex_audit.py:41-54,57-66; patch_manager_mods/k1_port/k1_port_tex_audit.py:62-69 | G1 note: No test calls gap_texture; asserts only PNG existence and size. Anchor exact at :41.
  - v2 evidence: patch_manager_mods/k1_port/test_k1_port_tex_audit.py:41-54,57-66; patch_manager_mods/k1_port/k1_port_tex_audit.py:62-69 | v2 notes: 

## K1-T13 (P2): Does cmd_gate/is_flagged ignore local_only/personal_only ledger flags and exclude .utc/.2da/.ncs from ASSET_SUFFIXES so such files pass gate --strict?
- **K1-5b-01** v2=CONFIRMED (G1 PARTIAL) | claimed high logic | Release gate ignores local_only/personal_only, .utc/.2da/.ncs files and sha mismatch | file patch_manager_mods/k1_port/k1_port_prune.py fn cmd_gate / is_flagged
  - G1 evidence: patch_manager_mods/k1_port/k1_port_prune.py:39,43-44,268-289 | G1 note: Suffix and is_flagged gaps real; blocks suffix-independent; sha-mismatch warn fails strict; flag semantics unverified.
  - v2 evidence: patch_manager_mods/k1_port/k1_port_sources.py:269 | v2 notes: [det CONFIRMED] d029: local_only set at sources.py:269 with no reader in prune or ingest; personal_only has zero hits || [gem2 UNSURE] u023: Conditional on sign-off clearing it (287); removal_group inheritance via add() unread; prune.py:43 unread

## K1-T14 (P2): In ingest existing_row, does a file already in the stage but absent from the ledger get approved=True and removal_group=None, excluded from flagged_files and credits?
- **K1-5b-02** v2=PARTIAL (G1 PARTIAL) | claimed high logic | Already-ingested file missing from ledger is treated as approved and unflagged | file patch_manager_mods/k1_port/k1_port_ingest.py fn existing_row / ingest
  - G1 evidence: patch_manager_mods/k1_port/k1_port_ingest.py:448-449 | G1 note: approved=True, removal_group=None, credits=[] confirmed at :448; flagged_files exclusion needs code outside range.
  - v2 evidence: patch_manager_mods/k1_port/k1_port_ingest.py:257; :490; :491 | v2 notes: [gem2 PARTIAL] u024: Unflagged, no-credit mechanics confirmed in range; ledger-absent-to-approved trigger lies outside range.

## K1-T15 (P2): Does _build_appearance's t.set_label(NEW_ROW, NEW_LABEL) replace the numeric row id (812) with a name and leave the label data column empty, contrary to char_build and the test asserting it?
- **K1-5b-03** v2=PARTIAL (G1 PARTIAL) | claimed high logic | Probe appearance row overwrites numeric row id and leaves label column empty | file patch_manager_mods/k1_port/k1_port_probe_build.py fn _build_appearance
  - G1 evidence: patch_manager_mods/k1_port/k1_port_probe_build.py:106-107 | G1 note: Mechanics true: row id replaced, label cell unset. Deliberate per test:159; engine impact unverified.
  - v2 evidence: /home/ethansm/Games/KOTOR2_Mod_Workspace/venvs/k1port/lib/python3.12/site-packages/pykotor/resource/formats/twoda/twoda_data.py:552 | v2 notes: [det PARTIAL] d030: add_row blanks absent headers and uses given row_label (:552); set_label step is not in the output || [re UNSURE] re.md K1-T15: 2DA row-label handling not located

## K1-T17 (P2): Does execute_rollback unlink created files in plan order without restoring/verifying replaced files first, so a failing restore leaves a partial rollback with no rolled_back mark and k1p_probe_latest.json untouched?
- **K1-5b-05** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | Rollback deletes created files before restores verify; partial failure leaves mixed state | file patch_manager_mods/k1_port/k1_port_probe.py fn execute_rollback / cmd_rollback
  - G1 evidence: patch_manager_mods/k1_port/k1_port_probe.py:549-557 | G1 note: Deletes run in manifest order; restore mismatch raises before rolled_back mark; anchor is L556 not L553.
  - v2 evidence: patch_manager_mods/k1_port/k1_port_probe.py:549-557 | v2 notes: 
