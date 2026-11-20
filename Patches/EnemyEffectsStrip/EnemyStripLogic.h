// Enemy Effects Strip -- pure logic (host-testable; libc headers only, no Win32/engine types). The engine reader (EnemyStrip.cpp)
// fills EffIn from the creature's effect list, resolving icon resrefs; everything here is classification, tiering, slot selection
// and layout arithmetic. Tested by enemy_strip_logic_test.cpp. Run: python3 scripts/run_tests.py enemy_strip_logic
#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

// ---------------------------------------------------------------- settings-page preview (0.4.1)
// The mod-config page shows an example of the strip in the PREVIEW pane: a mock target name bar (the real one is the target menu's name
// label) with the icon row above it, laid out by layoutRow from the CURRENT settings at 1:1 engine px, centred in the pane.
constexpr int kPreviewMaxIcons = 5;      // page item budget: bar + pill + 5 x (icon, overlay) + 2 stacked health fills = 14 controls
constexpr int kPreviewNameW = 200;       // mock name bar edge sizes (about the vanilla target name label); iconSize 0 = auto = kPreviewNameH
constexpr int kPreviewNameH = 24;
constexpr int kPreviewControls = 4 + 2 * kPreviewMaxIcons;
constexpr int kPreviewHealthPermille = 720;   // mock health fill: share of the bar's width (the real bar is the vanilla target menu's PB_HEALTH under the name)

// The mock health fill inside the name bar `bar`: the bottom ~30 % of the bar, inset by ~1/12 of its height, 72 % of the inner width (the rest of
// the black bar shows as the empty track). The name text is drawn top-centred above it. Never leaves `bar`.
inline RowRect previewHealthRect(const RowRect& bar) {
    const int inset = bar.h >= 12 ? bar.h / 12 : 1;
    int hh = bar.h * 30 / 100;
    if (hh < 2) hh = 2;
    RowRect r = { bar.x + inset, bar.y + bar.h - hh - inset, (bar.w - 2 * inset) * kPreviewHealthPermille / 1000, hh };
    if (r.w < 1 || r.y < bar.y) r = { bar.x, bar.y, 0, 0 };
    return r;
}

struct PreviewSample { const char* resref; float ratio; };
// The sample effects, in row order (the strip's real icons) with a spread of remaining-time ratios so the drain overlay is visible.
inline const PreviewSample& previewSample(int k) {
    static const PreviewSample t[kPreviewMaxIcons] = {
        { kIconFear, 0.25f }, { kIconStun, 0.5f }, { kIconSlow, 0.75f }, { kIconHold, 0.1f }, { kIconSleep, 0.9f },
    };
    return t[k < 0 ? 0 : (k >= kPreviewMaxIcons ? kPreviewMaxIcons - 1 : k)];
}

struct PreviewLayout {
    bool ok;          // the name bar fits the pane
    int n;            // icons shown (0 = row hidden: Enabled off, no room, or an icon edge below 4 px)
    bool pill;        // draw the pill (backing on and a row shown)
    RowRect name;     // the mock name bar
    Layout row;       // icon rects + pill rect (valid for row.n icons)
};

