"""G2.2 step 5: merge det judges, gem2 facts and RE answers into verdicts_v2.tsv. Run from repo root.

Sources per fid: det (g2/det_judged.*.tsv), gem2 (.claude/waves/gem2/uNNN.md, copied to g2/gem2/), re (g2/re.tsv, hand-written from g2/re.md).
v2_verdict: one source -> its verdict. Several: CONFIRMED and REFUTED together -> CONFLICT (decide by hand in g2/overrides.tsv);
otherwise the most decisive: CONFIRMED > REFUTED > PARTIAL > UNSURE. Rows not open in G1 keep the G1 verdict (v2_source g1).
g2/overrides.tsv (fid, verdict, evidence, why) wins over everything (Opus decisions on conflicts).
"""
import csv, shutil
from collections import Counter, defaultdict
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
G, W = D / "g2", Path(".claude/waves/gem2")
VERD = ["CONFIRMED", "REFUTED", "PARTIAL", "UNSURE"]
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE)) if Path(p).exists() else []
norm = lambda v: next((x for x in VERD if v.upper().strip("* ").startswith(x)), "UNSURE")

src = defaultdict(list)  # fid -> [(source, verdict, evidence, note)]
for p in sorted(G.glob("det_judged.*.tsv")):
    for r in rd(p):
        src[r["fid"]].append(("det", norm(r["verdict"]), r["evidence"], f"{r['did']}: {r['reason']}"))

(G / "gem2").mkdir(exist_ok=True)
problems = []
for u in rd(W / "units.tsv"):
    f = W / f"{u['uid']}.md"
    if not f.exists():
        problems.append(f"missing {u['uid']}")
        continue
    shutil.copyfile(f, G / "gem2" / f.name)
    want, seen = set(u["fids"].split(",")), set()
    for line in f.read_text().splitlines():
        c = [x.strip() for x in line.strip().strip("|").split("|")]
        if len(c) >= 3 and c[0] in want:
            seen.add(c[0])
            src[c[0]].append(("gem2", norm(c[1]), c[2], f"{u['uid']}: " + (c[4] if len(c) > 4 else "")))
    problems += [f"{u['uid']} no row for {x}" for x in want - seen]

for r in rd(G / "re.tsv"):
    src[r["fid"]].append(("re", norm(r["verdict"]), r["evidence"], r["note"]))
OV = {r["fid"]: r for r in rd(G / "overrides.tsv")}

out = []
for v in rd(D / "verdicts.tsv"):
    s, fid = src.get(v["fid"], []), v["fid"]
    row = dict(v)
    if fid in OV:
        o = OV[fid]
        row.update(v2_verdict=o["verdict"], v2_evidence=o["evidence"], v2_source="opus", v2_note=o["why"])
    elif not s:
        row.update(v2_verdict=v["verdict"], v2_evidence=v["evidence"], v2_source="g1", v2_note="")
    else:
        vs = {x[1] for x in s}
        if {"CONFIRMED", "REFUTED"} <= vs:
            best = "CONFLICT"
        else:
            best = next(x for x in VERD if x in vs)
        pick = [x for x in s if x[1] == best] or s
        row.update(v2_verdict=best, v2_evidence=" ; ".join(x[2] for x in pick)[:300],
                   v2_source="+".join(sorted({x[0] for x in s})), v2_note=" || ".join(f"[{x[0]} {x[1]}] {x[3]}" for x in s)[:600])
    out.append(row)

cols = list(rd(D / "verdicts.tsv")[0].keys()) + ["v2_verdict", "v2_evidence", "v2_source", "v2_note"]
with open(D / "verdicts_v2.tsv", "w", newline="") as f:
    w = csv.DictWriter(f, cols, delimiter="\t", quoting=csv.QUOTE_NONE, escapechar="\\", lineterminator="\n")
    w.writeheader()
    w.writerows([{k: str(x.get(k, "")).replace("\t", " ").replace("\n", " ") for k in cols} for x in out])

print("rows", len(out), "v2", dict(Counter(r["v2_verdict"] for r in out)))
print("G1 -> v2", dict(Counter(f"{r['verdict'][:4]}->{r['v2_verdict'][:4]}" for r in out)))
print("problems", len(problems), *problems[:20], sep="\n  ")
gate = [r["fid"] for r in out if r["verdict"] in ("PARTIAL", "UNSURE") and r["v2_source"] == "g1"]
bad_u = [r["fid"] for r in out if r["v2_verdict"] == "UNSURE" and "re" not in r["v2_source"]]
print("GATE open-without-v2", gate, "| UNSURE without re", bad_u, "| CONFLICT", [r["fid"] for r in out if r["v2_verdict"] == "CONFLICT"])
