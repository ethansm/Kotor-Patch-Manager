// Regression tests for BuffHudLogic.h (bug F7: unclamped `len += snprintf(buf + len, cap - len, ...)` idiom).
#include "BuffHudLogic.h"
#include "check.h"
#include <climits>
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
        len = bh::appendf(b, g.cap, len, " [%d id=%d]", 3, 42);
        len = bh::appendf(b, g.cap, len, " %02x", 0xab);
        KT_CHECK_EQ(b, " [3 id=42] ab");
        KT_CHECK_EQ(len, 13);
        KT_CHECK_EQ((int)strlen(b), len);
        KT_CHECK(g.canaryIntact());
    }

    // 2. Truncation: strlen == cap-1, return == cap-1, canary untouched.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = bh::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)strlen(b), 15);
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 3. Full buffer: repeated calls are no-ops, len stable, still terminated, no overrun.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = bh::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        for (int i = 0; i < 5; ++i) {
            int l2 = bh::appendf(b, g.cap, len, " more %d", i);
            KT_CHECK_EQ(l2, len);
            len = l2;
        }
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 3b. A caller that passes a len already past the end (the old buggy state) is clamped.
    {
        Guarded g(16); char* b = g.buf(); memset(b, 'x', 16);
        int len = bh::appendf(b, g.cap, 276, " boom");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)b[15], 0);
        KT_CHECK(g.canaryIntact());
    }

    // 4. cap = 1: buffer stays "" and the return is 0; cap <= 0 is a pure no-op.
    {
        Guarded g(1); char* b = g.buf(); b[0] = 0;
        int len = bh::appendf(b, g.cap, 0, "%s", "abc");
        KT_CHECK_EQ(len, 0);
        KT_CHECK_EQ(b, "");
        KT_CHECK(g.canaryIntact());
        char c = 'q';
        KT_CHECK_EQ(bh::appendf(&c, 0, 0, "zzz"), 0);
        KT_CHECK_EQ(bh::appendf(&c, -5, 3, "zzz"), 3);
        KT_CHECK_EQ((int)c, (int)'q');
    }

    // 5. Format error / empty output keeps the NUL and the length.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0;
        int len = bh::appendf(b, g.cap, 0, "%s", "ab");
        len = bh::appendf(b, g.cap, len, "%s", "");
        KT_CHECK_EQ(len, 2);
        KT_CHECK_EQ(b, "ab");
    }

    // 6. The ICONCACHE scenario: char ent[256], six maximum-width entries (k=5, id=-32768, f14=f1c=INT_MIN).
    {
        const int cap = 256;
        std::string full;
        for (int k = 0; k < 6; ++k) {
            char t[96]; snprintf(t, sizeof t, kIconFmt, 5, (short)-32768, INT_MIN, INT_MIN);
            full += t;
        }
        KT_CHECK(full.size() > (size_t)cap);   // the scenario really exceeds the buffer (~276 chars)

        Guarded g(cap); char* b = g.buf(); b[0] = 0; int len = 0;
        for (int k = 0; k < 6; ++k) {
            short id = -32768; int f14 = INT_MIN, f1c = INT_MIN;
            len = bh::appendf(b, cap, len, kIconFmt, 5, id, f14, f1c);
        }
        KT_CHECK_EQ(len, cap - 1);
        KT_CHECK_EQ((int)strlen(b), cap - 1);
        KT_CHECK_EQ(b, full.substr(0, cap - 1).c_str());   // truncated prefix is byte-identical to the untruncated text
        // Extra writes after truncation (what a longer loop would do) must still be harmless.
        for (int k = 0; k < 4; ++k) len = bh::appendf(b, cap, len, kIconFmt, 5, 1, 2, 3);
        KT_CHECK_EQ(len, cap - 1);
        KT_CHECK_EQ((int)strlen(b), cap - 1);
        KT_CHECK(g.canaryIntact());
    }

    // 7. Entries that fit are reproduced byte-for-byte (same as the old idiom when nothing truncates).
    {
        Guarded g(256); char* b = g.buf(); b[0] = 0; int len = 0;
        std::string want;
        for (int k = 0; k < 3; ++k) {
            len = bh::appendf(b, 256, len, kIconFmt, k, 100 + k, 7, -1);
            char t[96]; snprintf(t, sizeof t, kIconFmt, k, 100 + k, 7, -1); want += t;
        }
        KT_CHECK_EQ(b, want.c_str());
        KT_CHECK_EQ(len, (int)want.size());
    }

    return kt::Report("buff_hud_logic");
}
