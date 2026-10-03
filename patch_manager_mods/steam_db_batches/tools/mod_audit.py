#!/usr/bin/env python3
"""mod_audit.py - read-only cross-check of the Steam addresses hard-coded in patch_manager_mods/*_src/ against the
validated address DB (knowledge DB `steam_validated` + `name_map`, Steam fp.tsv function starts/sizes, branch DB data tables).

  python3 tools/mod_audit.py [--src-root DIR] [--out DIR] [--branch-db DB]

Writes `<out>/mod_audit.tsv` and `<out>/REPORT.md` (default out = steam_db_batches/mod_audit/) and prints a one-line
status summary.  Nothing under the source tree and no database is ever written (SQLite is opened with bdlib.connect_ro).

Status values
  validated-label-conflict validated, offset 0, label does not fit AND equals the method name of a different function
                           in steam_validated / trusted name_map ("mod thinks it is X, X lives elsewhere"; printed in
                           conflict_with; a bare name carried by >2 other functions is too ambiguous and is not a conflict)
  validated-label-differs  validated, offset 0, literal/ident/toml label shares no word with the validated name (weak)
  validated-agree          validated, offset 0, label shares a word with (or is a substring of) the validated name
  validated                validated function but no verdict: mid-function hook, comment-only / no label, or no usable word
  name_map-trusted         not validated; name_map has the function start with confidence high/medium
  name_map-untrusted       not validated; name_map has it with low/wrong/other confidence
  unnamed                  inside a known Steam function that is neither validated nor in name_map
  data                     inside no function but a known global / non-.text PE section / past the last function
  unknown                  inside no function and not data
The `mid` column is "mid" when the address is not the function start (hook in the middle of a function).

Labels: the printed label is the first of string literal (identifier-like only) > defined/assigned identifier (a TOML
key or the `function`/`name` of the same [[hooks]] table when the key is `address`) > comment text; `label_src` is
literal | ident | toml | comment | none and comments are never judged.  Fit = the CamelCase/snake_case word sets of the
method parts share a word (lower-cased; stop words like hook/fn/ptr/addr, tokens < 3 chars and hex-looking tokens dropped;
ctor=constructor=cstrconstructor, dtor=destructor, init=initialize), or the normalised names are substrings (>= 3 chars).

Stdlib only.
"""
from __future__ import annotations

import argparse
import bisect
import os
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bdlib as bd  # noqa: E402

SRC_SUFFIXES = {".cpp", ".h", ".hpp", ".inc", ".toml"}
SKIP_DIRS = {"build", "bin", "obj", ".vs", ".git", "__pycache__"}
STATUSES = ["validated-label-conflict", "validated-label-differs", "validated-agree", "validated", "name_map-trusted",
            "name_map-untrusted", "unnamed", "data", "unknown"]
TSV_COLS = ["mod", "file_line", "addr", "label", "label_src", "func_start", "offset", "validated_name", "kdb_name",
            "status", "mid", "conflict_with"]
COMPARABLE_SRC = {"literal", "ident", "toml"}      # label sources that may be judged (comments never are)
CONFLICT_MAX_OTHERS = 2                           # a bare label that names >2 other functions is too ambiguous to be a conflict
TRUSTED_CONF = {"high", "medium", "med"}
REPORT_ROW_CAP = 60

# 8-digit 0x00[4-9A-B]xxxxx (the A/B range is the .data/.bss globals, beyond the brief's 4-9), or 6-digit 0x[4-9]xxxxx;
# never a slice of a longer hex literal / identifier.
ADDR_RE = re.compile(r"(?<![0-9A-Za-z_])0[xX](?:00[4-9A-Ba-b]|[4-9])[0-9A-Fa-f]{5}(?![0-9A-Fa-f_])")
STR_RE = re.compile(r'"((?:[^"\\]|\\.)*)"')
DEFINE_RE = re.compile(r"#\s*define\s+([A-Za-z_]\w*)")
ASSIGN_RE = re.compile(r"([A-Za-z_][\w.\-]*)\s*(?:\[[^\]]*\])?\s*=(?!=)")
IDENT_LIKE = re.compile(r"^[A-Za-z_][\w:.\-]*$")


