#!/usr/bin/env python3
"""batch.py - Steam address-DB batch pipeline (contract: steam_db_batches/DESIGN.md section 4).

Subcommands (all deterministic; working dir per batch = $BD_ROOT/B<n>/):
  candidates B<n> --classes C1,C2 | --from-branch   -> candidates.tsv (+ mapper output under --findings-dir)
  gates      B<n> [--untrust FILE]                  -> evidence.tsv, gate_report.md (+ recheck.tsv candidates)
  cards      B<n> [--chunk 60] [--decoys N]         -> cards/chunk_<k>.md, cards/key.tsv
  finalize   B<n> [--with-likely] [--date D]        -> final_rows.tsv, functions.csv, kdb_upsert.json, recheck.tsv,
                                                       gog_label_suspects.tsv, opus_gate.md/json, validated.tsv
                                                       (B0 also: branch_deletes.tsv, branch_corrections.csv;
                                                       B0's validated.tsv comes from tools/branch_apply.py)
  opus-parts B<n> [--rows 100]                      -> opus/part_<x>.md + opus/parts.json (opus_gate.json rows split)
  persist    B<n> [--apply]                         -> dry-run plan; --apply (lead only) writes the knowledge DB
                                                       (name_map, lessons, steam_validated)
  check-validated [--branch-db DB] [--out FILE]     -> per branch-DB row: validated (skip) / stale / unvalidated

Only `persist --apply` ever writes a database, and only the one named by BD_KDB.
"""
from __future__ import annotations

import argparse
import bisect
import csv
import hashlib
import io
import json
import math
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
    "g6", "g7", "g8", "a_str", "a_callee", "a_sig", "a_srv", "a_vt", "families", "run_ok", "decoy_best", "gate_class",
    "review_ref", "flags",
]
CANDIDATE_COLS = ["class", "name", "gog", "gog_callconv", "gog_param", "gog_size", "gog_retn", "candidates"]
FUNCTIONS_CSV_HEADER = ["class_name", "function_name", "address", "calling_convention", "param_size_bytes", "notes"]
FINAL_VERDICTS = ("verified", "likely", "rejected", "unresolved", "label_suspect")
RECHECK_COLS = ["gog_addr", "corrected_addr", "source", "reason"]
LABEL_SUSPECT_COLS = ["gog_addr", "gog_name", "steam_counterpart", "body_fits_name", "source"]
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
                     srv_index: dict, from_branch: bool, near_k: int = 3,
                     extra: Optional[dict] = None) -> "OrderedDict[int, list[str]]":
    """All candidate Steam addresses of one GOG function with their source tags (section 4.1).
    `extra`: GOG addr -> [(steam addr, tag)] from `--extra-candidates` (e.g. the B15 command table)."""
    cands: "OrderedDict[int, list[str]]" = OrderedDict()

    def add(addr: int, tag: str) -> None:
        tags = cands.setdefault(addr, [])
        if tag not in tags:
            tags.append(tag)

    for s_addr, tag in (extra or {}).get(g.addr, []):
        add(s_addr, tag)

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


def parse_gog_range(spec: str) -> tuple[int, int]:
    """`lo:hi` (hex, 0x prefix optional): GOG addresses in [lo, hi).  die() on a bad format or an empty range."""
    m = re.fullmatch(r"\s*(?:0x)?([0-9a-fA-F]+)\s*:\s*(?:0x)?([0-9a-fA-F]+)\s*", spec or "")
    if not m:
        die("--gog-range %r: expected hex lo:hi, e.g. 0x400000:0x6a0000" % spec)
    lo, hi = int(m.group(1), 16), int(m.group(2), 16)
    if lo >= hi:
        die("--gog-range %r: lo must be below hi (lo inclusive, hi exclusive)" % spec)
    return lo, hi


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
    if args.name_regex:
        rows = [g for g in rows if re.search(args.name_regex, g.func)]
    if args.gog_range is not None:
        lo, hi = parse_gog_range(args.gog_range)
        rows = [g for g in rows if lo <= g.addr < hi]
    extra: dict[int, list] = {}
    if args.extra_candidates:
        for r in iter_tsv(Path(args.extra_candidates)):
            extra.setdefault(parse_addr(r["gog"]), []).append((parse_addr(r["steam"]), r["source"].strip()))
    if not args.from_branch:
        for c in classes:
            if not any(g.cls == c for g in rows):
                print("warning: class %s has no GOG functions in gog_functions.tsv (after --name-regex)" % c)
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
        cands = build_candidates(corpus, scorer, g, mapper, deltas.get(g.cls), srv_index, args.from_branch, extra=extra)
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
    twin_sig_max: float = 0.0           # same, over same-size/same-retn starts within +-8 starts (Opus gate R2)
    g7: str = "n/a"                     # Mac name contradiction check (Opus gate R3)
    g8: str = "n/a"                     # Mac rotation check: pass | n/a | rotation:<G'>:<fits> | rotation_in:<G'>:<fits>
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
        elif s in ("cmdtable", "cmdtable_overwritten"):      # engine dispatch table, equal slot sets (B15)
            v = 6
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
    elsewhere = corpus.trust.addrs_for_name(me) - {addr}
    parts = []
    if holders:
        parts.append("review:held_by=%s" % ",".join(sorted(holders)))
    if elsewhere:           # Opus gate R4: a trusted row already holds this name at another address
        parts.append(("" if parts else "review:") + "name_held_elsewhere=%s" % ",".join(hx8(a) for a in sorted(elsewhere)))
    if parts:
        return "+".join(parts), ""
    return "pass", ""


TWIN_WINDOW = 8


def sig_twins(corpus: bd.Corpus, eff: int) -> list[int]:
    """Steam starts within +-8 starts of eff with the same size and retn (Opus gate R2 twin window)."""
    ef = corpus.steam.get(eff)
    i = bisect.bisect_left(corpus.steam_starts, eff)
    if ef is None or i >= len(corpus.steam_starts) or corpus.steam_starts[i] != eff:
        return []
    lo, hi = max(0, i - TWIN_WINDOW), min(len(corpus.steam_starts), i + TWIN_WINDOW + 1)
    return [a for a in corpus.steam_starts[lo:hi]
            if a != eff and corpus.steam[a].size == ef.size and corpus.steam[a].retn == ef.retn]


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
    twins = sig_twins(corpus, eff)
    tsigs = [scorer.a_sig(g, d) for d in twins]
    ce.twin_sig_max = max([s.score for s in tsigs if s is not None], default=0.0)
    state, ov = corpus.mac_check(g, eff)
    ce.g7 = state if ov is None else "%s(%.2f)" % (state, ov)
    rstate, rdetail = corpus.mac_rotation(g)
    ce.g8 = rstate if not rdetail else "%s:%s" % (rstate, rdetail)
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


def sig_route_check(row: RowEval, run_ok: str, permuted: bool) -> str:
    """'' when the sig route holds, else the first failing condition ('twin' = only the R2 twin window fails)."""
    b = row.best
    if b is None or b.hard or b.thunk_unresolved or permuted or row.ambiguous or run_ok != "yes":
        return "base"
    if not (b.g1 == "pass" and b.g2 == "pass" and b.g3.startswith("pass") and b.g4 == "pass" and b.g6 == "pass"):
        return "invariant"
    if b.g7.startswith("mac_contra"):
        return "mac_contra"
    if b.g8.startswith("rotation"):
        return "mac_rotation"
    if [f for f in b.families if f != "VT"] != ["SIG"]:
        return "families"
    sig = b.fam.get("SIG")
    if sig is None or sig.score < SIG_ROUTE_MIN or b.decoy_sig_max > sig.score - SIG_ROUTE_MARGIN:
        return "sig"
    if b.contradictions or b.eff_size is None or b.gog_size is None or b.eff_size != b.gog_size:
        return "size"
    if b.twin_sig_max > sig.score - SIG_ROUTE_MARGIN:
        return "twin"
    return ""


