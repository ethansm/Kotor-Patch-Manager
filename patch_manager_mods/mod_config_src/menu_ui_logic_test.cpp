// Host test of mui (MenuUiLogic.h): mod registry (tokens, same-id replace, full, unregister and re-register, registration
// order, opaque values), page stack (every valid and invalid transition, double back, reset events), hover edge tracker
// (flicker, same item, out of range), release-edge click tracker (press/release item pairs, held across ticks), per-page
// item table (full, visibility, hit test), page table (full), and the truncate guard.
#include "MenuUiLogic.h"
#include <limits.h>
#include "check.h"
#include <stdio.h>

using namespace mui;

// One more id than the registry holds ("m0" .. "m<kMaxEntries>"), so the "full" tests have a rejected id to try.
static const char* kIdsInit(int i) {
    static char buf[kMaxEntries + 1][16];
    snprintf(buf[i], sizeof buf[i], "m%d", i);
    return buf[i];
}
struct IdTable {
    const char* v[kMaxEntries + 1];
    IdTable() { for (int i = 0; i <= kMaxEntries; ++i) v[i] = kIdsInit(i); }
    const char* operator[](int i) const { return v[i]; }
};
static const IdTable kIds;

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
    KT_CHECK_EQ(reg.registerEntry(kIds[kMaxEntries]), 0);      // one more distinct id than slots: rejected
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
    KT_CHECK_EQ(pt.addPage(), -1);                       // one page past the table
    KT_CHECK_EQ(pt.pageCount(), kMaxPages);
    KT_CHECK_EQ(kMaxPages, 2 + kMaxEntries + kMaxModals);   // MAIN, LIST, one ENTRY page per registry slot, one page per modal
    KT_CHECK_EQ(kMaxModals, 8); KT_CHECK_EQ(kMaxSlots, 32); KT_CHECK_EQ(kMaxPages, 34);
}

