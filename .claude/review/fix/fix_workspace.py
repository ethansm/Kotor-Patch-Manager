#!/usr/bin/env python3
"""Fix pipeline workspace (no LLM). Contract: ../INTERFACES.md, "Fix stage" / "fix_workspace.py".

Usage:
  fix_workspace.py build-base [--force] [--repo DIR] [--head SHA] [--no-snapshot-check]
  fix_workspace.py verify-base
  fix_workspace.py make-task <tid> --attempt N [--from-patch P]
  fix_workspace.py diff <tid> --attempt N
  fix_workspace.py make-review-copy <tid> --attempt N
  fix_workspace.py slot-reset <n>          check slot n back to the pinned base commit (flock slots/<n>.lock)
  fix_workspace.py integ-reset <batch>     same for integ/<batch>
  fix_workspace.py clean <tid>             remove every copy of the task (fixer + reviewer)
  fix_workspace.py install-bin [--batches B ...]   copy fixgate.sh/fix_settings.py/fcommon.py to $FIX_HOME/bin
  fix_workspace.py --self-test

$FIX_HOME = $KOTOR2_REVIEW_HOME/fix (outside the repo; every command refuses to run if it resolves inside it).

Deviations from the contract text (see NOTES of the delivery): slots/<n> and integ/<batch> are standalone
`git clone --local` copies of base (hard-linked objects), not `git worktree`s. A worktree's .git file points at
base/.git, which is hidden inside the gate sandbox (tmpfs over $KOTOR2_REVIEW_HOME); a clone is self-contained.
Slot .git/config, hooks and attributes are rewritten before every host-side git call (a sandboxed test could
otherwise plant core.fsmonitor / hooks that the host git would run).
"""
from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
import os
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402

for _p in (str(fc.REVIEW), str(fc.REVIEW / "tools")):
    if _p not in sys.path:
        sys.path.append(_p)
import prep  # type: ignore  # noqa: E402

OVERLAY_ROOTS = ["patch_manager_mods", "scripts", "tests", "research"]
OVERLAY_MAX = 20 * 1024 * 1024
BIN_FILES = ["fixgate.sh", "fix_settings.py", "fcommon.py"]
GIT_DATE = "2026-01-01T00:00:00+0000"
SLOT_ATTRS = "* -text -eol -filter -ident -working-tree-encoding -export-subst\n"
SLOT_CONFIG = ("[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = false\n\tlogallrefupdates = true\n"
               "\tautocrlf = false\n\thooksPath = /dev/null\n[user]\n\tname = fix-pipeline\n"
               "\temail = fix-pipeline@localhost\n")


def die(msg: str, code: int = 1):
    sys.stderr.write(f"fix_workspace: {msg}\n")
    sys.exit(code)


# ------------------------------------------------------------------ git helpers
def genv() -> dict:
    e = {k: v for k, v in os.environ.items() if not k.startswith("GIT_")}
    e.update(GIT_CONFIG_GLOBAL="/dev/null", GIT_CONFIG_NOSYSTEM="1", XDG_CONFIG_HOME="/nonexistent-xdg",
             GIT_TERMINAL_PROMPT="0", GIT_AUTHOR_NAME="fix-pipeline", GIT_AUTHOR_EMAIL="fix-pipeline@localhost",
             GIT_COMMITTER_NAME="fix-pipeline", GIT_COMMITTER_EMAIL="fix-pipeline@localhost",
             GIT_AUTHOR_DATE=GIT_DATE, GIT_COMMITTER_DATE=GIT_DATE)
    return e


def git(cwd, *args, check: bool = True, inp: str | None = None) -> subprocess.CompletedProcess:
    cmd = ["git", "-c", "core.hooksPath=/dev/null", "-c", "core.fsmonitor=false", "-c", "commit.gpgsign=false",
           "-c", "core.autocrlf=false", "-c", "core.safecrlf=false", "-C", str(cwd), *args]
    p = subprocess.run(cmd, capture_output=True, text=True, errors="surrogateescape", env=genv(), input=inp)
    if check and p.returncode:
        die(f"git {' '.join(args[:3])} failed in {cwd}: {(p.stderr or p.stdout).strip()[-400:]}")
    return p


# ------------------------------------------------------------------ small helpers
def guard_home(extra_repo: str | None = None) -> None:
    fh = fc.FIX_HOME.resolve()
    for r in {fc.REPO.resolve(), Path(extra_repo).resolve() if extra_repo else fc.REPO.resolve()}:
        if fh == r or r in fh.parents:
            die(f"FIX_HOME {fh} resolves inside the repo {r}; set KOTOR2_REVIEW_HOME outside the repository")


def is_binary(path: Path) -> bool:
    try:
        if path.suffix.lower() in prep.BINARY_EXT or path.stat().st_size > prep.TREE_MAX_FILE:
            return True
    except OSError:
        return True
    return prep.looks_binary(path)


def make_writable(path: Path) -> None:
    """chmod u+rwX the whole tree (never follows symlinks); best effort.
    Hardlinked files (clone --local objects shared with base) are skipped: chmod would change the shared inode,
    make base objects writable, and race a concurrent clone's hardlink check ("hardlink different from source")."""
    path = Path(path)
    if not os.path.lexists(path):
        return
    for dp, dns, fns in os.walk(path, topdown=True):
        try:
            os.chmod(dp, os.stat(dp).st_mode | 0o700)
        except OSError:
            pass
        for n in fns:
            p = os.path.join(dp, n)
            if not os.path.islink(p):
                try:
                    st = os.lstat(p)
                    if st.st_nlink > 1:
                        continue
                    os.chmod(p, st.st_mode | 0o600)
                except OSError:
                    pass
    try:
        os.chmod(path, os.stat(path).st_mode | 0o700)
    except OSError:
        pass


def rmtree(path: Path) -> None:
    path = Path(path)
    if os.path.lexists(path):
        if path.is_symlink() or path.is_file():
            path.unlink()
            return
        make_writable(path)
        shutil.rmtree(path)


