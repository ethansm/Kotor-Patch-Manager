# PL-T11

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-05 | PARTIAL | patch_manager_mods/perf_lab_src/resolve_sites.py:71, :73 | yes | Blank line in block raises IndexError at 73 (confirmed). RT int() ValueError real; producer emission unverified. |

- resolve_sites.py:71 `v = [int(x) for x in l.split()[1:]]` has no try/except; a token like `redemB=5` or `stat` makes int() raise ValueError, and mblocks (:60-74) has no handler. Anchor matches exactly.
- resolve_sites.py:70 gates the RT branch on `l.startswith('RT ')`; RTF (:49) is 11 integer counter names. No `redemB` field exists (closest is `redem60`).
- resolve_sites.py:73 `elif l[:1] != '#': ... l.split()[0]`: a blank line gives l[:1]=='' (not '#'), so `''.split()[0]` raises IndexError. Whitespace-only lines also raise.
- resolve_sites.py:66 `elif cur is None: continue` skips blank lines outside a #METRICS..#END block, so only blanks inside a block crash.
- resolve_sites.py:67 `#END` sets cur=None; blank lines after #END are safe.
- resolve_sites.py:53-57 kv() wraps float() in try/except ValueError and keeps the string, so non-numeric tokens do not crash header or line-section parsing. Only the RT path at :71 is unguarded.
- resolve_sites.py:71 an `RT ` line with no integer tokens leaves v empty; v[0] raises IndexError.
- resolve_sites.py:71 dict(zip(RTF, v[1:])) silently truncates; an RT line with fewer than 11 values lacks 'resident' and similar keys, raising KeyError later in mdiff (:85, :95-96).
- resolve_sites.py:128 metrics mode calls mblocks with no try/except; a raise aborts the run with a traceback and no partial output.
- The finding's phrase "launches not split" has no matching text in lines 45-141; not verified.
