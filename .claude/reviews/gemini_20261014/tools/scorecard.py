"""G2.4 steps 3-4: write refuted.md (false-positive ledger) and scorecard.md (Gemini precision) from validated.tsv. Run from repo root."""
import csv
from collections import Counter, defaultdict
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
V = rd(D / "validated.tsv")
F = {r["fid"]: r for r in rd(D / "findings.tsv")}
A = {r["fid"]: r for r in rd(D / "anchors.tsv")}
cell = lambda s: s.replace("|", "\\|")

# refuted.md
out = ["# Gemini 2026-10-14: false-positive ledger", "",
       "Future reviews can skip these. Source: `validated.tsv` (real=no). The disproving evidence is the skeptic's or the v2 source's.", "",
       "| fid | G1 | v2 | file | claim | disproving evidence | why |", "|---|---|---|---|---|---|---|"]
for r in V:
    if r["real"] == "no":
        out.append(f"| {r['fid']} | {r['g1']} | {r['v2']} | `{r['file']}` | {cell(r['title'])} | {cell(r['evidence'])} | {cell(r['why'][:220])} |")
out += ["", "## NOT-FOUND anchors (G1 anchor check) and what they really pointed at", "",
        "| fid | Gemini anchor | final verdict | real code / evidence |", "|---|---|---|---|"]
for r in V:
    if A.get(r["fid"], {}).get("found") == "NOT-FOUND":
        out.append(f"| {r['fid']} | `{cell(F[r['fid']]['anchor'][:90])}` | real={r['real']} | {cell(r['evidence'])} |")
(D / "refuted.md").write_text("\n".join(out) + "\n")

# scorecard.md
def stats(rows):
    c = Counter(r["real"] for r in rows)
    n = len(rows)
    found = sum(A.get(r["fid"], {}).get("found") not in ("NOT-FOUND", None) for r in rows)
    return n, c["yes"], c["latent"], c["no"], c["unsure"], f"{100 * found / n:.0f}%", f"{100 * c['yes'] / n:.0f}%", f"{100 * (c['yes'] + c['latent']) / n:.0f}%"


hdr = "| {} | findings | real | latent | refuted | unsure | anchor hit | precision (real) | precision (real+latent) |\n|---|---|---|---|---|---|---|---|---|"
sets, conf = defaultdict(list), defaultdict(list)
for r in V:
    sets[F[r["fid"]]["set"]].append(r)
    conf[F[r["fid"]]["gemini_confidence"]].append(r)
sc = ["# Gemini 4 Argon review 2026-10-14: scorecard (G2)", "",
      "real = defect or gap confirmed reachable today; latent = true in code but unreachable/benign today; refuted = false. Severity of real rows: "
      + ", ".join(f"{k} {v}" for k, v in Counter(r["severity"] for r in V if r["real"] == "yes").items())
      + "; Gemini claimed: " + ", ".join(f"{k} {v}" for k, v in Counter(F[r["fid"]]["claimed_severity"] for r in V).items()) + ".", "",
      "## By set", "", hdr.format("set")]
sc += [f"| {s} | " + " | ".join(map(str, stats(rows))) + " |" for s, rows in sorted(sets.items())]
sc.append("| **all** | " + " | ".join(map(str, stats(V))) + " |")
sc += ["", "## By Gemini's stated confidence", "", hdr.format("confidence")]
sc += [f"| {k} | " + " | ".join(map(str, stats(rows))) + " |" for k, rows in sorted(conf.items())]
sc += ["", "## By category (real rows)", "", "| category | real | latent | refuted |", "|---|---|---|---|"]
cat = defaultdict(Counter)
for r in V:
    cat[F[r["fid"]]["category"]][r["real"]] += 1
sc += [f"| {k} | {c['yes']} | {c['latent']} | {c['no']} |" for k, c in sorted(cat.items(), key=lambda x: -sum(x[1].values()))]
claimed_hi = [r for r in V if F[r["fid"]]["claimed_severity"] in ("high", "critical")]
sc += ["", f"Claimed high/critical: {len(claimed_hi)}; validated real at any severity: {sum(r['real'] == 'yes' for r in claimed_hi)}; "
       f"validated high: {sum(r['real'] == 'yes' and r['severity'] == 'high' for r in claimed_hi)}."]
(D / "scorecard.md").write_text("\n".join(sc) + "\n")
print("\n".join(sc))
