#!/usr/bin/env python3
"""assertion_delta.py - per-test-file assertion counts and removed assertion lines.

Input : <review-dir>/work/hunks.jsonl (class "a" records only are used).
Output: <review-dir>/work/assertion_delta.json (format: .claude/review/INTERFACES.md).
Usage : python3 -I tools/assertion_delta.py --review-dir D --repo R --tree T --base 11e1284
        python3 -I tools/assertion_delta.py --self-test

SECURITY: never reads, git-shows, or opens a path matching is_excluded() (.env, *.pem,
.credentials.json, sunshine/credentials/*, endpoint.env, tests/static/raw_addresses.toml,
*credential*, *secret*; case-insensitive, any depth). Post-image content comes only from the
sanitized --tree; base content only via read-only `git show <base>:<path>` for paths that
appear in hunks.jsonl. This module also hosts helpers shared with the sibling tools.
"""
import argparse
import collections
import json
import os
import posixpath
import re
import shutil
import subprocess
import sys
import tempfile

C_ASSERT = re.compile(r"\b(KT_CHECK\w*|CHECK\w*|EXPECT_\w+|ASSERT_\w+|REQUIRE\w*|assert)\s*\(")
PY_ASSERT = re.compile(r"\b(self\.assert\w+|assert\s|pytest\.raises)")
DEFINE = re.compile(r"^\s*#\s*define\b")
COMMENT_LINE = re.compile(r"^\s*(//|/\*|\*\s)")
NEAR = 40


class InputError(Exception):
    pass


def is_excluded(path):
    """True for any path the pipeline must never read (case-insensitive, any depth)."""
    if not path:
        return False
    p = path.replace("\\", "/").lower()
    parts = [x for x in p.split("/") if x]
    base = parts[-1] if parts else ""
    if ".env" in parts or base.endswith(".env") or base.startswith(".env."):
        return True
    if base.endswith(".pem") or base == ".credentials.json":
        return True
    for i in range(len(parts) - 1):
        if parts[i] == "sunshine" and parts[i + 1] == "credentials":
            return True
    if "/".join(parts[-3:]) == "tests/static/raw_addresses.toml":
        return True
    return "credential" in p or "secret" in p


def is_python(path):
    return path.lower().endswith(".py")


def is_test_file(path):
    name = posixpath.basename(path)
    if (name.endswith(("_test.cpp", "_test.c", "_test.py")) or
            (name.startswith("test_") and name.endswith(".py"))):
        return True
    return "tools/parity/" in ("/" + path) and name.endswith(".cpp")


def is_assert_line(line, path):
    if DEFINE.match(line):
        return False
    if is_python(path):
        return bool(PY_ASSERT.search(line)) and not line.lstrip().startswith("#")
    return bool(C_ASSERT.search(line)) and not COMMENT_LINE.match(line)


def count_assertions(text, path):
    """Counter of macro -> occurrences; #define lines and comment lines are skipped."""
    rx = PY_ASSERT if is_python(path) else C_ASSERT
    cnt = collections.Counter()
    for line in (text or "").splitlines():
        if is_assert_line(line, path):
            for m in rx.finditer(line):
                cnt[m.group(1).strip()] += 1
    return cnt


def load_hunks(review_dir):
    path = os.path.join(review_dir, "work", "hunks.jsonl")
    if not os.path.isfile(path):
        raise InputError(f"missing {path}")
    recs = []
    with open(path, encoding="utf-8") as fh:
        for n, line in enumerate(fh, 1):
            if not line.strip():
                continue
            try:
                r = json.loads(line)
                for k in ("hunk_id", "file", "class"):
                    r[k]
            except (ValueError, KeyError, TypeError) as e:
                raise InputError(f"{path}:{n}: bad record ({e!r})")
            if not isinstance(r, dict):
                raise InputError(f"{path}:{n}: not an object")
            for k in ("minus", "plus"):
                r[k] = r.get(k) or []
            for k in ("old_start", "old_len", "new_start", "new_len"):
                r[k] = r.get(k) or 0
            recs.append(r)
    return recs


