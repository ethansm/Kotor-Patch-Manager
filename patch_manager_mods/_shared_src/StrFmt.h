// StrFmt.h -- bounded printf-append shared by the mods (finding F26: sb::appendf and bh::appendf were identical copies).
// strfmt::appendf appends at buf[len], never writes past buf[cap-1], always NUL-terminated, returns the new length clamped to cap-1.
// Body is byte-identical to bh::appendf (audit 20a F7, abc01d8); strfmt_test.cpp pins it against both original copies.
#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

namespace strfmt {

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

}  // namespace strfmt
