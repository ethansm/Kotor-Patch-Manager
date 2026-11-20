// CreditsFormat.h -- compact credits text for the ISF compare box / credits label.
// Variant A: integer arithmetic only (no snprintf, no floats).
//
// Rules (u11.md 4.3):
//   v <  10000            plain integer           "0", "3239", "9999"
//   10000 .. 99999        one decimal, floor, 'k'  "10k", "10.2k", "99.9k"
//   100000 .. 999999      integer floor, 'k'       "100k", "101k", "999k"
//   1000000 .. 9999999    one decimal, floor, 'M'  "1M", "1.2M", "9.9M"
//   10000000 .. 999999999 integer floor, 'M'       "10M", "999M"
//   >= 1000000000         one decimal, floor, 'B'  "1B", "2.1B"
//   negative              plain with minus sign    "-5", "-2147483648"
// A trailing ".0" is dropped. Floor, never round.
#pragma once

#include <cstddef>
#include <cstdint>

namespace credfmt {

namespace detail {

// Copies n chars from s into out, truncating to cap-1, always NUL-terminates.
// Returns the number of chars written (excluding NUL). cap must be >= 1.
inline size_t emit(const char* s, size_t n, char* out, size_t cap) {
    const size_t maxc = cap - 1;
    const size_t len = (n < maxc) ? n : maxc;
    for (size_t i = 0; i < len; ++i) {
        out[i] = s[i];
    }
    out[len] = '\0';
    return len;
}

// Appends decimal digits of v to b at n. Buffer must have room (<= 10 digits).
inline void putU(char* b, size_t& n, uint32_t v) {
    char tmp[12];
    size_t k = 0;
    do {
        tmp[k++] = static_cast<char>('0' + (v % 10u));
        v /= 10u;
    } while (v != 0u);
    while (k != 0u) {
        b[n++] = tmp[--k];
    }
}

// Appends tenths/10 with one decimal digit (dropped when it is 0) and a suffix.
// tenths = value * 10 already floored by the caller.
inline void putTenths(char* b, size_t& n, uint32_t tenths, char suffix) {
    putU(b, n, tenths / 10u);
    const uint32_t frac = tenths % 10u;
    if (frac != 0u) {
        b[n++] = '.';
        b[n++] = static_cast<char>('0' + frac);
    }
    b[n++] = suffix;
}

}  // namespace detail

// Writes the compact form of `credits` into out (NUL-terminated).
// Returns the number of chars written, excluding the NUL.
// out == nullptr or cap == 0: writes nothing, returns 0.
// cap == 1: writes "" (just NUL), returns 0.
// Output longer than cap-1 is truncated (never overflows).
inline size_t format(int32_t credits, char* out, size_t cap) {
    if (out == nullptr || cap == 0) {
        return 0;
    }
    char b[16];
    size_t n = 0;
    if (credits < 0) {
        // Unsigned wrap avoids UB for INT32_MIN.
        const uint32_t mag = 0u - static_cast<uint32_t>(credits);
        b[n++] = '-';
        detail::putU(b, n, mag);
    } else {
        const uint32_t v = static_cast<uint32_t>(credits);
        if (v < 10000u) {
            detail::putU(b, n, v);
        } else if (v < 100000u) {
            detail::putTenths(b, n, v / 100u, 'k');
        } else if (v < 1000000u) {
            detail::putU(b, n, v / 1000u);
            b[n++] = 'k';
        } else if (v < 10000000u) {
            detail::putTenths(b, n, v / 100000u, 'M');
        } else if (v < 1000000000u) {
            detail::putU(b, n, v / 1000000u);
            b[n++] = 'M';
        } else {
            detail::putTenths(b, n, v / 100000000u, 'B');
        }
    }
    return detail::emit(b, n, out, cap);
}

}  // namespace credfmt
