// PanelSlots.h -- fixed-size per-panel slot table for KOTOR2 patch DLLs (header-only, no hooks, no engine access, no STL).
// Survey: project design doc 19b section D (the findSlot/claimSlot pair was copy-pasted into
// CharEffectsPanel, Inventory3DViewport and EquipCharPreview, each with its own idea of "this slot is in use").
//
// What is shared: WHICH slot a panel owns (find), picking one for a new panel (claim: existing, else free, else evict the oldest),
// and giving a slot back (release). What stays per mod: the payload (Slot) and what a slot MEANS (bound flag, scene resources...).
//
//   PanelSlots<Slot, N> t;                 // Slot: default-constructible + copy-assignable; value-initialised, so POD members start 0
//   t.onEvict = [](Slot& s, uintptr_t panel, unsigned seq) { ... };   // optional, plain function pointer (captureless lambda ok)
//   Slot* s = t.claim(panel, ++seq, &fresh);   // bind (or re-bind) a panel
//   Slot* s = t.find(panel);                   // every later frame; nullptr when the panel has no slot
//   t.release(panel);                          // panel closed
//
// Semantics (pinned by panelslots_test.cpp):
//   * "used" is tracked here, not in the payload. A mod that needs a second state (CharEffectsPanel's `bound`: slot claimed but
//     the page was unusable) keeps it in Slot and checks it after find().
//   * claim() of a panel that already has a slot returns THAT slot unchanged (payload kept, nothing evicted, onEvict NOT called)
//     and refreshes its seq to the one passed (callers always did `s->seq = ++g_seq` right after). *fresh (optional) = false, so
//     a mod that tears down on re-bind (EquipCharPreview) can do it by checking it. Seq is "newer = larger"; wrap-around ignored.
//   * Otherwise it takes the first free entry; with none free it evicts the entry with the smallest seq (ties: lowest index),
//     calling onEvict(slot, panel, seq) on the OLD occupant first (release its resources there). Either way the payload is then
//     reset to Slot() (value-initialised) and *fresh = true. The returned slot is never null.
//   * release() frees the entry and returns whether the panel had one. It does NOT call onEvict (the caller owns a deliberate
//     close and can find() first); the payload stays readable by pointer but is invisible to find()/forEach() and is reset at
//     the next claim() that reuses the entry.
//   * Panel 0 is an ordinary key (no sentinel; free entries are told apart by the used flag), so claim(0, ...) / find(0) work.
//     Mods that treat panel 0 as "no panel" should reject it before calling.
//   * forEach(f) calls f(Slot&) on every used entry in index order (not by seq).
// Host tests: g++ -m32 / x64. Win32 mod builds: only <stdint.h>/<stddef.h>.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace panelslots {

template <class Slot, int N>
struct PanelSlots {
    static_assert(N > 0, "PanelSlots needs at least one entry");
    typedef void (*EvictFn)(Slot& slot, uintptr_t panel, unsigned seq);

    EvictFn onEvict = nullptr;   // called on the old occupant of an entry claim() takes over; may be left null

    Slot* find(uintptr_t panel) {
        Entry* e = lookup(panel);
        return e ? &e->slot : nullptr;
    }

    Slot* claim(uintptr_t panel, unsigned seq, bool* fresh = nullptr) {
        if (Entry* hit = lookup(panel)) {
            hit->seq = seq;
            if (fresh) *fresh = false;
            return &hit->slot;
        }
        Entry* pick = nullptr;
        for (int i = 0; i < N; ++i) {
            if (!e_[i].used) { pick = &e_[i]; break; }
            if (!pick || e_[i].seq < pick->seq) pick = &e_[i];
        }
        if (pick->used && onEvict) onEvict(pick->slot, pick->panel, pick->seq);
        pick->slot = Slot();
        pick->panel = panel;
        pick->seq = seq;
        pick->used = true;
        if (fresh) *fresh = true;
        return &pick->slot;
    }

    bool release(uintptr_t panel) {
        Entry* e = lookup(panel);
        if (!e) return false;
        e->used = false;
        return true;
    }

    template <class F>
    void forEach(F f) {
        for (int i = 0; i < N; ++i)
            if (e_[i].used) f(e_[i].slot);
    }

    int count() const {
        int n = 0;
        for (int i = 0; i < N; ++i) n += e_[i].used ? 1 : 0;
        return n;
    }

private:
    struct Entry {
        uintptr_t panel = 0;
        unsigned seq = 0;
        bool used = false;
        Slot slot{};
    };
    Entry e_[N];

    Entry* lookup(uintptr_t panel) {
        for (int i = 0; i < N; ++i)
            if (e_[i].used && e_[i].panel == panel) return &e_[i];
        return nullptr;
    }
};

}  // namespace panelslots
