// HolsterAnimStyle.h -- pure decision logic for the holstered-creature unarmed-style override (doc 42; no Win32, host-testable;
// tests in holster_animstyle_test.cpp). The engine animation resolver (0x863650) picks walk/run/idle clip ids from a weapon style
// byte (+0xC5; 8 = unarmed). A creature whose weapon is on the hip should resolve as unarmed for the locomotion clips.
//   idInFilter           the animation ids the override looks at: 0x2711 (armed idle), 0x27A4, 0x2712, 0x2713 (walk rows), 0x2714 (run)
//   styleHasOwnLocoRow   styles with their OWN wield-dependent locomotion row: {2,3,4,7,9} (style compared as a byte)
//   decide               V_UNARMED only when every condition holds, otherwise V_KEEP (engine result untouched)
//   frameOk / overrideStyle   HW16 (0.13.0-dev, hook H4b mid-function in the resolver, after the style byte was spilled to [ebp-1]): the resolver-frame guard and the byte to store
//                        there (kStyleUnarmed) or "leave it" -- the handler writes ONE byte and only when overrideStyle says so
//   HolsterSet           fixed 64-slot owner -> base map with a 500 ms TTL (wrap-safe ms ticks); full set refuses, never overwrites a live entry.
//                        owner = the CLIENT creature, base = its body object ([creature + Creature_BodyObj]). The resolver hook receives the BODY object, not the creature
//                        (0.16.0-dev, .claude/waves/hr7/r3.md), so the resolver side asks ownerOfBase(body) to get the creature back.
#pragma once
#include <stdint.h>

namespace hs {

static const uint32_t kHolsterSetTtlMs = 500;
static const int kHolsterSetCap = 64;

inline bool idInFilter(uint32_t id) {
    return id == 0x2711u || id == 0x27A4u || id == 0x2712u || id == 0x2713u || id == 0x2714u;
}

inline bool styleHasOwnLocoRow(uint32_t style) {
    const uint32_t s = style & 0xFFu;
    return s == 2u || s == 3u || s == 4u || s == 7u || s == 9u;
}

static const uint8_t kStyleUnarmed = 8;   // +0xC5 / [ebp-1] value the resolver treats as "no weapon style"

enum Verdict { V_KEEP = 0, V_UNARMED = 1 };

inline Verdict decide(uint32_t id, uint32_t gate, uint32_t style, bool inHolsterSet, bool ownerCombatZero) {
    if (gate == 1u && idInFilter(id) && styleHasOwnLocoRow(style) && inHolsterSet && ownerCombatZero) return V_UNARMED;
    return V_KEEP;
}

// The resolver frame pointer handed to the H4b handler: non-null, above the low 64 KiB, dword aligned (an MSVC EBP frame always is). Anything else is refused untouched.
inline bool frameOk(uintptr_t ebp) { return ebp >= 0x10000u && (ebp & 3u) == 0; }

// The anim id the resolver compares: the u16 argument at [ebp+8], upper half is garbage.
inline uint32_t animId16(uint32_t raw) { return raw & 0xFFFFu; }

// True and *out = kStyleUnarmed when the style byte [ebp-1] should be overwritten, else false (*out untouched). style is the byte just read from [ebp-1].
inline bool overrideStyle(uint32_t idRaw, uint32_t gate, uint32_t style, bool inHolsterSet, bool ownerCombatZero, uint8_t* out) {
    if (out == 0) return false;
    if (decide(animId16(idRaw), gate, style, inHolsterSet, ownerCombatZero) != V_UNARMED) return false;
    *out = kStyleUnarmed;
    return true;
}

class HolsterSet {
public:
    // true = inserted or updated; false = refused (owner 0, or the set is full of live entries).
    bool touch(uintptr_t owner, uintptr_t base, uint32_t now) {
        if (owner == 0) return false;
        int freeSlot = -1;
        int expiredSlot = -1;
        for (int i = 0; i < kHolsterSetCap; ++i) {
            if (!e_[i].used) {
                if (freeSlot < 0) freeSlot = i;
                continue;
            }
            if (e_[i].owner == owner) {
                set(i, owner, base, now);
                return true;
            }
            if (expiredSlot < 0 && !live(e_[i], now)) expiredSlot = i;
        }
        int slot = freeSlot >= 0 ? freeSlot : expiredSlot;
        if (slot < 0) return false;
        set(slot, owner, base, now);
        return true;
    }

    // owner 0 is never stored (touch refuses it), so remove/contains need no explicit guard for it.
    void remove(uintptr_t owner) {
        for (int i = 0; i < kHolsterSetCap; ++i)
            if (e_[i].used && e_[i].owner == owner) e_[i] = Entry();
    }

    bool contains(uintptr_t owner, uintptr_t base, uint32_t now) const {
        for (int i = 0; i < kHolsterSetCap; ++i)
            if (e_[i].used && e_[i].owner == owner) return e_[i].base == base && live(e_[i], now);
        return false;
    }

    // 0.16.0-dev: the owner (client creature) of the live entry whose base (body object) equals `base`, else 0. base 0 never matches. Expired entries do not match.
    uintptr_t ownerOfBase(uintptr_t base, uint32_t now) const {
        if (base == 0) return 0;
        for (int i = 0; i < kHolsterSetCap; ++i)
            if (e_[i].used && e_[i].base == base && live(e_[i], now)) return e_[i].owner;
        return 0;
    }

    int count(uint32_t now) const {
        int n = 0;
        for (int i = 0; i < kHolsterSetCap; ++i)
            if (live(e_[i], now)) ++n;
        return n;
    }

private:
    struct Entry {
        uintptr_t owner;
        uintptr_t base;
        uint32_t stamp;
        bool used;
        Entry() : owner(0), base(0), stamp(0), used(false) {}
    };

    static bool live(const Entry& e, uint32_t now) {
        if (!e.used) return false;
        const int32_t age = (int32_t)(now - e.stamp);
        return age >= 0 && age <= (int32_t)kHolsterSetTtlMs;
    }

    void set(int i, uintptr_t owner, uintptr_t base, uint32_t now) {
        e_[i].owner = owner;
        e_[i].base = base;
        e_[i].stamp = now;
        e_[i].used = true;
    }

    Entry e_[kHolsterSetCap];
};

} // namespace hs
