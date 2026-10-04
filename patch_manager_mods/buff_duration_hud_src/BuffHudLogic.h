// Buff Duration HUD -- pure helpers (host-testable; only libc headers, no Win32 types). Everything below the appendf block is a
// behaviour-identical lift of the expression that used to sit inline in BuffDurationHUD.cpp (constants, rounding, < vs <=,
// float vs int arithmetic copied verbatim); the .cpp keeps every engine read, draw call and log line. Tested by
// buff_hud_logic_test.cpp (doc 20 T1).
#pragma once
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

namespace bh {

// ---------------------------------------------------------------- address-table gates (K2Addr.h, 0 = unknown)
// Templates over K2AddrTable so this header stays free of K2Addr.h. A feature whose gate is false does nothing (null table = unknown exe).
// arrow counters (ForceArrowCounters): slot gate word, the two counters, the three overlay words.
template <class T> inline bool arrowFieldsOk(const T* t) {
    return t && t->Slot_Flags && t->Slot_ArrowBuff && t->Slot_ArrowDebuff && t->Slot_Overlay0 && t->Slot_Overlay1 && t->Slot_Overlay2;
}
// hide / restore one native arrow (hooks 2/3): icon label -> border sub-object, its vtable check, its size fields.
template <class T> inline bool arrowHideFieldsOk(const T* t) {
    return t && t->Label_Border && t->BorderVtable && t->Border_Width && t->Border_Height;
}
// icon column drawer: border tint / alpha / rotation of the native arrow widget and the image Draw vtable slot.
template <class T> inline bool columnFieldsOk(const T* t) {
    return t && t->Label_Border && t->Border_Rot && t->Border_Alpha && t->Border_Tint && t->ImageVt_Draw;
}
// menu companion portraits: control flags + rect (x, y, w, h).
template <class T> inline bool menuFieldsOk(const T* t) {
    return t && t->Ctl_Flags && t->Ctl_X && t->Ctl_Y && t->Ctl_W && t->Ctl_H;
}
// optional ICONCACHE diagnostic: skipped (never gates the feature) unless all four are known.
template <class T> inline bool iconCacheFieldsOk(const T* t) {
    return t && t->Creature_IconCacheData && t->Creature_IconCacheCount && t->IconEnt_F14 && t->IconEnt_F1c;
}

// Bounded printf-append. Replaces the `len += snprintf(buf + len, cap - len, ...)` idiom, which overruns once a write
// truncates: snprintf returns the length it WOULD have written, so len passes cap and `cap - len` underflows to a huge size_t.
// Appends at buf[len], never writes past buf[cap-1], always leaves buf NUL-terminated, and returns the new length,
// which is clamped to cap-1 (so repeated calls on a full buffer are no-ops). cap <= 0: nothing is touched, len is returned.
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

// ---------------------------------------------------------------- engine arrow counters (ForceArrowCounters)
// The engine only runs the portrait row draw when slot+0xddc (buff) / +0xddd (debuff) is non-zero; FUN_00744f00 fills them from
// an icon cache, not from the effect list we track, so a creature can hold tracked effects at counter 0 (doc 02).

// slot+0x590 bit 1 = the engine's own "row visible" bit; also excludes the creature-less mirror slot.
inline bool arrowGateOpen(int gate) { return (gate & 2) != 0; }

// The three overlay words (+0x1b8 / +0x300 / +0x448; bit 1 = shown). Any set bit = the engine deliberately hid the arrows
// (dead/disabled) and the counters must be left alone.
inline bool arrowsHidden(int o0, int o1, int o2) { return ((o0 | o1 | o2) & 2) != 0; }

struct ArrowCounters { int buff, debuff; };
// raw = the engine's counters (unsigned char values 0..255), n* = tracked effects of that kind. Raise a zero counter to 1 when we
// track something, never lower an engine value; hidden (the caller then also skips collecting, so n* = 0) leaves both as is.
inline ArrowCounters bumpArrowCounters(int rawBuff, int rawDebuff, int nBuff, int nDebuff, bool hidden) {
    if (hidden) { nBuff = 0; nDebuff = 0; }
    ArrowCounters o = { rawBuff, rawDebuff };
    if (nBuff > 0 && rawBuff == 0) o.buff = 1;
    if (nDebuff > 0 && rawDebuff == 0) o.debuff = 1;
    return o;
}

// Per-slot last-seen (raw, tracked) tuple: the ARROWS / ICONCACHE lines are logged only when it changes.
struct ArrowRaw { int slot; unsigned char buff, debuff, nBuff, nDebuff; };

// Existing entry for slot, else the first free one (slot == 0) claimed and zeroed, else nullptr (table full: no change log).
inline ArrowRaw* arrowRawFor(ArrowRaw* tbl, int count, int slotPtr) {
    for (int i = 0; i < count; ++i) if (tbl[i].slot == slotPtr) return &tbl[i];
    for (int i = 0; i < count; ++i) if (tbl[i].slot == 0) { tbl[i] = {}; tbl[i].slot = slotPtr; return &tbl[i]; }
    return nullptr;
}

// Store the new tuple (counts truncated to unsigned char) and report whether it differs from the stored one.
inline bool arrowRawUpdate(ArrowRaw* raw, int rawBuff, int rawDebuff, int nBuff, int nDebuff) {
    unsigned char sig[4] = { (unsigned char)rawBuff, (unsigned char)rawDebuff, (unsigned char)nBuff, (unsigned char)nDebuff };
    bool changed = memcmp(sig, &raw->buff, 4) != 0;
    raw->buff = (unsigned char)rawBuff; raw->debuff = (unsigned char)rawDebuff;
    raw->nBuff = (unsigned char)nBuff; raw->nDebuff = (unsigned char)nDebuff;
    return changed;
}

// Hook 2 (per native arrow): the first `tracked` arrows of the row are hidden, the rest (untracked engine buffs) are kept.
inline bool arrowSuppressed(int index, int tracked) { return index < tracked; }

// ---------------------------------------------------------------- icon column geometry (drawColumnCore)
constexpr float SmallCardFraction = 0.19f, LargeCardFraction = 0.22f; // preferred icon size / card width
constexpr float MinIconSize = 5.0f, MaxIconSize = 24.0f;  // x HUD scale (=10 / 48 px at scale 2)
constexpr int MaxColumns = 3;
constexpr int RightInset = 0;                 // debuff column: extra px in from the card's right edge

struct ColumnLayout { int gap, size, cols, perCol, fit, drawN; };

// n = effects in this column, nOther = effects in the opposite column (both columns share one icon size), scale = HUD scale.
// Preferred size (smaller on the little companion cards so the face stays visible), then shrink so all of nSize fit down the
// column; below MinIconSize add another column (up to MaxColumns) instead. drawN = how many of the n icons fit.
inline ColumnLayout columnLayout(int n, int nOther, int isLarge, int cardW, int cardH, float scale) {
    int nSize = n > nOther ? n : nOther;
    int gap = int(scale + 0.5f); if (gap < 1) gap = 1;
    int size = int(cardW * (isLarge ? LargeCardFraction : SmallCardFraction));
    int maxSize = int(MaxIconSize * scale), minSize = int(MinIconSize * scale);
    if (size > maxSize) size = maxSize;
    if (size < minSize) size = minSize;
    int cols = 1;
    for (;; ++cols) {
        int perCol = (nSize + cols - 1) / cols; if (perCol < 1) perCol = 1;
        int fitSize = (cardH - gap) / perCol - gap;
        if (fitSize < size) size = fitSize > 0 ? fitSize : 1;
        if (size >= minSize || cols >= MaxColumns) break;
        size = int(cardW * (isLarge ? LargeCardFraction : SmallCardFraction));
        if (size > maxSize) size = maxSize;
    }
    int perCol = (cardH - gap) / (size + gap); if (perCol < 1) perCol = 1;
    int fit = perCol * cols;
    int drawN = n < fit ? n : fit;
    return { gap, size, cols, perCol, fit, drawN };
}

struct IconPos { int x, y; };
// Top-left of icon i inside the card: buffs run down the left edge, debuffs down the right edge; extra columns step inwards.
inline IconPos iconPos(const ColumnLayout& l, int i, bool debuff, int cardW) {
    int colIdx = i / l.perCol;
    int ix = debuff ? cardW - l.gap - l.size - colIdx * (l.size + l.gap) - RightInset : l.gap + colIdx * (l.size + l.gap);
    int iy = l.gap + (i % l.perCol) * (l.size + l.gap);
    return { ix, iy };
}

// Height of the bright (remaining) bottom slice of an icon of `size` px; ratio = remaining fraction. Clamped to size from above only:
// a negative ratio gives a negative height (the caller draws the slice only when > 0).
inline int brightPx(int size, float ratio) {
    int bright = int(size * ratio + 0.5f);
    if (bright > size) bright = size;
    return bright;
}

// Item-ability icons sit on a round backing disc: the 1px boundary line at y0 (top of the bright slice) is clipped to the chord of
// the disc there. lw = 0 when the line is outside the circle.
struct DiscLine { int x, w; };
inline DiscLine discLine(int ix, int iy, int size, int y0) {
    float r = size * 0.5f - 0.5f, dy = (y0 + 0.5f) - (iy + size * 0.5f);
    float chord2 = r * r - dy * dy;
    int lw = chord2 > 0.0f ? int(2.0f * sqrtf(chord2) + 0.5f) : 0;
    int lx = ix + (size - lw) / 2;
    return { lx, lw };
}

// Change-detect signature for the COLUMN log line (log only; fields overlap, so distinct inputs can collide).
inline int columnSig(int n, int debuff, int nOther, int size, int cardW, int cardH, int isLarge) {
    return (n << 16) ^ (debuff << 4) ^ (nOther << 12) ^ (size << 8) ^ (cardW << 20) ^ cardH ^ isLarge;
}

// Per-(slot, debuff) change gate for the COLUMN log line. One slot draws its buff and DEBUFF columns alternately and the sig includes
// debuff, so a per-slot "last sig" flips every call; keying by the pair keeps each column's own last sig. Zero-init = empty.
// Logs on first sight of a key (if the table has room) and when a known key's sig changes; a full table never logs unknown keys.
struct ColumnLogGate {
    int slot[16]; int debuff[16]; int sig[16]; int n;
    bool shouldLog(int slotPtr, int debuffFlag, int newSig) {
        for (int k = 0; k < n; ++k) {
            if (slot[k] == slotPtr && debuff[k] == debuffFlag) {
                if (sig[k] == newSig) return false;
                sig[k] = newSig;
                return true;
            }
        }
        if (n >= 16) return false;
        slot[n] = slotPtr; debuff[n] = debuffFlag; sig[n] = newSig; ++n;
        return true;
    }
};

// ---------------------------------------------------------------- menu companion portraits (onMenuOverlay)
// Control flag word at +0x48: bit 1 = visible.
inline bool menuPortraitVisible(int flags) { return (flags & 2) != 0; }
// Control rect (x, y, w, h) -> usable? Reject empty/negative and absurd (> 2000 px) sizes before using w/h as the card size.
inline bool menuRectOk(int w, int h) { return !(w < 1 || h < 1 || w > 2000 || h > 2000); }

}  // namespace bh
