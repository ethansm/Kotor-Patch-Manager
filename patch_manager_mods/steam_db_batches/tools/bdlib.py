#!/usr/bin/env python3
"""bdlib.py - shared library of the Steam address-DB batch pipeline (contract: steam_db_batches/DESIGN.md).

Contents
  * paths (DESIGN section 0, every path overridable by a BD_* environment variable, read at call time);
  * TSV / DB loaders (all databases are opened read-only);
  * trust tiers (section 1): UNAUDITED_SOURCE_RES, is_trusted(), TrustIndex;
  * CRT_ADDRS and the library-function test used by gate G1;
  * Corpus: everything loaded once (Steam dump, GOG list, call graph, strings, name_map, branch DB, B1 verified,
    server fingerprints when present);
  * Scorer: the anchor families A-STR / A-CALLEE / A-SIG / A-SRV / A-VT with the decoy-discrimination rule
    (section 4.2).

Stdlib only.  Nothing in this module writes a file or a database.
"""
from __future__ import annotations

import bisect
import importlib.util
import os
import re
import sqlite3
import statistics
import struct
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, Iterator, Optional

# ---------------------------------------------------------------------------------------------- constants
PM_ROOT = Path(__file__).resolve().parents[2]            # .../patch_manager_mods
STEAM_HASH_PREFIX = "6a522e71"                           # kotor2_steam_aspyr in knowledge-DB binaries / class_methods
HUB_INDEGREE = 40                                        # callee/caller anchors ignore Steam in-degree above this
CRT_ADDRS = {0x9196FD, 0x91D288, 0x9196EE, 0x9215E6, 0x919723, 0x91C6AA}
UNAUDITED_SOURCE_RES = [                                 # case-sensitive, section 1
    re.compile(r"^batch[2-8]_"),
    re.compile(r"^gog_.*_(cluster|block)$"),
    re.compile(r"^gog_cluster_delta_map\.py$"),
]
KNOWN_CONFIDENCES = {"high", "medium", "med", "low", "wrong"}
# name_map sources trusted ONLY through the B1 gold route (b1_verified_head.tsv): Gemini's unreviewed "Option A"
# edits (2026-10-02) were written under this source label for 13 rows Batch 1 had rejected/left unresolved.
B1_ONLY_SOURCES = {"steam-db-pr3-phase2"}
GOG_PROGRAM_DEFAULT = "k2_win_gog_aspyr_swkotor2.exe"
MAC_PROGRAM_DEFAULT = "k2_mac_KOTOR2_x86"
STEAM_PROGRAM_DEFAULT = "k2_win_steam_aspyr_swkotor2.exe"
STEAM_FLAT_FUNCTIONS = "swkotor2_aspyr.exe_functions.tsv"   # flat server export that exists before the per-program dir


# ---------------------------------------------------------------------------------------------- paths
@dataclass(frozen=True)
class Paths:
    """Resolved locations (DESIGN section 0)."""
    dump: Path
    server: Path
    kdb: Path
    branchdb: Path
    gogdb: Path
    batches: Path
    b1_verified: Path
    exe: Path
    gog_program: str
    mac_program: str
    steam_program: str


def _env_path(name: str, default: str | Path) -> Path:
    return Path(os.path.expanduser(os.environ.get(name) or str(default))).resolve()


def get_paths() -> Paths:
    """Read the BD_* environment now (so tests and the smoke run can redirect everything)."""
    games = "~/Games/KOTOR2_Mod_Workspace"
    batches = _env_path("BD_ROOT", PM_ROOT / "steam_db_batches")
    return Paths(
        dump=_env_path("BD_DUMP", games + "/ghidra_dumps/steam_6a522e71/"),
        server=_env_path("BD_SERVER", games + "/ghidra_dumps/upstream_server/"),
        kdb=_env_path("BD_KDB", PM_ROOT / "ghidra_knowledge/kotor2_ghidra_knowledge.db"),
        branchdb=_env_path("BD_BRANCHDB", games + "/tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db"),
        gogdb=_env_path("BD_GOGDB", games + "/tools/kpm-pr-steam-db/AddressDatabases/kotor2_gog_aspyr.db"),
        batches=batches,
        # Lead decision: the committed, Opus-reviewed Batch 1 file, NOT the working copy steam_db_pr3/verified.tsv.
        b1_verified=_env_path("BD_B1_VERIFIED", PM_ROOT / "steam_db_batches/gold/b1_verified_head.tsv"),
        exe=_env_path("BD_EXE", games + "/ghidra_dumps/steam_6a522e71/swkotor2_vanilla_6a522e71.exe"),
        gog_program=os.environ.get("BD_GOG_PROGRAM") or GOG_PROGRAM_DEFAULT,
        mac_program=os.environ.get("BD_MAC_PROGRAM") or MAC_PROGRAM_DEFAULT,
        steam_program=os.environ.get("BD_STEAM_PROGRAM") or STEAM_PROGRAM_DEFAULT,
    )


def tool_path(env_name: str, default_rel: str) -> Path:
    """Location of a reused tool script (env override, else relative to patch_manager_mods)."""
    return _env_path(env_name, PM_ROOT / default_rel)


def load_re_card_prep():
    """Import ghidra_knowledge/tools/re_card_prep.py by path (reused, never copied)."""
    path = tool_path("BD_RE_CARD_PREP", "ghidra_knowledge/tools/re_card_prep.py")
    spec = importlib.util.spec_from_file_location("re_card_prep", str(path))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


# ---------------------------------------------------------------------------------------------- small helpers
def parse_addr(s) -> int:
    """'0x0040d640' / '40d640' / int -> int."""
    if isinstance(s, int):
        return s
    t = str(s).strip().lower()
    neg = t.startswith("-")
    t = t.lstrip("-")
    if t.startswith("0x"):
        t = t[2:]
    v = int(t, 16)
    return -v if neg else v


def hx8(a: int) -> str:
    """int -> '0x0040d640'."""
    return "0x%08x" % a


