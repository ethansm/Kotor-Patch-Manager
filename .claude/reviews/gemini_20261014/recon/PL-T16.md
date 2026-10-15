# PL-T16
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-11 | CONFIRMED | patch_manager_mods/perf_lab_src/analyze_runs.py:1192 | yes | Loaded REFUSED with no REFUSED lines yields empty ref and 'REFUSED none' at :1195. |

- :1192 anchor matches exactly. The `"REFUSED" in rb.loaded or ref` test is a no-op: both branches return sorted(set(ref)), which is [] when ref is empty.
- :1191 builds ref from rb.where("REFUSED") plus rb.lines messages containing REFUSED, skipping COUNTERS lines. Empty when no separate REFUSED line exists.
- :1195 formats "REFUSED %s" with `" / ".join(ref) if ref else "none"`, so an empty ref prints "REFUSED none".
- :1194 first format arg is rb.loaded.split(" loaded")[0]. The loaded text reaches the output only as the "robustness:" prefix, never as REFUSED detail.
- :1193 parses rb.loaded only for the "N/M groups patched" regex; no REFUSED parsing from rb.loaded in range.
- :1187-1188 render block runs only when rb is truthy; else :1199 prints "no log block matched".
- :1197 COUNTERS suffix comes from rb.counters(); unrelated to REFUSED output.
- Fix note: a correct fix must change the display fallback at :1195 (for example show the loaded REFUSED text); deleting the no-op clause alone leaves "REFUSED none".
- Implied fix (surface loaded REFUSED text when no separate lines exist) is safe for display-only code. The brief gives no explicit fix text, so fix_sound is judged from the defect.
- OPEN: whether the robustness log parser (outside range) ever leaves the loaded line out of rb.lines. If it includes it, :1191 would capture it and the "REFUSED none" path is unreachable.
