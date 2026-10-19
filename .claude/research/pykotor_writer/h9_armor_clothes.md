# h9: armor / clothes on the imported K1 characters (recon, 10-18)

Question: Carth, Juhani, Mission, Zaalbar, Bastila spawn in undergarments. Do their bodies need anything special, or does any armor work natively?
Labels: VERIFIED = read from game data (pykotor, vanilla chitin 2DA/UTI) or the Steam decomp dump; INFERRED = deduced; UNVERIFIED = needs a live test.
Dump anchors: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/85/` (0085e320.c, 0085c220.c, 0085c330.c, 0085c490.c, 0085c150.c, 0085d430.c). 2DAs were read from chitin (`data/2DA.bif`), NOT the user's Override, so "vanilla" below is vanilla.

## Answer in four lines

1. Undergarments = `ModelA`. The spawned creature has an empty `Equip_ItemList`, so BodyVariation is 0 -> letter A. Nothing is wrong with the bodies. (VERIFIED: template UTC `c_ithorian003` equipment `{}`; builder never sets equipment; 0085e320 maps 0 -> 1 -> 'A'.)
2. Clothes are `ModelB` and our `k1p_<name>bb` IS the K1 clothes model. Equip any base-item-85 / BodyVariation-2 item (K2 ships `g_a_clothes01`, same resref as K1) and the engine picks `modelb` = `k1p_carthbb`. Data-only fix: add `Equip_ItemList` ARMOR slot to the generated UTC. (VERIFIED chain; the live result is UNVERIFIED.)
3. Armor classes C..N already resolve to vanilla K2 bodies (`PMBCM`.. / `PFBCM`..) via the copied template row. No K1 model conversion and no engine patch is needed. BUT the template row is the male commoner (17), so Juhani/Bastila/Mission would get MALE armor bodies (`PMB*`) on a female rig. Fix = recipe `template_row` (or 12 cells), data only.
4. Zaalbar (modeltype F) cannot show armor at all by engine design (single body from the `race` column). Same as K1.

## 1. How K2 composes the body model (VERIFIED, decomp)

Creature appearance descriptor `pDesc` (layout `npc_diversity_src/NpcDivLayout.h:14-30`): +0x00 BodyVariation (subType), +0x01 TextureVar, +0x14 evil/alignment flag (zero/nonzero), +0x18 appearance.2da row.
Source of +0x00/+0x01 = the EQUIPPED body-armor item (doc 38 sec 25: `0x804f10` -> `0x80fde0` reads item id, ModelVariation, and, only if baseitems `ModelType`==1, BodyVariation + TextureVar bytes; `0x80fde0` code confirmed in the dump).
UTC `BodyVariation`/`TextureVar` are ignored for creatures (doc 38 sec 25).

Model type dispatch (`0085c220` -> `0085c150`): appearance column `modeltype`:
- 'B' (code 3) -> `0085c490` -> `0085e320(pDesc, out_model, 0)` -> model name = appearance cell `Model<L>`; then load via creature vtable+0x158. Head is separate (`normalhead` -> heads.2da).
- 'F' or 'L' (code 2), 'S' (code 0xb) -> `0085c330` -> model name = appearance `RACE` cell (string at 0x99a958 is "RACE"). BodyVariation is not read. No armor variation possible.
Vanilla count in K2 appearance.2da: B 404, F 140, L 57, S 70 rows.

Letter and texture (`0085e320`): `n = pDesc[0]`, clamped to 14 (0xe), 0 -> 1; letter = `'@' + n` (1=A .. 14=N); column = "Model"+letter (template string "CModelA" at 0x99519f; "TEXA" at 0x9a4508 for textures). Texture file = `Tex<L>` cell + `"%s%02d"` with TextureVar (0 -> 1); if neither a TGA (res type 3) nor a TPC (0xbbf) of that name exists it retries `"%s01"` (helper identities 0x711ed0 = resource-exists is INFERRED from arguments 3 / 0xbbf). Missing column -> empty string (`DAT_00986925` is a NUL byte) passed on to the loader: result UNVERIFIED (see sec 3).
`0085d430` additionally picks Tex*Evil columns (TEXAEVIL, TEXBEVIL, TEXIEVIL, TEXLEVIL, TEXMEVIL) when +0x14 is nonzero, only for letters A/B/I/L/N and only when the head index < 0x29.

Letter table. Item side is VERIFIED from K2 chitin `baseitems.2da` (column `bodyvar` is a LETTER, not an index) and from chitin UTIs (UTI BodyVariation = letter index, A=1):

| letter / BV | meaning | baseitems row (bodyvar) | example K2 UTI (base, BV, TV) |
|---|---|---|---|
| A / 1 (or none) | underwear, no body armor | none | vanilla Atton `modela=PMBAM` generic underwear, Mira `PFBAM`, PC rows `PMBAM/PFBAM` |
| B / 2 | basic clothing / outfit | 85 Basic_Clothing, 90 Disguise (B) | `g_a_clothes01` (85, 2, 1), `a_light_x02`, `a_robe_01` |
| C..H / 3..8 | armor classes 4..9 | 38..43 (C..H); 103 Miner (C) | `a_light_02` (38,3,1), `a_medium_01` (40,5,1), `a_heavy_01` (42,7,1) |
| I / 9 | Jedi robe | 35 Jedi_Robe, 99 Restrictive (I) | `a_robe_02/03` (35,9,TV1/3) |
| J / 10 | Revan armor | 89 (J) | n/a |
| K / 11 | Armored flight suit | 98 (K) | `a_light_11` (98,11,3); `g_a_armrdfsuit` is (98, **4**, 1): the UTI BV, not the baseitem letter, decides |
| L / 12 | slave outfit | 100 (L) | `g_danceroutfit` (100,12,1); only female PC rows have `modell=PFBL` |
| M / 13 | Zeison Sha armor | 102 (M) | `a_robe_16` (102,13) |
| N / 14 | Knight / Master robe (K2) | 36, 37 (N) | `a_robe_13` (37,14,1), `a_robe_x01` (36,14,9) |

K1 (`swkotor/data/2da.bif`): same letters for B..J (rows 38-43 C..H, 35-37 Jedi robes I, 85 clothing B, 89 Revan J); K1 appearance.2da stops at `modelj` (no K..N columns; headers checked). K2 moved Knight/Master robes from I to N.
Whether the engine reads the baseitems `BodyVar` string at runtime is UNVERIFIED (loader `0x6d5240` reads it; the string is the only xref). The model path above uses only pDesc[0].

Size letter in the model NAME is data, not engine logic: `PMBCM`/`PFBCS` are just resrefs in the cell (S/M/L = body size per party member: Mission small `PFB*S` (row 10), Candy large `PMB*L`, Carth/Juhani/Bastila medium).

## 2. Vanilla K2 party members (VERIFIED, chitin appearance.2da)

| row | char | type | a | b | c..n | head |
|---|---|---|---|---|---|---|
| 4 Bastila | K1 party row kept in K2 | B | P_BastilaBA | P_BastilaBB | PFBCM, PFBDM, .. PFBIM(K1_PFBIM), PFBJM, PFBKM, PFBMM, modeln=PFBIM | 1 |
| 6 Carth | B | P_CarthBA | P_CarthBB | PMBCM..PMBIM, PMBJM, PMBKM, PMBMM, PMBNM (texn=PFBN) | 3 |
| 8 Juhani | B | P_JuhaniBA | P_JuhaniBB | PFBCM..PFBJM, PFBKM, PFBMM, PFBNM | 5 |
| 10 Mission | B | P_MissionBA | P_MissionBB | PFBCS..PFBJS, PFBKM, PFBMS, PFBNM (small) | 6 |
| 9 Zaalbar | F | P_Zaalbar (race too) | - | - | - |
| 452 Atton | B | PMBAM (generic underwear) | P_AttonBB | PMBCM.. | 96 |
| 456 Mira | B | PFBAM | P_MiraBB | PFBCM.., modell=PFBL | 117 |
| 457 Visas | B | PFBAM | P_VisasBB | PFBCM.. (I,N = P_VisasBB) | 98 |
| 451 Bao-Dur | B | P_BaoDurBB | P_BaoDurBB | BAODUR_BC..BF | 100 |
| 455 Kreia | F | all letters P_KreiaBB | | | |
| 454 Handmaiden | F | `&638` redirect | P_HandmaidenBB | `&638` | |

So K2 keeps the K1 party rows (4-10) intact, and every B-type party member ships exactly two bespoke bodies (A and B, sometimes A generic) and borrows C..N from the shared `PMB*/PFB*` set. Non-armor NPCs (HK-47 row 3, Zaalbar, Hanharr, T3, Mandalore 462) are type F: one model, `race`-driven. Pinning patterns that exist in vanilla and prove "alias in 2DA only" works as data: Kreia (every letter = one model), Mandalore no helmet row 639 (c..n = P_MandaloreBD), Handmaiden `&NNN` row redirect (semantics INFERRED, not decoded).
Shared bodies on disk (extracted lowercase): `pmb{a,b,c,d,e,f,g,h,i,j,k,m,n}{s,m,l}`, `pfb*` the same, plus `pfbl`; supermodel chain male/female bodies S_Female02 / S_Female03 (header read), e.g. `pmbcm`->S_Female02, `pfbcm`->S_Female03, `p_carthbb`->S_Female02, `p_bastilabb`->S_Female03. TPC textures `PMBC01/02`, `PMBI01`, `PFBC01`, `PFBI01/03`, `PMBJ01` exist; `pfbn01` is TGA; `pmbn01`, `pfbj01`, `pfbl01` do not exist as plain names (their rows use texj=PMBJ, texn=PFBN, texl=PFBLA, an inherited vanilla quirk).

## 3. Our imported rows today (VERIFIED, staged `char_build.json` + appearance.2da)

`k1_port_char_build.py:121-127` copies `--template-row` (all recipes: 17 = Commoner_Mal_Asian_01_A, N_CommM) then sets only: `label`, `race`=body_b, `modela`/`texa`=body_a, `modelb`/`texb`=body_b, `normalhead`; clears `texaevil`, `texbevil`, `backuphead`. UTC copied from `c_ithorian003` (`:143-147`) sets appearance, texture_var 1, soundset, gender, tag/resref, name; equipment untouched (empty).
Staged row 819 (Carth) and 818 (Juhani): modelc..n = PMBCM, PMBDM, PMBEM, PMBFM, PMBGM, PMBHM, PMBIM, PMBKM, PMBMM, PMBNM; **modelj and modell EMPTY** (row 17 has none), texn=PFBN. Row 818 (Juhani) has MALE bodies in C..N.
Pack route (`k1_port_import.py:1305-1318` `cells_for`) ships every column of the built row as 2DA cells (template_row kept); `.k2pack` asset types do not include UTI (K2PACK_FORMAT.md, res types 2002/3008/3007/2022/2027/2029/2010/2060/3004/4), but do include UTC 2027.

Consequences:
- Carth: C..I,K,M,N work natively (same PMB set as his vanilla row 6). J (Revan armor), L (slave) have empty cells.
- Juhani / Bastila / Mission: armor would show male PMB bodies; Mission would also be medium instead of small.
- Zaalbar: F type; no armor possible.
- What happens when the selected letter's cell is blank: UNVERIFIED. Static path: empty name reaches the loader (sec 1). No RE note in this repo says invisible vs fallback vs crash (searched docs 38, 40, 01, 34 and memory). Avoid by filling every letter A..N.
- texture: an item TextureVar with no matching file falls back to `01` (e.g. `k1p_carthbb02` missing -> `k1p_carthbb01` ok).
- npc-diversity (hooks 0x85E601 / 0x76D200) does not touch our rows (APP_COUNT 818, ours 818+) and never writes tv when subType >= 3 (`NpcDiversityDispatcher.cpp:8`, `NpcDivLog.h:141`). No conflict found.

## 4. K1 comparison (VERIFIED, K1 templates.bif)

K1 vanilla creatures: `p_carth` (appearance 6) wears `g_a_clothes01` (base 85, BV 2, TV 1); `p_bastilla` (4) same; `p_mission` (10) same; `p_juhani` (8) wears `g_a_kghtrobe03` (base 36, BV 9 = letter I, TV 3) -> body = shared `PFBIM` + texture `PFBI03`, NOT p_juhanibb; `p_zaalbar` (9) has no equipment (type F).
So K1 BA = underwear (no armor), BB = the party member's own clothes outfit, C..J = shared generic armor bodies: identical scheme to K2. Juhani's canonical K1 look in-game is the generic female Jedi-robe body, her `bb` is only her alternate outfit.

## 5. Options

| opt | what | feasible without new engine patch | effort | note |
|---|---|---|---|---|
| (e') UTC Equip_ItemList | generator adds ARMOR slot `g_a_clothes01` (K2 vanilla UTI) -> `modelb` | YES | ~30 lines in `k1_port_char_build.py` (arg `--armor RESREF`, recipe key `[equipment] armor`), 1 test; pykotor round-trip proven in scratch (`u.equipment[EquipmentSlot.ARMOR]=InventoryItem(ResRef('g_a_clothes01'))`, `bytes_utc` -> `read_utc` returns it) | fixes "undergarments" for Carth, Bastila, Mission. Juhani: use `a_robe_03` (35, BV9, TV3 -> PFBI03, her K1 look) or clothes01 (-> k1p_juhanibb). Real K2 item (stats apply) |
| (b) reuse vanilla bodies | set female characters' `template_row` to a female row (14 N_CommF, or K2 party rows 8 / 4 / 10 which carry the same C..N sets with Mission's small PFB*S) or add the cells | YES, recipe only | 5 recipes x 1 line + a diff check; party rows also bring `headtrack=1`, `height`, `prefatckdist`, `body_bag` instead of commoner values (diff of 17 vs 6/8/10: creperspace, height, headtrack, prefatckdist, body_bag) | needed for correct armor on females; also fill `modelj/texj` (PFBJM/PMBJ) and ideally `modell/texl` (PFBL/PFBLA) |
| (c) alias in 2DA | point selected letters at one model (Kreia/Mandalore pattern) | YES | trivial | armor stays invisible (always K1 clothes) - an option if nobody wants armor |
| (a) per-character extra bodies | K1 has only ba/bb per character | n/a | not needed | K1 itself uses shared bodies for C..J |
| (d) engine/KPM hook | force BV/TV in `RemapNpcAppearance 0x76D200` (npc-diversity owns this detour; npc-world force-tv queued behind their NB2) | needs peer | high | only useful to dress characters with no equipped item; not needed |
| (f) custom UTI | new armor items | NO (pack lacks UTI; KPM cannot ship Override) | needs pack-format bump + loader change | not needed, vanilla UTIs suffice |
| Zaalbar | needs a new B-type Wookiee body per letter | not natively | model authoring | skip; K1 shows no armor either |

## 6. Recommended path

1. Play-test first (see row below) with a throwaway UTC edit; no code.
2. Add `[equipment] armor = "<resref>"` to `k1_port_recipe.py` + `--armor` to `k1_port_char_build.py` (write `u.equipment[ARMOR]`), thread through `char_build_argv` (`k1_port_import.py:1254`), tests in `test_k1_port_char_build.py` + `test_k1_port_recipe.py`. Defaults: carth/bastilla/mission `g_a_clothes01`; juhani `a_robe_03`; zaalbar none.
3. Change `template_row`: juhani 8, bastilla 4, mission 10 (carth 6 optional, gives `modelj`). Re-run `char_build` and diff cells (`k1_port_pack.py inspect`), check the pack stays within 256 cells and 192 assets (no new cells or assets are added).
4. Optionally add a `[body_slots]` recipe table for overrides (modelj/modell) if a row leaves a letter blank; gate in `k1_port_validate`/char_build: refuse a B-type row with any blank `model[a-n]` the K2 party rows fill.
5. Rebuild packs (build/install only on user GO), play row below.

## 7. Play-test row (for PLAY_TEST_QUEUE.md; the one thing to confirm live)

Unverified key fact: a creature CREATED from a UTC with `Equip_ItemList` (spawn through npc-world / k1-spawn-probe, `0x572a70` arg2 = 0) reaches the client descriptor via `0x804f10` with BodyVariation from the equipped item (doc 38 sec 25 leaves "creation vs update" open).
- Steps: (1) stage Carth's UTC with ARMOR `g_a_clothes01`, install pack, spawn Carth; screenshot. (2) Same UTC with `a_light_02` (base 38, BV 3, TV 1); respawn. (3) Optional: Juhani with `a_robe_03`.
- Pass: (1) Carth wears his K1 clothes (`k1p_carthbb01`), not undergarments; npc-diversity log shows `RMP app=819` with `stype=2`; (2) Carth wears the vanilla PMBCM armor body (PMBC01 texture), head attached, no invisible body, `stype=3`; (3) Juhani in a female Jedi robe (PFBI03) only after her `template_row` is changed to a female row. Fail signals: still undergarments (`stype=0/1`, creation path ignores items -> then option d), invisible body (blank cell), male body on Juhani.

## Open

Does the client creature-CREATION path (not only update) carry the equipped item's BodyVariation for spawned NPCs, and what does a blank `Model<L>` cell do (invisible vs crash)?