def base_name(name: str) -> str:
    """'CFoo::Bar (note)' / 'Baz(name)' -> 'CFoo::Bar' / 'Baz' (names in name_map may carry suffixes)."""
    return re.split(r"[ (]", (name or "").strip(), maxsplit=1)[0]


def norm_conf(c: str | None) -> str:
    """Normalise a name_map confidence; anything outside the known set is a swapped evidence column -> medium."""
    c = (c or "").strip().lower()
    if c == "med":
        return "medium"
    return c if c in KNOWN_CONFIDENCES else "medium"


def iter_tsv(path: Path) -> Iterator[dict]:
    """Yield one dict per data row of a tab-separated file (no quoting; missing file -> nothing)."""
    try:
        f = open(path, encoding="utf-8", errors="replace", newline="")
    except OSError:
        return
    with f:
        header = f.readline().rstrip("\r\n").split("\t")
        n = len(header)
        for line in f:
            line = line.rstrip("\r\n")
            if not line:
                continue
            parts = line.split("\t")
            if len(parts) < n:
                parts += [""] * (n - len(parts))
            yield dict(zip(header, parts))


def connect_ro(path: Path) -> sqlite3.Connection:
    """Open a SQLite DB read-only."""
    return sqlite3.connect("%s?mode=ro" % Path(path).resolve().as_uri(), uri=True)


def _to_int(s, default=None):
    try:
        return int(str(s).strip())
    except (TypeError, ValueError):
        return default


def _hexlist(cell: str) -> list[int]:
    out = []
    for tok in (cell or "").split(","):
        tok = tok.strip()
        if not tok:
            continue
        try:
            out.append(int(tok, 16))
        except ValueError:
            continue
    return out


def is_unaudited_source(source: str | None) -> bool:
    """True for name_map sources that were never audited (section 1)."""
    s = source or ""
    return any(rx.search(s) for rx in UNAUDITED_SOURCE_RES)


def is_trusted(source: str | None, confidence: str | None, *, b1_verified: bool = False,
               later_batch_verified: bool = False) -> bool:
    """Section 1: the single trust test.

    Trusted iff the row is B1-verified, or later-batch-verified, or it is a name_map row with confidence `high`
    whose source is not on the unaudited list.  Branch-DB rows have no route of their own.
    """
    if b1_verified or later_batch_verified:
        return True
    if (source or "") in B1_ONLY_SOURCES:
        return False
    return norm_conf(confidence) == "high" and not is_unaudited_source(source)


def is_library_name(name: str, source: str) -> bool:
    """Ghidra ANALYSIS-sourced function with a real-looking (non FUN_/LAB_) name = library/CRT code."""
    if source != "ANALYSIS":
        return False
    return bool(name) and not re.match(r"^(FUN_|thunk_FUN_|LAB_|DAT_)", name)


def is_soft_autolabel(name: str) -> bool:
    """Library-looking Ghidra names that may still sit on genuine game functions: REVIEW, not REJECT.

    Calibration (A3): 6 of 428 verified positives carry FID names such as `Statistics`, `DoHillClimbing`,
    `GetMargins`, `GetRect`; only CRT-style underscore names and SEH funclets are treated as hard library hits.
    """
    return not re.match(r"^(_|Catch_All@|Unwind@)", name)


# ---------------------------------------------------------------------------------------------- reviewer scoring
def effective_verdict(row: dict | None, card_steam: int) -> str:
    """Reviewer row -> match | label_suspect | mismatch | unsure | missing, judged against the card's candidate.

    A `label_suspect` (or `mismatch`) whose corrected_addr differs from the card's candidate REJECTED that candidate:
    it counts as `mismatch` (for a decoy card: correctly rejected).
    """
    if row is None:
        return "missing"
    v = row.get("verdict", "missing")
    corrected = (row.get("corrected_addr") or "").strip()
    if v in ("label_suspect", "mismatch") and corrected:
        try:
            if parse_addr(corrected) != card_steam:
                return "mismatch"
        except ValueError:
            pass
    return v if v in ("match", "label_suspect", "mismatch", "unsure", "missing") else "unsure"


def reviewer_accepts(row: dict | None, card_steam: int) -> bool:
    """True only for `match`, or `label_suspect` that names no different corrected address (counterpart accepted)."""
    return effective_verdict(row, card_steam) in ("match", "label_suspect")


def chunk_key(c) -> tuple:
    s = str(c)
    return (0, int(s)) if s.isdigit() else (1, s)


def decoy_gate(chunks: Iterable, reviewed: set, decoys: dict, decoys_min: int = 10, partial: bool = False) -> dict:
    """DESIGN 4.6 decision over decoy cards.

    chunks: every chunk id of the batch; reviewed: chunk ids with reviewer rows; decoys: {chunk: [accepted bool, ...]}
    for the reviewed chunks.  Any accepted decoy escalates; an unreviewed chunk (unless `partial`) or a reviewed chunk
    with fewer than `decoys_min` decoys is INSUFFICIENT.
    """
    chunks = sorted({str(c) for c in chunks}, key=chunk_key)
    unreviewed = [c for c in chunks if c not in reviewed]
    accepted = sum(sum(1 for a in v if a) for c, v in decoys.items() if c in reviewed)
    short = [c for c in chunks if c in reviewed and len(decoys.get(c, [])) < decoys_min]
    n_dec = sum(len(v) for c, v in decoys.items() if c in reviewed)
    if accepted:
        text = "ESCALATE: %d decoy card(s) accepted -> 100%% of this batch's reviewer matches go to Opus" % accepted
    elif unreviewed and not partial:
        text = "INSUFFICIENT: chunk(s) %s not reviewed yet (use --partial to decide on the reviewed chunks only)" % ", ".join(unreviewed)
    elif not (set(chunks) & reviewed) or short:
        text = "INSUFFICIENT DECOYS (chunks %s; need >= %d per chunk)" % (
            ", ".join(short) if short else "none reviewed", decoys_min)
    else:
        text = "OK%s: 0 of %d decoy cards accepted" % (" (partial: unreviewed chunks %s ignored)" % ", ".join(unreviewed)
                                                         if unreviewed else "", n_dec)
    return {"decision": text, "accepted": accepted, "short": short, "unreviewed": unreviewed, "decoy_cards": n_dec}


