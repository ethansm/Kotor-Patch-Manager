# perf_lab_src -- engine research tooling (doc 10)

- `perflab/` -- source of record for the `perf-lab` patch (framework copy: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/PerfLab/`; build: `cd Patches/PerfLab && python3 ../create-patch.py`). 1a allocation census via IAT hooks; inert unless `perflab_debug.txt` exists (`.off` = disabled). Keys and outputs: header of `PerfLab.cpp`.
- `resolve_sites.py` -- `report|snaps|diff` on perflab output; maps site addresses through `ghidra_knowledge/steam_functions.tsv` and the DB `name_map`/`functions`.
- `harness/` -- Wine smoke tests: `smoke.cpp` (probe 1.1.0), `smoke_perflab.cpp` (census accounting + overhead), `smoke_probe12.cpp` (probe 1.2.0 rings/report). Build with `i686-w64-mingw32-g++ -O1 -static`; run `WINEPREFIX=<throwaway> WINEDEBUG=-all "$HOME/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine" x.exe` in a dir holding the DLL (`unzip -j <kpatch> binaries/windows_x86.dll`, rename for perflab: `perflab.dll`).
- `ghidra_scripts/` -- copies of scripts written for this program (live in `kotor-patch-manager/tools/ghidra_scripts/`): `ExportAllFunctions.java` (arg = out tsv), `FindFieldWrites.java` (`<offHex> <fromHex> <toHex>`). DecompileArgs/Callers/DisasmArgs/ImportCallers take BARE hex addresses (no `0x`).
- Probe 1.2.0 source: `../load_hang_probe_src/`.

## Ghidra headless recipe used in the 2026-09-30 session (read-only, shared project; check `pgrep -f analyzeHeadless` first, note `pgrep -f` also matches your own shell)
```
GHIDRA=~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC
"$GHIDRA/support/analyzeHeadless" ~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects KOTOR2_SteamAspyr -process swkotor2.exe -noanalysis -readOnly \
  -scriptPath <dir with the .java> -postScript DecompileArgs.java 00798820 00798c00 2>&1 | sed 's/^INFO  [A-Za-z0-9]*\.java> //; s/ (GhidraScript)$//' | sed -n '/=== FUN_/,$p'
```
`ghidra_scripts/RefsTo.java <hexaddr...>` (new): every reference to an address with function + instruction (finds writers/readers of a global such as 0xA1B4E0 and callers of a function). `DecompileArgs`/`DisasmArgs` take bare hex. Function containing an address: bisect `ghidra_knowledge/steam_functions.tsv` (python, `int(addr,16)`; no awk strtonum here). A whole headless run takes ~30-60 s.

## Wine harness for hooks that read fixed engine addresses (`harness/smoke_probe13.cpp`)
Wine fills 0x810000-0xB4A000, so `VirtualAlloc` at 0xA1B000 fails (487). Trick: link the harness exe dynamically at the default base with a 7 MB `.bss` array so its own image covers 0x400000-0xB00000 (engine globals and a stub at 0x40BCF0 become ordinary memory; `VirtualProtect` the stub page RWX); `-static-libgcc -static-libstdc++` (a plain dynamic MinGW exe dies silently under WINEDEBUG=-all). Never call `GetWindowText` on a window of the same process from a watchdog thread (blocks on the stalled main thread).

