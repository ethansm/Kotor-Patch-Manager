#!/usr/bin/env python3
"""Safe full hook reinstall (doc 24 H10 / doc 36): snapshot, stage kpatches flat, run KPatchLauncher, restore peer DLLs, verify, roll back on failure.

Usage:
  kpm_reinstall.py --mod MOD [MOD ...] [--dry-run]    reinstall (targets must read exactly REINSTALL in kpm_status; game closed)
  kpm_reinstall.py --rollback <backup dir|name|latest>  restore patches/ and both config files from a reinstall snapshot
Exit: 0 ok, 1 refused or failed, 2 usage.

A reinstall uninstalls and reinstalls EVERY live patch (the launcher drops any id it is not given), so it needs the user's GO and
the game closed. Peer mods whose live DLL was hot-swapped or built outside their kpatch get their DLL restored afterwards. The
config check is warn-only: this tool never edits patch_config.toml. Any launcher/verify failure rolls back automatically.
"""
import argparse
import datetime
import json
import os
import pathlib
import shlex
import shutil
import subprocess
import sys
import tomllib
import zipfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import kpm_commit as kc  # noqa: E402
import kpm_hotswap as hs  # noqa: E402
import kpm_ship  # noqa: E402
import kpm_status as ks  # noqa: E402

MANIFEST = "reinstall_manifest.json"
KEEP_KEYS = {"hooks", "dll", "id", "replacement_bytes"}
NEXT_STEP = {
    "SWAP": "a DLL hot-swap is enough: kpm_hotswap.py --mod {id} --apply",
    "DIRTY": "commit first (kpm_ship.py --mod {id} --prepare)",
    "MIRROR": "mirror/rebuild first (kpm_ship.py --mod {id} --prepare)",
    "REBUILD": "rebuild first (kpm_ship.py --mod {id} --prepare)",
    "NOT-LIVE": "a new id: build its kpatch in KPM first, then it is a valid target",
    "LIVE": "already live; nothing to do",
}


def launcher():
    return hs.KPM / "bin/NoLaunch/KPatchLauncher"


def run_launcher(cmd, env):
    """The single subprocess point (mocked in tests); output streams to the terminal."""
    return subprocess.run(cmd, env=env).returncode


def kpatch_hooks(kpatch):
    """Normalised Steam hooks of a kpatch, read the way kpm_status.status does."""
    with zipfile.ZipFile(kpatch) as z:
        name = ks.steam_hooks_name(z.namelist())
        raw = z.read(name) if name else b""
    return ks.norm_hooks(tomllib.loads(raw.decode()).get("hooks", []) if name else [])


def write_manifest(backup, meta):
    (backup / MANIFEST).write_text(json.dumps(meta, indent=1))


def set_state(backup, state):
    meta = json.loads((backup / MANIFEST).read_text())
    meta["state"] = state
    write_manifest(backup, meta)


def find_backup(target):
    backups = hs.GAME / "backups"
    if target == "latest":
        dirs = sorted(backups.glob("*_reinstall_*"), key=lambda d: d.stat().st_mtime, reverse=True)
        return dirs[0] if dirs else None
    snap = pathlib.Path(target)
    if not snap.is_dir():
        snap = backups / target
    return snap if snap.is_dir() else None


def do_rollback(target, check_running=True):
    """Restore patches/ and both config files from a reinstall snapshot. Config is restored unconditionally (full snapshot)."""
    if check_running and hs.is_game_running():
        print("REFUSED: KOTOR II is running; close swkotor2.exe first")
        return 1
    snap = target if isinstance(target, pathlib.Path) else find_backup(target)
    if snap is None:
        print(f"REFUSED: no reinstall backup '{target}' in {hs.GAME / 'backups'}")
        return 1
    if not (snap / MANIFEST).is_file():
        print(f"REFUSED: {snap} has no {MANIFEST} (not a kpm_reinstall snapshot)")
        return 1
    print(f"Rolling back {snap}")
    changed = []
    for src in sorted((snap / "patches").rglob("*")):
        if not src.is_file():
            continue
        dst = hs.GAME / "patches" / src.relative_to(snap / "patches")
        dst.parent.mkdir(parents=True, exist_ok=True)
        before = hs.file_md5(dst)
        if src.suffix == ".dll":
            hs.write_dll(dst, src.read_bytes())
            if before != hs.file_md5(dst):
                changed.append((dst.name, before, hs.file_md5(dst)))
        else:
            shutil.copy2(src, dst)
    for name in hs.CONFIG_FILES:
        if (snap / name).is_file():
            shutil.copy2(snap / name, hs.GAME / name)
    for name, old, new in changed:
        print(f"  {name}: {old[:8] or '-'} -> {new[:8]}")
    print(f"  {len(changed)} DLL(s) changed back; config files restored from the snapshot")
    return 0


