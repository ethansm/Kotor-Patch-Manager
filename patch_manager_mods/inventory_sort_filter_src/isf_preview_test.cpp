// Host unit tests for IsfPreview.h:  g++ -std=c++17 -Wall -Wextra -Werror -o isf_preview_test isf_preview_test.cpp && ./isf_preview_test
#include "IsfPreview.h"
#include <stdio.h>
#include <string.h>
using namespace isfprev;

static int g_fail = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

static int countKind(const Prim* p, int n, int kind) {
    int c = 0;
    for (int i = 0; i < n; ++i) if (p[i].kind == kind) ++c;
    return c;
}
static int countStrip(const Prim* p, int n) {
    return countKind(p, n, PK_STRIP) + countKind(p, n, PK_STRIP_TEXT1) + countKind(p, n, PK_STRIP_TEXT2);
}
static int firstRowY(const Prim* p, int n) {
    for (int i = 0; i < n; ++i) if (p[i].kind == PK_ROW) return p[i].y;
    return -1;
}

int main() {
    // 1. Each order: rows match isf::sortEntries on the demo entries (vanilla = demo order); with sort on, the new item leads.
    const isf::SortOrder orders[] = {isf::SO_Category, isf::SO_Base, isf::SO_Name, isf::SO_Vanilla};
    for (isf::SortOrder o : orders) {
        Settings s;
        s.order = o;
        s.cycle = false;
        Model m;
        build(s, &m);
        isf::Entry ref[kDemoCount] = {};
        for (int i = 0; i < kDemoCount; ++i) fillEntry(i, &ref[i]);
        isf::sortEntries(ref, kDemoCount, o);
        CHECK(m.shown == kDemoCount && m.offered == kDemoCount);
        for (int i = 0; i < kDemoCount; ++i) CHECK(m.rows[i].item == ref[i].item);
        if (o != isf::SO_Vanilla) CHECK(m.rows[0].item == 1);   // Vibroblade is the only new item
    }

    // 2. Category order is the expected one (new, usable by category then base item, then unusable by category).
    {
        Settings s;
        s.cycle = false;
        Model m;
        build(s, &m);
        const int wantCat[kDemoCount] = {1, 2, 6, 7, 4, 3, 5, 8};   // Vibroblade, Blaster, Medpac, Adrenal, Stealth, Heavy, Crystal, Datapad
        for (int i = 0; i < kDemoCount; ++i) CHECK(m.rows[i].item == wantCat[i]);
    }

    // 3. Sort off: demo (vanilla) order and the line 2 text says Vanilla.
    {
        Settings s;
        s.sortOn = false;
        s.order = isf::SO_Name;
        s.cycle = false;
        Model m;
        build(s, &m);
        for (int i = 0; i < kDemoCount; ++i) CHECK(m.rows[i].item == i + 1);
        CHECK(strcmp(m.line2, "Sort: Vanilla    8 items") == 0);
    }

    // 4. Mask without Upgradeable: the cycle never lands on Upgradeable, and line 1 never names it (both New and Usable do get visited).
    {
        Settings s;
        s.mask = (1u << isf::S_New) | (1u << isf::S_Usable);
        bool okState = true, okLine = true, sawNew = false, sawUsable = false;
        for (int t = 0; t <= 2000; ++t) {
            s.tick = t;
            Model m;
            build(s, &m);
            if (m.active == isf::S_Upgrade) okState = false;
            if (strstr(m.line1, "Upgradeable")) okLine = false;
            if (m.active == isf::S_New) sawNew = true;
            if (m.active == isf::S_Usable) sawUsable = true;
        }
        CHECK(okState);
        CHECK(okLine);
        CHECK(sawNew && sawUsable);
    }

    // 5. Filters: New shows only Vibroblade; Upgradeable shows only the Lightsaber Crystal and flags it in upg[].
    {
        Settings s;
        s.tick = 90;   // [All, New, Usable, Upgrade]: index 1
        Model m;
        build(s, &m);
        CHECK(m.active == isf::S_New);
        CHECK(m.shown == 1 && strcmp(m.rows[0].name, "Vibroblade") == 0);
        s.tick = 270;  // index 3
        build(s, &m);
        CHECK(m.active == isf::S_Upgrade);
        CHECK(m.shown == 1 && strcmp(m.rows[0].name, "Lightsaber Crystal") == 0 && m.upg[0]);
    }

    // 6. All state: 8 shown of 8 offered, the pane draws the 7-row cap, and every row has its icon and name label.
    {
        Settings s;
        Model m;
        build(s, &m);
        CHECK(m.active == isf::S_All);
        CHECK(m.shown == 8 && m.offered == 8);
        Prim p[kPoolSize] = {};
        const int n = buildPrims(m, p, kPoolSize);
        CHECK(countKind(p, n, PK_ROW) == kMaxRows);
        CHECK(countKind(p, n, PK_ICON) == kMaxRows);
        CHECK(countKind(p, n, PK_NAME) == kMaxRows);
        CHECK(strcmp(m.line2, "Sort: Category    8 items") == 0);
    }

    // 7. Strip off: no strip prims, and the rows move up to the top of the pane.
    {
        Settings on;
        Model mOn;
        build(on, &mOn);
        Prim pOn[kPoolSize] = {};
        const int nOn = buildPrims(mOn, pOn, kPoolSize);
        Settings off = on;
        off.strip = false;
        Model mOff;
        build(off, &mOff);
        Prim pOff[kPoolSize] = {};
        const int nOff = buildPrims(mOff, pOff, kPoolSize);
        CHECK(countStrip(pOn, nOn) == 3);
        CHECK(countStrip(pOff, nOff) == 0);
        CHECK(firstRowY(pOff, nOff) < firstRowY(pOn, nOn));
    }

    // 8. Tint off: no tinted prim. Tint on: the Heavy Battle Armor name is the only tinted prim.
    {
        Settings s;
        s.tint = false;
        Model m;
        build(s, &m);
        Prim p[kPoolSize] = {};
        int n = buildPrims(m, p, kPoolSize);
        int tinted = 0;
        for (int i = 0; i < n; ++i) if (p[i].tinted) ++tinted;
        CHECK(tinted == 0);

        s.tint = true;
        build(s, &m);
        n = buildPrims(m, p, kPoolSize);
        int heavy = -1;
        for (int r = 0; r < m.shown; ++r) if (strcmp(m.rows[r].name, "Heavy Battle Armor") == 0) heavy = r;
        CHECK(heavy >= 0 && heavy < kMaxRows);
        tinted = 0;
        for (int i = 0; i < n; ++i) {
            if (!p[i].tinted) continue;
            ++tinted;
            CHECK(p[i].kind == PK_NAME && p[i].row == heavy);
        }
        CHECK(tinted == 1);
    }

    // 9. Label pool: every mask x strip x tint x cycle x tick combination stays inside kPoolSize with unique labels; the full pane fills it.
    {
        int worst = 0;
        bool ok = true;
        for (int mask = 0; mask < 8; ++mask)
        for (int strip = 0; strip < 2; ++strip)
        for (int tint = 0; tint < 2; ++tint)
        for (int cyc = 0; cyc < 2; ++cyc)
        for (int t = 0; t < 360; t += 90) {
            Settings s;
            s.mask = (unsigned)mask << 1;   // bit 1 = New, 2 = Usable, 3 = Upgradeable
            s.strip = strip != 0;
            s.tint = tint != 0;
            s.cycle = cyc != 0;
            s.tick = t;
            Model m;
            build(s, &m);
            Prim p[kPoolSize] = {};
            const int n = buildPrims(m, p, kPoolSize);
            bool seen[kPoolSize] = {};
            if (n > kPoolSize) ok = false;
            for (int i = 0; i < n; ++i) {
                const int label = p[i].label;
                if (label < 0 || label >= kPoolSize || seen[label]) ok = false;
                else seen[label] = true;
            }
            if (n > worst) worst = n;
        }
        CHECK(ok);
        CHECK(worst == kPoolSize);
    }

    // 10. Zero matches: no PK_ROW, the caption still draws, and line 2 reads "0 of 8 shown" (stripLine2 output, not a special case).
    {
        Settings s;
        s.mask = 1u << isf::S_New;
        s.tick = 90;   // [All, New]: index 1 = New
        Model m;
        build(s, &m);
        CHECK(m.active == isf::S_New && m.shown == 1);
        m.shown = 0;   // no item matched
        isf::stripLine2(s.order, s.sortOn, m.shown, m.offered, m.line2, (int)sizeof m.line2);
        Prim p[kPoolSize] = {};
        const int n = buildPrims(m, p, kPoolSize);
        CHECK(countKind(p, n, PK_ROW) == 0);
        CHECK(countKind(p, n, PK_CAPTION) == 1);
        CHECK(strcmp(m.line2, "Sort: Category    0 of 8 shown") == 0);
    }

    if (g_fail) {
        printf("isf_preview_test: %d check(s) FAILED\n", g_fail);
        return 1;
    }
    printf("isf_preview_test: all checks passed\n");
    return 0;
}
