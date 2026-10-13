#!/usr/bin/env python3
"""npcvar_tables.py - NPC-variety recon tables built from the creature catalog.

Writes two TSVs under .claude/research/npc_variety/tables/:
  pool_candidates.tsv  chitin walker templates (faction 5, blank conv), with walk/run distances
  area_commoners.tsv   per deduped module: unique template counts all / loose / strict / hostile

Definitions follow u06.md (pool) and u08.md (area commoner counts).
Run `python3 scripts/npcvar_tables.py` to regenerate both files. The pure functions
below are imported by scripts/test_npcvar_tables.py.
"""
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CATALOG_DIR = REPO_ROOT / ".claude" / "research" / "creature_catalog"
TABLE_DIR = REPO_ROOT / ".claude" / "research" / "npc_variety" / "tables"
K2_UTC_TSV = CATALOG_DIR / "k2_utc.tsv"
APPEARANCES_TSV = CATALOG_DIR / "appearances.tsv"
POOL_OUT = TABLE_DIR / "pool_candidates.tsv"
AREA_OUT = TABLE_DIR / "area_commoners.tsv"

POOL_COLUMNS = ["resref", "group", "app_id", "walkdist", "rundist", "on_spawn"]
AREA_COLUMNS = ["module", "all", "loose", "strict", "hostile"]

# resref prefix -> group (u06.md: aliens, TSF/security, Czerka staff). Anything else is "other".
GROUP_PREFIXES = (
    ("n_rodian", "alien"), ("n_gand", "alien"), ("n_bith", "alien"), ("n_duros", "alien"),
    ("n_sullustan", "alien"), ("n_gran", "alien"), ("n_quarren", "alien"), ("n_weequay", "alien"),
    ("n_devaronian", "alien"), ("n_twilek", "alien"), ("n_wookie", "alien"), ("n_tusken", "alien"),
    ("n_walrus", "alien"), ("n_trando", "alien"), ("c_ithorian", "alien"), ("g_ithorian", "alien"),
    ("c_jawa", "alien"), ("g_jawa", "alien"), ("c_brith", "alien"),
    ("n_tsf", "tsf"), ("n_repsold", "tsf"), ("n_repoff", "tsf"),
    ("n_czerka", "czerka"),
)
GROUP_ORDER = {"alien": 0, "tsf": 1, "czerka": 2, "other": 3}

# u08.md definitions.
LOOSE_RE = re.compile(r"^(?:Commoner_|Onderon_Commoner_|Twilek_Commoner)")
HOSTILE_RE = re.compile(r"Sith|Dark_Jedi|Turret|Droid_Assassin|Soldier|Commando")

# Checked in order: "_s.rim" must come before ".rim".
MODULE_SUFFIXES = ("_s.rim", ".rim", ".mod")


def read_tsv(path):
    """Read a tab-separated file into dicts with stripped values (None -> '')."""
    with open(path, newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE)
        return [{k: (v or "").strip() for k, v in row.items() if k is not None} for row in reader]


def write_tsv(path, columns, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=columns, delimiter="\t", lineterminator="\n",
                                quoting=csv.QUOTE_NONE, extrasaction="raise")
        writer.writeheader()
        writer.writerows(rows)


def classify_group(resref):
    """Group from the resref prefix: alien | tsf | czerka | other."""
    name = resref.strip().lower()
    for prefix, group in GROUP_PREFIXES:
        if name.startswith(prefix):
            return group
    return "other"


def module_key(src):
    """Lowercase module stem: 'module:302NAR_S.RIM' -> '302nar'. Non-module src returns ''."""
    name = src.strip().lower()
    if not name.startswith("module:"):
        return ""
    name = name[len("module:"):]
    for suffix in MODULE_SUFFIXES:
        if name.endswith(suffix):
            return name[: -len(suffix)]
    return name


def dedupe_key(row):
    """(module stem, resref) with case and _s.rim/.rim/.mod variants collapsed."""
    return (module_key(row["src"]), row["resref"].strip().lower())


def is_loose(label):
    return LOOSE_RE.match(label) is not None


def is_strict(resref, label):
    name = resref.strip().lower()
    return is_loose(label) and name.startswith("n_") and not name.startswith("g_")


def is_hostile(label):
    return HOSTILE_RE.search(label) is not None


def pool_candidates(utc_rows, appearance_rows):
    """Chitin faction-5 rows with blank conv, joined to k2 walk/run distances by app_id."""
    appearances = {a["app_id"]: a for a in appearance_rows if a.get("game") == "k2"}
    out = []
    for r in utc_rows:
        if r["src"] != "chitin" or r["faction"] != "5" or r["conv"] != "":
            continue
        app = appearances.get(r["app_id"], {})
        out.append({
            "resref": r["resref"],
            "group": classify_group(r["resref"]),
            "app_id": r["app_id"],
            "walkdist": app.get("walkdist", ""),
            "rundist": app.get("rundist", ""),
            "on_spawn": r["on_spawn"],
        })
    return sorted(out, key=lambda row: (GROUP_ORDER[row["group"]], row["resref"].lower(), row["app_id"]))


def area_commoners(utc_rows):
    """Per deduped module: unique template counts all / loose / strict / hostile.

    Only modules with at least one loose commoner are returned (strict is a subset of loose).
    The first row in (src, resref, tag) order wins when duplicates disagree.
    """
    first_seen = {}
    for r in sorted(utc_rows, key=lambda r: (r["src"], r["resref"], r["tag"])):
        key = dedupe_key(r)
        if key[0]:
            first_seen.setdefault(key, r)

    counts = {}
    for (module, _), r in first_seen.items():
        c = counts.setdefault(module, {"all": 0, "loose": 0, "strict": 0, "hostile": 0})
        label = r["app_label"]
        c["all"] += 1
        if is_loose(label):
            c["loose"] += 1
        if is_strict(r["resref"], label):
            c["strict"] += 1
        if is_hostile(label):
            c["hostile"] += 1

    out = []
    for module in sorted(counts):
        c = counts[module]
        if c["loose"] == 0:
            continue
        out.append({"module": module, **{k: str(v) for k, v in c.items()}})
    return out


def main():
    utc_rows = read_tsv(K2_UTC_TSV)
    appearance_rows = read_tsv(APPEARANCES_TSV)
    pool = pool_candidates(utc_rows, appearance_rows)
    area = area_commoners(utc_rows)
    write_tsv(POOL_OUT, POOL_COLUMNS, pool)
    write_tsv(AREA_OUT, AREA_COLUMNS, area)
    print(f"pool_candidates: {len(pool)} rows -> {POOL_OUT}")
    print(f"area_commoners: {len(area)} modules -> {AREA_OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