def print_plan(targets, backup, stage, cmd, ids, peers):
    print(f"targets: {', '.join(targets)}")
    print(f"backup dir: {backup}")
    print(f"stage dir: {stage}")
    print(f"launcher: KPATCH_NO_LAUNCH=1 {shlex.join(cmd)}")
    print(f"ids: {len(ids)}")
    if peers:
        print("peer DLLs to restore after the launcher run:")
        for p in peers:
            print(f"  {p['id']}: {p['dll']} (md5 {p['md5'][:8]})")
    else:
        print("peer DLLs to restore: none")


def config_warnings(old_path, new_path):
    """Warn-only check that the launcher did not drop config keys it does not own."""
    try:
        old = tomllib.loads(old_path.read_text())
        new = tomllib.loads(new_path.read_text())
    except (OSError, tomllib.TOMLDecodeError) as e:
        return [f"WARNING: could not compare patch_config.toml ({e})"]
    warns = [f"WARNING: launcher dropped top-level key '{k}'" for k in old if k != "patches" and k not in new]
    new_by_id = {p.get("id"): p for p in new.get("patches", [])}
    for p in old.get("patches", []):
        np_ = new_by_id.get(p.get("id"))
        if np_ is None:
            continue
        warns += [f"WARNING: launcher dropped key '{k}' of patch {p.get('id')}" for k in p if k not in KEEP_KEYS and k not in np_]
    return warns


