#!/usr/bin/env python3
"""Stage KOTOR 1 textures under short new names into a staging override/ dir outside the repo.

  python3 k1_port_textures.py --source-id ID (--pack-dir DIR | --from-install PATH) --map-json FILE
                              [--stage-dir DIR] [--pack-name NAME] [--ledger PATH]

  --source-id     registry id from sources.json (python3 k1_port_sources.py show). Required; unknown ids are refused.
  --pack-dir      already-extracted pack directory (searched recursively).
  --from-install  a KotOR 1 install, read only. Each OLD is looked up the way pykotor's Installation does
                  (override, texture packs, chitin), TPC before TGA; its bytes are staged unchanged.
  --map-json      {"N_CommM04": "k1p_commm04", ...}   OLD (game resref) -> NEW (<= 31 chars, [a-z0-9_]+)
  Pack: each OLD is looked up case-insensitively as <OLD>.tpc (fallback <OLD>.tga) anywhere under --pack-dir.
  Several matches: the shallowest path wins and a warning is printed.
  Files land at <stage>/override/<new>.<ext> (lowercase); <stage>/stage_manifest.json is merged.
  Every staged file is registered in the removal ledger (kind "texture"). The ledger path is --ledger, else
  env K1PORT_LEDGER, else the library default; it obeys the same location guard as --stage-dir.
  The run's manifest is printed to stdout as JSON.

All-or-nothing: every name, source, target and guard is checked before the first byte is written.
A failure during the copy removes the files this run wrote and their ledger entries.
Exit: 0 staged, 1 refused or failed (nothing written), 2 bad arguments.
Pykotor (patched venv, see setup_venv.sh) is imported only when a texture is read, so --help works anywhere.
Env K1PORT_LOG_DIR overrides where pykotor writes its logs/ dir (default ~/.cache/k1_port_textures/pykotor_logs).
"""
from __future__ import annotations

import argparse
import contextlib
import hashlib
import json
import logging
import os
import re
import subprocess
import sys
from pathlib import Path
from types import SimpleNamespace

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(Path(__file__).resolve().parent))  # python -I drops the script dir from sys.path
from k1_port_sources import Ledger, SourceError, get_source  # noqa: E402  stdlib only

KOTOR_REPO = Path("/home/ethansm/orca/workspaces/local-setup/KOTOR-II")
DEFAULT_STAGE = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage").expanduser()
DEFAULT_LOG_DIR = Path("~/.cache/k1_port_textures/pykotor_logs").expanduser()
MAX_NEW_LEN = 31
NEW_RE = re.compile(r"[a-z0-9_]+")
EXTS = (".tpc", ".tga")  # lookup order: .tpc first, .tga only as fallback
MANIFEST_NAME = "stage_manifest.json"
OVERLAY_MARKER = "K1_HD_OVERLAY.json"  # written by scripts/k1_hd_overlay.py at the root of a K1 HD overlay install
OVERLAY_FLAG = "--allow-overlay-install"
PROVENANCE = ("source_id", "license_class", "requires_author_approval", "removal_group", "credits")


def fail(errors: list[str], warnings: list[str] | None = None) -> int:
    for w in warnings or []:
        print(f"WARN: {w}", file=sys.stderr)
    for e in errors:
        print(f"ERROR: {e}", file=sys.stderr)
    print(f"refused: {len(errors)} problem(s); nothing written", file=sys.stderr)
    return 1


def repo_roots() -> list[Path]:
    """The hard-coded repo path plus the git toplevel of this script's dir (worktree root)."""
    roots = [KOTOR_REPO.resolve()]
    try:
        top = subprocess.run(["git", "-C", str(HERE), "rev-parse", "--show-toplevel"],
                             capture_output=True, text=True, timeout=10, check=False)
        if top.returncode == 0 and top.stdout.strip():
            roots.append(Path(top.stdout.strip()).resolve())
    except (OSError, subprocess.SubprocessError):
        pass
    return roots


