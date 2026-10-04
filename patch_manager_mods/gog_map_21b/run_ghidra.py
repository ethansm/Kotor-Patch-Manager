#!/usr/bin/env python3
"""Run ghidra_server_api.py's export-script CLI with SCRIPT_PATH pointed at this dir's ghscripts/ (read-only Export*.java).
usage: run_ghidra.py gog|steam <outdir>   (outdir must contain requests.txt)"""
import sys, importlib.util
from pathlib import Path
which, out = sys.argv[1], sys.argv[2]
prog = {"gog": "k2_win_gog_aspyr_swkotor2.exe", "steam": "k2_win_steam_aspyr_swkotor2.exe"}[which]
spec = importlib.util.spec_from_file_location("gsa", "/home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/ghidra_server_api.py")
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
m.SCRIPT_PATH = Path(__file__).resolve().parent / "ghscripts"
sys.argv = ["ghidra_server_api.py", "export-script", prog, "ExportRangeDisasm.java", "--out", out]
m.main()
