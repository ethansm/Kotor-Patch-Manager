#!/usr/bin/env python3
"""fix_wave.py - CLI for the variant-fix-wave skill (.claude/skills/variant-fix-wave/SKILL.md). Python 3, stdlib only.

Usage:
  fix_wave.py select --wave W [--top 10] [--dry-run]
  fix_wave.py draft-collect --wave W
  fix_wave.py plan <items.tsv> --wave W [--findings F --cols C] [--max-units N --max-agents N --max-cost X]
  fix_wave.py brief W --batch A [--dry-run] [--force]
  fix_wave.py status W-A [--mark ID cons|rev] [--revise ID]
  fix_wave.py apply W-A ID [--dry-run] [--commit SHA]
  fix_wave.py cost W-A [--session S ...]

select         pick the top ready work items (wi_model eligibility), claim them, write candidates.tsv and a Haiku draft wave.
draft-collect  merge the drafters' rows into items.input.tsv, write unfit.tsv, release unfit claims.
plan    pack item rows into batches under the caps in limits.toml; exit 2 = a group over a cap or a bad `after`.
brief   write one batch's packs, consolidator and reviewer briefs, and its Haiku units (wave_tools); exit 3 = rejected units.
status  one line per item: `id stage next-action`; first line = in-flight count and free slots.
apply   gate one apply-ready item (git apply, run_tests, build_check) and print its close command; never commits.
cost    sum the batch's agent cost by model and role (VW haiku / cons / rev); current session unless --session repeats.
Each subcommand takes --waves-root / --repo-root (tests use temp dirs); `<subcommand> --help` lists the rest.
Logic lives in fix_wave_select.py, fix_wave_plan.py, fix_wave_brief.py, fix_wave_collect.py and fix_wave_apply.py.
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import fix_wave_apply    # noqa: E402
import fix_wave_brief    # noqa: E402
import fix_wave_collect  # noqa: E402
import fix_wave_plan     # noqa: E402
import fix_wave_select   # noqa: E402

# subcommand -> (module main, whether that main expects the subcommand name as argv[0])
COMMANDS = {
    "select": (fix_wave_select.main, True),
    "draft-collect": (fix_wave_select.main, True),
    "plan": (fix_wave_plan.main, False),
    "brief": (fix_wave_brief.main, True),
    "status": (fix_wave_collect.main, True),
    "cost": (fix_wave_collect.main, True),
    "apply": (fix_wave_apply.main, True),
}


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if not argv or argv[0] in ("-h", "--help") or argv[0] not in COMMANDS:
        print(__doc__.strip())
        return 0 if argv and argv[0] in ("-h", "--help") else 2
    cmd, rest = argv[0], argv[1:]
    fn, named = COMMANDS[cmd]
    try:
        rc = fn([cmd] + rest if named else rest)
    except SystemExit as e:   # argparse --help / usage errors inside the module
        rc = e.code if isinstance(e.code, int) else (0 if e.code is None else 2)
    return rc or 0


if __name__ == "__main__":
    sys.exit(main())
