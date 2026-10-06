// combat_logic_test.cpp -- host tests for CombatLogic.h (invoke-entry pick, key edge, slot fill, row layout, hit test).
#include "CombatLogic.h"
#include "check.h"
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
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


// ===================== CH6 additions (doc 31 CH6 mutation gate) =====================
// Struct-size contracts the DLL relies on (resref 16 + NUL, 16 entries per group, 8 pins/hides per group of 48-byte tokens).
// Runtime checks (not static_assert) so a mutated size is reported as a failed test rather than a build break.
static void ch6Contracts() {
  KT_CHECK_EQ((int)sizeof(EntryView::icon), 17);   // EntryView.icon is a 16-char resref + NUL
  KT_CHECK_EQ((int)sizeof(Slot::icon), 17);        // Slot.icon is a 16-char resref + NUL
  KT_CHECK_EQ((int)(sizeof(GroupView::e) / sizeof(EntryView)), 16);   // GroupView holds 16 entries
  KT_CHECK_EQ(ProfMax, 8); KT_CHECK_EQ(ProfName, 48);                  // profile list limits
  KT_CHECK_EQ((int)sizeof(Profile::pin), 3 * ProfMax * ProfName);      // pin/hide lists are per group (3)
  KT_CHECK_EQ((int)sizeof(Profile::hide), 3 * ProfMax * ProfName);
  KT_CHECK_EQ((int)sizeof(Profile::nPin), (int)(3 * sizeof(int))); KT_CHECK_EQ((int)sizeof(Profile::nHide), (int)(3 * sizeof(int)));
  KT_CHECK_EQ((int)sizeof(Profile::byCount), (int)(3 * sizeof(bool)));
}

struct Rng {
  uint32_t s;
  int next(int lo, int hi) { s = s * 1664525u + 1013904223u; return lo + (int)((s >> 8) % (uint32_t)(hi - lo + 1)); }
};
static const Rect kJunk = { 7777, 7777, 7777, 7777 };
static bool sameRect(const Rect& a, const Rect& b) { return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h; }
static bool sameRects(const std::vector<Rect>& a, const std::vector<Rect>& b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) if (!sameRect(a[i], b[i])) return false;
  return true;
}
static bool allZero(const std::vector<Rect>& v) { for (const Rect& r : v) if (!sameRect(r, Rect{0, 0, 0, 0})) return false; return true; }
// Shift [l, r) back inside [0, W): the right edge first, then the left edge wins (never shrinks).
static int refShift(int l, int r, int W) {
  int nl = l;
  if (r > W) nl = W - (r - l);
  if (nl < 0) nl = 0;
  return nl - l;
}
static int clampN(int n, int per) { return n < 0 ? 0 : (n > per ? per : n); }

// ---- reference models written from the header doc comments
static void refGroupGrid(const Rect box[3], const int count[3], int cols, int maxRows, int gap, int W, int H, std::vector<Rect>& out, int outN[3]) {
  const int per = cols * maxRows;
  out.assign(3 * per, Rect{0, 0, 0, 0});
  for (int g = 0; g < 3; ++g) {
    const int n = clampN(count[g], per); outN[g] = n;
    if (n == 0) continue;
    int btn = (box[g].w - gap * (cols - 1)) / cols; if (btn < 1) btn = 1;
    const int uc = n < cols ? n : cols, ur = (n + cols - 1) / cols;
    const int bw = uc * btn + (uc - 1) * gap, bh = ur * btn + (ur - 1) * gap;
    int x0 = box[g].x, y0 = box[g].y + box[g].h + gap;
    x0 += refShift(x0, x0 + bw, W); y0 += refShift(y0, y0 + bh, H);
    for (int k = 0; k < n; ++k) out[g * per + k] = Rect{ x0 + (k % cols) * (btn + gap), y0 + (k / cols) * (btn + gap), btn, btn };
  }
}

static void shiftUsed(std::vector<Rect>& out, const int outN[3], int per, int W, int H) {
  bool any = false; int l = 0, t = 0, r = 0, b = 0;
  for (int g = 0; g < 3; ++g) for (int k = 0; k < outN[g]; ++k) {
    const Rect& q = out[g * per + k];
    if (!any) { l = q.x; t = q.y; r = q.x + q.w; b = q.y + q.h; any = true; continue; }
    l = std::min(l, q.x); t = std::min(t, q.y); r = std::max(r, q.x + q.w); b = std::max(b, q.y + q.h);
  }
  if (!any) return;
  const int dx = refShift(l, r, W), dy = refShift(t, b, H);
  for (int g = 0; g < 3; ++g) for (int k = 0; k < outN[g]; ++k) { out[g * per + k].x += dx; out[g * per + k].y += dy; }
}

static void refBannerGrid(Rect banner, Rect bu, const int count[3], int cols, int maxRows, int btnPx, int pad, int gap, int W, int H, std::vector<Rect>& out, int outN[3]) {
  const int per = cols * maxRows;
  out.assign(3 * per, Rect{0, 0, 0, 0});
  const int span = std::max(bu.w, banner.w);
  const int spanX = banner.x + banner.w / 2 - span / 2;
  const int colW = (span - 2 * gap) / 3;
  const int fit = std::max(1, (colW - (cols - 1) * pad) / cols);
  const int btn = (btnPx > 0 && btnPx <= fit) ? btnPx : fit;
  for (int g = 0; g < 3; ++g) {
    const int n = clampN(count[g], per); outN[g] = n;
    const int colX = spanX + g * (colW + gap);
    for (int k = 0; k < n; ++k) {
      const int row = k / cols, inRow = std::min(cols, n - row * cols);
      const int rowW = inRow * btn + (inRow - 1) * pad;
      out[g * per + k] = Rect{ colX + (colW - rowW) / 2 + (k % cols) * (btn + pad), bu.y + row * (btn + pad), btn, btn };
    }
  }
  shiftUsed(out, outN, per, W, H);
}

static void refBannerGroups(Rect banner, Rect bu, const int count[3], int cols, int maxRows, int btnPx, int pad, int gap, int cap, int side, int W, int H, std::vector<Rect>& out, int outN[3], Rect cont[3]) {
  const int per = cols * maxRows;
  out.assign(3 * per, Rect{0, 0, 0, 0});
  const int span = std::max(bu.w, banner.w);
  const int spanX = banner.x + banner.w / 2 - span / 2;
  const int colW = (span - 2 * gap) / 3;
  const int fit = std::max(1, (colW - 2 * side - (cols - 1) * pad) / cols);
  const int btn = (btnPx > 0 && btnPx <= fit) ? btnPx : fit;
  int rows = 1;
  for (int g = 0; g < 3; ++g) { outN[g] = clampN(count[g], per); rows = std::max(rows, (outN[g] + cols - 1) / cols); }
  const int h = 2 * cap + rows * btn + (rows - 1) * pad;
  for (int g = 0; g < 3; ++g) {
    const int colX = spanX + g * (colW + gap);
    cont[g] = Rect{ colX, bu.y, colW, h };
    for (int k = 0; k < outN[g]; ++k) {
      const int row = k / cols, inRow = std::min(cols, outN[g] - row * cols);
      const int rowW = inRow * btn + (inRow - 1) * pad;
      out[g * per + k] = Rect{ colX + (colW - rowW) / 2 + (k % cols) * (btn + pad), bu.y + cap + row * (btn + pad), btn, btn };
    }
  }
  int l = cont[0].x, r = cont[0].x + cont[0].w;
  for (int g = 1; g < 3; ++g) { l = std::min(l, cont[g].x); r = std::max(r, cont[g].x + cont[g].w); }
  const int dx = refShift(l, r, W), dy = refShift(bu.y, bu.y + h, H);
  for (int g = 0; g < 3; ++g) {
    cont[g].x += dx; cont[g].y += dy;
    for (int k = 0; k < outN[g]; ++k) { out[g * per + k].x += dx; out[g * per + k].y += dy; }
  }
}

