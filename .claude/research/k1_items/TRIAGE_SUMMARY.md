# C2 triage (wave w1-k1items, 10-13): 4 Haiku units, 80 assets

Inputs `triage_in_*.tsv` (cut from `asset_diff_focus.tsv`), outputs `triage_out_*.tsv` (one row per asset). `wave_tools.py check`: u01-u04 PASS (WARNs are other units' files in the shared tree).

| unit | assets | result |
| --- | ---: | --- |
| w_a | 31 | `w_*` weapon MDLs, all have a K2 copy -> `needs_rename` (converter refuses collisions); 1 K1 MDL parse failure (`w_dblsbr_001`) |
| w_b | 30 | same; status PARTIAL only because 3 resrefs match several K1 items (`w_ionrfl_002`, `w_lghtsbr_002`, `w_rptnblstr_002`); first row in file order used for the name |
| p | 15 | K1-only companion bodies/heads (Bastila, Canderous, Carth, Jolee, Juhani, T3-M4); `direct` import, no K2 copy, no item rows |
| icons | 4 | `ii_bithtrombone`, `ip_pituseitm_033`, `ip_pltuseitm_069/070`; 069/070 are Armor Reinforcement / Mesh Underlay (matched via icon_resref, not model_resref) |

Caveats (Opus): node-count change is not proof of a visible difference (K2 recompiles MDLs; e.g. saber 7->9, pistol 5->4 may be hook/dummy nodes). Visual confirmation is C3. Texture "low/high" is a name heuristic, not an UV-island check. Licence for every row: k1-vanilla-local-only.
