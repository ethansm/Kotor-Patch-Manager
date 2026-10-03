# Test coverage of `steam_db_batches/tools`

To regenerate, from `tools/`:

```
../../../.venv/bin/python -m coverage run --branch -m unittest discover -s tests
../../../.venv/bin/python -m coverage report -m --omit='tests/*,*/re_card_prep.py'
```

coverage 7.16.2 is in the repo `.venv`; the system python3 does not have it.

## Baseline (2026-10-03, commit f3f1787, 151 tests)

| module | stmts | miss | branch | br-part | cover |
|---|---|---|---|---|---|
| batch.py | 1511 | 118 | 644 | 73 | 90% |
| bdlib.py | 803 | 60 | 292 | 31 | 91% |
| calibrate.py | 504 | 38 | 210 | 33 | 89% |
| cmdtable.py | 119 | 75 | 48 | 1 | 35% |
| ingest_server.py | 392 | 23 | 148 | 19 | 92% |
| score_review.py | 116 | 6 | 38 | 2 | 95% |
| branch_apply.py | - | - | - | - | 0% (no test imports it) |

`g8_study.py` and `recall_study.py` are one-off studies and are not covered.

## After the Phase 3 test expansion (2026-10-03, 257 tests, 0 expected failures)

New test files:
- `test_branch_apply.py` (31 tests);
- `test_cmdtable_main.py` (25);
- `test_batch_ledger_cards.py` (48);
- `test_opus_gate_fixes.py`, grown to 17.

| module | before | after |
|---|---|---|
| batch.py | 90% | 92% |
| bdlib.py | 91% | 91% |
| branch_apply.py | 0% | 99% |
| calibrate.py | 89% | 89% |
| cmdtable.py | 35% | 98% |
| ingest_server.py | 92% | 92% |
| score_review.py | 95% | 95% |
| **total** | 89% | 92% |

The new tests found these production bugs, all fixed:
- `branch_apply.py` ignored BD_ROOT / BD_BRANCHDB;
- its report counted parity rows that ship nothing;
- it left two files unclosed.

Known weak spots, recorded but not changed:
- an empty `fp_sha` mark never goes `stale` (`check-validated`);
- a decoy key error still counts toward the 10-decoy chunk minimum;
- `cmdtable.py`'s `ambiguous` match label is unreachable (one slot maps to one handler);
- a card prints an extra "Steam candidate fingerprint missing" line when the candidate has no fp row. This would
  leak a decoy only if real and decoy candidates differed in fp coverage.