static void test_item_table_full_and_bad_pages() {
    PageTable pt;
    int p = pt.addPage();
    Rect r = {10, 10, 20, 10};
    for (int i = 0; i < kMaxItems; ++i) KT_CHECK_EQ(pt.addItem(p, BUTTON, nullptr, i, r), i);
    KT_CHECK_EQ(kMaxItems, 80);                              // HW-UX3 L0a: cap raised from 40 to 80
    KT_CHECK_EQ(pt.addItem(p, LABEL, nullptr, 0, r), -1);   // 81st item refused
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

// ---------------------------------------------------------------- description split
static bool rectIs(const Rect& r, int x, int y, int w, int h) { return r.x == x && r.y == y && r.w == w && r.h == h; }

static void test_split_clamp() {
    KT_CHECK_EQ(clampSplit(0), 0);
    KT_CHECK_EQ(clampSplit(-3), 0);
    KT_CHECK_EQ(clampSplit(1), kSplitMin);
    KT_CHECK_EQ(clampSplit(599), kSplitMin);
    KT_CHECK_EQ(clampSplit(600), 600);
    KT_CHECK_EQ(clampSplit(725), 725);
    KT_CHECK_EQ(clampSplit(850), 850);
    KT_CHECK_EQ(clampSplit(851), kSplitMax);
    KT_CHECK_EQ(clampSplit(5000), kSplitMax);
}

static void test_split_vanilla_rect() {
    const Rect vanilla = {294, 134, 232, 336};
    DescSplit off = splitDesc(vanilla, 0);                        // 0 -> unchanged: both parts are the whole rect
    KT_CHECK(rectIs(off.preview, 294, 134, 232, 336) && rectIs(off.desc, 294, 134, 232, 336));
    DescSplit a = splitDesc(vanilla, 700);                        // gap 6, desc 101 (30 %), preview 229
    KT_CHECK(rectIs(a.preview, 294, 134, 232, 229));
    KT_CHECK(rectIs(a.desc, 294, 369, 232, 101));
    DescSplit lo = splitDesc(vanilla, 600);                       // desc 134 (40 %), preview 196
    KT_CHECK(rectIs(lo.preview, 294, 134, 232, 196));
    KT_CHECK(rectIs(lo.desc, 294, 336, 232, 134));
    DescSplit hi = splitDesc(vanilla, 850);                       // desc 50 (15 %), preview 280
    KT_CHECK(rectIs(hi.preview, 294, 134, 232, 280));
    KT_CHECK(rectIs(hi.desc, 294, 420, 232, 50));
}

static void test_split_out_of_range_clamps() {
    const Rect vanilla = {294, 134, 232, 336};
    DescSplit one = splitDesc(vanilla, 1), lo = splitDesc(vanilla, 600);
    KT_CHECK(rectIs(one.preview, lo.preview.x, lo.preview.y, lo.preview.w, lo.preview.h) && rectIs(one.desc, lo.desc.x, lo.desc.y, lo.desc.w, lo.desc.h));
    DescSplit big = splitDesc(vanilla, 5000), hi = splitDesc(vanilla, 850);
    KT_CHECK(rectIs(big.preview, hi.preview.x, hi.preview.y, hi.preview.w, hi.preview.h) && rectIs(big.desc, hi.desc.x, hi.desc.y, hi.desc.w, hi.desc.h));
    DescSplit neg = splitDesc(vanilla, -3);                       // negative = off
    KT_CHECK(rectIs(neg.preview, 294, 134, 232, 336) && rectIs(neg.desc, 294, 134, 232, 336));
}

static void test_split_zero_area() {
    const Rect zw = {10, 20, 0, 100}, zh = {10, 20, 100, 0}, neg = {5, 6, -5, -5}, none = {0, 0, 0, 0};
    DescSplit a = splitDesc(zw, 700), b = splitDesc(zh, 700), c = splitDesc(neg, 700), d = splitDesc(none, 850);
    KT_CHECK(rectIs(a.preview, 10, 20, 0, 100) && rectIs(a.desc, 10, 20, 0, 100));
    KT_CHECK(rectIs(b.preview, 10, 20, 100, 0) && rectIs(b.desc, 10, 20, 100, 0));
    KT_CHECK(rectIs(c.preview, 5, 6, -5, -5) && rectIs(c.desc, 5, 6, -5, -5));
    KT_CHECK(rectIs(d.preview, 0, 0, 0, 0) && rectIs(d.desc, 0, 0, 0, 0));
}

static void test_split_odd_heights() {
    DescSplit odd = splitDesc(Rect{0, 0, 232, 335}, 700);         // gap 6, desc 101, preview 228
    KT_CHECK(rectIs(odd.preview, 0, 0, 232, 228) && rectIs(odd.desc, 0, 234, 232, 101));
    DescSplit dbl = splitDesc(Rect{7, 3, 400, 672}, 700);         // 2x vanilla: gap 12, desc 202, preview 458
    KT_CHECK(rectIs(dbl.preview, 7, 3, 400, 458) && rectIs(dbl.desc, 7, 473, 400, 202));
    DescSplit tiny = splitDesc(Rect{0, 0, 10, 1}, 850);           // 1 px: gap 0, desc 0, preview 1
    KT_CHECK(rectIs(tiny.preview, 0, 0, 10, 1) && rectIs(tiny.desc, 0, 1, 10, 0));
    // Invariant over every height and a spread of permilles: preview + gap + desc == orig.h, nothing negative, the bottom edge stays,
    // x / w are kept, and the description keeps its share (+- rounding) once the rect is tall enough.
    const int pms[] = {1, 600, 601, 700, 725, 849, 850, 5000};
    for (int h = 1; h <= 700; ++h) {
        for (int pi = 0; pi < 8; ++pi) {
            const Rect o = {-12, 40, 181, h};
            const DescSplit s = splitDesc(o, pms[pi]);
            const int gap = s.desc.y - (s.preview.y + s.preview.h);
            KT_CHECK(s.preview.h >= 0 && s.desc.h >= 0 && gap >= 0);
            KT_CHECK_EQ(s.preview.h + gap + s.desc.h, h);
            KT_CHECK_EQ(s.desc.y + s.desc.h, o.y + o.h);
            KT_CHECK(s.preview.y == o.y && s.preview.x == o.x && s.desc.x == o.x && s.preview.w == o.w && s.desc.w == o.w);
        }
    }
    for (int h = 100; h <= 700; h += 37) {                        // 700: desc share is 30 % +- 1 px, gap is 6/336 +- 1 px
        const DescSplit s = splitDesc(Rect{0, 0, 50, h}, 700);
        const int want = (300 * h) / 1000;
        KT_CHECK(s.desc.h >= want && s.desc.h <= want + 1);
    }
}

// ---------------------------------------------------------------- option rows
static RowCfg stepper(int lo, int hi, int st, int v, int flags = 0) { return RowCfg{RK_STEPPER, lo, hi, st, v, flags}; }
static RowCfg toggle(int v, int flags = 0) { return RowCfg{RK_TOGGLE, 0, 1, 1, v, flags}; }

static void test_row_clamp_step() {
    const RowCfg hp = stepper(5, 95, 5, 35);                      // Heal below N%
    KT_CHECK_EQ(rowClamp(hp, 0), 5);
    KT_CHECK_EQ(rowClamp(hp, 100), 95);
    KT_CHECK_EQ(rowClamp(hp, 40), 40);
    KT_CHECK_EQ(rowStep(hp, 35, 1), 40);
    KT_CHECK_EQ(rowStep(hp, 35, -1), 30);
    KT_CHECK_EQ(rowStep(hp, 95, 1), 95);                          // clamp at the top
    KT_CHECK_EQ(rowStep(hp, 5, -1), 5);                           // clamp at the bottom
    KT_CHECK_EQ(rowStep(hp, 90, 1), 95);
    KT_CHECK_EQ(rowStep(hp, 10, -1), 5);
    KT_CHECK_EQ(rowStep(hp, 37, 1), 42);                          // off-grid value moves from where it is (the handler snaps)
    KT_CHECK_EQ(rowStep(hp, 37, 0), 37);                          // dir 0 = clamped value only
    KT_CHECK_EQ(rowStep(hp, 200, 0), 95);
    const RowCfg keep = stepper(0, 20, 1, 2);                     // Keep N
    KT_CHECK_EQ(rowStep(keep, 20, 1), 20);
    KT_CHECK_EQ(rowStep(keep, 0, -1), 0);
    KT_CHECK_EQ(rowStep(keep, 0, 1), 1);
    const RowCfg w = stepper(0, 20, 1, 20, RF_WRAP);
    KT_CHECK_EQ(rowStep(w, 20, 1), 0);                            // wrap both ways
    KT_CHECK_EQ(rowStep(w, 0, -1), 20);
    KT_CHECK_EQ(rowStep(w, 10, 1), 11);
    const RowCfg odd = stepper(0, 20, 3, 18, RF_WRAP);            // step does not divide the range
    KT_CHECK_EQ(rowStep(odd, 18, 1), 0);
    KT_CHECK_EQ(rowStep(odd, 0, -1), 20);
    KT_CHECK_EQ(rowStep(stepper(0, 20, 3, 18), 18, 1), 20);       // clamp variant lands on max
    KT_CHECK_EQ(rowStep(stepper(0, 5, 0, 2), 2, 1), 3);           // step < 1 reads as 1
    KT_CHECK_EQ(rowStep(stepper(9, 3, 1, 5), 5, 1), 6);           // swapped range reads as min..max
    KT_CHECK_EQ(rowClamp(stepper(9, 3, 1, 5), 100), 9);
    KT_CHECK_EQ(rowStep(stepper(4, 4, 1, 4, RF_WRAP), 4, 1), 4);  // one-value range
    // Ranges wider than INT_MAX: the distance math must not overflow (review MCF1/MCF2).
    const RowCfg wide = stepper(INT_MIN, INT_MAX, 10, 0);
    KT_CHECK_EQ(rowStep(wide, INT_MIN, 1), INT_MIN + 10);
    KT_CHECK_EQ(rowStep(wide, INT_MAX, -1), INT_MAX - 10);
    KT_CHECK_EQ(rowStep(wide, INT_MAX - 3, 1), INT_MAX);          // lands on max, no wrap past INT_MAX
    KT_CHECK_EQ(rowStep(wide, INT_MIN + 3, -1), INT_MIN);
    KT_CHECK_EQ(rowStep(stepper(INT_MIN, INT_MAX, 10, 0, RF_WRAP), INT_MAX - 3, 1), INT_MIN);
    KT_CHECK_EQ(rowStep(stepper(INT_MIN, INT_MAX, 10, 0, RF_WRAP), INT_MIN + 3, -1), INT_MAX);
    KT_CHECK_EQ(rowStep(stepper(0, 10, INT_MAX, 5), 5, 1), 10);   // a huge step clamps at the end
    KT_CHECK_EQ(rowStep(stepper(0, 10, INT_MAX, 5), 5, -1), 0);
}

static void test_row_toggle() {
    const RowCfg t = toggle(0);
    KT_CHECK_EQ(rowStep(t, 0, 0), 1);                             // a toggle flips whatever dir says
    KT_CHECK_EQ(rowStep(t, 1, 0), 0);
    KT_CHECK_EQ(rowStep(t, 0, 1), 1);
    KT_CHECK_EQ(rowStep(t, 1, -1), 0);
    KT_CHECK_EQ(rowStep(t, 7, 1), 0);                             // non-zero counts as on
    KT_CHECK_EQ(rowClamp(t, 5), 1);
    KT_CHECK_EQ(rowClamp(t, -3), 1);
    KT_CHECK_EQ(rowClamp(t, 0), 0);
}

static void test_row_handler_result() {
    const RowCfg hp = stepper(5, 95, 5, 35);
    KT_CHECK_EQ(rowApplyHandlerResult(hp, 40, 40, true), 40);     // accepted
    KT_CHECK_EQ(rowApplyHandlerResult(hp, 42, 40, true), 40);     // snapped: the handler's FINAL value wins
    KT_CHECK_EQ(rowApplyHandlerResult(hp, 40, 35, true), 35);     // rejected: handler returns the old value
    KT_CHECK_EQ(rowApplyHandlerResult(hp, 40, 500, true), 95);    // out of range is clamped
    KT_CHECK_EQ(rowApplyHandlerResult(hp, 40, -4, true), 5);
    KT_CHECK_EQ(rowApplyHandlerResult(hp, 40, 99, false), 40);    // no handler: the request is stored
    KT_CHECK_EQ(rowApplyHandlerResult(hp, 400, 99, false), 95);
    KT_CHECK_EQ(rowApplyHandlerResult(toggle(0), 1, 1, true), 1);
    KT_CHECK_EQ(rowApplyHandlerResult(toggle(0), 1, 3, true), 1);
    KT_CHECK_EQ(rowApplyHandlerResult(stepper(5, 95, 5, 35, RF_DISABLED), 40, 40, true), 35);   // disabled never changes
    KT_CHECK_EQ(rowApplyHandlerResult(stepper(5, 95, 5, 35, RF_DISABLED), 40, 40, false), 35);
}

static void test_row_format() {
    char b[64];
    RowCfg hp = stepper(5, 95, 5, 35);
    KT_CHECK(rowFormat(b, sizeof b, "Heal", hp, "Heal below %d%%", nullptr, 0, nullptr));
    KT_CHECK(strcmp(b, "Heal below 35%") == 0);
    RowCfg keep = stepper(0, 20, 1, 0);
    KT_CHECK(rowFormat(b, sizeof b, "Keep", keep, nullptr, nullptr, 0, "Auto"));
    KT_CHECK(strcmp(b, "Keep: Auto") == 0);                       // zero text
    keep.value = 4;
    KT_CHECK(rowFormat(b, sizeof b, "Keep", keep, nullptr, nullptr, 0, "Auto"));
    KT_CHECK(strcmp(b, "Keep: 4") == 0);                          // default %d
    keep.flags = RF_RESTART;
    KT_CHECK(rowFormat(b, sizeof b, "Keep", keep, nullptr, nullptr, 0, nullptr));
    KT_CHECK(strcmp(b, "Keep: 4 (restart)") == 0);
    const char* ch[] = {"Off", "Slow", "Fast"};
    RowCfg lst = stepper(0, 2, 1, 1);
    KT_CHECK(rowFormat(b, sizeof b, "Speed", lst, nullptr, ch, 3, nullptr));
    KT_CHECK(strcmp(b, "Speed: Slow") == 0);
    lst.value = 2;
    KT_CHECK(rowFormat(b, sizeof b, "Speed", lst, "ignored %d", ch, 3, nullptr));
    KT_CHECK(strcmp(b, "Speed: Fast") == 0);                      // choices beat fmt
    RowCfg tg = toggle(1);
    KT_CHECK(rowFormat(b, sizeof b, "Auto-heal", tg, nullptr, nullptr, 0, nullptr));
    KT_CHECK(strcmp(b, "Auto-heal: On") == 0);
    tg.value = 0;
    tg.flags = RF_RESTART;
    KT_CHECK(rowFormat(b, sizeof b, "Auto-heal", tg, nullptr, nullptr, 0, nullptr));
    KT_CHECK(strcmp(b, "Auto-heal: Off (restart)") == 0);
    RowCfg neg = stepper(-50, 50, 5, -15);
    KT_CHECK(rowFormat(b, sizeof b, "Bias", neg, nullptr, nullptr, 0, nullptr));
    KT_CHECK(strcmp(b, "Bias: -15") == 0);
    KT_CHECK(rowFormat(b, sizeof b, "", keep, nullptr, nullptr, 0, nullptr) && strcmp(b, "4 (restart)") == 0);   // no label
    KT_CHECK(rowFormat(b, sizeof b, "X", hp, "100%% of %d %s", nullptr, 0, nullptr));
    KT_CHECK(strcmp(b, "100% of 35 %s") == 0);                    // only %d and %% expand
    hp.value = 500;                                               // the shown value is clamped
    KT_CHECK(rowFormat(b, sizeof b, "X", hp, "%d", nullptr, 0, nullptr) && strcmp(b, "95") == 0);
    hp.value = 35;
    KT_CHECK(!rowFormat(b, 8, "Heal", hp, "Heal below %d%%", nullptr, 0, nullptr));   // cut text: false, still terminated
    KT_CHECK(strcmp(b, "Heal be") == 0);
    KT_CHECK(!rowFormat(b, 1, "Heal", hp, nullptr, nullptr, 0, nullptr) && b[0] == 0);
    KT_CHECK(!rowFormat(nullptr, 10, "x", hp, nullptr, nullptr, 0, nullptr));
    KT_CHECK(!rowFormat(b, 0, "x", hp, nullptr, nullptr, 0, nullptr));
    // Edge cases (review MCF1/MCF2): toggle choices, null choice strings, index past nChoices, zeroText on a toggle, INT_MIN.
    const char* yn[] = {"No", "Yes"};
    KT_CHECK(rowFormat(b, sizeof b, "Loot", toggle(1), nullptr, yn, 2, nullptr) && strcmp(b, "Loot: Yes") == 0);
    KT_CHECK(rowFormat(b, sizeof b, "Loot", toggle(0), nullptr, yn, 2, "Auto") && strcmp(b, "Loot: No") == 0);   // choices beat zeroText
    KT_CHECK(rowFormat(b, sizeof b, "Loot", toggle(0), nullptr, nullptr, 0, "Auto") && strcmp(b, "Loot: Off") == 0);   // zeroText: steppers only
    const char* holes[] = {"A", nullptr};
    KT_CHECK(rowFormat(b, sizeof b, "H", stepper(0, 1, 1, 1), nullptr, holes, 2, nullptr) && strcmp(b, "H: ") == 0);   // null choice = empty
    KT_CHECK(rowFormat(b, sizeof b, "H", stepper(0, 9, 1, 7), nullptr, ch, 3, nullptr) && strcmp(b, "H: Fast") == 0);   // index past nChoices = last
    KT_CHECK(rowFormat(b, sizeof b, "", stepper(INT_MIN, 0, 1, INT_MIN), nullptr, nullptr, 0, nullptr) && strcmp(b, "-2147483648") == 0);
    KT_CHECK(rowFormat(b, sizeof b, nullptr, stepper(0, 9, 1, 3), "%", nullptr, 0, nullptr) && strcmp(b, "%") == 0);   // a lone trailing % is literal
    KT_CHECK(rowFormat(b, sizeof b, "Z", stepper(0, 9, 1, 0), "%d left", nullptr, 0, "None") && strcmp(b, "Z: None") == 0);   // zeroText beats fmt at 0
}

static void test_row_geometry() {
    const Rect col = {100, 50, 240, 284};
    for (int k = 0; k < kRowsVisible; ++k) {
        const Rect s = rowSlotRect(col, k);
        KT_CHECK(rectIs(s, 100, 50 + 36 * k, 240, 32));
    }
    KT_CHECK_EQ(rowSlotRect(col, 7).y + rowSlotRect(col, 7).h, col.y + col.h);   // 8 slots fit exactly
    for (int h = 100; h <= 700; h += 13) {                        // 8 slots always fit, never overlap
        const Rect c = {0, 10, 200, h};
        for (int k = 0; k < kRowsVisible; ++k) {
            const Rect s = rowSlotRect(c, k);
            KT_CHECK(s.y >= c.y && s.y + s.h <= c.y + c.h);
            if (k > 0) KT_CHECK(s.y >= rowSlotRect(c, k - 1).y + rowSlotRect(c, k - 1).h);
        }
    }
    const Rect slot = rowSlotRect(col, 2);
    const RowGeom g = rowGeom(slot, RK_STEPPER);
    KT_CHECK_EQ(g.label.x, slot.x);
    KT_CHECK_EQ(g.left.w, 48);                                    // 20 % of 240
    KT_CHECK_EQ(g.right.w, 48);
    KT_CHECK_EQ(g.label.w, 240 - 96 - 12);
    KT_CHECK_EQ(g.left.x, g.label.x + g.label.w + kRowGap);
    KT_CHECK_EQ(g.right.x, g.left.x + g.left.w + kRowGap);
    KT_CHECK_EQ(g.right.x + g.right.w, slot.x + slot.w);          // parts + gaps sum to the slot width
    KT_CHECK(g.label.y == slot.y && g.left.y == slot.y && g.right.y == slot.y && g.label.h == slot.h && g.left.h == slot.h && g.right.h == slot.h);
    const RowGeom t = rowGeom(slot, RK_TOGGLE);
    KT_CHECK(rectIs(t.whole, slot.x, slot.y, slot.w, slot.h) && rectIs(t.label, slot.x, slot.y, slot.w, slot.h));
    KT_CHECK(t.left.w == 0 && t.right.w == 0);
    for (int w = 0; w <= 400; w += 7) {                           // never negative, still sums when the slot is wide enough
        const RowGeom q = rowGeom(Rect{5, 6, w, 30}, RK_STEPPER);
        KT_CHECK(q.label.w >= 0 && q.left.w >= 0 && q.right.w >= 0);
        if (w >= 60) KT_CHECK_EQ(q.right.x + q.right.w, 5 + w);
    }
}

static void test_row_hit_zones() {
    const Rect slot = {100, 80, 240, 32};
    const RowGeom g = rowGeom(slot, RK_STEPPER);
    KT_CHECK_EQ(rowHitZone(g, slot.x, slot.y), Z_LABEL);          // top-left corner
    KT_CHECK_EQ(rowHitZone(g, g.label.x + g.label.w - 1, slot.y + 31), Z_LABEL);
    KT_CHECK_EQ(rowHitZone(g, g.label.x + g.label.w, slot.y + 5), Z_NONE);      // the gap
    KT_CHECK_EQ(rowHitZone(g, g.left.x - 1, slot.y + 5), Z_NONE);
    KT_CHECK_EQ(rowHitZone(g, g.left.x, slot.y), Z_LEFT);
    KT_CHECK_EQ(rowHitZone(g, g.left.x + g.left.w - 1, slot.y + 31), Z_LEFT);
    KT_CHECK_EQ(rowHitZone(g, g.left.x + g.left.w, slot.y + 5), Z_NONE);        // gap between the arrows
    KT_CHECK_EQ(rowHitZone(g, g.right.x, slot.y + 5), Z_RIGHT);
    KT_CHECK_EQ(rowHitZone(g, g.right.x + g.right.w - 1, slot.y + 31), Z_RIGHT);
    KT_CHECK_EQ(rowHitZone(g, g.right.x + g.right.w, slot.y + 5), Z_NONE);      // right edge is exclusive
    KT_CHECK_EQ(rowHitZone(g, slot.x + 10, slot.y - 1), Z_NONE);                // above / below
    KT_CHECK_EQ(rowHitZone(g, slot.x + 10, slot.y + 32), Z_NONE);
    KT_CHECK_EQ(rowHitZone(g, slot.x - 1, slot.y + 5), Z_NONE);
    const RowGeom t = rowGeom(slot, RK_TOGGLE);
    KT_CHECK_EQ(rowHitZone(t, slot.x, slot.y), Z_WHOLE);
    KT_CHECK_EQ(rowHitZone(t, slot.x + 239, slot.y + 31), Z_WHOLE);
    KT_CHECK_EQ(rowHitZone(t, slot.x + 240, slot.y + 5), Z_NONE);
    // Zones never overlap: every pixel of the slot reports at most one zone and the three stepper rects are disjoint.
    int nLabel = 0, nL = 0, nR = 0, nNone = 0;
    for (int x = slot.x; x < slot.x + slot.w; ++x) {
        const int z = rowHitZone(g, x, slot.y + 10);
        if (z == Z_LABEL) ++nLabel; else if (z == Z_LEFT) ++nL; else if (z == Z_RIGHT) ++nR; else ++nNone;
    }
    KT_CHECK_EQ(nLabel, g.label.w);
    KT_CHECK_EQ(nL, g.left.w);
    KT_CHECK_EQ(nR, g.right.w);
    KT_CHECK_EQ(nNone, 2 * kRowGap);
}

// ---------------------------------------------------------------- scroll window
static void test_scroll_clamp_wheel() {
    ScrollWindow s;
    s.total = 24;
    KT_CHECK_EQ(s.visible, 8);
    KT_CHECK_EQ(s.maxOffset(), 16);
    KT_CHECK(!s.wheel(1));                                        // already at the top: + is up
    KT_CHECK_EQ(s.offset, 0);
    KT_CHECK(s.wheel(-1));                                        // down one row
    KT_CHECK_EQ(s.offset, 1);
    KT_CHECK(s.wheel(-3));
    KT_CHECK_EQ(s.offset, 4);
    KT_CHECK(s.wheel(2));                                         // up two
    KT_CHECK_EQ(s.offset, 2);
    KT_CHECK(s.wheel(-100));
    KT_CHECK_EQ(s.offset, 16);                                    // clamps at maxOffset
    KT_CHECK(!s.wheel(-1));
    KT_CHECK(s.wheel(100));
    KT_CHECK_EQ(s.offset, 0);
    KT_CHECK(s.setOffset(5) && s.offset == 5);
    KT_CHECK(!s.setOffset(5));
    KT_CHECK(s.setOffset(-9) && s.offset == 0);
    KT_CHECK(s.setOffset(99) && s.offset == 16);
    s.total = 20;                                                 // rows shrink under a large offset
    s.clamp();
    KT_CHECK_EQ(s.offset, 12);
    s.total = 5;                                                  // fewer rows than the window: no scrolling
    s.clamp();
    KT_CHECK(s.offset == 0 && s.maxOffset() == 0 && !s.needBar());
    KT_CHECK(!s.wheel(-1) && !s.setOffset(3));
    s.total = 0;
    KT_CHECK(s.maxOffset() == 0 && !s.needBar());
    ScrollWindow e;                                               // exactly 8 rows and 9 rows
    e.total = 8;
    KT_CHECK(!e.needBar() && e.maxOffset() == 0);
    e.total = 9;
    KT_CHECK(e.needBar() && e.maxOffset() == 1);
}

static void test_scroll_ensure_visible() {
    ScrollWindow s;
    s.total = 24;
    KT_CHECK(!s.ensureVisible(0) && s.offset == 0);               // inside: unchanged
    KT_CHECK(!s.ensureVisible(7) && s.offset == 0);
    KT_CHECK(s.ensureVisible(8) && s.offset == 1);                // one below: scroll just enough
    KT_CHECK(s.ensureVisible(23) && s.offset == 16);
    KT_CHECK(!s.ensureVisible(23));
    KT_CHECK(s.ensureVisible(3) && s.offset == 3);                // above: row 3 becomes the top
    KT_CHECK(!s.ensureVisible(10) && s.offset == 3);
    KT_CHECK(!s.ensureVisible(-1) && !s.ensureVisible(24) && s.offset == 3);   // out of range is ignored
    // After SetRowState hides the selected row the window can end up past the end: clamp then ensure.
    s.total = 10;
    s.clamp();
    KT_CHECK_EQ(s.offset, 2);
    KT_CHECK(s.ensureVisible(9) == false && s.offset == 2);
    KT_CHECK_EQ(s.slotOf(2), 0);                                  // slotOf: displayed slot or -1
    KT_CHECK_EQ(s.slotOf(9), 7);
    KT_CHECK_EQ(s.slotOf(1), -1);
    KT_CHECK_EQ(s.slotOf(10), -1);
    KT_CHECK_EQ(s.slotOf(-1), -1);
}

static void test_scroll_thumb() {
    const Rect track = {400, 100, 14, 250};
    ScrollWindow s;
    s.total = 8;                                                  // no bar needed: thumb is the whole track
    KT_CHECK(rectIs(s.thumb(track), 400, 100, 14, 250));
    s.total = 9;                                                  // 8/9 of 250 = 222
    KT_CHECK_EQ(s.thumb(track).h, 222);
    KT_CHECK_EQ(s.thumb(track).y, 100);
    s.offset = 1;
    KT_CHECK_EQ(s.thumb(track).y + s.thumb(track).h, track.y + track.h);   // bottom at maxOffset
    s.total = 24;                                                 // 8/24 of 250 = 83
    s.offset = 0;
    KT_CHECK_EQ(s.thumb(track).h, 83);
    s.offset = 16;
    KT_CHECK_EQ(s.thumb(track).y + s.thumb(track).h, track.y + track.h);
    s.total = 400;                                                // tiny share -> min 12 px
    s.offset = 0;
    KT_CHECK_EQ(s.thumb(track).h, kThumbMin);
    ScrollWindow sh;                                              // short track shorter than the minimum
    sh.total = 400;
    KT_CHECK_EQ(sh.thumb(Rect{0, 0, 14, 8}).h, 8);
    KT_CHECK_EQ(sh.thumb(Rect{0, 0, 14, 0}).h, 0);
    // px <-> offset round trip: the thumb top for offset o maps back to o whenever the travel has a pixel per row.
    const int totals[] = {9, 24, 40};
    for (int ti = 0; ti < 3; ++ti) {
        ScrollWindow w;
        w.total = totals[ti];
        for (int o = 0; o <= w.maxOffset(); ++o) {
            w.offset = o;
            const Rect th = w.thumb(track);
            KT_CHECK(th.y >= track.y && th.y + th.h <= track.y + track.h);
            KT_CHECK_EQ(w.offsetFromThumbTop(th.y, 0, track), o);
            KT_CHECK_EQ(w.offsetFromThumbTop(th.y + 5, 5, track), o);          // a grab offset cancels out
            if (o > 0) {
                w.offset = o - 1;
                KT_CHECK(w.thumb(track).y <= th.y);                            // monotonic
                w.offset = o;
            }
        }
        w.offset = 3;
        KT_CHECK_EQ(w.offsetFromThumbTop(-500, 0, track), 0);                  // dragged past either end clamps
        KT_CHECK_EQ(w.offsetFromThumbTop(5000, 0, track), w.maxOffset());
    }
    ScrollWindow no;                                              // nothing to scroll
    no.total = 4;
    KT_CHECK_EQ(no.offsetFromThumbTop(200, 0, track), 0);
    ScrollWindow cramped;                                         // travel smaller than the row count: still in range, monotonic
    cramped.total = 40;
    const Rect small = {0, 0, 14, 40};
    int last = 0;
    for (int py = 0; py <= 40; ++py) {
        const int o = cramped.offsetFromThumbTop(py, 0, small);
        KT_CHECK(o >= last && o <= cramped.maxOffset());
        last = o;
    }
    KT_CHECK_EQ(last, cramped.maxOffset());
}

static void test_scroll_list_rebind_maths() {
    // LIST page: 8 buttons over N entries; button i shows entry offset + i (the snapshot index), blank past the end.
    for (int n = 9; n <= kMaxEntries; ++n) {
        ScrollWindow s;
        s.total = n;
        for (int off = 0; off <= s.maxOffset(); ++off) {
            s.offset = off;
            for (int i = 0; i < s.visible; ++i) {
                const int idx = s.offset + i;
                KT_CHECK(idx < n);                                   // a clamped window never shows a slot past the last entry
                KT_CHECK_EQ(s.slotOf(idx), i);                       // and slotOf is the inverse of the rebind
            }
            KT_CHECK_EQ(s.slotOf(off - 1), -1);
            KT_CHECK_EQ(s.slotOf(off + s.visible), -1);
        }
        KT_CHECK_EQ(s.maxOffset(), n - 8);
    }
    // Option rows hidden at run time: the window shrinks with the visible count and the offset follows.
    ScrollWindow r;
    r.total = 24;
    r.offset = 16;
    r.total = 20;                                                    // 4 rows hidden under a scrolled page
    r.clamp();
    KT_CHECK_EQ(r.offset, 12);
    KT_CHECK(r.ensureVisible(19) == false && r.offset == 12);        // last row is on screen
    r.total = 8;
    r.clamp();
    KT_CHECK(r.offset == 0 && !r.needBar());                         // everything fits again: no bar
    // Scroll bar inside the column keeps the thumb on the track for every offset.
    const Rect track = {0, 30, 12, 200};
    ScrollWindow t;
    t.total = 24;
    for (int off = 0; off <= t.maxOffset(); ++off) {
        t.offset = off;
        const Rect th = t.thumb(track);
        KT_CHECK(th.y >= track.y && th.y + th.h <= track.y + track.h && th.h >= kThumbMin);
    }
}

// ---------------------------------------------------------------- tick-gap reset
static void test_tickreset() {
    const unsigned gaps[] = {0, 400, 500, 501, 600, 5000, 5001, 60000};
    for (int gi = 0; gi < 8; ++gi) {
        const unsigned g = gaps[gi];
        for (int lmb = 0; lmb < 2; ++lmb) {
            for (int drag = 0; drag < 2; ++drag) {
                // Panel hidden: today's behaviour, > 500 ms resets, buttons and drag are irrelevant.
                KT_CHECK_EQ(tickResetDue(g, 0, lmb != 0, drag != 0), g > 500);
                KT_CHECK_EQ(tickResetDue(g, 0x01, lmb != 0, drag != 0), g > 500);       // other flag bits do not count as shown
                // Panel shown: only a long gap with no button and no drag.
                KT_CHECK_EQ(tickResetDue(g, 0x40, lmb != 0, drag != 0), g > 5000 && !lmb && !drag);
                KT_CHECK_EQ(tickResetDue(g, 0xFF, lmb != 0, drag != 0), g > 5000 && !lmb && !drag);
            }
        }
    }
    KT_CHECK(!tickResetDue(600, 0x40, false, false));             // shown + 600 ms hitch: no reset
    KT_CHECK(tickResetDue(600, 0x00, false, false));              // hidden + 600 ms: reset
    KT_CHECK(tickResetDue(5001, 0x40, false, false));             // shown but gone for 5 s with the mouse up: reset
    KT_CHECK(!tickResetDue(5001, 0x40, true, false));             // ... unless the button is held
    KT_CHECK(tickResetDue(300, 0, false, false, 200, 1000));      // custom thresholds
    KT_CHECK(!tickResetDue(900, 0x40, false, false, 200, 1000));
    KT_CHECK(tickResetDue(1001, 0x40, false, false, 200, 1000));
}

// ---------------------------------------------------------------- modal pages (NewModal / SetModalOpen)
static void test_modal_table() {
    ModalTable md;
    PageStack ps;
    KT_CHECK_EQ(md.shownSlot(ps), -1);                             // MAIN: no entry page
    KT_CHECK_EQ(md.add(-1), -1); KT_CHECK_EQ(md.add(kMaxEntries), -1);   // parent must be an entry slot
    const int a = md.add(3), b = md.add(3), c = md.add(7);
    KT_CHECK_EQ(a, 0); KT_CHECK_EQ(b, 1); KT_CHECK_EQ(c, 2);
    KT_CHECK_EQ(ModalTable::slotOf(a), kMaxEntries); KT_CHECK_EQ(ModalTable::slotOf(c), kMaxEntries + 2);
    KT_CHECK(ModalTable::isModalSlot(kMaxEntries) && ModalTable::isModalSlot(kMaxSlots - 1));
    KT_CHECK(!ModalTable::isModalSlot(kMaxEntries - 1) && !ModalTable::isModalSlot(kMaxSlots));
    KT_CHECK_EQ(ModalTable::indexOf(kMaxEntries + 2), 2);
    KT_CHECK_EQ(md.parentOfSlot(3), 3); KT_CHECK_EQ(md.parentOfSlot(kMaxEntries + 2), 7); KT_CHECK_EQ(md.parentOfSlot(kMaxEntries + 5), -1);
    KT_CHECK_EQ(md.parentOfSlot(-1), -1); KT_CHECK_EQ(md.parentOfSlot(kMaxSlots), -1);
    // not shown yet: the parent is not the shown page, so opening is refused
    KT_CHECK(!md.setOpen(a, true, ps));
    ps.push(LIST, -1);
    KT_CHECK(!md.setOpen(a, true, ps));                            // LIST is not an entry page
    ps.push(ENTRY, 7);
    KT_CHECK(!md.setOpen(a, true, ps));                            // a belongs to entry 3, entry 7 is shown
    KT_CHECK_EQ(md.shownSlot(ps), 7);
    KT_CHECK(md.setOpen(c, true, ps));
    KT_CHECK_EQ(md.open, c); KT_CHECK_EQ(md.shownSlot(ps), kMaxEntries + 2);
    KT_CHECK(!md.setOpen(c, true, ps));                            // already open: no change
    KT_CHECK(!md.setOpen(99, true, ps)); KT_CHECK(!md.setOpen(-1, false, ps));
    KT_CHECK(!md.setOpen(a, false, ps));                           // a is not the open one
    KT_CHECK_EQ(md.open, c);
    KT_CHECK(md.setOpen(c, false, ps));
    KT_CHECK_EQ(md.open, -1); KT_CHECK_EQ(md.shownSlot(ps), 7);
    KT_CHECK(!md.setOpen(c, false, ps));                           // closing twice is a no-op
    // closeFirst consumes exactly one Back
    KT_CHECK(md.setOpen(c, true, ps));
    KT_CHECK(md.closeFirst()); KT_CHECK_EQ(md.open, -1); KT_CHECK(!md.closeFirst());
    // a second modal of the same parent replaces the first only through close + open (one open at a time)
    ps.back();
    ps.push(ENTRY, 3);
    KT_CHECK(md.setOpen(a, true, ps)); KT_CHECK(md.setOpen(b, true, ps));
    KT_CHECK_EQ(md.open, b);
    // a page change clears: clear() then shownSlot is the entry page; an open modal never shows for another entry
    md.clear(); KT_CHECK_EQ(md.open, -1);
    KT_CHECK(md.setOpen(a, true, ps));
    ps.reset();
    KT_CHECK_EQ(md.shownSlot(ps), -1);                             // MAIN
    ps.push(LIST, -1); ps.push(ENTRY, 7);
    KT_CHECK_EQ(md.shownSlot(ps), 7);                              // stale open modal of entry 3 does not capture entry 7
    // full table
    ModalTable full;
    for (int i = 0; i < kMaxModals; ++i) KT_CHECK_EQ(full.add(0), i);
    KT_CHECK_EQ(full.add(0), -1);
}

static void test_gear_rect() {
    // slot height 32 (column 284 px tall): side 27, margin 4, pane 300x200 at (100,50) -> right edge 100+300-4 = 396, top 54
    Rect g = gearButtonRect(Rect{100, 50, 300, 200}, 32);
    KT_CHECK_EQ(g.w, 27); KT_CHECK_EQ(g.h, 27); KT_CHECK_EQ(g.x, 396 - 27); KT_CHECK_EQ(g.y, 54);
    // larger slot: 64 -> side 54, margin 9
    g = gearButtonRect(Rect{0, 0, 400, 300}, 64);
    KT_CHECK_EQ(g.w, 54); KT_CHECK_EQ(g.x, 400 - 9 - 54); KT_CHECK_EQ(g.y, 9);
    // small pane: side shrinks to min(w,h) - 2*margin (50 - 8 = 42 > 27, so no shrink; 20 - 8 = 12 does)
    g = gearButtonRect(Rect{0, 0, 300, 20}, 32);
    KT_CHECK_EQ(g.w, 12); KT_CHECK_EQ(g.h, 12); KT_CHECK_EQ(g.y, 4);
    // no room: empty rect at the pane origin
    g = gearButtonRect(Rect{5, 6, 6, 6}, 32);
    KT_CHECK_EQ(g.w, 0); KT_CHECK_EQ(g.h, 0); KT_CHECK_EQ(g.x, 5); KT_CHECK_EQ(g.y, 6);
    g = gearButtonRect(Rect{0, 0, 100, 100}, 0);                   // zero slot height: nothing to draw
    KT_CHECK_EQ(g.w, 0);
}

// MCF5 part 2: the pure bits behind the slider row wiring (part counts, hit rules, grab rule, handler gating, caption text).
static void test_row_slider_wiring() {
    KT_CHECK_EQ(rowPartCount(0), 1); KT_CHECK_EQ(rowPartCount(1), 3); KT_CHECK_EQ(rowPartCount(2), 4);
    KT_CHECK_EQ(rowPartCount(-1), 0); KT_CHECK_EQ(rowPartCount(3), 0); KT_CHECK_EQ(rowPartCount(99), 0);
    KT_CHECK(rowKindValid(0) && rowKindValid(1) && rowKindValid(2));
    KT_CHECK(!rowKindValid(-1) && !rowKindValid(3));
    // item parts: 3 frame, 4 wedge, 5 mark; only the arrows repeat, only the frame and the caption are hit-tested
    KT_CHECK_EQ(RP_FRAME, 3); KT_CHECK_EQ(RP_WEDGE, 4); KT_CHECK_EQ(RP_MARK, 5);
    for (int p = 0; p <= 5; ++p) KT_CHECK_EQ(rowPartIsArrow(p), p == 1 || p == 2);
    for (int p = 0; p <= 5; ++p) KT_CHECK_EQ(rowPartHittable(p), p != 4 && p != 5);
    // grab rule: slider kind only, never disabled / hidden; the restart and wrap flags do not matter
    KT_CHECK(sliderGrabbable(2, 0));
    KT_CHECK(sliderGrabbable(2, RF_RESTART | RF_WRAP));
    KT_CHECK(!sliderGrabbable(2, RF_DISABLED));
    KT_CHECK(!sliderGrabbable(2, RF_HIDDEN));
    KT_CHECK(!sliderGrabbable(2, RF_DISABLED | RF_HIDDEN | RF_RESTART));
    KT_CHECK(!sliderGrabbable(1, 0));
    KT_CHECK(!sliderGrabbable(0, 0));
    // handler dir = sign of the move
    KT_CHECK_EQ(sliderChangeDir(10, 20), 1); KT_CHECK_EQ(sliderChangeDir(20, 10), -1); KT_CHECK_EQ(sliderChangeDir(7, 7), 0);
    KT_CHECK_EQ(sliderChangeDir(INT_MIN, INT_MAX), 1); KT_CHECK_EQ(sliderChangeDir(INT_MAX, INT_MIN), -1);
    // handler gating while held: only a moved value that was not already asked for
    KT_CHECK(sliderAskHandler(true, 60, 55));
    KT_CHECK(!sliderAskHandler(true, 60, 60));    // a rejected value under a still pointer is not asked again every tick
    KT_CHECK(!sliderAskHandler(false, 60, 55));   // not moved
    KT_CHECK(sliderAskHandler(true, 55, 60));     // back to the earlier value: asked again
    // a slider row's caption is the stepper text: "Label: value", fmt stands alone, choices and zero text apply, restart suffix
    const RowCfg sl = RowCfg{2, 0, 100, 5, 35, 0};
    char b[96];
    KT_CHECK(rowFormat(b, sizeof b, "Volume", sl, nullptr, nullptr, 0, nullptr)); KT_CHECK(strcmp(b, "Volume: 35") == 0);
    KT_CHECK(rowFormat(b, sizeof b, "Volume", sl, "Volume %d%%", nullptr, 0, nullptr)); KT_CHECK(strcmp(b, "Volume 35%") == 0);
    RowCfg z = sl; z.value = 0;
    KT_CHECK(rowFormat(b, sizeof b, "Volume", z, "Volume %d%%", nullptr, 0, "Muted")); KT_CHECK(strcmp(b, "Volume: Muted") == 0);
    static const char* const ch[] = {"Low", "Mid", "High"};
    const RowCfg cs = RowCfg{2, 0, 2, 1, 1, RF_RESTART};
    KT_CHECK(rowFormat(b, sizeof b, "Quality", cs, nullptr, ch, 3, nullptr)); KT_CHECK(strcmp(b, "Quality: Mid (restart)") == 0);
    // rowClamp / rowApplyHandlerResult treat kind 2 as a range (not 0/1); a disabled slider keeps its value
    KT_CHECK_EQ(rowClamp(sl, 150), 100); KT_CHECK_EQ(rowClamp(sl, -3), 0); KT_CHECK_EQ(rowClamp(sl, 42), 42);
    KT_CHECK_EQ(rowApplyHandlerResult(sl, 60, 55, true), 55);
    KT_CHECK_EQ(rowApplyHandlerResult(sl, 60, 0, false), 60);
    RowCfg dis = sl; dis.flags = RF_DISABLED;
    KT_CHECK_EQ(rowApplyHandlerResult(dis, 60, 60, true), 35);
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
    test_split_clamp();
    test_split_vanilla_rect();
    test_split_out_of_range_clamps();
    test_split_zero_area();
    test_split_odd_heights();
    test_row_clamp_step();
    test_row_toggle();
    test_row_handler_result();
    test_row_format();
    test_row_geometry();
    test_row_hit_zones();
    test_scroll_clamp_wheel();
    test_scroll_ensure_visible();
    test_scroll_thumb();
    test_scroll_list_rebind_maths();
    test_tickreset();
    test_modal_table();
    test_gear_rect();
    test_row_slider_wiring();
    return kt::Report("menu_ui_logic_test");
}
