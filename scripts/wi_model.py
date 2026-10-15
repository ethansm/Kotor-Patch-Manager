#!/usr/bin/env python3
"""wi_model.py - parse the dev work-item index and WI-NN.md headers, compute locksets and eligibility. Python 3, stdlib only.

Pure functions, no I/O beyond reading the work-item files; the claim/state layer is wi_state.py, the CLI is wi_driver.py.

Index row (dev_work_items.md):  | [ ] | rank | WI-NN | P | state | size | after | title | rows | files | why |
WI header (WI-NN.md), `key: value` lines between the `# WI-NN title` line and the first `## ` heading:
  area, size, ready, items, files, out-of-repo, blockers, docs, deferred   (always present, except deferred)
  after:   comma list of WI ids this item waits for (overrides the index cell when present)
  verify:  one gate command per line (repeatable); `verify: none <reason>` declares a gate-free item
`ready:` is ready | verify-only | hold | awaiting-user. The index state is authoritative except that a header value of
awaiting-user wins (the index has no such state).
"""
import fnmatch
import os
import re
import shlex
from dataclasses import dataclass, field
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
WORK_DIR = Path(".claude") / "work_items"
INDEX_NAME = "dev_work_items.md"

INDEX_ROW_RE = re.compile(
    r"^\| \[(?P<mark>[ x!])\] \| (?P<rank>\d+) \| (?P<id>WI-\d+[a-z]*) \| (?P<prio>P\d) \| (?P<state>[a-z-]+) \| "
    r"(?P<size>[A-Z]+) \| (?P<after>[^|]*?) \| (?P<title>[^|]*?) \| (?P<rows>\d+) \| (?P<files>\d+) \| (?P<why>.*?) \|\s*$")
WI_ID_RE = re.compile(r"WI-\d+[a-z]*")
TITLE_RE = re.compile(r"^# (WI-\d+[a-z]*) (.*)$")
HEADER_RE = re.compile(r"^([a-z][a-z-]*):\s*(.*)$")
SOURCE_ROW_RE = re.compile(r"^- (?P<item>C\d+) (?P<doc>\S+?):(?P<line>\d+) id=(?P<pid>\S+) (?P<status>[a-z-]+)(?: \((?P<note>.*)\))?\s*$")
DOC_PATH_RE = re.compile(r"[A-Za-z0-9_./-]+\.md")
NULLS = ("", "-", "unknown", "none", "n/a")
RUNNABLE_STATES = ("ready", "verify-only")
STATES = ("ready", "verify-only", "hold", "awaiting_user")
SHARED_DIR = "patch_manager_mods/_shared_src"
MOD_SRC_RE = re.compile(r"^(patch_manager_mods/[^/]+_src)/")


@dataclass
class SourceRow:
    item: str
    doc: str
    line: int
    plan_id: str | None
    status: str
    note: str = ""


@dataclass
class WorkItem:
    id: str
    title: str = ""
    header_title: str = ""  # title from the WI-NN.md `# WI-NN title` line, kept after load_all overrides title
    rank: int = 0
    prio: str = ""
    state: str = "ready"  # ready | verify-only | hold | awaiting_user
    size: str = ""
    mark: str = " "  # index done cell as rendered: ' ' open, 'x' done, '!' parked
    index_after: list = field(default_factory=list)
    header_after: list | None = None
    area: str = ""
    items: list = field(default_factory=list)
    files: list = field(default_factory=list)
    new_files: list = field(default_factory=list)  # subset of files written as `NEW:path`
    out_of_repo: list = field(default_factory=list)
    blockers: str = ""
    docs: str = ""
    deferred: list = field(default_factory=list)
    verify: list = field(default_factory=list)
    no_gate: str = ""  # reason when `verify: none <reason>` is declared
    source_rows: list = field(default_factory=list)
    goal: str = ""
    header_keys: list = field(default_factory=list)
    has_file: bool = True

    @property
    def after(self):
        got = self.header_after if self.header_after is not None else self.index_after
        return [a for a in got if a != self.id]

    @property
    def external(self):
        """No in-repo write file: only out-of-repo paths (cannot be committed in this repo)."""
        return not self.files and bool(self.out_of_repo)

    @property
    def runnable(self):
        return self.state in RUNNABLE_STATES