def reinstall(a):
    game = hs.GAME
    selected, bad, dirs = kc.resolve_mods(a.mod)
    if bad:
        print(f"usage error: unknown mod(s) {', '.join(bad)}; valid: {', '.join(dirs)}")
        return 2
    wanted = set(selected)
    trows = [r for r in kpm_ship.load_rows() if kpm_ship.src_dir(r) in wanted]
    missing = wanted - {kpm_ship.src_dir(r) for r in trows}
    if missing:
        print(f"no status row for {', '.join(sorted(missing))}; nothing touched")
        return 1
    refused = [r for r in trows if r["verdict"] not in ("REINSTALL", "NOT-LIVE")]  # NOT-LIVE = a new id; needs a built kpatch in KPM (checked below)
    for r in refused:
        print(f"REFUSED {r['id']}: {r['verdict']}: {NEXT_STEP.get(r['verdict'], 'not a REINSTALL target').format(id=r['id'])}")
    if refused:
        return 1
    targets = [r["id"] for r in trows]
    if hs.is_game_running():
        print("REFUSED: KOTOR II is running; close swkotor2.exe first")
        return 1
    if not launcher().is_file():
        print(f"REFUSED: launcher not found at {launcher()}")
        return 1

    # preflight: every live id needs a kpatch (the launcher drops ids it is not given); non-target peers must have matching hooks
    live = hs.load_live(game)
    kmap = ks.kpm_dirs(hs.KPM)
    kpatch, lost = {}, []
    for pid in live:
        kps = sorted(kmap[pid].glob("*.kpatch")) if pid in kmap else []
        if kps:
            kpatch[pid] = kps[0]
        else:
            lost.append(pid)
    if lost:
        print(f"REFUSED: no kpatch in KPM for live id(s) {', '.join(lost)}; the launcher would uninstall them")
        return 1
    newids = [t for t in targets if t not in live]
    nokp = [t for t in newids if not (t in kmap and sorted(kmap[t].glob("*.kpatch")))]
    if nokp:
        print(f"REFUSED: new id(s) {', '.join(nokp)} have no built kpatch in KPM; run kpm_hotswap.prepare for them first")
        return 1
    for pid in newids:
        kpatch[pid] = sorted(kmap[pid].glob("*.kpatch"))[0]
    peers = []
    for pid, p in live.items():
        if pid in targets or not p.get("dll"):
            continue
        if kpatch_hooks(kpatch[pid]) != ks.norm_hooks(p.get("hooks", [])):
            print(f"REFUSED: peer {pid} also has changed hooks; add it to --mod or fix it first")
            return 1
        live_dll = game / p["dll"]
        if live_dll.is_file():
            md5 = hs.file_md5(live_dll)
            if md5 != hs.full_md5(hs.kpatch_dll(kpatch[pid])):
                peers.append({"id": pid, "dll": p["dll"], "md5": md5})

    ids = list(live) + newids   # new ids last, in --mod order (the launcher orders loading by requires)
    label = targets[0] if len(targets) == 1 else "batch"
    backup_name = f"{label}_reinstall_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}"
    if a.dry_run:
        stage = game / "backups" / backup_name / "stage"
        print_plan(targets, game / "backups" / backup_name, stage,
                   [str(launcher()), str(game / "swkotor2.exe"), "--patches", str(stage), *ids], ids, peers)
        print("DRY RUN: nothing created or run")
        return 0

    # snapshot (manifest first, so an interrupted run is still rollbackable)
    backup = hs.unique_dir(game / "backups", backup_name)
    stage = backup / "stage"
    cmd = [str(launcher()), str(game / "swkotor2.exe"), "--patches", str(stage), *ids]
    shutil.copytree(game / "patches", backup / "patches")
    for name in hs.CONFIG_FILES:
        if (game / name).is_file():
            shutil.copy2(game / name, backup / name)
    for p in peers:  # a peer DLL outside patches/ is not in the tree copy
        rel = pathlib.Path(p["dll"])
        if not (backup / rel).is_file():
            (backup / rel).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(game / rel, backup / rel)
    meta = {"state": "started", "timestamp": datetime.datetime.now().isoformat(timespec="seconds"), "targets": targets,
            "peers": peers, "ids": ids, "config_md5": {n: hs.file_md5(game / n) for n in hs.CONFIG_FILES},
            "exe_md5": hs.file_md5(game / "swkotor2.exe")}
    write_manifest(backup, meta)

    # stage flat: one <id>.kpatch per live id
    stage.mkdir()
    for pid in ids:
        shutil.copy2(kpatch[pid], stage / f"{pid}.kpatch")

    print_plan(targets, backup, stage, cmd, ids, peers)
    rc = run_launcher(cmd, {**os.environ, "KPATCH_NO_LAUNCH": "1"})
    if rc != 0:
        do_rollback(backup, check_running=False)
        set_state(backup, "rolled_back")
        print(f"LAUNCHER FAILED (rc={rc}); rolled back from {backup}")
        return 1

    # restore peer DLLs the launcher reverted to the kpatch build
    for p in peers:
        hs.write_dll(game / p["dll"], (backup / p["dll"]).read_bytes())

    for w in config_warnings(backup / "patch_config.toml", game / "patch_config.toml"):
        print(w)
    if hs.file_md5(game / "swkotor2.exe") != meta["exe_md5"]:
        print("WARNING: swkotor2.exe md5 changed")

    # verify
    failed = [p["id"] for p in peers if hs.file_md5(game / p["dll"]) != p["md5"]]
    final = {r["id"]: r for r in kpm_ship.load_rows() if r["id"] in targets}
    failed += [t for t in targets if final.get(t, {}).get("verdict") != "LIVE"]
    if failed:
        do_rollback(backup, check_running=False)
        set_state(backup, "rolled_back")
        print(f"VERIFY FAILED for {', '.join(failed)}; rolled back from {backup}")
        return 1
    set_state(backup, "done")
    print(f"reinstalled: {', '.join(targets)} LIVE; {len(peers)} peer DLL(s) restored")
    print(f"backup: {backup}")
    print(f"rollback: python3 scripts/kpm_reinstall.py --rollback {backup}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--mod", nargs="+", action="extend", default=[], metavar="MOD", help="src dir name, dir name minus _src, or manifest id")
    ap.add_argument("--dry-run", action="store_true", help="print the plan; create and run nothing")
    ap.add_argument("--rollback", metavar="DIR", help="backup dir, name under GAME/backups, or 'latest'")
    try:
        a = ap.parse_args(argv)
    except SystemExit as e:
        return e.code if isinstance(e.code, int) else 2
    if a.rollback:
        if a.mod:
            print("usage error: --rollback takes no --mod")
            return 2
        return do_rollback(a.rollback)
    if not a.mod:
        print("usage error: pass --mod MOD [MOD ...] [--dry-run] or --rollback <dir|latest>")
        return 2
    return reinstall(a)


if __name__ == "__main__":
    sys.exit(main())
