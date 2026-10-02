// Host shims for the Win32 calls the M3 save-binding code uses that stub/windows.h lacks (force-included by parity.py build() after the stub).
#pragma once
#define GUIKIT_HOST 1   // GuiKit.h drops its Win32 platform defaults on the host (tests install fakes)
#include "windows.h"
#include <stdio.h>
#include <time.h>
union LARGE_INTEGER { long long QuadPart; };
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* q) { timespec t; clock_gettime(CLOCK_MONOTONIC, &t); q->QuadPart = (long long)t.tv_sec * 1000000000LL + t.tv_nsec; return 1; }
#define MOVEFILE_REPLACE_EXISTING 1
inline BOOL MoveFileExA(const char* a, const char* b, DWORD) { return rename(a, b) == 0; }
