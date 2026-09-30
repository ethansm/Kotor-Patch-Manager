#!/bin/bash
# usage: run.sh Script.java out.txt args...
G=~/Games/KOTOR2_Mod_Workspace/tools
S=$1; O=$2; shift 2
pgrep -af 'analyzeHeadless|Ghidra-Headless' | grep -v pgrep && { echo BUSY; exit 1; }
$G/ghidra/ghidra_12.1.4_PUBLIC/support/analyzeHeadless $G/ghidra_projects KOTOR2_SteamAspyr -process swkotor2.exe -noanalysis -readOnly -scriptPath $G/kotor-patch-manager/tools/ghidra_scripts -postScript $S "$@" 2>&1 | grep -v "^INFO  \(Random\|Using\|Loading\|Opening\|Processing\|HEADLESS\|Save\|REPORT\|Script\|Closing\|Release\|Project\|Import\)" | sed 's/^INFO  [A-Za-z]*.java> //; s/ (GhidraScript)  $//' > $O
grep -ci "lockexception" $O
