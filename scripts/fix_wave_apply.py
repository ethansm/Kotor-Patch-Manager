#!/usr/bin/env python3
"""fix_wave_apply.py - apply one approved fix-wave item to the main tree, gate it, print its close command. Python 3, stdlib only.

Usage:
  fix_wave_apply.py apply <W-A> <id> [--dry-run] [--waves-root DIR] [--repo-root DIR]
  fix_wave_apply.py apply <W-A> <id> --commit <sha> [--waves-root DIR] [--repo-root DIR]

Reads <waves>/<W-A>/items.tsv (the row for <id>), <waves>/<W-A>/<id>/review.md and <id>/final.diff, and never commits.
Gates run in the repo root, in this order, and stop at the first failure:
  1 refuse (exit 2) unless review.md says APPROVE or APPROVE_MINOR with BLOCKING 0, final.diff exists, the diff touches
    only paths in the write column, and state.tsv has no row for the id yet.
  2 git apply --check final.diff; if that fails, git apply -3. Conflict markers (or a -3 error) restore every touched file
    byte-for-byte and reset its index entry, print REBASE <id>, exit 4.
  3 run_tests.py <tests>; a write path under _shared_src first copies $KPM_DIR/Patches + lib to a scratch
    <build-root>/kpm, runs KPM_DIR=<scratch> check_shared_headers.py --sync there (never on the live clone), then
    compat_check.py --kpm-dir <scratch> <non-test shared files>.
  4 build_check.py --mod <m> --quiet --build-root /tmp/claude-1000/fixwave-<W>-<id> [--kpm-dir <scratch>], once per mod.
    Any failure: exit 5, the diff stays applied, and the revert command is printed.
  5 PASS: state.tsv row '<id>\\tgated\\t', then one close command: WI-NN -> wi_driver.py release && close --manual; other ids ->
    git commit -m "<id>: <constraint>" -- <paths>. A shared item also prints the manual live sync line (user OK). Exit 0.

--dry-run runs gate 1 and git apply --check only, prints the planned gates and changes nothing.
--commit <sha> writes state.tsv row '<id>\\tapplied\\t<sha>' with no gates.

Paths in items.tsv are repo-relative after ent(): prefixed with patch_manager_mods/ unless they start with
a ROOT_DIRS dir (patch_manager_mods/, scripts/, tests/, research/, .claude/). Exit: 0 ok; 2 refused or bad input; 4 conflict (REBASE); 5 gate failed.
"""
import argparse
import csv
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path

REPO_DEFAULT = Path(__file__).resolve().parent.parent
BUILD_ROOT = Path("/tmp/claude-1000")
KPM_DEFAULT = "~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"  # $KPM_DIR overrides, as in check_shared_headers.py
# gate helper: fresh copy of <live>/Patches and <live>/lib into <scratch> (Patches whole, so mod dir names match the live clone)
SCRATCH_KPM = ("import shutil, sys; from pathlib import Path\n"
               "live, dst = Path(sys.argv[1]), Path(sys.argv[2])\n"
               "shutil.rmtree(dst, ignore_errors=True)\n"
               "for sub in ('Patches', 'lib'):\n"
               "    shutil.copytree(live / sub, dst / sub, symlinks=True)\n")
LIVE_SYNC = "python3 scripts/check_shared_headers.py --sync"
MOD_PREFIX = "patch_manager_mods/"
ROOT_DIRS = (MOD_PREFIX, "scripts/", "tests/", "research/", ".claude/")  # repo-root dirs: no prefix
APPLY_OK = ("APPROVE", "APPROVE_MINOR")
STATE_HEADER = "id\tstage\tcommit"
WI_RE = re.compile(r"WI-\d+")
SHA_RE = re.compile(r"[0-9a-fA-F]{4,40}")
DECISION_RE = re.compile(r"^\W*DECISION:(.*)$")
BLOCKING_RE = re.compile(r"^\W*BLOCKING:(.*)$")
DIFF_HDR_RE = re.compile(r"^diff --git a/(.+) b/(.+)$")


class ApplyError(Exception):
    pass


def run(cmd, cwd):
    """Default runner: returns (rc, stdout+stderr)."""
    try:
        proc = subprocess.run([str(c) for c in cmd], cwd=str(cwd), capture_output=True, text=True,
                              encoding="utf-8", errors="replace")
    except OSError as exc:
        return 127, str(exc)
    return proc.returncode, (proc.stdout or "") + (proc.stderr or "")


