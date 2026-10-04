// Force Power Hotbar -- item target selection (pure C++17, no Win32/engine types so it can be unit-tested on the host).
// The DLL fills one Candidate per party position (HP, "readable", "down", "can this item be used on them"); this header decides.
#pragma once
#include "HotbarConfig.h"
#include <algorithm>
#include <cstdint>

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

// ======== pure units extracted from ForcePowerHotbar.cpp (behaviour-neutral; tested in hotbar_logic_test.cpp) ========
// (this header is included by the DLL after windows.h: only <cstdint>/<algorithm>/<string>/<vector> here, no Win32 types)

// HUD entry key -> stable ref: bit 0x40000000 = item (identity = normalised name), 0x20000000 = spell row, else other.
inline ActionRef decodeKey(int key, const std::string& name) {
    ActionRef r;
    if (key & 0x40000000) { r.kind = K_Item; r.name = normalizeItemName(name); if (r.name.empty()) r.kind = K_None; }
    else if (key & 0x20000000) { r.kind = K_Spell; r.id = key & 0x1FFFFFFF; }
    else { r.kind = K_Other; r.id = key; }
    return r;
}

// Picker tab of an entry key: 0 powers, 1 items, 2 modes.
inline int tabOfKey(int key) { return (key & 0x40000000) ? 1 : ((key & 0x20000000) ? 0 : 2); }

// Character key from the controlled creature's object id: "obj%08x"; "" = unknown. *invalid = id is 0 / 0xFFFFFFFF (transient during loads).
// A one-member party with id 0xFFFFFFFF is the Exile (valid id fffffffd), which the engine can report as 0xFFFFFFFF for a whole session.
inline std::string characterKeyFor(unsigned oid, int partyCount, bool* invalid = nullptr) {
    bool bad = (oid == 0xFFFFFFFFu || oid == 0);
    if (invalid) *invalid = bad;
    if (bad) return (oid == 0xFFFFFFFFu && partyCount == 1) ? "objfffffffd" : "";
    char b[24]; snprintf(b, sizeof b, "obj%08x", oid);
    return b;
}

// AutoFill: true for a spell key that is a known Force power (not an item, not a mode toggle).
inline bool isKnownPowerKey(int key) { return !((key & 0x40000000) || !(key & 0x20000000)); }

// True when v already holds an action Same() as r.
inline bool hasSame(const std::vector<ActionRef>& v, const ActionRef& r) { for (const ActionRef& x : v) if (Same(x, r)) return true; return false; }

// Index of the entry whose key == selKey, else 0 (the group's first entry); -1 for an empty list.
inline int pickIndexByKey(const int* keys, int n, unsigned selKey) {
    if (n <= 0) return -1;
    for (int i = 0; i < n; ++i) if ((unsigned)keys[i] == selKey) return i;
    return 0;
}

// AutoFill merge: each slot keeps its configured action; an empty, non-blank slot takes the next auto candidate. Returns how many autos were used.
inline int assignAutoFill(const ActionRef* cfg, const std::vector<ActionRef>& autos, ActionRef* eff, bool* isAuto) {
    size_t next = 0; int nAuto = 0;
    for (int s = 0; s < Slots; ++s) {
        ActionRef a = cfg[s]; bool au = false;
        if (a.empty() && !a.blank && next < autos.size()) { a = autos[next++]; au = true; ++nAuto; }   // explicit "none" stays empty
        eff[s] = a; isAuto[s] = au;
    }
    return nAuto;
}

// FNV-1a style signature of the effective bar (ref text + id + auto flag); the DLL logs the bar only when it changes.
inline unsigned effSignature(const ActionRef* eff, const bool* isAuto) {
    unsigned h = 2166136261u;
    for (int s = 0; s < Slots; ++s) {
        std::string f = Format(eff[s]);
        h = (h ^ (unsigned)f.size() ^ ((unsigned)eff[s].id << 8) ^ (isAuto[s] ? 0x5000000u : 0)) * 16777619u;
        for (char c : f) h = (h ^ (unsigned char)c) * 16777619u;
    }
    return h;
}

