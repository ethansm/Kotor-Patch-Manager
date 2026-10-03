#!/usr/bin/env python3
"""score_review.py - score a reviewer's TSV against a calibration key.tsv.

Usage: score_review.py KEY REVIEW [--annotations FILE] [--out FILE.md]
KEY    cards/key.tsv  (card_id chunk gog steam_addr kind expected)
REVIEW reviewer TSV   (gog_addr steam_addr verdict corrected_addr content_anchor confidence reason)
Rows are joined on (gog, steam).  Pairs annotated `label_suspect` in the annotations file (gold_annotations.tsv) are
counted separately: they are GOG DB label problems, so neither a `match` nor a `mismatch` on them is a reviewer error.
Reports: confusion table (incl. label_suspect), false-accept list, precision on `match` (matches on expected-match
rows / all matches, annotated rows excluded) and the annotated rows' verdicts.

Decoy cards (key kinds `decoy_prev` / `decoy_next` in real batches, DESIGN 4.6): counted per chunk; 0 tolerance.  The
tool prints the escalation decision: any decoy judged `match` or `label_suspect` (counterpart yes) means 100% of that
batch's reviewer matches go to Opus; fewer than --decoys-min decoys in a chunk is INSUFFICIENT.
"""
from __future__ import annotations

import argparse
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from bdlib import hx8, iter_tsv, parse_addr  # noqa: E402

VERDICTS = ("match", "label_suspect", "mismatch", "unsure", "missing")


def load_annotations(path) -> dict[tuple[int, int], str]:
    out = {}
    if path and Path(path).exists():
        for r in iter_tsv(Path(path)):
            try:
                out[(parse_addr(r["gog_addr"]), parse_addr(r["steam_addr"]))] = r.get("annotation", "")
            except (ValueError, KeyError):
                continue
    return out


def score(key_path, review_path, annotations_path=None) -> dict:
    """-> result dict with confusion counts, lists and precision."""
    ann = load_annotations(annotations_path)
    reviews = {}
    for r in iter_tsv(Path(review_path)):
        try:
            reviews[(parse_addr(r["gog_addr"]), parse_addr(r["steam_addr"]))] = r
        except (ValueError, KeyError):
            continue
    conf = {"expected match": Counter(), "expected mismatch": Counter(), "annotated label_suspect": Counter()}
    false_accepts, missed, annotated, unsure = [], [], [], []
    for k in iter_tsv(Path(key_path)):
        pair = (parse_addr(k["gog"]), parse_addr(k["steam_addr"]))
        rv = reviews.get(pair)
        verdict = rv.get("verdict", "missing") if rv else "missing"
        if verdict not in VERDICTS:
            verdict = "unsure"
        row = dict(k, verdict=verdict, reason=(rv or {}).get("reason", ""))
        if ann.get(pair) == "label_suspect":
            conf["annotated label_suspect"][verdict] += 1
            annotated.append(row)
            continue
        group = "expected match" if k["expected"] == "match" else "expected mismatch"
        conf[group][verdict] += 1
        if group == "expected mismatch" and verdict == "match":
            false_accepts.append(row)
        if group == "expected match" and verdict != "match":
            missed.append(row)
        if verdict == "unsure":
            unsure.append(row)
    decoy_chunks: dict[str, list[dict]] = {}
    for k in iter_tsv(Path(key_path)):
        if k["kind"].startswith("decoy"):
            pair = (parse_addr(k["gog"]), parse_addr(k["steam_addr"]))
            rv = reviews.get(pair)
            decoy_chunks.setdefault(k["chunk"], []).append(dict(k, verdict=rv.get("verdict", "missing") if rv else "missing",
                                                               reason=(rv or {}).get("reason", "")))
    tp = conf["expected match"]["match"]
    all_match = tp + conf["expected mismatch"]["match"]
    return {"confusion": conf, "false_accepts": false_accepts, "missed": missed, "annotated": annotated,
            "unsure": unsure, "tp": tp, "matches": all_match,
            "decoy_chunks": decoy_chunks,
            "precision": (tp / all_match) if all_match else None,
            "recall": (tp / sum(conf["expected match"].values())) if sum(conf["expected match"].values()) else None}


