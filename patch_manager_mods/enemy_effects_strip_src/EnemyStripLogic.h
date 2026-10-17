// Enemy Effects Strip -- pure logic (host-testable; libc headers only, no Win32/engine types). The engine reader (EnemyStrip.cpp)
// fills EffIn from the creature's effect list, resolving icon resrefs; everything here is classification, tiering, slot selection
// and layout arithmetic. Tested by enemy_strip_logic_test.cpp. Run: python3 scripts/run_tests.py enemy_strip_logic
#pragma once
#include <string.h>

namespace esl {

// ---------------------------------------------------------------- tunable constants (correct here if an icon name is wrong)
constexpr const char* kIconDroidStun = "ip_droidstun";   // type 0x08 state 3
constexpr const char* kIconStun      = "ip_stun";        // type 0x08 state 4
constexpr const char* kIconHold      = "ip_hold";        // type 0x08 state 5
constexpr const char* kIconSleep     = "ip_sleep";       // type 0x08 state 6
constexpr const char* kIconFear      = "ip_fear";        // type 0x08 state 8
constexpr const char* kIconSlow      = "ip_slow";        // type 0x1d MovementSpeedDecrease
constexpr const char* kIconEntangle  = "iw_adhsvgren_001";   // type 0x12 Entangle

constexpr int kMaxIcons = 8;

// ---------------------------------------------------------------- effect classification
struct EffIn {
    int type, sub, spell, state;   // state -1 = none
    float ratio;                   // remaining fraction, -1 = unknown
    bool byParty;
    const char* debuffIcon;        // set by the reader when the spell (or harmful type 0x43 row) has a debuff icon
    const char* buffIcon;
    bool item;
    int int2 = 0;                  // effect ints[2]; 0 when the effect has fewer than 3 ints
};

enum Cls { C_Never, C_Obvious, C_Debuff, C_Buff };

inline bool isBookkeepingType(int t) {
    switch (t) {
        case 0x01: case 0x09: case 0x13: case 0x17: case 0x1f: case 0x39: case 0x3c: case 0x44: case 0x67: return true;
        default: return false;
    }
}

inline bool isObviousState(int s) {
    switch (s) {
        case 1: case 2: case 3: case 4: case 5: case 6: case 8: case 16: return true;
        default: return false;
    }
}

inline Cls classify(const EffIn& e) {
    if (e.type == 0x1e) return C_Never;                       // visual effect
    if (e.type == 0x02 && (e.sub & 8) && e.int2 > 0) return C_Never;   // shield pool (R5: Energy Resistance, sub 9 / int2 0, is a normal buff)
    if (isBookkeepingType(e.type)) return C_Never;
    if (e.type == 0x08 && isObviousState(e.state)) return C_Obvious;
    if (e.type == 0x12 || e.type == 0x14 || e.type == 0x1d) return C_Obvious;   // Entangle, Knockdown, MovementSpeedDecrease
    if (e.debuffIcon) return C_Debuff;
    if (e.buffIcon) return C_Buff;
    return C_Never;
}

inline const char* stateFallbackIcon(int type, int state) {
    if (type == 0x08) {
        switch (state) {
            case 3: return kIconDroidStun;
            case 4: return kIconStun;
            case 5: return kIconHold;
            case 6: return kIconSleep;
            case 8: return kIconFear;
            default: return nullptr;
        }
    }
    if (type == 0x1d) return kIconSlow;
    if (type == 0x12) return kIconEntangle;
    return nullptr;
}

// ---------------------------------------------------------------- awareness tiers
struct Tiers { int obvious = 5, own = 12, full = 20; };

inline int tierFor(int awareness, const Tiers& t) {
    if (awareness < t.obvious) return 0;
    if (awareness < t.own) return 1;
    if (awareness < t.full) return 2;
    return 3;
}

inline bool visibleAt(Cls c, bool byParty, int tier) {
    switch (c) {
        case C_Obvious: return tier >= 1;
        case C_Debuff:  return tier >= 3 || (tier >= 2 && byParty);
        case C_Buff:    return tier >= 3;
        default:        return false;
    }
}

inline bool drainAt(int tier) { return tier >= 3; }

// ---------------------------------------------------------------- slot selection
struct Slot { const char* resref; float ratio; bool item; bool debuff; };

// out holds outCap Slots; at most min(maxIcons clamped to 1..8, outCap) are written. Null in/out, n <= 0 or outCap < 1 -> 0.
inline int buildSlots(const EffIn* in, int n, int tier, Slot* out, int outCap, int maxIcons) {
    if (!in || !out || n <= 0 || outCap < 1) return 0;
    if (maxIcons < 1) maxIcons = 1;
    if (maxIcons > kMaxIcons) maxIcons = kMaxIcons;
    if (maxIcons > outCap) maxIcons = outCap;
    int count = 0;
    const Cls order[3] = { C_Obvious, C_Debuff, C_Buff };
    for (int pass = 0; pass < 3; ++pass) {
        for (int i = 0; i < n; ++i) {
            const EffIn& e = in[i];
            Cls c = classify(e);
            if (c != order[pass] || !visibleAt(c, e.byParty, tier)) continue;
            const char* icon = e.debuffIcon;
            if (!icon) icon = stateFallbackIcon(e.type, e.state);
            if (!icon) icon = e.buffIcon;
            if (!icon) continue;
            int dup = -1;
            for (int k = 0; k < count; ++k) if (strcmp(out[k].resref, icon) == 0) { dup = k; break; }
            if (dup >= 0) {
                if (e.ratio >= 0.0f && e.ratio > out[dup].ratio) out[dup].ratio = e.ratio;
                continue;
            }
            if (count >= maxIcons) continue;
            out[count].resref = icon;
            out[count].ratio = e.ratio;
            out[count].item = e.item;
            out[count].debuff = (c == C_Obvious || c == C_Debuff);
            ++count;
        }
    }
    return count;
}

// ---------------------------------------------------------------- layout
struct RowRect { int x, y, w, h; };
struct Layout { int n; RowRect icon[8]; RowRect pill; bool ok; };

inline Layout layoutRow(int menuX, int menuY, int nameX, int nameY, int nameW, int nameH,
                        int n, int iconPx, int gapPx, int spacePx, int pillPad) {
    Layout l;
    memset(&l, 0, sizeof l);
    int s = iconPx > 0 ? iconPx : nameH;
    if (n < 0) n = 0;
    if (n > kMaxIcons) n = kMaxIcons;
    l.n = n;
    int rowW = n > 0 ? n * s + (n - 1) * spacePx : 0;
    int x0 = menuX + nameX + nameW / 2 - rowW / 2;
    int y = menuY + nameY - gapPx - s;
    for (int k = 0; k < n; ++k) l.icon[k] = { x0 + k * (s + spacePx), y, s, s };
    l.pill = { x0 - pillPad, y - pillPad, rowW + 2 * pillPad, s + 2 * pillPad };
    l.ok = !(n == 0 || s < 4 || y < 0);
    return l;
}

// ---------------------------------------------------------------- cooldown overlay
// Same semantics as bh::brightPx (BuffHudLogic.h) for known ratios, clamped to [0,s]; unknown ratio (<0) = fully bright.
inline int brightPx(int s, float ratio) {
    // NaN, negative (unknown) and >= 1 are decided before the cast: int() of NaN/inf/huge floats is UB.
    if (ratio != ratio || ratio < 0.0f) return s;
    if (ratio >= 1.0f) return s;
    if (ratio <= 0.0f) return 0;
    int bright = int(s * ratio + 0.5f);
    if (bright > s) bright = s;
    return bright;
}

inline int overlayH(int s, float ratio) { return s - brightPx(s, ratio); }

// The overlay texture has a baked alpha of 0.78; this is the draw alpha needed for an effective alpha of `want`.
inline float overlayAlpha(float wantEffective) {
    float a = wantEffective / 0.78f;
    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;
    return a;
}

}  // namespace esl
