#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace vfsreg {

// Tiny lock. Win32: CRITICAL_SECTION (recursive), initialised explicitly via init() from
// DllMain(DLL_PROCESS_ATTACH), never lazily. Until init() runs enter/leave are no-ops.
// Host builds: no-op shim (single-threaded tests). Never hold it while calling external callbacks.
struct Lock {
#if defined(_WIN32)
    CRITICAL_SECTION cs;
    bool ready;
    Lock() : ready(false) {}
    void init() {
        if (!ready) {
            InitializeCriticalSection(&cs);
            ready = true;
        }
    }
    void enter() { if (ready) EnterCriticalSection(&cs); }
    void leave() { if (ready) LeaveCriticalSection(&cs); }
#else
    Lock() {}
    void init() {}
    void enter() {}
    void leave() {}
#endif
    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;
};

struct LockGuard {
    Lock& l;
    explicit LockGuard(Lock& lock) : l(lock) { l.enter(); }
    ~LockGuard() { l.leave(); }
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

// Odyssey resource type constants
static constexpr uint16_t ResTypeWav = 4;
static constexpr uint16_t ResTypeTga = 3;
static constexpr uint16_t ResTypeNcs = 2010;
static constexpr uint16_t ResType2da = 2017;
static constexpr uint16_t ResTypeTxi = 2022;
static constexpr uint16_t ResTypeUtc = 2027;
static constexpr uint16_t ResTypeDlg = 2029;
static constexpr uint16_t ResTypeSsf = 2060;
static constexpr uint16_t ResTypeMdl = 2002;
static constexpr uint16_t ResTypeTpc = 3007;
static constexpr uint16_t ResTypeMdX = 3008;
static constexpr uint16_t ResTypeLip = 3004;

// Asset record flags
static constexpr uint16_t FlagResMan      = 0;
static constexpr uint16_t FlagMilesStream = 1;

// Packed ID encoding
// Source Type 1 = Image in bits 31:30 (0x40000000u) | KeyTable ID 0x3F (0x3Fu << 14 = 0x00FC0000u)
static constexpr uint32_t kVfsPackedPrefix = 0x40FC0000u;
static constexpr uint32_t kVfsIndexMask    = 0x00003FFFu;

inline uint32_t makePackedId(uint32_t idx) {
    return kVfsPackedPrefix | (idx & kVfsIndexMask);
}

inline bool isVfsPackedId(uint32_t packedId) {
    return (packedId & 0xFFFFC000u) == kVfsPackedPrefix;
}

inline uint32_t indexFromPackedId(uint32_t packedId) {
    return packedId & kVfsIndexMask;
}

#pragma pack(push, 1)
struct KeyTableEntry {
    char resref[16]; // +0x00: lowercase null-padded
#if defined(__i386__) || defined(_M_IX86) || (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4)
    void*    pRes;   // +0x10: bound CRes* (SetResObject writes here)
#else
    uint32_t pRes;   // +0x10: bound CRes* (32-bit engine pointer representation)
#endif
    uint32_t packedId; // +0x14: copied to CRes+0x08 by SetResObject
    uint16_t refCount; // +0x18
    uint16_t resType;  // +0x1A
};
static_assert(sizeof(KeyTableEntry) == 0x1C, "CKeyTableEntry must be 28 bytes");

struct FakeKeyTable {
    uint8_t  pad[0x1C];
    uint32_t tableType; // +0x1C: 4 = Image table (suppresses debug string alloc in GetResObject)
};
static_assert(sizeof(FakeKeyTable) == 0x20, "FakeKeyTable must be 32 bytes");
#pragma pack(pop)

inline bool normalizeResRef(const char* raw, char out16[16]) {
    if (!raw || !out16 || raw[0] == '\0') {
        return false;
    }

    // Strip leading directory prefixes: find character following last '\\', '/', ':'
    const char* start = raw;
    for (const char* p = raw; *p != '\0'; ++p) {
        if (*p == '\\' || *p == '/' || *p == ':') {
            start = p + 1;
        }
    }

    // Strip trailing .ext: find last '.' after start
    const char* lastDot = nullptr;
    for (const char* p = start; *p != '\0'; ++p) {
        if (*p == '.') {
            lastDot = p;
        }
    }

    const char* end = lastDot ? lastDot : (start + strlen(start));
    size_t stemLen = static_cast<size_t>(end - start);
    if (stemLen == 0 || stemLen > 16) {
        return false;
    }

    memset(out16, 0, 16);
    for (size_t i = 0; i < stemLen; ++i) {
        char c = start[i];
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c + ('a' - 'A'));
        } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
            // Valid character
        } else {
            return false;
        }
        out16[i] = c;
    }

    return true;
}

