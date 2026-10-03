// Buff Duration HUD -- pure helpers (host-testable; only libc headers, no Win32 types).
#pragma once
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

namespace bh {

// Bounded printf-append. Replaces the `len += snprintf(buf + len, cap - len, ...)` idiom, which overruns once a write
// truncates: snprintf returns the length it WOULD have written, so len passes cap and `cap - len` underflows to a huge size_t.
// Appends at buf[len], never writes past buf[cap-1], always leaves buf NUL-terminated, and returns the new length,
// which is clamped to cap-1 (so repeated calls on a full buffer are no-ops). cap <= 0: nothing is touched, len is returned.
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

}  // namespace bh
