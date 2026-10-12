// Host test of Effects.h + EffectIcons.h (doc 20 conventions) on fake creatures in a MAP_32BIT arena.
// Effect objects are rebuilt from REAL captured CGameEffect bytes (shield-probe log, see the fixtures below); only the
// +0x34 int-array pointer is re-pointed at an arena copy of the logged ints.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "EffectIcons.h"
#include "check.h"
#include <sys/mman.h>
#include <math.h>
#include <initializer_list>

using namespace k2fx;

// ---- fixtures: captured bytes -------------------------------------------------------------------------------------------
// Source: <KOTOR II dir>/shieldprobe_log.txt (Oct 3 01:41 session), 12-row hex dump (+00..+bf) of each effect plus the
// "arr@" int list (first 8 ints = the effect's int array, count at +0x30 = 8). Rows are kept verbatim from the log.
//
// Energy shield (droid shield, spell 110), line 1467:
//   APPLY ... type=0x0002 sub=0x0009 spell=110 creator=000002fb dur=226.00 ints=[4224,300,50,2048,1,0,0,0]
static const char* const kShieldRows[12] = {
    "+00: 83 0f 00 00 00 00 00 00 02 00 09 00 00 00 62 43", "+10: 92 00 00 00 2b 66 27 00 fb 02 00 00 6e 00 00 00",
    "+20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+30: 08 00 00 00 58 5f b8 07 00 00 00 00 00 00 00 00",
    "+40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
    "+60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+70: 00 00 00 00 00 00 00 00 00 00 00 7f 00 00 00 7f",
    "+80: 00 00 00 7f 00 00 00 7f 01 00 00 00 00 00 ff 00", "+90: 00 00 ff 08 21 1c ff 27 14 00 00 00 19 00 46 81",
    "+a0: 83 0f 00 00 00 00 00 00 6b 00 00 00 00 00 00 00", "+b0: 00 00 00 00 00 00 00 00 fb 02 00 00 6e 00 00 00" };
static const int kShieldInts[8] = { 4224, 300, 50, 2048, 1, 0, 0, 0 };   // arr@07b85f58
// Sibling visual effect, same spell 110, line 1481:
//   APPLY ... type=0x001e sub=0x0001 spell=110 creator=000002fb dur=226.00 ints=[2045,0,0,0,0,0,0,0]
static const char* const kVfxRows[12] = {
    "+00: 83 0f 00 00 00 00 00 00 1e 00 01 00 00 00 62 43", "+10: 92 00 00 00 2b 66 27 00 fb 02 00 00 6e 00 00 00",
    "+20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+30: 08 00 00 00 28 5f b8 07 00 00 00 00 00 00 00 00",
    "+40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
    "+60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+70: 00 00 00 00 00 00 00 00 00 00 00 7f 00 00 00 7f",
    "+80: 00 00 00 7f 00 00 00 7f 01 00 00 00 00 00 00 00", "+90: 00 00 00 00 00 00 00 00 14 00 0c 00 18 00 75 80",
    "+a0: 83 0f 00 00 00 00 00 00 02 00 09 00 00 00 62 43", "+b0: 92 00 00 00 2b 66 27 00 fb 02 00 00 6e 00 00 00" };
static const int kVfxInts[8] = { 2045, 0, 0, 0, 0, 0, 0, 0 };           // arr@07b85f28
// Timed buff (adrenaline stim, spell 69, duration 132 s), line 564:
//   APPLY ... type=0x0024 sub=0x0009 spell=69 creator=7ffffffd dur=132.00 ints=[0,4,0,0,0,0,0,0]
static const char* const kBuffRows[12] = {
    "+00: 86 0f 00 00 00 00 00 00 24 00 09 00 00 00 04 43", "+10: 92 00 00 00 d3 1f 27 00 fd ff ff 7f 45 00 00 00",
    "+20: 01 00 00 00 ff ff ff ff 00 00 00 00 00 00 00 00", "+30: 08 00 00 00 10 a4 23 16 00 00 00 00 00 00 00 00",
    "+40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
    "+60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "+70: 00 00 00 00 00 00 00 00 00 00 00 7f 00 00 00 7f",
    "+80: 00 00 00 7f 00 00 00 7f 00 00 00 00 fc 02 04 04", "+90: 04 40 60 00 00 01 01 ff 14 00 00 00 11 00 46 81",
    "+a0: ff ff fc 02 04 04 04 41 20 00 00 01 01 ff ff ff", "+b0: f8 00 04 1b 00 ff ff ff fc 02 04 04 04 41 70 00" };
