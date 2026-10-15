#!/usr/bin/env python3
"""review_bundle.py - pack tracked repo files into one reviewer upload (.txt) per manifest bundle. Python 3.11+, stdlib only.

Usage:
  review_bundle.py list
  review_bundle.py plan  <id>... | --all [--line-numbers]
  review_bundle.py build <id>... | --all [--out DIR] [--allow-over] [--line-numbers]
  common flags: --repo-root DIR (default: parent of scripts/)  --manifest FILE (default: patch_manager_mods/review_bundles.toml)

list    one line per bundle in the manifest.
plan    dry run: per-file table (role, est tokens, bytes, dirty), role totals, omitted files, dirty files,
        unresolved includes, cap status. Writes nothing.
build   writes <out>/<id>.txt, <id>.manifest.tsv and _index.tsv (default out: .claude/bundles). A bundle over its cap
        or with a secret-content finding prints its table and is NOT written (--allow-over bypasses the cap only).
Files come from `git ls-files` (tracked only) and are read from the worktree; modified ones are marked dirty.
Order inside a bundle: docs, deps, source, tests, config. Estimate = wave_budget._tokens (code bytes/3.5, prose bytes/4).
Skips (stub in the tree): credential/secret in any path component, binary, embedded data / files over max_file_tokens,
__pycache__, *_evidence/, baseline_*.txt, PULL_REQUEST.md, CHANGELOG.md. `</document` in content is escaped as `<\\/document`.
Output has no timestamps: two builds of the same tree are byte-identical.
Exit 0 ok, 1 over cap or finding, 2 usage / manifest error.
"""
import argparse
import fnmatch
import os
import posixpath
import re
import subprocess
import sys
import tomllib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import wave_budget as W  # noqa: E402

DEFAULT_MANIFEST = "patch_manager_mods/review_bundles.toml"
DEFAULT_OUT = ".claude/bundles"
DEFAULT_CAP = 250_000
# Gemini web read only ~375-400 KB of an uploaded .txt (probe 2026-10-14: saw MARK 15 at 375000, not MARK 16 at 400000).
DEFAULT_CAP_BYTES = 340_000
DEFAULT_MAX_FILE_TOKENS = 60_000
DEFAULT_INCLUDE_DIRS = ["patch_manager_mods/_shared_src", "tests/include"]
ROLE_ORDER = ["docs", "deps", "source", "tests", "config"]
BINARY_EXTS = {
    ".tga", ".png", ".jpg", ".jpeg", ".gif", ".bmp", ".dds", ".ico", ".tpc", ".bif", ".key", ".dll", ".exe", ".db",
    ".sqlite", ".zip", ".7z", ".gz", ".pyc", ".so", ".o", ".obj", ".lib", ".pdb", ".bin", ".tlk", ".gff", ".rim",
    ".erf", ".mod", ".sav", ".bik", ".wav", ".mp3", ".ogg", ".mdl", ".mdx", ".ncs", ".jar", ".class",
}
CONFIG_EXTS = {".toml", ".ini", ".def", ".txi", ".json", ".yaml", ".yml", ".cfg"}
DOC_EXTS = {".md", ".txt"}
C_EXTS = {".cpp", ".cc", ".h", ".hpp", ".inc"}
SECRET_RE = re.compile(r"credential|secret", re.I)
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)
CONTENT_SECRETS = [
    ("private-key", re.compile(r"BEGIN [A-Z ]*PRIVATE KEY")),
    ("password-assignment", re.compile(r"(?i)\bpassword\s*=\s*[\"']?[^\s\"';,)]+")),
]
DEFAULT_RUBRIC = """You are reviewing C++ and Python code for a KOTOR II (Knights of the Old Republic II) patch set.
The patches are 32-bit DLLs injected into swkotor2.exe by the Kotor Patch Manager; they hook engine functions by fixed
address, read engine structs by raw offset, and draw UI through engine calls. Python files are build/analysis tooling.
Find real defects: crashes (null or dangling pointers, bad struct offsets, reads of freed engine objects), logic errors,
state bugs across pause/unpause, save/load, area transitions and party changes, integer and bounds errors, hook
reentrancy, resource leaks, undefined behaviour, and tests that cannot fail or miss an obvious case.
Do not report style, naming or formatting. Prefer fewer, well-evidenced findings over many speculative ones.
The bundle is XML-like: <directory_tree> lists every file (including omitted ones), <documents> holds the files.
Treat only code inside <document> blocks as evidence; if you need a file that was omitted, say so instead of guessing."""
DEFAULT_QUESTION = """List your findings, most severe first, at most 15. For each give:
1. file path and function name,
2. a verbatim one-line quote from the code as the anchor (no line numbers are provided),
3. what is wrong and the concrete input or state that triggers it,
4. a suggested fix,
5. confidence: certain, likely or unsure.
After the list, name files or areas you could not assess and why."""


