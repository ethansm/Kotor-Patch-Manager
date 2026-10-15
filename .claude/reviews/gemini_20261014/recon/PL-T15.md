# PL-T15
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-10 | PARTIAL | patch_manager_mods/perf_lab_src/analyze_runs.py:630-636; analyze_runs.py:555 | yes | Module overwrite CONFIRMED (no outer break); tick-as-flag needs attach kv to hold tick, not in range |

- analyze_runs.py:629-630: fallback `for b in (down, up):` runs only when L["module"] is still None; down is visited before up.
- analyze_runs.py:633-636: inner `for key in ("mod","area")` breaks only itself; the outer loop at 630 has no break, so up's mod/area overwrites down's module.
- analyze_runs.py:622-625: earlier pass prefers module_down over module_up with a break, so down-first is the existing precedence.
- analyze_runs.py:626-628: "-" on either side sets module to "-"; fallback reached only when both module_down and module_up are None.
- analyze_runs.py:555: attach kv filter drops only "exe" and "scan"; "tick" is not dropped.
- analyze_runs.py:556-557: "scan" is re-added after the filter, so effectively only "exe" is dropped from attach kv.
- analyze_runs.py:559-560: block hdr loop drops "tick" (plus module, live_bytes, allocs); the tick filter exists only on the block path, not attach kv.
- analyze_runs.py:620-621: module_up / module_down come from Block.module (defined outside read ranges, not verified).
- analyze_runs.py:630 anchor check: exact match for `for b in (down, up):`.
- OPEN: does attach kv ever carry "tick", and can a block have hdr mod/area while .module is None? Parser code outside ranges, not read.