# ---------------------------------------------------------------------------------------------- scanning
def iter_source_files(src_root: Path):
    """Yield (mod, path) for every *.{cpp,h,hpp,inc,toml} under src_root/*_src/, skipping build-ish dirs."""
    for moddir in sorted(p for p in Path(src_root).glob("*_src") if p.is_dir()):
        for dirpath, dirnames, filenames in os.walk(moddir):
            dirnames[:] = sorted(d for d in dirnames if d not in SKIP_DIRS)
            for fn in sorted(filenames):
                if Path(fn).suffix.lower() in SRC_SUFFIXES:
                    yield moddir.name, Path(dirpath) / fn


def strip_strings(line: str) -> str:
    return STR_RE.sub(lambda m: '"' + " " * len(m.group(1)) + '"', line)


def comment_start(line: str, toml: bool) -> int:
    """Index where a `//` (or `/*`, or TOML `#`) comment starts outside string literals, else len(line)."""
    bare = strip_strings(line)
    marks = ["#"] if toml else ["//", "/*"]
    hits = [i for i in (bare.find(m) for m in marks) if i >= 0]
    return min(hits) if hits else len(line)


def split_comment(line: str, toml: bool) -> str:
    """Comment text on the line (outside string literals), '' if none."""
    i = comment_start(line, toml)
    if i >= len(line):
        return ""
    text = line[i:]
    text = text[1:] if toml else text[2:]
    return text.split("*/")[0].strip()


NAME_KEYS = {"name", "function", "symbol", "id", "label"}


def line_labels(line: str, addr_pos: int, toml: bool) -> tuple[str, str, list[tuple[str, str]]]:
    """(primary label, label_src, candidates [(text, src)]) for an address found at addr_pos in `line`.

    Priority: identifier-like string literal > defined/assigned identifier (src `toml` for a TOML key) > comment text.
    Literals and identifiers are taken from the code part of the line only (an address inside a comment is
    comment-labelled).  label_src is literal | ident | toml | comment | none; only the first three may be judged."""
    cend = comment_start(line, toml)
    in_code = addr_pos < cend
    comment = split_comment(line, toml)
    lits, ident = [], ""
    if in_code:
        lits = [m.group(1) for m in STR_RE.finditer(line)
                if m.start() < cend and m.group(1).strip() and IDENT_LIKE.match(m.group(1))]
        bare = strip_strings(line)[:cend]
        m = DEFINE_RE.search(bare)
        if m:
            ident = m.group(1)
        else:
            for m in ASSIGN_RE.finditer(bare[:addr_pos]):
                ident = m.group(1)
        if toml and ident.lower() not in NAME_KEYS | GENERIC_KEYS:
            ident = ""                                  # `description = ...`, `type = ...`: not a name
    id_src = "toml" if toml else "ident"
    cands = [(x, "literal") for x in lits] + ([(ident, id_src)] if ident else []) + \
            ([(comment[:60].strip(), "comment")] if comment else [])
    if not cands:
        return "", "none", []
    return cands[0][0], cands[0][1], cands


GENERIC_KEYS = {"address", "addr", "target", "va", "offset"}
BLOCK_NAME_RE = re.compile(r'^\s*(?:function|name|id|symbol)\s*=\s*"([^"]*)"')


def toml_block_name(lines: list[str], idx: int) -> str:
    """`function`/`name` string within the next 8 lines of the same TOML table (idx is the 1-based address line)."""
    for line in lines[idx:idx + 8]:
        if line.lstrip().startswith("["):
            break
        m = BLOCK_NAME_RE.match(line)
        if m:
            return m.group(1)
    return ""