def set_readonly(root: Path, writable_files: set[str] = frozenset(), exec_of=None) -> None:
    """Files 0444 (0644 for writable_files, +x preserved via exec_of(rel)); dirs 0555 (0755 above writable files)."""
    root = Path(root)
    wdirs = set()
    for rel in writable_files:
        d = os.path.dirname(rel)
        while True:
            wdirs.add(d)
            if not d:
                break
            d = os.path.dirname(d)
    for dp, dns, fns in os.walk(root):
        for n in fns:
            p = Path(dp) / n
            if p.is_symlink():
                continue
            rel = str(p.relative_to(root))
            x = bool(exec_of(rel)) if exec_of else bool(p.stat().st_mode & 0o111)
            mode = (0o644 if rel in writable_files else 0o444) | ((0o111 if rel in writable_files else 0o111) if x else 0)
            os.chmod(p, mode)
    for dp, dns, fns in os.walk(root, topdown=False):
        rel = str(Path(dp).relative_to(root))
        rel = "" if rel == "." else rel
        os.chmod(dp, 0o755 if rel in wdirs else 0o555)


def sha_file(p: Path) -> str:
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


def glob_re(g: str) -> re.Pattern:
    out, i = [], 0
    while i < len(g):
        if g.startswith("**/", i):
            out.append("(?:.*/)?")
            i += 3
        elif g.startswith("**", i):
            out.append(".*")
            i += 2
        elif g[i] == "*":
            out.append("[^/]*")
            i += 1
        elif g[i] == "?":
            out.append("[^/]")
            i += 1
        else:
            out.append(re.escape(g[i]))
            i += 1
    return re.compile("^" + "".join(out) + "$")


def base_info() -> dict:
    b = fc.rj(fc.FIX_HOME / "base.json")
    if not isinstance(b, dict) or not b.get("base_commit"):
        die("base.json missing or invalid; run `fix_workspace.py build-base`")
    return b


def base_files() -> list[str]:
    """Files of the pinned base commit (git ls-tree), regular files only (no symlinks / submodules)."""
    bc = base_info()["base_commit"]
    p = git(fc.BASE, "ls-tree", "-r", "-z", bc)
    out = []
    for ent in p.stdout.split("\0"):
        if not ent:
            continue
        meta, _, path = ent.partition("\t")
        if meta.split()[0] in ("100644", "100755"):
            out.append(path)
    return sorted(out)


def exec_in_base(rel: str) -> bool:
    try:
        return bool(os.stat(fc.BASE / rel).st_mode & 0o111)
    except OSError:
        return False


# ------------------------------------------------------------------ clean checks
def clean_problems(root: Path) -> list[str]:
    """No excluded path, no stripped path, no escaping symlink anywhere below root (the .git dir is skipped)."""
    root = Path(root)
    bad = []
    rroot = root.resolve()
    for dp, dns, fns in os.walk(root):
        dns[:] = sorted(d for d in dns if d != ".git")
        for n in sorted(dns + fns):
            p = Path(dp) / n
            rel = str(p.relative_to(root))
            if fc.is_excluded(rel):
                bad.append(f"excluded: {rel}")
            elif prep.tree_stripped(rel):
                bad.append(f"stripped: {rel}")
            if p.is_symlink():
                real = Path(os.path.realpath(p))
                if real != rroot and rroot not in real.parents:
                    bad.append(f"escaping symlink: {rel}")
    return bad


def assert_clean(root: Path) -> None:
    bad = clean_problems(root)
    if bad:
        die("sanitized base check FAILED: " + "; ".join(bad[:10]))


# ------------------------------------------------------------------ build-base
def snapshot_ok() -> bool:
    p = subprocess.run([sys.executable, "-I", str(fc.REVIEW / "prep.py"), "verify-snapshot"],
                       capture_output=True, text=True, env={**os.environ, "KOTOR2_REVIEW_HOME": str(fc.RHOME)})
    sys.stdout.write(p.stdout)
    return p.returncode == 0


def overlay_files(repo: Path) -> dict:
    out = {}
    for top in OVERLAY_ROOTS:
        start = repo / top
        if not start.is_dir() or start.is_symlink():
            continue
        for dp, dns, fns in os.walk(start, followlinks=False):
            keep = []
            for d in sorted(dns):
                rel = str((Path(dp) / d).relative_to(repo))
                if (Path(dp) / d).is_symlink() or prep.tree_stripped(rel) or fc.is_excluded(rel) \
                        or prep.out_of_scope_part(rel + "/"):
                    continue
                keep.append(d)
            dns[:] = keep
            for fn in sorted(fns):
                p = Path(dp) / fn
                rel = str(p.relative_to(repo))
                if p.is_symlink() or not p.is_file():
                    continue
                if prep.tree_stripped(rel) or fc.is_excluded(rel) or prep.out_of_scope_part(rel):
                    continue
                try:
                    size = p.stat().st_size
                except OSError:
                    continue
                if size > OVERLAY_MAX or not is_binary(p):
                    continue
                out[rel] = p
    return out


def cmd_build_base(a) -> int:
    repo = Path(a.repo).resolve() if a.repo else fc.REPO
    guard_home(a.repo)
    if fc.BASE.exists() and not a.force:
        die(f"{fc.BASE} exists; use --force to rebuild")
    if not a.no_snapshot_check and not snapshot_ok():
        die("prep.py verify-snapshot failed; the sanitized worktree changed (rebuild with prep.py build)")
    if not fc.TREE.is_dir():
        die(f"sanitized tree {fc.TREE} missing")
    if fc.BASE.exists():
        rmtree(fc.BASE)
    fc.FIX_HOME.mkdir(parents=True, exist_ok=True)
    fc.BASE.mkdir()
    r = subprocess.run(["rsync", "-a", "--exclude=/.git", "--chmod=Du+rwx,Fu+rw", str(fc.TREE) + "/", str(fc.BASE) + "/"],
                       capture_output=True, text=True)
    if r.returncode:
        die(f"rsync failed: {r.stderr.strip()[-300:]}")
    overlay = {}
    for rel, src in overlay_files(repo).items():
        dst = fc.BASE / rel
        if dst.exists() or dst.is_symlink():
            continue
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
        os.chmod(dst, 0o755 if os.stat(src).st_mode & 0o111 else 0o644)
        overlay[rel] = sha_file(dst)
    make_writable(fc.BASE)
    head = a.head or fc.PINNED_HEAD
    git(fc.BASE, "init", "-q")
    (fc.BASE / ".git" / "info").mkdir(exist_ok=True)
    (fc.BASE / ".git" / "info" / "attributes").write_text(SLOT_ATTRS)
    git(fc.BASE, "config", "user.name", "fix-pipeline")
    git(fc.BASE, "config", "user.email", "fix-pipeline@localhost")
    git(fc.BASE, "add", "-A", "-f")
    git(fc.BASE, "commit", "-q", "--no-verify", "--no-gpg-sign", "--allow-empty", "-m", f"pinned {head}")
    bc = git(fc.BASE, "rev-parse", "HEAD").stdout.strip()
    assert_clean(fc.BASE)
    snap = fc.rj(fc.REVIEW / "snapshot.json") or {}
    msha = fc.sha_hex(json.dumps(snap.get("manifest"), sort_keys=True)) if snap.get("manifest") else None
    fc.write_json(fc.FIX_HOME / "base.json", {"head": head, "snapshot_manifest_sha256": msha, "overlay": overlay,
                                              "base_commit": bc, "built": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())})
    install_bin([])
    print(fc.dumps({"base": str(fc.BASE), "base_commit": bc, "overlay_files": len(overlay)}), end="")
    return 0


