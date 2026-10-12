# B2a pk_fix: K2 trimesh header 18 -> 10 bytes

## Synthetic results (venv python -I, patched copy)
- Unpatched venv package: `bytes_mdl` raises `ValueError: _TrimeshHeader.write wrote 348 bytes, expected 340`.
- Patched copy (`synth.py`, node name "tri"): length 725, dirt_texture 2. Log: `B2a/synth.txt`.
- Expected 732 NOT reproduced. Length tracks node name length 1:1: no name 722, "tri" 725, "tri_abcdef" 732.
- Writer's own K2 size assertion (K2_SIZE 340, io_mdl.py:1104) now passes; it failed at 348 before.

## Diff hunks (B2a_pk_fix.diff, scratch-copy line numbers)
- Hunk 1 `@@ -1309,8 +1309,6 @@` reader: removed `k2_tail_long1` / `k2_tail_long2` reads.
- Hunk 2 `@@ -1470,15 +1468,13 @@` writer: removed the two `k2_tail` writes; comment now says CCssL (10 bytes).
- Dirt block unchanged: uint8 enabled, uint8 pad, int16 texture, int16 worldspace, uint32 hologram (read and write).

## Notes
- K1 branch and K2_SIZE/K1_SIZE constants unchanged.
- `k2_tail_long1/2` attribute defaults (io_mdl.py:1196-1197) left in `__init__` as dead zeros.
- Verified only the writer's own size invariant and the round trip; engine-side byte layout not checked.
