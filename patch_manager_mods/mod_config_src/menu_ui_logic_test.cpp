// Host test of mui (MenuUiLogic.h): mod registry (tokens, same-id replace, full, unregister and re-register, registration
// order, opaque values), page stack (every valid and invalid transition, double back, reset events), hover edge tracker
// (flicker, same item, out of range), release-edge click tracker (press/release item pairs, held across ticks), per-page
// item table (full, visibility, hit test), page table (full), and the truncate guard.
#include "MenuUiLogic.h"
#include "check.h"

using namespace mui;

static const char* kIds[9] = {"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8"};

// True when event i of r has this kind, state and entry.
static bool evIs(const PageEvents& r, int i, EventKind k, State s, int entry) {
    if (i < 0 || i >= r.n) return false;
    const PageEvent& e = r.ev[i];
    return e.kind == k && e.state == s && e.entry == entry;
}

static bool hoverIs(const HoverEvents& r, int i, EventKind k, int item) {
    if (i < 0 || i >= r.n) return false;
    return r.ev[i].kind == k && r.ev[i].item == item;
}

// ---------------------------------------------------------------- registry
static void test_registry_basic() {
    Registry reg;
    KT_CHECK_EQ(reg.count(), 0);
    int a = reg.registerEntry("alpha");
    int b = reg.registerEntry("beta");
    KT_CHECK(a > 0 && b > 0);
    KT_CHECK_EQ(a, 1);
    KT_CHECK_EQ(b, 2);
    KT_CHECK_EQ(reg.count(), 2);
    KT_CHECK_EQ(reg.at(0), 1);
    KT_CHECK_EQ(reg.at(1), 2);
    KT_CHECK_EQ(reg.at(2), 0);      // past the live count
    KT_CHECK_EQ(reg.at(-1), 0);
}

static void test_registry_null_and_empty_ids() {
    Registry reg;
    KT_CHECK_EQ(reg.registerEntry(nullptr), 0);
    KT_CHECK_EQ(reg.registerEntry(""), 0);
    KT_CHECK_EQ(reg.count(), 0);
}

static void test_registry_full() {
    Registry reg;
    for (int i = 0; i < kMaxEntries; ++i) KT_CHECK_EQ(reg.registerEntry(kIds[i]), i + 1);
    KT_CHECK_EQ(reg.count(), kMaxEntries);
    KT_CHECK_EQ(reg.registerEntry(kIds[8]), 0);      // ninth distinct id: rejected
    KT_CHECK_EQ(reg.count(), kMaxEntries);
    KT_CHECK_EQ(reg.at(kMaxEntries - 1), kMaxEntries);
    KT_CHECK_EQ(reg.at(kMaxEntries), 0);
}

static void test_registry_replace_same_id() {
    Registry reg;
    int a = reg.registerEntry("alpha");
    KT_CHECK(reg.setOpaque(a, 42));
    char again[] = "alpha";                              // same text, different buffer
    KT_CHECK_EQ(reg.registerEntry(again), a);            // same token
    KT_CHECK_EQ(reg.count(), 1);
    KT_CHECK(reg.slots[a - 1].id == again);              // id pointer replaced in place
    int v = 0;
    KT_CHECK(reg.opaqueOf(a, &v));
    KT_CHECK_EQ(v, 42);                                  // replace keeps the opaque value

    // Replacing an id while the registry is full still succeeds: it is not a new entry.
    Registry full;
    for (int i = 0; i < kMaxEntries; ++i) full.registerEntry(kIds[i]);
    KT_CHECK_EQ(full.registerEntry(kIds[5]), 6);
    KT_CHECK_EQ(full.count(), kMaxEntries);
}