// Greedy word wrap into '\n'-joined lines (at most maxLines, "..." when cut); a word longer than width stays whole.
inline std::string wrapText(const std::string& in, int width, int maxLines) {
    std::string out, line; int lines = 0; size_t i = 0;
    while (i < in.size()) {
        size_t e = in.find(' ', i); if (e == std::string::npos) e = in.size();
        std::string w = in.substr(i, e - i); i = e + 1;
        if (!line.empty() && (int)(line.size() + 1 + w.size()) > width) {
            if (++lines >= maxLines) { line += "..."; break; }
            out += line + "\n"; line.clear();
        }
        line += (line.empty() ? "" : " ") + w;
    }
    out += line;
    return out;
}

// Descriptions start with a category/prerequisite header ("Universal Power Restricted By Armor This power ..."): drop it and keep the first sentence.
inline std::string shortDesc(const std::string& d) {
    size_t p = d.find(" This "); std::string t = (p != std::string::npos && p < 120) ? d.substr(p + 1) : d;
    size_t e = t.find(". "); if (e != std::string::npos) t.resize(e + 1);
    return t;
}

// Tooltip metrics: px estimates for dialogfont16x16 at 2.667 px/unit.
constexpr int TipCharW = 21, TipLineH = 32, TipPad = 22, TipWrap = 38, TipMaxDescLines = 3;

// Tooltip box for `text` anchored above (fx, fy) (or above topAnchor when >= 0), clamped to 8 px inside the client's right/left/top edges.
struct TipRect { int lines = 1, maxLen = 0, x = 0, y = 0, w = 0, h = 0; };
inline TipRect tipLayout(const std::string& text, int fx, int fy, int topAnchor, int clientRight) {
    TipRect r; int cur = 0;
    for (char c : text) { if (c == '\n') { ++r.lines; cur = 0; } else if (++cur > r.maxLen) r.maxLen = cur; }
    r.w = r.maxLen * TipCharW + 2 * TipPad; r.h = r.lines * TipLineH + 2 * TipPad;
    int x = fx, y = fy - r.h - 12;
    if (topAnchor >= 0) y = topAnchor - r.h - 10;   // picker tooltips sit above the whole picker so they never cover it
    if (x + r.w > clientRight - 8) x = clientRight - 8 - r.w;
    if (x < 8) x = 8;
    if (y < 8) y = 8;
    r.x = x; r.y = y;
    return r;
}

// True while a character switch is fresh (HUD action lists lag behind): changeTick != 0 and fewer than settleMs ms ago (wrap-safe).
inline bool isSettling(uint32_t changeTick, uint32_t now, uint32_t settleMs) { return changeTick && now - changeTick < settleMs; }

// Slot dimming / empty-frame alpha factors and the out-of-stock quantity colour (gray "0", user request U1).
constexpr float EmptyFrameAlpha = 0.45f, DimAlpha = 0.35f;
constexpr float QtyOutOfStockRgb[3] = { 0.55f, 0.55f, 0.55f };

// What one bar slot shows. have = the want resolved to a HUD entry; flags = its entry flags (bit0 usable); settlingNormal = settling && Normal mode.
// hideSettling: not resolved while settling -> hide the slot this frame. Alpha factors multiply the widget's template alpha.
struct SlotView { const char* state = ""; bool hideSettling = false, outOfStock = false, showQty = false, grayQty = false; int qty = 0; const char* qtyHideWhy = ""; float bgFactor = 1.0f, icFactor = 1.0f; };
inline SlotView slotView(bool wantEmpty, bool wantIsItem, bool have, int flags, int key, const std::string& name, bool settlingNormal) {
    SlotView v;
    if (wantEmpty) v.state = "unbound";
    else if (!have) { if (settlingNormal) { v.hideSettling = true; return v; } v.state = "entry not available"; }
    else v.state = (flags & 1) ? "usable" : "unusable (dimmed)";
    // An item that left the HUD's lists is out of stock: dimmed icon + gray "0"; it comes back by itself once the party has one again.
    v.outOfStock = !have && wantIsItem;
    if (v.outOfStock) v.state = "out of stock";
    v.qty = have ? StackCount(key, name) : 0;   // the engine names item entries "Name (N)" only when N > 1
    v.showQty = v.qty > 1 || v.outOfStock; v.grayQty = v.outOfStock;
    v.qtyHideWhy = have ? "single item / not an item" : v.state;
    v.bgFactor = have ? 1.0f : EmptyFrameAlpha;
    v.icFactor = ((have && !(flags & 1)) || v.outOfStock) ? DimAlpha : 1.0f;
    return v;
}