# ---------------------------------------------------------------- items and paths
def ent(path):
    return path if path.startswith(ROOT_DIRS) else MOD_PREFIX + path


def lst(text):
    return [x.strip() for x in (text or "").split(";") if x.strip()]


def is_test(path):
    name = Path(path).name
    return name.endswith("_test.cpp") or (name.startswith("test_") and name.endswith(".py"))


def split_wave(wave_batch):
    wave, sep, letter = wave_batch.rpartition("-")
    if not sep or not wave or not letter:
        raise ApplyError(f"batch must look like W-A, got {wave_batch!r}")
    return wave


def item_row(batch_dir, item_id):
    path = Path(batch_dir) / "items.tsv"
    if not path.is_file():
        raise ApplyError(f"no items.tsv at {path}")
    with path.open(encoding="utf-8", newline="") as fh:
        for row in csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE):
            if (row.get("id") or "").strip() == item_id:
                return row
    raise ApplyError(f"{item_id} is not a row of {path}")


def write_paths(row):
    return [ent(p) for p in lst(row.get("write"))]


def test_ids(row):
    return lst(row.get("tests"))


def mod_names(row):
    return lst(row.get("mods"))


def touched_paths(diff_text):
    out = []
    for line in diff_text.splitlines():
        match = DIFF_HDR_RE.match(line)
        if match and match.group(2) not in out:
            out.append(match.group(2))
    return out


# ---------------------------------------------------------------- state.tsv
def read_state(batch_dir):
    path = Path(batch_dir) / "state.tsv"
    rows = {}
    if not path.is_file():
        return rows
    for line in path.read_text(encoding="utf-8").splitlines()[1:]:
        fields = line.split("\t")
        if fields and fields[0]:
            rows[fields[0]] = {"stage": fields[1] if len(fields) > 1 else "",
                               "commit": fields[2] if len(fields) > 2 else ""}
    return rows


def write_state(batch_dir, rows):
    lines = [STATE_HEADER] + [f"{k}\t{v['stage']}\t{v['commit']}" for k, v in rows.items()]
    Path(batch_dir, "state.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")


def record_commit(batch_dir, item_id, commit="", stage="applied"):
    """Upsert the state.tsv row for item_id: stage 'gated' (commit empty) or 'applied' (commit sha)."""
    rows = read_state(batch_dir)
    rows[item_id] = {"stage": stage, "commit": commit}
    write_state(batch_dir, rows)


# ---------------------------------------------------------------- readiness
def ready(batch_dir, item_id):
    """(True, reason) when review.md approves the item and nothing was applied or gated yet."""
    item_dir = Path(batch_dir) / item_id
    review = item_dir / "review.md"
    if not review.is_file():
        return False, "no review.md"
    decision = blocking = None
    for line in review.read_text(encoding="utf-8", errors="replace").splitlines():
        if decision is None:
            match = DECISION_RE.match(line)
            if match:
                words = match.group(1).strip(" *`").split()
                decision = words[0] if words else ""
        if blocking is None:
            match = BLOCKING_RE.match(line)
            if match:
                blocking = match.group(1).strip(" *`")
    if decision not in APPLY_OK:
        return False, f"DECISION is {decision or 'missing'}, need APPROVE or APPROVE_MINOR"
    try:
        count = int(blocking.split()[0]) if blocking else 0
    except (ValueError, IndexError):
        return False, f"BLOCKING is not an integer: {blocking!r}"
    if count > 0:
        return False, f"BLOCKING is {count}"
    if not (item_dir / "final.diff").is_file():
        return False, "no final.diff"
    state = read_state(batch_dir).get(item_id)
    if state and state["stage"] == "applied":
        return False, f"already applied at {state['commit']}"
    if state and state["stage"] == "gated":
        return False, "already gated: run its close command, or record the commit with --commit <sha>"
    return True, "apply-ready"


# ---------------------------------------------------------------- tree snapshot and rollback
def file_bytes(path):
    return path.read_bytes() if path.is_file() else None


def has_markers(data):
    return bool(data) and any(line.startswith((b"<<<<<<<", b">>>>>>>")) for line in data.splitlines())


def snapshot(root, rels):
    return {rel: file_bytes(Path(root) / rel) for rel in rels}


