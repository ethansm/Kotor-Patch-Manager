#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "VfsRegistry.h"
#include "TwoDaSplicer.h"
#include "CharacterProvider.h"

namespace assetvfs {

// H5 decision (CExoResMan::Release, see the hooks toml H5): the engine's own Release looks up the resource's archive
// source (FUN_00712d70) when packedId >> 30 == 1 and dereferences the result. A VFS resource
// (packedId 0x40FC....) has no archive source, so that lookup returns NULL and the engine faults (NULL + 0x30 read).
// True = skip that source-release block (the VFS owns the data; FreeResourceData already skips >>30 == 1).
inline bool skipEngineSourceRelease(uint32_t packedId) {
    return vfsreg::isVfsPackedId(packedId);
}

// CRes field offsets (Odyssey Engine 32-bit MSVC)
constexpr size_t kResOffVtbl       = 0x00;
constexpr size_t kResOffDemands    = 0x04;
constexpr size_t kResOffRequests   = 0x06;
constexpr size_t kResOffPackedId   = 0x08;
constexpr size_t kResOffStatus     = 0x0C;
constexpr size_t kResOffData       = 0x10;
constexpr size_t kResOffKeyEntry   = 0x14;
constexpr size_t kResOffSize       = 0x18;
constexpr size_t kRes2DaOffLoaded  = 0x28;

// Writable prefix bytes before m_pResource (CResMDL writes at -6, CRes2DA writes at -8)
constexpr size_t kWorkPrefixBytes  = 16;

struct TwoDaPatchTable {
    // Total across all registrants: each character bundle costs ~100 cells (appearance + heads rows); 256 held only two
    // (VFS-CT2 10-18: the third bundle got 47/102 and lost its heads row). Stack arrays below scale with this (~30 KB).
    static constexpr size_t kMax2DaCells = 1024;

    struct StoredCell {
        char table_name[16];
        uint32_t row_index;
        int32_t template_row;
        char column_name[32];
        char value[64];
        bool active;
    };

    StoredCell cells[kMax2DaCells] = {};
    uint32_t count = 0;

    void clear() {
        count = 0;
        std::memset(cells, 0, sizeof(cells));
    }

    // `dropped` (optional) receives the number of cells refused because the table was full.
    int32_t addCells(const K2Vfs2DaCell* newCells, uint32_t numCells, uint32_t* dropped = nullptr) {
        if (dropped) *dropped = 0;
        if (!newCells || numCells == 0) return 0;
        uint32_t added = 0;
        for (uint32_t i = 0; i < numCells; ++i) {
            const K2Vfs2DaCell& src = newCells[i];
            if (!src.table_name || !src.column_name) continue;

            char normTable[16] = {0};
            if (!vfsreg::normalizeResRef(src.table_name, normTable)) {
                size_t tlen = std::strlen(src.table_name);
                if (tlen > 15) tlen = 15;
                for (size_t k = 0; k < tlen; ++k) {
                    char c = src.table_name[k];
                    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
                    normTable[k] = c;
                }
            }

            bool updated = false;
            for (uint32_t j = 0; j < count; ++j) {
                if (cells[j].active && cells[j].row_index == src.row_index) {
                    char existingNorm[16] = {0};
                    vfsreg::normalizeResRef(cells[j].table_name, existingNorm);
                    if (std::memcmp(normTable, existingNorm, 16) == 0 &&
                        k2char::strEqualNoCase(cells[j].column_name, src.column_name)) {
                        cells[j].template_row = src.template_row;
                        std::snprintf(cells[j].value, sizeof(cells[j].value), "%s", src.value ? src.value : "");
                        updated = true;
                        added++;
                        break;
                    }
                }
            }
            if (updated) continue;

            if (count >= kMax2DaCells) {
                if (dropped) ++*dropped;
                continue;
            }

            StoredCell& dst = cells[count++];
            dst.active = true;
            dst.row_index = src.row_index;
            dst.template_row = src.template_row;
            std::snprintf(dst.table_name, sizeof(dst.table_name), "%s", src.table_name);
            std::snprintf(dst.column_name, sizeof(dst.column_name), "%s", src.column_name);
            std::snprintf(dst.value, sizeof(dst.value), "%s", src.value ? src.value : "");
            added++;
        }
        return static_cast<int32_t>(added);
    }

    // Number of distinct row indices with an active cell for this table (0 = nothing registered).
    uint32_t countRows(const char* tableResRef16) const {
        if (!tableResRef16) return 0;
        char targetNorm[16] = {0};
        if (!vfsreg::normalizeResRef(tableResRef16, targetNorm)) return 0;
        uint32_t seen[kMax2DaCells];
        uint32_t nSeen = 0;
        for (uint32_t i = 0; i < count; ++i) {
            if (!cells[i].active) continue;
            char tableNorm[16] = {0};
            if (!vfsreg::normalizeResRef(cells[i].table_name, tableNorm)) continue;
            if (std::memcmp(targetNorm, tableNorm, 16) != 0) continue;
            bool dup = false;
            for (uint32_t k = 0; k < nSeen; ++k) {
                if (seen[k] == cells[i].row_index) { dup = true; break; }
            }
            if (!dup) seen[nSeen++] = cells[i].row_index;
        }
        return nSeen;
    }

    uint32_t buildSpliced(const char* tableResRef16, const uint8_t* baseBuf, uint32_t baseSize, uint8_t* outBuf, uint32_t outCap) const {
        if (!tableResRef16 || !baseBuf || baseSize == 0) return 0;
        char targetNorm[16] = {0};
        if (!vfsreg::normalizeResRef(tableResRef16, targetNorm)) {
            return 0;
        }

        twoda::CellPatch patchBuf[kMax2DaCells];
        size_t numPatches = 0;
        int32_t templateRow = -1;

        for (uint32_t i = 0; i < count; ++i) {
            if (!cells[i].active) continue;
            char tableNorm[16] = {0};
            if (!vfsreg::normalizeResRef(cells[i].table_name, tableNorm)) continue;
            if (std::memcmp(targetNorm, tableNorm, 16) == 0) {
                patchBuf[numPatches].row = cells[i].row_index;
                patchBuf[numPatches].colName = cells[i].column_name;
                patchBuf[numPatches].value = cells[i].value;
                if (cells[i].template_row >= 0) {
                    templateRow = cells[i].template_row;
                }
                numPatches++;
            }
        }

        if (numPatches == 0) {
            return 0;
        }

        return twoda::spliceRows(baseBuf, baseSize, patchBuf, numPatches, outBuf, outCap, templateRow);
    }
};


// Engine data dword read once by the H4 hook for the log (0 = the stream gate branch is dead).
// Name: the InitializeSource stream-gate flag in .data (doc 44); no gameaddr:: name exists yet.
constexpr uintptr_t kStreamGateFlag = 0x00A10768;

// ---- configuration (asset_vfs.ini [vfs]) ----------------------------------------------------------
//
// mode=serve|observe|off (default observe when missing or unusable; legacy enabled=0 = off),
// log_prefix=<comma list> (case-insensitive prefix match on the normalised name; empty = counters only),
// log_all=0|1 (1 = log every name). Parsed from plain strings so it is host-testable.
constexpr int kModeOff     = 0;
constexpr int kModeObserve = 1;
constexpr int kModeServe   = 2;

inline const char* modeName(int mode) {
    return mode == kModeServe ? "serve" : (mode == kModeOff ? "off" : "observe");
}

inline const char* defaultLogPrefixes() { return "k1p_,kvj,p_juhani"; }

struct VfsConfig {
    static constexpr size_t kMaxPrefixes = 8;
    static constexpr size_t kPrefixLen = 24;

