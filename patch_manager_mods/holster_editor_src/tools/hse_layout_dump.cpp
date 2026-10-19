// Host dump of the holster editor page layout (HseLayoutV3.h) as JSON, for scripts/holster_editor_mockup.py (offline caption/box renderer).
// Build (from patch_manager_mods/holster_editor_src): g++ -std=c++17 -Wall -Wextra -I../_shared_src -Werror tools/hse_layout_dump.cpp -o <exe>
// Usage: hse_layout_dump <col x y w h> <view x y w h> <preview x y w h> [<desc x y w h>]   (12 or 16 ints; hint = none, rowFrac = the editor default 0.5;
// desc = the SHRUNK description pane (mod-config LB_DESC) -> left + right description boxes, omitted = the old layout without them;
// an optional 17th int = the ini desc_cover_px (default hse::kV3CoverDefault; the pane is grown by it with coverDescPane exactly like the editor does, no screen clamp))
#include "../HseLayoutV3.h"
#include <cstdio>
#include <cstdlib>

// Mirror of itemIsButton in HolsterEditor.cpp (~line 387): buttons of the page; every other item is a label, PI_VIEW is the 3D view control.
static const char* itemKind(int i) {
    using namespace hse;
    if (i == PI_VIEW) return "view";
    if (i == PI_HINT || i == PI_HINTR) return "frame";                   // the green description boxes (drawn as a frame, no caption)
    if (i == PI_HINT + 1 || i == PI_HINTR + 1) return "desctext";        // their text areas (top-left aligned, word wrapped)
    if (i == PI_MEMBER_PREV || i == PI_MEMBER_NEXT || i == PI_SAVE || i == PI_REVERT || i == PI_RESET_SCOPE || i == PI_VIEW_RESET || i == PI_VIEW_FLIP) return "button";
    if (i >= PI_WEAPON && i < PI_WEAPON + kV3Weapons) return "button";
    if (i >= PI_BONE && i < PI_BONE + kV3Bones) return "button";
    if (i >= PI_SCOPE && i < PI_SCOPE + kV3Scopes) return "button";
    if (i >= PI_MODE && i < PI_MODE + kV3Modes) return "button";
    if (i >= PI_STANCE && i < PI_STANCE + kV3Stances) return "button";
    for (int g = 0; g < kV3Groups; ++g)
        if (i == PI_GROUP + g * 2 + 1) return "button";
    for (int r = 0; r < kV3Rows; ++r)
        if (i == PI_ROW + r * kRowStride + 4) return "button";
    return "label";
}

static void pane(const char* name, const editkit::Box& b, bool last) {
    std::printf("\"%s\":[%d,%d,%d,%d]%s", name, b.x, b.y, b.w, b.h, last ? "" : ",");
}

int main(int argc, char** argv) {
    if (argc != 13 && argc != 17 && argc != 18) {
        std::fprintf(stderr, "usage: %s <col x y w h> <view x y w h> <preview x y w h> [<desc x y w h> [cover_px]]\n", argv[0]);
        return 2;
    }
    int v[17] = {0};
    for (int i = 0; i < argc - 1; ++i) v[i] = std::atoi(argv[i + 1]);
    const editkit::Box col{v[0], v[1], v[2], v[3]}, view{v[4], v[5], v[6], v[7]}, prev{v[8], v[9], v[10], v[11]}, desc{v[12], v[13], v[14], v[15]};
    const editkit::Box none{0, 0, 0, 0};
    const float rowFrac[hse::kV3Rows] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};   // HolsterEditor.cpp Editor::rowFrac default
    const int cover = argc == 18 ? v[16] : hse::kV3CoverDefault;
    const editkit::Box covered = hse::coverDescPane(desc, prev, none, cover);   // HolsterEditor.cpp applyLayout (the screen size is unknown here)
    const hse::PageLayoutV3 L = hse::pageLayoutV3(col, view, prev, none, rowFrac, covered);
    editkit::Box all[hse::kPageItemsV3];
    const int n = hse::pageBoxesV3(L, all);
    std::printf("{\"panes\":{");
    pane("column", col, false);
    pane("view", view, false);
    pane("preview", prev, false);
    pane("desc", desc, true);
    std::printf("},\"rows\":{\"leftRowH\":%d,\"inspRowH\":%d,\"barH\":%d},\"items\":[", L.leftRowH, L.inspRowH, L.barH);
    for (int i = 0; i < n; ++i)
        std::printf("%s{\"i\":%d,\"kind\":\"%s\",\"box\":[%d,%d,%d,%d]}", i ? "," : "", i, itemKind(i), all[i].x, all[i].y, all[i].w, all[i].h);
    std::printf("]}\n");
    return 0;
}
