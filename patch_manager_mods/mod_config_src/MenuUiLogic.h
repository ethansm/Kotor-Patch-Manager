// MenuUiLogic.h -- pure logic for the "Mod Configuration" provider inside the KOTOR II Options panel (header-only,
// namespace mui, ints and fixed arrays only: no engine access, no STL containers, no heap). Card c16.
// Host test: menu_ui_logic_test.cpp.
//
// Page model: MAIN (vanilla Options + our "Mod Configuration" button) -> LIST (one button per registered mod, hover
// shows its description) -> ENTRY(k) (mod k's own page). Entry index k = registry token - 1 (= registry slot).
//
//   mui::Registry reg;
//   int tok = reg.registerEntry("npc-diversity");        // token 1..24, 0 = rejected
//   mui::PageStack ps;
//   mui::PageEvents pe = ps.push(mui::LIST, -1);         // pe.n == 2: LEAVE(MAIN), ENTER(LIST)
//   mui::PageTable pt;                                   // page ids: 0 = MAIN, 1 = LIST, 2 + k = ENTRY(k)
//   mui::HoverTracker hov;  mui::HoverEvents he = hov.update(hitItem);   // hitItem -1 = none
//   mui::ClickTracker clk;  int clicked = clk.tick(lmbDown, hitItem);    // item id on release, else -1
//   bool go = mui::canTruncate(listed, count, vanilla, isOurs, ctx);
//   mui::DescSplit ds = mui::splitDesc(origDescRect, 700);   // ds.preview (top) + ds.desc (bottom 30 %), see "description split" below
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

// Page ids: 0 MAIN, 1 LIST, 2 + slot. Slots 0..kMaxEntries-1 are the entry pages (slot = token - 1); slots kMaxEntries.. are modal pages (ModalTable).
enum { kMaxEntries = 24, kMaxItems = 80, kMaxModals = 8, kMaxSlots = kMaxEntries + kMaxModals, kMaxPages = 2 + kMaxSlots, kMaxEvents = 4 };

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

// ---------------------------------------------------------------------------------------------------- modal pages (K2MenuUiApi.NewModal)
// A modal is an extra page slot (kMaxEntries + m) that belongs to one entry page (its parent). It is not a PageStack state: the entry
// page stays ENTRY(k), its enter / tick / leave callbacks keep running, and only the controls on screen change while the modal is open
// (the parent's provider-drawn controls hide, the modal's show). At most one modal is open; it can only be opened while its parent is the
// shown entry page. Back closes the modal first (closeFirst), and every page change (Esc, tick gap, panel hidden, leaving the entry) clears it.
//   shownSlot(ps): the slot whose controls take input: the open modal's slot while its parent is shown, else ps.entry, -1 outside an entry page.
struct ModalTable {
    int parent[kMaxModals];   // owning entry slot, -1 = unused
    int n = 0;
    int open = -1;            // open modal index, -1 = none
    ModalTable() { for (int i = 0; i < kMaxModals; ++i) parent[i] = -1; }

