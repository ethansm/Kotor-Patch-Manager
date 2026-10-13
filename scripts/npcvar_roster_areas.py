#!/usr/bin/env python3
"""npcvar_roster_areas.py - pick roster areas for the NPC-variety work (stdlib only).

Joins area_commoners.tsv (module, strict) with area_geometry.tsv (module, n_waypoints,
walkable_faces) on `module` and writes .claude/research/npc_variety/tables/roster_areas.tsv.

Eligible iff strict >= 5 and n_waypoints >= 40 and walkable_faces >= 300 (hostile is not a gate).
spawn_budget = min(SPAWN_CAP, max(1, strict // 4)) for eligible areas, 0 otherwise.
SPAWN_CAP is a placeholder until a live probe measures capacity.
Run `python3 scripts/npcvar_roster_areas.py` (prints `eligible N`). The pure functions below
are imported by scripts/test_npcvar_roster_areas.py.
"""
import csv
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TABLE_DIR = REPO_ROOT / ".claude" / "research" / "npc_variety" / "tables"
COMMONERS_TSV = TABLE_DIR / "area_commoners.tsv"
GEOMETRY_TSV = TABLE_DIR / "area_geometry.tsv"
ROSTER_OUT = TABLE_DIR / "roster_areas.tsv"

ROSTER_COLUMNS = ["module", "strict", "n_waypoints", "walkable_faces", "eligible", "reason", "budget", "region"]

MIN_STRICT = 5
MIN_WAYPOINTS = 40
MIN_FACES = 300
SPAWN_CAP = 4  # placeholder until a live probe measures capacity

# Lookup only: no other per-module rules live here.
REGION = {
    "203tel": "telos", "204tel": "telos", "207tel": "telos",
    "301nar": "nar_shaddaa", "302nar": "nar_shaddaa",
    "682dan": "dantooine",
}


def area_eligible(strict, n_waypoints, walkable_faces):
    """Return (eligible, reason). Reason is 'ok' or the failing gates joined by '+' in gate order."""
    failed = []
    if strict < MIN_STRICT:
        failed.append(f"strict<{MIN_STRICT}")
    if n_waypoints < MIN_WAYPOINTS:
        failed.append(f"waypoints<{MIN_WAYPOINTS}")
    if walkable_faces < MIN_FACES:
        failed.append(f"faces<{MIN_FACES}")
    if failed:
        return False, "+".join(failed)
    return True, "ok"


def spawn_budget(strict, cap=SPAWN_CAP):
    """Pure formula: min(cap, max(1, strict // 4)). The CLI applies 0 to ineligible areas."""
    return min(cap, max(1, strict // 4))


def parse_tsv(lines):
    """Parse TSV lines (header first) into dicts. Quoting disabled so stray quote characters are literal."""
    return list(csv.DictReader(lines, delimiter="\t", quoting=csv.QUOTE_NONE))


def read_tsv(path):
    with open(path, encoding="utf-8", newline="") as f:
        return parse_tsv(f)


def build_roster(commoner_rows, geometry_rows):
    """Join on module (modules in only one input are skipped). Rows are sorted by module."""
    commoners = {r["module"].strip(): r for r in commoner_rows}
    geometry = {r["module"].strip(): r for r in geometry_rows}
    out = []
    for module in sorted(set(commoners) & set(geometry)):
        strict = int(commoners[module]["strict"])
        n_waypoints = int(geometry[module]["n_waypoints"])
        walkable_faces = int(geometry[module]["walkable_faces"])
        ok, reason = area_eligible(strict, n_waypoints, walkable_faces)
        out.append({
            "module": module,
            "strict": strict,
            "n_waypoints": n_waypoints,
            "walkable_faces": walkable_faces,
            "eligible": "true" if ok else "false",
            "reason": reason,
            "budget": spawn_budget(strict) if ok else 0,
            "region": REGION.get(module, ""),
        })
    return out


def write_roster(rows, path):
    with open(path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=ROSTER_COLUMNS, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main():
    rows = build_roster(read_tsv(COMMONERS_TSV), read_tsv(GEOMETRY_TSV))
    write_roster(rows, ROSTER_OUT)
    print(f"eligible {sum(r['eligible'] == 'true' for r in rows)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