def staged_paths(runner, root):
    _, out = runner(["git", "diff", "--cached", "--name-only"], root)
    return {line.strip() for line in out.splitlines() if line.strip()}


def unstage(runner, root, rels, staged_before):
    todo = [rel for rel in rels if rel not in staged_before]
    if todo:
        runner(["git", "reset", "-q", "--", *todo], root)


def restore(root, snap, staged_before, runner):
    """Put every touched file back byte-for-byte (or remove it if it did not exist) and clear its index entry."""
    root = Path(root)
    for rel, data in snap.items():
        path = root / rel
        if data is None:
            if path.is_file() or path.is_symlink():
                path.unlink()
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
    unstage(runner, root, list(snap), staged_before)


# ---------------------------------------------------------------- gates and close command
def gate_plan(row, write, build_root, kpm_live=None):
    gates = []
    tests = test_ids(row)
    if tests:
        gates.append(("run_tests " + " ".join(tests), [sys.executable, "scripts/run_tests.py", *tests]))
    kpm_args = []
    shared = [w for w in write if "/_shared_src/" in w]
    if shared:
        # the live Patches/Common is never synced from a gate (BUILD_AND_INSTALL.md): sync a scratch copy and point
        # compat_check/build_check at it, so a new shared symbol passes the first time
        kpm_dir = Path(build_root) / "kpm"
        live = Path(kpm_live or os.environ.get("KPM_DIR") or KPM_DEFAULT).expanduser()
        gates.append((f"scratch kpm {kpm_dir}", [sys.executable, "-c", SCRATCH_KPM, str(live), str(kpm_dir)]))
        gates.append((f"KPM_DIR={kpm_dir} check_shared_headers --sync",
                      ["env", f"KPM_DIR={kpm_dir}", sys.executable, "scripts/check_shared_headers.py", "--sync"]))
        kpm_args = ["--kpm-dir", str(kpm_dir)]
        nontest = [w for w in shared if not is_test(w)]
        if nontest:
            gates.append(("compat_check " + " ".join(nontest),
                          [sys.executable, "scripts/compat_check.py", *kpm_args, *nontest]))
    for mod in mod_names(row):
        gates.append((f"build_check --mod {mod}",
                      [sys.executable, "scripts/build_check.py", "--mod", mod, "--quiet", "--build-root", str(build_root),
                       *kpm_args]))
    return gates


def shared_items(row):
    return any("/_shared_src/" in w for w in write_paths(row))


def dq(text):
    return '"' + re.sub(r'(["\\$`])', r"\\\1", text) + '"'


def close_command(row, wave_batch):
    item_id = (row.get("id") or "").strip()
    paths = " ".join(shlex.quote(p) for p in write_paths(row))
    if WI_RE.fullmatch(item_id):
        reason = dq(f"{item_id} fix wave {wave_batch}")
        # select claimed the WI as VW-<wave>; close --manual refuses while that claim is live
        return (f"python3 scripts/wi_driver.py release {item_id} --force && "
                f"python3 scripts/wi_driver.py close {item_id} --manual --paths {paths} --reason {reason}")
    text = " ".join((row.get("constraint") or "").split())[:60]
    message = f"{item_id}: {text}" if text else item_id
    return f"git commit -m {dq(message)} -- {paths}"


