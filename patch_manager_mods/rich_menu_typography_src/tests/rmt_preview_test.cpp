// rmt_preview_test.cpp -- host test of the config page preview layout (RichMenuTypographyPreview.h, doc 53): rows stack top-down in the
// pane, absent rows take no space, gaps only below a row with a later neighbour, and a row that does not fit is hidden with everything
// after it (never overlapped, never outside the pane).

#include "RichMenuTypographyPreview.h"
#include "check.h"

using namespace rmtprev;

static bool overlapsOrEscapes(const Rect& pane, const Stack& s) {
    int lastBottom = pane.y;
    for (int i = 0; i < kRows; ++i) {
        if (!s.show[i]) continue;
        const Rect& r = s.r[i];
        if (r.x < pane.x || r.x + r.w > pane.x + pane.w) return true;
        if (r.y < lastBottom || r.y + r.h > pane.y + pane.h) return true;
        lastBottom = r.y + r.h;
    }
    return false;
}

static void test_full_stack() {
    const Rect pane{ 100, 50, 400, 600 };
    const int h[kRows] = { 30, 20, 100, 200 };
    const Stack s = stackRows(pane, h);
    kt::CheckEq("all four rows shown", s.shown, 4);
    kt::CheckEq("none dropped", s.dropped, 0);
    kt::CheckEq("title y = pane top + pad", s.r[0].y, 50 + kPad);
    kt::CheckEq("subtitle y = title bottom + gap", s.r[1].y, s.r[0].y + 30 + kGapAfter[0]);
    kt::CheckEq("rules y = subtitle bottom + gap", s.r[2].y, s.r[1].y + 20 + kGapAfter[1]);
    kt::CheckEq("body y = rules bottom + gap", s.r[3].y, s.r[2].y + 100 + kGapAfter[2]);
    kt::CheckEq("row width = pane width - 2 pad", s.r[0].w, 400 - 2 * kPad);
    kt::CheckEq("row x = pane x + pad", s.r[0].x, 100 + kPad);
    kt::Check("no overlap, nothing outside the pane", !overlapsOrEscapes(pane, s));
}

static void test_absent_rows() {
    const Rect pane{ 0, 0, 300, 400 };
    const int h[kRows] = { 0, 0, 80, 120 };   // no title, no subtitle
    const Stack s = stackRows(pane, h);
    kt::CheckEq("two rows shown", s.shown, 2);
    kt::Check("title hidden", !s.show[0]);
    kt::Check("subtitle hidden", !s.show[1]);
    kt::CheckEq("rules starts at the top pad", s.r[2].y, kPad);
    kt::CheckEq("body follows rules with the rules gap", s.r[3].y, kPad + 80 + kGapAfter[2]);
    const int onlyBody[kRows] = { 0, 0, 0, 90 };
    const Stack b = stackRows(pane, onlyBody);
    kt::CheckEq("body alone starts at the top pad", b.r[3].y, kPad);
    const int none[kRows] = { 0, 0, 0, 0 };
    kt::CheckEq("nothing to show", stackRows(pane, none).shown, 0);
}

static void test_overflow_drops_tail() {
    const Rect pane{ 0, 0, 300, 200 };
    const int h[kRows] = { 30, 20, 90, 80 };   // title + sub + rules fit (inner height 180), body does not
    const Stack s = stackRows(pane, h);
    kt::Check("body dropped", !s.show[3]);
    kt::CheckEq("one row dropped", s.dropped, 1);
    kt::Check("earlier rows kept", s.show[0] && s.show[1] && s.show[2]);
    kt::Check("no overlap, nothing outside the pane", !overlapsOrEscapes(pane, s));
    // a short row after a tall one that did not fit must not sneak in (order is preserved)
    const int h2[kRows] = { 0, 0, 500, 20 };
    const Stack t = stackRows(pane, h2);
    kt::Check("tall rules hidden", !t.show[2]);
    kt::Check("short body after a dropped row stays hidden", !t.show[3]);
    kt::CheckEq("both dropped", t.dropped, 2);
}

static void test_degenerate_pane() {
    const int h[kRows] = { 10, 10, 10, 10 };
    const Stack narrow = stackRows(Rect{ 0, 0, 2 * kPad, 300 }, h);
    kt::CheckEq("zero inner width shows nothing", narrow.shown, 0);
    kt::CheckEq("zero inner width drops every present row", narrow.dropped, 4);
    const Stack flat = stackRows(Rect{ 0, 0, 300, 2 * kPad }, h);
    kt::CheckEq("zero inner height shows nothing", flat.shown, 0);
    kt::Check("split permille inside mod-config's clamp", kSplitPermille >= 600 && kSplitPermille <= 850);
}

int main() {
    test_full_stack();
    test_absent_rows();
    test_overflow_drops_tail();
    test_degenerate_pane();
    return kt::Report("rmt_preview_test");
}
