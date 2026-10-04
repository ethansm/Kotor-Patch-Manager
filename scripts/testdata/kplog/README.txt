Fixtures for scripts/test_kplog_parse.py.
Real core output (kpatch.log v1, produced by the Phase 1 smoke/H-check runs on 2026-10-03):
  logger-fixture_log.txt   smoke fixture run (header with interfaces list, WARN + plain line)
  kplog-a_log.txt, kplog-b_log.txt   two per-patch files of one session (channels alpha/beta/debug)
  trunc_kplog-a_log.txt    232-byte text cap: lines 2 and 3 end with the core's "[...]" marker
Hand-written, mirroring the format strings in log_service.cpp (no real run produced MARK/DROPPED lines):
  hand_markdrop_log.txt    MARK, DROPPED, WARN/ERR, f=N, one malformed line
  hand_session2_c_log.txt  a later session (21:10) for session-grouping tests
  fallback_log.txt         old-core fallback sink shape from Patches/Common/KpLog.h
