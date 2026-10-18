// Host test for PackFormat.h (.k2pack v1 reader; spec patch_manager_mods/k1_port/K2PACK_FORMAT.md).
// A small in-test writer builds valid packs; mutations (with the CRC recomputed so the deeper rule is reached)
// check every Err, and fuzz loops check that no truncation or single-byte flip crashes the reader (ASan/UBSan
// run through scripts/run_tests.py). If env K2PACK_FIXTURE names a file written by
// `k1_port_pack.py fixture OUT`, it is parsed and compared with the values in that module's docstring.
#include "PackFormat.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

int g_fail = 0;
int g_checks = 0;

#define CHECK(c) do { ++g_checks; if (!(c)) { std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)
#define CHECK_ERR(buf, want) do { \
        k2pack::Character ch_; std::vector<k2pack::AssetView> a_; std::vector<k2pack::CellView> c_; \
        k2pack::Err e_ = k2pack::parse((buf).data(), (buf).size(), ch_, a_, c_); \
        ++g_checks; \
        if (e_ != (want)) { std::printf("FAIL %s:%d got %s want %s\n", __FILE__, __LINE__, k2pack::errName(e_), k2pack::errName(want)); ++g_fail; } \
        if (e_ != k2pack::Err::Ok) { CHECK(a_.empty() && c_.empty()); } \
    } while (0)

typedef std::vector<uint8_t> Bytes;

void put16(Bytes& b, size_t off, uint16_t v) { b[off] = v & 0xFF; b[off + 1] = v >> 8; }
void put32(Bytes& b, size_t off, uint32_t v) {
    for (int i = 0; i < 4; ++i) b[off + i] = static_cast<uint8_t>(v >> (8 * i));
}
uint32_t get32(const Bytes& b, size_t off) {
    return b[off] | (b[off + 1] << 8) | (b[off + 2] << 16) | (static_cast<uint32_t>(b[off + 3]) << 24);
}
void putStr(Bytes& b, size_t off, size_t cap, const std::string& s) {
    for (size_t i = 0; i < cap; ++i) b[off + i] = i < s.size() ? static_cast<uint8_t>(s[i]) : 0;
}

struct TAsset { std::string resref; uint16_t type; uint16_t flags; std::string blob; };
struct TCell { std::string table; uint32_t row; int32_t tmpl; std::string column; std::string value; };

constexpr size_t kAssetTab = 240;

Bytes build(const std::vector<TAsset>& assets, const std::vector<TCell>& cells) {
    const size_t cellTab = kAssetTab + 32 * assets.size();
    const size_t tabEnd = cellTab + 120 * cells.size();
    size_t pos = (tabEnd + 7) & ~static_cast<size_t>(7);
    std::vector<size_t> offs;
    for (const TAsset& a : assets) { offs.push_back(pos); pos = (pos + a.blob.size() + 7) & ~static_cast<size_t>(7); }
    Bytes b(pos, 0);
    // header
    b[0] = 'K'; b[1] = '2'; b[2] = 'P'; b[3] = 'K';
    put32(b, 4, 1); put32(b, 8, 32);
    put32(b, 12, static_cast<uint32_t>(assets.size()));
    put32(b, 16, static_cast<uint32_t>(cells.size()));
    put32(b, 20, static_cast<uint32_t>(b.size()));
    // character block @32
    putStr(b, 32, 32, "hero_one");
    putStr(b, 64, 64, "Hero One");
    putStr(b, 128, 32, "hero_tag");
    putStr(b, 160, 32, "hero_utc");
    putStr(b, 192, 32, "hero_dlg");
    put32(b, 224, 900); put32(b, 228, 300); put32(b, 232, 11);
    for (size_t i = 0; i < assets.size(); ++i) {
        size_t e = kAssetTab + 32 * i;
        putStr(b, e, 16, assets[i].resref);
        put16(b, e + 16, assets[i].type);
        put16(b, e + 18, assets[i].flags);
        put32(b, e + 20, static_cast<uint32_t>(offs[i]));
        put32(b, e + 24, static_cast<uint32_t>(assets[i].blob.size()));
        for (size_t k = 0; k < assets[i].blob.size(); ++k) b[offs[i] + k] = static_cast<uint8_t>(assets[i].blob[k]);
    }
    for (size_t i = 0; i < cells.size(); ++i) {
        size_t e = cellTab + 120 * i;
        putStr(b, e, 16, cells[i].table);
        put32(b, e + 16, cells[i].row);
        put32(b, e + 20, static_cast<uint32_t>(cells[i].tmpl));
        putStr(b, e + 24, 32, cells[i].column);
        putStr(b, e + 56, 64, cells[i].value);
    }
    put32(b, 24, k2pack::crc32(b.data() + 32, b.size() - 32));
    return b;
}

