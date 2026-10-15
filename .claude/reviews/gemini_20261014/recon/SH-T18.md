# SH-T18
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3c-05 | PARTIAL | patch_manager_mods/_shared_src/filewatch_test.cpp:51-53,59; patch_manager_mods/_shared_src/FileWatch.h:81 | no | Out-param gap true for lines 29-100 only; coarse-FS skip is deliberate, not a defect |

- Anchor check: filewatch_test.cpp:78 FOUND (exact text match); it is a missing->present check, unrelated to the out-param or mtimeMs claims.
- filewatch_test.cpp:29-62 (test_stamp): calls stamp() only; no changed() call in this function.
- filewatch_test.cpp:70-100 (test_poll, read to line 100): every changed() call has 3 args, or force=true at :91; none passes the 5th arg Stamp* out.
- FileWatch.h:77 and :81: changed(..., Stamp* out = nullptr) writes *out only when non-null, so the out path is untested in lines 29-100.
- filewatch_test.cpp:48-50: unguarded check that n1.mtime equals stat st_mtim seconds*1e9 + nsec.
- filewatch_test.cpp:51-53: guard `mtime % 1e9 != 123456789` prints a note and skips the else block; line 52 is a printf only, nothing else records the skip.
- filewatch_test.cpp:59: mtimeMs()==1700000000123 sits inside the guarded else; a coarse FS would yield ...000, so the guard is needed here.
- filewatch_test.cpp:33 and :79: unguarded mtimeMs/seenMs checks using ns=0; they still run on a coarse FS.
- mtimeMs() definition is in FileWatch.h:1-69 (unread); inferred as mtime/1e6 truncated from line 59.
- Implied fix: passing a Stamp out in the test is sound; dropping the line-51 guard is unsafe, since line 59 fails on a coarse FS.