    int mode = kModeObserve;
    bool logAll = false;
    bool modeInvalid = false;           // mode= was present but not serve|observe|off
    char prefixes[kMaxPrefixes][kPrefixLen] = {};
    uint32_t nPrefixes = 0;

    static char lower(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c; }

    // Replaces the prefix list from a comma-separated string (trimmed, lowercased, empties skipped).
    void setPrefixes(const char* list) {
        nPrefixes = 0;
        std::memset(prefixes, 0, sizeof(prefixes));
        if (!list) return;
        const char* p = list;
        while (*p && nPrefixes < kMaxPrefixes) {
            while (*p == ',' || *p == ' ' || *p == '\t') ++p;
            char* dst = prefixes[nPrefixes];
            size_t n = 0;
            while (*p && *p != ',') {
                if (n + 1 < kPrefixLen && *p != ' ' && *p != '\t') {
                    dst[n++] = lower(*p);
                }
                ++p;
            }
            dst[n] = '\0';
            if (n > 0) ++nPrefixes;
        }
    }

    bool prefixMatch(const char* name) const {
        if (!name) return false;
        for (uint32_t i = 0; i < nPrefixes; ++i) {
            const char* pre = prefixes[i];
            size_t k = 0;
            while (pre[k] && name[k] && lower(name[k]) == pre[k]) ++k;
            if (pre[k] == '\0') return true;
        }
        return false;
    }

    bool wants(const char* name) const { return logAll || prefixMatch(name); }

    // modeStr: raw mode= value ("" or null = missing); legacyEnabled: enabled= value (1 when missing).
    static VfsConfig fromIni(const char* modeStr, int legacyEnabled, const char* prefixList, int logAllVal) {
        VfsConfig c;
        c.mode = kModeObserve;
        char m[16] = {0};
        size_t n = 0;
        if (modeStr) {
            const char* p = modeStr;
            while (*p == ' ' || *p == '\t') ++p;
            for (; *p && *p != ' ' && *p != '\t' && n + 1 < sizeof(m); ++p) m[n++] = lower(*p);
        }
        if (n > 0) {
            if (std::strcmp(m, "serve") == 0) c.mode = kModeServe;
            else if (std::strcmp(m, "observe") == 0) c.mode = kModeObserve;
            else if (std::strcmp(m, "off") == 0) c.mode = kModeOff;
            else c.modeInvalid = true;
        }
        if (legacyEnabled == 0) c.mode = kModeOff; // legacy opt-out always wins
        c.setPrefixes(prefixList);
        c.logAll = (logAllVal != 0);
        return c;
    }
};

// Per-name / total line budget for the `res` channel (call-count based, no allocation).
// Prefix test first, then: first kPerNameFirst lines per name, then every kPerNameEvery-th, total cap kTotalCap.
struct LogBudget {
    static constexpr uint32_t kSlots = 128;      // power of two
    static constexpr uint32_t kPerNameFirst = 20;
    static constexpr uint32_t kPerNameEvery = 100;
    static constexpr uint32_t kTotalCap = 400;

    uint32_t hashes[kSlots] = {};
    uint32_t counts[kSlots] = {};
    uint32_t total = 0;

    void clear() {
        std::memset(hashes, 0, sizeof(hashes));
        std::memset(counts, 0, sizeof(counts));
        total = 0;
    }

    static uint32_t hashName(const char* name) {
        uint32_t h = 2166136261u;
        for (const char* p = name; *p; ++p) {
            h = (h ^ static_cast<uint8_t>(VfsConfig::lower(*p))) * 16777619u;
        }
        return h | 1u; // never 0 (0 = empty slot)
    }

    // Call with the lock held. True = emit this line.
    bool take(const char* name) {
        if (!name || total >= kTotalCap) return false;
        const uint32_t h = hashName(name);
        uint32_t i = h & (kSlots - 1);
        for (uint32_t probe = 0; probe < kSlots; ++probe, i = (i + 1) & (kSlots - 1)) {
            if (hashes[i] == 0) hashes[i] = h;
            if (hashes[i] != h) continue;
            const uint32_t n = ++counts[i];
            const bool ok = n <= kPerNameFirst || ((n - kPerNameFirst) % kPerNameEvery) == 0;
            if (ok) ++total;
            return ok;
        }
        return false; // table full: silent
    }
};

struct Counters {
    std::atomic<uint32_t> h1Calls{0};
    std::atomic<uint32_t> misses{0};   // slow-path lookups (past the zero-handler fast path)
    std::atomic<uint32_t> served{0};
    std::atomic<uint32_t> observe{0};
    std::atomic<uint32_t> streams{0};
    std::atomic<uint32_t> splices{0};

    void clear() {
        h1Calls = 0; misses = 0; served = 0; observe = 0; streams = 0; splices = 0;
    }
};

// `res` channel outcomes for MissEventFn.
constexpr int kMissServe   = 1;
constexpr int kMissObserve = 2;
constexpr int kMissPass    = 3;
// Called WITHOUT the lock, only when the budget lets the line through.
typedef void (*MissEventFn)(const char* name, uint16_t resType, int outcome);
typedef void (*ServeEventFn)(const char* name, uint16_t resType, uint32_t size, int ret);

// Basename without extension, lowercased, truncated to cap-1 (the `res`/`audio` log name of a stream path).
inline void streamStem(const char* path, char* out, size_t cap) {
    if (!out || cap == 0) return;
    out[0] = '\0';
    if (!path) return;
    const char* start = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '\\' || *p == '/' || *p == ':') start = p + 1;
    }
    const char* end = start + std::strlen(start);
    for (const char* p = start; *p; ++p) {
        if (*p == '.') end = p;
    }
    size_t n = 0;
    for (const char* p = start; p < end && n + 1 < cap; ++p) out[n++] = VfsConfig::lower(*p);
    out[n] = '\0';
}

