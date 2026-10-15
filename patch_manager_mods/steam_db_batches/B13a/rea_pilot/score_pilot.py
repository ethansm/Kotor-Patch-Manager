#!/usr/bin/env python3
"""R1 pilot score: reviewer TSVs (review/<arm>_<k>.tsv, plus the original Sonnet reviews) against the B13a Opus
overrides, on the cards in key_pilot.tsv.  Acceptance uses bdlib.reviewer_accepts (finalize's rule); an accepted decoy
that equals an Opus-shipped override pair is a key error, not a decoy accept (finalize's rule).

    python3 B13a/rea_pilot/score_pilot.py        (from steam_db_batches/)"""
import csv
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
B = HERE.parent
sys.path.insert(0, str(B.parent / "tools"))
import bdlib as bd  # noqa: E402


def rd(p):
    return list(csv.DictReader(open(p, encoding="utf-8"), delimiter="\t"))


def h(s):
    return int(s, 16) if s and s.strip() else None


def main():
    key = rd(HERE / "key_pilot.tsv")
    ov = {}
    for f in sorted((B / "opus").glob("overrides_*.tsv")):
        for r in rd(f):
            ov[h(r["gog_addr"])] = r
    shipped = {(g, h(r["steam_addr"])) for g, r in ov.items() if r["final_verdict"] in ("verified", "likely")}
    cls = {}
    for k in key:
        g, s = h(k["gog"]), h(k["steam_addr"])
        if k["kind"] != "real":
            cls[(g, s)] = "KEYERR" if (g, s) in shipped else "DECOY"
            continue
        o = ov[g]
        cls[(g, s)] = ("LS" if o["final_verdict"] == "label_suspect" else
                       "ADDR" if h(o["steam_addr"]) != s else
                       "UNRES" if o["final_verdict"] == "unresolved" else "OK")
    arms = {"sonnet (orig)": sorted((B / "review").glob("sonnet_c*.tsv")),
            "haiku+REA": sorted((HERE / "review").glob("rea_*.tsv")),
            "haiku no-REA": sorted((HERE / "review").glob("norea_*.tsv"))}
    n = Counter(cls.values())
    print("cards: %s" % dict(sorted(n.items())))
    print("| arm | rows | LS flagged | ADDR rejected | OK accepted | decoys accepted | unsure |")
    print("|---|---|---|---|---|---|---|")
    for arm, files in arms.items():
        rev = {}
        for f in files:
            for r in rd(f):
                pair = (h(r.get("gog_addr")), h(r.get("steam_addr")))
                if pair in cls:
                    rev[pair] = r
        c = Counter()
        for pair, cl in cls.items():
            r = rev.get(pair)
            if r is None:
                c["missing"] += 1
                continue
            v = bd.effective_verdict(r, pair[1])
            acc = bd.reviewer_accepts(r, pair[1])
            c["unsure"] += v == "unsure"
            c["LS_flag"] += cl == "LS" and v == "label_suspect"
            c["ADDR_rej"] += cl == "ADDR" and not acc
            c["OK_acc"] += cl == "OK" and acc
            c["DEC_acc"] += cl == "DECOY" and acc
        print("| %s | %d | %d/%d | %d/%d | %d/%d | %d/%d | %d |" % (
            arm, len(rev), c["LS_flag"], n["LS"], c["ADDR_rej"], n["ADDR"], c["OK_acc"], n["OK"],
            c["DEC_acc"], n["DECOY"], c["unsure"]))


if __name__ == "__main__":
    main()
