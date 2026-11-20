// QuickLootErf.h -- pure ERF V1.0 reader for the bezel source lookup (doc 52 QL15). Finds one resource in a texture pack such as
// texturepacks/swpc_tex_gui.erf, which holds the vanilla pnl_* panels. Layout (pykotor io_erf.py): "ERF " + "V1.0" at 0, u32
// entryCount @16, u32 key list offset @24, u32 resource list offset @28. Key entry (24 B): char resref[16] (NUL padded), u32 ResID
// @16, u16 ResType @20, u16 unused. Resource entry (8 B): u32 offset, u32 size. Key i and resource i describe the same resource.
//
// The texture pack is ~132 MB, so the DLL reads only the header and tables (tablesEnd bytes), calls locate, and then seeks to the
// one resource. findResource is the whole-buffer form the host tests use. No Win32, no engine calls.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace qlerf {

constexpr size_t kHeaderBytes = 160;
constexpr size_t kKeyBytes = 24;
constexpr size_t kResBytes = 8;
constexpr uint32_t kMaxEntries = 1u << 20;   // our sanity limit; a count above it is refused before any table is read

struct Found {
    const uint8_t* data = nullptr;
    size_t size = 0;
};

struct Locator {
    uint64_t offset = 0;   // from the start of the file
    uint64_t size = 0;
};

inline uint16_t rd16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }
inline uint32_t rd32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

inline char lowerAscii(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c; }

// The key's 16-byte NUL-padded resref field against a name of 1..16 characters, case-insensitively (K2 resrefs are).
inline bool resrefMatches(const uint8_t* field, const char* name) {
    const size_t len = strlen(name);
    if (len == 0 || len > 16) return false;
    for (size_t i = 0; i < 16; ++i) {
        const char want = i < len ? name[i] : '\0';
        if (lowerAscii(static_cast<char>(field[i])) != lowerAscii(want)) return false;
    }
    return true;
}

// The number of bytes from the start of the file that hold the header and both tables. False when head is not ERF V1.0 with a
// count in 1..kMaxEntries. The caller reads this many bytes, then calls locate.
inline bool tablesEnd(const uint8_t* head, size_t headN, uint64_t* end) {
    if (!head || !end || headN < kHeaderBytes) return false;
    if (memcmp(head, "ERF ", 4) != 0 || memcmp(head + 4, "V1.0", 4) != 0) return false;
    const uint32_t count = rd32(head + 16);
    if (count == 0 || count > kMaxEntries) return false;
    const uint64_t keysEnd = uint64_t{rd32(head + 24)} + uint64_t{count} * kKeyBytes;
    const uint64_t resEnd = uint64_t{rd32(head + 28)} + uint64_t{count} * kResBytes;
    *end = keysEnd > resEnd ? keysEnd : resEnd;
    return true;
}

// Finds resref/restype. head holds the first headN bytes of the file, which must reach tablesEnd(); fileSize is the whole file's
// size. True with *out set to the resource's place in the file. False when the name is absent, the header is not ERF V1.0, the
// tables are not inside head, or the resource runs past the end of the file (refused, not read past).
inline bool locate(const uint8_t* head, size_t headN, uint64_t fileSize, const char* resref, uint16_t restype, Locator* out) {
    uint64_t end = 0;
    if (!resref || !out || !tablesEnd(head, headN, &end) || end > headN) return false;
    const uint32_t count = rd32(head + 16);
    const uint64_t keysAt = rd32(head + 24), resAt = rd32(head + 28);
    for (uint32_t i = 0; i < count; ++i) {
        const uint8_t* k = head + keysAt + size_t{i} * kKeyBytes;
        if (rd16(k + 20) != restype || !resrefMatches(k, resref)) continue;
        const uint8_t* r = head + resAt + size_t{i} * kResBytes;
        const uint64_t off = rd32(r), sz = rd32(r + 4);
        if (off + sz > fileSize) return false;
        out->offset = off;
        out->size = sz;
        return true;
    }
    return false;
}

// Whole-buffer form: finds the resource in e[0..n) and points *out into e. The same refusals as locate.
inline bool findResource(const uint8_t* e, size_t n, const char* resref, uint16_t restype, Found* out) {
    if (!e || !out) return false;
    Locator loc;
    if (!locate(e, n, n, resref, restype, &loc)) return false;
    out->data = e + loc.offset;   // offset + size <= n was checked by locate
    out->size = static_cast<size_t>(loc.size);
    return true;
}

}  // namespace qlerf