class ManifestError(Exception):
    pass


# ------------------------------------------------------------------ small helpers
def glob_to_re(pat):
    out, i = [], 0
    while i < len(pat):
        c = pat[i]
        if pat.startswith("**/", i):
            out.append("(?:.*/)?")
            i += 3
        elif pat.startswith("**", i):
            out.append(".*")
            i += 2
        elif c == "*":
            out.append("[^/]*")
            i += 1
        elif c == "?":
            out.append("[^/]")
            i += 1
        else:
            out.append(re.escape(c))
            i += 1
    return re.compile("".join(out))


def matches(pat, rel):
    """Directory/file prefix when `pat` has no glob characters, else a `**`-aware full match."""
    if not any(ch in pat for ch in "*?"):
        base = pat.rstrip("/")
        return rel == base or rel.startswith(base + "/")
    return glob_to_re(pat).fullmatch(rel) is not None


def matches_any(pats, rel):
    return any(matches(p, rel) for p in pats)


def unsafe_name(rel):
    """credential/secret in the basename or any directory component (stricter than check_skills.unsafe_name)."""
    return any(SECRET_RE.search(part) for part in rel.split("/"))


def is_noise(rel):
    parts = rel.split("/")
    base = parts[-1]
    if "__pycache__" in parts or any(p.endswith("_evidence") for p in parts[:-1]):
        return True
    return (fnmatch.fnmatch(base, "baseline_*.txt") or base in ("PULL_REQUEST.md", "CHANGELOG.md")
            or base.endswith(".pyc"))


def is_test(rel):
    base = posixpath.basename(rel)
    return bool(re.search(r"_test\.(cpp|cc|py)$", base)) or (base.startswith("test_") and base.endswith(".py"))


def classify(rel):
    ext = posixpath.splitext(rel)[1].lower()
    if ext in DOC_EXTS:
        return "docs"
    if is_test(rel):
        return "tests"
    if ext in CONFIG_EXTS:
        return "config"
    return "source"


def read_file(path):
    """The only place file content is read; tests wrap it to prove skipped files are never opened."""
    with open(path, "rb") as fh:
        return fh.read()


def est_tokens(nbytes, rel):
    return W._tokens(nbytes, rel)


def escape_content(text):
    return text.replace("</document", "<\\/document")


def attr(s):
    return str(s).replace("&", "&amp;").replace('"', "&quot;").replace("<", "&lt;")


