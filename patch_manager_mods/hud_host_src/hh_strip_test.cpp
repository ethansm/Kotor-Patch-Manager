// Host test of the hud-host portrait-strip math (HhStrip.h, K2HudApi::StripSlot, doc 24 CS10): right-to-left placement, per-order
// width registration (last wins), unregistered orders count 0, order bounds, vertical centring, plus the live 3840x1600 companion vit.
#include "HhStrip.h"
#include "check.h"

static void tPlacement() {
    hh::Strip s;
    hh::Box r{};
    const hh::Box vit{ 3671, 1285, 47, 96 };
    // order 0 = right-most: directly left of the vit bar
    KT_CHECK_EQ(s.slot(0, vit, 38, 38, &r), 1);
    KT_CHECK_EQ(r.x, 3671 - 38); KT_CHECK_EQ(r.y, 1285 + 29); KT_CHECK_EQ(r.w, 38); KT_CHECK_EQ(r.h, 38);
    // order 1 sits left of order 0's width
    KT_CHECK_EQ(s.slot(1, vit, 100, 38, &r), 1);
    KT_CHECK_EQ(r.x, 3671 - 38 - 100); KT_CHECK_EQ(r.w, 100);
    // order 2 stacks both
    KT_CHECK_EQ(s.slot(2, vit, 10, 10, &r), 1);
    KT_CHECK_EQ(r.x, 3671 - 38 - 100 - 10); KT_CHECK_EQ(r.y, 1285 + (96 - 10) / 2);
    // re-register order 0: last width wins and shifts the others
    KT_CHECK_EQ(s.slot(0, vit, 50, 38, &r), 1);
    KT_CHECK_EQ(s.slot(1, vit, 100, 38, &r), 1);
    KT_CHECK_EQ(r.x, 3671 - 50 - 100);
}

static void tUnregistered() {
    hh::Strip s;
    hh::Box r{};
    const hh::Box vit{ 1000, 200, 47, 96 };
    KT_CHECK_EQ(s.slot(1, vit, 60, 20, &r), 1);              // order 0 never registered: counts 0
    KT_CHECK_EQ(r.x, 1000 - 60);
    KT_CHECK_EQ(s.slot(3, vit, 5, 20, &r), 1);              // order 2 gap: counts 0
    KT_CHECK_EQ(r.x, 1000 - 60 - 5);
    KT_CHECK_EQ(s.sumBefore(0), 0); KT_CHECK_EQ(s.sumBefore(4), 65);
}

static void tBounds() {
    hh::Strip s;
    hh::Box r{ 7, 7, 7, 7 };
    const hh::Box vit{ 500, 0, 47, 96 };
    KT_CHECK_EQ(s.slot(-1, vit, 10, 10, &r), 0); KT_CHECK_EQ(r.x, 7);     // refused: out untouched
    KT_CHECK_EQ(s.slot(hh::Strip::kOrders, vit, 10, 10, &r), 0); KT_CHECK_EQ(r.x, 7);
    KT_CHECK_EQ(s.slot(0, vit, 10, 10, nullptr), 0);
    KT_CHECK_EQ(s.sumBefore(0), 0);                                        // refused calls registered nothing
    KT_CHECK_EQ(s.slot(hh::Strip::kOrders - 1, vit, 10, 10, &r), 1);       // last valid order (7) is accepted
    KT_CHECK_EQ(r.x, 500 - 10);
    KT_CHECK_EQ(s.slot(0, vit, -5, 10, &r), 1);                            // negative width -> 0
    KT_CHECK_EQ(r.x, 500); KT_CHECK_EQ(r.w, 0);
}

static void tChanged() {
    hh::Strip s;
    hh::Box r{};
    bool ch = false;
    const hh::Box vit{ 500, 0, 47, 96 };
    KT_CHECK_EQ(s.slot(0, vit, 38, 38, &r, &ch), 1); KT_CHECK(ch);        // first registration is a change
    KT_CHECK_EQ(s.slot(0, vit, 38, 38, &r, &ch), 1); KT_CHECK(!ch);       // per-frame repeat is not
    KT_CHECK_EQ(s.slot(0, vit, 40, 38, &r, &ch), 1); KT_CHECK(ch);
}

int main() {
    tPlacement(); tUnregistered(); tBounds(); tChanged();
    return kt::Report("hh_strip_test");
}
