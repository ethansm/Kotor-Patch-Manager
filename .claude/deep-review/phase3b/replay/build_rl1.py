#!/usr/bin/env python3
"""Idempotent scratch-run builder for the Haiku-ladder replay (cost: $0, no model calls).

  build_rl1.py --run rl1|rl2 --profile <profile.toml> [--dry-run]

1. `dr.py init <profile> --run <run>` (creates state.json; the global active.json pointer is saved and restored).
2. READ-ONLY copies from the p1 run: tree/, snapshot.json, canary.json, worker_settings.json, worker_system_prompt.md.
   snapshot.json is rewritten in the copy: tree_path -> <run>/tree, head -> current git HEAD of the profile repo
   (prep verify-snapshot also requires HEAD == pinned head; HEAD has moved since p1 and the tree content is unchanged).
3. Copies the 8 dossier pairs from .claude/review/work/adjudicate/ into <run>/work/adjudicate/.
Never copies verify_key.json or stored verdicts; never writes into p1 or locks/.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

BUNDLE = Path(os.environ.get("DR_BUNDLE") or Path.home() / ".claude/skills/deep-haiku-review/bundle")
ROOT = Path(__file__).resolve().parents[4]                      # repo/worktree root
DOSSIERS = ROOT / ".claude/review/work/adjudicate"
BATCHES = ["batch_01", "batch_03", "batch_07", "batch_08", "batch_09", "batch_10", "batch_14", "batch_26"]
SEED_FILES = ["canary.json", "worker_settings.json", "worker_system_prompt.md"]
DEFAULT_HOME = Path(os.environ.get("DR_HOME") or Path.home() / ".cache/deep_haiku_review") / "kotor2-pilot"
FORBIDDEN = {"verify_key.json"}


def die(msg: str, rc: int = 2) -> int:
    print(f"build_rl1: {msg}", file=sys.stderr)
    return rc


def load_profile(path: Path):
    sys.path.append(str(BUNDLE / "engine"))
    import dr_profile  # noqa: E402
    return dr_profile.load(str(path))


def same(a: Path, b: Path) -> bool:
    return b.is_file() and a.stat().st_size == b.stat().st_size and a.read_bytes() == b.read_bytes()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--run", required=True, choices=["rl1", "rl2"])
    ap.add_argument("--profile", required=True)
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    prof = Path(a.profile).expanduser()
    dry = a.dry_run
    P = None
    home, repo = DEFAULT_HOME, ROOT
    if prof.is_file():
        try:
            P = load_profile(prof)
            home, repo = Path(P["home"]), Path(P["_repo"])
        except Exception as e:  # noqa: BLE001 - profile errors are reported, dry-run still prints the plan
            if not dry:
                return die(f"profile error: {e}")
            print(f"note: profile did not load ({e}); plan uses defaults")
    elif not dry:
        return die(f"profile not found: {prof} (create it first; --dry-run works without it)")
    else:
        print(f"note: profile {prof} does not exist yet; plan uses defaults")
    p1, run = home / "p1", home / a.run
    if run.resolve() == p1.resolve() or run.name not in ("rl1", "rl2"):
        return die(f"refusing run dir {run}")
    try:
        run.resolve().relative_to(repo.resolve())
        return die(f"run dir {run} is inside the repo")
    except ValueError:
        pass
    missing = [str(p1 / n) for n in ["tree", "snapshot.json", *SEED_FILES] if not (p1 / n).exists()]
    missing += [str(DOSSIERS / f"{b}.{e}") for b in BATCHES for e in ("md", "json") if not (DOSSIERS / f"{b}.{e}").is_file()]
    if missing:
        return die("missing sources:\n  " + "\n  ".join(missing))
    if FORBIDDEN & {p.name for b in BATCHES for p in DOSSIERS.glob(f"{b}.*")}:
        return die("answer key among dossier sources")
    head = subprocess.run(["git", "-C", str(repo), "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
    snap = json.loads((p1 / "snapshot.json").read_text())
    snap["tree_path"] = str(run / "tree")
    snap["head"] = head or snap["head"]
    steps = [
        f"init      : python3 {BUNDLE}/dr.py init {prof} --run {a.run}   (active.json saved/restored)",
        f"tree      : copy {p1}/tree -> {run}/tree" + ("  [present, skip]" if (run / "tree").is_dir() else ""),
        f"snapshot  : write {run}/snapshot.json (tree_path={snap['tree_path']}, head={snap['head'][:7]})",
        *[f"seed      : copy {p1}/{n} -> {run}/{n}" for n in SEED_FILES],
        f"dossiers  : copy {len(BATCHES)} pairs ({', '.join(b[-2:] for b in BATCHES)}) -> {run}/work/adjudicate/",
        "check     : prep.py verify-snapshot (read-only)",
    ]
    print(("DRY RUN: " if dry else "") + f"plan for {a.run} (run dir {run})")
    print("\n".join("  " + s for s in steps))
    if dry:
        print("dry-run: nothing created or changed")
        return 0

    active = home.parent / "active.json"
    saved = active.read_bytes() if active.is_file() else None
    r = subprocess.run([sys.executable, str(BUNDLE / "dr.py"), "init", str(prof), "--run", a.run])
    if saved is not None:
        active.write_bytes(saved)
    elif active.exists():
        active.unlink()
    if r.returncode:
        return die(f"dr.py init failed rc={r.returncode}", 1)

    tree = run / "tree"
    if not tree.is_dir():
        part = run / "tree.partial"
        shutil.rmtree(part, ignore_errors=True)
        shutil.copytree(p1 / "tree", part, symlinks=True)
        part.rename(tree)
    (run / "snapshot.json").write_text(json.dumps(snap, indent=1, sort_keys=True) + "\n", encoding="utf-8")
    for n in SEED_FILES:
        if not same(p1 / n, run / n):
            shutil.copyfile(p1 / n, run / n)
    dst = run / "work/adjudicate"
    dst.mkdir(parents=True, exist_ok=True)
    for b in BATCHES:
        for e in ("md", "json"):
            if not same(DOSSIERS / f"{b}.{e}", dst / f"{b}.{e}"):
                shutil.copyfile(DOSSIERS / f"{b}.{e}", dst / f"{b}.{e}")
    n_items = sum(len(json.loads((dst / f"{b}.json").read_text())["items"]) for b in BATCHES)
    ok = subprocess.run([sys.executable, "-I", str(BUNDLE / "engine/prep.py"), "verify-snapshot",
                         "--profile", str(prof.resolve()), "--run", a.run], capture_output=True, text=True)
    print(f"built {run}: {len(BATCHES)} batches, {n_items} items; verify-snapshot rc={ok.returncode} {ok.stderr.strip()[:200]}")
    return 0 if ok.returncode == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
