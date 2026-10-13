#!/usr/bin/env python3
"""npcvar_roster_pool.py - NPC-variety roster pool: speed class and eligibility.

Joins pool_candidates.tsv (walker templates) with pool_armed.tsv (weapon-slot flags)
and writes roster_pool.tsv under .claude/research/npc_variety/tables/.

Eligible iff resref starts with "n_" AND group is alien/tsf/czerka AND the weapon
slot is not filled. "Armed" means weapon_slot_filled == true only (n_items is ignored).
A resref missing from pool_armed.tsv counts as armed=False.

Run `python3 scripts/npcvar_roster_pool.py` to regenerate. The pure functions below
are imported by scripts/test_npcvar_roster_pool.py.
"""
import csv
from pathlib import Path

from npcvar_tables import GROUP_ORDER, TABLE_DIR

POOL_IN = TABLE_DIR / "pool_candidates.tsv"
ARMED_IN = TABLE_DIR / "pool_armed.tsv"
ROSTER_OUT = TABLE_DIR / "roster_pool.tsv"

ROSTER_COLUMNS = ["resref", "group", "app_id", "walkdist", "rundist", "speed_class", "eligible", "reason"]
ELIGIBLE_GROUPS = ("alien", "tsf", "czerka")
SLOW_BELOW = 1.5   # walkdist < 1.5 -> slow
FAST_ABOVE = 2.2   # walkdist > 2.2 -> fast; 1.5..2.2 inclusive -> normal


def speed_class(walkdist: float) -> str:
    """Classify a walk distance as slow, normal or fast."""
    if walkdist < SLOW_BELOW:
        return "slow"
    if walkdist > FAST_ABOVE:
        return "fast"
    return "normal"


def eligibility(row: dict, armed: bool) -> tuple[bool, str]:
    """Return (eligible, reason). Checks run in order; the first failing check gives the reason."""
    if not row["resref"].startswith("n_"):
        return False, "non-n_ prefix"
    if row["group"] not in ELIGIBLE_GROUPS:
        return False, "group other"
    if armed:
        return False, "weapon slot filled"
    return True, "ok"


def load_pool(path: Path) -> list[dict]:
    """Read pool_candidates.tsv rows as dicts."""
    with path.open(newline="", encoding="utf-8") as fh:
        return list(csv.DictReader(fh, delimiter="\t"))


def load_armed(path: Path) -> dict[str, bool]:
    """Map resref -> True only when weapon_slot_filled is 'true'."""
    with path.open(newline="", encoding="utf-8") as fh:
        return {
            r["resref"]: r["weapon_slot_filled"].strip().lower() == "true"
            for r in csv.DictReader(fh, delimiter="\t")
        }


def build_rows(pool_rows: list[dict], armed_map: dict[str, bool]) -> list[dict]:
    """Build roster rows sorted by group order, then resref."""
    out = []
    for row in pool_rows:
        armed = armed_map.get(row["resref"], False)
        eligible, reason = eligibility(row, armed)
        out.append({
            "resref": row["resref"],
            "group": row["group"],
            "app_id": row["app_id"],
            "walkdist": row["walkdist"],
            "rundist": row["rundist"],
            "speed_class": speed_class(float(row["walkdist"])),
            "eligible": "true" if eligible else "false",
            "reason": reason,
        })
    out.sort(key=lambda r: (GROUP_ORDER.get(r["group"], GROUP_ORDER["other"]), r["resref"]))
    return out


def write_roster(rows: list[dict], path: Path) -> None:
    """Write roster rows as a TSV with ROSTER_COLUMNS."""
    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=ROSTER_COLUMNS, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    rows = build_rows(load_pool(POOL_IN), load_armed(ARMED_IN))
    write_roster(rows, ROSTER_OUT)
    eligible_count = sum(1 for r in rows if r["eligible"] == "true")
    print(f"wrote {ROSTER_OUT} rows={len(rows)}")
    print(f"eligible {eligible_count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