def cmd_verify_base(a) -> int:
    guard_home()
    b = fc.rj(fc.FIX_HOME / "base.json")
    if not fc.BASE.is_dir() or not isinstance(b, dict):
        print("verify-base: base missing")
        return 1
    bad = []
    head = git(fc.BASE, "rev-parse", "HEAD", check=False).stdout.strip()
    if head != b.get("base_commit"):
        bad.append(f"HEAD {head[:12]} != base_commit {str(b.get('base_commit'))[:12]}")
    st = git(fc.BASE, "status", "--porcelain", "--ignored", "-z", check=False).stdout
    if st.strip("\0"):
        bad.append("working tree not clean: " + ", ".join(x[:80] for x in st.split("\0")[:5] if x))
    for ent in git(fc.BASE, "ls-tree", "-r", "-z", "--name-only", b.get("base_commit", "HEAD"), check=False).stdout.split("\0"):
        if ent and (fc.is_excluded(ent) or prep.tree_stripped(ent)):
            bad.append(f"forbidden path in commit: {ent}")
    bad += clean_problems(fc.BASE)
    print("verify-base:", "OK" if not bad else "FAIL " + "; ".join(bad[:10]))
    return 1 if bad else 0


# ------------------------------------------------------------------ tasks
def task_dir(tid: str, n: int) -> Path:
    return fc.TASKS / f"{tid}.a{n}"


def patch_paths(text: str) -> list[str]:
    out = []
    for ln in text.splitlines():
        m = re.match(r"^(?:--- a/|\+\+\+ b/|diff --git a/)(\S+)", ln)
        if m:
            out.append(m.group(1))
    return out


def apply_patch_dir(patch: Path, dest: Path) -> None:
    text = Path(patch).read_text(encoding="utf-8", errors="replace")
    for rel in patch_paths(text):
        if fc.safe_rel(rel) is None:
            die(f"patch {patch} names an unsafe or excluded path: {rel}")
    r = subprocess.run(["patch", "-p1", "-s", "--batch", "--no-backup-if-mismatch", "-d", str(dest), "-i", str(patch)],
                       capture_output=True, text=True)
    if r.returncode:
        die(f"patch {patch} does not apply in {dest}: {(r.stdout + r.stderr).strip()[-400:]}")


def cmd_make_task(a) -> int:
    guard_home()
    tid, n = a.tid, a.attempt
    if not fc.TASK_ID_RE.match(tid):
        die(f"bad task id {tid}")
    t = fc.task_by_id(tid)
    if t.get("class") == "human-only":
        die(f"{tid} is human-only; no task copy")
    base_info()
    files = base_files()
    fileset = set(files)
    allowed = [x for x in t["allowed_files"]]
    for f in allowed:
        if fc.safe_rel(f) is None or fc.never_edit(f):
            die(f"allowed file {f} is excluded or never-edit")
        if f not in fileset:
            die(f"allowed file {f} is not in base")
        if is_binary(fc.BASE / f):
            die(f"allowed file {f} is not text")
    rx = [glob_re(g) for g in t.get("context_globs", [])]
    select = set(allowed)
    for f in files:
        if f in select or fc.is_excluded(f) or prep.tree_stripped(f):
            continue
        if any(r.match(f) for r in rx) and not is_binary(fc.BASE / f):
            select.add(f)
    dest = task_dir(tid, n)
    pre_json = fc.TASKS / f"{tid}.a{n}.pre.json"
    rmtree(dest)
    fc.TASKS.mkdir(parents=True, exist_ok=True)
    dest.mkdir()
    pre = {}
    for f in sorted(select):
        src, dst = fc.BASE / f, dest / f
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
        pre[f] = sha_file(dst)
    fc.write_json(pre_json, pre)
    if a.from_patch:
        apply_patch_dir(Path(a.from_patch), dest)
    set_readonly(dest, set(allowed), exec_in_base)
    print(fc.dumps({"copy": str(dest), "files": len(pre), "allowed": allowed}), end="")
    return 0


def is_text_bytes(data: bytes, rel: str) -> bool:
    return b"\0" not in data[:8192] and Path(rel).suffix.lower() not in prep.BINARY_EXT


def mk_diff(rel: str, old: Path, new: Path, mode: str) -> bytes:
    ob, nb = old.read_bytes(), new.read_bytes()
    oid = hashlib.sha1(b"blob %d\0" % len(ob) + ob).hexdigest()
    nid = hashlib.sha1(b"blob %d\0" % len(nb) + nb).hexdigest()
    r = subprocess.run(["diff", "-u", "--label", f"a/{rel}", "--label", f"b/{rel}", str(old), str(new)],
                       capture_output=True)
    if r.returncode not in (0, 1):
        die(f"diff failed for {rel}: {r.stderr.decode(errors='replace')[:200]}")
    if not r.stdout:
        return b""
    return f"diff --git a/{rel} b/{rel}\nindex {oid}..{nid} {mode}\n".encode() + r.stdout


