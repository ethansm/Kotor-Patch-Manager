### Task b: wave_tools list --json
- **Contract**: worker
- **Write Scope**: `scripts/wave_tools.py`, `scripts/test_wave_tools.py`
- **Target Files**: `scripts/wave_tools.py:273-281`, `scripts/wave_tools.py:508-511`, `scripts/test_wave_tools.py:196-210`
- **Objective**: Add `--json` to the `list` subcommand so `python3 scripts/wave_tools.py list <wave_id> --json` prints the wave's units as a JSON array (id, est_tokens, description). Default text output stays byte-identical. Add a test in the same class, directly after `test_list_prints_one_line_per_unit`. Also add `[--json]` to the usage line in the module docstring at `scripts/wave_tools.py:7`.
- **Data / Inlined Snippets** (anchors re-checked with `rg -n` on the current file; the argparse block is at 508, not the +6 shift the brief assumed):
  ```
  scripts/wave_tools.py:276      units = manifest.get("units", [])
  scripts/wave_tools.py:277      if not units:
  scripts/wave_tools.py:278          print("(no units)")
  scripts/wave_tools.py:508      p_list = sub.add_parser("list", help="list the units of a wave")
  scripts/wave_tools.py:509      p_list.add_argument("wave_id")
  scripts/wave_tools.py:510      _common(p_list)
  scripts/wave_tools.py:515      p_check.add_argument("--json", action="store_true", help="machine-readable output")
  scripts/test_wave_tools.py:196  def test_list_prints_one_line_per_unit(self):
  scripts/test_wave_tools.py:210          self.assertEqual(self.cli("list", "missing")[0], 1)
  ```
- **Guardrails**:
  - Add `p_list.add_argument("--json", action="store_true", help="machine-readable output")` after line 509 and before `_common(p_list)` at line 510. `p_list` only. `_common` (`scripts/wave_tools.py:483-485`) defines only `--repo-root`, so there is no clash. Leave the check flag at line 515 unchanged.
  - Insert the JSON branch after line 276 (the `units = ...` line) and before `if not units:` at line 277. Branch: `if a.json:` then `print(json.dumps(units))` then `return 0`. An empty wave prints `[]`. Never print `(no units)` to stdout under `--json`. Do not insert after line 275, because `units` is assigned on line 276.
  - Dump unit dicts as stored, with no reshaping. If units carry keys beyond id, est_tokens and description, keep them; the test compares against the stored units.
  - Default text output (lines 277-281) must not change.
  - `import json` already exists (`scripts/wave_tools.py:33`, `scripts/test_wave_tools.py:5`). Add no imports.
  - Module docstring line 7: change `  wave_tools.py list <wave_id> [--repo-root DIR]` to `  wave_tools.py list <wave_id> [--json] [--repo-root DIR]`.
  - Missing wave: `WaveError` goes to stderr with exit 1 and stdout stays empty. Do not add a JSON error body.
  - Do not touch `cmd_check` (`scripts/wave_tools.py:394`, its own `if a.json:` at line 404) or `patch_manager_mods/SUBAGENT_ROLES.md` (task c).
  - New test, inserted after line 210 in the same class, named `test_list_json_prints_units_array`: `code, out, _ = self.cli("list", "r1", "--json")`, then assert `code == 0` and `json.loads(out) == self.manifest("r1")["units"]`.
- **Verification**: `python3 scripts/run_tests.py test_wave_tools`