def stage_refusal(stage: Path, roots: list[Path]) -> str | None:
    """Pure path check (creates nothing). Returns the refusal reason, or None if the path is allowed.
    Also guards the ledger file path."""
    resolved = stage.resolve()
    for root in roots:
        if resolved == root or root in resolved.parents:
            return f"inside the repo ({root})"
    parts = [p.lower() for p in resolved.parts]
    for a, b in zip(parts, parts[1:]):
        if (a, b) == ("steamapps", "common"):
            return "under steamapps/common (game install)"
    if "compatdata" in parts:
        return "under compatdata (Proton prefix)"
    if parts and parts[-1] == "override":
        return "final component is 'override' (a game override dir)"
    return None


def load_map(path: Path) -> dict[str, str]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict) or not all(isinstance(k, str) and isinstance(v, str) for k, v in data.items()):
        raise ValueError("map-json must be an object of string OLD -> string NEW")
    return data


def validate_names(mapping: dict[str, str], errors: list[str]) -> set[str]:
    """Collect every name problem. Returns the set of NEW names (lowercase) that passed."""
    seen_old: dict[str, str] = {}
    seen_new: dict[str, str] = {}
    ok_new: set[str] = set()
    for old, new in mapping.items():
        if not old or "/" in old or "\\" in old:
            errors.append(f"bad OLD name {old!r}: must be a bare resource name")
        if old.lower() in seen_old:
            errors.append(f"duplicate OLD (case-insensitive): {old!r} and {seen_old[old.lower()]!r}")
        seen_old.setdefault(old.lower(), old)
        bad = False
        if len(new) > MAX_NEW_LEN:
            errors.append(f"NEW {new!r} is {len(new)} chars; max is {MAX_NEW_LEN}")
            bad = True
        if not NEW_RE.fullmatch(new):
            errors.append(f"NEW {new!r} must match [a-z0-9_]+")
            bad = True
        if new.lower() in seen_new:
            errors.append(f"duplicate NEW (case-insensitive): {new!r} (OLD {old!r} and {seen_new[new.lower()]!r})")
            bad = True
        seen_new.setdefault(new.lower(), old)
        if not bad:
            ok_new.add(new.lower())
    return ok_new


def existing_stems(override: Path, errors: list[str]) -> set[str]:
    if not override.exists():
        return set()
    if not override.is_dir():
        errors.append(f"{override} exists and is not a directory")
        return set()
    return {p.stem.lower() for p in override.iterdir() if p.is_file()}


def load_manifest(path: Path, errors: list[str]) -> dict | None:
    if not path.exists():
        return {"schema": 1, "files": []}
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as e:
        errors.append(f"cannot read existing manifest {path}: {e}")
        return None
    if not isinstance(data, dict) or not isinstance(data.get("files"), list):
        errors.append(f"{path} has no 'files' list; refusing to merge into it")
        return None
    return data


def find_sources(pack: Path, olds: list[str], errors: list[str], warnings: list[str]) -> dict[str, tuple[Path, str]]:
    """OLD -> (source path, ext). One recursive walk, filtered to the wanted file names."""
    want = {f"{old.lower()}{ext}" for old in olds for ext in EXTS}
    found: dict[str, list[Path]] = {}
    for dirpath, dirnames, filenames in os.walk(pack):
        dirnames.sort()
        for fn in filenames:
            if fn.lower() in want:
                found.setdefault(fn.lower(), []).append(Path(dirpath) / fn)
    sources: dict[str, tuple[Path, str]] = {}
    for old in olds:
        for ext in EXTS:
            cands = found.get(f"{old.lower()}{ext}")
            if not cands:
                continue
            cands.sort(key=lambda p: (len(p.relative_to(pack).parts), p.as_posix()))
            if len(cands) > 1:
                shown = ", ".join(p.relative_to(pack).as_posix() for p in cands)
                warnings.append(f"{old}: {len(cands)} matches ({shown}); using {cands[0].relative_to(pack).as_posix()}")
            sources[old] = (cands[0], ext)
            break
        else:
            errors.append(f"missing source: {old} (no {old}.tpc or {old}.tga under {pack})")
    return sources


def locate_in_pack(pack: Path, olds: list[str], errors: list[str],
                   warnings: list[str]) -> dict[str, tuple[str, str, bytes]]:
    """OLD -> (path relative to the pack, ext, bytes)."""
    located: dict[str, tuple[str, str, bytes]] = {}
    for old, (path, ext) in find_sources(pack, olds, errors, warnings).items():
        try:
            data = path.read_bytes()
        except OSError as e:
            errors.append(f"cannot read source {path}: {e}")
            continue
        located[old] = (path.relative_to(pack).as_posix(), ext, data)
    return located


