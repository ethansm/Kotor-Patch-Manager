# PRB1 weapon-hand probe: holster log (10-16)

Source: frozen `scratchpad/prb1_logs/holster_log.txt` (1495 lines). Bare form `scratchpad/prb1_logs/holster_bare.out` (1483 lines); line numbers below are bare-form lines.
Parse: `kotor2_logs.py check` parsed=1173 skipped=0 cap=no; `kplog_parse --stats` truncated=0 malformed=0.

## 1-2. Counts
- HANDATTACH: 11 total (R 7, L 4).
- HANDMAP: 11 total (R 7, L 4). hand=L present at bare 150, 281, 1282, 1298.
- Distinct models: R = 4 (w_Lngswrd_001, w_blstrpstl_015, w_Vbroswrd_001, w_SonicPstl_001). L = 2 (w_SonicPstl_002, w_Vbroshort_001).

## 3. Per-gob table

| gob | hand | model | ATTACH | MAP | PLACE lines |
|---|---|---|---|---|---|
| 9192b28 | R | w_Lngswrd_001 | 79 | 82 | 12 |
| b17c570 | L | w_SonicPstl_002 | 94 | 150 | 0 |
| b17c790 | R | w_blstrpstl_015 | 97 | 154 | 135 |
| bd181b0 | L | w_Vbroshort_001 | 273 | 281 | 0 |
| bd183d0 | R | w_Vbroswrd_001 | 276 | 285 | 0 |
| bb77a88 | R | w_SonicPstl_001 | 1200 | 1222 | 1 |
| ba167f8 | L | w_SonicPstl_002 | 1266 | 1282 | 0 |
| ba16a18 | R | w_blstrpstl_015 | 1268 | 1286 | 0 |
| ba17298 | R | w_Lngswrd_001 | 1273 | 1277 | 57 |
| b2b5988 | L | w_Vbroshort_001 | 1293 | 1298 | 0 |
| b2b5ba8 | R | w_Vbroswrd_001 | 1295 | 1301 | 0 |

PLACE sum 205 matches the kotor2_logs PLACE count.

## 4. Consistency
- 11 of 11 gobs: one HANDATTACH, one HANDMAP, same hand, same name.
- No gob seen with both hands. No name disagreement.
- All 4 PLACE gobs are R-hand. No L-hand gob has a PLACE line.
- L gobs get `PLACEREFUSE reason=nottarget` (b17c570 35, bd181b0 6, ba167f8 8). The lhand node has `target=0` (bare 153), so off-hand gobs are mapped but never placed.

## 5. Caps, WARN/ERROR, log end
- 64-line caps not hit (11 and 11 vs kCap=64, HolsterHand.h:52). No `cap reached max_lines` line.
- WARN/ERROR: 0 matches (case-insensitive).
- Log ends on `PLACE n=21720` (bare 1483). No area-change or shutdown marker, so a clean area change cannot be confirmed from this log.

## 6. Version note
- Log banner: `holster 0.1.0-dev loaded (P0 probe, log-only)`. Source holster.ini also says 0.1.0-dev. The brief said 0.1.1-dev.
- Emitters present: `holster_src/Holster.cpp:459` (HANDATTACH), `:477` (HANDMAP).

## 7. kWorldKnowsHand verdict
- Source `static const bool kWorldKnowsHand = false;` (Holster.cpp:630). This log ran with it off.
- Mapping consistency: YES. Off-hand mapping present: YES.
- The log does not show what enabling the flag changes; off-hand gobs never reach PLACE in this run.
- OPEN: Does `kWorldKnowsHand=true` clear the lhand `nottarget` refusal (target=0) so off-hand gobs place?

## VERDICT: CONFIRMED (criterion: HANDMAP hand=L present and mapping consistent)
