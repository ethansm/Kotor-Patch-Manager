```python
import os

for fname in ['5b_k1_port_source_b.txt', '5b_k1_port_source_b.manifest.tsv', '5d_k1_port_tests_b.txt', '5d_k1_port_tests_b.manifest.tsv']:
    if os.path.exists(fname):
        with open(fname, 'r', encoding='utf-8') as f:
            lines = f.read().splitlines()
        print(f"=== {fname} (lines: {len(lines)}) ===")
        if fname.endswith('.manifest.tsv'):
            for l in lines:
                print("  ", l)
        else:
            print("  Last line:", repr(lines[-1]))
    else:
        print(f"MISSING: {fname}")


```

```text
=== 5b_k1_port_source_b.txt (lines: 8522) ===
  Last line: '</review_bundle>'
=== 5b_k1_port_source_b.manifest.tsv (lines: 19) ===
   path	role	status	reason	est_tokens	bytes	dirty
   patch_manager_mods/k1_port/README.md	docs	included		1855	7420	0
   patch_manager_mods/k1_port/PERMISSION_REQUESTS.md	docs	included		1893	7571	0
   patch_manager_mods/k1_port/k1_port_char_build.py	source	included		3603	12608	0
   patch_manager_mods/k1_port/k1_port_cr_extract.py	source	included		4202	14706	0
   patch_manager_mods/k1_port/k1_port_cr_sheets.py	source	included		3796	13283	0
   patch_manager_mods/k1_port/k1_port_ingest.py	source	included		9127	31942	0
   patch_manager_mods/k1_port/k1_port_probe.py	source	included		8988	31456	0
   patch_manager_mods/k1_port/k1_port_probe_build.py	source	included		2997	10487	0
   patch_manager_mods/k1_port/k1_port_prune.py	source	included		5088	17808	0
   patch_manager_mods/k1_port/k1_port_sources.py	source	included		4500	15749	0
   patch_manager_mods/k1_port/k1_port_spawn_build.py	source	included		2912	10191	1
   patch_manager_mods/k1_port/k1_port_validate.py	source	included		3964	13871	0
   patch_manager_mods/k1_port/setup_venv.sh	source	included		354	1238	0
   patch_manager_mods/k1_port/looks/commoner_a.json	config	included		9295	32531	0
   patch_manager_mods/k1_port/looks/commoner_b.json	config	included		8861	31012	0
   patch_manager_mods/k1_port/looks/mandalorian.json	config	included		1332	4661	0
   patch_manager_mods/k1_port/looks/sith_commando.json	config	included		1015	3552	0
   patch_manager_mods/k1_port/sources.json	config	included		2213	7745	0
=== 5d_k1_port_tests_b.txt (lines: 4151) ===
  Last line: '</review_bundle>'
=== 5d_k1_port_tests_b.manifest.tsv (lines: 11) ===
   path	role	status	reason	est_tokens	bytes	dirty
   patch_manager_mods/k1_port/test_k1_port_char_build.py	tests	included		2568	8986	0
   patch_manager_mods/k1_port/test_k1_port_cr_extract.py	tests	included		2789	9759	0
   patch_manager_mods/k1_port/test_k1_port_cr_sheets.py	tests	included		3264	11422	0
   patch_manager_mods/k1_port/test_k1_port_ingest.py	tests	included		8106	28371	0
   patch_manager_mods/k1_port/test_k1_port_probe.py	tests	included		7025	24587	0
   patch_manager_mods/k1_port/test_k1_port_probe_build.py	tests	included		3970	13895	0
   patch_manager_mods/k1_port/test_k1_port_prune.py	tests	included		7070	24743	0
   patch_manager_mods/k1_port/test_k1_port_sources.py	tests	included		5900	20647	0
   patch_manager_mods/k1_port/test_k1_port_spawn_build.py	tests	included		5731	20056	1
   patch_manager_mods/k1_port/test_k1_port_validate.py	tests	included		4105	14366	0


```

### Step 1: File Integrity Check

* **`5b_k1_port_source_b.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).




* **`5d_k1_port_tests_b.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).





---

### Step 2: Code Reviews

#### `5b_k1_port_source_b.txt`

**Omitted files from `5b_k1_port_source_b.manifest.tsv`:** None (all 18 files listed in `5b_k1_port_source_b.manifest.tsv` are included in `5b_k1_port_source_b.txt`).

