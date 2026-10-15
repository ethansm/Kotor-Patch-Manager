"""Copy gem1 facts files to recon/<tid>.md and build verdicts.tsv. Run from repo root."""
import csv, re, shutil
from collections import Counter, defaultdict
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
W = Path(".claude/waves/gem1")
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
units = rd(W / "units.tsv")
F = {r["fid"]: r for r in rd(D / "findings.tsv")}
T = {r["tid"]: r for r in rd(D / "topics.tsv")}
VERD = {"CONFIRMED", "REFUTED", "PARTIAL", "UNSURE"}
(D / "recon").mkdir(exist_ok=True)

out, missing, problems = [], [], []
for u in units:
    src = W / f"{u['uid']}.md"
    if not src.exists():
        missing.append(u["uid"])
        continue
    shutil.copyfile(src, D / "recon" / f"{u['tid']}.md")
    want = set(T[u["tid"]]["fids"].split(","))
    seen = set()
    for line in src.read_text().splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) < 4 or cells[0] not in want:
            continue
        fid, v = cells[0], cells[1].upper().strip("* ")
        v = next((x for x in VERD if v.startswith(x)), v)
        if v not in VERD:
            problems.append(f"{u['uid']} {fid} bad verdict {cells[1]!r}")
        seen.add(fid)
        out.append({"fid": fid, "tid": u["tid"], "uid": u["uid"], "priority": u["priority"], "set": F[fid]["set"],
                    "verdict": v, "evidence": cells[2], "fix_sound": cells[3] if len(cells) > 3 else "",
                    "note": cells[4] if len(cells) > 4 else "", "title": F[fid]["title"]})
    for fid in want - seen:
        problems.append(f"{u['uid']} {u['tid']} no row for {fid}")

cols = ["fid", "tid", "uid", "priority", "set", "verdict", "evidence", "fix_sound", "note", "title"]
with open(D / "verdicts.tsv", "w", newline="") as f:
    w = csv.DictWriter(f, cols, delimiter="\t", quoting=csv.QUOTE_NONE, escapechar="\\", lineterminator="\n")
    w.writeheader()
    w.writerows(sorted(out, key=lambda r: (r["priority"], r["tid"], r["fid"])))

print("rows", len(out), "of", len(F), Counter(r["verdict"] for r in out))
print("missing units", missing)
print("problems", len(problems), *problems[:20], sep="\n  ")
by = defaultdict(Counter)
for r in out:
    by[r["set"]][r["verdict"]] += 1
for s in sorted(by):
    print(s, dict(by[s]))