// Notice codes passed to EngineState::noticeFn (the provider turns them into KP_WARN lines).
constexpr int kNoticeServedReject   = 1; // what = resref, owner = registrant
constexpr int kNoticePoolFull       = 2; // what = "static" | "dynamic", owner = registrant
constexpr int kNoticeRewriteIgnored = 3; // what = "", owner = stream filter owner (REWRITE is disabled)
constexpr int kNoticeStemConflict   = 4; // what = candidate path, owner = stream filter owner
constexpr int kNoticeServedKept     = 5; // what = decimal count of served records kept, owner = unregistered owner
constexpr int kNoticeOpenFail       = 6; // what = spec, owner = reason ("cursor-full" | "bad-spec")
constexpr int kNoticeTwoDaFull      = 7; // what = decimal count of 2DA cells dropped (table full), owner = registrant
typedef void (*NoticeFn)(int code, const char* what, const char* owner);

// Provider-owned copies of handler/filter data (HeapAlloc on Win32, malloc on host; tests inject fakes).
typedef uint8_t* (*CopyAllocFn)(uint32_t size);
typedef void (*CopyFreeFn)(void* p);

inline uint8_t* defaultCopyAlloc(uint32_t size) {
#if defined(_WIN32)
    return static_cast<uint8_t*>(HeapAlloc(GetProcessHeap(), 0, size));
#else
    return static_cast<uint8_t*>(std::malloc(size));
#endif
}

inline void defaultCopyFree(void* p) {
    if (!p) return;
#if defined(_WIN32)
    HeapFree(GetProcessHeap(), 0, p);
#else
    std::free(p);
#endif
}

// registerOwnedCopy: a stream MEMORY claim on a stem already owned by a different candidate path.
constexpr int kRegPathConflict = -4;

struct EngineState {
    bool enabled = true;          // false = mode off: every hook returns 0 at the top
    bool observe = false;         // true = mode observe: hooks log what they would do and change no engine state
    vfsreg::Lock lock;            // guards reg, cursors, twodaPatches, handler/filter lists, work buffers
    vfsreg::Registry reg;
    vfsreg::MilesCursorTable cursors;
    TwoDaPatchTable twodaPatches;
    k2char::Catalog chars;
    uint8_t* workAlloc[vfsreg::Registry::kMaxAssets] = {};
    uint32_t workCap[vfsreg::Registry::kMaxAssets] = {};

    static constexpr size_t kMaxMissHandlers = 16;
    static constexpr size_t kMaxStreamFilters = 16;
    static constexpr size_t kMaxPathKey = 260;

    struct MissHandlerEntry {
        K2VfsResManMissFn fn = nullptr;
        void* user = nullptr;
        char owner[32] = {0};
        int32_t token = 0;
        bool active = false;
    };

    struct StreamFilterEntry {
        K2VfsStreamFilterFn fn = nullptr;
        void* user = nullptr;
        char owner[32] = {0};
        int32_t token = 0;
        bool active = false;
        bool warnedRewrite = false;
    };

    // Per dynamic-pool slot: the provider-owned data copy, the handler/filter token that owns it and
    // (stream records) the full normalised candidate path the record was claimed for.
    struct DynInfo {
        uint8_t* copy;
        int32_t token;
        char path[kMaxPathKey];
    };

    MissHandlerEntry missHandlers[kMaxMissHandlers] = {};
    StreamFilterEntry streamFilters[kMaxStreamFilters] = {};
    int32_t nextToken = 1;

    DynInfo dyn[vfsreg::Registry::kDynamicReserve] = {};
    vfsreg::NegCache neg;
    volatile int32_t activeMissHandlers = 0; // read lock-free by the H1 fast path
    bool inMiss = false;                      // recursion guard for the miss-handler loop

    CopyAllocFn copyAllocFn = defaultCopyAlloc;
    CopyFreeFn copyFreeFn = defaultCopyFree;

    NoticeFn noticeFn = nullptr;
    MissEventFn missEventFn = nullptr;
    ServeEventFn serveEventFn = nullptr;
    VfsConfig logCfg;             // prefixes / log_all (no prefixes + !logAll = counters only)
    LogBudget budget;
    Counters counters;
    bool warnedFull[2] = {false, false}; // [0] static pool, [1] dynamic pool

    EngineState() {
        init();
    }

    void reset() {
        enabled = true;
        observe = false;
        logCfg = VfsConfig(); // no prefixes, log_all off: counters only
        budget.clear();
        counters.clear();
        for (size_t i = 0; i < vfsreg::Registry::kDynamicReserve; ++i) {
            freeCopy(dyn[i].copy);
        }
        std::memset(dyn, 0, sizeof(dyn));
        reg.clear();
        cursors.clear();
        twodaPatches.clear();
        chars.clear();
        for (size_t i = 0; i < vfsreg::Registry::kMaxAssets; ++i) {
            workAlloc[i] = nullptr;
            workCap[i] = 0;
        }
        for (size_t i = 0; i < kMaxMissHandlers; ++i) {
            missHandlers[i] = {};
        }
        for (size_t i = 0; i < kMaxStreamFilters; ++i) {
            streamFilters[i] = {};
        }
        neg.clear();
        activeMissHandlers = 0;
        inMiss = false;
        nextToken = 1;
        warnedFull[0] = warnedFull[1] = false;
    }

    void init() {
        reset();
    }

    // Applies a parsed [vfs] config (mode + log selection). Provider startup only.
    void applyConfig(const VfsConfig& c) {
        enabled = (c.mode != kModeOff);
        observe = (c.mode == kModeObserve);
        logCfg = c;
    }

    // Log gate: prefix test first (no allocation), then the per-name / total budget. Takes the lock briefly;
    // never call with the lock held.
    bool takeLog(const char* name) {
        if (!name || !logCfg.wants(name)) return false;
        vfsreg::LockGuard g(lock);
        return budget.take(name);
    }

    // ---- owned copies -----------------------------------------------------------------------

    void freeCopy(void* p) {
        if (p && copyFreeFn) {
            copyFreeFn(p);
        }
    }