def escalation(res: dict, decoys_min: int = 10) -> tuple[str, list[dict]]:
    """-> (decision text, accepted decoys).  Accepted = `match` or `label_suspect` on a decoy card."""
    accepted = [r for rows in res["decoy_chunks"].values() for r in rows if r["verdict"] in ("match", "label_suspect")]
    short = sorted(c for c, rows in res["decoy_chunks"].items() if len(rows) < decoys_min)
    if accepted:
        return "ESCALATE: %d decoy card(s) accepted -> 100%% of this batch's reviewer matches go to Opus" % len(accepted), accepted
    if short or not res["decoy_chunks"]:
        return "INSUFFICIENT DECOYS (chunks %s; need >= %d per chunk)" % (short or "none", decoys_min), accepted
    return "OK: 0 of %d decoy cards accepted" % sum(len(r) for r in res["decoy_chunks"].values()), accepted


def render(res: dict, key_path, review_path, decoys_min: int = 10) -> str:
    L = ["# Review scores", "", "key `%s`, review `%s`" % (key_path, review_path), "",
         "| rows | " + " | ".join(VERDICTS) + " | total |", "|---|" + "---|" * (len(VERDICTS) + 1)]
    for grp, c in res["confusion"].items():
        L.append("| %s | %s | %d |" % (grp, " | ".join(str(c[v]) for v in VERDICTS), sum(c.values())))
    pct = lambda x: "n/a" if x is None else "%.1f%%" % (100 * x)    # noqa: E731
    L += ["", "- precision on `match` (matches on expected-match rows / all matches, annotated rows excluded): "
          "%d / %d = %s" % (res["tp"], res["matches"], pct(res["precision"])),
          "- recall on expected-match rows: %s" % pct(res["recall"]),
          "- annotated label_suspect rows are counted apart: %d (verdicts: %s)" % (
              len(res["annotated"]), ", ".join("%s=%d" % kv for kv in sorted(
                  res["confusion"]["annotated label_suspect"].items())) or "none"),
          "", "## False accepts (expected mismatch, reviewer `match`)", ""]
    L += ["- %s %s -> %s (%s): %s" % (r["card_id"], r["gog"], r["steam_addr"], r["kind"], r["reason"])
          for r in res["false_accepts"]] or ["None."]
    L += ["", "## Expected-match rows not judged `match`", ""]
    L += ["- %s %s -> %s: %s (%s)" % (r["card_id"], r["gog"], r["steam_addr"], r["verdict"], r["reason"])
          for r in res["missed"]] or ["None."]
    L += ["", "## Annotated label_suspect rows", ""]
    L += ["- %s %s -> %s (%s): reviewer %s - %s" % (r["card_id"], r["gog"], r["steam_addr"], r["kind"],
                                                  r["verdict"], r["reason"]) for r in res["annotated"]] or ["None."]
    decision, accepted = escalation(res, decoys_min)
    L += ["", "## Decoy cards per chunk (DESIGN 4.6, 0 tolerance)", ""]
    L += ["- chunk %s: %d decoys, %d accepted" % (c, len(rows), sum(1 for r in rows if r["verdict"] in ("match", "label_suspect")))
          for c, rows in sorted(res["decoy_chunks"].items())] or ["None (this key has no decoy_* kinds)."]
    L += ["- decision: **%s**" % decision]
    L += ["- accepted: %s %s -> %s (%s)" % (r["card_id"], r["gog"], r["steam_addr"], r["verdict"]) for r in accepted]
    L += ["", "## Unsure", ""]
    L += ["- %s %s -> %s (%s)" % (r["card_id"], r["gog"], r["steam_addr"], r["kind"]) for r in res["unsure"]] or ["None."]
    return "\n".join(L) + "\n"


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("key")
    ap.add_argument("review")
    ap.add_argument("--annotations", default=None)
    ap.add_argument("--out", default=None)
    ap.add_argument("--decoys-min", type=int, default=10, help="decoy cards required per chunk")
    args = ap.parse_args(argv)
    res = score(args.key, args.review, args.annotations)
    text = render(res, args.key, args.review, args.decoys_min)
    if args.out:
        Path(args.out).write_text(text, encoding="utf-8")
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