static void refBannerPills(Rect banner, Rect bu, const int count[3], int per, int colsTotal, int btnPx, int pad, int gap, int cap, int side, int W, int H, std::vector<Rect>& out, int outN[3], Rect cont[3]) {
  out.assign(3 * per, Rect{0, 0, 0, 0});
  int nVis = 0, maxN = 0;
  for (int g = 0; g < 3; ++g) {
    outN[g] = clampN(count[g], per); cont[g] = Rect{0, 0, 0, 0};
    if (outN[g] > 0) { ++nVis; maxN = std::max(maxN, outN[g]); }
  }
  if (nVis == 0) return;
  const int cols = std::min(per, std::max(1, colsTotal / nVis));
  const int span = std::max(bu.w, banner.w);
  const int share = (span - (nVis - 1) * gap) / nVis;
  const int fit = std::max(1, (share - 2 * side - (cols - 1) * pad) / cols);
  const int btn = (btnPx > 0 && btnPx <= fit) ? btnPx : fit;
  const int usedCols = std::min(cols, maxN);
  int rows = 1;
  for (int g = 0; g < 3; ++g) if (outN[g] > 0) rows = std::max(rows, (outN[g] + cols - 1) / cols);
  const int pillW = 2 * side + usedCols * btn + (usedCols - 1) * pad;
  const int pillH = 2 * cap + rows * btn + (rows - 1) * pad;
  const int total = nVis * pillW + (nVis - 1) * gap;
  const int startX = banner.x + banner.w / 2 - total / 2;
  int vi = 0;
  for (int g = 0; g < 3; ++g) {
    const int n = outN[g];
    if (n <= 0) continue;
    cont[g] = Rect{ startX + vi * (pillW + gap), bu.y, pillW, pillH }; ++vi;
    for (int k = 0; k < n; ++k) {
      const int row = k / cols, inRow = std::min(cols, n - row * cols);
      const int rowW = inRow * btn + (inRow - 1) * pad;
      out[g * per + k] = Rect{ cont[g].x + (pillW - rowW) / 2 + (k % cols) * (btn + pad), cont[g].y + cap + row * (btn + pad), btn, btn };
    }
  }
  const int dx = refShift(startX, startX + total, W), dy = refShift(bu.y, bu.y + pillH, H);
  for (int g = 0; g < 3; ++g) {
    if (outN[g] <= 0) continue;
    cont[g].x += dx; cont[g].y += dy;
    for (int k = 0; k < outN[g]; ++k) { out[g * per + k].x += dx; out[g * per + k].y += dy; }
  }
}

// ---------------------------------------------------------------------------------------------------------------
static void ch6Basics() {
  ch6Contracts();
  // clampCount
  KT_CHECK_EQ(clampCount(-5), 0); KT_CHECK_EQ(clampCount(-1), 0); KT_CHECK_EQ(clampCount(0), 0); KT_CHECK_EQ(clampCount(1), 1);
  KT_CHECK_EQ(clampCount(15), 15); KT_CHECK_EQ(clampCount(16), 16); KT_CHECK_EQ(clampCount(17), 16); KT_CHECK_EQ(clampCount(20), 16); KT_CHECK_EQ(clampCount(1000), 16);

  // fillSlots: tight limits, tails of the icon copy
  {
    GroupView g[3]; memset(g, 0, sizeof g);
    g[0].count = 2; g[0].e[0] = mk(1, true, 1, "a"); g[0].e[1] = mk(2, true, 1, "b");
    g[1].count = 1; g[1].e[0] = mk(3, true, 1, "c");
    Slot s[8];
    KT_CHECK_EQ(fillSlots(g, s, 1, 1), 1); KT_CHECK_EQ(s[0].key, 1u);          // maxSlots == 1 still writes one
    KT_CHECK_EQ(fillSlots(g, s, 8, 1), 2); KT_CHECK_EQ(s[1].group, 1);          // perGroup == 1 still writes one per group
    KT_CHECK_EQ(fillSlots(g, s, 8, -2), 0); KT_CHECK_EQ(fillSlots(g, s, -2, 4), 0);
    KT_CHECK_EQ(fillSlots(nullptr, s, 8, 4), 0);                                 // null group view
    KT_CHECK_EQ(fillSlots(g, nullptr, 8, 4), 0);                                 // null output
    GroupView h[3]; memset(h, 0, sizeof h); h[0].count = 1;
    for (int i = 0; i < 16; ++i) h[0].e[0].icon[i] = (char)('a' + i);
    h[0].e[0].icon[16] = 'Z';
    Slot t[1]; memset(t, 0x55, sizeof t);
    KT_CHECK_EQ(fillSlots(h, t, 1, 1), 1);
    KT_CHECK(memcmp(t[0].icon, "abcdefghijklmnop", 16) == 0);   // all 16 bytes copied, incl. the last
    KT_CHECK_EQ((int)t[0].icon[16], 0);
  }

  // slotEntry
  {
    int gr = 5;
    KT_CHECK_EQ(slotEntry(0, 2, 3, &gr), 0); KT_CHECK_EQ(gr, 0);
    KT_CHECK_EQ(slotEntry(6, 2, 3, &gr), 0); KT_CHECK_EQ(gr, 1);
    KT_CHECK_EQ(slotEntry(17, 2, 3, &gr), 5); KT_CHECK_EQ(gr, 2);
    KT_CHECK_EQ(slotEntry(18, 2, 3, &gr), -1); KT_CHECK_EQ(gr, -1);
    gr = 5; KT_CHECK_EQ(slotEntry(-1, 2, 3, &gr), -1); KT_CHECK_EQ(gr, -1);
    gr = 5; KT_CHECK_EQ(slotEntry(0, 1, 1, &gr), 0); KT_CHECK_EQ(gr, 0);           // per == 1 is valid
    KT_CHECK_EQ(slotEntry(2, 1, 1, &gr), 0); KT_CHECK_EQ(gr, 2);
    KT_CHECK_EQ(slotEntry(3, 1, 1, &gr), -1);
    gr = 5; KT_CHECK_EQ(slotEntry(0, 0, 4, &gr), -1); KT_CHECK_EQ(gr, -1);          // per == 0
    gr = 5; KT_CHECK_EQ(slotEntry(0, -2, 4, &gr), -1); KT_CHECK_EQ(gr, -1);         // per < 0
    KT_CHECK_EQ(slotEntry(1, 2, 2, nullptr), 1); KT_CHECK_EQ(slotEntry(99, 2, 2, nullptr), -1);
  }

  // gridBounds
  {
    Rect out[6] = { {10, 20, 5, 6}, {0, 0, 0, 0}, {100, 50, 3, 3}, {-4, 7, 2, 2}, {0, 0, 0, 0}, {30, 90, 10, 1} };
    const int n3[3] = { 2, 0, 1 };
    // stride 3: group0 -> out[0..1], group1 -> none, group2 -> out[6..] ... use stride 2 instead
    Rect b = gridBounds(out, n3, 2);   // g0: out[0], out[1]; g2: out[4]
    KT_CHECK_EQ(b.x, 0); KT_CHECK_EQ(b.y, 0); KT_CHECK_EQ(b.w, 15); KT_CHECK_EQ(b.h, 26);
    const int n1[3] = { 1, 1, 1 };
    Rect c = gridBounds(out, n1, 1);   // out[0], out[1], out[2]
    KT_CHECK_EQ(c.x, 0); KT_CHECK_EQ(c.y, 0); KT_CHECK_EQ(c.w, 103); KT_CHECK_EQ(c.h, 53);
    const int n2[3] = { 0, 0, 1 };
    Rect d = gridBounds(out, n2, 1);   // only out[2]
    KT_CHECK_EQ(d.x, 100); KT_CHECK_EQ(d.y, 50); KT_CHECK_EQ(d.w, 3); KT_CHECK_EQ(d.h, 3);
    const int neg[3] = { -3, 1, -1 };
    Rect e = gridBounds(out, neg, 2);   // only g1 -> out[2]
    KT_CHECK_EQ(e.x, 100); KT_CHECK_EQ(e.y, 50); KT_CHECK_EQ(e.w, 3); KT_CHECK_EQ(e.h, 3);
    const int none[3] = { 0, 0, 0 };
    Rect z = gridBounds(out, none, 2);
    KT_CHECK_EQ(z.x, 0); KT_CHECK_EQ(z.y, 0); KT_CHECK_EQ(z.w, 0); KT_CHECK_EQ(z.h, 0);
    Rect zn = gridBounds(nullptr, n3, 2);   KT_CHECK_EQ(zn.x, 0); KT_CHECK_EQ(zn.y, 0); KT_CHECK_EQ(zn.w, 0); KT_CHECK_EQ(zn.h, 0);
    Rect zo = gridBounds(out, nullptr, 2);  KT_CHECK_EQ(zo.x, 0); KT_CHECK_EQ(zo.y, 0); KT_CHECK_EQ(zo.w, 0); KT_CHECK_EQ(zo.h, 0);
    Rect zs = gridBounds(out, n3, 0);       KT_CHECK_EQ(zs.x, 0); KT_CHECK_EQ(zs.y, 0); KT_CHECK_EQ(zs.w, 0); KT_CHECK_EQ(zs.h, 0);
    Rect zt = gridBounds(out, n3, -4);      KT_CHECK_EQ(zt.w, 0); KT_CHECK_EQ(zt.h, 0);
    // counts above the stride are capped to it
    const int big[3] = { 9, 0, 0 };
    Rect f = gridBounds(out, big, 2);       // only out[0], out[1]
    KT_CHECK_EQ(f.x, 0); KT_CHECK_EQ(f.w, 15); KT_CHECK_EQ(f.y, 0); KT_CHECK_EQ(f.h, 26);
    // negative origin and extents: w/h are right/bottom minus left/top
    Rect two[2] = { {-10, -20, 4, 4}, {-2, -3, 10, 5} };
    const int nn[3] = { 2, 0, 0 };
    Rect g2 = gridBounds(two, nn, 2);
    KT_CHECK_EQ(g2.x, -10); KT_CHECK_EQ(g2.y, -20); KT_CHECK_EQ(g2.w, 18); KT_CHECK_EQ(g2.h, 22);
  }

  // layoutRow guards
  {
    Rect r[3] = { kJunk, kJunk, kJunk };
    layoutRow(800, 600, Rect{300, 100, 200, 20}, 0, 48, 6, r);
    KT_CHECK(sameRect(r[0], kJunk));                       // n == 0 writes nothing
    layoutRow(800, 600, Rect{300, 100, 200, 20}, -2, 48, 6, r);
    KT_CHECK(sameRect(r[0], kJunk));
    layoutRow(800, 600, Rect{300, 100, 200, 20}, 1, 48, 6, r);
    KT_CHECK_EQ(r[0].x, 400 - 24); KT_CHECK_EQ(r[0].y, 126); KT_CHECK(sameRect(r[1], kJunk));   // n == 1 writes one
    layoutRow(800, 600, Rect{300, 100, 200, 20}, 3, 48, 6, nullptr);   // null out: no crash
  }
}

