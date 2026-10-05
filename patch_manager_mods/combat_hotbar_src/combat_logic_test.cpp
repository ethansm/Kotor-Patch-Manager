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
  return kt::Report("combat_logic_test");
}
