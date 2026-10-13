#!/usr/bin/env python3
"""kvoice stage writer: pack one clip into a stage and save the stage ledger.

Stage layout (flat, like k1_port):
  <stage>/Override/<resref>.wav   58-byte VO container (container 'vo58') or bare MP3 (container 'bare')
  <stage>/Override/<resref>.lip   LIP V1.0 bytes from kvoice_lip.write_lip
  <stage>/manifest.json           {"schema": 1, "files": {"Override/<file>": sha256}}
  <stage>/ledger.json             k1_port_sources schema 1 (write_ledger)

pack_clip and write_ledger refuse, before any write, a stage (or Override) that resolves (readlink -f)
inside the K2 game dir, and any resref that is not kpv-prefixed [a-z0-9_]{1,16}. The game dir is never
read or written by this module. Stdlib plus kvoice_container, kvoice_lip and kvoice_ledger.
"""
import hashlib
import json
import os
import re
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))
import kvoice_container as kc  # noqa: E402
import kvoice_lip as L  # noqa: E402
import kvoice_ledger as KL  # noqa: E402  (also puts k1_port on sys.path)

ks = KL.ks  # k1_port_sources: SCHEMA, Ledger

MANIFEST_SCHEMA = 1
CONTAINERS = ("vo58", "bare")
GAME_DIR_DEFAULT = "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
RESREF_RE = re.compile(r"kpv[a-z0-9_]{0,13}")  # use fullmatch: 3-16 chars with the kpv prefix


class PackError(ValueError):
    """Stage refused, resref rejected, or clip/lip/ledger rejected. Raised before any write."""


def game_dirs():
    """Dirs to refuse: the default install path, plus KVOICE_K2_DIR when that is set."""
    dirs = {Path(GAME_DIR_DEFAULT).expanduser()}
    env = os.environ.get("KVOICE_K2_DIR")
    if env:
        dirs.add(Path(env).expanduser())
    return [d.resolve() for d in dirs]


def refuse_game_dir(path) -> Path:
    """Return path resolved (readlink -f). Raises PackError when it is the game dir or inside it."""
    real = Path(path).expanduser().resolve()
    for root in game_dirs():
        if real == root or root in real.parents:
            raise PackError("refusing a stage inside the K2 game dir: %s" % real)
    return real


def check_resref(resref) -> str:
    if not isinstance(resref, str) or not RESREF_RE.fullmatch(resref):
        raise PackError("resref %r must be kpv-prefixed [a-z0-9_]{1,16}" % (resref,))
    return resref


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _atomic_write(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=str(path.parent), prefix="." + path.name + ".", suffix=".tmp")
    try:
        with os.fdopen(fd, "wb") as fh:
            fh.write(data)
            fh.flush()
            os.fsync(fh.fileno())
        os.replace(tmp, str(path))
    except BaseException:
        try:
            os.unlink(tmp)
        except FileNotFoundError:
            pass
        raise


def read_manifest(stage) -> dict:
    """Load <stage>/manifest.json, or an empty schema-1 manifest when the file does not exist yet."""
    p = Path(stage) / "manifest.json"
    if not p.exists():
        return {"schema": MANIFEST_SCHEMA, "files": {}}
    try:
        raw = json.loads(p.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        raise PackError("manifest.json unreadable: %s" % exc) from exc
    if not isinstance(raw, dict) or not isinstance(raw.get("files"), dict):
        raise PackError("manifest.json must hold a 'files' object")
    return raw


def pack_clip(stage_dir, resref, mp3_payload, lip_bytes, container="vo58") -> dict:
    """Write <stage>/Override/<resref>.wav and .lip and record both sha256 in <stage>/manifest.json.

    container 'vo58': mp3_payload is a bare MP3 chain (32 kHz mono 48 kbps); it is wrapped in the 58-byte header.
    container 'bare': mp3_payload is written unchanged (the E2 matrix; verify_wav reports it as R1).
    Returns {"wav": path, "lip": path, "sha256": {"Override/<resref>.wav": sha, "Override/<resref>.lip": sha}}.
    Raises PackError with nothing written when the stage is in the game dir, the resref is not kpv-prefixed,
    the container is unknown, or the clip or lip is rejected.
    """
    check_resref(resref)
    if container not in CONTAINERS:
        raise PackError("container must be one of %s, got %r" % (CONTAINERS, container))
    stage = refuse_game_dir(stage_dir)
    override = stage / "Override"
    if override.is_symlink():
        raise PackError("refusing a symlinked Override in the stage: %s" % override)
    refuse_game_dir(override)
    lip = bytes(lip_bytes)
    lip_problems = L.check_lip(lip)
    if lip_problems:
        raise PackError("lip rejected: " + "; ".join(lip_problems))
    payload = bytes(mp3_payload)
    if container == "vo58":
        try:
            wav = kc.wrap(payload)
        except kc.ContainerError as exc:
            raise PackError("clip rejected: %s" % exc) from exc
    else:
        if kc.frame_header(payload, 0) is None:
            raise PackError("bare clip has no MPEG-1 L3 frame at offset 0")
        wav = payload
    manifest = read_manifest(stage)  # read before any write, so a bad manifest refuses cleanly
    wav_rel, lip_rel = "Override/" + resref + ".wav", "Override/" + resref + ".lip"
    _atomic_write(override / (resref + ".wav"), wav)
    _atomic_write(override / (resref + ".lip"), lip)
    shas = {wav_rel: sha256_bytes(wav), lip_rel: sha256_bytes(lip)}
    manifest["schema"] = MANIFEST_SCHEMA
    manifest["files"].update(shas)
    _atomic_write(stage / "manifest.json",
                  (json.dumps(manifest, indent=2, sort_keys=True) + "\n").encode("utf-8"))
    return {"wav": str(override / (resref + ".wav")), "lip": str(override / (resref + ".lip")), "sha256": shas}


def write_ledger(stage_dir, ledger_obj) -> Path:
    """Save <stage>/ledger.json (atomic). ledger_obj is a schema-1 dict or a k1_port_sources.Ledger.

    Pass the stage's own ledger (rows for this stage's files), not the global removal ledger.
    Nothing is written when the object is not a schema-1 ledger or the stage is in the game dir.
    """
    data = ledger_obj._data if isinstance(ledger_obj, ks.Ledger) else ledger_obj
    if not isinstance(data, dict) or data.get("schema") != ks.SCHEMA or not isinstance(data.get("entries"), list):
        raise PackError("ledger must be schema %d with an 'entries' list" % ks.SCHEMA)
    out = dict(data)
    out.setdefault("approvals", {})
    stage = refuse_game_dir(stage_dir)
    path = stage / "ledger.json"
    _atomic_write(path, (json.dumps(out, indent=2, sort_keys=True) + "\n").encode("utf-8"))
    return path
