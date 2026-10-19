// combat_preview_test.cpp -- host tests for CombatPreview.h (CH6): the owner's edge-case matrix (no force powers, 1 item, 10 items, 16 each, all empty,
// one group only, unusable none/some/all) x 4 screens x HideEmpty 0/1 x per 8/12/16/32. Invariants come from the layout contract in CombatLogic.h,
// independent of the production implementation: rects on screen, buttons never overlap, a pill contains its buttons, counts capped at per.
#include "CombatPreview.h"
#include "check.h"
#include <algorithm>
using namespace chprev;

static bool overlaps(const ch::Rect& a, const ch::Rect& b) { return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h; }
static bool within(const ch::Rect& r, int w, int h) { return r.x >= 0 && r.y >= 0 && r.x + r.w <= w && r.y + r.h <= h; }
static bool contains(const ch::Rect& o, const ch::Rect& i) { return i.x >= o.x && i.y >= o.y && i.x + i.w <= o.x + o.w && i.y + i.h <= o.y + o.h; }

static int g_cases = 0;
static void checkModel(const Model& m, const Demo& d, const Settings& s, const char* tag) {
    ++g_cases;
    const int cols = std::min(4, std::max(1, s.cols)), mr = std::min(8, std::max(1, s.maxRows));
    KT_CHECK_EQ(m.per, cols * mr);
    int total = 0, visible = 0;
    for (int g = 0; g < 3; ++g) {
        const int want = std::min(std::max(d.count[g], 0), std::min(16, m.per));   // orderGroup yields at most 16 entries; the layout caps at per
        if (m.outN[g] != want) { printf("FAIL %s: outN[%d]=%d want %d\n", tag, g, m.outN[g], want); KT_CHECK_EQ(m.outN[g], want); }
        KT_CHECK(m.outN[g] <= m.per);
        total += m.outN[g];
        if (m.outN[g] > 0) ++visible;
        if (s.hideEmpty && m.outN[g] == 0) KT_CHECK(m.cont[g].w == 0 && m.cont[g].h == 0);   // an empty group's pill is hidden
        if (m.outN[g] > 0) {
            if (!within(m.cont[g], m.sw, m.sh)) { printf("FAIL %s: pill %d off screen {%d,%d,%d,%d} in %dx%d\n", tag, g, m.cont[g].x, m.cont[g].y, m.cont[g].w, m.cont[g].h, m.sw, m.sh); KT_CHECK(false); }
            for (int k = 0; k < m.outN[g]; ++k) {
                const ch::Rect& r = m.rects[g * m.per + k];
                KT_CHECK(r.w >= 1 && r.h == r.w);
                if (!within(r, m.sw, m.sh)) { printf("FAIL %s: button %d/%d off screen\n", tag, g, k); KT_CHECK(false); }
                if (!contains(m.cont[g], r)) { printf("FAIL %s: button %d/%d outside its pill\n", tag, g, k); KT_CHECK(false); }
                const int idx = g * m.per + k;
                KT_CHECK(m.usable[idx] == !entryUnusable(d.unusable, 0) || m.usable[idx] == !entryUnusable(d.unusable, 1));   // either parity
                KT_CHECK(m.alpha[idx] == (m.usable[idx] ? 1.f : kUnusableAlpha));
            }
        }
    }
    // no button overlaps another, across all groups
    for (int a = 0; a < 3; ++a) for (int i = 0; i < m.outN[a]; ++i)
        for (int b = a; b < 3; ++b) for (int j = (b == a ? i + 1 : 0); j < m.outN[b]; ++j)
            if (overlaps(m.rects[a * m.per + i], m.rects[b * m.per + j])) { printf("FAIL %s: buttons %d/%d and %d/%d overlap\n", tag, a, i, b, j); KT_CHECK(false); }
    // pills never overlap each other
    for (int a = 0; a < 3; ++a) for (int b = a + 1; b < 3; ++b)
        if (m.outN[a] > 0 && m.outN[b] > 0 && overlaps(m.cont[a], m.cont[b])) { printf("FAIL %s: pills %d and %d overlap\n", tag, a, b); KT_CHECK(false); }
    (void)total; (void)visible;
}