##### Findings

1. **`patch_manager_mods/k1_port/k1_port_prune.py` — `cmd_gate` (and `patch_manager_mods/k1_port/k1_port_probe.py` — `compute_plan`)**
* **Anchor:** `if f.suffix.lower() in ASSET_SUFFIXES and not known and f.resolve() not in registered:`

* **What is wrong and concrete input/state that triggers it:**
1. `ASSET_SUFFIXES` only includes `(".tpc", ".tga", ".mdl", ".mdx", ".wav", ".mp3", ".lip", ".ssf")`. Game files with `.2da`, `.utc`, or `.ncs` extensions (which are the exact files built by `k1_port_char_build.py`, `k1_port_probe_build.py`, and `k1_port_spawn_build.py`, or extracted from mod archives like `Characters Revisited Updates` which the docs state contain `.utc` entries) are excluded from `ASSET_SUFFIXES`. Even under `gate --strict`, an unregistered `.utc`, `.2da`, or `.ncs` file passes the gate with zero warnings and exit code 0.


2. In `cmd_gate`, `if f. resolve() not in registered:` allows a file sitting at a registered ledger path whose ledger entry has `sha256: None` (and is not flagged) to bypass the unregistered-asset warning even if its bytes were replaced after registration.


3. Worse, if a ledger entry is added via `Ledger.add_derived(...)` from a `vanilla_k2` or `vanilla_k1` parent (or an approved source), `Ledger.add_derived` sets `local_only=True` (`"any ai or edited copy is local_only until a person signs it off"`, and `sources.json` notes for `vanilla_k2`: `"kvoice forces personal_only/local_only on any ledger row whose parent is vanilla_*"`). However, `is_flagged(e)` in `k1_port_prune.py` only checks `bool(e.get("removal_group")) and not e.get("author_approved")` and completely ignores `e.get("local_only")` and `e.get("personal_only")`! As a result, `k1_port_prune.py gate` (and `k1_port_probe.py`) allows `local_only` / `personal_only` derived assets to pass the release gate unblocked.




* **Suggested fix:** Check `e.get("local_only") or e.get("personal_only")` inside `is_flagged(e)` (or block them explicitly in `cmd_gate` unless signed off), require a matching `sha256` (not just path membership in `registered`) to suppress the unregistered warning, and include `.utc` / `.ncs` in `ASSET_SUFFIXES` (or explicitly register built probe/char files).


* **Confidence:** certain


2. **`patch_manager_mods/k1_port/k1_port_ingest.py` — `existing_row` / `ingest**`
* **Anchor:** `rows.append(row(path, kind, "already_ingested", None, None, [], approved=True))`

* **What is wrong and concrete input/state that triggers it:** When `k1_port_ingest.py` is re-run and finds an existing output file in `<stage>/override/`, `<stage>/_hd/override/`, or `<stage>/models/` (`d["status"] = "already_ingested"`), `existing_row` looks up `by_file.get(str(path))` in the ledger. If the file is not found in the ledger (for example, if `--ledger` points to a fresh ledger file, or if the path string in the ledger used a different symlink/relative representation, or if an earlier run wrote the file before crashing on ledger save), `existing_row` passes `approved=True` and `removal_group=None`! Consequently, `out["flagged_files"]` omits the file, `out["credits_needed"]` omits its credits, the file is never registered in the ledger, and `k1_port_ingest.py` exits `0` (`status: "ok"`) even if the file on disk is in `<stage>/_hd/override/` or came from a flagged source.


* **Suggested fix:** Refuse (raise `Refused`) when an `already_ingested` file in the stage is missing from the ledger, or fall back to checking its `sha256` via `ledger.find_by_sha(...)` and re-staging/failing closed if its provenance is unknown.


* **Confidence:** certain


3. **`patch_manager_mods/k1_port/k1_port_probe_build.py` — `_build_appearance` vs. `patch_manager_mods/k1_port/k1_port_char_build.py` — `build**`
* **Anchor:** `t.set_label(NEW_ROW, NEW_LABEL)`

* **What is wrong and concrete input/state that triggers it:** In `pykotor`'s `TwoDA` class, `add_row(row_label, cells)` sets the row header (the 2DA row index column) to `row_label`, while `"label"` is a regular data column in `appearance.2da`.


