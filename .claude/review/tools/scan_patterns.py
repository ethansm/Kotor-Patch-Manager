#!/usr/bin/env python3
"""scan_patterns.py - deterministic pattern scanners that seed review packets with candidates.

Input : <review-dir>/work/hunks.jsonl (class "a" records only)
Output: <review-dir>/work/scanner_candidates.json (format: .claude/review/INTERFACES.md)
Usage : python3 -I tools/scan_patterns.py --review-dir D --repo R --tree T --base 11e1284
        python3 -I tools/scan_patterns.py --self-test
Scanners: sentinel-range, creature-lifecycle, pointer-math, hook-detour, export-no-definition.

SECURITY: never reads, git-shows, or opens a path matching is_excluded() (.env, *.pem,
.credentials.json, sunshine/credentials/*, endpoint.env, tests/static/raw_addresses.toml,
*credential*, *secret*; case-insensitive, any depth). Only the sanitized --tree is read.
"""
import hashlib
import json
import os
import posixpath
import re
import shutil
import sys
import tomllib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assertion_delta as ad  # noqa: E402

is_excluded = ad.is_excluded
C_EXT = (".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl", ".ipp")
SENTINEL = re.compile(r"(?<![\w.])0[xX][fF]{7}[0-9a-fA-F][uU]?[lL]{0,2}(?![\w])")
SENT_ID = re.compile(r"\b(isExile|kInvalid\w*|OBJECT_INVALID)\b")
LIFE = re.compile(r"\b(destroyCreature|allocCreature|createCreature|delete|free(?=\s*\())\b")
ALLOC = re.compile(r"\b(allocCreature|createCreature|new|malloc|calloc)\b")
DESTROY = re.compile(r"\b(destroyCreature|delete|free(?=\s*\())")
PTR = (("reinterpret_cast", re.compile(r"\breinterpret_cast\b")),
       ("c-pointer-cast", re.compile(r"\(\s*(?:(?:const|volatile|struct)\s+)*[A-Za-z_]\w*(?:::\w+)*\s*\*+\s*\)")),
       ("uintptr_t-cast", re.compile(r"\(\s*u?intptr_t\s*\)")),
       ("deref-cast-offset", re.compile(r"\*\s*\(\s*[A-Za-z_][\w\s:]*\*+\s*\)\s*\([^)]*\+\s*0[xX][0-9a-fA-F]+")),
       ("hex-offset", re.compile(r"\+\s*0[xX][0-9a-fA-F]+")))
HDR = re.compile(r"^\s*(\[\[?)\s*([^\]\s]+)\s*\]\]?\s*(#.*)?$")
EXIT_DEF = re.compile(r"^\s*([A-Za-z_][\w@.$?]*)\s*(?:=\s*([\w@.$?]+))?")
CTRL = re.compile(r"\b(return|if|while|for|switch|else|case|new|delete|sizeof|throw)\b")
NOISE = re.compile(r"sizeof\s*\([^)]*\)|=\s*delete\b")


def code_of(line):
    return NOISE.sub("", re.sub(r"//.*", "", line))


def cand(scanner, file, lines, hunk_id, detail):
    detail = detail[:300]
    cid = "S-" + hashlib.sha256("|".join((scanner, file, lines, detail)).encode()).hexdigest()[:10]
    return {"candidate_id": cid, "scanner": scanner, "file": file, "lines": lines,
            "hunk_id": hunk_id, "detail": detail}


def plus_lines(rec):
    return [(rec["new_start"] + i, t) for i, t in enumerate(rec["plus"])]


def text_of(tree, rel):
    d = ad.read_tree(tree, rel)
    return None if d is None else d.decode("utf-8", "replace")


def scan_lines(rec, out):
    """sentinel-range and pointer-math over added lines."""
    f, c_like = rec["file"], rec["file"].lower().endswith(C_EXT)
    for n, t in plus_lines(rec):
        hits = SENTINEL.findall(t) + SENT_ID.findall(t)
        if hits:
            out.append(cand("sentinel-range", f, f"{n}-{n}", rec["hunk_id"],
                            f"{', '.join(sorted(set(hits)))} | {t.strip()}"))
        if c_like:
            code = code_of(t)
            kinds = [k for k, rx in PTR if rx.search(code)]
            if kinds:
                out.append(cand("pointer-math", f, f"{n}-{n}", rec["hunk_id"],
                                f"{', '.join(kinds)} | {t.strip()}"))


def scan_lifecycle(f, recs, tree, out):
    if not f.lower().endswith(C_EXT):
        return
    text = text_of(tree, f)
    totals = "file totals n/a"
    if text is not None:
        code = [code_of(x) for x in text.splitlines()]
        totals = (f"file totals alloc={sum(len(ALLOC.findall(x)) for x in code)} "
                  f"destroy={sum(len(DESTROY.findall(x)) for x in code)}")
    for r in recs:
        for n, t in plus_lines(r):
            m = LIFE.findall(code_of(t))
            if m:
                out.append(cand("creature-lifecycle", f, f"{n}-{n}", r["hunk_id"],
                                f"{', '.join(sorted(set(m)))} | {t.strip()} | {totals}"))


