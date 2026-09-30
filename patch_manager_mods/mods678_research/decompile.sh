#!/usr/bin/env bash
# Headless Ghidra decompile of a comma-separated list of Steam addresses (function containing each,
# plus its xrefs), read-only, never re-analyzing. Output: $2 (the REPORT section only).
# Usage: decompile.sh 8a8df0,7b3a00,... OUT.txt
# One Ghidra job at a time (project lock; lesson 126: a LockException right after a previous run
# clears on a plain retry). Batch 1 (24 fns) took ~2 min.
set -euo pipefail
GHIDRA=~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC
SCRIPTS=~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts
RAW="$2.raw"
timeout 900 "$GHIDRA/support/analyzeHeadless" ~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects KOTOR2_SteamAspyr \
  -process swkotor2.exe -noanalysis -readOnly -scriptPath "$SCRIPTS" -postScript DecompileList.java "$1" > "$RAW" 2>&1
sed -n '/REPORT: Execute script/,$p' "$RAW" > "$2"
rm -f "$RAW"
grep -E "=== " "$2" | sed 's/.*=== //'
