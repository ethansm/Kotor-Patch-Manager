# Alien appearance rows as commoner replacements (recon 01)

Source: `patch_manager_mods/npc_diversity_src/override/appearance.2da` (812 rows) and `heads.2da` (same dir).
Game checks (read-only): `chitin.key` (MDL), `Override/` (loose MDL), `TexturePacks/*.erf` (TPC name index).

Class: **STD** = modela is N_CommM/N_CommF (human body, alien head only). **SPEC** = species-specific body.
Head = `normalhead` row in heads.2da, then its HEAD model and presence (KEY / OVR / MISSING).

## Table (38 rows; same-family duplicates omitted, see Not listed)

| idx | label | race | modela | texa | class | head (normalhead -> model: presence) | verdict |
|---|---|---|---|---|---|---|---|
| 570 | Alien_Devaronian_01 | N_CommM | N_CommM | N_CommM | STD | 129 -> N_DevH: KEY | OK, head swap only |
| 416 | Twilek_Commoner_M_01_A | N_CommM | N_CommM | N_CommM | STD | 78 -> twilek_m: KEY+OVR | OK, head swap only |
| 449 | Twilek_Commoner_M_01_C | N_CommM | N_CommM | N_CommM | STD | 62 -> twilek_m: KEY+OVR | OK, head swap only |
| 450 | Twilek_Commoner_M_01_D | N_CommM | N_CommM | N_CommM | STD | 64 -> twilek_m: KEY+OVR | OK, head swap only |
| 606 | Twilek_Commoner_M_01_F | N_CommM | N_CommM | N_CommM | STD | 67 -> twilek_m: KEY+OVR | OK, head swap only |
| 618 | Twilek_Commoner_M_01_G | N_CommM | N_CommM | N_CommM | STD | 157 -> twilek_m: KEY+OVR | OK, head swap only |
| 805 | Twilek_Commoner_M_02_A | N_CommM | N_CommM | N_CommM | STD | 192 -> llk_Ahrnell: OVR, alt TPC missing | PARTIAL |
| 806 | Twilek_Commoner_M_02_B | N_CommM | N_CommM | N_CommM | STD | 193 -> llk_Harra: OVR, alt TPC missing | PARTIAL |
| 446 | Twilek_Commoner_F_01_A | N_CommF | N_CommF | N_CommF | STD | 61 -> P_TwiH1: MISSING | NEEDS HEAD |
| 447 | Twilek_Commoner_F_01_B | N_CommF | N_CommF | N_CommF | STD | 63 -> P_TwiH2: MISSING | NEEDS HEAD |
| 280 | Twilek_Commoner_F_01_D | N_CommF | N_CommF | N_CommF | STD | 93 -> P_TwiHYuthura: MISSING | NEEDS HEAD |
| 620 | Twilek_Commoner_F_01_E | N_CommF | N_CommF | N_CommF | STD | 159 -> P_TwiH5: MISSING | NEEDS HEAD |
| 12 | Alien_Bith_01 | N_Bith | N_Bith | N_Bith | SPEC | none (F-type) | OK, body in KEY, TPC n_bith01 |
| 24 | Alien_Duros_01 | N_Duros | N_Duros | N_Duros | SPEC | none (F-type) | OK, body KEY, TPC n_duros01-05 |
| 39 | Alien_Rodian_01 | N_Rodian | N_Rodian | N_Rodian | SPEC | none (F-type) | OK, body KEY, TPC n_rodian01-05 |
| 40 | Alien_Selkath_01 | N_Selkath | N_Selkath | N_Selkath | SPEC | none (type L) | OK, type L unverified |
| 601 | Alien_Sullustan | N_Sullustan | N_Sullustan | N_Sullustan | SPEC | none (F-type) | OK, body KEY, TPC n_sullustan |
| 569 | Alien_Gand_01 | N_Gand | N_Gand | N_Gand | SPEC | none (F-type) | OK, body KEY, TPC n_gand_d / n_gandb |
| 565 | Alien_Gran_01 | N_Gran | N_Gran | N_Gran | SPEC | none (F-type) | OK, body KEY, TPC n_gran |
| 537 | Alien_Quarren_01 | N_Quarren | N_Quarren | N_Quarren | SPEC | none (F-type) | OK, body KEY, TPC n_quarrenh01 |
| 599 | Alien_Weeqay | N_Weeq | N_Weeq | N_Weeq | SPEC | none (F-type) | OK, body KEY, TPC n_weeqb01 / h01 |
| 789 | Alien_Weeqay_Dark | N_Weeq02 | N_Weeq02 | N_Weeq02 | SPEC | none (F-type) | PARTIAL, body OVR only, no TPC |
| 49 | Alien_Walrusman | N_Walrusman | N_Walrusman | N_Walrusman | SPEC | none (F-type) | OK, body KEY, TPC walrusman01-04 |
| 36 | Alien_Rakata_01 | N_Rakata | N_Rakata | N_Rakata | SPEC | none (F-type) | OK, body KEY, TPC rakata01 |
| 50 | Alien_Wookie_Female_01 | N_WookieF | N_WookieF | N_WookieF | SPEC | none (F-type) | OK, body KEY, TPC wookief01-04 |
| 782 | n_ewok | n_ewok | n_ewok | n_ewok | SPEC | none (F-type) | PARTIAL, body and TPC in OVR only |
| 48 | Twilek_Male_01_C | N_TwilekM | N_TwilekM | N_TwilekMA | SPEC | 62 -> twilek_m: KEY+OVR | OK, body KEY, TPC twilekma01 |
| 269 | Twilek_Male_02_C | N_TwilekM | N_TwilekM | N_TwilekMB | SPEC | 62 -> twilek_m: KEY+OVR | OK, body KEY, TPC twilekmb01 |
| 326 | Lite_Twilek_Male | L_TwilekM | N_TwilekM | N_TwilekM | SPEC | 62 (type F, ref unused?) | OK, UNVERIFIED head ref |
| 774 | N_TwilekFH01 | N_TwilekF | N_TwilekFH01 | (blank) | SPEC | 227 -> llk_DahnisH: OVR | PARTIAL, body OVR only, texa blank |
| 677 | Masked_Duros | N_MaskedDuros | N_MaskedDuros | (blank) | SPEC | (blank) | PARTIAL, body OVR only, texa blank |
| 47 | Twilek_Female_01_A | N_TwiDance_Boot | N_TwiDance_Boot | N_TwiDanceA | SPEC | 61 -> P_TwiH1: MISSING | NOT VIABLE, body MISSING |
| 270 | Twilek_Female_02_B | N_TwiDance | N_TwiDance | N_TwiDanceB | SPEC | 63 -> P_TwiH2: MISSING | NOT VIABLE, body MISSING |
| 809 | Twilek_Female_Dancer_Boot | N_CommF | N_TwiDance_Boot | N_TwiDanceA | SPEC | 198 -> P_TwiHNGreen: MISSING | NOT VIABLE, body and head MISSING |
| 811 | Twilek_Female_Underwear | N_CommF | N_TwiUnd | N_CommF | SPEC | 204 -> P_TwiHTRed: MISSING | NOT VIABLE, body and head MISSING |
| 72 | Alien_Ithorian_01 | C_Ithorian | (blank) | (blank) | S | (blank) | NOT VIABLE, no model in row |
| 73 | Alien_Jawa | C_Jawa | (blank) | (blank) | S | (blank) | NOT VIABLE, no model in row |
| 464 | Alien_Trandoshan_01 | Sch_Trando | (blank) | (blank) | F | (blank) | NOT VIABLE, no model in row |

