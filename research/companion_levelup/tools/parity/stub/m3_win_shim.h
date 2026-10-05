// Host shims for the Win32 calls the M3 save-binding code uses that stub/windows.h lacks (force-included by parity.py build() after the stub).
#pragma once
#define GUIKIT_HOST 1   // GuiKit.h drops its Win32 platform defaults on the host (tests install fakes)
#define EMBTEX_HOST 1   // EmbedTex.h: no engine defaults (null table = ensure() fails, the icon falls back); ui_test installs fakes
#include "windows.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
union LARGE_INTEGER { long long QuadPart; };
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* q) { timespec t; clock_gettime(CLOCK_MONOTONIC, &t); q->QuadPart = (long long)t.tv_sec * 1000000000LL + t.tv_nsec; return 1; }
// GetModuleFileNameA: tests set g_stubModulePath to a file whose crc32 the banner must log (null = unreadable -> crc32=00000000).
inline const char* g_stubModulePath = nullptr;
inline DWORD GetModuleFileNameA(HINSTANCE, char* out, DWORD cap) { if (!g_stubModulePath) return 0; snprintf(out, cap, "%s", g_stubModulePath); return (DWORD)strlen(out); }
#define MOVEFILE_REPLACE_EXISTING 1
#define MOVEFILE_WRITE_THROUGH 8
inline BOOL MoveFileExA(const char* a, const char* b, DWORD) { return rename(a, b) == 0; }
