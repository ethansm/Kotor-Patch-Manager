# W3 PyKotor K1 to K2 binary MDL conversion (recon)

Sources: OpenKotOR/PyKotor master (raw GitHub) and pykotor-2.3.12 sdist from PyPI. GitHub tag v2.3.12 does not exist (404), so the sdist was used for the pip version.

Q1 game/version param on binary write: VERIFIED. No game param. MDLBinaryWriter.__init__(mdl, target, target_ext) has none; self.game defaults to Game.K1 (io_mdl.py:3212) and write() auto-sets K2 only if a mesh has dirt/hologram fields (io_mdl.py:3237-3262; 2.3.12 sdist lines 3246/3250). write_mdl and bytes_mdl take no game arg (mdl_auto.py:443-448, 475-478). Only game default in the module is on the reader: Game.K2 (io_mdl.py:2295).

Q2 K2 writes 340 and K1 writes 332: REFUTED (static, not executed). Constants K1_SIZE=332, K2_SIZE=340 (io_mdl.py:1103-1104). K2 branch writes 18 bytes (io_mdl.py:1472-1481); K1 branch writes 2 bytes (1482-1483). Common tail is identical, so K2 is 16 bytes longer than K1. The check at io_mdl.py:1488-1491 expects only 8 more. Summed fields give K1=332, K2=348, so a K2 write raises ValueError("wrote 348 bytes, expected 340"). Same logic holds for 2.3.12 (io_mdl.py:1490).

Q3 ASCII read/write API: VERIFIED. io_mdl_ascii.py: class MDLAsciiWriter (313), write (341); class MDLAsciiReader (993), load (1038). Public dispatch in mdl_auto.py: read_mdl (262, ASCII branch 330), write_mdl (443, ASCII branch 468), bytes_mdl (475). Used in tests/resource/formats/test_mdl_ascii.py:1999.

Q4 documented or tested K1 to K2 path: VERIFIED, none found. test_model_parsers_against_mdlops.py finds K1 and K2 models separately (:89, :98) and round-trips same-game via write_mdl (:170). Its K2 test is skipped without a TSL install (:100). No K1/K2 handling in gl/models/mdl_converter.py or cli/commands/convert.py (2.3.12 sdist). Web and gh code search found no conversion function. Search was not exhaustive.

Side notes:
- A K1 MDL read and written by PyKotor keeps K1 defaults, so output is K1 layout (332), not K2.
- Reader K2 branch (io_mdl.py:1299-1321) reads the same 18-byte block, so real 340-byte K2 files may misalign (UNVERIFIED).
- Not run: no pip install (read-only brief). Executing a dirt-flagged mesh write would confirm the ValueError.