Not listed (same family as a row above): 417, 445, 619 (head OK); 448, 621, 622 (head MISSING); 807, 808 (llk heads, PARTIAL); 631, 256-258, 259, 260, 323, 418-420, 365, 320, 51, 271, 272, 275-278, 327, 427, 783.
Zabrak, Cathar, Aqualish and Mon Calamari have no rows in this 2DA. Walrusman is the closest Aqualish-type model.
Excluded by design: unique NPCs (13, 499, 498, 524, 556, 624, 682, 770, 767, 790); hostile (Tusken 46/266-268/382/90, Selkath Insane 335/336, Twi'lek assassins 652/653, Mandalorians 35/351/352/788); flying Toydarian 661; human bodies (N_Smuggler, Sith, Jedi).

## Counts (heuristic: race or label matches an alien species, excluding PC_/P_ player bodies)
- 139 alien-matched NPC rows.
- 33 with blank modela (not viable).
- 20 with modela N_Comm* (STD, all Twi'lek or Devaronian heads).
- 86 with non-blank modela not starting with N_Comm (SPEC). Includes hostile and human-body rows, so the usable count is lower.

## Row-swap hazards
1. Walk/run pace: commoners use walkdist/rundist/driveanimwalk 1.813/3.66/1.813. Selkath and Wookiee are 1.7/5.4/1.7, Rakata 2.05/4.05/2.05, Rodian 2/4/1.7, ewok 1.5/3.5/2. Mismatch is expected; in-game effect UNVERIFIED.
2. Body path: B rows (commoner, Twi'lek male) load a body plus a heads.2da head. F/L rows load one mesh with the head baked in. Per README the texture hook targets GetBTypeModelAndTextureResRef, so F/L rows skip commoner texture rotation. Exact texa names (e.g. N_Duros) have no TPC; files are suffixed (n_duros01-05).
3. Head: normalhead must resolve to a heads.2da row with an existing HEAD model. P_TwiH*, P_TwiHNGreen, P_TwiHTRed and P_TwiHYuthura are MISSING from KEY and Override; README ties them to the HD Twi'lek mod. Affects 47, 270, 280, 446-448, 620-622, 809, 811.
4. Sound and footsteps: footsteptype 1 on Bith (12), Rodian (39), Quarren (537), Twi'lek male (48, 326), Twi'lek F (774); soundapptype 1 on Wookiee F 275-278, 2 on ewok2 (783). combat_sound and armor_sound are blank on all listed rows.
5. Hit-box and size: hitradius 0.25, height 1, perspace 0.35, creperspace 0.4, sizecategory 3 are identical on all listed rows, so the row sets no hit-box change. Real collision comes from the MDL (UNVERIFIED in game).

## Open
- In-game test: F/L alien bodies (no commoner texture rotation) and head-missing rows. Playtest is the user's.
- Override-only assets (N_Weeq02, N_TwilekFH01, n_ewok, N_MaskedDuros, llk_* heads): confirm which mod ships them.
- Texture suffix rule for F/L texa is unverified (engine appends NN or not).
- modeltype L semantics unverified (Selkath 40, 259, 260, 323).