static void test_registry_unregister_reregister() {
    Registry reg;
    int a = reg.registerEntry("a");
    int b = reg.registerEntry("b");
    int c = reg.registerEntry("c");
    KT_CHECK(reg.unregister(b));
    KT_CHECK(!reg.unregister(b));                        // already gone
    KT_CHECK(!reg.unregister(0));
    KT_CHECK(!reg.unregister(kMaxEntries + 1));
    KT_CHECK(!reg.unregister(-1));
    KT_CHECK_EQ(reg.count(), 2);
    KT_CHECK_EQ(reg.at(0), a);                           // removal keeps the order of the rest
    KT_CHECK_EQ(reg.at(1), c);
    KT_CHECK_EQ(reg.at(2), 0);
    int v = 0;
    KT_CHECK(!reg.opaqueOf(b, &v));                      // removed token is not live
    KT_CHECK(!reg.setOpaque(b, 5));

    int b2 = reg.registerEntry("b");                     // lowest free slot again, so the same token
    KT_CHECK_EQ(b2, b);
    KT_CHECK_EQ(reg.count(), 3);
    KT_CHECK_EQ(reg.at(0), a);
    KT_CHECK_EQ(reg.at(1), c);
    KT_CHECK_EQ(reg.at(2), b2);                          // but a re-registered id goes to the end of the order

    KT_CHECK(reg.unregister(a));
    int d = reg.registerEntry("d");
    KT_CHECK_EQ(d, a);                                   // token reuse of a freed slot
    KT_CHECK_EQ(reg.at(0), c);
    KT_CHECK_EQ(reg.at(1), b2);
    KT_CHECK_EQ(reg.at(2), d);
}

static void test_registry_opaque() {
    Registry reg;
    int a = reg.registerEntry("a");
    int b = reg.registerEntry("b");
    int v = -7;
    KT_CHECK(reg.opaqueOf(b, &v));
    KT_CHECK_EQ(v, 1);                                   // default opaque is the slot index
    KT_CHECK(reg.setOpaque(b, 99));
    KT_CHECK(reg.opaqueOf(b, &v));
    KT_CHECK_EQ(v, 99);
    KT_CHECK(reg.opaqueOf(a, &v));
    KT_CHECK_EQ(v, 0);                                   // other entries untouched
    KT_CHECK(!reg.setOpaque(0, 5));
    KT_CHECK(!reg.setOpaque(3, 5));                      // token 3 never registered
    KT_CHECK(!reg.opaqueOf(3, &v));
}

// ---------------------------------------------------------------- page stack
static void test_stack_valid_transitions() {
    PageStack ps;
    KT_CHECK(ps.state == MAIN && ps.entry == -1);

    PageEvents r = ps.push(LIST, -1);
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(evIs(r, 0, LEAVE, MAIN, -1));
    KT_CHECK(evIs(r, 1, ENTER, LIST, -1));
    KT_CHECK(ps.state == LIST && ps.entry == -1);

    r = ps.push(ENTRY, 3);
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(evIs(r, 0, LEAVE, LIST, -1));
    KT_CHECK(evIs(r, 1, ENTER, ENTRY, 3));
    KT_CHECK(ps.state == ENTRY && ps.entry == 3);

    r = ps.back();
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(evIs(r, 0, LEAVE, ENTRY, 3));
    KT_CHECK(evIs(r, 1, ENTER, LIST, -1));
    KT_CHECK(ps.state == LIST && ps.entry == -1);

    r = ps.back();
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(evIs(r, 0, LEAVE, LIST, -1));
    KT_CHECK(evIs(r, 1, ENTER, MAIN, -1));
    KT_CHECK(ps.state == MAIN && ps.entry == -1);
}

static void test_stack_entry_index_bounds() {
    PageStack ps;
    ps.push(LIST, -1);
    PageEvents r = ps.push(ENTRY, 0);                    // lowest valid index
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(ps.state == ENTRY && ps.entry == 0);
    ps.back();
    r = ps.push(ENTRY, kMaxEntries - 1);                 // highest valid index
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(ps.state == ENTRY && ps.entry == kMaxEntries - 1);
}

static void test_stack_invalid_transitions() {
    PageStack ps;                                        // MAIN
    PageEvents r = ps.push(ENTRY, 0);                    // MAIN -> ENTRY skips LIST
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == MAIN && ps.entry == -1);
    r = ps.push(MAIN, -1);
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == MAIN);
    r = ps.back();                                       // back from MAIN is a no-op
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == MAIN);

    ps.push(LIST, -1);
    r = ps.push(LIST, -1);                               // LIST -> LIST
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == LIST);
    r = ps.push(ENTRY, kMaxEntries);                     // index past the end
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == LIST && ps.entry == -1);
    r = ps.push(ENTRY, -1);                              // negative index
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == LIST);
    r = ps.push(MAIN, -1);                               // LIST -> MAIN is back(), not push
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == LIST);

    ps.push(ENTRY, 5);
    r = ps.push(ENTRY, 2);                               // ENTRY -> ENTRY
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == ENTRY && ps.entry == 5);
    r = ps.push(LIST, -1);                               // ENTRY -> LIST is back()
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == ENTRY && ps.entry == 5);
    r = ps.push(MAIN, -1);
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == ENTRY && ps.entry == 5);
}

