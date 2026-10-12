| Model | Stock pykotor 2.3.12 | Patched (B2a pk_fix) |
|---|---|---|
| p_bastilah (36 nodes) | no exception; first mesh `necklwr_g`, dirt_texture=1, total_area=0.0, vertex_count=0 | no exception; `necklwr_g`, dirt_texture=1, total_area=0.04000000283, vertex_count=24 |
| p_carthh (35 nodes) | no exception; first mesh `tongue`, dirt_texture=1, total_area=2.16e-41, vertex_count=0 | no exception; `tongue`, dirt_texture=1, total_area=0.0, vertex_count=22 |
| p_hk47 (58 nodes) | no exception; first mesh `InnerTorso`, dirt_texture=1, total_area=0.0, vertex_count=0 | no exception; `InnerTorso`, dirt_texture=1, total_area=0.18869513, vertex_count=68 |

Verdict: patched reader parses real TSL MDLs with plausible geometry (every mesh node has vertices; area > 0 on 23/27, 23/26, 45/45 mesh nodes; the zero areas are clean 0.0 on meshes with hundreds of faces, not garbage), while stock returns vertex_count=0 and zero/denormal area on the first mesh of every model.
