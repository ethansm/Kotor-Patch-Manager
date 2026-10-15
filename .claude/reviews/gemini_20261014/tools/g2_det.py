"""G2.2 step 2: run every `det` row of g2/*.widen.tsv and write g2/det.tsv. Run from repo root. Re-runnable.

Only read-only commands run (rg, wc, awk, ls, git ls-files/grep/log); pipes between them are allowed, redirects, ';', '&&', '$(' and backticks are not.
A question in prose may carry the command after 'Run:'. Full output (<=60 lines) goes to g2/det_out/dNNN.txt for the judge.
verdict: from the row's rule ("0 hits => X; >=1 hit => Y"); '?' when the rule cannot be parsed (a Haiku judge decides, g2/det_judged.tsv).
"""
import csv, re, shlex, subprocess
from collections import Counter
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
ALLOW = {"rg", "wc", "ls", "awk"}
GIT_OK = {"ls-files", "grep", "log"}
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))


def verdict(rule, hits):
    for cond, v in re.findall(r"(0 hits?|>=?\s*1 hits?|[<>]=?\s*\d+[^=;]*?)\s*=>\s*([A-Z]+)", rule):
        n = re.search(r"\d+", cond)
        n = int(n.group()) if n else 0
        if cond.startswith("0") and hits == 0:
            return v
        if cond.startswith(">=") and hits >= n or cond.startswith(">") and not cond.startswith(">=") and hits > n:
            return v
        if cond.startswith("<") and hits < n:
            return v
    return "?"


rows, n = [], 0
for p in sorted((D / "g2").glob("*.widen.tsv")):
    for r in rd(p):
        if r.get("kind") != "det":
            continue
        q = r["question"].strip()
        m = re.search(r"Run:\s*(.+?)(?:\s+=>.*)?$", q)
        cmd = (m.group(1) if m else q).strip().strip("`").rstrip(".")
        lex = shlex.shlex(cmd, posix=True, punctuation_chars=True)
        lex.whitespace_split = True
        try:
            toks = list(lex)
        except ValueError:
            toks = [";"]
        ops = [t for t in toks if t and set(t) <= set("();<>|&")]
        segs = [x.strip() for x in cmd.split(" | ")]
        def ok(seg):
            try:
                a = shlex.split(seg)
            except ValueError:
                return False
            return bool(a) and (a[0] in ALLOW or (a[0] == "git" and len(a) > 1 and a[1] in GIT_OK))
        bad = any(o != "|" for o in ops) or "$(" in cmd or "`" in cmd or len(segs) != ops.count("|") + 1 or not all(ok(x) for x in segs)
        n += 1
        if bad:
            out, hits, v, full = "REJECTED (not an allowed read-only command)", -1, "?", ""
        else:
            res = subprocess.run(["bash", "-c", cmd.replace("~/", str(Path.home()) + "/")], capture_output=True, text=True)
            lines = [l for l in res.stdout.splitlines() if l.strip()]
            if len(segs) == 1 and segs[0].startswith("rg") and (" -c" in segs[0] or "--count" in segs[0]):
                hits = sum(int(l.rsplit(":", 1)[-1]) for l in lines if l.rsplit(":", 1)[-1].isdigit())
            else:
                hits = len(lines)
            full = "\n".join(lines[:60]) + (f"\n... ({len(lines)} lines)" if len(lines) > 60 else "") + (("\nSTDERR: " + res.stderr.strip()[:300]) if res.stderr.strip() else "")
            out = " / ".join(lines[:4])[:400]
            v = verdict(r.get("rule", ""), hits)
        (D / "g2" / "det_out").mkdir(exist_ok=True)
        (D / "g2" / "det_out" / f"d{n:03d}.txt").write_text(f"fids: {r['fids']}\nquestion: {q}\ncmd: {cmd}\nrule: {r.get('rule','')}\nhits: {hits}\n--- output ---\n{full}\n")
        for fid in [x.strip() for x in r["fids"].split(",") if x.strip()]:
            rows.append({"did": f"d{n:03d}", "batch": p.name.split(".")[0], "tid": r["tid"], "fid": fid, "cmd": cmd, "hits": hits,
                         "verdict": v, "rule": r.get("rule", ""), "sample": out.replace("\t", " ")})

cols = ["did", "batch", "tid", "fid", "cmd", "hits", "verdict", "rule", "sample"]
with open(D / "g2" / "det.tsv", "w", newline="") as f:
    w = csv.DictWriter(f, cols, delimiter="\t", quoting=csv.QUOTE_NONE, escapechar="\\", lineterminator="\n")
    w.writeheader()
    w.writerows(rows)
print("det rows", len(rows), dict(Counter(r["verdict"] for r in rows)))
for r in rows:
    if r["verdict"] == "?":
        print("  ?", r["fid"], r["hits"], r["cmd"][:120], "|", r["rule"][:80])
