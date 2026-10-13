# NPC head wearability scan (static)

Game dir: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II` (read-only). appearance.2da rows: 818. utc_equip.tsv rows read: 5417. UTC rows whose appearance_type is outside appearance.2da: 0 (apps none).

Premise: the helmet attaches at node `GoggleHook` on the head model. Output: `head_wearability.tsv` (one row per appearance row).

## Counts by has_goggle_hook

- yes: 276
- no: 529
- LOAD_FAIL: 11
- NO_HEAD: 2

## has_goggle_hook by head_source

normalhead / backuphead = head from heads.2da; body = no head column, the body model is the head (assumption); none = no head and no body model.

- normalhead: yes=244, no=242, LOAD_FAIL=4
- backuphead: none
- body: yes=32, no=287, LOAD_FAIL=7
- none: NO_HEAD=2

## has_goggle_hook x body_headhook (rows)

- has_goggle_hook yes: body_headhook yes=274, body_headhook no=1, body_headhook LOAD_FAIL=1
- has_goggle_hook no: body_headhook yes=349, body_headhook no=179, body_headhook LOAD_FAIL=1
- has_goggle_hook LOAD_FAIL: body_headhook yes=2, body_headhook LOAD_FAIL=9
- has_goggle_hook NO_HEAD: body_headhook NO_BODY=2

## Coverage

- appearance rows with >=1 template in utc_equip.tsv: 530
- appearance rows with >=1 head-item template: 54
- body_headhook: yes=625, no=180, LOAD_FAIL=11, NO_BODY=2
- heads.2da index failures (non-int, out of range; treated as absent): 0
- body_model values that are not plain resrefs (e.g. '&638' in modela; treated as the body model, so usually LOAD_FAIL): 2

## Contradictions: head items on rows without GoggleHook (28 rows; top 15 by n_utc)

- row 35 Alien_Mandalorian_01 (modeltype F): head `N_Mandalorian` [body], has_goggle_hook=no, body_headhook=yes, n_utc=32, n_utc_with_head_item=3
- row 596 Commoner_Mal_White_01_B (modeltype B): head `comm_w_m` [normalhead], has_goggle_hook=no, body_headhook=yes, n_utc=24, n_utc_with_head_item=1
- row 569 Alien_Gand_01 (modeltype F): head `N_Gand` [body], has_goggle_hook=no, body_headhook=yes, n_utc=23, n_utc_with_head_item=22
- row 17 Commoner_Mal_Asian_01_A (modeltype B): head `comm_a_m` [normalhead], has_goggle_hook=no, body_headhook=yes, n_utc=23, n_utc_with_head_item=3
- row 464 Alien_Trandoshan_01 (modeltype F): head `Sch_Trando` [body], has_goggle_hook=no, body_headhook=yes, n_utc=23, n_utc_with_head_item=3
- row 351 Alien_Mandalorian_02 (modeltype F): head `N_Mandalorian` [body], has_goggle_hook=no, body_headhook=yes, n_utc=22, n_utc_with_head_item=5
- row 56 Droid_Assassin (modeltype L): head `C_DrdAssassin` [body], has_goggle_hook=no, body_headhook=no, n_utc=17, n_utc_with_head_item=5
- row 701 DP_Republic_Doctor_Fem_B01 (modeltype F): head `DP_RepMed_F_B01` [body], has_goggle_hook=no, body_headhook=yes, n_utc=17, n_utc_with_head_item=2
- row 565 Alien_Gran_01 (modeltype F): head `N_Gran` [body], has_goggle_hook=no, body_headhook=yes, n_utc=17, n_utc_with_head_item=1
- row 591 Commoner_Mal_Asian_01_B (modeltype B): head `comm_a_m` [normalhead], has_goggle_hook=no, body_headhook=yes, n_utc=14, n_utc_with_head_item=2
- row 465 Alien_Trandoshan_02 (modeltype F): head `Sch_Trando` [body], has_goggle_hook=no, body_headhook=yes, n_utc=13, n_utc_with_head_item=2
- row 592 Commoner_Mal_Black_01_B (modeltype B): head `comm_b_m` [normalhead], has_goggle_hook=no, body_headhook=yes, n_utc=13, n_utc_with_head_item=2
- row 352 Alien_Mandalorian_03 (modeltype F): head `N_Mandalorian` [body], has_goggle_hook=no, body_headhook=yes, n_utc=12, n_utc_with_head_item=3
- row 366 Twilek_Female_03_C (modeltype B): head `P_TwiH3` [normalhead], has_goggle_hook=no, body_headhook=yes, n_utc=12, n_utc_with_head_item=2
- row 270 Twilek_Female_02_B (modeltype B): head `P_TwiH2` [normalhead], has_goggle_hook=no, body_headhook=yes, n_utc=12, n_utc_with_head_item=1

## GoggleHook yes but zero head-item templates, n_utc >= 10 (9 rows; top 15 by n_utc)

- row 528 Dark_Jedi_Low_Male_01 (modeltype B): head `N_DarkJH01` [normalhead], has_goggle_hook=yes, body_headhook=yes, n_utc=39, n_utc_with_head_item=0
- row 607 Onderon_Soldier_Mal_Masked (modeltype B): head `N_OndSoldierH` [normalhead], has_goggle_hook=yes, body_headhook=yes, n_utc=39, n_utc_with_head_item=0
- row 672 Handmaiden_Sister (modeltype F): head `N_HandSis` [normalhead], has_goggle_hook=yes, body_headhook=yes, n_utc=18, n_utc_with_head_item=0
- row 459 Sith_Assassin_01 (modeltype B): head `N_SithAssnH` [normalhead], has_goggle_hook=yes, body_headhook=yes, n_utc=12, n_utc_with_head_item=0
- row 534 Dark_Jedi_Mid_Male_01 (modeltype B): head `N_DarkJH01` [normalhead], has_goggle_hook=yes, body_headhook=yes, n_utc=12, n_utc_with_head_item=0
- row 536 Dark_Jedi_Mid_Male_03 (modeltype B): head `N_DkJH02` [normalhead], has_goggle_hook=yes, body_headhook=yes, n_utc=12, n_utc_with_head_item=0
- row 564 Dark_Jedi_Hi_Male_03 (modeltype B): head `N_DkJH02` [normalhead], has_goggle_hook=yes, body_headhook=yes, n_utc=12, n_utc_with_head_item=0
- row 629 Party_NPC_Atris (modeltype F): head `P_AtrisBB` [body], has_goggle_hook=yes, body_headhook=yes, n_utc=11, n_utc_with_head_item=0
- row 706 DP_Peragus_Miner_Security_Officer (modeltype F): head `DP_MineSctyOff` [body], has_goggle_hook=yes, body_headhook=yes, n_utc=11, n_utc_with_head_item=0

## Load failures (count only)

- rows with has_goggle_hook=LOAD_FAIL: 11 (distinct head resrefs: 9)
- rows with body_headhook=LOAD_FAIL: 11
- distinct MDL resrefs that failed (head and body candidates), by reason: MDL not found=21
- distinct MDL resrefs with MDX not found (read with empty MDX): 0

## Notes

- n_utc counts distinct template resrefs (case-insensitive) per appearance_type, so a template in several containers counts once. n_utc_with_head_item uses has_head_item == 1 (HEAD slot or a headgear base item; not every such item is a helmet).
- Zero head-item templates are not evidence that a row cannot wear a helmet: the sample may simply not equip one.
- body_headhook checks modela, modelb and race together, so it is a union, not the single body model in the body_model column.
- head_source=body is an assumption, not a verified engine path. Its has_goggle_hook result tests the body mesh for GoggleHook.
- Not checked here: whether the node is reachable in the rendered skeleton, whether a helmet item's model is attached there at runtime, or any engine-side visibility rule.

## Reading

276 of 818 appearance rows resolve to a head model with a GoggleHook node, and 529 resolve to one without. Eleven rows fail to load because the named MDL is not in the install; the failing names include p_CandH, p_JoleeH and P_JuhaniH, which heads.2da names but which are absent, while p_candh03 exists and has GoggleHook (the scan does not follow suffix fallbacks). Of the 54 rows that equip any head-slot item, 26 have GoggleHook and 28 do not (13 use a heads.2da head, 15 fall back to the body model), and 21 of those 28 equip a_helmet_* items. So GoggleHook on the head model does not account for every helmet-like item, and the data does not show which node carries them. Nine rows have GoggleHook but equip no head item across at least 10 templates each, which shows the node is present but not whether those rows can wear a helmet. The body_headhook check is uninformative (it is yes on 349 of the 529 no-GoggleHook rows), so it does not separate helmet-capable rows.
