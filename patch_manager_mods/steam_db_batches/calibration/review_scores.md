# Review scores

key `calibration/cards/key.tsv`, review `calibration/review/sonnet_r1.tsv`

| rows | match | label_suspect | mismatch | unsure | missing | total |
|---|---|---|---|---|---|---|
| expected match | 39 | 0 | 0 | 0 | 0 | 39 |
| expected mismatch | 1 | 0 | 15 | 1 | 0 | 17 |
| annotated label_suspect | 3 | 0 | 1 | 0 | 0 | 4 |

- precision on `match` (matches on expected-match rows / all matches, annotated rows excluded): 39 / 40 = 97.5%
- recall on expected-match rows: 100.0%
- annotated label_suspect rows are counted apart: 4 (verdicts: match=3, mismatch=1)

## False accepts (expected mismatch, reviewer `match`)

- CAL-1-55 0x006b1480 -> 0x0054a700 (decoy_prev): Size 153 and offsets equal except uniform -4 on 0x158/0x234; callee names differ but shapes align.

## Expected-match rows not judged `match`

None.

## Annotated label_suspect rows

- CAL-1-15 0x004e0c60 -> 0x007d0560 (neg_b1rej): reviewer match - Exact offset set and size; GOG label seems swapped with RemoveGUIPopUps but address fingerprint matches.
- CAL-1-28 0x004dbde0 -> 0x007cb6e0 (neg_b1rej): reviewer match - Four-offset set and size 55 equal; name semantics odd but address fingerprint matches.
- CAL-1-34 0x004df1f0 -> 0x007cf710 (pos): reviewer mismatch - GOG fingerprint (size 90, disp 0x9c, imm 0x440) matches 0x7ceaf0, not the 112-byte list scan.
- CAL-1-42 0x004e0ca0 -> 0x007d05a0 (neg_b1rej): reviewer match - Offsets, imms and GetMiniGame callee equal; GOG label seems swapped with GetMiniMapVisible.

## Unsure

- CAL-1-22 0x0084bb30 -> 0x00459560 (neg_b1rej)
