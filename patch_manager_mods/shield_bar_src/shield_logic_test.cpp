// Regression tests for ShieldLogic.h sb::appendf: a port of buff_hud_logic_test.cpp (F7) + a check that sb::appendf and
// sb::appendf (its original) give identical results, so the two copies cannot drift.
#include "ShieldLogic.h"
#include "K2Addr.h"
#include "../buff_duration_hud_src/BuffHudLogic.h"
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

// Address-table gates (doc 20 B2): every shield-bar feature runs only when the K2AddrTable has the fields it needs.
void testTableGates() {
    KT_CHECK(sb::slotFieldsOk(&kSteam)); KT_CHECK(sb::vitRectOk(&kSteam)); KT_CHECK(sb::menuFieldsOk(&kSteam));
    KT_CHECK(sb::vitalsFieldsOk(&kSteam)); KT_CHECK(sb::tooltipFieldsOk(&kSteam)); KT_CHECK(sb::drawFieldsOk(&kSteam));
    KT_CHECK(sb::effectProbeFieldsOk(&kSteam));
    // GOG column is filled from doc 21b (static-verified): every feature available
    KT_CHECK(sb::slotFieldsOk(&kGog)); KT_CHECK(sb::vitRectOk(&kGog)); KT_CHECK(sb::menuFieldsOk(&kGog));
    KT_CHECK(sb::vitalsFieldsOk(&kGog)); KT_CHECK(sb::tooltipFieldsOk(&kGog)); KT_CHECK(sb::drawFieldsOk(&kGog));
    KT_CHECK(sb::effectProbeFieldsOk(&kGog));
    // a table that knows nothing: every feature off
    static const K2AddrTable kEmpty{};
    KT_CHECK(!sb::slotFieldsOk(&kEmpty)); KT_CHECK(!sb::vitRectOk(&kEmpty)); KT_CHECK(!sb::menuFieldsOk(&kEmpty));
    KT_CHECK(!sb::vitalsFieldsOk(&kEmpty)); KT_CHECK(!sb::tooltipFieldsOk(&kEmpty)); KT_CHECK(!sb::drawFieldsOk(&kEmpty));
    KT_CHECK(!sb::effectProbeFieldsOk(&kEmpty));
    const K2AddrTable* none = nullptr;
    KT_CHECK(!sb::slotFieldsOk(none)); KT_CHECK(!sb::vitRectOk(none)); KT_CHECK(!sb::menuFieldsOk(none));
    KT_CHECK(!sb::vitalsFieldsOk(none)); KT_CHECK(!sb::tooltipFieldsOk(none)); KT_CHECK(!sb::drawFieldsOk(none));
    KT_CHECK(!sb::effectProbeFieldsOk(none));
    K2AddrTable t;
#define ZERO_BREAKS(gate, field) t = kSteam; t.field = 0; KT_CHECK(!gate(&t))
    ZERO_BREAKS(sb::slotFieldsOk, Slot_CardRect); ZERO_BREAKS(sb::slotFieldsOk, Slot_Flags); ZERO_BREAKS(sb::slotFieldsOk, Slot_VitCtl);
    ZERO_BREAKS(sb::slotFieldsOk, Slot_VitRect); ZERO_BREAKS(sb::slotFieldsOk, Slot_ForceCtl); ZERO_BREAKS(sb::slotFieldsOk, Slot_ForceRect);
    ZERO_BREAKS(sb::slotFieldsOk, Ctl_VtSetRect);
    ZERO_BREAKS(sb::vitRectOk, Slot_VitRect);
    ZERO_BREAKS(sb::menuFieldsOk, Ctl_Flags); ZERO_BREAKS(sb::menuFieldsOk, Ctl_X); ZERO_BREAKS(sb::menuFieldsOk, Ctl_Y);
    ZERO_BREAKS(sb::menuFieldsOk, Ctl_W); ZERO_BREAKS(sb::menuFieldsOk, Ctl_H); ZERO_BREAKS(sb::menuFieldsOk, Ctl_VtSetRect);
    ZERO_BREAKS(sb::menuFieldsOk, Menu_Char2); ZERO_BREAKS(sb::menuFieldsOk, Menu_Level2); ZERO_BREAKS(sb::menuFieldsOk, Menu_Back2);
    ZERO_BREAKS(sb::menuFieldsOk, Menu_Change2); ZERO_BREAKS(sb::menuFieldsOk, Menu_Char3); ZERO_BREAKS(sb::menuFieldsOk, Menu_Level3);
    ZERO_BREAKS(sb::menuFieldsOk, Menu_Back3); ZERO_BREAKS(sb::menuFieldsOk, Menu_Change3);
    ZERO_BREAKS(sb::vitalsFieldsOk, Creature_Stats); ZERO_BREAKS(sb::vitalsFieldsOk, Stats_ForceA); ZERO_BREAKS(sb::vitalsFieldsOk, Stats_ForceB);
    ZERO_BREAKS(sb::vitalsFieldsOk, Creature_VtMaxHp); ZERO_BREAKS(sb::vitalsFieldsOk, Creature_VtCurHp); ZERO_BREAKS(sb::vitalsFieldsOk, GetMaxForcePoints);
    ZERO_BREAKS(sb::tooltipFieldsOk, Tip_Slot); ZERO_BREAKS(sb::tooltipFieldsOk, Tip_Text); ZERO_BREAKS(sb::tooltipFieldsOk, CExoStringFormat);
    ZERO_BREAKS(sb::drawFieldsOk, ImageVt_Draw);
    ZERO_BREAKS(sb::effectProbeFieldsOk, Creature_EffectArray); ZERO_BREAKS(sb::effectProbeFieldsOk, Creature_EffectCount);
    ZERO_BREAKS(sb::effectProbeFieldsOk, Effect_Type); ZERO_BREAKS(sb::effectProbeFieldsOk, Effect_Subtype); ZERO_BREAKS(sb::effectProbeFieldsOk, Effect_SpellId);
    ZERO_BREAKS(sb::effectProbeFieldsOk, Effect_Creator); ZERO_BREAKS(sb::effectProbeFieldsOk, Effect_Duration);
    ZERO_BREAKS(sb::effectProbeFieldsOk, Effect_IntCount); ZERO_BREAKS(sb::effectProbeFieldsOk, Effect_Ints);
#undef ZERO_BREAKS
    k2addr::setForTest(nullptr); KT_CHECK(!sb::vitRectOk(k2addr::get()));
    k2addr::setForTest(&kSteam); KT_CHECK(sb::vitRectOk(k2addr::get()));
    k2addr::clearForTest();
}

