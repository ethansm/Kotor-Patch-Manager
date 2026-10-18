#!/usr/bin/env python3
"""Resolve KOTOR 1 resources (models, textures, templates) from a K1 install, read only.

  python3 k1_port_resolve.py [--k1-dir DIR] [--json] RESREF.EXT...

Search order for one resource (RESREF + extension):
  1. <k1>/Override        plain case-insensitive file lookup, no pykotor. Symlinks are refused.
  2. TexturePacks         tga/tpc only (pykotor Installation).
  3. chitin (BIFs)        pykotor Installation.
  4. modules              pykotor Installation.
(Override -> chitin -> modules, the game's own order. Texture packs come before chitin for textures, as in
k1_port_textures.locate_in_install.)

Each hit is a Resolved with its origin, a label, the sha256 of the bytes and a licence tag:
  override                      -> unverified_override  (flagged: could be any mod file)
  texturepack / chitin / module -> vanilla_k1, or hd_overlay (flagged) when the install carries K1_HD_OVERLAY.json
The K1 dir comes from --k1-dir, env KOTOR1_ROOT, the config ([k1] dir) or the Steam default, in that order,
and must contain chitin.key.

Never writes into the install or anywhere else. pykotor itself creates a logs/ dir; k1_port_textures.pykotor_cwd()
points that at env K1PORT_LOG_DIR (default ~/.cache/k1_port_textures/pykotor_logs), outside the install.
pykotor (patched k1port venv, see setup_venv.sh) is imported only when Override misses, so Override-only use and
--help work under plain python3.
Exit: 0 all found, 1 something missing or refused, 2 bad arguments.
"""
from __future__ import annotations

import argparse
import contextlib
import hashlib
import json
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))  # python -I drops the script dir from sys.path
import k1_port_textures as kt  # noqa: E402  stdlib only; pykotor is imported lazily inside it

DEFAULT_K1_DIR = "~/.local/share/Steam/steamapps/common/swkotor"
ENV_VAR = "KOTOR1_ROOT"
SETUP_HINT = ("pykotor is not importable. Run this under the patched venv: "
              "~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python "
              "(create it with patch_manager_mods/k1_port/setup_venv.sh)")
FLAGGED = {"unverified_override", "hd_overlay"}
ORIGINS = ("override", "texturepack", "chitin", "module")
TEXTURE_EXTS = {"tga", "tpc"}
RESREF_RE = re.compile(r"^[a-z0-9_]{1,16}$")
EXT_RE = re.compile(r"^[a-z0-9]{1,4}$")


class ResolveError(Exception):
    """A K1 dir, pykotor or resource problem the caller must fix; nothing was written."""


@dataclass(frozen=True)
class Resolved:
    data: bytes
    origin: str    # override | texturepack | chitin | module
    sha256: str
    path: str      # human label: install-relative file, or "chitin:<bif>" style
    licence: str   # unverified_override | vanilla_k1 | hd_overlay


def is_flagged(licence: str) -> bool:
    return licence in FLAGGED


# ---------------------------------------------------------------- K1 dir

def _has_chitin(d: Path) -> bool:
    try:
        return any(e.name.lower() == "chitin.key" and e.is_file() for e in os.scandir(d))
    except OSError:
        return False


def find_k1_dir(cli: str | Path | None, config: str | Path | None, env=os.environ) -> Path:
    """First of: cli, env KOTOR1_ROOT, config value, the Steam default. Must hold chitin.key (any case)."""
    candidates = [("--k1-dir", cli), (f"env {ENV_VAR}", env.get(ENV_VAR)),
                  ("config [k1] dir", config), ("default Steam path", DEFAULT_K1_DIR)]
    for source, value in candidates:
        if value is None or str(value).strip() == "":
            continue
        path = Path(str(value).strip()).expanduser()
        if _has_chitin(path):
            return path
        raise ResolveError(f"K1 dir from {source} is not a KOTOR 1 install: {path} (no chitin.key). "
                           f"Set {ENV_VAR} or [k1] dir in the config to the folder that holds chitin.key.")
    raise ResolveError(f"no K1 dir found. Set {ENV_VAR} or [k1] dir in the config.")  # unreachable: default is last


# ---------------------------------------------------------------- Override (no pykotor)

