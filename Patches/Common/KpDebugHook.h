#pragma once
/*
 * KpDebugHook.h -- the one seam between Common.h's debugLog() and KpLog.h.
 *
 * Common.h routes every debugLog()/debugHexDump() line through kp_debug_emit().
 * With no hook installed that is exactly what it always was: OutputDebugStringA
 * on Windows (byte-identical text, the caller's own '\n' included), stderr
 * elsewhere. A patch that includes KpLog.h and succeeds in KpLog_Init() installs
 * a hook, and from then on the same text goes to the "debug" channel of the
 * kpatch.log service. Patches that never include KpLog.h never set the hook and
 * behave as before.
 *
 * Includable standalone, and before or after <windows.h>. The header must not
 * pull in windows.h itself: Common.h users include their own Windows headers
 * with their own macros, so OutputDebugStringA is declared here instead (an
 * identical redeclaration after windows.h is harmless, and the reverse order is
 * too).
 *
 * Visibility (non-Windows): the hook pointer is an inline variable. At default
 * visibility GCC emits inline variables as STB_GNU_UNIQUE, and the dynamic
 * loader merges those across RTLD_LOCAL shared objects, so two patch modules
 * would share one hook and one would write into the other's log. Hidden
 * visibility keeps one copy per module. (PE/DLLs have no such merging.)
 */

#include <atomic>
#include <cstddef>
#include <cstdio>

#if defined(_WIN32)
#ifndef KP_ODS_DECLARED
#define KP_ODS_DECLARED 1
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
#endif
#endif

#if !defined(_WIN32)
#pragma GCC visibility push(hidden)
#endif

// Text is NUL-terminated and len == strlen(text). May be called from any thread.
using KpDebugHookFn = void (*)(const char* text, size_t len);

inline std::atomic<KpDebugHookFn> kp_debug_hook{nullptr};

inline void kp_debug_emit(const char* text, size_t len) {
    KpDebugHookFn hook = kp_debug_hook.load(std::memory_order_acquire);
    if (hook) {
        hook(text, len);
        return;
    }
#if defined(_WIN32)
    (void)len;
    OutputDebugStringA(text);
#else
    (void)len;
    std::fputs(text, stderr);
#endif
}

#if !defined(_WIN32)
#pragma GCC visibility pop
#endif
