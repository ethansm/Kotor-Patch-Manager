# K1b: K1 to K2 conversion on real K1 NPC models

Run: `S/venv_pk/bin/python -I -B S/K1b/k1b_run.py 2>&1 | rg -av DEBUG | tee S/K1b/run.log`
Results JSON: `S/K1b/k1b_results.json` (S = scratchpad). Reader: patched pk_force. Writer: `MDLBinaryWriter`, `force_game = Game.K2`.

| Model | Read K1 (mdl only / +mdx) | Output len (bytes) | Summary vs K1 | Output token pair |
|---|---|---|---|---|
| p_bastilah | OK / OK | 137708 | MATCH (36 nodes, anims 5->5) | K2 (4285200, 4216320) |
| p_carthh | OK / OK | 79078 | MATCH (35 nodes, anims 1->1) | K2 (4285200, 4216320) |
| n_darthmalak | OK / OK | 2253044 | MATCH (111 nodes, anims 66->66) | K2 (4285200, 4216320) |

Token pairs (`io_mdl.py`, `_GeometryHeader`, read at file offset 12 and 16):
- K1: `K1_LAYOUT_TOKEN0 = 4273776` (line 227), `K1_LAYOUT_TOKEN1 = 4216096` (line 232). Input: (4273776, 4216096) in 3 of 3.
- K2: `K2_LAYOUT_TOKEN0 = 4285200` (line 228), `K2_LAYOUT_TOKEN1 = 4216320` (line 233). Output: (4285200, 4216320) in 3 of 3.
- Trap: lines 1106-1118 are `_TrimeshHeader` tokens (different values), not the geometry pair.

Supermodel (header string only, supermodel file not read):

| Model | Header name | Found in output bytes | Read-back name |
|---|---|---|---|
| p_bastilah | P_BastilaBB | yes | P_BastilaBB |
| p_carthh | S_Female02 | yes | S_Female02 |
| n_darthmalak | NULL | yes (weak: 4-byte substring) | NULL |

VERDICT: PASS. 3 of 3 read, 3 of 3 MATCH, 3 of 3 output carry the K2 pair.
Caveats: step 1 mdl-only read succeeds, so it does not prove vertex data; summary covers counts, bounds, textures, dirt/hologram flags and anim count only.
