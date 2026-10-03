#!/usr/bin/env python3
"""calibrate.py - Phase A3 gate calibration with hold-out gold sets (DESIGN section 4.2: "AUTO is calibrated in A3").

What it does
  1. Gold sets.  Positives = B1-verified rows + trusted name_map rows (bdlib trust rules) whose name is a GOG
     function (dedupe by GOG address; GOG functions with conflicting trusted addresses are excluded and listed).
     Negatives = b1 `rejected` cand_orig addresses, name_map `wrong` rows, the 7 known-false AI0 mapper addresses,
     and generated decoys per positive (previous / next Steam start with the same retn, one seeded random function
     with equal retn and size within 10%).
  2. Hold-out.  Positives are split into halves A/B (by rank parity in GOG address order, `--block N` makes N
     consecutive positives share a half).  For each half the gates run with exactly one candidate per row and every
     positive of that half untrusted (plus the candidate address itself, so G6 cannot mask a bad anchor), then the
     halves swap.  Each variant (`novote`: no mapper tag, `vote`: the candidate also carries mapper:vote, the worst
     case for a wrong candidate) is run for the positive set and for every decoy / negative kind.
  3. Reports under <out>/ (default $BD_ROOT/calibration): CALIBRATION_draft.md, metrics.json, auto_negatives.tsv
     (every decoy/negative that reached AUTO; target 0), positive_rejects.tsv, gold_*.tsv, runs/<variant>_<half>_<kind>/
     (candidates.tsv, evidence.tsv, gate_report.md) and cards/chunk_N.md + cards/key[_N].tsv (mixed REVIEW positives and
     decoys/negatives, answers only in key.tsv).

Usage: calibrate.py [--quick] [--block N] [--variants novote,vote] [--out-dir DIR] [--sets-only]
                    [--cards-pos 40] [--cards-neg 20] [--no-cards] [--seed 20261003]
`--quick` = variant novote only and no cards unless --cards-pos / --cards-neg are given (fixtures / tests).
`--no-reexport` and `--findings-dir` are accepted for command-line parity with batch.py and ignored (this tool
neither re-exports the dump inputs nor runs the mapper).  Only files under the out dir are written.
"""
from __future__ import annotations

import argparse
import json
import random
import re
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))
import batch  # noqa: E402
import bdlib as bd  # noqa: E402
from bdlib import hx8, iter_tsv, parse_addr  # noqa: E402

SEED = 20261003
DECOY_KINDS = ("decoy_prev", "decoy_next", "decoy_rand")
NEG_KINDS = ("neg_b1rej", "neg_wrong", "neg_known", "neg_cluster")
SHIFT_STEPS = {"shift_p1": 1, "shift_m1": -1, "shift_p2": 2, "shift_m2": -2}
SHIFT_KINDS = tuple(SHIFT_STEPS)
STRESS_KINDS = SHIFT_KINDS + ("neg_cluster",)     # whole-segment errors: always tagged mapper:vote (worst case)
ALL_KINDS = ("pos",) + DECOY_KINDS + NEG_KINDS + SHIFT_KINDS
SHIFT_BLOCK = 20                                   # shift runs use contiguous (block-20) halves whatever --block is
GATE_CLASSES = ("AUTO", "REVIEW", "REJECT", "NOCAND")
DECOY_WINDOW = 8
KNOWN_FALSE_SUMMARY = "findings/AI0_mapper_summary.md"
KNOWN_FALSE_MAP = "findings/AI0_map_CSWSCreature_all.tsv"


@dataclass
class Case:
    """One (GOG function, Steam address) pair to gate, with the expected answer."""
    kind: str
    gog: int
    steam: int
    half: str
    note: str = ""

    @property
    def expected(self) -> str:
        return "match" if self.kind == "pos" else "mismatch"


