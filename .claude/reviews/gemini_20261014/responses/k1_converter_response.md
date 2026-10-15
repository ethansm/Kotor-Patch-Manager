### Step 1: File Verification

* **`5a_k1_port_source_a.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).




* **`5c_k1_port_tests_a.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).





---

### Step 2: Code Reviews

## `5a_k1_port_source_a.txt`

**Omitted files from `5a_k1_port_source_a.manifest.tsv`:**

* None (all 17 files listed in `5a_k1_port_source_a.manifest.tsv` are included in `5a_k1_port_source_a.txt`).



### Findings for `5a_k1_port_source_a.txt`

1. **`patch_manager_mods/k1_port/k1_port_tex_dxt.py` — `splice_colour**`

* **Anchor:** `ch = ch & ((p0 <= p1) == (n0 <= n1))`

* **What is wrong and concrete input/state that triggers it:** `encode_rgb` always produces 4-colour mode blocks (`c0 > c1`, via `_fit` forcing `n0 > n1`). In `splice_colour` for DXT1 (`fmt == "dxt1"`), `ch = ch & ((p0 <= p1) == (n0 <= n1))` checks whether the parent block and the newly encoded block have the same colour mode. Because `n0 <= n1` is always `False` for `encode_rgb` output, `splice_colour` **silently discards the RGB edit on every DXT1 parent block that happens to have `p0 <= p1**` (including fully opaque flat-colour blocks where the original encoder wrote `c0 == c1` with indices `0`, or 3-colour mode blocks with no transparent pixels), reverting those 4×4 blocks to the unmodified parent colours and leaving blocky un-recolored holes inside edited regions. Worse, if `new_colour` is passed directly with `n0 <= n1` over a `p0 <= p1` block that contains transparent (`index == 3`) pixels, `splice_colour` overwrites the block without preserving the index-3 bitmask, which then fails `k1_port_tex_gate.py`'s `_dxt1_block_diff` check (`index-3 (transparent) pixels changed`).


* **Suggested fix:** When a DXT1 parent block has `p0 <= p1` and has no `index == 3` transparent pixels, allow replacing it by flipping `p0, p1` or encoding in 4-colour mode (or re-encode in 3-colour mode `c0 <= c1` while preserving any `index == 3` transparent pixels from the parent block).


* **Confidence:** certain


2. **`patch_manager_mods/k1_port/k1_port_tex_tpc.py` — `parse` (and `patch_manager_mods/k1_port/k1_port_tex_gate.py` — `_parse_parent`)**

* **Anchor:** `if w < 1 or h < 1 or mips < 1:`

* **What is wrong and concrete input/state that triggers it:** Neither `k1_port_tex_tpc.parse` nor `k1_port_tex_gate._parse_parent` caps `mips` against the image dimensions (or a sane maximum like 16). Because `b[13]` is an unsigned 8-bit integer (`0..255`) and `mip_dims(w, h, k)` clamps `max(1, w >> k)` to `1x1` for all $k \ge \lfloor\log_2(\max(w, h))\rfloor$, a TPC whose `mips` byte is larger than the true mip pyramid depth (e.g., `mips = 20` on a `64x64` TPC that has a trailing TXI string) treats the first bytes of the ASCII TXI trailer as extra `1x1` DXT mip blocks (`8` or `16` bytes each). `apply_rgb_edit` then overwrites the start of the TXI trailer with DXT-encoded pixel blocks, corrupting the trailer while `_check_length` in `k1_port_tex_gate.py` still passes because `g.end` in the gate makes the identical miscalculation.


* **Suggested fix:** Validate that `mips <= max(w, h).bit_length()` (or stop the mip chain after the `1x1` level is reached) in both `k1_port_tex_tpc.parse` and `k1_port_tex_gate._parse_parent`.


* **Confidence:** certain


3. **`patch_manager_mods/k1_port/k1_port_tex_preview.py` — `load_texture**`

* **Anchor:** `raw = np.frombuffer(blob, dtype=np.uint8, count=w * h * bpp, offset=128).reshape(h, w, bpp)`