    static bool isDynIdx(size_t idx) {
        return idx >= vfsreg::Registry::kStaticSlots && idx < vfsreg::Registry::kMaxAssets;
    }

    DynInfo& dynOf(size_t idx) {
        return dyn[idx - vfsreg::Registry::kStaticSlots];
    }

    // Call with the lock held.
    bool tokenLiveLocked(int32_t token) const {
        if (token <= 0) return false;
        for (size_t i = 0; i < kMaxMissHandlers; ++i) {
            if (missHandlers[i].active && missHandlers[i].token == token) return true;
        }
        for (size_t i = 0; i < kMaxStreamFilters; ++i) {
            if (streamFilters[i].active && streamFilters[i].token == token) return true;
        }
        return false;
    }

    // Evicts every UNSERVED record owned by `token` (frees its copy); served records keep record and
    // copy (the engine still references them). Returns the number of served records kept.
    // Call with the lock held; the copies are freed here (copyFreeFn is not an external callback).
    uint32_t evictOwnerLocked(int32_t token) {
        uint32_t kept = 0;
        if (token <= 0) return 0;
        for (size_t i = 0; i < vfsreg::Registry::kDynamicReserve; ++i) {
            DynInfo& d = dyn[i];
            if (!d.copy || d.token != token) continue;
            const size_t idx = vfsreg::Registry::kStaticSlots + i;
            vfsreg::AssetRecord& rec = reg.records[idx];
            if (rec.active && rec.served) {
                ++kept;
                continue;
            }
            reg.evict(idx);
            freeCopy(d.copy);
            std::memset(&d, 0, sizeof(d));
        }
        return kept;
    }