def cmd_diff(a) -> int:
    guard_home()
    tid, n = a.tid, a.attempt
    copy = task_dir(tid, n)
    pre = fc.rj(fc.TASKS / f"{tid}.a{n}.pre.json")
    if not copy.is_dir() or not isinstance(pre, dict):
        die(f"no task copy / pre manifest for {tid}.a{n}")
    t = fc.task_by_id(tid)
    allowed = set(t["allowed_files"])
    cap = int(t.get("line_cap") or 0)
    viol, seen, changed, chunks = [], set(), [], []

    def v(path, reason):
        viol.append({"path": path, "reason": reason})

    for dp, dns, fns in os.walk(copy):
        dns.sort()
        fns.sort()
        for d in list(dns):
            p = Path(dp) / d
            rel = str(p.relative_to(copy))
            if p.is_symlink():
                v(rel, "symlink")
                dns.remove(d)
            elif fc.is_excluded(rel):
                v(rel, "excluded")
                dns.remove(d)
        for fn in fns:
            p = Path(dp) / fn
            rel = str(p.relative_to(copy))
            if fc.is_excluded(rel):
                v(rel, "excluded")
            elif p.is_symlink():
                v(rel, "symlink")
            elif rel not in pre:
                v(rel, "added-file")
            else:
                seen.add(rel)
    for rel in sorted(pre):
        if rel not in seen and not any(x["path"] == rel for x in viol):
            v(rel, "deleted-file")
    added = removed = 0
    for rel in sorted(seen):
        p = copy / rel
        data = p.read_bytes()
        same = hashlib.sha256(data).hexdigest() == pre[rel]
        if rel in allowed:
            if (bool(p.stat().st_mode & 0o111) != exec_in_base(rel)):
                v(rel, "mode")
                continue
        if same:
            continue
        if rel not in allowed:
            v(rel, "not-allowed")
            continue
        if not is_text_bytes(data, rel):
            v(rel, "binary")
            continue
        if sha_file(fc.BASE / rel) != pre[rel]:
            die(f"base content of {rel} changed since the task copy was made")
        chunk = mk_diff(rel, fc.BASE / rel, p, "100755" if exec_in_base(rel) else "100644")
        if not chunk:
            continue
        changed.append(rel)
        chunks.append(chunk)
        body = chunk.decode("utf-8", errors="replace").split("\n")
        i = next((k for k, ln in enumerate(body) if ln.startswith("+++ b/")), -1)
        for ln in body[i + 1:]:
            if ln.startswith("@@") or ln.startswith("\\"):
                continue
            if ln.startswith("+"):
                added += 1
            elif ln.startswith("-"):
                removed += 1
    patch = b"".join(chunks)
    fc.PATCHES.mkdir(parents=True, exist_ok=True)
    pf = fc.PATCHES / f"{tid}.a{n}.patch"
    tmp = pf.with_name(pf.name + ".tmp")
    tmp.write_bytes(patch)
    os.replace(tmp, pf)
    lines_changed = max(added, removed)
    over = bool(cap > 0 and lines_changed > cap)
    viol.sort(key=lambda x: (x["path"], x["reason"]))
    scope = {"task_id": tid, "attempt": n, "ok": not viol and not over, "empty": not patch, "changed_files": changed,
             "violations": viol, "lines_added": added, "lines_removed": removed, "lines_changed": lines_changed,
             "line_cap": cap, "over_cap": over}
    fc.write_json(fc.PATCHES / f"{tid}.a{n}.scope.json", scope)
    print(fc.dumps(scope), end="")
    return 0


def cmd_make_review_copy(a) -> int:
    guard_home()
    tid, n = a.tid, a.attempt
    pre = fc.rj(fc.TASKS / f"{tid}.a{n}.pre.json")
    scope = fc.rj(fc.PATCHES / f"{tid}.a{n}.scope.json")
    pf = fc.PATCHES / f"{tid}.a{n}.patch"
    if not isinstance(pre, dict) or not isinstance(scope, dict) or not pf.is_file():
        die(f"run `diff {tid} --attempt {n}` first (pre manifest / scope / patch missing)")
    if scope.get("violations"):
        die(f"{tid}.a{n} has scope violations; no review copy")
    dest = fc.TASKS / f"{tid}.a{n}.review"
    rmtree(dest)
    dest.mkdir(parents=True)
    for f in sorted(pre):
        if fc.is_excluded(f):
            continue
        (dest / f).parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(fc.BASE / f, dest / f)
    if pf.stat().st_size:
        apply_patch_dir(pf, dest)
    for f in scope.get("changed_files", []):
        (dest / "_pre" / f).parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(fc.BASE / f, dest / "_pre" / f)
    set_readonly(dest, set(), exec_in_base)
    print(fc.dumps({"review_copy": str(dest), "changed": scope.get("changed_files", [])}), end="")
    return 0


def cmd_clean(a) -> int:
    guard_home()
    n = 0
    if fc.TASKS.is_dir():
        for p in sorted(fc.TASKS.glob(f"{a.tid}.a*")):
            rmtree(p)
            n += 1
    print(f"clean: removed {n} entries for {a.tid}")
    return 0


# ------------------------------------------------------------------ slots / integ clones
def harden_git(root: Path) -> None:
    g = Path(root) / ".git"
    if g.is_symlink() or not g.is_dir():
        die(f"{root}/.git is not a plain directory")
    make_writable(g)
    (g / "config").write_text(SLOT_CONFIG)
    rmtree(g / "hooks")
    (g / "info").mkdir(exist_ok=True)
    (g / "info" / "attributes").write_text(SLOT_ATTRS)
    for junk in ("objects/info/alternates", "info/sparse-checkout", "index.lock", "shallow"):
        try:
            (g / junk).unlink()
        except OSError:
            pass


