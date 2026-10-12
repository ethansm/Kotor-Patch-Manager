#!/usr/bin/env python3
"""Build the T12h probe-only files for k1p_commm from the live game. The game dir is only read.

Writes ONLY into --out:
  appearance.2da     live table + row 812 (copy of row 17, label K1P_CommM_Probe,
                     race/modela/modelb/texa/texb = texa)
  per_miner_002.utc  the per_miner_002 UTC from modules/201TEL.mod with appearance_id 812 and
                     texture_variation = texture_var
Every check runs before the first write. A refusal raises BuildError (CLI: 'error: ...' on stderr, exit 1).

Needs pykotor. The CLI re-executes itself under the k1port venv (python -I) when pykotor is not
importable. build() imports pykotor lazily and raises BuildError when it is missing.

Usage: k1_port_probe_build.py --game-dir DIR [--out DIR] [--texa NAME] [--texture-var N] [--json]
Exit: 0 ok, 1 refused or error. argparse usage errors exit 2.
"""
import argparse
import hashlib
import json
import os
import re
import sys
from pathlib import Path
from types import SimpleNamespace

sys.dont_write_bytecode = True

VENV_PY = Path(os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python"))
DEFAULT_OUT = "~/Games/KOTOR2_Mod_Workspace/k1_port_stage/probe_T12h/override"
APPEARANCE_REL = "override/appearance.2da"
MODULE_REL = "modules/201TEL.mod"
UTC_NAME = "per_miner_002"
LIVE_ROWS = 812
TEMPLATE_ROW = 17
NEW_ROW = 812
NEW_LABEL = "K1P_CommM_Probe"
DEFAULT_TEXA = "k1p_commm"
DEFAULT_TEXTURE_VAR = 4
TEXA_COLUMNS = ("race", "modela", "modelb", "texa", "texb")
TEXA_RE = re.compile(r"[a-z0-9_]{1,16}")


class BuildError(ValueError):
    """A refusal or an unusable input. The CLI prints 'error: ...' and exits 1."""


def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _pykotor() -> SimpleNamespace:
    """Import the pykotor API the build needs. Refuses with a pointer to the venv when it is missing."""
    try:
        from pykotor.resource.formats.erf import read_erf
        from pykotor.resource.formats.twoda import bytes_2da, read_2da
        from pykotor.resource.generics.utc import bytes_utc, read_utc
        from pykotor.resource.type import ResourceType
    except ImportError as exc:
        raise BuildError(f"pykotor not importable ({exc}); run the CLI or use: "
                         f"{VENV_PY} -I {Path(__file__).resolve()}") from exc
    return SimpleNamespace(read_erf=read_erf, read_2da=read_2da, bytes_2da=bytes_2da,
                           read_utc=read_utc, bytes_utc=bytes_utc, ResourceType=ResourceType)


def _check_args(texa, texture_var, template_row) -> None:
    if not isinstance(texa, str) or not TEXA_RE.fullmatch(texa):
        raise BuildError(f"texa must be 1-16 chars of [a-z0-9_], got {texa!r}")
    if isinstance(texture_var, bool) or not isinstance(texture_var, int) or not 0 <= texture_var <= 255:
        raise BuildError(f"texture_var must be an int 0-255, got {texture_var!r}")
    if isinstance(template_row, bool) or not isinstance(template_row, int) or template_row < 0:
        raise BuildError(f"template_row must be an int >= 0, got {template_row!r}")


def _resolve_dirs(game_dir, out_dir) -> tuple[Path, Path]:
    game = Path(game_dir).expanduser()
    if not game.is_dir():
        raise BuildError(f"game dir not found: {game}")
    game = game.resolve()
    out = Path(out_dir).expanduser().resolve()
    if out == game or out.is_relative_to(game):
        raise BuildError(f"refusing --out {out}: it is inside the game dir {game} (the game dir is read-only here)")
    if out.exists() and not out.is_dir():
        raise BuildError(f"--out {out} exists and is not a directory")
    return game, out


def _read_live(game: Path) -> tuple[bytes, bytes]:
    app, mod = game / APPEARANCE_REL, game / MODULE_REL
    for p in (app, mod):
        if not p.is_file():
            raise BuildError(f"missing live file: {p}")
    return app.read_bytes(), mod.read_bytes()


def _build_appearance(api: SimpleNamespace, app_live: bytes, texa: str, template_row: int) -> bytes:
    t = api.read_2da(app_live)
    height = t.get_height()
    if height != LIVE_ROWS:
        raise BuildError(f"live appearance.2da has {height} rows, expected {LIVE_ROWS}; refusing (the table changed)")
    if template_row >= height:
        raise BuildError(f"template row {template_row} is outside 0..{height - 1}")
    t.add_row(str(NEW_ROW), {c: t.get_cell(template_row, c) for c in t.get_headers() if c != "label"})
    t.set_label(NEW_ROW, NEW_LABEL)
    for c in TEXA_COLUMNS:
        t.set_cell(NEW_ROW, c, texa)
    if t.get_height() != NEW_ROW + 1:
        raise BuildError(f"expected {NEW_ROW + 1} rows after the append, got {t.get_height()}")
    return api.bytes_2da(t)


def _build_utc(api: SimpleNamespace, mod_live: bytes, texture_var: int, template_row: int) -> bytes:
    erf = api.read_erf(mod_live)
    hits = [r for r in erf if r.resref.get().lower() == UTC_NAME and r.restype == api.ResourceType.UTC]
    if len(hits) != 1:
        raise BuildError(f"expected one {UTC_NAME}.utc in {MODULE_REL}, found {len(hits)}")
    u = api.read_utc(hits[0].data)
    if u.appearance_id != template_row:
        raise BuildError(f"{UTC_NAME}.utc appearance_id is {u.appearance_id}, expected {template_row}; refusing")
    u.appearance_id = NEW_ROW
    u.texture_variation = texture_var
    return api.bytes_utc(u)


def build(game_dir, out_dir, texa: str = DEFAULT_TEXA, texture_var: int = DEFAULT_TEXTURE_VAR,
          template_row: int = TEMPLATE_ROW) -> dict:
    """Write <out_dir>/appearance.2da and <out_dir>/per_miner_002.utc from the live game (read only).

    Returns {"appearance_2da", "utc", "base_sha256", "new_row"}. Raises BuildError; writes nothing on refusal.
    """
    _check_args(texa, texture_var, template_row)
    game, out = _resolve_dirs(game_dir, out_dir)
    api = _pykotor()
    app_live, mod_live = _read_live(game)
    app_bytes = _build_appearance(api, app_live, texa, template_row)
    utc_bytes = _build_utc(api, mod_live, texture_var, template_row)
    try:
        out.mkdir(parents=True, exist_ok=True)
        (out / "appearance.2da").write_bytes(app_bytes)
        (out / f"{UTC_NAME}.utc").write_bytes(utc_bytes)
    except OSError as exc:
        raise BuildError(f"cannot write into {out}: {exc}") from exc
    return {
        "appearance_2da": sha256_hex(app_bytes),
        "utc": sha256_hex(utc_bytes),
        "base_sha256": sha256_hex(app_live),
        "new_row": NEW_ROW,
    }


def _reexec_under_venv(argv: list) -> None:
    """If pykotor is not importable, re-run this file under the k1port venv python -I. Does not return then."""
    try:
        import pykotor  # noqa: F401
        return
    except ImportError:
        pass
    venv_root = VENV_PY.parent.parent
    if Path(sys.prefix).resolve() == venv_root.resolve():
        raise BuildError(f"pykotor not importable inside the venv {venv_root}")
    if not VENV_PY.exists():
        raise BuildError(f"pykotor not importable and the venv python is missing: {VENV_PY}")
    os.execv(str(VENV_PY), [str(VENV_PY), "-I", str(Path(__file__).resolve()), *argv])


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Build the k1p_commm probe appearance.2da row 812 and "
                                             "per_miner_002.utc from the live game (game dir is read only).")
    ap.add_argument("--game-dir", required=True, metavar="DIR", help="KOTOR II install dir (read only)")
    ap.add_argument("--out", default=DEFAULT_OUT, metavar="DIR", help="output dir (default: %(default)s)")
    ap.add_argument("--texa", default=DEFAULT_TEXA, metavar="NAME",
                    help="value for race/modela/modelb/texa/texb of row 812 (default: %(default)s)")
    ap.add_argument("--texture-var", type=int, default=DEFAULT_TEXTURE_VAR, metavar="N",
                    help="UTC texture_variation (default: %(default)s)")
    ap.add_argument("--json", action="store_true", help="print the result as JSON")
    args = ap.parse_args(argv)
    try:
        _reexec_under_venv(sys.argv[1:] if argv is None else list(argv))
        res = build(args.game_dir, args.out, texa=args.texa, texture_var=args.texture_var)
    except (BuildError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(res, indent=2, sort_keys=True))
    else:
        print(f"appearance_2da: {res['appearance_2da']}")
        print(f"utc: {res['utc']}")
        print(f"base_sha256: {res['base_sha256']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