// pane: the preview rect (engine px). The name bar is kPreviewNameW x kPreviewNameH (narrower/shorter when the pane is); the row sits above it
// exactly as the HUD places it (layoutRow with the name bar as the name label); the block (row + pill + name bar) is centred in the pane and
// never leaves it: icons are dropped from the right until the row (plus pill padding) fits the width, and the row is hidden when it does not
// fit the height. An empty pane gives ok = false.
inline PreviewLayout previewLayout(const RowRect& pane, int enabled, int maxIcons, int iconPx, int gapPx, int spacePx, int pillPad, bool backing,
                                   int nameWBase = kPreviewNameW, int nameHBase = kPreviewNameH) {
    PreviewLayout p;
    memset(&p, 0, sizeof p);
    if (pane.w <= 0 || pane.h <= 0) return p;
    if (gapPx < 0) gapPx = 0;
    if (spacePx < 0) spacePx = 0;
    if (pillPad < 0) pillPad = 0;
    const int nameW = pane.w < nameWBase ? pane.w : nameWBase;
    const int nameH = pane.h < nameHBase ? pane.h : nameHBase;
    if (pillPad > gapPx + nameH) pillPad = gapPx + nameH;   // the pill's lower padding must stay inside the block (the block ends at the name bar's bottom)
    const int s = iconPx > 0 ? iconPx : nameHBase;
    const int pad = backing ? pillPad : 0;
    int n = enabled ? maxIcons : 0;
    if (n > kPreviewMaxIcons) n = kPreviewMaxIcons;
    if (n < 0) n = 0;
    if (s < 4 || pad + s + gapPx + nameH > pane.h) n = 0;
    while (n > 0 && n * s + (n - 1) * spacePx + 2 * pad > pane.w) --n;
    const int rowW = n > 0 ? n * s + (n - 1) * spacePx : 0;
    const int bw = n > 0 && rowW + 2 * pad > nameW ? rowW + 2 * pad : nameW;
    const int bh = n > 0 ? nameH + gapPx + s + pad : nameH;
    const int bx = pane.x + (pane.w - bw) / 2;
    const int by = pane.y + (pane.h - bh) / 2;
    p.name = { bx + (bw - nameW) / 2, by + bh - nameH, nameW, nameH };
    p.n = n;
    p.pill = n > 0 && backing;
    // layoutRow centres the row on the name bar with integer halves; with rowW + 2 * pad <= bw the row and its pill padding stay inside the block.
    if (n > 0) p.row = layoutRow(0, 0, p.name.x, p.name.y, p.name.w, p.name.h, n, s, gapPx, spacePx, pillPad);
    p.ok = true;
    return p;
}

// Zoom for the settings-page preview, in percent (100 = 1:1 engine px, the real HUD size; clamped to 100..600). The real strip is tiny on a
// large display (default icon edge 14 px), so the page draws the whole example (icons, gaps, spacing, pill padding and the mock name bar)
// scaled up: the mock name bar takes about 55 % of the pane width, shrunk when the scaled icon row (90 % of the width) or the scaled block
// (90 % of the height) would not fit. Only the sizes scale; the ratios between settings stay as in the HUD.
inline int previewZoomPct(const RowRect& pane, int maxIcons, int iconPx, int gapPx, int spacePx, int pillPad, bool backing) {
    if (pane.w <= 0 || pane.h <= 0) return 100;
    if (gapPx < 0) gapPx = 0;
    if (spacePx < 0) spacePx = 0;
    if (pillPad < 0) pillPad = 0;
    const int s = iconPx > 0 ? iconPx : kPreviewNameH;
    const int pad = backing ? pillPad : 0;
    int n = maxIcons;
    if (n > kPreviewMaxIcons) n = kPreviewMaxIcons;
    if (n < 1) n = 1;
    long z = static_cast<long>(pane.w) * 55 / kPreviewNameW;
    const long rowW = static_cast<long>(n) * s + static_cast<long>(n - 1) * spacePx + 2L * pad;
    const long zr = rowW > 0 ? static_cast<long>(pane.w) * 9000 / 100 / rowW : z;
    const long blockH = static_cast<long>(kPreviewNameH) + gapPx + s + pad;
    const long zh = blockH > 0 ? static_cast<long>(pane.h) * 90 / blockH : z;
    if (zr < z) z = zr;
    if (zh < z) z = zh;
    if (z < 100) z = 100;
    if (z > 600) z = 600;
    return static_cast<int>(z);
}

