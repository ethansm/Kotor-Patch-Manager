// check.h - zero-dependency test checks (doc 20 Phase 0).
//
// Modeled on KPM tests/check.h (namespace kptest). Never uses assert(), so
// -DNDEBUG cannot disable it; scripts/run_tests.py --selftest proves that.
// Builds under host g++, -m32 and i686-w64-mingw32-g++ (C++17: inline vars).
//
//   KT_CHECK(x.size() == 3);
//   KT_CHECK_EQ(row.forcepoints, 25);
//   KT_CHECK_EQ(icon, "ip_cure");        // const char* -> strcmp
//   return kt::Report("twoda_test");     // 0 = all passed, 1 = any failed
//
// FAIL lines always print; PASS lines only with KT_VERBOSE set in the env.
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>

namespace kt {

inline int g_checks = 0;
inline int g_failures = 0;

inline bool Verbose() {
    static int v = -1;
    if (v < 0) {
        const char* e = std::getenv("KT_VERBOSE");
        v = (e && *e && std::strcmp(e, "0") != 0) ? 1 : 0;
    }
    return v == 1;
}

inline bool Check(const char* what, bool ok, const char* detail = "") {
    ++g_checks;
    if (!ok) {
        ++g_failures;
        std::printf("FAIL %s%s%s\n", what, (detail && *detail) ? "  -- " : "", detail ? detail : "");
        std::fflush(stdout);
    } else if (Verbose()) {
        std::printf("PASS %s\n", what);
    }
    return ok;
}

// Equality with both values shown on failure.
inline bool CheckEq(const char* what, long long a, long long b) {
    char buf[96];
    std::snprintf(buf, sizeof buf, "got %lld (0x%llx), want %lld (0x%llx)", a,
                  (unsigned long long)a, b, (unsigned long long)b);
    return Check(what, a == b, buf);
}
inline bool CheckEq(const char* what, unsigned long long a, unsigned long long b) {
    char buf[96];
    std::snprintf(buf, sizeof buf, "got %llu (0x%llx), want %llu (0x%llx)", a, a, b, b);
    return Check(what, a == b, buf);
}
inline bool CheckEq(const char* what, const char* a, const char* b) {
    bool ok = (a && b) ? std::strcmp(a, b) == 0 : a == b;
    char buf[256];
    std::snprintf(buf, sizeof buf, "got \"%.100s\", want \"%.100s\"", a ? a : "(null)", b ? b : "(null)");
    return Check(what, ok, buf);
}
// Route every integer type (incl. enums, bool, char) through one overload
// pair, and any char pointer (const or not) through strcmp.
template <class A, class B>
inline bool CheckEq(const char* what, A a, B b) {
    using DA = std::decay_t<A>;
    using DB = std::decay_t<B>;
    if constexpr (std::is_pointer_v<DA> && std::is_pointer_v<DB>) {
        return CheckEq(what, static_cast<const char*>(a), static_cast<const char*>(b));
    } else if constexpr (std::is_signed_v<DA> || std::is_signed_v<DB>) {
        static_assert(std::is_integral_v<DA> || std::is_enum_v<DA>, "KT_CHECK_EQ: integers or C strings only");
        return CheckEq(what, (long long)a, (long long)b);
    } else {
        static_assert(std::is_integral_v<DA> || std::is_enum_v<DA>, "KT_CHECK_EQ: integers or C strings only");
        return CheckEq(what, (unsigned long long)a, (unsigned long long)b);
    }
}

inline int Report(const char* name) {
    std::printf("%s: %d checks, %d failed\n", name, g_checks, g_failures);
    std::fflush(stdout);
    return g_failures != 0 ? 1 : 0;
}

}  // namespace kt

#define KT_STR2(x) #x
#define KT_STR(x) KT_STR2(x)
#define KT_CHECK(expr) ::kt::Check(__FILE__ ":" KT_STR(__LINE__) ": " #expr, static_cast<bool>(expr))
#define KT_CHECK_EQ(a, b) ::kt::CheckEq(__FILE__ ":" KT_STR(__LINE__) ": " #a " == " #b, (a), (b))