// Picker scroll: largest first-visible index for n entries and `cells` cells.
inline int pickerMaxOff(int n, int cells) { return std::max(0, n - cells); }
// Clamp the scroll offset from above only (as the DLL always did).
inline int clampOffset(int off, int maxOff) { return off > maxOff ? maxOff : off; }
// Wheel notches from the accumulated delta (120 per notch; a partial notch still moves one step); 0 for 0.
inline int wheelSteps(long acc) { if (!acc) return 0; int steps = (int)(acc / 120); if (!steps) steps = acc > 0 ? 1 : -1; return steps; }
// New scroll target clamped to [0, maxOff].
inline int scrollClamp(int target, int maxOff) { return std::max(0, std::min(maxOff, target)); }

// Holding a picker arrow: first repeat after 350 ms, then every 90 ms (wrap-safe ms ticks).
constexpr int ArrowFirstMs = 350, ArrowRepeatMs = 90;
inline bool arrowRepeatDue(uint32_t now, uint32_t heldSince, uint32_t lastRep) { return now - heldSince >= (uint32_t)ArrowFirstMs && now - lastRep >= (uint32_t)ArrowRepeatMs; }

// Layout units -> px as InitControl scales (sx, sy), then the x squeeze c.
struct Scale { double sx = 1.0, sy = 1.0, c = 1.0; };
// x squeeze for a w x h client over a rw x rh layout space: the HUD draws at a uniform (h/rh) px per unit, InitControl stretched x by w/rw. 1.0 when any size is unknown.
inline double aspectCorrection(int w, int h, int rw, int rh) { return (w > 0 && h > 0 && rw > 0 && rh > 0) ? ((double)h / rh) / ((double)w / rw) : 1.0; }
// Scale for the client; the identity when no correction is needed (c == 1.0).
inline Scale makeScale(int w, int h, int rw, int rh) {
    Scale k; double c = aspectCorrection(w, h, rw, rh);
    if (c != 1.0) { k.sx = (double)w / rw; k.sy = (double)h / rh; k.c = c; }
    return k;
}
// Layout rect (units) -> px rect r[4] = x, y, w, h: x and width go through the squeeze, y and height do not.
inline void unitRect(int x, int y, int w, int h, const Scale& k, int* r) {
    r[0] = (int)((int)(x * k.sx) * k.c + 0.5); r[1] = (int)(y * k.sy); r[2] = (int)((int)(w * k.sx) * k.c + 0.5); r[3] = (int)(h * k.sy);
}

// "SAVES:000012 - Game11" / "/000012 - Game11" -> "000012 - Game11"; "" if it is not "NNNNNN - Name".
inline std::string normalizeSaveFolder(std::string f) {
    if (f.size() > 6 && lower(f.substr(0, 6)) == "saves:") f = f.substr(6);
    while (!f.empty() && (f[0] == '\\' || f[0] == '/')) f.erase(0, 1);
    if (f.size() < 10 || f.compare(6, 3, " - ") != 0) return "";
    for (int i = 0; i < 6; ++i) if (f[i] < '0' || f[i] > '9') return "";
    return f;
}
// <save root> + folder: an empty root means "saves/"; a missing trailing slash is added.
inline std::string joinSaveDir(std::string base, const std::string& folder) {
    if (base.empty()) base = "saves/";
    if (base.back() != '/' && base.back() != '\\') base += '/';
    return base + folder;
}

