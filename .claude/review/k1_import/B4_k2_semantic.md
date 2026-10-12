p_bastilah: MATCH (36 nodes, 27 trimesh, 1822 verts, 1684 faces, anims 5->5; fields: name, node_type, child_count, textures, vertex/face counts, dirt_*, hologram flags, pos min/max 4dp)
p_carthh: MATCH (35 nodes, 26 trimesh, 1135 verts, 1082 faces, anims 1->1; same fields as above)
p_hk47: MATCH (58 nodes, 45 trimesh, 3247 verts, 2467 faces, anims 79->79; same fields as above)
VERDICT: 3 of 3 MATCH at model-summary level. Caveat: dirt_enabled is False and dirt_texture/worldspace are 1 in all inputs, so dirt fields are untested; normals, UVs, face indices, controllers and walkmesh are not compared.
