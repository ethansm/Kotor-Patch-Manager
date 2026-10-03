### calibration

| route | positives | decoys/negatives | negatives hit |
|---|---|---|---|
| R1 SIG>=0.75 | 171 / 435 | 0 / 1167 | - |
| R1 SIG>=0.9 | 171 / 435 | 0 / 1167 | - |
| R1 SIG>=1 | 171 / 435 | 0 / 1167 | - |
| R2 CALLEE>=2 | 5 / 435 | 0 / 1167 | - |
| R1* SIG>=0.75, run_ok not required | 183 / 435 | 1 / 1167 | CServerExoApp::SetLoadFromSaveGame(decoy_prev 0x0051cd60) |
| R3* SIG>=0.75+margin, run_ok not required | 176 / 435 | 1 / 1167 | CServerExoApp::SetLoadFromSaveGame(decoy_prev 0x0051cd60) |
| R3 SIG>=0.75+margin | 164 / 435 | 0 / 1167 | - |
| R3 SIG>=0.9+margin | 164 / 435 | 0 / 1167 | - |
| R3 SIG>=1+margin | 164 / 435 | 0 / 1167 | - |

current AUTO positives: 253 / 435. Pure positional (C0, run_ok yes, steam_size == gog_size, invariants pass): positives 90, decoys/negatives 0.

Stress sets (consistent shift of a whole segment / outlier cluster, all candidates mapper:vote):

| set | rows | pass all invariants | size==gog_size | current AUTO | R1 (SIG>=0.75) | R2 (CALLEE>=2) | R3 (R1+margin) | R1* (no run_ok) | R3* (no run_ok) |
|---|---|---|---|---|---|---|---|---|---|
| shift_p1 | 435 | 41 | 43 | 0 | 0 | 0 | 0 | 0 | 0 |
| shift_m1 | 435 | 38 | 46 | 0 | 0 | 0 | 0 | 1 | 1 |
| shift_p2 | 435 | 44 | 30 | 0 | 0 | 0 | 0 | 2 | 0 |
| shift_m2 | 435 | 43 | 30 | 0 | 0 | 0 | 0 | 2 | 1 |
| neg_cluster | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

R1*/R3* ignore run_ok: an upper bound for what a self-supporting wrong segment could reach if the route were adopted and its rows propagated as mapped neighbours (run_ok cannot be seeded by rows that are not AUTO, so the R1/R2/R3 columns above only count true seeds).

Every stress row reaching any route:

| set | GOG | name | Steam | families | run_ok | a_sig | routes |
|---|---|---|---|---|---|---|---|
| shift_m1 | 0x0053cf90 | CSWGuiMainMenu::OnLoadGamePicked | 0x008824f0 | SIG | n/a | 1.00 | R1*,R3* |
| shift_p2 | 0x00908ea0 | CAuroraInterface::GetProcessedTextureInternal | 0x0093eac0 | SIG | no | 0.86 | R1* |
| shift_p2 | 0x00909030 | CAuroraInterface::ReleaseProcessedTextureInternal | 0x0093ec50 | SIG | no | 0.75 | R1* |
| shift_m2 | 0x00908ea0 | CAuroraInterface::GetProcessedTextureInternal | 0x0093e640 | SIG | no | 0.86 | R1*,R3* |
| shift_m2 | 0x00909030 | CAuroraInterface::ReleaseProcessedTextureInternal | 0x0093e7d0 | SIG | no | 0.75 | R1* |

### calibration/alt_block20

| route | positives | decoys/negatives | negatives hit |
|---|---|---|---|
| R1 SIG>=0.75 | 91 / 435 | 0 / 1167 | - |
| R1 SIG>=0.9 | 91 / 435 | 0 / 1167 | - |
| R1 SIG>=1 | 91 / 435 | 0 / 1167 | - |
| R2 CALLEE>=2 | 3 / 435 | 0 / 1167 | - |
| R1* SIG>=0.75, run_ok not required | 216 / 435 | 1 / 1167 | CServerExoApp::SetLoadFromSaveGame(decoy_prev 0x0051cd60) |
| R3* SIG>=0.75+margin, run_ok not required | 207 / 435 | 1 / 1167 | CServerExoApp::SetLoadFromSaveGame(decoy_prev 0x0051cd60) |
| R3 SIG>=0.75+margin | 87 / 435 | 0 / 1167 | - |
| R3 SIG>=0.9+margin | 87 / 435 | 0 / 1167 | - |
| R3 SIG>=1+margin | 87 / 435 | 0 / 1167 | - |

current AUTO positives: 122 / 435. Pure positional (C0, run_ok yes, steam_size == gog_size, invariants pass): positives 35, decoys/negatives 0.

Stress sets (consistent shift of a whole segment / outlier cluster, all candidates mapper:vote):

| set | rows | pass all invariants | size==gog_size | current AUTO | R1 (SIG>=0.75) | R2 (CALLEE>=2) | R3 (R1+margin) | R1* (no run_ok) | R3* (no run_ok) |
|---|---|---|---|---|---|---|---|---|---|
| shift_p1 | 435 | 41 | 43 | 0 | 0 | 0 | 0 | 0 | 0 |
| shift_m1 | 435 | 38 | 46 | 0 | 0 | 0 | 0 | 1 | 1 |
| shift_p2 | 435 | 44 | 30 | 0 | 0 | 0 | 0 | 2 | 0 |
| shift_m2 | 435 | 43 | 30 | 0 | 0 | 0 | 0 | 2 | 1 |
| neg_cluster | 19 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

R1*/R3* ignore run_ok: an upper bound for what a self-supporting wrong segment could reach if the route were adopted and its rows propagated as mapped neighbours (run_ok cannot be seeded by rows that are not AUTO, so the R1/R2/R3 columns above only count true seeds).

Every stress row reaching any route:

| set | GOG | name | Steam | families | run_ok | a_sig | routes |
|---|---|---|---|---|---|---|---|
| shift_m1 | 0x0053cf90 | CSWGuiMainMenu::OnLoadGamePicked | 0x008824f0 | SIG | n/a | 1.00 | R1*,R3* |
| shift_p2 | 0x00908ea0 | CAuroraInterface::GetProcessedTextureInternal | 0x0093eac0 | SIG | no | 0.86 | R1* |
| shift_p2 | 0x00909030 | CAuroraInterface::ReleaseProcessedTextureInternal | 0x0093ec50 | SIG | no | 0.75 | R1* |
| shift_m2 | 0x00908ea0 | CAuroraInterface::GetProcessedTextureInternal | 0x0093e640 | SIG | no | 0.86 | R1*,R3* |
| shift_m2 | 0x00909030 | CAuroraInterface::ReleaseProcessedTextureInternal | 0x0093e7d0 | SIG | no | 0.75 | R1* |