def reset_clone(path: Path) -> str:
    bc = base_info()["base_commit"]
    path = Path(path)
    if not (path / ".git").exists():
        path.parent.mkdir(parents=True, exist_ok=True)
        if path.exists():
            rmtree(path)
        git(path.parent, "clone", "-q", "--local", "--no-checkout", str(fc.BASE), str(path))
        harden_git(path)
        git(path, "checkout", "-q", "-f", "--detach", bc)
    else:
        make_writable(path)
        harden_git(path)
        git(path, "checkout", "-q", "-f", "--detach", bc)
        git(path, "reset", "-q", "--hard", bc)
        git(path, "clean", "-fdxq")
    head = git(path, "rev-parse", "HEAD").stdout.strip()
    if head != bc:
        die(f"{path} is at {head[:12]}, expected {bc[:12]}")
    return head


def slot_reset(n: int | str, take_lock: bool = True) -> Path:
    n = str(n)
    if not re.fullmatch(r"[0-9]{1,3}", n):
        die(f"bad slot {n}")
    slot = fc.SLOTS / n
    fc.SLOTS.mkdir(parents=True, exist_ok=True)
    if not take_lock:
        reset_clone(slot)
        return slot
    with open(fc.SLOTS / f"{n}.lock", "w") as lk:
        fcntl.flock(lk, fcntl.LOCK_EX)
        reset_clone(slot)
    return slot


def integ_reset(batch: str) -> Path:
    if not fc.ID_RE.match(batch):
        die(f"bad batch id {batch}")
    d = fc.INTEG_HOME / batch
    reset_clone(d)
    return d


# ------------------------------------------------------------------ bin
def _atomic_write(dst: Path, data: bytes, mode: int) -> None:
    """Replace dst by rename, never in place: bash reads a running script incrementally and a bwrap --ro-bind
    pins the old inode, so an in-place rewrite corrupts a gate running in a parallel batch (bug 16)."""
    if dst.is_file() and not dst.is_symlink() and dst.read_bytes() == data:
        os.chmod(dst, mode)
        return
    tmp = dst.with_name(f".{dst.name}.{os.getpid()}.tmp")
    tmp.write_bytes(data)
    os.chmod(tmp, mode)
    os.replace(tmp, dst)


def install_bin(batches: list[str]) -> list[str]:
    guard_home()
    b = fc.FIX_HOME / "bin"
    b.mkdir(parents=True, exist_ok=True)
    for f in BIN_FILES:
        src = fc.CODE_DIR / f
        if not src.is_file():
            die(f"{src} missing")
        _atomic_write(b / f, src.read_bytes(), 0o755 if f.endswith(".sh") else 0o644)
    if not batches and fc.BATCHES.is_dir():
        for j in sorted(fc.BATCHES.glob("batch_*.json")):
            d = fc.rj(j)
            batches.append((d or {}).get("batch") or j.stem)
    made = []
    for bt in batches:
        if not fc.ID_RE.match(bt):
            die(f"bad batch id {bt}")
        w = b / f"fixgate_{bt}.sh"
        _atomic_write(w, f'#!/usr/bin/env bash\nexec "{b}/fixgate.sh" "{fc.INTEG_HOME / bt}" "$@"\n'.encode(), 0o755)
        made.append(str(w))
    return made


# ------------------------------------------------------------------ self-test world (shared with fix_checks/fix_settings)
STUB_BUILD = '''import argparse, pathlib, sys
ap = argparse.ArgumentParser(); ap.add_argument("--mod", action="append", default=[]); ap.add_argument("--repo")
ap.add_argument("--build-root"); ap.add_argument("--quiet", action="store_true"); a = ap.parse_args()
bad = 0
for m in a.mod:
    for p in sorted((pathlib.Path(a.repo) / "patch_manager_mods" / m).glob("*.cpp")):
        if "SYNTAX_ERROR" in p.read_text():
            print("FAIL", m, p.name); bad = 1
print("BUILD", "FAIL" if bad else "PASS", a.mod)
sys.exit(bad)
'''
STUB_TESTS = '''import argparse, json, pathlib, subprocess, sys
ROOT = pathlib.Path(__file__).resolve().parent.parent
ap = argparse.ArgumentParser(); ap.add_argument("filters", nargs="*"); ap.add_argument("--json"); ap.add_argument("--jobs")
a = ap.parse_args()
res = []
for p in sorted((ROOT / "scripts").glob("test_*.py")):
    i = "py:scripts/" + p.name
    if a.filters and not any(f in i for f in a.filters):
        continue
    r = subprocess.run([sys.executable, str(p)], cwd=ROOT, capture_output=True, text=True)
    res.append({"id": i, "kind": "py", "status": "PASS" if r.returncode == 0 else "FAIL", "seconds": 0, "reason": "",
                "tail": (r.stdout + r.stderr)[-200:]})
counts = {k: sum(1 for x in res if x["status"] == k) for k in ("PASS", "FAIL", "SKIP", "ERROR")}
print("%d passed, %d failed (%d items)" % (counts["PASS"], counts["FAIL"], len(res)))
if a.json:
    pathlib.Path(a.json).write_text(json.dumps({"counts": counts, "results": res}))
sys.exit(1 if counts["FAIL"] else 0)
'''
STUB_COMPAT = '''import argparse, json, pathlib
ap = argparse.ArgumentParser(); ap.add_argument("files", nargs="*"); ap.add_argument("--repo"); ap.add_argument("--list", action="store_true")
ap.add_argument("--json"); ap.add_argument("--jobs"); a = ap.parse_args()
mods = sorted(p.name for p in (pathlib.Path(a.repo) / "patch_manager_mods").glob("*_src") if p.name != "_shared_src")
plan = [{"kind": "build", "id": "build_check:" + m, "why": a.files, "cmd": []} for m in mods]
if a.json:
    pathlib.Path(a.json).write_text(json.dumps({"touched": a.files, "plan": plan, "results": []}))
print("compat", "plan" if a.list else "run", mods)
'''
STUB_DIFFCHECK = '''import json, sys
print(json.dumps({"verdict": "PASS"}))
'''

