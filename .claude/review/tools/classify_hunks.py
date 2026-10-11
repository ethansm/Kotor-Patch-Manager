#!/usr/bin/env python3
"""classify_hunks.py - deterministic "mechanical change" classifier for class-a hunks.

Input : <review-dir>/work/hunks.jsonl
Output: <review-dir>/work/classifier_verdicts.json (format: .claude/review/INTERFACES.md)
Usage : python3 -I tools/classify_hunks.py --review-dir D --repo R --tree T --base 11e1284
        python3 -I tools/classify_hunks.py --self-test
Rules : whitespace, comment-only, include-style, pure-rename, symlink-in-tree. One verdict per
        applicable rule attempt (failures recorded with the reason); a hunk is mechanical when any
        of its verdicts has pass=true.

SECURITY: never reads, git-shows, or opens a path matching is_excluded() (.env, *.pem,
.credentials.json, sunshine/credentials/*, endpoint.env, tests/static/raw_addresses.toml,
*credential*, *secret*; case-insensitive, any depth). Post-image content comes only from the
sanitized --tree; base content only via read-only `git show` for paths named in hunks.jsonl.
"""
import hashlib
import os
import posixpath
import re
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assertion_delta as ad  # noqa: E402

is_excluded = ad.is_excluded
C_EXT = (".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl", ".ipp")
HASH_EXT = (".py", ".toml", ".sh", ".bash")
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')
WS = re.compile(r"\s+")


def lang_of(path):
    p = path.lower()
    return "c" if p.endswith(C_EXT) else "hash" if p.endswith(HASH_EXT) else None


def comment_mask(text, lang):
    """1-based line numbers that are blank or contain only comment text."""
    res, in_block = set(), False
    for i, line in enumerate(text.split("\n"), 1):
        if lang == "hash":
            if not line.strip() or line.strip().startswith("#"):
                res.add(i)
            continue
        code, j, quote = False, 0, None
        while j < len(line):
            c = line[j]
            if in_block:
                if line.startswith("*/", j):
                    in_block, j = False, j + 2
                else:
                    j += 1
            elif quote:
                code = True
                j += 2 if c == "\\" else 1
                if c == quote:
                    quote = None
            elif line.startswith("//", j):
                break
            elif line.startswith("/*", j):
                in_block, j = True, j + 2
            else:
                if c in "\"'":
                    quote = c
                code = code or not c.isspace()
                j += 1
        if not code:
            res.add(i)
    return res


def heuristic_comment(line, lang):
    """Conservative per-line test used only when file text does not line up with the hunk."""
    s = line.strip()
    if not s:
        return True
    if lang == "hash":
        return s.startswith("#")
    return (s.startswith("//") or s in ("*", "*/") or
            (s.startswith("/*") and s.endswith("*/") and s.count("/*") == 1))


class Ctx:
    def __init__(self, repo, tree, recs):
        self.repo, self.tree, self.recs = repo, tree, recs
        self._base, self._post = {}, {}

    def base(self, rec):
        k = rec.get("old_path") or rec["file"]
        if k not in self._base:
            self._base[k] = ad.base_text(self.repo, rec)
        return self._base[k]

    def post(self, rec):
        f = rec["file"]
        if f not in self._post:
            d = ad.read_tree(self.tree, f)
            self._post[f] = None if d is None else d.decode("utf-8", "replace")
        return self._post[f]


def rng(r):
    return f"-{r['old_start']},{r['old_len']} +{r['new_start']},{r['new_len']}"


def rule_whitespace(r, ctx):
    if r["kind"] not in ("diff", "whole-file") or not (r["minus"] or r["plus"]):
        return None
    a, b = WS.sub("", "".join(r["minus"])), WS.sub("", "".join(r["plus"]))
    proof = {"minus_lines": len(r["minus"]), "plus_lines": len(r["plus"]),
             "tokens_equal": [x.split() for x in r["minus"]] == [x.split() for x in r["plus"]]}
    return a == b, proof


def rule_comment_only(r, ctx):
    lang = lang_of(r["file"])
    if lang is None or r["kind"] not in ("diff", "whole-file") or not (r["minus"] or r["plus"]):
        return None
    bad, methods = [], set()
    for side, lines, start, text in (("-", r["minus"], r["old_start"], ctx.base(r) if r["minus"] else ""),
                                     ("+", r["plus"], r["new_start"], ctx.post(r) if r["plus"] else "")):
        tl = text.split("\n") if text else []
        mask = comment_mask(text, lang) if text else set()
        for k, line in enumerate(lines):
            n = start + k
            if n - 1 < len(tl) and tl[n - 1].rstrip("\r") == line.rstrip("\r"):
                methods.add("mask")
                ok = n in mask
            else:
                methods.add("heuristic")
                ok = heuristic_comment(line, lang)
            if not ok and len(bad) < 5:
                bad.append(f"{side}{n}: {line.strip()[:80]}")
            elif not ok:
                bad.append("...")
                break
    return not bad, {"methods": sorted(methods), "non_comment_lines": bad}


