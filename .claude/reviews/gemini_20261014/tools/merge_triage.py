"""Merge triage/<batch>.{findings,topics}.tsv into findings.tsv/topics.tsv and run the anchor check.

usage: python3 merge_triage.py <review_dir>
"""
import csv, subprocess, sys
from pathlib import Path

D = Path(sys.argv[1])
BATCHES = ["hotbars", "queue_hud", "companion", "shared", "perf_lab", "k1", "holster_npc"]


def read(p):
    with open(p, newline="") as f:
        return list(csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE))


def write(p, rows, cols):
    with open(p, "w", newline="") as f:
        w = csv.DictWriter(f, cols, delimiter="\t", quoting=csv.QUOTE_NONE, escapechar="\\", extrasaction="ignore", lineterminator="\n")
        w.writeheader()
        for r in rows:
            w.writerow({k: (r.get(k) or "").replace("\t", " ").replace("\n", " ") for k in cols})


fcols = ["fid", "set", "bundle", "file", "function", "anchor", "gemini_confidence", "category", "claimed_severity", "title"]
tcols = ["tid", "fids", "priority", "question", "entries"]
findings, topics, errs = [], [], []
for b in BATCHES:
    fp, tp = D / "triage" / f"{b}.findings.tsv", D / "triage" / f"{b}.topics.tsv"
    if not fp.exists() or not tp.exists():
        errs.append(f"missing batch {b}")
        continue
    fr, tr = read(fp), read(tp)
    for r in fr:
        r["batch"] = b
    for r in tr:
        r["batch"] = b
    findings += fr
    topics += tr

fids = [r["fid"] for r in findings]
dup = {x for x in fids if fids.count(x) > 1}
if dup:
    errs.append(f"duplicate fids: {sorted(dup)}")
mapped = {}
for t in topics:
    for fid in [x.strip() for x in t["fids"].split(",") if x.strip()]:
        mapped.setdefault(fid, []).append(t["tid"])
for fid in fids:
    if fid not in mapped:
        errs.append(f"unmapped {fid}")
    elif len(mapped[fid]) > 1:
        errs.append(f"multi-mapped {fid}: {mapped[fid]}")
for fid in mapped:
    if fid not in fids:
        errs.append(f"topic cites unknown fid {fid}")

write(D / "findings.tsv", findings, ["batch"] + fcols)
write(D / "topics.tsv", topics, ["batch"] + tcols)

# anchor check
arows = []
for r in findings:
    a, f = (r.get("anchor") or "").strip(), (r.get("file") or "").strip()
    if a in ("", "-"):
        arows.append({"fid": r["fid"], "found": "no-anchor", "where": "-"})
        continue
    files = [x.strip() for x in f.split(",") if x.strip()]
    if f.startswith("MISSING:") or not all((Path.cwd() / x).is_file() for x in files):
        arows.append({"fid": r["fid"], "found": "no-file", "where": f})
        continue
    hits = []
    # try the full anchor, then progressively shorter leading fragments, then any ;-separated piece
    cands = [a] + [p.strip() for p in a.split(";") if len(p.strip()) >= 12] + [a[:60], a[:40]]
    how = "exact"
    for i, c in enumerate(cands):
        if not c:
            continue
        out = subprocess.run(["rg", "-F", "-n", "--no-heading", "--with-filename", "--", c, *files], capture_output=True, text=True).stdout
        if out:
            hits = [":".join(l.split(":", 2)[:2]) for l in out.splitlines()[:3]]
            how = "exact" if i == 0 else "partial"
            break
    if not hits:  # whitespace-insensitive: first identifier-ish token run
        toks = [t for t in a.replace("(", " ").replace(")", " ").split() if len(t) >= 6]
        for t in toks[:3]:
            out = subprocess.run(["rg", "-F", "-n", "--no-heading", "--with-filename", "--", t, *files], capture_output=True, text=True).stdout
            if out:
                hits = [":".join(l.split(":", 2)[:2]) for l in out.splitlines()[:3]]
                how = "token"
                break
    arows.append({"fid": r["fid"], "found": how if hits else "NOT-FOUND", "where": ",".join(hits) or f})
write(D / "anchors.tsv", arows, ["fid", "found", "where"])

from collections import Counter
print("findings", len(findings), "topics", sum(t["tid"] != "dropped" for t in topics), "dropped", sum(t["tid"] == "dropped" for t in topics))
print("per batch", Counter(r["batch"] for r in findings))
print("priority", Counter(t["priority"] for t in topics if t["tid"] != "dropped"))
print("anchors", Counter(r["found"] for r in arows))
print("ERRORS" if errs else "OK", *errs, sep="\n  ")
