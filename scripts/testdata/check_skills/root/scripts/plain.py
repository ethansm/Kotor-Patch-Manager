#!/usr/bin/env python3
import argparse
ap = argparse.ArgumentParser(prog="plain.py")
ap.add_argument("file")
ap.add_argument("--top", type=int)
ap.add_argument("--json", action="store_true")
ap.parse_args()
