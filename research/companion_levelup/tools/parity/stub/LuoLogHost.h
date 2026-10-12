// LuoLogHost.h - host-test plumbing for the level-up override's shared KpLog.h logger.
// Include after LevelUpOverride.cpp. The real mod picks the fallback level from its config; host tests don't load one, and KpLog's default
// (WARN) would drop every INFO line the tests grep for. The fallback sink appends, so a reset must remove() the old file first;
// KpLog_SetFallbackFile then drops its cached FILE* (it never closes it) and reopens `path` lazily on the next write, so later lines land in the fresh file.
#pragma once
#include <cstdio>

static inline void luoHostLogInit() { KpLog_SetFallbackLevel(KPLOG_INFO); }
static inline void luoHostLogReset(const char* path) { std::remove(path); KpLog_SetFallbackFile(path); }