# ================================================================================================ gold sets
def build_positives(corpus: bd.Corpus, skipped: list | None = None
                    ) -> tuple[dict[int, int], dict[int, str], list[tuple]]:
    """-> ({gog_addr: steam_addr}, {gog_addr: origin}, ambiguous [(gog_addr, [steam addrs])]).

    A name_map row only counts when its name is EXACTLY `Class::Func`: a suffix such as `CompareNoCase(char*)` marks
    an overload or a description, and bdlib.base_name would otherwise match it to the wrong GOG overload.  Skipped
    rows are appended to `skipped` as (steam_addr, name).
    """
    by_gog: dict[int, set[int]] = defaultdict(set)
    origin: dict[tuple[int, int], str] = {}
    for addr, name in corpus.trust.trusted_addrs():
        g = corpus.gog_by_name.get(name)
        if g is None:
            continue
        if name not in corpus.trust.b1.get(addr, ()) and name not in corpus.trust.later.get(addr, ()):
            nm = corpus.name_map.get(addr)
            if nm is not None and nm.name.strip() != name:
                if skipped is not None:
                    skipped.append((addr, nm.name))
                continue
        by_gog[g.addr].add(addr)
        if name in corpus.trust.b1.get(addr, ()):
            origin[(g.addr, addr)] = "b1"
        elif name in corpus.trust.later.get(addr, ()):
            origin[(g.addr, addr)] = "later_batch"
        else:
            nm = corpus.name_map.get(addr)
            origin[(g.addr, addr)] = "name_map:" + (nm.source if nm else "?")
    steam_users: dict[int, set[int]] = defaultdict(set)
    for ga, addrs in by_gog.items():
        for a in addrs:
            steam_users[a].add(ga)
    positives, origins, ambiguous = {}, {}, []
    for ga in sorted(by_gog):
        addrs = sorted(by_gog[ga])
        if len(addrs) > 1 or any(len(steam_users[a]) > 1 for a in addrs):
            ambiguous.append((ga, addrs))
            continue
        positives[ga] = addrs[0]
        origins[ga] = origin[(ga, addrs[0])]
    return positives, origins, ambiguous