* **What is wrong and concrete input/state that triggers it:** In `load_texture`, when a `.tpc` file is uncompressed (`data_size == 0`, `ptype in (2, 4)`), it reads `mip 0` directly from byte offset 128 in stored row order without flipping rows, whereas for `.tga` files two lines above it explicitly flips bottom-origin TGAs (`arr[::-1]`) with the comment `# PIL returns top-row-first, undoing a bottom-origin TGA's row order; sampling expects the stored order (same as a TPC)`. However, when `dump_model` writes `.tga` files from `inst.resource(tex, ResourceType.TGA)` and `Image.open(tga).convert("RGB")` loads them in `load_texture`, a top-origin TGA (`head[17] & 0x20 != 0`) has row 0 at the top of the image (visual top, $v = 1$ in OpenGL/KotOR UV convention), whereas a TPC has stored row 0 at $v = 0$ (bottom-up). Moreover, `load_texture` only checks `head[17] & 0x20` on raw file bytes (`head = fh.read(18)`), which is wrong if the TGA has a non-zero `id_length` (`head[0]`) or colour map, or if `head` is RLE-compressed (`head[2] == 10`), wait—`head[17]` is always byte 17 of the 18-byte TGA header regardless of `id_length`, so `head[17]` is the descriptor byte, **but** if `head[17] & 0x10` (right-to-left horizontal origin bit 4) is set, `PIL` flips horizontally and `load_texture` does not undo it.


* **Suggested fix:** Check both origin bits in `head[17]` (bit 5 vertical, bit 4 horizontal) or read raw orientation consistently with `k1_port_uvmask.py`.


* **Confidence:** likely


4. **`patch_manager_mods/k1_port/k1_port_tex_gate.py` — `read_mask**`

* **Anchor:** `if len(raw) < h * (stride + 1):`

* **What is wrong and concrete input/state that triggers it:** `read_mask` checks only `len(raw) < h * (stride + 1)` instead of `len(raw) != h * (stride + 1)`. If a corrupted PNG has extra decompressed bytes in `IDAT`, `read_mask` silently ignores the trailing data instead of rejecting the malformed PNG stream. Additionally, `zlib.crc32(ctype + body)` in Python 3 returns an unsigned 32-bit int, which matches `>I`, so CRC check is fine, but `ihdr` is accepted even if multiple `IHDR` chunks appear or if `IHDR` is not the first chunk (`pos == 8`).


* **Suggested fix:** Require `IHDR` to be the first chunk at `pos == 8` and check `len(raw) != h * (stride + 1)` in `read_mask`.


* **Confidence:** certain


5. **`patch_manager_mods/k1_port/k1_port_tex_bake.py` — `_child_name**`

* **Anchor:** `return f"{prefix}{nn:02d}"`

* **What is wrong and concrete input/state that triggers it:** `_child_name` strips trailing digits from `atlas` (`prefix = re.sub(r"\d+$", "", atlas)`) and appends `{nn:02d}` starting at `FIRST_NN = 36`. It never checks that `len(f"{prefix}{nn:02d}") <= 16` (the KotOR ResRef 16-character limit enforced by the engine and `RESREF_MAX = 16` in `k1_port_model.py`, whereas `k1_port_tex_gate.py` only checks `MAX_NEW_LEN = 31` for MDL texture fields). For example, `ATLAS_FOR_MODEL["n_mandalorian"]` is `"n_mandalorian01"` (15 chars), which gives `prefix = "n_mandalorian"` (13 chars) + `36` = 15 chars, and if `nn >= 100` it reaches 16 chars; any slightly longer atlas prefix (15+ chars) produces a child TPC filename exceeding the 16-char CResRef limit, which the engine cannot load by CResRef while `k1_port_tex_gate.py` still passes it (since `MAX_NEW_LEN = 31`).


* **Suggested fix:** Check `len(child) <= 16` (or `MAX_NEW_LEN`) in `_child_name` and raise `BakeError` if the generated child resref exceeds the limit.


* **Confidence:** certain


6. **`patch_manager_mods/k1_port/k1_port_uvmask.py` — `build_masks**`

* **Anchor:** `tex = getattr(mesh, "texture_1", None)`

* **What is wrong and concrete input/state that triggers it:** `build_masks` skips nodes whose `texture_1` is `""` or `"null"`, assuming every remaining textured node in the model uses the target `--atlas` passed to `k1_port_uvmask.py build --model MODEL --atlas ATLAS`. However, multi-texture models (such as characters with separate head/body/accessory textures or weapons/hooks in the same MDL) have nodes whose `mesh.texture_1` is a completely different texture from `--atlas`. `build_masks` merges the UVs of **all** textured nodes into the masks regardless of whether `mesh.texture_1` matches `--atlas`, polluting the UV island masks (`masks.json` and `<label>.png`) with UV triangles belonging to other textures.