static void test_stack_double_back() {
    PageStack ps;
    ps.push(LIST, -1);
    ps.push(ENTRY, 1);
    PageEvents r = ps.back();                            // ENTRY -> LIST
    KT_CHECK_EQ(r.n, 2);
    r = ps.back();                                       // LIST -> MAIN
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(ps.state == MAIN);
    r = ps.back();                                       // MAIN: nothing, no underflow
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK(ps.state == MAIN && ps.entry == -1);
}

static void test_stack_reset() {
    PageStack ps;
    PageEvents r = ps.reset();                           // from MAIN: nothing
    KT_CHECK_EQ(r.n, 0);
    r = ps.reset();                                      // still nothing
    KT_CHECK_EQ(r.n, 0);

    ps.push(LIST, -1);
    r = ps.reset();                                      // from LIST: LEAVE(LIST), ENTER(MAIN)
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(evIs(r, 0, LEAVE, LIST, -1));
    KT_CHECK(evIs(r, 1, ENTER, MAIN, -1));
    KT_CHECK(ps.state == MAIN && ps.entry == -1);
    r = ps.reset();
    KT_CHECK_EQ(r.n, 0);

    ps.push(LIST, -1);
    ps.push(ENTRY, 5);
    r = ps.reset();                                      // from ENTRY(5): LEAVE(ENTRY,5) exactly once, then ENTER(MAIN)
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(evIs(r, 0, LEAVE, ENTRY, 5));
    KT_CHECK(evIs(r, 1, ENTER, MAIN, -1));               // no LEAVE(LIST): the LIST page is skipped
    KT_CHECK(ps.state == MAIN && ps.entry == -1);
    r = ps.reset();
    KT_CHECK_EQ(r.n, 0);
}

// ---------------------------------------------------------------- hover edge
static void test_hover_edges() {
    HoverTracker h;
    HoverEvents r = h.update(-1);                        // none to none
    KT_CHECK_EQ(r.n, 0);
    r = h.update(2);                                     // none -> A
    KT_CHECK_EQ(r.n, 1);
    KT_CHECK(hoverIs(r, 0, ENTER, 2));
    r = h.update(2);                                     // A -> A
    KT_CHECK_EQ(r.n, 0);
    r = h.update(1);                                     // A -> B
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(hoverIs(r, 0, LEAVE, 2));
    KT_CHECK(hoverIs(r, 1, ENTER, 1));
    r = h.update(2);                                     // B -> A (flicker back)
    KT_CHECK_EQ(r.n, 2);
    KT_CHECK(hoverIs(r, 0, LEAVE, 1));
    KT_CHECK(hoverIs(r, 1, ENTER, 2));
    r = h.update(-1);                                    // A -> none
    KT_CHECK_EQ(r.n, 1);
    KT_CHECK(hoverIs(r, 0, LEAVE, 2));
    r = h.update(-1);                                    // none -> none
    KT_CHECK_EQ(r.n, 0);
    r = h.update(kMaxItems);                             // out of range counts as none
    KT_CHECK_EQ(r.n, 0);
    r = h.update(-5);
    KT_CHECK_EQ(r.n, 0);
    KT_CHECK_EQ(h.cur, -1);
}

// ---------------------------------------------------------------- click edge
static void test_click_press_release_same_item() {
    ClickTracker c;
    KT_CHECK_EQ(c.tick(true, 4), -1);                    // press on 4: no click yet
    KT_CHECK_EQ(c.tick(false, 4), 4);                    // release on 4: click
    KT_CHECK_EQ(c.tick(false, 4), -1);                   // no press left, no second click
}

static void test_click_press_a_release_b() {
    ClickTracker c;
    KT_CHECK_EQ(c.tick(true, 3), -1);
    KT_CHECK_EQ(c.tick(false, 5), -1);
}

static void test_click_press_none_release_a() {
    ClickTracker c;
    KT_CHECK_EQ(c.tick(true, -1), -1);
    KT_CHECK_EQ(c.tick(false, 3), -1);                   // press outside any item, release over one: no click
}