Bytes basePack() {
    std::vector<TAsset> a = {
        {"hero_mdl", 2002, 0, std::string("MDLDATA!") + "MDLDATA!" + "xyz"},
        {"hero_sixteen_ab", 2022, 0, "txi"},  // 15 chars
        {"hero_exactly_16", 2027, 0, "utcutc"},
        {"hero_vo", 4, 1, "RIFF"},
    };
    std::vector<TCell> c = {
        {"appearance", 900, 17, "label", "Hero One"},
        {"heads", 300, -1, "heads", "hero_hd"},
        {"appearance_long_", 5, 0, "col", ""},  // 16-char table name, empty value
    };
    return build(a, c);
}

void fixCrc(Bytes& b) { if (b.size() >= 32) put32(b, 24, k2pack::crc32(b.data() + 32, b.size() - 32)); }

// Applies `mut` to a fresh base pack, repairs total_size/crc is NOT needed (sizes unchanged), recomputes crc.
template <class F>
Bytes mutated(F mut) {
    Bytes b = basePack();
    mut(b);
    fixCrc(b);
    return b;
}

constexpr size_t kBlobStart = 240 + 32 * 4 + 120 * 3;  // 728, already a multiple of 8
constexpr size_t kCellTab = 240 + 32 * 4;

void testValid() {
    Bytes b = basePack();
    k2pack::Character ch; std::vector<k2pack::AssetView> a; std::vector<k2pack::CellView> c;
    CHECK(k2pack::parse(b.data(), b.size(), ch, a, c) == k2pack::Err::Ok);
    CHECK(std::string(ch.id) == "hero_one");
    CHECK(std::string(ch.display_name) == "Hero One");
    CHECK(std::string(ch.default_tag) == "hero_tag");
    CHECK(std::string(ch.default_utc) == "hero_utc");
    CHECK(std::string(ch.default_dlg) == "hero_dlg");
    CHECK(ch.appearance_id == 900 && ch.head_id == 300 && ch.soundset_id == 11);
    CHECK(a.size() == 4 && c.size() == 3);
    CHECK(std::string(a[0].resref) == "hero_mdl" && a[0].res_type == 2002 && a[0].flags == 0 && a[0].size == 19);
    CHECK(a[0].offset == kBlobStart && (a[0].offset & 7) == 0);
    CHECK(std::string(a[2].resref) == "hero_exactly_16");
    CHECK(a[3].flags == 1 && a[3].res_type == 4);
    CHECK(memcmp(b.data() + a[3].offset, "RIFF", 4) == 0);
    CHECK(std::string(c[0].table) == "appearance" && c[0].row == 900 && c[0].template_row == 17);
    CHECK(std::string(c[0].column) == "label" && std::string(c[0].value) == "Hero One");
    CHECK(c[1].template_row == -1);
    CHECK(std::string(c[2].table) == "appearance_long_" && std::string(c[2].value).empty());

    // a full 16-char resref and table (no NUL) are accepted
    std::vector<TAsset> a16 = {{"abcdefghijklmnop", 2002, 0, "x"}};
    std::vector<TCell> c16 = {{"abcdefghijklmnop", 1, -1, "c", "v"}};
    Bytes b16 = build(a16, c16);
    CHECK(k2pack::parse(b16.data(), b16.size(), ch, a, c) == k2pack::Err::Ok);
    CHECK(std::string(a[0].resref) == "abcdefghijklmnop" && std::string(c[0].table) == "abcdefghijklmnop");

    // zero cells, empty default fields
    Bytes b0 = build({{"only", 3007, 0, "tpc"}}, {});
    putStr(b0, 128, 32, ""); putStr(b0, 160, 32, ""); putStr(b0, 192, 32, ""); fixCrc(b0);
    CHECK(k2pack::parse(b0.data(), b0.size(), ch, a, c) == k2pack::Err::Ok);
    CHECK(c.empty() && ch.default_utc[0] == 0 && ch.default_dlg[0] == 0);

    // every allowed res_type
    const uint16_t types[] = {2002, 3008, 3007, 2022, 2027, 2029, 2010, 2060, 3004, 4};
    for (uint16_t t : types) {
        Bytes bt = build({{"t", t, 0, "d"}}, {});
        CHECK_ERR(bt, k2pack::Err::Ok);
    }

    // pool-limit packs: 192 assets and 256 cells fit
    std::vector<TAsset> big; std::vector<TCell> bigc;
    for (int i = 0; i < 192; ++i) big.push_back({"a" + std::to_string(i), 2002, 0, "d"});
    for (int i = 0; i < 256; ++i) bigc.push_back({"tab", static_cast<uint32_t>(i), -1, "col", "v"});
    Bytes bb = build(big, bigc);
    CHECK_ERR(bb, k2pack::Err::Ok);
    big.push_back({"a192", 2002, 0, "d"});
    Bytes bb2 = build(big, bigc);
    CHECK_ERR(bb2, k2pack::Err::BadAssetCount);
    big.pop_back();
    bigc.push_back({"tab", 256, -1, "col", "v"});
    Bytes bb3 = build(big, bigc);
    CHECK_ERR(bb3, k2pack::Err::BadCellCount);
}

