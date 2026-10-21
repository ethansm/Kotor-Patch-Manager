# H8: derive weapon shape from model bbox at load (recon)

Claim: main axis, length, far-end points, exit length, support-grip point from the model bbox at load.

## Runtime path (DLL)
- KNOWN: Gob AABB fields. GetMinimumBoundingBox_2 (Steam 0x00460040, verified) calls vtable+0x1cc (recompute), then copies this+0x14c..0x154 to min and this+0x158..0x160 to max. Source: patch_manager_mods/steam_db_pr3/verified.tsv:350.
- KNOWN, weaker: Gob::GetBoundingBox (Steam 0x00459fb0, "likely", same fields, ambiguous with the row above). verified.tsv:346.
- KNOWN, unmapped: Global::ComputePartBBox(PartTriMesh*) at GOG 0x0085fd80 / Linux 0x083ef57a. No Steam mapping in steam_db_pr3. Listed USER_DEFINED in steam_db_batches/server/INGEST_REPORT.md:1984.
- UNKNOWN: whether the equipped weapon has a Gob at holster-load time; whether the AABB is model or world space; whether emitter/light/dangly nodes are included.
- `query_ghidra.py struct Model | MdlNode | Gob` returned no results. The Gob offsets come from Steam DB notes only.
- No mod reads Gob min/max today (rg for 0x14c/0x158 in inventory and _shared_src hits only Creature_EffectCount).

## Offline table path (already shipped)
- patch_manager_mods/inventory_3d_viewport_src/ItemFitTable.inc: 341 entries sorted by (row,var); struct FitEntry {row, var, flags, c[3], e[3]}; flags bit0 = EMPTY (8 rows).
- Generator present: scripts/gen_inventory_3dview_table.py.
- e is the full model-space AABB extent (corners at +/-0.5*e, Inv3dLogic.h:165); c is the centre.
- R17 survey (patch_manager_mods/inventory_3d_viewport_src/research/R17_fit_survey_2026-09-29.txt; the contract path .claude/research/inventory_3d_viewport_src/ does not exist): 333 meshed rows, 262 distinct w_* stems, 79 of 341 rows flagged outliers (emitter/light/dangly).
- Longest axis is NOT always z: x=62, y=149, z=122 rows. Compute it per model.
- Lightsaber rows have ext 0.203 x 0.203 (w_lghtsbr_001). AABB corners sit about 0.10 m off the blade axis.

## What the bbox can and cannot give
- Main axis: yes (argmax of e).
- Length: yes (e[axis]).
- Far-end points: approximate only (AABB corners, not geometry extremes).
- Exit length: no (needs mesh or hook data).
- Support-grip point: no (needs attach/hand node data, not in the bbox).

## Recommendation
1. Do not compute shape at runtime from the bbox.
2. Reuse ItemFitTable (lookup by baseitem row + variation) or copy it into _shared_src as a generated table.
3. Take main axis and length from the table. Take grip and exit from authored per-class data or an offline mesh pass, not from the bbox.
4. Optional runtime probe: call GetMinimumBoundingBox_2 once on the equipped weapon Gob to confirm AABB space and node inclusion. Only if the Gob exists at load.