* In `k1_port_char_build.py`, the code explicitly notes: `# Vanilla shape: the row id (first column) stays the numeric index; the name goes in the label data column` and calls `app.set_cell(new_row, "label", label)` while keeping the row header `"818"`.


* In `k1_port_probe_build.py`, `_build_appearance` copies `{c: t.get_cell(template_row, c) for c in t.get_headers() if c != "label"}` (leaving the `"label"` data column empty `""`) and calls `t.set_label(NEW_ROW, NEW_LABEL)` which overwrites the numeric row index `"812"` with `"K1P_CommM_Probe"` while leaving the `"label"` column blank!




* **Suggested fix:** In `k1_port_probe_build.py::_build_appearance`, replace `t.set_label(NEW_ROW, NEW_LABEL)` with `t.set_cell(NEW_ROW, "label", NEW_LABEL)` so row 812 keeps its numeric row index `"812"` and puts `"K1P_CommM_Probe"` in the `"label"` column, matching vanilla `appearance.2da` and `k1_port_char_build.py`.


* **Confidence:** certain


4. **`patch_manager_mods/k1_port/k1_port_char_build.py` — `build**`
* **Anchor:** `u.tag = u.resref = utc`

* **What is wrong and concrete input/state that triggers it:** In `pykotor.resource.generics.utc.UTC`, `u.resref` is a `pykotor.common.misc.ResRef` instance, not a plain `str` (as seen in `k1_port_spawn_build.py`: `from pykotor.common.misc import ResRef; u.resref = ResRef(resref)`). In `k1_port_char_build.py`, `u.tag = u.resref = utc` assigns a plain Python `str` to `u.resref`. If `pykotor`'s `write_utc` / `bytes_utc` or callers access `u.resref.get()` or require `ResRef` type semantics, or if `utc` is serialized into the GFF `TemplateResRef` field expecting a `ResRef`, assigning a raw `str` breaks type consistency with `k1_port_spawn_build.py`.


* **Suggested fix:** Import `ResRef` from `pykotor.common.misc` in `_pykotor()` and set `u.tag = utc; u.resref = api.ResRef(utc)`.


* **Confidence:** likely


5. **`patch_manager_mods/k1_port/k1_port_probe.py` — `execute_rollback` and `cmd_rollback**`
* **Anchor:** `data["rolled_back"] = now_iso()`

* **What is wrong and concrete input/state that triggers it:**
1. If two probe packages were installed sequentially (e.g., package 1 replaced `appearance.2da`, was rolled back, or a second manifest was installed via `--manifest` for non-overlapping files) or if `rollback` rolls back the latest manifest, `execute_rollback` marks `manifest.json` with `"rolled_back"`, but never updates or removes `k1p_probe_latest.json` when an earlier active manifest still exists.


2. In `execute_rollback`, the loop deletes/restores files one by one without a `try/except` rollback or progress record. If restoring the second file raises `OSError` or `ProbeError("restore verify failed")`, the first file has already been unlinked, `"rolled_back"` is not written, and if the user was running `rollback --yes --force` (because the first file had drifted), re-running `rollback --yes` without `--force` will still be blocked if any remaining file drifted.




* **Suggested fix:** Restore replaced files (`kind == "restore"`) and verify their hashes *before* unlinking created files (`kind == "delete"`), so a failed restore does not leave created files partially deleted.


* **Confidence:** certain


6. **`patch_manager_mods/k1_port/k1_port_validate.py` — `_Checker.trimesh` and `_Checker.skin**`
* **Anchor:** `w, b = self.u32(S + 12), self.u32(S + 16)`

* **What is wrong and concrete input/state that triggers it:**
1. In `_Checker.skin`, the MDX row offsets for skin weights (`S + 12`) and bone indices (`S + 16`) are read as unsigned 32-bit integers (`self.u32`) instead of signed 32-bit integers (`self.i32`, unlike `BLOCKS` in `trimesh` which uses `self.i32`), and `if w > b:` only checks whether `w` is strictly greater than `b`, not whether `w == b` or whether `w` / `b` is `0xFFFFFFFF` (`-1`). Worse, if `vc == 0` (a skin node with 0 vertices), the `if vc > 0:` block in `trimesh` is skipped completely, so `BLOCK_RANGE` and `BLOCK_OVERLAP` never check `w` and `b`.


