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

## Import a character from your K1 install

The importer turns characters from your own KOTOR 1 install into a `.k2pack` file, then copies that file into your KOTOR 2 install. The repo holds the tool and the recipes. It holds no converted asset. Your packs stay on your machine.

### Prerequisites

| Need | Used for | Set up |
|---|---|---|
| k1port venv (pykotor 2.3.12) | resolve, model converter, 2DA rows | Run `patch_manager_mods/k1_port/setup_venv.sh`. Default python: `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python`. Override with `K1PORT_PY`. |
| Repo `.venv` with numpy | texture step (TGA to DXT1 TPC) | Create `.venv` in the repo root, then `pip install numpy`. Override with `REPO_VENV_PY`. |
| Python 3.11 or newer | the driver (`tomllib`) | Host Python. |
| Proton and MDLOps | only recipes with `converter = "mdlops"` (juhani) | Steam "Proton - Experimental" and the MDLOps tools. `doctor` reports what is missing. |

### Steps

Run every command from the repo root.

1. Copy the example config to a private place. Never commit the copy.

   ```bash
   mkdir -p ~/.config/k1_port
   cp patch_manager_mods/k1_port/k1_imports.example.toml ~/.config/k1_port/k1_imports.toml
   ```

   Set `[k1] dir`, `[k2] dir` and `[build] out` in the copy. Keep `out` outside any git work tree and outside both game dirs. `build` refuses an `out` that fails this check.

2. Check the setup. Fix every `FAIL` line before you build.

   ```bash
   python3 patch_manager_mods/k1_port/k1_port_import.py doctor
   ```

3. Build the packs. Writes only to `out`. Add `--only <id>` for one character. Add `--dry-run` to print the plan and write nothing.

   ```bash
   python3 patch_manager_mods/k1_port/k1_port_import.py build
   ```

4. Install the packs. Close the game first. This is the only step that writes to the game dir.

   ```bash
   python3 patch_manager_mods/k1_port/k1_port_import.py install
   ```

5. Start the game with the `character-packs` patch installed. Check its log.

   ```bash
   python3 scripts/kotor2_logs.py check character-packs_log.txt --new
   ```

6. Remove one character. Close the game first. Deletes only a pack that this tool installed, and drops its row ids.

   ```bash
   python3 patch_manager_mods/k1_port/k1_port_import.py remove <id>
   ```

`list` shows, for each character, whether it is built, whether it is installed, and its row ids. Exit codes: 0 ok, 1 failure (the message names the location, the cause and the fix), 2 safety refusal.

### What each command writes

| Command | Writes | Where |
|---|---|---|
| `doctor` | nothing | |
| `build` | packs, manifests, logs, row ids | `[build] out` |
| `install` | one copy of each pack, `installed.json` | `<K2>/k1_imports/` |
| `remove <id>` | deletes that pack, drops its row ids | `<K2>/k1_imports/`, `<out>/assignments.json` |
| `list` | nothing | |

The out dir looks like this:

```text
<out>/
  <id>.k2pack                 the pack; read back and checked before it is kept
  <id>.import_manifest.json   sources, flagged items, texture hashes, pack sha256, tool versions
  assignments.json            character id -> appearance and heads row ids
  installed.json              packs this tool installed (path, sha256)
  <id>/licence_report.json    what resolve found in the K1 install
  <id>/logs/                  one log per subprocess step
  <id>/                       work dir for one character; wiped on each build
```

### Row ids

- Appearance and heads row ids come from the live K2 tables `override/appearance.2da` and `override/heads.2da`.
- Character n in the `characters` list gets row `start + n`. `start` is the live row count, or `first_appearance_row` and `first_head_row` when set.
- `build` keeps the ids in `assignments.json` and reuses them.
- Append a new id at the end of `characters`. An id inserted earlier moves the rows of the ids after it, and `build` refuses.
- If the live tables change, or the config order changes, `build` refuses. Restore the old state, or run `build --reassign`. `--reassign` rebuilds every character.
- `remove` leaves its row ids unused until that character is built again.
- While `juhani-char` is installed, set `first_appearance_row = 821` and `first_head_row = 252`. `juhani-char` owns appearance rows 818-820 and heads rows 249-251. `doctor` and `build` warn when it is found.

### 2DA cell budget

The asset-vfs 2DA pool holds 1024 cells since asset-vfs 0.1.3-dev (256 before), and every mod shares it. A character uses about 102 cells, so about 10 fit into an empty pool. `juhani-char` also takes cells from this pool (107 for Juhani, 102 for Carth, 102 for its twin), so fewer fit while it is installed. `build` prints the total. It warns when the total goes over 1024. Packs past the limit get a partial row, and `character-packs_log.txt` shows `REGISTER failed`.

### What a pack does not do

