#!/usr/bin/env python3
"""batch.py - Steam address-DB batch pipeline (contract: steam_db_batches/DESIGN.md section 4).

Subcommands (all deterministic; working dir per batch = $BD_ROOT/B<n>/):
  candidates B<n> --classes C1,C2 | --from-branch   -> candidates.tsv (+ mapper output under --findings-dir)
  gates      B<n> [--untrust FILE]                  -> evidence.tsv, gate_report.md
  cards      B<n> [--chunk 60] [--decoys N]         -> cards/chunk_<k>.md, cards/key.tsv
  finalize   B<n> [--with-likely] [--date D]        -> final_rows.tsv, functions.csv, kdb_upsert.json
                                                       (B0 also: branch_deletes.tsv, branch_corrections.csv)
  persist    B<n> [--apply]                         -> dry-run plan; --apply (lead only) writes the knowledge DB

Only `persist --apply` ever writes a database, and only the one named by BD_KDB.
"""
from __future__ import annotations

import argparse
import csv
import io
import json
import random
import re
import sqlite3
import subprocess
import sys
from collections import Counter, OrderedDict
from dataclasses import dataclass, field
from datetime import date as _date
from pathlib import Path
from typing import Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bdlib as bd  # noqa: E402
from bdlib import (FAMILIES, GogFn, base_name, hx8, iter_tsv, parse_addr)  # noqa: E402

EVIDENCE_COLS = [
    "kind", "class", "name", "gog", "steam_addr", "verdict", "calling_convention", "param_size_bytes", "evidence",
    "cand_orig", "cand_status", "content_level", "wrapper_issue", "steam_size", "gog_param_size",
    "batch", "cand_sources", "mapper_tag", "gog_size", "gog_retn", "steam_retn", "g1", "g2", "g3", "g4", "g5_target",
    "g6", "a_str", "a_callee", "a_sig", "a_srv", "a_vt", "families", "run_ok", "decoy_best", "gate_class",
    "review_ref", "flags",
]
CANDIDATE_COLS = ["class", "name", "gog", "gog_callconv", "gog_param", "gog_size", "gog_retn", "candidates"]
FUNCTIONS_CSV_HEADER = ["class_name", "function_name", "address", "calling_convention", "param_size_bytes", "notes"]
FINAL_VERDICTS = ("verified", "likely", "rejected", "unresolved")
NEAR_WINDOW = 0x400
RUN_WINDOW = 12                    # nearest-mapped-neighbour search depth (GOG functions per side)
SOURCE_ORDER_NOTE = "server > trusted name_map > mapper:vote > branch > seeded > other"


# ================================================================================================ small utilities
def die(msg: str) -> "None":
    print("ERROR: %s" % msg, file=sys.stderr)
    raise SystemExit(2)


def batch_name(s: str) -> str:
    if not re.fullmatch(r"B[A-Za-z0-9_]+", s):
        raise argparse.ArgumentTypeError("batch name must look like B1 / B0 / Bsmoke")
    return s