def scan_file(mod: str, path: Path, rel_root: Path):
    """Yield dicts {mod,file_line,addr,label,cands,comment} for every address hit (deduped per line)."""
    toml = path.suffix.lower() == ".toml"
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return
    rel = path.relative_to(rel_root).as_posix() if rel_root in path.parents else path.as_posix()
    lines = text.splitlines()
    for n, line in enumerate(lines, 1):
        seen = set()
        for m in ADDR_RE.finditer(line):
            a = int(m.group(0), 16)
            if a in seen:
                continue
            seen.add(a)
            label, src, cands = line_labels(line, m.start(), toml)
            if toml and src == "toml" and label.lower() in GENERIC_KEYS:
                nxt = toml_block_name(lines, n)       # `address = 0x..` carries no name: use the block's `function = "..."`
                if nxt:
                    label, src, cands = nxt, "toml", [(nxt, "toml")] + cands
                elif src == "toml":
                    cands = [c for c in cands if c[1] != "toml"]
                    label, src = (cands[0][0], cands[0][1]) if cands else ("", "none")
            yield {"mod": mod, "file_line": "%s:%d" % (rel, n), "addr": a, "label": label, "label_src": src,
                   "cands": cands}


# ---------------------------------------------------------------------------------------------- label fitting
STOP_WORDS = {"addr", "address", "fn", "func", "ptr", "p", "k", "g", "the", "of", "on", "to", "is", "hook", "hooks",
              "orig", "call", "this", "thiscall", "mod", "and", "generic", "detour", "patch", "site", "cave", "stub",
              "trampoline", "tail"}
SYNONYMS = {"ctor": "constructor", "cstrconstructor": "constructor", "dtor": "destructor", "init": "initialize",
            "initialise": "initialize", "alloc": "allocate", "dealloc": "deallocate"}
WORD_RE = re.compile(r"[A-Z]+(?![a-z])|[A-Z]?[a-z]+|\d+")


def method_part(label: str) -> str:
    return (label or "").strip().split("::")[-1]


def normalize(label: str) -> str:
    s = method_part(label)
    s = re.sub(r"^(?:g_|addr_|fn_)", "", s, flags=re.I)
    s = re.sub(r"^[kp](?=[A-Z_])", "", s)
    s = re.sub(r"^_", "", s)
    return re.sub(r"[^a-z0-9]", "", s.lower())


def words(label: str) -> set[str]:
    """CamelCase / snake_case words of the method part: lower-cased, synonyms applied, stop words and tokens < 3 dropped."""
    out = set()
    for chunk in re.split(r"[^A-Za-z0-9]+", method_part(label)):
        for w in WORD_RE.findall(chunk):
            w = SYNONYMS.get(w.lower(), w.lower())
            if len(w) >= 3 and w not in STOP_WORDS and not re.fullmatch(r"\d+|[0-9a-f]*\d[0-9a-f]*", w):
                out.add(w)
    return out


def fits(label: str, validated_name: str) -> bool:
    """Word sets share a word, or the normalised method names are substrings of each other (both >= 3 chars)."""
    if words(label) & words(validated_name):
        return True
    a, b = normalize(label), normalize(validated_name)
    return len(a) >= 3 and len(b) >= 3 and (a in b or b in a)


def judgeable(cands: list[tuple[str, str]]) -> list[str]:
    """Candidate texts that may be judged: comparable source and at least one usable word."""
    return [t for t, src in cands if src in COMPARABLE_SRC and words(t)]


def any_fits(texts: list[str], names: list[str]) -> bool:
    return any(fits(t, n) for t in texts for n in names)


