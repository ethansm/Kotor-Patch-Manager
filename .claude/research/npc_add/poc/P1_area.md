# P1 area recon: Telos "medical center across from the Ithorian compound" (Residential West)

Read-only. pykotor from the live Modules/ dir. Area names resolved via dialog.tlk.

## 1. Module map (live `modules/`, Telos range)
- 201TEL: "Citadel Station - Dock Module" (strref 103397). OnEnter k_201tel_enter.
- 202TEL: "Citadel Station - Entertainment" (103414). OnEnter k_202tel_enter.
- 203TEL: "Citadel Station - Residential 082 East" (131570). OnEnter k_203tel_area.
- 204TEL: "Citadel Station - Residential 082 West" (131573). OnEnter k_204tel_enter. 16 creatures, 166 placeables, 7 doors, 48 waypoints, 6 triggers.
- 205TEL: ARE name strref 655 ("Untitled"). OnEnter k_205area_enter. 7 creatures, 19 placeables, 2 doors, 16 waypoints.
- 206TEL: does not exist in live `modules/` or `backups/vanilla_clean/Modules/`.
- 207TEL "Cantina", 208TEL "Bumani Exchange Corp.", 209TEL "Czerka Offices" (not candidates).
- Verdict on structure: there is no dedicated medical module. Medical beds sit inside 204TEL.

Archives (live, bytes):
- 204tel.mod 1,175,774 · 204TEL.rim 60,103 · 204TEL_s.rim 214,864 · 204TEL_dlg.erf 880,123
- 205tel.mod 234,699 (real file) · 205TEL.rim 19,259 · 205TEL_s.rim 71,464 · 205TEL_dlg.erf 144,056

## 2. Candidate A: medical bed ward inside 204TEL (north part)
- 20 x `plc_bed1` in a 4x5 grid: x in {-54.8, -61.8, -64.9, -71.8}, y 22.7 to 31.3, z 0.
- 1 x `plc_workb_med` at (-56.06, 18.16, 0).
- Ithorian compound (south part of 204TEL, for comparison): Chodo Habat `204_Habat` (-59.96, -23.8); Greeter `204_Ith_Rec` (-43.64, -27.54); 6 x Ithorian{Mobile} `com_ith` at about (-35..-51, -42..-57); Ithorian plant (-22.44, -23.75).
- Bed ward to Ithorian compound distance: about 45 to 50 units, on the opposite side of the module (north vs south). "Across" is INFERRED from positions.
- 204TEL has 16 creatures in total. That is not a "few NPCs" area as a whole.

## 3. Candidate B: 205TEL (separate module, small)
- Creatures (tag from UTC): Carth (-78.19, -50.84); Grenn (-65.19, -5.24); TSF_Officer x2 (-59.18, -14.79) and (-59.12, -16.06); Republic_Officer x2 (-70.26, -1.99) and (-74.22, -2.00); Republic_Grenn (-66.42, -6.18).
- Doors: TelosDoor15 (-72.28, -0.69); d_Carth (-77.13, -37.63).
- Transition trigger (-50.03, -4.25). Module entry (-48.96, -5.21).
- Placeables are office props only (desks, chairs, benches, filecabinets, commpanel, locker, plants). No bed or medical placeable.
- Waypoints: wp_pc_1..4, wp_grenn_1..4, wp_repoff_1..4, wp_tsfm_1, wp_tsff_1, wp_bastila_sp, wp_bastila_1, wp_pc_1..4 spread x -33..-78, y -52..-2.
- Module IFO: Area_Name = `204tel`. ARE/GIT keys inside 205 are also `204tel.*` (resref naming artifact). Engine loads by IFO list, so this area is still 205's content (INFERRED).

## 4. Candidate free floor points (UNCONFIRMED: walkmesh not read)
- WOK read failed: Installation lookup for `205tel` WOK returned None (chitin/BIF lookup). Walkability is therefore not confirmed.
- Method: midpoints of consecutive 205TEL waypoints, kept at least 3 units from every creature, placeable and door in 205TEL.
- Only 5 of 10 requested points pass the clearance rule:
  1. (-60.50, -5.25) wp_pc_1/wp_pc_2; nearest Grenn 4.7
  2. (-77.03, -46.64) wp_repoff_3/wp_pc_3; nearest Carth 4.4
  3. (-65.30, -23.93) wp_grenn_4/wp_repoff_4; nearest TSF officer about 11
  4. (-64.31, -14.36) wp_pc_2/wp_grenn_4; nearest TSF officer 5.1
  5. (-70.93, -28.65) wp_grenn_4/wp_pc_4; nearest plc_desk1 about 7.9
- Not computed for candidate A.

## 5. Not read
- Module IFO OnStart / OnClientEnter scripts (only Area_Name and entry read).
- Creature UTC stats beyond tag.
- Triggers' transition destinations in 205TEL. No 204TEL transition trigger sits at about (-50, -4), so the link into 205TEL is unresolved.

## 6. Candidate status
- User's description ("medical center", "smaller area", "few NPCs", "across from the Ithorian compound in Residential West") is ambiguous between A and B.
- A matches "medical" (beds, med workbench). B matches "smaller, few NPCs" (7 vs 16).
- The description cannot be resolved to one area from data alone.
