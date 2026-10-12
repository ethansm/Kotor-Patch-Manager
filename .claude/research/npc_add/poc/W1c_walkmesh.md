# W1c walkmesh recon: 204TEL ward (Doctor Ganius)

Read-only. pykotor Installation on the live Steam dir. Scratch scripts: scratchpad `W1c/` (`ward2.py`, `result.json`).

## 1. Walkmesh source
- Module archives hold no WOK, LYT, VIS or MDL: `204TEL.rim` (ARE, GIT, IFO), `204TEL_s.rim` (UTC/UTP/UTS/NCS...), `204tel.mod` (same plus DLG).
- Layout `204tel` LYT: chitin `data/Layouts.bif`. 7 rooms: 204TELa, b, c, d, e, f, o.
- Room WOKs: chitin `data/Models.bif`, keyed by ROOM model name, not the module name. Read with `Installation.resource(<room>, ResourceType.WOK)`.
- Likely cause of P1's "BIF lookup None": P1 looked up `205tel` (INFERRED).
- Ward room = **204TELb**. All 12 picks below are in it.
- WOK vertices are world coordinates. All 20 beds lie on walkable faces with no offset (20/20). Adding the LYT room offset gives 0/20.
- Floor faces: 519. Walkable (material 3 or 10): 344. Non-walkable (material 7): 175.

## 2. Ward GIT contents (`204tel.mod` GIT, within 12 of (-68.7, 31.3))
- Creature: `corteldc` (Doctor Ganius) at (-68.728, 31.272, 0.000), bearing 0.0. Present in the `.mod` GIT only. The `204TEL.rim` GIT has no creature in range (16 vs 17 creatures in total).
- Placeables: 15 x `plc_bed1`. Tags are blank on all 17 placeables in range. Positions as (x, y), distance to ward centre in brackets:
  - x -71.85: y 22.77 (9.1), 24.84 (7.2), 27.11 (5.2), 29.23 (3.8), 31.31 (3.2)
  - x -64.87: y 22.70 (9.4), 24.77 (7.6), 27.03 (5.8), 29.16 (4.4), 31.24 (3.8)
  - x -61.83: y 22.70 (11.0), 24.77 (9.5), 27.04 (8.1), 29.16 (7.2), 31.24 (6.8)
  - The other 5 beds sit at x about -54.85, beyond 12.
- Other placeables in range: `g_tresmilhig006` (-78.475, 26.252) at 11.0; `comppnl007` (-58.085, 32.878) at 10.7.
- Doors in range: none.

## 3. Free walkable points (12)
- Rules: inside a walkable face and at least 0.3 from its edges. At least 1.5 from every GIT placeable, creature and door POSITION. Bounding boxes were not used. Within 9 of (-68.7, 31.3), spread by farthest-point pick.
- Facing = atan2(yc - y, xc - x), toward the 20-bed centroid (-63.35, 27.00). Convention follows pykotor `game_object.py` L272 (atan2 dy, dx).
- z is from the face plane. All picks are z 0.00.

| # | x | y | facing (rad) | clear to nearest position |
|---|---|---|---|---|
| 1 | -67.20 | 31.30 | -0.839 | 1.53 (next to Ganius) |
| 2 | -76.95 | 27.80 | -0.058 | 2.17 |
| 3 | -68.70 | 22.30 | 0.721 | 3.18 |
| 4 | -60.45 | 27.80 | -2.873 | 1.54 |
| 5 | -73.45 | 31.55 | -0.423 | 1.58 |
| 6 | -73.45 | 24.05 | 0.284 | 1.79 |
| 7 | -66.45 | 26.55 | 0.145 | 1.70 |
| 8 | -60.95 | 32.55 | -1.978 | 1.59 |
| 9 | -70.45 | 28.30 | -0.180 | 1.72 |
| 10 | -63.45 | 24.05 | 1.535 | 1.57 |
| 11 | -63.45 | 29.80 | -1.533 | 1.59 |
| 12 | -73.70 | 27.30 | -0.029 | 1.90 |

- Minimum spacing between picks: 3.26.

## 4. Routes (3 waypoints each, z 0.0, room 204TELb)
- Each segment is sampled every 0.2 units. Every sample is on a walkable face, at least 1.5 from positions, with flat z.
- R1: (-65.70, 20.05) -> (-70.45, 26.30) -> (-66.20, 32.05). Length 15.0.
- R2: (-73.45, 20.30) -> (-65.70, 21.05) -> (-70.20, 27.55). Length 15.7.
- R3: (-68.70, 19.30) -> (-75.45, 22.55) -> (-73.70, 30.30). Length 15.4.

## 5. Caveats and next checks
1. Positions only. Picks 4, 8, 10 and 11 at x -60.45 to -63.45 sit between bed columns (centres about 3 apart) and may overlap bed bodies. Prefer picks at x <= -66.45 until a footprint check is done.
2. `plc_bed1` has a PWK in chitin (`Installation.resource('plc_bed1', ResourceType.PWK)`). A footprint check is possible as a follow-up. Not done, per the brief.
3. Facing points at the bed centroid, not at (-68.7, 31.3). The ward centre would give different facings.
4. Not checked in-engine. The walkability result comes from the WOK material IDs (3 and 10 walkable, 7 not), per pykotor `walkable()`.
