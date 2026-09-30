#!/bin/bash
# usage: perf_lab_src/gh.sh Script.java args... [-postScript Other.java args...]  (read-only, shared project; check `pgrep -f support/analyzeHeadless` first)
GHIDRA=~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC
R="$(cd "$(dirname "$0")" && pwd)/ghidra_scripts"
"$GHIDRA/support/analyzeHeadless" ~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects KOTOR2_SteamAspyr -process swkotor2.exe -noanalysis -readOnly -scriptPath "$R;$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts" -postScript "$@" 2>&1 | sed 's/^INFO  [A-Za-z0-9]*\.java> //; s/ (GhidraScript)$//'