def rule_include_style(r, ctx):
    f = r["file"]
    if lang_of(f) != "c" or r["kind"] not in ("diff", "whole-file"):
        return None
    lines = r["minus"] + r["plus"]
    if not any(INCLUDE.match(x) for x in lines):
        return None
    proof = {}
    ok = all(INCLUDE.match(x) for x in lines)
    if not ok:
        proof["reason"] = "non-include line changed"
    mb = sorted(posixpath.basename(INCLUDE.match(x).group(1)) for x in r["minus"] if INCLUDE.match(x))
    mp = sorted(posixpath.basename(INCLUDE.match(x).group(1)) for x in r["plus"] if INCLUDE.match(x))
    proof["minus_basenames"], proof["plus_basenames"] = mb, mp
    if mb != mp:
        ok = False
        proof["reason"] = "included basenames differ"
    if ad.is_test_file(f) or "tools/parity/" in "/" + f:
        bc = sum(ad.count_assertions(ctx.base(r), f).values())
        nc = sum(ad.count_assertions(ctx.post(r) or "", f).values())
        proof["base_assertions"], proof["new_assertions"] = bc, nc
        if bc != nc:
            ok = False
            proof["reason"] = "test-file assertion count changed"
    return ok, proof


def rule_pure_rename(r, ctx):
    if r["kind"] != "rename":
        return None
    f, old = r["file"], r.get("old_path")
    proof = {"old_path": old}
    other = [x for x in ctx.recs if x["file"] == f and x["hunk_id"] != r["hunk_id"]
             and x["kind"] in ("diff", "whole-file")]
    if r["minus"] or r["plus"] or other:
        proof["reason"] = "content hunks present"
        return False, proof
    b = ad.git_show(ctx.repo, r.get("diff_base"), old) if r.get("diff_base") else None
    p = ad.read_tree(ctx.tree, f)
    if b is None or p is None:
        proof["reason"] = "base or post-image unreadable"
        return False, proof
    proof["sha256"] = hashlib.sha256(p).hexdigest()
    proof["identical"] = b == p
    if b != p:
        proof["reason"] = "content differs (similarity < 100%)"
    return b == p, proof


def rule_symlink(r, ctx):
    if r["kind"] != "symlink":
        return None
    t = r.get("symlink_target") or ""
    proof = {"symlink_target": t}
    res = posixpath.normpath(posixpath.join(posixpath.dirname(r["file"]), t)) if t else ""
    proof["resolved"] = res
    if not t or t.startswith("/") or res.startswith("..") or res == ".":
        proof["reason"] = "target empty, absolute or escapes repo"
        return False, proof
    if is_excluded(res):
        proof["reason"] = "target is an excluded path"
        return False, proof
    data = ad.read_tree(ctx.tree, res)
    full = os.path.join(ctx.tree, res)
    if data is None or os.path.islink(full):
        proof["reason"] = "target is not a regular file in worker tree"
        return False, proof
    proof["sha256"] = hashlib.sha256(data).hexdigest()
    return True, proof


RULES = (("whitespace", rule_whitespace), ("comment-only", rule_comment_only),
         ("include-style", rule_include_style), ("pure-rename", rule_pure_rename),
         ("symlink-in-tree", rule_symlink))


def run(review_dir, repo, tree, base):
    recs, nexc = ad.filter_records(ad.load_hunks(review_dir))
    ctx = Ctx(repo, tree, recs)
    verdicts, mech = [], set()
    for r in recs:
        for name, fn in RULES:
            res = fn(r, ctx)
            if res is None:
                continue
            verdicts.append({"hunk_id": r["hunk_id"], "file": r["file"], "range": rng(r),
                             "rule": name, "pass": bool(res[0]), "proof": res[1]})
            if res[0]:
                mech.add(r["hunk_id"])
    return ({"generated_by": "tools/classify_hunks.py", "base": base, "verdicts": verdicts},
            f"records={len(recs)} verdicts={len(verdicts)} mechanical_hunks={len(mech)} "
            f"excluded_skipped={nexc}")


