#!/usr/bin/env python3
"""Hand-rolled dispatcher fixture.

Usage:
    handrolled.py run NAME [--fast]
"""
import sys

if __name__ == "__main__":
    a = sys.argv[1:]
    if not a or a[0] in ("-h", "--help"):
        print(__doc__)
    elif a[0] == "run" and a[1:2] in (["-h"], ["--help"]):
        print("usage: handrolled.py run NAME [--fast] [--deep]")
    sys.exit(0)