void testHeader() {
    CHECK_ERR(Bytes(), k2pack::Err::TooShort);
    {
        k2pack::Character ch; std::vector<k2pack::AssetView> a; std::vector<k2pack::CellView> c;
        CHECK(k2pack::parse(nullptr, 100, ch, a, c) == k2pack::Err::TooShort);
        CHECK(k2pack::parse(nullptr, 0, ch, a, c) == k2pack::Err::TooShort);
    }
    CHECK_ERR(Bytes(239, 0), k2pack::Err::TooShort);
    CHECK_ERR(mutated([](Bytes& b) { b[0] = 'X'; }), k2pack::Err::BadMagic);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 4, 2); }), k2pack::Err::BadVersion);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 4, 0); }), k2pack::Err::BadVersion);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 8, 36); }), k2pack::Err::BadHeaderSize);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 12, 0); }), k2pack::Err::BadAssetCount);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 12, 193); }), k2pack::Err::BadAssetCount);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 12, 0xFFFFFFFFu); }), k2pack::Err::BadAssetCount);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 16, 257); }), k2pack::Err::BadCellCount);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 16, 0xFFFFFFFFu); }), k2pack::Err::BadCellCount);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 20, static_cast<uint32_t>(b.size()) + 1); }), k2pack::Err::BadTotalSize);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 20, static_cast<uint32_t>(b.size()) - 1); }), k2pack::Err::BadTotalSize);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 28, 1); }), k2pack::Err::BadReserved);
    {
        Bytes b = basePack();
        put32(b, 24, get32(b, 24) ^ 1u);
        CHECK_ERR(b, k2pack::Err::BadCrc);
        b = basePack();
        b[b.size() - 1] ^= 0x01;  // body flip without crc fix
        CHECK_ERR(b, k2pack::Err::BadCrc);
    }
    // counts that put the tables past total_size (file consistent: total_size == len, crc ok)
    {
        Bytes b = build({{"one", 2002, 0, "d"}}, {});
        b.resize(240 + 32 * 2);  // room for one table entry only, claim 192 assets
        put32(b, 12, 192); put32(b, 20, static_cast<uint32_t>(b.size())); fixCrc(b);
        CHECK_ERR(b, k2pack::Err::TableBounds);
        Bytes c2 = build({{"one", 2002, 0, "d"}}, {});
        c2.resize(240 + 32 + 120 * 2);
        put32(c2, 16, 256); put32(c2, 20, static_cast<uint32_t>(c2.size())); fixCrc(c2);
        CHECK_ERR(c2, k2pack::Err::TableBounds);
    }
    // crc ok test vector: crc32("123456789") == 0xCBF43926
    const uint8_t v[] = {'1','2','3','4','5','6','7','8','9'};
    CHECK(k2pack::crc32(v, 9) == 0xCBF43926u);
    CHECK(k2pack::crc32(v, 0) == 0u);
    CHECK(std::string(k2pack::errName(k2pack::Err::Ok)) == "ok");
}

