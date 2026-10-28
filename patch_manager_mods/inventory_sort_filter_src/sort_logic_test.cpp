// Host unit tests for SortLogic.h:  g++ -std=c++17 -Wall -Wextra -o /tmp/sort_logic_test sort_logic_test.cpp && /tmp/sort_logic_test
#include "SortLogic.h"
#include <stdio.h>
#include <string.h>
#include <vector>
using namespace isf;

static int g_fail = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

int main() {
    // sort: new first, usable before unusable, base item ascending, stable
    std::vector<Entry> v = {
        {1, 1, 0, false, true, 50, false}, {2, 2, 1, false, false, 10, true}, {3, 3, 2, true, false, 99, true},
        {4, 4, 3, false, true, 50, false}, {5, 5, 4, false, true, 7, false},  {6, 6, 5, true, true, 3, false},
        {7, 7, 6, false, false, 10, false},
    };
    sortEntries(v.data(), (int)v.size());
    int want[] = {6, 3, 5, 1, 4, 2, 7};
    for (size_t i = 0; i < v.size(); ++i) CHECK(v[i].item == want[i]);

    // stability / strict weak order: identical keys keep vanilla order
    std::vector<Entry> w; for (int i = 0; i < 100; ++i) w.push_back({i, i, i, false, true, i % 3, false});
    sortEntries(w.data(), (int)w.size());
    for (size_t i = 1; i < w.size(); ++i) CHECK(!before(w[i], w[i - 1]));
    int lastBase = -1, lastOrig = -1;
    for (auto& e : w) { if (e.baseItem != lastBase) { lastBase = e.baseItem; lastOrig = -1; } CHECK(e.orig > lastOrig); lastOrig = e.orig; }

    // name tie-break (WI-21): same new/usable/base item, names "b" (orig 0) and "a" (orig 1) -> "a" first
    auto setName = [](Entry& e, const char* s) { snprintf(e.name, sizeof e.name, "%s", s); };
    std::vector<Entry> nm = {{10, 10, 0, false, true, 5, false}, {11, 11, 1, false, true, 5, false}};
    setName(nm[0], "b"); setName(nm[1], "a");
    sortEntries(nm.data(), (int)nm.size());
    CHECK(nm[0].item == 11 && nm[1].item == 10);

    // empty names (failed lookup) are equal: vanilla order decides; a default-built Entry has an empty name
    Entry blank;
    CHECK(blank.name[0] == 0 && blank.name[63] == 0);
    std::vector<Entry> em = {{20, 20, 1, false, true, 5, false}, {21, 21, 0, false, true, 5, false}};
    sortEntries(em.data(), (int)em.size());
    CHECK(em[0].item == 21 && em[1].item == 20);

    // name is the 4th key: isNew, usable and baseItem still decide first ("z" new, "a" base 9 vs "b" base 5, unusable "a")
    std::vector<Entry> pr = {{30, 30, 0, false, true, 9, false}, {31, 31, 1, false, true, 5, false},
                             {32, 32, 2, false, false, 5, false}, {33, 33, 3, true, false, 9, false}};
    setName(pr[0], "a"); setName(pr[1], "z"); setName(pr[2], "a"); setName(pr[3], "z");
    sortEntries(pr.data(), (int)pr.size());
    int wantP[] = {33, 31, 30, 32};
    for (size_t i = 0; i < pr.size(); ++i) CHECK(pr[i].item == wantP[i]);

    // mixed empty/named rows stay a strict weak order (no skip-when-empty cycle): "" first, then names, ties by orig
    std::vector<Entry> mx = {{40, 40, 0, false, true, 5, false}, {41, 41, 1, false, true, 5, false},
                             {42, 42, 2, false, true, 5, false}, {43, 43, 3, false, true, 5, false}};
    setName(mx[0], "b"); setName(mx[1], "a"); /* mx[2] stays empty */ setName(mx[3], "a");
    sortEntries(mx.data(), (int)mx.size());
    int wantM[] = {42, 41, 43, 40};
    for (size_t i = 0; i < mx.size(); ++i) CHECK(mx[i].item == wantM[i]);
    for (size_t i = 0; i < mx.size(); ++i) { CHECK(!before(mx[i], mx[i])); for (size_t j = 0; j < mx.size(); ++j) CHECK(!(before(mx[i], mx[j]) && before(mx[j], mx[i]))); }

    // case-insensitive name compare: "b" (orig 0) vs "A" (orig 1) -> "A" first
    std::vector<Entry> ci = {{50, 50, 0, false, true, 5, false}, {51, 51, 1, false, true, 5, false}};
    setName(ci[0], "b"); setName(ci[1], "A");
    sortEntries(ci.data(), (int)ci.size());
    CHECK(ci[0].item == 51 && ci[1].item == 50);
    // "apple" (orig 0) vs "Apple" (orig 1): equal under fold, vanilla order decides
    std::vector<Entry> eq = {{60, 60, 0, false, true, 5, false}, {61, 61, 1, false, true, 5, false}};
    setName(eq[0], "apple"); setName(eq[1], "Apple");
    sortEntries(eq.data(), (int)eq.size());
    CHECK(eq[0].item == 60 && eq[1].item == 61);
    CHECK(before(eq[0], eq[1]) && !before(eq[1], eq[0]));
    // mixed-case set incl. an empty name stays a strict weak order
    std::vector<Entry> mc = {{70, 70, 0, false, true, 5, false}, {71, 71, 1, false, true, 5, false}, {72, 72, 2, false, true, 5, false},
                             {73, 73, 3, false, true, 5, false}, {74, 74, 4, false, true, 5, false}};
    setName(mc[0], "Banana"); setName(mc[1], "apple"); /* mc[2] empty */ setName(mc[3], "APPLE"); setName(mc[4], "cherry");
    sortEntries(mc.data(), (int)mc.size());
    int wantC[] = {72, 71, 73, 70, 74};
    for (size_t i = 0; i < mc.size(); ++i) CHECK(mc[i].item == wantC[i]);
    for (size_t i = 0; i < mc.size(); ++i) { CHECK(!before(mc[i], mc[i])); for (size_t j = 0; j < mc.size(); ++j) CHECK(!(before(mc[i], mc[j]) && before(mc[j], mc[i]))); }
    // bytes >= 0x80 compare as unsigned raw bytes (cp1252 never negative): "\xE9" sorts after "z"
    std::vector<Entry> hi = {{80, 80, 0, false, true, 5, false}, {81, 81, 1, false, true, 5, false}};
    setName(hi[0], "\xE9"); setName(hi[1], "z");
    sortEntries(hi.data(), (int)hi.size());
    CHECK(hi[0].item == 81 && hi[1].item == 80);

    // cycle state machine, all states enabled: New = engine mode 1, Usable/Upgrade = mode 0 + our extra filter
    unsigned all = (1u << S_New) | (1u << S_Usable) | (1u << S_Upgrade);
    State s = S_All; Extra ex = X_None; int mode = 0;
    auto click = [&](int clicked, unsigned mask) {
        State n;
        if (!onTabClick(clicked, mode, ex, mask, &n)) return false;
        mode = engineModeOf(n); ex = extraOf(n); s = n; return true;
    };
    CHECK(click(0, all) && s == S_New && mode == 1 && ex == X_None);
    CHECK(click(0, all) && s == S_Usable && mode == 0 && ex == X_Usable);
    CHECK(click(0, all) && s == S_Upgrade && mode == 0 && ex == X_Upgrade);
    CHECK(click(0, all) && s == S_All && mode == 0 && ex == X_None);
    // another tab is never consumed, even inside the cycle
    mode = 1; CHECK(!click(3, all)); CHECK(!click(1, all));
    // a mode outside the cycle (weapons tab = 7 etc., and engine mode 5 = the old 0.1.0 Usable) is never consumed
    mode = 2; ex = X_None; CHECK(!click(0, all));
    mode = 7; CHECK(!click(0, all));
    mode = 5; CHECK(!click(0, all));
    // subset masks skip disabled states
    mode = 0; ex = X_None; s = S_All;
    CHECK(click(0, 1u << S_Upgrade) && s == S_Upgrade);
    CHECK(click(0, 1u << S_Upgrade) && s == S_All);
    CHECK(click(0, (1u << S_New) | (1u << S_Upgrade)) && s == S_New);
    CHECK(click(0, (1u << S_New) | (1u << S_Upgrade)) && s == S_Upgrade && mode == 0 && ex == X_Upgrade);
    // empty mask: All stays All, click not consumed (vanilla handles it)
    mode = 0; ex = X_None; CHECK(!click(0, 0));
    // stateOf
    State st;
    CHECK(stateOf(0, X_Usable, &st) && st == S_Usable);
    CHECK(stateOf(1, X_None, &st) && st == S_New);
    CHECK(!stateOf(5, X_None, &st) && !stateOf(0xFF, X_None, &st) && !stateOf(-1, X_None, &st));
    // equip predicate helpers
    std::vector<unsigned> seen;
    CHECK(anySlotBit(0x30, [&](unsigned b) { seen.push_back(b); return b == 0x20; }) && seen.size() == 2 && seen[0] == 0x10);
    seen.clear();
    CHECK(!anySlotBit(0x2 | 0x80000, [&](unsigned b) { seen.push_back(b); return false; }) && seen.size() == 2 && seen[1] == 0x80000);
    CHECK(!anySlotBit(0, [](unsigned) { return true; }));
    CHECK(anySlotBit(0x80000000u, [](unsigned b) { return b == 0x80000000u; }));
    CHECK(genderAllows(0, 7, 1, 2) && genderAllows(1, 1, 1, 2) && !genderAllows(1, 2, 1, 2) && genderAllows(2, 2, 1, 2) && genderAllows(3, 9, 1, 2));
    CHECK(tintRow(true, false) && !tintRow(true, true) && !tintRow(false, false));
    CHECK(usableForCycle(true, false) && usableForCycle(false, true) && !usableForCycle(false, false));
    // parse
    CHECK(parseCycleStates("new,usable,upgrade") == all);
    CHECK(parseCycleStates(" NEW , Upgradeable ") == ((1u << S_New) | (1u << S_Upgrade)));
    CHECK(parseCycleStates("") == 0);
    CHECK(parseCycleStates("bogus,usable") == (1u << S_Usable));
    CHECK(ieq("Abc", "aBC") && !ieq("ab", "abc"));
    // captions
    CHECK(!strcmp(captionOf(S_All), "Filter") && !strcmp(captionOf(S_Upgrade), "Filter: Upgradeable"));
    // captions with the visible-row count (WI-151)
    char cb[kCaptionBytes];
    captionWithCount(S_All, all, 5, cb, sizeof cb);        CHECK(!strcmp(cb, "Filter"));
    captionWithCount(S_Usable, all, 0, cb, sizeof cb);     CHECK(!strcmp(cb, "Filter: Usable 2/3 (0)"));
    captionWithCount(S_Upgrade, all, 123, cb, sizeof cb);  CHECK(!strcmp(cb, "Filter: Upgradeable 3/3 (123)"));
    captionWithCount(S_New, all, 7, cb, sizeof cb);        CHECK(!strcmp(cb, "Filter: New 1/3 (7)"));
    char tiny[8];
    captionWithCount(S_Upgrade, all, 123, tiny, sizeof tiny); CHECK(!strcmp(tiny, "Filter:") && tiny[7] == 0);
    char one[1];
    captionWithCount(S_Usable, all, 0, one, sizeof one);   CHECK(one[0] == 0);
    // open upgrade slots
    int none[6] = {0, 0, 0, 0, 0, 0};
    CHECK(!hasOpenSlot(0, none));
    int t1[6] = {0, -1, 0, 0, 0, 0};            // only slot 1 empty: never counted for type 1
    CHECK(!hasOpenSlot(1, t1));
    t1[4] = -1; CHECK(hasOpenSlot(1, t1));
    int t3[6] = {5, 5, -1, 0, 0, 0};            // armor: 3 slots
    CHECK(hasOpenSlot(3, t3));
    int t4[6] = {5, 5, -1, 0, 0, 0};            // type 4: only first 2 slots count
    CHECK(!hasOpenSlot(4, t4));
    int t2[6] = {-1, 0, 0, 0, 0, 0};
    CHECK(hasOpenSlot(2, t2) && upgradeSlotCount(9) == 0);
    // lazy tie names (WI-146): keys A, B, B, C -> readName runs only for the two B rows; order equals the all-named order
    {
        auto mk = []() {
            return std::vector<Entry>{{100, 100, 0, false, true, 1, false}, {101, 101, 1, false, true, 5, false},
                                      {102, 102, 2, false, true, 5, false}, {103, 103, 3, false, true, 9, false}};
        };
        std::vector<Entry> full = mk();
        setName(full[0], "m"); setName(full[1], "zulu"); setName(full[2], "alpha"); setName(full[3], "q");
        sortEntries(full.data(), (int)full.size());
        std::vector<Entry> lz = mk();
        std::vector<int> calledFor;
        int calls = fillTiedNames(lz.data(), (int)lz.size(), [&](Entry& e) {
            calledFor.push_back(e.item);
            setName(e, e.item == 101 ? "zulu" : "alpha");
        });
        CHECK(calls == 2 && calledFor.size() == 2);
        CHECK(std::count(calledFor.begin(), calledFor.end(), 101) == 1 && std::count(calledFor.begin(), calledFor.end(), 102) == 1);
        CHECK(lz[0].name[0] == 0 && lz[3].name[0] == 0);   // unique keys keep an empty name
        sortEntries(lz.data(), (int)lz.size());
        int wantL[] = {100, 102, 101, 103};
        for (size_t i = 0; i < lz.size(); ++i) { CHECK(lz[i].item == wantL[i]); CHECK(lz[i].item == full[i].item); }
        // no shared key: no call at all
        std::vector<Entry> uq = {{110, 110, 0, false, true, 1, false}, {111, 111, 1, false, true, 2, false}};
        CHECK(fillTiedNames(uq.data(), (int)uq.size(), [](Entry&) {}) == 0);
        // above kMaxTieRows every name is read (fallback), n = 0 is a no-op
        std::vector<Entry> big;
        for (int i = 0; i < kMaxTieRows + 1; ++i) big.push_back({i, i, i, false, true, i, false});
        int nb = 0;
        CHECK(fillTiedNames(big.data(), (int)big.size(), [&](Entry&) { ++nb; }) == kMaxTieRows + 1 && nb == kMaxTieRows + 1);
        CHECK(fillTiedNames(big.data(), 0, [](Entry&) {}) == 0);
    }
    // caption_cycle_position (WI-152): place among the ENABLED New<Usable<Upgradeable states; stale state and empty mask get none
    {
        const unsigned nu = 1u << S_New, us = 1u << S_Usable, ug = 1u << S_Upgrade, all3 = nu | us | ug;
        struct Cap { State s; unsigned mask; int rows; const char* want; };
        const Cap caps[] = {
            {S_New, all3, 5, "Filter: New 1/3 (5)"},             {S_Usable, all3, 14, "Filter: Usable 2/3 (14)"},
            {S_Upgrade, all3, 1024, "Filter: Upgradeable 3/3 (1024)"},
            {S_New, nu | us, 5, "Filter: New 1/2 (5)"},          {S_Usable, nu | us, 14, "Filter: Usable 2/2 (14)"},
            {S_Usable, us | ug, 14, "Filter: Usable 1/2 (14)"},  {S_Upgrade, us | ug, 1024, "Filter: Upgradeable 2/2 (1024)"},
            {S_Usable, nu | ug, 14, "Filter: Usable (14)"},      {S_Upgrade, nu | ug, 2, "Filter: Upgradeable 2/2 (2)"},
            {S_Usable, us, 14, "Filter: Usable (14)"},           {S_New, ug, 2, "Filter: New (2)"},
            {S_New, 0u, 14, "Filter"},                           {S_Upgrade, 0u, 1024, "Filter"},
            {S_All, all3, 14, "Filter"},
        };
        for (const Cap& c : caps) {
            char buf[isf::kCaptionBytes];
            captionWithCount(c.s, c.mask, c.rows, buf, sizeof buf);
            CHECK(strcmp(buf, c.want) == 0);
        }
        char buf[isf::kCaptionBytes];
        captionWithCount(S_Usable, all3, -1, buf, sizeof buf);
        CHECK(strcmp(buf, "Filter: Usable 2/3") == 0);   // rows < 0: count left to captionCountTo
    }
    // caption_longest_fits (WI-152): the 30-char longest caption fits kCaptionBytes with its NUL
    {
        char buf[isf::kCaptionBytes];
        captionWithCount(S_Upgrade, (1u << S_New) | (1u << S_Usable) | (1u << S_Upgrade), 1024, buf, sizeof buf);
        CHECK(strlen(buf) == 30 && strlen(buf) < sizeof buf);
    }
    // caption_truncation_nul (WI-152): a small cap truncates and stays NUL-terminated; cap 0 writes nothing
    {
        const unsigned all3 = (1u << S_New) | (1u << S_Usable) | (1u << S_Upgrade);
        char six[6] = {'X', 'X', 'X', 'X', 'X', 'X'};
        captionWithCount(S_Upgrade, all3, 1024, six, sizeof six);
        CHECK(six[5] == 0 && strcmp(six, "Filte") == 0);
        char one[1] = {'X'};
        captionWithCount(S_Upgrade, all3, 1024, one, sizeof one);
        CHECK(one[0] == 0);
        char none[1] = {'X'};
        captionWithCount(S_Upgrade, all3, 1024, none, 0);
        CHECK(none[0] == 'X');
    }
    printf(g_fail ? "sort_logic_test: %d FAILED\n" : "sort_logic_test: ALL PASS\n", g_fail);
    return g_fail ? 1 : 0;
}