- **Spawning.** A pack holds models, textures, a UTC, a DLG, 2DA cells and the character definition. Nothing spawns it. A patch spawns the character through `k2.characterlib@1` (`Spawn`). No spawn is set from the config yet.
- **Audio.** The recipe `[audio]` table is reserved and not read. The pack format can hold WAV, LIP and SSF files. The importer does not write them yet.

### Pick any K1 NPC (no recipe needed)

The importer is NPC-agnostic. Search the K1 creature templates, preview what it would rip, then add the NPC to your config:

```bash
python3 patch_manager_mods/k1_port/k1_port_import.py npcs --grep bastila     # K1 UTC resref, tag, name, models, soundset, conversation
python3 patch_manager_mods/k1_port/k1_port_import.py scan p_bastilla          # prints the derived recipe (models, textures, bark audio)
```

In your private config, write `characters = ["carth", "utc:p_bastilla"]`. An entry with a committed `recipes/<id>.toml` uses that file (an override); a `utc:<resref>` entry is derived from the K1 UTC, `appearance.2da`, `heads.2da`, `soundset.2da` and the meshes at build time, and the derived recipe is saved to `<out>/recipes_auto/<id>.toml`. Defaults: MDLOps converter, `k1p_` names, the K1 SSF barks (plus every `<ssf>_*` wav in `streamsounds/`), fallback soundset 11 or 57 by gender. The conversation resref is only reported; dialog, voice-over and lip import are a later phase.

### Add a recipe

1. Copy `patch_manager_mods/k1_port/recipes/carth.toml` (mdlops, with `[audio]`) or `recipes/juhani.toml` (mdlops) to `recipes/<id>.toml`. `<id>` matches `[a-z0-9_]{1,31}`.
2. Set the K1 model and texture names, and the K2 names the importer writes.
3. Leave out row numbers. The builder assigns them.
4. Append `<id>` to `characters` in your private config.
5. Dry-run the character, then build it:

   ```bash
   python3 patch_manager_mods/k1_port/k1_port_import.py build --dry-run --only <id>
   python3 patch_manager_mods/k1_port/k1_port_import.py build --only <id>
   ```

Recipe keys:

| Table | Keys | Rules |
|---|---|---|
| `[character]` | `id`, `display_name`, `utc`, `soundset`, `template_row`, `gender` (required); `tag`, `dlg` (optional) | `display_name` is 1 to 63 printable ASCII characters. `utc` and `dlg` are resrefs `[a-z0-9_]{1,16}`. `soundset` and `template_row` are integers (`soundset` cannot be `"auto"`). `gender` is 0 or 1. |
| `[models]` | `converter` (`pykotor` or `mdlops`), `body_a` (required), `body_b`, `head`, `keep_dangly` | Each model is `{ k1 = "...", k2 = "..." }`. The builder needs `head`. `keep_dangly` (true or false, default false) makes the pykotor converter keep danglymesh nodes (hair and flap sway) instead of demoting them; it needs the `pykotor_k2_dangly` venv patch (run `setup_venv.sh`), and the sway needs a play check. With `converter = "mdlops"` it is ignored with a recipe warning. The build manifest records it next to the converter. |
| `[[textures]]` | `k1`, `k2`, `force_opaque`, `txi` | `force_opaque` is true or false. `txi` is text, up to 1024 characters. List every texture the K1 meshes use: each model is ingested with the recipe textures its mesh references (`k1_port_resolve.py --scan`), and a mesh texture missing from the recipe fails validation. |
| `[audio]` | `k1_ssf`, `k2_ssf` (both required if the table exists) | Imports the character's own K1 barks. `k1_ssf` is the K1 soundset file (for example `p_carth`); `k2_ssf` is the SSF name the pack serves. The builder checks that the K1 SSF's strrefs carry the same voice names in the K2 `dialog.tlk`, copies the K1 voice wavs from `<K1>/streamsounds/` (read only) into the pack as streamed assets, and adds a new `soundset.2da` row that the UTC uses. `character.soundset` stays required as the fallback voice. Dialog voice-over, lips and DLG are not imported. |

## Caveat

K1 HD overlay (`scripts/k1_hd_overlay.py`): a local-only install dir holding flagged Characters Revisited files in
its `override/`. It is never an `--install-k1` / `--from-install` without `--allow-overlay-install` (the tools
refuse it, and with the flag attribute its `override/` files to `characters_revisited_main`, never `vanilla_k1`).
`k1_port_ingest.py --prefer-source <id>` lets a flagged source outrank vanilla for textures; `gate` still blocks CR shas.

Never commit K1 pack assets (textures, models, archives) to this repo.
Only the patch and scripts belong here.

See [38_k1_to_k2_asset_import.md](../38_k1_to_k2_asset_import.md).
