#!/usr/bin/env python3
"""Adapter between the engine's generic compat gate and scripts/compat_check.py (KOTOR2).

Engine contract (fixgate.sh): run in the gate clone (cwd), files as positional args, --json <path>.
  DR_GATE_PLAN=1  -> write {"plan": [{"kind": "build", "unit": <mod>, "id": ...}], "results": []} (no build, no test)
  otherwise       -> run the compat check and let it write its own results json (rc passes through).
compat_check.py names the units to rebuild as plan ids "build_check:<mod>"; the engine wants a "unit" field.
"""
import json
import os
import re
import subprocess
import sys


def main(argv):
    jobs, out, files = "4", None, []
    i = 0
    while i < len(argv):
        if argv[i] == "--jobs":
            jobs = argv[i + 1]; i += 2
        elif argv[i] == "--json":
            out = argv[i + 1]; i += 2
        else:
            files.append(argv[i]); i += 1
    if not out or not files:
        print("kotor2_compat_adapter: need --json <path> and at least one file", file=sys.stderr)
        return 2
    base = [sys.executable, "scripts/compat_check.py", "--repo", ".", *files]
    if os.environ.get("DR_GATE_PLAN") != "1":
        return subprocess.run(base + ["--jobs", jobs, "--json", out]).returncode
    rc = subprocess.run(base + ["--list", "--json", out], stdout=subprocess.DEVNULL).returncode
    try:
        data = json.load(open(out, encoding="utf-8"))
    except (OSError, ValueError):
        data = {"plan": [], "results": []}
    plan = []
    for it in data.get("plan", []):
        m = re.match(r"^build_check:([A-Za-z0-9_.-]+)$", str(it.get("id", "")))
        if it.get("kind") == "build" and m:
            plan.append({"kind": "build", "unit": m.group(1), "id": it["id"]})
    json.dump({"plan": plan, "results": []}, open(out, "w", encoding="utf-8"))
    return rc


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