def git(root, *args):
    proc = subprocess.run(["git", *args], cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
    if proc.returncode != 0:
        raise ManifestError("git %s failed: %s" % (" ".join(args), proc.stderr.decode(errors="replace").strip()))
    return proc.stdout


def tracked_files(root):
    raw = git(root, "ls-files", "-z")
    return sorted(p.decode("utf-8", errors="replace") for p in raw.split(b"\x00") if p)


def dirty_files(root):
    toks = git(root, "status", "--porcelain", "-z", "--untracked-files=no").split(b"\x00")
    out, i = set(), 0
    while i < len(toks):
        t = toks[i]
        i += 1
        if len(t) < 4:
            continue
        xy, path = t[:2].decode(), t[3:].decode("utf-8", errors="replace")
        if "R" in xy or "C" in xy:
            i += 1  # skip the origin path of a rename/copy
        out.add(path)
    return out


def head_sha(root):
    try:
        return git(root, "rev-parse", "--short=12", "HEAD").decode().strip()
    except ManifestError:
        return "unknown"


# ------------------------------------------------------------------ manifest
def load_manifest(path):
    try:
        with open(path, "rb") as fh:
            data = tomllib.load(fh)
    except FileNotFoundError:
        raise ManifestError("manifest not found: %s" % path)
    except tomllib.TOMLDecodeError as exc:
        raise ManifestError("manifest %s: %s" % (path, exc))
    defaults = data.get("defaults", {})
    prompt = data.get("prompt", {})
    cfg = {
        "cap_tokens": int(defaults.get("cap_tokens", DEFAULT_CAP)),
        "cap_bytes": int(defaults.get("cap_bytes", DEFAULT_CAP_BYTES)),
        "max_file_tokens": int(defaults.get("max_file_tokens", DEFAULT_MAX_FILE_TOKENS)),
        "include_dirs": list(defaults.get("include_dirs", DEFAULT_INCLUDE_DIRS)),
        "rubric": prompt.get("rubric", DEFAULT_RUBRIC).strip(),
        "question": prompt.get("question", DEFAULT_QUESTION).strip(),
    }
    bundles, seen = [], set()
    for b in data.get("bundle", []):
        for key in ("id", "roots"):
            if key not in b:
                raise ManifestError("bundle missing '%s': %r" % (key, b))
        if b["id"] in seen:
            raise ManifestError("duplicate bundle id: %s" % b["id"])
        seen.add(b["id"])
        bundles.append({
            "id": b["id"],
            "title": b.get("title", b["id"]),
            "roots": list(b["roots"]),
            "exclude": list(b.get("exclude", [])),
            "omit": list(b.get("omit", [])),
            "docs": list(b.get("docs", [])),
            "deps": bool(b.get("deps", False)),
            "cap_tokens": int(b.get("cap_tokens", cfg["cap_tokens"])),
            "cap_bytes": int(b.get("cap_bytes", cfg["cap_bytes"])),
            "review_set": b.get("review_set", ""),
            "focus": b.get("focus", "").strip(),
            "note": b.get("note", "").strip(),
        })
    return cfg, bundles


# ------------------------------------------------------------------ selection
class Entry:
    __slots__ = ("rel", "role", "doc_idx", "status", "reason", "nbytes", "tokens", "dirty", "text")

    def __init__(self, rel, role, doc_idx=0):
        self.rel, self.role, self.doc_idx = rel, role, doc_idx
        self.status, self.reason, self.nbytes, self.tokens, self.dirty, self.text = "included", "", 0, 0, False, ""

    def omit(self, reason):
        self.status, self.reason, self.text, self.tokens = "omitted", reason, "", 0


def render_text(raw, line_numbers):
    text = raw.decode("utf-8", errors="replace").replace("\r\n", "\n")
    if text and not text.endswith("\n"):
        text += "\n"
    text = escape_content(text)
    if line_numbers:
        text = "".join("%5d| %s\n" % (i, ln) for i, ln in enumerate(text.splitlines(), 1))
    return text


def load_entry(e, root, bundle, cfg, line_numbers, dirty):
    """Decide include/omit for one file. Name and size checks run before any open()."""
    e.dirty = e.rel in dirty
    if unsafe_name(e.rel):
        return e.omit("secret-name")
    if is_noise(e.rel):
        return e.omit("noise")
    if matches_any(bundle["omit"], e.rel):
        return e.omit("manifest-omit")
    full = os.path.join(root, e.rel)
    if os.path.islink(full):
        return e.omit("symlink")
    if posixpath.splitext(e.rel)[1].lower() in BINARY_EXTS:
        e.nbytes = _size(full)
        return e.omit("binary")
    if not os.path.isfile(full):
        return e.omit("missing")
    e.nbytes = os.path.getsize(full)
    if est_tokens(e.nbytes, e.rel) > cfg["max_file_tokens"]:
        return e.omit("over max_file_tokens")
    raw = read_file(full)
    if b"\x00" in raw[:8192]:
        return e.omit("binary")
    text = render_text(raw, line_numbers)
    for label, rx in CONTENT_SECRETS:
        m = rx.search(text)
        if m:
            line = text.count("\n", 0, m.start()) + 1
            e.omit("finding:%s@%d" % (label, line))
            return
    e.text = text
    e.tokens = est_tokens(len(text.encode("utf-8")), e.rel)
    if e.tokens > cfg["max_file_tokens"]:
        e.omit("over max_file_tokens")


def _size(path):
    try:
        return os.path.getsize(path)
    except OSError:
        return 0


def resolve_include(inc, including_rel, tracked_set, include_dirs):
    cands = [posixpath.join(posixpath.dirname(including_rel), inc)]
    cands += [posixpath.join(d, inc) for d in include_dirs]
    for c in cands:
        n = posixpath.normpath(c)
        if not n.startswith("..") and n in tracked_set:
            return n
    return None


def select(root, bundle, cfg, tracked, dirty, line_numbers=False):
    """Returns (entries sorted in output order, unresolved [(file, include)])."""
    tracked_set = set(tracked)
    entries = {}
    for i, d in enumerate(bundle["docs"]):
        if d not in tracked_set:
            raise ManifestError("bundle %s: doc not tracked: %s" % (bundle["id"], d))
        entries[d] = Entry(d, "docs", i)
    for rel in tracked:
        if rel in entries or not matches_any(bundle["roots"], rel) or matches_any(bundle["exclude"], rel):
            continue
        entries[rel] = Entry(rel, classify(rel), 10**6)
    if not entries:
        raise ManifestError("bundle %s selects no files" % bundle["id"])
    for e in list(entries.values()):
        load_entry(e, root, bundle, cfg, line_numbers, dirty)
    unresolved = set()
    if bundle["deps"]:
        queue = [e for e in entries.values() if e.status == "included" and posixpath.splitext(e.rel)[1] in C_EXTS]
        while queue:
            cur = queue.pop(0)
            for inc in INCLUDE_RE.findall(cur.text):
                hit = resolve_include(inc, cur.rel, tracked_set, cfg["include_dirs"])
                if hit is None:
                    unresolved.add((cur.rel, inc))
                elif hit not in entries:
                    dep = Entry(hit, "deps")
                    load_entry(dep, root, bundle, cfg, line_numbers, dirty)
                    entries[hit] = dep
                    if dep.status == "included" and posixpath.splitext(hit)[1] in C_EXTS:
                        queue.append(dep)
    ordered = sorted(entries.values(), key=lambda e: (ROLE_ORDER.index(e.role), e.doc_idx, e.rel))
    return ordered, sorted(unresolved)


# ------------------------------------------------------------------ rendering
def tree_text(entries):
    lines, prev = [], []
    for e in sorted(entries, key=lambda e: e.rel):
        parts = e.rel.split("/")
        common = 0
        while common < len(prev) and common < len(parts) - 1 and prev[common] == parts[common]:
            common += 1
        for depth in range(common, len(parts) - 1):
            lines.append("  " * depth + parts[depth] + "/")
        if e.status == "included":
            info = "[%s, ~%d tokens%s]" % (e.role, e.tokens, ", dirty" if e.dirty else "")
        else:
            info = "[omitted: %s, %d bytes]" % (e.reason, e.nbytes)
        lines.append("  " * (len(parts) - 1) + parts[-1] + "  " + info)
        prev = parts[:-1]
    return "\n".join(lines)


def compose(bundle, cfg, entries, unresolved, head, est):
    inc = [e for e in entries if e.status == "included"]
    rubric = cfg["rubric"] + ("\n\nFocus for this bundle: " + bundle["focus"] if bundle["focus"] else "")
    # Repeated from the end of the bundle: an upload can be cut off before <question> is reached.
    rubric += "\n\nYour task and answer format (repeated at the end of the bundle):\n" + cfg["question"]
    out = ['<review_bundle id="%s" title="%s" git_head="%s" est_tokens="%d" files="%d">'
           % (attr(bundle["id"]), attr(bundle["title"]), head, est, len(inc)),
           "<instructions>", rubric, "</instructions>", "<directory_tree>", tree_text(entries)]
    if unresolved:
        out.append("")
        out.append("Includes not resolvable inside the repo (files not provided):")
        out += ["  %s -> \"%s\"" % (f, i) for f, i in unresolved]
    out += ["</directory_tree>", "<documents>"]
    for e in inc:
        out.append('<document path="%s" role="%s" est_tokens="%d"%s>'
                   % (attr(e.rel), e.role, e.tokens, ' dirty="1"' if e.dirty else ""))
        out.append(e.text + "</document>")
    out += ["</documents>", "<question>", cfg["question"], "</question>", "</review_bundle>", ""]
    return "\n".join(out)


def total_estimate(bundle, cfg, entries, unresolved, head):
    """File tokens use the per-file ratio; wrapper, tree and prompt text is counted as prose (bytes/4)."""
    inc = [e for e in entries if e.status == "included"]
    file_tokens = sum(e.tokens for e in inc)
    file_bytes = sum(len(e.text.encode("utf-8")) for e in inc)
    est = 0
    for _ in range(4):  # est_tokens sits in the header, so iterate to a fixed point
        text = compose(bundle, cfg, entries, unresolved, head, est)
        new = file_tokens + est_tokens(len(text.encode("utf-8")) - file_bytes, "x.txt")
        if new == est:
            break
        est = new
    return est, text


def manifest_tsv(entries):
    rows = ["path\trole\tstatus\treason\test_tokens\tbytes\tdirty"]
    for e in entries:
        rows.append("%s\t%s\t%s\t%s\t%d\t%d\t%d" % (e.rel, e.role, e.status, e.reason, e.tokens, e.nbytes or len(e.text.encode()), int(e.dirty)))
    return "\n".join(rows) + "\n"


# ------------------------------------------------------------------ commands
class Plan:
    def __init__(self, bundle, entries, unresolved, est, text):
        self.bundle, self.entries, self.unresolved, self.est, self.text = bundle, entries, unresolved, est, text
        self.findings = [e for e in entries if e.reason.startswith("finding:")]
        self.nbytes = len(text.encode("utf-8"))
        self.over = est > bundle["cap_tokens"] or self.nbytes > bundle["cap_bytes"]


def make_plan(root, bundle, cfg, tracked, dirty, head, line_numbers):
    entries, unresolved = select(root, bundle, cfg, tracked, dirty, line_numbers)
    est, text = total_estimate(bundle, cfg, entries, unresolved, head)
    return Plan(bundle, entries, unresolved, est, text)


def print_plan(p, out=None):
    out = out or sys.stdout
    b = p.bundle
    print("== %s: %s" % (b["id"], b["title"]), file=out)
    print("%-7s %8s %9s %s  %s" % ("role", "tokens", "bytes", "D", "path"), file=out)
    for e in p.entries:
        if e.status == "included":
            print("%-7s %8d %9d %s  %s" % (e.role, e.tokens, len(e.text.encode()), "*" if e.dirty else " ", e.rel), file=out)
    totals = {}
    for e in p.entries:
        if e.status == "included":
            totals[e.role] = totals.get(e.role, 0) + e.tokens
    print("role totals: " + ", ".join("%s=%d" % (r, totals[r]) for r in ROLE_ORDER if r in totals), file=out)
    omitted = [e for e in p.entries if e.status == "omitted"]
    for e in omitted:
        print("omitted: %s (%s, %d bytes)" % (e.rel, e.reason, e.nbytes), file=out)
    dirty = [e.rel for e in p.entries if e.dirty and e.status == "included"]
    print("dirty (%d): %s" % (len(dirty), ", ".join(dirty) or "-"), file=out)
    for f, i in p.unresolved:
        print("unresolved include: %s -> \"%s\"" % (f, i), file=out)
    for e in p.findings:
        print("FINDING: %s (%s)" % (e.rel, e.reason), file=out)
    pct = 100.0 * p.est / b["cap_tokens"]
    print("total est_tokens=%d cap=%d (%.0f%%) bytes=%d cap_bytes=%d (%.0f%%) files=%d omitted=%d -> %s"
          % (p.est, b["cap_tokens"], pct, p.nbytes, b["cap_bytes"], 100.0 * p.nbytes / b["cap_bytes"],
             len(p.entries) - len(omitted), len(omitted), "OVER CAP" if p.over else "ok"), file=out)


def pick(bundles, args):
    if args.all:
        return bundles
    if not args.ids:
        raise ManifestError("give a bundle id or --all")
    by_id = {b["id"]: b for b in bundles}
    missing = [i for i in args.ids if i not in by_id]
    if missing:
        raise ManifestError("unknown bundle id: %s (known: %s)" % (", ".join(missing), ", ".join(by_id)))
    return [by_id[i] for i in args.ids]


def write_index(out_dir, rows):
    path = out_dir / "_index.tsv"
    merged = {}
    if path.exists():
        for line in path.read_text(encoding="utf-8").splitlines()[1:]:
            if line:
                merged[line.split("\t")[0]] = line
    for r in rows:
        merged[r.split("\t")[0]] = r
    body = "id\test_tokens\tcap_tokens\tfiles\tomitted\tdirty\tbytes\tgit_head\n"
    body += "".join(merged[k] + "\n" for k in sorted(merged))
    path.write_text(body, encoding="utf-8")


def cmd_build(root, plans, args):
    out_dir = Path(args.out) if args.out else Path(root) / DEFAULT_OUT
    rc, rows = 0, []
    for p in plans:
        b = p.bundle
        if p.findings or (p.over and not args.allow_over):
            print_plan(p, sys.stderr)
            print("NOT WRITTEN: %s" % ("secret-content finding" if p.findings else "over cap (use --allow-over to bypass)"),
                  file=sys.stderr)
            rc = 1
            continue
        out_dir.mkdir(parents=True, exist_ok=True)
        data = p.text.encode("utf-8")
        (out_dir / (b["id"] + ".txt")).write_bytes(data)
        (out_dir / (b["id"] + ".manifest.tsv")).write_text(manifest_tsv(p.entries), encoding="utf-8")
        inc = [e for e in p.entries if e.status == "included"]
        rows.append("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%s" % (
            b["id"], p.est, b["cap_tokens"], len(inc), len(p.entries) - len(inc),
            sum(1 for e in inc if e.dirty), len(data), head_sha(root)))
        print("wrote %s/%s.txt est_tokens=%d bytes=%d%s" % (out_dir, b["id"], p.est, len(data),
                                                            " (OVER CAP, --allow-over)" if p.over else ""))
    if rows:
        write_index(out_dir, rows)
    return rc


def build_parser():
    ap = argparse.ArgumentParser(prog="review_bundle.py", description="Pack tracked files into reviewer upload bundles.")
    ap.add_argument("--repo-root", default=str(Path(__file__).resolve().parent.parent))
    ap.add_argument("--manifest", default=None, help="default: %s under the repo root" % DEFAULT_MANIFEST)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("list")
    for name in ("plan", "build"):
        sp = sub.add_parser(name)
        sp.add_argument("ids", nargs="*")
        sp.add_argument("--all", action="store_true")
        sp.add_argument("--line-numbers", action="store_true")
        if name == "build":
            sp.add_argument("--out", default=None, help="default: %s under the repo root" % DEFAULT_OUT)
            sp.add_argument("--allow-over", action="store_true")
    return ap


def main(argv=None):
    try:
        args = build_parser().parse_args(argv)
    except SystemExit as exc:
        return 0 if exc.code in (0, None) else 2
    root = os.path.abspath(args.repo_root)
    manifest = args.manifest or os.path.join(root, DEFAULT_MANIFEST)
    try:
        cfg, bundles = load_manifest(manifest)
        if args.cmd == "list":
            for b in bundles:
                print("%s\tset=%s\tcap=%d\troots=%d\tdeps=%s\t%s" % (b["id"], b["review_set"] or "-", b["cap_tokens"], len(b["roots"]), b["deps"], b["title"]))
            return 0
        chosen = pick(bundles, args)
        tracked, dirty, head = tracked_files(root), dirty_files(root), head_sha(root)
        plans = [make_plan(root, b, cfg, tracked, dirty, head, args.line_numbers) for b in chosen]
    except ManifestError as exc:
        print("error: %s" % exc, file=sys.stderr)
        return 2
    if args.cmd == "plan":
        for p in plans:
            print_plan(p)
        return 1 if any(p.over or p.findings for p in plans) else 0
    return cmd_build(root, plans, args)


if __name__ == "__main__":
    sys.exit(main())
