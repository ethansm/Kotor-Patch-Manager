#!/usr/bin/env python3
"""Mutation gate for ONE C++ header (doc 20 section 3 "Mutation gate"; reference design: research/companion_levelup/
tools/parity/mutate.py).

  python3 scripts/mutate_header.py patch_manager_mods/_shared_src/SafeRead.h --jobs 4
  python3 scripts/mutate_header.py <header> --list              # the mutants, run nothing
  python3 scripts/mutate_header.py <header> --only 7            # one mutant
  python3 scripts/mutate_header.py <header> --test a_test.cpp   # extra / explicit tests (fixtures live outside discovery)
  also: run_tests.py --mutate <header> [--jobs N] [--json PATH]

Operators (light tokenizer, no libclang; comments, string/char literals, preprocessor lines and static_assert are skipped):
  < <= > >= == !=  flipped (a bare < or > only when spaced on both sides, so templates are left alone)
  && <-> ||        (spaced on both sides, so T&& is left alone)
  binary + <-> -   (spaced; ++ -- += -= untouched)
  integer constants +1 / -1, decimal and hex (floats, constants >= 0x400000 (engine addresses), 0 -> -1 are skipped)
  `if (cond) return X;` on one line -> `if (cond) {}`
  true <-> false

Tests per mutant: every *_test.cpp under patch_manager_mods/ whose include closure (quote includes, resolved next to
the including file, next to the test, and via the -I flags of its test.toml) reaches the header, plus any --test. They are
built and run exactly as run_tests.py does (it is imported: same flags, test.toml, sanitizers, classification).

Overlay approach: relative includes ("../_shared_src/X.h") and -I../x flags must still resolve and the mutated header must
SHADOW the real one even for tests in its own directory (quote includes look next to the includer first, so a plain -I overlay
cannot win). So each mutant gets a scratch tree $TMPDIR/kotor-mutate-h/<n>/ that mirrors the repo layout: the header's
directory and each test's directory are shallow copies (large files and subdirectories such as testdata/ are symlinks), every
other entry on the way down from the repo root is a symlink to the real one. ONE mutation is applied to the scratch copy of the
header. Real sources are never written.

Result per mutant: KILLED (a test fails/crashes), TIMEOUT (a test timed out; counts as killed), SURVIVED (all pass),
BROKEN (the mutant, or a test with it, does not compile; reported separately, not counted as killed).
The unmutated scratch tree runs first; if that is not green the tool exits 2 (a red baseline kills everything).
Exit: 1 if any survivor, 2 on usage/baseline errors, else 0.
"""
import argparse
import concurrent.futures as cf
import json
import os
import re
import shutil
import threading
import sys
import time
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

REPO = rt.REPO
MUT_ROOT = Path(os.environ.get("TMPDIR") or f"/tmp/claude-{os.getuid()}") / "kotor-mutate-h"
ENGINE_ADDR = 0x400000
BIG_FILE = 256 * 1024


# ---------------------------------------------------------------- tokenizer / mutation finder

@dataclass
class Mutation:
    start: int
    end: int
    before: str
    after: str
    line: int
    id: int = 0


def mask(text: str) -> str:
    """Same-length copy of text with comments, string/char literal contents, preprocessor lines and static_assert(...)
    statements blanked (newlines kept), so regexes over it only see real code."""
    out = list(text)
    n = len(text)

    def blank(a, b):
        for k in range(a, b):
            if out[k] != "\n":
                out[k] = " "
    i = 0
    line_start = True
    while i < n:
        c = text[i]
        if c == "\n":
            line_start = True
            i += 1
            continue
        if c in " \t":
            i += 1
            continue
        if line_start and c == "#":           # preprocessor line incl. backslash continuations
            j = i
            while j < n:
                if text[j] == "\n" and text[j - 1] != "\\":
                    break
                j += 1
            # keep a trailing // comment from swallowing nothing: just blank the whole line
            blank(i, j)
            i = j
            continue
        line_start = False
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            blank(i, j)
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            blank(i, j)
            i = j
        elif c == '"' or c == "'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            blank(i + 1, min(j, n))
            i = j + 1
        else:
            i += 1
    m = "".join(out)
    # static_assert( ... ); (parens balanced)
    for sm in list(re.finditer(r"\bstatic_assert\s*\(", m)):
        depth, j = 1, sm.end()
        while j < n and depth:
            depth += (m[j] == "(") - (m[j] == ")")
            j += 1
        k = m.find(";", j)
        k = n if k < 0 else k + 1
        for q in range(sm.start(), k):
            if m[q] != "\n":
                out[q] = " "
        m = "".join(out)
    return m


