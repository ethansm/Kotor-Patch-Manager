#!/usr/bin/env python3
"""Deliberately failing script for `scripts/run_tests.py --selftest`: must exit 1.
Not matched by discovery (not test_*.py)."""
import sys

print("fail_py: deliberate failure")
sys.exit(1)