def import_read_tpc(errors: list[str]):
    try:
        from pykotor.resource.formats.tpc import read_tpc  # imported here so --help works without pykotor
    except Exception as e:  # noqa: BLE001
        errors.append(f"cannot import pykotor; run with the patched venv (K1PORT_PY, see setup_venv.sh): "
                      f"{type(e).__name__}: {e}")
        return None
    return read_tpc


def import_install_api(errors: list[str]):
    """pykotor pieces for --from-install. None (error appended) if pykotor cannot be imported."""
    logging.disable(logging.CRITICAL)  # pykotor logs heavily while opening an Installation
    try:
        from pykotor.common.misc import Game
        from pykotor.extract.installation import Installation
        from pykotor.resource.type import ResourceType
    except Exception as e:  # noqa: BLE001
        errors.append(f"cannot import pykotor; run with the patched venv (K1PORT_PY, see setup_venv.sh): "
                      f"{type(e).__name__}: {e}")
        return None
    return SimpleNamespace(Game=Game, Installation=Installation, ResourceType=ResourceType)


def open_install(path: Path, api, errors: list[str]):
    """A read-only handle on a KotOR 1 install, or None (error appended)."""
    try:
        inst = api.Installation(str(path))
    except Exception as e:  # noqa: BLE001  pykotor raises several types for a bad install dir
        errors.append(f"cannot open install {path}: {type(e).__name__}: {e}")
        return None
    if inst.game() != api.Game.K1:
        errors.append(f"{path} is not a KotOR 1 install (game={inst.game()})")
        return None
    return inst


