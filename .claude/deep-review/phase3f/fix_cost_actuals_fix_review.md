# Fix / review cost actuals (USD per call)

Source: `cost_usd` field in `.claude/review/fix/results/*.json` (fixer) and `.claude/review/fix/reviews/*.json` (review). Class joined from `.claude/review/work/fix/ladder_ws/fix_queue.json` (`tasks[].class`) by `task_id`.

| Group | n | mean | p90 | sum |
|---|---|---|---|---|
| Fixer, all | 309 | 0.0084 | 0.0086 | 2.6067 |
| Fixer, auto-fixable | 104 | 0.0035 | 0.0053 | 0.3636 |
| Fixer, needs-design | 201 | 0.0111 | 0.0475 | 2.2306 |
| Fixer, needs-review | 4 | 0.0031 | 0.0046 | 0.0124 |
| Review, all | 194 | 0.0030 | 0.0047 | 0.5795 |
| Review, auto-fixable | 77 | 0.0026 | 0.0036 | 0.2029 |
| Review, needs-design | 106 | 0.0033 | 0.0055 | 0.3446 |
| Review, needs-review | 3 | 0.0025 | 0.0031 | 0.0074 |
| Review, unjoined task_id | 8 | 0.0031 | 0.0033 | 0.0246 |

Notes: no null `cost_usd` values. Fixer total 2.6067 USD; review total 0.5795 USD. Fixer cost is dominated by needs-design (2.2306 of 2.6067).
