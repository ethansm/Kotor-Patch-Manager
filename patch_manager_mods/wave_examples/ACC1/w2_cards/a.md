### Task a: wave_budget check --json
- **Contract**: worker
- **Write Scope**: `scripts/wave_budget.py`, `scripts/test_wave_budget.py`
- **Target Files**: `scripts/wave_budget.py`, `scripts/test_wave_budget.py`
- **Objective**: Add `--json` to the `check` subcommand only, so `python3 scripts/wave_budget.py check <entry>... --json` prints one JSON object built from the check() dict (draft keys: `total`, `per_entry`, `ok`, `verdict`) instead of the text line. Text output stays byte-identical. Exit codes stay 0 (ok), 1 (over), 2 (usage or error). Add one CliTests method for the JSON path.
  ```
  if args.json:
      print(json.dumps({"total": res["total"], "per_entry": res["per_entry"], "ok": res["ok"], "verdict": verdict}))
  else:
      print(f"total={res['total']} per_entry={res['per_entry']} verdict={verdict}")
  ```
- **Data / Inlined Snippets** (anchors re-checked with `rg -n`; wave_budget.py did not shift):
  ```
  scripts/wave_budget.py:20    import io          (insert "import json" after this line)
  scripts/wave_budget.py:131   p_check.add_argument("--brief-bytes", type=int, default=0, metavar="N")   (insert --json line after)
  scripts/wave_budget.py:134   args = parser.parse_args(argv)
  scripts/wave_budget.py:166   verdict = "OK" if res["ok"] else "OVER (" + "; ".join(reasons) + ")"
  scripts/wave_budget.py:167   print(f"total={res['total']} per_entry={res['per_entry']} verdict={verdict}")
  scripts/wave_budget.py:168   return 0 if res["ok"] else 1
  scripts/test_wave_budget.py:3    import os      (no json import in this file: add "import json" above it)
  scripts/test_wave_budget.py:233  self.assertIn("verdict=OVER", r.stdout)
  ```
- **Guardrails**:
  - `nargs="+"` is greedy: tests and docs put `--json` after the entries (`check a.md --json`); `check a.md --json b.md` fails.
  - Add `--json` to `p_check` (after line 131), not the parent parser, so windows mode does not silently accept it.
  - Keep the JSON branch above `return 0 if res["ok"] else 1` (line 168); that line stays last.
  - Under `--json`, error paths (stderr, exit 2) keep stdout empty.
  - `from fractions import Fraction` (line 25): if `per_entry` holds a Fraction, `json.dumps` raises TypeError; convert first. The check() body (`scripts/wave_budget.py:26-125`) is unread: confirm its keys and value types before writing the branch.
  - Add `import json` to `scripts/test_wave_budget.py` before the test uses it.
  - Insert the new test as the last method of `CliTests` (last existing method: `test_windows_prints_one_per_line`, line 260; file is 268 lines). Check for a trailing `if __name__` block first.
  - Edit only the Write Scope files. No other repo source.
- **Verification**: `python3 scripts/run_tests.py py:scripts/test_wave_budget.py`
