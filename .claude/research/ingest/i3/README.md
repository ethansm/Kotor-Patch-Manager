# INGEST I3: research-notes names (2026-10-27) - gate FAIL, nothing written

- Extract: `ingest_names.py extract --source notes [--top 100]` -> `extract_notes_all.tsv` (1007 rows: new 143, agrees 554,
  conflicts 310) and `extract_notes_top100.tsv` (97). REA tool names (`procedure_pseudo_code`, `read_bytes`) and
  `invalid_parameter` are now rejected by `TOOL_NAME_RX`.
- Dry-run ledger: `ledger --in extract_notes_top100.tsv --topic ingest-notes --out ledger_dry_top100.tsv` -> 85 rows
  (79 function, 6 global). Nothing applied; no `rea_findings/*ingest-notes*` file exists.
- Cards: `cards --seed 1028` -> 106 cards (21 decoys, 20 of them function), 9 chunks.
- Pass 1 (`verdicts_[1-9].tsv`): decoys 8/21 = 38%, 3 decoys accepted. Real rows 52/85 consistent (61%, above the 30% stop rule).
- Retry chunks 1/2/4/5/6/8 (`verdicts_r2_*`, Class::Method calibration): 12/21 = 57% FAIL, 4 decoys accepted.
  Real rows: 56 consistent, 49 consistent in both passes.
- Cause: decoys are same-family `Class::Method` names (mostly CSWGui*), and the static card for a function shows
  mostly unnamed callees, so a blind reader cannot tell `ListBox::ClearItems` from `ListBox::AddControls`.
  I2 passed because its global decoys swapped roles (vtable vs IAT vs float), which the facts do show.
- Options: richer function cards (a 10-line `named_decomp.py` excerpt) and re-run; or a Sonnet layer (2 Haiku layers done);
  or park I3.

## Round 2 (richer cards) - PASS, 56 ledger rows
- `cards --decomp-lines 10 --seed 1029` -> `cards_v2/` (106 cards, 21 decoys): each function card adds a pseudo-C excerpt
  from the dump (placeholders renamed from KDB/Steam DB names, own name blinded as `this_fn`).
- One Haiku pass with the Class::Method calibration: decoys 17/21 = 81% PASS (function 17/20); 2 decoys accepted, both
  one-field getters (`C2DA::GetINTEntry`, `CSWMessage::ReadDWORD`): getter names stay weak. Real rows 61/85 consistent.
- Pairing bug found after apply: `NAME_THEN_ADDR`/`ADDR_THEN_NAME` spanned ", " so "0x0040ee40 A::B, 0x00411b20" paired
  A::B with the next address. Fixed (separator excludes `,;`) and `.claude/research/ingest/` is no longer read as notes.
  5 rows lost source support and were removed: 0x00411b20, 0x00416720, 0x00656af0 (from i2 cards), 0x00745560
  (08_shield_bar.md:152, name belongs to 0x0041bb20), 0x008d4f60 (31b_ch4c_style_re.md:25).
- Ledger: `rea_findings/20261027_ingest-notes.tsv` 56 rows (probable). Lint 84 -> 84. Notes gap 455 (ingest dir excluded).
- I1 re-check with the fixed regex: only `0x0098bfe0 TexMgr::Tick` (I2-rejected, not in name_map) loses support.