def self_test():
    mk = ad.make_record
    ws = mk("a/w.cpp", minus=["int  x = 1;"], plus=["int x = 1;"])
    wsbad = mk("a/w.cpp", old_start=5, new_start=5, minus=["int x = 1;"], plus=["int x = 2;"])
    cm = mk("a/c.cpp", old_start=3, new_start=3, minus=[" * old text"], plus=[" * new text"])
    cmbad = mk("a/c.cpp", old_start=6, new_start=6, minus=["*p = 1;"], plus=["*p = 2;"])
    cmpy = mk("a/s.py", minus=["# a"], plus=["# b"])
    inc = mk("a/i.cpp", minus=['#include "old/b.h"'], plus=['#include "new/b.h"'])
    incadd = mk("a/i.cpp", old_start=9, new_start=9, minus=[], plus=['#include "x.h"'])
    tinc = mk("a/m_test.cpp", minus=['#include "old/b.h"'], plus=['#include "new/b.h"'])
    tinc2 = mk("a/n_test.cpp", minus=['#include "old/b.h"'], plus=['#include "new/b.h"'])
    ren = mk("a/new.cpp", old_path="a/old.cpp", kind="rename", status="R")
    ren2 = mk("a/new2.cpp", old_path="a/old2.cpp", kind="rename", status="R")
    sl = mk("d/ok.h", kind="symlink", symlink_target="../real.h", status="T")
    sl_esc = mk("d/esc.h", kind="symlink", symlink_target="../../x.h", status="T")
    sl_sec = mk("d/sec.h", kind="symlink", symlink_target="../my_secret.h", status="T")
    sl_dir = mk("d/dir.h", kind="symlink", symlink_target="../adir", status="T")
    envrec = mk(".env", minus=["A"], plus=["B"])
    clsb = mk("docs/x.md", **{"class": "b"}, minus=[" a"], plus=["a"])
    files = {"a/c.cpp": "int a;\n/* first\n * new text\n */\nint b;\n*p = 2;\n",
             "a/w.cpp": "int x = 1;\n", "a/i.cpp": "#include \"new/b.h\"\n",
             "a/s.py": "# b\n", "a/m_test.cpp": "CHECK(1);\n", "a/n_test.cpp": "CHECK(1);\nCHECK(2);\n",
             "a/new.cpp": "same\n", "a/new2.cpp": "changed\n", "real.h": "int r;\n",
             "my_secret.h": "x", "adir/f": "x"}
    bases = {"a/c.cpp": "int a;\n/* first\n * old text\n */\nint b;\n*p = 1;\n",
             "a/m_test.cpp": "CHECK(1);\nCHECK(2);\n", "a/n_test.cpp": "CHECK(1);\nCHECK(2);\n",
             "a/old.cpp": "same\n", "a/old2.cpp": "original\n"}
    tmp, review, tree = ad.make_env([ws, wsbad, cm, cmbad, cmpy, inc, incadd, tinc, tinc2, ren, ren2,
                                     sl, sl_esc, sl_sec, sl_dir, envrec, clsb], files)
    orig = ad.git_show
    ad.git_show = lambda repo, ref, path: bases[path].encode() if path in bases else None
    try:
        out, summary = run(review, "/nonexistent", tree, "11e1284")
    finally:
        ad.git_show = orig
        shutil.rmtree(tmp)
    got = {(v["hunk_id"], v["rule"]): v for v in out["verdicts"]}

    def p(rec, rule):
        return got[(rec["hunk_id"], rule)]["pass"]
    assert p(ws, "whitespace") and not p(wsbad, "whitespace")
    assert p(cm, "comment-only") and not p(cmbad, "comment-only")
    assert p(cmpy, "comment-only")
    assert p(inc, "include-style") and not p(incadd, "include-style")
    assert not p(tinc, "include-style"), "test-file assertion count changed in tree (1 vs 2)"
    assert got[(tinc["hunk_id"], "include-style")]["proof"]["base_assertions"] == 2
    assert p(tinc2, "include-style"), "equal assertion counts must pass"
    assert p(ren, "pure-rename") and not p(ren2, "pure-rename")
    assert p(sl, "symlink-in-tree") and len(got[(sl["hunk_id"], "symlink-in-tree")]["proof"]["sha256"]) == 64
    assert not p(sl_esc, "symlink-in-tree") and not p(sl_sec, "symlink-in-tree")
    assert not p(sl_dir, "symlink-in-tree")
    assert not any(v["file"] in (".env", "docs/x.md") for v in out["verdicts"])
    assert "excluded_skipped=1" in summary, summary
    print("classify_hunks self-test OK (13 checks)")


if __name__ == "__main__":
    sys.exit(ad.cli_main("classify_hunks", "classifier_verdicts.json", run, self_test))