2. In `_Checker.trimesh`, `end = mdx_base + vc * stride` only updates `self.max_end` for the single mesh with the largest `end`, and `check_mdx_length` only checks `self.max_end[0] > len(self.mdx)`. Because `mdx_base` is read as `u32` and checked only at the very end via `max_end`, if multiple meshes have out-of-bounds `mdx_base` values, only the one with the largest `end` is reported instead of each offending mesh.




* **Suggested fix:** Read `w` and `b` with `self.i32` (or flag `w >= b` in `SKIN_ROW_ORDER` since weights and bone-indices cannot share the same offset `w == b`), and check `mdx_base + vc * stride > len(self.mdx)` per mesh when `self.mdx is not None`.


* **Confidence:** certain


7. **`patch_manager_mods/k1_port/k1_port_cr_extract.py` — `flatten_shard` and `run_shard**`
* **Anchor:** `os.replace(f, shard_dir / f.name.lower())`

* **What is wrong and concrete input/state that triggers it:**
1. In `flatten_shard(shard_dir)`, `nested = [p for p in shard_dir.rglob("*") if p.is_file() and p.parent != shard_dir]` iterates in filesystem (`rglob`) order and moves every nested file to `shard_dir / f.name.lower()`. If a RAR archive (or a shard combining multiple subdirectories in `updates` or `main`) contains two files with the same case-insensitive basename in different subdirectories, `os.replace` silently overwrites one with the other in non-deterministic `rglob` order. Then, in `run_shard`, `is_present(shard_dir, p, s)` checks `t.stat().st_size == s`; if the two same-basename entries had different sizes in `cr_files.tsv`, one of them fails `t.stat().st_size == s`, so `present < len(items)` and `cmd_run` always fails with exit code 1 on every run!


2. `_entries` deduplicates by `(pack, path_inside)` instead of `(shard, base.lower())`, so same-basename files from different subdirectories (or from `main` + `creatures` in the `"creature"` shard!) both remain in `items` for the same shard. Specifically, `shard_of("creatures", ...)` returns `"creature"`, and `shard_of("main", "c_drdwar", "tga")` ALSO returns `"creature"`! If `creatures` (the Creatures Addon RAR) overrides a creature file that also exists in `main` with a different byte size, both entries are placed into the `"creature"` shard's `items` list; `run_shard` extracts `main` (`unrar x -o-` refuses to overwrite existing files!) or `flatten_shard` overwrites one with the other, leaving a single file on disk whose size can only match one of the two entries in `items`, causing `run_shard` to report `extracted < expected` and exit 1!




* **Suggested fix:** Deduplicate entries per `(shard, basename_lower)` in `_entries` (with a defined precedence between `creatures` addon and `main`, and between subdirectories), and pass `-o+` (or extract per-pack in precedence order) so the intended override wins deterministically.


* **Confidence:** certain



##### Files or Areas Not Assessed for `5b_k1_port_source_b.txt`

* **`k1_port_textures.py` and `k1_port_model.py`:** Imported/invoked by `k1_port_ingest.py` (`import k1_port_textures as kt` and `MODEL_TOOL = HERE / "k1_port_model.py"`), and `pykotor_k2_mdl.patch` referenced by `setup_venv.sh`, are not part of bundle `5b_k1_port_source_b.txt` (they belong to the part-A bundle).



---

#### `5d_k1_port_tests_b.txt`

**Omitted files from `5d_k1_port_tests_b.manifest.tsv`:** None (all 10 files listed in `5d_k1_port_tests_b.manifest.tsv` are included in `5d_k1_port_tests_b.txt`).

##### Findings

1. **`patch_manager_mods/k1_port/test_k1_port_char_build.py`, `test_k1_port_probe_build.py`, `test_k1_port_spawn_build.py`, and `test_k1_port_ingest.py` — `__main__` / `main` (Silent `SKIP` with exit 0 when venv is missing)**
* **Anchor:** `print(f"SKIP test_k1_port_char_build: {VENV_PY} missing (pykotor venv not set up)")`

* **What is wrong and concrete input/state that triggers it:** `test_k1_port_char_build.py`, `test_k1_port_probe_build.py`, `test_k1_port_spawn_build.py`, and `test_k1_port_ingest.py` all check whether `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python` exists before running any tests, and if it is absent (e.g., on a clean runner or when `K1PORT_VENV` was used to create the venv at a non-default path via `setup_venv.sh`!), all four test files print `SKIP` and exit `0` without running a single test. Notably:


1. `setup_venv.sh` supports `K1PORT_VENV=/path/to/venv`, whereas `k1_port_probe_build.py`, `test_k1_port_char_build.py`, `test_k1_port_probe_build.py`, and `test_k1_port_spawn_build.py` hardcode `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python` and ignore `K1PORT_VENV` and `K1PORT_PY`.


2. In `test_k1_port_ingest.py`, the in-process validation gate tests (`validation_finding_rejects_model_and_registers_nothing`, `skip_validate_registers_model_with_warning`, `clean_validation_registers_model`, `supermodel_map_reaches_the_model_tool`) are completely stubbed (`stubbed(root)`) and do not need `pykotor` or `VENV_PY` at all, yet `main()` skips the entire file if `Path(PY).exists()` is false.




* **Suggested fix:** Honor `K1PORT_PY` / `K1PORT_VENV` in `VENV_PY`, and in `test_k1_port_ingest.py`, run the stubbed in-process tests under system `python3` even when the `pykotor` venv is absent (only raising `Skip` inside tests that call `make_tpcs` or `run_ingest`).


* **Confidence:** certain


2. **`patch_manager_mods/k1_port/test_k1_port_probe_build.py` — `test_row_812_appended_with_expected_cells**`
* **Anchor:** `check(t.get_label(812) == "K1P_CommM_Probe", f"row 812 row label {t.get_label(812)}")`

* **What is wrong and concrete input/state that triggers it:** `test_row_812_appended_with_expected_cells` asserts that `t.get_label(812) == "K1P_CommM_Probe"` (the 2DA row index header) and never checks `t.get_cell(812, "label")`! In `base_2da_bytes`, rows 0..811 have numeric row labels `str(i)` (`"0"`..`"811"`) and store `"N_CommM"` / `"ROW_nnn"` in the `"label"` data column (`t.get_cell(i, "label")`). For row 812, however, `_build_appearance` left the `"label"` data column empty (`""`) and overwrote the row index header `t.get_label(812)` with `"K1P_CommM_Probe"`. The test cemented the bug instead of catching that row 812's `"label"` column was empty and its row index was non-numeric (unlike `test_k1_port_char_build.py`, which explicitly checks `check(a.get_label(APP_ROWS) == str(APP_ROWS), "row id must stay numeric")` and `check(row["label"] == "K1P_Juhani")`).


* **Suggested fix:** Update `_build_appearance` to set the `"label"` cell and keep the row index numeric `"812"`, and update `test_row_812_appended_with_expected_cells` to assert `t.get_label(812) == "812"` and `t.get_cell(812, "label") == "K1P_CommM_Probe"`.


* **Confidence:** certain


3. **`patch_manager_mods/k1_port/test_k1_port_cr_extract.py` — Missing test coverage for `run_shard` (non-dry-run extraction, resumability, and manifest writing)**
* **Anchor:** `["run", "--shard", "all", "--dry-run", "--file-list", str(fx), "--dest", str(dest)],`

* **What is wrong and concrete input/state that triggers it:** `test_k1_port_cr_extract.py` never calls `run_shard` or `cmd_run` with `dry_run=False`. Although `flatten_shard` and `write_manifest` are unit-tested in isolation, `run_shard` itself—which checks `is_present` for resumability, invokes `check_disk`, writes `.names.txt`, runs `unrar`, flattens the shard, computes `sha256_of`, and deduplicates `seen` filenames before calling `write_manifest`—is 0% covered by any test. If `run_shard` passed wrong arguments to `write_manifest` or `check_disk`, or failed to skip already-present files (`is_present`), `test_k1_port_cr_extract.py` could not fail.


* **Suggested fix:** Add a test in `test_k1_port_cr_extract.py` that puts a fake `unrar` script on `PATH` (or tests `run_shard` with pre-populated files matching `is_present` and with a fake `unrar` script that creates `Override/<file>`), verifying both initial extraction + `manifest.tsv` generation and resumable skipping when files are already present.


* **Confidence:** certain


4. **`patch_manager_mods/k1_port/test_k1_port_validate.py` — Missing test for `SKIN_ARRAY_ORDER` and `w == b` in `_Checker.skin**`
* **Anchor:** `expect(got, "SKIN_ROW_ORDER")`