// Menu companion bars (user 10-04: bars flush with the frame, companion gap = leader-to-companion gap). Numbers from the 10-04
// 00:48 Steam session log at 3840x1600: companion cards 96x96 at y 1365, leader force rect x 2640 w 100, companion 1 at 2750.
void testMenuBars() {
    sb::MenuBars b = sb::menuBars(2750, 1365, 96, 96);
    KT_CHECK_EQ(b.vx, 2713); KT_CHECK_EQ(b.bw, 49); KT_CHECK_EQ(b.bwF, 59);      // health unchanged vs the live build
    KT_CHECK_EQ(b.by, 1363); KT_CHECK_EQ(b.bh, 100);                             // vertical unchanged: already flush within 0.5 px
    KT_CHECK_EQ(b.fx, 2832);                                                     // force 1 px left (was 2833: ~1 px gap to the frame)
    // every card size: visible art meets the frame within 1 px on both sides and top / bottom
    for (int h = 20; h <= 400; ++h) for (int x = 0; x < 3; ++x) {
        const int w = h, y = 100 + x;
        sb::MenuBars m = sb::menuBars(1000 + x, y, w, h);
        float hr = m.vx + sb::kHealthArtRight / sb::kArtW * m.bw, fl = m.fx + sb::kForceArtLeft / sb::kArtW * m.bwF;
        float top = m.by + sb::kArtTop / sb::kArtH * m.bh, bot = m.by + sb::kArtBottom / sb::kArtH * m.bh;
        KT_CHECK(hr >= 1000 + x && hr < 1000 + x + 1.0f);                       // health edge on / just inside the left frame edge
        KT_CHECK(fl <= 1000 + x + w && fl > 1000 + x + w - 1.0f);               // force edge on / just inside the right frame edge
        KT_CHECK(top > y - 1.0f && top < y + 1.0f);                             // integer by: each art edge within 1 px of the frame edge
        KT_CHECK(bot > y + h - 1.0f && bot < y + h + 1.0f);
    }
    // companion 3 placement: same visible gap as leader force -> companion 1 health
    int c3 = sb::secondCompanionX(2640, 100, 2750, 1365, 96, 96);
    KT_CHECK_EQ(c3, 2920);                                                       // was 2942 (gap 62 px vs 40 px)
    sb::MenuBars b3 = sb::menuBars(c3, 1365, 96, 96);
    float gapLead = sb::healthVisLeft(b.vx, b.bw) - sb::forceVisRight(2640, 100);
    float gapComp = sb::healthVisLeft(b3.vx, b3.bw) - sb::forceVisRight(b.fx, b.bwF);
    KT_CHECK(gapComp - gapLead > -1.0f && gapComp - gapLead < 1.0f);
}