_TOK = re.compile(r"0[xX][0-9a-fA-F']+[uUlL]*|\d[\d']*(?:\.\d*)?(?:[eE][+-]?\d+)?[fFuUlL]*|\.\d+|[A-Za-z_]\w*"
                  r"|<=>|<<=|>>=|<=|>=|==|!=|&&|\|\||<<|>>|->|\+\+|--|\+=|-=|[<>+\-]")
_FLIP = {"<": "<=", "<=": "<", ">": ">=", ">=": ">", "==": "!=", "!=": "==", "&&": "||", "||": "&&", "+": "-", "-": "+",
         "true": "false", "false": "true"}


def _spaced(m: str, a: int, b: int) -> bool:
    return a > 0 and b < len(m) and m[a - 1] in " \t" and m[b] in " \t"


def _number_mutants(tok: str):
    low = tok.lower()
    if low.startswith("0x"):
        digits = tok[2:].rstrip("uUlL")
        suffix = tok[2 + len(digits):]
        if "'" in digits:
            return []
        v = int(digits, 16)
        if v >= ENGINE_ADDR:
            return []
        upper = any(ch.isupper() for ch in digits)

        def fmt(x):
            h = f"{x:0{len(digits)}X}" if upper or not any(ch.islower() for ch in digits) else f"{x:0{len(digits)}x}"
            return tok[:2] + h + suffix
        res = [fmt(v + 1)]
        if v > 0:
            res.append(fmt(v - 1))
        return res
    body = tok.rstrip("uUlL")
    suffix = tok[len(body):]
    if not body.isdigit():
        return []                                  # float, digit separator, ...
    if len(body) > 1 and body[0] == "0":
        return []                                  # octal
    v = int(body)
    if v >= ENGINE_ADDR:
        return []
    res = [f"{v + 1}{suffix}"]
    if v > 0:
        res.append(f"{v - 1}{suffix}")
    return res


def find_mutations(text: str) -> list:
    m = mask(text)
    muts = []

    def add(a, b, after):
        muts.append(Mutation(a, b, text[a:b], after, text.count("\n", 0, a) + 1))

    prev_ident = ""
    for t in _TOK.finditer(m):
        tok, a, b = t.group(0), t.start(), t.end()
        if tok[0].isdigit() or (tok[0] == "." and len(tok) > 1):
            if not (a > 0 and m[a - 1] in ".") and tok[0] != ".":
                for after in _number_mutants(tok):
                    add(a, b, after)
        elif tok in ("true", "false"):
            add(a, b, _FLIP[tok])
        elif tok in ("==", "!=", "<=", ">="):
            if prev_ident != "operator":
                add(a, b, _FLIP[tok])
        elif tok in ("<", ">", "&&", "||", "+", "-"):
            if _spaced(m, a, b):
                if tok in ("+", "-"):
                    j = a - 1
                    while j >= 0 and m[j] in " \t":
                        j -= 1
                    if j < 0 or not (m[j].isalnum() or m[j] in "_)]"):
                        continue
                if prev_ident != "operator":
                    add(a, b, _FLIP[tok])
        prev_ident = tok if (tok[0].isalpha() or tok[0] == "_") else ""

    # `if (cond) return X;` on one line
    for im in re.finditer(r"\bif\s*\(", m):
        depth, j = 1, im.end()
        while j < len(m) and depth:
            depth += (m[j] == "(") - (m[j] == ")")
            j += 1
        rm = re.compile(r"[ \t]*(return\b[^;{}\n]*;)").match(m, j)
        if depth == 0 and rm:
            add(rm.start(1), rm.end(1), "{}")
    muts.sort(key=lambda x: x.start)   # stable: +1 before -1
    for i, x in enumerate(muts, 1):
        x.id = i
    return muts


def apply_mutation(text: str, mu: Mutation) -> str:
    assert text[mu.start:mu.end] == mu.before
    return text[:mu.start] + mu.after + text[mu.end:]


# ---------------------------------------------------------------- tests per header

_INC = re.compile(r'^[ \t]*#[ \t]*include[ \t]*"([^"]+)"', re.M)


def toml_include_dirs(src: Path) -> list:
    raw = rt.load_toml(src.parent / "test.toml")
    sub = raw.get(src.stem) if isinstance(raw.get(src.stem), dict) else {}
    cfg = {k: v for k, v in raw.items() if not isinstance(v, dict)}
    cfg.update(sub)
    flags = [rt.expand(f) for f in cfg.get("cxxflags", [])]
    dirs, i = [], 0
    while i < len(flags):
        f = flags[i]
        if f == "-I" and i + 1 < len(flags):
            dirs.append(src.parent / flags[i + 1])
            i += 1
        elif f.startswith("-I") and len(f) > 2:
            dirs.append(src.parent / f[2:])
        i += 1
    return dirs


