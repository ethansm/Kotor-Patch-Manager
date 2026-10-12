# P2 species to appearance row (recon, read-only, 2026-10-11)

Sources: vanilla appearance/heads/creaturespeed/racialtypes exported from game Data/2DA.bif via pykotor Installation; vanilla users from `.claude/npc_div/registry.jsonl` (`app` = appearance row, `race` = UTC race byte); collisions from `patch_manager_mods/npc_diversity_src/DiversityTables.h`, `DiversityRows.h`, `DiversityLogic.h`. Prior recon 01 cited, not repeated.

Speed key: moverate -> creaturespeed.2da idx: NORM=4 (1.70/5.40), SLOW=3 (1.25/2.50), FAST=5 (2.0/6.0), WEE_FOLK=11 (0.7/1.4).
Race key: 6 = Human (racialtypes row 6), 5 = Droid (row 5), 2 = Selkath (inferred from registry, row blank in vanilla file).

## Table (species | row | body / type | head | textures | speed | race | gender | conf | risk)

| Species | Row | Body / type | Head (heads row) | Textures | Speed | Race | Gender | Conf | Risk |
|---|---|---|---|---|---|---|---|---|---|
| Human M | 17 | N_CommM / B | 9 | n_commm01-11 Y | NORM | 6 | M | HIGH | COLLIDES: IsGenericMaleCommoner input. Not alien. |
| Human F | 14 | N_CommF / B | 8 | n_commf01-17 Y | NORM | 6 | F | HIGH | COLLIDES: IsGenericFemaleCommoner input. Not alien. |
| Twi'lek M | 48 | N_TwilekM / B | 62 twilek_m Y | n_twilekma01 (prior recon) | NORM | 6 | M | MED | 0 vanilla users (269 has 1). No collision. 416/449/450/606/618 COLLIDE (IsGenericTwilekMale). 805/806 are remap outputs. |
| Twi'lek F | 47 | N_TwiDance_Boot / B | 61 P_TwiH1 Y | N_TwiDanceA unverified | NORM | 6 | F | MED | Body and head now resolve (prior "MISSING" wrong for P_TwiH*). 446/447/620/280 would work but COLLIDE (IsGenericTwilekFemale). |
| Wookiee M | 271 | N_WookieM / F | none | n_wookiem01 Y | NORM | 6 | M | HIGH | 7 vanilla users (Wookie Slave, Wookiee Guard). Armed guard roles mixed. |
| Wookiee F | 50 | N_WookieF / F | none | n_wookief01-05 Y | NORM | 6 | F | MED | 1 vanilla user (named slave). 275-278 unused in registry. |
| Rodian | 39 | N_Rodian / F | none | n_rodian01-05 Y | NORM | 6 | unset | HIGH | 16 users, 12 generic (Citizen, Gang Member). footsteptype 1. |
| Bith | 12 | N_Bith / F | none | n_bith01 Y | NORM | 6 | unset | HIGH | 12 users, 4 generic (Bith Musician, ambient). footsteptype 1. |
| Duros | 24 | N_Duros / F | none | n_duros01-05 Y | NORM | 6 | unset | HIGH | 11 users (Dendis Dobo, Duros Dock). |
| Sullustan | 601 | N_Sullustan / F | none | n_sullustan01 Y (name grep only) | NORM | 6 | unset | MED | 8 users, mostly named. |
| Gand | 569 | N_Gand / F | none | n_gand, n_gandb, n_gand_d Y | NORM | 6 | unset | HIGH | 64 users, 58 generic (Gand Findsman). Most-used alien row. |
| Gran (extra) | 565 | N_Gran / F | none | n_gran, n_granb Y | NORM | 6 | unset | HIGH | 26 users, 17 generic (mostly gang). |
| Quarren (extra) | 537 | N_Quarren / F | none | n_quarrenh01 Y | NORM | 6 | unset | HIGH | 9 users, 4 generic. Mon Cal stand-in. |
| Selkath | 260 | N_Selkath / L | none | n_selkath01 Y | NORM | 2 | unset | MED | 13 users (Selkath Port Official, ambient). Row 40 has 5. L-type semantics unverified. Insane 335/336 hostile. |
| Devaronian | 570 | N_CommM body / B | 129 N_DevH Y | body = human N_CommM; n_devh01 Y | NORM | 6 | M | HIGH | Human body with devaronian head only. 21 users, 17 generic (Czerka Employee). No collision (FamilyOf = None). |
| Ithorian | 72 | blank / S | none | c_ithorian01-03 Y | SLOW | 6 | unset | MED | Prior "NOT VIABLE" overturned: 13 users (Fazza, Greeter). Body likely from race C_Ithorian (c_ithorian.mdl Y). |
| Jawa | 73 | blank / S | none | c_jawa01 Y | WEE_FOLK | 6 | unset | MED | 17 users, 6 generic (Jawa). Prior "NOT VIABLE" overturned. Race C_Jawa (c_jawa.mdl Y). Walk 0.7/1.4. |
| Weequay | 599 | N_Weeq / F | none | n_weeqb01, n_weeqh01 Y | NORM | 6 | unset | HIGH | 39 users, 36 generic, mostly gang/merc. Ambient risk. 789 PARTIAL. |
| Trandoshan | 464 | blank (race Sch_Trando; sch_trando.mdl Y) / F | none | p_trndoshan 01-03 Y | NORM | 6 | unset | MED | 37 users, 32 generic, mostly gang. Prior "NOT VIABLE" overturned. |
| Gamorrean civilian | 337 | blank (race C_GammoreanP; c_gammoreanp Y) / S | none | c_gammoreanp Y (racetex blank) | NORM | 6 | unset | MED | Only 2 users (named bodyguard). Row 68 (26 guard/thug users) is armored-type, not ambient. |
| Rakata (extra) | 255 | blank (race N_Rakata; n_rakata.mdl Y) / F | none | rakata01-03 Y (no n_ prefix) | NORM | 6 | unset | HIGH | 54 users, 50 generic. Not in prior recon table. |
| Walrusman (Aqualish stand-in) | 49 | N_Walrusman / F | none | walrusman01-04 Y | NORM | 6 | unset | MED | 6 users, 4 generic gang. Aqualish: no row, n_aqualish.mdl absent. |
| Protocol droid | 63 | blank (race C_DrdProt; c_drdprot.mdl Y) / L | none | n/a | SLOW | 5 Droid | n/a | MED | 13 users (Protocol Droid, Battered Protocol Droid). Walk 0.6 in row. Not armored. |
| Zabrak / Cathar / Mon Cal | none | n_zabrak, n_cathar, n_mon_cal .mdl absent | - | - | - | - | - | NOT USABLE | No row and no MDL. |
| Sith human (non-alien) | 291-302 | N_SithComF/M / B | 12/13 | n/a | NORM | 6 | F/M | n/a | Not alien. Not in remap lists. |

