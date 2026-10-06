// TestHarness.h - zero-dependency test checks (doc 20 Phase 0).
//
// Canonical here (tests/include); mirrored byte-identical to KPM
// Patches/Common/test/TestHarness.h by scripts/check_shared_headers.py --sync
// (doc 20 Phase 4 / doc 24 T9b).
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
#include <string>
#include <type_traits>
#ifndef _WIN32
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

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

#ifndef _WIN32
// Per-test scratch directory (POSIX host only): $TMPDIR (else /tmp)/kt_<pid>_<counter>, created 0700 on construction and removed
// with everything inside it on destruction (also when a check failed or the test returned early). Not copyable.
//   kt::TempDir td; std::string p = td.file("x.ini");   // nothing is created for file(); the test writes it, the dtor cleans up
class TempDir {
public:
    TempDir() {
        static int counter = 0;
        const char* t = std::getenv("TMPDIR");
        path_ = std::string(t && *t ? t : "/tmp") + "/kt_" + std::to_string((long)::getpid()) + "_" + std::to_string(counter++);
        ok_ = ::mkdir(path_.c_str(), 0700) == 0;
    }
    ~TempDir() { if (ok_) RemoveTree(path_); }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;
    bool ok() const { return ok_; }
    const std::string& path() const { return path_; }
    std::string file(const std::string& name) const { return path_ + "/" + name; }

private:
    static void RemoveTree(const std::string& p) {
        struct stat st;
        if (::lstat(p.c_str(), &st) != 0) return;
        if (!S_ISDIR(st.st_mode)) { ::unlink(p.c_str()); return; }
        ::chmod(p.c_str(), 0700);   // a test may have made it read-only
        if (DIR* d = ::opendir(p.c_str())) {
            while (dirent* e = ::readdir(d)) {
                if (!std::strcmp(e->d_name, ".") || !std::strcmp(e->d_name, "..")) continue;
                RemoveTree(p + "/" + e->d_name);
            }
            ::closedir(d);
        }
        ::rmdir(p.c_str());
    }
    std::string path_;
    bool ok_ = false;
};
#endif  // !_WIN32

}  // namespace kt

#define KT_STR2(x) #x
#define KT_STR(x) KT_STR2(x)
#define KT_CHECK(expr) ::kt::Check(__FILE__ ":" KT_STR(__LINE__) ": " #expr, static_cast<bool>(expr))
#define KT_CHECK_EQ(a, b) ::kt::CheckEq(__FILE__ ":" KT_STR(__LINE__) ": " #a " == " #b, (a), (b))

#if defined(KT_MSVC_CRT_SHIM) && !defined(_WIN32)
// Opt-in (-DKT_MSVC_CRT_SHIM): lets KPM Common.h / Platform.h compile for host
// -m32 tests. MinGW and MSVC already provide these, so it is off by default.
#include <cstdarg>
#include <cstddef>
#include <cstdio>

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif
#if defined(__i386__) && !defined(__fastcall)
#define __fastcall __attribute__((fastcall))
#endif
inline int vsnprintf_s(char* b, size_t n, size_t, const char* f, va_list a) { return vsnprintf(b, n, f, a); }
template <size_t N, typename... A> int sprintf_s(char (&b)[N], const char* f, A... a) { return snprintf(b, N, f, a...); }
inline int sprintf_s(char* b, size_t n, const char* f, ...) { va_list a; va_start(a, f); int r = vsnprintf(b, n, f, a); va_end(a); return r; }
#endif  // KT_MSVC_CRT_SHIM && !_WIN32
