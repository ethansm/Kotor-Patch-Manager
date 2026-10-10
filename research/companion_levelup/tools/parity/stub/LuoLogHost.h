// LuoLogHost.h - host-test plumbing for the level-up override's shared KpLog.h logger.
// Include after LevelUpOverride.cpp. The real mod picks the fallback level from its config; host tests don't load one, and KpLog's default
// (WARN) would drop every INFO line the tests grep for. KpLog also keeps the fallback file open, so remove()ing it leaves later writes on an
// unlinked inode: reset through the public API instead.
#pragma once
#include <cstdio>

static inline void luoHostLogInit() { KpLog_SetFallbackLevel(KPLOG_INFO); }
static inline void luoHostLogReset(const char* path) { std::remove(path); KpLog_SetFallbackFile(path); }
