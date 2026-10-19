# h10: party-row templates + no blank body cells (10-18/19)

Labels: VERIFIED = read with pykotor from the live K2 tables / chitin (read only). Follow-up to h9 sec 5(b) and sec 6.3-6.4.

## 1. Which table the builder reads

`k1_port_char_build.py` reads `<K2>/override/appearance.2da` (818 rows today, other mods appended rows 671+; chitin has 671). VERIFIED: rows 4, 6, 8, 9, 10 and 17 are byte-identical cell for cell in override and chitin (same 94 columns), so "vanilla party row" and "live row" agree. Rows 818+ are the juhani-char virtual rows (not templates).

## 2. Row 17 (N_CommM, old template) vs the party rows: every differing column

Only columns that differ from row 17 are listed; "keep" = copy from the party row, "set" = the builder overwrites it, "fill" = blank-letter fill (sec 3).

| column | 17 | 4 Bastila | 6 Carth | 8 Juhani | 10 Mission | 9 Zaalbar (F) | decision |
|---|---|---|---|---|---|---|---|
| label | Commoner_Mal_Asian_01_A | Party_NPC_Bastila | Party_NPC_Carth | Party_NPC_Juhani | Party_NPC_Mission | Party_NPC_Zaalbar | set (K1P_<Id>) |
| race | N_CommM | P_BastilaBB | P_CarthBB | P_JuhaniBB | P_MissionBB | P_Zaalbar | set (body_b; Zaalbar: body_a) |
| modela, texa | N_CommM | P_BastilaBA | P_CarthBA | P_JuhaniBA | P_MissionBA | P_Zaalbar | set |
| modelb, texb | N_CommM | P_BastilaBB | P_CarthBB | P_JuhaniBB | P_MissionBB | blank | set (F: kept blank, like vanilla) |
| normalhead | 9 | 1 | 3 | 5 | 6 | blank | set (new heads row; F without head: blank) |
| modeltype | B | B | B | B | B | F | keep (decides single-model handling) |
| modelc..h, texc..h | PMB?M / PMB? | PFB?M / PFB? | = 17 | PFB?M / PFB? | PFB?S / PFB? (small) | blank | keep (shared armour bodies, right sex/size; the point of the change) |
| modeli, texi | PMBIM / PMBI | K1_PFBIM / K1_PFBI | = 17 | PFBIM / PFBI | PFBIS / PFBI | blank | keep (K1_PFBIM: MDL in chitin, K1_PFBI01.tpc in Override; K2 vanilla Bastila row) |
| modelj, texj | blank / blank | PFBJM / PMBJ | PMBJM / PMBJ | PFBJM / PMBJ | PFBJS / PMBJ | blank | keep (texj PMBJ on females is a vanilla quirk; pmbj01 exists) |
| modelk, texk | PMBKM / PMBK | PFBKM / PFBK | = 17 | PFBKM / PFBK | PFBKM / PFBK | blank | keep |
| modelm, texm | PMBMM / PMBM | PFBMM / PFBM | = 17 | PFBMM / PFBM | PFBMS / PFBM | blank | keep |
| modeln, texn | PMBNM / PFBN | PFBIM / PFBI | = 17 | PFBNM / PFBN | PFBNM / PFBN | blank | keep |
| modell, texl | blank | blank | blank | blank | blank | blank | fill (sec 3); no vanilla party row has a slave body |
| walkdist, driveanimwalk | 1.813 | 1.6 | = 17 | 1.6 | 1.6 | 1.9 | keep (movement speed of the body) |
| rundist, driveanimrun_pc, driveanimrun_xbox | 3.66 / 3.66 / 3.96 | = 17 | = 17 | = 17 | = 17 | 5.4 | keep (Zaalbar run speed) |
| creperspace | 0.4 | 0.36 | 0.36 | 0.36 | = 17 | = 17 | keep (creature personal space) |
| height | 1 | 1.6 | 1.6 | = 17 | = 17 | = 17 | keep (camera/target height) |
| headtrack | 0 | 1 | 1 | 1 | 1 | 1 | keep (head follows target) |
| prefatckdist | 1.4 | 0.5 | 0.5 | 0.5 | 0.5 | 0.5 | keep (preferred melee distance) |
| body_bag | 6 | 3 | 1 | 3 | 1 | = 17 | keep (loot-bag model index) |
| cameraspace, cameraheightoffset | blank | blank | blank | blank | blank | 0.49 / -0.26 | keep (Zaalbar camera, wookiee size) |

Every other column is identical in all six rows (string_ref, racetex, skin, headtex*, portrait, sounds, backuphead, deathvfx*, ... all blank; envmap DEFAULT, racialtype 20, sizecategory 3, ...), so none of them leaks a vanilla identity. VERIFIED: `texaevil`, `texbevil`, `texievil`, `texlevil`, `texnevil`, `backuphead` are blank in rows 4/6/8/9/10/17. They are NOT blank in vanilla Mira (row 456: N_TwiUndMiraD, P_MiraAD, PFBLMiraD), so `CLEARED` now lists all five evil texture columns plus backuphead: a different template row can no longer leak its character (no change for the party rows).
Nothing in rows 4/6/8/9/10 names the K2 character except the set columns above (P_*BA/BB, P_Zaalbar) and `label`.

## 3. Blank letters (task 2)