def filter_records(recs):
    """-> (sorted class-a records, number of excluded-path records skipped)."""
    keep, nexc = [], 0
    for r in recs:
        if r["class"] == "excluded" or is_excluded(r["file"]) or is_excluded(r.get("old_path")):
            nexc += 1
        elif r["class"] == "a":
            keep.append(r)
    keep.sort(key=lambda r: (r["file"], r["new_start"], r["old_start"], r["hunk_id"]))
    return keep, nexc


def git_show(repo, ref, path):
    """Read-only `git show ref:path` -> bytes or None (missing/excluded)."""
    if not path or not ref or is_excluded(path):
        return None
    try:
        r = subprocess.run(["git", "-C", repo, "show", f"{ref}:{path}"],
                           capture_output=True, timeout=60)
    except (OSError, subprocess.SubprocessError):
        return None
    return r.stdout if r.returncode == 0 else None


def read_tree(tree, rel):
    """Bytes of <tree>/<rel> or None; never leaves the tree, never reads excluded paths."""
    if not rel or is_excluded(rel):
        return None
    norm = posixpath.normpath(rel)
    if norm.startswith("..") or norm.startswith("/"):
        return None
    full = os.path.join(tree, norm)
    root = os.path.realpath(tree)
    if not os.path.realpath(full).startswith(root + os.sep) or not os.path.isfile(full):
        return None
    with open(full, "rb") as fh:
        return fh.read()


def base_text(repo, rec):
    """Base-image text for a record (empty if added/untracked/missing)."""
    if rec.get("diff_base") is None:
        return ""
    data = git_show(repo, rec["diff_base"], rec.get("old_path") or rec["file"])
    return (data or b"").decode("utf-8", "replace")


def dump_json(path, obj):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(json.dumps(obj, sort_keys=True, indent=1, ensure_ascii=False) + "\n")


def cli_main(prog, out_name, run, self_test, argv=None):
    ap = argparse.ArgumentParser(prog=prog)
    ap.add_argument("--review-dir")
    ap.add_argument("--repo")
    ap.add_argument("--tree")
    ap.add_argument("--base", default="11e1284")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args(argv)
    if a.self_test:
        self_test()
        return 0
    if not (a.review_dir and a.repo and a.tree):
        print(f"{prog}: --review-dir, --repo and --tree are required", file=sys.stderr)
        return 2
    try:
        out, summary = run(a.review_dir, a.repo, a.tree, a.base)
    except InputError as e:
        print(f"{prog}: bad input: {e}", file=sys.stderr)
        return 2
    dump_json(os.path.join(a.review_dir, "work", out_name), out)
    print(f"{prog}: {summary}")
    return 0


def make_record(file, **kw):
    r = {"hunk_id": "h-" + format(abs(hash((file, repr(sorted(kw.items()))))) % 16**12, "012x"),
         "file": file, "old_path": file, "source": "committed", "diff_base": "11e1284",
         "diff_target": "HEAD", "kind": "diff", "status": "M", "old_start": 1, "old_len": 0,
         "new_start": 1, "new_len": 0, "minus": [], "plus": [], "symlink_target": None,
         "class": "a", "class_reason": "", "domain": "wp1", "commits": []}
    r.update(kw)
    r["old_len"] = kw.get("old_len", len(r["minus"]))
    r["new_len"] = kw.get("new_len", len(r["plus"]))
    return r


def make_env(records, files):
    """Synthetic review dir + tree under a temp dir. -> (tmp, review_dir, tree)."""
    tmp = tempfile.mkdtemp(prefix="review_selftest_")
    review, tree = os.path.join(tmp, "review"), os.path.join(tmp, "tree")
    os.makedirs(os.path.join(review, "work"))
    for rel, txt in files.items():
        os.makedirs(os.path.dirname(os.path.join(tree, rel)) or tree, exist_ok=True)
        with open(os.path.join(tree, rel), "w", encoding="utf-8") as fh:
            fh.write(txt)
    os.makedirs(tree, exist_ok=True)
    with open(os.path.join(review, "work", "hunks.jsonl"), "w", encoding="utf-8") as fh:
        for r in records:
            fh.write(json.dumps(r, sort_keys=True) + "\n")
    return tmp, review, tree


