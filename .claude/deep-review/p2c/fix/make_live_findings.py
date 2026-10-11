#!/usr/bin/env python3
"""Drop STALE-* triage verdicts from p2c verified_findings.json; write a live copy (original untouched)."""
import json, pathlib
base = pathlib.Path(".claude/deep-review/p2c")
vf = json.load(open(base / "verified_findings.json"))
triage = {t["id"]: t["verdict"] for t in json.load(open(base / "triage/triage.json"))}
stale = {i for i, v in triage.items() if v in ("STALE-FIXED", "STALE-REMOVED")}
dropped = []
for tier in ("CONFIRMED", "CONFIRMED-BY-SONNET-ONLY", "CONTESTED-NEEDS-HUMAN"):
    keep = []
    for f in vf["tiers"].get(tier, []):
        fid = f.get("id") or f.get("finding_id")
        if fid in stale:
            dropped.append(fid)
        else:
            keep.append(f)
    vf["tiers"][tier] = keep
vf["live_filter"] = {"source": "verified_findings.json", "dropped": sorted(dropped), "triage": "p2c/triage/triage.json"}
json.dump(vf, open(base / "fix/verified_findings.live.json", "w"), indent=2)
print("dropped", sorted(dropped), {k: len(v) for k, v in vf["tiers"].items()})
