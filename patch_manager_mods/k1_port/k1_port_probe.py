#!/usr/bin/env python3
"""Reversible in-game probe for one k1p_* NPC: copies prepared stage files into <game>/override/.

Design: .claude/review/k1_import/T12h_probe_design.md (sections 3-6). Only copies files that are
already built in the stage and named with --files. It does not build 2DA or UTC files.

Subcommands (plan is the default when none is given):
  plan      dry run. Writes nothing. Prints the files that would be copied and every precheck.
  install   copies the named files into <game>/override/. Dry run unless --yes.
  rollback  removes the files the manifest lists as created and restores replaced files from backup.
            Dry run unless --yes. Idempotent. A drifted file (edited since install) blocks unless --force.
  status    installed / not installed, and sha256 drift against the manifest.

Install prechecks (every one must pass; any failure refuses the copy):
  1. k1_port_validate: zero findings on every named .mdl/.mdx pair (K2 layout, lowercase names).
  2. k1_port_prune gate --strict over the named files against <stage>/ledger.json: no flagged file.
     A missing ledger blocks, because the gate would otherwise pass with nothing to compare.
  3. no swkotor2 process (scans <proc-root>/<pid>/cmdline; --proc-root is a test hook, default /proc).
  4. no target already in override/, except appearance.2da. That file must hash to --base-sha256 and is
     backed up first. A target that an active install manifest already lists is also refused.

Backups: <game>/backups/k1p_probe_<YYYYMMDD_HHMM>/manifest.json, originals under original/.
Save slots (<game>/cloudsaves/*/000000 - QUICKSAVE and 000001 - AUTOSAVE) are copied to saves/ and
verified by tree sha256 before any override file is touched. Rollback never restores them; it prints cp hints.
Pointer to the latest manifest: <game>/backups/k1p_probe_latest.json.

Stdlib only. Runs under system python3 and python3 -I (this directory is put on sys.path below).
Exit: 0 ok or dry run, 1 refused / blocked / error, 2 bad arguments.
"""
import argparse
import contextlib
import hashlib
import io
import json
import os
import re
import shutil
import sys
from datetime import datetime
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import k1_port_prune as kp  # noqa: E402
import k1_port_sources as ks  # noqa: E402
import k1_port_validate as kv  # noqa: E402

DEFAULT_STAGE = "~/Games/KOTOR2_Mod_Workspace/k1_port_stage"
DEFAULT_FILES = (
    "probe_T12h/override/appearance.2da",
    "probe_T12h/override/per_miner_002.utc",
    "models/k1p_commm.mdl",
    "models/k1p_commm.mdx",
    "override/k1p_commm04.tpc",
)
REPLACED_NAMES = frozenset({"appearance.2da"})  # the only override file this probe may replace
PROCESS_PREFIX = "swkotor2"
BACKUP_PREFIX = "k1p_probe_"
POINTER_NAME = "k1p_probe_latest.json"
MANIFEST_NAME = "manifest.json"
SCHEMA = "k1p_probe/1"  # manifest "saves" is optional on read: older manifests simply lack it
SAVE_SLOTS = ("000000 - QUICKSAVE", "000001 - AUTOSAVE")  # <game>/cloudsaves/<user>/<slot>, backed up by install
COMMANDS = ("plan", "install", "rollback", "status")
SHA_RE = re.compile(r"[0-9a-f]{64}")


class ProbeError(Exception):
    """A refusal or an unusable input. Printed as 'error: ...', exit 1."""


def now_iso() -> str:
    return datetime.now().astimezone().isoformat(timespec="seconds")


def now_stamp() -> str:
    return datetime.now().strftime("%Y%m%d_%H%M")


def sha_of(path: Path) -> str:
    return kp.sha256_of(path)


def tree_entries(root: Path) -> list:
    """Sorted (posix relpath, sha256) for every regular file under root. A symlink or special file raises."""
    entries = []
    for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
        d = Path(dirpath)
        for name in dirnames:
            if (d / name).is_symlink():
                raise ProbeError(f"save tree has a symlink: {d / name}")
        for name in filenames:
            p = d / name
            if p.is_symlink() or not p.is_file():
                raise ProbeError(f"save tree entry is not a regular file: {p}")
            entries.append((p.relative_to(root).as_posix(), sha_of(p)))
    return sorted(entries)