def _ci_child(parent: Path, name: str) -> Path | None:
    """Entry of `parent` whose name equals `name` ignoring case (sorted first on ties), else None."""
    want = name.lower()
    try:
        hits = sorted(e.name for e in os.scandir(parent) if e.name.lower() == want)
    except OSError:
        return None
    return parent / hits[0] if hits else None


def _override_lookup(k1: Path, resref: str, ext: str) -> tuple[bytes, str] | None:
    odir = _ci_child(k1, "override")
    if odir is None:
        return None
    if odir.is_symlink() or not odir.is_dir():
        raise ResolveError(f"refused: {odir} is a symlink or not a directory; Override must live inside the install")
    f = _ci_child(odir, f"{resref}.{ext}")
    if f is None:
        return None
    if f.is_symlink():
        raise ResolveError(f"refused: {f} is a symlink (could point outside the install)")
    if not f.is_file():
        return None
    try:
        return f.read_bytes(), f"{odir.name}/{f.name}"
    except OSError as e:
        raise ResolveError(f"cannot read {f}: {e}") from e


# ---------------------------------------------------------------- pykotor (lazy)

class _PykotorBackend:
    """pykotor Installation lookups by group: texturepack | chitin | module."""

    def __init__(self, inst, api):
        self._inst = inst
        self._api = api
        loc = api.SearchLocation
        self._groups = {
            "texturepack": [loc.TEXTURES_TPA, loc.TEXTURES_TPB, loc.TEXTURES_TPC, loc.TEXTURES_GUI],
            "chitin": [loc.CHITIN],
            "module": [loc.MODULES],
        }

    def find(self, resref: str, ext: str, group: str) -> tuple[bytes, str] | None:
        rt = self._api.ResourceType.from_extension(ext)
        if rt is None or getattr(rt, "type_id", 0) < 0:
            raise ResolveError(f"unknown resource type extension: {ext!r}")
        with contextlib.redirect_stdout(sys.stderr), kt.pykotor_cwd():
            res = self._inst.resource(resref, rt, self._groups[group])
        if res is None:
            return None
        name = Path(str(res.filepath)).name
        label = {"texturepack": f"texturepack:{name}", "chitin": f"chitin:{name}", "module": f"module:{name}"}[group]
        return bytes(res.data), label


@contextlib.contextmanager
def _open_backend(k1: Path):
    """Yields an object with find(resref, ext, group) -> (bytes, label) | None. Tests replace this."""
    errors: list[str] = []
    api = kt.import_install_api(errors)
    if api is None:
        raise ResolveError(f"{SETUP_HINT}. ({'; '.join(errors)})")
    from pykotor.extract.installation import SearchLocation  # noqa: PLC0415  import_install_api already proved pykotor works
    api.SearchLocation = SearchLocation
    try:
        with contextlib.redirect_stdout(sys.stderr), kt.pykotor_cwd():
            inst = kt.open_install(k1, api, errors)
    except OSError as e:
        raise ResolveError(f"cannot prepare pykotor log dir: {type(e).__name__}: {e}") from e
    if inst is None:
        raise ResolveError("; ".join(errors))
    yield _PykotorBackend(inst, api)


# ---------------------------------------------------------------- resolve

def _check_item(resref: str, ext: str) -> tuple[str, str]:
    r, e = str(resref).lower(), str(ext).lower().lstrip(".")
    if not RESREF_RE.match(r):
        raise ResolveError(f"bad resref {resref!r}: need 1-16 chars of [a-z0-9_]")
    if not EXT_RE.match(e):
        raise ResolveError(f"bad extension {ext!r}: need 1-4 chars of [a-z0-9]")
    return r, e


def _licence_for(origin: str, k1: Path) -> str:
    if origin == "override":
        return "unverified_override"
    return "hd_overlay" if kt.overlay_marker(k1) is not None else "vanilla_k1"


def _make(data: bytes, origin: str, label: str, licence: str) -> Resolved:
    return Resolved(data=data, origin=origin, sha256=hashlib.sha256(data).hexdigest(), path=label, licence=licence)


def _resolve_one(k1: Path, resref: str, ext: str, get_backend, licences: dict[str, str]) -> Resolved | None:
    hit = _override_lookup(k1, resref, ext)
    if hit is not None:
        return _make(hit[0], "override", hit[1], licences["override"])
    backend = get_backend()
    groups = ("texturepack", "chitin", "module") if ext in TEXTURE_EXTS else ("chitin", "module")
    for group in groups:
        got = backend.find(resref, ext, group)
        if got is not None:
            return _make(got[0], group, got[1], licences[group])
    return None


