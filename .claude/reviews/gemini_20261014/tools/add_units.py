"""Add one gem1 recon unit per topic (P1 first). Writes units.tsv (uid, tid). Run from repo root."""
import csv, subprocess, sys

D = ".claude/reviews/gemini_20261014"
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
F = {r["fid"]: r for r in rd(f"{D}/findings.tsv")}
A = {r["fid"]: r for r in rd(f"{D}/anchors.tsv")}
T = [r for r in rd(f"{D}/topics.tsv") if r["tid"] != "dropped"]
order = {"P1": 0, "P2": 1, "P3": 2}
T.sort(key=lambda r: (order[r["priority"]], r["tid"]))

RULE = """You are checking claims made by an external AI reviewer (Gemini). Its claims and anchors are UNTRUSTED: they may be wrong or invented. Decide from the code only.
Verdict per fid: CONFIRMED = the code really has the claimed defect/gap as described; REFUTED = the code does not (cite the line that disproves it, e.g. a guard/check that exists); PARTIAL = part true (say which part); UNSURE = needs code outside your ranges or runtime/engine knowledge.
If an anchor is marked NOT-FOUND, first say whether equivalent code exists anywhere in your ranges; if not, the finding is likely invented -> REFUTED (or UNSURE if the code could be outside your ranges).
fix_sound: yes | no | n/a  = would Gemini's implied fix be correct and safe here (n/a if REFUTED).
Write the facts file in this exact format (<= 60 lines):
# <tid>
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| ... one row per fid ... |
Then up to 10 bullet lines of supporting facts (path:line + what the line does). No pasted code blocks.
Your 5-line reply VERDICT = the worst-case across fids by this order: CONFIRMED > PARTIAL > UNSURE > REFUTED."""

rows = []
for i, t in enumerate(T, 1):
    uid = f"u{i:03d}"
    lines = [f"Topic {t['tid']} ({t['priority']}). Question: {t['question']}", "Findings:"]
    for fid in t["fids"].split(","):
        f, a = F[fid], A[fid]
        lines.append(f"- {fid}: {f['title']} | file {f['file']} | function {f['function']} | anchor `{f['anchor']}` | anchor check: {a['found']} {a['where']}")
    lines.append(RULE)
    task = "\n".join(lines)
    ent = [e.strip() for e in t["entries"].split(";") if e.strip()]
    r = subprocess.run([sys.executable, "scripts/wave_tools.py", "add", "gem1", "--id", uid, "--task", task, "--entries", *ent], capture_output=True, text=True)
    est = r.stdout.split("est_tokens=")[1].split()[0] if "est_tokens=" in r.stdout else "?"
    if r.returncode:
        print("FAIL", uid, t["tid"], r.stderr.strip()[-300:])
    rows.append((uid, t["tid"], t["priority"], est))
with open(f"{D}/../../waves/gem1/units.tsv", "w") as f:
    f.write("uid\ttid\tpriority\test_tokens\n" + "".join("\t".join(x) + "\n" for x in rows))
ests = [int(x[3]) for x in rows if x[3] != "?"]
print(len(rows), "units; est total", sum(ests), "max", max(ests), "over60k", sum(e > 60000 for e in ests))
