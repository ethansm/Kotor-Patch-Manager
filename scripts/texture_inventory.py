#!/usr/bin/env python3
"""Inventory KOTOR II texture sources: our upscales vs vanilla vs the 4x pack.

Writes two TSV files into <workspace>/texture_inventory/:
  upscale_manifest.tsv     live .tpc/.tga files whose size matches a record in
                           the ultrawide upscale log (upscaled.json)
  item_texture_status.tsv  item-prefix textures with one status each:
                           vanilla unchanged / vanilla only (not overridden) /
                           our upscale / 4x pack / changed (other source) /
                           live only (no vanilla copy)

Vanilla bytes come from the game's texturepacks/swpc_tex_*.erf archives.
The game folder is only read. Run with the repo venv (it has pykotor):

  .venv/bin/python scripts/texture_inventory.py \
      [--game DIR] [--workspace DIR] [--pack DIR] [--out DIR]

--pack is an optional folder of 4x pack files (the pack's "Copy contents to ...
Override folder"); without it the 4x-pack status is never assigned.
"""
from __future__ import annotations

import argparse
import collections
import csv
import datetime
import glob
import hashlib
import json
import os
import struct
import subprocess
import sys
from pathlib import Path

DEFAULT_GAME = Path.home() / ".steam/steam/steamapps/common/Knights of the Old Republic II"
DEFAULT_WORKSPACE = Path.home() / "Games/KOTOR2_Mod_Workspace"
ITEM_PREFIXES = ("i_", "w_", "p_", "lmg_", "lbl_", "load_", "pnl_", "uibit_")


def md5_bytes(data: bytes) -> str:
    return hashlib.md5(data).hexdigest()


def texture_dims(path: Path) -> tuple[int, int, str]:
    """Width, height and format from a TPC or TGA header (no full decode)."""
    head = path.read_bytes()[:18]
    if path.suffix.lower() == ".tpc":
        _, _, w, h, enc, _ = struct.unpack("<IfHHBB", head[:14])
        return w, h, {1: "grey", 2: "DXT1", 4: "DXT5"}.get(enc, str(enc))
    w, h = struct.unpack("<HH", head[12:16])
    return w, h, "tga"


def birth_time(path: Path) -> str:
    """Creation time via stat(1); empty when the filesystem does not record it."""
    try:
        out = subprocess.run(["stat", "-c", "%W", str(path)], capture_output=True, text=True, check=True).stdout.strip()
        ts = int(out)
    except (subprocess.CalledProcessError, ValueError, OSError):
        return ""
    if ts <= 0:
        return ""
    return datetime.datetime.fromtimestamp(ts).isoformat(sep=" ", timespec="minutes")


def load_upscale_records(workspace: Path) -> dict[str, dict]:
    """Map lowercase texture stem -> upscale record (last model wins)."""
    log = workspace / "ultrawide_gui" / "upscaled" / "upscaled.json"
    data = json.loads(log.read_text(encoding="utf-8"))
    records: dict[str, dict] = {}
    for stem, models in data.items():
        for model, rec in models.items():
            if "size" in rec:
                records[stem.lower()] = {"model": model, **rec}
    return records


def pack_names(pack: Path | None) -> set[str]:
    if pack is None or not pack.is_dir():
        return set()
    names = set()
    for root, _, files in os.walk(pack):
        names.update(f.lower() for f in files)
    return names


def live_index(live: Path) -> dict[str, Path]:
    return {f.lower(): live / f for f in os.listdir(live)}


