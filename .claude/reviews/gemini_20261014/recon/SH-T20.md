# SH-T20
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3c-07 | PARTIAL | patch_manager_mods/_shared_src/item_read_test.cpp:51,63,65; patch_manager_mods/_shared_src/ItemRead.h:33,36,40-44 | yes | Anchor is in ItemRead.h:36, not the test. Only count 100000 tested; unreadable-entry, count -1/257, 16-byte icon untested in range. |
- ItemRead.h:33 `if (count < 0 || count > MaxEntries ...) return 0;` guards negative and over-max counts; MaxEntries is defined outside range.
- item_read_test.cpp:51 sets group 2 count 100000; :63 asserts readGroup returns 0, so the upper-bound branch is covered.
- item_read_test.cpp:52,64 group 3 count 0 returns 0, so the empty-list branch is covered.
- item_read_test.cpp:65 passes g=7 and g=-1; these are group indices, not entry counts, so count -1 is untested.
- ItemRead.h:36 probeRead per entry then `break`; no case in item_read_test.cpp:40-80 makes this probe fail.
- item_read_test.cpp:50,53,55 build two readable entries at 0x5000; nothing unreadable is set up in range.
- ItemRead.h:40-44 `ib[17]` is zeroed and copies 16 bytes, so ib[16] stays NUL; a full 16-byte icon is terminated and safe.
- item_read_test.cpp:54 memcpy copies 9 bytes of "i_medpac\x01", not the NUL; byte +9 relies on zeroed fake memory.
- item_read_test.cpp:60 expects "i_medpac?", which tests 0x01 sanitizing only, not a 16-byte icon.
- Anchor `if (!saferead::probeRead(SiteBase, ...)) break;` matches ItemRead.h:36 within my ranges; NOT-FOUND in item_read_test.cpp:40-80.
