#!/usr/bin/env python3
"""npcvar_faction_matrix.py - standard-faction hostility table for the NPC-variety recon.

Reads repute.2da read-only via pykotor (Installation.resource), takes the
STANDARD_FACTION_* ids from the nwscript constants, and writes
.claude/research/npc_variety/tables/faction_matrix.tsv with one row per ordered
(faction_a, faction_b) pair of standard factions.

repute.2da layout: row index = faction_a id (row 0 is the player), value columns
are faction_b in order, so the column at 1-based position k is faction id k.

hostile = true when repute_value < 50. This is the tabulated rule only. If any
value falls outside 0-100 the hostile column is left blank instead.

Nothing is written into the game directory. Run
`.venv/bin/python scripts/npcvar_faction_matrix.py`. The pure helpers below are
imported by scripts/test_npcvar_faction_matrix.py.
"""
import argparse
import logging
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple

REPO_ROOT = Path(__file__).resolve().parent.parent
TABLE_DIR = REPO_ROOT / ".claude" / "research" / "npc_variety" / "tables"
OUT_TSV = TABLE_DIR / "faction_matrix.tsv"
NWSCRIPT = REPO_ROOT / ".claude" / "scratch_npcvar" / "nss_all" / "nwscript.nss"
DEFAULT_GAME_DIR = (
    Path.home() / ".local" / "share" / "Steam" / "steamapps" / "common" / "Knights of the Old Republic II"
)

PLAYER_FACTION_ID = 0  # repute.2da row 0; not a STANDARD_FACTION_ constant
COMMONER_FACTION_ID = 5  # STANDARD_FACTION_NEUTRAL in nwscript; the commoner pool faction
HOSTILE_BELOW = 50
REPUTE_MIN, REPUTE_MAX = 0, 100
COLUMNS = ["faction_a", "faction_b", "repute_value", "hostile"]
RULE_LINE = (
    "# rule: hostile = true when repute_value < 50 (tabulated rule only; "
    "hostile left blank when a value is outside 0-100)"
)

# Live (non-commented) `int STANDARD_FACTION_X = N;` lines only.
CONST_RE = re.compile(r"^\s*int\s+(STANDARD_FACTION_\w+)\s*=\s*(-?\d+)\s*;")


def parse_standard_factions(text: str) -> Dict[str, int]:
    """Map STANDARD_FACTION_* constant name -> id from live nwscript lines. First live line wins."""
    out: Dict[str, int] = {}
    for line in text.splitlines():
        m = CONST_RE.match(line)
        if m:
            out.setdefault(m.group(1), int(m.group(2)))
    return out


def build_matrix(rows: Dict[int, Dict[int, int]], ids: Set[int]) -> List[Dict[str, object]]:
    """One record per ordered pair (a, b) of ids present in rows, sorted by (a, b).

    rows = {faction_a: {faction_b: repute_value}}. hostile is None (blank) for every
    record when any repute value falls outside 0-100.
    """
    present = sorted(i for i in ids if i in rows)
    pairs: List[Tuple[int, int, int]] = []
    for a in present:
        for b in present:
            if b in rows[a]:
                pairs.append((a, b, rows[a][b]))
    scale_ok = all(REPUTE_MIN <= value <= REPUTE_MAX for _, _, value in pairs)
    records: List[Dict[str, object]] = []
    for a, b, value in pairs:
        records.append({
            "faction_a": a,
            "faction_b": b,
            "repute_value": value,
            "hostile": (value < HOSTILE_BELOW) if scale_ok else None,
        })
    return records


def count_hostile(records: List[Dict[str, object]]) -> int:
    return sum(1 for r in records if r["hostile"] is True)


def _tsv_cell(value: object) -> str:
    if value is None:
        return ""
    if isinstance(value, bool):
        return "true" if value else "false"
    return str(value)


def format_tsv(records: List[Dict[str, object]]) -> str:
    lines = [RULE_LINE, "\t".join(COLUMNS)]
    for rec in records:
        lines.append("\t".join(_tsv_cell(rec[c]) for c in COLUMNS))
    return "\n".join(lines) + "\n"


def _parse_cell(value: Optional[str]) -> Optional[int]:
    try:
        return int(str(value).strip())
    except (TypeError, ValueError):
        return None


def load_repute_rows(game_dir: Path) -> Tuple[Optional[Dict[int, Dict[int, int]]], str]:
    """Read repute.2da via pykotor, read-only. Returns ({a: {b: value}}, "") or (None, reason).

    Row labels are faction_a ids. Value columns (every header except `label`) map to
    faction_b ids by 1-based position.
    """
    if not (game_dir / "chitin.key").is_file():
        return None, f"no chitin.key under {game_dir}"
    try:
        from pykotor.extract.installation import Installation
        from pykotor.resource.formats.twoda import read_2da
        from pykotor.resource.type import ResourceType
    except ImportError as exc:
        return None, f"pykotor not importable ({exc})"

    logging.disable(logging.CRITICAL)
    try:
        resource = Installation(str(game_dir)).resource("repute", ResourceType.TwoDA)
        if resource is None:
            return None, "repute.2da not found in chitin or override"
        twoda = read_2da(resource.data)
        value_headers = [h for h in twoda.get_headers() if str(h).lower() != "label"]
        rows: Dict[int, Dict[int, int]] = {}
        for pos, label in enumerate(twoda.get_labels()):
            try:
                faction_a = int(str(label).strip())
            except ValueError:
                continue
            row = twoda.get_row(pos)
            cells: Dict[int, int] = {}
            for col_pos, header in enumerate(value_headers, start=1):
                value = _parse_cell(row.get_string(header))
                if value is not None:
                    cells[col_pos] = value
            rows[faction_a] = cells
        return rows, ""
    except Exception as exc:  # pykotor raises several exception types on a damaged install
        return None, f"repute.2da unreadable ({exc})"
    finally:
        logging.disable(logging.NOTSET)


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="Write faction_matrix.tsv from repute.2da and nwscript constants.")
    parser.add_argument("--game-dir", type=Path, default=DEFAULT_GAME_DIR)
    args = parser.parse_args(argv)

    try:
        standard = parse_standard_factions(NWSCRIPT.read_text(encoding="utf-8", errors="replace"))
    except OSError as exc:
        print(f"SKIP nwscript unreadable at {NWSCRIPT}: {exc}. No TSV written.")
        return 0
    if not standard:
        print(f"SKIP no STANDARD_FACTION_ constants in {NWSCRIPT}. No TSV written.")
        return 0
    ids = set(standard.values()) | {COMMONER_FACTION_ID, PLAYER_FACTION_ID}

    rows, reason = load_repute_rows(args.game_dir)
    if rows is None:
        print(f"SKIP repute.2da not loaded: {reason}. No TSV written.")
        return 0

    records = build_matrix(rows, ids)
    TABLE_DIR.mkdir(parents=True, exist_ok=True)
    OUT_TSV.write_text(format_tsv(records), encoding="utf-8")

    factions = len({r["faction_a"] for r in records})
    print(f"factions {factions} hostile_pairs {count_hostile(records)}")
    if any(r["hostile"] is None for r in records):
        print("WARN repute values outside 0-100; hostile column left blank")
    print(f"wrote {OUT_TSV.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
