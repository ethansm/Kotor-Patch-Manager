// diversity_log_test.cpp -- Unit tests for NpcDivLog.h (pure log formatters, counters, sets, budget).
#include "NpcDivLog.h"
#include "check.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace nl = NpcDiversity::log;

namespace {

// GOLDEN: mirrored in scripts/test_npc_div_log_check.py
constexpr const char* kGoldBanner0 = "BANNER v=1.0.0 kpl=1 sha=6A522E71 base=00400000 ts=5f3a1b2c";
constexpr const char* kGoldBannerHook = "BANNER hook=tex addr=0085e601 pre=ok";
constexpr const char* kGoldBannerRows = "BANNER rows app=802-811 n=10 head=192-196 n=5 appcount=816 headcount=249";
constexpr const char* kGoldTex =
    "TEX pd=0a1b2c3d fr=00001234 ret=00abcdef app=805 g=1 tv=1 dm=0 hd=0 ph=2 cur=n_commm01 "
    "h=06c232b2 st=1 slot=09 new=1 sw=1";
constexpr const char* kGoldRmp =
    "RMP this=12345678 pd=0a1b2c3d oid=000003e9 tp=00000010 tl=6 tag=Bastila xy=12.5,-3.5 "
    "xyb=41480000,c0600000 app=805>806 tv=1>2 h=06c232b2 why=ok";
constexpr const char* kGoldTexExt =
    "TEX pd=0a1b2c3d fr=00001234 ret=00abcdef app=805 g=1 tv=1 dm=0 hd=0 ph=2 cur=n_commm01 "
    "h=06c232b2 st=1 slot=09 new=1 sw=1 stype=3 oid=000003e9";
constexpr const char* kGoldRmpExt =
    "RMP this=12345678 pd=0a1b2c3d oid=000003e9 tp=00000010 tl=6 tag=Bastila xy=12.5,-3.5 "
    "xyb=41480000,c0600000 app=805>806 tv=1>2 h=06c232b2 why=ok stype=3";
constexpr const char* kGoldPtr =
    "PTR this=12345678 off=01c p=0a8363d0 hex="
    "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f";
constexpr const char* kGoldCenTex = "CEN k=tex app=805 g=1 tv=1 dm=0 cur=n_commm01";
constexpr const char* kGoldCenRemap = "CEN k=remap app=805 g=1 tv=1 tag=Bastila";
constexpr const char* kGoldStatTex =
    "STAT tex calls=10 nullg=0 unread=1 bcskip=2 nonc=3 comm=4 swap=5 frd=6 retd=7 skiprow=8 "
    "stashhit=9 stashmiss=10";
constexpr const char* kGoldStatRemap =
    "STAT remap calls=11 nullg=0 unread=1 bcskip=2 excl=3 tagun=4 chg=5 nochg=6 oidd=7 skiprow=8 skipg=9 "
    "tvchg=10 tvarmor=11";
constexpr const char* kGoldStatAbi = "STAT abi app=1 g=2 dm=3";
constexpr const char* kGoldStatXy = "STAT xy x=-3.5..12.5 y=0.0..100.5";
constexpr const char* kGoldStatSlot0 =
    "STAT slot g=0 00:3 01:0 02:0 03:0 04:0 05:7 06:0 07:0 08:0 09:0 10:0 11:0";
constexpr const char* kGoldStatApp = "STAT app k=tex 802:4 1000:9";
constexpr const char* kGoldStatRead = "STAT read 1:1 2:2 3:3 4:4 5:5 6:6 7:7 8:8 9:9 10:10";
constexpr const char* kGoldStatCensus = "STAT census keys=17 ovf=0";
constexpr const char* kGoldDsc0 =
    "DSC0 pd=0a1b2c3d hex=000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";
constexpr const char* kGoldCre0 =
    "CRE0 this=12345678 off=000 hex="
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f";
constexpr const char* kGoldCreN40 =
    "CRE0 this=12345678 off=040 hex="
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f";
constexpr const char* kGoldHookBytes = "HOOKBYTES addr=0085e601 want=8b442404 got=e9000000";
constexpr const char* kGoldAbi = "ABI app=820 g=5 dm=2 tv=3 pd=0a1b2c3d where=remap";
constexpr const char* kGoldRead = "READ site=3 addr=0076d200 len=16 caller=0085e601 n=1";
constexpr const char* kGoldDump = "DUMP trigger=file path=npc-diversity.dump";

// Checks one formatter's buffer against its golden line and its return value against the stored length.
void ExpectLine(int ret, const char* got, const char* want) {
    KT_CHECK_EQ(got, want);
    KT_CHECK_EQ(ret, static_cast<int>(std::strlen(want)));
}

// Parses every "<idx>:<val>" token of a STAT line. Header words such as "STAT" or "g=0" are skipped.
int ParseTokens(const char* line, unsigned* idx, unsigned* val, int max) {
    int n = 0;
    const char* p = line;
    while (*p) {
        while (*p == ' ') ++p;
        if (!*p) break;
        unsigned a = 0;
        unsigned b = 0;
        int used = 0;
        if (std::sscanf(p, "%u:%u%n", &a, &b, &used) == 2 && used > 0) {
            if (n < max) {
                idx[n] = a;
                val[n] = b;
            }
            ++n;
            p += used;
        } else {
            while (*p && *p != ' ') ++p;
        }
    }
    return n;
}

void testLogFormatGolden() {
    char b[nl::kLineMax];
    int r = 0;

    r = nl::FormatBanner0(b, sizeof b, "1.0.0", 1, "6A522E71", 0x00400000u, 0x5F3A1B2Cu);
    ExpectLine(r, b, kGoldBanner0);

    r = nl::FormatBannerHook(b, sizeof b, "tex", 0x0085E601u, "ok");
    ExpectLine(r, b, kGoldBannerHook);

    r = nl::FormatBannerRows(b, sizeof b, 802u, 811u, 10u, 192u, 196u, 5u, 816u, 249u);
    ExpectLine(r, b, kGoldBannerRows);

    r = nl::FormatTex(b, sizeof b, 0x0A1B2C3Du, 0x00001234u, 0x00ABCDEFu, 805u, 1u, 1u, 0, 0, 2u, "n_commm01",
                      9u, 0x06C232B2u, 1, 9u, "1", 1);
    ExpectLine(r, b, kGoldTex);

    r = nl::FormatRmp(b, sizeof b, 0x12345678u, 0x0A1B2C3Du, 0x3E9u, 0x10u, 6, "Bastila", 7u, 12.5f, -3.5f,
                      0x41480000u, 0xC0600000u, 805u, 806u, 1u, 2u, 0x06C232B2u, "ok");
    ExpectLine(r, b, kGoldRmp);

    r = nl::FormatTexExt(b, sizeof b, 0x0A1B2C3Du, 0x00001234u, 0x00ABCDEFu, 805u, 1u, 1u, 0, 0, 2u, "n_commm01",
                         9u, 0x06C232B2u, 1, 9u, "1", 1, 3, 0x3E9u);
    ExpectLine(r, b, kGoldTexExt);

    r = nl::FormatRmpExt(b, sizeof b, 0x12345678u, 0x0A1B2C3Du, 0x3E9u, 0x10u, 6, "Bastila", 7u, 12.5f, -3.5f,
                         0x41480000u, 0xC0600000u, 805u, 806u, 1u, 2u, 0x06C232B2u, "ok", 3);
    ExpectLine(r, b, kGoldRmpExt);

    {
        uint8_t pb[64];
        for (int i = 0; i < 64; ++i) pb[i] = static_cast<uint8_t>(i);
        r = nl::FormatPtrN(b, sizeof b, 0x12345678u, 0x1Cu, 0x0A8363D0u, pb);
        ExpectLine(r, b, kGoldPtr);
    }

    r = nl::FormatCenTex(b, sizeof b, 805u, 1u, 1u, 0, "n_commm01", 9u);
    ExpectLine(r, b, kGoldCenTex);

    r = nl::FormatCenRemap(b, sizeof b, 805u, 1u, 1u, "Bastila", 7u);
    ExpectLine(r, b, kGoldCenRemap);

    nl::Counts c;
    c.reset();
    c.texCalls = 10;
    c.texUnread = 1;
    c.texBodySkip = 2;
    c.texNonCommoner = 3;
    c.texCommoner = 4;
    c.texSwap = 5;
    c.frDistinct = 6;
    c.retDistinct = 7;
    c.texSkipRow = 8;
    c.texStashHit = 9;
    c.texStashMiss = 10;
    r = nl::FormatStatTex(b, sizeof b, c);
    ExpectLine(r, b, kGoldStatTex);

    c.reset();
    c.remCalls = 11;
    c.remUnread = 1;
    c.remBodySkip = 2;
    c.remExcluded = 3;
    c.remTagUnread = 4;
    c.remChanged = 5;
    c.remNoChange = 6;
    c.oidDistinct = 7;
    c.remSkipRow = 8;
    c.remSkipGender = 9;
    c.remTvChg = 10;
    c.remTvArmor = 11;
    r = nl::FormatStatRemap(b, sizeof b, c);
    ExpectLine(r, b, kGoldStatRemap);

    c.reset();
    c.abiBadApp = 1;
    c.abiBadGender = 2;
    c.abiBadDesc14 = 3;
    r = nl::FormatStatAbi(b, sizeof b, c);
    ExpectLine(r, b, kGoldStatAbi);

    c.reset();
    c.xMin = -3.5f;
    c.xMax = 12.5f;
    c.yMin = 0.0f;
    c.yMax = 100.5f;
    r = nl::FormatStatXy(b, sizeof b, c);
    ExpectLine(r, b, kGoldStatXy);

    c.reset();
    c.slot[0][0] = 3;
    c.slot[0][5] = 7;
    r = nl::FormatStatSlot(b, sizeof b, c, 0u, 0u);  // returns the next chunk start, not a length
    KT_CHECK_EQ(b, kGoldStatSlot0);
    KT_CHECK_EQ(r, 12);

    c.reset();
    c.appTex[802] = 4;
    c.appTex[1000] = 9;
    r = nl::FormatStatApp(b, sizeof b, c, 0u, 0u);  // -1: this line is the last one
    KT_CHECK_EQ(b, kGoldStatApp);
    KT_CHECK_EQ(r, -1);

    c.reset();
    for (uint32_t i = 1; i <= 10; ++i) c.fail[i] = i;
    r = nl::FormatStatRead(b, sizeof b, c);
    ExpectLine(r, b, kGoldStatRead);

    r = nl::FormatStatCensus(b, sizeof b, 17u, 0u);
    ExpectLine(r, b, kGoldStatCensus);

    uint8_t dsc[32];
    for (uint32_t i = 0; i < 32; ++i) dsc[i] = static_cast<uint8_t>(i);
    r = nl::FormatDsc0(b, sizeof b, 0x0A1B2C3Du, dsc);
    ExpectLine(r, b, kGoldDsc0);

    uint8_t cre[64];
    for (uint32_t i = 0; i < 64; ++i) cre[i] = static_cast<uint8_t>(i);
    r = nl::FormatCre0(b, sizeof b, 0x12345678u, cre);
    ExpectLine(r, b, kGoldCre0);

    r = nl::FormatCreN(b, sizeof b, 0x12345678u, 0x40u, cre);
    ExpectLine(r, b, kGoldCreN40);

    r = nl::FormatHookBytes(b, sizeof b, 0x0085E601u, "8b442404", "e9000000");
    ExpectLine(r, b, kGoldHookBytes);

    r = nl::FormatAbi(b, sizeof b, 820u, 5u, 2, 3u, 0x0A1B2C3Du, "remap");
    ExpectLine(r, b, kGoldAbi);

    r = nl::FormatRead(b, sizeof b, 3u, 0x0076D200u, 16u, 0x0085E601u, 1u);
    ExpectLine(r, b, kGoldRead);

    r = nl::FormatDump(b, sizeof b);
    ExpectLine(r, b, kGoldDump);
}

void testLineTruncationAndCaps() {
    char longTag[300];
    std::memset(longTag, 'x', sizeof longTag);

    // Tag bounded to 32 bytes; huge floats make the line longer than 231, so it must be cut there.
    char b[4096];
    int r = nl::FormatRmp(b, sizeof b, 0x12345678u, 0x0A1B2C3Du, 0x3E9u, 0x10u, 6, longTag, sizeof longTag,
                          3.0e38f, -3.0e38f, 0u, 0u, 805u, 806u, 1u, 2u, 0x06C232B2u, "ok");
    KT_CHECK_EQ(r, 231);
    KT_CHECK_EQ(static_cast<int>(std::strlen(b)), r);
    KT_CHECK(b[r] == '\0');
    KT_CHECK(std::strncmp(b, "RMP this=12345678 ", 18) == 0);

    // Tag bound: exactly 32 'x' after "tag=".
    r = nl::FormatCenRemap(b, sizeof b, 805u, 1u, 1u, longTag, sizeof longTag);
    KT_CHECK_EQ(static_cast<int>(std::strlen(b)), static_cast<int>(std::strlen("CEN k=remap app=805 g=1 tv=1 tag=")) + 32);
    KT_CHECK_EQ(r, static_cast<int>(std::strlen(b)));

    // Small cap: writes cap-1 bytes and NUL.
    r = nl::FormatBanner0(b, 20, "1.0.0", 1, "6A522E71", 0u, 0u);
    KT_CHECK_EQ(r, 19);
    KT_CHECK_EQ(b, "BANNER v=1.0.0 kpl=");

    // CREN window with a small cap: writes cap-1 bytes and NUL, same as CRE0.
    const uint8_t zeroBytes[64] = {};
    r = nl::FormatCreN(b, 20, 0x12345678u, 0x40u, zeroBytes);
    KT_CHECK_EQ(r, 19);
    KT_CHECK_EQ(b, "CRE0 this=12345678 ");

    // cap 0 writes nothing and returns 0.
    b[0] = 'Z';
    KT_CHECK_EQ(nl::FormatDump(b, 0), 0);
    KT_CHECK(b[0] == 'Z');
    b[0] = 'Z';
    KT_CHECK_EQ(nl::FormatCreN(b, 0, 0x12345678u, 0x40u, zeroBytes), 0);
    KT_CHECK(b[0] == 'Z');
}

void testSanitize() {
    char d[64];
    const char src[] = "a b\tc\x7f\xc3\x80z";  // 9 bytes: a, space, b, tab, c, 0x7F, 0xC3, 0x80, z
    KT_CHECK_EQ(nl::SanitizeInto(d, sizeof d, src, 9), 9u);
    KT_CHECK_EQ(d, "a?b?c???z");

    // Output bounded by cap-1.
    KT_CHECK_EQ(nl::SanitizeInto(d, 4, "abcdef", 6), 3u);
    KT_CHECK_EQ(d, "abc");

    // Sanitised inside a formatted line: whitespace and control bytes in the tag never split fields.
    char b[nl::kLineMax];
    const int r = nl::FormatCenRemap(b, sizeof b, 805u, 1u, 1u, "Bas tila\n", 9u);
    ExpectLine(r, b, "CEN k=remap app=805 g=1 tv=1 tag=Bas?tila?");
}

void testCountsReset() {
    nl::Counts c;
    c.texCalls = 5;
    c.slot[1][63] = 9;
    c.appRem[1023] = 2;
    c.remTvChg = 4;
    c.reset();
    KT_CHECK_EQ(c.texCalls, 0u);
    KT_CHECK_EQ(c.slot[1][63], 0u);
    KT_CHECK_EQ(c.appRem[1023], 0u);
    KT_CHECK_EQ(c.remTvChg, 0u);
    KT_CHECK(c.xMin == std::numeric_limits<float>::infinity());
    KT_CHECK(c.xMax == -std::numeric_limits<float>::infinity());
    KT_CHECK(c.yMin == std::numeric_limits<float>::infinity());
    KT_CHECK(c.yMax == -std::numeric_limits<float>::infinity());
}

void testU32Set() {
    nl::U32Set<256> s;
    KT_CHECK(s.insert(5u));
    KT_CHECK(!s.insert(5u));
    KT_CHECK(s.insert(0u));  // key 0 is a normal key
    KT_CHECK(!s.insert(0u));
    KT_CHECK_EQ(s.count, 2u);
    KT_CHECK_EQ(s.ovf, 0u);

    // Full table: 0..7 includes key 0.
    nl::U32Set<8> u;
    for (uint32_t k = 0; k < 8; ++k) KT_CHECK(u.insert(k));
    KT_CHECK_EQ(u.count, 8u);
    KT_CHECK(!u.insert(0u));  // present and full: found, not an overflow
    KT_CHECK_EQ(u.ovf, 0u);
    KT_CHECK(!u.insert(100u));  // absent and full: counted, returns without looping
    KT_CHECK_EQ(u.ovf, 1u);
    KT_CHECK(!u.insert(100u));
    KT_CHECK_EQ(u.ovf, 2u);
    KT_CHECK_EQ(u.count, 8u);

    // 256 distinct keys (odd multiplier, so all distinct), then one overflow.
    nl::U32Set<256> big;
    for (uint32_t k = 0; k < 256; ++k) KT_CHECK(big.insert(k * 0x9E3779B9u));
    KT_CHECK_EQ(big.count, 256u);
    KT_CHECK(!big.insert(0xFFFFFFFFu));
    KT_CHECK_EQ(big.ovf, 1u);

    // Reset clears keys and counters.
    big.reset();
    KT_CHECK_EQ(big.count, 0u);
    KT_CHECK_EQ(big.ovf, 0u);
    KT_CHECK(big.insert(0u));
}

void testBudget() {
    // First 300 calls always pass. After that, once per 500 calls: 800, 1300, 1800 within 2000 calls.
    nl::Budget b;
    uint32_t trues = 0;
    uint32_t mismatches = 0;
    for (uint32_t k = 1; k <= 2000; ++k) {
        const bool t = b.take(300, 500);
        const bool want = (k <= 300u) || ((k - 300u) % 500u == 0u);
        if (t != want) ++mismatches;
        if (t) ++trues;
    }
    KT_CHECK_EQ(mismatches, 0u);
    KT_CHECK_EQ(trues, 303u);

    nl::Budget first;
    uint32_t firstTrues = 0;
    for (uint32_t k = 1; k <= 300; ++k) firstTrues += first.take(300, 500) ? 1u : 0u;
    KT_CHECK_EQ(firstTrues, 300u);
    KT_CHECK(!first.take(300, 500));  // call 301
}

void testStatSlotChunking() {
    nl::Counts c;
    c.reset();
    for (int g = 0; g < 2; ++g) {
        for (uint32_t i = 0; i < nl::kSlotCount; ++i) c.slot[g][i] = static_cast<uint32_t>(g * 100) + i;
    }

    for (uint32_t g = 0; g < 2; ++g) {
        bool seen[nl::kSlotCount] = {};
        int lines = 0;
        int start = 0;
        for (;;) {
            char b[nl::kLineMax];
            const int nxt = nl::FormatStatSlot(b, sizeof b, c, g, static_cast<uint32_t>(start));
            ++lines;
            unsigned idx[16];
            unsigned val[16];
            const int n = ParseTokens(b, idx, val, 16);
            KT_CHECK(n <= 12);
            for (int i = 0; i < n && i < 12; ++i) {
                const bool inRange = idx[i] < nl::kSlotCount;
                KT_CHECK(inRange);
                if (!inRange) continue;
                KT_CHECK(!seen[idx[i]]);
                seen[idx[i]] = true;
                KT_CHECK_EQ(val[i], static_cast<unsigned>(g * 100u) + idx[i]);
            }
            if (nxt < 0) break;
            start = nxt;
        }
        KT_CHECK_EQ(lines, 6);
        int covered = 0;
        for (uint32_t i = 0; i < nl::kSlotCount; ++i) covered += seen[i] ? 1 : 0;
        KT_CHECK_EQ(covered, 64);
    }

    // Gender out of range writes nothing.
    char b[nl::kLineMax];
    b[0] = 'Z';
    KT_CHECK_EQ(nl::FormatStatSlot(b, sizeof b, c, 2u, 0u), -1);
    KT_CHECK(b[0] == '\0');
}

void testStatAppChunking() {
    nl::Counts c;
    c.reset();
    for (uint32_t i = 900; i <= 914; ++i) c.appRem[i] = 1u;

    // No nonzero tex entry: nothing written, -1.
    char b[nl::kLineMax];
    KT_CHECK_EQ(nl::FormatStatApp(b, sizeof b, c, 0u, 0u), -1);
    KT_CHECK(b[0] == '\0');

    // 15 nonzero remap entries: two lines (12 + 3), each entry exactly once.
    bool seen[nl::kAppCount] = {};
    int lines = 0;
    int start = 0;
    for (;;) {
        const int nxt = nl::FormatStatApp(b, sizeof b, c, 1u, static_cast<uint32_t>(start));
        if (b[0] == '\0') break;
        ++lines;
        unsigned idx[16];
        unsigned val[16];
        const int n = ParseTokens(b, idx, val, 16);
        KT_CHECK(n <= 12);
        for (int i = 0; i < n && i < 12; ++i) {
            const bool inRange = idx[i] < nl::kAppCount;
            KT_CHECK(inRange);
            if (!inRange) continue;
            KT_CHECK(!seen[idx[i]]);
            seen[idx[i]] = true;
            KT_CHECK_EQ(val[i], 1u);
        }
        if (nxt < 0) break;
        start = nxt;
    }
    KT_CHECK_EQ(lines, 2);
    int covered = 0;
    for (uint32_t i = 900; i <= 914; ++i) covered += seen[i] ? 1 : 0;
    KT_CHECK_EQ(covered, 15);
}

}  // namespace

int main() {
    testLogFormatGolden();
    testLineTruncationAndCaps();
    testSanitize();
    testCountsReset();
    testU32Set();
    testBudget();
    testStatSlotChunking();
    testStatAppChunking();
    return kt::Report("diversity_log_test");
}
