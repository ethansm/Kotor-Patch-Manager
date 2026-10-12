# B3: forced-K2 MDL round-trip (TSL models)

Setup: scratch copy of B2a/pk_fix/pykotor. Hook added to MDLBinaryWriter.write() after the auto-detect loop: `if getattr(self, "force_game", None) is not None: self.game = self.force_game`. Caller sets `writer.force_game = Game.K2`. Models read via Installation (read-only), MDX passed as source_ext. Script: `<scratch>/B3/roundtrip.py`, log: `<scratch>/B3/roundtrip.log`.

Model names: `p_bastila` and `p_mission` do not exist in Models.bif. Used `p_bastilah`, `p_carthh` (exist) and `p_hk47` (substitute for p_mission).

| model | MDL in | MDL out | identical | first diff (0-based) | MDX in | MDX out | MDX identical | MDX first diff |
|---|---|---|---|---|---|---|---|---|
| p_bastilah | 144232 | 137708 | no | 4 | 74784 | 74784 | no | 600 |
| p_carthh | 80502 | 79334 | no | 4 | 53120 | 53120 | no | 0 |
| p_hk47 | 1046899 | 1046203 | no | 4 | 229992 | 111200 | no | 32 |

Notes:
- Offset 4 is the MDL content-size word (in-file length minus 12). It differs only because total length differs.
- First MDL body diff after the 12-byte header: p_bastilah 32, p_carthh 29, p_hk47 56.
- Length deltas (MDL): -6524, -1168, -696 bytes.

Verdict: FAIL. The forced K2 writer does not reproduce any of the three TSL MDL files byte-for-byte.