    static bool isModalSlot(int slot) { return slot >= kMaxEntries && slot < kMaxSlots; }
    static int slotOf(int m) { return kMaxEntries + m; }
    static int indexOf(int slot) { return slot - kMaxEntries; }
    // New modal for entry slot `parentEntry` (0 <= parentEntry < kMaxEntries). Returns the modal index, or -1 when the table is full or the parent is invalid.
    int add(int parentEntry) {
        if (parentEntry < 0 || parentEntry >= kMaxEntries || n >= kMaxModals) return -1;
        parent[n] = parentEntry;
        return n++;
    }
    bool valid(int m) const { return m >= 0 && m < n; }
    // Parent entry slot of a slot: the entry itself for an entry slot, the owner for a modal slot, -1 when unknown.
    int parentOfSlot(int slot) const {
        if (slot >= 0 && slot < kMaxEntries) return slot;
        return isModalSlot(slot) && valid(indexOf(slot)) ? parent[indexOf(slot)] : -1;
    }
    // Opens (open = true) or closes modal m. Opening needs ps to show m's parent (ENTRY, entry == parent) and closes any other open modal.
    // Returns true when the open state changed.
    bool setOpen(int m, bool wantOpen, const PageStack& ps) {
        if (!valid(m)) return false;
        if (wantOpen) {
            if (ps.state != ENTRY || ps.entry != parent[m] || open == m) return false;
            open = m;
            return true;
        }
        if (open != m) return false;
        open = -1;
        return true;
    }
    // Back / Escape: closes the open modal and reports true (the press is consumed); false when none is open.
    bool closeFirst() {
        if (open < 0) return false;
        open = -1;
        return true;
    }
    void clear() { open = -1; }
    int shownSlot(const PageStack& ps) const {
        if (ps.state != ENTRY || ps.entry < 0 || ps.entry >= kMaxEntries) return -1;
        if (open >= 0 && parent[open] == ps.entry) return slotOf(open);
        return ps.entry;
    }
};

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
    // Two passes: non-CUSTOM items first, CUSTOM (a preview's label pool covers the whole pane) only when nothing else is hit.
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < p.nItems; ++i) {
            const Item& it = p.items[i];
            if (!it.visibleInPage || (it.kind == CUSTOM) != (pass == 1)) continue;
            if (px >= it.rect.x && px < it.rect.x + it.rect.w && py >= it.rect.y && py < it.rect.y + it.rect.h) return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------------------------------- description split
// SetDescSplit math (K2MenuUiApi.SetDescSplit / MUI_PANE_PREVIEW). The right-hand description pane keeps the bottom part of its ORIGINAL
// rect, the preview area is the top part, with a small gap between them. All integer math, rounded to nearest.
//   previewPermille <= 0 (off): preview = desc = orig (the preview pane is the whole original rect, nothing shrinks).
//   1..5000: clamped to kSplitMin..kSplitMax (description keeps 15..40 %).
//   gap = round(6 * orig.h / 336) (6 px on the vanilla 336 px rect, scaled), descH = round((1000 - permille) * orig.h / 1000),
//   preview.h = orig.h - gap - descH, so preview.h + gap + desc.h == orig.h exactly. Both rects keep orig.x and orig.w.
//   A rect with no area (w <= 0 or h <= 0) comes back unchanged in both parts.
enum { kSplitMin = 600, kSplitMax = 850, kSplitRefH = 336, kSplitRefGap = 6 };

struct DescSplit {
    Rect preview;
    Rect desc;
};

// 0 for off (<= 0), else the permille clamped to kSplitMin..kSplitMax.
inline int clampSplit(int previewPermille) {
    if (previewPermille <= 0) return 0;
    return previewPermille < kSplitMin ? kSplitMin : (previewPermille > kSplitMax ? kSplitMax : previewPermille);
}

inline DescSplit splitDesc(const Rect& orig, int previewPermille) {
    DescSplit r;
    r.preview = orig;
    r.desc = orig;
    const int pm = clampSplit(previewPermille);
    if (pm == 0 || orig.w <= 0 || orig.h <= 0) return r;
    int gap = (kSplitRefGap * orig.h + kSplitRefH / 2) / kSplitRefH;
    if (gap > orig.h) gap = orig.h;
    int descH = ((1000 - pm) * orig.h + 500) / 1000;
    if (descH > orig.h - gap) descH = orig.h - gap;
    const int previewH = orig.h - gap - descH;
    r.preview.h = previewH;
    r.desc.y = orig.y + previewH + gap;
    r.desc.h = descH;
    return r;
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

// ---------------------------------------------------------------------------------------------------- option rows (toggle / stepper)
// Pure model behind K2MenuUiApi.AddRow (doc 46). A row is one button (toggle) or a label + "<" + ">" trio (stepper); button rows stay AddButton.
//   rowClamp:  toggle -> 0/1 (any non-zero is 1); stepper -> [minV, maxV] (a swapped range is read as min..max).
//   rowStep:   toggle flips; stepper moves by dir * step (step < 1 reads as 1) from the clamped value. Past an end it wraps (RF_WRAP) or clamps.
//              dir == 0 returns the clamped value. A value off the step grid (e.g. 37 at step 5) moves from where it is: the handler snaps.
//   rowApplyHandlerResult: the handler returns the FINAL value to store. The result is clamped to the row's range; with no handler the request
//              is stored (clamped). A disabled row never changes (returns c.value). "Reject" = the handler returns the old value.
//   Slider rows (kind 2, SliderRowLogic.h) use the stepper range / step / value / rowFormat captions; rowStep is not used by them.
//   rowFormat: caption text. value text = choices[v] if choices, else zeroText at 0 (steppers), else On/Off (toggle), else fmt applied to v.
//              Only "%d" and "%%" are expanded in fmt (every other '%' stays literal: no printf on consumer strings). A caption built from fmt
//              stands alone ("Heal below 35%"); every other value text is "Label: value". RF_RESTART appends " (restart)".
//              Returns false when out is null/cap < 1 or the text was cut; out is always NUL-terminated when cap > 0.
enum RowKind { RK_TOGGLE = 0, RK_STEPPER = 1 };         // RK_SLIDER = 2 lives in SliderRowLogic.h (MCF5)
enum RowFlag { RF_WRAP = 1, RF_RESTART = 2, RF_DISABLED = 4, RF_HIDDEN = 8 };

// Parts (controls) of a row, as kept in itemPart[][]: toggle = 1 control (RP_CAPTION, the whole slot); stepper = caption + "<" + ">" = 3;
// slider = caption + frame + wedge + mark = 4 (SliderRowLogic.h geometry). An unknown kind has 0 parts (AddRow refuses it).
enum RowPart { RP_CAPTION = 0, RP_LEFT = 1, RP_RIGHT = 2, RP_FRAME = 3, RP_WEDGE = 4, RP_MARK = 5 };
inline int rowPartCount(int kind) { return kind == 0 ? 1 : (kind == 1 ? 3 : (kind == 2 ? 4 : 0)); }
inline bool rowKindValid(int kind) { return kind >= 0 && kind <= 2; }
inline bool rowPartIsArrow(int part) { return part == RP_LEFT || part == RP_RIGHT; }
// The wedge and the mark sit on top of the frame and are never hit-tested: the frame (the whole track) is the one hit area of a slider.
inline bool rowPartHittable(int part) { return part != RP_WEDGE && part != RP_MARK; }
// A press on the slider track grabs it only on a live slider row (kind 2 = RK_SLIDER): a disabled or hidden one takes no grab.
inline bool sliderGrabbable(int kind, int flags) { return kind == 2 && (flags & (RF_DISABLED | RF_HIDDEN)) == 0; }
// dir the change handler gets for a slider move: the sign of (requested - old); 0 when unchanged.
inline int sliderChangeDir(int oldV, int reqV) { return reqV > oldV ? 1 : (reqV < oldV ? -1 : 0); }
// During a held drag the handler is called only when the pointer value differs from what is stored AND from the last value already asked for
// (a rejected value under a still pointer is asked once, not every tick).
inline bool sliderAskHandler(bool moved, int v, int lastAsked) { return moved && v != lastAsked; }

struct RowCfg {
    int kind;
    int minV, maxV, step, value, flags;
};

inline int rowClamp(const RowCfg& c, int v) {
    if (c.kind == RK_TOGGLE) return v != 0 ? 1 : 0;
    int lo = c.minV, hi = c.maxV;
    if (hi < lo) { const int t = lo; lo = hi; hi = t; }
    return v < lo ? lo : (v > hi ? hi : v);
}

inline int rowStep(const RowCfg& c, int v, int dir) {
    if (c.kind == RK_TOGGLE) return v != 0 ? 0 : 1;
    const int cur = rowClamp(c, v);
    if (dir == 0) return cur;
    int lo = c.minV, hi = c.maxV;
    if (hi < lo) { const int t = lo; lo = hi; hi = t; }
    const int st = c.step < 1 ? 1 : c.step;
    const bool wrap = (c.flags & RF_WRAP) != 0;
    // Distances in 64 bits: a consumer range wider than INT_MAX (e.g. INT_MIN..INT_MAX) must not overflow hi - cur / cur - lo.
    if (dir > 0) {
        if (cur >= hi) return wrap ? lo : hi;
        return ((long long)hi - cur < st) ? (wrap ? lo : hi) : cur + st;
    }
    if (cur <= lo) return wrap ? hi : lo;
    return ((long long)cur - lo < st) ? (wrap ? hi : lo) : cur - st;
}

inline int rowApplyHandlerResult(const RowCfg& c, int requested, int handlerFinal, bool haveHandler) {
    if (c.flags & RF_DISABLED) return rowClamp(c, c.value);
    return rowClamp(c, haveHandler ? handlerFinal : requested);
}

struct FmtBuf {
    char* p;
    int cap;
    int n;
    bool ok;
    void put(char ch) {
        if (n + 1 < cap) { p[n++] = ch; p[n] = 0; } else ok = false;
    }
    void str(const char* s) {
        if (!s) return;
        while (*s) put(*s++);
    }
    void num(int v) {
        char t[12];
        int k = 0;
        unsigned u = v < 0 ? 0u - (unsigned)v : (unsigned)v;
        do { t[k++] = (char)('0' + (u % 10)); u /= 10; } while (u);
        if (v < 0) put('-');
        while (k) put(t[--k]);
    }
};

// Expands "%d" (the value) and "%%" in fmt; null/empty fmt = "%d".
inline void fmtExpand(FmtBuf& b, const char* fmt, int v) {
    if (!fmt || !fmt[0]) { b.num(v); return; }
    for (const char* s = fmt; *s; ++s) {
        if (s[0] == '%' && s[1] == 'd') { b.num(v); ++s; }
        else if (s[0] == '%' && s[1] == '%') { b.put('%'); ++s; }
        else b.put(*s);
    }
}

inline bool rowFormat(char* out, int cap, const char* label, const RowCfg& c, const char* fmt, const char* const* choices, int nChoices, const char* zeroText) {
    if (!out || cap < 1) return false;
    out[0] = 0;
    FmtBuf b = {out, cap, 0, true};
    const int v = rowClamp(c, c.value);
    const bool hasLabel = label && label[0];
    const char* text = nullptr;
    bool stand = false;                                  // the caption is fmt's output alone
    if (choices && nChoices > 0) {
        const int i = v < 0 ? 0 : (v >= nChoices ? nChoices - 1 : v);
        text = choices[i] ? choices[i] : "";
    } else if (c.kind == RK_TOGGLE) {
        text = v ? "On" : "Off";
    } else if (v == 0 && zeroText && zeroText[0]) {
        text = zeroText;
    } else if (fmt && fmt[0]) {
        stand = true;
    }
    if (stand) {
        fmtExpand(b, fmt, v);
    } else {
        if (hasLabel) { b.str(label); b.str(": "); }
        if (text) b.str(text); else b.num(v);
    }
    if (c.flags & RF_RESTART) b.str(" (restart)");
    return b.ok;
}

// ---------------------------------------------------------------------------------------------------- row geometry
// Rows sit in the COLUMN pane (the vanilla button column, 284 px tall at the reference size): slot k at y + k * pitch, pitch = h*36/284,
// slot height = h*32/284, so 8 slots fit exactly (7*36 + 32 = 284). A stepper slot is [label][gap][<][gap][>]: arrows 20 % of the width each,
// gap 6, the label takes the rest; widths sum to the slot width. A toggle slot is the whole slot (label/left/right are empty).
enum { kRowsVisible = 8, kRowRefH = 284, kRowPitchRef = 36, kRowSlotRef = 32, kRowGap = 6, kRowArrowPct = 20 };

inline Rect rowSlotRect(const Rect& col, int visibleIndex) {
    Rect r = col;
    r.y = col.y + visibleIndex * ((col.h * kRowPitchRef) / kRowRefH);
    r.h = (col.h * kRowSlotRef) / kRowRefH;
    return r;
}

struct RowGeom {
    int kind;
    Rect whole, label, left, right;
};

inline RowGeom rowGeom(const Rect& slot, int kind) {
    RowGeom g;
    g.kind = kind;
    g.whole = slot;
    const Rect none = {slot.x, slot.y, 0, 0};
    g.label = g.left = g.right = none;
    if (kind == RK_TOGGLE) { g.label = slot; return g; }
    const int aw = (slot.w * kRowArrowPct) / 100;
    int lw = slot.w - 2 * aw - 2 * kRowGap;
    if (lw < 0) lw = 0;
    g.label = Rect{slot.x, slot.y, lw, slot.h};
    g.left = Rect{slot.x + lw + kRowGap, slot.y, aw, slot.h};
    g.right = Rect{slot.x + lw + kRowGap + aw + kRowGap, slot.y, aw, slot.h};
    return g;
}

enum Zone { Z_NONE = 0, Z_LABEL = 1, Z_LEFT = 2, Z_RIGHT = 3, Z_WHOLE = 4 };

inline bool rectHas(const Rect& r, int px, int py) {
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}

// Toggle: Z_WHOLE inside the slot. Stepper: Z_LEFT / Z_RIGHT / Z_LABEL; the 6 px gaps and anything outside are Z_NONE.
inline int rowHitZone(const RowGeom& g, int px, int py) {
    if (g.kind == RK_TOGGLE) return rectHas(g.whole, px, py) ? Z_WHOLE : Z_NONE;
    if (rectHas(g.left, px, py)) return Z_LEFT;
    if (rectHas(g.right, px, py)) return Z_RIGHT;
    if (rectHas(g.label, px, py)) return Z_LABEL;
    return Z_NONE;
}

// ---------------------------------------------------------------------------------------------------- gear button (K2MenuUiApi.AddGear)
// The provider-drawn settings gear: a square in the top-right corner of a pane (the preview pane, else the description pane), the same on every page.
// side = 85 % of the row slot height (slotH, 32/284 of the column height) so it reads as one of the option rows' controls; margin = max(2, side/6) from the
// top and right edges; the side shrinks to fit a small pane (min(w, h) - 2 * margin); a pane with no room (side < 1) gives an empty rect.
inline Rect gearButtonRect(const Rect& pane, int slotH) {
    Rect r = {pane.x, pane.y, 0, 0};
    int side = (slotH * 85) / 100;
    const int margin = (side / 6) > 2 ? side / 6 : 2;
    const int room = (pane.w < pane.h ? pane.w : pane.h) - 2 * margin;
    if (side > room) side = room;
    if (side < 1) return r;
    r.x = pane.x + pane.w - margin - side;
    r.y = pane.y + margin;
    r.w = side;
    r.h = side;
    return r;
}

// ---------------------------------------------------------------------------------------------------- scroll window
// Rows [offset, offset + visible) of `total` are shown. Wheel: + = up (offset shrinks), 1 row per notch. The bar thumb is
// visible/total of the track (min kThumbMin px, or the whole track when it is shorter), its top maps linearly to the offset.
enum { kThumbMin = 12 };

struct ScrollWindow {
    int total = 0, visible = kRowsVisible, offset = 0;

    int maxOffset() const { return total > visible ? total - visible : 0; }
    void clamp() { offset = offset < 0 ? 0 : (offset > maxOffset() ? maxOffset() : offset); }
    bool setOffset(int v) {
        const int old = offset;
        offset = v;
        clamp();
        return offset != old;
    }
    bool wheel(int notches) { return setOffset(offset - notches); }
    bool ensureVisible(int idx) {                       // idx outside [0, total) is ignored
        if (idx < 0 || idx >= total) return false;
        if (idx < offset) return setOffset(idx);
        if (idx >= offset + visible) return setOffset(idx - visible + 1);
        return false;
    }
    bool needBar() const { return total > visible; }
    int slotOf(int idx) const {                         // displayed slot of row idx, -1 when outside the window
        const int s = idx - offset;
        return (idx >= 0 && idx < total && s >= 0 && s < visible) ? s : -1;
    }
    int thumbHeight(const Rect& track) const {
        if (!needBar() || track.h <= 0) return track.h > 0 ? track.h : 0;
        int h = (track.h * visible) / total;
        if (h < kThumbMin) h = kThumbMin;
        return h > track.h ? track.h : h;
    }
    Rect thumb(const Rect& track) const {
        Rect t = track;
        t.h = thumbHeight(track);
        const int travel = track.h - t.h, mo = maxOffset();
        if (travel > 0 && mo > 0) t.y = track.y + (travel * offset + mo / 2) / mo;
        return t;
    }
    // Offset for a thumb dragged so the cursor (py) holds the thumb `grab` px below its top.
    int offsetFromThumbTop(int py, int grab, const Rect& track) const {
        const int travel = track.h - thumbHeight(track), mo = maxOffset();
        if (travel <= 0 || mo <= 0) return 0;
        int rel = py - grab - track.y;
        rel = rel < 0 ? 0 : (rel > travel ? travel : rel);
        return (rel * mo + travel / 2) / travel;
    }
};

// ---------------------------------------------------------------------------------------------------- tick-gap reset
// McTick runs every frame the Options panel draws. A long gap used to mean "the panel closed" (reset the page stack at > 500 ms), but
// a game hitch with the panel still open and the mouse held also gives a long gap. flags48 bit 0x40 is the Options panel's shown bit
// (live logs 10-17): while it is set only a LONG gap with no button held and no thumb drag counts as a close.
inline bool tickResetDue(unsigned gapMs, int flags48, bool lmbDown, bool dragging, unsigned shortGap = 500, unsigned longGap = 5000) {
    const bool shown = (flags48 & 0x40) != 0;
    if (!shown) return gapMs > shortGap;
    return gapMs > longGap && !lmbDown && !dragging;
}

}  // namespace mui