# ---------------------------------------------------------------------------------------------- dump records
@dataclass
class SteamFn:
    addr: int
    size: int
    retn: int
    name: str
    ns: str
    source: str
    thunk: bool


@dataclass
class GogFn:
    addr: int
    cls: str
    func: str
    callconv: str
    param: Optional[int]
    notes: str

    @property
    def name(self) -> str:
        return "%s::%s" % (self.cls, self.func)


@dataclass
class NameRow:
    addr: int
    name: str
    source: str
    conf: str          # normalised
    evidence: str
    date: str


@dataclass
class BranchFn:
    cls: str
    func: str
    addr: int
    cc: str
    param: Optional[int]
    notes: str

    @property
    def name(self) -> str:
        return "%s::%s" % (self.cls, self.func)


@dataclass
class Fp:
    """One fp.tsv row (DESIGN section 2)."""
    addr: int
    size: int
    retn: int
    n_insn: int
    callconv: str
    thunk_target: Optional[int]
    callees: frozenset
    str_refs: frozenset
    disp: Counter
    imm: Counter
    name: str
    namespace: str
    source: str


def load_steam_functions(dump: Path) -> dict[int, SteamFn]:
    out = {}
    for r in iter_tsv(dump / "inputs" / "steam_functions.tsv"):
        a = parse_addr(r["addr"])
        out[a] = SteamFn(a, _to_int(r.get("size"), 0), _to_int(r.get("retn"), -1), r.get("name", ""),
                         r.get("namespace", ""), r.get("source", ""), r.get("thunk", "") == "true")
    return out


def load_gog_functions(dump: Path) -> list[GogFn]:
    rows = []
    for r in iter_tsv(dump / "inputs" / "gog_functions.tsv"):
        rows.append(GogFn(parse_addr(r["gog_addr"]), r["class"], r["function"], (r.get("callconv") or "").strip(),
                          _to_int(r.get("param_size_bytes")), r.get("notes", "")))
    rows.sort(key=lambda g: g.addr)
    return rows


class CallGraph:
    """Steam call graph from callgraph.tsv: CALL and TAILJMP edges between function entries."""

    def __init__(self, path: Path):
        self.callees: dict[int, set[int]] = defaultdict(set)
        self.callers: dict[int, set[int]] = defaultdict(set)
        for r in iter_tsv(path):
            if r.get("type") not in ("CALL", "TAILJMP"):
                continue
            try:
                a, b = int(r["caller_fn"], 16), int(r["callee_fn"], 16)
            except (ValueError, KeyError):
                continue
            self.callees[a].add(b)
            self.callers[b].add(a)

    def indegree(self, addr: int) -> int:
        return len(self.callers.get(addr, ()))


class SteamStrings:
    """DUMP/strings.tsv: text, reference count and referring functions per string address."""

    def __init__(self, path: Path):
        self.text: dict[int, str] = {}
        self.refcount: dict[int, int] = {}
        self.by_func: dict[int, set[int]] = defaultdict(set)
        for r in iter_tsv(path):
            try:
                a = int(r["addr"], 16)
            except (ValueError, KeyError):
                continue
            self.text[a] = r.get("string", "")
            self.refcount[a] = _to_int(r.get("ref_count"), 0)
            for tok in (r.get("functions") or "").replace(",", ";").split(";"):
                head = tok.split(":", 1)[0].strip()
                if re.fullmatch(r"[0-9a-fA-F]{1,8}", head):
                    self.by_func[int(head, 16)].add(a)


def load_name_map(kdb: Path) -> dict[int, NameRow]:
    out = {}
    if not Path(kdb).exists():
        return out
    con = connect_ro(kdb)
    try:
        for a, n, s, c, e, d in con.execute("select steam_addr,name,source,confidence,evidence,date from name_map"):
            try:
                addr = parse_addr(a)
            except ValueError:
                continue
            out[addr] = NameRow(addr, n or "", s or "", norm_conf(c), e or "", d or "")
    finally:
        con.close()
    return out


def load_class_methods(kdb: Path) -> tuple[dict, dict]:
    """-> (steam_by_addr, gog_by_addr): addr -> [(class, vtable_index, symbol)] from the knowledge DB class_methods."""
    steam, gog = defaultdict(list), defaultdict(list)
    if not Path(kdb).exists():
        return steam, gog
    con = connect_ro(kdb)
    try:
        labels = dict(con.execute("select hash,label from binaries"))
        for h, cls, idx, addr, sym in con.execute(
                "select binary_hash,class_name,vtable_index,address,symbol from class_methods"):
            target = steam if h.startswith(STEAM_HASH_PREFIX) else gog if "gog" in labels.get(h, "") else None
            if target is None:
                continue
            try:
                target[parse_addr(addr)].append((cls, idx, sym or ""))
            except ValueError:
                continue
    except sqlite3.OperationalError:
        pass
    finally:
        con.close()
    return steam, gog


def load_branch_functions(path: Path) -> list[BranchFn]:
    """functions table of the KPM branch DB (class_name, function_name, address INTEGER, ...)."""
    if not Path(path).exists():
        return []
    con = connect_ro(path)
    try:
        rows = con.execute("select class_name,function_name,address,calling_convention,param_size_bytes,notes "
                           "from functions order by address").fetchall()
    except sqlite3.OperationalError:
        rows = []
    finally:
        con.close()
    return [BranchFn(c, f, int(a), cc or "", p, n or "") for c, f, a, cc, p, n in rows]


def load_gog_db_functions(path: Path) -> list[BranchFn]:
    """functions table of the KPM GOG DB (same schema as the branch DB); used for label cross-checks."""
    return load_branch_functions(path)


def load_verified_rows(path: Path, verdict_col: str = "verdict") -> list[dict]:
    """Rows of a verified.tsv / final_rows.tsv whose verdict column is `verified` (kind F only)."""
    out = []
    for r in iter_tsv(path):
        if r.get("kind", "F") != "F" or r.get(verdict_col) != "verified":
            continue
        if not r.get("steam_addr"):
            continue
        out.append(r)
    return out