def write_tsv(path: Path, cols: list[str], rows: list[dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.write("\t".join(cols) + "\n")
        for r in rows:
            f.write("\t".join(clean(r.get(c, "")) for c in cols) + "\n")


def clean(v) -> str:
    return str(v if v is not None else "").replace("\t", " ").replace("\r", " ").replace("\n", " ")


def sanitize_source(s: str) -> str:
    """Candidate sources are joined with + | ; so those characters (and blanks) must not occur inside one."""
    return re.sub(r"[|+;\s]+", "_", s)[:48]


def parse_candidates(cell: str) -> list[tuple[int, list[str]]]:
    """'0x1|a+b;0x2|c' -> [(1, ['a','b']), (2, ['c'])]."""
    out = []
    for part in (cell or "").split(";"):
        part = part.strip()
        if not part:
            continue
        addr, _, srcs = part.partition("|")
        out.append((parse_addr(addr), [s for s in srcs.split("+") if s]))
    return out


def format_candidates(cands: "OrderedDict[int, list[str]]") -> str:
    return ";".join("%s|%s" % (hx8(a), "+".join(s)) for a, s in cands.items())


def today(args=None) -> str:
    d = getattr(args, "date", None) if args is not None else None
    return d or _date.today().isoformat()


def fmt_score(fs: Optional[bd.FamScore], kind: str = "num") -> str:
    if fs is None:
        return "n/a"
    return "%.2f" % fs.score if kind == "float" else "%g" % fs.score


# ================================================================================================ candidates
def maybe_reexport(paths: bd.Paths, no_reexport: bool) -> None:
    """Refresh DUMP/inputs/name_map.tsv when the knowledge DB is newer (section 4.1)."""
    nm = paths.dump / "inputs" / "name_map.tsv"
    stale = (not nm.exists()) or (paths.kdb.exists() and nm.stat().st_mtime < paths.kdb.stat().st_mtime)
    if not stale:
        return
    if no_reexport:
        print("note: DUMP/inputs/name_map.tsv is older than the KDB; --no-reexport given, leaving it alone")
        return
    exporter = bd.tool_path("BD_EXPORTER", "engine_ai_atlas/tools/export_dump_inputs.py")
    print("re-exporting dump inputs from the KDB ...")
    subprocess.run([sys.executable, str(exporter), "--root", str(paths.dump), "--db", str(paths.kdb)], check=True)


def run_mapper(patterns: list[str], out_file: Path) -> dict[int, tuple[int, str]]:
    """Run gog_cluster_delta_map.py as a subprocess; -> {gog_addr: (steam_addr, tag)}."""
    mapper = bd.tool_path("BD_MAPPER", "ghidra_knowledge/gog_cluster_delta_map.py")
    out_file.parent.mkdir(parents=True, exist_ok=True)
    res = subprocess.run([sys.executable, str(mapper), *patterns, "--out", str(out_file)],
                         capture_output=True, text=True)
    if res.returncode != 0:
        die("mapper failed (%d): %s" % (res.returncode, (res.stderr or res.stdout)[-400:]))
    out = {}
    try:
        text = out_file.read_text()
    except OSError:
        text = ""
    for line in text.splitlines():
        f = line.split("\t")
        if len(f) < 4:
            continue
        try:
            out[parse_addr(f[1])] = (parse_addr(f[0]), f[3])
        except ValueError:
            continue
    return out


def server_name_index(corpus: bd.Corpus) -> dict[str, list[int]]:
    """Trusted server Steam names (USER_DEFINED / IMPORTED) -> addresses."""
    idx: dict[str, list[int]] = {}
    for a, r in corpus.steam_srv.items():
        if r.get("source") in ("USER_DEFINED", "IMPORTED") and r.get("name"):
            ns = r.get("namespace", "")
            idx.setdefault("%s::%s" % (ns, r["name"]) if ns else r["name"], []).append(a)
    return idx


def build_candidates(corpus: bd.Corpus, scorer: bd.Scorer, g: GogFn, mapper: dict, run_delta: Optional[int],
                     srv_index: dict, from_branch: bool, near_k: int = 3) -> "OrderedDict[int, list[str]]":
    """All candidate Steam addresses of one GOG function with their source tags (section 4.1)."""
    cands: "OrderedDict[int, list[str]]" = OrderedDict()

    def add(addr: int, tag: str) -> None:
        tags = cands.setdefault(addr, [])
        if tag not in tags:
            tags.append(tag)

    b = corpus.branch_by_name.get(g.name)
    if from_branch and b is not None:
        add(b.addr, "branch")
    if g.addr in mapper:
        s, tag = mapper[g.addr]
        add(s, "mapper:%s" % tag)
    for row in corpus.names_by_base.get(g.name, []):
        if row.conf != "wrong":
            add(row.addr, "name_map:%s:%s" % (row.conf, sanitize_source(row.source)))
    if b is not None and not from_branch:
        add(b.addr, "branch")
    for a in srv_index.get(g.name, []):
        add(a, "server")
    if run_delta is not None:
        center = g.addr + run_delta
        pool = [s for s in corpus.steam_window(center - NEAR_WINDOW, center + NEAR_WINDOW)
                if not (corpus.trust.trusted_names(s) - {g.name})]
        exp = corpus.expected_ret(g)

        def key(s: int):
            retn_off = 0 if (exp is not None and corpus.steam[s].retn == exp) else 1
            fp = -scorer.total(g, s) if corpus.has_gog_fp else 0.0
            return (fp, retn_off, abs(s - center), s)

        for s in sorted(pool, key=key)[:near_k]:
            add(s, "near")
    return cands


def cmd_candidates(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    bdir.mkdir(parents=True, exist_ok=True)
    maybe_reexport(paths, args.no_reexport)
    corpus = bd.Corpus(paths, exclude_batch=args.batch)
    scorer = bd.Scorer(corpus)
    classes = [c for c in (args.classes or "").split(",") if c]
    if not classes and not args.from_branch:
        die("give --classes C1,C2 or --from-branch")
    if args.from_branch:
        rows = [g for g in corpus.gog if g.name in corpus.branch_by_name and (not classes or g.cls in classes)]
        classes = sorted({g.cls for g in rows})
    else:
        rows = [g for g in corpus.gog if g.cls in classes]
        for c in classes:
            if not any(g.cls == c for g in rows):
                print("warning: class %s has no GOG functions in gog_functions.tsv" % c)
    findings = Path(args.findings_dir).resolve() if args.findings_dir else paths.dump / "findings"
    mapper: dict[int, tuple[int, str]] = {}
    if args.from_branch:
        if classes:
            mapper.update(run_mapper(["^%s$" % re.escape(c) for c in classes], findings / ("%s_map_all.tsv" % args.batch)))
    else:
        for c in classes:
            mapper.update(run_mapper(["^%s$" % re.escape(c)], findings / ("%s_map_%s.tsv" % (args.batch, c))))
    srv_index = server_name_index(corpus)
    deltas = {c: corpus.mapped_run_delta(c, mapper) for c in classes}
    out = []
    for g in rows:
        cands = build_candidates(corpus, scorer, g, mapper, deltas.get(g.cls), srv_index, args.from_branch)
        gs, gr = corpus.gog_size(g.addr), corpus.gog_retn(g.addr)
        out.append({"class": g.cls, "name": g.func, "gog": hx8(g.addr), "gog_callconv": g.callconv,
                    "gog_param": "" if g.param is None else g.param, "gog_size": "" if gs is None else gs,
                    "gog_retn": "" if gr is None else gr, "candidates": format_candidates(cands)})
    write_tsv(bdir / "candidates.tsv", CANDIDATE_COLS, out)
    n_with = sum(1 for r in out if r["candidates"])
    print("%s: %d GOG rows, %d with candidates -> %s" % (args.batch, len(out), n_with, bdir / "candidates.tsv"))
    for c in classes:
        d = deltas.get(c)
        print("  %-28s run_delta=%s" % (c, "none" if d is None else "%+#x" % d))
    return 0


# ================================================================================================ evaluation
@dataclass
class CandEval:
    """Invariants and anchor families of one (GOG function, Steam candidate) pair."""
    addr: int
    sources: list
    eff: int = 0
    thunk: bool = False
    thunk_unresolved: bool = False
    steam_size: Optional[int] = None
    steam_retn: Optional[int] = None
    g1: str = "n/a"
    g2: str = "n/a"
    g3: str = "n/a"
    g4: str = "n/a"
    g5: str = ""
    g6: str = "n/a"
    hard: str = ""
    flags: list = field(default_factory=list)
    fam: dict = field(default_factory=dict)
    counts: dict = field(default_factory=dict)
    decoy_best: str = ""
    contradictions: int = 0
    decoy_sig_max: float = 0.0          # highest A-SIG score among the decoys (n/a counts as 0)
    eff_size: Optional[int] = None      # size of the thunk-resolved Steam function
    gog_size: Optional[int] = None      # size of the (thunk-resolved) GOG function

    @property
    def families(self) -> list:
        return [f for f in FAMILIES if self.counts.get(f)]

    @property
    def n_fam(self) -> int:
        """Families that count toward C1/C2.  VT is informational only: class_methods symbols come from the same
        labelling as the rows under test (circular, calibration 2026-10-03)."""
        return len([f for f in self.families if f != "VT"])

    @property
    def inv_pass(self) -> int:
        return sum(1 for v in (self.g1, self.g2, self.g3, self.g4, self.g6) if v.startswith("pass"))

    @property
    def mapper_tag(self) -> str:
        for s in self.sources:
            if s.startswith("mapper:"):
                return s.split(":", 1)[1]
        return ""


@dataclass
class RowEval:
    g: GogFn
    cands: list
    best: Optional[CandEval] = None
    ambiguous: bool = False
    cand_orig: Optional[int] = None


def source_strength(corpus_sources: list) -> int:
    """Ordering of candidate sources (SOURCE_ORDER_NOTE); used only as a tie-break after anchors and invariants."""
    best = 0
    for s in corpus_sources:
        if s == "server":
            v = 7
        elif s.startswith("name_map:high:") and not bd.is_unaudited_source(s[len("name_map:high:"):]):
            v = 6
        elif s == "mapper:vote":
            v = 5
        elif s == "branch":
            v = 4
        elif s == "mapper:seeded":
            v = 3
        elif s.startswith(("name_map:", "mapper:")):
            v = 2
        else:
            v = 1
        best = max(best, v)
    return best


def check_dup(corpus: bd.Corpus, addr: int, name: str) -> tuple[str, str]:
    """G6: -> (status, hard-reject reason).  Trusted other holder = REJECT, untrusted/branch holder = REVIEW."""
    me = base_name(name)
    trusted_other = corpus.trust.trusted_names(addr) - {me}
    if trusted_other:
        return "fail:trusted_holder=%s" % ",".join(sorted(trusted_other)), \
            "G6 address held by trusted %s" % ",".join(sorted(trusted_other))
    nm = corpus.name_map.get(addr)
    if nm and base_name(nm.name) == me and nm.conf == "wrong":
        return "fail:known_wrong", "G6 name_map marks this name at this address as wrong"
    holders = set()
    if nm and base_name(nm.name) != me and nm.conf != "wrong":
        holders.add(base_name(nm.name))
    for b in corpus.branch_by_addr.get(addr, []):
        if b.name != name:
            holders.add(b.name)
    if holders:
        return "review:held_by=%s" % ",".join(sorted(holders)), ""
    return "pass", ""


def evaluate_candidate(corpus: bd.Corpus, scorer: bd.Scorer, g: GogFn, addr: int, sources: list,
                       other_effs: list) -> CandEval:
    """Compute G1-G6 and the anchor families of one candidate (section 4.2)."""
    ce = CandEval(addr, sources)
    sf = corpus.steam.get(addr)
    if sf is None:
        ce.g1, ce.hard = "fail:not_a_function_start", "G1 not a function start in steam_functions.tsv"
        ce.eff = addr
        return ce
    eff, was, unres = corpus.resolve_thunk(addr)
    ce.eff, ce.thunk, ce.thunk_unresolved = eff, was, unres
    ef = corpus.steam.get(eff, sf)
    ce.steam_size = sf.size
    ce.steam_retn = ef.retn if ef.retn >= 0 else None
    ce.g5 = ("%s" % hx8(eff) if eff != addr else "") if not unres else "unresolved"
    # G1 start / CRT / library
    trusted_here = corpus.trust.is_trusted(addr, g.name)
    if addr in bd.CRT_ADDRS or eff in bd.CRT_ADDRS:
        ce.g1, ce.hard = "fail:crt", "G1 CRT/library function (bdlib.CRT_ADDRS)"
    elif sf.name.startswith("Unwind@"):
        ce.g1, ce.hard = "fail:unwind_funclet", "G1 SEH unwind funclet, not a function"
    elif not trusted_here and bd.is_library_name(sf.name, sf.source):
        if bd.is_soft_autolabel(sf.name):
            ce.g1 = "warn:autolabel"
            ce.flags.append("autolabel:%s" % sf.name[:30])      # REVIEW: Ghidra FID labels also sit on game code
        else:
            ce.g1, ce.hard = "fail:library", "G1 library function %s (source ANALYSIS)" % sf.name
    else:
        ce.g1 = "pass"
    # G2 alignment (signal only)
    ce.g2 = "pass" if addr % 16 == 0 else "fail:unaligned"
    # G3 prologue on the thunk-resolved function
    asm = corpus.asm(eff)
    if asm:
        found = corpus.rp.classify_prologue("".join(b for _a, b, _i in asm[:6]))
        ce.g3 = "pass" if found else "fail:no_standard_prologue"
    else:                                   # no .asm: read the first bytes from the exe copy
        raw = corpus.exe_bytes(eff, 16)
        if raw:
            found = corpus.rp.classify_prologue(raw.hex())
            ce.g3 = ("pass" if found else "fail:no_standard_prologue") + "(exe)"
        else:
            ce.g3 = "n/a"
    # G4 ret
    gr = corpus.gog_retn(g.addr)
    exp = corpus.expected_ret(g)
    if gr is not None and exp is not None and exp != gr:
        ce.flags.append("gog_db_param_bad")
    if gr is None or ce.steam_retn is None:
        ce.g4 = "n/a"
        if ce.steam_retn is not None and exp is not None and exp != ce.steam_retn:
            ce.flags.append("retn_meta_mismatch")
    elif ce.steam_retn == gr:
        ce.g4 = "pass"
    elif unres:
        ce.g4 = "n/a"
    else:
        ce.g4 = "fail:retn %d != gog %d" % (ce.steam_retn, gr)
        ce.hard = ce.hard or "G4 steam retn %d vs GOG retn %d (no thunk explanation)" % (ce.steam_retn, gr)
    # G6 duplicates
    status, reason = check_dup(corpus, addr, g.name)
    ce.g6 = status
    if reason:
        ce.hard = ce.hard or reason
    # families, with decoys = neighbours of the effective function + the row's other candidates
    decoys = []
    for d in corpus.steam_neighbours(eff) + other_effs:
        if d != eff and d not in decoys:
            decoys.append(d)
    ce.fam, ce.counts, ce.decoy_best = scorer.discriminating(g, eff, decoys)
    sigs = [scorer.a_sig(g, d) for d in decoys]
    ce.decoy_sig_max = max([s.score for s in sigs if s is not None], default=0.0)
    ce.eff_size = ef.size
    ce.gog_size = corpus.gog_size(corpus.resolve_gog(g.addr))
    callee = ce.fam.get("CALLEE")
    ce.contradictions = callee.extra.get("contradictions", 0) if callee else 0
    return ce


def evaluate_row(corpus: bd.Corpus, scorer: bd.Scorer, g: GogFn, cands: list) -> RowEval:
    """Evaluate every candidate of a row and pick the best one (anchor families, invariants, source strength)."""
    effs = {a: corpus.resolve_thunk(a)[0] if a in corpus.steam else a for a, _ in cands}
    evals = []
    for a, srcs in cands:
        others = [e for o, e in effs.items() if o != a]
        evals.append(evaluate_candidate(corpus, scorer, g, a, srcs, others))
    row = RowEval(g, evals, cand_orig=cands[0][0] if cands else None)
    if evals:
        row.best = max(evals, key=lambda e: (0 if e.hard else 1, e.n_fam, e.inv_pass,
                                             source_strength(e.sources), -e.addr))
        rivals = [e for e in evals if e is not row.best and not e.hard and e.eff != row.best.eff and e.n_fam >= 2]
        row.ambiguous = row.best.n_fam >= 2 and bool(rivals)
    return row


# ================================================================================================ run consistency
def run_ok_state(corpus: bd.Corpus, g_addr: int, steam_addr: int, mapped: dict) -> str:
    """'yes' if S-G equals the delta of the nearest mapped GOG neighbour on at least one side."""
    i = corpus.gog_index[g_addr]
    delta = steam_addr - g_addr
    seen = False
    for step in (-1, 1):
        j, n = i + step, 0
        while 0 <= j < len(corpus.gog) and n < RUN_WINDOW:
            ga = corpus.gog[j].addr
            if ga in mapped:
                seen = True
                if mapped[ga] - ga == delta:
                    return "yes"
                break
            j += step
            n += 1
    return "no" if seen else "n/a"


def find_permuted(corpus: bd.Corpus, scorer: bd.Scorer, rows: list[RowEval]) -> set[int]:
    """GOG addresses of adjacent same-size pairs whose fingerprint scores prefer the swapped assignment."""
    chosen = {r.g.addr: r.best for r in rows if r.best is not None and not r.best.hard}
    out = set()
    for ga, ce in chosen.items():
        i = corpus.gog_index[ga]
        if i + 1 >= len(corpus.gog):
            continue
        g2 = corpus.gog[i + 1]
        ce2 = chosen.get(g2.addr)
        if ce2 is None or ce2.eff == ce.eff:
            continue
        a, b = corpus.steam.get(ce.eff), corpus.steam.get(ce2.eff)
        if a is None or b is None or a.retn != b.retn or abs(a.size - b.size) > max(8, 0.25 * max(a.size, b.size)):
            continue
        g1 = corpus.gog_by_addr[ga]
        assigned = scorer.total(g1, ce.eff) + scorer.total(g2, ce2.eff)
        swapped = scorer.total(g1, ce2.eff) + scorer.total(g2, ce.eff)
        if swapped > assigned:
            out.update((ga, g2.addr))
    return out


SIG_ROUTE_MIN = 0.75
SIG_ROUTE_MARGIN = 0.25


def sig_route_ok(row: RowEval, run_ok: str, permuted: bool) -> bool:
    """Second AUTO route `auto_route=sig` (lead decision 2026-10-03, pending Opus calibration sign-off).

    Every invariant passes, the only counting family (VT excluded) is SIG with score >= 0.75, no decoy's SIG score
    exceeds ours minus 0.25, run_ok is yes (mapper:vote does NOT help), the Steam and GOG function sizes are equal,
    and the row is neither permuted, ambiguous nor contradicted by a trusted callee.
    """
    b = row.best
    if b is None or b.hard or b.thunk_unresolved or permuted or row.ambiguous or run_ok != "yes":
        return False
    if not (b.g1 == "pass" and b.g2 == "pass" and b.g3.startswith("pass") and b.g4 == "pass" and b.g6 == "pass"):
        return False
    if [f for f in b.families if f != "VT"] != ["SIG"]:
        return False
    sig = b.fam.get("SIG")
    if sig is None or sig.score < SIG_ROUTE_MIN or b.decoy_sig_max > sig.score - SIG_ROUTE_MARGIN:
        return False
    if b.contradictions or b.eff_size is None or b.gog_size is None or b.eff_size != b.gog_size:
        return False
    return True


def classify(row: RowEval, run_ok: str, permuted: bool) -> tuple[str, list[str], str]:
    """-> (gate class, reasons that kept the row out of AUTO, auto route 'c2' | 'sig' | '')."""
    if not row.cands:
        return "NOCAND", [], ""
    b = row.best
    if b.hard:
        return "REJECT", [b.hard], ""
    why = []
    if b.g1 != "pass":
        why.append("g1")
    if b.g2 != "pass":
        why.append("g2")
    if not b.g3.startswith("pass"):
        why.append("g3")
    if b.g4 != "pass":
        why.append("g4" if b.g4 != "n/a" else "g4_na")
    if b.thunk_unresolved:
        why.append("g5")
    if b.g6 != "pass":
        why.append("g6")
    if b.n_fam < 2:
        why.append("c%d" % b.n_fam)
    if run_ok != "yes" and b.mapper_tag != "vote":
        why.append("run_" + run_ok.replace("/", ""))
    if permuted:
        why.append("permuted")
    if row.ambiguous:
        why.append("ambiguous")
    callee = b.fam.get("CALLEE")
    if callee is not None and b.contradictions > callee.score:
        why.append("callee_contradiction")
    if not why:
        return "AUTO", [], "c2"
    if sig_route_ok(row, run_ok, permuted):
        return "AUTO", [], "sig"
    return "REVIEW", why, ""


def assign_classes(corpus: bd.Corpus, rows: list[RowEval], permuted: set[int]) -> dict[int, tuple[str, str, list, str]]:
    """-> {gog_addr: (gate_class, run_ok, why, route)}.

    Trusted pairs and `c2`-route AUTO rows form the mapped-neighbour set for run_ok and are iterated to a fixpoint.
    `sig`-route AUTO rows are NEVER added to it (non-propagating).
    """
    mapped = dict(corpus.pairs)
    result: dict[int, tuple[str, str, list, str]] = {}
    for _ in range(40):
        new_auto = False
        for r in rows:
            ga = r.g.addr
            if ga in result and result[ga][3] == "c2":
                continue
            ok = "n/a"
            if r.best is not None and not r.best.hard:
                ok = run_ok_state(corpus, ga, r.best.addr, {k: v for k, v in mapped.items() if k != ga})
            cls, why, route = classify(r, ok, ga in permuted)
            result[ga] = (cls, "permuted" if ga in permuted else ok, why, route)
            if route == "c2":
                mapped[ga] = r.best.addr
                new_auto = True
        if not new_auto:
            break
    return result


# ================================================================================================ evidence rows
def guess_callconv(corpus: bd.Corpus, g: GogFn, ce: CandEval) -> str:
    """GOG callconv when known; else thiscall if ECX is read before it is written, else stdcall/cdecl by retn."""
    cc = g.callconv.lstrip("_").lower()
    if cc in ("thiscall", "stdcall", "cdecl", "fastcall"):
        return cc
    asm = corpus.asm(ce.eff)
    written = False
    for _a, _b, insn in (asm or [])[:30]:
        mn, ops = corpus.rp.split_insn(insn)
        if mn in ("RET", "RETN"):
            break
        dst = ops[0].upper().replace(" ", "") if ops else ""
        src = ",".join(ops[1:]).upper()
        reads = ("ECX" in src) or ("[ECX" in dst) or (mn == "PUSH" and dst == "ECX") or \
            (mn in ("ADD", "SUB", "CMP", "TEST", "AND", "OR", "INC", "DEC", "NEG") and dst == "ECX")
        if dst == "ECX" and mn in ("MOV", "LEA", "POP") and "ECX" not in src:
            written = True
        elif mn == "XOR" and dst == "ECX" and src.replace(" ", "") == "ECX":
            written = True
        elif reads and not written:
            return "thiscall"
    return "stdcall" if (ce.steam_retn or 0) > 0 else "cdecl"


def evidence_summary(ce: CandEval, corpus: bd.Corpus) -> str:
    """Short machine-written anchor summary of the families that count."""
    parts = []
    f = ce.fam
    if ce.counts.get("STR"):
        parts.append("STR %s" % f["STR"].detail)
    if ce.counts.get("CALLEE"):
        parts.append("CALLEE %s" % f["CALLEE"].detail)
    if ce.counts.get("SIG"):
        parts.append("SIG %.2f" % f["SIG"].score)
    if ce.counts.get("SRV"):
        parts.append("SRV %s" % f["SRV"].detail)
    if ce.counts.get("VT"):
        parts.append("VT %s" % f["VT"].detail)
    return "; ".join(parts) if parts else "no content anchor (C0)"


def evidence_row(batch: str, corpus: bd.Corpus, row: RowEval, gate: str, run_ok: str, why: list,
                 route: str = "") -> dict:
    g, b = row.g, row.best
    gr, gs = corpus.gog_retn(g.addr), corpus.gog_size(g.addr)
    d = {c: "" for c in EVIDENCE_COLS}
    d.update(kind="F", **{"class": g.cls}, name=g.func, gog=hx8(g.addr), batch=batch, gog_size="" if gs is None else gs,
             gog_retn="" if gr is None else gr, gog_param_size="" if g.param is None else g.param,
             gate_class=gate, review_ref="%s:%s" % (batch, hx8(g.addr)),
             cand_sources=";".join("%s|%s" % (hx8(c.addr), "+".join(c.sources)) for c in row.cands),
             verdict={"AUTO": "auto", "REVIEW": "review", "REJECT": "rejected", "NOCAND": "unresolved"}[gate])
    if b is None:
        d.update(evidence="no candidate", cand_status="none", content_level="C0", run_ok="n/a")
        return d
    cc = guess_callconv(corpus, g, b)
    sretn = b.steam_retn
    d.update(steam_addr=hx8(b.addr), calling_convention=cc,
             param_size_bytes="" if sretn is None else (g.param if cc == "cdecl" and g.param is not None else sretn),
             evidence=evidence_summary(b, corpus), cand_orig=hx8(row.cand_orig) if row.cand_orig is not None else "",
             cand_status="confirmed" if b.addr == row.cand_orig else "changed",
             content_level="C%d" % min(b.n_fam, 2), steam_size="" if b.steam_size is None else b.steam_size,
             mapper_tag=b.mapper_tag, steam_retn="" if sretn is None else sretn, g1=b.g1, g2=b.g2, g3=b.g3, g4=b.g4,
             g5_target=b.g5, g6=b.g6, a_str=fmt_score(b.fam.get("STR")), a_callee=fmt_score(b.fam.get("CALLEE")),
             a_sig=fmt_score(b.fam.get("SIG"), "float"), a_srv=fmt_score(b.fam.get("SRV")),
             a_vt=fmt_score(b.fam.get("VT")), families=",".join(b.families), run_ok=run_ok, decoy_best=b.decoy_best)
    flags = list(b.flags)
    if gate == "AUTO" and route:
        flags.append("auto_route=%s" % route)
    if gate in ("REVIEW", "REJECT") and why:
        flags.append("why:" + ",".join(why))
    if b.contradictions:
        flags.append("callee_contra=%d" % b.contradictions)
    d["flags"] = ";".join(flags)
    return d


# ================================================================================================ gates command
def read_lessons_section(report: Path) -> str:
    """The user-written `## Lessons` section of an existing gate_report.md (carried over on regeneration)."""
    try:
        text = report.read_text(encoding="utf-8")
    except OSError:
        return ""
    m = re.search(r"^## Lessons\s*\n(.*?)(?=^## |\Z)", text, re.S | re.M)
    return m.group(1).strip("\n") if m else ""


LESSONS_PLACEHOLDER = ("<!-- one lesson per line: `- scope | topic | note`; persist inserts each once "
                       "(dedupe on scope+topic) -->")


def write_gate_report(path: Path, batch: str, corpus: bd.Corpus, evs: list[dict], rows: list[RowEval],
                      classes: dict) -> None:
    lessons = read_lessons_section(path) or LESSONS_PLACEHOLDER
    order = ["AUTO", "REVIEW", "REJECT", "NOCAND"]
    by_class: dict[str, Counter] = {}
    for e in evs:
        by_class.setdefault(e["class"], Counter())[e["gate_class"]] += 1
    out = ["# Gate report %s" % batch, "", "Rows: %d. Source order for tie-breaks: %s." % (len(evs), SOURCE_ORDER_NOTE), ""]
    if not corpus.has_gog_fp:
        out += ["NOTE: no GOG fingerprints (SERVER/%s/fp.tsv) were found: A-STR / A-CALLEE / A-SIG are n/a and no row "
                "can reach AUTO (C2 needs two families)." % corpus.paths.gog_program, ""]
    out += ["## Counts per class and gate class", "", "| class | " + " | ".join(order) + " | total |",
            "|---|" + "---|" * (len(order) + 1)]
    total = Counter()
    for cls in sorted(by_class):
        c = by_class[cls]
        total.update(c)
        out.append("| %s | %s | %d |" % (cls, " | ".join(str(c[k]) for k in order), sum(c.values())))
    out.append("| **all** | %s | %d |" % (" | ".join(str(total[k]) for k in order), sum(total.values())))
    routes = Counter(next((p.split("=", 1)[1] for p in e["flags"].split(";") if p.startswith("auto_route=")), "")
                     for e in evs if e["gate_class"] == "AUTO")
    out += ["", "AUTO by route: c2=%d sig=%d (the sig route is non-propagating, see DESIGN 4.2)" % (
        routes["c2"], routes["sig"])]
    out += ["", "## REJECT rows", ""]
    rej = [(e, r) for e, r in zip(evs, rows) if e["gate_class"] == "REJECT"]
    out += ["- %s::%s gog %s -> %s: %s" % (e["class"], e["name"], e["gog"], e["steam_addr"], r.best.hard)
            for e, r in rej] or ["(none)"]
    out += ["", "## Duplicates and conflicts (G6)", ""]
    dups = [e for e in evs if e["g6"] not in ("pass", "n/a", "")]
    out += ["- %s::%s gog %s -> %s: %s" % (e["class"], e["name"], e["gog"], e["steam_addr"], e["g6"]) for e in dups] \
        or ["(none)"]
    out += ["", "## G4 metadata flags", ""]
    flagged = [e for e in evs if "gog_db_param_bad" in e["flags"] or "retn_meta_mismatch" in e["flags"]]
    out += ["- %s::%s gog %s: %s (gog_param=%s gog_retn=%s steam_retn=%s)" % (
        e["class"], e["name"], e["gog"], [f for f in e["flags"].split(";") if "param_bad" in f or "meta" in f],
        e["gog_param_size"], e["gog_retn"], e["steam_retn"]) for e in flagged] or ["(none)"]
    out += ["", "## Ambiguous rows (second candidate also reaches C2)", ""]
    amb = [e for e, r in zip(evs, rows) if r.ambiguous]
    out += ["- %s::%s gog %s" % (e["class"], e["name"], e["gog"]) for e in amb] or ["(none)"]
    out += ["", "## Lessons", "", lessons, ""]
    path.write_text("\n".join(out), encoding="utf-8")


def load_batch_candidates(bdir: Path) -> list[dict]:
    rows = list(iter_tsv(bdir / "candidates.tsv"))
    if not rows and not (bdir / "candidates.tsv").exists():
        die("%s not found (run `candidates` first)" % (bdir / "candidates.tsv"))
    return rows


def evaluate_batch(batch: str, corpus: bd.Corpus, scorer: bd.Scorer, crows: list[dict]):
    """candidates.tsv rows -> (RowEval list, class assignment dict)."""
    rows = []
    for r in crows:
        try:
            ga = parse_addr(r["gog"])
        except (ValueError, KeyError):
            continue
        g = corpus.gog_by_addr.get(ga)
        if g is None:
            print("warning: GOG address %s not in gog_functions.tsv; row skipped" % r.get("gog"))
            continue
        rows.append(evaluate_row(corpus, scorer, g, parse_candidates(r.get("candidates", ""))))
    permuted = find_permuted(corpus, scorer, rows)
    return rows, assign_classes(corpus, rows, permuted)


def load_untrust(path) -> set[int]:
    """`--untrust FILE`: one hex Steam address per line ('#' comments and blank lines allowed)."""
    out = set()
    try:
        text = Path(path).read_text(encoding="utf-8")
    except OSError:
        die("cannot read untrust file %s" % path)
    for line in text.splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            out.add(parse_addr(line))
    return out


def run_gates(batch: str, bdir: Path, corpus: bd.Corpus, scorer: bd.Scorer, crows: list[dict]) -> list[dict]:
    """Evaluate candidate rows, write evidence.tsv + gate_report.md into bdir, return the evidence rows."""
    rows, classes = evaluate_batch(batch, corpus, scorer, crows)
    evs = []
    for r in rows:
        gate, run_ok, why, route = classes[r.g.addr]
        evs.append(evidence_row(batch, corpus, r, gate, run_ok, why, route))
    write_tsv(bdir / "evidence.tsv", EVIDENCE_COLS, evs)
    write_gate_report(bdir / "gate_report.md", batch, corpus, evs, rows, classes)
    return evs


def cmd_gates(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    crows = load_batch_candidates(bdir)
    untrust = load_untrust(args.untrust) if args.untrust else set()
    corpus = bd.Corpus(paths, exclude_batch=args.batch, untrust=untrust)
    scorer = bd.Scorer(corpus)
    evs = run_gates(args.batch, bdir, corpus, scorer, crows)
    cnt = Counter(e["gate_class"] for e in evs)
    print("%s gates: %d rows  %s%s" % (args.batch, len(evs), "  ".join("%s=%d" % (k, cnt[k]) for k in
                                                                      ("AUTO", "REVIEW", "REJECT", "NOCAND")),
                                     "  (untrusted %d addresses)" % len(untrust) if untrust else ""))
    by_class: dict[str, Counter] = {}
    for e in evs:
        by_class.setdefault(e["class"], Counter())[e["gate_class"]] += 1
    for cls in sorted(by_class):
        c = by_class[cls]
        print("  %-26s %s" % (cls, "  ".join("%s=%d" % (k, c[k]) for k in ("AUTO", "REVIEW", "REJECT", "NOCAND"))))
    return 0


# ================================================================================================ cards
CARD_HEADER = """# Review cards %s chunk %d

Answer each card with one TSV row in `%s/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.
"""


def fmt_prologue(corpus: bd.Corpus, addr: int) -> list[str]:
    asm = corpus.asm(addr)
    if not asm:
        return ["prologue: no asm for %s" % hx8(addr)]
    lines = ["prologue for %s: 16-byte aligned: %s" % (hx8(addr), "YES" if addr % 16 == 0 else
                                                       "NO (addr %% 16 = %d)" % (addr % 16))]
    for a, b, insn in asm[:3]:
        lines.append("  %08x  %-14s %s" % (a, " ".join(b[i:i + 2] for i in range(0, len(b), 2)).upper(), insn))
    found = corpus.rp.classify_prologue("".join(b for _a, b, _i in asm[:6]))
    lines.append("standard prologue: %s" % ("YES - " + "; ".join(found) if found else "NO"))
    return lines


def fmt_anchors(corpus: bd.Corpus, addr: int) -> list[str]:
    asm = corpus.asm(addr)
    if not asm:
        return ["anchors: no asm for %s" % hx8(addr)]
    rp = corpus.rp
    lines = ["anchors for %s" % hx8(addr)]
    hits = rp.string_anchors(asm, corpus.strings.text)
    lines.append("string literals (%d):" % len(hits))
    lines += ["  0x%08X  at %08x  %r" % (v, a, s) for a, v, s in hits[:12]]
    callees = [c for c in rp.asm_call_targets(asm) if c >= 0x401000]
    lines.append("direct callees (%d):" % len(callees))
    for c in callees[:20]:
        nm = corpus.name_map.get(c)
        lines.append("  0x%08X  %s" % (c, nm.name if nm else "(unnamed)"))
    offs = rp.struct_offsets(asm, top=10)
    lines.append("struct offsets touched (top %d):" % len(offs))
    lines += ["  +0x%X [dec %d]  x%d  via %s" % (o, o, c, ",".join(rg)) for o, c, rg in offs]
    return lines


def neighbour_line(corpus: bd.Corpus, g: GogFn, step: int, ev_by_gog: dict) -> str:
    j = corpus.gog_index[g.addr] + step
    if not 0 <= j < len(corpus.gog):
        return "(none)"
    n = corpus.gog[j]
    e = ev_by_gog.get(n.addr)
    if e is not None:
        return "%s %s -> %s  gate_class=%s" % (hx8(n.addr), n.name, e["steam_addr"] or "-", e["gate_class"])
    if n.addr in corpus.pairs:
        return "%s %s -> %s  trusted" % (hx8(n.addr), n.name, hx8(corpus.pairs[n.addr]))
    return "%s %s  (not in this batch, no trusted mapping)" % (hx8(n.addr), n.name)


def fmt_hexset(vals, common, limit=14) -> str:
    """Sorted hex values, a trailing `*` on those present on both sides, truncated."""
    items = sorted(vals)
    s = ",".join("0x%x%s" % (v, "*" if v in common else "") for v in items[:limit])
    if len(items) > limit:
        s += ",...(+%d)" % (len(items) - limit)
    return "{" + s + "}"


def gog_side_section(corpus: bd.Corpus, g: GogFn, ce: CandEval) -> list[str]:
    """`### GOG side` card section: GOG callees / strings / operand sets beside the Steam candidate's (<= 25 lines)."""
    L = ["### GOG side (from the server GOG fingerprint)"]
    gfp = corpus.gog_fp.get(corpus.resolve_gog(g.addr))
    if gfp is None:
        return L + ["(no GOG fingerprint available for %s)" % hx8(g.addr), ""]
    s_callees = set(corpus.cg.callees.get(ce.eff, set()))
    sfp = corpus.steam_fp.get(ce.eff)
    if sfp is not None:
        s_callees |= sfp.callees
    callees = sorted(gfp.callees)
    L.append("GOG callees (%d), mapped to Steam through trusted rows only:" % len(callees))
    for c in callees[:8]:
        gg = corpus.gog_by_addr.get(c)
        sa = corpus.pairs.get(c)
        L.append("  0x%08x %s -> Steam %s  in candidate callees: %s" % (
            c, gg.name if gg else "unnamed", hx8(sa) if sa else "?",
            "n/a" if sa is None else ("yes" if sa in s_callees else "no")))
    if len(callees) > 8:
        L.append("  ... %d more GOG callees" % (len(callees) - 8))
    texts = [corpus.gog_texts.get(a) for a in sorted(gfp.str_refs)]
    texts = [t for t in texts if t]
    L.append("GOG strings (%d): %s" % (len(texts), "; ".join(repr(t[:40]) for t in texts[:4]) + (" ..." if len(texts) > 4 else "")))
    for label, gset, sset in (("disp", set(gfp.disp), set(sfp.disp) if sfp else None),
                              ("imm", set(gfp.imm), set(sfp.imm) if sfp else None)):
        if sset is None:
            L.append("GOG %s: %s | Steam candidate fingerprint missing" % (label, fmt_hexset(gset, set())))
            continue
        common = gset & sset
        L.append("%s sets (* = in both, %d common): GOG %s | Steam %s" % (
            label, len(common), fmt_hexset(gset, common), fmt_hexset(sset, common)))
    L.append("")
    return L


def make_card(card_id: str, corpus: bd.Corpus, g: GogFn, ce: CandEval, ev_by_gog: dict, mapper_tag: str) -> str:
    rp = corpus.rp
    root = str(corpus.paths.dump)
    L = ["## Card %s" % card_id, "",
         "GOG function: %s  gog %s  callconv=%s  param_size_bytes=%s  gog_size=%s  gog_retn=%s" % (
             g.name, hx8(g.addr), g.callconv or "(blank)", "?" if g.param is None else g.param,
             corpus.gog_size(g.addr), corpus.gog_retn(g.addr)),
         "Candidate Steam address: %s (size %s, retn %s)%s" % (
             hx8(ce.addr), ce.steam_size, ce.steam_retn,
             "  THUNK -> %s (everything below is judged on the target)" % hx8(ce.eff) if ce.thunk else ""),
         "Candidate sources: %s    mapper tag: %s" % ("+".join(ce.sources) or "-", mapper_tag or "-"), "",
         "### Computed facts (script output, do not recompute)",
         "- G1 start: %s | G2 align: %s | G3 prologue: %s | G4 ret: %s | G5 thunk: %s | G6 dup: %s" % (
             ce.g1, ce.g2, ce.g3, ce.g4, ce.g5 or "none", ce.g6),
         "- anchors: A-STR %s | A-CALLEE %s | A-SIG %s | A-SRV %s | A-VT %s" % (
             fmt_score(ce.fam.get("STR")), fmt_score(ce.fam.get("CALLEE")), fmt_score(ce.fam.get("SIG"), "float"),
             fmt_score(ce.fam.get("SRV")), fmt_score(ce.fam.get("VT"))),
         "- families that discriminate against the neighbours: %s  (content level C%d)" % (
             ",".join(ce.families) or "none", min(ce.n_fam, 2)),
         "- evidence summary: %s" % evidence_summary(ce, corpus),
         "- best decoy (neighbour/other candidate) scores: %s" % (ce.decoy_best or "none"),
         "- flags: %s" % (";".join(ce.flags) or "none"),
         "- GOG-order neighbour before: %s" % neighbour_line(corpus, g, -1, ev_by_gog),
         "- GOG-order neighbour after:  %s" % neighbour_line(corpus, g, 1, ev_by_gog)]
    srv = corpus.steam_srv.get(ce.eff)
    if srv and srv.get("source") in ("USER_DEFINED", "IMPORTED"):
        L.append("- server Steam name at this address: %s::%s" % (srv.get("namespace", ""), srv.get("name", "")))
    L += ["", "DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature "
          "until a content anchor proves otherwise.", ""]
    L += gog_side_section(corpus, g, ce)
    L += ["### abi", "```"]
    asm = corpus.asm(ce.eff)
    ctext = rp.read_text(rp.c_path(root, ce.eff))
    hdr = rp.parse_header(ctext)
    if g.param is not None:
        cc = g.callconv.lstrip("_").lower() or None
        if cc not in (None, "thiscall", "stdcall", "cdecl", "fastcall"):
            cc = None
        lines, _v = rp.abi_report(g.param, None, cc, "" if cc else " (GOG callconv blank -> assumed thiscall)",
                                  hdr["ret"] if hdr else None, rp.asm_rets(asm))
        L += lines
    else:
        L.append("GOG param size unknown")
    L += ["```", "", "### prologue", "```"] + fmt_prologue(corpus, ce.eff) + ["```", "", "### anchors", "```"]
    L += fmt_anchors(corpus, ce.eff)
    L += ["```", "", "### annotate (candidate, first 80 lines)", "```"]
    L.append(rp.annotate_text(ctext, asm, 80) if ctext else "(no decompile for %s)" % hx8(ce.eff))
    L += ["```"]
    dec = re.match(r"(0x[0-9a-fA-F]+):", ce.decoy_best or "")
    if dec:
        da = parse_addr(dec.group(1))
        L += ["", "### decoy: %s (neighbour that must NOT be confused with the candidate)" % hx8(da), "```"]
        L += fmt_prologue(corpus, da) + fmt_anchors(corpus, da) + ["```"]
    return "\n".join(L) + "\n"


def decoy_target(ev: dict) -> Optional[int]:
    m = re.match(r"(0x[0-9a-fA-F]+):", ev.get("decoy_best", ""))
    return parse_addr(m.group(1)) if m else None


def cmd_cards(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    evs = list(iter_tsv(bdir / "evidence.tsv"))
    if not evs:
        die("%s missing or empty (run `gates` first)" % (bdir / "evidence.tsv"))
    corpus = bd.Corpus(paths, exclude_batch=args.batch)
    scorer = bd.Scorer(corpus)
    ev_by_gog = {parse_addr(e["gog"]): e for e in evs}
    crows = {parse_addr(r["gog"]): r for r in iter_tsv(bdir / "candidates.tsv")}
    rng = random.Random("cards-" + args.batch)
    review = [e for e in evs if e["gate_class"] == "REVIEW"]
    autos = [e for e in evs if e["gate_class"] == "AUTO" and decoy_target(e) is not None]
    rng.shuffle(autos)
    per_chunk_decoys = max(1, args.chunk // 10) if args.decoys is None else args.decoys
    per_chunk_real = max(1, args.chunk - per_chunk_decoys)
    chunks = [review[i:i + per_chunk_real] for i in range(0, len(review), per_chunk_real)]
    cards_dir = bdir / "cards"
    cards_dir.mkdir(parents=True, exist_ok=True)
    for old in cards_dir.glob("chunk_*.md"):
        old.unlink()
    key_rows, used_auto = [], 0
    for k, real in enumerate(chunks, 1):
        items = [("real", e, None, "") for e in real]
        take = autos[used_auto:used_auto + per_chunk_decoys]
        used_auto += len(take)
        for n, e in enumerate(take):
            if n % 3 == 2:                       # a known-good row with its real address: expected match
                items.append(("gold", e, parse_addr(e["steam_addr"]), "match"))
            else:                                # the neighbour decoy of a known-good row: expected mismatch
                items.append(("decoy", e, decoy_target(e), "mismatch"))
        rng.shuffle(items)
        text = [CARD_HEADER % (args.batch, k, "steam_db_batches/" + args.batch)]
        for n, (kind, e, forced, expected) in enumerate(items, 1):
            g = corpus.gog_by_addr[parse_addr(e["gog"])]
            cid = "%s-%d-%02d" % (args.batch, k, n)
            if forced is None:
                cands = parse_candidates(crows[g.addr]["candidates"])
                row = evaluate_row(corpus, scorer, g, cands)
                want = parse_addr(e["steam_addr"])
                ce = next((c for c in row.cands if c.addr == want), row.best)
            else:
                ce = evaluate_row(corpus, scorer, g, [(forced, ["card"])]).best
            text.append(make_card(cid, corpus, g, ce, ev_by_gog, ce.mapper_tag if kind == "real" else ""))
            key_rows.append({"card_id": cid, "chunk": k, "gog": hx8(g.addr), "steam_addr": hx8(ce.addr),
                             "kind": kind, "expected": expected})
        (cards_dir / ("chunk_%d.md" % k)).write_text("\n".join(text), encoding="utf-8")
    write_tsv(cards_dir / "key.tsv", ["card_id", "chunk", "gog", "steam_addr", "kind", "expected"], key_rows)
    print("%s cards: %d REVIEW rows, %d chunks, %d calibration cards -> %s" % (
        args.batch, len(review), len(chunks), sum(1 for r in key_rows if r["kind"] != "real"), cards_dir))
    return 0


# ================================================================================================ finalize
REVIEW_COLS = ["gog_addr", "steam_addr", "verdict", "corrected_addr", "content_anchor", "confidence", "reason"]


def load_reviews(bdir: Path) -> dict[int, list[dict]]:
    out: dict[int, list[dict]] = {}
    for f in sorted((bdir / "review").glob("*.tsv")):
        for r in iter_tsv(f):
            try:
                out.setdefault(parse_addr(r["gog_addr"]), []).append(r)
            except (ValueError, KeyError):
                continue
    return out


def load_overrides(bdir: Path) -> dict[int, dict]:
    out = {}
    for r in iter_tsv(bdir / "overrides.tsv"):
        try:
            ga = parse_addr(r["gog_addr"])
        except (ValueError, KeyError):
            continue
        v = (r.get("final_verdict") or "").strip()
        if v not in FINAL_VERDICTS:
            die("overrides.tsv: bad final_verdict %r for %s" % (v, r.get("gog_addr")))
        out[ga] = r
    return out


def decide_review(ev: dict, reviews: list[dict]) -> tuple[str, str, str]:
    """Reviewer TSVs -> (final_verdict, steam_addr, note) for a REVIEW row (Haiku `match` alone never promotes)."""
    cand = ev["steam_addr"]
    level = ev.get("content_level", "C0")
    mine = [r for r in reviews if r.get("steam_addr") and parse_addr(r["steam_addr"]) == parse_addr(cand)] or reviews
    matches = [r for r in mine if r.get("verdict") == "match"]
    mism = [r for r in mine if r.get("verdict") == "mismatch"]
    if matches and mism:
        return "unresolved", cand, "reviewers disagree (match vs mismatch)"
    if matches:
        anchored = [r for r in matches if (r.get("content_anchor") or "").strip()]
        if anchored and level in ("C1", "C2"):
            return "verified", cand, "reviewer match with anchor: %s" % anchored[0]["content_anchor"][:200]
        if anchored or level in ("C1", "C2"):
            return "likely", cand, "reviewer match; %s" % ("no cited anchor" if not anchored else "content level C0")
        return "unresolved", cand, "reviewer match without a cited anchor and content level C0"
    if mism:
        fixed = [r for r in mism if (r.get("corrected_addr") or "").strip()]
        if fixed:
            return "likely", hx8(parse_addr(fixed[0]["corrected_addr"])), \
                "reviewer corrected the address (not re-gated): %s" % (fixed[0].get("reason") or "")[:160]
        return "unresolved", cand, "reviewer mismatch: %s" % (mism[0].get("reason") or "")[:160]
    return "unresolved", cand, "reviewer unsure" if reviews else "REVIEW row without a reviewer verdict"


def final_row_for(ev: dict, reviews: dict, overrides: dict) -> dict:
    ga = parse_addr(ev["gog"])
    gate = ev["gate_class"]
    steam, note = ev["steam_addr"], ""
    if gate == "AUTO":
        verdict, note = "verified", "AUTO: " + ev.get("evidence", "")[:160]
    elif gate == "REJECT":
        verdict, note = "rejected", ev.get("flags", "")
    elif gate == "NOCAND":
        verdict, note = "unresolved", "no candidate"
    else:
        verdict, steam, note = decide_review(ev, reviews.get(ga, []))
    ov = overrides.get(ga)
    if ov:
        verdict = ov["final_verdict"]
        if (ov.get("steam_addr") or "").strip():
            steam = hx8(parse_addr(ov["steam_addr"]))
        note = "override: %s" % (ov.get("note") or "")
    out = dict(ev)
    if steam and steam != ev["steam_addr"]:
        out["cand_status"] = "corrected"
    out.update(steam_addr=steam, final_verdict=verdict, final_note=note)
    return out


def finalize_params(row: dict, steam: dict) -> tuple[str, object, str]:
    """-> (calling_convention, param_size_bytes, extra note) for functions.csv from the final address."""
    cc = row.get("calling_convention") or "thiscall"
    gog_param = row.get("gog_param_size")
    sf = steam.get(parse_addr(row["steam_addr"]))
    retn = sf.retn if sf is not None and sf.retn >= 0 else None
    if cc == "cdecl":
        return cc, gog_param if gog_param != "" else 0, ""
    if retn is None:
        return cc, row.get("param_size_bytes", ""), ""
    note = ""
    if gog_param not in ("", None) and str(retn) != str(gog_param):
        note = "; GOG param_size_bytes %s" % gog_param
    return cc, retn, note


def write_functions_csv(path: Path, rows: list[dict], batch: str, steam: dict) -> int:
    buf = io.StringIO()
    w = csv.writer(buf, lineterminator="\n")
    w.writerow(FUNCTIONS_CSV_HEADER)
    n = 0
    for r in rows:
        cc, param, extra = finalize_params(r, steam)
        notes = "Steam batch %s: %s (evidence steam_db_batches/%s/evidence.tsv)%s" % (
            batch, r.get("evidence", ""), batch, extra)
        w.writerow([r["class"], r["name"], "0x%08X" % parse_addr(r["steam_addr"]), "__" + cc, param, notes])
        n += 1
    path.write_text(buf.getvalue(), encoding="utf-8")
    return n


def cmd_finalize(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    evs = list(iter_tsv(bdir / "evidence.tsv"))
    if not evs:
        die("%s missing or empty (run `gates` first)" % (bdir / "evidence.tsv"))
    reviews, overrides = load_reviews(bdir), load_overrides(bdir)
    finals = [final_row_for(e, reviews, overrides) for e in evs]
    write_tsv(bdir / "final_rows.tsv", EVIDENCE_COLS + ["final_verdict", "final_note"], finals)
    steam = bd.load_steam_functions(paths.dump)
    keep = ("verified", "likely") if args.with_likely else ("verified",)
    export = [r for r in finals if r["final_verdict"] in keep and r["steam_addr"]]
    n_csv = write_functions_csv(bdir / "functions.csv", export, args.batch, steam)
    date = today(args)
    upserts = []
    for r in export:
        conf = "high" if r["final_verdict"] == "verified" else "medium"
        ev = "[%s; %s retn=%s; GOG %s] Steam batch %s: %s | %s" % (
            r["final_verdict"], r.get("calling_convention", ""), r.get("steam_retn", ""), r["gog"], args.batch,
            r.get("evidence", ""), r.get("final_note", ""))
        upserts.append({"steam_addr": hx8(parse_addr(r["steam_addr"])), "name": "%s::%s" % (r["class"], r["name"]),
                        "source": "steam-db-%s" % args.batch, "confidence": conf, "evidence": ev[:900], "date": date})
    (bdir / "kdb_upsert.json").write_text(json.dumps(upserts, indent=1, sort_keys=True) + "\n", encoding="utf-8")
    if args.batch == "B0":
        deletes = [r for r in finals if r["final_verdict"] == "rejected" and r["cand_orig"]]
        write_tsv(bdir / "branch_deletes.tsv", ["class", "function", "address", "reason"],
                  [{"class": r["class"], "function": r["name"], "address": r["cand_orig"],
                    "reason": r["final_note"] or r["flags"]} for r in deletes])
        corr = [r for r in export if r["cand_orig"] and parse_addr(r["cand_orig"]) != parse_addr(r["steam_addr"])]
        write_functions_csv(bdir / "branch_corrections.csv", corr, args.batch, steam)
    cnt = Counter(r["final_verdict"] for r in finals)
    print("%s finalize: %s; functions.csv rows=%d; kdb_upsert rows=%d" % (
        args.batch, "  ".join("%s=%d" % (k, cnt[k]) for k in FINAL_VERDICTS), n_csv, len(upserts)))
    return 0


# ================================================================================================ persist
CONF_RANK = {"wrong": 0, "low": 1, "medium": 2, "high": 3}


def parse_lessons(report: Path) -> list[tuple[str, str, str]]:
    out = []
    for line in read_lessons_section(report).splitlines():
        line = line.strip()
        if not line.startswith("- "):
            continue
        parts = [p.strip() for p in line[2:].split(" | ", 2)]
        if len(parts) == 3 and all(parts):
            out.append((parts[0], parts[1], parts[2]))
    return out


def plan_persist(con: sqlite3.Connection, upserts: list[dict], rejected: list[dict], lessons: list, batch: str,
                 date: str) -> list[tuple]:
    """Compute the exact change list against the current name_map (no writes).

    Ops: ('insert', row) ('update', row, why) ('demote', addr, evidence_suffix) ('lesson', scope, topic, note)
         ('skip', addr, why).  Demotions run first, then upserts, with an overlay so the plan equals what apply does.
    """
    source = "steam-db-%s" % batch
    overlay: dict[str, tuple] = {}

    def current(addr: str):
        if addr in overlay:
            return overlay[addr]
        return con.execute("select name,source,confidence,evidence from name_map where steam_addr=?",
                           (addr,)).fetchone()

    ops: list[tuple] = []
    for r in rejected:
        addr = hx8(parse_addr(r["steam_addr"]))
        name = "%s::%s" % (r["class"], r["name"])
        cur = current(addr)
        if cur and base_name(cur[0]) == name and cur[2] != "wrong" and cur[1] != source:
            ev = "%s | demoted by %s %s: rejected (%s)" % (cur[3] or "", source, date, (r.get("final_note") or "")[:160])
            ops.append(("demote", addr, ev))
            overlay[addr] = (cur[0], cur[1], "wrong", ev)
    for u in upserts:
        addr = u["steam_addr"]
        cur = current(addr)
        new = (u["name"], u["source"], u["confidence"], u["evidence"])
        if cur is None:
            ops.append(("insert", u))
            overlay[addr] = new
        elif base_name(cur[0]) == base_name(u["name"]):
            # stored evidence always starts with the upsert evidence (replace/was suffixes are appended), so a
            # re-run recognises its own row and changes nothing
            same = cur[1] == u["source"] and cur[2] == u["confidence"] and (cur[3] or "").startswith(u["evidence"])
            cur_trusted = bd.is_trusted(cur[1], cur[2])
            if same:
                continue
            if cur_trusted and CONF_RANK.get(bd.norm_conf(cur[2]), 2) >= CONF_RANK[u["confidence"]] \
                    and cur[1] != u["source"]:
                ops.append(("skip", addr, "already trusted as %s (%s/%s)" % (cur[0], cur[1], cur[2])))
                continue
            ev = u["evidence"] if cur[1] == u["source"] else \
                "%s | was %s/%s %s" % (u["evidence"], cur[1], cur[2], date)
            row = dict(u, evidence=ev)
            ops.append(("update", row, "same name, %s/%s -> %s/%s" % (cur[1], cur[2], u["source"], u["confidence"])))
            overlay[addr] = (row["name"], row["source"], row["confidence"], row["evidence"])
        else:
            if bd.is_trusted(cur[1], cur[2]):
                ops.append(("skip", addr, "CONFLICT: trusted holder %s (%s/%s) kept" % (cur[0], cur[1], cur[2])))
                continue
            ev = "%s | replaces %s (%s/%s) %s" % (u["evidence"], cur[0], cur[1], cur[2], date)
            row = dict(u, evidence=ev)
            ops.append(("update", row, "replaces %s (%s/%s)" % (cur[0], cur[1], cur[2])))
            overlay[addr] = (row["name"], row["source"], row["confidence"], row["evidence"])
    seen = set()
    for scope, topic, note in lessons:
        if (scope, topic) in seen:
            continue
        seen.add((scope, topic))
        if con.execute("select 1 from lessons where scope=? and topic=?", (scope, topic)).fetchone() is None:
            ops.append(("lesson", scope, topic, note))
    return ops


def apply_ops(con: sqlite3.Connection, ops: list[tuple], date: str) -> int:
    """Execute a plan; -> number of rows changed."""
    n = 0
    for op in ops:
        kind = op[0]
        if kind == "insert":
            u = op[1]
            con.execute("insert into name_map(steam_addr,name,source,confidence,evidence,date) values(?,?,?,?,?,?)",
                        (u["steam_addr"], u["name"], u["source"], u["confidence"], u["evidence"], u["date"]))
            n += 1
        elif kind == "update":
            u = op[1]
            n += con.execute("update name_map set name=?,source=?,confidence=?,evidence=?,date=? where steam_addr=?",
                             (u["name"], u["source"], u["confidence"], u["evidence"], u["date"], u["steam_addr"])
                             ).rowcount
        elif kind == "demote":
            n += con.execute("update name_map set confidence='wrong',evidence=?,date=? where steam_addr=?",
                             (op[2], date, op[1])).rowcount
        elif kind == "lesson":
            con.execute("insert into lessons(scope,topic,note,date) values(?,?,?,?)", (op[1], op[2], op[3], date))
            n += 1
    return n


def cmd_persist(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    try:
        upserts = json.loads((bdir / "kdb_upsert.json").read_text(encoding="utf-8"))
    except (OSError, ValueError):
        die("%s missing or invalid (run `finalize` first)" % (bdir / "kdb_upsert.json"))
    rejected = [r for r in iter_tsv(bdir / "final_rows.tsv") if r.get("final_verdict") == "rejected" and r.get("steam_addr")]
    lessons = parse_lessons(bdir / "gate_report.md")
    date = upserts[0]["date"] if upserts else today(args)
    if args.apply:
        if not paths.kdb.exists():
            die("knowledge DB %s does not exist" % paths.kdb)
        con = sqlite3.connect(str(paths.kdb), isolation_level=None)
    else:
        con = bd.connect_ro(paths.kdb)
    try:
        ops = plan_persist(con, upserts, rejected, lessons, args.batch, date)
        cnt = Counter(o[0] for o in ops)
        print("%s persist %s: insert=%d update=%d demote=%d lesson=%d skip=%d" % (
            args.batch, "APPLY" if args.apply else "DRY-RUN", cnt["insert"], cnt["update"], cnt["demote"],
            cnt["lesson"], cnt["skip"]))
        for o in ops:
            if o[0] in ("insert",):
                print("  insert %s %s (%s)" % (o[1]["steam_addr"], o[1]["name"], o[1]["confidence"]))
            elif o[0] == "update":
                print("  update %s %s: %s" % (o[1]["steam_addr"], o[1]["name"], o[2]))
            elif o[0] == "demote":
                print("  demote %s -> wrong" % o[1])
            elif o[0] == "lesson":
                print("  lesson [%s] %s" % (o[1], o[2]))
            else:
                print("  skip   %s: %s" % (o[1], o[2]))
        if not args.apply:
            print("dry-run: nothing written (pass --apply to write; lead only)")
            return 0
        con.execute("BEGIN")
        changed = apply_ops(con, ops, date)
        ok = con.execute("PRAGMA integrity_check").fetchone()[0]
        if ok != "ok":
            con.execute("ROLLBACK")
            die("integrity_check failed (%s); rolled back" % ok)
        con.execute("COMMIT")
        print("applied: %d rows changed; integrity_check: %s" % (changed, ok))
        return 0
    finally:
        con.close()


# ================================================================================================ CLI
def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("candidates", help="candidate Steam addresses per GOG function")
    p.add_argument("batch", type=batch_name)
    p.add_argument("--classes", default=None, help="comma-separated class names")
    p.add_argument("--from-branch", action="store_true", help="B0: audit the branch-DB rows")
    p.add_argument("--no-reexport", action="store_true", help="never refresh DUMP/inputs/name_map.tsv")
    p.add_argument("--findings-dir", default=None, help="where mapper output goes (default DUMP/findings)")
    p = sub.add_parser("gates", help="invariants, anchors, gate classes")
    p.add_argument("batch", type=batch_name)
    p.add_argument("--untrust", default=None, help="file of Steam addresses (hex, one per line) to drop from trust")
    p = sub.add_parser("cards", help="evidence cards for REVIEW rows")
    p.add_argument("batch", type=batch_name)
    p.add_argument("--chunk", type=int, default=60)
    p.add_argument("--decoys", type=int, default=None, help="calibration cards per chunk (default chunk//10)")
    p = sub.add_parser("finalize", help="merge reviews and overrides")
    p.add_argument("batch", type=batch_name)
    p.add_argument("--with-likely", action="store_true")
    p.add_argument("--date", default=None, help="date stamped into kdb_upsert.json (default today)")
    p = sub.add_parser("persist", help="dry-run (default) or --apply the name_map upsert")
    p.add_argument("batch", type=batch_name)
    p.add_argument("--apply", action="store_true")
    p.add_argument("--date", default=None)
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    fn = {"candidates": cmd_candidates, "gates": cmd_gates, "cards": cmd_cards, "finalize": cmd_finalize,
          "persist": cmd_persist}[args.cmd]
    return fn(args)


if __name__ == "__main__":
    sys.exit(main())
