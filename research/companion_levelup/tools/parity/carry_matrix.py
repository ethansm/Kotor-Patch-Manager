#!/usr/bin/env python3
"""B2 early-conversion matrix (34 presets with a hold class, every conversion level, hold on/off, both initial creature states) through the REAL carry-over
rule (simulate_rewrite.carry_rewrite, the Python twin of computeCarry; the C++ is proven equal to it by parity.py), audited with B2's independent 2da checks.
Re-uses phase6_evidence/CO_carryover_check.py (unchanged) with the rule swapped for the shipped one and the rule data parsed from runtime/levelup_paths.ini.
Needs pykotor (../../.venv) and the 2da snapshot of B2 (so it is not part of mutate.py). Writes the report to $TMPDIR (or --out PATH), never into phase6_evidence.
Run from research/companion_levelup:  ../../.venv/bin/python tools/parity/carry_matrix.py [--out PATH]
"""
import os, sys, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, "phase6_evidence"))
import simulate_rewrite as S
import CO_carryover_check as CK
import CO_carryover_proto as C

out = sys.argv[sys.argv.index("--out") + 1] if "--out" in sys.argv else os.path.join(os.environ.get("TMPDIR") or tempfile.gettempdir(), "carry_matrix_results.txt")
_cs, _pr, _recs, RULES = S.parse_table_v3(open(S.INI).read())
CK.OUT = out
C.carry_rewrite = S.carry_rewrite
C.build_rules_text = lambda recs, T: open(S.INI).read()
C.parse_rules_text = lambda text: RULES
CK.C = C
try:
    CK.main()
except SystemExit as e:
    print("exit", e.code, "-> report", out)
    sys.exit(e.code)