def load_later_batches(batches: Path, exclude: str | None) -> list[dict]:
    """Verified rows of earlier steam_db_batches/B<n>/final_rows.tsv (the batch being worked on is excluded)."""
    rows = []
    for p in sorted(Path(batches).glob("B*/final_rows.tsv")):
        if exclude and p.parent.name == exclude:
            continue
        rows.extend(load_verified_rows(p, "final_verdict"))
    return rows


# ---------------------------------------------------------------------------------------------- exe image
class PeImage:
    """Minimal PE32 reader: maps a VA to file bytes through the section table (no hard-coded section deltas)."""

    def __init__(self, path: Path):
        self.data = Path(path).read_bytes()
        d = self.data
        pe = struct.unpack_from("<I", d, 0x3C)[0]
        nsec = struct.unpack_from("<H", d, pe + 6)[0]
        opt_size = struct.unpack_from("<H", d, pe + 20)[0]
        self.base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
        table = pe + 24 + opt_size
        self.sections = []              # (name, start VA, span, raw pointer, raw size)
        for i in range(nsec):
            name, vsize, rva, rsize, rptr = struct.unpack_from("<8sIIII", d, table + 40 * i)
            self.sections.append((name.rstrip(b"\0").decode("latin1"), self.base + rva, max(vsize, rsize), rptr, rsize))

    def offset(self, va: int) -> Optional[int]:
        for _n, start, span, rptr, rsize in self.sections:
            if start <= va < start + span and va - start < rsize:
                return rptr + va - start
        return None

    def read(self, va: int, n: int) -> Optional[bytes]:
        off = self.offset(va)
        return None if off is None else self.data[off:off + n]


# ---------------------------------------------------------------------------------------------- trust
class TrustIndex:
    """Which (address, name) pairs may be used as anchors / gold / confirmed neighbours (section 1)."""

    def __init__(self, name_map: dict[int, NameRow], b1_rows: list[dict], later_rows: list[dict],
                 untrust: Iterable[int] = ()):
        self.name_map = name_map
        self.untrusted = set(untrust)      # addresses removed from every trusted route (hold-out calibration)
        self.b1: dict[int, set[str]] = defaultdict(set)
        self.later: dict[int, set[str]] = defaultdict(set)
        for tgt, rows in ((self.b1, b1_rows), (self.later, later_rows)):
            for r in rows:
                cls = r.get("class") or r.get("class_name") or ""
                tgt[parse_addr(r["steam_addr"])].add("%s::%s" % (cls, r.get("name") or r.get("function_name") or ""))

    def trusted_names(self, addr: int, exact: bool = False) -> set[str]:
        """Names a trusted route attaches to this address.

        exact=True keeps only names that equal the row's full name (no `base_name` suffix stripping): the form that
        may seed `pairs` (DESIGN 4.2, Opus gate R1).
        """
        if addr in self.untrusted:
            return set()
        names = set(self.b1.get(addr, ())) | set(self.later.get(addr, ()))
        nm = self.name_map.get(addr)
        if nm and is_trusted(nm.source, nm.conf):
            if not exact or nm.name.strip() == base_name(nm.name):
                names.add(base_name(nm.name))
        return names

    def addrs_for_name(self, name: str) -> set[int]:
        """Every trusted Steam address that holds this (base) name."""
        if getattr(self, "_by_name", None) is None:
            idx: dict[str, set[int]] = defaultdict(set)
            for a, n in self.trusted_addrs():
                idx[n].add(a)
            self._by_name = idx
        return set(self._by_name.get(base_name(name), ()))

    def is_trusted(self, addr: int, name: str | None = None) -> bool:
        names = self.trusted_names(addr)
        return bool(names) if name is None else base_name(name) in names

    def trusted_addrs(self) -> Iterator[tuple[int, str]]:
        seen = set()
        for addr in (set(self.b1) | set(self.later) | set(self.name_map)) - self.untrusted:
            for n in self.trusted_names(addr):
                if (addr, n) not in seen:
                    seen.add((addr, n))
                    yield addr, n


# ---------------------------------------------------------------------------------------------- fingerprints
def load_fp(path: Path) -> dict[int, Fp]:
    out = {}
    for r in iter_tsv(path):
        try:
            a = parse_addr(r["addr"])
        except (ValueError, KeyError):
            continue
        tt = r.get("thunk_target", "").strip()
        out[a] = Fp(a, _to_int(r.get("size"), 0), _to_int(r.get("retn"), -1), _to_int(r.get("n_insn"), 0),
                    r.get("callconv", ""), parse_addr(tt) if tt else None,
                    frozenset(_hexlist(r.get("callees", ""))), frozenset(_hexlist(r.get("str_refs", ""))),
                    Counter(_hexlist(r.get("disp", ""))), Counter(_hexlist(r.get("imm", ""))),
                    r.get("name", ""), r.get("namespace", ""), r.get("source", ""))
    return out


def load_server_strings(path: Path) -> dict[int, str]:
    out = {}
    for r in iter_tsv(path):
        try:
            out[parse_addr(r["addr"])] = r.get("text", "")
        except (ValueError, KeyError):
            continue
    return out


def load_server_functions(directory: Path, flat_fallback: Path | None = None) -> dict[int, dict]:
    """addr -> row of `<directory>/*_functions.tsv` (header addr size retn name namespace callconv prototype source thunk)."""
    files = sorted(Path(directory).glob("*_functions.tsv")) if Path(directory).is_dir() else []
    if not files and flat_fallback is not None and Path(flat_fallback).exists():
        files = [Path(flat_fallback)]
    out = {}
    for f in files:
        for r in iter_tsv(f):
            try:
                out[parse_addr(r["addr"])] = r
            except (ValueError, KeyError):
                continue
    return out


