// Shield Bar -- pure helpers (host-testable; only libc headers, no Win32 types). See shield_logic_test.cpp.
#pragma once
#include <stdio.h>
#include <stdarg.h>
#include "StrFmt.h"

namespace sb {

// ---------------------------------------------------------------- address-table gates (K2Addr.h, 0 = unknown)
// Templates over K2AddrTable so this header stays free of K2Addr.h. A feature whose gate is false does nothing (null table = unknown exe).
// HUD slot geometry (SLOT phase leader force-bar slide + log, HUD shield bars): flags, health / force controls + rects, card rect, SetRect slot.
template <class T> inline bool slotFieldsOk(const T* t) {
    return t && t->Slot_CardRect && t->Slot_Flags && t->Slot_VitCtl && t->Slot_VitRect && t->Slot_ForceCtl && t->Slot_ForceRect && t->Ctl_VtSetRect;
}
// the shield bars only need the health-bar rect of a slot (HUD frame, menu top bar)
template <class T> inline bool vitRectOk(const T* t) { return t && t->Slot_VitRect; }
// menu companion portraits: LBL_CHARn control + the whole 4-control groups that get spread apart, the control rect / flags / SetRect slot.
template <class T> inline bool menuFieldsOk(const T* t) {
    return t && t->Ctl_Flags && t->Ctl_X && t->Ctl_Y && t->Ctl_W && t->Ctl_H && t->Ctl_VtSetRect &&
           t->Menu_Char2 && t->Menu_Level2 && t->Menu_Back2 && t->Menu_Change2 &&
           t->Menu_Char3 && t->Menu_Level3 && t->Menu_Back3 && t->Menu_Change3;
}
// companion vitals (readVitals): creature vtable HP getters, stats object + the two force shorts, GetMaxForcePoints.
template <class T> inline bool vitalsFieldsOk(const T* t) {
    return t && t->Creature_Stats && t->Stats_ForceA && t->Stats_ForceB && t->Creature_VtMaxHp && t->Creature_VtCurHp && t->GetMaxForcePoints;
}
// image Draw vtable slot (every bar / gauge draw)
template <class T> inline bool drawFieldsOk(const T* t) { return t && t->ImageVt_Draw; }
// diagnostic effect probe (shieldprobe.txt only): the CGameEffect field offsets and the creature's effect list.
template <class T> inline bool effectProbeFieldsOk(const T* t) {
    return t && t->Creature_EffectArray && t->Creature_EffectCount && t->Effect_Type && t->Effect_Subtype && t->Effect_SpellId &&
           t->Effect_Creator && t->Effect_Duration && t->Effect_IntCount && t->Effect_Ints;
}
// tooltip line: the char button's slot + text fields and the engine's sprintf-into-CExoString.
template <class T> inline bool tooltipFieldsOk(const T* t) {
    return t && t->Tip_Slot && t->Tip_Text && t->CExoStringFormat;
}

// ---------------------------------------------------------------- menu companion bar geometry
// Bar art (uibit_bar_vp_p / uibit_bar_fp_p, 128x256 override TGAs, alpha > 50%): health columns 58..97 (col 57 half), force 30..69
// (col 29 half); rows 4..251 for both on the portrait side. Edges below are the half-alpha boundaries, in art units.
constexpr float kHealthArtLeft = 57.5f, kHealthArtRight = 98.0f, kForceArtLeft = 29.5f, kForceArtRight = 70.0f;
constexpr float kArtTop = 4.0f, kArtBottom = 252.0f, kArtW = 128.0f, kArtH = 256.0f;

// Health bar left of a companion card (x, y, w, h), force bar right of it, both drawn rects. The visible art meets the card frame on
// both sides and its top / bottom rows line up with the card's top / bottom edge (user 10-04: "flush with frame").
struct MenuBars { int vx, fx, by, bw, bwF, bh; };
inline int ceilPos(float f) { int i = int(f); return (f > float(i)) ? i + 1 : i; }   // f >= 0
inline MenuBars menuBars(int x, int y, int w, int h) {
    MenuBars b;
    const float visH = (kArtBottom - kArtTop) / kArtH;                 // visible fraction of the drawn height
    b.bh = ceilPos(h / visH);                                          // visible art at least as tall as the card
    b.by = y - int(kArtTop / kArtH * b.bh + (visH * b.bh - h) / 2.0f + 0.5f);   // overhang (< 1 px) split evenly top / bottom
    b.bw = int(47.0f * (h + 4) / 96.0f + 0.5f);
    b.bwF = int(b.bw * 1.2f + 0.5f);                                   // force bar 20% wider (read skinny next to health)
    b.vx = x - int(kHealthArtRight / kArtW * b.bw);                    // visible right edge at or just inside the card's left edge
    b.fx = x + w - ceilPos(kForceArtLeft / kArtW * b.bwF);             // visible left edge at or just inside the card's right edge
    return b;
}
inline float healthVisLeft(int vx, int bw) { return vx + kHealthArtLeft / kArtW * bw; }
inline float forceVisRight(int fx, int bwF) { return fx + kForceArtRight / kArtW * bwF; }

// x for the second companion card so the companion-to-companion gap equals the gap between the leader's force bar
// (lfx, lfw: drawn rect) and the first companion's health bar. c1x: first companion card x; w, h: companion card size.
inline int secondCompanionX(int lfx, int lfw, int c1x, int y, int w, int h) {
    MenuBars b = menuBars(c1x, y, w, h);
    float gap = healthVisLeft(b.vx, b.bw) - forceVisRight(lfx, lfw);
    float c2VisLeft = forceVisRight(b.fx, b.bwF) + gap;
    return int(c2VisLeft - (healthVisLeft(b.vx, b.bw) - c1x) + 0.5f);   // same card-to-visible-left offset on every card
}

// Leader card (LBL_CHAR1) bottom from the leader vit rect (vy, vh): top_p.gui PB_VIT1 y476 h76 (bottom 552), LBL_CHAR1 y479 h70
// (bottom 549), so the card ends 3/76 of the vit height above the vit bottom. 3840x1600: vit (y 1269, h 202) -> 1463.
inline int leaderCardBottom(int vy, int vh) { return vy + vh - int(vh * 3.0f / 76.0f + 0.5f); }
// y nudge that puts a companion card's bottom (compY + compH) on the leader card bottom (user 10-04 option b: 1461 -> 1463).
// Only a small nudge: |d| > 8 returns 0, so a bad read never moves the cards far.
inline int companionDy(int leaderBottom, int compY, int compH) {
    int d = leaderBottom - (compY + compH);
    return (d >= -8 && d <= 8) ? d : 0;
}

// Bounded printf-append: the shared strfmt::appendf (StrFmt.h), re-exported so sb::appendf call sites and tests are unchanged.
using strfmt::appendf;

// ---------------------------------------------------------------- hover tooltip (C094)
// Shield kind from the pool's damage-coverage mask (ints[0], DAMAGE_TYPE_* bits), the generator's rule (scripts/gen_shield_table.py):
// ion/sonic coverage = "Enviro" (so Sith/Arkanian/Verpine energy shields, which also cover ion+sonic, read "Enviro"; the bar colour
// can differ because shieldColor tries the vfx row first), physical-only coverage (bits 1|2|4, no blaster 4096) = "Melee", else "Energy".
inline const char* shieldKind(int damageFlags) {
    if (damageFlags & (2048 | 1024)) return "Enviro";
    if ((damageFlags & 7) && !(damageFlags & 4096)) return "Melee";
    return "Energy";
}
// Append one pool as "32/40 Energy 1m05s" (time omitted when secsLeft < 0 = unknown / permanent). Returns the new length.
inline int appendPoolText(char* out, int cap, int len, int remaining, int max, int damageFlags, float secsLeft) {
    len = appendf(out, cap, len, "%d/%d %s", remaining, max, shieldKind(damageFlags));
    if (secsLeft >= 0.0f) {
        int s = int(secsLeft + 0.5f);
        len = s >= 60 ? appendf(out, cap, len, " %dm%02ds", s / 60, s % 60) : appendf(out, cap, len, " %ds", s);
    }
    return len;
}

}  // namespace sb
