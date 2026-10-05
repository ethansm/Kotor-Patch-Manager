// combat_logic_test.cpp -- host tests for CombatLogic.h (invoke-entry pick, key edge, slot fill, row layout, hit test).
#include "CombatLogic.h"
#include "check.h"
#include <cstring>
#include <string>
using namespace ch;

static EntryView mk(uint32_t key, bool usable, uint32_t owner, const char* icon = "ic") {
  EntryView e; memset(&e, 0, sizeof e); e.key = key; e.usable = usable; e.owner = owner;
  strncpy(e.icon, icon, 16); return e;
}

static bool inside(const Rect& r, int lo, int hi) { return r.x >= lo && r.x + r.w <= hi; }
static bool overlap(const Rect& a, const Rect& b) { return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h; }

static void checkLayout(int W, int H, Rect banner, int n, int btn, int gap, const char* tag) {
  Rect r[8]; layoutRow(W, H, banner, n, btn, gap, r);
  int safeW = H * 4 / 3, lo = 0, hi = W;
  if (W > safeW) { lo = (W - safeW) / 2; hi = lo + safeW; }
  bool allIn = true, noOverlap = true, below = true;
  for (int i = 0; i < n; ++i) {
    allIn = allIn && inside(r[i], lo, hi);
    below = below && r[i].y == banner.y + banner.h + gap && r[i].w == btn && r[i].h == btn;
    for (int j = i + 1; j < n; ++j) noOverlap = noOverlap && !overlap(r[i], r[j]);
  }
  kt::Check((std::string(tag) + ": inside safe area").c_str(), allIn);
  kt::Check((std::string(tag) + ": no overlap").c_str(), noOverlap);
  kt::Check((std::string(tag) + ": below banner, square").c_str(), below);
}

