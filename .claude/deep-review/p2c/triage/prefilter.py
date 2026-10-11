import json, subprocess, os
T = ".claude/deep-review/p2c/triage"
d = json.load(open(".claude/deep-review/p2c/verified_findings.json"))
norm = lambda s: " ".join(s.split())
cache = {}
def filetext(p):
    if p not in cache:
        try: cache[p] = norm(open(p, errors="replace").read())
        except OSError: cache[p] = None
    return cache[p]
def sh(*a):
    return subprocess.run(a, capture_output=True, text=True).stdout
out = []
for tier in ("CONFIRMED", "CONFIRMED-BY-SONNET-ONLY", "CONTESTED-NEEDS-HUMAN"):
    for it in d["tiers"][tier]:
        qp = []
        for q in it.get("evidence_quotes", []):
            t = filetext(q["file"])
            qp.append(t is not None and norm(q["text"]) in t)
        allq = bool(qp) and all(qp)
        cs = sh("git", "log", "--format=%h %ad %s", "--date=short", "7020d54..HEAD", "--", it["file"]).splitlines()[:8]
        dirty = bool(sh("git", "status", "--short", "--", it["file"]).strip())
        cls = "NEEDS-AGENT" if not allq else ("UNCHANGED-REAL" if not cs and not dirty else "LIKELY-REAL")
        out.append(dict(id=it["id"], tier=tier, final_severity=it.get("final_severity"), file=it["file"], lines=it.get("lines"),
            claim=it["claim"], evidence_quotes=it.get("evidence_quotes", []), quotes_present=qp, commits_since=cs, dirty=dirty, **{"class": cls}))
json.dump(out, open(f"{T}/prefilter.json", "w"), indent=1)
from collections import Counter
c = Counter(o["class"] for o in out)
print(len(out)); print(dict(c))
for o in out: print(o["id"], o["tier"][:9], o["final_severity"], o["class"], o["file"], len(o["commits_since"]), o["dirty"])