def assign_halves(gog_addrs, block: int) -> dict[int, str]:
    """Rank parity (block 1) of the GOG-address-sorted positives; larger blocks keep neighbours in one half."""
    return {ga: "AB"[(i // max(1, block)) % 2] for i, ga in enumerate(sorted(gog_addrs))}


def half_of(ga: int, halves: dict[int, str], block: int) -> str:
    """Half of any GOG address: positives use their rank, others the parity of their 16-byte slot."""
    return halves.get(ga) or "AB"[((ga >> 4) // max(1, block)) % 2]


def parse_known_false(dump: Path, corpus: bd.Corpus) -> list[tuple[int, int, str]]:
    """The known-false mapper addresses of AI0_mapper_summary.md section 3 -> [(gog_addr, steam_addr, label)]."""
    try:
        text = (dump / KNOWN_FALSE_SUMMARY).read_text(encoding="utf-8")
    except OSError:
        return []
    m = re.search(r"^Re-mapped by this run[^\n]*", text, re.M)
    if not m:
        return []
    addrs = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{6})\b", m.group(0))]
    mapped = {}
    for r in _read_map(dump / KNOWN_FALSE_MAP):
        mapped[r[0]] = r
    out = []
    for a in addrs:
        row = mapped.get(a)
        g = corpus.gog_by_name.get(row[2]) if row else None
        if g is None:
            continue
        out.append((g.addr, a, row[2]))
    return out


def _read_map(path: Path) -> list[tuple]:
    rows = []
    try:
        for line in path.read_text(encoding="utf-8").splitlines():
            f = line.split("\t")
            if len(f) >= 4:
                rows.append((parse_addr(f[0]), parse_addr(f[1]), f[2], f[3]))
    except (OSError, ValueError):
        pass
    return rows


def build_negatives(corpus: bd.Corpus, positives: dict[int, int], halves: dict[int, str], block: int
                    ) -> tuple[list[Case], list[tuple[int, int, str]]]:
    """(i) b1 rejected cand_orig, (ii) name_map `wrong`, (iii) the known-false mapper addresses.

    Pairs that equal a gold positive are skipped.  -> (cases, known_false list for the report).
    """
    cases: list[Case] = []
    seen: set[tuple[str, int]] = set()

    def add(kind: str, g: bd.GogFn, steam: int, note: str) -> None:
        if positives.get(g.addr) == steam or (kind, g.addr) in seen or steam not in corpus.steam:
            return
        seen.add((kind, g.addr))
        cases.append(Case(kind, g.addr, steam, half_of(g.addr, halves, block), note))

    for r in iter_tsv(corpus.paths.b1_verified):
        if r.get("kind") == "F" and r.get("verdict") == "rejected" and r.get("cand_orig"):
            g = corpus.gog_by_name.get("%s::%s" % (r["class"], r["name"]))
            if g is not None:
                add("neg_b1rej", g, parse_addr(r["cand_orig"]), "b1 rejected")
    for addr, nm in sorted(corpus.name_map.items()):
        if nm.conf == "wrong":
            g = corpus.gog_by_name.get(bd.base_name(nm.name))
            if g is not None:
                add("neg_wrong", g, addr, "name_map wrong (%s)" % nm.source)
    known = parse_known_false(corpus.paths.dump, corpus)
    for ga, steam, label in known:
        add("neg_known", corpus.gog_by_addr[ga], steam, "AI0 known false (%s)" % label)
    return cases, known


def build_decoys(corpus: bd.Corpus, positives: dict[int, int], halves: dict[int, str], seed: int
                 ) -> list[Case]:
    """Per positive: nearest same-retn Steam start before / after (within 8 functions) and one seeded random."""
    rng = random.Random(seed)
    starts = corpus.steam_starts
    index = {a: i for i, a in enumerate(starts)}
    by_retn: dict[int, list[int]] = defaultdict(list)
    for a in starts:
        by_retn[corpus.steam[a].retn].append(a)
    cases: list[Case] = []
    for ga in sorted(positives):
        s = positives[ga]
        sf = corpus.steam.get(s)
        i = index.get(s)
        if sf is None or i is None:
            continue
        half = halves[ga]
        picked = {s}
        for kind, step in (("decoy_prev", -1), ("decoy_next", 1)):
            for k in range(1, DECOY_WINDOW + 1):
                j = i + step * k
                if not 0 <= j < len(starts):
                    break
                a = starts[j]
                if corpus.steam[a].retn == sf.retn and a not in picked:
                    picked.add(a)
                    cases.append(Case(kind, ga, a, half, "%+d functions" % (step * k)))
                    break
        pool = [a for a in by_retn[sf.retn] if a not in picked and abs(corpus.steam[a].size - sf.size) <= 0.1 * sf.size]
        if pool:
            cases.append(Case("decoy_rand", ga, rng.choice(pool), half, "seeded random"))
    return cases


def contiguous_runs(positives: dict[int, int], corpus: bd.Corpus, halves: dict[int, str]) -> list[list[int]]:
    """Runs of positives that are consecutive in GOG order among the positives, same class, same half."""
    runs: list[list[int]] = []
    for ga in sorted(positives):
        g = corpus.gog_by_addr[ga]
        if runs:
            prev = corpus.gog_by_addr[runs[-1][-1]]
            if prev.cls == g.cls and halves[runs[-1][-1]] == halves[ga]:
                runs[-1].append(ga)
                continue
        runs.append([ga])
    return runs


def build_shifts(corpus: bd.Corpus, positives: dict[int, int], halves: dict[int, str]) -> list[Case]:
    """Consistent-shift decoys: every positive's candidate moves k Steam function starts (+1, -1, +2, -2).

    Starts exclude `Unwind@` funclets (a real mapper never lands on those).  All rows of a half go into one gates
    run, so a whole segment is wrong by the same k and the rows can vouch for each other through run_ok.
    """
    starts = corpus.steam_starts
    index = {a: i for i, a in enumerate(starts)}
    cases: list[Case] = []
    for ga in sorted(positives):
        i = index.get(positives[ga])
        if i is None:
            continue
        for kind, k in SHIFT_STEPS.items():
            j = i + k
            if 0 <= j < len(starts):
                cases.append(Case(kind, ga, starts[j], halves[ga], "shift %+d" % k))
    return cases


def build_cluster(corpus: bd.Corpus, positives: dict[int, int]) -> list[Case]:
    """The CSWSCreature outlier cluster of AI0_mapper_summary.md section 3 (one run, half label `C`)."""
    try:
        text = (corpus.paths.dump / KNOWN_FALSE_SUMMARY).read_text(encoding="utf-8")
    except OSError:
        return []
    m = re.search(r"^Full outlier cluster[^\n]*\n((?:[ \t]+[0-9a-fA-F]{8}:[^\n]*\n?)+)", text, re.M)
    if not m:
        return []
    steam_addrs = [int(a, 16) for a in re.findall(r"\b([0-9a-fA-F]{8}):\w+", m.group(1))]
    mapped = {r[0]: r for r in _read_map(corpus.paths.dump / KNOWN_FALSE_MAP)}
    cases = []
    for a in steam_addrs:
        row = mapped.get(a)
        g = corpus.gog_by_name.get(row[2]) if row else None
        if g is not None and positives.get(g.addr) != a and a in corpus.steam:
            cases.append(Case("neg_cluster", g.addr, a, "C", "AI0 outlier cluster"))
    return cases


# ================================================================================================ runs
def candidate_rows(corpus: bd.Corpus, cases: list[Case], variant: str, force_vote: bool = False) -> list[dict]:
    """Synthetic candidates.tsv rows: exactly one candidate per GOG function."""
    rows = []
    for c in cases:
        g = corpus.gog_by_addr[c.gog]
        gs, gr = corpus.gog_size(g.addr), corpus.gog_retn(g.addr)
        src = "calib" + ("+mapper:vote" if variant == "vote" or force_vote else "")
        rows.append({"class": g.cls, "name": g.func, "gog": hx8(g.addr), "gog_callconv": g.callconv,
                     "gog_param": "" if g.param is None else g.param, "gog_size": "" if gs is None else gs,
                     "gog_retn": "" if gr is None else gr, "candidates": "%s|%s" % (hx8(c.steam), src)})
    return rows


def run_cases(corpus, scorer, out: Path, variant: str, half: str, kind: str, cases: list[Case],
              half_untrust: set[int]) -> tuple[list[dict], set[int]]:
    """Gate one run directory; -> (evidence rows, untrust set used)."""
    untrust = set(half_untrust) | {c.steam for c in cases}
    rundir = out / "runs" / ("%s_%s_%s" % (variant, half, kind))
    crows = candidate_rows(corpus, cases, variant, force_vote=kind in STRESS_KINDS)
    batch.write_tsv(rundir / "candidates.tsv", batch.CANDIDATE_COLS, crows)
    corpus.apply_untrust(untrust)
    evs = batch.run_gates("CAL-%s-%s-%s" % (variant, half, kind), rundir, corpus, scorer, crows)
    return evs, untrust


# ================================================================================================ metrics
def why_codes(ev: dict) -> list[str]:
    for part in (ev.get("flags") or "").split(";"):
        if part.startswith("why:"):
            return [c for c in part[4:].split(",") if c]
    return []


def auto_route(ev: dict) -> str:
    """`c2` / `sig` for an AUTO evidence row (from the auto_route= flag), else ''."""
    for part in (ev.get("flags") or "").split(";"):
        if part.startswith("auto_route="):
            return part.split("=", 1)[1]
    return ""


def reject_reason(ev: dict) -> str:
    bits = ["%s=%s" % (k, ev[k]) for k in ("g1", "g4", "g6") if ev.get(k, "").startswith("fail")]
    return "; ".join(bits) or ev.get("flags", "")


def summarize(results: dict, variants: list[str]) -> dict:
    """results[(variant, half, kind)] = evidence rows -> metrics dict."""
    m: dict = {"variants": {}}
    for v in variants:
        per_kind: dict[str, Counter] = {k: Counter() for k in ALL_KINDS}
        why: Counter = Counter()
        pos_rows, auto_neg, pos_reject = [], [], []
        pos_routes: Counter = Counter()
        g3_na_pos = g3_na_all = 0
        for (var, half, kind), evs in results.items():
            if var != v:
                continue
            for e in evs:
                per_kind[kind][e["gate_class"]] += 1
                if e["g3"] == "n/a":
                    g3_na_all += 1
                if kind == "pos":
                    pos_rows.append(e)
                    if e["gate_class"] == "AUTO":
                        pos_routes[auto_route(e) or "c2"] += 1
                    g3_na_pos += e["g3"] == "n/a"
                    if e["gate_class"] == "REVIEW":
                        why.update(why_codes(e))
                    if e["gate_class"] == "REJECT":
                        pos_reject.append(dict(e, _half=half, _reason=reject_reason(e)))
                elif e["gate_class"] == "AUTO":
                    auto_neg.append(dict(e, _kind=kind, _half=half))
        n_pos = sum(per_kind["pos"].values())
        m["variants"][v] = {
            "per_kind": {k: dict(c) for k, c in per_kind.items() if c},
            "positives": n_pos,
            "positive_auto": per_kind["pos"]["AUTO"],
            "positive_auto_c2": pos_routes["c2"],
            "positive_auto_sig": pos_routes["sig"],
            "positive_auto_recall": (per_kind["pos"]["AUTO"] / n_pos) if n_pos else None,
            "positive_review_share": (per_kind["pos"]["REVIEW"] / n_pos) if n_pos else None,
            "positive_reject": per_kind["pos"]["REJECT"],
            "negatives_total": sum(sum(c.values()) for k, c in per_kind.items() if k != "pos" and k not in STRESS_KINDS),
            "negatives_auto": sum(1 for e in auto_neg if e["_kind"] not in STRESS_KINDS),
            "stress_total": sum(sum(c.values()) for k, c in per_kind.items() if k in STRESS_KINDS),
            "stress_auto": sum(1 for e in auto_neg if e["_kind"] in STRESS_KINDS),
            "top_why_positive_review": why.most_common(10),
            "g3_na_positives": g3_na_pos,
            "g3_na_all": g3_na_all,
            "_auto_neg": auto_neg, "_pos_reject": pos_reject,
        }
    return m


EV_TAIL = ["variant", "half", "kind"]


def write_lists(out: Path, metrics: dict, variants: list[str]) -> None:
    rows_auto, rows_rej = [], []
    for v in variants:
        for e in metrics["variants"][v]["_auto_neg"]:
            rows_auto.append(dict(e, variant=v, half=e["_half"], kind=e["_kind"]))
        for e in metrics["variants"][v]["_pos_reject"]:
            rows_rej.append(dict(e, variant=v, half=e["_half"], kind="pos", reason=e["_reason"]))
    batch.write_tsv(out / "auto_negatives.tsv", batch.EVIDENCE_COLS + EV_TAIL, rows_auto)
    batch.write_tsv(out / "positive_rejects.tsv", batch.EVIDENCE_COLS + EV_TAIL + ["reason"], rows_rej)


def pct(x: Optional[float]) -> str:
    return "n/a" if x is None else "%.1f%%" % (100 * x)


def write_draft(out: Path, corpus: bd.Corpus, metrics: dict, variants: list[str], gold: dict, args,
                cards_info: str) -> None:
    L = ["# Calibration draft (Phase A3)", "",
         "Generated by `tools/calibrate.py` (seed %d, split block %d, variants %s)." % (
             args.seed, args.block, ",".join(variants)),
         "GOG fingerprints present: %s. Steam fingerprints present: %s." % (
             "yes" if corpus.has_gog_fp else "NO (A-STR/A-CALLEE/A-SIG n/a, nothing can reach AUTO)",
             "yes" if corpus.has_steam_fp else "NO"), "",
         "## Headline", "", "| variant | positives | AUTO recall (c2 + sig) | REVIEW share | positive REJECT | G3 n/a (pos / all) | "
         "decoys+negatives | reached AUTO | shift/cluster rows | reached AUTO |", "|---|---|---|---|---|---|---|---|---|---|"]
    for v in variants:
        d = metrics["variants"][v]
        L.append("| %s | %d | %s (%d = %d c2 + %d sig) | %s | %d | %d / %d | %d | **%d** | %d | **%d** |" % (
            v, d["positives"], pct(d["positive_auto_recall"]), d["positive_auto"], d["positive_auto_c2"],
            d["positive_auto_sig"], pct(d["positive_review_share"]),
            d["positive_reject"], d["g3_na_positives"], d["g3_na_all"], d["negatives_total"], d["negatives_auto"],
            d["stress_total"], d["stress_auto"]))
    L += ["", "Target: 0 decoys/negatives reaching AUTO. The `vote` variant adds mapper:vote to the candidate (the "
          "mapper endorses the wrong answer), so it is the worst case.", "", "## Gold sets", ""]
    for k, v in gold["counts"].items():
        L.append("- %s: %d" % (k, v))
    L += ["", "Stress sets (`shift_p1/m1/p2/m2`, `neg_cluster`): a whole segment is wrong by the same shift k, every candidate "
          "tagged mapper:vote, one gates run per half (block-%d halves, whole half untrusted); `neg_cluster` is the AI0 "
          "CSWSCreature outlier cluster in one run. Contiguous positive runs (same class, consecutive, same half): %d "
          "(longest %d)." % (SHIFT_BLOCK, len(gold["pos_runs"]), max((len(r) for r in gold["pos_runs"]), default=0)),
          "", "Hold-out: for each half, every positive of that half (and each run's candidate address) is untrusted; "
          "halves by %s." % ("rank parity of the GOG-address-sorted positives" if args.block == 1 else
                              "blocks of %d consecutive positives" % args.block), ""]
    if gold["ambiguous"]:
        L += ["### Ambiguous gold (excluded: conflicting trusted addresses, or one Steam address for two GOG names)", ""]
        L += ["- GOG %s -> %s" % (hx8(g), ", ".join(hx8(a) for a in addrs)) for g, addrs in gold["ambiguous"][:40]]
        L.append("")
    if gold["skipped"]:
        L += ["### Excluded from positives: trusted name_map rows whose name is not exactly `Class::Func` (%d)" %
              len(gold["skipped"]), "", "Suffixes mark overloads or descriptions (e.g. `CompareNoCase(char*)`), so the GOG "
              "match would be ambiguous.", ""]
        L += ["- %s %s" % (hx8(a), n) for a, n in gold["skipped"][:30]]
        L.append("")
    if gold.get("annotations"):
        L += ["### Gold annotations (label_suspect pairs leave the positive and negative sets; %d cases dropped)" %
              len(gold["excluded"]), ""]
        L += ["- GOG %s / Steam %s: %s - %s" % (hx8(g), hx8(a), ann, note[:100])
              for (g, a), (ann, note) in sorted(gold["annotations"].items())]
        L.append("")
    L += ["### Known-false mapper addresses used as negatives (AI0_mapper_summary.md)", ""]
    L += ["- %s -> GOG %s %s" % (hx8(s), hx8(g), n) for g, s, n in gold["known"]] or ["- (none parsed)"]
    L += ["", "## Gate classes per set", "", "| variant | set | AUTO | REVIEW | REJECT | NOCAND | total |",
          "|---|---|---|---|---|---|---|"]
    for v in variants:
        for k, c in metrics["variants"][v]["per_kind"].items():
            L.append("| %s | %s | %s | %d |" % (v, k, " | ".join(str(c.get(x, 0)) for x in GATE_CLASSES), sum(c.values())))
    for v in variants:
        d = metrics["variants"][v]
        L += ["", "## Decoys / negatives that reached AUTO (%s)" % v, ""]
        if not d["_auto_neg"]:
            L.append("None.")
        else:
            L += ["| set | half | GOG | name | Steam | families | run_ok | evidence |", "|---|---|---|---|---|---|---|---|"]
            for e in d["_auto_neg"]:
                L.append("| %s | %s | %s | %s::%s | %s | %s | %s | %s |" % (
                    e["_kind"], e["_half"], e["gog"], e["class"], e["name"], e["steam_addr"], e["families"],
                    e["run_ok"], e["evidence"][:100].replace("|", "/")))
        L += ["", "## Positive REJECT list (%s): gate bugs or gold errors" % v, ""]
        if not d["_pos_reject"]:
            L.append("None.")
        else:
            L += ["| half | GOG | name | Steam | reason |", "|---|---|---|---|---|"]
            for e in d["_pos_reject"][:150]:
                L.append("| %s | %s | %s::%s | %s | %s |" % (e["_half"], e["gog"], e["class"], e["name"],
                                                           e["steam_addr"], e["_reason"].replace("|", "/")))
            if len(d["_pos_reject"]) > 150:
                L.append("(%d more in positive_rejects.tsv)" % (len(d["_pos_reject"]) - 150))
        L += ["", "## Positive REVIEW reasons (%s)" % v, "", "Top `why` codes (a row can carry several): " +
              (", ".join("%s=%d" % kv for kv in d["top_why_positive_review"]) or "none"), ""]
    L += ["## Cards", "", cards_info, ""]
    (out / "CALIBRATION_draft.md").write_text("\n".join(L), encoding="utf-8")


# ================================================================================================ cards
def load_key_gogs(paths) -> set[int]:
    """GOG addresses of earlier chunks' key.tsv files (`--exclude-cards`): those functions never appear again."""
    gogs = set()
    for p in paths or []:
        for r in iter_tsv(Path(p)):
            if r.get("gog"):
                gogs.add(parse_addr(r["gog"]))
    return gogs


def build_cards(out: Path, corpus, scorer, results: dict, run_untrust: dict, case_index: dict, variant: str,
                n_pos: int, n_neg: int, seed: int, chunk_id: int = 1, exclude_gogs: set | None = None) -> str:
    """One mixed chunk of REVIEW positives and REVIEW decoys/negatives; answers only in key[_N].tsv."""
    rng = random.Random(seed)
    exclude_gogs = exclude_gogs or set()
    pos, neg = [], []
    for (v, half, kind), evs in sorted(results.items()):
        if v != variant:
            continue
        if kind in STRESS_KINDS:                     # stress rows are tallied, not reviewed
            continue
        for e in evs:
            if e["gate_class"] == "REVIEW" and parse_addr(e["gog"]) not in exclude_gogs:
                (pos if kind == "pos" else neg).append((v, half, kind, e))
    rng.shuffle(pos)
    rng.shuffle(neg)
    named = [t for t in neg if t[2] in NEG_KINDS][:n_neg // 2]     # real past mistakes first, then generated decoys
    rest = [t for t in neg if t not in named]
    chosen = pos[:n_pos] + named + rest[:max(0, n_neg - len(named))]
    if not chosen:
        return "No REVIEW rows to build cards from."
    rng.shuffle(chosen)
    chosen.sort(key=lambda t: (t[0], t[1], t[2]))        # group by run so untrust is re-applied rarely
    cards = []
    ev_by_run = {k: {parse_addr(e["gog"]): e for e in evs} for k, evs in results.items()}
    for v, half, kind, e in chosen:
        key = (v, half, kind)
        corpus.apply_untrust(run_untrust[key])
        ga = parse_addr(e["gog"])
        g = corpus.gog_by_addr[ga]
        case = case_index[(kind, ga)]
        ce = batch.evaluate_row(corpus, scorer, g, [(case.steam, ["calib"])]).best
        cards.append((kind, case, g, ce, ev_by_run[key]))
    rng.shuffle(cards)
    text = [batch.CARD_HEADER % ("CAL", chunk_id, "steam_db_batches/calibration")]
    key_rows = []
    for n, (kind, case, g, ce, evd) in enumerate(cards, 1):
        cid = "CAL-%d-%02d" % (chunk_id, n)
        text.append(batch.make_card(cid, corpus, g, ce, evd, ""))
        key_rows.append({"card_id": cid, "chunk": chunk_id, "gog": hx8(g.addr), "steam_addr": hx8(case.steam),
                         "kind": kind, "expected": case.expected})
    cdir = out / "cards"
    cdir.mkdir(parents=True, exist_ok=True)
    key_name = "key.tsv" if chunk_id == 1 else "key_%d.tsv" % chunk_id
    (cdir / ("chunk_%d.md" % chunk_id)).write_text("\n".join(text), encoding="utf-8")
    batch.write_tsv(cdir / key_name, ["card_id", "chunk", "gog", "steam_addr", "kind", "expected"], key_rows)
    npos = sum(1 for r in key_rows if r["kind"] == "pos")
    return "%d cards (%d positives, %d decoys/negatives), all REVIEW in variant `%s`: cards/chunk_%d.md, %s%s." % (
        len(key_rows), npos, len(key_rows) - npos, variant, chunk_id, key_name,
        "; %d earlier-chunk GOG functions excluded" % len(exclude_gogs) if exclude_gogs else "")


# ================================================================================================ main
def load_annotations(path: Path | None) -> dict[tuple[int, int], tuple[str, str]]:
    """gold_annotations.tsv (gog_addr, steam_addr, annotation, note) -> {(gog, steam): (annotation, note)}."""
    out = {}
    if path is None:
        return out
    for r in iter_tsv(Path(path)):
        try:
            out[(parse_addr(r["gog_addr"]), parse_addr(r["steam_addr"]))] = (r.get("annotation", ""), r.get("note", ""))
        except (ValueError, KeyError):
            continue
    return out


def build_gold(corpus: bd.Corpus, block: int, seed: int, annotations: dict | None = None) -> dict:
    skipped: list = []
    positives, origins, ambiguous = build_positives(corpus, skipped)
    halves = assign_halves(positives, block)
    negatives, known = build_negatives(corpus, positives, halves, block)
    decoys = build_decoys(corpus, positives, halves, seed)
    cases = [Case("pos", ga, s, halves[ga], origins[ga]) for ga, s in sorted(positives.items())] + decoys + negatives
    halves20 = assign_halves(positives, SHIFT_BLOCK)
    shifts = build_shifts(corpus, positives, halves20)
    cluster = build_cluster(corpus, positives)
    cases += shifts + cluster
    runs = contiguous_runs(positives, corpus, halves20)
    counts = {"positives": len(positives)}
    counts.update({k: sum(1 for c in cases if c.kind == k) for k in ALL_KINDS + ("neg_cluster",) if k != "pos"})
    # run plan: (half, kind, cases, positives untrusted for the run)
    pos_steam = {h: {c.steam for c in cases if c.kind == "pos" and c.half == h} for h in "AB"}
    pos20 = {h: {positives[ga] for ga in positives if halves20[ga] == h} for h in "AB"}
    plan = []
    for kind in ALL_KINDS:
        for half in "AB":
            base = pos20[half] if kind in SHIFT_KINDS else pos_steam[half]
            sel = [c for c in cases if c.kind == kind and c.half == half]
            if sel:
                plan.append((half, kind, sel, base))
    if cluster:
        classes = {corpus.gog_by_addr[c.gog].cls for c in cluster}
        base = {positives[ga] for ga in positives if corpus.gog_by_addr[ga].cls in classes}
        plan.append(("C", "neg_cluster", cluster, base))
    # annotations: pairs marked label_suspect leave both the positive and the negative sets (their trusted address
    # stays untrusted in the half bases above); a suspect positive takes every case of its GOG function with it,
    # because the true counterpart may be one of that function's decoys.
    excluded = []
    suspect = {k for k, (a, _n) in (annotations or {}).items() if a == "label_suspect"}
    if suspect:
        drop_gogs = {c.gog for c in cases if c.kind == "pos" and (c.gog, c.steam) in suspect}
        keep = lambda c: (c.gog, c.steam) not in suspect and c.gog not in drop_gogs   # noqa: E731
        excluded = [c for c in cases if not keep(c)]
        cases = [c for c in cases if keep(c)]
        plan = [(h, k, [c for c in sel if keep(c)], base) for h, k, sel, base in plan]
        plan = [p for p in plan if p[2]]
        counts = {"positives": sum(1 for c in cases if c.kind == "pos")}
        counts.update({k: sum(1 for c in cases if c.kind == k) for k in ALL_KINDS + ("neg_cluster",) if k != "pos"})
    return {"positives": positives, "origins": origins, "ambiguous": ambiguous, "halves": halves, "skipped": skipped,
            "cases": cases, "known": known, "counts": counts, "plan": plan, "pos_runs": runs, "excluded": excluded,
            "annotations": annotations or {}}


def write_gold(out: Path, corpus: bd.Corpus, gold: dict) -> None:
    rows = []
    for c in gold["cases"]:
        g = corpus.gog_by_addr[c.gog]
        rows.append({"kind": c.kind, "half": c.half, "gog": hx8(c.gog), "name": g.name, "steam_addr": hx8(c.steam),
                     "expected": c.expected, "note": c.note})
    batch.write_tsv(out / "gold_sets.tsv", ["kind", "half", "gog", "name", "steam_addr", "expected", "note"], rows)


def cmd(args) -> int:
    paths = bd.get_paths()
    out = Path(args.out_dir).resolve() if args.out_dir else paths.batches / "calibration"
    out.mkdir(parents=True, exist_ok=True)
    variants = [v for v in args.variants.split(",") if v] if args.variants else (["novote"] if args.quick else
                                                                                  ["novote", "vote"])
    corpus = bd.Corpus(paths)
    scorer = bd.Scorer(corpus)
    ann_path = Path(args.annotations) if args.annotations else paths.batches / "calibration" / "gold_annotations.tsv"
    annotations = load_annotations(ann_path if ann_path.exists() else None)
    gold = build_gold(corpus, args.block, args.seed, annotations)
    write_gold(out, corpus, gold)
    print("gold sets: " + ", ".join("%s=%d" % kv for kv in gold["counts"].items()) +
          "; ambiguous excluded=%d" % len(gold["ambiguous"]))
    if args.sets_only:
        return 0
    results, run_untrust = {}, {}
    case_index = {(c.kind, c.gog): c for c in gold["cases"]}
    for v in variants:
        for half, kind, cases, base in gold["plan"]:
            evs, untrust = run_cases(corpus, scorer, out, v, half, kind, cases, base)
            results[(v, half, kind)] = evs
            run_untrust[(v, half, kind)] = untrust
            cnt = Counter(e["gate_class"] for e in evs)
            print("  %-7s %s %-11s n=%-5d %s" % (v, half, kind, len(evs),
                                                " ".join("%s=%d" % (k, cnt[k]) for k in GATE_CLASSES)))
    metrics = summarize(results, variants)
    write_lists(out, metrics, variants)
    want_cards = (not args.no_cards) and (not args.quick or args.cards_pos_given or args.cards_neg_given)
    cards_info = "Cards not built."
    if want_cards:
        cseed = args.cards_seed if args.cards_seed is not None else args.seed + 1
        cards_info = build_cards(out, corpus, scorer, results, run_untrust, case_index, variants[0],
                                 args.cards_pos, args.cards_neg, cseed, args.chunk_id,
                                 load_key_gogs(args.exclude_cards))
    write_draft(out, corpus, metrics, variants, gold, args, cards_info)
    slim = {v: {k: x for k, x in d.items() if not k.startswith("_")} for v, d in metrics["variants"].items()}
    (out / "metrics.json").write_text(json.dumps(slim, indent=1, default=str) + "\n", encoding="utf-8")
    for v in variants:
        d = metrics["variants"][v]
        print("[%s] positives=%d AUTO recall=%s (c2=%d sig=%d) REVIEW share=%s positive REJECT=%d G3 n/a=%d/%d | "
              "decoys+negatives=%d reached AUTO=%d" % (
                  v, d["positives"], pct(d["positive_auto_recall"]), d["positive_auto_c2"], d["positive_auto_sig"], pct(d["positive_review_share"]),
                  d["positive_reject"], d["g3_na_positives"], d["g3_na_all"], d["negatives_total"],
                  d["negatives_auto"]))
        print("  stress rows (shift/cluster)=%d reached AUTO=%d" % (d["stress_total"], d["stress_auto"]))
        for e in d["_auto_neg"]:
            print("  AUTO NEGATIVE %s %s %s::%s -> %s" % (e["_kind"], e["gog"], e["class"], e["name"], e["steam_addr"]))
    print("reports in %s" % out)
    return 0


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--quick", action="store_true", help="variant novote only; no cards unless requested")
    ap.add_argument("--variants", default=None, help="comma list of novote,vote")
    ap.add_argument("--block", type=int, default=1, help="consecutive positives per half (1 = rank parity)")
    ap.add_argument("--seed", type=int, default=SEED)
    ap.add_argument("--out-dir", default=None, help="default $BD_ROOT/calibration")
    ap.add_argument("--sets-only", action="store_true", help="build and write the gold sets, run nothing")
    ap.add_argument("--no-cards", action="store_true")
    ap.add_argument("--chunk-id", type=int, default=1, help="cards/chunk_N.md and key[_N].tsv (chunk 1 uses key.tsv)")
    ap.add_argument("--cards-seed", type=int, default=None, help="seed of the card sampling (default seed + 1)")
    ap.add_argument("--exclude-cards", action="append", default=None,
                    help="key.tsv of an earlier chunk; its GOG functions are not sampled again (repeatable)")
    ap.add_argument("--annotations", default=None, help="gold annotations TSV (default $BD_ROOT/calibration/gold_annotations.tsv)")
    ap.add_argument("--cards-pos", type=int, default=None)
    ap.add_argument("--cards-neg", type=int, default=None)
    ap.add_argument("--no-reexport", action="store_true", help="accepted, ignored (this tool never re-exports)")
    ap.add_argument("--findings-dir", default=None, help="accepted, ignored (this tool never runs the mapper)")
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    args.cards_pos_given, args.cards_neg_given = args.cards_pos is not None, args.cards_neg is not None
    args.cards_pos = 40 if args.cards_pos is None else args.cards_pos
    args.cards_neg = 20 if args.cards_neg is None else args.cards_neg
    return cmd(args)


if __name__ == "__main__":
    sys.exit(main())