    void emitServedKept(uint32_t kept, const char* owner) {
        if (kept > 0 && noticeFn) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(kept));
            noticeFn(kNoticeServedKept, buf, owner ? owner : "");
        }
    }

    // ---- registry (locked) ------------------------------------------------------------------

    // Returns slot index (>= 0) or vfsreg::kRegInvalid / kRegServed / kRegPoolFull.
    // `dynamic` registrations (miss handler / stream filter) use the reserved top slots.
    // Public registrations clear the miss negative cache (a handler may now answer differently).
    int registerAsset(const char* resref, uint16_t resType, const void* data, uint32_t size,
                      uint16_t flags, bool dynamic, const char* owner) {
        int rc = vfsreg::kRegInvalid;
        bool firstFull = false;
        {
            vfsreg::LockGuard g(lock);
            neg.clear();
            rc = reg.registerAsset(resref, resType, data, size, flags, dynamic);
            if (rc == vfsreg::kRegPoolFull && !warnedFull[dynamic ? 1 : 0]) {
                warnedFull[dynamic ? 1 : 0] = true;
                firstFull = true;
            }
        }
        if (noticeFn) {
            if (rc == vfsreg::kRegServed) {
                noticeFn(kNoticeServedReject, resref ? resref : "", owner ? owner : "");
            } else if (firstFull) {
                noticeFn(kNoticePoolFull, dynamic ? "dynamic" : "static", owner ? owner : "");
            }
        }
        return rc;
    }

    // Registers a provider-owned COPY of `data` in the dynamic pool, tagged with `token`.
    // `pathKey` (stream records) is the full normalised candidate path stored with the record; a record
    // for the same stem owned by a different path is rejected with kRegPathConflict, a record for the
    // same path is reused. Returns the slot index or a negative code. Never calls external code.
    int registerOwnedCopy(const char* resref, uint16_t resType, const void* data, uint32_t size,
                          uint16_t flags, const char* owner, int32_t token, const char* pathKey) {
        if (!data || size == 0 || !copyAllocFn) return vfsreg::kRegInvalid;
        uint8_t* copy = copyAllocFn(size);
        if (!copy) return vfsreg::kRegInvalid;
        std::memcpy(copy, data, size);

        int rc = vfsreg::kRegInvalid;
        bool firstFull = false;
        {
            vfsreg::LockGuard g(lock);
            rc = adoptCopyLocked(resref, resType, copy, size, flags, token, pathKey);
            if (rc == vfsreg::kRegPoolFull && !warnedFull[1]) {
                warnedFull[1] = true;
                firstFull = true;
            }
        }
        if (noticeFn) {
            if (rc == vfsreg::kRegServed) {
                noticeFn(kNoticeServedReject, resref ? resref : "", owner ? owner : "");
            } else if (firstFull) {
                noticeFn(kNoticePoolFull, "dynamic", owner ? owner : "");
            }
        }
        return rc;
    }

    // Takes ownership of `copy` (always either stored in dyn[] or freed). Lock held.
    int adoptCopyLocked(const char* resref, uint16_t resType, uint8_t* copy, uint32_t size,
                        uint16_t flags, int32_t token, const char* pathKey) {
        if (!tokenLiveLocked(token)) {
            freeCopy(copy); // handler/filter unregistered while its callback ran
            return vfsreg::kRegInvalid;
        }
        vfsreg::AssetRecord* ex = (flags == vfsreg::FlagMilesStream)
            ? reg.findStream(resref) : reg.findResMan(resref, resType);
        if (ex) {
            const size_t eidx = static_cast<size_t>(ex - reg.records);
            if (!isDynIdx(eidx)) {
                freeCopy(copy); // a static registration owns this key
                return vfsreg::kRegInvalid;
            }
            if (pathKey) {
                if (std::strcmp(dynOf(eidx).path, pathKey) != 0) {
                    freeCopy(copy);
                    return kRegPathConflict;
                }
                if (ex->served) {
                    freeCopy(copy); // same path, already streaming: reuse the served record as-is
                    return static_cast<int>(eidx);
                }
            }
        }
        int rc = reg.registerAsset(resref, resType, copy, size, flags, true);
        if (rc < 0 || !isDynIdx(static_cast<size_t>(rc))) {
            freeCopy(copy);
            return rc < 0 ? rc : vfsreg::kRegInvalid;
        }
        DynInfo& d = dynOf(static_cast<size_t>(rc));
        if (d.copy && d.copy != copy) {
            freeCopy(d.copy); // in-place update of an unserved record: drop the superseded copy
        }
        d.copy = copy;
        d.token = token;
        std::snprintf(d.path, sizeof(d.path), "%s", pathKey ? pathKey : "");
        return rc;
    }

    bool hasAsset(const char* resref, uint16_t resType, uint16_t flags) {
        vfsreg::LockGuard g(lock);
        if (flags == vfsreg::FlagResMan) {
            return reg.findResMan(resref, resType) != nullptr;
        }
        if (flags == vfsreg::FlagMilesStream) {
            return reg.findStream(resref) != nullptr;
        }
        return reg.findResMan(resref, resType) != nullptr || reg.findStream(resref) != nullptr;
    }

    int32_t addTwoDaCells(const K2Vfs2DaCell* newCells, uint32_t numCells, const char* owner = nullptr) {
        uint32_t dropped = 0;
        int32_t added;
        {
            vfsreg::LockGuard g(lock);
            added = twodaPatches.addCells(newCells, numCells, &dropped);
        }
        if (dropped > 0 && noticeFn) { // lock released
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(dropped));
            noticeFn(kNoticeTwoDaFull, buf, owner ? owner : "");
        }
        return added;
    }

    uint32_t countTwoDaRows(const char* tableResRef16) {
        vfsreg::LockGuard g(lock);
        return twodaPatches.countRows(tableResRef16);
    }

    uint32_t buildTwoDaSpliced(const char* tableResRef16, const uint8_t* baseBuf, uint32_t baseSize,
                               uint8_t* outBuf, uint32_t outCap) {
        vfsreg::LockGuard g(lock);
        return twodaPatches.buildSpliced(tableResRef16, baseBuf, baseSize, outBuf, outCap);
    }

    // ---- Miles cursor wrappers (locked) -----------------------------------------------------

    bool isVfsHandle(uint32_t h) {
        vfsreg::LockGuard g(lock);
        return cursors.isVfsHandle(h);
    }

    int32_t cursorSeek(uint32_t h, int32_t offset, uint32_t origin) {
        vfsreg::LockGuard g(lock);
        return cursors.seek(h, offset, origin);
    }

    uint32_t cursorRead(uint32_t h, void* dst, uint32_t bytes) {
        vfsreg::LockGuard g(lock);
        return cursors.read(h, dst, bytes);
    }

    bool cursorClose(uint32_t h) {
        vfsreg::LockGuard g(lock);
        return cursors.close(h);
    }

    // ---- handler / filter lists (locked) ----------------------------------------------------

    int32_t registerMissHandler(K2VfsResManMissFn fn, void* user, const char* owner) {
        if (!fn) return 0;
        vfsreg::LockGuard g(lock);
        neg.clear();
        for (size_t i = 0; i < kMaxMissHandlers; ++i) {
            if (!missHandlers[i].active) {
                int32_t tok = nextToken++;
                missHandlers[i].fn = fn;
                missHandlers[i].user = user;
                missHandlers[i].token = tok;
                missHandlers[i].active = true;
                if (owner) {
                    std::snprintf(missHandlers[i].owner, sizeof(missHandlers[i].owner), "%s", owner);
                } else {
                    missHandlers[i].owner[0] = '\0';
                }
                activeMissHandlers = activeMissHandlers + 1;
                return tok;
            }
        }
        return 0;
    }

    void unregisterMissHandler(int32_t token) {
        if (token <= 0) return;
        char owner[32] = {0};
        uint32_t kept = 0;
        {
            vfsreg::LockGuard g(lock);
            bool found = false;
            for (size_t i = 0; i < kMaxMissHandlers; ++i) {
                if (missHandlers[i].active && missHandlers[i].token == token) {
                    std::memcpy(owner, missHandlers[i].owner, sizeof(owner));
                    missHandlers[i] = {};
                    if (activeMissHandlers > 0) activeMissHandlers = activeMissHandlers - 1;
                    found = true;
                    break;
                }
            }
            if (!found) return;
            kept = evictOwnerLocked(token);
        }
        emitServedKept(kept, owner);
    }

    int32_t registerStreamFilter(K2VfsStreamFilterFn fn, void* user, const char* owner) {
        if (!fn) return 0;
        vfsreg::LockGuard g(lock);
        for (size_t i = 0; i < kMaxStreamFilters; ++i) {
            if (!streamFilters[i].active) {
                int32_t tok = nextToken++;
                streamFilters[i].fn = fn;
                streamFilters[i].user = user;
                streamFilters[i].token = tok;
                streamFilters[i].active = true;
                streamFilters[i].warnedRewrite = false;
                if (owner) {
                    std::snprintf(streamFilters[i].owner, sizeof(streamFilters[i].owner), "%s", owner);
                } else {
                    streamFilters[i].owner[0] = '\0';
                }
                return tok;
            }
        }
        return 0;
    }

    void unregisterStreamFilter(int32_t token) {
        if (token <= 0) return;
        char owner[32] = {0};
        uint32_t kept = 0;
        {
            vfsreg::LockGuard g(lock);
            bool found = false;
            for (size_t i = 0; i < kMaxStreamFilters; ++i) {
                if (streamFilters[i].active && streamFilters[i].token == token) {
                    std::memcpy(owner, streamFilters[i].owner, sizeof(owner));
                    streamFilters[i] = {};
                    found = true;
                    break;
                }
            }
            if (!found) return;
            kept = evictOwnerLocked(token);
        }
        emitServedKept(kept, owner);
    }

    // ---- H1: CExoResMan::GetKeyEntry miss ---------------------------------------------------

    // Lock-free pre-check for the H1 hook: true = this lookup certainly cannot be served by the VFS
    // (disabled, or no miss handler active AND no ResMan record of this type; log_all=1 keeps the slow
    // path open so every name can be logged), so the hook can zero
    // *ppTable and return 0 without any SafeRead or lock. Plain aligned 32-bit reads only.
    bool missCannotHit(uint16_t resType) const {
        return !enabled || (activeMissHandlers == 0 && !reg.mayHaveResMan(resType) && !logCfg.logAll);
    }

    // Hook H1 uses skip_original_bytes = true, so the stolen `MOV DWORD [ecx],0` (zeroing *ppTable)
    // never runs. This function therefore owns that write: on EVERY path returning 0 it zeroes
    // *ppTable (when ppTable is non-null); on a hit (1) it sets *ppTable = fake table and *ppEntry.
    // The native tail at 0x712457 (reached on 0) still zeroes *ppEntry and EAX.
    int handleGetKeyEntryMiss(const char* rawResRef16, uint16_t resType, void** ppTable, void** ppEntry) {
        int rc = resolveKeyEntryMiss(rawResRef16, resType, ppTable, ppEntry);
        if (rc == 0 && ppTable) {
            *ppTable = nullptr;
        }
        return rc;
    }

    // Reports one H1 outcome: counters always, the `res` line only when the log gate lets it through.
    void emitMiss(const char* name, uint16_t resType, int outcome) {
        counters.misses.fetch_add(1, std::memory_order_relaxed);
        if (outcome == kMissServe) counters.served.fetch_add(1, std::memory_order_relaxed);
        else if (outcome == kMissObserve) counters.observe.fetch_add(1, std::memory_order_relaxed);
        if (missEventFn && takeLog(name)) {
            missEventFn(name, resType, outcome);
        }
    }

    int resolveKeyEntryMiss(const char* rawResRef16, uint16_t resType, void** ppTable, void** ppEntry) {
        if (!enabled || !rawResRef16 || !ppTable || !ppEntry) {
            return 0;
        }

        char rawCopy[17] = {0};
        for (size_t i = 0; i < 16; ++i) {
            char c = rawResRef16[i];
            rawCopy[i] = c;
            if (c == '\0') break;
        }
        char norm[17] = {0};
        const bool normOk = vfsreg::normalizeResRef(rawCopy, norm);
        const char* logName = normOk ? norm : rawCopy;

        struct HandlerSnap {
            K2VfsResManMissFn fn;
            void* user;
            char owner[32];
            int32_t token;
        } snap[kMaxMissHandlers];
        size_t nSnap = 0;
        bool staticHit = false;
        {
            vfsreg::LockGuard g(lock);
            vfsreg::AssetRecord* rec = reg.findResMan(rawResRef16, resType);
            if (rec) {
                if (!observe) {
                    *ppTable = &reg.fakeTable;
                    *ppEntry = &rec->keyEntry;
                }
                staticHit = true;
            }
        }
        if (staticHit) {
            emitMiss(logName, resType, observe ? kMissObserve : kMissServe);
            return observe ? 0 : 1;
        }
        {
            vfsreg::LockGuard g(lock);
            if (inMiss || !normOk) {
                return 0; // re-entrant miss from inside a handler (or unusable name): plain miss
            }
            if (neg.contains(norm, resType)) {
                counters.misses.fetch_add(1, std::memory_order_relaxed);
                return 0;
            }
            for (size_t i = 0; i < kMaxMissHandlers; ++i) {
                if (missHandlers[i].active && missHandlers[i].fn) {
                    snap[nSnap].fn = missHandlers[i].fn;
                    snap[nSnap].user = missHandlers[i].user;
                    snap[nSnap].token = missHandlers[i].token;
                    std::memcpy(snap[nSnap].owner, missHandlers[i].owner, sizeof(snap[nSnap].owner));
                    nSnap++;
                }
            }
            if (nSnap == 0) {
                // fallthrough to the pass event below (after the lock is released)
            } else {
                inMiss = true;
            }
        }
        if (nSnap == 0) {
            emitMiss(logName, resType, kMissPass);
            return 0;
        }

        // Handlers run WITHOUT the lock held (they may call back into the VFS API), in slot order.
        // Observe mode: the handler is still called, its result is only logged (no registration, no copy).
        bool claimed = false;
        bool regFailed = false;
        bool wouldServe = false;
        for (size_t i = 0; i < nSnap; ++i) {
            K2VfsAssetEntry outEntry = {};
            int32_t handled = snap[i].fn(norm, resType, &outEntry, snap[i].user);
            if (handled == 1 && outEntry.data != nullptr && outEntry.size > 0) {
                if (observe) {
                    wouldServe = true;
                    break;
                }
                int idx = registerOwnedCopy(norm, resType, outEntry.data, outEntry.size,
                                            vfsreg::FlagResMan, snap[i].owner, snap[i].token, nullptr);
                if (idx >= 0) {
                    *ppTable = &reg.fakeTable;
                    *ppEntry = &reg.records[idx].keyEntry;
                    claimed = true;
                    break;
                }
                regFailed = true; // pool full / served / handler gone: not a real "nobody has it"
            }
        }

        {
            vfsreg::LockGuard g(lock);
            inMiss = false;
            if (!claimed && !wouldServe && !regFailed) {
                neg.add(norm, resType);
            }
        }
        emitMiss(logName, resType, claimed ? kMissServe : (wouldServe ? kMissObserve : kMissPass));
        return claimed ? 1 : 0;
    }

    // ---- H2: CExoResMan::ServiceFromImage ---------------------------------------------------

    // Mirrors the native contract: returns 0 (do nothing, native runs) when the CRes already has
    // flag 4 set or is not a servable VFS record; otherwise fills the CRes and returns the result of
    // OnResourceServiced (callOnServiced) or 1 when no callee is given. Non-zero = consumed.
    // Observe mode: returns 0 before touching anything (the CRes is never written).
    int handleServiceFromImage(uint8_t* pRes, uint8_t* (*allocFn)(uint32_t), int (*callOnServiced)(void*)) {
        if (!enabled || observe || !pRes) {
            return 0;
        }
        char logName[17] = {0};
        uint16_t logType = 0;
        uint32_t logSize = 0;
        {
            vfsreg::LockGuard g(lock);
            uint32_t packedId = *reinterpret_cast<const uint32_t*>(pRes + kResOffPackedId);
            vfsreg::AssetRecord* rec = reg.findByPackedId(packedId);
            if (!rec || !rec->data || rec->size == 0) {
                return 0;
            }
            uint32_t idx = static_cast<uint32_t>(rec - reg.records);
            if (idx >= vfsreg::Registry::kMaxAssets) {
                return 0;
            }
            if ((*reinterpret_cast<const uint32_t*>(pRes + kResOffStatus) & 0x4u) != 0) {
                return 0; // already serviced: native would return 0, leave everything untouched
            }

            uint32_t neededSize = rec->size + static_cast<uint32_t>(kWorkPrefixBytes);
            if (!workAlloc[idx] || workCap[idx] < neededSize) {
                if (!allocFn) {
                    return 0;
                }
                workAlloc[idx] = allocFn(neededSize);
                if (!workAlloc[idx]) {
                    return 0;
                }
                workCap[idx] = neededSize;
            }
            std::memcpy(logName, rec->keyEntry.resref, 16);
            logType = rec->keyEntry.resType;
            logSize = rec->size;
            std::memset(workAlloc[idx], 0, kWorkPrefixBytes);
            std::memcpy(workAlloc[idx] + kWorkPrefixBytes, rec->data, rec->size);

            *reinterpret_cast<uint32_t*>(pRes + kResOffSize) = rec->size;
            *reinterpret_cast<void**>(pRes + kResOffData) = workAlloc[idx] + kWorkPrefixBytes;
            *reinterpret_cast<uint32_t*>(pRes + kResOffStatus) |= 0x4;
            reg.markServed(idx);
        }

        // External callee: lock released. A 0 result is passed through (nothing is rolled back).
        const int ret = callOnServiced ? callOnServiced(pRes) : 1;
        if (serveEventFn && takeLog(logName)) {
            serveEventFn(logName, logType, logSize, ret);
        }
        return ret;
    }

    // ---- H4: streaming source open ----------------------------------------------------------

    static bool isVfsSpec(const char* s) {
        return s && std::strncmp(s, "#*VFS_", 6) == 0;
    }

    // Formats "#*VFS_<idx><ext>" for an active stream record. Lock held.
    bool makeStreamSpecLocked(size_t idx, uint16_t streamKind, char* outSpec, size_t specCap) {
        if (idx >= vfsreg::Registry::kMaxAssets || !reg.records[idx].active) {
            return false;
        }
        const vfsreg::AssetRecord& rec = reg.records[idx];
        vfsreg::AudioSlice slice = vfsreg::inspectAudioContainer(rec.data, rec.size, streamKind);
        const char* ext = slice.ext ? slice.ext : ".wav";
        int n = std::snprintf(outSpec, specCap, "#*VFS_%u%s", static_cast<unsigned>(idx), ext);
        return (n > 0 && static_cast<size_t>(n) < specCap);
    }

    // `allowFilters` is true only for the primary candidate (this+0x28); the fallback candidates are
    // resolved against the static registry only. Filters run only for streamKind 4 and 8.
    bool resolveStreamSpec(const char* candidatePath, uint16_t streamKind, char* outSpec, size_t specCap,
                           bool allowFilters = true) {
        if (!candidatePath || !outSpec || specCap == 0) {
            return false;
        }

        char npath[kMaxPathKey];
        vfsreg::normalizePath(candidatePath, npath, sizeof(npath));

        struct FilterSnap {
            K2VfsStreamFilterFn fn;
            void* user;
            char owner[32];
            int32_t token;
            size_t slot;
        } snap[kMaxStreamFilters];
        size_t nSnap = 0;
        {
            vfsreg::LockGuard g(lock);
            if (isVfsSpec(candidatePath)) {
                // Already a spec we issued (replay, loop, save/load): accept as-is while its slot is live.
                const char* numPtr = candidatePath + 6;
                char* endPtr = nullptr;
                unsigned long idx = std::strtoul(numPtr, &endPtr, 10);
                if (endPtr != numPtr && idx < vfsreg::Registry::kMaxAssets &&
                    reg.records[idx].active && reg.records[idx].flags == vfsreg::FlagMilesStream) {
                    int n = std::snprintf(outSpec, specCap, "%s", candidatePath);
                    return (n > 0 && static_cast<size_t>(n) < specCap);
                }
                return false;
            }
            vfsreg::AssetRecord* rec = reg.findStream(candidatePath);
            if (rec && rec->data && rec->size > 0) {
                size_t idx = static_cast<size_t>(rec - reg.records);
                // A filter-owned (dynamic) record only serves the exact candidate path it was claimed for.
                if (idx < vfsreg::Registry::kMaxAssets &&
                    (!isDynIdx(idx) || std::strcmp(dynOf(idx).path, npath) == 0)) {
                    return makeStreamSpecLocked(idx, streamKind, outSpec, specCap);
                }
            }
            if (!allowFilters || (streamKind != 4 && streamKind != 8)) {
                return false;
            }
            for (size_t i = 0; i < kMaxStreamFilters; ++i) {
                if (streamFilters[i].active && streamFilters[i].fn) {
                    snap[nSnap].fn = streamFilters[i].fn;
                    snap[nSnap].user = streamFilters[i].user;
                    snap[nSnap].token = streamFilters[i].token;
                    snap[nSnap].slot = i;
                    std::memcpy(snap[nSnap].owner, streamFilters[i].owner, sizeof(snap[nSnap].owner));
                    nSnap++;
                }
            }
        }

        // Not found in the static registry AND at least one stream filter is active (called unlocked,
        // in registration slot order; the first MEMORY claim wins).
        for (size_t i = 0; i < nSnap; ++i) {
            const void* outData = nullptr;
            uint32_t outSize = 0;
            char rewritten[260] = {0};
            int32_t rc = snap[i].fn(
                candidatePath, streamKind, &outData, &outSize,
                rewritten, sizeof(rewritten), snap[i].user);

            if (rc == K2VFS_STREAM_MEMORY && outData != nullptr && outSize > 0) {
                if (observe) {
                    // Observe: the claim is only reported (no copy, no registration).
                    int n = std::snprintf(outSpec, specCap, "observe");
                    return (n > 0 && static_cast<size_t>(n) < specCap);
                }
                int idx = registerOwnedCopy(candidatePath, vfsreg::ResTypeWav, outData, outSize,
                                            vfsreg::FlagMilesStream, snap[i].owner, snap[i].token, npath);
                if (idx == kRegPathConflict) {
                    if (noticeFn) noticeFn(kNoticeStemConflict, candidatePath, snap[i].owner);
                    continue; // treated as PASS
                }
                if (idx >= 0 && static_cast<size_t>(idx) < vfsreg::Registry::kMaxAssets) {
                    vfsreg::LockGuard g(lock);
                    return makeStreamSpecLocked(static_cast<size_t>(idx), streamKind, outSpec, specCap);
                }
            } else if (rc == K2VFS_STREAM_REWRITE) {
                // REWRITE is disabled: treated as PASS, one WARN per filter owner.
                bool warn = false;
                {
                    vfsreg::LockGuard g(lock);
                    StreamFilterEntry& e = streamFilters[snap[i].slot];
                    if (e.active && e.token == snap[i].token && !e.warnedRewrite) {
                        e.warnedRewrite = true;
                        warn = true;
                    }
                }
                if (warn && noticeFn) noticeFn(kNoticeRewriteIgnored, "", snap[i].owner);
            }
        }

        return false;
    }

    // Opens a cursor for a "#*VFS_<idx>.<ext>" spec. Names without the tag are NOT claimed here (they are
    // disk specs for Miles' own callbacks); there is deliberately no stem lookup fallback.
    bool parseVfsStreamSpec(const char* filename, uint32_t* outHandle) {
        if (!filename || !outHandle) {
            return false;
        }
        *outHandle = 0;

        const char* vfsTag = std::strstr(filename, "#*VFS_");
        if (!vfsTag) {
            return false;
        }

        const char* failReason = "bad-spec";
        {
            vfsreg::LockGuard g(lock);
            const char* numPtr = vfsTag + 6;
            char* endPtr = nullptr;
            unsigned long idx = std::strtoul(numPtr, &endPtr, 10);
            if (endPtr != numPtr && idx < vfsreg::Registry::kMaxAssets && reg.records[idx].active) {
                const vfsreg::AssetRecord& rec = reg.records[idx];
                vfsreg::AudioSlice slice = vfsreg::inspectAudioContainer(rec.data, rec.size, rec.keyEntry.resType);
                uint32_t h = cursors.open(slice.payload, slice.length);
                if (h != 0) {
                    reg.markServed(idx);
                    *outHandle = h;
                    return true;
                }
                failReason = "cursor-full";
            }
        }
        if (noticeFn) noticeFn(kNoticeOpenFail, filename, failReason); // lock released
        return false;
    }
};

