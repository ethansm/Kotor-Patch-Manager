// GuiTip.h -- hover tooltips for GuiKit controls (Steam Aspyr swkotor2.exe). Header-only, no STL; includes GuiKit.h.
//
// HYBRID, self-verifying (RE spike research/companion_levelup/phase7_evidence/UX_native_tooltip.md, verdict NATIVE+PROBE):
//   1. NATIVE: the engine's base CSWGuiControl::DisplayToolTip (label vtable slot 36) shows the CExoString at ctl+0x2C when the int strref at
//      ctl+0x28 is 0. setNative() writes those two fields once per control; the engine draws the box next to the cursor after its own delay.
//      The engine box auto-sizes and does NOT word-wrap: tip strings need explicit '\n'.
//   2. CUSTOM fallback: a mod-drawn box (two GuiKit labels) shown only when the native box never appeared. tick() is a pure state machine: it
//      shows the custom box after (engine appear time + FallbackGraceMs) of steady hover, and never again once the engine has been seen to own
//      the hovered control (mgr owner == control; sticky), so the two never double up.
//   3. PROBE: readMgr()/probeChanged() read the engine tooltip manager by saferead only (no engine calls) so the mod can log TIPPROBE lines and
//      the first live run tells which path works. Nothing here has been observed live for a GuiKit control yet.
//
// Steam-only: the control offsets and the manager layout below come from the Steam decomp (spike sections 1-2). Every engine-touching function
// returns false on any other exe table (k2addr::get() != &kSteam) so a GOG build does nothing rather than write guessed offsets. The manager
// pointer cell is the table's GuiSoundMgr (GameAddr.h names the same object): guikit::g_gk.guiSoundMgrAddr, never a raw address here.
// Host tests substitute g_gk (see _shared_src/guitip_test.cpp).
#pragma once
#include "GuiKit.h"
#include <stdint.h>

