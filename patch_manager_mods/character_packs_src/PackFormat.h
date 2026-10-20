// PackFormat.h: reader for the .k2pack format, version 1 (spec: patch_manager_mods/k1_port/K2PACK_FORMAT.md).
// Header-only, pure (no Windows includes, no I/O, no allocation beyond the output vectors), host-testable.
// parse() checks EVERY reader rule of the spec and never reads past `len` for any input. On any failure it
// returns the first failed rule and leaves `assets` and `cells` empty. All integers are read byte-wise
// (little-endian), so there are no unaligned casts.
#ifndef K2PACK_FORMAT_H
#define K2PACK_FORMAT_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <vector>

namespace k2pack {

constexpr uint32_t kVersion = 1;
constexpr uint32_t kHeaderSize = 32;
constexpr uint32_t kCharBlockSize = 208;
constexpr uint32_t kAssetEntrySize = 32;
constexpr uint32_t kCellEntrySize = 120;
constexpr uint32_t kMaxAssets = 448;  // asset-vfs shared static pool (Registry::kStaticSlots; was 192 until 10-20)
constexpr uint32_t kMaxCells = 256;   // asset-vfs shared 2DA cell pool
constexpr uint64_t kMaxFileBytes = 256ull * 1024 * 1024;

constexpr uint16_t kStreamFlag = 1;
constexpr uint16_t kTypeWav = 4;

struct Character {
    char id[32];
    char display_name[64];
    char default_tag[32];
    char default_utc[32];
    char default_dlg[32];
    uint32_t appearance_id;
    uint32_t head_id;
    uint32_t soundset_id;
};

struct AssetView {
    char resref[17];  // NUL-terminated copy
    uint16_t res_type;
    uint16_t flags;
    uint32_t offset;  // absolute file offset of the blob
    uint32_t size;
};

struct CellView {
    char table[17];  // NUL-terminated copy
    uint32_t row;
    int32_t template_row;
    char column[32];
    char value[64];
};

enum class Err {
    Ok = 0,
    TooShort,        // file shorter than header + character block
    BadMagic,
    BadVersion,
    BadHeaderSize,
    BadAssetCount,   // not 1..192
    BadCellCount,    // not 0..256
    BadTotalSize,    // total_size != file length
    BadCrc,
    BadReserved,     // a reserved field is not 0
    TableBounds,     // asset/cell tables do not fit inside total_size
    FieldNoNul,      // a char[N] field has no NUL
    FieldPad,        // non-NUL bytes after the first NUL of a padded field
    CharId,
    DisplayName,
    DefaultTag,
    DefaultUtc,
    DefaultDlg,
    AssetResref,
    AssetType,       // res_type not on the allow-list
    AssetFlags,      // flags not 0/1, or stream flag on a non-wav type
    AssetAlign,      // data_offset not a multiple of 8
    AssetSize,       // size == 0
    AssetRange,      // blob before the blob area or past total_size
    AssetOverlap,
    AssetDup,
    CellTable,
    CellTemplate,    // template_row < -1
    CellColumn,
    CellValue,
    CellDup,
};

inline const char* errName(Err e) {
    switch (e) {
        case Err::Ok: return "ok";
        case Err::TooShort: return "too-short";
        case Err::BadMagic: return "bad-magic";
        case Err::BadVersion: return "bad-version";
        case Err::BadHeaderSize: return "bad-header-size";
        case Err::BadAssetCount: return "bad-asset-count";
        case Err::BadCellCount: return "bad-cell-count";
        case Err::BadTotalSize: return "bad-total-size";
        case Err::BadCrc: return "bad-crc";
        case Err::BadReserved: return "bad-reserved";
        case Err::TableBounds: return "table-bounds";
        case Err::FieldNoNul: return "field-no-nul";
        case Err::FieldPad: return "field-padding";
        case Err::CharId: return "bad-character-id";
        case Err::DisplayName: return "bad-display-name";
        case Err::DefaultTag: return "bad-default-tag";
        case Err::DefaultUtc: return "bad-default-utc";
        case Err::DefaultDlg: return "bad-default-dlg";
        case Err::AssetResref: return "bad-asset-resref";
        case Err::AssetType: return "bad-asset-type";
        case Err::AssetFlags: return "bad-asset-flags";
        case Err::AssetAlign: return "bad-asset-align";
        case Err::AssetSize: return "bad-asset-size";
        case Err::AssetRange: return "bad-asset-range";
        case Err::AssetOverlap: return "asset-overlap";
        case Err::AssetDup: return "duplicate-asset";
        case Err::CellTable: return "bad-cell-table";
        case Err::CellTemplate: return "bad-cell-template-row";
        case Err::CellColumn: return "bad-cell-column";
        case Err::CellValue: return "bad-cell-value";
        case Err::CellDup: return "duplicate-cell";
    }
    return "unknown";
}

inline uint32_t crc32(const uint8_t* p, size_t n) {
    static const struct Table {
        uint32_t t[256];
        Table() {
            for (uint32_t i = 0; i < 256; ++i) {
                uint32_t c = i;
                for (int k = 0; k < 8; ++k) c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
                t[i] = c;
            }
        }
    } tab;
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; ++i) c = tab.t[(c ^ p[i]) & 0xFFu] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

namespace detail {

inline uint16_t rd16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }
inline uint32_t rd32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

// Length of the string in a fixed field of `cap` bytes: -1 when there is no NUL inside the field.
inline int fieldLen(const uint8_t* f, size_t cap) {
    for (size_t i = 0; i < cap; ++i)
        if (f[i] == 0) return static_cast<int>(i);
    return -1;
}

// All bytes after the first NUL are NUL as well.
inline bool padClean(const uint8_t* f, size_t cap, size_t len) {
    for (size_t i = len; i < cap; ++i)
        if (f[i] != 0) return false;
    return true;
}

inline bool isResrefCh(uint8_t c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; }
inline bool isPrintable(uint8_t c) { return c >= 0x20 && c <= 0x7E; }

inline bool resrefOk(const uint8_t* s, size_t n, size_t minLen, size_t maxLen) {
    if (n < minLen || n > maxLen) return false;
    for (size_t i = 0; i < n; ++i)
        if (!isResrefCh(s[i])) return false;
    return true;
}

inline bool printableOk(const uint8_t* s, size_t n, bool allowSpace) {
    for (size_t i = 0; i < n; ++i) {
        if (!isPrintable(s[i])) return false;
        if (!allowSpace && s[i] == 0x20) return false;
    }
    return true;
}

inline bool isAllowedType(uint16_t t) {
    switch (t) {
        case 2002: case 3008: case 3007: case 2022: case 2027: case 2029: case 2010: case 2060: case 3004: case 4:
            return true;
        default:
            return false;
    }
}

// Reads a NUL-padded char[cap] field with at most cap-1 chars into dst (dst must hold cap bytes).
// Returns Ok, FieldNoNul or FieldPad.
inline Err readField(const uint8_t* f, size_t cap, char* dst, int& outLen) {
    int n = fieldLen(f, cap);
    if (n < 0) return Err::FieldNoNul;
    if (!padClean(f, cap, static_cast<size_t>(n))) return Err::FieldPad;
    memcpy(dst, f, static_cast<size_t>(n));
    dst[n] = '\0';
    outLen = n;
    return Err::Ok;
}

}  // namespace detail

