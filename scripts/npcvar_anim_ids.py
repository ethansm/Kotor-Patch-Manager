#!/usr/bin/env python3
"""npcvar_anim_ids.py - animations.2da id -> name table for the NPC-variety V1 recon.

Joins the nwscript ANIMATION_* constants (value -> constant name) with the real
animations.2da rows (model name plus fireforget/looping/stationary flags) and
writes .claude/research/npc_variety/tables/anim_ids.tsv for ids 0-46 and 100-110.

animations.2da is read read-only from the game's chitin BIFs via pykotor
(Installation.resource). Nothing is written into the game directory.
Run `.venv/bin/python scripts/npcvar_anim_ids.py`. The pure helpers below are
imported by scripts/test_npcvar_anim_ids.py.
"""
import argparse
import logging
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple

REPO_ROOT = Path(__file__).resolve().parent.parent
TABLE_DIR = REPO_ROOT / ".claude" / "research" / "npc_variety" / "tables"
OUT_TSV = TABLE_DIR / "anim_ids.tsv"
NWSCRIPT = REPO_ROOT / ".claude" / "scratch_npcvar" / "nss_all" / "nwscript.nss"
DEFAULT_GAME_DIR = (
    Path.home() / ".local" / "share" / "Steam" / "steamapps" / "common" / "Knights of the Old Republic II"
)

SELECTED_IDS = tuple(range(0, 47)) + tuple(range(100, 111))
COLUMNS = ["id", "name", "nwscript_constant", "fireforget", "looping", "stationary", "v1_claim_ok"]

# Live (non-commented) `int ANIMATION_X = N;` lines only. A leading `//` breaks the match.
CONST_RE = re.compile(r"^\s*int\s+(ANIMATION_\w+)\s*=\s*(-?\d+)\s*;")

# V1 claims to check against the real 2da name. mode: "eq" = exact name,
# "in" = name contains any token, "startswith" = name starts with a token.
V1_CLAIMS: Dict[int, Tuple[str, Tuple[str, ...]]] = {
    1: ("eq", ("pause2",)),
    24: ("eq", ("pause3",)),
    100: ("in", ("turn", "head")),
    101: ("in", ("turn", "head")),
    102: ("in", ("pausesh", "scratch")),
    103: ("in", ("bored", "pause")),
    104: ("eq", ("salute",)),
    105: ("eq", ("bow",)),
    106: ("eq", ("greeting",)),
    107: ("eq", ("taunt",)),
    108: ("startswith", ("victory",)),
}

NULL_CELL = "****"


def parse_nwscript_constants(text: str) -> Dict[int, str]:
    """Map value -> ANIMATION_* constant name from live lines. First live line wins on a shared value."""
    out: Dict[int, str] = {}
    for line in text.splitlines():
        m = CONST_RE.match(line)
        if m:
            out.setdefault(int(m.group(2)), m.group(1))
    return out


def claim_ok(anim_id: int, name: Optional[str]) -> Optional[bool]:
    """True/False when V1 claims this id, None otherwise. A missing name fails the claim."""
    claim = V1_CLAIMS.get(anim_id)
    if claim is None:
        return None
    mode, tokens = claim
    low = (name or "").strip().lower()
    if not low:
        return False
    if mode == "eq":
        return low in tokens
    if mode == "startswith":
        return any(low.startswith(t) for t in tokens)
    return any(t in low for t in tokens)


def join(constants: Dict[int, str], rows: Dict[int, Dict[str, object]]) -> List[Dict[str, object]]:
    """Build one record per row id, sorted by id. rows = {id: {name, fireforget, looping, stationary}}."""
    records: List[Dict[str, object]] = []
    for anim_id in sorted(rows):
        row = rows[anim_id]
        name = str(row.get("name") or "")
        records.append({
            "id": anim_id,
            "name": name,
            "nwscript_constant": constants.get(anim_id, ""),
            "fireforget": row.get("fireforget"),
            "looping": row.get("looping"),
            "stationary": row.get("stationary"),
            "v1_claim_ok": claim_ok(anim_id, name),
        })
    return records


def count_mismatches(records: List[Dict[str, object]]) -> int:
    return sum(1 for r in records if r["v1_claim_ok"] is False)


def _tsv_cell(value: object) -> str:
    if value is None:
        return ""
    if isinstance(value, bool):
        return "true" if value else "false"
    return str(value)


def format_tsv(records: List[Dict[str, object]]) -> str:
    lines = ["\t".join(COLUMNS)]
    for rec in records:
        lines.append("\t".join(_tsv_cell(rec[c]) for c in COLUMNS))
    return "\n".join(lines) + "\n"


def _clean(value: Optional[str]) -> str:
    v = (value or "").strip()
    return "" if v == NULL_CELL else v


def parse_flag(value: Optional[str]) -> Optional[bool]:
    v = (value or "").strip()
    if v == "1":
        return True
    if v == "0":
        return False
    return None


def load_animation_rows(game_dir: Path) -> Tuple[Optional[Dict[int, Dict[str, object]]], str]:
    """Read SELECTED_IDS rows of animations.2da via pykotor, read-only.

    Returns (rows, "") on success or (None, reason) when the data cannot be read.
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
        resource = Installation(str(game_dir)).resource("animations", ResourceType.TwoDA)
        if resource is None:
            return None, "animations.2da not found in chitin or override"
        twoda = read_2da(resource.data)
        wanted = set(SELECTED_IDS)
        rows: Dict[int, Dict[str, object]] = {}
        for pos, label in enumerate(twoda.get_labels()):
            try:
                anim_id = int(str(label).strip())
            except ValueError:
                continue
            if anim_id not in wanted:
                continue
            row = twoda.get_row(pos)
            rows[anim_id] = {
                "name": _clean(row.get_string("name")),
                "fireforget": parse_flag(row.get_string("fireforget")),
                "looping": parse_flag(row.get_string("looping")),
                "stationary": parse_flag(row.get_string("stationary")),
            }
        return rows, ""
    except Exception as exc:  # pykotor raises several exception types on a damaged install
        return None, f"animations.2da unreadable ({exc})"
    finally:
        logging.disable(logging.NOTSET)


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="Write anim_ids.tsv from animations.2da and nwscript constants.")
    parser.add_argument("--game-dir", type=Path, default=DEFAULT_GAME_DIR)
    args = parser.parse_args(argv)

    constants: Dict[int, str] = {}
    try:
        constants = parse_nwscript_constants(NWSCRIPT.read_text(encoding="utf-8", errors="replace"))
    except OSError as exc:
        print(f"WARN nwscript constants unreadable at {NWSCRIPT}: {exc}", file=sys.stderr)

    rows, reason = load_animation_rows(args.game_dir)
    if rows is None:
        print(f"SKIP animations.2da not loaded: {reason}. No TSV written.")
        return 0

    records = join(constants, rows)
    TABLE_DIR.mkdir(parents=True, exist_ok=True)
    OUT_TSV.write_text(format_tsv(records), encoding="utf-8")

    for rec in records:
        if rec["v1_claim_ok"] is False:
            _, tokens = V1_CLAIMS[int(rec["id"])]
            print(f"mismatch id={rec['id']} name={rec['name']} claim={'|'.join(tokens)}")
    print(f"rows {len(records)} mismatches {count_mismatches(records)}")
    print(f"wrote {OUT_TSV.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