def toml_blocks(lines, offset=0):
    """[[array-of-table]] blocks -> [(header, first, last, body_lines)], 1-based + offset."""
    heads = []
    for i, l in enumerate(lines):
        m = HDR.match(l)
        if m:
            heads.append((i, m.group(1) == "[[", m.group(2)))
    blocks = []
    for k, (i, arr, name) in enumerate(heads):
        if not arr:
            continue
        end = len(lines)
        for j, a2, n2 in heads[k + 1:]:
            if not n2.startswith(name + "."):
                end = j
                break
        while end > i + 1 and not lines[end - 1].strip():
            end -= 1
        body = [re.sub(r"^(\s*\[\[?)\s*" + re.escape(name) + r"\.", r"\1", l) for l in lines[i + 1:end]]
        blocks.append((name, i + 1 + offset, end + offset, body))
    return blocks


def block_fields(body):
    try:
        d = tomllib.loads("\n".join(body))
    except tomllib.TOMLDecodeError:
        d = {}
        for l in body:
            m = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*(.+?)\s*$", l)
            if m and m.group(1) not in d:
                d[m.group(1)] = m.group(2)
    parts = []
    for k in ("name", "address", "original_bytes"):
        parts.append(f"{k}={json.dumps(d.get(k, '<absent>'), sort_keys=True)}")
    exits = sorted(k for k in d if "exit" in k.lower())
    parts += [f"{k}={json.dumps(d[k], sort_keys=True)}" for k in exits] or ["exits=<absent>"]
    for k in ("params", "exclude_from_restore"):
        parts.append(f"{k}={json.dumps(d.get(k, '<absent>'), sort_keys=True)}")
    return "; ".join(parts)


def scan_hooks(f, recs, tree, out):
    changed = {n for r in recs for n, _ in plus_lines(r)}
    text = text_of(tree, f)
    if text is not None:
        lines = text.split("\n")
        units = [(toml_blocks(lines), lines, 0)]
    else:  # post-image unavailable: fall back to the added fragments
        units = [(toml_blocks(r["plus"], r["new_start"] - 1), r["plus"], r["new_start"] - 1) for r in recs]
    seen = set()
    for blocks, lines, off in units:
        for name, a, b, body in blocks:
            if (a, b) in seen or not any(a <= n <= b for n in changed):
                continue
            seen.add((a, b))
            hid = next((r["hunk_id"] for r in recs
                        if any(a <= n <= b for n, _ in plus_lines(r))), None)
            out.append(cand("hook-detour", f, f"{a}-{b}", hid, f"[[{name}]] {block_fields(body)}"))
        inblock = {n for _, a, b, _ in blocks for n in range(a, b + 1)}
        for r in recs:
            for n, t in plus_lines(r):
                if text is None and n - off - 1 >= len(lines):
                    continue
                if n not in inblock and t.strip() and not t.strip().startswith("#"):
                    out.append(cand("hook-detour", f, f"{n}-{n}", r["hunk_id"], f"non-block line | {t.strip()}"))


def scan_exports(f, recs, tree, out):
    text = text_of(tree, f)
    if text is None:
        return
    moddir = posixpath.dirname(f)
    root = os.path.join(tree, moddir)
    sources = []
    for dp, dns, fns in os.walk(root):
        dns[:] = sorted(d for d in dns if not os.path.islink(os.path.join(dp, d)))
        for fn in sorted(fns):
            rel = posixpath.relpath(os.path.join(dp, fn), tree).replace(os.sep, "/")
            if fn.lower().endswith((".cpp", ".c", ".h")) and not is_excluded(rel):
                sources.append(text_of(tree, rel) or "")
    for n, line in enumerate(text.split("\n"), 1):
        s = line.split(";")[0].strip()
        if not s or s.split()[0].upper() in ("EXPORTS", "LIBRARY", "NAME", "DESCRIPTION", "VERSION", "SECTIONS"):
            continue
        m = EXIT_DEF.match(s)
        if not m:
            continue
        name, internal = m.group(1).split("@")[0], (m.group(2) or "").split("@")[0]
        sym = internal or name
        if not any(defines(src, sym) for src in sources):
            hid = next((r["hunk_id"] for r in recs if any(x == n for x, _ in plus_lines(r))),
                       recs[0]["hunk_id"])
            out.append(cand("export-no-definition", f, f"{n}-{n}", hid,
                            f"symbol {name}" + (f" (= {internal})" if internal else "") +
                            f" has no definition in {len(sources)} source files under {moddir or '.'}"))


