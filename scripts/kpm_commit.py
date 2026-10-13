#!/usr/bin/env python3
"""Repo-side auto-committer (doc 24 H7): for each dirty patch_manager_mods/*_src, run that mod's tests,
then commit only its production files (explicit paths, never -a, never push).

Use --dry-run in a shared worktree: without it this commits every dirty mod, including other sessions' edits.
"""
import argparse
import subprocess
import sys

import kpm_status as ks

REPO = ks.REPO
MODS = ks.MODS


def git(*args):
    return subprocess.run(["git", "-C", str(REPO), *args], capture_output=True, text=True)


def dirty_files(src):
    """(untracked, modified) repo-relative production paths under one mod dir."""
    rel = src.relative_to(REPO).as_posix()
    p = git("status", "--porcelain", "-uall", "--", rel)
    untracked, modified = [], []
    for line in p.stdout.splitlines():
        if len(line) < 4:
            continue
        path = line[3:]
        if " -> " in path:
            path = path.split(" -> ", 1)[1]
        path = path.strip('"')
        if not ks.is_production(path.split("/")[-1]):
            continue
        (untracked if line.startswith("??") else modified).append(path)
    return untracked, modified


def run_mod_tests(stem):
    """Scoped test run for one mod; returns (rc, output)."""
    p = subprocess.run([sys.executable, str(REPO / "scripts/run_tests.py"), "-k", stem], capture_output=True, text=True)
    return p.returncode, p.stdout + p.stderr


def resolve_mods(names):
    """Map --mod names (src dir, dir minus _src, or manifest id) to src dir names; None for unknown."""
    dirs = sorted(d.name for d in MODS.glob("*_src") if d.is_dir())
    ids = {}
    for m in sorted(MODS.glob("*_src/**/manifest.toml")):
        try:
            pid = ks.tomllib.load(open(m, "rb"))["patch"]["id"]
        except (KeyError, ks.tomllib.TOMLDecodeError, OSError):
            continue
        ids[pid] = m.relative_to(MODS).parts[0]
    out, bad = [], []
    for n in names:
        if n in dirs:
            out.append(n)
        elif n + "_src" in dirs:
            out.append(n + "_src")
        elif n in ids:
            out.append(ids[n])
        else:
            bad.append(n)
    return out, bad, dirs


def main(argv=None):
    ap = argparse.ArgumentParser(description="Run each dirty mod's tests, then commit only its production files.")
    ap.add_argument("--mod", nargs="+", action="extend", default=[], metavar="MOD", help="src dir name, dir name minus _src, or manifest id")
    ap.add_argument("--dry-run", action="store_true", help="list what would be tested and committed; run nothing")
    ap.add_argument("--prefix", default="feat", choices=["feat", "fix", "refactor"], help="conventional commit prefix (default feat)")
    try:
        a = ap.parse_args(argv)
    except SystemExit as e:
        return e.code if isinstance(e.code, int) else 2

    selected, bad, dirs = resolve_mods(a.mod)
    if bad:
        print(f"usage error: unknown mod(s) {', '.join(bad)}; valid: {', '.join(dirs)}")
        return 2
    wanted = set(selected)

    any_dirty = False
    for src in sorted(d for d in MODS.glob("*_src") if d.is_dir()):
        if wanted and src.name not in wanted:
            continue
        stem = src.name[:-4]
        untracked, modified = dirty_files(src)
        paths = untracked + modified
        if not paths:
            continue
        any_dirty = True
        if a.dry_run:
            shown = ", ".join(p.split("/", 2)[-1] for p in paths[:4]) + ("..." if len(paths) > 4 else "")
            print(f"DRY  {stem}: would test -k {stem}, commit {len(paths)} file(s): {shown}")
            continue
        rc, out = run_mod_tests(stem)
        if rc != 0:
            print(f"TEST FAIL for {stem}; commit aborted\n{out[-400:]}")
            return 1
        if "0 items" in out:
            print(f"no tests matched {stem}")
        steps = ([["add", "--"] + untracked] if untracked else []) + [["commit", "-m", f"{a.prefix}({stem}): update production sources", "--"] + paths]
        for cmd in steps:
            p = git(*cmd)
            if p.returncode != 0:
                print(f"FAILED git {cmd[0]} for {stem} (rc={p.returncode}): {(p.stdout + p.stderr).strip()[-400:]}")
                return 1
        sha = git("rev-parse", "--short", "HEAD").stdout.strip()
        print(f"committed {stem} {sha} ({len(paths)} files)")

    if not any_dirty:
        print("All mod directories clean; nothing to commit")
    return 0


if __name__ == "__main__":
    sys.exit(main())
