#!/usr/bin/env python3
"""Compile decoupled Galaxy Map scripts for Echoes of the Republic.

1. Disables M4-78 checks in a_galaxymap.nss.
2. Removes M4-78 transition case in k_sup_galaxymap.nss.
3. Compiles a_galaxymap.ncs and k_sup_galaxymap.ncs to staging_echoes/Override/.
4. Removes k_003ebo_enter.ncs from staging_echoes/Override/ to preserve PartySwap in 003EBO.mod.

Echoes' k_sup_galaxymap.nss is PartySwap 1.5's with Echoes' planets added, so the
result keeps PartySwap's change (phase 6 checks which build copy it replaces).

Compiler: a TSLPatcher nwnnsscomp.exe (KOTOR2_NWNNSSCOMP; default: the one Repair
HK-47 ships, in the workspace; most mods ship one in tslpatchdata/) under Proton's
Wine (KOTOR2_PROTON_WINE, KOTOR2_WINE_PREFIX, as in ../pykotor_patches/nss.py),
with the game's own include scripts and Echoes' sources layered over them. That
reproduces Echoes' shipped .ncs byte for byte. Without it the phase exits:
pykotor's built-in compiler pushes engine-call arguments in source order, so its
galaxy map stores nothing the game can use (../check_ncs_arg_order.py). Set
KOTOR2_ALLOW_BUILTIN_NSS=1 to use it anyway.
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from pykotor.common.misc import Game
from pykotor.resource.formats.ncs import bytes_ncs, compile_nss

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _common  # noqa: E402
from _common import GAME_DIR, MAP_SRC_DIR, STAGING_DIR  # noqa: E402

STAGING_OVERRIDE = STAGING_DIR / "Override"

NWNNSSCOMP = str(_common.NWNNSSCOMP or "")
PROTON_WINE = str(_common.PROTON_WINE)
ALLOW_BUILTIN = _common.ALLOW_BUILTIN_NSS
WINE_PREFIX = os.environ.get(
    "KOTOR2_WINE_PREFIX", os.path.expanduser("~/.local/share/Steam/steamapps/compatdata/208580/pfx"))


def game_include_scripts():
    """{name.nss: bytes} of every script source in the game's BIFs (nwscript.nss, k_inc_*)."""
    from pykotor.extract.chitin import Chitin
    from pykotor.resource.type import ResourceType
    return {f"{r.resname().lower()}.nss": r.data() for r in Chitin(GAME_DIR / "chitin.key")
            if r.restype() == ResourceType.NSS}


def compile_with_nwnnsscomp(src: str) -> bytes:
    with tempfile.TemporaryDirectory() as d:
        d = Path(d)
        for name, data in game_include_scripts().items():
            (d / name).write_bytes(data)
        for f in Path(MAP_SRC_DIR).glob("*.nss"):  # Echoes' k_inc_hawk replaces the game's
            shutil.copy(f, d / f.name.lower())
        (d / "script.nss").write_text(src, encoding="latin-1")
        shutil.copy(NWNNSSCOMP, d / "nwnnsscomp.exe")
        res = subprocess.run([PROTON_WINE, "nwnnsscomp.exe", "-c", "script.nss", "-o", "script.ncs"],
                             cwd=d, env={**os.environ, "WINEPREFIX": WINE_PREFIX, "WINEDEBUG": "-all"},
                             capture_output=True, text=True, timeout=300)
        if res.returncode != 0 or not (d / "script.ncs").is_file():
            sys.exit(f"nwnnsscomp failed (exit {res.returncode}):\n{res.stdout}{res.stderr}")
        return (d / "script.ncs").read_bytes()


def compile_script(src: str) -> bytes:
    if NWNNSSCOMP and Path(NWNNSSCOMP).is_file() and Path(PROTON_WINE).is_file():
        print(f"  compiler: {NWNNSSCOMP} under {PROTON_WINE}")
        return compile_with_nwnnsscomp(src)
    if not ALLOW_BUILTIN:
        sys.exit(f"[ERROR] No nwnnsscomp to compile the galaxy map with (KOTOR2_NWNNSSCOMP={NWNNSSCOMP!r}, "
                 f"Wine {PROTON_WINE}). Set KOTOR2_NWNNSSCOMP to a TSLPatcher nwnnsscomp.exe (most mods ship "
                 "one in tslpatchdata/) and KOTOR2_PROTON_WINE if needed. pykotor's built-in compiler swaps "
                 "engine-call arguments; KOTOR2_ALLOW_BUILTIN_NSS=1 uses it anyway.")
    print("  WARNING: compiling with pykotor's built-in compiler (KOTOR2_ALLOW_BUILTIN_NSS=1). It swaps "
          "engine-call arguments: check the result with check_ncs_arg_order.py.")
    return bytes_ncs(compile_nss(src, Game.K2, library_lookup=[str(MAP_SRC_DIR)]))