FAKE_TREE = {
    "scripts/a.py": "def add(x, y):\n    return x + y\n\n\ndef sub(x, y):\n    return x - y\n",
    "scripts/test_a.py": "import sys\nsys.path.insert(0, 'scripts')\nimport a\nassert a.add(1, 2) == 3\nassert a.add(2, 2) == 4\nassert a.sub(3, 1) == 2\n",
    "scripts/test_old.py": "import sys\nsys.exit(1)\n",
    "scripts/b.py": "X = 1\n",
    "scripts/ledger_target.txt": "",
    "scripts/build_check.py": STUB_BUILD,
    "scripts/run_tests.py": STUB_TESTS,
    "scripts/compat_check.py": STUB_COMPAT,
    "scripts/agent_diff_check.py": STUB_DIFFCHECK,
    "patch_manager_mods/foo_src/Foo.cpp": "int foo() { return 1; }\n",
    "patch_manager_mods/foo_src/foo_test.cpp": "#include \"../_shared_src/H.h\"\nint main() {\n    KT_CHECK(foo() == 1);\n    KT_CHECK(foo() != 2);\n    return 0;\n}\n",
    "patch_manager_mods/foo_src/exports.def": "EXPORTS\n    foo\n",
    "patch_manager_mods/_shared_src/H.h": "#pragma once\n#define KT_CHECK(x) ((void)(x))\n",
    "tests/include/t.h": "#pragma once\n",
    "README.md": "fake\n",
}
FAKE_TASKS = [
    ("T0000000001", ["scripts/a.py"], ["scripts/*.py", "scripts/*.toml", "scripts/*.txt"], None, 40, {}),
    ("T0000000002", ["scripts/test_a.py"], ["scripts/*.py"], None, 40, {}),
    ("T0000000003", ["patch_manager_mods/foo_src/foo_test.cpp"], ["patch_manager_mods/foo_src/**", "patch_manager_mods/_shared_src/**"], "foo_src", 40, {}),
    ("T0000000004", ["patch_manager_mods/foo_src/Foo.cpp"], ["patch_manager_mods/foo_src/**", "patch_manager_mods/_shared_src/**", "tests/include/**"], "foo_src", 40, {}),
    ("T0000000005", ["patch_manager_mods/_shared_src/H.h"], ["patch_manager_mods/**", "tests/include/**"], "_shared_src", 120, {}),
    ("T0000000006", ["scripts/b.py"], ["scripts/*.py"], None, 2, {}),
    ("T0000000007", ["scripts/test_a.py"], ["scripts/*.py", "scripts/*.txt"], None, 40, {"sandbox_tests_only": True}),
    ("T0000000008", ["scripts/test_old.py"], ["scripts/*.py"], None, 40, {}),
    ("T000000000a", ["scripts/a.py"], ["scripts/*.py"], None, 40, {}),
]


def fake_world(T: Path) -> dict:
    """Create a fake sanitized tree + fake 'real repo' (binary overlay source) + queue under T. Returns paths."""
    T = Path(T)
    tree, repo = T / "rh" / "worktree", T / "repo"
    for root in (tree,):
        for rel, txt in FAKE_TREE.items():
            p = root / rel
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(txt)
    (tree / "scripts" / "ledger_target.txt").write_text(str(T / "guard_ledger.jsonl") + "\n")
    os.symlink("a.py", tree / "scripts" / "alias.py")
    (repo / "patch_manager_mods" / "foo_src").mkdir(parents=True)
    (repo / "patch_manager_mods" / "foo_src" / "tex.tga").write_bytes(b"\0TGA" * 100)
    (repo / "patch_manager_mods" / "foo_src" / "my_secret.bin").write_bytes(b"\0SECRET")
    (repo / "scripts" / "__pycache__").mkdir(parents=True)
    (repo / "scripts" / "__pycache__" / "x.pyc").write_bytes(b"\0pyc")
    (repo / "scripts" / "notes.txt").write_text("text, not overlaid\n")
    q = {"tasks": []}
    for tid, allowed, ctx, mod, cap, flags in FAKE_TASKS:
        q["tasks"].append({"task_id": tid, "class": "auto-fixable", "allowed_files": allowed, "context_globs": ctx,
                           "mod": mod, "line_cap": cap, "flags": flags, "primary_file": allowed[0]})
    fc.write_json(T / "work" / "fix_queue.json", q)
    return {"tree": tree, "repo": repo}


def selftest_env(T: Path, extra: dict | None = None) -> dict:
    e = dict(os.environ)
    e.update(KOTOR2_REVIEW_HOME=str(T / "rh"), FIX_DIR=str(T / "fixdir"), FIX_WORK=str(T / "work"),
             FIX_SANDBOX="none", KOTOR2_FIX_OUT=str(T / "out"), KOTOR2_FIX_REPORT=str(T / "rep"),
             FIX_FINDINGS=str(T / "none.json"))
    e.update(extra or {})
    return e


def reexec_selftest(script: str, inner_flag: str = "--self-test-inner", extra_env: dict | None = None) -> int:
    T = Path(tempfile.mkdtemp(prefix="fixst_"))
    try:
        fake_world(T)
        r = subprocess.run([sys.executable, "-I", script, inner_flag, str(T)], env=selftest_env(T, extra_env))
        return r.returncode
    finally:
        rmtree(T)


# ------------------------------------------------------------------ self-test
def run_cli(*args, env=None, expect=0):
    p = subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), *args], capture_output=True, text=True,
                       env=env or os.environ)
    if expect is not None and p.returncode != expect:
        raise AssertionError(f"{' '.join(args)} -> rc {p.returncode}, wanted {expect}: {p.stdout[-300:]} {p.stderr[-300:]}")
    return p


