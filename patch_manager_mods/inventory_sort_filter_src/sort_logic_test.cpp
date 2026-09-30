// Host unit tests for SortLogic.h:  g++ -std=c++17 -Wall -Wextra -o /tmp/sort_logic_test sort_logic_test.cpp && /tmp/sort_logic_test
#include "SortLogic.h"
#include <stdio.h>
#include <vector>
using namespace isf;

static int g_fail = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

int main() {
    // sort: new first, usable before unusable, base item ascending, stable
    std::vector<Entry> v = {
        {1, 1, 0, false, true, 50}, {2, 2, 1, false, false, 10}, {3, 3, 2, true, false, 99}, {4, 4, 3, false, true, 50},
        {5, 5, 4, false, true, 7},  {6, 6, 5, true, true, 3},    {7, 7, 6, false, false, 10},
    };
    sortEntries(v.data(), (int)v.size());
    int want[] = {6, 3, 5, 1, 4, 2, 7};
    for (size_t i = 0; i < v.size(); ++i) CHECK(v[i].item == want[i]);

    // stability / strict weak order: identical keys keep vanilla order
    std::vector<Entry> w; for (int i = 0; i < 100; ++i) w.push_back({i, i, i, false, true, i % 3});
    sortEntries(w.data(), (int)w.size());
    for (size_t i = 1; i < w.size(); ++i) CHECK(!before(w[i], w[i - 1]));
    int lastBase = -1, lastOrig = -1;
    for (auto& e : w) { if (e.baseItem != lastBase) { lastBase = e.baseItem; lastOrig = -1; } CHECK(e.orig > lastOrig); lastOrig = e.orig; }

    // cycle state machine, all states enabled
    unsigned all = (1u << S_New) | (1u << S_Usable) | (1u << S_Upgrade);
    State s = S_All; bool up = false; int mode = 0;
    auto click = [&](int clicked, unsigned mask) {
        State n;
        if (!onTabClick(clicked, mode, up, mask, &n)) return false;
        mode = engineModeOf(n); up = (n == S_Upgrade); s = n; return true;
    };
    CHECK(click(0, all) && s == S_New && mode == 1 && !up);
    CHECK(click(0, all) && s == S_Usable && mode == 5 && !up);
    CHECK(click(0, all) && s == S_Upgrade && mode == 0 && up);
    CHECK(click(0, all) && s == S_All && mode == 0 && !up);
    // another tab is never consumed, even inside the cycle
    mode = 1; CHECK(!click(3, all)); CHECK(!click(1, all));
    // a mode outside the cycle (weapons tab = 2 etc.) is never consumed, even for clicked==0
    mode = 2; up = false; CHECK(!click(0, all));
    mode = 7; CHECK(!click(0, all));
    // subset masks skip disabled states
    mode = 0; up = false; s = S_All;
    CHECK(click(0, 1u << S_Upgrade) && s == S_Upgrade);
    CHECK(click(0, 1u << S_Upgrade) && s == S_All);
    // empty mask: All stays All, click not consumed (vanilla handles it)
    mode = 0; up = false; CHECK(!click(0, 0));
    // parse
    CHECK(parseCycleStates("new,usable,upgrade") == all);
    CHECK(parseCycleStates(" NEW , Upgradeable ") == ((1u << S_New) | (1u << S_Upgrade)));
    CHECK(parseCycleStates("") == 0);
    CHECK(parseCycleStates("bogus,usable") == (1u << S_Usable));
    CHECK(ieq("Abc", "aBC") && !ieq("ab", "abc"));
    // captions
    CHECK(!strcmp(captionOf(S_All), "Filter") && !strcmp(captionOf(S_Upgrade), "Filter: Upgradeable"));
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
    printf(g_fail ? "sort_logic_test: %d FAILED\n" : "sort_logic_test: ALL PASS\n", g_fail);
    return g_fail ? 1 : 0;
}