struct AssetRecord {
    KeyTableEntry keyEntry;
    const uint8_t* data;
    uint32_t size;
    uint16_t flags; // 0 = ResMan, 1 = MilesStream
    bool active;
    bool served;    // set once the engine copied/opened the data; re-registering the key is then rejected
};

// registerAsset return codes (>= 0 is the slot index)
static constexpr int kRegInvalid    = -1; // bad args / rejected container / bad resref
static constexpr int kRegServed     = -2; // key already registered AND already served to the engine
static constexpr int kRegPoolFull   = -3; // no free slot in the requested pool

struct Registry {
    static constexpr size_t kMaxAssets = 256;
    // Top 64 slots are reserved for dynamic registrations (miss handler / stream filter);
    // static RegisterAsset(s) calls use 0..kStaticSlots-1 so dynamic traffic cannot starve them.
    static constexpr size_t kDynamicReserve = 64;
    static constexpr size_t kStaticSlots = kMaxAssets - kDynamicReserve;

    AssetRecord records[kMaxAssets];
    FakeKeyTable fakeTable;

    Registry() {
        init();
    }

    void init() {
        clear();
    }

    void clear() {
        memset(records, 0, sizeof(records));
        memset(&fakeTable, 0, sizeof(fakeTable));
        fakeTable.tableType = 4;
    }

    // Returns slot index, kRegInvalid, kRegServed or kRegPoolFull.
    int registerAsset(const char* resref, uint16_t resType, const void* data, uint32_t size, uint16_t flags = 0, bool dynamic = false) {
        if (!data || size == 0) {
            return kRegInvalid;
        }
        if (flags == FlagResMan && resType == ResTypeTga) {
            return kRegInvalid; // TGA uses ReadRaw; VFS requires TPC 3007
        }
        if (flags == FlagResMan && resType == ResTypeWav) {
            const uint8_t* buf = static_cast<const uint8_t*>(data);
            // Layer 1 (CResWAV / PlaySound) requires bare RIFF WAVE PCM; reject 58-byte VO MP3
            // containers (which play silence per E2b) and 470-byte (0x1D6) SFX containers.
            if (size > 58 && memcmp(buf, "RIFF", 4) == 0) {
                uint32_t riffSz = 0;
                memcpy(&riffSz, buf + 4, sizeof(riffSz));
                if (riffSz == 50) {
                    return kRegInvalid;
                }
            }
            if (size > 0x1D6 && memcmp(buf + 0x1D6, "RIFF", 4) == 0) {
                return kRegInvalid;
            }
        }
        char norm[16];
        if (!normalizeResRef(resref, norm)) {
            return kRegInvalid;
        }

        // If already exists (either pool), update in place and return slot index,
        // unless the record was already served (the engine holds a copy / open cursor).
        for (size_t i = 0; i < kMaxAssets; ++i) {
            if (records[i].active &&
                records[i].flags == flags &&
                records[i].keyEntry.resType == resType &&
                memcmp(records[i].keyEntry.resref, norm, 16) == 0) {
                if (records[i].served) {
                    return kRegServed;
                }
                records[i].data = static_cast<const uint8_t*>(data);
                records[i].size = size;
                records[i].keyEntry.refCount = 1;
                return static_cast<int>(i);
            }
        }

        // Allocate next free slot in the requested pool
        const size_t lo = dynamic ? kStaticSlots : 0;
        const size_t hi = dynamic ? kMaxAssets : kStaticSlots;
        for (size_t i = lo; i < hi; ++i) {
            if (!records[i].active) {
                records[i].active = true;
                records[i].data = static_cast<const uint8_t*>(data);
                records[i].size = size;
                records[i].flags = flags;
                memset(&records[i].keyEntry, 0, sizeof(records[i].keyEntry));
                memcpy(records[i].keyEntry.resref, norm, 16);
                records[i].keyEntry.pRes = 0;
                records[i].keyEntry.packedId = makePackedId(static_cast<uint32_t>(i));
                records[i].keyEntry.refCount = 1;
                records[i].keyEntry.resType = resType;
                return static_cast<int>(i);
            }
        }

        return kRegPoolFull;
    }