def include_closure(src: Path, inc_dirs=()) -> set:
    """Resolved paths of every repo file src reaches through "quoted" includes."""
    src = Path(src).resolve()
    seen, todo = set(), [src]
    dirs = [src.parent] + [Path(d) for d in inc_dirs]
    while todo:
        f = todo.pop()
        if f in seen:
            continue
        seen.add(f)
        try:
            body = f.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for inc in _INC.findall(body):
            for base in [f.parent] + dirs:
                cand = (base / inc)
                if cand.is_file():
                    todo.append(cand.resolve())
                    break
    return seen


def discover_tests(header: Path) -> list:
    header = header.resolve()
    found = []
    for dp, fns in rt.walk(rt.CPP_ROOT):
        for fn in fns:
            if fn.endswith("_test.cpp"):
                src = dp / fn
                if header in include_closure(src, toml_include_dirs(src)):
                    found.append(src.resolve())
    return found


# ---------------------------------------------------------------- scratch tree

def _copy_dir_shallow(src: Path, dst: Path):
    dst.mkdir(parents=True, exist_ok=True)
    for e in src.iterdir():
        t = dst / e.name
        if e.is_dir() or e.is_symlink() or (e.is_file() and e.stat().st_size > BIG_FILE):
            os.symlink(e, t)
        else:
            shutil.copy2(e, t)


def build_scratch(root: Path, copy_dirs) -> None:
    """Mirror the repo under root: copy_dirs (absolute) are shallow copies; everything else on the way down is a symlink."""
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    rels = sorted({Path(d).resolve().relative_to(REPO) for d in copy_dirs})
    relset = set(rels)

    def populate(real: Path, scratch: Path, rel: Path):
        keep = {r.parts[len(rel.parts)] for r in rels if r.parts[:len(rel.parts)] == rel.parts and len(r.parts) > len(rel.parts)}
        scratch.mkdir(parents=True, exist_ok=True)
        for e in real.iterdir():
            if e.name in keep:
                continue
            os.symlink(e, scratch / e.name)
        for name in keep:
            sub = rel / name
            if sub in relset:
                _copy_dir_shallow(real / name, scratch / name)
            else:
                populate(real / name, scratch / name, sub)
    populate(REPO, root, Path("."))


# ---------------------------------------------------------------- running

def run_tests_in(root: Path, tests, header_rel: Path, text: str | None, tag: str, sanitize: bool, timeout: int):
    """Write `text` over the scratch header (when given) and build+run every test there. Returns (status, detail)."""
    if text is not None:
        (root / header_rel).write_text(text, encoding="utf-8")
    for ti, t in enumerate(tests):
        st = root / t.relative_to(REPO)
        it = rt.cpp_item(st, sanitize, tag=f"{tag}_{ti}")
        it.timeout, it.build_timeout = timeout, 600
        r = rt.execute(it)
        if r.status == "PASS":
            continue
        if r.status == "SKIP":
            continue
        if r.reason.startswith("build failed"):
            return "BROKEN", f"{t.name}: {r.reason}\n{rt.tail(r.output, 12)}"
        if "timeout" in r.reason:
            return "TIMEOUT", f"{t.name}: {r.reason}"
        return "KILLED", f"{t.name}: {r.reason}"
    return "SURVIVED", ""


