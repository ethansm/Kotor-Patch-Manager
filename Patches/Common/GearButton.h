// GearButton.h -- the shared settings-gear button: geometry + art, one look on every page (MCF8, doc 46).
// Origin: level_up_override_src/LevelUpUi.h (uiIconRect / uiGearRect / kGearInset / kHiTint); the pixels are _shared_src/GearIconTex.inc
// (e3d_gear, e3d_gear_hi, e3d_fill + two slider textures), byte-identical to LuoTextures.inc's luo_gear / luo_gear_hi / luo_fill.
//
// Usage: set embtex::g_log, call gearbtn::registerArt() once while building (idempotent, Steam only), embtex::heal() every UI tick, then give the
// frame control gearbtn::iconRect() and the glyph control gearbtn::gearRect(); gearbtn::artName(hover) / gearbtn::fillName() are the resrefs.
#pragma once
#include "EmbedTex.h"
#include <math.h>

namespace gearbtn {

namespace detail {
#include "GearIconTex.inc"   // kGearTex, kGearHiTex, kFillTex, kTrackTex, kThumbTex, kEditTex[] (5), kEditTexCount
}

constexpr float kInset = 0.18f;   // glyph rect = icon rect inset by round(side * kInset) per side (the button frame shows around it)
constexpr float kHiTint[3] = {0.8f, 0.8f, 0.69804f};   // vanilla BTN_LEVELUP HILIGHT COLOR: the hover tint

struct Rect { int x, y, w, h; };

// Icon (frame) rect in engine px: a square of `side` px with its right edge at `right` and its top at `top`.
inline Rect iconRect(int right, int top, int side) { return Rect{right - side, top, side, side}; }
// Side in px for a layout-unit side on a screen whose unit-per-px scale along Y is sy (the icon stays square on a wide screen).
inline int sidePx(int sideUnits, float sy) { return static_cast<int>(lroundf(sideUnits * sy)); }
// Glyph rect: the icon rect inset by round(side * kInset) per side (still square).
inline Rect gearRect(const Rect& icon) {
    const int in = static_cast<int>(lroundf(icon.w * kInset));
    return Rect{icon.x + in, icon.y + in, icon.w - 2 * in, icon.h - 2 * in};
}

inline const char* artName(bool hover) { return detail::kEditTex[hover ? 1 : 0].resref; }
inline const char* fillName() { return detail::kEditTex[2].resref; }
// Hover state swaps the glyph to the _hi texture; the frame fill stays.
inline bool hoverFill(bool hovered) { return hovered; }

inline const embtex::Tex* textures() { return detail::kEditTex; }
inline int textureCount() { return static_cast<int>(detail::kEditTexCount); }
// Create + fill + queue the textures (EmbedTex ensure(): idempotent, Steam-only; false on another exe or when an engine call fails).
inline bool registerArt() { return embtex::ensure(detail::kEditTex, static_cast<int>(detail::kEditTexCount)); }

}  // namespace gearbtn
