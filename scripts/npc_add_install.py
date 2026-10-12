#!/usr/bin/env python3
"""Install the NPC-add build (204tel.mod) into <game>/Modules/, with a backup and a rollback.

  python3 scripts/npc_add_install.py                       dry run: print the plan, write nothing (default)
  python3 scripts/npc_add_install.py --apply               back up the live .mod, then replace it
  python3 scripts/npc_add_install.py --rollback latest     restore the original from the newest backup
  python3 scripts/npc_add_install.py --rollback <stamp>    restore a named backup (YYYYMMDD_HHMMSS or npc_add_<stamp>)

Backup: <game>/backups/npc_add_<stamp>/ holds 204tel.mod (the original bytes) and manifest.json (original and
installed sha256). The manifest is written last, so a backup dir without one is incomplete and ignored.

Refused (exit 1): KOTOR II running (swkotor2.exe), missing or non-MOD source, live .mod missing, live .mod not the
original (apply, while a backup is outstanding), live .mod not the installed build (rollback: hash drift), unknown
or malformed stamp, corrupt backup. A failed write puts the original bytes back before exiting.
Out of scope: override/ UTCs (embedded in the .mod) and replay/replay_log.jsonl.
"""
import argparse
import contextlib
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
MODULE = "204tel.mod"
DEFAULT_GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
DEFAULT_SOURCE = REPO / "patch_manager_mods" / "npc_add_src" / "out" / "modules" / MODULE
BACKUP_PREFIX = "npc_add_"
MANIFEST_NAME = "manifest.json"
STAMP_RE = re.compile(r"^\d{8}_\d{6}$")
MOD_MAGIC = (b"MOD ", b"ERF ")


class Refused(Exception):
    """A precondition failed. Nothing was left changed."""


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def game_running() -> bool:
    """Same probe as scripts/kpm_status.py and kpm_hotswap.py. Fails closed: no pgrep counts as running."""
    try:
        return subprocess.run(["pgrep", "-f", r"^S:.*swkotor2\.exe"], stdout=subprocess.DEVNULL).returncode == 0
    except OSError:
        return True


def live_path(game: Path) -> Path:
    return game / "Modules" / MODULE


def backups_root(game: Path) -> Path:
    return game / "backups"


def atomic_write(dst: Path, data: bytes) -> None:
    """Write a temp file in the same directory, fsync it, then os.replace it over dst."""
    tmp = dst.with_name(dst.name + ".npcadd.tmp")
    try:
        with open(tmp, "wb") as f:
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp, dst)
    except BaseException:
        with contextlib.suppress(OSError):
            tmp.unlink()
        raise


def _is_stamp_dir(d: Path) -> bool:
    return d.is_dir() and d.name.startswith(BACKUP_PREFIX) and bool(STAMP_RE.match(d.name[len(BACKUP_PREFIX):]))


def backup_dirs(game: Path) -> list:
    """Complete backups (stamp-named dir with a manifest), oldest first. Stamps sort chronologically by name."""
    root = backups_root(game)
    if not root.is_dir():
        return []
    return sorted((d for d in root.iterdir() if _is_stamp_dir(d) and (d / MANIFEST_NAME).is_file()),
                  key=lambda d: d.name)


def load_backup(d: Path) -> dict:
    try:
        m = json.loads((d / MANIFEST_NAME).read_text())
    except (OSError, ValueError) as e:
        raise Refused(f"{d.name}: manifest unreadable ({e})")
    if not isinstance(m, dict):
        raise Refused(f"{d.name}: manifest is not a JSON object")
    for key in ("stamp", "module", "backup_file", "original_sha256", "installed_sha256"):
        if not isinstance(m.get(key), str):
            raise Refused(f"{d.name}: manifest has no '{key}'")
    if m["module"] != MODULE or m["backup_file"] != MODULE:
        raise Refused(f"{d.name}: manifest is for '{m['module']}', not {MODULE}")
    return m