// Companion cards aligned to the leader card bottom (user 10-04 option b). Leader vit rect (2395,1269,100,202), leader card bottom
// 1463 (GUI check), companion cards 96x96 at y 1365 (bottom 1461).
void testCompanionDy() {
    KT_CHECK_EQ(sb::leaderCardBottom(1269, 202), 1463);
    KT_CHECK_EQ(sb::companionDy(1463, 1365, 96), 2);
    KT_CHECK_EQ(sb::companionDy(1463, 1367, 96), 0);                            // already aligned
    KT_CHECK_EQ(sb::companionDy(1463, 1359, 96), 8);  KT_CHECK_EQ(sb::companionDy(1463, 1375, 96), -8);
    KT_CHECK_EQ(sb::companionDy(1463, 1358, 96), 0);  KT_CHECK_EQ(sb::companionDy(1463, 1376, 96), 0);   // |d| > 8: bad read, no move
    KT_CHECK_EQ(sb::companionDy(1463, 0, 96), 0);
    // bars follow the moved card: visible bottom on the card bottom, still above the leader bar's visible bottom
    sb::MenuBars m = sb::menuBars(2750, 1365 + 2, 96, 96);
    float bot = m.by + sb::kArtBottom / sb::kArtH * m.bh;
    float leaderBot = 1269 + sb::kArtBottom / sb::kArtH * 202;
    KT_CHECK(bot > 1463 - 1.0f && bot < 1463 + 1.0f);
    KT_CHECK(bot < leaderBot);
}

}  // namespace