# ---------------------------------------------------------------------------------------------- reference data
class Ref:
    """Everything the classifier needs, loaded once (all read-only)."""

    def __init__(self, paths, branch_db: Path | None = None):
        self.starts: list[int] = []
        self.size: dict[int, int] = {}
        for r in bd.iter_tsv(paths.server / paths.steam_program / "fp.tsv"):
            try:
                a = bd.parse_addr(r["addr"])
            except (ValueError, KeyError):
                continue
            self.size[a] = bd._to_int(r.get("size"), 0) or 1
        self.starts = sorted(self.size)
        self.last_end = (self.starts[-1] + self.size[self.starts[-1]]) if self.starts else None

        self.validated: dict[int, list[str]] = defaultdict(list)
        self.name_map: dict[int, tuple[str, str]] = {}
        if Path(paths.kdb).exists():
            con = bd.connect_ro(paths.kdb)
            try:
                try:
                    for a, n in con.execute("select steam_addr,name from steam_validated"):
                        self._add_validated(a, n)
                except Exception:
                    pass
                try:
                    for a, n, c in con.execute("select steam_addr,name,confidence from name_map"):
                        try:
                            self.name_map[bd.parse_addr(a)] = (n or "", (c or "").strip().lower())
                        except ValueError:
                            pass
                except Exception:
                    pass
            finally:
                con.close()

        self.globals: dict[int, str] = {}
        bpath = Path(branch_db) if branch_db else Path(paths.branchdb)
        if bpath.exists():
            con = bd.connect_ro(bpath)
            try:
                try:
                    for n, a in con.execute("select pointer_name,address from global_pointers"):
                        self.globals[int(a)] = n
                except Exception:
                    pass
            finally:
                con.close()
        self._build_name_index()
        self.global_addrs = set(self.globals)
        for r in bd.iter_tsv(Path(paths.dump) / "globals.tsv"):
            try:
                self.global_addrs.add(bd.parse_addr(r["target_addr"]))
            except (ValueError, KeyError):
                pass
        self.data_sections: list[tuple[int, int]] = []
        try:
            if Path(paths.exe).is_file():
                pe = bd.PeImage(paths.exe)
                self.data_sections = [(s, s + span) for name, s, span, _p, _r in pe.sections if name != ".text"]
        except Exception:
            self.data_sections = []

    def _build_name_index(self):
        """normalised method name -> {addr: full name} over steam_validated and trusted name_map (for conflicts);
        and normalised full `class::method` -> {addr: full name}."""
        self.by_method: dict[str, dict[int, str]] = defaultdict(dict)
        self.by_full: dict[str, dict[int, str]] = defaultdict(dict)
        entries = [(a, n) for a, ns in self.validated.items() for n in ns]
        entries += [(a, n) for a, (n, c) in self.name_map.items() if c in TRUSTED_CONF]
        for a, n in entries:
            base = bd.base_name(n)
            if not base or base.startswith("FUN_"):
                continue
            self.by_method[normalize(base)][a] = base
            self.by_full[re.sub(r"[^a-z0-9]", "", base.lower())][a] = base

    def conflicts(self, label: str, own: int) -> list[tuple[int, str]]:
        """Other functions (not `own`) whose name IS this label: full name for `Cls::fn` labels, else the method name
        when at most CONFLICT_MAX_OTHERS other functions carry it (a name like Update/Draw is not evidence)."""
        if "::" in label:
            hits = self.by_full.get(re.sub(r"[^a-z0-9]", "", label.lower()), {})
        else:
            hits = self.by_method.get(normalize(label), {})
        others = sorted((a, n) for a, n in hits.items() if a != own)
        if "::" not in label and len(others) > CONFLICT_MAX_OTHERS:
            return []
        return others

    def _add_validated(self, a, n):
        try:
            self.validated[bd.parse_addr(a)].append(n or "")
        except ValueError:
            pass

    def containing(self, addr: int) -> int | None:
        i = bisect.bisect_right(self.starts, addr) - 1
        if i < 0:
            return None
        s = self.starts[i]
        return s if addr < s + self.size[s] else None

    def is_data(self, addr: int) -> bool:
        if addr in self.global_addrs:
            return True
        if any(lo <= addr < hi for lo, hi in self.data_sections):
            return True
        return bool(self.starts) and addr >= self.last_end


