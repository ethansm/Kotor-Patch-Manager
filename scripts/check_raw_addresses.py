#!/usr/bin/env python3
"""Static check that no new raw engine address sneaks into the mod sources (doc 20 T0 extra, Phase 3; companion to GameAddr.h).

  python3 scripts/check_raw_addresses.py                      # check the repo
  python3 scripts/check_raw_addresses.py --quiet
  python3 scripts/check_raw_addresses.py --root DIR [--gameaddr GameAddr.h] [--allowlist raw_addresses.toml]
  python3 scripts/check_raw_addresses.py --write-baseline     # regenerate the allowlist (see the warning below)

Scans patch_manager_mods/*_src/**/*.{cpp,h,inc} (nested dirs included; _shared_src is one of them) for 8-hex-digit
image-range addresses 0x00400000-0x00BFFFFF (regex \\b0x00[4-9A-Ba-b][0-9A-Fa-f]{5}\\b; comments count too). Excluded:
testdata/ and retired/ directories, *_test.cpp, _shared_src/GameAddr.h itself (the source of truth) and the GENERATED .inc
tables (GENERATED_INC below, plus any .inc whose first lines say "generated").

Each occurrence must be
  - a value declared in _shared_src/GameAddr.h (`constexpr uintptr_t Name = 0x...;`, compared by integer value), or
  - listed for that file in tests/static/raw_addresses.toml (schema in that file's header).
Results:
  FAIL  an address that is neither (printed as file:line)
  WARN  an allowlist entry that is stale: the file no longer holds the address, the address is now a GameAddr value, or
        the file is no longer scanned / does not exist
  INFO  one line per file whose raw constants equal GameAddr values: "migration candidates: use gameaddr::Name"
Exit 1 on any FAIL.

--write-baseline rewrites the allowlist with every non-GameAddr address per file and prints an added/removed summary per
file. IT IS RUN ONLY BY A HUMAN OR OPUS AFTER REVIEWING THE NEW ADDRESSES, NEVER BY AUTOMATION: a regenerated baseline
turns every unreviewed new address into an accepted one. Read-only otherwise.
"""
import argparse
import re
import sys
import tomllib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402

CHECK = "raw-address"
ADDR_RE = re.compile(r"\b0x00[4-9A-Ba-b][0-9A-Fa-f]{5}\b")
GAMEADDR_RE = re.compile(r"\bconstexpr\s+uintptr_t\s+(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;")
SRC_SUFFIXES = (".cpp", ".h", ".inc")
EXCLUDED_DIRS = ("testdata", "retired")
GAMEADDR_REL = "patch_manager_mods/_shared_src/GameAddr.h"
ALLOWLIST_REL = "tests/static/raw_addresses.toml"

# Generated .inc tables (repo-relative path -> why they are skipped). Their contents are data, not code addresses, and
# are rewritten by the named script; any other .inc stays scanned unless its header says "generated" (is_generated_inc).
GENERATED_INC = {
    "patch_manager_mods/_shared_src/EffectIconTable.inc": "scripts/gen_buff_hud_icon_table.py (spells.2da icon table)",
    "patch_manager_mods/_shared_src/DebuffIconTable.inc": "scripts/gen_debuff_icon_table.py (debuff icon table)",
    "patch_manager_mods/_shared_src/FontWidths.inc": "scripts/gen_effect_name_table.py (font width tables)",
    "patch_manager_mods/active_effects_panel_src/EffectNameTable.inc": "scripts/gen_effect_name_table.py (effect/spell name table)",
    "patch_manager_mods/shield_bar_src/ShieldTable.inc": "scripts/gen_shield_table.py (shield rows)",
    "patch_manager_mods/equipment_3d_character_src/EquipWeaponTable.inc": "scripts/gen_equip3d_weapon_table.py (baseitems.2da)",
    "patch_manager_mods/inventory_3d_viewport_src/ItemModelTable.inc": "scripts/gen_inventory_3dview_table.py (baseitems.2da)",
    "patch_manager_mods/inventory_3d_viewport_src/ItemFitTable.inc": "scripts/gen_inventory_3dview_table.py (model fit bboxes)",
    "patch_manager_mods/inventory_3d_viewport_src/RigCamera.inc": "scripts/make_inv3d_light.py (camera rest pose)",
    "patch_manager_mods/action_hotbar_src/HotbarTextures.inc": "scripts/gen_hotbar_textures.py (TGA byte arrays)",
    "patch_manager_mods/companion_selfheal_src/SelfHealTextures.inc": "scripts/gen_selfheal_textures.py (TGA byte arrays)",
    "patch_manager_mods/_shared_src/HudKitTextures.inc": "scripts/gen_hud_kit_textures.py (TGA byte arrays)",
}