VERIFIED: modell/texl are blank in rows 4, 6, 8, 10 and 17; modelj/texj blank in row 17 only. Fallbacks used by `fill_blank_letters` (pure, tested) for a modeltype B row:
- female character (recipe gender 1), letter L: `PFBL` / `PFBLA`. VERIFIED present in K2: MDL `pfbl` in chitin, `pfbla01.tpc` in swpc_tex_tp{a,b,c}.erf (also Override `PFBLA01.tpc`); PC female rows 91..102 use exactly modell PFBL / texl PFBLA. (Mira row 456 uses texl PFBLMira, a Mira recolour; PFBLA is the generic one.)
- male Carth, letter L, and any other blank letter: alias to the clothes body (`k1p_<id>bb` / tex `k1p_<id>bb`), the Kreia pattern (vanilla K2 has no male slave body).
- gender comes from the recipe (`--gender`), not from the template row.
- Guard: `check_no_blank_letters` raises BuildError if any model/tex a..n cell is blank or missing after the fill (a set model with a blank texture is not auto-filled and trips the guard).
With the five party rows the only fills are the L pair (2 cells per character). Cells shipped per row: the pack route ships every column of the built row (94 columns; non-empty 64 for a B row, 39 for Zaalbar). Versus the old row-17 build that is +4 non-empty cells per B character (J and L pairs) and +2 versus the party row.

## 4. Zaalbar (modeltype F)

Row 9: race = modela = texa = P_Zaalbar, modelb..n blank, normalhead blank, modeltype F. Before this change the builder wrote race = body_b (importer passes body_b = body_a), modelb/texb = body_b and normalhead = new head row on top of the type-B commoner row 17: a B-type row with one model and a head, wrong for a single-body Wookiee.
Now: when the template's modeltype is F, L or S the builder sets race, modela, texa to body_a, leaves modelb..n as the template has them (blank), sets normalhead only if a head is given, appends a heads row only if a head is given, and refuses a body_b that differs from body_a. A B template without body_b/head is refused. VERIFIED on the real table below: race = k1p_zaalbar, modeltype F kept, no `P_Zaalbar` left in the row (the old race/modela/texa values are overwritten).

## 5. Real-table builds (scratch only, game dir untouched)

`k1_port_char_build.py --game-dir <K2> --template-row <n> ... --out <scratch>` per recipe, rows 818 / head 249 (live counts). model/tex per letter:

| char (tpl) | A | B | C | D | E | F | G | H | I | J | K | L | M | N |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| bastilla (4) | k1p_bastilaba | k1p_bastilabb | PFBCM/PFBC | PFBDM/PFBD | PFBEM/PFBE | PFBFM/PFBF | PFBGM/PFBG | PFBHM/PFBH | K1_PFBIM/K1_PFBI | PFBJM/PMBJ | PFBKM/PFBK | PFBL/PFBLA | PFBMM/PFBM | PFBIM/PFBI |
| carth (6) | k1p_carthba | k1p_carthbb | PMBCM/PMBC | PMBDM/PMBD | PMBEM/PMBE | PMBFM/PMBF | PMBGM/PMBG | PMBHM/PMBH | PMBIM/PMBI | PMBJM/PMBJ | PMBKM/PMBK | k1p_carthbb (alias) | PMBMM/PMBM | PMBNM/PFBN |
| juhani (8) | k1p_juhaniba | k1p_juhanibb | PFBCM/PFBC | PFBDM/PFBD | PFBEM/PFBE | PFBFM/PFBF | PFBGM/PFBG | PFBHM/PFBH | PFBIM/PFBI | PFBJM/PMBJ | PFBKM/PFBK | PFBL/PFBLA | PFBMM/PFBM | PFBNM/PFBN |
| mission (10) | k1p_missionba | k1p_missionbb | PFBCS/PFBC | PFBDS/PFBD | PFBES/PFBE | PFBFS/PFBF | PFBGS/PFBG | PFBHS/PFBH | PFBIS/PFBI | PFBJS/PMBJ | PFBKM/PFBK | PFBL/PFBLA | PFBMS/PFBM | PFBNM/PFBN |

(A and B: model = tex.) Blank cells in a..n: none, all four B rows. Female rows have only PFB* bodies (Mission small S), Carth only PMB*. Physics copied: bastila height 1.6 / headtrack 1 / body_bag 3; carth 1.6 / 1 / 1; juhani 1 / 1 / 3; mission 1 / 1 / 1.
Zaalbar (9, F): race = modela = texa = k1p_zaalbar, modeltype F, modelb blank, normalhead blank, 39 non-empty cells, filled = none.

## 6. Importer follow-up (k1_port_import.py, not edited)

- `build` refuses a recipe without `[models] head` (L1431) and `char_build_argv` (L1260) dereferences `r.head.k2` and always passes `--head`/`--new-head-row`, `--body-b body_a`; `cells_for` (L1307) and the row allocation always include a heads row. The builder now accepts the importer's current shape for Zaalbar (body_b = body_a, with a head: race = k1p_zaalbar, an orphan heads row) but Zaalbar has no head model, so the importer must: skip the head requirement for a recipe whose template row is modeltype F, omit `--head`/`--new-head-row` (and `--body-b`) from `char_build_argv` when `r.head is None`, drop the heads spec from `cells_for` and the head row from `rows`. The builder then writes no heads.2da (char_build.json `heads_row: null`).
- The pack cell `template_row` follows the recipe (6/8/4/10/9 now); vfs_pack still defaults an unset appearance template to 17.
- `k1_port_scan.py` still derives `template_row = 17` for `utc:` NPCs; for K1 party members (appearance 4/6/8/9/10) it should use the same-index K2 party row.
