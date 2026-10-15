"""G2.2 step 1: write g2/<batch>.input.md for each resolver (open PARTIAL/UNSURE fids + G1 context). Run from repo root."""
import csv
from collections import defaultdict
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
W = Path(".claude/waves/gem1")
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
T = {r["tid"]: r for r in rd(D / "topics.tsv")}
F = {r["fid"]: r for r in rd(D / "findings.tsv")}
R = {r["fid"]: r for r in rd(D / "reanchor.tsv")}
U = {r["tid"]: r["uid"] for r in rd(D / "recon" / "units.tsv")}
(D / "g2").mkdir(exist_ok=True)

by = defaultdict(lambda: defaultdict(list))
for v in rd(D / "verdicts.tsv"):
    if v["verdict"] in ("PARTIAL", "UNSURE") or R[v["fid"]]["drift"] == "gone":
        by[T[v["tid"]]["batch"]][v["tid"]].append(v)

for b, tids in sorted(by.items()):
    out = [f"# G2 resolver input: batch {b} ({sum(map(len, tids.values()))} open fids, {len(tids)} topics)", ""]
    for tid, vs in sorted(tids.items()):
        t, uid = T[tid], U[tid]
        reply = (W / f"{uid}.reply.txt").read_text() if (W / f"{uid}.reply.txt").exists() else ""
        opn = next((l[5:].strip() for l in reply.splitlines() if l.startswith("OPEN:")), "-")
        out += [f"## {tid} ({t['priority']}) — G1 unit {uid}, facts file recon/{tid}.md",
                f"Question: {t['question']}", f"G1 windows (do NOT reuse): {t['entries']}", f"G1 OPEN: {opn}"]
        for v in vs:
            f, r = F[v["fid"]], R[v["fid"]]
            out.append(f"- {v['fid']} [{v['verdict']}] {f['title']} | file {f['file']} | fn {f['function']} | cat {f['category']}"
                       f" | G1 evidence {v['evidence']} | G1 note: {v['note']} | anchor now {r['anchor_now']} drift {r['drift']}"
                       f"{' | EVIDENCE LINES CHANGED SINCE BUNDLE' if r['ev_changed'] == 'yes' else ''}")
        out.append("")
    (D / "g2" / f"{b}.input.md").write_text("\n".join(out))
    print(b, sum(map(len, tids.values())), "fids", len(tids), "topics", len("\n".join(out)), "chars")