static const int kBuffInts[8] = { 0, 4, 0, 0, 0, 0, 0, 0 };             // arr@1623a410

// ---- arena ----------------------------------------------------------------------------------------------------------------
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 18;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// Parse the log's "+xx: aa bb ..." rows into 0xc0 bytes.
static void parseRows(const char* const rows[12], uint8_t out[0xc0]) {
    for (int r = 0; r < 12; ++r) {
        const char* p = rows[r] + 5;
        for (int i = 0; i < 16; ++i, p += 3) out[r * 16 + i] = (uint8_t)strtoul(p, nullptr, 16);
    }
}
// A CGameEffect in the arena from captured bytes (0x100 bytes, tail zero), ints copied to an arena array and +0x34 patched.
static uintptr_t makeEffect(const char* const rows[12], const int ints[8]) {
    uint8_t raw[0xc0]; parseRows(rows, raw);
    uintptr_t e = arenaAlloc(0x100); memcpy(reinterpret_cast<void*>(e), raw, sizeof raw);
    uintptr_t ia = arenaAlloc(8 * sizeof(int)); memcpy(reinterpret_cast<void*>(ia), ints, 8 * sizeof(int));
    at<uint32_t>(e, Effect_Ints) = (uint32_t)ia;
    return e;
}
static uintptr_t shield() { return makeEffect(kShieldRows, kShieldInts); }
static uintptr_t vfx() { return makeEffect(kVfxRows, kVfxInts); }
static uintptr_t buff() { return makeEffect(kBuffRows, kBuffInts); }
static int* intsOf(uintptr_t e) { return reinterpret_cast<int*>((uintptr_t)at<uint32_t>(e, Effect_Ints)); }

// A creature with an effect list in the arena.
static uintptr_t makeCreature(std::initializer_list<uintptr_t> effs) {
    uintptr_t c = arenaAlloc(0x200);
    uintptr_t arr = arenaAlloc((effs.size() + 1) * 4);
    int i = 0; for (uintptr_t e : effs) at<uint32_t>(arr, 4 * i++) = (uint32_t)e;
    at<uint32_t>(c, Creature_EffectArray) = (uint32_t)arr; at<int>(c, Creature_EffectCount) = (int)effs.size();
    return c;
}
static bool near(float a, float b) { return fabsf(a - b) < 1e-5f; }

// ---- fake world time ------------------------------------------------------------------------------------------------------
static int g_fakeDay = 0, g_fakeMs = 0; static uintptr_t g_lastTimer = 0; static int g_timeCalls = 0;
static void fakeGetTime(uintptr_t timer, int* day, int* ms) { ++g_timeCalls; g_lastTimer = timer; *day = g_fakeDay; *ms = g_fakeMs; }

