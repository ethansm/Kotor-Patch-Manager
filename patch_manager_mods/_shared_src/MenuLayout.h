// MenuLayout.h -- pure geometry for inserting a row into a vertical menu column (header-only, ints only, no engine access, no STL).
// Card c12. Host test: menu_layout_test.cpp.
//
//   Rect col[n];                                   // column rects, top -> bottom, all with the same pitch
//   Rect nw, moved[n - insertAfter - 1];
//   if (menulayout::columnReflow(col, n, insertAfter, &nw, moved)) { ... }
//
//   columnReflow: the new row takes the rect of the row that followed insertAfter (col[insertAfter+1]). Each later row i
//     (insertAfter+1 .. n-2) moves to the rect of row i+1; the last row moves down by one pitch, keeping its x/w/h.
//     moved[0] is the row that was insertAfter+1. Output pointers must not alias col.
//   slotBelow: y of the k-th free slot below the last row.
//   statusStrip: a 28 px strip under a view, clamped so it never passes floorY.
//
// Survey of the vanilla pause column: 8 rows, x 78, w 318, h 32, y 134 + 36*i.
#pragma once

namespace menulayout {

struct Rect {
    int x;
    int y;
    int w;
    int h;
};

// Returns false, writing NOTHING, unless: pointers are non-null, insertAfter >= 0, n >= insertAfter + 2,
// y is strictly ascending over the whole column, every gap is within +-2 px of the pitch, and pitch > 0.
// The pitch is col[insertAfter+1].y - col[insertAfter].y.
inline bool columnReflow(const Rect* col, int n, int insertAfter, Rect* outNew, Rect* outMoved) {
    if (!col || !outNew || !outMoved) return false;
    if (insertAfter < 0 || insertAfter > n - 2) return false;

    const int pitch = col[insertAfter + 1].y - col[insertAfter].y;
    if (pitch <= 0) return false;
    for (int i = 0; i + 1 < n; ++i) {
        const int gap = col[i + 1].y - col[i].y;
        if (gap <= 0) return false;
        if (gap - pitch > 2 || pitch - gap > 2) return false;
    }

    *outNew = col[insertAfter + 1];

    const int moved = n - insertAfter - 1;
    for (int j = 0; j < moved - 1; ++j) outMoved[j] = col[insertAfter + 2 + j];
    Rect last = col[n - 1];
    last.y += pitch;
    outMoved[moved - 1] = last;
    return true;
}

// y of the k-th free slot below `last` (k = 1 is the first slot under it).
inline int slotBelow(const Rect& last, int pitch, int k) {
    return last.y + k * pitch;
}

// Strip under `view`: same x/w, y = view bottom + 4, h = min(28, floorY - y - 2), clamped to 0 (0 = no room).
inline Rect statusStrip(const Rect& view, int floorY) {
    Rect r;
    r.x = view.x;
    r.y = view.y + view.h + 4;
    int h = floorY - r.y - 2;
    if (h > 28) h = 28;
    if (h < 0) h = 0;
    r.h = h;
    r.w = view.w;
    return r;
}

}  // namespace menulayout