    void markServed(size_t idx) {
        if (idx < kMaxAssets && records[idx].active) {
            records[idx].served = true;
        }
    }

    AssetRecord* findResMan(const char* rawResRef16, uint16_t resType) {
        if (!rawResRef16) {
            return nullptr;
        }
        char target[16] = {0};
        bool hitNull = false;
        for (int i = 0; i < 16; ++i) {
            if (hitNull) {
                target[i] = 0;
            } else {
                char c = rawResRef16[i];
                if (c == '\0') {
                    hitNull = true;
                    target[i] = 0;
                } else {
                    if (c >= 'A' && c <= 'Z') {
                        c = static_cast<char>(c + ('a' - 'A'));
                    }
                    target[i] = c;
                }
            }
        }

        for (size_t i = 0; i < kMaxAssets; ++i) {
            if (records[i].active &&
                records[i].flags == FlagResMan &&
                records[i].keyEntry.resType == resType &&
                memcmp(records[i].keyEntry.resref, target, 16) == 0) {
                return &records[i];
            }
        }
        return nullptr;
    }

    AssetRecord* findByPackedId(uint32_t packedId) {
        if (!isVfsPackedId(packedId)) {
            return nullptr;
        }
        uint32_t idx = indexFromPackedId(packedId);
        if (idx < kMaxAssets && records[idx].active) {
            return &records[idx];
        }
        return nullptr;
    }

    AssetRecord* findStream(const char* pathOrResRef) {
        if (!pathOrResRef) {
            return nullptr;
        }
        char norm[16];
        if (!normalizeResRef(pathOrResRef, norm)) {
            return nullptr;
        }
        for (size_t i = 0; i < kMaxAssets; ++i) {
            if (records[i].active &&
                records[i].flags == FlagMilesStream &&
                memcmp(records[i].keyEntry.resref, norm, 16) == 0) {
                return &records[i];
            }
        }
        return nullptr;
    }
};

struct AudioSlice {
    const uint8_t* payload;
    uint32_t length;
    uint32_t offset;
    const char* ext;
};

inline AudioSlice inspectAudioContainer(const uint8_t* buf, uint32_t size, uint16_t streamKind) {
    if (buf && size > 58 && memcmp(buf, "RIFF", 4) == 0) {
        uint32_t riffSz = 0;
        memcpy(&riffSz, buf + 4, sizeof(riffSz));
        if (riffSz == 50) {
            return { buf + 58, size - 58, 58, ".mp3" };
        }
    }
    if (buf && size > 0x1D6 && memcmp(buf + 0x1D6, "RIFF", 4) == 0) {
        return { buf + 0x1D6, size - 0x1D6, 0x1D6, ".wav" };
    }
    return { buf, size, 0, (streamKind == 8) ? ".mp3" : ".wav" };
}

