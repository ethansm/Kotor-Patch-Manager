# B9 Summary: Server Core & World Lifecycle (2026-10-03)

Scope: 270 GOG rows across `CServerExoApp` (45), `CServerExoAppInternal` (87), and `CSWSModule` (138).

## Inputs & Gate Breakdown
- `evidence.tsv`: 270 candidate rows
  - AUTO: 102
  - REVIEW: 166
  - NOCAND: 1
  - REJECT: 1
- Reviewer pass: Sonnet reviewed 4 chunks (166 REVIEW rows + 40 decoy cards).
  - 1 decoy card was accepted (decoy tolerance = 0), triggering ESCALATE: 100% of reviewer matches sent to Opus.
- Opus Gate: Split into three parts (`opus/part_a.md`, `part_b.md`, `part_c.md`) covering all 166 REVIEW rows and 8 AUTO meaning checks:
  - `overrides_a.tsv`: 60 rows (59 verified, 1 likely, 0 sample errors)
  - `overrides_b.tsv`: 60 rows (58 verified, 2 likely, 0 sample errors)
  - `overrides_c.tsv`: 54 rows (52 verified, 1 likely, 1 label_suspect, 0 sample errors)
  - Combined `overrides.tsv`: 174 rows (169 verified, 4 likely, 1 label_suspect, 0 sample errors).

## Finalization Results
Executed `python3 patch_manager_mods/steam_db_batches/tools/batch.py finalize B9 --opus-sample-errors 0`:
- **Verified**: 263
- **Likely**: 4
- **Rejected**: 1
- **Unresolved**: 1
- **Label Suspect**: 1
- Total: 270 rows
- `functions.csv`: 263 rows

## Knowledge Base & Validated Ledger
Executed `python3 patch_manager_mods/steam_db_batches/tools/batch.py persist B9 --apply`:
- Upserted high-confidence Steam mappings into KDB `name_map`.
- Updated `steam_validated` ledger with sha1 fingerprints for all 263 verified functions.
- Verified idempotence (subsequent run produced 0 changes).

## Branch Database Import (`kotor2_steam_aspyr.db`)
Executed `SqliteTools import-ghidra --csv B9/functions.csv --database AddressDatabases/kotor2_steam_aspyr.db --mode append`:
- 247 new functions added.
- 16 existing functions confirmed at identical addresses.
- 0 address or name conflicts.
- Total functions in database: **2,321 -> 2,568 entries**.
- `SqliteTools validate` passed with 0 schema or integrity errors.
- Committed to branch `addressdb/k2-steam-acc` at `4b81a2b`.