// previewLayout with every size scaled by previewZoomPct (rounded to whole px). At 100 % the result equals previewLayout exactly.
inline PreviewLayout previewLayoutZoomed(const RowRect& pane, int enabled, int maxIcons, int iconPx, int gapPx, int spacePx, int pillPad, bool backing) {
    const int z = previewZoomPct(pane, maxIcons, iconPx, gapPx, spacePx, pillPad, backing);
    auto sc = [z](int v) { return v <= 0 ? v : static_cast<int>((static_cast<long>(v) * z + 50) / 100); };
    return previewLayout(pane, enabled, maxIcons, iconPx > 0 ? sc(iconPx) : 0, sc(gapPx), sc(spacePx), sc(pillPad), backing,
                         sc(kPreviewNameW), sc(kPreviewNameH));
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

// The drain overlay is drawn only for a visibly non-zero alpha (> 0.001); 0 (the default), NaN and negatives mean no overlay widget is shown.
inline bool overlayVisible(float drainAlpha) { return drainAlpha > 0.001f; }

// The overlay texture has a baked alpha of 0.78; this is the draw alpha needed for an effective alpha of `want`.
inline float overlayAlpha(float wantEffective) {
    float a = wantEffective / 0.78f;
    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;
    return a;
}

// ---------------------------------------------------------------- INI value parsing and range clamps
// The engine side (EnemyStrip.cpp loadIni) reads the raw INI values and hands them here, so every range rule is host-testable.
inline int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// A NaN value falls back to `def`; infinities clamp like any out-of-range number.
inline float clampFloat(float v, float lo, float hi, float def) {
    if (v != v) return def;
    return v < lo ? lo : (v > hi ? hi : v);
}

// Whole-string float parse: surrounding whitespace is fine, trailing junk ("0.65abc"), an empty string and NaN give `def`.
inline float parseFloatStrict(const char* s, float def) {
    if (!s) return def;
    char* end = nullptr;
    double v = strtod(s, &end);
    if (end == s || v != v) return def;
    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') ++end;
    if (*end != '\0') return def;
    if (v > 1e30) v = 1e30;      // keep the double -> float cast defined
    if (v < -1e30) v = -1e30;
    return static_cast<float>(v);
}

// Raw (unclamped) [Strip] values; the defaults equal the shipped enemy_strip.ini.
struct CfgRaw {
    int enabled = 1, verbose = 0, maxIcons = 5, iconSize = 0, gap = 4, spacing = 4;
    int tierObvious = 5, tierOwn = 12, tierFull = 20;
    int backing = 0, backingPad = 4, forceTier = -1;
    float drainAlpha = 0.0f, backingAlpha = 0.55f;   // drain shading is OFF by default (0.0); set e.g. 0.65 for the dark drain overlay
};

// Clamps every field to its documented range, then enforces the tier ordering obvious <= own <= full by raising the LATER value.
inline CfgRaw sanitize(CfgRaw c) {
    const CfgRaw d;
    c.enabled = clampInt(c.enabled, 0, 1);
    c.verbose = clampInt(c.verbose, 0, 1);
    c.maxIcons = clampInt(c.maxIcons, 1, kMaxIcons);
    c.iconSize = clampInt(c.iconSize, 0, 256);
    if (c.iconSize > 0 && c.iconSize < 4) c.iconSize = 0;   // layoutRow hides the row below 4 px: treat 1..3 as auto so the HUD and the preview agree
    c.gap = clampInt(c.gap, 0, 200);
    c.spacing = clampInt(c.spacing, 0, 100);
    c.tierObvious = clampInt(c.tierObvious, 0, 127);
    c.tierOwn = clampInt(c.tierOwn, 0, 127);
    c.tierFull = clampInt(c.tierFull, 0, 127);
    c.drainAlpha = clampFloat(c.drainAlpha, 0.0f, 1.0f, d.drainAlpha);
    c.backing = clampInt(c.backing, 0, 1);
    c.backingAlpha = clampFloat(c.backingAlpha, 0.0f, 1.0f, d.backingAlpha);
    c.backingPad = clampInt(c.backingPad, 0, 32);
    c.forceTier = clampInt(c.forceTier, -1, 3);
    if (c.tierOwn < c.tierObvious) c.tierOwn = c.tierObvious;
    if (c.tierFull < c.tierOwn) c.tierFull = c.tierOwn;
    return c;
}

// ---------------------------------------------------------------- mod-config settings page (doc 39 item 4)
// The page edits one [Strip] key at a time: stepSetting moves the CfgRaw by one step (sanitized), formatIniValue/formatRowText give the INI
// text and the row text. Row order = enum order = page order (top to bottom).
enum Setting { S_Enabled, S_MaxIcons, S_IconSize, S_Gap, S_Spacing, S_Backing, S_BackingAlpha, S_DrainAlpha, S_Count };

inline const char* settingKey(int s) {
    switch (s) {
        case S_Enabled:      return "Enabled";
        case S_MaxIcons:     return "MaxIcons";
        case S_IconSize:     return "IconSize";
        case S_Gap:          return "Gap";
        case S_Spacing:      return "Spacing";
        case S_Backing:      return "Backing";
        case S_BackingAlpha: return "BackingAlpha";
        case S_DrainAlpha:   return "DrainAlpha";
        default:             return nullptr;
    }
}
inline bool settingIsToggle(int s) { return s == S_Enabled || s == S_Backing; }
inline bool settingIsFloat(int s) { return s == S_BackingAlpha || s == S_DrainAlpha; }

// Icon edge steps: 0 (auto = name label height) -> kIconFirst -> +2 ...; at or below kIconMin a down step returns to 0.
constexpr int kIconFirst = 16, kIconMin = 8, kIconStep = 2;
constexpr int kAlphaStepHundredths = 5;

inline int settingInt(const CfgRaw& c, int s) {
    switch (s) {
        case S_Enabled:  return c.enabled;
        case S_MaxIcons: return c.maxIcons;
        case S_IconSize: return c.iconSize;
        case S_Gap:      return c.gap;
        case S_Spacing:  return c.spacing;
        case S_Backing:  return c.backing;
        default:         return 0;
    }
}
inline float settingFloat(const CfgRaw& c, int s) {
    return s == S_BackingAlpha ? c.backingAlpha : (s == S_DrainAlpha ? c.drainAlpha : 0.0f);
}
inline bool sameSetting(const CfgRaw& a, const CfgRaw& b, int s) {
    return settingIsFloat(s) ? settingFloat(a, s) == settingFloat(b, s) : settingInt(a, s) == settingInt(b, s);
}

// Alpha one step (0.05) up (dir > 0) or down (dir < 0): snaps to the 0.05 grid first, works in whole hundredths so nothing drifts, clamps 0..1.
inline float stepAlpha(float v, int dir) {
    v = clampFloat(v, 0.0f, 1.0f, 0.0f);
    int q = static_cast<int>(v * 100.0f + 0.5f);
    q = ((q + kAlphaStepHundredths / 2) / kAlphaStepHundredths) * kAlphaStepHundredths;
    q += (dir > 0 ? 1 : (dir < 0 ? -1 : 0)) * kAlphaStepHundredths;
    q = clampInt(q, 0, 100);
    return static_cast<float>(q) / 100.0f;
}

inline int stepIconSize(int v, int dir) {
    if (dir > 0) return v <= 0 ? kIconFirst : v + kIconStep;
    if (dir < 0) return v <= kIconMin ? 0 : v - kIconStep;
    return v;
}

// c with setting s moved one step (dir > 0 up, dir < 0 down; a toggle flips on any non-zero dir), then sanitized. At a limit the value is unchanged.
inline CfgRaw stepSetting(CfgRaw c, int s, int dir) {
    if (dir != 0) {
        switch (s) {
            case S_Enabled:      c.enabled = c.enabled ? 0 : 1; break;
            case S_Backing:      c.backing = c.backing ? 0 : 1; break;
            case S_MaxIcons:     c.maxIcons += dir > 0 ? 1 : -1; break;
            case S_IconSize:     c.iconSize = stepIconSize(c.iconSize, dir); break;
            case S_Gap:          c.gap += dir > 0 ? 1 : -1; break;
            case S_Spacing:      c.spacing += dir > 0 ? 1 : -1; break;
            case S_BackingAlpha: c.backingAlpha = stepAlpha(c.backingAlpha, dir); break;
            case S_DrainAlpha:   c.drainAlpha = stepAlpha(c.drainAlpha, dir); break;
            default: break;
        }
    }
    return sanitize(c);
}

// ---- staged edits (0.4.3): the page edits a pending CfgRaw; Save writes the changed keys and applies them, Reset / Defaults only move the pending copy.
// Alpha rows are sliders: a cursor x over the track [trackX, trackX + trackW] maps to whole hundredths on the 0.05 grid, clamped 0..100.
inline bool settingIsSlider(int s) { return settingIsFloat(s); }
inline int sliderHundredths(int x, int trackX, int trackW) {
    if (trackW <= 0 || x <= trackX) return 0;
    if (x >= trackX + trackW) return 100;
    const long raw = (static_cast<long>(x - trackX) * 100 + trackW / 2) / trackW;
    return clampInt(static_cast<int>((raw + kAlphaStepHundredths / 2) / kAlphaStepHundredths) * kAlphaStepHundredths, 0, 100);
}
// Filled part of a slider track in px for value v (0..1, NaN -> 0).
inline int sliderFillPx(float v, int trackW) {
    if (trackW <= 0) return 0;
    v = clampFloat(v, 0.0f, 1.0f, 0.0f);
    return clampInt(static_cast<int>(v * static_cast<float>(trackW) + 0.5f), 0, trackW);
}
// c with an alpha setting moved to `hundredths` (0..100), sanitized. Other settings: unchanged.
inline CfgRaw setSliderValue(CfgRaw c, int s, int hundredths) {
    const float v = static_cast<float>(clampInt(hundredths, 0, 100)) / 100.0f;
    if (s == S_BackingAlpha) c.backingAlpha = v;
    else if (s == S_DrainAlpha) c.drainAlpha = v;
    return sanitize(c);
}
// dst with page setting s taken from src.
inline CfgRaw copySetting(CfgRaw dst, const CfgRaw& src, int s) {
    switch (s) {
        case S_Enabled:      dst.enabled = src.enabled; break;
        case S_MaxIcons:     dst.maxIcons = src.maxIcons; break;
        case S_IconSize:     dst.iconSize = src.iconSize; break;
        case S_Gap:          dst.gap = src.gap; break;
        case S_Spacing:      dst.spacing = src.spacing; break;
        case S_Backing:      dst.backing = src.backing; break;
        case S_BackingAlpha: dst.backingAlpha = src.backingAlpha; break;
        case S_DrainAlpha:   dst.drainAlpha = src.drainAlpha; break;
        default: break;
    }
    return dst;
}
// True when any page setting differs (Verbose, tiers, BackingPad and ForceTier are INI-only and never count).
inline bool pageDiffers(const CfgRaw& a, const CfgRaw& b) {
    for (int s = 0; s < S_Count; ++s) if (!sameSetting(a, b, s)) return true;
    return false;
}
// `keep` with every page setting at its default (INI-only fields stay as they are).
inline CfgRaw pageDefaults(const CfgRaw& keep) {
    const CfgRaw d;
    CfgRaw r = keep;
    for (int s = 0; s < S_Count; ++s) r = copySetting(r, d, s);
    return r;
}

// The text written to the INI for setting s: integers "%d", alphas "%.2f". cap < 1 or a null out is a no-op; an unknown s writes "".
inline void formatIniValue(int s, const CfgRaw& c, char* out, int cap) {
    if (!out || cap < 1) return;
    out[0] = '\0';
    if (s < 0 || s >= S_Count) return;
    if (settingIsFloat(s)) snprintf(out, static_cast<size_t>(cap), "%.2f", static_cast<double>(settingFloat(c, s)));
    else snprintf(out, static_cast<size_t>(cap), "%d", settingInt(c, s));
}

// The row text on the page: "Gap: 4", "IconSize: Auto", "Enabled: On", "DrainAlpha: 0.65"; needsRestart appends " (restart)".
inline void formatRowText(int s, const CfgRaw& c, bool needsRestart, char* out, int cap) {
    if (!out || cap < 1) return;
    out[0] = '\0';
    if (s < 0 || s >= S_Count) return;
    char val[32];
    if (settingIsToggle(s)) snprintf(val, sizeof val, "%s", settingInt(c, s) ? "On" : "Off");
    else if (s == S_IconSize && c.iconSize == 0) snprintf(val, sizeof val, "Auto");
    else formatIniValue(s, c, val, static_cast<int>(sizeof val));
    snprintf(out, static_cast<size_t>(cap), "%s: %s%s", settingKey(s), val, needsRestart ? " (restart)" : "");
}

}  // namespace esl