struct MilesCursorTable {
    static constexpr size_t kMaxCursors = 16;
    // Handle = 0xFFFF0000 | (slot << 1) | 1. The exe is large-address-aware so heap pointers may have
    // bit 31 set; this tag (top 16 bits set, bit 0 set, tiny slot, slot active) cannot collide with a
    // kernel file HANDLE (small ints) and isVfsHandle additionally checks the slot is live.
    static constexpr uint32_t kCursorHandleTag  = 0xFFFF0000u;
    static constexpr uint32_t kCursorHandleMask = 0xFFFF0001u; // tag bits + the odd bit

    struct Cursor {
        const uint8_t* payload;
        uint32_t length;
        uint32_t pos;
        bool active;
    };

    Cursor cursors[kMaxCursors];

    MilesCursorTable() {
        clear();
    }

    void init() {
        clear();
    }

    void clear() {
        memset(cursors, 0, sizeof(cursors));
    }

    uint32_t open(const uint8_t* payload, uint32_t length) {
        if (!payload) {
            return 0;
        }
        for (size_t i = 0; i < kMaxCursors; ++i) {
            if (!cursors[i].active) {
                cursors[i].payload = payload;
                cursors[i].length = length;
                cursors[i].pos = 0;
                cursors[i].active = true;
                return kCursorHandleTag | (static_cast<uint32_t>(i) << 1) | 1u;
            }
        }
        return 0; // Full
    }

    static bool hasVfsTag(uint32_t h) {
        return (h & kCursorHandleMask) == (kCursorHandleTag | 1u);
    }

    static uint32_t slotOf(uint32_t h) {
        return (h & 0xFFFFu) >> 1;
    }

    // True only for a handle this table issued and has not closed yet.
    bool isVfsHandle(uint32_t h) const {
        if (!hasVfsTag(h)) {
            return false;
        }
        uint32_t slot = slotOf(h);
        return slot < kMaxCursors && cursors[slot].active;
    }

    int32_t seek(uint32_t h, int32_t offset, uint32_t origin) {
        if (!isVfsHandle(h)) {
            return -1;
        }
        uint32_t slot = slotOf(h);
        if (slot >= kMaxCursors || !cursors[slot].active) {
            return -1;
        }

        int64_t newPos = 0;
        if (origin == 0) { // SEEK_SET / begin
            newPos = static_cast<int64_t>(offset);
        } else if (origin == 1) { // SEEK_CUR / cur
            newPos = static_cast<int64_t>(cursors[slot].pos) + static_cast<int64_t>(offset);
        } else if (origin == 2) { // SEEK_END / end
            newPos = static_cast<int64_t>(cursors[slot].length) + static_cast<int64_t>(offset);
        } else {
            return -1; // Invalid origin
        }

        if (newPos < 0) {
            newPos = 0;
        }
        if (newPos > static_cast<int64_t>(cursors[slot].length)) {
            newPos = static_cast<int64_t>(cursors[slot].length);
        }

        cursors[slot].pos = static_cast<uint32_t>(newPos);
        return static_cast<int32_t>(cursors[slot].pos);
    }

    uint32_t read(uint32_t h, void* dst, uint32_t bytes) {
        if (!isVfsHandle(h) || !dst || bytes == 0) {
            return 0;
        }
        uint32_t slot = slotOf(h);
        if (slot >= kMaxCursors || !cursors[slot].active) {
            return 0;
        }

        if (cursors[slot].pos >= cursors[slot].length) {
            return 0; // EOF
        }

        uint32_t avail = cursors[slot].length - cursors[slot].pos;
        uint32_t toCopy = (bytes < avail) ? bytes : avail;
        memcpy(dst, cursors[slot].payload + cursors[slot].pos, toCopy);
        cursors[slot].pos += toCopy;
        return toCopy;
    }

    bool close(uint32_t h) {
        if (!isVfsHandle(h)) {
            return false;
        }
        uint32_t slot = slotOf(h);
        if (slot >= kMaxCursors || !cursors[slot].active) {
            return false;
        }
        cursors[slot].active = false;
        cursors[slot].payload = nullptr;
        cursors[slot].length = 0;
        cursors[slot].pos = 0;
        return true;
    }
};

} // namespace vfsreg
