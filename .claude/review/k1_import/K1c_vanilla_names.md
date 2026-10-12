# K1 vanilla NPC names (ground truth, read-only install, pykotor 2.3.12)
2DA: data/2da.bif appearance.2da, 509 rows. Label is a numeric index; the name is in race / modela / texa.
Tex cols: racetex texa texaevil texb..texj headtexve headtexe headtexg headtexvg
Model cols: race modeltype normalhead backuphead modela..modelj headtrack head_arc_h head_arc_v headbone
NPC rows (140 by race/modela/texa scan, 27 combos):
N_CommF (modela N_CommF; texa N_CommF, N_CommFD, N_CommFC)
N_CommM (texa N_CommM, N_CommMD)
N_CommKidF, N_CommKidFD, N_CommKidM, N_CommKidMD
N_CzerkaOff (texa N_CzerkaOff, N_CzerkaOffC)
N_SithComM (texa N_SithComM)
N_TwilekF (texa N_TwilekFA/FB/FC), N_TwilekM (texa N_TwilekMA/MB/MD)
N_FatComF, N_FatComM (rows 25-26, race blank)
L_CommF, L_CommM, L_CommMB (tex cols N_Comm_low_F*, N_Comm_low_M*)
L_TwilekF, L_TwilekM (modela N_TwilekF / N_TwilekM)
MDL texture refs (file names to match; all_textures()):
p_carthh: super S_Female02; P_CarthH01 FOUND swpc_tex_tpa.erf
n_commf: super S_Female03; N_CommF01 FOUND
n_commm: super S_Female02; N_CommM04 FOUND
n_commkidf: super S_Female03; N_CommKidF01, N_childFH01, P_missionbb01 FOUND
n_commkidm: super S_Male02; N_CommKidM01, N_child_M FOUND; PMBF MISSING as exact (pmbf01 exists)
n_czerkaoff: super S_Female02; N_CzerkaOff01 FOUND
n_sithcomm: super S_Female02; N_SithComM03, N_sithoff_H, PMHC02 FOUND
n_twilekf: super S_Female03; N_TwilekFA01, twilek_f01, twilek_f02 FOUND
n_twilekm: super S_Female02; N_TwilekMA01, twilek_m02 FOUND
n_fatcomf: super S_Female03; N_FatComF01, comm_w_f01 FOUND
n_fatcomm: super S_Male02; N_CommM04, N_FatComM01 FOUND
2DA texture values, exact name (case-insensitive):
FOUND: pmbal01, P_BastilaBA, N_Comm_low_F, N_Comm_low_F2, N_Comm_low_F2B, N_Comm_low_F3, N_Comm_low_M2, N_Comm_low_M3, N_Twilek_low_F, N_Twilek_low_M
MISSING exact, numbered file in TPA: N_CommF (N_CommF01), N_CommM (N_CommM01-08), N_CzerkaOff (01), N_SithComM (01-03), N_FatComF/M (01), N_CommKidF/M (01), N_TwilekFA/MA (01), N_Bith (01), N_Calonord (01), N_Duros (01), N_Dodonna (01), N_Smuggler (01), N_DarthRevan (01), N_SithSoldier (01), N_DarkJediF/M (01), P_CandBA/BB (01), P_JoleeBA/BB (01), P_JuhaniBA/BB (01), P_MissionBA/BB (01), P_BastillaBB (p_bastillabb01), P_Zaalbar (p_zaalbar02), PFBC..PFBI and PMBC..PMBI (01-03)
Totals: 107 unique names checked (case-insensitive), 43 found (search-order winner: swpc_tex_tpa.erf)
Trap: appearance texa/texb values and the PFB/PMB body prefixes are base names, not files. Match downloads on the MDL texture names (N_CommF01, N_CommM04, P_CarthH01).