TOML_HEADER = """\
# raw_addresses.toml - engine addresses that may appear as raw 0x00XXXXXX literals in the mod sources.
# Consumed by scripts/check_raw_addresses.py (doc 20 T0 extra, Phase 3).
#
# The checker scans patch_manager_mods/*_src/**/*.{cpp,h,inc} (minus testdata/, retired/, *_test.cpp, GameAddr.h and the
# generated .inc tables) for image-range addresses 0x00400000-0x00BFFFFF. Every occurrence must be a value in
# patch_manager_mods/_shared_src/GameAddr.h or be listed here for that file:
#
#   [files."patch_manager_mods/x_src/X.cpp"]      # repo-relative path, forward slashes
#   addresses = ["0x004122A0", ...]               # normalized upper-case 0x00XXXXXX, sorted, no duplicates
#
# A new address in neither place is a FAIL. An entry the file no longer contains (or that GameAddr.h now covers) is a
# stale-entry WARN. To retire an entry, move the address into GameAddr.h and use gameaddr::Name.
#
# GENERATED FILE: `python3 scripts/check_raw_addresses.py --write-baseline` rewrites everything below this line.
# --write-baseline is run only by a human or Opus after reviewing the new addresses, never by automation.
"""


def norm(addr) -> str:
    return f"0x{int(addr, 16) if isinstance(addr, str) else addr:08X}"


def is_generated_inc(path: Path) -> bool:
    try:
        with open(path, errors="replace") as f:
            head = "".join(f.readline() for _ in range(3))
    except OSError:
        return False
    return "generated" in head.lower()


def scan_files(root):
    """Sorted [(repo-relative posix path, Path)] of the sources both static checkers scan (exclusions: module docstring)."""
    root = Path(root)
    out = []
    for src in sorted(root.glob("patch_manager_mods/*_src")):
        if not src.is_dir():
            continue
        for p in sorted(src.rglob("*")):
            if not p.is_file() or p.suffix not in SRC_SUFFIXES:
                continue
            rel = p.relative_to(root).as_posix()
            if any(part in EXCLUDED_DIRS for part in p.relative_to(src).parts[:-1]):
                continue
            if p.name.endswith("_test.cpp") or rel == GAMEADDR_REL:
                continue
            if p.suffix == ".inc" and (rel in GENERATED_INC or is_generated_inc(p)):
                continue
            out.append((rel, p))
    return out


def load_gameaddr(path):
    """{int value: [names]} from GameAddr.h."""
    vals = {}
    for name, v in GAMEADDR_RE.findall(Path(path).read_text(errors="replace")):
        vals.setdefault(int(v, 16), []).append(name)
    return vals


def load_allowlist(path):
    """{repo-relative path: set of normalized addresses}; a missing file is an empty allowlist."""
    p = Path(path)
    if not p.is_file():
        return {}
    with open(p, "rb") as f:
        data = tomllib.load(f)
    return {k: {norm(a) for a in v.get("addresses", [])} for k, v in data.get("files", {}).items()}


def scan_occurrences(path):
    """[(line number, normalized address)] for one file."""
    out = []
    with open(path, errors="replace") as f:
        for n, line in enumerate(f, 1):
            out.extend((n, norm(m)) for m in ADDR_RE.findall(line))
    return out


