# PL-T09
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-03 | PARTIAL | patch_manager_mods/perf_lab_src/analyze_runs.py:415-416, 524-526, 1327 | no | No-merge is real; tick compare is half-range; ALL_FILES content outside range. |

- analyze_runs.py:409-417 parse_perflab_pair returns the whole new pair or the whole old pair; no concatenation, so old-file attaches are dropped when new wins (415-416).
- analyze_runs.py:514-526 parse_textlog_pair is also pick-one; blocks are relabelled to the legacy name (523) and returned whole (525-526).
- analyze_runs.py:515-517 docstring documents pick-one (newest loaded: tick wins; ties and empty fall back to legacy). The drop is intended, not an oversight.
- analyze_runs.py:524 anchor matches exactly; the 415 expression has the same and/or precedence, so both conditions behave as written.
- analyze_runs.py:415 and 524 use tdiff(a,b) < 0x80000000, a half-range compare. It orders ticks only when the two files' last ticks are under about 24.8 days apart; a reboot resets the tick base and can misorder.
- tdiff() is defined outside the range, so whether it is modular is unverified.
- analyze_runs.py:1327 render() Sources table iterates ALL_FILES only; a core-named file gets no row unless ALL_FILES lists it.
- analyze_runs.py:1335 a later loop iterates LOG_FILES, so the two file sets differ; its output is outside range.
- ABSENT comes from infos[n].present and status(), built outside range; unverified.
- fix_sound no: a merge across both files risks double-counting launches present in both; no dedup key in range.
