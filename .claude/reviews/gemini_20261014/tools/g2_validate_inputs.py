"""G2.3 step 1: write g2/<part>.validate_input.md for the Sonnet skeptics (every non-REFUTED fid after v2). Run from repo root.

Batches over 18 fids are split into parts (<batch>_a, <batch>_b) on topic boundaries.
"""
import csv
from collections import defaultdict
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
T = {r["tid"]: r for r in rd(D / "topics.tsv")}
F = {r["fid"]: r for r in rd(D / "findings.tsv")}
CAP = 18

by = defaultdict(lambda: defaultdict(list))
for v in rd(D / "verdicts_v2.tsv"):
    if v["v2_verdict"] != "REFUTED":
        by[T[v["tid"]]["batch"]][v["tid"]].append(v)

parts = {}
for b, tids in sorted(by.items()):
    cur, n, k = [], 0, 0
    for tid in sorted(tids):
        if n and n + len(tids[tid]) > CAP:
            parts[f"{b}_{'abcd'[k]}"] = cur
            cur, n, k = [], 0, k + 1
        cur.append((tid, tids[tid]))
        n += len(tids[tid])
    parts[f"{b}_{'abcd'[k]}" if k else b] = cur

for name, items in parts.items():
    out = [f"# G2.3 skeptic input: {name} ({sum(len(v) for _, v in items)} fids)", ""]
    for tid, vs in items:
        t = T[tid]
        out += [f"## {tid} ({t['priority']}): {t['question']}"]
        for v in vs:
            f = F[v["fid"]]
            out.append(f"- **{v['fid']}** v2={v['v2_verdict']} (G1 {v['verdict']}) | claimed {f['claimed_severity']} {f['category']} | {f['title']}"
                       f" | file {f['file']} fn {f['function']}\n  - G1 evidence: {v['evidence']} | G1 note: {v['note']}"
                       f"\n  - v2 evidence: {v['v2_evidence']} | v2 notes: {v['v2_note']}")
        out.append("")
    (D / "g2" / f"{name}.validate_input.md").write_text("\n".join(out))
    print(name, sum(len(v) for _, v in items), "fids", len("\n".join(out)), "chars")