def overlay_marker(install: Path) -> dict | None:
    """The K1_HD_OVERLAY.json of an overlay install ({} if unreadable), None for a plain install."""
    marker = install.expanduser() / OVERLAY_MARKER
    if not marker.is_file():
        return None
    try:
        data = json.loads(marker.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return {}
    return data if isinstance(data, dict) else {}


def overlay_refusal(install: Path, allowed: bool, flag_for: str = "--install-k1") -> str | None:
    """Reason to refuse `install` (an overlay holds flagged CR files that would look like vanilla), else None."""
    if allowed or overlay_marker(install) is None:
        return None
    return (f"{flag_for} {install} is a K1 HD overlay ({OVERLAY_MARKER}): its override/ holds flagged "
            f"Characters Revisited files that would be recorded as vanilla_k1. Pass {OVERLAY_FLAG} to read it, "
            "in which case those override/ files are attributed to the marker's source.")


def is_override_label(label: str) -> bool:
    return label.replace("\\", "/").lower().startswith("override/")


def overlay_source_id(marker: dict) -> str:
    return str(marker.get("source_id") or "characters_revisited_main")


def install_label(path: Path, root: Path) -> str:
    """Where the resource lives, relative to the install root when it is inside it."""
    try:
        return path.resolve().relative_to(root).as_posix()
    except (OSError, ValueError):
        return str(path)


@contextlib.contextmanager
def pykotor_cwd():
    """pykotor creates logs/ in the CWD on first use; keep that out of the repo and the stage dir."""
    log_dir = Path(os.environ.get("K1PORT_LOG_DIR") or DEFAULT_LOG_DIR)
    log_dir.mkdir(parents=True, exist_ok=True)
    prev = os.getcwd()
    os.chdir(log_dir)
    try:
        yield
    finally:
        os.chdir(prev)


def locate_in_install(install: Path, olds: list[str], errors: list[str]) -> dict[str, tuple[str, str, bytes]]:
    """OLD -> (install-relative label, ext, bytes). Same search as pykotor's texture_resource_result: override,
    texture packs, chitin. TPC wins over TGA. Read only; pykotor stdout goes to stderr so stdout stays JSON."""
    api = import_install_api(errors)
    if api is None:
        return {}
    try:
        with contextlib.redirect_stdout(sys.stderr), pykotor_cwd():
            inst = open_install(install, api, errors)
            if inst is None:
                return {}
            root = install.resolve()
            located: dict[str, tuple[str, str, bytes]] = {}
            for old in olds:
                result, _txi = inst.texture_resource_result(old)
                if result is None:
                    errors.append(f"missing source: {old} (no {old}.tpc or {old}.tga in {install})")
                    continue
                ext = ".tga" if result.restype == api.ResourceType.TGA else ".tpc"
                located[old] = (install_label(result.filepath, root), ext, bytes(result.data))
            return located
    except OSError as e:
        errors.append(f"cannot prepare pykotor log dir: {type(e).__name__}: {e}")
        return {}


def read_texture(read_tpc, data: bytes, label: str, errors: list[str]):
    """Validates texture bytes. Returns (format name, width, height) or None (error appended)."""
    try:
        tpc = read_tpc(data)
        width, height = tpc.dimensions()
        fmt = tpc.format()
    except Exception as e:  # noqa: BLE001  pykotor raises a mix of ValueError/IndexError/struct.error
        errors.append(f"unreadable texture {label}: {type(e).__name__}: {e}")
        return None
    if width <= 0 or height <= 0:
        errors.append(f"texture {label} has no pixels ({width}x{height})")
        return None
    return getattr(fmt, "name", str(fmt)), width, height


def open_ledger(path_arg: Path | None, roots: list[Path], errors: list[str]) -> Ledger | None:
    """The removal ledger, or None (error appended). Its path obeys the same guard as --stage-dir."""
    try:
        led = Ledger(path_arg)  # None -> env K1PORT_LEDGER, else the library default
    except (SourceError, OSError) as e:
        errors.append(f"cannot open ledger: {e}")
        return None
    where = led.path.absolute()
    reason = stage_refusal(where, roots)
    if reason:
        errors.append(f"refused --ledger {where}: {reason}")
        return None
    return led


def commit(led: Ledger, stage: Path, override: Path, manifest_path: Path, manifest: dict,
           staged: list[tuple[Path, bytes]], new_entries: list[dict]) -> None:
    """Exclusive-create every target, save the ledger once, then merge the manifest.
    On any failure, remove the files this batch wrote and drop its ledger entries, then re-raise.
    So a staged file is either registered in the ledger or absent."""
    written: list[Path] = []
    saved = False
    try:
        stage.mkdir(parents=True, exist_ok=True)
        override.mkdir(parents=True, exist_ok=True)
        for target, data in staged:
            with open(target, "xb") as fh:
                fh.write(data)
            written.append(target)
        led.save()
        saved = True
        manifest["files"].extend(new_entries)
        tmp = manifest_path.parent / (manifest_path.name + ".tmp")
        tmp.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        os.replace(tmp, manifest_path)
    except BaseException:
        for p in written:
            p.unlink(missing_ok=True)
        for target, _data in staged:
            led.remove(target)
        if saved:
            # A stale entry for a missing file is harmless: removal finds nothing to delete.
            with contextlib.suppress(OSError):
                led.save()
        raise


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description="Stage KOTOR 1 textures under new short names into <stage>/override/.")
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--pack-dir", type=Path, help="already-extracted pack directory (searched recursively)")
    src.add_argument("--from-install", type=Path, help="KotOR 1 install root, read only")
    ap.add_argument("--source-id", required=True, help="registry id from sources.json; unknown ids are refused")
    ap.add_argument(OVERLAY_FLAG, action="store_true",
                    help="allow --from-install to be a K1 HD overlay; its override/ files are then ledgered under "
                         "the overlay marker's source (flagged) instead of --source-id")
    ap.add_argument("--map-json", required=True, type=Path, help='JSON object {"OLD": "new_name", ...}')
    ap.add_argument("--stage-dir", type=Path, default=DEFAULT_STAGE, help=f"default: {DEFAULT_STAGE}")
    ap.add_argument("--pack-name", help="source_pack label for the manifest (default: pack or install dir name)")
    ap.add_argument("--ledger", type=Path,
                    help="removal ledger JSON (default: env K1PORT_LEDGER, else the library default)")
    return ap


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)

    errors: list[str] = []
    warnings: list[str] = []
    stage = args.stage_dir.expanduser().absolute()
    roots = repo_roots()
    reason = stage_refusal(stage, roots)
    if reason:
        return fail([f"refused --stage-dir {stage}: {reason}"])

    try:
        get_source(args.source_id)
    except SourceError as e:
        errors.append(str(e))
    led = open_ledger(args.ledger, roots, errors)

    is_pack = args.pack_dir is not None
    src_dir = (args.pack_dir if is_pack else args.from_install).expanduser().resolve()
    marker = None
    if not is_pack:
        marker = overlay_marker(src_dir)
        reason = overlay_refusal(src_dir, args.allow_overlay_install, "--from-install")
        if reason:
            return fail([reason])
        if marker is not None:
            try:
                get_source(overlay_source_id(marker))
            except SourceError as e:
                errors.append(str(e))
    if not src_dir.is_dir():
        errors.append(f"{'--pack-dir' if is_pack else '--from-install'} is not a directory: {src_dir}")

    mapping: dict[str, str] = {}
    try:
        mapping = load_map(args.map_json.expanduser())
    except (OSError, ValueError) as e:
        errors.append(f"cannot load --map-json {args.map_json}: {e}")
    else:
        if not mapping:
            errors.append("--map-json is empty")
    if errors:
        return fail(errors, warnings)

    ok_new = validate_names(mapping, errors)
    override = stage / "override"
    manifest_path = stage / MANIFEST_NAME
    manifest = load_manifest(manifest_path, errors)
    taken = existing_stems(override, errors)
    olds = list(mapping)
    if is_pack:
        located = locate_in_pack(src_dir, olds, errors, warnings)
    else:
        located = locate_in_install(src_dir, olds, errors)

    pack_name = args.pack_name or src_dir.name
    staged: list[tuple[Path, bytes]] = []
    new_entries: list[dict] = []
    if located:
        read_tpc = import_read_tpc(errors)
        if read_tpc is not None:
            try:
                with pykotor_cwd():
                    for old, new in mapping.items():
                        if old not in located:
                            continue
                        rel, ext, data = located[old]
                        got = read_texture(read_tpc, data, rel, errors)
                        if got is None or new.lower() not in ok_new:
                            continue  # unreadable (reported) or bad NEW (reported by validate_names)
                        if new.lower() in taken:
                            errors.append(f"target exists: {new.lower()}{ext} (stage override/ already has {new.lower()}.*)")
                            continue
                        fmt_name, width, height = got
                        target = override / f"{new.lower()}{ext}"
                        staged.append((target, data))
                        entry_sid = (overlay_source_id(marker)
                                     if marker is not None and is_override_label(rel) else args.source_id)
                        new_entries.append({
                            "_source_id": entry_sid,
                            "source_pack": pack_name,
                            "source_rel_path": rel,
                            "sha256": hashlib.sha256(data).hexdigest(),
                            "old": old,
                            "new": new,
                            "format": fmt_name,
                            "width": width,
                            "height": height,
                            "bytes": len(data),
                            "staged_rel_path": f"override/{target.name}",
                        })
            except OSError as e:
                errors.append(f"cannot prepare pykotor log dir: {type(e).__name__}: {e}")
    if errors or manifest is None or led is None:
        return fail(errors, warnings)

    for (target, _data), entry in zip(staged, new_entries):
        led_entry = led.add(str(target), "texture", entry.pop("_source_id"), sha256=entry["sha256"],
                            old_name=entry["old"], new_name=entry["new"],
                            width=entry["width"], height=entry["height"])
        entry.update({k: led_entry[k] for k in PROVENANCE})

    try:
        commit(led, stage, override, manifest_path, manifest, staged, new_entries)
    except OSError as e:
        return fail([f"write failed, batch rolled back: {type(e).__name__}: {e}"], warnings)

    for w in warnings:
        print(f"WARN: {w}", file=sys.stderr)
    run = {
        "pack_name": pack_name,
        "source_id": args.source_id,
        "pack_dir": str(src_dir) if is_pack else None,
        "install_dir": None if is_pack else str(src_dir),
        "stage_dir": str(stage),
        "override_dir": str(override),
        "manifest": str(manifest_path),
        "ledger": str(led.path.absolute()),
        "warnings": warnings,
        "files": new_entries,
    }
    print(json.dumps(run, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
