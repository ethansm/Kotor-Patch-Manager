// FileWatch.h -- config-file change detection and atomic save for KOTOR2 patch DLLs (header-only, no hooks, no engine access).
// Survey: project design doc 19b section D item #4 (INI reload / save code was copied per mod).
//
// What is shared: WHEN a file changed (stamp + throttled poll) and HOW to replace it safely (atomicWrite). Parsing stays per mod.
//   - stamp():  {exists, size, mtime}. mtime is FILETIME (100 ns) on Win32 and st_mtim (ns) on POSIX, so two edits inside one second
//               are told apart (a 1 s `stat` mtime misses them; the size usually changes too, and that is compared as well).
//   - ReloadPoll + changed(): at most one stamp() per throttle window, true when the stamp differs from the last one seen.
//   - atomicWrite(): <path>.tmp, flush, then replace; the old file survives any failure.
// Win32 DLL: uses GetFileAttributesExA / MoveFileExA. Host tests: the POSIX branch (stat / rename). No other headers needed.
#pragma once
#include <string>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace filewatch {

struct Stamp {
    bool exists = false;
    unsigned long long size = 0;
    long long mtime = 0;    // Win32: FILETIME ticks (100 ns since 1601). POSIX: ns since 1970. 0 when missing. Opaque: compare, or use mtimeMs().

    // Milliseconds since the Unix epoch; -1 when missing (the hotbar's old "mtime" log value, now with sub-second resolution).
    long long mtimeMs() const {
        if (!exists) return -1;
#ifdef _WIN32
        return (mtime - 116444736000000000LL) / 10000;
#else
        return mtime / 1000000;
#endif
    }
    bool operator==(const Stamp& o) const { return exists == o.exists && size == o.size && mtime == o.mtime; }
    bool operator!=(const Stamp& o) const { return !(*this == o); }
};

inline Stamp stamp(const char* path) {
    Stamp s;
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &d)) return s;
    s.exists = true;
    s.size = (static_cast<unsigned long long>(d.nFileSizeHigh) << 32) | d.nFileSizeLow;
    s.mtime = static_cast<long long>((static_cast<unsigned long long>(d.ftLastWriteTime.dwHighDateTime) << 32) | d.ftLastWriteTime.dwLowDateTime);
#else
    struct stat st;
    if (stat(path, &st) != 0) return s;
    s.exists = true;
    s.size = static_cast<unsigned long long>(st.st_size);
    s.mtime = static_cast<long long>(st.st_mtim.tv_sec) * 1000000000LL + st.st_mtim.tv_nsec;
#endif
    return s;
}

// Polling state, one per watched file. Semantics (the hotbar's): the FIRST changed() call after construction reports a change
// (nothing has been seen yet, so the caller does its initial load -- also for a missing file, where the caller writes a template or
// logs "cannot read"). A mod that loads at init calls accept() instead, so its first poll is quiet.
struct ReloadPoll {
    unsigned throttleMs = 1000;     // minimum ms between stamp() calls
    Stamp last;                     // last stamp seen (valid when primed)
    bool primed = false;            // a stamp has been seen / accepted
    bool checked = false;           // a check has run (throttle window is open)
    unsigned lastCheck = 0;         // nowMs of the last check

    // -2 before the first stamp, -1 when the file was missing, else Stamp::mtimeMs() (heartbeat/log value).
    long long seenMs() const { return primed ? last.mtimeMs() : -2; }
};

// True when the file differs from the last stamp seen (missing->present and present->missing both count). The new stamp is
// recorded when true is returned (the caller is expected to reload now), and *out (optional) receives it. force skips the throttle
// (use before overwriting the file, so a hand edit made in the last window is picked up first). nowMs is GetTickCount() or injected;
// unsigned subtraction makes tick wrap-around safe.
inline bool changed(ReloadPoll& p, const char* path, unsigned nowMs, bool force = false, Stamp* out = nullptr) {
    if (p.checked && !force && nowMs - p.lastCheck < p.throttleMs) return false;
    p.checked = true; p.lastCheck = nowMs;
    Stamp s = stamp(path);
    if (out) *out = s;
    if (p.primed && s == p.last) return false;
    p.last = s; p.primed = true;
    return true;
}

// Records `s` as seen without reporting a change: after the mod's own Save (so it does not reload its own write), or after an
// initial load.
inline Stamp accept(ReloadPoll& p, const Stamp& s) { p.last = s; p.primed = true; return s; }
inline Stamp accept(ReloadPoll& p, const char* path) { return accept(p, stamp(path)); }

namespace detail {
// Copies src to dst (binary); on any error removes the partial dst and returns false.
inline bool copyFile(const char* src, const char* dst) {
    FILE* in = fopen(src, "rb");
    if (!in) return false;
    FILE* out = fopen(dst, "wb");
    if (!out) { fclose(in); return false; }
    char buf[4096]; size_t n; bool ok = true;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) if (fwrite(buf, 1, n, out) != n) { ok = false; break; }
    if (ferror(in)) ok = false;
    fclose(in);
    ok = (fclose(out) == 0) && ok;
    if (!ok) remove(dst);
    return ok;
}
} // namespace detail

// Replaces `path` with `text` atomically: writes <path>.tmp, flushes it to disk, then renames it over `path` (Win32
// MoveFileExA REPLACE_EXISTING, POSIX rename). With makeBakOnce, the old file is first copied to <path>.bak when that does not
// exist yet (so the .bak keeps the file as it was before the FIRST managed save). Any failure -- including a failed .bak copy --
// removes the .tmp, leaves `path` exactly as it was, and returns false.
inline bool atomicWrite(const char* path, const std::string& text, bool makeBakOnce) {
    std::string tmp = std::string(path) + ".tmp", bak = std::string(path) + ".bak";
    FILE* f = fopen(tmp.c_str(), "wb");
    if (!f) return false;
    bool ok = fwrite(text.data(), 1, text.size(), f) == text.size();
    ok = (fflush(f) == 0) && ok;
#ifdef _WIN32
    ok = (_commit(_fileno(f)) == 0) && ok;     // FlushFileBuffers
#else
    ok = (fsync(fileno(f)) == 0) && ok;
#endif
    ok = (fclose(f) == 0) && ok;
    if (ok && makeBakOnce && stamp(bak.c_str()).exists == false && stamp(path).exists) ok = detail::copyFile(path, bak.c_str());
    if (ok) {
#ifdef _WIN32
        ok = MoveFileExA(tmp.c_str(), path, MOVEFILE_REPLACE_EXISTING) != 0;
#else
        ok = rename(tmp.c_str(), path) == 0;
#endif
    }
    if (!ok) remove(tmp.c_str());
    return ok;
}

} // namespace filewatch
