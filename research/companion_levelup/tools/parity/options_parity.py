#!/usr/bin/env python3
"""Compile options_test.cpp (host g++), run it and options_twin.py, diff the traces."""
import difflib
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CL = os.path.abspath(os.path.join(HERE, "..", ".."))
INI = os.path.join(CL, "runtime", "levelup_paths.ini")
BIN = os.path.join(HERE, "options_test_bin")


def main():
    r = subprocess.run(["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", "-o", BIN,
                        os.path.join(HERE, "options_test.cpp")], capture_output=True, text=True)
    if r.returncode:
        sys.stderr.write(r.stderr)
        print("options parity: COMPILE FAILED")
        return 1
    c = subprocess.run([BIN, INI], capture_output=True, text=True)
    t = subprocess.run([sys.executable, os.path.join(CL, "options_twin.py"), "--trace-only", INI],
                       capture_output=True, text=True)
    if c.returncode or t.returncode:
        sys.stderr.write(c.stderr + t.stderr)
        print("options parity: run failed (cpp rc=%d, twin rc=%d)" % (c.returncode, t.returncode))
    cl = [x for x in c.stdout.splitlines() if not x.startswith("# ")]
    tl = [x for x in t.stdout.splitlines() if not x.startswith("# ")]
    diff = [d for d in difflib.unified_diff(tl, cl, "twin", "cpp", lineterm="", n=0)]
    mism = sum(1 for d in diff if d[:1] in "+-" and not d.startswith(("+++", "---")))
    for d in diff[:60]:
        print(d)
    print("options parity: %d lines, %d mismatches" % (len(tl), mism))
    return 1 if (mism or c.returncode or t.returncode or not tl) else 0


if __name__ == "__main__":
    sys.exit(main())