def run(root, gameaddr=None, allowlist=None, reporter=None):
    """Check `root`; returns (reporter, stats) with stats = {occurrences, distinct, files, per_file_raw}."""
    root = Path(root)
    rep = reporter or sc.Reporter()
    gameaddr = Path(gameaddr) if gameaddr else root / GAMEADDR_REL
    allow_path = Path(allowlist) if allowlist else root / ALLOWLIST_REL
    stats = {"occurrences": 0, "distinct": set(), "files": 0, "raw": {}}
    if not gameaddr.is_file():
        rep.fail(CHECK, f"{gameaddr} not found")
        return rep, stats
    ga = load_gameaddr(gameaddr)
    allow = load_allowlist(allow_path)
    files = scan_files(root)
    scanned = {rel for rel, _ in files}
    for rel, p in files:
        occ = scan_occurrences(p)
        stats["files"] += 1
        stats["occurrences"] += len(occ)
        raw = sorted({a for _, a in occ if int(a, 16) not in ga})
        stats["raw"][rel] = raw
        stats["distinct"].update(a for _, a in occ if int(a, 16) not in ga)
        allowed = allow.get(rel, set())
        for line, a in occ:
            if int(a, 16) not in ga and a not in allowed:
                rep.fail(CHECK, f"{rel}:{line} {a} is not in GameAddr.h or {ALLOWLIST_REL}; add a gameaddr:: name (preferred) "
                                f"or have a human/Opus review it into the allowlist (--write-baseline)")
        mig = {}
        for _, a in occ:
            if int(a, 16) in ga:
                mig[a] = ga[int(a, 16)][0]
        if mig:
            n = sum(1 for _, a in occ if int(a, 16) in ga)
            names = ", ".join(f"{a}={nm}" for a, nm in sorted(mig.items()))
            rep.info(CHECK, f"{rel}: {n} raw constant(s) equal GameAddr values - migration candidates: use gameaddr::Name ({names})")
        present = {a for _, a in occ}
        for a in sorted(allowed):
            if a not in present:
                rep.warn(CHECK, f"stale allowlist entry {a} for {rel}: the file no longer contains it")
            elif int(a, 16) in ga:
                rep.warn(CHECK, f"stale allowlist entry {a} for {rel}: now covered by gameaddr::{ga[int(a, 16)][0]}")
    for rel in sorted(set(allow) - scanned):
        rep.warn(CHECK, f"stale allowlist file {rel}: not scanned (missing, excluded or no longer a source file)")
    if not rep.of("FAIL"):
        rep.passed(CHECK, f"{stats['occurrences']} occurrences in {stats['files']} files; "
                          f"{len(stats['distinct'])} distinct addresses outside GameAddr.h, all allowlisted")
    return rep, stats


def render_baseline(raw) -> str:
    out = [TOML_HEADER]
    for rel in sorted(raw):
        if raw[rel]:
            addrs = ", ".join(f'"{a}"' for a in sorted(raw[rel]))
            out.append(f'[files."{rel}"]\naddresses = [{addrs}]\n')
    return "\n".join(out)


def write_baseline(root, gameaddr, allowlist, out=sys.stdout):
    """Rewrite the allowlist from the sources; prints an added/removed summary per file. Returns the (added, removed) maps."""
    root = Path(root)
    gameaddr = Path(gameaddr) if gameaddr else root / GAMEADDR_REL
    allow_path = Path(allowlist) if allowlist else root / ALLOWLIST_REL
    ga = load_gameaddr(gameaddr)
    old = load_allowlist(allow_path)
    new = {}
    for rel, p in scan_files(root):
        raw = sorted({a for _, a in scan_occurrences(p) if int(a, 16) not in ga})
        if raw:
            new[rel] = set(raw)
    added, removed = {}, {}
    for rel in sorted(set(old) | set(new)):
        a, r = sorted(new.get(rel, set()) - old.get(rel, set())), sorted(old.get(rel, set()) - new.get(rel, set()))
        if a:
            added[rel] = a
        if r:
            removed[rel] = r
    allow_path.parent.mkdir(parents=True, exist_ok=True)
    allow_path.write_text(render_baseline({k: sorted(v) for k, v in new.items()}))
    n_total = sum(len(v) for v in new.values())
    print(f"wrote {allow_path}: {len(new)} files, {n_total} file/address entries, "
          f"{len({a for v in new.values() for a in v})} distinct addresses", file=out)
    for rel in sorted(set(added) | set(removed)):
        print(f"  {rel}: +{len(added.get(rel, []))} -{len(removed.get(rel, []))}"
              + (f"  added {' '.join(added[rel])}" if rel in added else "")
              + (f"  removed {' '.join(removed[rel])}" if rel in removed else ""), file=out)
    if not added and not removed:
        print("  no changes", file=out)
    return added, removed


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--root", default=str(sc.REPO), help="repo root holding patch_manager_mods/ (default: this repo)")
    ap.add_argument("--gameaddr", help=f"GameAddr.h (default <root>/{GAMEADDR_REL})")
    ap.add_argument("--allowlist", help=f"allowlist toml (default <root>/{ALLOWLIST_REL})")
    ap.add_argument("--write-baseline", action="store_true",
                    help="regenerate the allowlist; run only by a human or Opus after reviewing the new addresses, never by automation")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    a = ap.parse_args(argv)
    if a.write_baseline:
        write_baseline(a.root, a.gameaddr, a.allowlist)
        return 0
    rep = sc.Reporter(quiet=a.quiet)
    run(a.root, a.gameaddr, a.allowlist, rep)
    return rep.finish("check_raw_addresses")


if __name__ == "__main__":
    sys.exit(main())