* **Suggested fix:** Filter or optionally restrict nodes in `build_masks` whose `mesh.texture_1` does not match the target atlas family/prefix (or record per-texture masks when a model references multiple textures).


* **Confidence:** likely


7. **`patch_manager_mods/k1_port/k1_port_model.py` — `pad_skin_bones**`

* **Anchor:** `n = real_bone_count(skin)`

* **What is wrong and concrete input/state that triggers it:** `real_bone_count(skin)` counts how many entries in `skin.bonemap` are `>= 0` (`sum(1 for b in skin.bonemap if int(b) >= 0)`), and `pad_skin_bones` zeroes `skin.bone_indices[n:16]`, assuming the active bone slots in `bones[16]` are always densely packed in `0..n-1` and that `n` equals the count of non-negative `bonemap` entries. If a skin mesh has sparse bone serials in `bonemap` (e.g., `max(bonemap) + 1 > n`) or if two nodes map to the same bone serial, `n = real_bone_count(skin)` does not equal `max(int(b) for b in skin.bonemap if int(b) >= 0) + 1`, so `slots[:n]` can zero out a highest-numbered bone slot that a vertex weight actually references.


* **Suggested fix:** Compute the active slot count as `max((int(b) + 1 for b in skin.bonemap if int(b) >= 0), default=0)` (or the max bone index referenced by `skin.bonemap` and `skin.vertex_bones`), so no referenced slot in `bones[16]` can be zeroed.


* **Confidence:** likely



### Files or Areas Not Assessed for `5a_k1_port_source_a.txt`

* **`k1_port_sources.py` (`Ledger`, `SourceError`, `get_source`) and `sources.json**`: Imported by `k1_port_model.py` and `k1_port_textures.py`, but not included in this bundle.


* **`scripts/npc_div_texture_sheets.py`**: Imported lazily by `k1_port_tex_bake.py` (`_default_pack_lookup`) and `k1_port_uvmask_overlay.py` (`load_atlas`), but not included in this bundle.


* **Unpatched portions of `pykotor` (`io_mdl.py`, `mdl_data.py`, `tpc`)**: Only the diff hunks in `pykotor_k2_mdl.patch` were provided in `<documents>`.



---

## `5c_k1_port_tests_a.txt`

**Omitted files from `5c_k1_port_tests_a.manifest.tsv`:**

* None (all 13 files listed in `5c_k1_port_tests_a.manifest.tsv` are included in `5c_k1_port_tests_a.txt`).



### Findings for `5c_k1_port_tests_a.txt`

1. **`patch_manager_mods/k1_port/test_k1_port_tex_dxt.py` — `gradient_and_noise_round_trip` and `real_k1p_commm04_vs_pykotor**`

* **Anchor:** `print(f"INFO k1p_commm04 mip0 mean abs diff vs pykotor RGBA {mae(rgba, ref):.2f} (no threshold)", flush=True)`

* **What is wrong and concrete input/state that triggers it:** `real_k1p_commm04_vs_pykotor` decodes `k1p_commm04.tpc` with both `k1_port_tex_dxt.decode` (`rgba`) and `pykotor` (`ref`), checks only that `rgba.shape == ref.shape`, and prints `mae(rgba, ref)` with `(no threshold)` and **no assertion on pixel values or error**. Even if `decode()` returned all zeros or completely inverted colours on the real TPC file, `real_k1p_commm04_vs_pykotor` would still pass. Similarly, in `gradient_and_noise_round_trip`, the random-noise round-trip only checks `out_n.shape == (256, 256, 4)` and prints `mae(out_n[..., :3], noise)` with `(no threshold)`, which cannot fail on bad pixel output.


* **Suggested fix:** Assert a bounded MAE (e.g., `check(mae(rgba, ref) < 2.0, ...)` and `check(np.array_equal(rgba[..., 3], ref[..., 3]), ...)` in `real_k1p_commm04_vs_pykotor`, and a reasonable upper bound in `gradient_and_noise_round_trip`) so that a broken decoder fails the test.


* **Confidence:** certain


2. **`patch_manager_mods/k1_port/test_k1_port_tex_dxt.py` — `splice_dxt1_keeps_parent_block_when_colour_mode_would_flip` (and `test_k1_port_tex_tpc.py` — `test_red_tint_decode`)**