def _split_list(value, seps=";"):
    out = []
    for part in re.split("[" + re.escape(seps) + "]", value):
        part = part.strip().strip("`")
        if part and part.lower().split(" ")[0] not in NULLS:
            out.append(part)
    return out


def parse_wi_ids(text):
    return list(dict.fromkeys(WI_ID_RE.findall(text or "")))


def parse_wi_file(path):
    """Parse one WI-NN.md into a WorkItem (index-only fields left at defaults)."""
    lines = Path(path).read_text(encoding="utf-8").splitlines()
    m = TITLE_RE.match(lines[0]) if lines else None
    if not m:
        raise ValueError(f"{path}: first line is not '# WI-NN title'")
    wi = WorkItem(id=m.group(1), title=m.group(2).strip(), header_title=m.group(2).strip())
    section = ""
    goal = []
    for line in lines[1:]:
        if line.startswith("## "):
            section = line[3:].strip().lower()
            continue
        if not section:
            h = HEADER_RE.match(line)
            if h:
                _apply_header(wi, h.group(1), h.group(2).strip())
            continue
        if section == "goal":
            goal.append(line)
        elif section.startswith("source rows"):
            s = SOURCE_ROW_RE.match(line)
            if s:
                pid = s.group("pid")
                wi.source_rows.append(SourceRow(s.group("item"), s.group("doc"), int(s.group("line")),
                                                None if pid.lower() in NULLS else pid, s.group("status"),
                                                s.group("note") or ""))
    wi.goal = " ".join(" ".join(goal).split())
    return wi


def _apply_header(wi, key, value):
    wi.header_keys.append(key)
    if key == "area":
        wi.area = value
    elif key == "size":
        wi.size = value
    elif key == "ready":
        if value.lower().replace("_", "-") == "awaiting-user":
            wi.state = "awaiting_user"
        else:
            wi.state = value.lower()
    elif key == "items":
        wi.items = re.findall(r"C\d+", value)
    elif key == "files":
        raw = _split_list(value)
        wi.new_files = [f[4:].strip() for f in raw if f.startswith("NEW:")]
        wi.files = [f[4:].strip() if f.startswith("NEW:") else f for f in raw]
    elif key == "out-of-repo":
        wi.out_of_repo = _split_list(value)
    elif key == "blockers":
        wi.blockers = value
    elif key == "docs":
        wi.docs = value
    elif key == "deferred":
        wi.deferred = re.findall(r"C\d+", value)
    elif key == "after":
        wi.header_after = parse_wi_ids(value) if value.lower().split(" ")[0] not in NULLS else []
    elif key == "verify":
        if value.lower().startswith("none"):
            wi.no_gate = value[4:].strip(" :-") or "declared gate-free"
        elif value:
            wi.verify.append(value)


def parse_index(path):
    """{wi_id: WorkItem} index columns only, in rank order (dict insertion order)."""
    out = {}
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        m = INDEX_ROW_RE.match(line)
        if not m:
            continue
        w = WorkItem(id=m.group("id"), title=m.group("title"), rank=int(m.group("rank")), prio=m.group("prio"),
                     state=m.group("state"), size=m.group("size"), mark=m.group("mark"),
                     index_after=parse_wi_ids(m.group("after")))
        out[w.id] = w
    return out