// Parses and validates a whole pack. `data` must stay valid for the call only; the output holds copies and
// offsets into `data`. On failure `assets` and `cells` are cleared.
inline Err parse(const uint8_t* data, size_t len, Character& out, std::vector<AssetView>& assets,
                 std::vector<CellView>& cells) {
    using namespace detail;
    assets.clear();
    cells.clear();
    memset(&out, 0, sizeof(out));

    auto fail = [&](Err e) {
        assets.clear();
        cells.clear();
        return e;
    };

    if (!data || len < kHeaderSize + kCharBlockSize) return fail(Err::TooShort);
    if (len > 0xFFFFFFFFull || len > kMaxFileBytes) return fail(Err::BadTotalSize);

    // ---- header ----
    if (memcmp(data, "K2PK", 4) != 0) return fail(Err::BadMagic);
    if (rd32(data + 4) != kVersion) return fail(Err::BadVersion);
    if (rd32(data + 8) != kHeaderSize) return fail(Err::BadHeaderSize);
    const uint32_t assetCount = rd32(data + 12);
    const uint32_t cellCount = rd32(data + 16);
    const uint32_t totalSize = rd32(data + 20);
    const uint32_t crcStored = rd32(data + 24);
    if (assetCount < 1 || assetCount > kMaxAssets) return fail(Err::BadAssetCount);
    if (cellCount > kMaxCells) return fail(Err::BadCellCount);
    if (totalSize != len) return fail(Err::BadTotalSize);
    if (rd32(data + 28) != 0) return fail(Err::BadReserved);
    if (crc32(data + kHeaderSize, len - kHeaderSize) != crcStored) return fail(Err::BadCrc);

    // ---- table bounds (counts are capped, so the 64-bit sum cannot overflow) ----
    const uint64_t assetTab = kHeaderSize + kCharBlockSize;
    const uint64_t cellTab = assetTab + static_cast<uint64_t>(kAssetEntrySize) * assetCount;
    const uint64_t tabEnd = cellTab + static_cast<uint64_t>(kCellEntrySize) * cellCount;
    if (tabEnd > totalSize) return fail(Err::TableBounds);
    const uint64_t blobStart = (tabEnd + 7) & ~static_cast<uint64_t>(7);

    // ---- character block ----
    {
        const uint8_t* b = data + kHeaderSize;
        int n = 0;
        Err e = readField(b, 32, out.id, n);
        if (e != Err::Ok) return fail(e);
        if (!resrefOk(b, static_cast<size_t>(n), 1, 31)) return fail(Err::CharId);

        e = readField(b + 32, 64, out.display_name, n);
        if (e != Err::Ok) return fail(e);
        if (n < 1 || !printableOk(b + 32, static_cast<size_t>(n), true)) return fail(Err::DisplayName);

        e = readField(b + 96, 32, out.default_tag, n);
        if (e != Err::Ok) return fail(e);
        if (!printableOk(b + 96, static_cast<size_t>(n), true)) return fail(Err::DefaultTag);

        e = readField(b + 128, 32, out.default_utc, n);
        if (e != Err::Ok) return fail(e);
        if (n != 0 && !resrefOk(b + 128, static_cast<size_t>(n), 1, 16)) return fail(Err::DefaultUtc);

        e = readField(b + 160, 32, out.default_dlg, n);
        if (e != Err::Ok) return fail(e);
        if (n != 0 && !resrefOk(b + 160, static_cast<size_t>(n), 1, 16)) return fail(Err::DefaultDlg);

        out.appearance_id = rd32(b + 192);
        out.head_id = rd32(b + 196);
        out.soundset_id = rd32(b + 200);
        if (rd32(b + 204) != 0) return fail(Err::BadReserved);
    }

    // ---- asset table ----
    assets.reserve(assetCount);
    for (uint32_t i = 0; i < assetCount; ++i) {
        const uint8_t* e = data + assetTab + static_cast<uint64_t>(i) * kAssetEntrySize;
        AssetView a;
        memset(&a, 0, sizeof(a));

        // resref may fill all 16 bytes (no NUL): length is the span up to the first NUL or 16
        size_t n = 0;
        while (n < 16 && e[n] != 0) ++n;
        if (!padClean(e, 16, n)) return fail(Err::FieldPad);
        if (!resrefOk(e, n, 1, 16)) return fail(Err::AssetResref);
        memcpy(a.resref, e, n);
        a.resref[n] = '\0';

        a.res_type = rd16(e + 16);
        a.flags = rd16(e + 18);
        a.offset = rd32(e + 20);
        a.size = rd32(e + 24);
        if (rd32(e + 28) != 0) return fail(Err::BadReserved);

        if (!isAllowedType(a.res_type)) return fail(Err::AssetType);
        if (a.flags > kStreamFlag) return fail(Err::AssetFlags);
        if (a.flags == kStreamFlag && a.res_type != kTypeWav) return fail(Err::AssetFlags);
        if (a.size == 0) return fail(Err::AssetSize);
        if ((a.offset & 7u) != 0) return fail(Err::AssetAlign);
        if (a.offset < blobStart) return fail(Err::AssetRange);
        if (static_cast<uint64_t>(a.offset) + a.size > totalSize) return fail(Err::AssetRange);
        assets.push_back(a);
    }

    // duplicates (flags, res_type, resref) and blob overlap: n <= 192, quadratic is fine
    for (size_t i = 0; i < assets.size(); ++i) {
        for (size_t j = i + 1; j < assets.size(); ++j) {
            const AssetView& x = assets[i];
            const AssetView& y = assets[j];
            if (x.flags == y.flags && x.res_type == y.res_type && strcmp(x.resref, y.resref) == 0)
                return fail(Err::AssetDup);
            const uint64_t xe = static_cast<uint64_t>(x.offset) + x.size;
            const uint64_t ye = static_cast<uint64_t>(y.offset) + y.size;
            if (x.offset < ye && y.offset < xe) return fail(Err::AssetOverlap);
        }
    }

    // ---- cell table ----
    cells.reserve(cellCount);
    for (uint32_t i = 0; i < cellCount; ++i) {
        const uint8_t* e = data + cellTab + static_cast<uint64_t>(i) * kCellEntrySize;
        CellView c;
        memset(&c, 0, sizeof(c));

        size_t n = 0;
        while (n < 16 && e[n] != 0) ++n;
        if (!padClean(e, 16, n)) return fail(Err::FieldPad);
        if (!resrefOk(e, n, 1, 16)) return fail(Err::CellTable);
        memcpy(c.table, e, n);
        c.table[n] = '\0';

        c.row = rd32(e + 16);
        c.template_row = static_cast<int32_t>(rd32(e + 20));
        if (c.template_row < -1) return fail(Err::CellTemplate);

        int len2 = 0;
        Err er = readField(e + 24, 32, c.column, len2);
        if (er != Err::Ok) return fail(er);
        if (len2 < 1 || !printableOk(e + 24, static_cast<size_t>(len2), false)) return fail(Err::CellColumn);

        er = readField(e + 56, 64, c.value, len2);
        if (er != Err::Ok) return fail(er);
        if (!printableOk(e + 56, static_cast<size_t>(len2), true)) return fail(Err::CellValue);
        cells.push_back(c);
    }
    for (size_t i = 0; i < cells.size(); ++i)
        for (size_t j = i + 1; j < cells.size(); ++j)
            if (cells[i].row == cells[j].row && strcmp(cells[i].table, cells[j].table) == 0 &&
                strcmp(cells[i].column, cells[j].column) == 0)
                return fail(Err::CellDup);

    return Err::Ok;
}

}  // namespace k2pack

#endif  // K2PACK_FORMAT_H
