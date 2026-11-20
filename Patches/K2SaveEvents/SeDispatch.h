// SeDispatch.h -- the save-events registry: kind-masked consumer list, save-folder parsing, the "current save folder" and the
// slow-consumer guard. Pure C++ (no Win32, no engine reads, clock injected), so every rule is host-tested (se_registry_test.cpp).
// Used by SaveEventsProvider.cpp. The folder helpers are copies of ActionHotbar's HotbarLogic.h normalizeSaveFolder / joinSaveDir
// (the hotbar keeps its own until it migrates, doc 22 section 4.1a).
#pragma once
#include "../Common/SaveEvents.h"
#include <stdint.h>
#include <string.h>
#include <string>

namespace se {

constexpr int MaxConsumers = 16;
constexpr uint32_t SlowThresholdUs = 5000;   // a callback slower than this (strictly) is reported; dispatching continues

inline char lowerAscii(char c) { return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c; }

// "SAVES:000012 - Game11" / "/000012 - Game11" -> "000012 - Game11"; "" if it is not "NNNNNN - Name".
inline std::string normalizeSaveFolder(std::string f) {
    if (f.size() > 6) {
        static const char kAlias[] = "saves:";
        bool alias = true;
        for (int i = 0; i < 6; ++i) if (lowerAscii(f[i]) != kAlias[i]) { alias = false; break; }
        if (alias) f = f.substr(6);
    }
    while (!f.empty() && (f[0] == '\\' || f[0] == '/')) f.erase(0, 1);
    if (f.size() < 10 || f.compare(6, 3, " - ") != 0) return "";
    for (int i = 0; i < 6; ++i) if (f[i] < '0' || f[i] > '9') return "";
    return f;
}
// <save root> + folder: an empty root means "saves/"; a missing trailing slash is added.
inline std::string joinSaveDir(std::string base, const std::string& folder) {
    if (base.empty()) base = "saves/";
    if (base.back() != '/' && base.back() != '\\') base += '/';
    return base + folder;
}
// Raw engine folder ("SAVES:000012 - Game11") -> the root-qualified folder of K2SaveEvent.folder, "" if implausible.
// cloudFlag = the engine's Steam Cloud byte; cloudRoot = the CExoString "cloudsaves/<id>/" (may be empty/unreadable).
// *cloudOut (optional) = 1 only when the cloud root was really used: a set flag with an empty root falls back to saves/ and reports 0.
inline std::string absFolder(bool cloudFlag, const std::string& cloudRoot, const std::string& raw, int32_t* cloudOut = nullptr) {
    if (cloudOut) *cloudOut = 0;
    std::string f = normalizeSaveFolder(raw);
    if (f.empty()) return "";
    const bool useCloud = cloudFlag && !cloudRoot.empty();
    if (cloudOut) *cloudOut = useCloud ? 1 : 0;
    return joinSaveDir(useCloud ? cloudRoot : std::string("saves/"), f);
}

// PS2: one entry of the save root listing (folder name, last-write time in ms on any monotonic-enough wall clock).
struct FolderStamp { std::string name; uint64_t mtimeMs; };
constexpr uint64_t RecentSaveWindowMs = 10000;
// The save folder the engine just wrote, when the request's folder name was unreadable: the newest "NNNNNN - Name" folder written
// within windowMs of nowMs (a little clock skew into the future is allowed). "" when none qualifies or two share the newest time.
inline std::string pickRecentFolder(const FolderStamp* c, int n, uint64_t nowMs, uint64_t windowMs = RecentSaveWindowMs) {
    const FolderStamp* best = nullptr; bool tie = false;
    for (int i = 0; i < n; ++i) {
        const FolderStamp& f = c[i];
        if (normalizeSaveFolder(f.name).empty() || f.name != normalizeSaveFolder(f.name)) continue;
        if (f.mtimeMs + windowMs < nowMs || f.mtimeMs > nowMs + 2000) continue;
        if (!best || f.mtimeMs > best->mtimeMs) { best = &f; tie = false; }
        else if (f.mtimeMs == best->mtimeMs) tie = true;
    }
    return best && !tie ? best->name : "";
}

// snprintf-style copy of s: always NUL-terminates when cap > 0; returns s.size() (>= cap = truncated); 0 for the empty string.
inline int32_t copyText(const std::string& s, char* buf, uint32_t cap) {
    if (buf && cap) {
        size_t n = s.size() < size_t(cap) - 1 ? s.size() : size_t(cap) - 1;
        memcpy(buf, s.data(), n);
        buf[n] = 0;
    }
    return int32_t(s.size());
}

inline bool singleKind(int32_t kind) { return (kind & K2SE_ALL) != 0 && (kind & ~K2SE_ALL) == 0 && (kind & (kind - 1)) == 0; }

struct Entry { int32_t token; int32_t mask; K2SaveCallback fn; void* user; const char* owner; };

// Injectable timing: now() returns a free-running microsecond counter (wraps at 2^32, the subtraction handles it); slow() is called
// after a callback that took longer than thresholdUs. now == nullptr disables the guard.
struct Timing {
    uint32_t (*now)(void* ctx) = nullptr;
    void (*slow)(void* ctx, const Entry& e, uint32_t us, int32_t kind) = nullptr;
    void* ctx = nullptr;
    uint32_t thresholdUs = SlowThresholdUs;
};

struct Dispatcher {
    Entry e[MaxConsumers] = {};
    int n = 0;
    int32_t nextToken = 1;
    Timing timing;
    uint32_t slowCalls = 0;