def load_all(root=REPO):
    """Merge index rows with WI files. Index fields win for rank/prio/title/mark; the file wins for everything else.

    A header `ready: awaiting-user` overrides the index state. Returns {id: WorkItem} in rank order."""
    root = Path(root)
    idx = parse_index(root / WORK_DIR / INDEX_NAME)
    items = {}
    for wid, row in idx.items():
        f = root / WORK_DIR / f"{wid}.md"
        if not f.is_file():
            row.has_file = False
            items[wid] = row
            continue
        wi = parse_wi_file(f)
        header_state = wi.state
        wi.rank, wi.prio, wi.mark, wi.index_after = row.rank, row.prio, row.mark, row.index_after
        wi.title = row.title or wi.title
        wi.size = row.size or wi.size
        wi.state = "awaiting_user" if header_state == "awaiting_user" else row.state
        items[wid] = wi
    return items


# ---------------------------------------------------------------- locks
def norm_path(p):
    p = p.strip()
    if p.startswith("./"):
        p = p[2:]
    return p.rstrip("/") or p


def paths_overlap(a, b):
    """True when two path/glob locks can name the same file: glob match either way, or one is a directory prefix of the other."""
    a, b = norm_path(a), norm_path(b)
    if a == b or fnmatch.fnmatch(a, b) or fnmatch.fnmatch(b, a):
        return True
    return a.startswith(b + "/") or b.startswith(a + "/")


def doc_paths(wi):
    """Repo-relative plan docs this item edits: Source row docs plus .md paths named on the `docs:` header."""
    docs = [r.doc for r in wi.source_rows]
    docs += [d for d in DOC_PATH_RE.findall(wi.docs) if "/" in d]
    return list(dict.fromkeys(norm_path(d) for d in docs))


def mod_changelogs(wi):
    return list(dict.fromkeys(f"{m.group(1)}/CHANGELOG.md" for f in wi.files if (m := MOD_SRC_RE.match(f))))


def shares_common(wi):
    area = wi.area.lower()
    return any(f.startswith(SHARED_DIR) for f in wi.files) or bool(re.search(r"common|shared|library", area))


def lockset(wi):
    """Paths/globs two concurrent sessions must not both hold: write files, plan docs, mod CHANGELOGs, shared tree."""
    locks = [norm_path(f) for f in wi.files] + doc_paths(wi) + mod_changelogs(wi) + [norm_path(o) for o in wi.out_of_repo]
    if shares_common(wi):
        locks.append(SHARED_DIR + "/**")
    return list(dict.fromkeys(locks))


def locksets_conflict(a, b):
    """First (x, y) pair of overlapping locks between two locksets, else None."""
    for x in a:
        for y in b:
            if paths_overlap(x, y):
                return x, y
    return None


# ---------------------------------------------------------------- eligibility
@dataclass
class Verdict:
    ok: bool
    reason: str = ""


def eligibility(wi, items, done, live_claims, dirty_paths, parked=None):
    """Is `wi` claimable now? `done`: set of done WI ids. `live_claims`: {wi_id: lockset list}. `dirty_paths`: set of
    repo-relative dirty paths. Predicates are checked in plan order (state, after, claim, lockset, dirty)."""
    if wi.id in done:
        return Verdict(False, "already done")
    if parked and wi.id in parked:
        return Verdict(False, f"parked: {parked[wi.id]}"[:120])
    if wi.state not in RUNNABLE_STATES:
        why = "needs the user" if wi.state == "awaiting_user" else "hold"
        return Verdict(False, f"{why}: {wi.blockers[:100]}" if wi.blockers else why)
    if not wi.has_file:
        return Verdict(False, "WI file missing")
    if wi.external and wi.state == "ready":
        return Verdict(False, "edits out-of-repo files (cannot be committed or scope-checked in v1)")
    for dep in wi.after:
        if dep not in items:
            return Verdict(False, f"after {dep}: unknown WI")
        if dep not in done:
            return Verdict(False, f"waits for {dep} (state {items[dep].state})")
    if wi.id in live_claims:
        return Verdict(False, "claimed by another session")
    mine = lockset(wi)
    for other, locks in live_claims.items():
        hit = locksets_conflict(mine, locks)
        if hit:
            return Verdict(False, f"lock {hit[0]} overlaps {other} ({hit[1]})")
    touched = [f for f in wi.files if _is_dirty(f, dirty_paths)]
    if touched:
        return Verdict(False, f"dirty by another session: {touched[0]}")
    return Verdict(True)