# ---------------------------------------------------------------------------------------------- corpus
class Corpus:
    """Everything the pipeline needs, loaded once."""

    def __init__(self, paths: Paths | None = None, exclude_batch: str | None = None,
                 untrust: Iterable[int] = ()):
        self.paths = paths or get_paths()
        p = self.paths
        self.rp = load_re_card_prep()
        self.steam = load_steam_functions(p.dump)
        self.steam_starts = sorted(a for a, f in self.steam.items() if not f.name.startswith("Unwind@"))
        self.gog = load_gog_functions(p.dump)
        self.gog_index = {g.addr: i for i, g in enumerate(self.gog)}
        self.gog_by_addr = {g.addr: g for g in self.gog}
        self.gog_by_name = {g.name: g for g in self.gog}
        self.cg = CallGraph(p.dump / "callgraph.tsv")
        self.strings = SteamStrings(p.dump / "strings.tsv")
        self.name_map = load_name_map(p.kdb)
        self.names_by_base: dict[str, list[NameRow]] = defaultdict(list)
        for row in self.name_map.values():
            self.names_by_base[base_name(row.name)].append(row)
        self.cm_steam, self.cm_gog = load_class_methods(p.kdb)
        self.branch = load_branch_functions(p.branchdb)
        self.branch_by_addr: dict[int, list[BranchFn]] = defaultdict(list)
        self.branch_by_name: dict[str, BranchFn] = {}
        for b in self.branch:
            self.branch_by_addr[b.addr].append(b)
            self.branch_by_name[b.name] = b
        self.b1_rows = load_verified_rows(p.b1_verified)
        self.later_rows = load_later_batches(p.batches, exclude_batch)
        self.trust = TrustIndex(self.name_map, self.b1_rows, self.later_rows)
        # server exports (all optional)
        gdir, sdir = p.server / p.gog_program, p.server / p.steam_program
        self.gog_fp = load_fp(gdir / "fp.tsv")
        self.gog_texts = load_server_strings(gdir / "strings.tsv")
        self.steam_fp = load_fp(sdir / "fp.tsv")
        self.steam_srv_texts = load_server_strings(sdir / "strings.tsv")
        self.gog_srv = load_server_functions(gdir)
        self.mac_fp = load_fp(p.server / p.mac_program / "fp.tsv")
        self.mac_by_name: dict[str, list[Fp]] = defaultdict(list)
        for fp in self.mac_fp.values():
            if fp.name and fp.namespace:
                self.mac_by_name["%s::%s" % (fp.namespace, fp.name)].append(fp)
        self.steam_srv = load_server_functions(sdir, p.server / STEAM_FLAT_FUNCTIONS)
        self.has_gog_fp = bool(self.gog_fp)
        self.has_steam_fp = bool(self.steam_fp)
        self.pairs: dict[int, int] = {}
        self._pe: PeImage | None = None
        self._pe_loaded = False
        self._asm_cache: dict[int, object] = {}
        self._thunk_cache: dict[int, tuple[int, bool, bool]] = {}
        self._gog_callers: dict[int, set[int]] | None = None
        self._steam_fp_referrers: Counter | None = None
        self.apply_untrust(untrust)

    def apply_untrust(self, addrs: Iterable[int]) -> None:
        """Remove Steam addresses from the trust index and from `pairs` (both directions) for this run.

        `pairs` are the trusted GOG -> Steam mappings that may seed run_ok, serve as A-CALLEE anchors and count as
        gold positives (Opus gate R1): the GOG DB name equals the trusted Steam name EXACTLY and the GOG and Steam
        fingerprints agree on the body (`body_agrees`).  Trusted rows that fail stay in the trust index (G6 holders)
        and are listed in `pair_conflicts`.
        """
        self.untrusted = set(addrs)
        self.trust = TrustIndex(self.name_map, self.b1_rows, self.later_rows, self.untrusted)
        self.pairs = {}
        self.pair_conflicts: list[tuple[int, int, str, str]] = []
        for addr, name in sorted(self.trust.trusted_addrs()):
            g = self.gog_by_name.get(name)
            if g is None or g.addr in self.pairs or addr in self.untrusted:
                continue
            if name not in self.trust.trusted_names(addr, exact=True):
                self.pair_conflicts.append((g.addr, addr, name, "name is not exact (suffix on the trusted row)"))
                continue
            ok, why = self.body_agrees(g.addr, addr)
            if ok:
                self.pairs[g.addr] = addr
            else:
                self.pair_conflicts.append((g.addr, addr, name, why))

    def body_agrees(self, g_addr: int, s_addr: int) -> tuple[bool, str]:
        """Opus gate R1: equal retn, and equal size or A-SIG >= 0.5 with intersection >= 3 (thunks resolved).

        Missing fingerprints cannot contradict: -> (True, 'no fingerprint').  A retn of -1 (no RET instruction seen,
        e.g. a tail-jump body) is unknown and is not compared.
        """
        key = (g_addr, s_addr)
        cache = self.__dict__.setdefault("_body_cache", {})
        if key not in cache:
            cache[key] = self._body_agrees(g_addr, s_addr)
        return cache[key]

    def _body_agrees(self, g_addr: int, s_addr: int) -> tuple[bool, str]:
        gfp = self.gog_fp.get(self.resolve_gog(g_addr))
        sfp = self.steam_fp.get(self.resolve_thunk(s_addr)[0])
        if gfp is None or sfp is None:
            return True, "no fingerprint"
        if gfp.retn != sfp.retn and gfp.retn >= 0 and sfp.retn >= 0:      # -1 = no RET seen: unknown, not a mismatch
            return False, "retn GOG %d vs Steam %d" % (gfp.retn, sfp.retn)
        if gfp.size == sfp.size:
            return True, "equal size"
        jd, ji = jaccard(gfp.disp, sfp.disp), jaccard(gfp.imm, sfp.imm)
        inter = len(set(gfp.disp) & set(sfp.disp)) + len(set(gfp.imm) & set(sfp.imm))
        if (jd + ji) / 2 >= 0.5 and inter >= 3:
            return True, "SIG %.2f" % ((jd + ji) / 2)
        return False, "size GOG %d vs Steam %d, SIG %.2f (intersection %d)" % (gfp.size, sfp.size, (jd + ji) / 2, inter)

    def _class_order(self) -> dict[str, list[GogFn]]:
        if getattr(self, "_class_lists", None) is None:
            lists: dict[str, list[GogFn]] = defaultdict(list)
            for g in self.gog:
                lists[g.cls].append(g)
            self._class_lists = lists
        return self._class_lists

    def mac_fit(self, x: GogFn, m: Fp) -> Optional[float]:
        """Opus/G8 fit of a GOG function body against a Mac function (see DESIGN 4.2 G8).

        fit = 0.75 * overlap + 0.25 * closeness.  overlap = average of the two directions of the +-4 value overlap of
        V(x) (GOG fp) and V(m) (Mac fp); closeness = min(size)/max(size).  None when either side has < 2 values.
        """
        xfp = self.gog_fp.get(self.resolve_gog(x.addr))
        if xfp is None:
            return None
        vx, vm = sorted(self._values(xfp)), sorted(self._values(m))
        if len(vx) < 2 or len(vm) < 2:
            return None

        def share(a, b):
            hit = 0
            for v in a:
                i = bisect.bisect_left(b, v - 4)
                if i < len(b) and b[i] <= v + 4:
                    hit += 1
            return hit / len(a)

        overlap = (share(vx, vm) + share(vm, vx)) / 2
        closeness = (min(xfp.size, m.size) / max(xfp.size, m.size)) if xfp.size and m.size else 0.0
        return 0.75 * overlap + 0.25 * closeness

    def mac_rotation(self, g: GogFn) -> tuple[str, str]:
        """Invariant G8 `mac_rotation` (DESIGN 4.2): -> (state, detail), state pass | n/a | rotation | rotation_in.

        rotation: some other GOG function G' of the same class within +-4 positions fits a Mac function named like G
        better than G does (fit(G') > fit(G) + 0.15 and fit(G') >= 0.5; overloads of G, i.e. names equal up to a `_N`
        suffix, are not compared because one Mac symbol cannot tell them apart).  rotation_in: G's name has no Mac symbol but G's
        body fits the Mac function named like a neighbour G' better than G''s own body does (same thresholds).
        """
        if os.environ.get("BD_DISABLE_G8"):          # study switch: baseline without G8
            return "n/a", ""
        cache = self.__dict__.setdefault("_rot_cache", {})
        if g.addr in cache:
            return cache[g.addr]
        order = self._class_order()[g.cls]
        i = order.index(g)
        stem = lambda f: re.sub(r"_\d+$", "", f.func)      # noqa: E731  overloads (X, X_2) share one Mac symbol
        near = [x for x in order[max(0, i - 4):i + 5] if x is not g and stem(x) != stem(g)]
        macs = self.mac_by_name.get(g.name, [])
        res = ("n/a", "")
        if macs:
            fits = [self.mac_fit(g, m) for m in macs]
            own = max([f for f in fits if f is not None], default=None)
            if own is not None:
                best = None
                for x in near:
                    f = max([v for v in (self.mac_fit(x, m) for m in macs) if v is not None], default=None)
                    if f is not None and f > own + 0.15 and f >= 0.5 and (best is None or f > best[0]):
                        best = (f, x)
                res = ("rotation", "%s:%.2f>%.2f" % (hx8(best[1].addr), best[0], own)) if best else ("pass", "")
        else:
            best = None
            for x in near:
                for m in self.mac_by_name.get(x.name, []):
                    own_x, mine = self.mac_fit(x, m), self.mac_fit(g, m)
                    if mine is not None and own_x is not None and mine > own_x + 0.15 and mine >= 0.5 \
                            and (best is None or mine > best[0]):
                        best = (mine, x, own_x)
            if best:
                res = ("rotation_in", "%s:%.2f>%.2f" % (hx8(best[1].addr), best[0], best[2]))
        cache[g.addr] = res
        return res

    @staticmethod
    def _values(fp: Fp) -> set[int]:
        return {v for v in list(fp.disp) + list(fp.imm) if 0x10 <= v < 0x100000}

    def mac_check(self, g: GogFn, s_addr: int) -> tuple[str, Optional[float]]:
        """Opus gate R3 / invariant G7: -> ('pass' | 'n/a' | 'mac_contra', best overlap or None).

        V(f) = disp and imm values in [0x10, 0x100000).  overlap = share of V(S) with a Mac value within 4.  Flag
        `mac_contra` only if S has >= 2 values and EVERY Mac function named exactly Class::Name has >= 2 values and
        overlap 0.  No Mac function of that name, or too few values, is `n/a`.
        """
        macs = self.mac_by_name.get(g.name)
        sfp = self.steam_fp.get(s_addr)
        if not macs or sfp is None:
            return "n/a", None
        vs = self._values(sfp)
        if len(vs) < 2:
            return "n/a", None
        overlaps = []
        for m in macs:
            vm = sorted(self._values(m))
            if len(vm) < 2:
                return "n/a", None
            hit = 0
            for x in vs:
                i = bisect.bisect_left(vm, x - 4)
                if i < len(vm) and vm[i] <= x + 4:
                    hit += 1
            overlaps.append(hit / len(vs))
        best = max(overlaps)
        return ("mac_contra", 0.0) if best == 0 else ("pass", best)

    # ---- exe bytes (G3 fallback when no .asm exists)
    def exe_bytes(self, addr: int, n: int = 16) -> Optional[bytes]:
        if not self._pe_loaded:
            self._pe_loaded = True
            try:
                self._pe = PeImage(self.paths.exe) if Path(self.paths.exe).is_file() else None
            except (OSError, struct.error):
                self._pe = None
        return self._pe.read(addr, n) if self._pe is not None else None

    # ---- asm / thunks
    def asm(self, addr: int):
        if addr not in self._asm_cache:
            self._asm_cache[addr] = self.rp.read_asm(str(self.paths.dump), addr)
        return self._asm_cache[addr]

    def _raw_thunk_target(self, addr: int) -> Optional[int]:
        """Direct thunk target of one Steam function: int, 0 = thunk with unknown target, None = not a thunk."""
        fp = self.steam_fp.get(addr)
        if fp is not None and fp.thunk_target:
            return fp.thunk_target
        asm = self.asm(addr)
        if asm:
            t = self.rp.asm_thunk_target(asm)
            if t is not None:
                return t if t in self.steam else 0
        f = self.steam.get(addr)
        if f is not None and f.thunk:
            return 0
        return None

    def resolve_thunk(self, addr: int, max_depth: int = 8) -> tuple[int, bool, bool]:
        """-> (final address, was_thunk, unresolved).  A thunk chain is followed to its last non-thunk."""
        if addr in self._thunk_cache:
            return self._thunk_cache[addr]
        cur, was, unresolved = addr, False, False
        for _ in range(max_depth):
            t = self._raw_thunk_target(cur)
            if t is None:
                break
            was = True
            if t == 0 or t == cur:
                unresolved = True
                break
            cur = t
        res = (cur, was, unresolved)
        self._thunk_cache[addr] = res
        return res

    def resolve_gog(self, addr: int, max_depth: int = 8) -> int:
        """Follow GOG thunks through the GOG fingerprints."""
        cur = addr
        for _ in range(max_depth):
            fp = self.gog_fp.get(cur)
            if fp is None or not fp.thunk_target or fp.thunk_target == cur:
                break
            cur = fp.thunk_target
        return cur

    # ---- GOG side metadata
    def gog_retn(self, addr: int) -> Optional[int]:
        """GOG retn: fingerprint (thunk-resolved) first, else the GOG server function export; None if unknown."""
        fp = self.gog_fp.get(self.resolve_gog(addr))
        if fp is not None and fp.retn >= 0:
            return fp.retn
        row = self.gog_srv.get(addr)
        if row is not None:
            v = _to_int(row.get("retn"))
            if v is not None and v >= 0:
                return v
        return None

    def gog_size(self, addr: int) -> Optional[int]:
        fp = self.gog_fp.get(addr)
        if fp is not None:
            return fp.size
        row = self.gog_srv.get(addr)
        return _to_int(row.get("size")) if row else None

    def gog_callers(self) -> dict[int, set[int]]:
        if self._gog_callers is None:
            inv: dict[int, set[int]] = defaultdict(set)
            for a, fp in self.gog_fp.items():
                for c in fp.callees:
                    inv[c].add(a)
            self._gog_callers = inv
        return self._gog_callers

    # ---- Steam side strings
    def steam_str_refs(self, addr: int) -> set[int]:
        refs = set(self.strings.by_func.get(addr, ()))
        fp = self.steam_fp.get(addr)
        if fp is not None:
            refs |= fp.str_refs
        return refs

    def steam_text(self, a: int) -> str | None:
        t = self.strings.text.get(a)
        return t if t is not None else self.steam_srv_texts.get(a)

    def steam_refcount(self, a: int) -> int:
        if a in self.strings.refcount:
            return self.strings.refcount[a]
        if self._steam_fp_referrers is None:
            self._steam_fp_referrers = Counter(s for fp in self.steam_fp.values() for s in fp.str_refs)
        return self._steam_fp_referrers.get(a, 99)

    # ---- layout
    def steam_neighbours(self, addr: int) -> list[int]:
        """Previous and next Steam function starts (Unwind funclets excluded)."""
        i = bisect.bisect_left(self.steam_starts, addr)
        out = []
        if i > 0:
            out.append(self.steam_starts[i - 1])
        j = i + 1 if i < len(self.steam_starts) and self.steam_starts[i] == addr else i
        if j < len(self.steam_starts):
            out.append(self.steam_starts[j])
        return out

    def steam_window(self, lo: int, hi: int) -> list[int]:
        i = bisect.bisect_left(self.steam_starts, lo)
        j = bisect.bisect_right(self.steam_starts, hi)
        return self.steam_starts[i:j]

    def expected_ret(self, g: GogFn) -> Optional[int]:
        """Metadata expectation ceil(param/4)*4 (cdecl 0); None when the GOG param size is unknown."""
        if g.param is None:
            return None
        cc = g.callconv.lstrip("_").lower() or "thiscall"
        return self.rp.expected_ret(g.param, cc)

    def mapped_run_delta(self, class_name: str, mapper_rows: dict[int, tuple[int, str]]) -> Optional[int]:
        """Median delta of the class's trusted rows, else of its mapper `vote` rows."""
        deltas = [s - g for g, s in self.pairs.items() if self.gog_by_addr[g].cls == class_name]
        if not deltas:
            deltas = [s - g for g, (s, tag) in mapper_rows.items()
                      if tag == "vote" and self.gog_by_addr.get(g) and self.gog_by_addr[g].cls == class_name]
        return int(statistics.median_low(deltas)) if deltas else None


