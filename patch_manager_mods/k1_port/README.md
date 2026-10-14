# K1 port: patched pykotor for writing KOTOR 2 MDL

Purpose: pykotor 2.3.12 can read K2 MDL but fails to write it
(`_TrimeshHeader.write wrote 348 bytes, expected 340`). This patch fixes the write.

## Run

    patch_manager_mods/k1_port/setup_venv.sh

Creates `~/Games/KOTOR2_Mod_Workspace/venvs/k1port` (override: `K1PORT_VENV=/path`),
installs `pykotor==2.3.12` and `py7zr`, applies the patch, prints the venv python.
Re-running is safe: an already-applied patch is skipped.

## What the patch changes

`pykotor/resource/formats/mdl/io_mdl.py` and `mdl_data.py`:
- Reader and writer drop the two `k2_tail` uint32 fields after the K2 dirt fields.
- `MDLBinaryWriter.write()` honors an optional `writer.force_game` after game auto-detect.
- `_update_mdx`'s UV1 zero-fill runs only when `writer.synth_zero_uv` is true (stock default). `k1_port_model.py` sets it false; its `--synth-zero-uv` flag turns it back on.
- Textured meshes with no UV1 stay UV-less by default, matching K1 (n_commm: 11 meshes). Only `texture_1` is guarded; the `texture_2` UV2 fill still zero-fills.
- Skin bonemap/qbones/tbones pointers follow the writer's data order (`_calc_skin_offsets` starts at `children_offsets_offset() - skin_extra_size()`). Before this, n_commm ArmL pointers were 1532 B short and read back as face bits.
- Skin MDX rows: weights at +32, bone indices at +48 (vanilla K1/K2 order; header offsets match; stride unchanged). Engine acceptance is untested.
- Skin `unknown0` (u32 array after tbones) is kept: the reader fills `MDLSkin.unknown0`, the writer sets count == count2 == its length and includes it in the skin block size and pointers. n_commm MDL = K1 + 352 B (8 per trimesh header), no 708 B gap. Engine acceptance is untested.
- Tangent block hunk (`mdl_data.py` `MDLMesh.tangent_block` + writer emit): the reader keeps each vertex's raw 36-byte MDX tangent block and the writer re-emits it after the UV blocks, restoring the K1 stride (68 / 100 B) and offset. Flag-only meshes are unchanged. Engine acceptance is untested.
- All-zero tangent block (llk_LenaH, 9 meshes): the writer clears flag 0x80 and omits the block (stride 68 -> 32, 100 -> 64, offset -1). Any non-zero row keeps the block. `writer.keep_zero_tangents = True` restores the old zero-filled block. See `.claude/review/k1_import/T6i_tangent_zero_rows.md`.

## Ingest one model and its textures

    ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_ingest.py \
        --resref n_commm --new-resref k1p_commm --texture N_CommM04=k1p_commm04 --also-hd [--dry-run] [--json]

1. Candidates: `vanilla_k1` (the K1 install, read by pykotor) plus every registered source in `sources.json`
   whose `local_dir` exists. Textures: `OLD.tpc` or `.tga`. Models: `<resref>.mdl` and `.mdx` in one folder.
   Recursive, case-insensitive.
2. Choice (`k1_port_sources.pick`): an approval-free source always wins over a flagged one. Then the lower
   class rank, then the higher quality (width x height). The output lists each rejected candidate and why.
3. Staging: textures to `<stage>/override/`, the model to `<stage>/models/`. The subtools register every file
   in the ledger (`<stage>/ledger.json`).
4. `--also-hd`: a higher-quality variant that needs approval goes to `<stage>/_hd/override/<NEW>.tpc`, flagged.
5. Re-runs report `already_ingested` and write nothing. `--dry-run` writes nothing and prints the decisions.
   `--no-model` ingests textures only.

Stage default: `~/Games/KOTOR2_Mod_Workspace/k1_port_stage`. Refused inside the repo, under steamapps/common,
or in compatdata. Nothing is written to a game directory.

## Flagged files and removal

