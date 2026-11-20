#pragma once
/*
 * KpLog.h -- header-only convenience layer over the "kpatch.log" v1 service
 * (see kpatch_api.h and docs/PatchRegistry.md).
 *
 * Minimal use, in exactly one .cpp of the patch:
 *
 *     #include "KpLog.h"
 *     KP_LOG_PATCH("my-patch");              // the manifest id; once per patch
 *
 *     // KPatchInit must be written out literally in a .cpp (not produced by a
 *     // macro): create-patch's export scan reads the source text and cannot see
 *     // through a macro. It must also be listed in the patch's exports.def.
 *     extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }
 *
 *     KP_CHANNEL(chDraw, "draw");            // namespace scope or function-local
 *     void OnDraw(int quads) {
 *         KP_LOG(chDraw, "quads=%d", quads);                // INFO
 *         KP_LOGV(chDraw, KPLOG_DEBUG, "detail %d", quads); // explicit level
 *         KP_LOG_EVERY(chDraw, 1000, "alive");              // at most 1/s per call site
 *         KP_WARN("something odd: %d", quads);              // built-in "main" channel
 *         if (KP_ENABLED(chDraw, KPLOG_TRACE)) { ... expensive dump ... }
 *     }
 *
 * What it does
 * ------------
 * With the core's service present, every call is "Enabled() then format then
 * Submit()": nothing is formatted for a disabled channel. Without the service
 * (older core, KpLog_Init never called or failed) the header falls back to a
 * small sink inside the patch itself -- see "Fallback rule" below -- so a patch
 * built against this header still works, just with less to show.
 *
 * Fallback rule
 * -------------
 * No api means the fallback sink on every call. There is no pre-init ring and
 * no timeout heuristic: until KpLog_Init() succeeds, a line is either written
 * to the fallback sink (level <= fallback level, default WARN) or dropped. On a
 * core with the registry KPatchInit precedes every hook, so only lines from
 * static constructors / DllMain fall back; on an old core everything does.
 * The sink is the file given to KpLog_SetFallbackFile() (opened lazily on the
 * first write), else OutputDebugStringA on Windows, else stderr. Lines look like
 *     <ms> <patch-id>/<channel> [WARN |ERR ]text
 * where <ms> is steady_clock milliseconds truncated to 32 bits.
 *
 * Tunables (define before including): KP_LOG_FALLBACK_FILE (path literal, default
 * none), KP_LOG_FALLBACK_LEVEL (default KPLOG_WARN), KP_LOG_FALLBACK_MAX_LINES
 * (default 60000; the sink then writes "--- log cap reached ---" once and drops
 * the rest), KP_DEBUGLOG_ODS (default 1: with the service up, debugLog() text is
 * ALSO sent to OutputDebugStringA on Windows so a debugger still sees it).
 *
 * Design notes (the why)
 * ----------------------
 * - Visibility: on non-Windows everything here is wrapped in
 *   `#pragma GCC visibility push(hidden)`. GCC would otherwise emit inline
 *   variables and function-local statics as STB_GNU_UNIQUE, which the dynamic
 *   loader merges across RTLD_LOCAL shared objects; two patches would then share
 *   one api pointer and one patch id. Hidden gives each module its own copy and
 *   keeps the header's symbols out of the module's exports. The extern
 *   declaration of kplog::kPatchId is inside the region on purpose so the
 *   definition from KP_LOG_PATCH inherits hidden visibility.
 * - No 64-bit atomics anywhere (millisecond stamps are uint32): on i386 a
 *   std::atomic<uint64_t> would need libatomic. Wraparound (49 days) is handled
 *   by unsigned subtraction.
 * - No <thread>/<mutex> (MinGW's win32 thread model has neither) and no
 *   windows.h. The one tiny lock is a spin flag held only around path copy and
 *   fopen.
 * - DllMain safety: KpLog_Init does no I/O and allocates nothing. Logging calls
 *   before it are the fallback path above.
 * - Optional macro arguments are done with default parameters on inline
 *   functions, not ##__VA_ARGS__/__VA_OPT__ or argument-count tricks, so the
 *   MSVC traditional preprocessor works.
 * - Channels are lazily resolved on first use (the service's Channel() is
 *   idempotent per (patch, name), so racing threads converge on one id). A
 *   rejected channel (bad name, 256-channel limit) is cached dead, reported once
 *   in the fallback sink, and every later call on it is a no-op.
 * - Never call KP_FRAME_TICK from a hook that is not frame-global: the first
 *   patch to tick owns the shared frame counter and a second ticker per frame
 *   would double it.
 *
 * Constraints: C++17; builds with -fno-exceptions -fno-rtti.
 */