def self_test_inner(T: Path) -> int:
    bad = []
    n_chk = [0]

    def chk(cond, msg):
        n_chk[0] += 1
        if not cond:
            bad.append(msg)

    env = dict(os.environ)
    repo = T / "repo"
    # refuse FIX_HOME inside the repo
    e2 = dict(env, KOTOR2_REVIEW_HOME=str(fc.REPO / ".claude" / "never_created_rh"))
    p = run_cli("build-base", "--no-snapshot-check", env=e2, expect=None)
    chk(p.returncode != 0 and "inside the repo" in p.stderr, "FIX_HOME inside repo not refused")
    chk(not (fc.REPO / ".claude" / "never_created_rh").exists(), "refusal created dirs")
    # build-base
    run_cli("build-base", "--no-snapshot-check", "--repo", str(repo), env=env)
    chk(fc.rj(fc.FIX_HOME / "base.json") is not None, "base.json")
    bj = fc.rj(fc.FIX_HOME / "base.json")
    chk("patch_manager_mods/foo_src/tex.tga" in bj["overlay"], "binary overlay missing")
    chk(not (fc.BASE / "patch_manager_mods/foo_src/my_secret.bin").exists(), "excluded overlay file copied")
    chk(not (fc.BASE / "scripts/__pycache__").exists(), "pycache overlaid")
    chk(not (fc.BASE / "scripts/notes.txt").exists(), "text file overlaid")
    chk(run_cli("verify-base", env=env, expect=None).returncode == 0, "verify-base on fresh base")
    chk(run_cli("build-base", "--no-snapshot-check", "--repo", str(repo), env=env, expect=None).returncode != 0,
        "build-base without --force overwrote")
    chk((fc.FIX_HOME / "bin" / "fixgate.sh").exists() if (fc.CODE_DIR / "fixgate.sh").exists() else True, "bin")
    # make-task + the injected excluded files (committed to base on purpose)
    (fc.BASE / ".env").write_text("TOKEN=1\n")
    (fc.BASE / "scripts" / "my_secret_x.py").write_text("S=1\n")
    git(fc.BASE, "add", "-f", "-A")
    git(fc.BASE, "commit", "-q", "-m", "inject", "--no-verify")
    bj["base_commit"] = git(fc.BASE, "rev-parse", "HEAD").stdout.strip()
    fc.write_json(fc.FIX_HOME / "base.json", bj)
    chk(run_cli("verify-base", env=env, expect=None).returncode != 0, "verify-base missed excluded files")
    t1 = "T0000000001"
    run_cli("make-task", t1, "--attempt", "1", env=env)
    c = fc.TASKS / f"{t1}.a1"
    chk((c / "scripts/a.py").is_file() and (c / "scripts/b.py").is_file(), "task copy files")
    chk(not (c / ".env").exists() and not (c / "scripts/my_secret_x.py").exists(), "excluded file in task copy")
    chk(not os.path.lexists(c / "scripts/alias.py"), "symlink copied")
    chk(not (c / "patch_manager_mods").exists(), "unrelated mod copied")
    chk(stat.S_IMODE((c / "scripts/a.py").stat().st_mode) == 0o644, "allowed perms")
    chk(stat.S_IMODE((c / "scripts/b.py").stat().st_mode) == 0o444, "context perms")
    chk(stat.S_IMODE((c / "scripts").stat().st_mode) == 0o755, "allowed dir perms")
    chk(stat.S_IMODE(c.stat().st_mode) == 0o755 or stat.S_IMODE(c.stat().st_mode) == 0o555, "root perms")
    chk((fc.TASKS / f"{t1}.a1.pre.json").is_file() and ".env" not in fc.rj(fc.TASKS / f"{t1}.a1.pre.json"), "pre manifest")
    # clean diff, empty
    s = json.loads(run_cli("diff", t1, "--attempt", "1", env=env).stdout)
    chk(s["empty"] and s["ok"] and not s["violations"], f"empty diff scope {s}")
    # good edit
    a = c / "scripts/a.py"
    a.write_text(a.read_text().replace("return x - y", "return x - y  # checked"))
    s = json.loads(run_cli("diff", t1, "--attempt", "1", env=env).stdout)
    chk(s["ok"] and not s["empty"] and s["changed_files"] == ["scripts/a.py"] and s["lines_changed"] == 1
        and s["lines_added"] == 1 and s["lines_removed"] == 1 and not s["over_cap"], f"good diff scope {s}")
    pf = fc.PATCHES / f"{t1}.a1.patch"
    chk(git(fc.BASE, "apply", "--check", str(pf), check=False).returncode == 0, "git apply --check on base")
    chk(pf.read_text().startswith("diff --git a/scripts/a.py b/scripts/a.py\nindex ") and "--- a/scripts/a.py\n+++ b/scripts/a.py\n" in pf.read_text(), "patch headers")
    # out-of-scope edit + added file + deleted file + excluded add + symlink
    b = c / "scripts/b.py"
    os.chmod(b, 0o644)
    b.write_text("X = 2\n")
    os.chmod(c / "scripts", 0o755)
    (c / "scripts/new.py").write_text("N=1\n")
    (c / ".env").write_text("TOKEN=2\n")
    os.unlink(c / "scripts/test_old.py")
    os.symlink("a.py", c / "scripts/lnk.py")
    s = json.loads(run_cli("diff", t1, "--attempt", "1", env=env).stdout)
    got = {(x["path"], x["reason"]) for x in s["violations"]}
    for want in [("scripts/b.py", "not-allowed"), ("scripts/new.py", "added-file"), (".env", "excluded"),
                 ("scripts/test_old.py", "deleted-file"), ("scripts/lnk.py", "symlink")]:
        chk(want in got, f"violation missing {want}: {sorted(got)}")
    chk(not s["ok"], "violations but ok")
    chk("scripts/b.py" not in pf.read_text(), "out-of-scope file leaked into patch")
    # review copy refused with violations; works after restoring
    chk(run_cli("make-review-copy", t1, "--attempt", "1", env=env, expect=None).returncode != 0, "review copy with violations")
    # over cap + mode
    t6 = "T0000000006"
    run_cli("make-task", t6, "--attempt", "1", env=env)
    c6 = fc.TASKS / f"{t6}.a1"
    (c6 / "scripts/b.py").write_text("X = 1\nY = 2\nZ = 3\nW = 4\n")
    s = json.loads(run_cli("diff", t6, "--attempt", "1", env=env).stdout)
    chk(s["over_cap"] and s["line_cap"] == 2 and s["lines_changed"] == 3 and not s["ok"], f"over cap {s}")
    # review copy on a clean patch
    t2 = "T0000000002"
    run_cli("make-task", t2, "--attempt", "1", env=env)
    c2 = fc.TASKS / f"{t2}.a1"
    ta = c2 / "scripts/test_a.py"
    ta.write_text(ta.read_text() + "assert a.add(0, 0) == 0\n")
    s2 = json.loads(run_cli("diff", t2, "--attempt", "1", env=env).stdout)
    chk(s2["ok"], f"t2 scope {s2}")
    run_cli("make-review-copy", t2, "--attempt", "1", env=env)
    rc = fc.TASKS / f"{t2}.a2.review"
    rc = fc.TASKS / f"{t2}.a1.review"
    chk((rc / "scripts/test_a.py").read_text() == ta.read_text(), "review post-image")
    chk((rc / "_pre/scripts/test_a.py").read_text() == (fc.BASE / "scripts/test_a.py").read_text(), "review _pre")
    chk(stat.S_IMODE((rc / "scripts/test_a.py").stat().st_mode) == 0o444 and stat.S_IMODE(rc.stat().st_mode) == 0o555, "review read-only")
    # --from-patch reproduces the patch
    run_cli("make-task", t2, "--attempt", "2", "--from-patch", str(fc.PATCHES / f"{t2}.a1.patch"), env=env)
    s3 = json.loads(run_cli("diff", t2, "--attempt", "2", env=env).stdout)
    chk((fc.PATCHES / f"{t2}.a2.patch").read_bytes() == (fc.PATCHES / f"{t2}.a1.patch").read_bytes() and s3["ok"], "from-patch roundtrip")
    # slot-reset: clone, dirty, reset; patch applies with --3way in the slot
    run_cli("slot-reset", "1", env=env)
    slot = fc.SLOTS / "1"
    chk(git(slot, "rev-parse", "HEAD").stdout.strip() == bj["base_commit"], "slot at base")
    chk(git(slot, "apply", "--3way", str(fc.PATCHES / f"{t2}.a1.patch")).returncode == 0, "3way apply in slot")
    (slot / "junk.txt").write_text("x")
    os.makedirs(slot / "rodir")
    (slot / "rodir" / "f").write_text("x")
    os.chmod(slot / "rodir", 0o555)
    (slot / ".git" / "hooks").mkdir(exist_ok=True)
    (slot / ".git" / "hooks" / "post-checkout").write_text("#!/bin/sh\ntouch /tmp/.fix_hook_ran\n")
    with open(slot / ".git" / "config", "a") as fh:
        fh.write("[core]\n\tfsmonitor = /bin/false\n")
    run_cli("slot-reset", "1", env=env)
    chk(not (slot / "junk.txt").exists() and not (slot / "rodir").exists(), "slot not clean after reset")
    chk(git(slot, "status", "--porcelain").stdout == "", "slot dirty after reset")
    chk("fsmonitor" not in (slot / ".git" / "config").read_text() and not (slot / ".git" / "hooks").exists(), "slot git not hardened")
    objs = [p for p in (fc.BASE / ".git" / "objects").glob("??/*") if p.is_file()]
    chk(bool(objs) and not any(os.stat(p).st_mode & 0o222 for p in objs), "slot reset made shared base objects writable")
    # fixgate reset keeps the integrator's _patches/_resolved (bug 13)
    for d in ("_patches", "_resolved"):
        (slot / d).mkdir(exist_ok=True)
        (slot / d / "x.patch").write_text("x")
    (slot / "junk2.txt").write_text("x")
    subprocess.run(["bash", str(fc.CODE_DIR / "fixgate.sh"), str(slot), "reset"], capture_output=True,
                   env={**(env or os.environ), "FIX_SANDBOX": "none"})
    chk((slot / "_patches" / "x.patch").exists() and (slot / "_resolved" / "x.patch").exists()
        and not (slot / "junk2.txt").exists(), "fixgate reset must keep _patches/_resolved and clean the rest")
    # install-bin + wrappers
    made = install_bin(["batch_01"])
    chk(all(Path(m).exists() for m in made), "wrapper")
    chk("batch_01" in Path(made[0]).read_text(), "wrapper root baked")
    ino = os.stat(fc.FIX_HOME / "bin" / "fixgate.sh").st_ino
    install_bin(["batch_01"])
    chk(os.stat(fc.FIX_HOME / "bin" / "fixgate.sh").st_ino == ino, "unchanged bin file must not be rewritten")
    (fc.FIX_HOME / "bin" / "fixgate.sh").write_text("old")
    ino = os.stat(fc.FIX_HOME / "bin" / "fixgate.sh").st_ino
    install_bin(["batch_01"])
    chk(os.stat(fc.FIX_HOME / "bin" / "fixgate.sh").st_ino != ino, "changed bin file must be replaced by rename (bug 16)")
    # clean
    run_cli("clean", t1, env=env)
    chk(not list(fc.TASKS.glob(f"{t1}.a*")), "clean left files")
    print("fix_workspace self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad), f"({n_chk[0]} checks)")
    return 1 if bad else 0


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return reexec_selftest(str(Path(__file__).resolve()))
    if argv[:1] == ["--self-test-inner"]:
        return self_test_inner(Path(argv[1]))
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("build-base")
    s.add_argument("--force", action="store_true")
    s.add_argument("--repo")
    s.add_argument("--head")
    s.add_argument("--no-snapshot-check", action="store_true")
    sub.add_parser("verify-base")
    for name in ("make-task", "diff", "make-review-copy"):
        s = sub.add_parser(name)
        s.add_argument("tid")
        s.add_argument("--attempt", type=int, required=True)
        if name == "make-task":
            s.add_argument("--from-patch")
    sub.add_parser("slot-reset").add_argument("n")
    sub.add_parser("integ-reset").add_argument("batch")
    sub.add_parser("clean").add_argument("tid")
    s = sub.add_parser("install-bin")
    s.add_argument("--batches", nargs="*", default=[])
    a = ap.parse_args(argv)
    if a.cmd == "build-base":
        return cmd_build_base(a)
    if a.cmd == "verify-base":
        return cmd_verify_base(a)
    if a.cmd == "make-task":
        return cmd_make_task(a)
    if a.cmd == "diff":
        return cmd_diff(a)
    if a.cmd == "make-review-copy":
        return cmd_make_review_copy(a)
    if a.cmd == "clean":
        return cmd_clean(a)
    guard_home()
    if a.cmd == "slot-reset":
        print(slot_reset(a.n))
    elif a.cmd == "integ-reset":
        print(integ_reset(a.batch))
    elif a.cmd == "install-bin":
        for m in install_bin(list(a.batches)):
            print(m)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