def classify(hit: dict, ref: Ref) -> dict:
    a = hit["addr"]
    fs = ref.containing(a)
    src = hit.get("label_src", "none")
    row = {"mod": hit["mod"], "file_line": hit["file_line"], "addr": bd.hx8(a), "label": hit["label"] or "(none)",
           "label_src": src, "func_start": "", "offset": "", "validated_name": "", "kdb_name": "", "status": "",
           "mid": "", "conflict_with": ""}
    if fs is None:
        row["kdb_name"] = ref.globals.get(a, "")
        row["status"] = "data" if ref.is_data(a) else "unknown"
        return row
    row["func_start"] = bd.hx8(fs)
    row["offset"] = str(a - fs)
    row["mid"] = "mid" if a != fs else ""
    nm = ref.name_map.get(fs)
    if nm:
        row["kdb_name"] = nm[0]
    vnames = ref.validated.get(fs)
    if vnames:
        row["validated_name"] = "|".join(vnames)
        texts = judgeable(hit["cands"]) if (a == fs and src in COMPARABLE_SRC) else []
        if not texts:
            row["status"] = "validated"                      # mid-function / comment-only / no usable label: no verdict
        elif any_fits(texts, vnames):
            row["status"] = "validated-agree"
        else:
            row["status"] = "validated-label-differs"
            for t in texts:
                others = ref.conflicts(t, fs)
                if others:
                    row["status"] = "validated-label-conflict"
                    row["conflict_with"] = "; ".join("%s %s" % (bd.hx8(oa), on) for oa, on in others[:3])
                    break
    elif nm:
        row["status"] = "name_map-trusted" if nm[1] in TRUSTED_CONF else "name_map-untrusted"
    else:
        row["status"] = "unnamed"
    return row


def audit(src_root: Path, ref: Ref) -> list[dict]:
    rows, seen = [], set()
    for mod, path in iter_source_files(src_root):
        for hit in scan_file(mod, path, Path(src_root)):
            key = (hit["mod"], hit["addr"], hit["file_line"])
            if key in seen:
                continue
            seen.add(key)
            rows.append(classify(hit, ref))
    return rows


# ---------------------------------------------------------------------------------------------- output
def write_tsv(rows: list[dict], path: Path):
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.write("\t".join(TSV_COLS) + "\n")
        for r in rows:
            f.write("\t".join(str(r[c]).replace("\t", " ").replace("\n", " ") for c in TSV_COLS) + "\n")


def md_cell(s) -> str:
    return str(s).replace("|", "\\|")


def class_of(name: str) -> str:
    b = bd.base_name(name)
    return b.split("::")[0] if "::" in b else "(global)"


