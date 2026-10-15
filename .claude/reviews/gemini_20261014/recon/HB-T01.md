# HB-T01
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1a-01 | PARTIAL | patch_manager_mods/action_hotbar_src/TwoDA.h:87-88 | no | Single scan already ends at d.size(); real issue is aggregate copy cost. Rejecting NUL-less cells breaks test :177. |
| HB-1a-02 | CONFIRMED | patch_manager_mods/action_hotbar_src/TwoDA.h:79-80,86 | yes | 32-bit size_t wrap of nrows*cols defeats :80 check; :86 index into rd16 is unchecked. 64-bit safe. |
| HB-1d-01 | PARTIAL | patch_manager_mods/action_hotbar_src/twoda_test.cpp:177-186 | yes | EOF-terminated last cell is tested. Mid-blob NUL-less case is absent in read ranges; expected value must be decided. |
| HB-1d-02 | UNSURE | patch_manager_mods/action_hotbar_src/twoda_test.cpp:102-107 | no | Only row-count tests in range. Anchor :24 is outside range. 64-bit host cannot wrap size_t; needs guard at TwoDA.h:79. |

- TwoDA.h:73 caps nrows at 100000; no cap on cols.size() anywhere in ParseImpl (header loop :65-69 is unbounded).
- TwoDA.h:79-80: cells = nrows * cols.size() in size_t; the :80 check uses the wrapped value.
- TwoDA.h:86: offset index (r*cols.size()+c)*2 computed in size_t with no check; wrapped result reaches rd16.
- TwoDA.h:96: rd16 reads d[p] and d[p+1] with no bounds check; the only guard is the caller's table check at :80.
- TwoDA.h:87-88: cell copy guarded by o >= d.size(); std::string(c_str()+o) stops at the NUL terminator at d.size() (C++11).
- TwoDA.h:85-90: no cap on total copied bytes; a NUL-less tail shared by many cells is copied once per cell.
- twoda_test.cpp:97 covers offsets past EOF (empty result), not NUL-less cells.
- twoda_test.cpp:102-107: row-count limit tests only (100001, 0xFFFFFFFF, 100000, 65536, 0x01000000, 0x80000000, 3); no column-count case in range.
- twoda_test.cpp:177-186: last cell without NUL at EOF is required to load (64 KiB + 1 file); any reject-no-NUL fix breaks this test.
- OPEN: twoda_test.cpp:24 (outside read range, cited by Gemini) may already exercise high column counts; if so HB-1d-02 becomes REFUTED.
