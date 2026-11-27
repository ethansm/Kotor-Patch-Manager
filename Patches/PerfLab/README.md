# PerfLab -- engine performance diagnostics

> **STATUS** Track: DIAGNOSTIC. Version 0.6.0 (matches manifest.toml).
> **What it does / explores:** engine performance measurement: heap-allocation census via IAT hooks, the M1-M7 metrics pack, an address-space census, engine live counters, per-hook cost, and flag-gated experiments (`tx`, `g1skip`, `hwmip`, `abreserve`). Writes `perflab_*.txt` in the game dir only.
> **Requires:** Steam/Aspyr KOTOR II build 6A522E71 (LAA); KPM framework for the build; no mod dependency (`requires = []`). Wine smokes also need Proton (see Environment).
> **Known limits:** the experiments are off by default and change engine behaviour when on; the census is a CPU and disk cost in normal play; the 0.6.0 counters (RB, AI, TEXSCAN, FOCUS_STRCMP, TOLOWER) have no smoke row; `resolve_sites.py` and the `PerfLab.cpp` logic have no host unit test. **Testing:** this is research output, not a pass/fail gate.
> **Learned:** the resman hooks cost about 0.8-1 us per call (SafeRead `VirtualQuery`), so they add up over a load; the census left on from 10-05 to 10-25 (`alloc=1 heapfilter=1 scan=8 metrics=1 vq=1`) wrote a 97 MB `perflab_metrics.txt` in one session.
> **Install:** do not install for normal play. To measure: rename `perflab_debug.txt.disabled` back to `perflab_debug.txt` in the game dir, play, then rename it off again (`.off` holds a different key set; keep both). The DLL stays installed and is inert without the flag file.


## Usage

Inert unless `perflab_debug.txt` exists in the game directory. Results are written to `perflab_*.txt`. Experiments (`tx=1`, `g1skip=1`, `hwmip=1`) are off by default. It conflicts with `k2-texture-memory-release` (both touch `0x004261CF` when `tx=1`), so install only one.

## Build

From `Patches/PerfLab`: `python3 ../create-patch.py`.
