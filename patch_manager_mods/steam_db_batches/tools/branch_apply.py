#!/usr/bin/env python3
"""branch_apply.py - build the KPM branch-DB functions table after a re-audit batch (B0).

Reads the branch DB (read-only), B<n>/final_rows.tsv (one row per branch row, matched by `cand_orig` = the branch
address) and B<n>/parity.tsv (user decision 2026-10-03: GameAPI-required names whose GOG label is wrong ship at the
Steam counterpart of GOG's address). Writes B<n>/branch_functions.csv (SqliteTools import-ghidra format, the FULL
table for `--mode replace`), B<n>/validated.tsv (the validated ledger, shipped names; `batch.py persist` writes it to
the KDB table steam_validated) and B<n>/branch_apply_report.md. Never writes a database.

Rules:
- existing rows keep their branch class/function names (KPM GameAPI looks them up by name);
- final_verdict verified/likely -> shipped at the final Steam address; parity rows -> parity address;
- every other verdict (label_suspect, rejected, unresolved) -> row removed;
- no Steam address under two names, no name twice (ICF folds must be listed in parity/allow file to pass).
"""
import argparse
import csv
import sqlite3
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import batch  # noqa: E402
import bdlib as bd  # noqa: E402
ROOT = None         # batches dir; None = bdlib paths (BD_ROOT), tests may pin it
SHIP = ("verified", "likely")


def hx(s):
    return int(s, 16) if isinstance(s, str) else int(s)


def load_branch(path):
    con = sqlite3.connect("file:%s?mode=ro" % path, uri=True)
    rows = con.execute("SELECT id, class_name, function_name, address, calling_convention, param_size_bytes, notes "
                       "FROM functions ORDER BY id").fetchall()
    return [dict(zip(("id", "cls", "fn", "addr", "cc", "param", "notes"), r)) for r in rows]


def match_rows(branch, final):
    """branch id -> final row; final rows are matched by branch address, then by name when an address repeats."""
    by_addr = {}
    for f in final:
        by_addr.setdefault(hx(f["cand_orig"]), []).append(f)
    out, used = {}, set()
    for b in branch:                              # pass 1: exact class::name at the branch address
        for f in by_addr.get(b["addr"], []):
            if id(f) not in used and f["name"] == b["fn"] and f["class"] == b["cls"]:
                out[b["id"]], _ = f, used.add(id(f))
                break
    unmatched = []
    for b in branch:                              # pass 2: the one leftover row at that address (renamed rows)
        if b["id"] in out:
            continue
        left = [f for f in by_addr.get(b["addr"], []) if id(f) not in used]
        if len(left) == 1:
            out[b["id"]], _ = left[0], used.add(id(left[0]))
        else:
            unmatched.append(b)
    return out, unmatched


def batches_root() -> Path:
    return ROOT if ROOT is not None else bd.get_paths().batches


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("batch")
    ap.add_argument("--branch-db", default=None, help="default: bdlib paths (BD_BRANCHDB)")
    a = ap.parse_args(argv)
    a.branch_db = a.branch_db or str(bd.get_paths().branchdb)
    bdir = batches_root() / a.batch
    branch = load_branch(a.branch_db)
    with open(bdir / "final_rows.tsv") as fh:
        final = list(csv.DictReader(fh, delimiter="\t"))
    parity = {}
    if (bdir / "parity.tsv").exists():
        with open(bdir / "parity.tsv") as fh:
            for r in csv.DictReader(fh, delimiter="\t"):
                parity[(r["class"], r["function"])] = r
    m, unmatched = match_rows(branch, final)
    if unmatched:
        print("UNMATCHED branch rows:", [(b["id"], b["cls"], b["fn"]) for b in unmatched])
        return 2

    out, marks, removed, moved, kept, n_parity = [], [], [], [], 0, 0
    for b in branch:
        f = m[b["id"]]
        p = parity.get((b["cls"], b["fn"]))
        if p:
            addr, cc, param = hx(p["steam_addr"]), p["calling_convention"], p["param_size_bytes"]
            note = "Steam B0 GOG parity: %s (evidence steam_db_batches/%s/parity.tsv)" % (p["note"], a.batch)
            verdict, tier, ref = "parity", "opus", "parity.tsv"
            n_parity += 1
        elif f["final_verdict"] in SHIP:
            addr, cc = hx(f["steam_addr"]), f["calling_convention"] or b["cc"] or ""
            param = f["param_size_bytes"] if f["param_size_bytes"] != "" else b["param"]
            why = (f.get("final_note") or f.get("evidence") or "").replace("\n", " ")[:160]
            note = "Steam B0 %s: %s (evidence steam_db_batches/%s/final_rows.tsv)" % (f["final_verdict"], why, a.batch)
            verdict, tier, ref = f["final_verdict"], batch.gate_tier(f), "final_rows.tsv"
        else:
            removed.append((b, f["final_verdict"], f.get("final_note", "")))
            continue
        if addr != b["addr"]:
            moved.append((b, addr))
        else:
            kept += 1
        if cc and not cc.startswith("__"):
            cc = "__" + cc
        out.append({"class_name": b["cls"], "function_name": b["fn"], "address": "0x%08X" % addr,
                    "calling_convention": cc, "param_size_bytes": "" if param in (None, "") else int(param),
                    "notes": batch.csv_safe_note(note)})
        marks.append({"steam_addr": "0x%08x" % addr, "name": "%s::%s" % (b["cls"], b["fn"]), "verdict": verdict,
                      "tier": tier, "gog_addr": f.get("gog", ""),
                      "evidence_ref": "steam_db_batches/%s/%s" % (a.batch, ref)})

    # uniqueness
    seen_addr, seen_name, dup = {}, set(), []
    allow = {hx(r["steam_addr"]) for r in parity.values() if "ICF" in r.get("note", "")}
    for r in out:
        k = (r["class_name"], r["function_name"])
        if k in seen_name:
            dup.append(("name", k))
        seen_name.add(k)
        ad = hx(r["address"])
        if ad in seen_addr and ad not in allow:
            dup.append(("addr", r["address"], seen_addr[ad], k))
        seen_addr.setdefault(ad, k)

    with open(bdir / "branch_functions.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=["class_name", "function_name", "address", "calling_convention",
                                           "param_size_bytes", "notes"])
        w.writeheader()
        w.writerows(out)
    if not dup:
        batch.write_validated(bdir, a.batch, marks)
    rep = ["# %s branch apply plan" % a.batch, "",
           "branch rows %d -> shipped %d (same address %d, moved %d, parity %d), removed %d" %
           (len(branch), len(out), kept, len(moved), n_parity, len(removed)), "",
           "## Duplicates", ""] + (["- %s" % (d,) for d in dup] or ["none"]) + ["", "## Moved", ""] + \
          ["- id %d %s::%s 0x%08x -> 0x%08x" % (b["id"], b["cls"], b["fn"], b["addr"], ad) for b, ad in moved] + \
          ["", "## Removed", ""] + \
          ["- id %d %s::%s 0x%08x (%s) %s" % (b["id"], b["cls"], b["fn"], b["addr"], v, (n or "")[:120])
           for b, v, n in removed]
    (bdir / "branch_apply_report.md").write_text("\n".join(rep) + "\n")
    print(rep[2])
    print("duplicates:", len(dup))
    return 1 if dup else 0


if __name__ == "__main__":
    sys.exit(main())