int main() {
    // 1. Fits: exact text, exact length, NUL-terminated.
    {
        Guarded g(64); char* b = g.buf(); b[0] = 0; int len = 0;
        len = sb::appendf(b, g.cap, len, " [%d id=%d]", 3, 42);
        len = sb::appendf(b, g.cap, len, " %02x", 0xab);
        KT_CHECK_EQ(b, " [3 id=42] ab");
        KT_CHECK_EQ(len, 13);
        KT_CHECK_EQ((int)strlen(b), len);
        KT_CHECK(g.canaryIntact());
    }

    // 2. Truncation: strlen == cap-1, return == cap-1, canary untouched.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = sb::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)strlen(b), 15);
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 3. Full buffer: repeated calls are no-ops, len stable, still terminated, no overrun.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = sb::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        for (int i = 0; i < 5; ++i) {
            int l2 = sb::appendf(b, g.cap, len, " more %d", i);
            KT_CHECK_EQ(l2, len);
            len = l2;
        }
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 3b. A caller that passes a len already past the end (the old buggy state) is clamped.
    {
        Guarded g(16); char* b = g.buf(); memset(b, 'x', 16);
        int len = sb::appendf(b, g.cap, 276, " boom");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)b[15], 0);
        KT_CHECK(g.canaryIntact());
    }

    // 4. cap = 1: buffer stays "" and the return is 0; cap <= 0 is a pure no-op.
    {
        Guarded g(1); char* b = g.buf(); b[0] = 0;
        int len = sb::appendf(b, g.cap, 0, "%s", "abc");
        KT_CHECK_EQ(len, 0);
        KT_CHECK_EQ(b, "");
        KT_CHECK(g.canaryIntact());
        char c = 'q';
        KT_CHECK_EQ(sb::appendf(&c, 0, 0, "zzz"), 0);
        KT_CHECK_EQ(sb::appendf(&c, -5, 3, "zzz"), 3);
        KT_CHECK_EQ((int)c, (int)'q');
    }

    // 5. Format error / empty output keeps the NUL and the length.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0;
        int len = sb::appendf(b, g.cap, 0, "%s", "ab");
        len = sb::appendf(b, g.cap, len, "%s", "");
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
            len = sb::appendf(b, cap, len, kIconFmt, 5, id, f14, f1c);
        }
        KT_CHECK_EQ(len, cap - 1);
        KT_CHECK_EQ((int)strlen(b), cap - 1);
        KT_CHECK_EQ(b, full.substr(0, cap - 1).c_str());   // truncated prefix is byte-identical to the untruncated text
        // Extra writes after truncation (what a longer loop would do) must still be harmless.
        for (int k = 0; k < 4; ++k) len = sb::appendf(b, cap, len, kIconFmt, 5, 1, 2, 3);
        KT_CHECK_EQ(len, cap - 1);
        KT_CHECK_EQ((int)strlen(b), cap - 1);
        KT_CHECK(g.canaryIntact());
    }

    // 7. Entries that fit are reproduced byte-for-byte (same as the old idiom when nothing truncates).
    {
        Guarded g(256); char* b = g.buf(); b[0] = 0; int len = 0;
        std::string want;
        for (int k = 0; k < 3; ++k) {
            len = sb::appendf(b, 256, len, kIconFmt, k, 100 + k, 7, -1);
            char t[96]; snprintf(t, sizeof t, kIconFmt, k, 100 + k, 7, -1); want += t;
        }
        KT_CHECK_EQ(b, want.c_str());
        KT_CHECK_EQ(len, (int)want.size());
    }

    // 8. Drift guard: identical buffers and lengths from sb::appendf and bh::appendf over caps, start lengths and payloads.
    {
        const char* payloads[] = { "", "x", "Shield: ", "0123456789abcdefghij", "%", " +a0: 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f" };
        int diffs = 0, cases = 0;
        for (int cap = -1; cap <= 40; ++cap)
            for (int start = -2; start <= 45; start += 3)
                for (const char* pl : payloads) {
                    char a[64], b[64]; memset(a, 0x5A, sizeof a); memset(b, 0x5A, sizeof b);
                    if (cap > 0) { int fill = start < 0 ? 0 : (start < cap ? start : cap - 1); memset(a, 'p', fill); memset(b, 'p', fill); a[fill] = b[fill] = 0; }
                    int la = sb::appendf(a, cap, start, "%s|%d", pl, cap);
                    int lb = bh::appendf(b, cap, start, "%s|%d", pl, cap);
                    ++cases; if (la != lb || memcmp(a, b, sizeof a) != 0) ++diffs;
                }
        KT_CHECK(cases > 3000);
        KT_CHECK_EQ(diffs, 0);
    }

    testTableGates();
    testMenuBars();
    testCompanionDy();

    return kt::Report("shield_logic");
}
