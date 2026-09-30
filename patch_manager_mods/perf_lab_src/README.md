# perf_lab_src -- engine research tooling (doc 10)

- `perflab/` -- source of record for the `perf-lab` patch (framework copy: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/PerfLab/`; build: `cd Patches/PerfLab && python3 ../create-patch.py`). 1a allocation census via IAT hooks; inert unless `perflab_debug.txt` exists (`.off` = disabled). Keys and outputs: header of `PerfLab.cpp`.
- `resolve_sites.py` -- `report|snaps|diff` on perflab output; maps site addresses through `ghidra_knowledge/steam_functions.tsv` and the DB `name_map`/`functions`.
- `harness/` -- Wine smoke tests: `smoke.cpp` (probe 1.1.0), `smoke_perflab.cpp` (census accounting + overhead), `smoke_probe12.cpp` (probe 1.2.0 rings/report). Build with `i686-w64-mingw32-g++ -O1 -static`; run `WINEPREFIX=<throwaway> WINEDEBUG=-all "$HOME/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine" x.exe` in a dir holding the DLL (`unzip -j <kpatch> binaries/windows_x86.dll`, rename for perflab: `perflab.dll`).
- `ghidra_scripts/` -- copies of scripts written for this program (live in `kotor-patch-manager/tools/ghidra_scripts/`): `ExportAllFunctions.java` (arg = out tsv), `FindFieldWrites.java` (`<offHex> <fromHex> <toHex>`). DecompileArgs/Callers/DisasmArgs/ImportCallers take BARE hex addresses (no `0x`).
- Probe 1.2.0 source: `../load_hang_probe_src/`.
