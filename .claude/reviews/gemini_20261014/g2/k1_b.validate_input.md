# G2.3 skeptic input: k1_b (7 fids)

## K1-T18 (P2): Does _Checker.skin read weights/bone offsets with u32, flag only w>b (not w==b), and is SKIN_ARRAY_ORDER (validate.py:261) never asserted in test_k1_port_validate.py?
- **K1-5b-06** v2=CONFIRMED (G1 PARTIAL) | claimed medium logic | Skin row offset check uses u32 and w>b only; MDX overrun reported for one mesh | file patch_manager_mods/k1_port/k1_port_validate.py fn _Checker.trimesh / _Checker.skin
  - G1 evidence: patch_manager_mods/k1_port/k1_port_validate.py:264-265, :232-235, :271-274 | G1 note: u32 read and w>b-only check are true; w==b caught by BLOCK_OVERLAP only if vc>0; MDX report names one mesh by design
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d032: SKIN_ARRAY_ORDER is never referenced in test_k1_port_validate.py: untested
- **K1-5d-04** v2=CONFIRMED (G1 UNSURE) | claimed low test-gap | SKIN_ARRAY_ORDER finding never triggered by a test | file patch_manager_mods/k1_port/test_k1_port_validate.py fn _Checker.skin
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_validate.py:235-265 (no SKIN_ARRAY_ORDER); :257 asserts SKIN_ROW_ORDER | G1 note: No in-range assertion; rest of file outside range; anchor :257 is a different code
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d032: SKIN_ARRAY_ORDER is never referenced in test_k1_port_validate.py: untested

## K1-T19 (P2): Do _entries dedupe by (pack,path) not (shard,basename.lower()), letting main and creatures same-basename entries land in one shard so flatten_shard overwrites and run_shard's size check fails; and does no test call run_shard with dry_run False?
- **K1-5b-07** v2=UNSURE (G1 PARTIAL) | claimed medium logic | Same-basename files across subdirs/packs overwrite each other; run always fails | file patch_manager_mods/k1_port/k1_port_cr_extract.py fn flatten_shard / run_shard / _entries
  - G1 evidence: patch_manager_mods/k1_port/k1_port_cr_extract.py:125,129; 203-205; 296-303 | G1 note: Overwrite real; \\\"always fails\\\" overstated; equal-size collisions silent. Fix sound if winner deterministic and drops logged.
  - v2 evidence: patch_manager_mods/k1_port/k1_port_cr_extract.py:189; :192-194; :141 | v2 notes: [gem2 UNSURE] u026: Mechanism in range; reachability outside range. Fix key matches :189; needs deterministic winner and logged drops.
- **K1-5d-03** v2=CONFIRMED (G1 PARTIAL) | claimed medium test-gap | run_shard non-dry-run extraction, resume and manifest paths untested | file patch_manager_mods/k1_port/test_k1_port_cr_extract.py fn run_shard / cmd_run
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_cr_extract.py:213,233 (only run calls, both --dry-run); 178-200 (unit tests only) | G1 note: Non-dry run_shard, resume, row build untested in range; flatten and manifest unit-tested
  - v2 evidence: patch_manager_mods/k1_port/test_k1_port_cr_extract.py:233 | v2 notes: [det CONFIRMED] d033: Both run calls (213, 233) pass --dry-run; zero run_shard hits.

## K1-T20 (P3): Do the four test files skip the whole file with exit 0 when the hardcoded venv python is missing, ignoring K1PORT_VENV, including stubbed ingest tests that do not need pykotor?
- **K1-5d-01** v2=CONFIRMED (G1 PARTIAL) | claimed medium test-gap | Venv-missing SKIP exits 0 and ignores K1PORT_VENV; stubbed ingest tests skipped too | file patch_manager_mods/k1_port/test_k1_port_char_build.py fn main (also probe_build, spawn_build, ingest tests)
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_char_build.py:198-200; patch_manager_mods/k1_port/test_k1_port_probe_build.py:335-337 | G1 note: Exit-0 whole-file SKIP confirmed in 2 files; K1PORT_VENV and ingest parts unverified in range
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d034: Zero K1PORT_VENV hits; ingest honors only K1PORT_PY; SKIP paths documented exit 0. || [gem2 PARTIAL] u027: Exit-0 gate runs before TESTS loop, so all tests skip. K1PORT_VENV unverified; PY defined outside range.

## K1-T21 (P3): Does test_k1_port_probe.py lack tests for the active-manifest collision (probe.py:399), ambiguous cloudsaves slot (hits>1) and save-tree symlink checks?
- **K1-5d-05** v2=CONFIRMED (G1 UNSURE) | claimed medium test-gap | Active-manifest collision, ambiguous save slot and save symlink checks untested | file patch_manager_mods/k1_port/test_k1_port_probe.py fn compute_plan / plan_saves
  - G1 evidence: k1_port_probe.py:289-292, 401-402; test_k1_port_probe.py:182-191 | G1 note: Branches exist; file-wide test absence needs out-of-range read; anchor 399 is off
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d035: Zero symlink/ambig/active/collision hits in test_k1_port_probe.py.

## K1-T22 (P3): Does test_k1_port_ingest.py never exercise vanilla_lookup with a real install dir, so overlay-vs-vanilla_k1 attribution (ingest.py:116) is untested?
- **K1-5d-06** v2=CONFIRMED (G1 PARTIAL) | claimed medium test-gap | vanilla_lookup overlay attribution to flagged source never tested | file patch_manager_mods/k1_port/test_k1_port_ingest.py fn vanilla_lookup
  - G1 evidence: patch_manager_mods/k1_port/test_k1_port_ingest.py:524-535; patch_manager_mods/k1_port/k1_port_ingest.py:116,143-149 | G1 note: Anchor exact at 524. That test asserts refusal and no writes, not attribution. Whole-file \\\"never\\\" unverified.
  - v2 evidence: patch_manager_mods/k1_port/test_k1_port_ingest.py:533 | v2 notes: [det CONFIRMED] d036: Only the overlay-refusal assert; no allow_overlay=True run, no vanilla_lookup.
