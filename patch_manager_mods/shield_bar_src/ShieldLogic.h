// Shield Bar -- pure helpers (host-testable; only libc headers, no Win32 types). See shield_logic_test.cpp.
#pragma once
#include <stdio.h>
#include <stdarg.h>

namespace sb {

// Bounded printf-append: a copy of bh::appendf (buff_duration_hud_src/BuffHudLogic.h, audit 20a F7, abc01d8). The two patches build
// from separate folders and cannot include each other's headers; shield_logic_test.cpp checks the copies stay identical.
// Appends at buf[len], never writes past buf[cap-1], always NUL-terminated, returns the new length clamped to cap-1.
inline int appendf(char* buf, int cap, int len, const char* fmt, ...) {
    if (cap <= 0) return len;
    if (len < 0) len = 0;
    if (len >= cap - 1) { buf[cap - 1] = 0; return cap - 1; }
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf + len, (size_t)(cap - len), fmt, ap);
    va_end(ap);
    if (n < 0) { buf[len] = 0; return len; }   // encoding error: keep what we had, stay terminated
    return (n >= cap - len) ? cap - 1 : len + n;
}

}  // namespace sb
