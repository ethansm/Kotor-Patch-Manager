# Fix cost actuals (read-only recon, 2026-10-10)

## A. Integration: .claude/review/fix/integ/batch_*.run.json, field `cost_usd`
- 17 batches, all with cost_usd. Model claude-sonnet-5-5.
- mean $0.1469, p90 $0.2030, max $0.7374, total $2.4974.

## B. Judge: .claude/review/fix/judge/*.json, field `cost_usd` (kind=judge)
- 23 judge calls with cost_usd: mean $0.0027, max $0.0056, total $0.0630.
- 10 ladder/pick files (keys auto, parent, pick_vtid) have NO cost field. No raw/judge_* dir exists.
- calib/ (4 files, cost_usd present) excluded from totals above.

## C. P2c map: ~/.cache/deep_haiku_review/kotor2-p2/p2c/runs/*.json, field `cost_usd`
- 39 packets, all status ok: mean $0.0296, p90 $0.0411, max $0.1490, total $1.1528.
- state.json map stage: $1.1375, status failed (rc 1). dr.py status shows the same.
- p90 = sorted value at index round(0.9*(n-1)).
