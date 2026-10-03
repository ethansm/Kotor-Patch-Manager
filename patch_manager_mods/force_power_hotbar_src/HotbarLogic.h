// Force Power Hotbar -- item target selection (pure C++17, no Win32/engine types so it can be unit-tested on the host).
// The DLL fills one Candidate per party position (HP, "readable", "down", "can this item be used on them"); this header decides.
#pragma once
#include "HotbarConfig.h"

namespace hb {

struct Candidate { int cur = 0, max = 0; bool ok = false, down = false, eligible = true; };   // index = party position, 0 = controlled character

// Index of the target, or -1 = do nothing. Lowest HP% strictly below the threshold wins; none -> self if fallbackSelf and self is usable.
inline int chooseTarget(const Candidate* m, int n, int selfIdx, int thresholdPct, bool fallbackSelf) {
    if (!m || n <= 0) return -1;
    if (thresholdPct < 0) thresholdPct = 0;
    if (thresholdPct > 100) thresholdPct = 100;
    int best = -1;
    for (int i = 0; i < n; ++i) {
        const Candidate& c = m[i];
        if (!c.ok || c.down || !c.eligible || c.max <= 0) continue;
        if ((long long)c.cur * 100 >= (long long)thresholdPct * c.max) continue;   // strictly below, no division rounding
        if (best < 0) { best = i; continue; }
        long long a = (long long)c.cur * m[best].max, b = (long long)m[best].cur * c.max;   // cur/max < best.cur/best.max
        if (a < b || (a == b && i == selfIdx)) best = i;   // ties: self wins, else the earlier index stays
    }
    if (best >= 0) return best;
    if (fallbackSelf && selfIdx >= 0 && selfIdx < n && m[selfIdx].ok && !m[selfIdx].down && m[selfIdx].eligible) return selfIdx;
    return -1;
}

// False => draw the slot dimmed and ignore clicks/keys: lowest-HP with no fallback and nobody to heal.
inline bool slotEnabled(const Candidate* m, int n, int selfIdx, const TargetSpec& t, const General& g) {
    return !(t.mode == TM_LowHp && !effectiveFallbackSelf(t, g) && chooseTarget(m, n, selfIdx, effectiveThreshold(t, g), false) == -1);
}

// Vanilla's "usable on another party member" rule: baseitems.2da itemtype (engine GetBaseItem+0xAC) is not 20 (forearm bands =
// energy shields, self only) and not 47 (squad recovery kit, used directly). Negative = unknown -> false.
inline bool isTargetableItemType(int itemType) { return itemType >= 0 && itemType != 20 && itemType != 47; }

}  // namespace hb
