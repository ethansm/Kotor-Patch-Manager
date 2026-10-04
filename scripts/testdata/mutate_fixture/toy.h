// Toy header for scripts/test_mutate_header.py and test_run_tests_coverage.py.
// Comment text must never be mutated: a < b, 42 + 1, true, 0x10.
#pragma once
#include <stdint.h>
static_assert(sizeof(int) >= 2, "static_assert is never mutated");

inline bool isAdult(int age) { return age >= 18; }

inline int scale(int n) {
    if (n < 0) return -1;
    return n * 2;
}

template <bool B> struct Need;
template <> struct Need<true> { static int v() { return 1; } };
inline int needTrue() { return Need<true>::v(); }

inline const char* label() { return "a < b, 99, true"; }

// Deliberately untested by toy_check.cpp.
inline int untested(int v) { return v == 7 ? 1 : 0; }
