// Shield Bar -- pure helpers (host-testable; only libc headers, no Win32 types). See shield_logic_test.cpp.
#pragma once
#include <stdio.h>
#include <stdarg.h>

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

// Bounded printf-append: a copy of bh::appendf (buff_duration_hud_src/BuffHudLogic.h, audit 20a F7, abc01d8). The two patches build
// from separate folders and cannot include each other's headers; shield_logic_test.cpp checks the copies stay identical.
// Appends at buf[len], never writes past buf[cap-1], always NUL-terminated, returns the new length clamped to cap-1.
inline int appendf(char* buf, int cap, int len, const char* fmt, ...) {
    if (cap <= 0) return len;
    if (len < 0) len = 0;
    if (len >= cap - 1) { buf[cap - 1] = 0; return cap - 1; }
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf + len, (size_t)(cap - len), fmt, ap);
    va_end(ap);
    if (n < 0) { buf[len] = 0; return len; }   // encoding error: keep what we had, stay terminated
    return (n >= cap - len) ? cap - 1 : len + n;
}

}  // namespace sb