def resolve_backup(game: Path, target: str):
    """'latest' or a stamp (YYYYMMDD_HHMMSS, optionally prefixed npc_add_). Returns (dir, manifest)."""
    if target == "latest":
        dirs = backup_dirs(game)
        if not dirs:
            raise Refused(f"no complete {BACKUP_PREFIX}* backup in {backups_root(game)}")
        d = dirs[-1]
    else:
        stamp = target[len(BACKUP_PREFIX):] if target.startswith(BACKUP_PREFIX) else target
        if not STAMP_RE.match(stamp):
            raise Refused(f"unknown stamp '{target}' (use latest or YYYYMMDD_HHMMSS)")
        d = backups_root(game) / f"{BACKUP_PREFIX}{stamp}"
        if not (d / MANIFEST_NAME).is_file():
            raise Refused(f"unknown stamp '{target}': no complete backup {d}")
    return d, load_backup(d)


def make_plan(game: Path, source: Path, running: bool) -> dict:
    """Everything --apply would do, checked without writing. plan['blockers'] lists the refusals."""
    plan = {"game": game, "source": source, "live": live_path(game), "running": running, "blockers": [],
            "source_sha": None, "source_bytes": None, "live_sha": None, "latest": None, "noop": False}
    if running:
        plan["blockers"].append("KOTOR II is running (swkotor2.exe); close it first")
    if not source.is_file():
        plan["blockers"].append(f"missing source {source} (build it first)")
    else:
        data = source.read_bytes()
        plan["source_bytes"], plan["source_sha"] = data, sha256_bytes(data)
        if data[:4] not in MOD_MAGIC:
            plan["blockers"].append(f"source is not an ERF/MOD file: {source}")
    live = plan["live"]
    if not live.is_file():
        plan["blockers"].append(f"live target missing: {live}")
    else:
        plan["live_sha"] = sha256_file(live)
    dirs = backup_dirs(game)
    if dirs:
        d = dirs[-1]
        m = load_backup(d)
        plan["latest"] = d.name
        if plan["live_sha"] == m["installed_sha256"]:
            plan["blockers"].append(f"already applied by {d.name}; roll back first: --rollback {d.name}")
        elif plan["live_sha"] is not None and plan["live_sha"] != m["original_sha256"]:
            plan["blockers"].append(f"live {MODULE} matches neither the original nor the installed build of "
                                    f"{d.name}; inspect it before anything else")
    if not plan["blockers"] and plan["live_sha"] == plan["source_sha"]:
        plan["noop"] = True
    return plan


def print_plan(plan: dict) -> None:
    game = plan["game"]
    print("DRY RUN: nothing written. Add --apply to install.")
    print(f"  source        {plan['source']}  sha256 {plan['source_sha'] or 'MISSING'}")
    print(f"  live target   {plan['live']}  sha256 {plan['live_sha'] or 'MISSING'}")
    print(f"  backup dir    {backups_root(game)}/{BACKUP_PREFIX}<stamp>/  (stamp taken when --apply runs)")
    print(f"  game running  {'yes' if plan['running'] else 'no'}")
    print(f"  latest backup {plan['latest'] or 'none'}")
    if plan["blockers"]:
        print("  result        BLOCKED: " + "; ".join(plan["blockers"]))
    elif plan["noop"]:
        print("  result        no-op: live .mod already equals the source")
    else:
        print("  result        would apply (backup, then replace the live .mod)")


def _put_back(live: Path, data: bytes) -> bool:
    """Undo: make the live file hold data again. True when the bytes are verified in place."""
    try:
        try:
            cur = live.read_bytes()
        except OSError:
            cur = None
        if cur != data:
            atomic_write(live, data)
        return live.read_bytes() == data
    except OSError:
        return False


