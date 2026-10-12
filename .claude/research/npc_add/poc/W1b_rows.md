# W1b: appearance.2da row resolution (pykotor, live install, 2026-10-11)

Method: pykotor Installation on Steam KOTOR II (read-only). Row data = live Override appearance.2da (what the game reads). Body/head MDL names resolved per container; texture names read from MDL meshes (`all_textures()`). Speed = creaturespeed.2da (chitin) row whose 2daname = moverate. Column logic cited from P2_species.md, not redone.
Containers: O = Override loose, C = chitin BIF, TPA/TPB/TPC = TexturePacks ERF. Modules and GUI had no hits. All texture hits are TPC except N_CommM04 (also TGA in O).

| row | label | body (modela, or race if blank) -> MDL/MDX | head row -> MDL | textures -> containers | soundapptype | creaturespeed idx walk/run (row walkdist/rundist) | footstepsound / footsteptype | portrait / sex | status |
|---|---|---|---|---|---|---|---|---|---|
| 48 | Twilek_Male_01_C | N_TwilekM -> C / C | 62 twilek_m -> O,C; alt twilek_m01 O,TPA,TPB,TPC | N_TwilekMA01 O,TPA,TPB,TPC; twilek_m02 O,TPA,TPB,TPC (texa N_TwilekMA itself: no texture) | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 1 Hard | blank / no column | OK |
| 47 | Twilek_Female_01_A | N_TwiDance_Boot -> O / O (mod-shipped, identical to npc_diversity_src) | 61 P_TwiH1 -> O only (not in mod src) | N_TwiDanceA01 O; P_Twi1 O; P_Twi2 O (none in C or TPs) | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.6/3.66) | blank / 1 Hard | blank / no column | PARTIAL: N_TwiDanceA01, P_Twi1, P_Twi2, P_TwiH1.mdl (Override loose, not shipped by mod) |
| 271 | Alien_Wookie_Male_02 | N_WookieM -> O,C / O,C | none | N_WookieM06 O,TPA,TPB,TPC; racetex N_WookieM02 O,TPA,TPB,TPC | 1 Humanoid_Large | idx4 NORM 1.70/5.40 (1.9/5.4) | blank / 0 Soft | blank / no column | OK |
| 50 | Alien_Wookie_Female_01 | N_WookieF -> O,C / O,C | none | N_WookieF01 O,TPA,TPB,TPC | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.7/5.4) | blank / 0 Soft | blank / no column | OK |
| 39 | Alien_Rodian_01 | N_Rodian -> O,C / O,C | none | N_Rodian01 O,TPA,TPB,TPC (racetex same) | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (2/4) | blank / 1 Hard | blank / no column | OK |
| 12 | Alien_Bith_01 | N_Bith -> O,C / O,C | none | N_Bith01 O,TPA,TPB,TPC | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 1 Hard | blank / no column | OK |
| 24 | Alien_Duros_01 | N_Duros -> O,C / O,C (58 mesh nodes, all texture NULL) | none | MDL has no texture; racetex N_Duros01 O,TPA,TPB,TPC used | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.9/3.66) | blank / 0 Soft | blank / no column | OK |
| 601 | Alien_Sullustan | N_Sullustan -> C / C | none | N_Sullustan TPA,TPB,TPC (texa name also resolves there) | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 0 Soft | blank / no column | OK |
| 569 | Alien_Gand_01 | N_Gand -> C / C | none | N_gand_D TPA,TPB,TPC; texa/racetex N_Gand: no texture | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 0 Soft | blank / no column | OK |
| 260 | Alien_Selkath_03 | N_Selkath -> O,C / O,C | none | N_Selkath01 O,TPA,TPB,TPC; racetex N_Selkath03 O,TPA,TPB,TPC | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.7/5.4) | blank / 0 Soft | blank / no column | OK |
| 570 | Alien_Devaronian_01 | N_CommM (human) -> O,C / O,C | 129 N_DevH -> C | N_CommM04 O,TPA,TPB,TPC (+TGA O); N_DevH01 O,TPA,TPB,TPC | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 0 Soft | blank / no column | OK |
| 565 | Alien_Gran_01 | N_Gran -> C / C | none | N_Gran TPA,TPB,TPC (texa name also resolves there) | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 0 Soft | blank / no column | OK |
| 537 | Alien_Quarren_01 | N_Quarren -> C / C | none | N_QarrenB01 O,TPA,TPB,TPC; N_QuarrenH01 O,TPA,TPB,TPC (texa N_Quarren: no texture) | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 1 Hard | blank / no column | OK |
| 599 | Alien_Weeqay | N_Weeq -> C / C | none | N_WeeqB01 O,TPA,TPB,TPC; N_WeeqH01 O,TPA,TPB,TPC (texa N_Weeq: no texture) | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.813/3.66) | blank / 0 Soft | blank / no column | OK |
| 72 | Alien_Ithorian_01 | modela blank -> race C_Ithorian -> O,C / O,C | none | C_Ithorian01 O,TPA,TPB,TPC | 0 Humanoid_Normal | idx3 SLOW 1.25/2.50 (1/2) | blank / 0 Soft | blank / no column | OK (race fallback, unconfirmed) |
| 73 | Alien_Jawa | modela blank -> race C_Jawa -> C / C | none | C_Jawa01 O,TPA,TPB,TPC | 2 Simple_Small | idx11 WEE_FOLK 0.7/1.4 (0.7/1.4) | blank / 0 Soft | blank / no column | OK (race fallback, unconfirmed) |
| 464 | Alien_Trandoshan_01 | modela blank -> race Sch_Trando -> O only / O only | none | P_Trndoshan O,TPA,TPB,TPC; racetex p_trndoshan O,TPA,TPB,TPC | 0 Humanoid_Normal | idx4 NORM 1.70/5.40 (1.9/5.4) | blank / 0 Soft | blank / no column | PARTIAL: Sch_Trando.mdl/mdx Override loose, not in mod src |
| 63 | Droid_Protocol_01 | modela blank -> race C_DrdProt -> O,C / O,C | none | C_DrdProt01 O,TPA,TPB,TPC | 6 Droid_Normal | idx3 SLOW 1.25/2.50 (0.6/0.7) | blank / 2 Droid | blank / no column | OK (race fallback, unconfirmed) |

Live vs vanilla (appearance.2da):
- Vanilla (chitin 2DA.bif): 671 rows. Live Override: 812 rows. Live file is byte-identical in content to npc_diversity_src/override/appearance.2da (sha256 43e312dd...), so the Override copy is the diversity mod's. Rows 671-811 are mod-added.
- The 18 rows: 16 equal vanilla. Row 47 differs (vanilla N_TwilekF body, texa N_TwilekFA; live N_TwiDance_Boot, texa N_TwiDanceA). Row 464 differs (vanilla race N_Trandoshan, walk/run 1.813/3.66; live race Sch_Trando, 1.9/5.4). Vanilla N_Trandoshan.mdl exists in O and C.
- heads.2da: live Override equals mod src (sha256 e0f8381c...).

Totals: OK 16, PARTIAL 2 (47, 464), MISSING 0 of 18. No MISSING resref.
Corrections to P2 open items: Sullustan texture N_Sullustan and Twi'lek M N_TwilekMA01 are now resolved (TPC). Twi'lek F texture N_TwiDanceA01 resolves only from loose Override files.