def build_manifest(live: Path, records: dict[str, dict], pack: set[str], out: Path) -> dict[str, dict]:
    """Write upscale_manifest.tsv. Returns name (lowercase) -> row for the status pass."""
    index = live_index(live)
    rows = []
    for stem, rec in records.items():
        path = index.get(stem + ".tpc") or index.get(stem + ".tga")
        if path is None:
            continue
        w, h, fmt = texture_dims(path)
        if [w, h] != list(rec["size"]):
            continue  # live file is not our output at the recorded size
        rows.append({
            "name": path.name,
            "live_path": str(path),
            "width": w,
            "height": h,
            "format": fmt,
            "birth": birth_time(path),
            "upscale_model": rec["model"],
            "upscale_time": rec.get("timestamp", ""),
            "source_kind": rec.get("source_kind", ""),
            "source_sha256": rec.get("source_sha256", ""),
            "output_sha256": rec.get("output_sha256", ""),
            "in_4x_pack": path.name.lower() in pack,
            "live_md5": md5_bytes(path.read_bytes()),
        })
    rows.sort(key=lambda r: r["name"].lower())
    with (out / "upscale_manifest.tsv").open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=list(rows[0].keys()), delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)
    return {r["name"].lower(): r for r in rows}


def load_vanilla(game: Path) -> dict[str, str]:
    """Item-prefix .tpc name -> md5 of the vanilla copy in texturepacks ERFs (first archive wins)."""
    from pykotor.resource.formats.erf import read_erf

    vanilla: dict[str, str] = {}
    for erf_path in sorted(glob.glob(str(game / "texturepacks" / "*.erf"))):
        for res in read_erf(erf_path):
            name = res.resref.get().lower()
            if name.startswith(ITEM_PREFIXES):
                vanilla.setdefault(name + ".tpc", md5_bytes(res.data))
    return vanilla


def build_status(live: Path, vanilla: dict[str, str], ours: dict[str, dict], pack: set[str], out: Path) -> collections.Counter:
    index = live_index(live)
    names = set(vanilla) | {k for k in index if k.endswith(".tpc") and k.startswith(ITEM_PREFIXES)}
    counts: collections.Counter = collections.Counter()
    rows = []
    for name in sorted(names):
        if name in index:
            live_md5 = md5_bytes(index[name].read_bytes())
            if name in vanilla and live_md5 == vanilla[name]:
                status = "vanilla unchanged (live)"
            elif name in ours:
                status = "our upscale (live)"
            elif name in pack:
                status = "4x pack (live)"
            elif name in vanilla:
                status = "changed vs vanilla, other source"
            else:
                status = "live only, no vanilla (other mod or new)"
        else:
            status = "vanilla only (ERF, not overridden)" if name in vanilla else "missing"
        counts[status] += 1
        rows.append((name, status))
    with (out / "item_texture_status.tsv").open("w", newline="", encoding="utf-8") as fh:
        writer = csv.writer(fh, delimiter="\t")
        writer.writerow(["name", "status"])
        writer.writerows(rows)
    return counts


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--game", type=Path, default=DEFAULT_GAME, help="KOTOR II install folder")
    parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="mod workspace with ultrawide_gui/upscaled")
    parser.add_argument("--pack", type=Path, default=None, help="optional folder of 4x pack override files")
    parser.add_argument("--out", type=Path, default=None, help="output folder (default <workspace>/texture_inventory)")
    args = parser.parse_args(argv)

    live = args.game / "override"
    out = args.out or (args.workspace / "texture_inventory")
    out.mkdir(parents=True, exist_ok=True)
    if not live.is_dir():
        print(f"error: override folder not found: {live}", file=sys.stderr)
        return 1

    records = load_upscale_records(args.workspace)
    pack = pack_names(args.pack)
    manifest = build_manifest(live, records, pack, out)
    vanilla = load_vanilla(args.game)
    counts = build_status(live, vanilla, manifest, pack, out)

    print(f"upscale manifest rows: {len(manifest)} -> {out / 'upscale_manifest.tsv'}")
    print(f"vanilla item textures (ERF): {len(vanilla)}")
    for status, n in sorted(counts.items()):
        print(f"  {status}: {n}")
    print(f"item status -> {out / 'item_texture_status.tsv'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
