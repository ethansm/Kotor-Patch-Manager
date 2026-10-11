#!/usr/bin/env python3
"""Find loose animation TXIs in override/ that animate a texture which isn't a flipbook.

A `.txi` with `proceduretype cycle` tells the engine to split its texture into a
numx x numy grid of frames and cycle them at `fps`. Mods that ship an animated
texture ship it as a flipbook `.tga` plus that `.txi`. If a later texture pack puts a
plain `.tpc` of the same name into override/, the `.tpc` wins (engine order: override
.tpc, then .tga, then the TexturePacks ERFs) but the loose `.txi` still applies, so a
single still image is sliced and cycled: fast flickering stripes in game (the
workbench, lab station and desks, 2026-09-25).

Flagged: a cycle TXI whose winning texture is
  - an override .tpc whose own embedded TXI is not a cycle, or
  - the ERF texture (nothing in override) whose embedded TXI is not a cycle.
A winning override .tga is not flagged: a mod's flipbook is shipped as a .tga, and
nothing in a plain .tga says whether it is one.

Calibrated on the modded build: before the fix it flagged exactly the 8 textures
whose mod flipbooks were shadowed, and none of the other 24 cycle TXIs.

Usage: check_txi_cycles.py OVERRIDE_DIR [--texturepacks DIR]   (exit 1 on a finding)
"""

import argparse
import logging
import os
from pathlib import Path
import sys

logging.disable(logging.CRITICAL)

try:
    from pykotor.resource.formats.erf import read_erf
    from pykotor.resource.formats.tpc import read_tpc
except ImportError:
    print("Error: pykotor must be installed in python environment.", file=sys.stderr)
    sys.exit(1)

ERF_ORDER = ["swpc_tex_gui.erf", "swpc_tex_tpa.erf", "swpc_tex_tpb.erf", "swpc_tex_tpc.erf"]


def txi_fields(text: str) -> dict[str, str]:
    """TXI key -> value (lower-case keys). A disabled `#key` line never matches a real key."""
    out = {}
    for line in text.splitlines():
        parts = line.strip().split(None, 1)
        if parts:
            out[parts[0].lower()] = parts[1].strip() if len(parts) > 1 else ""
    return out


def is_cycle(fields: dict[str, str]) -> bool:
    return fields.get("proceduretype", "").lower() == "cycle"


def embedded_txi(tpc_data: bytes) -> dict[str, str]:
    return txi_fields(read_tpc(tpc_data).txi or "")


def erf_textures(texpacks: Path | None, names: set[str]) -> dict[str, bytes]:
    """name -> TPC data for the wanted names, first ERF in ERF_ORDER wins."""
    found: dict[str, bytes] = {}
    if not names or texpacks is None or not texpacks.is_dir():
        return found
    files = {p.name.lower(): p for p in texpacks.iterdir()}
    for erf in ERF_ORDER:
        if erf not in files:
            continue
        for res in read_erf(files[erf]):
            key = str(res.resref).lower()
            if key in names and key not in found and res.restype.extension.lower() == "tpc":
                found[key] = res.data
    return found


def find_orphans(override: Path, texpacks: Path | None = None) -> tuple[list[str], int]:
    """Returns (findings, number of cycle TXIs checked)."""
    files = {p.name.lower(): p for p in override.iterdir() if p.is_file()}
    cycles = {}
    for name, path in files.items():
        if name.endswith(".txi"):
            fields = txi_fields(path.read_text(encoding="latin-1"))
            if is_cycle(fields):
                cycles[name[:-4]] = (path, fields)
    erf = erf_textures(texpacks, {s for s in cycles if f"{s}.tpc" not in files and f"{s}.tga" not in files})
    findings = []
    for stem, (path, fields) in sorted(cycles.items()):
        grid = f"{fields.get('numx', '?')}x{fields.get('numy', '?')}"
        if f"{stem}.tpc" in files:
            tpc = files[f"{stem}.tpc"]
            if not is_cycle(embedded_txi(tpc.read_bytes())):
                findings.append(f"{path.name}: cycles a {grid} grid, but the winning {tpc.name} is not a flipbook "
                                f"(no cycle in its own TXI); a shadowed mod .tga, or a stray .txi")
        elif f"{stem}.tga" in files:
            continue   # a mod flipbook ships as .tga; also excluded from the ERF lookup above
        elif stem in erf and not is_cycle(embedded_txi(erf[stem])):
            findings.append(f"{path.name}: cycles a {grid} grid over the vanilla ERF texture, which is not a flipbook")
    return findings, len(cycles)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("override", type=Path)
    ap.add_argument("--texturepacks", type=Path, default=None,
                    help="TexturePacks dir (default: ../TexturePacks next to override).")
    args = ap.parse_args(argv)
    texpacks = args.texturepacks or args.override.parent / "TexturePacks"
    findings, checked = find_orphans(args.override, texpacks)
    for f in findings:
        print(f"  {f}")
    print(f"{checked} cycle TXI(s) checked, {len(findings)} animating a texture that isn't a flipbook")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