* **Anchor:** `new[0, 3] = np.frombuffer(three, dtype=np.uint8) ^ 1  # 3-colour over 3-colour: spliced`

* **What is wrong and concrete input/state that triggers it:** `splice_dxt1_keeps_parent_block_when_colour_mode_would_flip` hand-crafts a fake `new[0, 3]` with `c0 <= c1` (`3-colour over 3-colour`) to test `splice_colour`, and in `test_k1_port_tex_tpc.py::test_red_tint_decode`, `make_parent` generates uniform random bytes for the DXT1 blocks (where ~50% of blocks randomly have `c0 <= c1`) and only asserts `check(red_shift.mean() > 120)` averaged across the whole 16×16 mask! Because `encode_rgb` **never** outputs `c0 <= c1` (it forces `c0 > c1` in `_fit`), in the real `apply_rgb_edit` pipeline (`encode_rgb` $\to$ `splice_colour`), **100% of DXT1 parent blocks that had `c0 <= c1` are silently skipped and left unedited**. In `test_red_tint_decode`, ~half of the masked DXT1 blocks completely fail to turn red (staying random noise), yet the test still passes because the *other* half (~50% of blocks with `c0 > c1`) brings the mean `red_shift` just above `120` (`~127`)! Neither test checks per-block or minimum pixel error on DXT1 edits when parent blocks have `c0 <= c1`.


* **Suggested fix:** In `test_k1_port_tex_tpc.py::test_red_tint_decode`, assert that **every** pixel (or block) inside `mask` on an opaque DXT1 texture is updated toward `(255, 0, 0)` (e.g., `check(red_shift.min() > 100)`), and add a test with an opaque DXT1 parent block where `c0 == c1`.


* **Confidence:** certain


3. **`patch_manager_mods/k1_port/test_k1_port_tex_gate.py` — missing failure tests for `_check_header` and `_check_length**`

* **Anchor:** `ALL_IDS = ["dims", "mips", "format", "header", "length", "name", "txi_sidecar", "alpha"]`

* **What is wrong and concrete input/state that triggers it:** `k1_port_tex_gate.py` implements 9 checks (`dims`, `mips`, `format`, `header`, `length`, `name`, `txi_sidecar`, `alpha`, and `mask_scope`). `test_k1_port_tex_gate.py` has negative (`expect_only`) tests that trigger failures for `dims`, `mips`, `format`, `name`, `txi_sidecar`, `alpha`, and `mask_scope`, but **never tests a failure of `header` (`_check_header`) or `length` (`_check_length`)**. Note that `dropped_last_mip_fails_only_mips` patches byte 13 (`mips` from 9 to 8) without truncating the child's byte length, and `_check_length` uses `g.end` from the *parent*, so `len(child) == len(parent)` and `child[g.end:] == parent[g.end:]` both stay true! Consequently, if `_check_header` or `_check_length` in `k1_port_tex_gate.py` were changed to unconditionally return `ok=True`, every test in `test_k1_port_tex_gate.py` would still pass.


* **Suggested fix:** Add two unit tests in `test_k1_port_tex_gate.py`:
```python
@test
def changed_header_reserved_byte_fails_only_header():
    p = make_tpc(64, 64, 4, 7)
    expect_only(run(p, patch(p, 20, b"\x01")), "header")

@test
def changed_trailer_or_length_fails_only_length():
    p = make_tpc(64, 64, 4, 7, trailer=b"envmaptexture x\r\n")
    expect_only(run(p, p[:-1] + b"X"), "length")
    expect_only(run(p, p + b"extra"), "length")

```


* **Confidence:** certain


4. **`patch_manager_mods/k1_port/test_k1_port_model.py` — missing end-to-end test for skin meshes (`bonemap`, `qbones`, `tbones`, `unknown0`, `vertex_bones`, `pad_skin_bones`) through `MDLBinaryWriter` / `MDLBinaryReader**`

* **Anchor:** `def make_mdl(spec):`

