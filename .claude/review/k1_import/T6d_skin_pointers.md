# T6d: skin pointer root cause (N_CommM ArmL, torso, ArmR)
Scratch: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/4b302fe0-554c-4823-af59-025f7d920a16/scratchpad/T6d/` (rawskin.py, locate.py, convert_run.py, skin_fix.diff). No commit. Repo patch not edited.

## 1. Confirmation (raw bytes, stdlib only)
Offsets are MDL-relative. Skin header = node+412 (K1, trimesh 332 B) or node+420 (K2, 340 B). "actual" = exact byte match of the K1 source array inside the output.

| File | Skin | bonemap ptr -> actual | qbones ptr -> actual | tbones ptr -> actual | unknown0 |
|---|---|---|---|---|---|
| K1 source | ArmL | 99179 -> 99179 | 99415 -> 99415 | 100359 -> 100359 | 101067 ok |
| K1 source | torso | 114283 -> 114283 | 114519 -> 114519 | 115463 -> 115463 | 116171 ok |
| K1 source | ArmR | 150719 -> 150719 | 150955 -> 150955 | 151899 -> 151899 | 152607 ok |
| K2 PoC (poc/pack) | ArmL | 110383 -> 111915 (+1532) | 110619 -> 112151 (+1532) | 111563 -> 113095 (+1532) | dropped (count 0) |
| K2 PoC | torso | 143903 -> 148123 (+4220) | 144139 -> 148359 (+4220) | 145083 -> 149303 (+4220) | dropped |
| K2 PoC | ArmR | 161467 -> 162999 (+1532) | 161703 -> 163235 (+1532) | 162647 -> 164179 (+1532) | dropped |

- K1 puts skin arrays right after the skin header, before faces. The writer puts them after vertindexes. PoC gap = indices_offsets 4 + counters 4 + vertindexes 254 x 6 = 1532 B (not the faces block: faces are 32 B and come earlier). Torso: 4 + 4 + 702 x 6 = 4220.

## 2. Root cause (patched venv file; stock has identical lines)
- Offsets: `pykotor/resource/formats/mdl/io_mdl.py:4180-4184` (`_calc_skin_offsets`) start skin blocks at the end of the vertex array.
- Data order: `_write_trimesh_data` at io_mdl.py:598 (vertices 680, indices_offsets 684, counters 688, vertindexes 692-698), then `_write_skin_extra` at :602 (def 799).
- `children_offsets_offset` (io_mdl.py:915-928) already follows the data order. Only skin pointers are wrong. Dangly constraints use the same pattern at io_mdl.py:4131 (untested).

## 3. Stock 2.3.12 (venv_pk), K1-layout write: bug present
Stock gives the same deltas: ArmL +1532, torso +4220, ArmR +1532 (`stock_k1_skins.json`, `loc_stock_k1.json`). The patch changes no skin code (4 hunks at lines 1309, 1468, 3252, 3830). Stock also drops unknown0 (708 B).

## 4. Fix (scratch only): skin_fix.diff
Change: `_calc_skin_offsets` starts at `children_offsets_offset() - skin_extra_size()`, i.e. the writer's data order. Pointers change, data order does not.
- Control (unpatched copy): MDL sha c8e96efe = PoC output. 0/9 pointers correct. Readback unequal at entry 0 for 9/9 arrays.
- Fixed K2 write: 9/9 pointers equal byte-exact positions (ArmL 111915/112151/113095; torso 148123/148359/149303; ArmR 162999/163235/164179).
- Readback (patched reader) equals K1 source for bonemap, qbones, tbones: 9/9 arrays, 59 entries each.
- Fixed vs control: only the 9 pointer fields differ (18 bytes). MDX identical (173d2b4d). Fixed MDL sha 02ff2cf9.
- K1-layout write with fix: 9/9 pointers correct. `patch -p1` on a copy of the venv file gives a byte-identical result.

Diff (`skin_fix.diff`, 21 lines):
```diff
--- a/pykotor/resource/formats/mdl/io_mdl.py
+++ b/pykotor/resource/formats/mdl/io_mdl.py
@@ -4176,12 +4176,12 @@
         bin_node: _Node,
     ) -> None:
         assert bin_node.skin is not None, "Skin node is required"
-        # Place skin variable payload blocks immediately after the vertex array.
-        after_vertices: int = node_offset + bin_node.vertices_offset(self.game)
-        if bin_node.trimesh is not None:
-            after_vertices += bin_node.trimesh.vertices_size()
-
-        cur: int = after_vertices
+        # _write_trimesh_data writes indices_offsets, counters and vertindexes AFTER the vertex array,
+        # and _write_skin_extra writes the skin blocks after those (and after dangly extras).
+        # children_offsets_offset() already accounts for that order, so the skin blocks start at
+        # children_offsets_offset() minus the skin block size. The old code used the end of the
+        # vertex array, which left the pointers 1532 B (ArmL) short of the data.
+        cur: int = node_offset + bin_node.children_offsets_offset(self.game) - bin_node.skin_extra_size()
         if bin_node.skin.bonemap_count:
             bin_node.skin.offset_to_bonemap = cur
             cur += int(bin_node.skin.bonemap_count) * 4
```

Open: fixed pointers keep the writer's order, not K1's (arrays before faces); engine acceptance untested. `_write_skin_extra` skips empty lists while `skin_extra_size` counts them (not hit here).