void testCharacter() {
    using k2pack::Err;
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 32, 32, "Hero"); }), Err::CharId);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 32, 32, ""); }), Err::CharId);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 32, 32, "has-dash"); }), Err::CharId);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 32, 32, std::string(31, 'a')); }), Err::Ok);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 32, 32, std::string(32, 'a')); }), Err::FieldNoNul);
    CHECK_ERR(mutated([](Bytes& b) { b[32 + 31] = 'a'; b[32 + 5] = 'q'; }), Err::FieldPad);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 64, 64, ""); }), Err::DisplayName);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 64, 64, std::string(64, 'x')); }), Err::FieldNoNul);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 64, 64, std::string(63, 'x')); }), Err::Ok);
    CHECK_ERR(mutated([](Bytes& b) { b[64] = 0x01; }), Err::DisplayName);
    CHECK_ERR(mutated([](Bytes& b) { b[64] = 0x7F; }), Err::DisplayName);
    CHECK_ERR(mutated([](Bytes& b) { b[64] = 0x80; }), Err::DisplayName);
    CHECK_ERR(mutated([](Bytes& b) { b[128] = '\t'; }), Err::DefaultTag);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 128, 32, std::string(32, 'x')); }), Err::FieldNoNul);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 160, 32, "UPPER"); }), Err::DefaultUtc);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 160, 32, std::string(17, 'a')); }), Err::DefaultUtc);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 192, 32, "bad name"); }), Err::DefaultDlg);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 192, 32, std::string(32, 'a')); }), Err::FieldNoNul);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 236, 5); }), Err::BadReserved);
}

void testAssets() {
    using k2pack::Err;
    // entry 0 is at offset 240
    CHECK_ERR(mutated([](Bytes& b) { b[240] = 'A'; }), Err::AssetResref);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 240, 16, ""); }), Err::AssetResref);
    CHECK_ERR(mutated([](Bytes& b) { b[240] = '-'; }), Err::AssetResref);
    CHECK_ERR(mutated([](Bytes& b) { b[240 + 12] = 'z'; }), Err::FieldPad);  // byte after the NUL
    CHECK_ERR(mutated([](Bytes& b) { put16(b, 256, 3); }), Err::AssetType);     // tga refused
    CHECK_ERR(mutated([](Bytes& b) { put16(b, 256, 2017); }), Err::AssetType);  // 2da refused
    CHECK_ERR(mutated([](Bytes& b) { put16(b, 256, 0); }), Err::AssetType);
    CHECK_ERR(mutated([](Bytes& b) { put16(b, 256, 0xFFFF); }), Err::AssetType);
    CHECK_ERR(mutated([](Bytes& b) { put16(b, 258, 2); }), Err::AssetFlags);
    CHECK_ERR(mutated([](Bytes& b) { put16(b, 258, 1); }), Err::AssetFlags);    // stream on mdl
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 264, 0); }), Err::AssetSize);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 260, static_cast<uint32_t>(kBlobStart) + 4); }), Err::AssetAlign);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 260, static_cast<uint32_t>(kBlobStart) - 8); }), Err::AssetRange);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 260, 0); }), Err::AssetRange);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 260, static_cast<uint32_t>(b.size()) & ~7u); put32(b, 264, 64); }), Err::AssetRange);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 264, static_cast<uint32_t>(b.size())); }), Err::AssetRange);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 260, 0xFFFFFFF8u); put32(b, 264, 0x10); }), Err::AssetRange);  // overflow
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 264, 0xFFFFFFFFu); }), Err::AssetRange);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 268, 1); }), Err::BadReserved);
    // overlap: entry 1 (at 272) points inside entry 0's blob (19 bytes @ 728)
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 272 + 20, static_cast<uint32_t>(kBlobStart) + 8); }), Err::AssetOverlap);
    // exact same range
    CHECK_ERR(mutated([](Bytes& b) { put32(b, 272 + 20, static_cast<uint32_t>(kBlobStart)); }), Err::AssetOverlap);
    // adjacent blobs are fine (entry 1 right after entry 0 padded to 8 is the normal layout)
    CHECK_ERR(basePack(), Err::Ok);
    // duplicate (flags, res_type, resref)
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 272, 16, "hero_mdl"); put16(b, 272 + 16, 2002); }), Err::AssetDup);
    // same resref, different type is fine
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, 272, 16, "hero_mdl"); }), Err::Ok);
}

