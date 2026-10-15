# SH-T19
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3c-06 | PARTIAL | patch_manager_mods/_shared_src/guitip_test.cpp:53,66,101-105 | yes | Missing: null out-params for measure, non-null unreadable mgr, partial field reads; null mgr and unreadable cell covered. |

- guitip_test.cpp:53 covers readMgr with cell holding null manager; GuiTip.h:61 `|| !mgr` returns before any read.
- guitip_test.cpp:66 sets guiSoundMgrAddr=0 so readMgr fails: unreadable-cell case is covered.
- No test writes a non-zero unreadable pointer into the cell (guitip_test.cpp:54 always stores valid mgr); setNative uses 0x10 at line 43 but readMgr never does.
- Partial readability: readMgr reads six fields in one chain (GuiTip.h:62-63); no test makes a single field read fail. Only the all-readable path (guitip_test.cpp:56) and null paths run.
- measure() guards `if (lines)` and `if (maxLen)` (GuiTip.h:99-100); every test call (guitip_test.cpp:101-105) passes both &l and &mx, so null out-params are untested.
- readMgr(nullptr) is tested at guitip_test.cpp:58 (GuiTip.h:60 guard), so the null out-param gap is specific to measure().
- guitip_test.cpp:104 measure(nullptr,...) tests null text, not null out-params.
- Anchor mismatch: the quoted rd32/rd<float> chain is at GuiTip.h:62, not in guitip_test.cpp; guitip_test.cpp:28 only asserts Mgr_* constants.
- Fix note: adding measure null-out-param calls and a 0x10 manager-pointer case is safe; a partial-read case depends on the fake-engine stub (guitip_test.cpp:24).