// ---- Miles file-callback chaining ----------------------------------------------------------------
//
// Miles has exactly one global set of file callbacks (_AIL_set_file_callbacks); the engine opens every
// disk stream through them (specs like "*<path>*<len>*<off>"). We install a set that serves "#*VFS_n"
// names from the cursor table and forwards EVERYTHING else, unchanged, to whatever was installed before
// us (normally Miles' own defaults). The decision logic lives here, behind function pointers, so it is
// host-testable; the provider only supplies the Mss32 get/set shims and __stdcall thunks.

#if defined(_WIN32)
#define ASSETVFS_AILCALL __stdcall
#else
#define ASSETVFS_AILCALL
#endif

typedef uint32_t (ASSETVFS_AILCALL *AilOpenCb)(const char* filename, uint32_t* file_handle);
typedef void     (ASSETVFS_AILCALL *AilCloseCb)(uint32_t file_handle);
typedef int32_t  (ASSETVFS_AILCALL *AilSeekCb)(uint32_t file_handle, int32_t offset, uint32_t type);
typedef uint32_t (ASSETVFS_AILCALL *AilReadCb)(uint32_t file_handle, void* buffer, uint32_t bytes);

struct MilesCallbackSet {
    AilOpenCb open = nullptr;
    AilCloseCb close = nullptr;
    AilSeekCb seek = nullptr;
    AilReadCb read = nullptr;