int main() {
    // ---- screens table ----
    KT_CHECK_EQ(kNumScreens, 4);
    KT_CHECK_EQ(kScreens[0].w, 1280); KT_CHECK_EQ(kScreens[1].h, 1080); KT_CHECK_EQ(kScreens[2].w, 2560); KT_CHECK_EQ(kScreens[3].w, 3440);
    // ---- 1080p production rects: banner {760,40,400,20}, boxes {760,64,400,56} ----
    { ch::Rect b, u; screenRects(1920, 1080, &b, &u);
      KT_CHECK_EQ(b.x, 760); KT_CHECK_EQ(b.y, 40); KT_CHECK_EQ(b.w, 400); KT_CHECK_EQ(b.h, 20);
      KT_CHECK_EQ(u.x, 760); KT_CHECK_EQ(u.y, 64); KT_CHECK_EQ(u.w, 400); KT_CHECK_EQ(u.h, 56);
      screenRects(3440, 1440, &b, &u);   // 4/3 scale, centred on 1720
      KT_CHECK_EQ(b.w, 533); KT_CHECK_EQ(b.y, 53); KT_CHECK_EQ(b.x + b.w / 2, 1720); KT_CHECK_EQ(u.y, 85); }
    // ---- all empty: nothing visible, everything zero ----
    { Demo d; d.count[0] = d.count[1] = d.count[2] = 0; Settings s; Model m = build(d, s);
      KT_CHECK_EQ(m.outN[0] + m.outN[1] + m.outN[2], 0); KT_CHECK(m.cont[0].w == 0 && m.cont[1].w == 0 && m.cont[2].w == 0); }
    // ---- single item in an 8-slot pill: one lone pill takes all 6 columns of the span, so the 64px auto button shrinks to fit = (400 - 2*16 - 5*8)/6 = 54 ----
    { Demo d; d.count[0] = d.count[1] = 0; d.count[2] = 1; Settings s; Model m = build(d, s);
      KT_CHECK_EQ(m.per, 8); KT_CHECK_EQ(m.outN[2], 1); KT_CHECK_EQ(m.btnPx, 64); KT_CHECK_EQ(m.rects[2 * 8].w, 54);
      KT_CHECK_EQ(m.cont[2].w, 2 * 16 + 54); KT_CHECK_EQ(m.cont[2].h, 2 * 16 + 54);
      KT_CHECK_EQ(m.rects[2 * 8].x + 27, m.banner.x + m.banner.w / 2); }   // the lone pill is centred on the banner
    // ---- explicit 10+ items in a pill with per 12: ten buttons, two columns wide each ----
    { Demo d; d.count[0] = d.count[1] = 0; d.count[2] = 10; Settings s; s.cols = 3; s.maxRows = 4; Model m = build(d, s);
      KT_CHECK_EQ(m.per, 12); KT_CHECK_EQ(m.outN[2], 10); checkModel(m, d, s, "10items"); }
    // ---- 16 in every group: capped at per for per 8 / 12, all 16 at per 16 / 32 ----
    { Demo d; d.count[0] = d.count[1] = d.count[2] = 16; const int perCfg[4][2] = {{2, 4}, {3, 4}, {4, 4}, {4, 8}}, wantN[4] = {8, 12, 16, 16};
      for (int c = 0; c < 4; ++c) { Settings s; s.cols = perCfg[c][0]; s.maxRows = perCfg[c][1]; Model m = build(d, s);
        for (int g = 0; g < 3; ++g) KT_CHECK_EQ(m.outN[g], wantN[c]); } }
    // ---- unusable: item order is by stack count (live orderGroup), unusable alpha is 0.35 ----
    { Demo d; d.count[0] = d.count[1] = 0; d.count[2] = 4; d.unusable = UNUSABLE_ALL; Settings s; Model m = build(d, s);
      for (int k = 0; k < 4; ++k) { KT_CHECK(!m.usable[2 * m.per + k]); KT_CHECK(m.alpha[2 * m.per + k] == 0.35f); }
      d.unusable = UNUSABLE_NONE; m = build(d, s); for (int k = 0; k < 4; ++k) { KT_CHECK(m.usable[2 * m.per + k]); KT_CHECK(m.alpha[2 * m.per + k] == 1.f); }
      d.unusable = UNUSABLE_SOME; d.count[2] = 0; d.count[0] = 4; m = build(d, s);
      int nu = 0; for (int k = 0; k < 4; ++k) if (!m.usable[k]) ++nu; KT_CHECK_EQ(nu, 2); }
    // ---- HideEmpty=0: three equal columns even when two groups are empty ----
    { Demo d; d.count[0] = 0; d.count[1] = 0; d.count[2] = 3; Settings s; s.hideEmpty = 0; Model m = build(d, s);
      KT_CHECK(m.cont[0].w > 0 && m.cont[0].w == m.cont[1].w && m.cont[1].w == m.cont[2].w); KT_CHECK_EQ(m.outN[2], 3); }
    // ---- the owner matrix x screens x HideEmpty x per ----
    static const int kCounts[][3] = {
        {0, 0, 0},      // all empty
        {4, 0, 3},      // no force powers
        {4, 3, 1},      // exactly one item
        {4, 3, 10},     // ten items
        {16, 16, 16},   // everything full
        {0, 0, 5}, {5, 0, 0}, {0, 5, 0},   // one group only
        {1, 1, 1}, {8, 8, 8}, {12, 0, 16},
    };
    static const int kPers[][2] = {{2, 4}, {3, 4}, {4, 4}, {4, 8}, {1, 1}};
    for (const auto& cn : kCounts) for (int u = 0; u < 3; ++u) for (int scr = 0; scr < kNumScreens; ++scr) for (int he = 0; he < 2; ++he) for (const auto& pr : kPers) for (int bp = 0; bp < 2; ++bp) {
        Demo d; for (int g = 0; g < 3; ++g) d.count[g] = cn[g]; d.unusable = u; d.screen = scr;
        Settings s; s.hideEmpty = he; s.cols = pr[0]; s.maxRows = pr[1]; s.btnPx = bp ? 32 : 0;
        char tag[96]; snprintf(tag, sizeof tag, "c=%d,%d,%d u=%d scr=%d he=%d per=%dx%d btn=%d", cn[0], cn[1], cn[2], u, scr, he, pr[0], pr[1], s.btnPx);
        Model m = build(d, s); checkModel(m, d, s, tag);
    }
    // ---- page rows change the layout: bigger spacing/size shrink-to-fit still stays on screen ----
    { Demo d; d.count[0] = 8; d.count[1] = 8; d.count[2] = 8; d.screen = 0; Settings s; s.cols = 4; s.maxRows = 8; s.btnPx = 64; s.pad = 16; s.groupGap = 64; s.capPad = 64; s.sidePad = 64;
      Model m = build(d, s); checkModel(m, d, s, "max-spacing-720p"); }
    // out-of-range demo inputs are clamped, never crash
    { Demo d; d.count[0] = -5; d.count[1] = 99; d.count[2] = 3; d.screen = 9; Settings s; s.cols = 0; s.maxRows = 99; Model m = build(d, s);
      KT_CHECK_EQ(m.sw, 3440); KT_CHECK_EQ(m.per, 1 * 8); KT_CHECK_EQ(m.outN[0], 0); KT_CHECK_EQ(m.outN[1], 8); }
    KT_CHECK(g_cases > 1000);
    return kt::Report("combat_preview_test");
}