    // Token > 0, or -1 (null fn / no valid kind bit), -2 (table full). Unknown mask bits are ignored (a newer consumer on an older provider).
    int32_t add(int32_t mask, K2SaveCallback fn, void* user, const char* owner) {
        mask &= K2SE_ALL;
        if (!fn || mask == 0) return -1;
        if (n >= MaxConsumers) return -2;
        e[n++] = { nextToken, mask, fn, user, owner ? owner : "" };
        return nextToken++;
    }
    bool remove(int32_t token) {
        for (int i = 0; i < n; ++i)
            if (e[i].token == token) {
                for (int k = i + 1; k < n; ++k) e[k - 1] = e[k];
                --n;
                return true;
            }
        return false;
    }
    bool live(int32_t token) const { for (int i = 0; i < n; ++i) if (e[i].token == token) return true; return false; }
    int count(int32_t kind) const { int c = 0; for (int i = 0; i < n; ++i) if (e[i].mask & kind) ++c; return c; }

    // Calls every consumer whose mask has ev.kind, in registration order; returns how many were called (0 for an invalid kind).
    // Iterates a snapshot: a consumer registered by a callback waits for the next event; one unregistered by a callback is skipped.
    int dispatch(const K2SaveEvent& ev) {
        if (!singleKind(ev.kind) || n == 0) return 0;
        Entry snap[MaxConsumers];
        const int cnt = n;
        memcpy(snap, e, sizeof(Entry) * cnt);
        int called = 0;
        for (int i = 0; i < cnt; ++i) {
            if (!(snap[i].mask & ev.kind) || !live(snap[i].token)) continue;
            const uint32_t t0 = timing.now ? timing.now(timing.ctx) : 0;
            snap[i].fn(&ev, snap[i].user);
            ++called;
            if (timing.now) {
                const uint32_t us = timing.now(timing.ctx) - t0;
                if (us > timing.thresholdUs) { ++slowCalls; if (timing.slow) timing.slow(timing.ctx, snap[i], us, ev.kind); }
            }
        }
        return called;
    }
};

// The provider core: dispatcher + CurrentSaveFolder. Events with a folder are built here so struct_size / folder / cloud are filled
// in one place.
struct Hub {
    Dispatcher disp;
    std::string current;   // root-qualified folder of the last SAVED / AUTOSAVED / LOAD_REQUEST, "" = none

    // folder = the root-qualified folder (K2SE_NEW_GAME ignores it). Returns the number of consumers called, or -1 for a bad kind or a
    // folder event without a folder (nothing dispatched, `current` unchanged).
    int emit(int32_t kind, const char* folder, int32_t cloud) {
        if (!singleKind(kind)) return -1;
        std::string f;   // our own copy: a callback that re-enters cannot invalidate ev.folder
        K2SaveEvent ev = {};
        ev.struct_size = sizeof(K2SaveEvent);
        ev.kind = kind;
        if (kind == K2SE_NEW_GAME) {
            current.clear();
            ev.folder = nullptr; ev.cloud = 0;
        } else {
            if (!folder || !*folder) return -1;
            f = folder;
            current = f;
            ev.folder = f.c_str(); ev.cloud = cloud ? 1 : 0;
        }
        return disp.dispatch(ev);
    }
    int32_t copyCurrent(char* buf, uint32_t cap) const { return copyText(current, buf, cap); }
};

} // namespace se