def _is_dirty(path, dirty_paths):
    p = norm_path(path)
    return any(d == p or d.startswith(p + "/") or fnmatch.fnmatch(d, p) for d in dirty_paths)


def pick_next(items, done, live_claims, dirty_paths, want=None, parked=None, exclude=None):
    """(wi, skipped) where skipped is [(wi_id, reason)] for every row passed over before the pick, in rank order.
    With `want`, only that WI is tried and its failed predicate is the single skipped entry."""
    skipped = []
    pool = [items[want]] if want and want in items else ([] if want else list(items.values()))
    if want and want not in items:
        return None, [(want, "unknown WI id")]
    for wi in pool:
        if exclude and wi.id in exclude:
            skipped.append((wi.id, exclude[wi.id]))
            continue
        v = eligibility(wi, items, done, live_claims, dirty_paths, parked)
        if v.ok:
            return wi, skipped
        skipped.append((wi.id, v.reason))
    return None, skipped


# ---------------------------------------------------------------- verify mapping
def mod_build_dirs(root, mod, files):
    """Dirs (relative to patch_manager_mods/) that build_check can build: the nearest ancestor holding exports.def for each file."""
    out = []
    for f in files:
        if not f.startswith(mod + "/"):
            continue
        d = Path(f).parent
        while str(d).startswith(mod):
            if (Path(root) / d / "exports.def").is_file():
                rel = d.as_posix().split("/", 1)[1]
                if rel not in out:
                    out.append(rel)
                break
            d = d.parent
    return out


def mod_test_stems(root, mod):
    """Test item stems under a mod dir: *_test.cpp and test_*.py files, skipping testdata/fixtures/retired."""
    base = Path(root) / mod
    skip = {"testdata", "fixtures", "retired", "__pycache__"}
    stems = []
    for p in sorted(base.rglob("*")):
        if not p.is_file() or skip & set(p.relative_to(base).parts):
            continue
        if p.name.endswith("_test.cpp") or (p.name.startswith("test_") and p.suffix == ".py"):
            stems.append(p.stem)
    return stems


def derive_gate(wi, root=REPO):
    """Gate commands (argv lists, no shell) from the WI's `verify:` lines, else derived from its write files.

    Returns (commands, reason). Empty commands with a reason means no gate could be derived; a declared `verify: none`
    returns ([], reason) with reason starting 'none:'."""
    root = Path(root)
    if wi.verify:
        return [v for v in wi.verify], "declared"
    if wi.no_gate:
        return [], f"none: {wi.no_gate}"
    cmds = []
    mods = list(dict.fromkeys(m.group(1) for f in wi.files if (m := MOD_SRC_RE.match(f))))
    for mod in mods:
        for bd in mod_build_dirs(root, mod, wi.files):
            cmds.append(f"python3 scripts/build_check.py --mod {bd}")
        stems = mod_test_stems(root, mod)
        if stems:
            cmds.append("python3 scripts/run_tests.py " + " ".join(stems))
    for f in wi.files:
        m = re.fullmatch(r"scripts/([A-Za-z0-9_]+)\.py", f)
        if m and not m.group(1).startswith("test_") and (root / "scripts" / f"test_{m.group(1)}.py").is_file():
            cmds.append(f"python3 scripts/run_tests.py test_{m.group(1)}")
    if cmds:
        return cmds, "derived"
    return [], "no gate derivable: add a `verify:` header line (or `verify: none <reason>`)"


# ---------------------------------------------------------------- gate policy
GATE_PROGRAMS = {"python3", "rg", "bash", "test", "sed", "wc", "stat"}
APPLY_ABBREV_RE = re.compile(r"^--ap(p(l(y)?)?)?(=|$)")  # argparse accepts unique prefixes of --apply
FILTER_WORD_RE = re.compile(r"[\w:.\-]+")


