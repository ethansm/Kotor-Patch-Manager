# W1d: 204TEL GIT on-disk shape (append creature + waypoint)

Live archives: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/Modules/`
- `204tel.mod` 1175774 B (real file, Sep 24 11:59; case symlinks 204TEL.mod/.MOD)
- `204TEL.rim` 60103 B (ARE, GIT, IFO; byte-identical to vanilla_clean)
- `204TEL_s.rim` 214864 B (161 res: NCS 81, PTH 1, UTC 22, UTD 7, UTE 4, UTI 3, UTP 10, UTS 15, UTT 7, UTW 11)
- `204TEL_dlg.erf` 880123 B (17 DLG)

## 1. GIT shape (inner header `GIT V3.2`, outer ERF `MOD V1.0`)
Root labels (struct_id -1): `UseTemplates`, `AreaProperties`(id 100), `Creature List`, `Door List`, `Encounter List`, `List`(empty, keep), `SoundList`, `StoreList`(empty), `TriggerList`, `WaypointList` (NO space), `Placeable List`, `CameraList`.
- Creature List, struct_id 4 (17 in mod, 16 in vanilla): vanilla order XPosition, YPosition, ZPosition (Single), XOrientation, YOrientation (Single), TemplateResRef (ResRef). No Tag/Appearance.
- Example Doctor Ganius (mod index 16, `corteldc`): XPosition -68.7276840209961, YPosition 31.272323608398438, ZPosition 0.0, XOrientation 6.12e-17, YOrientation 1.0. Field order differs from vanilla (TemplateResRef first), so it was hand-appended, not toolset-written.
- WaypointList, struct_id 5 (48, all identical sig), 14 fields in order: Appearance UInt8, LinkedTo String, TemplateResRef ResRef, Tag String, LocalizedName LocString, Description LocString, HasMapNote UInt8, MapNote LocString, MapNoteEnabled UInt8, XPosition/YPosition/ZPosition/XOrientation/YOrientation Single.
- Example waypoint (index 0): Tag `from_203TEL`, LocalizedName stringref 81678, Description -1, MapNote -1, HasMapNote 0, MapNoteEnabled 1, XPos -37.0977, YPos -15.8064, Z 0.0001, XOri -1.0, YOri 2.1e-6.
- `wp_pc_1` does NOT exist in 204TEL (no Tag contains "pc"). Use any existing tag as a template.
- Other struct_ids: Door 8, Encounter 7 (Geometry 1, SpawnPointList 2), Trigger 1 (Geometry 3), Sound 6 (TemplateResRef, GeneratedType UInt32, X/Y/Z), Placeable 9, Camera 14.

## 2. mod vs rim+s_rim
- mod has 186 res; rim+s_rim 164 (all present in mod). Extras in mod: DLG 18 (17 from _dlg.erf + corteldc), NCS +3 (a_ranwander, endcormedtelqst, planttheft), UTC corteldc.
- Bytes differ in mod vs rim/s_rim: `204tel` GIT, `203_merc003` UTC, NCS a_b4d4control/a_chodo/a_czerkapad. 6 of 17 DLG differ from _dlg.erf.
- mod GIT vs rim GIT: +1 creature (corteldc at end); 9 waypoint Tags renamed WP_commoner_01..09 -> WP_22_01..09; Encounter 2 SpawnPointList differs.
- Only the .mod holds corteldc UTC and DLG.

## 3. Dry run (in memory, no file written)
- GIT model (`construct_git`/`dismantle_git`) is LOSSY on a no-edit round trip: field reorder in all lists; creature/waypoint orientation drift <=2.4e-8; Placeable 162 + Door 2 TweakColor 16777215 -> 0; Waypoint MapNoteEnabled 1 -> 0 (35), one MapNote 351 -> -1; 5 Triggers gain empty LinkedTo/Tag/LinkedToModule/TransitionDestin/LinkedToFlags. Do NOT use the GIT model for edits.
- GFF-level append works: `g=read_gff(erf.get("204tel", ResourceType.GIT))`; `lst=g.root.get_list("Creature List")` (returns COPY); `s=lst.add(4)`; set fields; `g.root.set_list("Creature List", lst)`; same for "WaypointList" with `add(5)`.
- Result: Creature 17 -> 18 (appended n_telf03 at -60,25,0); Waypoint 48 -> 49 (tag W1D_DRYRUN_WP). Other 8 lists: 0 order diffs, 0 value diffs, bit-exact.
- MOD rebuild: `erf.set_data("204tel", ResourceType.GIT, bytes_gff(g))`; `bytes_erf(erf, ResourceType.MOD)` (or `write_erf(erf, path, ResourceType.MOD)`). Preserves 186/186 resources, same type counts, header `MOD V1.0`, only GIT bytes change (1175774 -> 1176193 B).

## Open
- Engine precedence (.mod vs .rim+_s.rim) NOT verified; docs search found nothing. The live .mod carries the Doctor Ganius UTC/DLG, so edits must reach the .mod.
- New creature's TemplateResRef needs a matching UTC in the same archive (not checked for n_telf03).
- Waypoint example `wp_pc_1` absent; pick an existing waypoint template for the PoC.
