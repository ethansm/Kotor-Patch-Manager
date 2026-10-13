#!/usr/bin/env python3
"""npcvar_anim_pool.py - per-body animation pool table from the u03b target-row inventory.

Reads .claude/research/npc_variety/u03b_target_rows.txt (per-row blocks:
`row N Name type=.. model=M chain=[a > b > c] n_anims=K`, then an `idles:` line and a
`flourish: name(1.6s), ...` line) and writes .claude/research/npc_variety/tables/anim_pool.tsv:
one row per (model, flourish) plus one per idle. A model that appears under several rows is
kept once (first row wins).
Run `python3 scripts/npcvar_anim_pool.py` to regenerate. The pure functions below are imported
by scripts/test_npcvar_anim_pool.py.
"""
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TABLE_DIR = REPO_ROOT / ".claude" / "research" / "npc_variety" / "tables"
ROWS_TXT = REPO_ROOT / ".claude" / "research" / "npc_variety" / "u03b_target_rows.txt"
ANIM_POOL_OUT = TABLE_DIR / "anim_pool.tsv"

COLUMNS = ["model", "row", "chain_len", "anim_name", "kind", "length_s", "class"]

CLASS_BY_NAME = {
    # ambient flourishes: everyday stances usable in any area
    "pausesh": "ambient", "scanning": "ambient", "shrug": "ambient", "rolleyes": "ambient",
    "touchheart": "ambient", "nodyes": "ambient", "greeting": "ambient",
    # military flourish
    "salute": "military",
    # excluded: event, combat-reward or gameplay-coupled stances (and anything unlisted)
    "victory": "excluded", "taunt": "excluded", "bow": "excluded", "powerup": "excluded",
    "point": "excluded", "equip": "excluded", "itemequip": "excluded",
    # idle stances
    "pause1": "idle", "pause2": "idle", "pause3": "idle", "cpause1": "idle", "cpause2": "idle",
    "standstill": "idle", "neutral": "idle", "good": "idle", "evil": "idle",
}

ROW_RE = re.compile(r"^row\s+(\d+)\s+(\S+)\s+type=(\S+)\s+model=(\S+)\s+chain=\[([^\]]*)\]")
IDLES_RE = re.compile(r"^\s+idles:\s*(.*?)\s*$")
FLOURISH_RE = re.compile(r"^\s+flourish:\s*(.*?)\s*$")
FLOURISH_ITEM_RE = re.compile(r"([A-Za-z0-9_]+)\(([0-9]+(?:\.[0-9]+)?)s\)")


def classify(name):
    """Return ambient | military | excluded | idle. Unlisted names are excluded."""
    return CLASS_BY_NAME.get(name, "excluded")


def parse_rows(text):
    """Parse u03b row blocks into one dict per distinct model (first row wins), in file order.

    Keys: model (str, original case), row (int), chain (list of lowercase str),
    idles (list of str), flourishes (dict name -> seconds float).
    """
    models = []
    seen = set()
    current = None
    for line in text.splitlines():
        if line.startswith("#"):
            current = None
            continue
        m = ROW_RE.match(line)
        if m:
            row_no, _name, _type, model, chain_txt = m.groups()
            if model in seen:
                current = None
                continue
            seen.add(model)
            current = {
                "model": model,
                "row": int(row_no),
                "chain": [c.strip() for c in chain_txt.split(">") if c.strip()],
                "idles": [],
                "flourishes": {},
            }
            models.append(current)
            continue
        if current is None:
            continue
        m = IDLES_RE.match(line)
        if m:
            current["idles"] = [s.strip() for s in m.group(1).split(",") if s.strip()]
            continue
        m = FLOURISH_RE.match(line)
        if m:
            current["flourishes"] = {
                name: float(secs) for name, secs in FLOURISH_ITEM_RE.findall(m.group(1))
            }
    return models


def ambient_names(models):
    """Distinct ambient flourish names present across all models."""
    return {name for m in models for name in m["flourishes"] if classify(name) == "ambient"}


def tsv_rows(models):
    """One row per (model, idle) and per (model, flourish), sorted by model then anim_name."""
    out = []
    for m in models:
        chain_len = len(m["chain"])
        for idle in m["idles"]:
            out.append({
                "model": m["model"], "row": m["row"], "chain_len": chain_len,
                "anim_name": idle, "kind": "idle", "length_s": "", "class": classify(idle),
            })
        for name, secs in m["flourishes"].items():
            out.append({
                "model": m["model"], "row": m["row"], "chain_len": chain_len,
                "anim_name": name, "kind": "flourish", "length_s": f"{secs:.1f}",
                "class": classify(name),
            })
    out.sort(key=lambda r: (r["model"], r["anim_name"]))
    return out


def write_tsv(path, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main():
    models = parse_rows(ROWS_TXT.read_text(encoding="utf-8"))
    rows = tsv_rows(models)
    write_tsv(ANIM_POOL_OUT, rows)
    print(f"models {len(models)} ambient {len(ambient_names(models))}")
    print(f"anim_pool: {len(rows)} rows -> {ANIM_POOL_OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