def write_report(rows: list[dict], path: Path):
    total = Counter(r["status"] for r in rows)
    mods = sorted({r["mod"] for r in rows})
    per_mod = {m: Counter(r["status"] for r in rows if r["mod"] == m) for m in mods}
    out = ["# Mod address audit", "",
           "Generated by `tools/mod_audit.py` (read-only). %d address hits, %d mods. Labels are judged only at a function "
           "start (offset 0) and only when they come from a literal, identifier or TOML key (never a comment)."
           % (len(rows), len(mods)), ""]

    def table(title, sel, cols, heads, cap, note):
        out.extend(["## %s (%d)" % (title, len(sel)), "", note, ""])
        if not sel:
            out.extend(["None.", ""])
            return
        out.append("| " + " | ".join(heads) + " |")
        out.append("|" + "---|" * len(heads))
        for r in sel[:cap]:
            out.append("| " + " | ".join(md_cell(r[k]) for k in cols) + " |")
        if len(sel) > cap:
            out.extend(["", "(%d more rows in mod_audit.tsv, capped at %d here)" % (len(sel) - cap, cap)])
        out.append("")

    conflicts = [r for r in rows if r["status"] == "validated-label-conflict"]
    table("validated-label-conflict", conflicts,
          ("mod", "file_line", "addr", "label", "label_src", "validated_name", "kdb_name", "conflict_with"),
          ("mod", "file_line", "addr", "mod label", "label_src", "validated name here", "kdb name here",
           "label lives at"), 80,
          "The mod calls this address X, but X is the name of a different function in steam_validated / trusted "
          "name_map: the strongest bug signal.")
    differs = [r for r in rows if r["status"] == "validated-label-differs"]
    table("validated-label-differs", differs,
          ("mod", "file_line", "addr", "label", "label_src", "validated_name", "kdb_name"),
          ("mod", "file_line", "addr", "mod label", "label_src", "validated name", "kdb name"), REPORT_ROW_CAP,
          "Function start, literal/identifier/TOML label shares no word with the validated name (weak: often just a "
          "hook name).")

    out += ["## Counts per mod x status", ""]
    cols = [s for s in STATUSES if total[s]]
    out.append("| mod | " + " | ".join(cols) + " | total |")
    out.append("|---|" + "---:|" * (len(cols) + 1))
    for m in mods:
        out.append("| %s | %s | %d |" % (m, " | ".join(str(per_mod[m][s]) for s in cols), sum(per_mod[m].values())))
    out.append("| **all** | %s | %d |" % (" | ".join(str(total[s]) for s in cols), len(rows)))
    out.append("")

    todo = [r for r in rows if r["status"] in ("name_map-trusted", "name_map-untrusted") and r["kdb_name"]]
    by_class: dict[str, list[dict]] = defaultdict(list)
    for r in todo:
        by_class[class_of(r["kdb_name"])].append(r)
    out += ["## Not yet validated: mod addresses per class (candidate batch coverage)", "",
            "Non-validated rows with a name_map name; class = name prefix before `::`.", ""]
    if by_class:
        out += ["| class | mod addresses | distinct functions | mods |", "|---|---:|---:|---|"]
        ranked = sorted(by_class.items(), key=lambda kv: (-len(kv[1]), kv[0]))
        for cls, rs in ranked[:REPORT_ROW_CAP]:
            out.append("| %s | %d | %d | %s |" % (md_cell(cls), len({r["addr"] for r in rs}),
                       len({r["func_start"] for r in rs}), md_cell(", ".join(sorted({r["mod"] for r in rs})))))
        if len(ranked) > REPORT_ROW_CAP:
            out += ["", "(%d more classes in mod_audit.tsv, capped at %d here)" % (len(ranked) - REPORT_ROW_CAP, REPORT_ROW_CAP)]
        out += ["", "### Classes per mod", ""]
        for m in mods:
            cs = Counter(class_of(r["kdb_name"]) for r in todo if r["mod"] == m)
            if cs:
                top = ", ".join("%s (%d)" % (c, n) for c, n in sorted(cs.items(), key=lambda kv: (-kv[1], kv[0]))[:8])
                out.append("- %s: %s%s" % (m, top, " ..." if len(cs) > 8 else ""))
    else:
        out.append("None.")
    out.append("")
    out += ["## Not classifiable", "",
            "- unnamed (inside a function with no validated or name_map name): %d" % total["unnamed"],
            "- data: %d, unknown: %d" % (total["data"], total["unknown"]), ""]
    path.write_text("\n".join(out), encoding="utf-8")


def main(argv=None) -> int:
    paths = bd.get_paths()
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--src-root", default=str(bd.PM_ROOT), help="dir containing the *_src mod dirs")
    ap.add_argument("--out", default=str(paths.batches / "mod_audit"))
    ap.add_argument("--branch-db", default=None)
    args = ap.parse_args(argv)
    ref = Ref(paths, Path(args.branch_db) if args.branch_db else None)
    rows = audit(Path(args.src_root), ref)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    write_tsv(rows, out / "mod_audit.tsv")
    write_report(rows, out / "REPORT.md")
    c = Counter(r["status"] for r in rows)
    print("mod_audit: %d rows, %s -> %s" % (len(rows), ", ".join("%s=%d" % (s, c[s]) for s in STATUSES if c[s]), out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