void testCells() {
    using k2pack::Err;
    CHECK_ERR(mutated([](Bytes& b) { b[kCellTab] = 'A'; }), Err::CellTable);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab, 16, ""); }), Err::CellTable);
    CHECK_ERR(mutated([](Bytes& b) { b[kCellTab + 12] = 'z'; }), Err::FieldPad);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, kCellTab + 20, 0xFFFFFFFEu); }), Err::CellTemplate);  // -2
    CHECK_ERR(mutated([](Bytes& b) { put32(b, kCellTab + 20, 0x80000000u); }), Err::CellTemplate);
    CHECK_ERR(mutated([](Bytes& b) { put32(b, kCellTab + 20, 0xFFFFFFFFu); }), Err::Ok);  // -1
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab + 24, 32, ""); }), Err::CellColumn);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab + 24, 32, "a b"); }), Err::CellColumn);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab + 24, 32, " "); }), Err::CellColumn);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab + 24, 32, std::string(31, 'c')); }), Err::Ok);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab + 24, 32, std::string(32, 'c')); }), Err::FieldNoNul);
    CHECK_ERR(mutated([](Bytes& b) { b[kCellTab + 24] = 0x01; }), Err::CellColumn);
    CHECK_ERR(mutated([](Bytes& b) { b[kCellTab + 56] = 0x0A; }), Err::CellValue);
    CHECK_ERR(mutated([](Bytes& b) { b[kCellTab + 56] = 0xE9; }), Err::CellValue);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab + 56, 64, std::string(64, 'v')); }), Err::FieldNoNul);
    CHECK_ERR(mutated([](Bytes& b) { putStr(b, kCellTab + 56, 64, std::string(63, 'v')); }), Err::Ok);
    CHECK_ERR(mutated([](Bytes& b) { b[kCellTab + 56 + 40] = 'x'; }), Err::FieldPad);
    // duplicate (table, row, column): copy cell 0 over cell 1
    CHECK_ERR(mutated([](Bytes& b) {
        for (size_t i = 0; i < 120; ++i) b[kCellTab + 120 + i] = b[kCellTab + i];
    }), Err::CellDup);
    // same table/row, different column is fine
    CHECK_ERR(mutated([](Bytes& b) {
        for (size_t i = 0; i < 120; ++i) b[kCellTab + 120 + i] = b[kCellTab + i];
        putStr(b, kCellTab + 120 + 24, 32, "other");
    }), Err::Ok);
}

void testFuzz() {
    const Bytes base = basePack();
    k2pack::Character ch; std::vector<k2pack::AssetView> a; std::vector<k2pack::CellView> c;

    // every truncation: never Ok (total_size no longer matches), never a crash
    for (size_t n = 0; n < base.size(); ++n) {
        Bytes t(base.begin(), base.begin() + static_cast<long>(n));
        k2pack::Err e = k2pack::parse(t.data(), t.size(), ch, a, c);
        CHECK(e != k2pack::Err::Ok);
    }
    // truncation with total_size and crc repaired: must not crash, any result
    for (size_t n = 32; n < base.size(); ++n) {
        Bytes t(base.begin(), base.begin() + static_cast<long>(n));
        put32(t, 20, static_cast<uint32_t>(n));
        fixCrc(t);
        (void)k2pack::parse(t.data(), t.size(), ch, a, c);
    }
    // every single-byte flip: never Ok without a crc repair (except the header counts, see below)
    for (size_t i = 0; i < base.size(); ++i) {
        for (uint8_t x : {uint8_t(0x01), uint8_t(0x80), uint8_t(0xFF)}) {
            Bytes t = base;
            t[i] ^= x;
            k2pack::Err e = k2pack::parse(t.data(), t.size(), ch, a, c);
            // the counts (bytes 12..19) are outside the crc by spec: a smaller count is still a valid pack
            if (i < 12 || i >= 20) CHECK(e != k2pack::Err::Ok);
            if (i >= 32 || (i >= 24 && i < 28)) CHECK(e == k2pack::Err::BadCrc);
            // with the crc repaired the flip reaches the deeper rules: any outcome, no crash
            if (i >= 32) {
                fixCrc(t);
                (void)k2pack::parse(t.data(), t.size(), ch, a, c);
            }
        }
    }
    // pseudo-random bodies behind a valid header and crc
    uint32_t s = 0x12345678u;
    auto rnd = [&]() { s = s * 1664525u + 1013904223u; return s >> 8; };
    int ok = 0;
    for (int it = 0; it < 3000; ++it) {
        size_t len = 240 + (rnd() % 4000);
        Bytes t(len);
        for (auto& v : t) v = static_cast<uint8_t>(rnd());
        if (it % 3 == 0) { for (size_t i = 240; i < len; ++i) if (rnd() % 4) t[i] = 0; }
        t[0] = 'K'; t[1] = '2'; t[2] = 'P'; t[3] = 'K';
        put32(t, 4, 1); put32(t, 8, 32);
        put32(t, 12, 1 + rnd() % 192); put32(t, 16, rnd() % 257);
        put32(t, 20, static_cast<uint32_t>(len)); put32(t, 28, 0);
        fixCrc(t);
        if (k2pack::parse(t.data(), t.size(), ch, a, c) == k2pack::Err::Ok) ++ok;
    }
    CHECK(ok == 0);
}

