# B22 Opus gate part a (lead, inline)
B22 = I10a re-gate: 4 B20 graph corrections, Camera::MapToScreen (unique fp twin, never batched), 2 fresh graphpair rows,
6 B21 likely (AUTO here, no cards). 7 REVIEW cards in 2 chunks, 13 decoys, 0 decoys accepted.
- Graph check (scratch gcheck.py: mapped callee + caller hits via ingest_names.trusted_map, best rival from mapped
  neighbours) on ALL 13 rows.
- The 4 corrections: the gate top lost again in every case (0-1 hits) and the B20 graph pick won with fp identical.
  In the cards the graph pick sat as a decoy (e.g. B22-1-06 0x004356e0): Haiku marked it unsure (exact size + imm), not match.
  3 -> verified at the graph pick, AurEnableShadows -> likely (single hit, rival ties).
- Haiku matches (MapToScreen, GetLowResolutionTimer, ~CSWCModule) agree with the graph (fp identical, no higher rival).
- 6 B21 likely rows: graph unchanged since B21 (0 hits or rival ties) -> stay likely.
- No verified row contradicted by the graph.
sample_errors: 0