// Keyboard capture: DIK 0x02..0x0B are the digit keys 1..0.
inline bool isHotbarDigit(unsigned dik) { return dik >= 0x02 && dik <= 0x0B; }
// One DirectInput key event with the state the decision needs (hudActive = gate open and the tick is fresh).
struct KeyEvent { unsigned dik = 0; bool down = false, ctrl = false, alt = false, shift = false, hudActive = false, editOpen = false, downSwallowed = false, slotBound = false, ringFull = false; };
// What to do with it: swallow = zero the record; markDown/clearDown = set/clear the "down was swallowed" flag; enqueue = push `slot`; escHit = tell the editor.
struct KeyAction { const char* decision = "pass"; bool swallow = false, markDown = false, clearDown = false, enqueue = false, escHit = false; int slot = -1; };
inline KeyAction decideKey(const KeyEvent& k) {
    KeyAction a;
    if (k.dik == 0x01) {   // while the editor is open Esc backs out of it instead of opening the game menu
        if (!k.down) { if (k.downSwallowed) { a.clearDown = true; a.swallow = true; a.decision = "swallow-up (edit)"; } }
        else if (k.editOpen && k.hudActive) { a.markDown = true; a.escHit = true; a.swallow = true; a.decision = "SWALLOW (edit Esc)"; }
        else a.decision = "pass(edit closed)";
    }
    if (isHotbarDigit(k.dik)) {
        if (!k.down) {
            if (k.downSwallowed) { a.clearDown = true; a.swallow = true; a.decision = "swallow-up"; }
        } else if (!k.shift) {
            int slot = (int)k.dik - kFirstDik;
            if (k.ctrl || k.alt) a.decision = "pass(ctrl/alt)";   // only plain digits are ours; Ctrl/Alt+digit stay the game's
            else if (!k.hudActive) a.decision = "pass(hud inactive)";
            else if (k.editOpen) a.decision = "pass(edit mode: never invoke)";
            else if (!k.slotBound) a.decision = "pass(unbound)";
            else {   // a bound key is ALWAYS swallowed: passing it on would switch party members (keys 1-3)
                a.markDown = true; a.swallow = true;
                if (k.ringFull) a.decision = "SWALLOW(ring full, request dropped)";
                else { a.enqueue = true; a.slot = slot; a.decision = "SWALLOW"; }
            }
        } else a.decision = "pass(shift)";
    }
    return a;
}

// Embedded TGA (uncompressed 32-bit, as written by scripts/gen_hotbar_textures.py) -> RGBA pixels.
// TGA_TOO_LARGE: a dimension above MaxTgaDim (the old 32-bit w*h*4 wrapped for big headers, passed the bounds check and read past the array).
enum TgaStatus { TGA_OK = 0, TGA_NOT_UNCOMPRESSED32, TGA_TRUNCATED, TGA_TOO_LARGE };
constexpr int MaxTgaDim = 4096;   // <= 4096x4096x4 = 64 MiB; the shipped art is 128x128
struct TgaInfo { int w = 0, h = 0; unsigned off = 0, bytes = 0; };   // pixel data offset, w*h*4 (valid for TGA_OK)
inline TgaStatus tgaInspect(const unsigned char* d, size_t size, TgaInfo* t) {
    if (size < 18 || d[1] != 0 || d[2] != 2 || d[16] != 32) return TGA_NOT_UNCOMPRESSED32;
    int w = d[12] | d[13] << 8, h = d[14] | d[15] << 8; unsigned off = 18u + d[0];
    t->w = w; t->h = h; t->off = off; t->bytes = 0;
    if (w <= 0 || h <= 0) return TGA_TRUNCATED;
    if (w > MaxTgaDim || h > MaxTgaDim) return TGA_TOO_LARGE;
    uint64_t n = (uint64_t)w * (uint64_t)h * 4u;   // 64-bit: <= 2^26 after the limit, and off + n cannot wrap
    if ((uint64_t)off + n > (uint64_t)size) return TGA_TRUNCATED;
    t->bytes = (unsigned)n;
    return TGA_OK;
}
// BGRA -> RGBA, rows as stored (= CResTGA); dst holds t.bytes bytes.
inline void tgaBgraToRgba(const unsigned char* d, const TgaInfo& t, unsigned char* dst) {
    for (unsigned j = 0; j < t.bytes; j += 4) { dst[j] = d[t.off + j + 2]; dst[j + 1] = d[t.off + j + 1]; dst[j + 2] = d[t.off + j]; dst[j + 3] = d[t.off + j + 3]; }
}

}  // namespace hb