def tree_sha256(entries) -> str:
    h = hashlib.sha256()
    for rel, sha in entries:
        h.update(f"{rel}\0{sha}\n".encode("utf-8"))
    return h.hexdigest()


def within(child: Path, parent: Path) -> bool:
    try:
        child.relative_to(parent)
    except ValueError:
        return False
    return True


def write_json_atomic(path: Path, data: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name("." + path.name + ".tmp")
    tmp.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    os.replace(tmp, path)


def load_json(path: Path) -> dict:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as e:
        raise ProbeError(f"cannot read {path}: {e}") from e
    if not isinstance(data, dict):
        raise ProbeError(f"not a JSON object: {path}")
    return data


def game_processes(proc_root) -> list:
    """PIDs whose argv names a swkotor2 binary. Real use scans /proc; tests pass a fake tree."""
    found = []
    me = os.getpid()
    try:
        entries = list(Path(proc_root).iterdir())
    except OSError:
        return found
    for entry in entries:
        if not entry.name.isdigit() or int(entry.name) == me:
            continue
        try:
            argv = (entry / "cmdline").read_bytes().split(b"\0")
        except OSError:
            continue  # the process exited while scanning
        for arg in argv:
            base = re.split(r"[\\/]", arg.decode("utf-8", "replace"))[-1].lower()
            if base.startswith(PROCESS_PREFIX):
                found.append(int(entry.name))
                break
    return sorted(found)


def summarize_findings(findings) -> str:
    if not findings:
        return "0 finding(s)"
    shown = ", ".join(f"{f.code}@0x{f.offset:x}" for f in findings[:5])
    more = "" if len(findings) <= 5 else f" (+{len(findings) - 5} more)"
    return f"{len(findings)} finding(s): {shown}{more}"


def resolve_staged(stage: Path, raw: str) -> Path:
    p = Path(raw).expanduser()
    if not p.is_absolute():
        p = stage / p
    real = p.resolve()
    if not within(real, stage.resolve()):
        raise ProbeError(f"outside the stage dir: {raw}")
    if not real.is_file():
        raise ProbeError(f"not found in stage: {raw}")
    return real


def run_gate(led, stage: Path, paths) -> tuple:
    """Runs the prune gate (strict) by import and returns (ok, one-line detail)."""
    ns = argparse.Namespace(paths=[str(p) for p in paths], strict=True)
    out, err = io.StringIO(), io.StringIO()
    try:
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = kp.cmd_gate(led, stage, ns)
    except (ks.SourceError, OSError) as e:
        return False, f"gate error: {e}"
    lines = [ln for ln in out.getvalue().splitlines() if ln.strip()]
    warns = [ln.strip() for ln in err.getvalue().splitlines() if ln.strip()]
    summary = lines[-1] if lines else "gate printed nothing"
    extra = lines[:-1] + warns
    detail = summary + ("; " + "; ".join(extra) if extra else "")
    return rc == 0, detail


def scan_manifests(game: Path) -> list:
    """(manifest path, parsed dict or None when unreadable) for each k1p_probe_* backup dir."""
    out = []
    bdir = game / "backups"
    if not bdir.is_dir():
        return out
    for d in sorted(bdir.glob(BACKUP_PREFIX + "*")):
        m = d / MANIFEST_NAME
        if d.is_dir() and m.is_file():
            try:
                out.append((m, load_json(m)))
            except ProbeError:
                out.append((m, None))
    return out


def pointer_path(game: Path) -> Path:
    return game / "backups" / POINTER_NAME


def resolve_manifest(args, game: Path):
    if args.manifest:
        return Path(args.manifest).expanduser()
    ptr = pointer_path(game)
    if not ptr.is_file():
        return None
    target = load_json(ptr).get("manifest")
    if not target:
        raise ProbeError(f"pointer has no manifest: {ptr}")
    return Path(str(target))


def entry_dest(game: Path, entry: dict) -> Path:
    rel = str(entry.get("rel", ""))
    name = Path(rel).name
    if not name or rel != f"override/{name}":
        raise ProbeError(f"manifest entry has a bad path: {rel!r}")
    return game / "override" / name


def entry_state(game: Path, entry: dict) -> str:
    dest = entry_dest(game, entry)
    if not os.path.lexists(dest):
        return "missing"
    if dest.is_symlink() or not dest.is_file():
        return "drift"
    return "ok" if sha_of(dest) == entry.get("sha256") else "drift"


class Plan:
    def __init__(self):
        self.checks = []  # (ok, name, detail)
        self.warnings = []  # (name, detail): reported as WARN, never blocks
        self.files = []   # dicts: name, src, src_rel, sha256, size, dest, rel, action, backup
        self.saves = []   # dicts: slot, src (Path), backup (Path, under backup_dir/saves/)
        self.manifest = None
        self.backup_dir = None
        self.base = None

    def check(self, name, ok, detail=""):
        self.checks.append((bool(ok), name, detail))

    def warn(self, name, detail=""):
        self.warnings.append((name, detail))

    def failed(self):
        return [c for c in self.checks if not c[0]]


def active_targets(game: Path, plan: Plan) -> dict:
    """rel path -> manifest path, for every target an active (not rolled back) install lists."""
    active = {}
    for mpath, data in scan_manifests(game):
        if data is None:
            plan.check(f"manifest readable {mpath}", False, "unreadable JSON: resolve before installing")
            continue
        if data.get("rolled_back"):
            continue
        for e in data.get("installed", []):
            active[str(e.get("rel"))] = str(mpath)
    return active


def plan_saves(game: Path, plan: Plan, no_backup: bool) -> None:
    """Fill plan.saves with one entry per save slot found. Ambiguous or non-directory slots block;
    an absent slot or cloudsaves/ only warns, so install still runs."""
    if no_backup:
        plan.warn("save backup", "skipped by --no-save-backup: save slots are NOT backed up")
        return
    cloud = game / "cloudsaves"
    for slot in SAVE_SLOTS:
        hits = sorted(cloud.glob(f"*/{slot}")) if cloud.is_dir() else []
        if not hits:
            why = "no cloudsaves/*/ match" if cloud.is_dir() else "cloudsaves/ absent"
            plan.warn(f"save slot {slot}", f"not found ({why}): not backed up, install continues")
        elif len(hits) > 1:
            plan.check(f"save slot {slot}", False, f"{len(hits)} matches under cloudsaves/*/: resolve which one first")
        elif hits[0].is_symlink() or not hits[0].is_dir():
            plan.check(f"save slot {slot}", False, f"{hits[0]} is not a plain directory")
        else:
            try:
                tree_entries(hits[0])
            except ProbeError as e:
                plan.check(f"save slot {slot}", False, str(e))
                continue
            dst = plan.backup_dir / "saves" / slot
            plan.check(f"save slot {slot}", True, f"found {hits[0]}; backup to {dst}")
            plan.saves.append({"slot": slot, "src": hits[0], "backup": dst})


def compute_plan(args, stamp: str) -> Plan:
    plan = Plan()
    stage = Path(args.stage).expanduser()
    game = Path(args.game_dir).expanduser() if args.game_dir else None
    names = list(args.files) if args.files else list(DEFAULT_FILES)
    base = args.base_sha256.strip().lower() if args.base_sha256 else None
    plan.base = base

    plan.check("stage dir", stage.is_dir(), str(stage))
    if base is not None:
        plan.check("--base-sha256 format", bool(SHA_RE.fullmatch(base)), base)
    if not names:
        plan.check("files", False, "no files named")

    staged = []
    for raw in names:
        try:
            real = resolve_staged(stage, raw)
        except ProbeError as e:
            plan.check(f"staged {raw}", False, str(e))
            continue
        plan.check(f"staged {raw}", True, f"{real.stat().st_size} bytes")
        staged.append((raw, real))

    seen = set()
    for raw, real in staged:
        if real.name in seen:
            plan.check(f"unique name {real.name}", False, f"more than one named file lands in override/{real.name}")
            continue
        seen.add(real.name)
        plan.files.append({"name": real.name, "src": real, "src_rel": raw, "sha256": sha_of(real),
                           "size": real.stat().st_size, "dest": None, "rel": f"override/{real.name}",
                           "action": "create", "backup": None})

    pairs = {}
    for _, real in staged:
        if real.suffix.lower() in (".mdl", ".mdx"):
            pairs.setdefault((str(real.parent), real.stem.lower()), {})[real.suffix.lower()] = real
    for (_, stem), pr in pairs.items():
        if set(pr) != {".mdl", ".mdx"}:
            plan.check(f"model pair {stem}", False, "name both .mdl and .mdx in --files")
            continue
        findings = kv.validate(pr[".mdl"].read_bytes(), pr[".mdx"].read_bytes(), require_lower_k1p=True)
        plan.check(f"validate {stem} (.mdl+.mdx)", not findings, summarize_findings(findings))

    ledger_path = stage / "ledger.json"
    led = None
    if ledger_path.is_file():
        try:
            led = kp.Ledger(ledger_path)
            plan.check("stage ledger", True, str(ledger_path))
        except (ks.SourceError, OSError, ValueError) as e:
            plan.check("stage ledger", False, str(e))
    else:
        plan.check("stage ledger", False, f"missing {ledger_path}: prune gate cannot run")
    if led is not None and plan.files:
        ok, detail = run_gate(led, stage.resolve(), [f["src"] for f in plan.files])
        plan.check("prune gate --strict (no flagged file)", ok, detail)

    procs = game_processes(args.proc_root)
    plan.check("swkotor2 not running", not procs, f"pids {procs}" if procs else "no process")

    if game is None:
        plan.check("--game-dir", False, "not given: game dir and override/ checks cannot run")
        return plan
    is_k2 = (game / "chitin.key").is_file() and (game / "override").is_dir()
    plan.check("game dir is a KOTOR 2 install", is_k2, f"{game} (needs chitin.key and override/)")
    active = active_targets(game, plan)

    if args.manifest:
        mpath = Path(args.manifest).expanduser()
    else:
        mpath = game / "backups" / f"{BACKUP_PREFIX}{stamp}" / MANIFEST_NAME
    plan.manifest = mpath
    plan.backup_dir = mpath.parent
    plan.check("backup dir is new", not os.path.lexists(plan.backup_dir), str(plan.backup_dir))
    plan.check("manifest outside override/", not within(Path(os.path.abspath(mpath)), game / "override"), str(mpath))
    plan_saves(game, plan, bool(getattr(args, "no_save_backup", False)))

    for f in plan.files:
        dest = game / "override" / f["name"]
        f["dest"] = dest
        rel = f["rel"]
        if rel in active:
            plan.check(f"target {rel}", False, f"listed by an active install ({active[rel]}): run rollback first")
        elif not os.path.lexists(dest):
            plan.check(f"target {rel}", True, "absent: will create")
        elif f["name"].lower() in REPLACED_NAMES and dest.is_file() and not dest.is_symlink():
            live = sha_of(dest)
            f["action"] = "replace"
            f["backup"] = plan.backup_dir / "original" / f["name"]
            if base is None:
                plan.check(f"target {rel}", False, f"exists (live sha256 {live}): pass --base-sha256 {live}")
            elif live != base:
                plan.check(f"target {rel}", False, f"live sha256 {live} != --base-sha256 {base}")
            else:
                plan.check(f"target {rel}", True, f"replace; live sha256 matches base; backup {f['backup']}")
        else:
            plan.check(f"target {rel}", False, "exists and is not a replaceable probe file: refusing to overwrite")
    return plan


def print_plan(plan: Plan, title: str) -> None:
    print(f"k1_port_probe {title}")
    print(f"  manifest: {plan.manifest or '(needs --game-dir)'}")
    print(f"  files ({len(plan.files)}):")
    for f in plan.files:
        target = f"override/{f['name']}" if f["dest"] is None else str(f["dest"])
        extra = f"  backup {f['backup']}" if f["action"] == "replace" and f["backup"] else ""
        print(f"    {f['action']:<7} {f['src_rel']} -> {target}  sha256 {f['sha256'][:12]}  {f['size']} B{extra}")
    print("  checks:")
    for ok, name, detail in plan.checks:
        print(f"    {'PASS ' if ok else 'BLOCK'} {name}: {detail}")
    for name, detail in plan.warnings:
        print(f"    WARN  {name}: {detail}")
    if plan.failed():
        print(f"  verdict: BLOCKED ({len(plan.failed())} check(s) failed); install --yes would copy nothing")
    else:
        saves = f" and back up {len(plan.saves)} save slot(s)" if plan.saves else ""
        print(f"  verdict: READY; install --yes would copy {len(plan.files)} file(s) into override/{saves}")


def undo_install(undo) -> None:
    for kind, dest, bak in reversed(undo):
        try:
            if kind == "delete":
                if os.path.lexists(dest):
                    dest.unlink()
            elif kind == "rmtree":
                if os.path.lexists(dest):
                    shutil.rmtree(dest)
            else:
                shutil.copyfile(bak, dest)
        except OSError as e:
            print(f"undo failed for {dest}: {e}", file=sys.stderr)


def backup_save(s: dict, undo: list) -> dict:
    """Copy one save slot into the backup dir, then verify the copy's tree hash against the source.
    The undo entry is added before the copy, so a failed or partial copy is removed on failure."""
    src, dst = s["src"], s["backup"]
    src_entries = tree_entries(src)
    want = tree_sha256(src_entries)
    dst.parent.mkdir(parents=True, exist_ok=True)
    undo.append(("rmtree", dst, None))
    shutil.copytree(src, dst, symlinks=True)
    got_entries = tree_entries(dst)
    got = tree_sha256(got_entries)
    if got != want or got_entries != src_entries:
        raise ProbeError(f"save backup of {src} hashes {got}, expected {want}")
    return {"slot": s["slot"], "src": str(src), "backup": f"saves/{s['slot']}",
            "files": len(src_entries), "tree_sha256": want}


def install_files(plan: Plan, game: Path, stage: Path, base) -> dict:
    bdir = plan.backup_dir
    bdir.mkdir(parents=True)  # raises if the dir exists: never clobber an earlier backup
    undo, records, save_records = [], [], []
    try:
        for s in plan.saves:  # save slots first: no override file is touched until every save copy verifies
            save_records.append(backup_save(s, undo))
        for f in plan.files:
            rec = {"name": f["name"], "rel": f["rel"], "source": f["src_rel"], "sha256": f["sha256"],
                   "size": f["size"], "action": "created", "backup": None, "backup_sha256": None}
            if f["action"] == "replace":
                bak = f["backup"]
                bak.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(f["dest"], bak)
                bsha = sha_of(bak)
                if bsha != base:
                    raise ProbeError(f"backup of {f['dest']} hashes {bsha}, expected {base}")
                rec.update(action="replaced", backup=f"original/{f['name']}", backup_sha256=bsha)
                undo.append(("restore", f["dest"], bak))
            else:
                undo.append(("delete", f["dest"], None))
            records.append(rec)
            shutil.copyfile(f["src"], f["dest"])
            if sha_of(f["dest"]) != f["sha256"]:
                raise ProbeError(f"copy verify failed: {f['dest']}")
        manifest = {"schema": SCHEMA, "created": now_iso(), "game_dir": str(game), "stage": str(stage),
                    "base_sha256": base, "backup_dir": str(bdir), "installed": records, "saves": save_records}
        write_json_atomic(plan.manifest, manifest)
        write_json_atomic(pointer_path(game), {"manifest": str(plan.manifest), "written": manifest["created"]})
        bad = [r["rel"] for r in records if entry_state(game, r) != "ok"]
        if bad:
            raise ProbeError(f"post-install hash check failed: {bad}")
    except (ProbeError, OSError) as e:
        undo_install(undo)
        raise ProbeError(f"install undone after failure: {e}") from e
    return manifest


def plan_rollback(game: Path, mpath: Path, data: dict, force: bool) -> tuple:
    """(actions, blockers). Action kinds: delete, restore, skip. Changes nothing."""
    actions, blockers = [], []
    if os.path.realpath(str(data.get("game_dir", ""))) != os.path.realpath(str(game)):
        blockers.append(f"manifest game_dir {data.get('game_dir')} is not --game-dir {game}")
        return actions, blockers
    bdir = mpath.parent
    for e in data.get("installed", []):
        dest = entry_dest(game, e)
        rel = e["rel"]
        if e.get("action") == "replaced":
            base = e.get("backup_sha256")
            bak = bdir / str(e.get("backup") or "")
            if not e.get("backup") or not bak.is_file() or sha_of(bak) != base:
                blockers.append(f"{rel}: backup missing or sha mismatch ({bak}): not restoring")
                continue
            if dest.is_file() and not dest.is_symlink() and sha_of(dest) == base:
                actions.append({"kind": "skip", "rel": rel, "dest": dest, "bak": None, "entry": e, "note": "already restored"})
            elif entry_state(game, e) in ("ok", "missing") or force:
                actions.append({"kind": "restore", "rel": rel, "dest": dest, "bak": bak, "entry": e, "note": "from backup"})
            else:
                blockers.append(f"{rel}: edited since install (not the probe copy or the base): --force to restore")
        else:
            state = entry_state(game, e)
            if dest.is_dir():
                blockers.append(f"{rel}: is a directory")
            elif state == "missing":
                actions.append({"kind": "skip", "rel": rel, "dest": dest, "bak": None, "entry": e, "note": "already absent"})
            elif state == "ok" or force:
                actions.append({"kind": "delete", "rel": rel, "dest": dest, "bak": None, "entry": e, "note": "created by probe"})
            else:
                blockers.append(f"{rel}: sha256 differs from manifest (edited?): --force to delete")
    return actions, blockers


def execute_rollback(actions, mpath: Path, data: dict) -> None:
    for a in actions:
        if a["kind"] == "delete":
            a["dest"].unlink()
        elif a["kind"] == "restore":
            shutil.copyfile(a["bak"], a["dest"])
            if sha_of(a["dest"]) != a["entry"]["backup_sha256"]:
                raise ProbeError(f"restore verify failed: {a['dest']}")
    data["rolled_back"] = now_iso()
    write_json_atomic(mpath, data)


def require_game(args) -> Path:
    game = Path(args.game_dir).expanduser()
    if not game.is_dir():
        raise ProbeError(f"game dir not found: {game}")
    return game


def print_save_hints(data: dict, mpath: Path) -> None:
    """Rollback never restores save slots. Print one manual copy command per backed-up slot."""
    saves = data.get("saves") or []
    if saves:
        print("  saves are NOT restored by rollback. Manual restore, one line per slot:")
    for s in saves:
        bak = os.path.abspath(mpath.parent / str(s.get("backup", "")))
        parent = Path(str(s.get("src", ""))).parent
        print(f'    cp -a "{bak}" "{parent}/"')


def print_save_status(data: dict, mpath: Path) -> None:
    saves = data.get("saves") or []
    if not saves:
        print("  save backups: none recorded")
        return
    for s in saves:
        bak = mpath.parent / str(s.get("backup", ""))
        state = "present" if bak.is_dir() else "MISSING"
        print(f"  save backup {state:<8} {s.get('slot')}  ({s.get('files')} file(s))  {bak}")


def cmd_plan(args) -> int:
    plan = compute_plan(args, now_stamp())
    print("(dry run: nothing is written)")
    print_plan(plan, "plan")
    return 0


def cmd_install(args) -> int:
    game = require_game(args)
    stage = Path(args.stage).expanduser()
    plan = compute_plan(args, now_stamp())
    print_plan(plan, "install")
    if not args.yes:
        print("dry run: nothing written. Re-run with --yes to copy.")
        return 0
    if plan.failed():
        print("refused: nothing written.")
        return 1
    manifest = install_files(plan, game, stage, plan.base)
    print(f"installed {len(manifest['installed'])} file(s). manifest: {plan.manifest}")
    if manifest["saves"]:
        print(f"saved {len(manifest['saves'])} save slot(s) under {plan.backup_dir / 'saves'} (tree sha256 verified)")
    return 0


def cmd_rollback(args) -> int:
    game = require_game(args)
    mpath = resolve_manifest(args, game)
    if mpath is None or not mpath.is_file():
        raise ProbeError(f"no manifest to roll back (pointer: {pointer_path(game)}; or pass --manifest)")
    data = load_json(mpath)
    if data.get("rolled_back"):
        print(f"already rolled back ({data['rolled_back']}): nothing to do. manifest: {mpath}")
        return 0
    actions, blockers = plan_rollback(game, mpath, data, args.force)
    procs = game_processes(args.proc_root)
    if procs:
        blockers.insert(0, f"swkotor2 running (pids {procs})")
    print(f"k1_port_probe rollback: manifest {mpath}")
    for a in actions:
        print(f"  {a['kind']:<8} {a['rel']}  ({a['note']})")
    for b in blockers:
        print(f"  BLOCK    {b}")
    print_save_hints(data, mpath)
    if not args.yes:
        print("dry run: nothing removed or restored. Re-run with --yes to apply.")
        return 0
    if blockers:
        print(f"refused: {len(blockers)} blocker(s); nothing removed or restored.")
        return 1
    execute_rollback(actions, mpath, data)
    print(f"rolled back. manifest marked: {mpath}")
    return 0


def cmd_status(args) -> int:
    game = require_game(args)
    mpath = resolve_manifest(args, game)
    procs = game_processes(args.proc_root)
    print(f"k1_port_probe status: game {game}")
    print(f"  swkotor2 running: {'yes, pids ' + str(procs) if procs else 'no'}")
    if mpath is None or not mpath.is_file():
        print("  state: NOT INSTALLED (no manifest)")
        return 0
    data = load_json(mpath)
    print(f"  manifest: {mpath}")
    print_save_status(data, mpath)
    if data.get("rolled_back"):
        print(f"  state: NOT INSTALLED (rolled back {data['rolled_back']})")
        return 0
    states = []
    for e in data.get("installed", []):
        st = entry_state(game, e)
        states.append(st)
        print(f"    {st:<8} {e.get('rel')}  manifest sha256 {str(e.get('sha256'))[:12]}")
    if states and all(s == "ok" for s in states):
        state = "INSTALLED"
    elif any(s == "drift" for s in states):
        state = "DRIFT (a file differs from the manifest; rollback refuses unless --force)"
    else:
        state = "PARTIAL (some files missing)"
    print(f"  state: {state}")
    return 0


HANDLERS = {"plan": cmd_plan, "install": cmd_install, "rollback": cmd_rollback, "status": cmd_status}


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description="Reversible in-game probe: copy prepared k1p_* files into <game>/override/. "
                                             "Default command: plan.")
    sub = ap.add_subparsers(dest="cmd", required=True)

    def common(p, staged: bool):
        p.add_argument("--game-dir", metavar="DIR", help="KOTOR 2 install dir (required for install, rollback, status)")
        p.add_argument("--manifest", metavar="PATH", help="backup manifest (default: the pointer in <game>/backups/)")
        p.add_argument("--proc-root", default="/proc", help=argparse.SUPPRESS)
        if staged:
            p.add_argument("--stage", default=DEFAULT_STAGE, metavar="DIR", help=f"stage dir (default {DEFAULT_STAGE})")
            p.add_argument("--files", nargs="+", metavar="PATH",
                           help="staged files relative to --stage (default: the design-doc probe set)")
            p.add_argument("--base-sha256", metavar="HEX",
                           help="sha256 of the live override/appearance.2da that install replaces")
            p.add_argument("--no-save-backup", action="store_true",
                           help="skip the backup of cloudsaves QUICKSAVE/AUTOSAVE (tests, non-Steam installs)")

    p = sub.add_parser("plan", help="dry run: list what would be copied and every precheck (writes nothing)")
    common(p, staged=True)
    p = sub.add_parser("install", help="copy the named files into override/ (dry run unless --yes)")
    common(p, staged=True)
    p.add_argument("--yes", action="store_true", help="actually copy")
    p = sub.add_parser("rollback", help="undo an install from its manifest (dry run unless --yes)")
    common(p, staged=False)
    p.add_argument("--yes", action="store_true", help="actually remove and restore")
    p.add_argument("--force", action="store_true", help="remove or restore files whose sha256 differs from the manifest")
    p = sub.add_parser("status", help="installed / not installed, and sha drift")
    common(p, staged=False)
    return ap


def main(argv=None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if not argv or (argv[0] not in COMMANDS and argv[0] not in ("-h", "--help")):
        argv = ["plan"] + argv
    args = build_parser().parse_args(argv)
    if args.cmd != "plan" and not args.game_dir:
        print(f"error: --game-dir is required for {args.cmd}", file=sys.stderr)
        return 2
    try:
        return HANDLERS[args.cmd](args)
    except (ProbeError, ks.SourceError, OSError, ValueError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
