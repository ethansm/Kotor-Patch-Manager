// Host test of StrFmt.h strfmt::appendf (finding F26): the bounded printf-append cases ported from buff_hud_logic_test.cpp (F7)
// plus a drift guard against the two original copies (bh::appendf, sb::appendf), so the extraction cannot change behaviour.
#include "StrFmt.h"
#include "check.h"
#include "../buff_duration_hud_src/BuffHudLogic.h"
#include "../shield_bar_src/ShieldLogic.h"
#include <climits>
#include <string.h>
#include <string>

namespace {

// ICONCACHE entry format, copied verbatim from BuffDurationHUD.cpp.
const char* kIconFmt = " [%d id=%d f14=%d f1c=%d]";

// Backing store bigger than the logical buffer; bytes past `cap` are canaries that must never change.
struct Guarded {
    static constexpr int Pad = 128;
    unsigned char raw[1024 + Pad];
    int cap;
    explicit Guarded(int c) : cap(c) { memset(raw, 0xAA, sizeof raw); }
    char* buf() { return (char*)raw; }
    bool canaryIntact() const {
        for (int i = cap; i < cap + Pad; ++i) if (raw[i] != 0xAA) return false;
        return true;
    }
};

}  // namespace

int main() {
    // 1. Fits: exact text, exact length, NUL-terminated.
    {
        Guarded g(64); char* b = g.buf(); b[0] = 0; int len = 0;
        len = strfmt::appendf(b, g.cap, len, " [%d id=%d]", 3, 42);
        len = strfmt::appendf(b, g.cap, len, " %02x", 0xab);
        KT_CHECK_EQ(b, " [3 id=42] ab");
        KT_CHECK_EQ(len, 13);
        KT_CHECK_EQ((int)strlen(b), len);
        KT_CHECK(g.canaryIntact());
    }

    // 2. Truncation: strlen == cap-1, return == cap-1, canary untouched.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = strfmt::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)strlen(b), 15);
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 2b. Exact fit: a payload of cap-1 chars is not truncated (n == cap-len-1 is the last non-truncating size), cap chars is.
    {
        Guarded g(8); char* b = g.buf(); b[0] = 0;
        int len = strfmt::appendf(b, g.cap, 0, "%s", "abcdefg");
        KT_CHECK_EQ(len, 7); KT_CHECK_EQ(b, "abcdefg"); KT_CHECK(g.canaryIntact());
        Guarded h(8); char* c = h.buf(); c[0] = 0;
        len = strfmt::appendf(c, h.cap, 0, "%s", "abcdefgh");
        KT_CHECK_EQ(len, 7); KT_CHECK_EQ(c, "abcdefg"); KT_CHECK(h.canaryIntact());
        // appended at a non-zero start: 3 + 4 chars fits exactly, 3 + 5 truncates
        Guarded k(8); char* d = k.buf(); strcpy(d, "abc");
        len = strfmt::appendf(d, k.cap, 3, "%s", "wxyz");
        KT_CHECK_EQ(len, 7); KT_CHECK_EQ(d, "abcwxyz"); KT_CHECK(k.canaryIntact());
        Guarded m(8); char* e = m.buf(); strcpy(e, "abc");
        len = strfmt::appendf(e, m.cap, 3, "%s", "wxyz!");
        KT_CHECK_EQ(len, 7); KT_CHECK_EQ(e, "abcwxyz"); KT_CHECK(m.canaryIntact());
    }

    // 3. Full buffer: repeated calls are no-ops, len stable, still terminated, no overrun.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = strfmt::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        for (int i = 0; i < 5; ++i) {
            int l2 = strfmt::appendf(b, g.cap, len, " more %d", i);
            KT_CHECK_EQ(l2, len);
            len = l2;
        }
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 3b. A caller that passes a len already past the end (the old buggy state) is clamped; len == cap-1 is already full.
    {
        Guarded g(16); char* b = g.buf(); memset(b, 'x', 16);
        int len = strfmt::appendf(b, g.cap, 276, " boom");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)b[15], 0);
        KT_CHECK(g.canaryIntact());
        Guarded h(16); char* c = h.buf(); memset(c, 'x', 16);
        len = strfmt::appendf(c, h.cap, 15, "%s", "Z");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)c[15], 0);
        KT_CHECK_EQ((int)c[14], (int)'x');
        KT_CHECK(h.canaryIntact());
    }

    // 3b2. len == cap: still terminated at cap-1 (vsnprintf with size 0 would leave it untouched).
    {
        Guarded g(16); char* b = g.buf(); memset(b, 'x', 16);
        int len = strfmt::appendf(b, g.cap, 16, "%s", "Z");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)b[15], 0);
        KT_CHECK_EQ((int)b[14], (int)'x');
        KT_CHECK(g.canaryIntact());
    }

    // 3c. Negative len is treated as 0 (overwrites from the start).
    {
        Guarded g(16); char* b = g.buf(); memset(b, 'x', 16);
        int len = strfmt::appendf(b, g.cap, -7, "%s", "hi");
        KT_CHECK_EQ(len, 2);
        KT_CHECK_EQ(b, "hi");
        KT_CHECK(g.canaryIntact());
    }

    // 4. cap = 1: buffer stays "" and the return is 0; cap <= 0 is a pure no-op.
    {
        Guarded g(1); char* b = g.buf(); b[0] = 0;
        int len = strfmt::appendf(b, g.cap, 0, "%s", "abc");
        KT_CHECK_EQ(len, 0);
        KT_CHECK_EQ(b, "");
        KT_CHECK(g.canaryIntact());
        char c = 'q';
        KT_CHECK_EQ(strfmt::appendf(&c, 0, 0, "zzz"), 0);
        KT_CHECK_EQ(strfmt::appendf(&c, -5, 3, "zzz"), 3);
        KT_CHECK_EQ((int)c, (int)'q');
    }

    // 5. Empty output keeps the NUL and the length.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0;
        int len = strfmt::appendf(b, g.cap, 0, "%s", "ab");
        len = strfmt::appendf(b, g.cap, len, "%s", "");
        KT_CHECK_EQ(len, 2);
        KT_CHECK_EQ(b, "ab");
    }

    // 5b. Encoding error (vsnprintf < 0): keep length, stay terminated at buf[len]. glibc: %ls with an invalid wide char -> EILSEQ.
    {
        Guarded g(32); char* b = g.buf(); b[0] = 0;
        int len = strfmt::appendf(b, g.cap, 0, "%s", "ab");
        const wchar_t bad[] = { (wchar_t)0x7FFFFFFF, 0 };
        char probe[8];
        if (snprintf(probe, sizeof probe, "%ls", bad) < 0) {
            memset(b + 2, 'x', 8);
            len = strfmt::appendf(b, g.cap, len, "%ls", bad);
            KT_CHECK_EQ(len, 2);
            KT_CHECK_EQ(b, "ab");
            KT_CHECK(g.canaryIntact());
        } else {
            KT_CHECK(false);   // libc did not report an encoding error; the n < 0 branch would go untested
        }
    }

    // 6. The ICONCACHE scenario: char ent[256], six maximum-width entries (k=5, id=-32768, f14=f1c=INT_MIN).
    {
        const int cap = 256;
        Guarded g(cap); char* b = g.buf(); b[0] = 0; int len = 0;
        std::string full;
        for (int k = 0; k < 6; ++k) {
            char t[96]; snprintf(t, sizeof t, kIconFmt, 5, (short)-32768, INT_MIN, INT_MIN);
            full += t;
            len = strfmt::appendf(b, cap, len, kIconFmt, 5, (short)-32768, INT_MIN, INT_MIN);
        }
        std::string want = full.substr(0, cap - 1);
        KT_CHECK_EQ(b, want.c_str());
        KT_CHECK_EQ(len, (int)want.size());
        KT_CHECK(g.canaryIntact());
    }

    // 7. Drift guard: identical buffers and lengths from strfmt::, bh:: and sb:: appendf over caps, start lengths and payloads.
    {
        const char* payloads[] = { "", "x", "Shield: ", "0123456789abcdefghij", "%", " +a0: 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f" };
        int diffs = 0, cases = 0;
        for (int cap = -1; cap <= 40; ++cap)
            for (int start = -2; start <= 45; start += 3)
                for (const char* pl : payloads) {
                    char a[64], b[64], c[64]; memset(a, 0x5A, sizeof a); memset(b, 0x5A, sizeof b); memset(c, 0x5A, sizeof c);
                    if (cap > 0) {
                        int fill = start < 0 ? 0 : (start < cap ? start : cap - 1);
                        memset(a, 'p', fill); memset(b, 'p', fill); memset(c, 'p', fill); a[fill] = b[fill] = c[fill] = 0;
                    }
                    int la = strfmt::appendf(a, cap, start, "%s|%d", pl, cap);
                    int lb = bh::appendf(b, cap, start, "%s|%d", pl, cap);
                    int lc = sb::appendf(c, cap, start, "%s|%d", pl, cap);
                    ++cases;
                    if (la != lb || la != lc || memcmp(a, b, sizeof a) != 0 || memcmp(a, c, sizeof a) != 0) ++diffs;
                }
        KT_CHECK(cases > 3000);
        KT_CHECK_EQ(diffs, 0);
    }

    return kt::Report("strfmt_test");
}