def run(review_dir, repo, tree, base):
    recs, nexc = filter_records(load_hunks(review_dir))
    byfile = collections.OrderedDict()
    for r in recs:
        if is_test_file(r["file"]):
            byfile.setdefault(r["file"], []).append(r)
    files, nrem = [], 0
    for f, rs in byfile.items():
        bc = count_assertions(base_text(repo, rs[0]), f)
        nc = count_assertions((read_tree(tree, f) or b"").decode("utf-8", "replace"), f)
        added = [(r["new_start"] + j, t.strip()) for r in rs for j, t in enumerate(r["plus"])
                 if is_assert_line(t, f)]
        removed = []
        for r in rs:
            for i, t in enumerate(r["minus"]):
                if not is_assert_line(t, f):
                    continue
                mapped = r["new_start"] + (min(i, r["new_len"] - 1) if r["new_len"] > 0 else 0)
                near = sorted((abs(n - mapped), n, s) for n, s in added if abs(n - mapped) <= NEAR)
                removed.append({"old_line": r["old_start"] + i, "text": t.strip(),
                                "nearest_added": [{"new_line": n, "text": s}
                                                  for _, n, s in near[:3]]})
        removed.sort(key=lambda x: (x["old_line"], x["text"]))
        nrem += len(removed)
        files.append({"file": f, "base_count": sum(bc.values()), "new_count": sum(nc.values()),
                      "by_macro": {m: [bc.get(m, 0), nc.get(m, 0)] for m in sorted(set(bc) | set(nc))},
                      "removed": removed})
    return ({"files": files},
            f"files={len(files)} removed_assertions={nrem} excluded_skipped={nexc}")


def self_test():
    base = "CHECK(a);\nKT_CHECK_EQ(b, 1);\n#define CHECK(x) foo\n// CHECK(c);\nint y;\n"
    post = "CHECK(a);\nint y;\nEXPECT_EQ(2, 2);\n"
    recs = [make_record("m/x_test.cpp", old_start=2, new_start=2, minus=["KT_CHECK_EQ(b, 1);"],
                        plus=[]),
            make_record("m/x_test.cpp", old_start=4, new_start=3, minus=["int y;"],
                        plus=["EXPECT_EQ(2, 2);"]),
            make_record("m/helper.cpp", plus=["CHECK(1);"]),
            make_record("tools/parity/p.cpp", plus=["REQUIRE(1);"], status="A", diff_base=None,
                        old_path=None, kind="whole-file"),
            make_record("m/test_secret.py", plus=["assert 1"]),
            make_record("m/b_test.cpp", **{"class": "b"})]
    tmp, review, tree = make_env(recs, {"m/x_test.cpp": post, "tools/parity/p.cpp": "REQUIRE(1);\n"})
    global git_show
    orig = git_show
    git_show = lambda repo, ref, path: base.encode() if path == "m/x_test.cpp" else None
    try:
        out, summary = run(review, "/nonexistent", tree, "11e1284")
    finally:
        git_show = orig
        shutil.rmtree(tmp)
    byf = {f["file"]: f for f in out["files"]}
    assert sorted(byf) == ["m/x_test.cpp", "tools/parity/p.cpp"], sorted(byf)
    x = byf["m/x_test.cpp"]
    assert (x["base_count"], x["new_count"]) == (2, 2), x   # #define and comment line skipped
    assert x["by_macro"] == {"CHECK": [1, 1], "EXPECT_EQ": [0, 1], "KT_CHECK_EQ": [1, 0]}, x
    assert len(x["removed"]) == 1 and x["removed"][0]["old_line"] == 2, x["removed"]
    assert x["removed"][0]["nearest_added"] == [{"new_line": 3, "text": "EXPECT_EQ(2, 2);"}], x
    assert byf["tools/parity/p.cpp"]["new_count"] == 1 and byf["tools/parity/p.cpp"]["base_count"] == 0
    assert "excluded_skipped=1" in summary, summary
    assert is_excluded("a/B/.ENV") and is_excluded("x/Sunshine/Credentials/k") and not is_excluded("a/b.cpp")
    print("assertion_delta self-test OK (7 checks)")


if __name__ == "__main__":
    sys.exit(cli_main("assertion_delta", "assertion_delta.json", run, self_test))