# ---------------------------------------------------------------------------------------------- scoring
FAMILIES = ("STR", "CALLEE", "SIG", "SRV", "VT")


@dataclass
class FamScore:
    """One anchor family measured for one (GOG function, Steam address) pair."""
    score: float
    ok: bool                      # meets the family's own threshold
    detail: str = ""
    extra: dict = field(default_factory=dict)


def jaccard(a: Iterable, b: Iterable) -> float:
    a, b = set(a), set(b)
    if not a and not b:
        return 0.0
    return len(a & b) / len(a | b)


class Scorer:
    """Anchor-family scores (DESIGN 4.2).  Every method returns None when the family is not computable (n/a)."""

    def __init__(self, corpus: Corpus):
        self.c = corpus

    # -- A-STR
    def a_str(self, g: GogFn, x: int) -> Optional[FamScore]:
        c = self.c
        gfp = c.gog_fp.get(c.resolve_gog(g.addr))
        if gfp is None or not c.gog_texts:
            return None
        gtexts = {t for a in gfp.str_refs if (t := c.gog_texts.get(a)) and len(t) >= 4}
        hits, seen = [], set()
        for a in sorted(c.steam_str_refs(x)):
            t = c.steam_text(a)
            if t and len(t) >= 4 and c.steam_refcount(a) <= 3 and t in gtexts and t not in seen:
                seen.add(t)
                hits.append((t, a))
        detail = "; ".join("'%s'@%08x" % (t[:40], a) for t, a in hits[:3])
        return FamScore(float(len(hits)), len(hits) >= 1, detail, {"hits": hits})

    # -- A-CALLEE
    def _expand(self, addrs: Iterable[int]) -> set[int]:
        """Add thunk targets of small callees so a call through a thunk matches a direct call."""
        out = set(addrs)
        for a in list(out):
            f = self.c.steam.get(a)
            if f is not None and f.size <= 16:
                tgt, was, _u = self.c.resolve_thunk(a)
                if was:
                    out.add(tgt)
        return out

    def a_callee(self, g: GogFn, x: int) -> Optional[FamScore]:
        c = self.c
        gaddr = c.resolve_gog(g.addr)
        gfp = c.gog_fp.get(gaddr)
        if gfp is None:
            return None
        s_callees = self._expand(c.cg.callees.get(x, set()) | (c.steam_fp[x].callees if x in c.steam_fp else set()))
        s_callers = self._expand(c.cg.callers.get(x, set()))
        g_callers = c.gog_callers().get(g.addr, set()) | c.gog_callers().get(gaddr, set())
        matched, contra = [], []
        for label, gset, sset in (("callee", gfp.callees, s_callees), ("caller", g_callers, s_callers)):
            for ga in sorted(gset):
                sa = c.pairs.get(ga) or c.pairs.get(c.resolve_gog(ga))
                if sa is None or c.cg.indegree(sa) > HUB_INDEGREE:
                    continue
                if sa in sset:
                    matched.append((label, sa))
                elif label == "callee":
                    contra.append(sa)
        parts = []
        for label in ("callee", "caller"):
            items = []
            for _lab, sa in [m for m in matched if m[0] == label][:3]:
                nm = c.name_map.get(sa)
                items.append("%s 0x%x" % (base_name(nm.name).split("::")[-1] if nm else "?", sa))
            if items:
                parts.append("%ss: %s" % (label, ", ".join(items)))
        detail = "%d (%s)" % (len(matched), "; ".join(parts)) if matched else "0"
        return FamScore(float(len(matched)), len(matched) >= 1, detail,
                        {"contradictions": len(contra), "matched": matched})

    # -- A-SIG
    def a_sig(self, g: GogFn, x: int) -> Optional[FamScore]:
        c = self.c
        gfp = c.gog_fp.get(c.resolve_gog(g.addr))
        sfp = c.steam_fp.get(x)
        if gfp is None or sfp is None:
            return None
        jd, ji = jaccard(gfp.disp, sfp.disp), jaccard(gfp.imm, sfp.imm)
        inter = len(set(gfp.disp) & set(sfp.disp)) + len(set(gfp.imm) & set(sfp.imm))
        mean = (jd + ji) / 2
        ok = mean >= 0.5 and inter >= 3
        return FamScore(round(mean, 3), ok, "%.2f" % mean, {"inter": inter})

    # -- A-SRV
    def a_srv(self, g: GogFn, x: int) -> FamScore:
        row = self.c.steam_srv.get(x)
        if row and row.get("source") in ("USER_DEFINED", "IMPORTED"):
            ns, nm = row.get("namespace", ""), row.get("name", "")
            full = "%s::%s" % (ns, nm) if ns else nm
            if full == g.name:
                return FamScore(1.0, True, "server %s" % full)
        return FamScore(0.0, False, "")

    # -- A-VT
    def a_vt(self, g: GogFn, x: int) -> FamScore:
        c = self.c
        steam_rows = c.cm_steam.get(x, [])
        gog_rows = c.cm_gog.get(g.addr, [])
        for cls, idx, sym in steam_rows:
            if cls == g.cls and (sym == g.func or sym == g.name):
                return FamScore(1.0, True, "class_methods %s::%s" % (cls, sym))
            for gcls, gidx, _gs in gog_rows:
                if gcls == cls and gidx is not None and gidx == idx:
                    return FamScore(1.0, True, "vtable slot %s[%s]" % (cls, idx))
        return FamScore(0.0, False, "")

    # -- combined
    def family_scores(self, g: GogFn, x: int) -> dict[str, Optional[FamScore]]:
        return {"STR": self.a_str(g, x), "CALLEE": self.a_callee(g, x), "SIG": self.a_sig(g, x),
                "SRV": self.a_srv(g, x), "VT": self.a_vt(g, x)}

    def total(self, g: GogFn, x: int) -> float:
        """Scalar fingerprint score used only to rank candidates and detect permuted pairs."""
        f = self.family_scores(g, x)
        t = 0.0
        if f["STR"]:
            t += f["STR"].score
        if f["CALLEE"]:
            t += f["CALLEE"].score
        if f["SIG"] and f["SIG"].ok:
            t += 4 * f["SIG"].score
        if f["SRV"].ok:
            t += 3
        if f["VT"].ok:
            t += 3
        return t

    def discriminating(self, g: GogFn, x: int, decoys: Iterable[int]) -> tuple[dict, dict, str]:
        """Family scores of x and of its decoys.

        -> (scores of x, {family: counts?}, best-decoy summary).  A family counts only if x meets the family's own
        threshold AND x's score is strictly greater than every decoy's score for that family.
        """
        mine = self.family_scores(g, x)
        dec = {d: self.family_scores(g, d) for d in decoys if d != x}
        counts = {}
        for fam in FAMILIES:
            m = mine[fam]
            if m is None or not m.ok:
                counts[fam] = False
                continue
            best_other = max((d[fam].score for d in dec.values() if d[fam] is not None), default=-1.0)
            counts[fam] = m.score > best_other
        best_decoy = ""
        if dec:
            bd = max(dec, key=lambda d: sum((s.score if s else 0) for s in dec[d].values()))
            parts = []
            for fam in FAMILIES:
                s = dec[bd][fam]
                parts.append("%s%s" % (fam, "n/a" if s is None else ("%g" % s.score)))
            best_decoy = "%s:%s" % (hx8(bd), "/".join(parts))
        return mine, counts, best_decoy
