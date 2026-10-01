#!/usr/bin/env python3
"""Build the Override/aiscripts.2da of Mod 8 (ranged-ai): vanilla table + stance row 16.

aiscripts.2da maps the five vanilla party AI stances to their menu name strref and the
engine's ai state id. Mod 8 adds a sixth, "Ranged (Kiting)":

    5 | RangedKite | <name strref of "Ranged (Kiting)"> | 16

The vanilla table comes from 2DA.bif via chitin.key (the game has no override copy; if one
exists it is reported, because it would shadow what this script read). It must be exactly
the five vanilla rows or the script refuses: a mod that already edited the table would be
overwritten by a blind rebuild.

Output is written to a STAGING dir, never to the game: <out>/Override/aiscripts.2da as
binary 2DA V2.b. The script re-reads it, prints every row, and runs verify_2da.py on
<out>/Override. Copying it into the game's Override is a separate, user-gated step.

Stock pykotor 2.3.12 blanks some 2DAs; use the repo .venv (patched) only.

The name strref comes from append_ranged_kite_tlk.py (dialog.tlk entry N, read it from
ranged_ai_strrefs.json, don't assume 136538). Rebuild with the real value at install time.

Usage:
  build_ranged_kite_2da.py --name-strref 136538 --out ~/Games/KOTOR2_Mod_Workspace/stage_mod8s3_draft
  build_ranged_kite_2da.py --strrefs-json ~/Games/KOTOR2_Mod_Workspace/ranged_ai_strrefs.json --out DIR
Environment: KOTOR2_ROOT (see _common.py).
"""

import argparse
import json
import logging
from pathlib import Path
import subprocess
import sys

logging.disable(logging.CRITICAL)  # pykotor logs at import time; keep this script's output clean

from _common import GAME_DIR  # noqa: E402

try:
    from pykotor.extract.chitin import Chitin
    from pykotor.resource.formats.twoda import read_2da, write_2da
    from pykotor.resource.type import ResourceType
except ImportError:
    print("Error: pykotor must be installed in python environment (use the repo .venv).", file=sys.stderr)
    sys.exit(1)

HEADERS = ["label", "name_strref", "aistate"]
# (row label, label, name_strref, aistate) of the vanilla table, from the task spec / doc 13.
VANILLA_ROWS = [
    ("0", "Aggressive", "123703", "9"),
    ("1", "Defensive", "123704", "10"),
    ("2", "RangedSupport", "123705", "11"),
    ("3", "Stationary", "123706", "12"),
    ("4", "Support", "126013", "13"),
]
NEW_ROW = ("5", "RangedKite", None, "16")  # name_strref filled from the command line
VERIFY_2DA = Path(__file__).with_name("verify_2da.py")


def table_rows(twoda):
    """Every row as (row label, label, name_strref, aistate), all strings."""
    return [(twoda.get_label(i), *(twoda.get_row(i).get_string(h) for h in HEADERS))
            for i in range(twoda.get_height())]


def print_rows(title, twoda):
    print(f"{title}: columns {twoda.get_headers()}, {twoda.get_height()} rows")
    for row in table_rows(twoda):
        print("  " + " | ".join(row))


def read_vanilla(game_dir):
    """aiscripts.2da as stored in 2DA.bif."""
    for res in Chitin(game_dir / "chitin.key"):
        if res.resname().lower() == "aiscripts" and res.restype() == ResourceType.TwoDA:
            return read_2da(res.data())
    raise SystemExit(f"error: aiscripts.2da not found in {game_dir / 'chitin.key'}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--name-strref", type=int, help="dialog.tlk strref of the 'Ranged (Kiting)' name")
    ap.add_argument("--strrefs-json", type=Path,
                    help="ranged_ai_strrefs.json from append_ranged_kite_tlk.py (uses its 'name')")
    ap.add_argument("--out", type=Path, required=True, help="staging dir; writes <out>/Override/aiscripts.2da")
    args = ap.parse_args()

    if (args.name_strref is None) == (args.strrefs_json is None):
        ap.error("give exactly one of --name-strref and --strrefs-json")
    name_strref = args.name_strref if args.strrefs_json is None else json.loads(args.strrefs_json.read_text())["name"]

    live_copies = [f for d in (GAME_DIR / "Override", GAME_DIR / "override") if d.is_dir()
                   for f in d.iterdir() if f.name.lower() == "aiscripts.2da"]
    print(f"game Override aiscripts.2da: {'EXISTS at ' + str(live_copies[0]) if live_copies else 'does not exist'}"
          + ("  (the .bif copy is NOT what the game uses; re-check before installing)" if live_copies else ""))

    if live_copies:
        # Another mod may own that table: only proceed if it is still the vanilla rows (or vanilla + our row).
        live = read_2da(live_copies[0].read_bytes())
        rows = table_rows(live)
        ours = list(VANILLA_ROWS) + [tuple(str(x) for x in (NEW_ROW[0], NEW_ROW[1], name_strref, NEW_ROW[3]))]
        if (live.get_headers() == HEADERS and len(rows) == 6 and rows[:5] == list(VANILLA_ROWS)
                and rows[5][1] == NEW_ROW[1] and rows[5] != ours[5]):
            print(f"error: the live Override aiscripts.2da has our RangedKite row with a different name strref "
                  f"({rows[5][2]}, now {name_strref}); delete it and re-run.", file=sys.stderr)
            return 2
        if live.get_headers() != HEADERS or rows not in (list(VANILLA_ROWS), ours):
            print_rows("live Override aiscripts.2da", live)
            print("error: the live Override aiscripts.2da is not vanilla (or vanilla + our row); another mod owns it. "
                  "Merge by hand; refusing to build over it.", file=sys.stderr)
            return 2
        print("live Override aiscripts.2da is vanilla" + (" + our row" if rows == ours else "") + "; rebuilding it")

    twoda = read_vanilla(GAME_DIR)
    print_rows("vanilla aiscripts.2da (2DA.bif)", twoda)
    if twoda.get_headers() != HEADERS or table_rows(twoda) != list(VANILLA_ROWS):
        print("error: the table is not the five vanilla rows; refusing to build over it.", file=sys.stderr)
        print("  expected:", file=sys.stderr)
        for row in VANILLA_ROWS:
            print("    " + " | ".join(row), file=sys.stderr)
        return 2

    new_label, new_name, _, new_state = NEW_ROW
    index = twoda.add_row(new_label, {"label": new_name, "name_strref": str(name_strref), "aistate": new_state})
    assert index == 5, index

    out_file = args.out.expanduser() / "Override" / "aiscripts.2da"
    out_file.parent.mkdir(parents=True, exist_ok=True)
    write_2da(twoda, out_file, ResourceType.TwoDA)
    magic = out_file.read_bytes()[:8]
    print(f"\nwrote {out_file} ({out_file.stat().st_size} bytes, header {magic!r})")
    if magic != b"2DA V2.b":
        print("error: not a binary 2DA V2.b", file=sys.stderr)
        return 1

    reread = read_2da(out_file.read_bytes())
    print_rows("re-read", reread)
    expected = list(VANILLA_ROWS) + [(new_label, new_name, str(name_strref), new_state)]
    if reread.get_headers() != HEADERS or table_rows(reread) != expected:
        print("error: re-read table differs from what was built", file=sys.stderr)
        return 1

    print(f"\nverify_2da.py {out_file.parent}")
    result = subprocess.run([sys.executable, str(VERIFY_2DA), str(out_file.parent)])
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
