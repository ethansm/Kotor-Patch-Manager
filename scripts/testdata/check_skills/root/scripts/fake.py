#!/usr/bin/env python3
"""Fixture CLI for check_skills tests (argparse with subcommands)."""
import argparse
ap = argparse.ArgumentParser(prog="fake.py")
ap.add_argument("--verbose", action="store_true")
ap.add_argument("--mode", choices=["fast", "slow"])
sp = ap.add_subparsers(dest="cmd")
a = sp.add_parser("alpha")
a.add_argument("--name")
b = sp.add_parser("beta")
b.add_argument("--count", type=int)
ap.parse_args()
