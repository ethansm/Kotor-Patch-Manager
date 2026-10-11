#!/usr/bin/env python3
"""Scan override/ for 2DA files with a corrupted-blank-prefix fingerprint.

Why this shape and not a "% blank" threshold: many 2DA columns are blank for
most rows by design, so a "mostly blank" alarm would fire on healthy tables.
The corruption leaves a specific shape: rows 0..N-1 have a blank *first
column*, then real values resume at row N. The only known cause is pykotor's
Kaitai-based 2DA reader returning "" for every cell of a table it mis-parsed
(pykotor_patches/README.md, patch #5). A mod then appends its rows to that
blanked table and writes the result back to disk. No healthy file in this
build had that shape.

Only the first column is checked, not "every column blank", because of what
the real corrupted files looked like. In the corrupted appearance.2da from
this build, some wiped rows still held common shared values in a few columns
(a `walkdist`-style default, a shared texture name). An "all columns blank"
check stopped at row 91 when the real blank range ran to row 677. The
`label` column was blank for the whole range. The first column of shipped
KOTOR/TSL 2DAs is normally a per-row identifier (`label`, `head`,
`baseresref`, ...), so it's the column least likely to be blank by accident.

Run this after every batch of 2DA-touching mod installs, and definitely
before a first launch. Needs pykotor importable (run inside the same venv as
pk_install.py).

Usage: verify_2da.py [override_dir] [--min-blank-prefix N]
"""
import argparse
import glob
import logging
import os
import sys

logging.disable(logging.CRITICAL)  # pykotor logs at import time; keep this script's output clean

from pykotor.resource.formats.twoda import read_2da  # noqa: E402

# Override with KOTOR2_ROOT if the game isn't at the default Steam-on-Linux
# path (a different Steam library, GOG, a manual copy, etc).
_KOTOR2_ROOT = os.environ.get(
    "KOTOR2_ROOT",
    os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"),
)
DEFAULT_OVERRIDE = os.path.join(_KOTOR2_ROOT, "override")


def blank_prefix_length(twoda) -> int:
    """Length of the run of rows starting at row 0 whose *first column* is
    blank. Stops at the first row with a non-blank first-column value, or at
    the end of the table. See the module docstring for why the first column
    specifically, rather than requiring every column to be blank.
    """
    headers = twoda.get_headers()
    if not headers:
        return 0
    first_col = headers[0]
    n = 0
    for i in range(twoda.get_height()):
        if twoda.get_row(i).get_string(first_col) != "":
            break
        n += 1
    return n


def check_file(path: str, min_blank_prefix: int) -> dict | None:
    """Returns a report dict if this file matches the corruption fingerprint,
    else None.
    """
    try:
        twoda = read_2da(path)
    except Exception as exc:  # noqa: BLE001 -- a parse failure is itself worth reporting
        return {"path": path, "reason": f"failed to parse: {exc!r}"}

    total_rows = twoda.get_height()
    if total_rows == 0:
        return None

    prefix = blank_prefix_length(twoda)
    # Require at least one real row after the blank prefix. The corruption
    # always leaves the mod-appended rows intact at the end. A first column
    # that is blank all the way down is some other problem, or an unusual
    # but valid table, and flagging it would be noise.
    if prefix >= min_blank_prefix and prefix < total_rows:
        first_col = twoda.get_headers()[0]
        return {
            "path": path,
            "reason": f"rows 0-{prefix - 1} have a blank '{first_col}' column, "
            f"but row {prefix} onward has real '{first_col}' values ({total_rows - prefix} row(s))",
        }
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("override_dir", nargs="?", default=DEFAULT_OVERRIDE)
    parser.add_argument(
        "--min-blank-prefix",
        type=int,
        default=5,
        help="Minimum number of leading rows with a blank first column before a file is flagged "
        "(default: 5). This keeps a table with one or two unlabeled leading rows from being "
        "flagged; the real corruption blanked hundreds.",
    )
    args = parser.parse_args()

    paths = sorted(glob.glob(os.path.join(args.override_dir, "*.2da")))
    if not paths:
        print(f"No .2da files found in {args.override_dir}")
        return 0

    suspects = []
    for path in paths:
        report = check_file(path, args.min_blank_prefix)
        if report:
            suspects.append(report)

    print(f"Scanned {len(paths)} .2da file(s) in {args.override_dir}")
    if not suspects:
        print("No corrupted-blank-prefix files found.")
        return 0

    print(f"\n{len(suspects)} SUSPECT FILE(S):")
    for s in suspects:
        print(f"  {os.path.basename(s['path'])}: {s['reason']}")
    print(
        "\nSee pykotor_patches/README.md patch #5 for the root cause and how to "
        "rebuild an affected file from a TSLRCM/vanilla baseline."
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
