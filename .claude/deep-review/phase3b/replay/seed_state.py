#!/usr/bin/env python3
"""Mark the audit stages before `adjudicate` as done in a SCRATCH replay run (rl1/rl2). Usage: seed_state.py <run>.
Their artifacts were copied from p1 by build_rl1.py. Cost 0, note says seeded. Never touches human gates, never run on p1."""
import json, sys, datetime
run = sys.argv[1]
assert run not in ("p1", "p1-lens", "p2", "h1"), "scratch runs only"
p = f"/home/ethansm/.cache/deep_haiku_review/kotor2-pilot/{run}/state.json"
s = json.load(open(p))
now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
for n in ["prep", "canary", "facts", "map-first", "resize", "map", "reduce", "merge", "verify-select", "verify"]:
    s["stages"].setdefault(n, {"status": "done", "rc": 0, "cost_usd": 0.0, "duration_s": 0.0, "finished": now, "log": "", "pid": None,
                                "runs": 0, "note": "seeded from p1 for replay scratch run"})
s["log"].append({"at": now, "event": "seed_state: audit stages marked done (replay scratch)", "by": "orchestrator"})
json.dump(s, open(p, "w"), indent=1)
q = p.rsplit("/", 1)[0] + "/work/verify_queue.json"   # build_dossiers needs a queue; empty = no extra batches
json.dump({"items": []}, open(q, "w"))
print("seeded", run)
# build_dossiers --keep-existing only keeps work/adjudicate when verify/adjudications/batch_*.json exists; with our empty queue it would wipe the copied
# dossiers. An inert placeholder (batch_00, no items) makes the stage keep them. score_replay.py must ignore batch_00.
import os
d = os.path.dirname(p) + "/verify/adjudications"
os.makedirs(d, exist_ok=True)
json.dump({"batch": 0, "status": "seeded-placeholder", "output": {"items": []}}, open(d + "/batch_00.json", "w"))