Key: Y = present (pykotor Installation or name grep of chitin.key and TexturePacks). "Not in registry" = 0 vanilla users. Gender "unset" = F/L-type row has no sex field; UTC decides (not verified).

## Corrections to prior recon 01
1. Blank modela is not "not viable". Rows 72, 73, 464, 255, 68, 337, 63 have blank modela and 6-54 vanilla users each. Race names match an existing MDL (c_ithorian, c_jawa, sch_trando, n_rakata, c_gammorean, c_gammoreanp, c_drdprot). Engine fallback to race is a hypothesis, not confirmed in code.
2. Twi'lek F heads P_TwiH1, P_TwiH2, P_TwiH3, P_TwiH5, P_TwiH6, P_TwiHYuthura, P_TwiHNGreen, P_TwiHTRed resolve in the game install (live Override/ has the loose .mdl). Prior "MISSING" holds only for the mod's own override/ folder.

## Remap collision check (Q3)
- Remap inputs: IsGenericMaleCommoner (24 rows), IsGenericFemaleCommoner (26), IsGenericTwilekMale (416, 417, 445, 449, 450, 606, 618, 619), IsGenericTwilekFemale (280, 446, 447, 448, 620, 621, 622).
- Remap outputs: 802-811 (805-808 are Twi'lek M_02 outputs).
- COLLISIONS for chosen rows: 416, 449, 450, 606, 618, 805, 806, 280, 446, 620.
- No collision: 17/14 are human inputs (not alien), and 48, 47, 271, 50, 39, 12, 24, 601, 569, 565, 537, 260, 40, 570, 72, 73, 599, 464, 337, 255, 49, 63 are clear.
- FamilyOf(570) = SlotFamily::None, so Devaronian gets no slot-family effect.

## Open
1. Engine fallback from blank modela to race MDL (rea-recon on SetAppearance MDL-name path).
2. F/L-type MDL-embedded texture names (e.g. rakata01 vs N_Rakata): parse MDL.
3. UTC gender for species rows (not in appearance.2da).
4. modeltype L/S semantics.
5. Sullustan TPC and Twi'lek male texa (n_twilekma01) only name-grepped, not loaded.
