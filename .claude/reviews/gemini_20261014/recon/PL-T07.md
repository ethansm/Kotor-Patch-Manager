# PL-T07
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-02 | CONFIRMED | patch_manager_mods/perf_lab_src/ghidra_scripts/ExportCallGraph.java:42-46 | yes | Internal thunk falls through to line 46 and emits thunk entry; target gets no credit. |

- ExportCallGraph.java:29 `if (f.isThunk()) continue;` skips thunks as callers, so thunk-to-target edges are never emitted.
- ExportCallGraph.java:38 `fm.getFunctionAt(t)` returns the thunk Function object g for an internal thunk call site.
- ExportCallGraph.java:43 `g.getThunkedFunction(true)` resolves the chain to the final target tg.
- ExportCallGraph.java:44 only special-cases `tg != null && tg.isExternal()`; an internal tg does not match and falls through.
- ExportCallGraph.java:46 emits `hx(g.getEntryPoint())` with CALL or TAILJMP, i.e. the thunk entry, not tg's entry.
- ExportCallGraph.java:48 import/indirect fallback runs only when outs is empty, so it does not rescue the internal-thunk path.
- ExportCallGraph.java:63-64 fan-in sites and fns are keyed on the emitted callee string, so credit goes to the thunk address.
- ExportCallGraph.java:76 callers_count name lookup uses getFunctionAt on the emitted address, so the thunk's own name is listed.
- ExportCallGraph.java:1-5 header lists CALL, TAILJMP, INDIRECT, IMPORT only; no thunk-resolution contract is documented.
- Fix sound: emitting tg.getEntryPoint() for internal tg would be correct; it changes callee identity and drops thunk nodes from output.
