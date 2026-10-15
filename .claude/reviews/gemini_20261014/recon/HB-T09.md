# HB-T09
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1d-04 | PARTIAL | patch_manager_mods/action_hotbar_src/tlk_test.cpp:66-68, 227-228 | yes | mkdtemp plus system rm -rf confirmed; early-exit leak unverifiable in ranges; kt::TempDir unchecked |
| HB-1d-05 | CONFIRMED | patch_manager_mods/action_hotbar_src/twoda_test.cpp:188-191 | yes | TLK step is printf only, no KT_CHECK; skip branch also passes silently |

- tlk_test.cpp:66 template = $TMPDIR (or /tmp) + "/tlk_test.XXXXXX"; TMPDIR is a user-controlled env var.
- tlk_test.cpp:68-69 mkdtemp result stored in global g_dir; failure path returns 1 before any dir exists (no leak there); no RAII owner.
- tlk_test.cpp:227-228 cleanup builds "rm -rf '<g_dir>'" and calls system(); nonzero exit only prints a warning.
- tlk_test.cpp:225-230 cleanup sits at end of main; any return in lines 76-219 (not read, out of range) would skip it. OPEN.
- tlk_test.cpp:227 single-quoted shell string: a TMPDIR containing a single quote breaks rm and leaves the dir behind.
- twoda_test.cpp:188-189 hb::Tlk k is opened only if argc > 2, so the TLK step is optional.
- twoda_test.cpp:190 printf of name/desc for rows 8 and 10; no KT_CHECK anywhere in this step.
- twoda_test.cpp:191-192 else-branch prints "tlk skipped" with k.error; no failure recorded; kt::Report at 192 gets nothing from this step.
- twoda_test.cpp:185-187 is a separate block with its own KT_CHECK; not part of the TLK step.
- Anchors: HB-1d-04 NOT-FOUND only from quoting (equivalent at tlk_test.cpp:227). HB-1d-05 text matches line 190; cited :34/:40/:41 are outside ranges, unverified.