# ---------------------------------------------------------------- apply
def apply_item(waves_root, repo_root, wave_batch, item_id, dry_run=False, runner=run):
    root = Path(repo_root)
    batch_dir = Path(waves_root) / wave_batch
    diff = batch_dir / item_id / "final.diff"
    try:
        wave = split_wave(wave_batch)
        row = item_row(batch_dir, item_id)
        ok, reason = ready(batch_dir, item_id)
        if not ok:
            raise ApplyError(reason)
        write = write_paths(row)
        if not write:
            raise ApplyError("items.tsv write column is empty")
        touched = touched_paths(diff.read_bytes().decode("utf-8", errors="replace"))
        if not touched:
            raise ApplyError("final.diff touches no files")
        outside = [p for p in touched if p not in write]
        if outside:
            raise ApplyError("final.diff touches paths outside the write column: " + ", ".join(outside))
    except ApplyError as exc:
        print(f"REFUSED {item_id}: {exc}")
        return 2

    gates = gate_plan(row, write, BUILD_ROOT / f"fixwave-{wave}-{item_id}")
    check_rc, _ = runner(["git", "apply", "--check", str(diff)], root)
    if dry_run:
        if check_rc != 0:
            print(f"DRY-RUN CONFLICT {item_id}: git apply --check failed; a real run tries git apply -3")
            return 4
        print(f"DRY-RUN {item_id}: git apply --check OK; nothing changed. Planned gates:")
        for label, _ in gates:
            print(f"  {label}")
        return 0

    snap = snapshot(root, touched)
    staged_before = staged_paths(runner, root)
    if check_rc == 0:
        apply_rc, apply_out = runner(["git", "apply", str(diff)], root)
        if apply_rc != 0:
            print(f"CONFLICT {item_id}: git apply failed after a clean --check; nothing was changed: {apply_out.strip()[-300:]}")
            print(f"REBASE {item_id}: send consolidator a rebase request (1 allowed, then park)")
            return 4
    else:
        three_rc, three_out = runner(["git", "apply", "-3", str(diff)], root)
        preexisting = {p for p in touched if has_markers(snap[p])}
        bad = [p for p in touched if p not in preexisting and has_markers(file_bytes(root / p))]
        if three_rc != 0 or bad:
            restore(root, snap, staged_before, runner)
            detail = f"; conflict markers in {', '.join(bad)}" if bad else f": {three_out.strip()[-300:]}"
            print(f"CONFLICT {item_id}: git apply --check failed and git apply -3 did not merge cleanly{detail}. "
                  "Working tree restored byte-for-byte.")
            print(f"REBASE {item_id}: send consolidator a rebase request (1 allowed, then park)")
            return 4
        unstage(runner, root, touched, staged_before)
    print(f"APPLIED {item_id}: {len(touched)} file(s) changed in the tree")

    if not test_ids(row):
        print("SKIP run_tests: items.tsv tests column is empty")
    for label, cmd in gates:
        rc, out = runner(cmd, root)
        if rc != 0:
            print(f"GATE FAIL {label} (rc {rc}); last lines:")
            for line in out.splitlines()[-15:]:
                print(f"  {line}")
            print(f"Diff is still applied. Revert before any retry: git apply -R {diff.resolve()}")
            return 5
        print(f"PASS {label}")

    record_commit(batch_dir, item_id, "", "gated")
    print(f"GATED {item_id}: all gates passed, state.tsv -> gated. Commit with the close command only:")
    print(f"close: {close_command(row, wave_batch)}")
    if shared_items(row):
        print(f"after the commit, sync live Common (user OK): {LIVE_SYNC}")
    return 0


def record_applied(waves_root, wave_batch, item_id, sha):
    batch_dir = Path(waves_root) / wave_batch
    try:
        item_row(batch_dir, item_id)
    except ApplyError as exc:
        print(f"REFUSED {item_id}: {exc}")
        return 2
    if not SHA_RE.fullmatch(sha):
        print(f"REFUSED {item_id}: not a commit sha: {sha!r}")
        return 2
    record_commit(batch_dir, item_id, sha, "applied")
    print(f"APPLIED-RECORDED {item_id}: state.tsv -> applied {sha}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description="Apply one approved fix-wave item, gate it, print its close command.")
    sub = ap.add_subparsers(dest="cmd", required=True, metavar="apply")
    p = sub.add_parser("apply", help="apply one item from a batch (never commits)")
    p.add_argument("wave_batch", metavar="W-A")
    p.add_argument("item_id", metavar="ID")
    p.add_argument("--dry-run", action="store_true", help="gate 1 and git apply --check only; change nothing")
    p.add_argument("--commit", metavar="SHA", default=None, help="record an applied row for ID (no gates)")
    p.add_argument("--waves-root", default=None, help="waves root (default <repo>/.claude/waves)")
    p.add_argument("--repo-root", default=None, help="repo root (default: parent of scripts/)")
    args = ap.parse_args(argv)
    repo = Path(args.repo_root).resolve() if args.repo_root else REPO_DEFAULT
    waves = Path(args.waves_root).resolve() if args.waves_root else repo / ".claude" / "waves"
    if args.commit is not None:
        if args.dry_run:
            print("REFUSED: --commit and --dry-run are exclusive")
            return 2
        return record_applied(waves, args.wave_batch, args.item_id, args.commit)
    return apply_item(waves, repo, args.wave_batch, args.item_id, dry_run=args.dry_run)


if __name__ == "__main__":
    sys.exit(main())