static void ch6GuardsAndNulls() {
  const Rect box[3] = { {10, 10, 100, 20}, {120, 10, 100, 20}, {230, 10, 100, 20} };
  const int cnt[3] = { 2, 1, 3 };
  // ---- layoutGroupGrid
  {
    int outN[3] = { 9, 9, 9 };
    layoutGroupGrid(box, cnt, 2, 2, 1, 400, 300, nullptr, outN);
    KT_CHECK_EQ(outN[0], 9);   // null out: untouched
    std::vector<Rect> o(12, kJunk);
    layoutGroupGrid(box, cnt, 2, 2, 1, 400, 300, o.data(), nullptr);
    KT_CHECK(sameRect(o[0], kJunk));   // null outN: untouched
    for (int variant = 0; variant < 4; ++variant) {   // invalid input -> everything zeroed (when the grid size is valid), outN zeroed
      std::vector<Rect> z(3 * 1 * 1, kJunk); int n9[3] = { 9, 9, 9 };
      layoutGroupGrid(variant == 0 ? nullptr : box, variant == 1 ? nullptr : cnt, 1, 1, 1, 400, 300, z.data(), n9);
      if (variant < 2) { KT_CHECK(allZero(z)); KT_CHECK(n9[0] == 0 && n9[1] == 0 && n9[2] == 0); }
    }
    {
      std::vector<Rect> z(3 * 1 * 2, kJunk); int n9[3] = { 9, 9, 9 };
      layoutGroupGrid(nullptr, cnt, 1, 2, 1, 400, 300, z.data(), n9);   // cols == 1 and maxRows == 2 are valid sizes
      KT_CHECK(allZero(z)); KT_CHECK(n9[0] == 0 && n9[1] == 0 && n9[2] == 0);
      std::vector<Rect> y(3 * 2 * 1, kJunk); int m9[3] = { 9, 9, 9 };
      layoutGroupGrid(nullptr, cnt, 2, 1, 1, 400, 300, y.data(), m9);   // maxRows == 1
      KT_CHECK(allZero(y));
      int k9[3] = { 9, 9, 9 };
      layoutGroupGrid(nullptr, cnt, -1, 4, 1, 400, 300, y.data(), k9);  // invalid sizes: no memset at all
      KT_CHECK(k9[0] == 0 && k9[1] == 0 && k9[2] == 0);
      layoutGroupGrid(nullptr, cnt, 3, -1, 1, 400, 300, y.data(), k9);
      layoutGroupGrid(nullptr, cnt, 0, 4, 1, 400, 300, y.data(), k9);
      layoutGroupGrid(box, cnt, 0, 4, 1, 400, 300, y.data(), k9);
      layoutGroupGrid(box, cnt, 3, 0, 1, 400, 300, y.data(), k9);
      KT_CHECK(k9[0] == 0 && k9[1] == 0 && k9[2] == 0);
    }
    // valid, but with out-of-range / negative counts and a tiny box (btn clamps to 1)
    Rect tiny[3] = { {0, 0, 0, 5}, {0, 0, -9, 5}, {0, 0, 2, 5} };
    const int neg[3] = { -1, 1, 1 };
    std::vector<Rect> o2(3 * 2 * 2, kJunk); int n2[3] = { 9, 9, 9 };
    layoutGroupGrid(tiny, neg, 2, 2, 3, 400, 300, o2.data(), n2);
    KT_CHECK_EQ(n2[0], 0); KT_CHECK_EQ(n2[1], 1); KT_CHECK_EQ(n2[2], 1);
    KT_CHECK_EQ(o2[4].w, 1); KT_CHECK_EQ(o2[8].w, 1); KT_CHECK(sameRect(o2[0], Rect{0, 0, 0, 0}));
  }
  // ---- layoutBannerGrid / layoutBannerGroups / layoutBannerPills guards
  {
    const Rect ban = { 100, 10, 200, 20 }, bu = { 50, 40, 300, 20 };
    int outN[3] = { 9, 9, 9 }; Rect cont[3] = { kJunk, kJunk, kJunk };
    layoutBannerGrid(ban, bu, cnt, 2, 2, 0, 2, 4, 400, 300, nullptr, outN);
    KT_CHECK_EQ(outN[0], 9);
    std::vector<Rect> o(12, kJunk);
    layoutBannerGrid(ban, bu, cnt, 2, 2, 0, 2, 4, 400, 300, o.data(), nullptr);
    KT_CHECK(sameRect(o[0], kJunk));
    layoutBannerGroups(ban, bu, cnt, 2, 2, 0, 2, 4, 3, 3, 400, 300, nullptr, outN, cont);
    layoutBannerGroups(ban, bu, cnt, 2, 2, 0, 2, 4, 3, 3, 400, 300, o.data(), nullptr, cont);
    layoutBannerGroups(ban, bu, cnt, 2, 2, 0, 2, 4, 3, 3, 400, 300, o.data(), outN, nullptr);
    KT_CHECK_EQ(outN[0], 9); KT_CHECK(sameRect(o[0], kJunk)); KT_CHECK(sameRect(cont[0], kJunk));
    layoutBannerPills(ban, bu, cnt, 4, 6, 0, 2, 4, 3, 3, 400, 300, nullptr, outN, cont);
    layoutBannerPills(ban, bu, cnt, 4, 6, 0, 2, 4, 3, 3, 400, 300, o.data(), nullptr, cont);
    layoutBannerPills(ban, bu, cnt, 4, 6, 0, 2, 4, 3, 3, 400, 300, o.data(), outN, nullptr);
    KT_CHECK_EQ(outN[0], 9); KT_CHECK(sameRect(o[0], kJunk)); KT_CHECK(sameRect(cont[0], kJunk));

    // invalid sizes / null counts zero everything that is sized validly
    for (int cols = 1; cols <= 2; ++cols) for (int rows = 1; rows <= 2; ++rows) {
      std::vector<Rect> z(3 * cols * rows, kJunk); int n9[3] = { 9, 9, 9 };
      layoutBannerGrid(ban, bu, nullptr, cols, rows, 0, 2, 4, 400, 300, z.data(), n9);
      KT_CHECK(allZero(z)); KT_CHECK(n9[0] == 0 && n9[1] == 0 && n9[2] == 0);
      std::vector<Rect> z2(3 * cols * rows, kJunk); Rect c2[3] = { kJunk, kJunk, kJunk }; int m9[3] = { 9, 9, 9 };
      layoutBannerGroups(ban, bu, nullptr, cols, rows, 0, 2, 4, 3, 3, 400, 300, z2.data(), m9, c2);
      KT_CHECK(allZero(z2)); KT_CHECK(m9[0] == 0 && m9[1] == 0 && m9[2] == 0);
      for (int g = 0; g < 3; ++g) KT_CHECK(sameRect(c2[g], Rect{0, 0, 0, 0}));
    }
    {
      std::vector<Rect> z(16, kJunk); Rect c2[3] = { kJunk, kJunk, kJunk }; int n9[3] = { 9, 9, 9 };
      layoutBannerGrid(ban, bu, cnt, -1, 4, 0, 2, 4, 400, 300, z.data(), n9);
      KT_CHECK(n9[0] == 0 && n9[1] == 0 && n9[2] == 0); KT_CHECK(sameRect(z[0], kJunk));
      layoutBannerGrid(ban, bu, cnt, 4, -1, 0, 2, 4, 400, 300, z.data(), n9);
      layoutBannerGrid(ban, bu, cnt, 0, 4, 0, 2, 4, 400, 300, z.data(), n9);
      layoutBannerGrid(ban, bu, cnt, 4, 0, 0, 2, 4, 400, 300, z.data(), n9);
      KT_CHECK(sameRect(z[0], kJunk));
      layoutBannerGroups(ban, bu, cnt, -1, 4, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      layoutBannerGroups(ban, bu, cnt, 4, -1, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      layoutBannerGroups(ban, bu, cnt, 0, 4, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      layoutBannerGroups(ban, bu, cnt, 4, 0, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      KT_CHECK(sameRect(z[0], kJunk)); KT_CHECK(sameRect(c2[0], Rect{0, 0, 0, 0}));
      // with a valid grid size the caller's counts must still be zeroed
      layoutBannerGrid(ban, bu, nullptr, 1, 1, 0, 2, 4, 400, 300, z.data(), n9);
      layoutBannerGrid(ban, bu, nullptr, 3, 1, 0, 2, 4, 400, 300, z.data(), n9);
    }
    // pills: count == null / per < 1
    for (int per = 1; per <= 3; ++per) {
      std::vector<Rect> z(3 * per, kJunk); Rect c2[3] = { kJunk, kJunk, kJunk }; int n9[3] = { 9, 9, 9 };
      layoutBannerPills(ban, bu, nullptr, per, 6, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      KT_CHECK(allZero(z)); KT_CHECK(n9[0] == 0 && n9[1] == 0 && n9[2] == 0);
      for (int g = 0; g < 3; ++g) KT_CHECK(sameRect(c2[g], Rect{0, 0, 0, 0}));
    }
    for (int per = -2; per <= 0; ++per) {
      std::vector<Rect> z(4, kJunk); Rect c2[3] = { kJunk, kJunk, kJunk }; int n9[3] = { 9, 9, 9 };
      layoutBannerPills(ban, bu, cnt, per, 6, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      KT_CHECK(n9[0] == 0 && n9[1] == 0 && n9[2] == 0); KT_CHECK(sameRect(z[0], kJunk));
      for (int g = 0; g < 3; ++g) KT_CHECK(sameRect(c2[g], Rect{0, 0, 0, 0}));
    }
    {   // per == 1, valid counts: one pill button per group
      std::vector<Rect> z(3, kJunk); Rect c2[3] = { kJunk, kJunk, kJunk }; int n9[3] = { 9, 9, 9 };
      layoutBannerPills(ban, bu, cnt, 1, 6, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      KT_CHECK(n9[0] == 1 && n9[1] == 1 && n9[2] == 1);
      KT_CHECK(c2[0].w > 0 && c2[1].w > 0 && c2[2].w > 0);
    }
    {   // all-empty counts: everything zero, the visible-pill math never runs
      const int none[3] = { 0, -3, 0 };
      std::vector<Rect> z(3 * 4, kJunk); Rect c2[3] = { kJunk, kJunk, kJunk }; int n9[3] = { 9, 9, 9 };
      layoutBannerPills(ban, bu, none, 4, 6, 0, 2, 4, 3, 3, 400, 300, z.data(), n9, c2);
      KT_CHECK(allZero(z)); KT_CHECK(n9[0] == 0 && n9[1] == 0 && n9[2] == 0);
      for (int g = 0; g < 3; ++g) KT_CHECK(sameRect(c2[g], Rect{0, 0, 0, 0}));
    }
  }
  // ---- fewer visible pills than colsTotal columns / more groups than columns (cols clamps to 1, per cap)
  {
    const Rect ban = { 100, 10, 200, 20 }, bu = { 50, 40, 300, 20 };
    const int three[3] = { 2, 2, 2 };
    std::vector<Rect> o(3 * 4), ref; Rect c[3], rc[3]; int n[3], rn[3];
    layoutBannerPills(ban, bu, three, 4, 2, 0, 2, 4, 3, 3, 1000, 800, o.data(), n, c);   // colsTotal 2 < nVis 3
    refBannerPills(ban, bu, three, 4, 2, 0, 2, 4, 3, 3, 1000, 800, ref, rn, rc);
    KT_CHECK(sameRects(o, ref)); KT_CHECK(c[0].w > 0);
    layoutBannerPills(ban, bu, three, 2, 12, 0, 2, 4, 3, 3, 1000, 800, o.data(), n, c);   // cols capped by per
    std::vector<Rect> o2(3 * 2); layoutBannerPills(ban, bu, three, 2, 12, 0, 2, 4, 3, 3, 1000, 800, o2.data(), n, c);
    refBannerPills(ban, bu, three, 2, 12, 0, 2, 4, 3, 3, 1000, 800, ref, rn, rc);
    KT_CHECK(sameRects(o2, ref));
  }
}

template <class F> static void sweep(int iters, uint32_t seed, F f) { Rng r{ seed }; for (int i = 0; i < iters; ++i) f(r); }

static void ch6LayoutSweeps() {
  int bad = 0;
  // ---- layoutGroupGrid vs the reference
  sweep(6000, 11, [&](Rng& r) {
    const int cols = r.next(1, 4), rows = r.next(1, 3), gap = r.next(0, 8), W = r.next(60, 500), H = r.next(60, 400), per = cols * rows;
    Rect box[3]; int cnt[3];
    for (int g = 0; g < 3; ++g) { box[g] = Rect{ r.next(-40, W + 40), r.next(-20, H + 20), r.next(-10, 150), r.next(0, 40) }; cnt[g] = r.next(-2, per + 3); }
    std::vector<Rect> o(3 * per, kJunk), ref; int n[3], rn[3];
    layoutGroupGrid(box, cnt, cols, rows, gap, W, H, o.data(), n);
    refGroupGrid(box, cnt, cols, rows, gap, W, H, ref, rn);
    if (!sameRects(o, ref) || n[0] != rn[0] || n[1] != rn[1] || n[2] != rn[2]) ++bad;
  });
  KT_CHECK_EQ(bad, 0); bad = 0;

  // ---- layoutBannerGrid vs the reference
  sweep(8000, 22, [&](Rng& r) {
    const int cols = r.next(1, 4), rows = r.next(1, 3), per = cols * rows, W = r.next(60, 500), H = r.next(60, 400);
    const Rect ban = { r.next(-60, W + 60), r.next(0, H), r.next(0, 300), r.next(0, 30) };
    const Rect bu = { r.next(-20, W), r.next(-30, H + 30), r.next(0, 420), r.next(0, 40) };
    const int btnPx = r.next(-2, 70), pad = r.next(0, 8), gap = r.next(0, 14);
    int cnt[3]; for (int g = 0; g < 3; ++g) cnt[g] = r.next(-2, per + 3);
    std::vector<Rect> o(3 * per, kJunk), ref; int n[3], rn[3];
    layoutBannerGrid(ban, bu, cnt, cols, rows, btnPx, pad, gap, W, H, o.data(), n);
    refBannerGrid(ban, bu, cnt, cols, rows, btnPx, pad, gap, W, H, ref, rn);
    if (!sameRects(o, ref) || n[0] != rn[0] || n[1] != rn[1] || n[2] != rn[2]) ++bad;
  });
  KT_CHECK_EQ(bad, 0); bad = 0;

  // ---- layoutBannerGroups vs the reference
  sweep(8000, 33, [&](Rng& r) {
    const int cols = r.next(1, 4), rows = r.next(1, 3), per = cols * rows, W = r.next(60, 500), H = r.next(60, 400);
    const Rect ban = { r.next(-60, W + 60), r.next(0, H), r.next(0, 300), r.next(0, 30) };
    const Rect bu = { r.next(-20, W), r.next(-30, H + 30), r.next(0, 420), r.next(0, 40) };
    const int btnPx = r.next(-2, 70), pad = r.next(0, 8), gap = r.next(0, 14), cap = r.next(0, 12), side = r.next(0, 12);
    int cnt[3]; for (int g = 0; g < 3; ++g) cnt[g] = r.next(-2, per + 3);
    std::vector<Rect> o(3 * per, kJunk), ref; int n[3], rn[3]; Rect c[3] = { kJunk, kJunk, kJunk }, rc[3];
    layoutBannerGroups(ban, bu, cnt, cols, rows, btnPx, pad, gap, cap, side, W, H, o.data(), n, c);
    refBannerGroups(ban, bu, cnt, cols, rows, btnPx, pad, gap, cap, side, W, H, ref, rn, rc);
    bool ok = sameRects(o, ref) && n[0] == rn[0] && n[1] == rn[1] && n[2] == rn[2];
    for (int g = 0; g < 3; ++g) ok = ok && sameRect(c[g], rc[g]);
    if (!ok) ++bad;
  });
  KT_CHECK_EQ(bad, 0); bad = 0;

  // ---- layoutBannerPills vs the reference
  sweep(10000, 44, [&](Rng& r) {
    const int per = r.next(1, 8), W = r.next(60, 500), H = r.next(60, 400);
    const Rect ban = { r.next(-60, W + 60), r.next(0, H), r.next(0, 300), r.next(0, 30) };
    const Rect bu = { r.next(-20, W), r.next(-30, H + 30), r.next(0, 420), r.next(0, 40) };
    const int colsTotal = r.next(-1, 14), btnPx = r.next(-2, 70), pad = r.next(0, 8), gap = r.next(0, 14), cap = r.next(0, 12), side = r.next(0, 12);
    int cnt[3]; for (int g = 0; g < 3; ++g) cnt[g] = r.next(-2, per + 3);
    if (r.next(0, 5) == 0) cnt[r.next(0, 2)] = 0;
    std::vector<Rect> o(3 * per, kJunk), ref; int n[3], rn[3]; Rect c[3] = { kJunk, kJunk, kJunk }, rc[3];
    layoutBannerPills(ban, bu, cnt, per, colsTotal, btnPx, pad, gap, cap, side, W, H, o.data(), n, c);
    refBannerPills(ban, bu, cnt, per, colsTotal, btnPx, pad, gap, cap, side, W, H, ref, rn, rc);
    bool ok = sameRects(o, ref) && n[0] == rn[0] && n[1] == rn[1] && n[2] == rn[2];
    for (int g = 0; g < 3; ++g) ok = ok && sameRect(c[g], rc[g]);
    if (!ok) ++bad;
  });
  KT_CHECK_EQ(bad, 0); bad = 0;

  // ---- gridBounds vs the reference
  sweep(3000, 55, [&](Rng& r) {
    const int stride = r.next(1, 5); std::vector<Rect> rs(3 * stride);
    for (Rect& q : rs) q = Rect{ r.next(-50, 50), r.next(-50, 50), r.next(0, 30), r.next(0, 30) };
    int n[3]; for (int g = 0; g < 3; ++g) n[g] = r.next(-1, stride + 2);
    bool any = false; int l = 0, t = 0, rr = 0, b = 0;
    for (int g = 0; g < 3; ++g) for (int k = 0; k < clampN(n[g], stride); ++k) {
      const Rect& q = rs[g * stride + k];
      if (!any) { l = q.x; t = q.y; rr = q.x + q.w; b = q.y + q.h; any = true; continue; }
      l = std::min(l, q.x); t = std::min(t, q.y); rr = std::max(rr, q.x + q.w); b = std::max(b, q.y + q.h);
    }
    const Rect got = gridBounds(rs.data(), n, stride);
    const Rect want = any ? Rect{ l, t, rr - l, b - t } : Rect{ 0, 0, 0, 0 };
    if (!sameRect(got, want)) ++bad;
  });
  KT_CHECK_EQ(bad, 0);
}

// Names table helper for the profile tests.
struct Names { char n[16][64]; Names() { memset(n, 0, sizeof n); } void set(int i, const char* s) { strncpy(n[i], s, 63); } };

static uint32_t refFnv(const GroupView& g, const int* ord, int n) {
  uint32_t h = 2166136261u;
  auto byte = [&](uint32_t b) { h ^= b; h *= 16777619u; };
  auto word = [&](uint32_t v) { byte(v & 0xFF); byte((v >> 8) & 0xFF); byte((v >> 16) & 0xFF); byte(v >> 24); };
  word((uint32_t)n);
  if (!ord) return h;
  for (int i = 0; i < n; ++i) word((ord[i] >= 0 && ord[i] < 16) ? g.e[ord[i]].key : 0u);
  return h;
}

static std::vector<int> order(const GroupView& g, const Names& nm, const Profile& p, int gi, int maxOut = 16) {
  std::vector<int> o(maxOut > 0 ? maxOut : 1, -77);   // exact size: a write past maxOut trips the sanitizer
  const int n = orderGroup(g, nm.n, p, gi, o.data(), maxOut);
  o.resize(n < 0 ? 0 : n);
  return o;
}

static void ch6Profile() {
  // ---- orderSig: pinned values + reference
  {
    GroupView g; memset(&g, 0, sizeof g);
    for (int i = 0; i < 16; ++i) g.e[i].key = 0x01000193u * (uint32_t)(i + 1) + 0x11u * (uint32_t)i + 7u;
    KT_CHECK_EQ(orderSig(g, nullptr, 0), 0x4b95f515u);   // FNV-1a of the four zero bytes of n = 0
    GroupView h; memset(&h, 0, sizeof h); h.e[0].key = 0x11; h.e[1].key = 0x22; h.e[2].key = 0x33;
    const int o3[3] = { 0, 1, 2 };
    KT_CHECK_EQ(orderSig(h, o3, 3), 0x96199c46u);
    GroupView k; memset(&k, 0, sizeof k); k.e[0].key = 0x01020304u;
    const int o2[2] = { 0, 99 };
    KT_CHECK_EQ(orderSig(k, o2, 2), 0x96207b5fu);        // out-of-range index hashes key 0
    const int sets[][6] = { { 0, 1, 2, 3, 4, 5 }, { 15, 0, 15, 7, 8, 9 }, { 0, -1, 16, 1, 17, 15 }, { 3, 3, 3, 3, 3, 3 }, { -1, -1, 16, 99, -50, 0 }, { 14, 15, 0, 1, 2, 3 } };
    for (const auto& s : sets) for (int n = 0; n <= 6; ++n) KT_CHECK_EQ(orderSig(g, s, n), refFnv(g, s, n));
    KT_CHECK_EQ(orderSig(g, nullptr, 5), refFnv(g, nullptr, 5));
    KT_CHECK(orderSig(g, o3, 3) != orderSig(g, o3, 2));
  }

  // ---- parseNameList
  {
    char out[4][ProfName];
    for (int i = 0; i < 4; ++i) { memset(out[i], 0, ProfName); }
    strcpy(out[1], "foo");   // stale text just past the filled part must not count as a duplicate
    KT_CHECK_EQ(parseNameList("bar,foo", out, 4), 2);
    KT_CHECK(strcmp(out[1], "foo") == 0);
    KT_CHECK_EQ(parseNameList("a,b,c", out, 1), 1);   // max == 1
    KT_CHECK_EQ(parseNameList("a,b,c", out, 0), 0);
    KT_CHECK_EQ(parseNameList("a,b,c", out, -1), 0);
    KT_CHECK_EQ(parseNameList(nullptr, out, 3), 0);
    KT_CHECK_EQ(parseNameList("a", nullptr, 3), 0);
    // truncation to ProfName-1 chars: 46, 47, 48 and 60 long tokens
    for (int len : { 46, 47, 48, 60 }) {
      std::string tok((size_t)len, 'q');
      char o2[2][ProfName]; memset(o2, 0x5A, sizeof o2);
      KT_CHECK_EQ(parseNameList(tok.c_str(), o2, 1), 1);
      KT_CHECK_EQ((int)strlen(o2[0]), len < ProfName - 1 ? len : ProfName - 1);
    }
    // a token that only differs past the truncation point is a duplicate of the truncated first one
    std::string a(47, 'x'), b = a + "yz";
    char o3[3][ProfName]; memset(o3, 0, sizeof o3);
    KT_CHECK_EQ(parseNameList((a + "," + b).c_str(), o3, 3), 1);
    // trailing blank handling, tabs, empty tokens
    char o4[4][ProfName]; memset(o4, 0, sizeof o4);
    KT_CHECK_EQ(parseNameList(" \t a b \t,, ,\tc ", o4, 4), 2);
    KT_CHECK(strcmp(o4[0], "a b") == 0); KT_CHECK(strcmp(o4[1], "c") == 0);
  }

  // ---- nameMatches
  {
    KT_CHECK(!nameMatches(" x", 0, "x"));          // a leading blank alone is not a suffix match
    KT_CHECK(nameMatches("a x", 0, "x"));          // one-char prefix
    KT_CHECK(nameMatches("ab x", 0, "x"));
    KT_CHECK(nameMatches("Master Flurry", 0, "flurry"));
    KT_CHECK(!nameMatches("MasterFlurry", 0, "Flurry"));
    KT_CHECK(nameMatches("Ax", 0, "Ax"));
    KT_CHECK(nameMatches("0a", 0, "0a"));          // starts with '0' but is not hex
    KT_CHECK(nameMatches("ax", 0, "AX"));          // 'x' second but not a hex prefix
    KT_CHECK(nameMatches("1x", 0, "1x"));
    KT_CHECK(nameMatches("x", 0, "x"));
    // hex tokens
    KT_CHECK(nameMatches(nullptr, 0x5, "0x5"));
    KT_CHECK(nameMatches("zz", 0x5, "0X5"));
    KT_CHECK(nameMatches("zz", 0x12345678u, "0x12345678"));
    KT_CHECK(nameMatches("zz", 0x2345678u, "0x02345678"));
    KT_CHECK(!nameMatches("zz", 0x23456789u, "0x123456789"));   // 9 digits: never
    KT_CHECK(!nameMatches("zz", 0x23456789u, "0x1234567890"));
    KT_CHECK(!nameMatches("zz", 0, "0x"));
    KT_CHECK(!nameMatches("0x", 0, "0x"));
    KT_CHECK(!nameMatches("zz", 0xFFFFFFFFu, "0xg"));
    KT_CHECK(!nameMatches("zz", 0xFFFFFFFFu, "0xG"));
    KT_CHECK(!nameMatches("zz", 0x1, "0x1g"));
    KT_CHECK(!nameMatches("zz", 0xA, "0xAZ"));
    KT_CHECK(!nameMatches("zz", 0xF, "0xF@"));
    KT_CHECK(!nameMatches("zz", 0xFFFFFFFFu, "0x@"));
    KT_CHECK(!nameMatches("zz", 0xFFFFFFFFu, "0x/"));
    KT_CHECK(!nameMatches("zz", 0xFFFFFFFFu, "0x:"));
    KT_CHECK(!nameMatches("zz", 0xFFFFFFFFu, "0x`"));
    KT_CHECK(!nameMatches("zz", 0xFFFFFFFFu, "0x["));
    KT_CHECK(nameMatches("zz", 0xABCDEFu, "0xabcdef")); KT_CHECK(nameMatches("zz", 0xABCDEFu, "0xABCDEF"));
    KT_CHECK(nameMatches("zz", 0x9u, "0x9")); KT_CHECK(nameMatches("zz", 0x0u, "0x0"));
    KT_CHECK(nameMatches("zz", 0xAu, "0xa")); KT_CHECK(nameMatches("zz", 0xFu, "0xf")); KT_CHECK(nameMatches("zz", 0xFu, "0xF"));
    KT_CHECK(!nameMatches("zz", 0x10u, "0x1")); KT_CHECK(!nameMatches("zz", 0u, "0x1"));
    // detail helpers through the public surface: every hex digit and its neighbours
    for (int c = 0; c < 256; ++c) {
      char tok[5] = { '0', 'x', (char)c, 0, 0 };
      if (c == 0) continue;
      int want = -1;
      if (c >= '0' && c <= '9') want = c - '0'; else if (c >= 'a' && c <= 'f') want = c - 'a' + 10; else if (c >= 'A' && c <= 'F') want = c - 'A' + 10;
      for (uint32_t key : { 0u, 1u, 9u, 10u, 15u, 16u, 0xFFFFFFFFu, 0xFFFFFFF0u }) {
        const bool exp = want >= 0 && key == (uint32_t)want;
        if (nameMatches("zz", key, tok) != exp) KT_CHECK(false);
      }
    }
    for (int c = 0; c < 256; ++c) {   // case folding: every char vs. its case twin
      if (c == 0) continue;
      char a[3] = { (char)c, 0, 0 }, b[3] = { (char)c, 0, 0 };
      if (c >= 'a' && c <= 'z') b[0] = (char)(c - 32); else if (c >= 'A' && c <= 'Z') b[0] = (char)(c + 32);
      const bool hexLead = false;
      (void)hexLead;
      if (c == '0') continue;
      KT_CHECK(nameMatches(a, 0, b));
      char d[3] = { (char)(c == 'q' ? 'r' : 'q'), 0, 0 };
      if (c != 'q' && c != 'Q' && c != 'r' && c != 'R') KT_CHECK(!nameMatches(d, 0, a));
    }
    KT_CHECK(!nameMatches("@", 0, "`")); KT_CHECK(!nameMatches("[", 0, "{")); KT_CHECK(!nameMatches("`", 0, "@")); KT_CHECK(!nameMatches("{", 0, "["));
    KT_CHECK(nameMatches("Z", 0, "z")); KT_CHECK(nameMatches("z", 0, "Z")); KT_CHECK(nameMatches("A", 0, "a")); KT_CHECK(nameMatches("a", 0, "A"));
    KT_CHECK(!nameMatches("a", 0, "b")); KT_CHECK(!nameMatches("A", 0, "B"));
    KT_CHECK(!nameMatches("ab", 0, "ac"));   // equal length, differs
    KT_CHECK(!nameMatches("abc", 0, "abcd"));
  }

  // ---- orderGroup
  {
    GroupView g; memset(&g, 0, sizeof g); Names nm; Profile p; defaultProfile(p);
    // hex token vs a decoy entry whose NAME is that token text
    g.count = 2; g.e[0].key = 0x99; g.e[1].key = 0x10; nm.set(0, "0x10"); nm.set(1, "Alpha");
    strcpy(p.pin[1][0], "0x10"); p.nPin[1] = 1;
    KT_CHECK(order(g, nm, p, 1) == (std::vector<int>{ 1, 0 }));
    strcpy(p.pin[1][0], "0X10");
    KT_CHECK(order(g, nm, p, 1) == (std::vector<int>{ 1, 0 }));
    // exact-name rule for plain tokens, incl. tokens that start with '0' / have 'x' second
    for (const char* tok : { "Ax", "0k", "ax", "aX", "1x", "xk", "q" }) {
      GroupView h; memset(&h, 0, sizeof h); Names hn; Profile q; defaultProfile(q);
      h.count = 2; h.e[0].key = 1; h.e[1].key = 2;
      std::string big = std::string("Big ") + tok;
      hn.set(0, big.c_str()); hn.set(1, tok);
      strcpy(q.hide[0][0], tok); q.nHide[0] = 1;
      KT_CHECK(order(h, hn, q, 0) == (std::vector<int>{ 0 }));    // only the exact entry is hidden
      hn.set(0, tok); hn.set(1, big.c_str());
      KT_CHECK(order(h, hn, q, 0) == (std::vector<int>{ 1 }));
      q.nHide[0] = 0; strcpy(q.pin[0][0], tok); q.nPin[0] = 1;
      hn.set(0, big.c_str()); hn.set(1, tok);
      KT_CHECK(order(h, hn, q, 0) == (std::vector<int>{ 1, 0 }));   // only the exact entry is pinned first
    }
    // a hex token pins by key even if another entry is named with the token text
    // gi bounds
    GroupView h; memset(&h, 0, sizeof h); Names hn; Profile q; defaultProfile(q);
    h.count = 3; hn.set(0, "a"); hn.set(1, "b"); hn.set(2, "c");
    for (int i = 0; i < 3; ++i) h.e[i].key = 10 + i;
    int o[4] = { -1, -1, -1, -1 };
    KT_CHECK_EQ(orderGroup(h, hn.n, q, 0, o, 4), 3); KT_CHECK_EQ(o[2], 2);
    KT_CHECK_EQ(orderGroup(h, hn.n, q, 2, o, 4), 3);
    KT_CHECK_EQ(orderGroup(h, hn.n, q, -1, o, 4), 0); KT_CHECK_EQ(orderGroup(h, hn.n, q, 3, o, 4), 0);
    KT_CHECK_EQ(orderGroup(h, nullptr, q, 0, o, 4), 0); KT_CHECK_EQ(orderGroup(h, hn.n, q, 0, nullptr, 4), 0);
    KT_CHECK_EQ(orderGroup(h, hn.n, q, 0, o, 0), 0); KT_CHECK_EQ(orderGroup(h, hn.n, q, 0, o, -1), 0);
    KT_CHECK(order(h, hn, q, 1, 1) == (std::vector<int>{ 0 }));      // maxOut caps, exact-size buffer
    KT_CHECK(order(h, hn, q, 1, 2) == (std::vector<int>{ 0, 1 }));
    KT_CHECK(order(h, hn, q, 1, 3) == (std::vector<int>{ 0, 1, 2 }));
    KT_CHECK(order(h, hn, q, 1, 9) == (std::vector<int>{ 0, 1, 2 }));
    // entries / tokens past the live count must be ignored (stale data beyond count, stale tokens beyond nHide/nPin)
    {
      GroupView s; memset(&s, 0, sizeof s); Names sn; Profile t; defaultProfile(t);
      s.count = 2; s.e[0].key = 1; s.e[1].key = 2; s.e[2].key = 3; sn.set(0, "Power Attack"); sn.set(1, "Other"); sn.set(2, "Attack");
      strcpy(t.hide[1][0], "Attack"); t.nHide[1] = 1;
      KT_CHECK(order(s, sn, t, 1) == (std::vector<int>{ 1 }));       // names[2]=="Attack" is beyond the count: suffix rule applies
      t.nHide[1] = 0; strcpy(t.hide[1][0], "Other");                 // stale hide token beyond nHide
      KT_CHECK(order(s, sn, t, 1) == (std::vector<int>{ 0, 1 }));
      strcpy(t.pin[1][0], "Zed"); t.nPin[1] = 0;                     // stale pin token beyond nPin
      sn.set(1, "Zed");
      KT_CHECK(order(s, sn, t, 1) == (std::vector<int>{ 0, 1 }));
      t.nPin[1] = 1; strcpy(t.pin[1][1], "Power Attack");
      KT_CHECK(order(s, sn, t, 1) == (std::vector<int>{ 1, 0 }));
      // an entry past the count that matches a pin must not be placed
      GroupView u; memset(&u, 0, sizeof u); Names un; Profile w; defaultProfile(w);
      u.count = 1; u.e[0].key = 1; un.set(0, "One"); u.e[1].key = 2; un.set(1, "Two");
      strcpy(w.pin[0][0], "Nope"); strcpy(w.pin[0][1], "Two"); w.nPin[0] = 2;
      KT_CHECK(order(u, un, w, 0) == (std::vector<int>{ 0 }));
      // hide beats pin; pin first match only; stable engine order for the rest
    }
    // the first entry can be pinned / hidden; the pin for entry 0 and ties
    {
      GroupView s; memset(&s, 0, sizeof s); Names sn; Profile t; defaultProfile(t);
      s.count = 3; for (int i = 0; i < 3; ++i) s.e[i].key = 20 + i;
      sn.set(0, "Alpha"); sn.set(1, "Beta"); sn.set(2, "Gamma");
      strcpy(t.pin[2][0], "Alpha"); t.nPin[2] = 1;
      KT_CHECK(order(s, sn, t, 2) == (std::vector<int>{ 0, 1, 2 }));
      strcpy(t.pin[2][0], "Gamma"); strcpy(t.pin[2][1], "Alpha"); t.nPin[2] = 2;
      KT_CHECK(order(s, sn, t, 2) == (std::vector<int>{ 2, 0, 1 }));
      strcpy(t.hide[2][0], "Alpha"); t.nHide[2] = 1;
      KT_CHECK(order(s, sn, t, 2) == (std::vector<int>{ 2, 1 }));
    }
    // all 16 entries: pin/hide/sort touch every array slot (sanitizers catch an undersized scratch array)
    {
      GroupView s; memset(&s, 0, sizeof s); Names sn; Profile t; defaultProfile(t);
      s.count = 16; char nbuf[16][16];
      for (int i = 0; i < 16; ++i) { s.e[i].key = 100 + i; s.e[i].count = (uint32_t)(i % 5); snprintf(nbuf[i], sizeof nbuf[i], "N%d", i); sn.set(i, nbuf[i]); }
      strcpy(t.pin[1][0], "N15"); strcpy(t.pin[1][1], "N0"); t.nPin[1] = 2;
      strcpy(t.hide[1][0], "N7"); t.nHide[1] = 1; t.byCount[1] = true;
      std::vector<int> r = order(s, sn, t, 1);
      KT_CHECK_EQ((int)r.size(), 15); KT_CHECK_EQ(r[0], 15); KT_CHECK_EQ(r[1], 0);
      // rest by count descending, stable: counts i%5 -> 4: {4,9,14}, 3: {3,8,13}, 2: {2,12} (7 hidden), 1: {1,6,11}, 0: {5,10}
      const int want[] = { 15, 0, 4, 9, 14, 3, 8, 13, 2, 12, 1, 6, 11, 5, 10 };
      for (int i = 0; i < 15; ++i) KT_CHECK_EQ(r[i], want[i]);
      t.nPin[1] = 0; t.nHide[1] = 0; t.byCount[1] = false;
      r = order(s, sn, t, 1); KT_CHECK_EQ((int)r.size(), 16); KT_CHECK_EQ(r[15], 15);
      t.byCount[1] = true;
      r = order(s, sn, t, 1); KT_CHECK_EQ((int)r.size(), 16); KT_CHECK_EQ(r[0], 4);
      strcpy(t.hide[1][0], "N3"); t.nHide[1] = 1;
      r = order(s, sn, t, 1); KT_CHECK_EQ((int)r.size(), 15);
    }
    // byCount sort: the insertion loop starts at the second 'rest' entry; pins before it stay put
    {
      GroupView s; memset(&s, 0, sizeof s); Names sn; Profile t; defaultProfile(t);
      s.count = 4; const uint32_t cnts[4] = { 1, 9, 5, 7 };
      for (int i = 0; i < 4; ++i) { s.e[i].key = i + 1; s.e[i].count = cnts[i]; }
      sn.set(0, "a"); sn.set(1, "b"); sn.set(2, "c"); sn.set(3, "d");
      t.byCount[0] = true;
      KT_CHECK(order(s, sn, t, 0) == (std::vector<int>{ 1, 3, 2, 0 }));
      strcpy(t.pin[0][0], "a"); t.nPin[0] = 1;                    // pinned entry has the lowest count and stays first
      KT_CHECK(order(s, sn, t, 0) == (std::vector<int>{ 0, 1, 3, 2 }));
      strcpy(t.pin[0][0], "c");
      KT_CHECK(order(s, sn, t, 0) == (std::vector<int>{ 2, 1, 3, 0 }));
      s.e[0].count = s.e[1].count = s.e[2].count = s.e[3].count = 3;   // ties stay in engine order
      t.nPin[0] = 0;
      KT_CHECK(order(s, sn, t, 0) == (std::vector<int>{ 0, 1, 2, 3 }));
      s.e[2].count = 4;
      KT_CHECK(order(s, sn, t, 0) == (std::vector<int>{ 2, 0, 1, 3 }));
      s.count = 1; KT_CHECK(order(s, sn, t, 0) == (std::vector<int>{ 0 }));
      s.count = 0; KT_CHECK(order(s, sn, t, 0).empty());
      s.count = -4; KT_CHECK(order(s, sn, t, 0).empty());
    }
  }

  // ---- formatProfile
  {
    Profile p; defaultProfile(p); strcpy(p.pin[0][0], "A"); p.nPin[0] = 1; strcpy(p.hide[2][0], "B"); strcpy(p.hide[2][1], "C"); p.nHide[2] = 2;
    char buf[256]; memset(buf, 'x', sizeof buf);
    const int len = formatProfile(p, buf, sizeof buf);
    KT_CHECK(len > 0); KT_CHECK_EQ((int)strlen(buf), len);
    KT_CHECK(strcmp(buf, "Pin0=A\nHide0=\nSort0=engine\nPin1=\nHide1=\nSort1=engine\nPin2=\nHide2=B,C\nSort2=count") == 0);
    // guards
    KT_CHECK_EQ(formatProfile(p, nullptr, 64), -1); KT_CHECK_EQ(formatProfile(p, nullptr, 0), -1);
    char one[4] = { 'x', 'x', 'x', 'x' };
    KT_CHECK_EQ(formatProfile(p, one, 0), -1); KT_CHECK_EQ((int)one[0], 'x');       // size 0: buffer untouched
    KT_CHECK_EQ(formatProfile(p, one, -3), -1); KT_CHECK_EQ((int)one[0], 'x');
    KT_CHECK_EQ(formatProfile(p, one, 1), -1); KT_CHECK_EQ((int)one[0], 0);          // size 1: NUL only
    for (int sz = 1; sz <= len; ++sz) {   // every size below the exact fit fails and leaves an empty string
      std::vector<char> v((size_t)sz, 'x');
      KT_CHECK_EQ(formatProfile(p, v.data(), sz), -1); KT_CHECK_EQ((int)v[0], 0);
    }
    { std::vector<char> v((size_t)len + 1, 'x'); KT_CHECK_EQ(formatProfile(p, v.data(), len + 1), len); KT_CHECK_EQ((int)v[(size_t)len], 0); }
    // a corrupt (over-large) count never prints more than ProfMax entries
    Profile q; defaultProfile(q);
    for (int i = 0; i < 3 * ProfMax; ++i) { /* fill the whole pin block so entries beyond ProfMax of group 0 are group 1's */ }
    for (int i = 0; i < ProfMax; ++i) { snprintf(q.pin[0][i], ProfName, "p%d", i); snprintf(q.pin[1][i], ProfName, "z%d", i); }
    q.nPin[0] = 20;
    char big[512]; const int bl = formatProfile(q, big, sizeof big);
    KT_CHECK(bl > 0); KT_CHECK(strstr(big, "p7\n") != nullptr); KT_CHECK(strstr(big, "z0") == nullptr || strstr(big, "Pin1=z0") != nullptr);
    KT_CHECK(strstr(big, "p7,z0") == nullptr);
    q.nHide[0] = 20; for (int i = 0; i < ProfMax; ++i) { snprintf(q.hide[0][i], ProfName, "h%d", i); snprintf(q.hide[1][i], ProfName, "y%d", i); }
    KT_CHECK(formatProfile(q, big, sizeof big) > 0); KT_CHECK(strstr(big, "h7,y0") == nullptr);
  }

  // ---- parseProfileLine
  {
    Profile q; defaultProfile(q);
    // trailing blanks and the empty value: val[-1] must not be read
    char* heap = (char*)malloc(1); heap[0] = 0;
    KT_CHECK(!parseProfileLine(q, "Sort0", heap));
    free(heap);
    KT_CHECK(!parseProfileLine(q, "Sort0", " "));
    KT_CHECK(!parseProfileLine(q, "Sort0", "\t \t"));
    KT_CHECK(parseProfileLine(q, "Sort1", "\tcount\t")); KT_CHECK(q.byCount[1]);
    KT_CHECK(parseProfileLine(q, "Sort1", "  engine  ")); KT_CHECK(!q.byCount[1]);
    KT_CHECK(!parseProfileLine(q, "Sort1", "counts")); KT_CHECK(!parseProfileLine(q, "Sort1", "coun"));
    KT_CHECK(parseProfileLine(q, "Hide2", "x, y")); KT_CHECK_EQ(q.nHide[2], 2);
    KT_CHECK(parseProfileLine(q, "hide0", nullptr)); KT_CHECK_EQ(q.nHide[0], 0);
    KT_CHECK(parseProfileLine(q, "PIN2", "m")); KT_CHECK_EQ(q.nPin[2], 1);
    KT_CHECK(parseProfileLine(q, "pin0", "n")); KT_CHECK_EQ(q.nPin[0], 1);
    KT_CHECK(!parseProfileLine(q, "Pin-1", "a")); KT_CHECK(!parseProfileLine(q, "Pin9", "a")); KT_CHECK(!parseProfileLine(q, "Pin12", "a"));
    KT_CHECK(!parseProfileLine(q, "Hide", "a")); KT_CHECK(!parseProfileLine(q, "Hide3", "a")); KT_CHECK(!parseProfileLine(q, "Sort3", "count"));
    KT_CHECK(!parseProfileLine(q, "Sort", "count")); KT_CHECK(!parseProfileLine(q, "Sort/", "count")); KT_CHECK(!parseProfileLine(q, "Pin0x", "a"));
    KT_CHECK(!parseProfileLine(q, "Pinn0", "a")); KT_CHECK(!parseProfileLine(q, "Sin0", "a"));
  }
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
  // ================= CH6 mutation-gate additions (doc 31 CH6): contract pins, guards, reference-model sweeps =================
  ch6Basics();
  ch6GuardsAndNulls();
  ch6LayoutSweeps();
  ch6Profile();
  return kt::Report("combat_logic_test");
}
