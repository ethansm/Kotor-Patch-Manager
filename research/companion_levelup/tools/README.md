# Phase 1 helper scripts (copied from the session scratchpad)
- `ghidra_run.sh Script.java out.txt args...`: serialised read-only headless Ghidra run (refuses if another analyzeHeadless is running), strips log noise. Scripts live in ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts (DecompileArgs, DisasmRanges, Callers, ...). Output: multi-line decompile text; only the first line of each println block carries the `INFO ...>` prefix, which the script strips.
- `kdb_add.py`: `add_names([(addr,name,conf,evidence)])` -> name_map (source level-up-P1-decompile-confirmed), `add_lesson(scope,topic,note)`.
- `pebytes.py [exe] VA...`: PE VA -> file offset + first 24 bytes (default exe = `swkotor2.exe.unpatched_vanilla` in the Steam game dir; pass `swkotor2.exe` for live).