def apply_install(game: Path, source: Path, running: bool, stamp: str = None) -> int:
    plan = make_plan(game, source, running)
    if plan["blockers"]:
        raise Refused("; ".join(plan["blockers"]))
    if plan["noop"]:
        print(f"NO-OP: {plan['live']} already equals {source} (sha256 {plan['source_sha']}); nothing written")
        return 0
    stamp = stamp or datetime.now().strftime("%Y%m%d_%H%M%S")
    bdir = backups_root(game) / f"{BACKUP_PREFIX}{stamp}"
    if bdir.exists():
        raise Refused(f"backup dir {bdir} already exists (stamp collision); retry in a second")
    live = plan["live"]
    original = live.read_bytes()
    orig_sha, installed_sha = sha256_bytes(original), plan["source_sha"]
    bdir.mkdir(parents=True)
    try:
        (bdir / MODULE).write_bytes(original)
        if sha256_file(bdir / MODULE) != orig_sha:
            raise OSError("backup copy sha256 differs from the live file")
        atomic_write(live, plan["source_bytes"])
        if sha256_file(live) != installed_sha:
            raise OSError("written sha256 differs from the source")
        manifest = {"tool": "npc_add_install", "stamp": stamp, "module": MODULE, "backup_file": MODULE,
                    "source": str(source.resolve()), "original_sha256": orig_sha, "original_bytes": len(original),
                    "installed_sha256": installed_sha, "installed_bytes": len(plan["source_bytes"])}
        (bdir / MANIFEST_NAME).write_text(json.dumps(manifest, indent=2) + "\n")
    except Exception as e:
        restored = _put_back(live, original)
        if restored:
            shutil.rmtree(bdir, ignore_errors=True)
            detail = "live .mod restored to the original (sha256 verified)"
        else:
            detail = f"LIVE .MOD NOT RESTORED: copy {bdir / MODULE} over {live} by hand"
        raise Refused(f"apply failed: {e}; {detail}")
    print(f"APPLIED {MODULE}: sha256 {installed_sha}")
    print(f"  backup    {bdir}  (original sha256 {orig_sha})")
    print(f"  rollback  npc_add_install.py --game-dir {game} --rollback {stamp}")
    return 0


def rollback_install(game: Path, target: str, running: bool) -> int:
    if running:
        raise Refused("KOTOR II is running (swkotor2.exe); close it first")
    d, m = resolve_backup(game, target)
    backup = d / m["backup_file"]
    try:
        original = backup.read_bytes()
    except OSError:
        raise Refused(f"backup file missing: {backup}; nothing restored")
    if sha256_bytes(original) != m["original_sha256"]:
        raise Refused(f"backup {backup} is corrupt (sha256 differs from the manifest); nothing restored")
    live = live_path(game)
    try:
        current = live.read_bytes()
    except OSError:
        raise Refused(f"live target missing: {live}; nothing restored")
    cur_sha = sha256_bytes(current)
    if cur_sha != m["installed_sha256"]:
        raise Refused(f"hash drift: live {MODULE} sha256 {cur_sha[:12]} is not the build {d.name} installed "
                      f"({m['installed_sha256'][:12]}); nothing restored")
    try:
        atomic_write(live, original)
        if sha256_file(live) != m["original_sha256"]:
            raise OSError("restored sha256 differs from the manifest")
    except Exception as e:
        kept = _put_back(live, current)
        detail = "live .mod kept as the installed build (sha256 verified)" if kept else \
            f"LIVE .MOD NOT RESTORED: copy {backup} over {live} by hand"
        raise Refused(f"rollback failed: {e}; {detail}")
    print(f"RESTORED {MODULE} from {d.name}: sha256 {m['original_sha256']}")
    return 0


def main(argv=None, running=None) -> int:
    ap = argparse.ArgumentParser(description="Install or roll back the NPC-add 204tel.mod. Default: dry run.")
    ap.add_argument("--apply", action="store_true", help="back up the live .mod and install the source")
    ap.add_argument("--rollback", metavar="latest|STAMP", help="restore the original .mod from a backup")
    ap.add_argument("--game-dir", "--game", dest="game_dir", type=Path, default=DEFAULT_GAME, help=f"game root (default {DEFAULT_GAME})")
    ap.add_argument("--source", type=Path, default=DEFAULT_SOURCE, help="built .mod to install")
    a = ap.parse_args(argv)
    if a.apply and a.rollback:
        ap.error("--apply and --rollback are exclusive")
    probe = running if running is not None else game_running
    game, source = a.game_dir.expanduser(), a.source.expanduser()
    try:
        if a.rollback:
            return rollback_install(game, a.rollback, probe())
        if a.apply:
            return apply_install(game, source, probe())
        plan = make_plan(game, source, probe())
        print_plan(plan)
        return 1 if plan["blockers"] else 0
    except Refused as e:
        print(f"REFUSED: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