void testFixture() {
    const char* path = std::getenv("K2PACK_FIXTURE");
    if (!path || !*path) { std::printf("fixture: K2PACK_FIXTURE not set, skipped\n"); return; }
    FILE* f = std::fopen(path, "rb");
    if (!f) { std::printf("fixture: %s not readable, skipped\n", path); return; }
    Bytes b;
    uint8_t tmp[4096];
    size_t n;
    while ((n = std::fread(tmp, 1, sizeof(tmp), f)) > 0) b.insert(b.end(), tmp, tmp + n);
    std::fclose(f);
    k2pack::Character ch; std::vector<k2pack::AssetView> a; std::vector<k2pack::CellView> c;
    k2pack::Err e = k2pack::parse(b.data(), b.size(), ch, a, c);
    if (e != k2pack::Err::Ok) std::printf("fixture parse: %s\n", k2pack::errName(e));
    CHECK(e == k2pack::Err::Ok);
    if (e != k2pack::Err::Ok) return;
    CHECK(b.size() == 671);
    CHECK(std::string(ch.id) == "fixture" && std::string(ch.display_name) == "Fixture");
    CHECK(std::string(ch.default_tag) == "fixture_tag" && std::string(ch.default_utc) == "fixture_utc");
    CHECK(std::string(ch.default_dlg).empty());
    CHECK(ch.appearance_id == 900 && ch.head_id == 300 && ch.soundset_id == 11);
    CHECK(a.size() == 4 && c.size() == 2);
    if (a.size() != 4 || c.size() != 2) return;
    struct Exp { const char* resref; uint16_t type, flags; uint32_t off, size; const char* blob; } exp[4] = {
        {"fixture_mdl", 2002, 0, 608, 24, "K2PKTESTK2PKTESTK2PKTEST"},
        {"fixture_sixteen_", 2022, 0, 632, 8, "K2PKTXI\n"},
        {"fixture_utc", 2027, 0, 640, 21, "K2PKUTC:fake-utc-blob"},
        {"fixture_vo", 4, 1, 664, 7, "K2PKWAV"},
    };
    for (int i = 0; i < 4; ++i) {
        CHECK(std::string(a[i].resref) == exp[i].resref);
        CHECK(a[i].res_type == exp[i].type && a[i].flags == exp[i].flags);
        CHECK(a[i].offset == exp[i].off && a[i].size == exp[i].size);
        CHECK(std::string(reinterpret_cast<const char*>(b.data() + a[i].offset), a[i].size) == exp[i].blob);
    }
    CHECK(std::string(c[0].table) == "appearance" && c[0].row == 900 && c[0].template_row == 17);
    CHECK(std::string(c[0].column) == "label" && std::string(c[0].value) == "Fixture");
    CHECK(std::string(c[1].table) == "heads" && c[1].row == 300 && c[1].template_row == -1);
    CHECK(std::string(c[1].column) == "heads" && std::string(c[1].value) == "fixture_hd");
    std::printf("fixture: %s parsed and checked\n", path);
}

}  // namespace

int main() {
    testValid();
    testHeader();
    testCharacter();
    testAssets();
    testCells();
    testFuzz();
    testFixture();
    std::printf("pack_format_test: %s (%d checks, %d failure(s))\n", g_fail ? "FAIL" : "PASS", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
