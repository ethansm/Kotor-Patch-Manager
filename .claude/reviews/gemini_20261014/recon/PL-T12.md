# PL-T12
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-07 | CONFIRMED | patch_manager_mods/perf_lab_src/persist_findings.py:8,15,16 | yes | Empty or trailing-comma argv[2] yields '' token; every row forced high |

- persist_findings.py:8 - anchor `u=sys.argv[1]; ver=sys.argv[2].split(',')` matches exactly; ''.split(',') gives [''], 'a,'.split(',') gives ['a','']. No empty-token filter.
- persist_findings.py:15 - `v=any(x in ev.lower() for x in ver)`; '' is a substring of every string, so v is True for every row.
- persist_findings.py:16 - conf becomes 'high' whenever v is True, regardless of the row's own confidence.
- persist_findings.py:19 - rows absent from name_map are inserted with the forced confidence.
- persist_findings.py:20 - existing non-high rows are overwritten to high, replacing name, source, evidence and date.
- persist_findings.py:23 - functions rows get the "Opus-verified" tag whenever v is True, a false provenance label.
- persist_findings.py:24 - c.commit() persists all writes; the file has no dry-run and no guard.
- persist_findings.py:8 - no len(sys.argv) check; a missing argv[2] raises IndexError instead of failing silently.
- Implied fix: drop empty tokens (filter on t.strip()) and reject an empty argv[2]. Sound; no other behaviour changes.
