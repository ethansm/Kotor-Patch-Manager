# SH-T16
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3c-02 | PARTIAL | patch_manager_mods/_shared_src/gui2d_test.cpp:47-48; patch_manager_mods/_shared_src/Gui2D.h:83,86,95,107 | no | Up-front overrides and unreached defaults hold in range; hook setup and rest of file unread. Removing overrides reads engine addrs on host; unsafe. |
- gui2d_test.cpp:47 sets flushArgAddr = base and clipDepthAddr = base+16, so Gui2D.h:86 and :107 take the override branch, not K2AddrTable.
- gui2d_test.cpp:48 sets g_g2d.clipNoFill = kNoFill (non-null) before any test; anchor is an exact match at line 48.
- Gui2D.h:83 is the only K2AddrTable fallback for clipNoFill; unreachable in range because g_g2d.clipNoFill is non-null.
- Gui2D.h:94-95: pushClip returns 0 only when clipNoFill() is null; in range that state never occurs.
- gui2d_test.cpp:66-67: explicit nullptr colour resolves through clipNoFill() to kNoFill, so it does not hit the null return.
- gui2d_test.cpp:51-52: checks AddrPushClipRect and AddrClipNoFillColor constant values only; does not call the k2addr table path.
- gui2d_test.cpp:68-69: g_pushResult passthrough (0 and 7) is tested; this covers push return, not the null-colour gap.
- gui2d_test.cpp:44,46: g_stubVirtualQueryAll = true and installFakes() called; installFakes body (hook assignments) sits above line 35, outside range.
- Gui2D.h:81: comment says nullptr when exe address unknown; k2addr::addr behaviour for unknown addresses is outside range.
- OPEN (not verified): do gui2d_test.cpp lines 71+ or installFakes exercise the null return or table defaults, and do they set all hooks?
