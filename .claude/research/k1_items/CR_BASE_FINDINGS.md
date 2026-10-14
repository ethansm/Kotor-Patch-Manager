# Characters Revisited (CR) as the K1 base: findings (10-13)

CR is `license_class: unknown`, flagged `pending_author_approval`: local-only, nothing derived enters the repo, a release or a K2 install.

## Layout vs K1 (`cr_k1_layout.tsv`, tool `scripts/cr_k1_layout_check.py`)
- K2pack-palette stems that exist in K1: 342 checked, **332 same layout vs K1 vanilla**, 10 redesign (9 `n_wookie*`, `pfhc03`).
- Control (K1pack rule, 99 rows): 95 ok, 4 redesign at this metric (`pfhc02`, `pfhc02d1..d3`, corr 0.85-0.89): excluded.
- Final selection `cr_overlay_final.tsv`: **418 stems** (93 K1pack-rule minus 4, plus 332 + overlap dedup).

## Overlay (`scripts/k1_hd_overlay.py`, `~/Games/KOTOR2_Mod_Workspace/k1_hd_install`)
- 10 symlinks into the real K1 install + real `override/` copies (745 files, 6.6 GB, sha-verified; `check` OK).
- A stem in two CR shards takes `updates` (6 stems, e.g. `p_candh01`).
- `mesh --game <overlay> --textures` yields 2048 px `p_candh01.tga` (vanilla: 43 KB TPC). K1 `Override/` still 0 files.

## Ledger safety
- `k1_port_{ingest,model,textures}.py` refuse an overlay install unless `--allow-overlay-install`; allowed reads attribute `override/` files to `characters_revisited_main`.
- `k1_port_ingest.py --prefer-source <id>` (textures only); an `updates/` folder wins same-name ties in a source.
- `k1_port_prune.py gate` BLOCKS CR shas, including a renamed copy (verified on overlay files and a renamed stage file).

## p_* import (`scripts/k1_import_companions.py`)
- 15 models, vanilla K1 mdl/mdx -> `k1p_<resref minus p_>` (`k1p_bastilabb02` 15 chars), 16 CR textures staged `k1p_*` (flagged, 16/16 ledger rows). Validator (ingest gate) rejected none.
- Sheets (viewed, upright, CR textures): `tex_looks/run1/c3/sheets_p_cr/`.

## Recon (Haiku)
- `cr_stem_to_model.tsv`: 99 K1pack stems -> models: 28 exact, 61 prefix-only, 10 none (not yet consumed by a render).
- `tga_audit_bakes.tsv`: no gated look reads a TGA; only `preview.load_texture` decodes TGAs (already fixed in 96b6721); `bake`/`overlay`/`decals` have none. Clean.
