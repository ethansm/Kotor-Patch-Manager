// Host test of DiversityAnim.h (NB2 scheduling): flourish pool, delays, roster, Due().
#include "DiversityAnim.h"
#include "check.h"
#include <set>
#include <string>

using namespace NpcDiversity;
using namespace NpcDiversity::Anim;

static bool inPool(const Flourish* f, bool military) {
    if (!f) return false;
    for (int i = 0; i < kAmbientCount; ++i) if (!strcmp(f->name, kAmbient[i].name)) return true;
    return military && !strcmp(f->name, "salute");
}

int main() {
    // 1. pool contract: exactly the 7 ambient names, none of the excluded ones, military only salute
    {
        KT_CHECK_EQ(kAmbientCount, 7);
        const char* excluded[] = {"victory", "taunt", "bow", "point", "equip", "itemequip"};
        for (int i = 0; i < kAmbientCount; ++i) {
            for (const char* e : excluded) KT_CHECK(strcmp(kAmbient[i].name, e) != 0);
            KT_CHECK(kAmbient[i].lenMs >= 1000 && kAmbient[i].lenMs <= 6000);
        }
        KT_CHECK(!strcmp(kMilitary.name, "salute"));
    }
    // 2. Pick: civilians never salute; military sometimes do (about 1 in 4); every result is in the pool
    {
        int salutes = 0, civSalutes = 0; std::set<std::string> seen;
        for (uint32_t i = 1; i <= 4000; ++i) {
            uint32_t h = HashCreatureId(i, 100, 0);
            const Flourish& c = Pick(h, false); if (!strcmp(c.name, "salute")) ++civSalutes; seen.insert(c.name);
            const Flourish& m = Pick(h, true);  KT_CHECK(inPool(&m, true)); if (!strcmp(m.name, "salute")) ++salutes;
        }
        KT_CHECK_EQ(civSalutes, 0);
        KT_CHECK(salutes > 800 && salutes < 1200);     // ~1000 of 4000
        KT_CHECK_EQ(static_cast<int>(seen.size()), 7); // all ambient names get used
    }
    // 3. delays stay in range and gaps never go under 20 s
    {
        for (uint32_t i = 1; i <= 2000; ++i) {
            uint32_t h = HashCreatureId(i, 7, 0x1234);
            uint32_t f = FirstDelayMs(h);
            KT_CHECK(f >= kFirstMinMs && f < kFirstMinMs + kFirstSpanMs);
            for (uint32_t n = 0; n < 3; ++n) { uint32_t g = GapMs(h, n); KT_CHECK(g >= 20000u && g < 45000u); }
        }
        uint32_t h = HashCreatureId(5, 7, 0); KT_CHECK(GapMs(h, 0) != GapMs(h, 1) || GapMs(h, 1) != GapMs(h, 2));   // fresh draw per play
    }
    // 4. roster: add once, dedupe, cap, remove, clear; oid 0 refused
    {
        Roster r; r.clear();
        KT_CHECK(r.add(0, 1, 1, false, 0) == nullptr);
        Slot* a = r.add(10, 100, 5, false, 1000); KT_CHECK(a != nullptr);
        KT_CHECK(r.add(10, 100, 5, false, 9999) == a);
        KT_CHECK_EQ(r.count, 1);
        KT_CHECK(a->nextMs >= 1000 + kFirstMinMs);
        for (uint32_t i = 100; i < 100 + kMaxSlots; ++i) r.add(i, 1, 5, false, 0);
        KT_CHECK_EQ(r.count, kMaxSlots);
        KT_CHECK(r.dropped >= 1);
        r.remove(10); KT_CHECK(r.find(10) == nullptr); KT_CHECK_EQ(r.count, kMaxSlots - 1);
        r.clear(); KT_CHECK_EQ(r.count, 0); KT_CHECK_EQ(r.dropped, 0);
    }
    // 5. Due: nothing before nextMs, not idle retries later without playing, idle plays and re-arms past the length plus the gap
    {
        Roster r; r.clear();
        Slot* s = r.add(77, 100, 9, false, 0);
        uint32_t t0 = s->nextMs;
        KT_CHECK(Due(*s, t0 - 1, true) == nullptr);
        KT_CHECK(Due(*s, t0, false) == nullptr);                         // busy creature: retry soon
        KT_CHECK_EQ(s->nextMs, t0 + kBusyRetryMs);
        KT_CHECK(Due(*s, t0 + 1, true) == nullptr);                      // still before the retry time
        uint32_t t1 = s->nextMs;
        const Flourish* f = Due(*s, t1, true);
        KT_CHECK(inPool(f, false));
        KT_CHECK_EQ(s->plays, 1u);
        KT_CHECK(s->busyUntilMs == t1 + f->lenMs);
        KT_CHECK(s->nextMs >= t1 + f->lenMs + 20000u);
        KT_CHECK(Due(*s, s->busyUntilMs - 1, true) == nullptr);          // never overlaps our own flourish
        KT_CHECK(Due(*s, s->nextMs - 1, true) == nullptr);
        KT_CHECK(Due(*s, s->nextMs, true) != nullptr);
        KT_CHECK_EQ(s->plays, 2u);
    }
    // 6. clock wrap: a slot armed just before the 32-bit wrap still fires after it
    {
        Roster r; r.clear();
        Slot* s = r.add(5, 1, 1, false, 0xFFFFF000u);
        KT_CHECK(Due(*s, 0xFFFFF000u, true) == nullptr);
        KT_CHECK(Due(*s, s->nextMs, true) != nullptr);
    }
    // 7. a new visit salt changes the plan (the variety goal), the same salt repeats it
    {
        Roster a, b, c; a.clear(); b.clear(); c.clear();
        int diff = 0;
        for (uint32_t i = 1; i <= 90; ++i) {
            Slot* x = a.add(i, 100, 1, false, 0); Slot* y = b.add(i, 100, 2, false, 0); Slot* z = c.add(i, 100, 1, false, 0);
            KT_CHECK_EQ(x->nextMs, z->nextMs);
            if (x->nextMs != y->nextMs) ++diff;
        }
        KT_CHECK(diff > 80);
    }
    // 8. eligibility: only the two armed TSF rows; civilians, appended custom rows and party-ish rows are out
    {
        bool mil = true;
        KT_CHECK(!EligibleRow(185, &mil) && !mil);   // unarmed civilians do not flourish (user 10-18)
        KT_CHECK(!EligibleRow(190) && !EligibleRow(12) && !EligibleRow(569) && !EligibleRow(601));
        KT_CHECK(EligibleRow(479, &mil) && mil);
        KT_CHECK(EligibleRow(485, &mil) && mil);
        KT_CHECK(!EligibleRow(802) && !EligibleRow(813) && !EligibleRow(817) && !EligibleRow(0) && !EligibleRow(6));
    }
    // 9. idle test: every single blocker flips it
    {
        ServerFacts f = {5, 0, 0, 0xFFFF, 0, 0, kObjInvalid};
        KT_CHECK(IsIdle(f));
        { ServerFacts g = f; g.dialog = 0; KT_CHECK(IsIdle(g)); }
        { ServerFacts g = f; g.type = 9; KT_CHECK(!IsIdle(g)); }
        { ServerFacts g = f; g.party = 1; KT_CHECK(!IsIdle(g)); }
        { ServerFacts g = f; g.isPc = 1; KT_CHECK(!IsIdle(g)); }
        { ServerFacts g = f; g.action = 3; KT_CHECK(!IsIdle(g)); }
        { ServerFacts g = f; g.queueHead = 0x1000; KT_CHECK(IsIdle(g)); }   // queue head is not part of the idle test (Run A2)
        { ServerFacts g = f; g.combat = 1; KT_CHECK(!IsIdle(g)); }
        { ServerFacts g = f; g.dialog = 0x7F000123u; KT_CHECK(!IsIdle(g)); }
    }
    return kt::Report("diversity_anim_test");
}
