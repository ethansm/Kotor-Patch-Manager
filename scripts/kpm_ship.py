#!/usr/bin/env python3
"""End-to-end deploy pipeline (doc 24 H9): status -> commit -> gate -> prepare (+KPM commit) -> apply, halting at the first failure.

Usage: python3 scripts/kpm_ship.py [--mod MOD [MOD ...]] [--all] [--apply] [--skip-gate] [--dry-run]
Shared-worktree safety: a mutating run needs an explicit --mod list (or --all); without either it prints the pending
table and exits 2. --dry-run reads status only and prints the planned steps. Every mutation is delegated to
kpm_commit / pre_install_gate / kpm_hotswap, which keep their own refusals. Exit: 0 ok, 1 step failed or refused, 2 usage.
"""
import argparse
import contextlib
import io
import json
import pathlib
import shlex
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import kpm_commit as kc  # noqa: E402
import kpm_hotswap as hs  # noqa: E402
import kpm_status as ks  # noqa: E402

SCRIPTS = pathlib.Path(__file__).resolve().parent
PENDING = {"DIRTY", "MIRROR", "REBUILD", "SWAP"}
REFUSED = {"NOT-LIVE": "not installed in the live game; install it manually first",
           "REINSTALL": "hooks differ from live: needs kpm_reinstall.py / a full reinstall, not a hot-swap"}
PREFIX = "patch_manager_mods/"


def load_rows():
    """kpm_status rows (the same dicts `kpm_status.py --json` prints)."""
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = ks.main(["--json"])
    if rc != 0:
        raise RuntimeError(f"kpm_status failed (rc={rc})")
    return json.loads(buf.getvalue())


def run_step(label, cmd):
    """Run one pipeline step with streamed output; the single subprocess point (mocked in tests)."""
    print(f"\n== {label}: {shlex.join(cmd)}", flush=True)
    return subprocess.run(cmd).returncode


def src_dir(row):
    """Top-level X_src dir name of a status row (kpm_commit / kpm_hotswap --mod value)."""
    return row["src"].removeprefix(PREFIX).split("/", 1)[0]


def gate_dir(row):
    """Gate --mod value: dir relative to patch_manager_mods, nested path kept (perf_lab_src/perflab)."""
    return row["src"].removeprefix(PREFIX)


def print_table(rows):
    for r in rows:
        md = f"{r.get('live') or '-'}/{r.get('kpatch') or '-'}"
        print(f"{r['verdict']:<10} {r['id']:<28} {r['version']:<14} {md:<17} {r.get('detail', '')}")


def uniq(items):
    return list(dict.fromkeys(items))


def planned(a, rows):
    """The ordered (label, cmd) list for these target rows; same list drives --dry-run and the real run."""
    py = sys.executable
    mods = uniq(src_dir(r) for r in rows)
    dirty = uniq(src_dir(r) for r in rows if r["verdict"] == "DIRTY")
    steps = []
    if dirty:
        steps.append(("commit", [py, str(SCRIPTS / "kpm_commit.py"), "--mod", *dirty]))
    if not a.skip_gate:
        gate = [py, str(SCRIPTS / "pre_install_gate.py")]
        for d in uniq(gate_dir(r) for r in rows):
            gate += ["--mod", d]
        steps.append(("gate", gate))
    steps.append(("prepare", [py, str(SCRIPTS / "kpm_hotswap.py"), "--mod", *mods, "--prepare", "--commit-kpm"]))
    if a.apply:
        steps.append(("apply", [py, str(SCRIPTS / "kpm_hotswap.py"), "--mod", *mods, "--apply"]))
    return steps


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--mod", nargs="+", action="extend", default=[], metavar="MOD", help="src dir name, dir name minus _src, or manifest id")
    ap.add_argument("--all", action="store_true", help="ship every pending mod (includes other sessions' dirty edits)")
    ap.add_argument("--apply", action="store_true", help="after prepare, hot-swap into the live game (game must be closed)")
    ap.add_argument("--skip-gate", action="store_true", help="skip pre_install_gate (loud warning)")
    ap.add_argument("--dry-run", action="store_true", help="read status and print the planned steps; run nothing")
    try:
        a = ap.parse_args(argv)
    except SystemExit as e:
        return e.code if isinstance(e.code, int) else 2

    # 0. targets
    if a.mod:
        selected, bad, dirs = kc.resolve_mods(a.mod)
        if bad:
            print(f"usage error: unknown mod(s) {', '.join(bad)}; valid: {', '.join(dirs)}")
            return 2
    elif not a.all and not a.dry_run:
        print("usage error: name the mods with --mod (or pass --all); a shared worktree may hold other sessions' dirty mods. Pending:")
        print_table([r for r in load_rows() if r["verdict"] in PENDING])
        return 2
    rows = load_rows()
    if a.mod:
        wanted = set(selected)
        targets = [r for r in rows if src_dir(r) in wanted]
        missing = wanted - {src_dir(r) for r in targets}
        if missing:
            print(f"no status row for {', '.join(sorted(missing))}; nothing touched")
            return 1
    else:
        targets = [r for r in rows if r["verdict"] in PENDING]

    # 1. status check
    print_table(targets)
    refused = [r for r in targets if r["verdict"] in REFUSED]
    if refused:
        for r in refused:
            print(f"REFUSED: {r['id']} is {r['verdict']}: {REFUSED[r['verdict']]}")
        return 1
    if a.mod:
        targets = [r for r in targets if r["verdict"] != "LIVE"]
    if not targets:
        print("nothing to ship")
        return 0

    # 2. game guard
    if a.apply and hs.is_game_running():
        print("REFUSED: KOTOR II is running; close swkotor2.exe first")
        return 1

    steps = planned(a, targets)
    if a.dry_run:
        for i, (label, cmd) in enumerate(steps, 1):
            print(f"DRY  step {i}/{len(steps)} {label}: {shlex.join(cmd)}")
        if a.skip_gate:
            print("WARNING: gate skipped")
        return 0

    if a.skip_gate:
        print("WARNING: gate skipped; nothing validated this build before KPM/live")
    for label, cmd in steps:
        if label == "apply" and hs.is_game_running():
            print("REFUSED: KOTOR II started while the gate ran; close swkotor2.exe first (prepared state is untouched)")
            return 1
        rc = run_step(label, cmd)
        if rc != 0:
            msg = "GATE FAILED: halting before mirror/KPM/live" if label == "gate" else f"{label.upper()} FAILED (rc={rc}); halting"
            print(msg)
            return 1

    if not a.apply:
        print("\nprepared; run again with --apply with the game closed")
        return 0

    ids = {r["id"] for r in targets}
    final = [r for r in load_rows() if r["id"] in ids]
    print("\nfinal status:")
    print_table(final)
    not_live = sorted(r["id"] for r in final if r["verdict"] != "LIVE") + sorted(ids - {r["id"] for r in final})
    if not_live:
        print(f"NOT LIVE after apply: {', '.join(not_live)}")
        return 1
    print("shipped: all targets LIVE")
    return 0


if __name__ == "__main__":
    sys.exit(main())
