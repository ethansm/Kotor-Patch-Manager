"""Paths and file lookup shared by the Echoes of the Republic compatch scripts.

Every path comes from an environment variable, with the Steam-on-Linux layout as
the default. install_echoes_compatch.py sets these for each phase from its
--game-dir / --staging-dir / --map-src options, so a GOG or custom install is
checked and patched in the same place.

  KOTOR2_ROOT        game dir (dialog.tlk, override/, Modules/)
  KOTOR2_WORKSPACE   mod workspace, the base of the two defaults below
  ECHOES_STAGING     extracted Echoes of the Republic archive (Override/, Modules/, ...)
  ECHOES_MAP_SRC     Echoes' galaxy map .nss sources (phase 3)
  KOTOR2_NWNNSSCOMP  TSLPatcher nwnnsscomp.exe for phase 3 (default: the one
                     Repair HK-47 ships, if the workspace has it)
  KOTOR2_PROTON_WINE Wine binary nwnnsscomp runs under (default: Proton Experimental)
  KOTOR2_ALLOW_BUILTIN_NSS=1  let phase 3 fall back to pykotor's built-in
                     compiler, which swaps engine-call arguments
"""
import os
from pathlib import Path


def _env_path(var, default):
    return Path(os.environ.get(var) or default).expanduser()


GAME_DIR = _env_path("KOTOR2_ROOT", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
WORKSPACE = _env_path("KOTOR2_WORKSPACE", "~/Games/KOTOR2_Mod_Workspace")
STAGING_DIR = _env_path("ECHOES_STAGING", WORKSPACE / "staging_echoes")
MAP_SRC_DIR = _env_path(
    "ECHOES_MAP_SRC", WORKSPACE / "00_downloads" / "KOTOR 2 Echoes of the Republic Galaxy Map Source Files"
)

_MOD127_NWNNSSCOMP = WORKSPACE / "01_extracted" / "mod127_repair_hk47" / "tslpatchdata" / "nwnnsscomp.exe"
NWNNSSCOMP = (Path(os.environ["KOTOR2_NWNNSSCOMP"]).expanduser() if os.environ.get("KOTOR2_NWNNSSCOMP")
              else _MOD127_NWNNSSCOMP if _MOD127_NWNNSSCOMP.is_file() else None)
PROTON_WINE = _env_path("KOTOR2_PROTON_WINE",
                        "~/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine")
ALLOW_BUILTIN_NSS = os.environ.get("KOTOR2_ALLOW_BUILTIN_NSS") == "1"


def find_one_ci(directory, name) -> Path:
    """The one file in `directory` named `name` in any letter case.

    Linux can hold "701kor.mod" and "701KOR.mod" as two files, and Wine then loads
    either one, so picking the first match would merge into an arbitrary copy.
    Case-alias symlinks (symlink_pass.py) that point at another match are the same
    file and don't count. Two different real files raise RuntimeError.
    """
    directory = Path(directory)
    matches = sorted(p for p in directory.iterdir() if p.name.lower() == name.lower())
    if not matches:
        raise FileNotFoundError(f"Could not find {name} in {directory}")
    files = {}  # real path -> the spelling to use for it
    for p in matches:
        real = os.path.realpath(p)
        if real not in files or files[real].is_symlink():
            files[real] = p
    if len(files) > 1:
        spellings = ", ".join(p.name for p in files.values())
        raise RuntimeError(
            f"{len(files)} different files match {name} in {directory}: {spellings}. "
            f"Keep one spelling (see ../case_dedup.py) and re-run."
        )
    return next(iter(files.values()))