Classes `permission_required`, `personal_use_only` and `unknown` need the author's approval. Their ledger
entries carry `removal_group: pending_author_approval`. Such files must not ship until approved.
`credit_required` sources are not flagged; their credits are listed by `credits`.

    python3 patch_manager_mods/k1_port/k1_port_prune.py list [--flagged-only]
    python3 patch_manager_mods/k1_port/k1_port_prune.py prune            # dry-run; add --apply to delete
    python3 patch_manager_mods/k1_port/k1_port_prune.py approve --source-id ID --note "who, where, when"
    python3 patch_manager_mods/k1_port/k1_port_prune.py gate PATH [PATH ...]   # exit 1 on a flagged match
    python3 patch_manager_mods/k1_port/k1_port_prune.py credits

Known gap: `gate` also blocks an approval-free file that shares a flagged file's name. The `_hd` variant keeps
the NEW name, so `gate` blocks the approval-free `override/<NEW>.tpc`. Only sha256 matches are reliable.

## Naming

K2 already ships `n_commm` and other K1 resrefs, so converted assets use `k1p_` names. Model default:
`k1p_` + the lowercase K1 resref, cut to 16 characters (`n_commm` -> `k1p_n_commm`). Textures: lowercase,
at most 31 characters, `--texture OLD=k1p_...`.

## Texture output gate and UV masks (B0, B1)

Overlay edits of NPC textures (overlay, not repaint) must pass a gate that compares the child TPC with its parent,
never with a constant: parents vary (1024 DXT5, 512 DXT1, 256 DXT5 atlases).

- `k1_port_tex_gate.py check --parent P --child C [--mask M.png] [--json]` (stdlib; exit 0 pass, 1 gate failure,
  2 usage). Checks dims, mips, format, header, length and trailer, name rule, no `.txi` sidecar, DXT5 alpha halves
  (DXT1: 1-bit-alpha block class), and with a mask every block outside the mask footprint is byte-identical.
- `k1_port_tex_dxt.py` and `k1_port_tex_tpc.py` (numpy, run with the repo `.venv`): own block codec and raw TPC
  editor. `apply_rgb_edit(parent, rgb, mask)` splices only colour halves of blocks the mask touches, at every mip,
  and never touches DXT5 alpha. pykotor cannot bake: its DXT encoder is lossy (pure red decodes to (49,0,54)) and
  its DXT1 decode swaps palette entries 2 and 3, so use it only to reload a result as a validity check.
- `k1_port_uvmask.py build --model n_commm --atlas n_commm05 --size 512 --flip 0 --out DIR` (patched k1port venv,
  pure Python; the stock reader misreads K2 MDX offsets): per-label masks `<label>.png` in STORED row order plus
  `masks.json`. Nodes with out-of-range UVs or a NULL texture are skipped. `torso` is the whole body node;
  `torso_arm` and `legs` are nested sub-islands of it. Mask size must equal the atlas mip 0 size for the gate.
- `k1_port_uvmask_overlay.py render --masks DIR --atlas NAME_OR_FILE --out DIR` (repo `.venv`): per-label overlays and
  `contact.png` for human review. Output goes outside the repo.

Verified 10-12 on the pilots n_commm05, n_sithcom01, n_mandalorian01: masks align with the atlas islands with
`--flip 0` (masks and gate both use stored row order); the in-game V direction is still the user's T5 test.

## Caveat

K1 HD overlay (`scripts/k1_hd_overlay.py`): a local-only install dir holding flagged Characters Revisited files in
its `override/`. It is never an `--install-k1` / `--from-install` without `--allow-overlay-install` (the tools
refuse it, and with the flag attribute its `override/` files to `characters_revisited_main`, never `vanilla_k1`).
`k1_port_ingest.py --prefer-source <id>` lets a flagged source outrank vanilla for textures; `gate` still blocks CR shas.

Never commit K1 pack assets (textures, models, archives) to this repo.
Only the patch and scripts belong here.

See [38_k1_to_k2_asset_import.md](../38_k1_to_k2_asset_import.md).