#include <atomic>
#ifndef _WIN32
#include <chrono>
#endif
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#if defined(_WIN32)
// Declared rather than taken from windows.h (see KpDebugHook.h). Identical to
// windows.h's declaration (DWORD WINAPI GetTickCount(VOID)), so either order
// of includes compiles.
extern "C" __declspec(dllimport) unsigned long __stdcall GetTickCount(void);
#endif

#include "kpatch_api.h"
#include "KpDebugHook.h"

#ifndef KP_LOG_FALLBACK_LEVEL
#define KP_LOG_FALLBACK_LEVEL KPLOG_WARN
#endif
#ifndef KP_LOG_FALLBACK_MAX_LINES
#define KP_LOG_FALLBACK_MAX_LINES 60000u
#endif
#ifndef KP_DEBUGLOG_ODS
#define KP_DEBUGLOG_ODS 1
#endif

#if defined(__GNUC__)
#if defined(__MINGW32__) && defined(__MINGW_PRINTF_FORMAT)
#define KPLOG_PRINTF_ATTR(fmt, args) __attribute__((format(__MINGW_PRINTF_FORMAT, fmt, args)))
#else
#define KPLOG_PRINTF_ATTR(fmt, args) __attribute__((format(printf, fmt, args)))
#endif
#else
#define KPLOG_PRINTF_ATTR(fmt, args)
#endif

#if !defined(_WIN32)
#pragma GCC visibility push(hidden)
#endif

