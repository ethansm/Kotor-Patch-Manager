#!/usr/bin/env python3
"""recall_study.py - report-only study of extra AUTO routes on calibration evidence (does NOT change any rule).

Reads <calibration dir>/runs/novote_*/evidence.tsv (a calibrate.py output) and counts, for each candidate route, how many
positives and how many decoys/negatives (with names) would reach it.  Usage: recall_study.py CALIBRATION_DIR [...]
"""
import csv
import glob
import re
import sys
from collections import Counter, defaultdict

THRESHOLDS = (0.75, 0.9, 1.0)
STRESS = ("shift_p1", "shift_m1", "shift_p2", "shift_m2", "neg_cluster")   # whole-segment errors, tagged mapper:vote


def rows(cal, variant="novote"):
    out = []
    for f in sorted(glob.glob("%s/runs/%s_*/evidence.tsv" % (cal, variant))):
        kind = f.split("/")[-2].split("_", 2)[2]
        with open(f, encoding="utf-8") as fh:
            for r in csv.DictReader(fh, delimiter="\t"):
                r["_kind"] = kind
                out.append(r)
    return out


def fams(r):
    return {x for x in r["families"].split(",") if x} - {"VT"}


def invariants(r):
    return (r["g1"] == "pass" and r["g2"] == "pass" and r["g3"].startswith("pass") and r["g4"] == "pass"
            and r["g6"] == "pass")


def why(r):
    for p in r["flags"].split(";"):
        if p.startswith("why:"):
            return set(p[4:].split(","))
    return set()


def common(r, need_run=True):
    w = why(r)
    return (invariants(r) and (r["run_ok"] == "yes" or not need_run) and r["steam_size"] != ""
            and r["steam_size"] == r["gog_size"] and "permuted" not in w and "ambiguous" not in w)


def num(x):
    try:
        return float(x)
    except ValueError:
        return None


def decoy_sig(r):
    m = re.search(r"/SIG([0-9.]+|n/a)/", r["decoy_best"] or "")
    return 0.0 if not m or m.group(1) == "n/a" else float(m.group(1))


def r1(r, t, need_run=True):
    s = num(r["a_sig"])
    return common(r, need_run) and fams(r) == {"SIG"} and s is not None and s >= t


def r2(r, t=None):
    c = num(r["a_callee"])
    return common(r) and fams(r) == {"CALLEE"} and c is not None and c >= 2 and "callee_contra" not in r["flags"]


def r3(r, t, need_run=True):
    return r1(r, t, need_run) and decoy_sig(r) <= num(r["a_sig"]) - 0.25


def study(cal):
    data = rows(cal)
    pos = [r for r in data if r["_kind"] == "pos"]
    neg = [r for r in data if r["_kind"] != "pos" and r["_kind"] not in STRESS]
    stress = [r for r in data if r["_kind"] in STRESS]
    routes = [("R1 SIG>=%g" % t, (lambda r, t=t: r1(r, t))) for t in THRESHOLDS]
    routes += [("R2 CALLEE>=2", r2)]
    routes += [("R1* SIG>=0.75, run_ok not required", lambda r: r1(r, 0.75, False)),
               ("R3* SIG>=0.75+margin, run_ok not required", lambda r: r3(r, 0.75, False))]
    routes += [("R3 SIG>=%g+margin" % t, (lambda r, t=t: r3(r, t))) for t in THRESHOLDS]
    lines = ["| route | positives | decoys/negatives | negatives hit |", "|---|---|---|---|"]
    for name, fn in routes:
        p = sum(1 for r in pos if fn(r))
        hits = [r for r in neg if fn(r)]
        names = ", ".join(sorted({"%s::%s(%s %s)" % (r["class"], r["name"], r["_kind"], r["steam_addr"]) for r in hits}))
        lines.append("| %s | %d / %d | %d / %d | %s |" % (name, p, len(pos), len(hits), len(neg), names or "-"))
    auto_pos = sum(1 for r in pos if r["gate_class"] == "AUTO")
    pure = lambda r: (r["content_level"] == "C0" and r["run_ok"] == "yes" and r["steam_size"] == r["gog_size"]  # noqa: E731
                      and invariants(r))
    lines.append("")
    lines.append("current AUTO positives: %d / %d. Pure positional (C0, run_ok yes, steam_size == gog_size, "
                 "invariants pass): positives %d, decoys/negatives %d." % (
                     auto_pos, len(pos), sum(1 for r in pos if pure(r)), sum(1 for r in neg if pure(r))))
    lines += ["", "Stress sets (consistent shift of a whole segment / outlier cluster, all candidates mapper:vote):", "",
              "| set | rows | pass all invariants | size==gog_size | current AUTO | R1 (SIG>=0.75) | R2 (CALLEE>=2) | R3 (R1+margin) "
              "| R1* (no run_ok) | R3* (no run_ok) |", "|---|---|---|---|---|---|---|---|---|---|"]
    routes = {"AUTO": lambda r: r["gate_class"] == "AUTO", "R1": lambda r: r1(r, 0.75), "R2": r2,
              "R3": lambda r: r3(r, 0.75), "R1*": lambda r: r1(r, 0.75, False), "R3*": lambda r: r3(r, 0.75, False)}
    hits = []
    for kind in STRESS:
        rs = [r for r in stress if r["_kind"] == kind]
        if not rs:
            continue
        lines.append("| %s | %d | %d | %d | %s |" % (
            kind, len(rs), sum(1 for r in rs if invariants(r)), sum(1 for r in rs if r["steam_size"] == r["gog_size"]),
            " | ".join(str(sum(1 for r in rs if fn(r))) for fn in routes.values())))
        for r in rs:
            hit = [n for n, fn in routes.items() if fn(r)]
            if hit:
                hits.append((kind, r, hit))
    lines += ["", "R1*/R3* ignore run_ok: an upper bound for what a self-supporting wrong segment could reach if the "
              "route were adopted and its rows propagated as mapped neighbours (run_ok cannot be seeded by rows that "
              "are not AUTO, so the R1/R2/R3 columns above only count true seeds).", "",
              "Every stress row reaching any route:", ""]
    if hits:
        lines += ["| set | GOG | name | Steam | families | run_ok | a_sig | routes |", "|---|---|---|---|---|---|---|---|"]
        for kind, r, hit in hits:
            lines.append("| %s | %s | %s::%s | %s | %s | %s | %s | %s |" % (
                kind, r["gog"], r["class"], r["name"], r["steam_addr"], r["families"] or "-", r["run_ok"],
                r["a_sig"], ",".join(hit)))
    else:
        lines.append("None.")
    return lines


def main(argv):
    for cal in argv[1:]:
        print("### %s\n" % cal)
        print("\n".join(study(cal)) + "\n")


if __name__ == "__main__":
    main(sys.argv)