def sig_route_ok(row: RowEval, run_ok: str, permuted: bool) -> bool:
    """Second AUTO route `auto_route=sig` (lead decision 2026-10-03, signed off with changes R1-R7).

    Every invariant passes (G7 may not be mac_contra), the only counting family (VT excluded) is SIG with score >=
    0.75, no decoy's SIG score exceeds ours minus 0.25 (decoys: previous / next starts, other candidates, and every
    start within +-8 with the same size and retn), run_ok is yes (mapper:vote does NOT help), the Steam and GOG
    function sizes are equal, and the row is neither permuted, ambiguous nor contradicted by a trusted callee.
    """
    return sig_route_check(row, run_ok, permuted) == ""


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
    if b.g7.startswith("mac_contra"):
        why.append("mac_contra")
    if b.g8.startswith("rotation_in"):
        why.append("mac_rotation_in")
    elif b.g8.startswith("rotation"):
        why.append("mac_rotation")
    if not why:
        return "AUTO", [], "c2"
    code = sig_route_check(row, run_ok, permuted)
    if code == "":
        return "AUTO", [], "sig"
    if code == "twin":
        why.append("sig_twin")
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
             g5_target=b.g5, g6=b.g6, g7=b.g7, g8=b.g8, a_str=fmt_score(b.fam.get("STR")), a_callee=fmt_score(b.fam.get("CALLEE")),
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
    out += ["", "## Trusted pair body conflicts", "",
            "Trusted rows that are NOT seeds (exact name + equal retn + equal size or A-SIG >= 0.5 with intersection >= 3 "
            "required, Opus gate R1). They stay trusted as G6 holders until Opus rules on them.", ""]
    conflicts = getattr(corpus, "pair_conflicts", [])
    out += ["- GOG %s %s -> Steam %s: %s" % (hx8(g), name, hx8(s), why) for g, s, name, why in conflicts] or ["(none)"]
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


def load_rechecks(bdir: Path) -> dict[int, list[int]]:
    """recheck.tsv (written by finalize from reviewer corrected_addr): GOG addr -> corrected Steam addresses."""
    out: dict[int, list[int]] = {}
    for r in iter_tsv(bdir / "recheck.tsv"):
        try:
            out.setdefault(parse_addr(r["gog_addr"]), []).append(parse_addr(r["corrected_addr"]))
        except (ValueError, KeyError):
            continue
    return out


def evaluate_batch(batch: str, corpus: bd.Corpus, scorer: bd.Scorer, crows: list[dict], rechecks: dict | None = None):
    """candidates.tsv rows -> (RowEval list, class assignment dict).  `rechecks` add re-gated candidates (R5)."""
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
        cands = parse_candidates(r.get("candidates", ""))
        for extra in (rechecks or {}).get(g.addr, []):
            if all(a != extra for a, _ in cands):
                cands.append((extra, ["recheck"]))
        rows.append(evaluate_row(corpus, scorer, g, cands))
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
    rows, classes = evaluate_batch(batch, corpus, scorer, crows, load_rechecks(bdir))
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
    s_callers = set(corpus.cg.callers.get(ce.eff, set()))
    g_callers = sorted(corpus.gog_callers().get(g.addr, set()) | corpus.gog_callers().get(corpus.resolve_gog(g.addr), set()))
    mapped_callers = [(c, corpus.pairs[c]) for c in g_callers if c in corpus.pairs]
    L.append("GOG callers mapped through trusted rows (%d of %d): the card's A-CALLEE credit may come from these, not from callees" % (
        len(mapped_callers), len(g_callers)))
    for c, sa in mapped_callers[:4]:
        gg = corpus.gog_by_addr.get(c)
        L.append("  0x%08x %s -> Steam %s  in candidate callers: %s" % (c, gg.name if gg else "unnamed", hx8(sa),
                                                                   "yes" if sa in s_callers else "no"))
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


def load_card_facts(bdir: Path) -> dict[int, list[str]]:
    """`B<n>/card_facts.tsv` (`gog fact`): GOG-side facts printed on every card of that GOG function (real and decoy
    alike, so they never reveal which Steam address is the counterpart)."""
    out: dict[int, list[str]] = {}
    for r in iter_tsv(bdir / "card_facts.tsv"):
        out.setdefault(parse_addr(r["gog"]), []).append(r["fact"])
    return out


