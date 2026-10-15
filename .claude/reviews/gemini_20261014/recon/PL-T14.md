# PL-T14

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-09 | PARTIAL | patch_manager_mods/perf_lab_src/analyze_runs.py:331; :987-988 | no | Read gap real (987 reads only TPC key). Origin text, TPC_RESTYPE value, other readers out of range. Stat-as-TPC fallback unproven. |

- analyze_runs.py:331: when `ints` is None, parsed redemB is stored as blk.rt_redem["stat"]; otherwise under rt_redem[ints] (line 329).
- analyze_runs.py:318-326: redemB read from tokens p[13:], fallback p[1:]; stored only if is_num(red).
- analyze_runs.py:987: analyze builds reds from rt_redem[TPC_RESTYPE] only; the "stat" key is not read anywhere in range.
- analyze_runs.py:988: a["redemB"] = last element of reds, else None; no fallback to rt_redem["stat"] in range.
- analyze_runs.py:983-986: other a[...] fields come from x.cum(); none touch rt_redem, so no alternate path to redemB in range.
- OPEN: the "RT stat redemB=" text and the `ints` None condition sit above line 318, outside the brief's ranges; not verified.
- OPEN: TPC_RESTYPE definition is outside range; if it equalled "stat" the finding would be false (very unlikely).
- OPEN: no check for other readers of rt_redem["stat"] or other writers of a["redemB"] outside 318-992.
- OPEN: unproven that stat redemB is the same metric as TPC-type redemB; Gemini's fallback would mix metrics if not.
- Intent unclear: line 987 is explicitly TPC-keyed, so the omission may be deliberate scoping rather than a defect.
