// RichMenuTypographyPreview.h -- layout of the config page's own preview area (doc 53).
// The page splits the right half (SetDescSplit): the top part is this preview (four labels: title, subtitle, rules, body, drawn with the
// same fonts/colours as the in-game description box), the bottom part stays mod-config's description pane for the hovered row.
// Pure geometry, no engine calls, so it is unit-tested on the host.
#ifndef RMT_PREVIEW_H
#define RMT_PREVIEW_H

namespace rmtprev {

constexpr int kRows = 4;                 // 0 title, 1 subtitle, 2 rules, 3 body
constexpr int kPad = 10;                 // inset from the pane edge
constexpr int kGapAfter[kRows] = { 6, 4, 8, 0 };   // gap below a row, only when a later row exists
constexpr int kSplitPermille = 650;      // preview share of the description rect (mod-config clamps to 600..850)

struct Rect { int x = 0, y = 0, w = 0, h = 0; };

struct Stack {
    Rect r[kRows];
    bool show[kRows] = { false, false, false, false };
    int shown = 0;      // rows laid out
    int dropped = 0;    // rows that did not fit (hidden, never overlapped)
};

// h[i] = measured height of row i in px (<= 0: the row is absent). Rows stack top-down inside the pane inset by kPad. A row that does not
// fit completely is hidden together with every row after it, so nothing is drawn outside the pane or over a neighbour.
inline Stack stackRows(const Rect& pane, const int h[kRows]) {
    Stack s;
    const int innerW = pane.w - 2 * kPad;
    const int bottom = pane.y + pane.h - kPad;
    if (innerW <= 0 || pane.h <= 2 * kPad) { for (int i = 0; i < kRows; ++i) if (h[i] > 0) ++s.dropped; return s; }
    int y = pane.y + kPad;
    bool full = false;
    for (int i = 0; i < kRows; ++i) {
        if (h[i] <= 0) continue;
        if (full || y + h[i] > bottom) { full = true; ++s.dropped; continue; }
        s.r[i] = Rect{ pane.x + kPad, y, innerW, h[i] };
        s.show[i] = true;
        ++s.shown;
        bool later = false;
        for (int j = i + 1; j < kRows; ++j) if (h[j] > 0) { later = true; break; }
        y += h[i] + (later ? kGapAfter[i] : 0);
    }
    return s;
}

}   // namespace rmtprev

#endif