    bool allSet() const { return open && close && seek && read; }
    bool sameAs(const MilesCallbackSet& o) const {
        return open == o.open && close == o.close && seek == o.seek && read == o.read;
    }
};

constexpr int kMilesNoticeChanged    = 1; // someone replaced the callbacks since we installed ours
constexpr int kMilesNoticeNoDefaults = 2; // callbacks still NULL after resetting to Miles' defaults (once)
typedef void (*MilesNoticeFn)(int code);

struct MilesChain {
    typedef void (*GetFn)(MilesCallbackSet* out);
    typedef void (*SetFn)(const MilesCallbackSet& set);

    EngineState* st = nullptr;
    GetFn getFn = nullptr;   // reads the current Miles callbacks (_AIL_file_callbacks)
    SetFn setFn = nullptr;   // writes them (_AIL_set_file_callbacks); all-NULL restores Miles' defaults
    MilesNoticeFn noticeFn = nullptr;
    MilesCallbackSet ours;
    MilesCallbackSet prev;   // the target every non-VFS call is forwarded to
    bool installed = false;
    bool noDefaultsNoticed = false;

    void bind(EngineState* state, const MilesCallbackSet& oursSet, GetFn get, SetFn set, MilesNoticeFn notice) {
        st = state;
        ours = oursSet;
        getFn = get;
        setFn = set;
        noticeFn = notice;
        prev = MilesCallbackSet();
        installed = false;
        noDefaultsNoticed = false;
    }