def make_card(card_id: str, corpus: bd.Corpus, g: GogFn, ce: CandEval, ev_by_gog: dict, mapper_tag: str,
              card_facts: Optional[dict] = None) -> str:
    rp = corpus.rp
    root = str(corpus.paths.dump)
    L = ["## Card %s" % card_id, "",
         "GOG function: %s  gog %s  callconv=%s  param_size_bytes=%s  gog_size=%s  gog_retn=%s" % (
             g.name, hx8(g.addr), g.callconv or "(blank)", "?" if g.param is None else g.param,
             corpus.gog_size(g.addr), corpus.gog_retn(g.addr)),
         "Candidate Steam address: %s (size %s, retn %s)%s" % (
             hx8(ce.addr), ce.steam_size, ce.steam_retn,
             "  THUNK -> %s (everything below is judged on the target)" % hx8(ce.eff) if ce.thunk else ""),
         # candidate sources / mapper tag are withheld: decoy cards have none, so printing them marks the decoys
         "Candidate sources: (withheld on every card; judge the body)", "",
         "### Computed facts (script output, do not recompute)",
         "- G1 start: %s | G2 align: %s | G3 prologue: %s | G4 ret: %s | G5 thunk: %s | G6 dup: %s | G7 mac: %s | "
         "G8 mac_rotation: %s" % (ce.g1, ce.g2, ce.g3, ce.g4, ce.g5 or "none", ce.g6, ce.g7, ce.g8),
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
    for fact in (card_facts or {}).get(g.addr, []):
        L.append("- GOG-side fact: %s" % fact)
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


MIN_DECOYS = 10                  # per chunk, Opus gate 4.6; 0 tolerance


def same_retn_neighbours(corpus: bd.Corpus, addr: int) -> list[tuple[str, int]]:
    """[('decoy_prev', a), ('decoy_next', b)]: nearest same-retn Steam starts within 8 starts of addr."""
    f = corpus.steam.get(addr)
    i = bisect.bisect_left(corpus.steam_starts, addr)
    if f is None or i >= len(corpus.steam_starts) or corpus.steam_starts[i] != addr:
        return []
    out = []
    for kind, step in (("decoy_prev", -1), ("decoy_next", 1)):
        for k in range(1, 9):
            j = i + step * k
            if not 0 <= j < len(corpus.steam_starts):
                break
            a = corpus.steam_starts[j]
            if corpus.steam[a].retn == f.retn:
                out.append((kind, a))
                break
    return out


PROOF_SOURCES = ("cmdtable", "cmdtable_overwritten", "server", "recheck")


def is_proof_source(tag: str) -> bool:
    """True for a candidate source tag that is evidence the candidate is the real counterpart: the engine command
    table, a server name, a re-check, or a `name_map:high:<source>` row whose source is audited.  Never `near`,
    `mapper:*`, `branch`, or a medium / unaudited name_map row."""
    if tag in PROOF_SOURCES:
        return True
    parts = tag.split(":", 2)
    return len(parts) == 3 and parts[0] == "name_map" and parts[1] == "high" and not bd.is_unaudited_source(parts[2])


def proof_candidates(ev: dict) -> set:
    """Steam addresses that carry at least one proof source in the row's `cand_sources` cell."""
    out = set()
    for addr, srcs in parse_candidates(ev.get("cand_sources", "")):
        if any(is_proof_source(t) for t in srcs):
            out.add(addr)
    return out


def pick_decoy_cards(corpus: bd.Corpus, hosts: list[dict], need: int, rng: random.Random,
                     taken: set) -> list[tuple[str, dict, int]]:
    """Decoy cards: prev / next same-retn neighbours of REVIEW rows (hosts), never an address already carried and
    never an address that is a proof-source candidate of the host row itself (it may be the true function)."""
    out = []
    pool = list(hosts)
    rng.shuffle(pool)
    for ev in pool:
        cand = parse_addr(ev["steam_addr"])
        proven = proof_candidates(ev)
        options = [(k, a) for k, a in same_retn_neighbours(corpus, cand)
                   if (ev["gog"], a) not in taken and a not in proven]
        rng.shuffle(options)
        for kind, a in options:
            if len(out) >= need:
                return out
            taken.add((ev["gog"], a))
            out.append((kind, ev, a))
    return out


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
    facts = load_card_facts(bdir)
    review = [e for e in evs if e["gate_class"] == "REVIEW"]
    per_chunk_decoys = max(MIN_DECOYS, args.chunk // 6) if args.decoys is None else args.decoys
    per_chunk_real = max(1, args.chunk - per_chunk_decoys)
    chunks = [review[i:i + per_chunk_real] for i in range(0, len(review), per_chunk_real)]
    cards_dir = bdir / "cards"
    cards_dir.mkdir(parents=True, exist_ok=True)
    for old in list(cards_dir.glob("chunk_*.md")):
        old.unlink()
    key_rows, taken = [], set()
    for k, real in enumerate(chunks, 1):
        in_chunk = {e["gog"] for e in real}
        others = [e for e in review if e["gog"] not in in_chunk]     # hosts outside this chunk first
        decoys = pick_decoy_cards(corpus, others, per_chunk_decoys, rng, taken)
        if len(decoys) < per_chunk_decoys:
            decoys += pick_decoy_cards(corpus, real, per_chunk_decoys - len(decoys), rng, taken)
        items = [("real", e, None, "") for e in real] + [(kind, e, a, "mismatch") for kind, e, a in decoys]
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
            text.append(make_card(cid, corpus, g, ce, ev_by_gog, ce.mapper_tag if kind == "real" else "", facts))
            key_rows.append({"card_id": cid, "chunk": k, "gog": hx8(g.addr), "steam_addr": hx8(ce.addr),
                             "kind": kind, "expected": expected})
        (cards_dir / ("chunk_%d.md" % k)).write_text("\n".join(text), encoding="utf-8")
    write_tsv(cards_dir / "key.tsv", ["card_id", "chunk", "gog", "steam_addr", "kind", "expected"], key_rows)
    n_dec = Counter(r["chunk"] for r in key_rows if r["kind"] != "real")
    short = [k for k in range(1, len(chunks) + 1) if n_dec[k] < MIN_DECOYS]
    print("%s cards: %d REVIEW rows, %d chunks, %d decoy cards (key: %s)%s" % (
        args.batch, len(review), len(chunks), sum(n_dec.values()), cards_dir / "key.tsv",
        "; WARNING chunks %s carry fewer than %d decoys" % (short, MIN_DECOYS) if short else ""))
    return 0


# ================================================================================================ finalize
REVIEW_COLS = ["gog_addr", "steam_addr", "verdict", "corrected_addr", "content_anchor", "confidence", "reason"]


def load_reviews(bdir: Path) -> dict[int, list[dict]]:
    out: dict[int, list[dict]] = {}
    for f in sorted((bdir / "review").glob("*.tsv")):
        for r in iter_tsv(f):
            r["_file"] = f.name
            try:
                out.setdefault(parse_addr(r["gog_addr"]), []).append(r)
            except (ValueError, KeyError):
                continue
    return out


def decoy_pairs(bdir: Path) -> set:
    """(gog, steam) pairs of the decoy cards in cards/key*.tsv."""
    out = set()
    for kf in sorted((bdir / "cards").glob("key*.tsv")):
        for k in iter_tsv(kf):
            if (k.get("kind") or "").startswith("decoy"):
                try:
                    out.add((parse_addr(k["gog"]), parse_addr(k["steam_addr"])))
                except (ValueError, KeyError):
                    continue
    return out


def real_reviews(reviews: dict, decoys: set) -> dict:
    """Drop the reviewer rows that answer a decoy card: they score the decoys only, and must never drive a row's
    verdict, a recheck or the Opus 100% list (B15: 18 of 25 'reviewer corrected_addr' rows were decoy rejections)."""
    def is_decoy(ga: int, r: dict) -> bool:
        try:
            return bool(r.get("steam_addr")) and (ga, parse_addr(r["steam_addr"])) in decoys
        except ValueError:
            return False
    return {ga: [r for r in rows if not is_decoy(ga, r)] for ga, rows in reviews.items()}


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
    suspects = [r for r in mine if r.get("verdict") == "label_suspect" and (r.get("content_anchor") or "").strip()]
    if suspects and mism:
        return "unresolved", cand, "reviewers disagree (label_suspect vs mismatch)"
    if suspects:            # counterpart confirmed, GOG name contradicts the body: recorded, never shipped
        return "label_suspect", cand, "reviewer label_suspect: %s" % (suspects[0].get("reason") or "")[:160]
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
        if fixed:       # Opus gate R5: never `likely` by itself; the corrected address is re-gated (recheck.tsv)
            return "unresolved", cand, "reviewer mismatch with corrected_addr %s: recheck (re-gate or Opus override): %s" % (
                hx8(parse_addr(fixed[0]["corrected_addr"])), (fixed[0].get("reason") or "")[:140])
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
    if gate == "AUTO":      # a reviewer who looked at an AUTO row can still flag the GOG label
        v2, _s, n2 = decide_review(ev, reviews.get(ga, []))
        if v2 == "label_suspect":
            verdict, note = v2, n2
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


def csv_safe_note(note: str) -> str:
    """SqliteTools' CSV reader is line-based and drops every `"`: keep notes on one line and quote-free."""
    return " ".join(note.replace('"', "'").split())


def shipped_anchor(r: dict) -> tuple[str, str]:
    """-> (anchor text, evidence file) for a shipped row.  AUTO rows cite the gate evidence; reviewed or overridden
    rows cite their final note, because the gate evidence describes the gate's pick, which an Opus correction
    replaced (B15: ChangeFaction 0x66F0A0 carried the C0 evidence of the wrong pick 0x66F330)."""
    note = (r.get("final_note") or "").strip()
    if note.startswith("override:"):
        return "Opus: " + note[len("override:"):].strip(), "final_rows.tsv"
    if r.get("gate_class") != "AUTO" and note:
        return note, "final_rows.tsv"
    return r.get("evidence", ""), "evidence.tsv"


def write_functions_csv(path: Path, rows: list[dict], batch: str, steam: dict) -> int:
    buf = io.StringIO()
    w = csv.writer(buf, lineterminator="\n")
    w.writerow(FUNCTIONS_CSV_HEADER)
    n = 0
    for r in rows:
        cc, param, extra = finalize_params(r, steam)
        anchor, ev_file = shipped_anchor(r)
        notes = csv_safe_note("Steam batch %s: %s (evidence steam_db_batches/%s/%s)%s" % (
            batch, anchor, batch, ev_file, extra))
        w.writerow([r["class"], r["name"], "0x%08X" % parse_addr(r["steam_addr"]), "__" + cc, param, notes])
        n += 1
    path.write_text(buf.getvalue(), encoding="utf-8")
    return n


# ---------------------------------------------------------------------------------------------- Opus gate 4.6
LAYOUT_RE = re.compile(r"layout|offset (?:shift|differ)|shifted|enlarg|grew|alloc", re.I)


def auto_route_of(ev: dict) -> str:
    for part in (ev.get("flags") or "").split(";"):
        if part.startswith("auto_route="):
            return part.split("=", 1)[1]
    return ""


def prior_batches_clean(bdir: Path) -> int:
    """Number of consecutive clean batches (opus_gate.json `clean`) immediately before this one."""
    m = re.fullmatch(r"B(\d+)", bdir.name)
    if not m:
        return 0
    cur = int(m.group(1))
    prior = sorted((int(re.fullmatch(r"B(\d+)", p.name).group(1)), p) for p in bdir.parent.iterdir()
                   if re.fullmatch(r"B\d+", p.name) and int(p.name[1:]) < cur and (p / "opus_gate.json").exists())
    run_len = 0
    for _n, p in reversed(prior):
        try:
            clean = json.loads((p / "opus_gate.json").read_text()).get("clean")
        except (OSError, ValueError):
            clean = False
        if not clean:
            break
        run_len += 1
    return run_len


def opus_gate(bdir: Path, batch: str, evs: list[dict], finals: list[dict], reviews: dict,
              sample_errors: Optional[int], partial: bool = False, overrides: Optional[dict] = None) -> dict:
    """DESIGN 4.6: score the decoy cards, decide the escalation, list what Opus must read.  Writes opus_gate.md/json.
    `overrides` (load_overrides): an accepted decoy that equals an Opus-shipped (verified/likely) override pair is a
    key error (the decoy was the true counterpart), not a decoy accept."""
    ev_by = {parse_addr(e["gog"]): e for e in evs}
    rev_by_pair: dict[tuple[int, int], list[dict]] = {}
    for ga, rows in reviews.items():
        for r in rows:
            if r.get("steam_addr"):
                rev_by_pair.setdefault((ga, parse_addr(r["steam_addr"])), []).append(r)
    key_rows = []
    for kf in sorted((bdir / "cards").glob("key*.tsv")):
        key_rows += list(iter_tsv(kf))

    def accepted_by_reviewer(k: dict) -> bool:
        pair = (parse_addr(k["gog"]), parse_addr(k["steam_addr"]))
        return any(bd.reviewer_accepts(r, pair[1]) for r in rev_by_pair.get(pair, []))

    chunks = sorted({k["chunk"] for k in key_rows}, key=bd.chunk_key)
    reviewed = {k["chunk"] for k in key_rows if (parse_addr(k["gog"]), parse_addr(k["steam_addr"])) in rev_by_pair}
    decoys = [k for k in key_rows if k["kind"].startswith("decoy") and k["chunk"] in reviewed]
    shipped_pairs = set()
    for ga, ov in (overrides or {}).items():
        if (ov.get("final_verdict") or "").strip() in ("verified", "likely") and (ov.get("steam_addr") or "").strip():
            try:
                shipped_pairs.add((ga, parse_addr(ov["steam_addr"])))
            except ValueError:
                continue

    def is_key_error(k: dict) -> bool:
        return (parse_addr(k["gog"]), parse_addr(k["steam_addr"])) in shipped_pairs

    reviewer_accepted = [k for k in decoys if accepted_by_reviewer(k)]
    key_errors = [k for k in reviewer_accepted if is_key_error(k)]
    accepted = [k for k in reviewer_accepted if not is_key_error(k)]
    # a key-error decoy was the true counterpart, so it is not a valid control: it counts toward no chunk's minimum
    valid_decoys = [k for k in decoys if not is_key_error(k)]
    per_chunk = Counter(k["chunk"] for k in valid_decoys)
    unanswered = [k for k in decoys if (parse_addr(k["gog"]), parse_addr(k["steam_addr"])) not in rev_by_pair]
    gate = bd.decoy_gate(chunks, reviewed, {c: [accepted_by_reviewer(k) for k in valid_decoys if k["chunk"] == c]
                                            for c in reviewed}, MIN_DECOYS, partial)
    short = gate["short"]
    # rows that Opus reads in full
    full, plain = [], []
    real = real_reviews(reviews, decoy_pairs(bdir))     # decoy answers score the decoys only, never the 100% list
    for f in finals:
        ga = parse_addr(f["gog"])
        rvs = real.get(ga, [])
        ev = ev_by.get(ga, {})
        why = []
        if any((r.get("corrected_addr") or "").strip() for r in rvs):
            why.append("reviewer corrected_addr")
        if f["final_verdict"] == "label_suspect":
            why.append("label_suspect")
        matched = [r for r in rvs if r.get("verdict") == "match"]
        if matched:
            if any(LAYOUT_RE.search((r.get("reason") or "") + " " + (r.get("content_anchor") or "")) for r in matched):
                why.append("match cites a layout/offset difference")
            flags = ev.get("flags", "")
            if "mac_contra" in flags:
                why.append("match on a mac_contra row")
            if "sig_twin" in flags:
                why.append("match on a twin row")
            if "name_held_elsewhere" in ev.get("g6", ""):
                why.append("match on a name_held_elsewhere row")
        if why:
            full.append((f, ", ".join(why)))
        elif matched and ev.get("gate_class") == "REVIEW":
            plain.append(f)
    if not chunks and (full or plain):
        short = ["(no cards/key.tsv: nothing was scored)"]
    unreviewed = gate["unreviewed"]
    escalate = bool(accepted) or bool(sample_errors)
    clean_streak = prior_batches_clean(bdir)
    rate = 0.10 if clean_streak >= 3 else 0.20
    rng = random.Random("opus-" + batch)
    plain.sort(key=lambda f: (ev_by[parse_addr(f["gog"])].get("content_level") != "C0", rng.random()))
    n_sample = len(plain) if escalate else min(len(plain), max(5, math.ceil(rate * len(plain))))
    sample = plain[:n_sample]
    autos = [e for e in evs if e["gate_class"] == "AUTO"]
    sigs = [e for e in autos if auto_route_of(e) == "sig"]
    c2s = [e for e in autos if auto_route_of(e) == "c2"]
    rng.shuffle(sigs)
    rng.shuffle(c2s)
    if escalate:
        decision = ("ESCALATE: %s -> send 100%% of this batch's reviewer matches to Opus"
                    % ("decoy cards accepted" if accepted else "Opus-found error in the sample"))
    elif not chunks and short:
        decision = "INSUFFICIENT DECOYS in chunks %s (need >= %d per chunk, 0 tolerance): re-run `cards`" % (short, MIN_DECOYS)
    elif gate["decision"].startswith("INSUFFICIENT"):
        decision = gate["decision"]
    else:
        decision = "OK: Opus reads the 100%% classes plus a %d%% sample (min 5) of the other reviewer matches%s" % (
            int(rate * 100), " (partial: chunks %s unreviewed)" % ", ".join(unreviewed) if unreviewed else "")
    if key_errors and decision.startswith("OK"):
        decision += "; %d decoy key error(s) excluded (decoy = Opus-shipped counterpart)" % len(key_errors)
    rows_full = [{"class": f["class"], "name": f["name"], "gog": f["gog"], "steam_addr": f["steam_addr"], "why": w}
                 for f, w in full]
    rows_sample = [{"class": f["class"], "name": f["name"], "gog": f["gog"], "steam_addr": f["steam_addr"],
                    "content_level": ev_by[parse_addr(f["gog"])].get("content_level")} for f in sample]
    rows_auto = [{"route": r, "class": e["class"], "name": e["name"], "gog": e["gog"], "steam_addr": e["steam_addr"]}
                 for r, group in (("sig", sigs[:5]), ("c2", c2s[:3])) for e in group]
    result = {"batch": batch, "decoy_cards": len(decoys), "decoys_accepted": len(accepted),
              "key_errors": len(key_errors), "key_error_cards": [k["card_id"] for k in key_errors],
              "rows_full": rows_full, "rows_sample": rows_sample, "rows_auto": rows_auto,
              "decoys_per_chunk": dict(per_chunk), "chunks_short": short, "chunks_unreviewed": unreviewed,
              "unanswered_decoys": len(unanswered),
              "full_read": len(full), "sample_pool": len(plain), "sample_size": len(sample), "sample_rate": rate,
              "sample_errors": sample_errors, "decision": decision, "clean_streak_before": clean_streak,
              "clean": (not escalate and not short and not unreviewed and sample_errors == 0)}
    L = ["# Opus gate %s (DESIGN 4.6)" % batch, "", "Decision: **%s**" % decision, "",
         "- decoy cards: %d (per chunk %s), accepted by the reviewer: %d, unanswered: %d" % (
             len(decoys), dict(per_chunk), len(accepted), len(unanswered)),
         "- chunks not reviewed: %s%s" % (", ".join(unreviewed) or "none", " (--partial)" if unreviewed and partial else ""),
         "- Opus-found sample errors: %s (pass `finalize --opus-sample-errors N` after the Opus pass)" % (
             "pending" if sample_errors is None else sample_errors),
         "- consecutive clean batches before this one: %d -> sample rate %d%%" % (clean_streak, int(rate * 100)), "",
         "## Decoy key errors (decoy = Opus-shipped counterpart)", ""]
    L += ["- %s %s -> %s (%s): the decoy is the Opus-shipped counterpart, so the reviewer was right; fix the key" % (
        k["card_id"], k["gog"], k["steam_addr"], k["kind"]) for k in key_errors] or ["None."]
    L += ["", "## Decoys accepted (0 tolerance)", ""]
    L += ["- %s %s -> %s (%s)" % (k["card_id"], k["gog"], k["steam_addr"], k["kind"]) for k in accepted] or ["None."]
    L += ["", "## Opus reads 100%% (%d rows)" % len(full), ""]
    L += ["- %s::%s gog %s -> %s: %s" % (f["class"], f["name"], f["gog"], f["steam_addr"], w) for f, w in full] or ["None."]
    L += ["", "## Sample of the other reviewer matches (%d of %d)" % (len(sample), len(plain)), ""]
    L += ["- %s::%s gog %s -> %s (%s)" % (f["class"], f["name"], f["gog"], f["steam_addr"],
                                         ev_by[parse_addr(f["gog"])].get("content_level")) for f in sample] or ["None."]
    L += ["", "## AUTO meaning checks (5 sig + 3 c2)", ""]
    L += ["- sig: %s::%s gog %s -> %s" % (e["class"], e["name"], e["gog"], e["steam_addr"]) for e in sigs[:5]]
    L += ["- c2: %s::%s gog %s -> %s" % (e["class"], e["name"], e["gog"], e["steam_addr"]) for e in c2s[:3]]
    if not sigs[:5] and not c2s[:3]:
        L.append("None.")
    (bdir / "opus_gate.md").write_text("\n".join(L) + "\n", encoding="utf-8")
    (bdir / "opus_gate.json").write_text(json.dumps(result, indent=1, sort_keys=True) + "\n", encoding="utf-8")
    return result


def cmd_finalize(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    evs = list(iter_tsv(bdir / "evidence.tsv"))
    if not evs:
        die("%s missing or empty (run `gates` first)" % (bdir / "evidence.tsv"))
    all_reviews, overrides = load_reviews(bdir), load_overrides(bdir)
    reviews = real_reviews(all_reviews, decoy_pairs(bdir))      # decoy answers only score the decoys (opus_gate)
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
        corrected = r.get("cand_status") == "corrected"     # gate evidence/retn describe the replaced pick
        sf = steam.get(parse_addr(r["steam_addr"]))
        ev = "[%s; %s retn=%s; GOG %s] Steam batch %s: %s | %s" % (
            r["final_verdict"], r.get("calling_convention", ""),
            (sf.retn if sf is not None else "?") if corrected else r.get("steam_retn", ""), r["gog"], args.batch,
            "gate evidence was for a corrected pick" if corrected else r.get("evidence", ""), r.get("final_note", ""))
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
    rechecks = []
    for r in finals:
        ga = parse_addr(r["gog"])
        if r["final_verdict"] != "unresolved":
            continue
        for x in reviews.get(ga, []):
            if x.get("verdict") == "mismatch" and (x.get("corrected_addr") or "").strip():
                rechecks.append({"gog_addr": r["gog"], "corrected_addr": hx8(parse_addr(x["corrected_addr"])),
                                 "source": "review:%s" % x.get("_file", "?"), "reason": (x.get("reason") or "")[:160]})
    write_tsv(bdir / "recheck.tsv", RECHECK_COLS, rechecks)
    suspects = []
    for r in finals:
        if r["final_verdict"] != "label_suspect":
            continue
        ga = parse_addr(r["gog"])
        ov = overrides.get(ga)
        if ov and ov["final_verdict"] == "label_suspect":
            src, fits = "override", ov.get("note") or ""
        else:
            rv = next((x for x in reviews.get(ga, []) if x.get("verdict") == "label_suspect"), {})
            src, fits = "review:%s" % rv.get("_file", "?"), rv.get("reason") or ""
        counterpart = (rv.get("corrected_addr") or "").strip() if not (ov and ov["final_verdict"] == "label_suspect") else ""
        suspects.append({"gog_addr": r["gog"], "gog_name": "%s::%s" % (r["class"], r["name"]),
                         "steam_counterpart": hx8(parse_addr(counterpart)) if counterpart else r["steam_addr"], "body_fits_name": fits, "source": src})
    write_tsv(bdir / "gog_label_suspects.tsv", LABEL_SUSPECT_COLS, suspects)
    if args.batch != "B0":      # B0 ships the branch (KPM) names: tools/branch_apply.py writes its validated.tsv
        write_validated(bdir, args.batch, [
            {"steam_addr": r["steam_addr"], "name": "%s::%s" % (r["class"], r["name"]), "verdict": r["final_verdict"],
             "tier": gate_tier(r), "gog_addr": r["gog"],
             "evidence_ref": "steam_db_batches/%s/final_rows.tsv" % args.batch} for r in export], paths)
    cnt = Counter(r["final_verdict"] for r in finals)
    print("%s finalize: %s; functions.csv rows=%d; kdb_upsert rows=%d; rechecks=%d" % (
        args.batch, "  ".join("%s=%d" % (k, cnt[k]) for k in FINAL_VERDICTS), n_csv, len(upserts), len(rechecks)))
    gate = opus_gate(bdir, args.batch, evs, finals, all_reviews, args.opus_sample_errors, args.partial, overrides)
    print("opus gate: decoy cards %d (accepted %d, key errors %d); %s" % (
        gate["decoy_cards"], gate["decoys_accepted"], gate["key_errors"], gate["decision"]))
    return 0


# ================================================================================================ opus-parts
def part_letters(n: int) -> list[str]:
    """a, b, ..., z, aa, ab, ... (n names)."""
    out = []
    for i in range(n):
        x, name = i, ""
        while True:
            name = chr(97 + x % 26) + name
            x = x // 26 - 1
            if x < 0:
                break
        out.append(name)
    return out


def opus_rows(gate: dict) -> list[dict]:
    """opus_gate.json row lists in the order of opus_gate.md: 100% -> sample -> AUTO (sig then c2)."""
    rows = []
    for r in gate["rows_full"]:
        rows.append({"section": "100%", "class": r["class"], "name": r["name"], "gog": r["gog"],
                     "steam_addr": r["steam_addr"], "note": r.get("why", "")})
    for r in gate["rows_sample"]:
        rows.append({"section": "sample", "class": r["class"], "name": r["name"], "gog": r["gog"],
                     "steam_addr": r["steam_addr"], "note": r.get("content_level") or ""})
    for r in gate["rows_auto"]:
        rows.append({"section": "AUTO %s" % r["route"], "class": r["class"], "name": r["name"], "gog": r["gog"],
                     "steam_addr": r["steam_addr"], "note": ""})
    return rows


def cmd_opus_parts(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    jf = bdir / "opus_gate.json"
    if not jf.exists():
        die("%s missing: run `finalize %s` first" % (jf, args.batch))
    try:
        gate = json.loads(jf.read_text(encoding="utf-8"))
    except ValueError as e:
        die("%s unreadable (%s): re-run finalize" % (jf, e))
    if not all(k in gate for k in ("rows_full", "rows_sample", "rows_auto")):
        die("%s lacks the row lists (rows_full / rows_sample / rows_auto): re-run finalize" % jf)
    if args.rows < 1:
        die("--rows must be >= 1")
    rows = opus_rows(gate)
    chunks = [rows[i:i + args.rows] for i in range(0, len(rows), args.rows)]
    letters = part_letters(len(chunks))
    out_dir = bdir / "opus"
    out_dir.mkdir(parents=True, exist_ok=True)
    for old in list(out_dir.glob("part_*.md")):
        old.unlink()
    parts, base = [], 1
    for k, (letter, part) in enumerate(zip(letters, chunks), 1):
        L = ["# Opus gate %s part %s (%d of %d)" % (args.batch, letter, k, len(chunks)), "",
             "- batch: %s, part: %s, rows: %d (rows %d-%d of %d in the full -> sample -> AUTO order of opus_gate.md)" % (
                 args.batch, letter, len(part), base, base + len(part) - 1, len(rows)),
             "- Decide every row listed here; output per OPUS_GATE_BRIEF.md.", "",
             "| # | section | Class::Name | GOG addr | Steam addr | why / content level |",
             "|---|---|---|---|---|---|"]
        for i, r in enumerate(part, base):
            L.append("| %d | %s | %s::%s | %s | %s | %s |" % (
                i, r["section"], r["class"], r["name"], r["gog"], r["steam_addr"], (r["note"] or "").replace("|", "/")))
        fn = "part_%s.md" % letter
        (out_dir / fn).write_text("\n".join(L) + "\n", encoding="utf-8")
        parts.append({"part": letter, "file": "opus/" + fn, "n_rows": len(part), "gog_addrs": [r["gog"] for r in part]})
        base += len(part)
    (out_dir / "parts.json").write_text(json.dumps(
        {"batch": args.batch, "rows_per_part": args.rows, "total_rows": len(rows), "parts": parts},
        indent=1) + "\n", encoding="utf-8")
    print("%s opus-parts: %d rows -> %d part(s) of <= %d in %s" % (args.batch, len(rows), len(parts), args.rows, out_dir))
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


def load_kdb_overrides(bdir: Path) -> list[dict]:
    """`B<n>/kdb_overrides.tsv` (steam_addr action name note): Opus rulings on existing KDB rows (demote | rename)."""
    out = []
    for r in iter_tsv(bdir / "kdb_overrides.tsv"):
        action = (r.get("action") or "").strip()
        if action not in ("demote", "rename"):
            die("kdb_overrides.tsv: action must be demote or rename, got %r for %s" % (action, r.get("steam_addr")))
        if action == "rename" and not (r.get("name") or "").strip():
            die("kdb_overrides.tsv: rename of %s needs a name" % r.get("steam_addr"))
        out.append(dict(r, steam_addr=hx8(parse_addr(r["steam_addr"])), action=action))
    return out


def plan_persist(con: sqlite3.Connection, upserts: list[dict], rejected: list[dict], lessons: list, batch: str,
                 date: str, opus_rejected: set | None = None, kdb_overrides: list | None = None,
                 suspects: list | None = None) -> list[tuple]:
    """Compute the exact change list against the current name_map (no writes).

    Ops: ('insert', row) ('update', row, why) ('demote', addr, evidence) ('rename', addr, new_name, evidence)
         ('lesson', scope, topic, note) ('skip', addr, why).
    Order: kdb_overrides (Opus demote / rename, trusted rows included), label_suspect demotions, rejected-row
    demotions, then the upserts, with an overlay so the plan equals what apply does.  Rows already in the target
    state produce no op (idempotent).
    """
    source = "steam-db-%s" % batch
    overlay: dict[str, tuple] = {}

    def current(addr: str):
        if addr in overlay:
            return overlay[addr]
        return con.execute("select name,source,confidence,evidence from name_map where steam_addr=?",
                           (addr,)).fetchone()

    opus_rejected = opus_rejected or set()

    def trusted_holders(name: str, except_addr: str) -> list[str]:
        """Addresses where a trusted row (db or this plan) already holds this name (Opus gate R4)."""
        found = {}
        for a, n, src, conf in con.execute("select steam_addr,name,source,confidence from name_map where name like ?",
                                           (base_name(name) + "%",)):
            found[a] = (n, src, conf)
        for a, (n, src, conf, _e) in overlay.items():
            found[a] = (n, src, conf)
        return sorted(a for a, (n, src, conf) in found.items()
                      if a != except_addr and base_name(n) == base_name(name) and bd.is_trusted(src, conf))

    ops: list[tuple] = []
    named = set()
    for o in kdb_overrides or []:                     # 1. Opus rulings on existing rows
        addr = o["steam_addr"]
        named.add(addr)
        cur = current(addr)
        note = (o.get("note") or "").strip()
        if cur is None:
            ops.append(("skip", addr, "kdb_overrides %s: no name_map row at this address" % o["action"]))
        elif cur[1] == source:
            continue            # the row was written by this batch (an earlier run replaced the row the override ruled on)
        elif o["action"] == "demote":
            if cur[2] != "wrong":
                ev = "%s | [demoted %s by Opus %s: %s]" % (cur[3] or "", date, batch, note)
                ops.append(("demote", addr, ev))
                overlay[addr] = (cur[0], cur[1], "wrong", ev)
        elif base_name(cur[0]) != base_name(o["name"]) or cur[0].strip() != o["name"].strip():
            ev = "%s | [renamed %s by Opus %s: was %s (%s/%s): %s]" % (cur[3] or "", date, batch, cur[0], cur[1], cur[2], note)
            ops.append(("rename", addr, o["name"].strip(), ev))
            overlay[addr] = (o["name"].strip(), cur[1], cur[2], ev)
    for s_ in suspects or []:                         # 2. KDB rows carrying a GOG name the batch proved wrong
        addr = hx8(parse_addr(s_["steam_addr"]))
        gname = "%s::%s" % (s_["class"], s_["name"])
        cur = current(addr)
        if cur is None or base_name(cur[0]) != gname or cur[2] == "wrong" or addr in named:
            continue
        if bd.is_trusted(cur[1], cur[2]):
            ops.append(("skip", addr, "label_suspect: trusted row %s (%s/%s) carries the GOG name %s that this batch proved "
                                      "wrong; needs a kdb_overrides.tsv row" % (cur[0], cur[1], cur[2], gname)))
        else:
            ev = "%s | [demoted %s by %s: label_suspect, the body is not %s]" % (cur[3] or "", date, "steam-db-" + batch, gname)
            ops.append(("demote", addr, ev))
            overlay[addr] = (cur[0], cur[1], "wrong", ev)
    for r in rejected:
        addr = hx8(parse_addr(r["steam_addr"]))
        name = "%s::%s" % (r["class"], r["name"])
        cur = current(addr)
        if cur and base_name(cur[0]) == name and bd.is_trusted(cur[1], cur[2]) and addr not in opus_rejected \
                and cur[1] != source:
            ops.append(("skip", addr, "R4: trusted row %s (%s/%s) is demoted only by an Opus overrides.tsv row naming "
                                      "this address" % (cur[0], cur[1], cur[2])))
        elif cur and base_name(cur[0]) == name and cur[2] != "wrong" and cur[1] != source:
            ev = "%s | demoted by %s %s: rejected (%s)" % (cur[3] or "", source, date, (r.get("final_note") or "")[:160])
            ops.append(("demote", addr, ev))
            overlay[addr] = (cur[0], cur[1], "wrong", ev)
    for u in upserts:
        addr = u["steam_addr"]
        cur = current(addr)
        new = (u["name"], u["source"], u["confidence"], u["evidence"])
        held = trusted_holders(u["name"], addr)
        if held:
            ops.append(("skip", addr, "R4: %s is already held by a trusted row at %s; no second address" % (
                u["name"], ",".join(held))))
            continue
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
        elif kind == "rename":
            n += con.execute("update name_map set name=?,evidence=?,date=? where steam_addr=?",
                             (op[2], op[3], date, op[1])).rowcount
        elif kind == "lesson":
            con.execute("insert into lessons(scope,topic,note,date) values(?,?,?,?)", (op[1], op[2], op[3], date))
            n += 1
        elif kind == "mark":
            m = op[1]
            con.execute("insert or replace into steam_validated(%s,date) values(%s)" % (
                ",".join(VALIDATED_COLS), ",".join("?" * (len(VALIDATED_COLS) + 1))),
                tuple(m.get(c, "") for c in VALIDATED_COLS) + (date,))
            n += 1
        elif kind == "unmark":
            n += con.execute("delete from steam_validated where steam_addr=? and name=?", (op[1], op[2])).rowcount
    return n


# ================================================================================================ validated ledger
# Every row a batch validated (shipped verdict verified / likely, or B0 GOG parity) is marked, so a later final
# validation pass reads only what is not marked: `check-validated` lists branch-DB rows as validated (skip) /
# stale (the mark is for another address, or the Steam body hash changed) / unvalidated.
VALIDATED_COLS = ["steam_addr", "name", "verdict", "tier", "gog_addr", "batch", "fp_sha", "evidence_ref"]
VALIDATED_DDL = ("CREATE TABLE IF NOT EXISTS steam_validated(steam_addr TEXT NOT NULL, name TEXT NOT NULL, "
                 "verdict TEXT NOT NULL, tier TEXT, gog_addr TEXT, batch TEXT NOT NULL, fp_sha TEXT, evidence_ref TEXT, "
                 "date TEXT, PRIMARY KEY(steam_addr, name))")
FP_SHA_FIELDS = ("size", "retn", "n_insn", "thunk_target", "callees", "str_refs", "disp", "imm")
CHECK_COLS = ["steam_addr", "name", "status", "verdict", "tier", "batch", "note"]


def steam_fp_shas(paths) -> dict[int, str]:
    """Steam address -> short sha1 of its fp.tsv body cells (a mark whose body hash changed is stale)."""
    out = {}
    for r in iter_tsv(paths.server / paths.steam_program / "fp.tsv"):
        try:
            a = parse_addr(r["addr"])
        except (ValueError, KeyError):
            continue
        out[a] = hashlib.sha1("|".join(r.get(k, "") for k in FP_SHA_FIELDS).encode()).hexdigest()[:16]
    return out


def gate_tier(r: dict) -> str:
    """Who validated a final row: opus (override) / auto:c2 / auto:sig / reviewer."""
    if (r.get("final_note") or "").startswith("override"):
        return "opus"
    if r.get("gate_class") == "AUTO":
        m = re.search(r"auto_route=(\w+)", r.get("flags") or "")
        return "auto:%s" % (m.group(1) if m else "c2")
    return "reviewer"


def write_validated(bdir: Path, batch: str, rows: list[dict], paths=None) -> int:
    """`B<n>/validated.tsv`: one row per validated (address, shipped name), with the Steam body hash."""
    shas = steam_fp_shas(paths or bd.get_paths())
    out = {}
    for r in rows:
        a = parse_addr(r["steam_addr"])
        out[(hx8(a), r["name"])] = dict(r, steam_addr=hx8(a), batch=batch, fp_sha=shas.get(a, ""))
    write_tsv(bdir / "validated.tsv", VALIDATED_COLS, [out[k] for k in sorted(out)])
    return len(out)


def load_marks(con: sqlite3.Connection) -> dict[tuple, tuple]:
    """(steam_addr, name) -> row of VALIDATED_COLS ('' for NULL); empty when the table does not exist yet."""
    if not con.execute("select 1 from sqlite_master where type='table' and name='steam_validated'").fetchone():
        return {}
    return {(r[0], r[1]): tuple(x or "" for x in r)
            for r in con.execute("select %s from steam_validated" % ",".join(VALIDATED_COLS))}


def plan_marks(con: sqlite3.Connection, marks: list[dict], batch: str) -> list[tuple]:
    """Ops ('mark', row) / ('unmark', addr, name, why). A name is marked at one address only; this batch's
    earlier marks that it no longer validates are removed. Rows already in the target state produce no op."""
    have = load_marks(con)
    want = {(m["steam_addr"], m["name"]): tuple(m.get(c, "") for c in VALIDATED_COLS) for m in marks}
    at = {n: a for a, n in want}
    ops = [("mark", dict(zip(VALIDATED_COLS, v))) for k, v in sorted(want.items()) if have.get(k) != v]
    for (a, n), row in sorted(have.items()):
        if (a, n) in want:
            continue
        if n in at:
            ops.append(("unmark", a, n, "%s now validated at %s" % (n, at[n])))
        elif row[VALIDATED_COLS.index("batch")] == batch:
            ops.append(("unmark", a, n, "no longer validated by %s" % batch))
    return ops


def cmd_check_validated(args) -> int:
    paths = bd.get_paths()
    bcon = bd.connect_ro(Path(args.branch_db) if args.branch_db else paths.branchdb)
    kcon = bd.connect_ro(paths.kdb)
    try:
        branch = bcon.execute("select class_name,function_name,address from functions order by address").fetchall()
        marks = load_marks(kcon)
    finally:
        bcon.close()
        kcon.close()
    by_name: dict[str, list[str]] = {}
    for a, n in marks:
        by_name.setdefault(n, []).append(a)
    shas = steam_fp_shas(paths)
    ix = {c: i for i, c in enumerate(VALIDATED_COLS)}
    out = []
    for cls, fn, addr in branch:
        a, name = hx8(addr), "%s::%s" % (cls, fn)
        m = marks.get((a, name))
        row = {"steam_addr": a, "name": name, "verdict": "", "tier": "", "batch": "", "note": ""}
        if m:
            row.update(verdict=m[ix["verdict"]], tier=m[ix["tier"]], batch=m[ix["batch"]])
            if not m[ix["fp_sha"]]:
                row.update(status="nohash", note="mark has no body hash; re-check")
            elif shas.get(addr, "") != m[ix["fp_sha"]]:
                row.update(status="stale", note="Steam body hash changed since %s" % m[ix["batch"]])
            else:
                row["status"] = "validated"
        elif name in by_name:
            row.update(status="stale", note="validated at %s, branch has %s" % (",".join(sorted(by_name[name])), a))
        else:
            row["status"] = "unvalidated"
        out.append(row)
    if args.out:
        write_tsv(Path(args.out), CHECK_COLS, out)
    cnt = Counter(r["status"] for r in out)
    print("check-validated: branch rows %d: validated %d (skip), stale %d, unvalidated %d, nohash %d" % (
        len(out), cnt["validated"], cnt["stale"], cnt["unvalidated"], cnt["nohash"]))
    for r in [r for r in out if r["status"] != "validated"][:args.show]:
        print("  %-11s %s %s %s" % (r["status"], r["steam_addr"], r["name"], r["note"]))
    return 0


def cmd_persist(args) -> int:
    paths = bd.get_paths()
    bdir = paths.batches / args.batch
    try:
        upserts = json.loads((bdir / "kdb_upsert.json").read_text(encoding="utf-8"))
    except (OSError, ValueError):
        die("%s missing or invalid (run `finalize` first)" % (bdir / "kdb_upsert.json"))
    rejected = [r for r in iter_tsv(bdir / "final_rows.tsv") if r.get("final_verdict") == "rejected" and r.get("steam_addr")]
    lessons = parse_lessons(bdir / "gate_report.md")
    if not (bdir / "validated.tsv").exists():
        die("%s missing (run `finalize`; for B0 run tools/branch_apply.py)" % (bdir / "validated.tsv"))
    marks = list(iter_tsv(bdir / "validated.tsv"))
    date = upserts[0]["date"] if upserts else today(args)
    if args.apply:
        if not paths.kdb.exists():
            die("knowledge DB %s does not exist" % paths.kdb)
        con = sqlite3.connect(str(paths.kdb), isolation_level=None)
        con.execute(VALIDATED_DDL)
    else:
        con = bd.connect_ro(paths.kdb)
    try:
        opus_rejected = {hx8(parse_addr(r["steam_addr"])) for r in iter_tsv(bdir / "overrides.tsv")
                         if r.get("final_verdict") == "rejected" and (r.get("steam_addr") or "").strip()}
        suspects = [r for r in iter_tsv(bdir / "final_rows.tsv") if r.get("final_verdict") == "label_suspect" and r.get("steam_addr")]
        ops = plan_persist(con, upserts, rejected, lessons, args.batch, date, opus_rejected, load_kdb_overrides(bdir), suspects)
        ops += plan_marks(con, marks, args.batch)
        cnt = Counter(o[0] for o in ops)
        print("%s persist %s: insert=%d update=%d demote=%d rename=%d lesson=%d skip=%d mark=%d unmark=%d" % (
            args.batch, "APPLY" if args.apply else "DRY-RUN", cnt["insert"], cnt["update"], cnt["demote"],
            cnt["rename"], cnt["lesson"], cnt["skip"], cnt["mark"], cnt["unmark"]))
        for o in ops:
            if o[0] in ("insert",):
                print("  insert %s %s (%s)" % (o[1]["steam_addr"], o[1]["name"], o[1]["confidence"]))
            elif o[0] == "update":
                print("  update %s %s: %s" % (o[1]["steam_addr"], o[1]["name"], o[2]))
            elif o[0] == "demote":
                print("  demote %s -> wrong: %s" % (o[1], o[2][-130:]))
            elif o[0] == "rename":
                print("  rename %s -> %s: %s" % (o[1], o[2], o[3][-130:]))
            elif o[0] == "lesson":
                print("  lesson [%s] %s" % (o[1], o[2]))
            elif o[0] == "mark":
                continue                                  # counted in the summary line; the rows are validated.tsv
            elif o[0] == "unmark":
                print("  unmark %s %s: %s" % (o[1], o[2], o[3]))
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
    p.add_argument("--name-regex", default=None, help="keep only GOG functions whose name matches (re.search)")
    p.add_argument("--gog-range", default=None, metavar="LO:HI",
                   help="keep only GOG functions with LO <= address < HI (hex, e.g. 0x400000:0x6a0000)")
    p.add_argument("--extra-candidates", default=None,
                   help="TSV `gog steam source`: extra candidates with that source tag (e.g. B<n>/extra_candidates.tsv)")
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
    p.add_argument("--partial", action="store_true", help="opus gate decision on the reviewed chunks only")
    p.add_argument("--opus-sample-errors", type=int, default=None,
                   help="errors Opus found in its sample of reviewer matches (omit until the Opus pass is done)")
    p.add_argument("--date", default=None, help="date stamped into kdb_upsert.json (default today)")
    p = sub.add_parser("opus-parts", help="split opus_gate.json rows into B<n>/opus/part_<x>.md files")
    p.add_argument("batch", type=batch_name)
    p.add_argument("--rows", type=int, default=100, help="rows per part (default 100)")
    p = sub.add_parser("persist", help="dry-run (default) or --apply the name_map upsert")
    p.add_argument("batch", type=batch_name)
    p.add_argument("--apply", action="store_true")
    p.add_argument("--date", default=None)
    p = sub.add_parser("check-validated", help="which branch-DB rows a final validation pass can skip")
    p.add_argument("--branch-db", default=None, help="default BD_BRANCHDB")
    p.add_argument("--out", default=None, help="write every row's status as TSV")
    p.add_argument("--show", type=int, default=20, help="print this many non-validated rows")
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    fn = {"candidates": cmd_candidates, "gates": cmd_gates, "cards": cmd_cards, "finalize": cmd_finalize,
          "persist": cmd_persist, "opus-parts": cmd_opus_parts,
          "check-validated": cmd_check_validated}[args.cmd]
    return fn(args)


if __name__ == "__main__":
    sys.exit(main())
