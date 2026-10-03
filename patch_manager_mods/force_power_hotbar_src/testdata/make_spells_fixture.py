#!/usr/bin/env python3
"""Generate testdata/spells_fixture.2da (binary 2DA V2.b) for twoda_test.cpp.

Run with the repo venv:  .venv/bin/python patch_manager_mods/force_power_hotbar_src/testdata/make_spells_fixture.py

43 rows (labels "0".."42", like the real spells.2da). Rows 8, 10, 22, 42 carry real
cells copied from vanilla spells.2da (category is the hex string, e.g. "0x1808");
every other row is synthetic filler. The file is read back and verified; exit 1 on mismatch.
"""
import logging
import sys
from pathlib import Path

logging.disable(logging.CRITICAL)

from pykotor.resource.formats.twoda import TwoDA, read_2da, write_2da  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

HEADERS = ["label", "name", "forcepoints", "range", "category", "iconresref", "spelldesc"]
NROWS = 43
REAL = {
    8: ["FORCE_POWER_SPEED_BURST", "267", "25", "P", "0x1808", "ip_speedburst", "1250"],
    10: ["FORCE_POWER_CURE", "287", "30", "T", "0x2441", "ip_cure", "136478"],
    22: ["FORCE_POWER_FORCE_VALOR", "281", "25", "P", "0x1808", "ip_mind", "1222"],
    42: ["FORCE_POWER_RESIST_ENERGY_1", "301", "20", "P", "0x1808", "ip_resistenergy1", "1240"],
}


def expected_row(i):
    return REAL.get(i, [f"FILLER_{i}", "****", "****", "****", "****", "****", "****"])


def main():
    out = Path(__file__).resolve().parent / "spells_fixture.2da"
    t = TwoDA(HEADERS)
    for i in range(NROWS):
        t.add_row(str(i), dict(zip(HEADERS, expected_row(i))))
    write_2da(t, out, ResourceType.TwoDA)

    data = out.read_bytes()
    if data[:8] != b"2DA V2.b":
        print(f"error: bad magic {data[:8]!r}", file=sys.stderr)
        return 1
    r = read_2da(data)
    bad = 0
    if r.get_headers() != HEADERS or r.get_height() != NROWS:
        print(f"error: headers/height mismatch {r.get_headers()} {r.get_height()}", file=sys.stderr)
        bad += 1
    for i in range(NROWS):
        if r.get_label(i) != str(i):
            print(f"error: row {i} label {r.get_label(i)!r}", file=sys.stderr)
            bad += 1
        for col, want in zip(HEADERS, expected_row(i)):
            got = r.get_cell(i, col)
            if got != want:
                print(f"error: row {i} col {col}: got {got!r} want {want!r}", file=sys.stderr)
                bad += 1
    if bad:
        return 1
    print(f"wrote {out} ({len(data)} bytes, {NROWS} rows, verified)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