    void notify(int code) {
        if (noticeFn) noticeFn(code);
    }

    // Called on every VFS stream match. True = our callbacks are installed and `prev` is a complete
    // chain target; false = leave the stream to the engine's normal path.
    bool ensure() {
        if (!st || !getFn || !setFn || !ours.allSet()) {
            return false;
        }
        if (st->observe || !st->enabled) {
            return false; // observe/off: never touch the Miles callbacks
        }
        MilesCallbackSet cur;
        getFn(&cur);
        if (installed && cur.sameAs(ours)) {
            return true;
        }
        if (!cur.allSet()) {
            setFn(MilesCallbackSet()); // 4 NULLs: Miles restores its default callbacks
            cur = MilesCallbackSet();
            getFn(&cur);
        }
        // Entries that are still ours (partial overwrite) keep the target saved earlier, never ourselves.
        MilesCallbackSet np = cur;
        if (cur.open == ours.open) np.open = prev.open;
        if (cur.close == ours.close) np.close = prev.close;
        if (cur.seek == ours.seek) np.seek = prev.seek;
        if (cur.read == ours.read) np.read = prev.read;
        if (!np.allSet()) {
            if (!noDefaultsNoticed) {
                noDefaultsNoticed = true;
                notify(kMilesNoticeNoDefaults);
            }
            return false;
        }
        const bool changed = installed;
        prev = np;
        if (changed) {
            notify(kMilesNoticeChanged);
        }
        setFn(ours);
        installed = true;
        return true;
    }

    // ---- the callbacks themselves (the provider's __stdcall thunks call these) ----

    uint32_t open(const char* filename, uint32_t* handle) {
        if (!filename || !handle) return 0;
        if (std::strstr(filename, "#*VFS_")) {
            // Ours (never a disk path): serve from the cursor table, do not forward.
            return (st && st->parseVfsStreamSpec(filename, handle)) ? 1u : 0u;
        }
        return prev.open ? prev.open(filename, handle) : 0u;
    }

    void close(uint32_t handle) {
        if (st && st->cursorClose(handle)) return;
        if (vfsreg::MilesCursorTable::hasVfsTag(handle)) return; // stale/double close of ours: never forward
        if (prev.close) prev.close(handle);
    }

    int32_t seek(uint32_t handle, int32_t offset, uint32_t type) {
        if (st && st->isVfsHandle(handle)) return st->cursorSeek(handle, offset, type);
        if (vfsreg::MilesCursorTable::hasVfsTag(handle)) return -1;
        return prev.seek ? prev.seek(handle, offset, type) : -1;
    }

    uint32_t read(uint32_t handle, void* buffer, uint32_t bytes) {
        if (st && st->isVfsHandle(handle)) return st->cursorRead(handle, buffer, bytes);
        if (vfsreg::MilesCursorTable::hasVfsTag(handle)) return 0;
        return prev.read ? prev.read(handle, buffer, bytes) : 0u;
    }
};

} // namespace assetvfs