def rel_to_repo(p: Path) -> str:
    return rt.rel(p)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Mutation gate for one C++ header.",
                                 epilog="See the module docstring for operators and the overlay design.")
    ap.add_argument("header")
    ap.add_argument("--test", action="append", default=[], metavar="CPP", help="extra/explicit test source (repeatable)")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--list", action="store_true", help="list the mutants and exit")
    ap.add_argument("--only", type=int, metavar="K", help="run only mutant K")
    ap.add_argument("--timeout", type=int, default=60, metavar="S", help="per test run, seconds (default 60)")
    ap.add_argument("--json", metavar="PATH")
    ap.add_argument("--no-sanitize", action="store_true", help="build tests without ASan/UBSan (faster)")
    ap.add_argument("--keep", action="store_true", help="keep the scratch trees")
    ap.add_argument("-v", action="store_true", help="print every mutant's result")
    ap.add_argument("--progress", type=int, default=25, metavar="N",
                    help="flushed 'progress i/n killed= survived=' line every N mutants (0 = off)")
    a = ap.parse_args(argv)

    header = Path(a.header).resolve()
    if not header.is_file():
        print(f"mutate_header: no such header {a.header}", file=sys.stderr)
        return 2
    try:
        header.relative_to(REPO)
    except ValueError:
        print(f"mutate_header: {header} is outside the repo {REPO}", file=sys.stderr)
        return 2
    hrel = header.relative_to(REPO)
    text = header.read_text(encoding="utf-8")
    muts = find_mutations(text)
    if a.only is not None:
        muts = [m for m in muts if m.id == a.only]
        if not muts:
            print(f"mutate_header: no mutant {a.only}", file=sys.stderr)
            return 2
    if a.list:
        for m in muts:
            print(f"{m.id} {hrel}:{m.line} {m.before} -> {m.after}")
        return 0

    tests = discover_tests(header)
    for t in a.test:
        p = Path(t).resolve()
        if not p.is_file():
            print(f"mutate_header: no such test {t}", file=sys.stderr)
            return 2
        if p not in tests:
            tests.append(p)
    if not tests:
        print(f"mutate_header: no test includes {hrel}; pass --test", file=sys.stderr)
        return 2
    try:
        for t in tests:
            t.relative_to(REPO)
    except ValueError:
        print("mutate_header: tests must live inside the repo", file=sys.stderr)
        return 2
    copy_dirs = {header.parent} | {t.parent for t in tests}
    sanitize = not a.no_sanitize
    MUT_ROOT.mkdir(parents=True, exist_ok=True)
    t0 = time.monotonic()

    base_root = MUT_ROOT / "base"
    build_scratch(base_root, copy_dirs)
    st, detail = run_tests_in(base_root, tests, hrel, None, "_hb", sanitize, a.timeout)
    shutil.rmtree(base_root, ignore_errors=True)
    if st != "SURVIVED":
        print(f"mutate_header: baseline (unmutated) is not green: {st} {detail}", file=sys.stderr)
        return 2

    done = {"n": 0, "KILLED": 0, "SURVIVED": 0}
    done_lock = threading.Lock()

    def work(m: Mutation):
        root = MUT_ROOT / str(m.id)
        try:
            build_scratch(root, copy_dirs)
            s, d = run_tests_in(root, tests, hrel, apply_mutation(text, m), f"_h{m.id}", sanitize, a.timeout)
        except OSError as e:
            s, d = "BROKEN", f"scratch error: {e}"
        if not a.keep:
            shutil.rmtree(root, ignore_errors=True)
        if a.v:
            print(f"{s:<8} {m.id} {hrel}:{m.line} {m.before} -> {m.after}", flush=True)
        with done_lock:   # so `tail -1` on a tee'd log shows a live run is moving
            done["n"] += 1
            done[s] = done.get(s, 0) + 1
            if a.progress > 0 and (done["n"] % a.progress == 0 or done["n"] == len(muts)):
                print(f"progress {done['n']}/{len(muts)} killed={done['KILLED']} survived={done['SURVIVED']}", flush=True)
        return s, d

    with cf.ThreadPoolExecutor(max(1, a.jobs)) as ex:
        results = list(ex.map(work, muts))

    rows = []
    for m, (s, d) in zip(muts, results):
        rows.append({"id": m.id, "line": m.line, "before": m.before, "after": m.after,
                     "status": "KILLED" if s == "TIMEOUT" else s, "detail": d.splitlines()[0] if d else "",
                     "timeout": s == "TIMEOUT"})
    cnt = {k: sum(1 for r in rows if r["status"] == k) for k in ("KILLED", "SURVIVED", "BROKEN")}
    n = len(rows)
    denom = n - cnt["BROKEN"]
    score = f"{cnt['KILLED']}/{denom}"
    print(f"mutants {n}  killed {cnt['KILLED']}  survived {cnt['SURVIVED']}  broken {cnt['BROKEN']}  score {score}"
          f"  ({time.monotonic() - t0:.0f}s, {len(tests)} test(s): {', '.join(t.name for t in tests)})")
    for r in rows:
        if r["status"] == "SURVIVED":
            print(f"SURVIVED {r['id']} {hrel}:{r['line']} {r['before']} -> {r['after']}")
    if a.json:
        Path(a.json).write_text(json.dumps(
            {"header": str(hrel), "tests": [rel_to_repo(t) for t in tests],
             "counts": {"mutants": n, "killed": cnt["KILLED"], "survived": cnt["SURVIVED"], "broken": cnt["BROKEN"]},
             "score": score, "mutants": rows}, indent=2), encoding="utf-8")
    shutil.rmtree(MUT_ROOT, ignore_errors=True) if not a.keep and not any(MUT_ROOT.iterdir()) else None
    return 1 if cnt["SURVIVED"] else 0


if __name__ == "__main__":
    sys.exit(main())