def resolve_many(k1_dir: str | Path, items: list[tuple[str, str]]) -> dict[tuple[str, str], Resolved | None]:
    """{(resref, ext): Resolved | None}. Keys are the lowercased pairs. pykotor opens once, and only on an Override miss."""
    k1 = Path(k1_dir).expanduser()
    if not _has_chitin(k1):
        raise ResolveError(f"not a KOTOR 1 install: {k1} (no chitin.key). Set {ENV_VAR} or [k1] dir in the config.")
    checked = [_check_item(r, e) for r, e in items]
    licences = {o: _licence_for(o, k1) for o in ORIGINS}
    out: dict[tuple[str, str], Resolved | None] = {}
    with contextlib.ExitStack() as stack:
        backend: list = []

        def get_backend():
            if not backend:
                backend.append(stack.enter_context(_open_backend(k1)))
            return backend[0]

        for key in checked:
            if key not in out:
                out[key] = _resolve_one(k1, key[0], key[1], get_backend, licences)
    return out


def resolve(k1_dir: str | Path, resref: str, restype_ext: str) -> Resolved | None:
    r, e = _check_item(resref, restype_ext)
    return resolve_many(k1_dir, [(r, e)])[(r, e)]


_NAME_BYTE = frozenset(b"abcdefghijklmnopqrstuvwxyz0123456789_")


def mesh_names(data: bytes, names) -> list[str]:
    """Names (lowercased, input order) that occur in a binary MDL as a NUL-terminated string field (case-insensitive):
    the bytes must end in NUL and not continue a longer name on the left. Used to find which textures a mesh uses."""
    low = bytes(data).lower()
    found = []
    for name in names:
        n = str(name).lower().encode("ascii")
        i = low.find(n)
        while i >= 0:
            end = i + len(n)
            if end < len(low) and low[end] == 0 and (i == 0 or low[i - 1] not in _NAME_BYTE):
                found.append(n.decode())
                break
            i = low.find(n, i + 1)
    return found


def licence_report(resolved: dict) -> list[dict]:
    """One manifest row per found item: resref, ext, origin, path, sha256, licence, flagged. Missing items are skipped."""
    rows = []
    for (resref, ext), res in sorted(resolved.items()):
        if res is None:
            continue
        rows.append({"resref": resref, "ext": ext, "origin": res.origin, "path": res.path, "sha256": res.sha256,
                     "licence": res.licence, "flagged": is_flagged(res.licence)})
    return rows


# ---------------------------------------------------------------- CLI

def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Resolve K1 resources read only (Override, texture packs, chitin, modules).")
    ap.add_argument("--k1-dir", help=f"K1 install (else env {ENV_VAR}, else the Steam default)")
    ap.add_argument("--json", action="store_true", help="print the licence report as JSON")
    ap.add_argument("--scan", action="append", default=[], metavar="NAME",
                    help="with --json: list which of these names each found .mdl references (key 'contains')")
    ap.add_argument("items", nargs="+", metavar="RESREF.EXT")
    args = ap.parse_args(argv)
    pairs = []
    for it in args.items:
        stem, dot, ext = it.rpartition(".")
        if not dot or not stem or not ext:
            ap.error(f"{it!r} must look like RESREF.EXT")
        pairs.append((stem, ext))
    try:
        k1 = find_k1_dir(args.k1_dir, None)
        got = resolve_many(k1, pairs)
    except ResolveError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    missing = [f"{r}.{e}" for (r, e), v in got.items() if v is None]
    if args.json:
        rows = licence_report(got)
        if args.scan:
            for row in rows:
                if row["ext"] == "mdl":
                    row["contains"] = mesh_names(got[(row["resref"], "mdl")].data, args.scan)
        print(json.dumps({"k1_dir": str(k1), "items": rows, "missing": missing}, indent=2))
    else:
        for (r, e), v in got.items():
            if v is None:
                print(f"{r}.{e}: MISSING")
            else:
                flag = " FLAGGED" if is_flagged(v.licence) else ""
                print(f"{r}.{e}: {v.origin} {v.licence}{flag} sha256={v.sha256[:16]} {v.path}")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
