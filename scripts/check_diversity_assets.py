#!/usr/bin/env python3
"""Verify the NPC diversity generic-row tables and APP_* rows against override/appearance.2da.

Reads the four IsGeneric* case lists straight from DiversityTables.h (no second copy of the rows) and the APP_*
constants from DiversityRows.h, then checks them against the appearance.2da that the generator writes.

  tables  every `case N:` in IsGenericMaleCommoner / IsGenericFemaleCommoner / IsGenericTwilekMale / IsGenericTwilekFemale
          (a) row N exists in appearance.2da                                          -> FAIL
          (b) label starts with Commoner_Mal_ / Commoner_Fem_ (commoner tables) or
              Twilek_Commoner_M_ / Twilek_Commoner_F_ (Twi'lek tables)                 -> WARN (listed with actual labels)
          (c) modela starts with N_CommM (male tables) or N_CommF (female tables)     -> FAIL
              appearance.2da has no gender column; modela is the model-family gender marker (190 = N_CommM, 185 = N_CommF).
  rows    every APP_* constant in DiversityRows.h: row exists and modela is non-empty; rows 802-811 also need texa.
          Rows 812-815 are KNOWN-BROKEN (to be dropped by the generator): reported, and they fail the run unless
          --allow-known-broken is passed.

Exit 0 when nothing fails, 1 otherwise. Warnings never fail the run.

  .venv/bin/python scripts/check_diversity_assets.py
  .venv/bin/python scripts/check_diversity_assets.py --allow-known-broken
  .venv/bin/python scripts/check_diversity_assets.py --appearance /path/to/appearance.2da

Pure parsing (parse_tables, parse_app_constants, run_checks) needs no pykotor; only load_appearance does, and main()
re-execs itself with the repo .venv when pykotor is not importable.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple

REPO = Path(__file__).resolve().parent.parent
SRC = REPO / "patch_manager_mods" / "npc_diversity_src"
DEFAULT_APPEARANCE = SRC / "override" / "appearance.2da"
DEFAULT_TABLES_HEADER = SRC / "DiversityTables.h"
DEFAULT_ROWS_HEADER = SRC / "DiversityRows.h"

COLUMNS = ("label", "modela", "texa")
NULL_CELL = "****"

# IsGeneric* function -> (gender, kind)
TABLES: Dict[str, Tuple[str, str]] = {
    "IsGenericMaleCommoner": ("male", "commoner"),
    "IsGenericFemaleCommoner": ("female", "commoner"),
    "IsGenericTwilekMale": ("male", "twilek"),
    "IsGenericTwilekFemale": ("female", "twilek"),
}
LABEL_PREFIX: Dict[Tuple[str, str], str] = {
    ("male", "commoner"): "Commoner_Mal_",
    ("female", "commoner"): "Commoner_Fem_",
    ("male", "twilek"): "Twilek_Commoner_M_",
    ("female", "twilek"): "Twilek_Commoner_F_",
}
MODEL_PREFIX: Dict[str, str] = {"male": "N_CommM", "female": "N_CommF"}

KNOWN_BROKEN_APP_ROWS = frozenset({812, 813, 814, 815})
TEXA_REQUIRED_APP_ROWS = frozenset(range(802, 812))
MAX_FAIL_LINES = 8

_FUNC_RE = re.compile(r"inline\s+bool\s+(\w+)\s*\(\s*uint16_t\s+app\s*\)\s*\{")
_CASE_RE = re.compile(r"\bcase\s+(\d+)\s*:")
_APP_CONST_RE = re.compile(r"constexpr\s+uint(?:8|16|32)_t\s+(APP_\w+)\s*=\s*(\d+)\s*;")

Rows = Dict[int, Dict[str, str]]


def _strip_line_comments(text: str) -> str:
    return "\n".join(line.split("//", 1)[0] for line in text.splitlines())


def parse_tables(header_text: str) -> Dict[str, List[int]]:
    """Return {function name: [case rows in source order]} for each `inline bool X(uint16_t app)` in the header.

    A function body runs from its signature to the next signature (or end of text). Comments are stripped first, so a
    `case N:` inside a comment is ignored.
    """
    text = _strip_line_comments(header_text)
    starts = list(_FUNC_RE.finditer(text))
    tables: Dict[str, List[int]] = {}
    for i, m in enumerate(starts):
        end = starts[i + 1].start() if i + 1 < len(starts) else len(text)
        tables[m.group(1)] = [int(n) for n in _CASE_RE.findall(text[m.end():end])]
    return tables


def parse_app_constants(rows_text: str) -> Dict[str, int]:
    """Return {APP_* name: row index} for each `constexpr uintN_t APP_X = N;` in DiversityRows.h."""
    return {m.group(1): int(m.group(2)) for m in _APP_CONST_RE.finditer(_strip_line_comments(rows_text))}


def _cell(value: Optional[str]) -> str:
    v = (value or "").strip()
    return "" if v == NULL_CELL else v


def load_appearance(path: Path) -> Rows:
    """Return {row index: {label, modela, texa}} from a 2DA file. Needs pykotor."""
    import logging

    from pykotor.resource.formats.twoda import read_2da

    logging.disable(logging.CRITICAL)
    twoda = read_2da(str(path))
    rows: Rows = {}
    for i in range(twoda.get_height()):
        row = twoda.get_row(i)
        rows[i] = {col: _cell(row.get_string(col)) for col in COLUMNS}
    return rows


def _rel(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def run_checks(tables_text: str, rows_text: str, rows: Rows, allow_known_broken: bool = False) -> Tuple[int, List[str]]:
    """Run every check on in-memory inputs. Returns (exit code, output lines)."""
    fails: List[str] = []
    warns: List[str] = []
    table_lines = [f"{'TABLE':<26}{'ROWS':>5}{'LABEL-WARN':>12}{'FAIL':>6}"]
    parsed = parse_tables(tables_text)

    for name, (gender, kind) in TABLES.items():
        ids = parsed.get(name, [])
        if not ids:
            fails.append(f"{name}: no `case N:` rows parsed from DiversityTables.h")
            table_lines.append(f"{name:<26}{0:>5}{'-':>12}{1:>6}")
            continue
        label_prefix = LABEL_PREFIX[(gender, kind)]
        model_prefix = MODEL_PREFIX[gender]
        label_mismatch: List[str] = []
        n_fail = 0
        for n in ids:
            row = rows.get(n)
            if row is None:
                fails.append(f"{name}: row {n} missing from appearance.2da")
                n_fail += 1
                continue
            if not row["label"].startswith(label_prefix):
                label_mismatch.append(f"{n}={row['label']}")
            if not row["modela"].startswith(model_prefix):
                fails.append(f"{name}: row {n} ({row['label']}) modela={row['modela']!r}, expected {model_prefix}*")
                n_fail += 1
        table_lines.append(f"{name:<26}{len(ids):>5}{len(label_mismatch):>12}{n_fail:>6}")
        if label_mismatch:
            warns.append(f"WARN {name}: label not {label_prefix}*: " + ", ".join(label_mismatch))

    consts = parse_app_constants(rows_text)
    known: List[str] = []
    app_ok = 0
    app_fail = 0
    if not consts:
        fails.append("no APP_* constants parsed from DiversityRows.h")
    for name, n in sorted(consts.items(), key=lambda kv: (kv[1], kv[0])):
        if n in KNOWN_BROKEN_APP_ROWS:
            known.append(f"{name}={n}")
            continue
        row = rows.get(n)
        if row is None:
            fails.append(f"{name}: row {n} missing from appearance.2da")
            app_fail += 1
            continue
        problems = []
        if not row["modela"]:
            problems.append("empty modela")
        if n in TEXA_REQUIRED_APP_ROWS and not row["texa"]:
            problems.append("empty texa")
        if problems:
            fails.append(f"{name}: row {n} has {' and '.join(problems)}")
            app_fail += 1
        else:
            app_ok += 1

    out = list(table_lines)
    out.append(
        f"APP_* rows: {len(consts)} parsed, {app_ok} OK, {app_fail} FAIL, {len(known)} KNOWN-BROKEN"
    )
    if known:
        suffix = "  [accepted: --allow-known-broken]" if allow_known_broken else ""
        out.append("KNOWN-BROKEN (to be dropped by generator): " + ", ".join(known) + suffix)
    out.extend(warns)
    for line in fails[:MAX_FAIL_LINES]:
        out.append("FAIL " + line)
    if len(fails) > MAX_FAIL_LINES:
        out.append(f"... and {len(fails) - MAX_FAIL_LINES} more FAIL lines")

    problems = len(fails) + (0 if allow_known_broken else len(known))
    out.append("RESULT: PASS" if problems == 0 else f"RESULT: FAIL ({problems} problem(s))")
    return (0 if problems == 0 else 1), out


def _ensure_pykotor() -> None:
    try:
        import pykotor  # noqa: F401
        return
    except ImportError:
        pass
    venv_py = REPO / ".venv" / "bin" / "python"
    if venv_py.exists() and not os.environ.get("_CDA_REEXEC"):
        os.environ["_CDA_REEXEC"] = "1"
        os.execv(str(venv_py), [str(venv_py), __file__] + sys.argv[1:])
    print("Error: pykotor must be importable; run with the repo venv: .venv/bin/python scripts/check_diversity_assets.py",
          file=sys.stderr)
    sys.exit(1)


def main(argv: Optional[Sequence[str]] = None) -> int:
    ap = argparse.ArgumentParser(description="Check NPC diversity generic-row tables and APP_* rows against appearance.2da.")
    ap.add_argument("--appearance", type=Path, default=DEFAULT_APPEARANCE, help="appearance.2da to check against")
    ap.add_argument("--tables-header", type=Path, default=DEFAULT_TABLES_HEADER, help="DiversityTables.h")
    ap.add_argument("--rows-header", type=Path, default=DEFAULT_ROWS_HEADER, help="DiversityRows.h")
    ap.add_argument("--allow-known-broken", action="store_true",
                    help="KNOWN-BROKEN APP rows (812-815) are reported but do not fail the run")
    args = ap.parse_args(argv)

    for p in (args.appearance, args.tables_header, args.rows_header):
        if not p.is_file():
            print(f"Error: not found: {p}", file=sys.stderr)
            return 1

    _ensure_pykotor()
    rows = load_appearance(args.appearance)
    code, lines = run_checks(args.tables_header.read_text(encoding="utf-8"),
                             args.rows_header.read_text(encoding="utf-8"),
                             rows, args.allow_known_broken)
    print(f"check_diversity_assets: {_rel(args.appearance)} ({len(rows)} rows)")
    print("\n".join(lines))
    return code


if __name__ == "__main__":
    sys.exit(main())