static void test_click_press_a_release_none() {
    ClickTracker c;
    KT_CHECK_EQ(c.tick(true, 3), -1);
    KT_CHECK_EQ(c.tick(false, -1), -1);
}

static void test_click_held_across_ticks() {
    ClickTracker c;
    KT_CHECK_EQ(c.tick(true, 3), -1);                    // press on A
    KT_CHECK_EQ(c.tick(true, 5), -1);                    // held, cursor moved onto B
    KT_CHECK_EQ(c.tick(true, 3), -1);                    // back on A, still held
    KT_CHECK_EQ(c.tick(false, 3), 3);                    // release on A: click on A

    ClickTracker d;
    KT_CHECK_EQ(d.tick(true, 3), -1);
    KT_CHECK_EQ(d.tick(true, 5), -1);
    KT_CHECK_EQ(d.tick(false, 5), -1);                   // press A, release on B: no click

    ClickTracker e;
    KT_CHECK_EQ(e.tick(true, 3), -1);
    KT_CHECK_EQ(e.tick(true, -1), -1);                   // dragged off every item
    KT_CHECK_EQ(e.tick(false, 3), 3);                    // release back on the press item: click
}

static void test_click_press_outside_drag_in() {
    ClickTracker c;
    KT_CHECK_EQ(c.tick(true, -1), -1);
    KT_CHECK_EQ(c.tick(true, 3), -1);                    // pressed outside, dragged onto an item while held
    KT_CHECK_EQ(c.tick(false, 3), -1);                   // release over it: no click
    KT_CHECK_EQ(c.tick(true, 5), -1);                    // a fresh press works afterwards
    KT_CHECK_EQ(c.tick(false, 5), 5);
}

// ---------------------------------------------------------------- page table and items
static void test_page_table_full() {
    PageTable pt;
    for (int i = 0; i < kMaxPages; ++i) KT_CHECK_EQ(pt.addPage(), i);
    KT_CHECK_EQ(pt.addPage(), -1);                       // eleventh page
    KT_CHECK_EQ(pt.pageCount(), kMaxPages);
    KT_CHECK_EQ(kMaxPages, 10);
    KT_CHECK_EQ(kMaxPages, 2 + kMaxEntries);             // MAIN, LIST, one ENTRY page per registry slot
}

static void test_item_table_full_and_bad_pages() {
    PageTable pt;
    int p = pt.addPage();
    Rect r = {10, 10, 20, 10};
    for (int i = 0; i < kMaxItems; ++i) KT_CHECK_EQ(pt.addItem(p, BUTTON, nullptr, i, r), i);
    KT_CHECK_EQ(pt.addItem(p, LABEL, nullptr, 0, r), -1);   // twenty-fifth item
    KT_CHECK_EQ(pt.itemCount(p), kMaxItems);
    KT_CHECK_EQ(pt.addItem(9, BUTTON, nullptr, 0, r), -1);  // page not added
    KT_CHECK_EQ(pt.addItem(-1, BUTTON, nullptr, 0, r), -1);
    KT_CHECK_EQ(pt.itemCount(9), 0);
}

static void test_item_fields_and_visibility() {
    static int marker = 0;
    PageTable pt;
    int p = pt.addPage();
    Rect r = {78, 134, 318, 32};
    int b = pt.addItem(p, BUTTON, &marker, 3, r);
    KT_CHECK_EQ(b, 0);
    const Item* it = pt.item(p, b);
    KT_CHECK(it != nullptr);
    if (it) {
        KT_CHECK(it->kind == BUTTON);
        KT_CHECK(it->ctl == &marker);
        KT_CHECK_EQ(it->flags, 3);
        KT_CHECK(it->rect.x == 78 && it->rect.y == 134 && it->rect.w == 318 && it->rect.h == 32);
        KT_CHECK(it->visibleInPage);                     // new items are visible
    }
    KT_CHECK(pt.item(p, 1) == nullptr);                  // id not added
    KT_CHECK(pt.item(p, -1) == nullptr);
    KT_CHECK(pt.item(5, 0) == nullptr);                  // page not added

    KT_CHECK(pt.setVisible(p, b, false));
    KT_CHECK(pt.item(p, b) != nullptr && !pt.item(p, b)->visibleInPage);
    KT_CHECK(!pt.setVisible(p, 7, true));
    KT_CHECK(!pt.setVisible(3, 0, true));
}