def defines(src, sym):
    rx = re.compile(r"^[ \t]*([^\n;=]*?[\s*&:])?" + re.escape(sym) + r"[ \t]*\(", re.M)
    for m in rx.finditer(src):
        if m.group(1) is None or CTRL.search(m.group(1)):
            continue
        tail = re.search(r"[{;]", src[m.end():])
        if tail and tail.group(0) == "{":
            return True
    return False


def run(review_dir, repo, tree, base):
    recs, nexc = ad.filter_records(ad.load_hunks(review_dir))
    out, byfile = [], {}
    for r in recs:
        if r["kind"] in ("diff", "whole-file"):
            byfile.setdefault(r["file"], []).append(r)
            scan_lines(r, out)
    for f, rs in byfile.items():
        scan_lifecycle(f, rs, tree, out)
        if f.endswith(".hooks.toml"):
            scan_hooks(f, rs, tree, out)
        if posixpath.basename(f).lower() == "exports.def":
            scan_exports(f, rs, tree, out)
    uniq = {c["candidate_id"]: c for c in out}
    cands = sorted(uniq.values(), key=lambda c: (c["file"], int(c["lines"].split("-")[0]),
                                                 c["scanner"], c["candidate_id"]))
    counts = {}
    for c in cands:
        counts[c["scanner"]] = counts.get(c["scanner"], 0) + 1
    return ({"candidates": cands},
            f"candidates={len(cands)} " + " ".join(f"{k}={v}" for k, v in sorted(counts.items())) +
            f" excluded_skipped={nexc}")


def self_test():
    mk = ad.make_record
    code = mk("m/a.cpp", new_start=10, plus=[
        "if (x == 0xFFFFFFF0u) {}", "y = 0xfffffffe;", "z = 0xFFFFFF0;", "w = 0xFFFFFFFFFF;",
        "if (isExile) {}", "int q = sizeof(int*);", "*(int*)(p + 0x1c) = 3;", "auto s = reinterpret_cast<char*>(p);",
        "destroyCreature(c);", "allocCreature(c);", "Foo(const Foo&) = delete;", "// delete the thing"])
    hooks = mk("m/x.hooks.toml", new_start=8, plus=['name = "b"', 'address = "0x401000"'])
    exp = mk("m/exports.def", new_start=1, plus=["EXPORTS"])
    env = mk(".env", plus=["0xFFFFFFF0"])
    toml_txt = ('target = "x"\n\n[[hooks]]\nname = "a"\naddress = "0x1"\n\n[[hooks]]\n'
                'name = "b"\naddress = "0x401000"\noriginal_bytes = "90 90"\nexit_addresses = ["0x401010"]\n')
    files = {"m/a.cpp": "x\ny\ndestroyCreature(c);\nallocCreature(c);\n", "m/x.hooks.toml": toml_txt,
             "m/exports.def": "EXPORTS\n  KpInit @1\n  KpMissing\n  Internal=Impl\n; comment\n",
             "m/impl.cpp": 'extern "C" __declspec(dllexport) int __stdcall KpInit(void* p)\n{\n  return KpMissing(1);\n}\n'
                           "void Impl() {}\nint KpOther(int);\n"}
    tmp, review, tree = ad.make_env([code, hooks, exp, env], files)
    try:
        out, summary = run(review, "/nonexistent", tree, "11e1284")
    finally:
        shutil.rmtree(tmp)
    cs = out["candidates"]

    def sel(s):
        return [c for c in cs if c["scanner"] == s]
    sent = {c["lines"] for c in sel("sentinel-range")}
    assert sent == {"10-10", "11-11", "14-14"}, sent        # 0xFFFFFFF0u, 0xfffffffe, isExile only
    ptr = {c["lines"] for c in sel("pointer-math")}
    assert ptr == {"16-16", "17-17"}, ptr                    # sizeof(int*) is not a hit
    life = sel("creature-lifecycle")
    assert {c["lines"] for c in life} == {"18-18", "19-19"} and "alloc=1 destroy=1" in life[0]["detail"], life
    hk = sel("hook-detour")
    assert len(hk) == 1 and hk[0]["lines"] == "7-11" and "0x401000" in hk[0]["detail"], hk
    ex = [c["detail"] for c in sel("export-no-definition")]
    assert len(ex) == 1 and "KpMissing" in ex[0] and "KpInit" not in ex[0], ex   # call != definition
    assert not any(c["file"] == ".env" for c in cs) and "excluded_skipped=1" in summary
    assert all(c["candidate_id"].startswith("S-") and len(c["candidate_id"]) == 12 for c in cs)
    print("scan_patterns self-test OK (8 checks)")


if __name__ == "__main__":
    sys.exit(ad.cli_main("scan_patterns", "scanner_candidates.json", run, self_test))