namespace kplog {

// Defined by KP_LOG_PATCH in exactly one .cpp of the patch (a missing one is a
// link error). Declared here, inside the visibility region, so it is hidden.
extern const char kPatchId[];

// One logging channel. Constant-initialized (no static-init guard, so it is
// usable from static constructors and function-local statics alike).
// id: -2 unresolved, -1 dead (rejected), >= 0 the service's handle.
struct Channel {
    const char* name;
    std::atomic<int32_t> id;
    constexpr explicit Channel(const char* n) : name(n), id(-2) {}
};

namespace detail {

// Same rule as the core's Register(): 1..63 characters of [A-Za-z0-9_.-].
constexpr bool ValidId(const char* s) {
    if (!s || !s[0]) return false;
    for (size_t n = 0; s[n]; ++n) {
        const char c = s[n];
        const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                        (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-';
        if (!ok || n >= 63) return false;
    }
    return true;
}

// ---- shared state (one copy per module, see the visibility note) -----------
inline std::atomic<const KPatchLogApi*> gApi{nullptr};
inline std::atomic<int32_t> gPatch{-1};
inline std::atomic<int> gFallbackLevel{KP_LOG_FALLBACK_LEVEL};
inline std::atomic<uint32_t> gFallbackLines{0};
inline Channel gMain{"main"};
inline Channel gDebug{"debug"};

// Fallback file state. gSpin guards gPath and the open; writers only read gFile.
inline std::atomic<int> gSpin{0};
inline std::atomic<std::FILE*> gFile{nullptr};
inline std::atomic<bool> gOpenFailed{false};
#ifdef KP_LOG_FALLBACK_FILE
inline char gPath[260] = KP_LOG_FALLBACK_FILE;
inline std::atomic<bool> gWantFile{true};
#else
inline char gPath[260] = {0};
inline std::atomic<bool> gWantFile{false};
#endif

inline void SpinLock() {
    while (gSpin.exchange(1, std::memory_order_acquire)) {
    }
}
inline void SpinUnlock() { gSpin.store(0, std::memory_order_release); }

// Millisecond clock, 32 bits. 0xFFFFFFFF is reserved as the "never" sentinel.
// Windows uses GetTickCount (also a 32-bit ms counter): std::chrono's
// steady_clock statically pulls libstdc++/winpthreads clock code into a MinGW
// patch (about 100 KB, plus imports like SetSystemTime that look alarming).
inline uint32_t NowMs() {
#ifdef _WIN32
    const uint32_t t = static_cast<uint32_t>(GetTickCount());
#else
    const uint32_t t = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
#endif
    return t == 0xFFFFFFFFu ? 0xFFFFFFFEu : t;
}

inline std::FILE* OpenFallbackFile() {
    SpinLock();
    std::FILE* f = gFile.load(std::memory_order_relaxed);
    if (!f && !gOpenFailed.load(std::memory_order_relaxed)) {
#if defined(_MSC_VER)
        // _fsopen with _SH_DENYNO (0x40), not fopen_s: fopen_s denies write
        // sharing, so a second patch or a tail -f could not open the same file.
        f = _fsopen(gPath, "a", 0x40);
#else
        f = std::fopen(gPath, "a");
#endif
        if (f) gFile.store(f, std::memory_order_release);
        else gOpenFailed.store(true, std::memory_order_relaxed);
    }
    SpinUnlock();
    return f;
}

// One complete line (ends with '\n', NUL-terminated at buf[len]) to the sink.
// One fwrite+fflush / one OutputDebugStringA / one fputs, so concurrent lines
// do not interleave (the CRT locks the stream).
inline void SinkWrite(const char* buf, size_t len) {
    if (gWantFile.load(std::memory_order_acquire) &&
        !gOpenFailed.load(std::memory_order_relaxed)) {
        std::FILE* f = gFile.load(std::memory_order_acquire);
        if (!f) f = OpenFallbackFile();
        if (f) {
            std::fwrite(buf, 1, len, f);
            std::fflush(f);
            return;
        }
    }
#if defined(_WIN32)
    OutputDebugStringA(buf);
#else
    std::fputs(buf, stderr);
#endif
}

// Format and write one fallback line: "<ms> <id>/<chan> [WARN |ERR ]text\n".
inline void FallbackLine(int lvl, const char* chan, const char* text, size_t len) {
    if (gFallbackLines.load(std::memory_order_relaxed) > KP_LOG_FALLBACK_MAX_LINES) return;
    const uint32_t n = gFallbackLines.fetch_add(1, std::memory_order_relaxed);
    if (n > KP_LOG_FALLBACK_MAX_LINES) return;
    if (n == KP_LOG_FALLBACK_MAX_LINES) {
        static const char kCap[] = "--- log cap reached ---\n";
        SinkWrite(kCap, sizeof(kCap) - 1);
        return;
    }
    const char* tag = lvl <= KPLOG_ERR ? "ERR " : (lvl == KPLOG_WARN ? "WARN " : "");
    char buf[768];
    int k = std::snprintf(buf, sizeof(buf), "%u %s/%s %s", static_cast<unsigned>(NowMs()),
                          kPatchId, chan, tag);
    if (k < 0) k = 0;
    size_t pos = static_cast<size_t>(k);
    if (pos > sizeof(buf) - 2) pos = sizeof(buf) - 2;
    size_t room = sizeof(buf) - 2 - pos;  // keep space for '\n' and NUL
    if (len > room) len = room;
    std::memcpy(buf + pos, text, len);
    pos += len;
    buf[pos++] = '\n';
    buf[pos] = '\0';
    SinkWrite(buf, pos);
}

// Calls f(piece, pieceLen) for each non-empty '\n'-separated piece (a trailing
// '\r' is dropped, empty pieces are skipped -- the core drops them anyway).
template <typename F>
inline void ForEachLine(const char* text, size_t len, F f) {
    size_t start = 0;
    while (start <= len) {
        size_t end = start;
        while (end < len && text[end] != '\n') ++end;
        size_t plen = end - start;
        if (plen > 0 && text[start + plen - 1] == '\r') --plen;
        if (plen > 0) f(text + start, plen);
        if (end >= len) break;
        start = end + 1;
    }
}

// Resolve (once) the service handle of a channel. Returns -1 for a dead one.
inline int32_t Resolve(Channel& ch, const KPatchLogApi* api) {
    const int32_t cur = ch.id.load(std::memory_order_acquire);
    if (cur != -2) return cur;
    int32_t r = api->Channel(gPatch.load(std::memory_order_relaxed), ch.name);
    if (r < 0) r = -1;
    int32_t expected = -2;
    if (ch.id.compare_exchange_strong(expected, r, std::memory_order_acq_rel)) {
        if (r == -1) {
            char msg[96];
            const int k = std::snprintf(msg, sizeof(msg), "channel '%.40s' rejected", ch.name);
            FallbackLine(KPLOG_WARN, ch.name, msg, k > 0 ? static_cast<size_t>(k) : 0);
        }
        return r;
    }
    return expected;  // another thread resolved it first; same id (Channel() is idempotent)
}

// True when `last` is unset or at least `ms` old, and this caller won the right
// to refresh it (so concurrent callers log once per interval).
inline bool EveryGate(std::atomic<uint32_t>& last, uint32_t ms) {
    const uint32_t now = NowMs();
    uint32_t prev = last.load(std::memory_order_relaxed);
    if (prev != 0xFFFFFFFFu && static_cast<uint32_t>(now - prev) < ms) return false;
    return last.compare_exchange_strong(prev, now, std::memory_order_relaxed);
}

// Installed in kp_debug_hook by KpLog_Init: debugLog() text goes to the "debug"
// channel at DEBUG, one Submit per '\n'-separated piece.
inline void DebugHookFn(const char* text, size_t len) {
    const KPatchLogApi* api = gApi.load(std::memory_order_acquire);
    if (api) {
        const int32_t id = Resolve(gDebug, api);
        if (id >= 0 && api->Enabled(id, KPLOG_DEBUG)) {
            ForEachLine(text, len, [&](const char* p, size_t n) {
                api->Submit(id, KPLOG_DEBUG, p, static_cast<uint32_t>(n));
            });
        }
    }
#if defined(_WIN32) && KP_DEBUGLOG_ODS
    OutputDebugStringA(text);
#endif
}

}  // namespace detail

// ---- logging primitives -----------------------------------------------------

// Would (ch, lvl) be recorded? Cheap; call before formatting.
inline bool On(Channel& ch, int lvl) {
    const KPatchLogApi* api = detail::gApi.load(std::memory_order_acquire);
    if (!api) return lvl <= detail::gFallbackLevel.load(std::memory_order_relaxed);
    const int32_t id = detail::Resolve(ch, api);
    if (id < 0) return false;
    return api->Enabled(id, lvl) != 0;
}

inline bool Enabled(Channel& ch, int lvl = KPLOG_INFO) { return On(ch, lvl); }

// Format into a stack buffer (a longer line ends with "[...]"), split on '\n'
// and send each piece to the service, or to the fallback sink without one.
KPLOG_PRINTF_ATTR(3, 4)
inline void Write(Channel& ch, int lvl, const char* fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    const int n = std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    size_t len;
    if (n < 0) {
        std::memcpy(buf, "(format error)", 15);
        len = 14;
    } else if (static_cast<size_t>(n) >= sizeof(buf)) {
        len = sizeof(buf) - 1;
        std::memcpy(buf + len - 5, "[...]", 5);
    } else {
        len = static_cast<size_t>(n);
    }
    buf[len] = '\0';

    const KPatchLogApi* api = detail::gApi.load(std::memory_order_acquire);
    if (api) {
        const int32_t id = detail::Resolve(ch, api);
        if (id < 0) return;
        detail::ForEachLine(buf, len, [&](const char* p, size_t m) {
            api->Submit(id, lvl, p, static_cast<uint32_t>(m));
        });
    } else {
        const char* name = ch.name;
        detail::ForEachLine(buf, len, [&](const char* p, size_t m) {
            detail::FallbackLine(lvl, name, p, m);
        });
    }
}

inline void Mark(const char* label) {
    const KPatchLogApi* api = detail::gApi.load(std::memory_order_acquire);
    if (api) api->Mark(label);
}

inline void FrameTick() {
    const KPatchLogApi* api = detail::gApi.load(std::memory_order_acquire);
    if (api) api->FrameTick(detail::gPatch.load(std::memory_order_relaxed));
}

}  // namespace kplog

// ---- setup API ----------------------------------------------------------------

// Call from the patch's own KPatchInit. Returns 1 when the "kpatch.log" v1
// service was found, is large enough and accepted the patch id; 0 means the
// patch stays on the fallback sink (null api, no service, short struct, bad id).
// No I/O, no allocation: safe under the loader lock. Calling it again just
// re-stores the same state; channel ids cached from an earlier, different api
// are not invalidated.
inline int KpLog_Init(const KPatchApi* api) {
    if (!api || !api->require) return 0;
    const KPatchLogApi* log =
        static_cast<const KPatchLogApi*>(api->require(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION));
    if (!log || log->struct_size < sizeof(KPatchLogApi)) return 0;
    if (!log->Register || !log->Channel || !log->Enabled || !log->Submit || !log->Mark ||
        !log->FrameTick)
        return 0;
    const int32_t patch = log->Register(::kplog::kPatchId);
    if (patch < 0) return 0;
    // Patch handle first, then the api with release: a reader that sees the api
    // is guaranteed to see the handle.
    ::kplog::detail::gPatch.store(patch, std::memory_order_relaxed);
    ::kplog::detail::gApi.store(log, std::memory_order_release);
    kp_debug_hook.store(&::kplog::detail::DebugHookFn, std::memory_order_release);
    return 1;
}

inline bool KpLog_HasCore() {
    return ::kplog::detail::gApi.load(std::memory_order_acquire) != nullptr;
}

inline void KpLog_SetFallbackLevel(int level) {
    ::kplog::detail::gFallbackLevel.store(level, std::memory_order_relaxed);
}

// Send fallback lines to `path` (appended, opened on the first write; if it
// cannot be opened the sink goes back to OutputDebugStringA / stderr). nullptr or
// "" turns the file off. A path of 260+ characters is ignored. A previously
// opened file is left open, not closed: another thread may be mid-write.
inline void KpLog_SetFallbackFile(const char* path) {
    namespace d = ::kplog::detail;
    const size_t n = path ? std::strlen(path) : 0;
    if (n >= sizeof(d::gPath)) return;
    d::SpinLock();
    if (n == 0) {
        d::gPath[0] = '\0';
        d::gWantFile.store(false, std::memory_order_release);
    } else {
        std::memcpy(d::gPath, path, n + 1);
        d::gWantFile.store(true, std::memory_order_release);
    }
    d::gFile.store(nullptr, std::memory_order_release);
    d::gOpenFailed.store(false, std::memory_order_relaxed);
    d::SpinUnlock();
}

#if !defined(_WIN32)
#pragma GCC visibility pop
#endif

// ---- macros -------------------------------------------------------------------

// Once per patch, in one .cpp, at global scope, with a trailing semicolon. The id
// is checked at compile time against the core's rule.
#define KP_LOG_PATCH(id)                                                                    \
    static_assert(::kplog::detail::ValidId(id),                                             \
                  "KP_LOG_PATCH: id must be 1-63 characters of [A-Za-z0-9_.-]");            \
    namespace kplog {                                                                       \
    extern const char kPatchId[] = id;                                                      \
    }                                                                                       \
    static_assert(true, "")

#define KP_CHANNEL(var, name) static ::kplog::Channel var{name}

#define KP_ENABLED(...) (::kplog::Enabled(__VA_ARGS__))

#define KP_LOGV(ch, lvl, ...)                                                               \
    do {                                                                                    \
        if (::kplog::On((ch), (lvl))) ::kplog::Write((ch), (lvl), __VA_ARGS__);             \
    } while (0)

#define KP_LOG(ch, ...) KP_LOGV(ch, KPLOG_INFO, __VA_ARGS__)
#define KP_WARN(...) KP_LOGV(::kplog::detail::gMain, KPLOG_WARN, __VA_ARGS__)
#define KP_ERR(...) KP_LOGV(::kplog::detail::gMain, KPLOG_ERR, __VA_ARGS__)

// INFO, at most once per `ms` milliseconds per call site; the first call logs.
#define KP_LOG_EVERY(ch, ms, ...)                                                           \
    do {                                                                                    \
        static std::atomic<uint32_t> kp_every_last_{0xFFFFFFFFu};                           \
        if (::kplog::On((ch), KPLOG_INFO) &&                                                \
            ::kplog::detail::EveryGate(kp_every_last_, static_cast<uint32_t>(ms)))          \
            ::kplog::Write((ch), KPLOG_INFO, __VA_ARGS__);                                  \
    } while (0)

#define KP_MARK(label) ::kplog::Mark(label)
// Only from a frame-global hook (the first caller owns the counter).
#define KP_FRAME_TICK() ::kplog::FrameTick()
