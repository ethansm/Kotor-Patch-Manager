# T2b: K1 supermodels vs KOTOR 2 (S_Female02, S_Female03, S_Male02)

Method: read-only. K1 = `swkotor`, K2 = `Knights of the Old Republic II`. k1port venv absent, so used fallback `venv_pk` with `B2a/pk_fix` (sys.path set in-script because `-I` drops PYTHONPATH). Scripts in scratchpad `T2/`.
K1 reader: pykotor `read_mdl` and `read_mdl_fast` both raise `OSError: This operation would exceed the streams boundaries` in `_get_node_order` for all 3 K1 supermodels. K1 anim names therefore come from a raw MDL header walk (anim array at header+8/+12, name at anim header+8). Validated: K2 3/3 exact order match with pykotor; K1 sample of 349 models pykotor parses, 349/349 exact match.

| Supermodel | K2 chitin | K2 Override loose copy | K1 anims | K2 anims (chitin / Override) | Overlap | Missing in K2 |
|---|---|---|---|---|---|---|
| S_Female02 | yes | yes, md5 differs, same 88 names | 43 | 88 / 88 | 35 | 8 |
| S_Female03 | yes | yes, md5 differs, 70 names | 64 | 68 / 70 | 64 | 0 |
| S_Male02 | yes | yes, md5 differs, 232 names | 166 | 230 / 232 | 165 | 1 |

Overlap = K1 names found in K2 set (case-insensitive). Missing = K1 names with no K2 match. Missing sets are identical against chitin and Override.

Missing in K2 (K1 names):
1. S_Female02 (8): f2a1, f2a2, f3a1, f3a2, f3a3, f4a1, f4a2, f4a3. K2 has suffixed variants only (f2a1a, f2a2a, f3a1a-c, f4a1a-c).
2. S_Male02 (1): f2a3. K2 has f2a3a, f2a3b, f2a3c.
3. S_Female03 (0): none.

Presence: each name exists as an exact (case-insensitive) MDL in K2 chitin. K1 Override has no s_* MDL files. K2 Override holds loose copies of all three; S_Female03 and S_Male02 Override anim lists add names not in chitin (dead3, emotions, g4r1, g4w1, talkprone, talkpronedie; animloop04-05, c10a1-5, c10n1-2, cut015 and others).
