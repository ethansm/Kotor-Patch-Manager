// MenuUiLogic.h -- pure logic for the "Mod Configuration" provider inside the KOTOR II Options panel (header-only,
// namespace mui, ints and fixed arrays only: no engine access, no STL containers, no heap). Card c16.
// Host test: menu_ui_logic_test.cpp.
//
// Page model: MAIN (vanilla Options + our "Mod Configuration" button) -> LIST (one button per registered mod, hover
// shows its description) -> ENTRY(k) (mod k's own page). Entry index k = registry token - 1 (= registry slot).
//
//   mui::Registry reg;
//   int tok = reg.registerEntry("npc-diversity");        // token 1..8, 0 = rejected
//   mui::PageStack ps;
//   mui::PageEvents pe = ps.push(mui::LIST, -1);         // pe.n == 2: LEAVE(MAIN), ENTER(LIST)
//   mui::PageTable pt;                                   // page ids: 0 = MAIN, 1 = LIST, 2 + k = ENTRY(k)
//   mui::HoverTracker hov;  mui::HoverEvents he = hov.update(hitItem);   // hitItem -1 = none
//   mui::ClickTracker clk;  int clicked = clk.tick(lmbDown, hitItem);    // item id on release, else -1
//   bool go = mui::canTruncate(listed, count, vanilla, isOurs, ctx);
//
// Event order, PageStack: every real page change emits LEAVE(from) then ENTER(to). entry is -1 unless the state is ENTRY.
//   push(LIST) from MAIN:      LEAVE(MAIN), ENTER(LIST)
//   push(ENTRY, k) from LIST:  LEAVE(LIST), ENTER(ENTRY,k)
//   back() from ENTRY(k):      LEAVE(ENTRY,k), ENTER(LIST)
//   back() from LIST:          LEAVE(LIST), ENTER(MAIN)
//   reset() from LIST:         LEAVE(LIST), ENTER(MAIN)
//   reset() from ENTRY(k):     LEAVE(ENTRY,k), ENTER(MAIN)   (no LEAVE(LIST): the LIST page is skipped)
//   reset() from MAIN, back() from MAIN, and every invalid push: no events, no change.
// Hover, HoverTracker: update(new) emits LEAVE(old) when old >= 0, then ENTER(new) when new >= 0. Same item: nothing.
//   Any hitItem < 0 or >= kMaxItems counts as none.
// Click, ClickTracker: a click fires on the release tick when the item under the cursor at release equals the item at
//   press. Press on none then release over an item: no click. Held across ticks keeps the press item.
#pragma once

#include <stdint.h>
#include <string.h>

namespace mui {

enum { kMaxEntries = 8, kMaxItems = 24, kMaxPages = 2 + kMaxEntries, kMaxEvents = 4 };

// ---------------------------------------------------------------------------------------------------- registry
// A slot is live while registered. seq is the registration order of the id's first registration: removing an entry
// does not reorder the rest, and a re-registered id goes to the end. Tokens are slot + 1 and stay valid for other entries.
struct RegSlot {
    const char* id = nullptr;   // not copied: the string must outlive the registry (string literals in practice)
    int opaque = 0;             // caller value; registerEntry sets it to the slot index, setOpaque changes it
    unsigned seq = 0;
    bool live = false;
};

struct Registry {
    RegSlot slots[kMaxEntries];
    unsigned nextSeq = 1;