// Captured effect time fields: adrenaline buff expires day 146 ms 0x271fd3 after 132 s.
constexpr int BuffDay = 0x92, BuffMs = 0x271fd3;

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;

    // ---- fixture sanity: the captured bytes decode to what the log header line says ----
    {
        uintptr_t s = shield(), v = vfx(), b = buff();
        KT_CHECK_EQ((long long)at<unsigned short>(s, Effect_Type), (long long)EffType_Pool);
        KT_CHECK_EQ((long long)at<unsigned short>(s, Effect_Subtype), 9ll);
        KT_CHECK_EQ((long long)at<int>(s, Effect_SpellId), 110ll);
        KT_CHECK(near(at<float>(s, Effect_Duration), 226.0f));
        KT_CHECK_EQ((long long)at<int>(s, Effect_IntCount), 8ll);
        KT_CHECK_EQ((long long)intsOf(s)[2], 50ll);
        KT_CHECK_EQ((long long)at<unsigned short>(v, Effect_Type), (long long)EffType_Visual);
        KT_CHECK_EQ((long long)at<int>(v, Effect_SpellId), 110ll);
        KT_CHECK_EQ((long long)intsOf(v)[0], 2045ll);
        KT_CHECK_EQ((long long)at<int>(b, Effect_SpellId), 69ll);
        KT_CHECK(near(at<float>(b, Effect_Duration), 132.0f));
        KT_CHECK_EQ((long long)at<int>(b, Effect_ExpiryDay), (long long)BuffDay);
        KT_CHECK_EQ((long long)at<int>(b, Effect_ExpiryTime), (long long)BuffMs);
    }

    // ---- readEffectList / effectListNonEmpty / plausibleCreature ----
    {
        int arr = 99, cnt = 99;
        KT_CHECK(!readEffectList(0, &arr, &cnt));
        KT_CHECK(!readEffectList(0x20, &arr, &cnt));                 // below 0x10000: unreadable
        KT_CHECK(!effectListNonEmpty(0, &arr, &cnt));
        KT_CHECK(!plausibleCreature(0)); KT_CHECK(!plausibleCreature(0x20));

        uintptr_t c = arenaAlloc(0x200);                             // empty: arr 0, count 0
        KT_CHECK(readEffectList(c, &arr, &cnt)); KT_CHECK_EQ((long long)arr, 0ll); KT_CHECK_EQ((long long)cnt, 0ll);
        KT_CHECK(!effectListNonEmpty(c, &arr, &cnt));
        KT_CHECK(plausibleCreature(c));                              // count 0 is plausible
        PoolRaw pr[4]; Buff bf[4];
        KT_CHECK_EQ((long long)scanShieldPools(c, pr, 4), 0ll);
        KT_CHECK_EQ((long long)collectBuffs(c, 0, 0, false, bf, 4), 0ll);

        uintptr_t listBuf = arenaAlloc(16);
        at<uint32_t>(c, Creature_EffectArray) = (uint32_t)listBuf;   // array present, count 0
        KT_CHECK(readEffectList(c, &arr, &cnt)); KT_CHECK(!effectListNonEmpty(c, &arr, &cnt));
        at<int>(c, Creature_EffectCount) = 1;
        KT_CHECK(effectListNonEmpty(c, &arr, &cnt)); KT_CHECK_EQ((long long)arr, (long long)(uint32_t)listBuf);
        at<int>(c, Creature_EffectCount) = MaxEffects;               // 512 is the last sane count
        KT_CHECK(effectListNonEmpty(c, &arr, &cnt)); KT_CHECK(plausibleCreature(c)); KT_CHECK_EQ((long long)cnt, 512ll);
        at<int>(c, Creature_EffectCount) = MaxEffects + 1;           // 513: rejected, but still read raw
        KT_CHECK(readEffectList(c, &arr, &cnt)); KT_CHECK_EQ((long long)cnt, 513ll);
        KT_CHECK(!effectListNonEmpty(c, &arr, &cnt)); KT_CHECK(!plausibleCreature(c));
        KT_CHECK_EQ((long long)scanShieldPools(c, pr, 4), 0ll);
        KT_CHECK_EQ((long long)collectBuffs(c, 0, 0, false, bf, 4), 0ll);
        at<int>(c, Creature_EffectCount) = -1;
        KT_CHECK(!effectListNonEmpty(c, &arr, &cnt)); KT_CHECK(!plausibleCreature(c));
        at<int>(c, Creature_EffectCount) = 3; at<uint32_t>(c, Creature_EffectArray) = 0;   // null array, count 3
        KT_CHECK(readEffectList(c, &arr, &cnt)); KT_CHECK(!effectListNonEmpty(c, &arr, &cnt));
        KT_CHECK(plausibleCreature(c));                              // plausibleCreature ignores the array pointer
        KT_CHECK_EQ((long long)scanShieldPools(c, pr, 4), 0ll);
        KT_CHECK_EQ((long long)collectBuffs(c, 0, 0, false, bf, 4), 0ll);
    }

    // ---- currentTime: app -> g -> a -> b -> timer chain ----
    {
        uintptr_t cell = arenaAlloc(16), g = arenaAlloc(32), a = arenaAlloc(32), b = arenaAlloc(0x10100);
        const uintptr_t Timer = 0x1234560;
        auto build = [&] {
            at<uint32_t>(cell, 0) = (uint32_t)g; at<uint32_t>(g, 8) = (uint32_t)a; at<uint32_t>(a, 4) = (uint32_t)b;
            at<uint32_t>(b, 0x10048) = (uint32_t)Timer; g_timeCalls = 0; g_lastTimer = 0;
        };
        auto savedGet = g_fx.getTime; auto savedApp = g_fx.appGlobal;
        g_fx.getTime = fakeGetTime; g_fx.appGlobal = cell;
        int day = -7, ms = -7;
        build(); g_fakeDay = 146; g_fakeMs = 2500000;
        KT_CHECK(currentTime(&day, &ms));
        KT_CHECK_EQ((long long)day, 146ll); KT_CHECK_EQ((long long)ms, 2500000ll);
        KT_CHECK_EQ((long long)g_timeCalls, 1ll); KT_CHECK_EQ((unsigned long long)g_lastTimer, (unsigned long long)Timer);
        day = ms = -7;
        build(); at<uint32_t>(b, 0x10048) = 0;                        // no world timer (menu / load)
        KT_CHECK(!currentTime(&day, &ms)); KT_CHECK_EQ((long long)g_timeCalls, 0ll); KT_CHECK_EQ((long long)day, -7ll);
        build(); at<uint32_t>(a, 4) = 0; KT_CHECK(!currentTime(&day, &ms));
        build(); at<uint32_t>(g, 8) = 0; KT_CHECK(!currentTime(&day, &ms));
        build(); at<uint32_t>(cell, 0) = 0; KT_CHECK(!currentTime(&day, &ms));
        KT_CHECK_EQ((long long)g_timeCalls, 0ll);
        g_fx.appGlobal = 0; KT_CHECK(!currentTime(&day, &ms));
        g_fx.getTime = savedGet; g_fx.appGlobal = savedApp;
    }

    // ---- remainingRatio ----
    {
        KT_CHECK(near(remainingRatio(10, 5000, 10.0f, 10, 0), 0.5f));
        // across a day boundary: 1 s before midnight today, expires 1 s after midnight tomorrow = 2 s left of 4 s
        KT_CHECK(near(remainingRatio(147, 1000, 4.0f, 146, MsPerDay - 1000), 0.5f));
        KT_CHECK(near(remainingRatio(147, 1000, 2.0f, 146, MsPerDay - 1000), 1.0f));
        KT_CHECK(near(remainingRatio(147, 1000, 8.0f, 146, MsPerDay - 1000), 0.25f));
        // expiry day behind today
        KT_CHECK(near(remainingRatio(146, 500, 10.0f, 147, 0), 0.0f));
        KT_CHECK(near(remainingRatio(146, 500, 10.0f, 147, 0, 0.1f), 0.1f));
        // clamp high (not yet applied / clock skew) and low
        KT_CHECK(near(remainingRatio(10, 9000, 1.0f, 10, 0), 1.0f));
        KT_CHECK(near(remainingRatio(10, 0, 10.0f, 10, 1), 0.0f));
        KT_CHECK(near(remainingRatio(10, 0, 10.0f, 10, 6000, 0.05f), 0.05f));
        KT_CHECK(near(remainingRatio(10, 5000, 10.0f, 10, 0, 0.9f), 0.9f));   // min above real ratio wins
        KT_CHECK(near(remainingRatio(10, 5000, 10.0f, 10, 0, 0.0f), 0.5f));
    }

    // ---- effectRatio on the captured buff ----
    {
        uintptr_t b = buff();
        int tNow = BuffMs - 66000;                                    // half of 132 s left
        KT_CHECK(near(effectRatio(b, BuffDay, tNow, true), 0.5f));
        KT_CHECK(near(effectRatio(b, BuffDay, BuffMs - 132000, true), 1.0f));
        KT_CHECK(near(effectRatio(b, BuffDay, tNow, false), 1.0f));   // no world time
        KT_CHECK(near(effectRatio(b, BuffDay, BuffMs + 5000, true), 0.0f));
        KT_CHECK(near(effectRatio(b, BuffDay, BuffMs + 5000, true, 0.2f), 0.2f));
        at<float>(b, Effect_Duration) = 0.0f;                        // permanent
        KT_CHECK(near(effectRatio(b, BuffDay, tNow, true), 1.0f));
        at<float>(b, Effect_Duration) = -3.0f;
        KT_CHECK(near(effectRatio(b, BuffDay, tNow, true), 1.0f));
        KT_CHECK(near(effectRatio(0x20, BuffDay, tNow, true), 1.0f)); // unreadable effect
        KT_CHECK(near(effectRatio(0, BuffDay, tNow, true), 1.0f));
    }

    // ---- isItemAbility ranges ----
    {
        const int yes[] = { 69, 77, 99, 108, 110, 115, 129, 132, 257 };
        const int no[] = { -1, 0, 68, 78, 98, 109, 116, 128, 130, 131, 133, 256, 258 };
        for (int s : yes) { char w[32]; snprintf(w, sizeof w, "isItemAbility(%d)", s); kt::Check(w, isItemAbility(s)); }
        for (int s : no) { char w[32]; snprintf(w, sizeof w, "!isItemAbility(%d)", s); kt::Check(w, !isItemAbility(s)); }
    }

    // ---- scanShieldPools on the captured shield + sibling ----
    {
        PoolRaw p[4];
        // decoy vfx: same type 0x1e but spell 111 / row 2041, listed BEFORE the real sibling
        uintptr_t decoy = vfx(); at<int>(decoy, Effect_SpellId) = 111; intsOf(decoy)[0] = 2041;
        uintptr_t s = shield(), v = vfx(), b = buff();
        uintptr_t c = makeCreature({ decoy, b, s, v });
        int n = scanShieldPools(c, p, 4);
        KT_CHECK_EQ((long long)n, 1ll);
        KT_CHECK_EQ((unsigned long long)p[0].eff, (unsigned long long)s);
        KT_CHECK_EQ((long long)p[0].spell, 110ll); KT_CHECK_EQ((long long)p[0].flags, 4224ll);
        KT_CHECK_EQ((long long)p[0].remaining, 50ll);
        KT_CHECK_EQ((long long)p[0].vfx, 2045ll);                     // sibling with the same spell, not the decoy

        // no sibling at all -> vfx 0 (decoy only, spell 111)
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ decoy, shield() }), p, 4), 1ll);
        KT_CHECK_EQ((long long)p[0].vfx, 0ll);
        // the type 0x43 / buff effects of the same spell are not a vfx sibling
        uintptr_t notVfx = vfx(); at<unsigned short>(notVfx, Effect_Type) = 0x43;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ notVfx, shield() }), p, 4), 1ll);
        KT_CHECK_EQ((long long)p[0].vfx, 0ll);

        // remaining 0 / negative -> skipped
        uintptr_t s0 = shield(); intsOf(s0)[2] = 0;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ s0, vfx() }), p, 4), 0ll);
        intsOf(s0)[2] = -5;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ s0, vfx() }), p, 4), 0ll);
        intsOf(s0)[2] = 1;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ s0, vfx() }), p, 4), 1ll);
        KT_CHECK_EQ((long long)p[0].remaining, 1ll);
        // < 3 ints -> skipped (2 skipped, 3 accepted)
        uintptr_t sc = shield(); at<int>(sc, Effect_IntCount) = 2;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ sc, vfx() }), p, 4), 0ll);
        at<int>(sc, Effect_IntCount) = 0;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ sc }), p, 4), 0ll);
        at<int>(sc, Effect_IntCount) = 3;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ sc }), p, 4), 1ll);
        // null int pointer, wrong type, subtype without the pool bit
        uintptr_t sn = shield(); at<uint32_t>(sn, Effect_Ints) = 0;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ sn }), p, 4), 0ll);
        uintptr_t st = shield(); at<unsigned short>(st, Effect_Type) = 3;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ st }), p, 4), 0ll);
        uintptr_t sb = shield(); at<unsigned short>(sb, Effect_Subtype) = 1;
        KT_CHECK_EQ((long long)scanShieldPools(makeCreature({ sb }), p, 4), 0ll);
        // null / unreadable effect entries are skipped, real pool after them still found
        uintptr_t cn = makeCreature({ 0, 0x40, shield() });
        KT_CHECK_EQ((long long)scanShieldPools(cn, p, 4), 1ll);

        // two pools, each paired with its own sibling, order kept; maxOut limits
        uintptr_t s2 = shield(); at<int>(s2, Effect_SpellId) = 111; intsOf(s2)[2] = 20; intsOf(s2)[0] = 4096;
        uintptr_t c2 = makeCreature({ s, decoy, s2, v });
        KT_CHECK_EQ((long long)scanShieldPools(c2, p, 4), 2ll);
        KT_CHECK_EQ((long long)p[0].spell, 110ll); KT_CHECK_EQ((long long)p[0].vfx, 2045ll);
        KT_CHECK_EQ((long long)p[1].spell, 111ll); KT_CHECK_EQ((long long)p[1].vfx, 2041ll);
        KT_CHECK_EQ((long long)p[1].remaining, 20ll); KT_CHECK_EQ((long long)p[1].flags, 4096ll);
        KT_CHECK_EQ((long long)scanShieldPools(c2, p, 1), 1ll);
        KT_CHECK_EQ((long long)p[0].spell, 110ll);
        KT_CHECK_EQ((long long)scanShieldPools(c2, p, 0), 0ll);
    }

    // ---- iconForSpell ----
    {
        bool item = true;
        KT_CHECK_EQ(iconForSpell(69, false, &item), "ii_adrnaline_001"); KT_CHECK(item);
        KT_CHECK_EQ(iconForSpell(8, false, &item), "ip_speedburst"); KT_CHECK(!item);
        KT_CHECK_EQ(iconForSpell(110, false, &item), "ii_drdshld_001"); KT_CHECK(item);
        KT_CHECK(iconForSpell(0, false, &item) == nullptr); KT_CHECK(!item);
        KT_CHECK_EQ(iconForSpell(4, true, &item), "ip_advlsthrow"); KT_CHECK(!item);
        KT_CHECK_EQ(iconForSpell(116, true, &item), "ii_drdutldev_003"); KT_CHECK(item);
        KT_CHECK(iconForSpell(69, true, &item) == nullptr);           // buff row is not a debuff
        KT_CHECK(iconForSpell(4, false, &item) == nullptr);           // debuff row is not a buff
        KT_CHECK(iconForSpell(-1, false, &item) == nullptr); KT_CHECK(!item);
        item = true; KT_CHECK(iconForSpell(kEffectIconTableSize, true, &item) == nullptr); KT_CHECK(!item);
        KT_CHECK(iconForSpell(100000, false, &item) == nullptr);
    }

    // ---- collectBuffs ----
    {
        Buff o[8];
        uintptr_t b1 = buff();                                        // spell 69, 132 s, half left at tNow
        uintptr_t b2 = buff(); at<int>(b2, Effect_ExpiryTime) = BuffMs + 60000;   // same spell id, different expiry
        uintptr_t sh = shield();                                      // spell 110, 226 s
        uintptr_t speed = buff(); at<int>(speed, Effect_SpellId) = 8; at<float>(speed, Effect_Duration) = 0.0f;
        uintptr_t none = buff(); at<int>(none, Effect_SpellId) = -1;  // untracked
        uintptr_t big = buff(); at<int>(big, Effect_SpellId) = 5000;  // out of table
        int tNow = BuffMs - 66000;
        uintptr_t c = makeCreature({ none, b1, b2, big, sh, speed, 0, b1 });
        int n = collectBuffs(c, BuffDay, tNow, true, o, 8);
        KT_CHECK_EQ((long long)n, 3ll);                               // 69 once, 110, 8; dups/untracked/null dropped
        KT_CHECK_EQ(o[0].resref, "ii_adrnaline_001"); KT_CHECK(o[0].item); KT_CHECK(near(o[0].ratio, 0.5f));   // first one wins
        KT_CHECK_EQ(o[1].resref, "ii_drdshld_001"); KT_CHECK(o[1].item);
        KT_CHECK(near(o[1].ratio, remainingRatio(at<int>(sh, Effect_ExpiryDay), at<int>(sh, Effect_ExpiryTime), 226.0f, BuffDay, tNow)));
        KT_CHECK_EQ(o[2].resref, "ip_speedburst"); KT_CHECK(!o[2].item);
        KT_CHECK(near(o[2].ratio, 1.0f));                             // duration 0 -> 1.0
        // first-listed duplicate wins (here b2, listed before b1)
        n = collectBuffs(makeCreature({ b2, b1 }), BuffDay, tNow, true, o, 8);
        KT_CHECK_EQ((long long)n, 1ll); KT_CHECK(near(o[0].ratio, 126000.0f / 132000.0f));   // b2 (listed first) wins: expires 60 s after BuffMs -> 126 s left of 132 s
        // duration 0 stays 1.0 whatever the time, and without world time every ratio is 1.0
        n = collectBuffs(c, BuffDay, BuffMs + 99999, true, o, 8);
        KT_CHECK(near(o[0].ratio, 0.0f)); KT_CHECK(near(o[2].ratio, 1.0f));
        n = collectBuffs(c, 0, 0, false, o, 8);
        KT_CHECK_EQ((long long)n, 3ll);
        for (int i = 0; i < n; ++i) KT_CHECK(near(o[i].ratio, 1.0f));
        // minRatio floor
        n = collectBuffs(c, BuffDay, BuffMs + 99999, true, o, 8, false, 0.15f);
        KT_CHECK(near(o[0].ratio, 0.15f)); KT_CHECK(near(o[2].ratio, 1.0f));
        // maxOut caps, in order
        KT_CHECK_EQ((long long)collectBuffs(c, BuffDay, tNow, true, o, 2), 2ll);
        KT_CHECK_EQ(o[1].resref, "ii_drdshld_001");
        KT_CHECK_EQ((long long)collectBuffs(c, BuffDay, tNow, true, o, 0), 0ll);
        // debuff mode: same effects, debuff table; spells 69 / 110 / 8 have no debuff row
        KT_CHECK_EQ((long long)collectBuffs(c, BuffDay, tNow, true, o, 8, true), 0ll);
        uintptr_t d1 = buff(); at<int>(d1, Effect_SpellId) = 4;
        uintptr_t d2 = buff(); at<int>(d2, Effect_SpellId) = 116;
        uintptr_t d3 = buff(); at<int>(d3, Effect_SpellId) = 4;
        n = collectBuffs(makeCreature({ d1, b1, d2, d3 }), BuffDay, tNow, true, o, 8, true);
        KT_CHECK_EQ((long long)n, 2ll);
        KT_CHECK_EQ(o[0].resref, "ip_advlsthrow"); KT_CHECK(!o[0].item);
        KT_CHECK_EQ(o[1].resref, "ii_drdutldev_003"); KT_CHECK(o[1].item);
        // ... and in buff mode spells 4 / 116 are not tracked
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ d1, d2 }), BuffDay, tNow, true, o, 8), 0ll);

        // script SetEffectIcon (type 0x43, ints[0] = effecticon.2da row): a harmful row is a debuff icon even with spell -1 (Slow = row 28)
        auto iconFx = [&](int row, int spell, int nInts) {
            uintptr_t e = buff(); at<unsigned short>(e, Effect_Type) = 0x43; at<int>(e, Effect_SpellId) = spell;
            intsOf(e)[0] = row; at<int>(e, Effect_IntCount) = nInts; return e; };
        uintptr_t slowIcon = iconFx(28, -1, 1);
        n = collectBuffs(makeCreature({ none, slowIcon }), BuffDay, tNow, true, o, 8, true);
        KT_CHECK_EQ((long long)n, 1ll); KT_CHECK_EQ(o[0].resref, "ip_slow"); KT_CHECK(!o[0].item);
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ slowIcon }), BuffDay, tNow, true, o, 8), 0ll);   // never in the buff column
        // same icon via the spell-45 effect and the row, either order, and repeated rows: one entry
        uintptr_t slow45 = buff(); at<int>(slow45, Effect_SpellId) = 45;
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ slowIcon, slow45 }), BuffDay, tNow, true, o, 8, true), 1ll);
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ slow45, slowIcon, iconFx(28, -1, 1) }), BuffDay, tNow, true, o, 8, true), 1ll);
        // beneficial row (2 = speed burst, good 1), out of range, row 0, negative, no ints: nothing
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ iconFx(2, -1, 1) }), BuffDay, tNow, true, o, 8, true), 0ll);
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ iconFx(kEffectIconRowCount, -1, 1), iconFx(5000, -1, 1), iconFx(0, -1, 1), iconFx(-3, -1, 1) }), BuffDay, tNow, true, o, 8, true), 0ll);
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ iconFx(28, -1, 0) }), BuffDay, tNow, true, o, 8, true), 0ll);
        // a non-0x43 effect whose first int happens to be 28 is not an icon row
        uintptr_t fake = buff(); intsOf(fake)[0] = 28;
        KT_CHECK_EQ((long long)collectBuffs(makeCreature({ fake }), BuffDay, tNow, true, o, 8, true), 0ll);
        // another harmful row mixes with spell debuffs, order kept (row 5 = ip_droiddestroy)
        n = collectBuffs(makeCreature({ d1, iconFx(28, -1, 1), iconFx(5, -1, 1) }), BuffDay, tNow, true, o, 8, true);
        KT_CHECK_EQ((long long)n, 3ll); KT_CHECK_EQ(o[0].resref, "ip_advlsthrow"); KT_CHECK_EQ(o[1].resref, "ip_slow"); KT_CHECK_EQ(o[2].resref, "ip_droiddestroy");
    }

    return kt::Report("effects_test");
}