int main() {
  // ---- pickInvokeEntry
  {
    GroupView g; memset(&g, 0, sizeof g);
    g.count = 4; g.e[0] = mk(1, true, 5); g.e[1] = mk(2, true, 6); g.e[2] = mk(3, false, 7); g.e[3] = mk(4, true, 0);
    KT_CHECK_EQ(pickInvokeEntry(g, 0), 1);   // last usable with owner: e[3] owner 0 skipped, e[2] unusable skipped
    KT_CHECK_EQ(pickInvokeEntry(g, 3), 1);
    KT_CHECK_EQ(pickInvokeEntry(g, 0xFF), -1);
    KT_CHECK_EQ(pickInvokeEntry(g, 4), -1);
    KT_CHECK_EQ(pickInvokeEntry(g, -1), -1);
    g.e[3] = mk(4, true, 9);
    KT_CHECK_EQ(pickInvokeEntry(g, 2), 3);
    g.e[0].usable = g.e[1].usable = g.e[3].usable = false;
    KT_CHECK_EQ(pickInvokeEntry(g, 1), -1);   // nothing usable
    GroupView e; memset(&e, 0, sizeof e);
    KT_CHECK_EQ(pickInvokeEntry(e, 0), -1);   // empty
    GroupView o; memset(&o, 0, sizeof o); o.count = 2; o.e[0] = mk(1, true, 0); o.e[1] = mk(2, true, 0);
    KT_CHECK_EQ(pickInvokeEntry(o, 0), -1);   // all owner 0
    GroupView c; memset(&c, 0, sizeof c); c.count = 99;   // clamped to 16
    for (int i = 0; i < 16; ++i) c.e[i] = mk(i, true, 1);
    KT_CHECK_EQ(pickInvokeEntry(c, 0), 15);
    c.count = -3;
    KT_CHECK_EQ(pickInvokeEntry(c, 0), -1);
  }

  // ---- KeyEdge
  {
    KeyEdge k;
    KT_CHECK(!k.update(false));
    KT_CHECK(k.update(true));    // down
    KT_CHECK(!k.update(true));   // still down
    KT_CHECK(!k.update(false));  // up
    KT_CHECK(k.update(true));    // down again
    KT_CHECK(!k.update(true));
  }

  // ---- fillSlots
  {
    GroupView g[3]; memset(g, 0, sizeof g);
    g[0].count = 3; g[1].count = 0; g[2].count = 20;   // 20 clamps to 16
    for (int i = 0; i < 3; ++i) g[0].e[i] = mk(100 + i, i != 1, 1, "a");
    for (int i = 0; i < 16; ++i) g[2].e[i] = mk(300 + i, true, 1, "c");
    Slot s[64];
    int n = fillSlots(g, s, 64, 4);
    KT_CHECK_EQ(n, 3 + 4);   // group0: 3 (< perGroup), group1: 0, group2: capped at 4
    KT_CHECK_EQ(s[0].group, 0); KT_CHECK_EQ(s[0].key, 100u); KT_CHECK_EQ(s[1].key, 101u); KT_CHECK(!s[1].usable);
    KT_CHECK_EQ(s[2].key, 102u);
    KT_CHECK_EQ(s[3].group, 2); KT_CHECK_EQ(s[3].key, 300u); KT_CHECK_EQ(s[6].key, 303u);
    KT_CHECK(strcmp(s[3].icon, "c") == 0);
    n = fillSlots(g, s, 64, 100);
    KT_CHECK_EQ(n, 3 + 16);   // perGroup beyond count clamps to count
    n = fillSlots(g, s, 5, 4);   // maxSlots cap
    KT_CHECK_EQ(n, 5); KT_CHECK_EQ(s[4].group, 2); KT_CHECK_EQ(s[4].key, 301u);
    KT_CHECK_EQ(fillSlots(g, s, 0, 4), 0);
    KT_CHECK_EQ(fillSlots(g, s, 8, 0), 0);
    // icon NUL termination: source filled with 16 non-NUL chars (+ a poisoned 17th byte)
    GroupView h[3]; memset(h, 0, sizeof h); h[1].count = 1;
    memset(h[1].e[0].icon, 'x', 16); h[1].e[0].icon[16] = 'Z';
    Slot t[2]; memset(t, 0x7F, sizeof t);
    KT_CHECK_EQ(fillSlots(h, t, 2, 4), 1);
    KT_CHECK_EQ(t[0].group, 1);
    KT_CHECK_EQ((int)strlen(t[0].icon), 16);
    KT_CHECK_EQ((int)t[0].icon[16], 0);
  }

  // ---- layoutRow
  {
    checkLayout(1920, 1080, Rect{760, 900, 400, 40}, 6, 48, 6, "1920x1080 centred");
    checkLayout(3440, 1440, Rect{1520, 1200, 400, 40}, 6, 64, 8, "3440x1440 centred");
    checkLayout(2560, 1080, Rect{1080, 900, 400, 40}, 6, 48, 6, "2560x1080 centred");
    checkLayout(3440, 1440, Rect{0, 1200, 300, 40}, 6, 64, 8, "3440x1440 banner at far left");
    checkLayout(2560, 1080, Rect{300, 900, 200, 40}, 6, 48, 6, "2560x1080 banner left of safe area");
    checkLayout(2560, 1080, Rect{2300, 900, 250, 40}, 6, 48, 6, "2560x1080 banner right edge");
    checkLayout(1920, 1080, Rect{0, 900, 100, 40}, 6, 48, 6, "1920x1080 banner left edge");
    checkLayout(1024, 768, Rect{0, 700, 100, 40}, 4, 48, 6, "1024x768 4:3");

    Rect r[6];
    layoutRow(1920, 1080, Rect{760, 900, 400, 40}, 6, 48, 6, r);   // 4:3 safe area = [240,1680]; row width 6*48+5*6 = 318, centre 960
    KT_CHECK_EQ(r[0].x, 960 - 159);
    KT_CHECK_EQ(r[1].x - r[0].x, 54);
    KT_CHECK_EQ(r[0].y, 946);
    layoutRow(2560, 1080, Rect{300, 900, 200, 40}, 6, 48, 6, r);   // safe = [560,2000]: clamped to its left edge
    KT_CHECK_EQ(r[0].x, 560);
    layoutRow(2560, 1080, Rect{2300, 900, 250, 40}, 6, 48, 6, r);
    KT_CHECK_EQ(r[5].x + r[5].w, 2000);   // clamped to its right edge
    layoutRow(1920, 1080, Rect{0, 900, 100, 40}, 6, 48, 6, r);
    KT_CHECK(r[0].x >= 240);
    layoutRow(1024, 768, Rect{0, 700, 100, 40}, 4, 48, 6, r);   // screen exactly 4:3 -> [0, screenW]
    KT_CHECK_EQ(r[0].x, 0);
  }

  // ---- hitSlot
  {
    Rect r[3]; layoutRow(1920, 1080, Rect{760, 900, 400, 40}, 3, 48, 6, r);
    KT_CHECK_EQ(hitSlot(r, 3, r[0].x, r[0].y), 0);                          // top-left corner inclusive
    KT_CHECK_EQ(hitSlot(r, 3, r[1].x + 10, r[1].y + 10), 1);
    KT_CHECK_EQ(hitSlot(r, 3, r[2].x + 47, r[2].y + 47), 2);                // last inside pixel
    KT_CHECK_EQ(hitSlot(r, 3, r[2].x + 48, r[2].y + 10), -1);               // right edge exclusive
    KT_CHECK_EQ(hitSlot(r, 3, r[2].x + 10, r[2].y + 48), -1);               // bottom edge exclusive
    KT_CHECK_EQ(hitSlot(r, 3, r[0].x - 1, r[0].y + 5), -1);                 // left of row
    KT_CHECK_EQ(hitSlot(r, 3, r[0].x + 5, r[0].y - 1), -1);                 // above
    KT_CHECK_EQ(hitSlot(r, 3, r[0].x + 48, r[0].y + 5), -1);                // gap between 0 and 1
    KT_CHECK_EQ(hitSlot(r, 3, r[0].x + 53, r[0].y + 5), -1);                // last gap pixel
    KT_CHECK_EQ(hitSlot(r, 3, r[1].x, r[1].y), 1);                          // first pixel of next button
    KT_CHECK_EQ(hitSlot(r, 0, r[0].x, r[0].y), -1);
    KT_CHECK_EQ(hitSlot(nullptr, 3, 0, 0), -1);
  }
  // ---- layoutGroupGrid / slotEntry
  {
    const Rect box[3] = {{100, 50, 56, 56}, {160, 50, 56, 56}, {220, 50, 56, 56}};
    const int cols = 2, maxRows = 4, gap = 2, per = cols * maxRows;   // btn = (56-2)/2 = 27
    Rect out[3 * 8]; int n[3];
    const int c1[3] = {4, 0, 3};
    layoutGroupGrid(box, c1, cols, maxRows, gap, 1920, 1080, out, n);
    KT_CHECK_EQ(n[0], 4); KT_CHECK_EQ(n[1], 0); KT_CHECK_EQ(n[2], 3);
    KT_CHECK_EQ(out[0].x, 100); KT_CHECK_EQ(out[0].y, 108); KT_CHECK_EQ(out[0].w, 27); KT_CHECK_EQ(out[0].h, 27);
    KT_CHECK_EQ(out[3].x, 100 + 29); KT_CHECK_EQ(out[3].y, 108 + 29);          // group 0 last slot: col 1, row 1
    KT_CHECK_EQ(out[2 * per + 2].x, 220); KT_CHECK_EQ(out[2 * per + 2].y, 108 + 29);   // group 2 last slot: col 0, row 1
    KT_CHECK_EQ(out[2 * per + 1].x, 220 + 29); KT_CHECK_EQ(out[2 * per + 1].y, 108);
    KT_CHECK(out[per].w == 0 && out[per].h == 0 && out[per + 5].x == 0);          // empty group zeroed
    KT_CHECK(out[4].w == 0 && out[2 * per + 3].w == 0);                           // unused tail zeroed

    const int c2[3] = {16, -5, 99};
    layoutGroupGrid(box, c2, cols, maxRows, gap, 1920, 1080, out, n);
    KT_CHECK_EQ(n[0], 8); KT_CHECK_EQ(n[1], 0); KT_CHECK_EQ(n[2], 8);             // capped to cols*maxRows
    KT_CHECK_EQ(out[7].x, 100 + 29); KT_CHECK_EQ(out[7].y, 108 + 3 * 29);         // 4 rows

    // no overlap among all used slots, within and across groups
    const int c3[3] = {8, 8, 8};
    layoutGroupGrid(box, c3, cols, maxRows, gap, 1920, 1080, out, n);
    bool noOv = true; int used = 0;
    for (int i = 0; i < 3 * per; ++i)
      for (int j = i + 1; j < 3 * per; ++j)
        if (out[i].w > 0 && out[j].w > 0) noOv = noOv && !overlap(out[i], out[j]);
    for (int i = 0; i < 3 * per; ++i) used += out[i].w > 0;
    KT_CHECK(noOv); KT_CHECK_EQ(used, 24);

    // bottom edge clamp: grid shifted up, size unchanged
    const Rect low[3] = {{100, 1040, 56, 56}, {160, 50, 56, 56}, {220, 50, 56, 56}};
    const int c4[3] = {4, 0, 0};
    layoutGroupGrid(low, c4, cols, maxRows, gap, 1920, 1080, out, n);
    KT_CHECK_EQ(out[0].w, 27); KT_CHECK_EQ(out[3].h, 27);
    KT_CHECK_EQ(out[3].y + out[3].h, 1080);                                       // last row flush with the bottom
    KT_CHECK_EQ(out[3].y - out[0].y, 29);                                         // row pitch preserved
    KT_CHECK_EQ(out[0].x, 100);
    // right edge clamp shifts left
    const Rect rt[3] = {{1900, 50, 56, 56}, {160, 50, 56, 56}, {220, 50, 56, 56}};
    layoutGroupGrid(rt, c4, cols, maxRows, gap, 1920, 1080, out, n);
    KT_CHECK_EQ(out[1].x + out[1].w, 1920);

    // hitSlot over the grid
    layoutGroupGrid(box, c1, cols, maxRows, gap, 1920, 1080, out, n);
    KT_CHECK_EQ(hitSlot(out, 3 * per, out[3].x + 13, out[3].y + 13), 3);
    KT_CHECK_EQ(hitSlot(out, 3 * per, out[2 * per + 1].x + 13, out[2 * per + 1].y + 13), 2 * per + 1);
    KT_CHECK_EQ(hitSlot(out, 3 * per, out[0].x + 27, out[0].y + 5), -1);          // gap between col 0 and col 1
    KT_CHECK_EQ(hitSlot(out, 3 * per, out[0].x + 5, out[0].y + 28), -1);          // gap between row 0 and row 1

    // slotEntry round-trip
    bool rt_ok = true;
    for (int s = 0; s < 3 * per; ++s) {
      int g = -9; int e = slotEntry(s, cols, maxRows, &g);
      rt_ok = rt_ok && e >= 0 && e < per && g >= 0 && g < 3 && g * per + e == s;
    }
    KT_CHECK(rt_ok);
    int g = 7;
    KT_CHECK_EQ(slotEntry(-1, cols, maxRows, &g), -1); KT_CHECK_EQ(g, -1);
    g = 7; KT_CHECK_EQ(slotEntry(3 * per, cols, maxRows, &g), -1); KT_CHECK_EQ(g, -1);
    g = 7; KT_CHECK_EQ(slotEntry(per, cols, maxRows, &g), 0); KT_CHECK_EQ(g, 1);
  }
  // ---- layoutBannerGrid / gridBounds / freezeOrigin
  {
    const int cols = 2, maxRows = 4, per = cols * maxRows;
    Rect out[3 * per]; int n[3];
    const Rect boxes = {100, 50, 300, 56};
    const int c403[3] = {4, 0, 3};
    // banner wider than the boxes -> span follows the banner, centred on it
    const Rect wide = {50, 20, 600, 30};
    layoutBannerGrid(wide, boxes, c403, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    Rect b = gridBounds(out, n, per);
    KT_CHECK(b.w > boxes.w);
    KT_CHECK(b.x >= 50 && b.x + b.w <= 650);
    KT_CHECK_EQ(out[0].w, 28); KT_CHECK_EQ(out[0].h, 28);
    KT_CHECK_EQ(out[0].y, 50);                                    // starts at boxesUnion.y
    KT_CHECK(out[per].x == 0 && out[per].w == 0);                 // empty group zeroed
    KT_CHECK(out[2 * per].x > out[0].x + 28 * 2);                 // group 2 is in a later column
    // counts 4/0/3
    KT_CHECK_EQ(n[0], 4); KT_CHECK_EQ(n[1], 0); KT_CHECK_EQ(n[2], 3);
    // no overlaps
    {
      bool ok = true;
      for (int g = 0; g < 3; ++g) for (int i = 0; i < n[g]; ++i)
        for (int h = 0; h < 3; ++h) for (int j = 0; j < n[h]; ++j)
          if ((g != h || i != j) && overlap(out[g * per + i], out[h * per + j])) ok = false;
      KT_CHECK(ok);
    }
    // narrower banner -> span follows the boxes
    const Rect narrow = {150, 20, 100, 30};
    const int c1[3] = {2, 2, 2};
    layoutBannerGrid(narrow, boxes, c1, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    b = gridBounds(out, n, per);
    KT_CHECK(b.x >= 50 && b.x + b.w <= 350);               // span 300 centred on the banner centre (200)
    // cap at maxRows, negative counts clamp to 0
    const int c20[3] = {20, -5, 9};
    layoutBannerGrid(wide, boxes, c20, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    KT_CHECK_EQ(n[0], 8); KT_CHECK_EQ(n[1], 0); KT_CHECK_EQ(n[2], 8);
    KT_CHECK_EQ(out[7].y, 50 + 3 * 30);                           // row 3 of group 0
    int grp = -1; KT_CHECK_EQ(slotEntry(2 * per + 3, cols, maxRows, &grp), 3); KT_CHECK_EQ(grp, 2);
    KT_CHECK_EQ(hitSlot(out, 3 * per, out[2 * per + 3].x + 5, out[2 * per + 3].y + 5), 2 * per + 3);
    // clamp at the left edge keeps the size
    const Rect leftB = {-100, 20, 300, 30};
    layoutBannerGrid(leftB, {-100, 50, 300, 56}, c403, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    b = gridBounds(out, n, per);
    KT_CHECK_EQ(b.x, 0); KT_CHECK_EQ(out[0].w, 28);
    // clamp at the right edge keeps the size, shifts all groups together
    layoutBannerGrid(wide, boxes, c403, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    const Rect before = gridBounds(out, n, per);
    const Rect rightB = {1700, 20, 600, 30};
    layoutBannerGrid(rightB, {1700, 50, 300, 56}, c403, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    b = gridBounds(out, n, per);
    KT_CHECK_EQ(b.x + b.w, 1920); KT_CHECK_EQ(b.w, before.w); KT_CHECK_EQ(out[0].w, 28);
    // bottom edge shifts up, top edge shifts down
    layoutBannerGrid(wide, {100, 1070, 300, 56}, c403, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    b = gridBounds(out, n, per);
    KT_CHECK_EQ(b.y + b.h, 1080);
    layoutBannerGrid(wide, {100, -40, 300, 56}, c403, cols, maxRows, 28, 2, 6, 1920, 1080, out, n);
    KT_CHECK_EQ(gridBounds(out, n, per).y, 0);
    // btnPx = 0 -> auto-fit; btnPx larger than fit is capped
    layoutBannerGrid(wide, boxes, c403, cols, maxRows, 0, 2, 6, 1920, 1080, out, n);
    KT_CHECK_EQ(out[0].w, 97); KT_CHECK_EQ(out[0].h, 97);         // colW (600-12)/3 = 196, fit (196-2)/2 = 97
    layoutBannerGrid(wide, boxes, c403, cols, maxRows, 500, 2, 6, 1920, 1080, out, n);
    KT_CHECK_EQ(out[0].w, 97);
    // degenerate input
    layoutBannerGrid(wide, boxes, c403, 0, maxRows, 28, 2, 6, 1920, 1080, out, n);
    KT_CHECK_EQ(n[0], 0); KT_CHECK_EQ(n[2], 0);

    // gridBounds
    const int none[3] = {0, 0, 0};
    b = gridBounds(out, none, per);
    KT_CHECK_EQ(b.w, 0); KT_CHECK_EQ(b.h, 0);
    Rect g2[3 * 2] = {};
    g2[0] = {10, 20, 5, 5}; g2[1] = {30, 25, 5, 5}; g2[4] = {0, 40, 8, 8};
    const int n2[3] = {2, 0, 1};
    b = gridBounds(g2, n2, 2);
    KT_CHECK_EQ(b.x, 0); KT_CHECK_EQ(b.y, 20); KT_CHECK_EQ(b.w, 35); KT_CHECK_EQ(b.h, 28);

    // freezeOrigin: holds the entry point while hovering, releases on exit, re-captures on re-entry
    FreezeState fs = {false, 0, 0};
    int ox = -1, oy = -1;
    KT_CHECK(!freezeOrigin(fs, false, 5, 6, &ox, &oy)); KT_CHECK_EQ(ox, 5); KT_CHECK_EQ(oy, 6);
    KT_CHECK(freezeOrigin(fs, true, 100, 200, &ox, &oy)); KT_CHECK_EQ(ox, 100); KT_CHECK_EQ(oy, 200);
    KT_CHECK(freezeOrigin(fs, true, 130, 210, &ox, &oy)); KT_CHECK_EQ(ox, 100); KT_CHECK_EQ(oy, 200);
    KT_CHECK(fs.active);
    KT_CHECK(!freezeOrigin(fs, false, 300, 400, &ox, &oy)); KT_CHECK_EQ(ox, 300); KT_CHECK_EQ(oy, 400);
    KT_CHECK(!fs.active);
    KT_CHECK(freezeOrigin(fs, true, 7, 8, &ox, &oy)); KT_CHECK_EQ(ox, 7); KT_CHECK_EQ(oy, 8);
  }
  // ---- layoutBannerGroups (pill containers)
  {
    const int cols = 2, maxRows = 4, per = cols * maxRows, pad = 2, gap = 6, cap = 4, side = 3, btnPx = 28;
    Rect out[3 * per], cont[3]; int n[3];
    const Rect wide = {50, 20, 600, 30}, boxes = {100, 50, 300, 56};   // span 600, colW (600-12)/3 = 196
    const int c403[3] = {4, 0, 3};
    layoutBannerGroups(wide, boxes, c403, cols, maxRows, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    KT_CHECK_EQ(n[0], 4); KT_CHECK_EQ(n[1], 0); KT_CHECK_EQ(n[2], 3);
    // every button inside its own container, at least cap from top and bottom
    bool inCont = true;
    for (int g = 0; g < 3; ++g) for (int i = 0; i < n[g]; ++i) {
      const Rect& r = out[g * per + i];
      inCont = inCont && r.x >= cont[g].x && r.x + r.w <= cont[g].x + cont[g].w
                      && r.y - cont[g].y >= cap && (cont[g].y + cont[g].h) - (r.y + r.h) >= cap;
    }
    KT_CHECK(inCont);
    // equal heights, no overlap between containers
    KT_CHECK_EQ(cont[0].h, cont[1].h); KT_CHECK_EQ(cont[1].h, cont[2].h);
    KT_CHECK(!overlap(cont[0], cont[1]) && !overlap(cont[1], cont[2]) && !overlap(cont[0], cont[2]));
    KT_CHECK_EQ(cont[1].x - (cont[0].x + cont[0].w), gap);
    // empty group still gets a container; unused slots zeroed
    KT_CHECK(cont[1].w > 0 && cont[1].h > 0);
    KT_CHECK_EQ(cont[1].y, boxes.y);
    KT_CHECK(out[per].w == 0 && out[per].x == 0 && out[4].w == 0 && out[2 * per + 3].w == 0);
    // counts 4/0/3 with cols 2 -> 2 rows
    KT_CHECK_EQ(cont[0].h, 2 * cap + 2 * btnPx + pad);
    KT_CHECK_EQ(out[0].y, boxes.y + cap); KT_CHECK_EQ(out[3].y, boxes.y + cap + btnPx + pad);
    // count above cols*maxRows caps at cols*maxRows (and the container grows to maxRows rows)
    const int cBig[3] = {99, -3, 9};
    layoutBannerGroups(wide, boxes, cBig, cols, maxRows, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    KT_CHECK_EQ(n[0], per); KT_CHECK_EQ(n[1], 0); KT_CHECK_EQ(n[2], per);
    KT_CHECK_EQ(cont[2].h, 2 * cap + maxRows * btnPx + (maxRows - 1) * pad);
    // banner near the right edge: all shifted left together, sizes unchanged
    layoutBannerGroups(wide, boxes, c403, cols, maxRows, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    const Rect c0 = cont[0], b0 = out[0], b3 = out[3];
    const Rect rightB = {1700, 20, 600, 30};
    layoutBannerGroups(rightB, {1700, 50, 300, 56}, c403, cols, maxRows, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    KT_CHECK_EQ(cont[2].x + cont[2].w, 1920);
    KT_CHECK(cont[0].x + cont[0].w <= 1920 && cont[1].x + cont[1].w <= 1920 && cont[2].x + cont[2].w <= 1920);
    KT_CHECK_EQ(cont[0].w, c0.w); KT_CHECK_EQ(cont[0].h, c0.h); KT_CHECK_EQ(out[0].w, b0.w);
    KT_CHECK_EQ(out[3].x - out[0].x, b3.x - b0.x); KT_CHECK_EQ(out[3].y - out[0].y, b3.y - b0.y);
    KT_CHECK_EQ(out[0].x - cont[0].x, b0.x - c0.x);               // buttons keep their place inside the container
    KT_CHECK_EQ(cont[0].y, c0.y);                                 // no vertical shift
    // btnPx = 0 auto-fits: (196 - 2*3 - 2)/2 = 94, buttons keep >= side from the container sides
    layoutBannerGroups(wide, boxes, c403, cols, maxRows, 0, pad, gap, cap, side, 1920, 1080, out, n, cont);
    const int fit = (196 - 2 * side - (cols - 1) * pad) / cols;
    KT_CHECK_EQ(out[0].w, fit); KT_CHECK_EQ(out[0].h, fit); KT_CHECK_EQ(fit, 94);
    bool sides = true;
    for (int g = 0; g < 3; ++g) for (int i = 0; i < n[g]; ++i) {
      const Rect& r = out[g * per + i];
      sides = sides && r.x - cont[g].x >= side && (cont[g].x + cont[g].w) - (r.x + r.w) >= side;
    }
    KT_CHECK(sides);
    // bottom edge shifts the whole pill row up, top edge shifts it down
    layoutBannerGroups(wide, {100, 1070, 300, 56}, c403, cols, maxRows, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    KT_CHECK_EQ(cont[0].y + cont[0].h, 1080); KT_CHECK_EQ(out[0].y - cont[0].y, cap);
    layoutBannerGroups(wide, {100, -40, 300, 56}, c403, cols, maxRows, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    KT_CHECK_EQ(cont[1].y, 0);
    // degenerate input zeroes everything
    layoutBannerGroups(wide, boxes, c403, 0, maxRows, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    KT_CHECK_EQ(n[0], 0); KT_CHECK_EQ(cont[0].w, 0); KT_CHECK_EQ(cont[2].h, 0);
  }
  // ---- layoutBannerPills (content-sized pills, empty groups hidden)
  {
    const int per = 8, colsTotal = 6, btnPx = 40, pad = 4, gap = 8, cap = 10, side = 6;
    Rect out[3 * per], cont[3]; int n[3];
    const Rect banner = {760, 40, 400, 20}, boxes = {760, 64, 400, 56};
    const int cx = banner.x + banner.w / 2;
    auto run = [&](const int c[3], int colsT, int bPx, Rect bn, Rect bx) {
      layoutBannerPills(bn, bx, c, per, colsT, bPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    };
    // [4,0,3]: middle hidden, cols 3, group 0 has 2 rows
    const int c403[3] = {4, 0, 3};
    run(c403, colsTotal, btnPx, banner, boxes);
    KT_CHECK_EQ(n[0], 4); KT_CHECK_EQ(n[1], 0); KT_CHECK_EQ(n[2], 3);
    KT_CHECK(cont[1].x == 0 && cont[1].y == 0 && cont[1].w == 0 && cont[1].h == 0);
    KT_CHECK(out[per].w == 0 && out[per].x == 0 && out[per + 1].h == 0);
    KT_CHECK(out[3].y > out[0].y); KT_CHECK_EQ(out[3].y, out[0].y + btnPx + pad);
    KT_CHECK_EQ(out[2].y, out[0].y);                                       // 3 columns per row
    KT_CHECK_EQ(cont[0].w, 2 * side + 3 * btnPx + 2 * pad);                // usedCols 3
    KT_CHECK_EQ(cont[0].w, cont[2].w); KT_CHECK_EQ(cont[0].h, cont[2].h);
    KT_CHECK_EQ(cont[0].h, 2 * cap + 2 * btnPx + pad);
    KT_CHECK_EQ(cont[0].y, boxes.y); KT_CHECK_EQ(cont[2].y, boxes.y);
    KT_CHECK(abs((cont[0].x + cont[2].x + cont[2].w) / 2 - cx) <= 1);
    KT_CHECK_EQ(cont[0].x + cont[0].w + gap, cont[2].x);
    // every button inside its pill with >= side / cap margin
    bool inPill = true;
    for (int g = 0; g < 3; ++g) for (int i = 0; i < n[g]; ++i) {
      const Rect& r = out[g * per + i];
      inPill = inPill && r.x - cont[g].x >= side && (cont[g].x + cont[g].w) - (r.x + r.w) >= side
                      && r.y - cont[g].y >= cap && (cont[g].y + cont[g].h) - (r.y + r.h) >= cap;
    }
    KT_CHECK(inPill);
    // [1,1,0]: two single-button pills
    const int c110[3] = {1, 1, 0};
    run(c110, colsTotal, btnPx, banner, boxes);
    KT_CHECK(cont[0].w > 0 && cont[1].w > 0); KT_CHECK(cont[2].w == 0 && cont[2].h == 0);
    KT_CHECK_EQ(cont[0].w, 2 * side + btnPx); KT_CHECK_EQ(cont[0].h, 2 * cap + btnPx);
    KT_CHECK_EQ(cont[1].w, cont[0].w);
    KT_CHECK(abs((cont[0].x + cont[1].x + cont[1].w) / 2 - cx) <= 1);
    // [2,2,2]: three pills, 2 columns each
    const int c222[3] = {2, 2, 2};
    run(c222, colsTotal, btnPx, banner, boxes);
    for (int g = 0; g < 3; ++g) KT_CHECK_EQ(cont[g].w, 2 * side + 2 * btnPx + pad);
    KT_CHECK(abs((cont[0].x + cont[2].x + cont[2].w) / 2 - cx) <= 1);
    KT_CHECK_EQ(cont[0].x + cont[0].w + gap, cont[1].x); KT_CHECK_EQ(cont[1].x + cont[1].w + gap, cont[2].x);
    // [5,0,0]: one pill, one row of 5
    const int c500[3] = {5, 0, 0};
    run(c500, colsTotal, btnPx, banner, boxes);
    KT_CHECK(cont[1].w == 0 && cont[2].w == 0);
    for (int i = 1; i < 5; ++i) KT_CHECK_EQ(out[i].y, out[0].y);
    KT_CHECK_EQ(cont[0].w, 2 * side + 5 * btnPx + 4 * pad);
    KT_CHECK_EQ(cont[0].h, 2 * cap + btnPx);
    KT_CHECK(abs(cont[0].x + cont[0].w / 2 - cx) <= 1);
    // [0,0,0]: everything zero
    const int c000[3] = {0, 0, 0};
    run(c000, colsTotal, btnPx, banner, boxes);
    bool zero = true;
    for (int g = 0; g < 3; ++g) zero = zero && n[g] == 0 && cont[g].x == 0 && cont[g].y == 0 && cont[g].w == 0 && cont[g].h == 0;
    for (int i = 0; i < 3 * per; ++i) zero = zero && out[i].x == 0 && out[i].y == 0 && out[i].w == 0 && out[i].h == 0;
    KT_CHECK(zero);
    // [8,0,0]: cols 6, 2 rows (6 + 2)
    const int c800[3] = {8, 0, 0};
    run(c800, colsTotal, btnPx, banner, boxes);
    KT_CHECK_EQ(n[0], 8);
    KT_CHECK_EQ(out[5].y, out[0].y); KT_CHECK(out[6].y > out[0].y);
    KT_CHECK_EQ(cont[0].w, 2 * side + 6 * btnPx + 5 * pad);
    KT_CHECK_EQ(cont[0].h, 2 * cap + 2 * btnPx + pad);
    // right-edge clamp: shifted left together, sizes unchanged
    run(c403, colsTotal, btnPx, banner, boxes);
    const Rect c0 = cont[0], c2 = cont[2], b0 = out[0], b3 = out[3];
    run(c403, colsTotal, btnPx, {1800, 40, 400, 20}, {1800, 64, 400, 56});
    KT_CHECK_EQ(cont[2].x + cont[2].w, 1920);
    KT_CHECK(cont[0].x + cont[0].w <= 1920 && cont[2].x + cont[2].w <= 1920);
    KT_CHECK_EQ(cont[0].w, c0.w); KT_CHECK_EQ(cont[0].h, c0.h); KT_CHECK_EQ(cont[2].w, c2.w);
    KT_CHECK_EQ(cont[2].x - cont[0].x, c2.x - c0.x);
    KT_CHECK_EQ(out[0].x - cont[0].x, b0.x - c0.x); KT_CHECK_EQ(out[3].y - out[0].y, b3.y - b0.y);
    KT_CHECK_EQ(cont[0].y, c0.y);
    // left-edge clamp
    run(c403, colsTotal, btnPx, {-100, 40, 400, 20}, {-100, 64, 400, 56});
    KT_CHECK_EQ(cont[0].x, 0); KT_CHECK_EQ(cont[0].w, c0.w);
    // narrow banner: buttons shrink to fit, all the same size ([3,0,0] -> cols 6 clamp 8, fit (120-12-20)/6 = 14)
    const int c300[3] = {3, 0, 0};
    run(c300, colsTotal, btnPx, {800, 40, 120, 20}, {800, 64, 56, 56});
    KT_CHECK(out[0].w < btnPx); KT_CHECK_EQ(out[0].w, 14); KT_CHECK_EQ(out[0].h, 14);
    KT_CHECK_EQ(out[1].w, out[0].w); KT_CHECK_EQ(out[2].w, out[0].w);
    KT_CHECK(cont[0].w <= 120);
    // colsTotal 1: one column, rows stack
    run(c300, 1, btnPx, banner, boxes);
    KT_CHECK_EQ(out[1].x, out[0].x); KT_CHECK_EQ(out[2].x, out[0].x);
    KT_CHECK_EQ(out[1].y, out[0].y + btnPx + pad); KT_CHECK_EQ(out[2].y, out[0].y + 2 * (btnPx + pad));
    KT_CHECK_EQ(cont[0].h, 2 * cap + 3 * btnPx + 2 * pad);
    KT_CHECK_EQ(cont[0].w, 2 * side + btnPx);
    // degenerate: per < 1 or null count zero outputs without touching memory
    layoutBannerPills(banner, boxes, c403, 0, colsTotal, btnPx, pad, gap, cap, side, 1920, 1080, out, n, cont);
    KT_CHECK_EQ(n[0], 0); KT_CHECK_EQ(cont[0].w, 0); KT_CHECK_EQ(cont[2].h, 0);
  }
  // ---- parseNameList
  {
    char out[8][ProfName];
    int n = parseNameList("  Flurry ,, \tPower Attack\t, flurry,FLURRY ,Sonic Grenade", out, 8);
    KT_CHECK_EQ(n, 3);
    KT_CHECK(strcmp(out[0], "Flurry") == 0); KT_CHECK(strcmp(out[1], "Power Attack") == 0); KT_CHECK(strcmp(out[2], "Sonic Grenade") == 0);
    KT_CHECK_EQ(parseNameList(nullptr, out, 8), 0);
    KT_CHECK_EQ(parseNameList("", out, 8), 0);
    KT_CHECK_EQ(parseNameList(" , ,\t,", out, 8), 0);
    std::string longName(60, 'x');
    n = parseNameList(longName.c_str(), out, 8);
    KT_CHECK_EQ(n, 1); KT_CHECK_EQ((int)strlen(out[0]), 47);
    n = parseNameList("a,b,c,d,e", out, 3);
    KT_CHECK_EQ(n, 3); KT_CHECK(strcmp(out[2], "c") == 0);
    // truncated duplicates dedupe against the stored (truncated) form
    std::string two = longName + "y," + longName + "z";
    KT_CHECK_EQ(parseNameList(two.c_str(), out, 8), 1);
  }

  // ---- nameMatches (suffix rule: name == token, or name ends with " " + token; "Power Attack" DOES match "Attack" by definition)
  {
    KT_CHECK(nameMatches("Master Flurry", 1, "Flurry"));
    KT_CHECK(nameMatches("flurry", 1, "FLURRY"));
    KT_CHECK(nameMatches("Master Flurry", 1, "master flurry"));
    KT_CHECK(!nameMatches("Minor Sonic Detonator", 1, "Sonic Grenade"));
    KT_CHECK(!nameMatches("Master Flurry", 1, "Flur"));
    KT_CHECK(!nameMatches("FlurryX", 1, "Flurry"));
    KT_CHECK(!nameMatches("Master Flurry", 1, "Master"));
    KT_CHECK(nameMatches("Power Attack", 1, "Attack"));
    KT_CHECK(!nameMatches("Attack", 1, "Power Attack"));
    KT_CHECK(nameMatches("anything", 0x10000035u, "0x10000035"));
    KT_CHECK(nameMatches(nullptr, 0x10000035u, "0X10000035"));
    KT_CHECK(nameMatches("", 0xABu, "0xab"));
    KT_CHECK(!nameMatches("anything", 0x10000036u, "0x10000035"));
    KT_CHECK(!nameMatches("0x10000035", 1, "0x10000035"));   // hex token compares key only
    KT_CHECK(!nameMatches(nullptr, 1, "Flurry"));
    KT_CHECK(!nameMatches("", 1, "Flurry"));
    KT_CHECK(!nameMatches("Flurry", 1, nullptr));
    KT_CHECK(!nameMatches("Flurry", 1, ""));
  }

  // ---- orderGroup / orderSig
  {
    GroupView g; memset(&g, 0, sizeof g);
    static const char names[16][64] = {"Flurry", "Power Attack", "Master Flurry", "Sonic Grenade", "Heal"};
    g.count = 5;
    for (int i = 0; i < 5; ++i) g.e[i] = mk(100 + i, true, 1);
    Profile p; defaultProfile(p);
    KT_CHECK(!p.byCount[0] && !p.byCount[1] && p.byCount[2]);
    KT_CHECK_EQ(p.nPin[1], 0); KT_CHECK_EQ(p.nHide[2], 0);
    int ord[16];
    int n = orderGroup(g, names, p, 0, ord, 16);
    KT_CHECK_EQ(n, 5);
    for (int i = 0; i < 5; ++i) KT_CHECK_EQ(ord[i], i);
    // pins first, in INI order; each pin places the first unplaced match
    p.nPin[0] = parseNameList("Heal,Flurry,Flurry", p.pin[0], ProfMax);   // deduped to Heal,Flurry
    n = orderGroup(g, names, p, 0, ord, 16);
    KT_CHECK_EQ(n, 5);
    KT_CHECK_EQ(ord[0], 4); KT_CHECK_EQ(ord[1], 0); KT_CHECK_EQ(ord[2], 1); KT_CHECK_EQ(ord[3], 2); KT_CHECK_EQ(ord[4], 3);
    // pin matching several entries places one at a time: key-less suffix token matches both Flurry and Master Flurry, first wins
    p.nPin[0] = parseNameList("Master Flurry,Flurry", p.pin[0], ProfMax);
    n = orderGroup(g, names, p, 0, ord, 16);
    KT_CHECK_EQ(ord[0], 2); KT_CHECK_EQ(ord[1], 0);
    // hide removes; hide wins over pin
    p.nHide[0] = parseNameList("Power Attack,Flurry", p.hide[0], ProfMax);
    n = orderGroup(g, names, p, 0, ord, 16);
    KT_CHECK_EQ(n, 3);   // exact "Flurry" exists, so the suffix rule is off for that token: Flurry + Power Attack hidden, Master Flurry stays pinned
    KT_CHECK_EQ(ord[0], 2); KT_CHECK_EQ(ord[1], 3); KT_CHECK_EQ(ord[2], 4);
    // checklist v6 case: Hide0=Attack + Pin0=Power Attack -> only the basic Attack is hidden, Power Attack first
    {
      static const char an[16][64] = {"Attack", "Flurry", "Power Attack"};
      GroupView a; memset(&a, 0, sizeof a); a.count = 3;
      for (int i = 0; i < 3; ++i) a.e[i] = mk(200 + i, true, 1);
      Profile r; defaultProfile(r);
      r.nHide[0] = parseNameList("Attack", r.hide[0], ProfMax);
      r.nPin[0] = parseNameList("Power Attack", r.pin[0], ProfMax);
      n = orderGroup(a, an, r, 0, ord, 16);
      KT_CHECK_EQ(n, 2); KT_CHECK_EQ(ord[0], 2); KT_CHECK_EQ(ord[1], 1);
      // no exact "Attack" entry -> suffix rule applies again
      static const char bn[16][64] = {"Flurry", "Power Attack"};
      GroupView b; memset(&b, 0, sizeof b); b.count = 2;
      b.e[0] = mk(1, true, 1); b.e[1] = mk(2, true, 1);
      n = orderGroup(b, bn, r, 0, ord, 16);
      KT_CHECK_EQ(n, 1); KT_CHECK_EQ(ord[0], 0);
    }
    // hex key hide
    Profile q; defaultProfile(q);
    q.nHide[1] = parseNameList("0x66", q.hide[1], ProfMax);   // key 102 = entry 2
    n = orderGroup(g, names, q, 1, ord, 16);
    KT_CHECK_EQ(n, 4); KT_CHECK_EQ(ord[0], 0); KT_CHECK_EQ(ord[1], 1); KT_CHECK_EQ(ord[2], 3); KT_CHECK_EQ(ord[3], 4);
    // byCount: counts {1,5,3} -> 1,2,0
    GroupView c; memset(&c, 0, sizeof c); c.count = 3;
    c.e[0] = mk(1, true, 1); c.e[0].count = 1;
    c.e[1] = mk(2, true, 1); c.e[1].count = 5;
    c.e[2] = mk(3, true, 1); c.e[2].count = 3;
    n = orderGroup(c, names, p, 2, ord, 16);
    KT_CHECK_EQ(n, 3); KT_CHECK_EQ(ord[0], 1); KT_CHECK_EQ(ord[1], 2); KT_CHECK_EQ(ord[2], 0);
    // ties stable
    c.count = 4; c.e[3] = mk(4, true, 1); c.e[3].count = 5; c.e[0].count = 3;
    n = orderGroup(c, names, p, 2, ord, 16);   // counts {3,5,3,5} -> 1,3,0,2
    KT_CHECK_EQ(n, 4); KT_CHECK_EQ(ord[0], 1); KT_CHECK_EQ(ord[1], 3); KT_CHECK_EQ(ord[2], 0); KT_CHECK_EQ(ord[3], 2);
    // pins precede the count sort
    p.nPin[2] = parseNameList("Heal", p.pin[2], ProfMax);   // names[4] is beyond count 4? index 3 is "Sonic Grenade"; use names[2]
    static const char names2[16][64] = {"A", "B", "C", "D"};
    p.nPin[2] = parseNameList("C", p.pin[2], ProfMax);
    n = orderGroup(c, names2, p, 2, ord, 16);
    KT_CHECK_EQ(n, 4); KT_CHECK_EQ(ord[0], 2); KT_CHECK_EQ(ord[1], 1); KT_CHECK_EQ(ord[2], 3); KT_CHECK_EQ(ord[3], 0);
    // maxOut cap
    n = orderGroup(c, names2, p, 2, ord, 2);
    KT_CHECK_EQ(n, 2); KT_CHECK_EQ(ord[0], 2); KT_CHECK_EQ(ord[1], 1);
    // bad args
    KT_CHECK_EQ(orderGroup(c, names2, p, -1, ord, 16), 0);
    KT_CHECK_EQ(orderGroup(c, names2, p, 3, ord, 16), 0);
    KT_CHECK_EQ(orderGroup(c, nullptr, p, 2, ord, 16), 0);
    KT_CHECK_EQ(orderGroup(c, names2, p, 2, nullptr, 16), 0);
    KT_CHECK_EQ(orderGroup(c, names2, p, 2, ord, 0), 0);

    // orderSig
    int a[4] = {0, 1, 2, 3}, b[4] = {0, 1, 2, 3}, d[4] = {1, 0, 2, 3};
    KT_CHECK_EQ(orderSig(c, a, 4), orderSig(c, b, 4));
    KT_CHECK(orderSig(c, a, 4) != orderSig(c, d, 4));
    KT_CHECK(orderSig(c, a, 3) != orderSig(c, a, 4));
    KT_CHECK_EQ(orderSig(c, nullptr, 4), orderSig(c, nullptr, 4));
    KT_CHECK(orderSig(c, nullptr, 4) != orderSig(c, nullptr, 3));
    // FNV-1a known value: n = 0 -> hash of four zero bytes
    uint32_t h = 2166136261u; for (int i = 0; i < 4; ++i) { h ^= 0u; h *= 16777619u; }
    KT_CHECK_EQ(orderSig(c, nullptr, 0), h);
  }

  // ---- formatProfile / parseProfileLine round trip
  {
    Profile p; defaultProfile(p);
    p.nPin[0] = parseNameList("Flurry,Heal", p.pin[0], ProfMax);
    p.nHide[1] = parseNameList("0x10000035, Sonic Grenade", p.hide[1], ProfMax);
    p.byCount[0] = true; p.byCount[2] = false;
    char buf[1024];
    int len = formatProfile(p, buf, sizeof buf);
    KT_CHECK(len > 0); KT_CHECK_EQ(len, (int)strlen(buf));
    KT_CHECK(buf[len - 1] != '\n');
    KT_CHECK(strstr(buf, "Pin0=Flurry,Heal\n") != nullptr);
    KT_CHECK(strstr(buf, "Sort2=engine") != nullptr);
    Profile r; defaultProfile(r);
    r.byCount[2] = true;
    std::string s(buf); size_t pos = 0; int lines = 0; bool allOk = true;
    while (pos <= s.size()) {
      size_t e = s.find('\n', pos); if (e == std::string::npos) e = s.size();
      std::string line = s.substr(pos, e - pos); pos = e + 1;
      size_t eq = line.find('=');
      if (eq == std::string::npos) { allOk = false; continue; }
      allOk = parseProfileLine(r, line.substr(0, eq).c_str(), line.substr(eq + 1).c_str()) && allOk;
      ++lines;
    }
    KT_CHECK(allOk); KT_CHECK_EQ(lines, 9);
    bool same = true;
    for (int g = 0; g < 3; ++g) {
      same = same && r.nPin[g] == p.nPin[g] && r.nHide[g] == p.nHide[g] && r.byCount[g] == p.byCount[g];
      for (int i = 0; i < p.nPin[g]; ++i) same = same && strcmp(r.pin[g][i], p.pin[g][i]) == 0;
      for (int i = 0; i < p.nHide[g]; ++i) same = same && strcmp(r.hide[g][i], p.hide[g][i]) == 0;
    }
    KT_CHECK(same);
    // small buffer
    char small[8]; small[0] = 'x';
    KT_CHECK_EQ(formatProfile(p, small, sizeof small), -1); KT_CHECK_EQ((int)small[0], 0);
    KT_CHECK_EQ(formatProfile(p, buf, len), -1);       // no room for the NUL
    KT_CHECK_EQ(formatProfile(p, buf, len + 1), len);  // exact fit
    // parseProfileLine rules
    Profile q; defaultProfile(q);
    KT_CHECK(parseProfileLine(q, "PIN1", "a, b"));  KT_CHECK_EQ(q.nPin[1], 2);
    KT_CHECK(parseProfileLine(q, "pin1", nullptr)); KT_CHECK_EQ(q.nPin[1], 0);
    KT_CHECK(parseProfileLine(q, "Sort0", " COUNT ")); KT_CHECK(q.byCount[0]);
    KT_CHECK(parseProfileLine(q, "sort2", "Engine")); KT_CHECK(!q.byCount[2]);
    KT_CHECK(!parseProfileLine(q, "Sort2", "bogus")); KT_CHECK(!q.byCount[2]);
    KT_CHECK(!parseProfileLine(q, "Sort1", nullptr));
    KT_CHECK(!parseProfileLine(q, "Pin3", "a")); KT_CHECK(!parseProfileLine(q, "Foo0", "a"));
    KT_CHECK(!parseProfileLine(q, "Pin", "a")); KT_CHECK(!parseProfileLine(q, nullptr, "a"));
  }
  return kt::Report("combat_logic_test");
}