    // Returns the token (slot + 1, > 0). Same id again: keeps token, seq and opaque, and stores the new id pointer.
    // Returns 0 for a null or empty id, or when full and the id is not already registered.
    int registerEntry(const char* id);
    bool unregister(int token);                    // false when the token is not live
    bool setOpaque(int token, int value);          // false when the token is not live
    bool opaqueOf(int token, int* out) const;      // false when the token is not live or out is null
    int count() const;                             // number of live entries
    int at(int i) const;                           // token of the i-th live entry in registration order, 0 if out of range
};

inline int Registry::registerEntry(const char* id) {
    if (!id || !id[0]) return 0;
    int freeSlot = -1;
    for (int s = 0; s < kMaxEntries; ++s) {
        if (!slots[s].live) {
            if (freeSlot < 0) freeSlot = s;
        } else if (strcmp(slots[s].id, id) == 0) {
            slots[s].id = id;
            return s + 1;
        }
    }
    if (freeSlot < 0) return 0;
    RegSlot& e = slots[freeSlot];
    e.id = id;
    e.opaque = freeSlot;
    e.seq = nextSeq++;
    e.live = true;
    return freeSlot + 1;
}

inline bool Registry::unregister(int token) {
    if (token < 1 || token > kMaxEntries) return false;
    RegSlot& e = slots[token - 1];
    if (!e.live) return false;
    e = RegSlot();
    return true;
}

inline bool Registry::setOpaque(int token, int value) {
    if (token < 1 || token > kMaxEntries || !slots[token - 1].live) return false;
    slots[token - 1].opaque = value;
    return true;
}

inline bool Registry::opaqueOf(int token, int* out) const {
    if (!out || token < 1 || token > kMaxEntries || !slots[token - 1].live) return false;
    *out = slots[token - 1].opaque;
    return true;
}

inline int Registry::count() const {
    int n = 0;
    for (int s = 0; s < kMaxEntries; ++s)
        if (slots[s].live) ++n;
    return n;
}

// A live slot's rank is the number of live slots with a smaller seq. seq is unique among live slots, so ranks are 0..count-1.
inline int Registry::at(int i) const {
    if (i < 0) return 0;
    for (int s = 0; s < kMaxEntries; ++s) {
        if (!slots[s].live) continue;
        int rank = 0;
        for (int t = 0; t < kMaxEntries; ++t)
            if (slots[t].live && slots[t].seq < slots[s].seq) ++rank;
        if (rank == i) return s + 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------- events
enum EventKind { ENTER = 0, LEAVE = 1 };

// ---------------------------------------------------------------------------------------------------- page stack
enum State { MAIN = 0, LIST = 1, ENTRY = 2 };

struct PageEvent {
    EventKind kind = ENTER;
    State state = MAIN;
    int entry = -1;             // ENTRY index k when state == ENTRY, else -1
};

struct PageEvents {
    PageEvent ev[kMaxEvents] = {};
    int n = 0;
    void add(EventKind kind, State state, int entry) {
        if (n >= kMaxEvents) return;
        ev[n].kind = kind;
        ev[n].state = state;
        ev[n].entry = (state == ENTRY) ? entry : -1;
        ++n;
    }
};

struct PageStack {
    State state = MAIN;
    int entry = -1;

    // MAIN -> LIST (k ignored), LIST -> ENTRY(k) with 0 <= k < kMaxEntries. Any other push: no events, no change.
    PageEvents push(State to, int k);
    // ENTRY -> LIST, LIST -> MAIN. From MAIN: no events, no change.
    PageEvents back();
    // Any page -> MAIN. Event order is documented at the top of this file. From MAIN: no events.
    PageEvents reset();
};

inline PageEvents PageStack::push(State to, int k) {
    PageEvents r;
    if (state == MAIN && to == LIST) {
        r.add(LEAVE, MAIN, -1);
        r.add(ENTER, LIST, -1);
        state = LIST;
        entry = -1;
    } else if (state == LIST && to == ENTRY && k >= 0 && k < kMaxEntries) {
        r.add(LEAVE, LIST, -1);
        r.add(ENTER, ENTRY, k);
        state = ENTRY;
        entry = k;
    }
    return r;
}

inline PageEvents PageStack::back() {
    PageEvents r;
    if (state == ENTRY) {
        r.add(LEAVE, ENTRY, entry);
        r.add(ENTER, LIST, -1);
        state = LIST;
    } else if (state == LIST) {
        r.add(LEAVE, LIST, -1);
        r.add(ENTER, MAIN, -1);
        state = MAIN;
    } else {
        return r;
    }
    entry = -1;
    return r;
}

inline PageEvents PageStack::reset() {
    PageEvents r;
    if (state == LIST) {
        r.add(LEAVE, LIST, -1);
        r.add(ENTER, MAIN, -1);
    } else if (state == ENTRY) {
        r.add(LEAVE, ENTRY, entry);
        r.add(ENTER, MAIN, -1);
    }
    state = MAIN;
    entry = -1;
    return r;
}

// ---------------------------------------------------------------------------------------------------- hover edge
struct HoverEvent {
    EventKind kind = ENTER;
    int item = -1;
};

struct HoverEvents {
    HoverEvent ev[2] = {};
    int n = 0;
    void add(EventKind kind, int item) {
        ev[n].kind = kind;
        ev[n].item = item;
        ++n;
    }
};

struct HoverTracker {
    int cur = -1;               // item under the cursor, -1 = none
    // Emits LEAVE(old) then ENTER(new), each only when that side is a real item. Same item: nothing.
    HoverEvents update(int hitItem);
};

inline HoverEvents HoverTracker::update(int hitItem) {
    if (hitItem < 0 || hitItem >= kMaxItems) hitItem = -1;
    HoverEvents r;
    if (hitItem == cur) return r;
    if (cur >= 0) r.add(LEAVE, cur);
    if (hitItem >= 0) r.add(ENTER, hitItem);
    cur = hitItem;
    return r;
}

// ---------------------------------------------------------------------------------------------------- release-edge click
struct ClickTracker {
    bool down = false;
    int pressItem = -1;         // item under the cursor when the press started, -1 = none
    // Call once per tick. Returns the clicked item id on the release tick, else -1.
    int tick(bool lmbDown, int hitItem);
};

inline int ClickTracker::tick(bool lmbDown, int hitItem) {
    if (hitItem < 0 || hitItem >= kMaxItems) hitItem = -1;
    if (lmbDown) {
        if (!down) {
            down = true;
            pressItem = hitItem;
        }
        return -1;
    }
    if (!down) return -1;
    const int clicked = (pressItem >= 0 && hitItem == pressItem) ? pressItem : -1;
    down = false;
    pressItem = -1;
    return clicked;
}

// ---------------------------------------------------------------------------------------------------- page item tables
enum ItemKind { BUTTON = 0, LABEL = 1, CUSTOM = 2 };

struct Rect {
    int x;
    int y;
    int w;
    int h;
};

struct Item {
    ItemKind kind = BUTTON;
    void* ctl = nullptr;
    int flags = 0;
    Rect rect = {0, 0, 0, 0};
    bool visibleInPage = true;
};

struct Page {
    Item items[kMaxItems];
    int nItems = 0;
};

struct PageTable {
    Page pages[kMaxPages];
    int nPages = 0;

    // Returns the page id (0 .. kMaxPages-1), or -1 when full.
    int addPage() {
        if (nPages >= kMaxPages) return -1;
        return nPages++;
    }
    int pageCount() const { return nPages; }
    // Returns the item id (0 .. kMaxItems-1), or -1 when the page is invalid or full. New items are visible.
    int addItem(int page, ItemKind kind, void* ctl, int flags, Rect rect);
    int itemCount(int page) const {
        return (page < 0 || page >= nPages) ? 0 : pages[page].nItems;
    }
    // Null when the page or item id is invalid.
    const Item* item(int page, int id) const;
    bool setVisible(int page, int id, bool visible);
    // First visible item (lowest id) whose rect contains (px, py), half-open: x <= px < x+w, y <= py < y+h. -1 if none.
    int hitTest(int page, int px, int py) const;
};

inline int PageTable::addItem(int page, ItemKind kind, void* ctl, int flags, Rect rect) {
    if (page < 0 || page >= nPages) return -1;
    Page& p = pages[page];
    if (p.nItems >= kMaxItems) return -1;
    Item& it = p.items[p.nItems];
    it.kind = kind;
    it.ctl = ctl;
    it.flags = flags;
    it.rect = rect;
    it.visibleInPage = true;
    return p.nItems++;
}

inline const Item* PageTable::item(int page, int id) const {
    if (page < 0 || page >= nPages || id < 0 || id >= pages[page].nItems) return nullptr;
    return &pages[page].items[id];
}

inline bool PageTable::setVisible(int page, int id, bool visible) {
    if (page < 0 || page >= nPages || id < 0 || id >= pages[page].nItems) return false;
    pages[page].items[id].visibleInPage = visible;
    return true;
}

inline int PageTable::hitTest(int page, int px, int py) const {
    if (page < 0 || page >= nPages) return -1;
    const Page& p = pages[page];
    for (int i = 0; i < p.nItems; ++i) {
        const Item& it = p.items[i];
        if (!it.visibleInPage) continue;
        if (px >= it.rect.x && px < it.rect.x + it.rect.w && py >= it.rect.y && py < it.rect.y + it.rect.h) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------------------------------------- truncate guard
// True only when count >= vanilla and every listed[i] for i in [vanilla, count) is ours. count == vanilla is true
// (nothing to remove). vanilla < 0, count < vanilla, or a null list/predicate with entries to check: false.
inline bool canTruncate(const uintptr_t* listed, int count, int vanilla, bool (*isOurs)(uintptr_t, void*), void* ctx) {
    if (vanilla < 0 || count < vanilla) return false;
    if (count == vanilla) return true;
    if (!listed || !isOurs) return false;
    for (int i = vanilla; i < count; ++i)
        if (!isOurs(listed[i], ctx)) return false;
    return true;
}

}  // namespace mui