def gate_argv(cmd):
    """shlex-split and expand a gate command, as run_gate_cmd runs it. Raises ValueError when unparseable."""
    return [os.path.expandvars(os.path.expanduser(t)) for t in shlex.split(cmd)]


def gate_rejection(argv):
    """Policy check for one gate argv. Returns the rejection detail, or None when the gate may run.

    Rejects programs outside GATE_PROGRAMS, any --apply (or argparse abbreviation) and any token naming kpm_hotswap.
    Exception: for `python3 <...run_tests.py> words...` the plain positional filter words before the first dash token
    are test-name filters, not scripts, so `run_tests.py test_kpm_hotswap` may run."""
    if not argv or argv[0] not in GATE_PROGRAMS:
        return f"gate program not allowed: {argv[:1]}"
    exempt = set()
    if len(argv) >= 2 and argv[0] == "python3" and argv[1].endswith("run_tests.py"):
        for i in range(2, len(argv)):
            if argv[i].startswith("-") or not FILTER_WORD_RE.fullmatch(argv[i]):
                break  # options and their values (e.g. -k kpm_hotswap) stay checked
            exempt.add(i)
    for i, t in enumerate(argv):
        if APPLY_ABBREV_RE.match(t) or ("kpm_hotswap" in t and i not in exempt):
            return "gate may not install or hot-swap"
    return None


def gate_cmd_rejection(cmd):
    """gate_rejection for a command string; an unparseable command is rejected too."""
    try:
        argv = gate_argv(cmd)
    except ValueError as e:
        return f"unparseable gate: {e}"
    return gate_rejection(argv)


def gate_preflight(wi, root=REPO):
    """[(cmd, detail)] for every declared or derived gate command of `wi` that the gate policy would reject."""
    cmds, _ = derive_gate(wi, root)
    return [(c, d) for c in cmds if (d := gate_cmd_rejection(c))]


# ---------------------------------------------------------------- lint
def lint(items, root=REPO):
    """List of (level, wi_id, message). level is ERROR or WARN."""
    problems = []
    ids = set(items)
    for wi in items.values():
        if not wi.has_file:
            problems.append(("ERROR", wi.id, "index row has no WI file"))
            continue
        it, ht = wi.title.strip(), wi.header_title.strip()
        # index column truncates at 60 chars, so a strict prefix is allowed only for headers longer than 60
        if it != ht and not (len(ht) > 60 and ht.startswith(it)):
            problems.append(("WARN", wi.id, f"{wi.id}: index title differs from header title"))
        raw = wi.header_after if wi.header_after is not None else wi.index_after
        if wi.id in raw:
            problems.append(("ERROR", wi.id, "self-loop in after"))
        for dep in wi.after:
            if dep not in ids:
                problems.append(("ERROR", wi.id, f"after {dep} is not in the index"))
        for key in ("area", "size", "ready", "items", "files", "out-of-repo", "blockers", "docs"):
            if key not in wi.header_keys:
                problems.append(("ERROR", wi.id, f"missing header {key}:"))
        for m in re.finditer(r"\bafter:?\s+(WI-\d+[a-z]*(?:\s*(?:,|and|/)\s*WI-\d+[a-z]*)*)", wi.blockers):
            for dep in parse_wi_ids(m.group(1)):
                if dep != wi.id and dep not in wi.after:
                    problems.append(("WARN", wi.id, f"blockers prose says 'after {dep}' but the after list lacks it"))
        if wi.runnable and wi.external and wi.state == "ready":
            continue  # external-edit items are not claimable in v1 (see eligibility)
        if wi.runnable and not wi.verify and not wi.no_gate:
            cmds, why = derive_gate(wi, root)
            if not cmds:
                problems.append(("ERROR", wi.id, why))
        if wi.runnable:
            for c, detail in gate_preflight(wi, root):
                problems.append(("ERROR", wi.id, f"(gate rejected: {detail}) {c}"))
    return problems