* **What is wrong and concrete input/state that triggers it:** Six of the most critical hunks in `pykotor_k2_mdl.patch` (and `pad_skin_bones` in `k1_port_model.py`) modify how skin nodes are read and written in binary MDL/MDX: `_calc_skin_offsets`, `unknown0` read/write, MDX weights-before-bones order (`offset_to_mdx_weights` at `+0`, `offset_to_mdx_bones` at `+16`), and `bones[16]` padding. However, `HELPER.make_mdl` in `test_k1_port_model.py` only creates `MDLNodeType.TRIMESH` nodes (`node.skin` is always `None`), and the synthetic `PURE_TESTS` only test `compare()` and `pad_skin_bones()` on in-memory dicts/SimpleNamespaces. The only test in `test_k1_port_model.py` that ever exercises a binary skin node is `test_golden_n_commm_matches_k2_vanilla_skin_and_planes`, which **skips** whenever the local K1 or K2 Steam installation is absent (`if not (k1 / "chitin.key").is_file() or not (REAL_K2 / "chitin.key").is_file(): raise Skip(...)`). On any CI or host without both game installs present, binary skin reading/writing in `pykotor_k2_mdl.patch` has zero test coverage and cannot fail.


* **Suggested fix:** Extend `HELPER.make_mdl` in `test_k1_port_model.py` to support a synthetic `skin` spec (setting `bonemap`, `qbones`, `tbones`, `unknown0`, `bone_indices` with junk in unused slots, and `vertex_bones`), and add a synthetic end-to-end CLI test verifying that `unknown0`, weights-before-bones MDX layout, `_calc_skin_offsets`, and `bones16_slots_zeroed` round-trip accurately without needing the real game installs.


* **Confidence:** certain


5. **`patch_manager_mods/k1_port/test_k1_anim_pose.py` — missing tests for `cmd_pose`, `cmd_table`, `_rot_angle_deg`, and `_wrap180**`

* **Anchor:** `TESTS = [v for k, v in sorted(globals().items()) if k.startswith("test_") and callable(v)]`

* **What is wrong and concrete input/state that triggers it:** `test_k1_anim_pose.py` tests the lower-level math and track helpers (`build_tracks`, `pose_nodes`, `model_chain`, `find_anim`, `matrix_to_euler`), but never calls `_rot_angle_deg`, `_wrap180`, `_sample_times`, `cmd_pose`, or `cmd_table` (which contains the relative hand-to-bone Euler range calculation `vals = [e0[ax] + _wrap180(e[ax] - e0[ax]) for e in rel_eulers]` and `bone_dev_max`). If `_rot_angle_deg`, `_wrap180`, or the relative transform math `_matmul(_transpose(rb), rh)` in `cmd_table` were broken, `test_k1_anim_pose.py` could not fail.


* **Suggested fix:** Add a synthetic test in `test_k1_anim_pose.py` (stubbing `Session.load` with an in-memory model containing `rhand` and `pelvis_g` and a 2-key animation) that runs `cmd_pose` and `cmd_table` and asserts the computed JSON pose and Euler range / `bone_dev_max` output.


* **Confidence:** certain


6. **`patch_manager_mods/k1_port/test_k1_port_tex_audit.py` — `gap_texture` classification branches and missing `--masks` default not tested**

* **Anchor:** `def test_audit_writes_png_and_stats_per_atlas() -> None:`

* **What is wrong and concrete input/state that triggers it:** `test_k1_port_tex_audit.py` has only two tests, which verify that `<atlas>.png` and `<atlas>.json` exist and check basic JSON keys. It never checks the pixel classifications of `gap_texture(rgb, painted, skin, any_island)` — specifically that unpainted non-skin content inside an island is coloured red `(255, 40, 40)`, unpainted skin-like content inside an island is coloured yellow `(255, 220, 0)`, unpainted content outside every island is coloured cyan `(0, 230, 230)`, and dark background (`luma <= BG_LUMA`) or painted pixels are dimmed `(rgb * 0.3)`. If `gap_texture` swapped its colours or inverted `~any_island`, `test_k1_port_tex_audit.py` could not fail.


* **Suggested fix:** Add a direct unit test for `au.gap_texture` in `test_k1_port_tex_audit.py` with a 4-pixel synthetic image covering each of the 4 cases (painted, unpainted non-skin in island, unpainted skin in island, unpainted outside island) and asserting exact RGB values.


* **Confidence:** certain



### Files or Areas Not Assessed for `5c_k1_port_tests_a.txt`

* **`test_k1_port_validate.py`**: Imported dynamically by `test_k1_port_model.py` (`test_golden_n_commm_matches_k2_vanilla_skin_and_planes`), but not included in this bundle.


* **External game installations (`swkotor` and `Knights of the Old Republic II`) and staged files (`~/Games/KOTOR2_Mod_Workspace/...`)**: Referenced by optional real-data tests that skip when absent.