namespace guitip {

// ---- control fields (CSWGuiControl, spike section 1.3) ----
constexpr int Ctl_TipStrRef = 0x28;          // int TLK strref; must be 0 for the inline string to be used
constexpr int Ctl_TipText = 0x2C;            // inline CExoString {char*, uint}, always a constructed (possibly empty) string
constexpr uint32_t CtlFlag_TipShown = 0x40;  // set by DisplayToolTip when it showed the box for this control

// ---- tooltip manager fields (CSWGuiManager, spike section 1) ----
constexpr int Mgr_Over = 0x08;               // control under the mouse
constexpr int Mgr_Flags = 0x1C;              // bit3 = the tooltip panel is drawn
constexpr int Mgr_Owner = 0x40;              // control whose tooltip is up
constexpr int Mgr_Timer = 0x44;              // float seconds since the mouse settled, < 0 = already shown this hover
constexpr int Mgr_Appear = 0x48;             // float seconds until the tooltip appears
constexpr int Mgr_Disabled = 0x4C;           // non-zero = tooltips off
constexpr uint32_t MgrFlag_TipDrawn = 8;

constexpr uint32_t DefaultDelayMs = 400;     // used when the engine's appear time is unreadable / implausible
constexpr uint32_t FallbackGraceMs = 250;    // extra wait after the engine delay before the custom box may appear

// True when the active exe table is Steam (the only one the offsets above are decoded for).
inline bool nativeSupported() { return k2addr::get() == &kSteam; }

// ---- 1. native text on a control ----
// strref := 0 and the CExoString at ctl+0x2C := text (dtor of the old value, then ctor). Call outside engine callbacks (setup / UI tick only).
inline bool setNative(uintptr_t ctl, const char* text) {
    uint32_t probe = 0;
    if (!ctl || !text || !nativeSupported() || !guikit::g_gk.exoCtor || !guikit::g_gk.exoDtor) return false;
    if (!guikit::detail::rd32(ctl, Ctl_TipText, &probe) || !guikit::detail::wr<int>(ctl, Ctl_TipStrRef, 0)) return false;
    guikit::g_gk.exoDtor(reinterpret_cast<void*>(ctl + Ctl_TipText));
    guikit::g_gk.exoCtor(reinterpret_cast<void*>(ctl + Ctl_TipText), text);
    return true;
}
inline bool clearNative(uintptr_t ctl) { return setNative(ctl, ""); }

// ---- 2. manager snapshot (saferead only) ----
struct Mgr { bool ok = false; uintptr_t over = 0, owner = 0; float timer = 0.f, appear = 0.f; uint32_t flags = 0, disabled = 0; };
inline bool readMgr(Mgr* out) {
    Mgr m;
    uint32_t mgr = 0, over = 0, owner = 0;
    if (out) *out = m;
    if (!out || !nativeSupported() || !guikit::g_gk.guiSoundMgrAddr || !guikit::detail::rd32(guikit::g_gk.guiSoundMgrAddr, 0, &mgr) || !mgr) return false;
    if (!guikit::detail::rd32(mgr, Mgr_Over, &over) || !guikit::detail::rd32(mgr, Mgr_Owner, &owner) || !guikit::detail::rd<float>(mgr, Mgr_Timer, &m.timer) ||
        !guikit::detail::rd<float>(mgr, Mgr_Appear, &m.appear) || !guikit::detail::rd32(mgr, Mgr_Flags, &m.flags) || !guikit::detail::rd32(mgr, Mgr_Disabled, &m.disabled)) return false;
    m.over = over; m.owner = owner; m.ok = true;
    *out = m;
    return true;
}
// Engine tooltip delay in ms: the appear time when readable and plausible (0 < s < 10), else DefaultDelayMs.
inline uint32_t delayMs(const Mgr& m) {
    if (m.ok && m.appear > 0.f && m.appear < 10.f) return static_cast<uint32_t>(m.appear * 1000.f + 0.5f);
    return DefaultDelayMs;
}

// ---- 3. fallback state machine (pure) ----
struct State { int key = -1; uint32_t since = 0; bool boxShown = false; bool nativeSeen = false; };
inline void reset(State& s) { s.since = 0; s.key = -1; s.boxShown = false; }   // click / page close: hide, restart the hover clock (nativeSeen stays)
// hoveredKey: caller's id of the control under the cursor that has a tip (-1 = none). nativeOwnsHovered: the engine's tooltip owner is that
// control. Returns whether the CUSTOM box should be shown for s.key.
inline bool tick(State& s, int hoveredKey, bool nativeOwnsHovered, uint32_t nowMs, uint32_t delay) {
    if (hoveredKey != s.key) { s.key = hoveredKey; s.since = nowMs; s.boxShown = false; return false; }
    if (s.key < 0) { s.boxShown = false; return false; }
    if (nativeOwnsHovered) s.nativeSeen = true;
    if (nativeOwnsHovered || s.nativeSeen) { s.boxShown = false; return false; }
    s.boxShown = static_cast<uint32_t>(nowMs - s.since) >= delay + FallbackGraceMs;
    return s.boxShown;
}

// ---- 4. custom box layout (pure; any unit system as long as all inputs share it) ----
struct Rect { int x = 0, y = 0, w = 0, h = 0; };
struct Metrics { int charW, lineH, padX, padY; };
struct Region { int x0, y0, x1, y1; };
// Lines and longest line (chars) of a '\n'-separated text; empty text = 0 lines.
inline void measure(const char* t, int* lines, int* maxLen) {
    int n = 0, cur = 0, mx = 0;
    if (t && *t) {
        n = 1;
        for (; *t; ++t) { if (*t == '\n') { ++n; cur = 0; } else if (++cur > mx) mx = cur; }
    }
    if (lines) *lines = n;
    if (maxLen) *maxLen = mx;
}
// Box below the control (gap units under it), flipped above when it would pass reg.y1, then clamped inside the region
// (x: right edge first, then left; y: bottom first, then top, so the top wins in a region too small for the box).
inline Rect layout(int lines, int maxLen, const Metrics& m, const Rect& ctl, const Region& reg, int gap) {
    Rect r;
    r.w = maxLen * m.charW + 2 * m.padX;
    r.h = lines * m.lineH + 2 * m.padY;
    r.x = ctl.x;
    r.y = ctl.y + ctl.h + gap;
    if (r.y + r.h > reg.y1) r.y = ctl.y - r.h - gap;
    if (r.x + r.w > reg.x1) r.x = reg.x1 - r.w;
    if (r.x < reg.x0) r.x = reg.x0;
    if (r.y + r.h > reg.y1) r.y = reg.y1 - r.h;
    if (r.y < reg.y0) r.y = reg.y0;
    return r;
}

// ---- 5. probe change detector (pure) ----
struct ProbeSnap { int over = -1, owner = -1, shown = -1, ctl40 = -1; };
// True (and `last` updated) when any of {mgr.over == ourCtl, mgr.owner == ourCtl, mgr flags bit3, ctl flags bit 0x40} differs from `last`.
inline bool probeChanged(ProbeSnap& last, const Mgr& m, uintptr_t ourCtl, int ctlFlags) {
    ProbeSnap n;
    n.over = m.ok && m.over == ourCtl;
    n.owner = m.ok && m.owner == ourCtl;
    n.shown = m.ok && (m.flags & MgrFlag_TipDrawn) != 0;
    n.ctl40 = (static_cast<uint32_t>(ctlFlags) & CtlFlag_TipShown) != 0;
    if (n.over == last.over && n.owner == last.owner && n.shown == last.shown && n.ctl40 == last.ctl40) return false;
    last = n;
    return true;
}

} // namespace guitip