* **What is wrong and concrete input/state that triggers it:** `k1_port_validate.py` defines the finding code `SKIN_ARRAY_ORDER` (`if cur[0] < prev[0] + prev[1]: self.add("SKIN_ARRAY_ORDER", ...)`) when skin arrays (`bonemap`, `qbones`, `tbones`, `unk0`) overlap or appear out of order. However, no test in `test_k1_port_validate.py` ever triggers or asserts `SKIN_ARRAY_ORDER`. If the `SKIN_ARRAY_ORDER` check in `k1_port_validate.py` were deleted or broken, `test_k1_port_validate.py` would still pass 100%.


* **Suggested fix:** Add a test in `test_k1_port_validate.py` that swaps the `bonemap` (`S + 20`) and `qbones` (`S + 28`) pointers on a `build(skin=True)` model and asserts `expect(kv.validate(bytes(b), mdx), "SKIN_ARRAY_ORDER")`.


* **Confidence:** certain


5. **`patch_manager_mods/k1_port/test_k1_port_probe.py` — Missing test for active-manifest target collision and save-slot symlink/ambiguity blocking**
* **Anchor:** `def test_install_blocked_by_foreign_file(root):`

* **What is wrong and concrete input/state that triggers it:** `k1_port_probe.py` has three critical safety checks in `compute_plan` / `plan_saves` that are never exercised in `test_k1_port_probe.py`:
1. Refusing an install when a target file is already listed in an active (non-rolled-back) install manifest (`if rel in active: plan.check(..., False, "listed by an active install ...")`) — especially important for `appearance.2da` or `heads.2da`, which would otherwise pass the replaceable-file branch if `--base-sha256` matched the currently installed file!


2. Refusing an install when `cloudsaves/*/<slot>` matches multiple user directories (`len(hits) > 1`).


3. Refusing an install when a save tree contains a symlink (`tree_entries` raising `ProbeError("save tree has a symlink: ...")`).
If any of those three checks were removed from `k1_port_probe.py`, `test_k1_port_probe.py` could not fail.




* **Suggested fix:** Add tests in `test_k1_port_probe.py` verifying that: (a) running a second `install --yes` while the first manifest is still active is blocked even if `--base-sha256` is set to the installed `appearance.2da` hash, (b) two Steam user dirs under `cloudsaves/` with `000000 - QUICKSAVE` block the install, and (c) a symlink inside `000000 - QUICKSAVE` blocks the install.


* **Confidence:** certain


6. **`patch_manager_mods/k1_port/test_k1_port_ingest.py` — Missing test for `vanilla_lookup` and overlay attribution**
* **Anchor:** `def overlay_install_refused_without_flag(root):`

* **What is wrong and concrete input/state that triggers it:** `test_k1_port_ingest.py` tests that an overlay install directory is refused when `--allow-overlay-install` is omitted (`overlay_install_refused_without_flag`), but never tests `vanilla_lookup` with a valid install or with `--allow-overlay-install`. Specifically, in `k1_port_ingest.py::vanilla_lookup`, when `overlay is not None and kt.is_override_label(label)` is true, textures in the K1 install's `override/` are attributed to `kt.overlay_source_id(overlay)` (flagged) instead of `vanilla_k1` (unflagged). Because every test in `test_k1_port_ingest.py` passes `--install-k1 <nonexistent_dir>`, `vanilla_lookup` always returns early at `if not install.is_dir(): return out` (except in `overlay_install_refused_without_flag` which fails before `vanilla_lookup`). If `vanilla_lookup` attributed overlay `override/` textures to `vanilla_k1` instead of the flagged overlay source, `test_k1_port_ingest.py` could not fail!


* **Suggested fix:** Add an in-process test in `test_k1_port_ingest.py` (stubbing `kt.import_install_api` / `kt.open_install`) that exercises `vanilla_lookup` both for a normal `vanilla_k1` hit and for an `--allow-overlay-install` hit under `override/`, asserting that the overlay hit is attributed to the flagged `source_id` from `K1_HD_OVERLAY.json` and never to `vanilla_k1`.


* **Confidence:** certain



##### Files or Areas Not Assessed for `5d_k1_port_tests_b.txt`

* None; all 10 test files in `5d_k1_port_tests_b.txt` and their corresponding source files in `5b_k1_port_source_b.txt` were present and assessed.