static void test_hit_test() {
    PageTable pt;
    int p = pt.addPage();
    Rect A = {10, 10, 50, 20};    // x 10..59, y 10..29
    Rect B = {100, 10, 50, 20};   // x 100..149, y 10..29
    Rect C = {20, 15, 10, 10};    // x 20..29, y 15..24, overlaps A
    pt.addItem(p, BUTTON, nullptr, 0, A);   // id 0
    pt.addItem(p, BUTTON, nullptr, 0, B);   // id 1
    pt.addItem(p, LABEL, nullptr, 0, C);    // id 2
    KT_CHECK_EQ(pt.hitTest(p, 15, 12), 0);
    KT_CHECK_EQ(pt.hitTest(p, 120, 20), 1);
    KT_CHECK_EQ(pt.hitTest(p, 59, 20), 0);  // last column inside
    KT_CHECK_EQ(pt.hitTest(p, 60, 20), -1); // x + w is exclusive
    KT_CHECK_EQ(pt.hitTest(p, 30, 29), 0);  // last row inside
    KT_CHECK_EQ(pt.hitTest(p, 30, 30), -1); // y + h is exclusive
    KT_CHECK_EQ(pt.hitTest(p, 9, 20), -1);
    KT_CHECK_EQ(pt.hitTest(p, 25, 20), 0);  // overlap of A and C: lowest id wins
    KT_CHECK(pt.setVisible(p, 0, false));
    KT_CHECK_EQ(pt.hitTest(p, 25, 20), 2);  // hidden A skipped, C is hit
    KT_CHECK_EQ(pt.hitTest(p, 30, 20), -1); // only hidden A covers this point
    KT_CHECK_EQ(pt.hitTest(7, 25, 20), -1); // page not added
    KT_CHECK_EQ(pt.hitTest(-1, 25, 20), -1);
}

// ---------------------------------------------------------------- truncate guard
static bool oddIsOurs(uintptr_t v, void* ctx) {
    ++*static_cast<int*>(ctx);
    return (v & 1u) != 0;
}

static void test_can_truncate() {
    int calls = 0;
    const uintptr_t allOurs[4] = {2, 3, 5, 7};
    KT_CHECK(canTruncate(allOurs, 4, 1, oddIsOurs, &calls));   // [1,4) = 3,5,7 all ours
    KT_CHECK_EQ(calls, 3);

    const uintptr_t mixed[4] = {2, 3, 4, 5};
    KT_CHECK(!canTruncate(mixed, 4, 1, oddIsOurs, &calls));    // 4 in [1,4) is not ours

    const uintptr_t notOursBelow[2] = {4, 3};
    KT_CHECK(canTruncate(notOursBelow, 2, 1, oddIsOurs, &calls));   // only [vanilla, count) is checked

    KT_CHECK(canTruncate(allOurs, 4, 4, oddIsOurs, &calls));   // count == vanilla: nothing to remove
    KT_CHECK(!canTruncate(allOurs, 3, 4, oddIsOurs, &calls));  // count < vanilla
    KT_CHECK(!canTruncate(allOurs, 4, -1, oddIsOurs, &calls)); // negative vanilla
    KT_CHECK(!canTruncate(nullptr, 4, 1, oddIsOurs, &calls));  // null list with entries to check
    KT_CHECK(!canTruncate(allOurs, 4, 1, nullptr, &calls));    // null predicate
}

int main() {
    test_registry_basic();
    test_registry_null_and_empty_ids();
    test_registry_full();
    test_registry_replace_same_id();
    test_registry_unregister_reregister();
    test_registry_opaque();
    test_stack_valid_transitions();
    test_stack_entry_index_bounds();
    test_stack_invalid_transitions();
    test_stack_double_back();
    test_stack_reset();
    test_hover_edges();
    test_click_press_release_same_item();
    test_click_press_a_release_b();
    test_click_press_none_release_a();
    test_click_press_a_release_none();
    test_click_held_across_ticks();
    test_click_press_outside_drag_in();
    test_page_table_full();
    test_item_table_full_and_bad_pages();
    test_item_fields_and_visibility();
    test_hit_test();
    test_can_truncate();
    return kt::Report("menu_ui_logic_test");
}
