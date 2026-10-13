# K1 vs K2 resource diff: what it means for items (10-13)

Source: `asset_diff_*.tsv` (tool `scripts/k1_k2_asset_diff.py`, run with the k1port venv). Opus reading of the counts, not tool output.

1. **No item mesh exists only in K1.** `only_k1` MDLs (531): m* rooms (491), p_* (15), mgf_* minigames (14), stunt* (21), a few others. Zero `i_*`, `w_*`, `a_*`. K1-only items therefore reuse meshes K2 already has (matches the UTI inventory in SUMMARY.md).
2. **MDL byte hashes are useless for "different".** All 2286 shared MDLs differ in hash (K2 recompiled the format) and all 2286 have identical texture lists. Use node count instead: 139 shared MDLs have a different node count.
3. **Real mesh differences in item families:** 61 `w_*` weapon models have different node counts (blaster carbines/pistols/rifles, double saber, bowcaster, disruptors, heavy repeaters), `n_commm` 59 vs 60 nodes. These share a resref with K2, so the converter's name-collision refusal applies: an import would need a renamed resref.
4. **K1-only characters:** 15 `p_*` party/companion models (Bastila, Canderous, Carth, Jolee, Juhani, T3-M4 bodies and heads). Not items; relevant to doc 38.
5. **K1-only icons:** only 4 (`ii_bithtrombone`, `ip_pituseitm_033`, `ip_pltuseitm_069/070`); 135 K1-only `i_*` TPCs are feat/skill/UI icons, not item icons.
6. Consequence for C2: triaging all `only_k1` plus `both_different` assets would mostly review rooms and recompiled duplicates. A useful triage set is about 61 weapon MDLs (node-count changed) + 15 p_* + 4 icons.

Verification (10-13): two full runs byte-identical (35202 rows, about 30 min per run). Caveats: K2 OVERRIDE holds 7898 TGAs (chitin only 10), so K2 "primary" copies may be installed or imported files rather than vanilla, not checked per file; K1 TPCs live only in TexturePacks (pack order unverified); MDL parse failures K1 44, K2 25. `asset_diff_all.tsv` (6 MB) is left uncommitted; rerun the tool to regenerate it.
