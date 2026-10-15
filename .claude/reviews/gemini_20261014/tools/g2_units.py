"""G2.2 step 3: add one gem2 recon unit per `read` row of g2/*.widen.tsv. Run from repo root after `wave_tools.py new --kind recon gem2`."""
import csv, subprocess, sys
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
F = {r["fid"]: r for r in rd(D / "findings.tsv")}
V = {r["fid"]: r for r in rd(D / "verdicts.tsv")}
RULE = (D / "tools" / "add_units.py").read_text().split('RULE = """', 1)[1].split('"""', 1)[0]
RULE = RULE.replace("UNSURE = needs code outside your ranges or runtime/engine knowledge.",
                    "UNSURE = needs code outside your ranges or runtime/engine knowledge (say exactly which in OPEN).")

rows, n = [], 0
for p in sorted((D / "g2").glob("*.widen.tsv")):
    for r in rd(p):
        if r.get("kind") != "read":
            continue
        n += 1
        uid = f"u{n:03d}"
        fids = [x.strip() for x in r["fids"].split(",") if x.strip()]
        lines = [f"Topic {r['tid']} (G2 widen). Question: {r['question']}", "Findings:"]
        for fid in fids:
            f, v = F[fid], V[fid]
            lines.append(f"- {fid}: {f['title']} | file {f['file']} | function {f['function']} | anchor `{f['anchor']}`"
                         f" | G1 verdict {v['verdict']} because: {v['note']} (G1 evidence {v['evidence']})")
        lines.append("The G1 verdicts above came from narrower windows. Decide each fid using the NEW windows below; you may cite G1 evidence as context.")
        lines.append(RULE.replace("# <tid>", f"# {r['tid']} ({uid})"))
        ent = [e.strip() for e in r["entries"].split(";") if e.strip() and e.strip() != "-"]
        res = subprocess.run([sys.executable, "scripts/wave_tools.py", "add", "gem2", "--id", uid, "--task", "\n".join(lines), "--entries", *ent],
                             capture_output=True, text=True)
        est = res.stdout.split("est_tokens=")[1].split()[0] if "est_tokens=" in res.stdout else "?"
        if res.returncode:
            print("FAIL", uid, r["tid"], res.stderr.strip()[-300:])
        rows.append((uid, r["tid"], ",".join(fids), p.name.split(".")[0], est))
Path(".claude/waves/gem2/units.tsv").write_text("uid\ttid\tfids\tbatch\test_tokens\n" + "".join("\t".join(x) + "\n" for x in rows))
ests = [int(x[4]) for x in rows if x[4] != "?"]
print(len(rows), "units; est total", sum(ests), "max", max(ests or [0]), "over60k", sum(e > 60000 for e in ests))