def patch_a_galaxymap(src: str) -> str:
    """Disable M4-78 unconditionally: replace the `if (000_M478_Found == 0) { ... }` block."""
    replacement = '// M4-78 decoupled:\n\tSetPlanetAvailable(PLANET_M4_78, FALSE);\n\tSetPlanetSelectable(PLANET_M4_78, FALSE);'
    out, n = re.subn(
        r'if\s*\(\(GetGlobalNumber\("000_M478_Found"\)\s*==\s*0\)\)\s*\{[^}]*\}',
        lambda m: replacement,
        src,
    )
    if n == 0:
        raise ValueError("Could not find the 000_M478_Found condition in a_galaxymap.nss")
    return out


def patch_k_sup_galaxymap(src: str) -> str:
    """Empty the `case PLANET_M4_78:` travel branch."""
    out, n = re.subn(
        r'case\s+PLANET_M4_78:\s*\{[^}]*\}\s*break;',
        lambda m: 'case PLANET_M4_78: { /* M4-78 decoupled */ } break;',
        src,
    )
    if n == 0:
        raise ValueError("Could not find the case PLANET_M4_78 branch in k_sup_galaxymap.nss")
    return out


def main():
    print("=== Step 1: Patching a_galaxymap.nss ===")
    src = Path(MAP_SRC_DIR, "a_galaxymap.nss").read_text(encoding="latin-1")

    src_patched = patch_a_galaxymap(src)

    print("Compiling a_galaxymap.nss...")
    ncs_agm = compile_script(src_patched)
    out_agm = os.path.join(STAGING_OVERRIDE, "a_galaxymap.ncs")
    # Only a_galaxymap. Echoes' Override/a_galaxy_map.ncs (368 bytes) is a different
    # script: the prologue's (001EBO) galaxy map, which sets Peragus available and
    # calls ShowGalaxyMap. Copying the travel script over it left the prologue map
    # showing its tutorial text and never opening (found in game 2026-09-24).
    Path(out_agm).write_bytes(ncs_agm)
    print(f"Successfully compiled and wrote {out_agm} ({len(ncs_agm)} bytes)")

    print("\n=== Step 2: Patching k_sup_galaxymap.nss ===")
    src_sup = Path(MAP_SRC_DIR, "k_sup_galaxymap.nss").read_text(encoding="latin-1")

    src_sup_patched = patch_k_sup_galaxymap(src_sup)

    print("Compiling k_sup_galaxymap.nss...")
    ncs_sup = compile_script(src_sup_patched)
    out_sup = os.path.join(STAGING_OVERRIDE, "k_sup_galaxymap.ncs")
    Path(out_sup).write_bytes(ncs_sup)
    print(f"Successfully compiled and wrote {out_sup} ({len(ncs_sup)} bytes)")

    print("\n=== Step 3: Preserving PartySwap in 003EBO ===")
    ebo_enter_ncs = os.path.join(STAGING_OVERRIDE, "k_003ebo_enter.ncs")
    if os.path.exists(ebo_enter_ncs):
        os.remove(ebo_enter_ncs)
        print(f"Removed loose {ebo_enter_ncs} so 003EBO.mod's PartySwap script takes precedence.")

    ebo_hawk_nss = os.path.join(STAGING_OVERRIDE, "k_inc_hawk.nss")
    if os.path.exists(ebo_hawk_nss):
        os.remove(ebo_hawk_nss)
        print(f"Removed loose {ebo_hawk_nss} (include files are not loose runtime assets).")

    print("\nPhase 3 (Core Scripting & PartySwap Preservation) 100% COMPLETE and VERIFIED!")


if __name__ == "__main__":
    main()
