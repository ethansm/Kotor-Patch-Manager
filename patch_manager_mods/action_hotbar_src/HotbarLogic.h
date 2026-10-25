// Action Hotbar -- item target selection (pure C++17, no Win32/engine types so it can be unit-tested on the host).
// The DLL fills one Candidate per party position (HP, "readable", "down", "can this item be used on them"); this header decides.
#pragma once
#include "HotbarConfig.h"
#include "HealItems.h"
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

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

// RH4: a heal the hotbar sent that queue-hud may still hold. Live while younger than 5 s (wrap-safe) and the member's HP has not risen since the press (a rise = it resolved).
constexpr uint32_t PendingHealMs = 5000;
inline bool pendingHealLive(uint32_t now, uint32_t since, int hpNow, int hpAtPress) { return now - since < PendingHealMs && hpNow <= hpAtPress; }

// False => draw the slot dimmed and ignore clicks/keys: lowest-HP with no fallback and nobody to heal.
inline bool slotEnabled(const Candidate* m, int n, int selfIdx, const TargetSpec& t, const General& g) {
    return !(t.mode == TM_LowHp && !effectiveFallbackSelf(t, g) && chooseTarget(m, n, selfIdx, effectiveThreshold(t, g), false) == -1);
}

// Vanilla's "usable on another party member" rule: baseitems.2da itemtype (engine GetBaseItem+0xAC) is not 20 (forearm bands =
// energy shields, self only) and not 47 (squad recovery kit, used directly). Negative = unknown -> false.
inline bool isTargetableItemType(int itemType) { return itemType >= 0 && itemType != 20 && itemType != 47; }

// ======== pure units extracted from ActionHotbar.cpp (behaviour-neutral; tested in hotbar_logic_test.cpp) ========
// (this header is included by the DLL after windows.h: only <cstdint>/<algorithm>/<string>/<vector> here, no Win32 types)

// Request ring (click/key -> HUD tick): single producer / single consumer on the game thread, one slot always empty (full = RingSize - 1 queued).
constexpr int RingSize = 16;
inline int ringNext(int i) { return (i + 1) % RingSize; }
inline int ringCount(int head, int tail) { return (head - tail + RingSize) % RingSize; }
inline bool ringFull(int head, int tail) { return ringNext(head) == tail; }
template <class R> inline void ringPush(R* ring, int& head, const R& r) { ring[head] = r; head = ringNext(head); }   // caller checks ringFull first

// HUD entry key -> stable ref: KeyItemFlag = item (identity = normalised name), KeySpellFlag = spell row, else other (flag constants: HotbarConfig.h).
inline ActionRef decodeKey(int key, const std::string& name) {
    ActionRef r;
    if (key & KeyItemFlag) { r.kind = K_Item; r.name = normalizeItemName(name); if (r.name.empty()) r.kind = K_None; }
    else if (key & KeySpellFlag) { r.kind = K_Spell; r.id = key & KeyRowMask; }
    else { r.kind = K_Other; r.id = key; }
    return r;
}

// Picker tab of an entry key: 0 powers, 1 items, 2 modes.
enum Tab { Tab_Power = 0, Tab_Item = 1, Tab_Mode = 2 };
inline int tabOfKey(int key) { return (key & KeyItemFlag) ? Tab_Item : ((key & KeySpellFlag) ? Tab_Power : Tab_Mode); }

// Party slot of the controlled creature: pointer match first, then object id (module loads may wrap the pointer),
// but never by the transient ids 0 / 0xFFFFFFFF alone, except 0xFFFFFFFF held by exactly one member (the Exile on the Ebon Hawk).
// ptrs/oids: the member pointers and ids for slots 0..count-1 (ptr 0 = empty slot). Returns -1 when unmatched or ambiguous.
inline int pickPartySlot(uintptr_t controlled, unsigned oid, const uintptr_t* ptrs, const unsigned* oids, int count) {
    if (count > 3) count = 3;
    for (int i = 0; i < count; ++i) if (ptrs[i] != 0 && ptrs[i] == controlled) return i;
    if (oid == 0) return -1;
    int hit = -1, n = 0;
    for (int i = 0; i < count; ++i) if (ptrs[i] != 0 && oids[i] == oid) { if (hit < 0) hit = i; ++n; }
    if (oid == 0xFFFFFFFFu) return n == 1 ? hit : -1;
    return hit;
}

// Character key from creature tag, party slot, or creature object id:
// 1. Tag (if present and not player/exile): returns lowercase trimmed tag (e.g. "atton", "kreia").
// 2. Player character (Exile): recognized by party slot 0 / isMember0, partyCount == 1, tag "player"/"exile",
//    or sentinel IDs 0xFFFFFFF0..0xFFFFFFFE (the Odyssey negative player sentinel range,
//    e.g. 0xFFFFFFF9 during fast-travel); all map to canonical "objfffffffd".)
// 3. Transient invalid IDs (0 / 0xFFFFFFFF): returns "" (load in progress).
// 4. Companion without tag: returns "slot1" / "slot2" if partySlot > 0.
// 5. Fallback: "obj%08x".
inline std::string characterKeyFor(unsigned oid, int partyCount, bool* invalid = nullptr, bool isMember0 = false, const char* tag = nullptr, int partySlot = -1) {
    bool bad = (oid == 0xFFFFFFFFu || oid == 0);
    if (invalid) *invalid = bad;

    if (tag && *tag) {
        std::string t = lower(trim(tag));
        if (t == "player" || t == "exile") return "objfffffffd";
        return t;
    }

    bool isExile = ((oid >= 0xFFFFFFF0u && oid <= 0xFFFFFFFEu) || partyCount == 1 || isMember0 || partySlot == 0);
    if (isExile) {
        if (oid == 0) return "";   // id 0 never gets the rescue
        if (oid == 0xFFFFFFFFu && !(partyCount == 1 || isMember0 || partySlot == 0)) return "";
        return "objfffffffd";
    }

    if (bad) return "";

    if (partySlot > 0) {
        char b[16]; snprintf(b, sizeof b, "slot%d", partySlot);
        return b;
    }

    char b[24]; snprintf(b, sizeof b, "obj%08x", oid);
    return b;
}

// AutoFill: true for a spell key that is a known Force power (not an item, not a mode toggle).
inline bool isKnownPowerKey(int key) { return tabOfKey(key) == Tab_Power; }

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

// HB04: before an assignment rewrites the profile row, every empty, non-"none" slot of the row is frozen to what the bar currently shows
// (the effective/auto-filled action), so saving never silently drops an auto-filled binding. Returns how many slots were frozen.
inline int freezeEffective(ActionRef (&row)[Slots], const ActionRef* eff) {
    int frozen = 0;
    for (int i = 0; i < Slots; ++i)
        if (row[i].empty() && !row[i].blank && !eff[i].empty()) { row[i] = eff[i]; row[i].blank = false; ++frozen; }
    return frozen;
}

// HB05: how drainRing runs a resolved slot. Direct routes (item @self needs probe >= 2, item @lowhp needs probe > 0) may still fail to a
// vanilla invoke: the DLL falls through to invokeEntry when Invoke=1 after a direct route returns false. Invoke/LogOnly = no direct route.
enum UseRoute { UR_Invoke, UR_SelfDirect, UR_LowHpDirect, UR_LogOnly };
inline UseRoute useRoute(const ActionRef& a, int probe, bool invoke) {
    if (a.kind == K_Item && a.target.mode == TM_Self && probe >= 2) return UR_SelfDirect;
    if (a.kind == K_Item && a.target.mode == TM_LowHp && probe > 0) return UR_LowHpDirect;
    return invoke ? UR_Invoke : UR_LogOnly;
}

// RL24: item @self takes the direct route only when the HUD entry's bit0 (usable) is set; else the vanilla invoke reports why not.
// @lowhp is NOT gated: bit0 describes the controlled character, and useOnMember already asks the engine (canUseItem) about the TARGET member.
inline bool directRouteAllowed(UseRoute r, int entryFlags) { return r != UR_SelfDirect || (entryFlags & EntryUsable) != 0; }

// RL26: backup file name for generation gen (0 = newest).
inline std::string loadBakName(const char* base, int gen) { return gen <= 0 ? std::string(base) : std::string(base) + "." + std::to_string(gen); }

// HB07: the AutoFill candidate list. powers = group 0 entries (HUD key, ref) in HUD order; known Force powers (isKnownPowerKey) not already
// explicit are taken first. Then one pick per item group (HUD groups 1..3 = groups[0..2]): the entry whose key == selKey, else the first;
// skipped (dupMask bit g set) when already explicit or already collected, or when its ref is empty (no dup bit for that).
struct AutoGroup { std::vector<int> keys; std::vector<ActionRef> refs; unsigned selKey = 0; };
inline std::vector<ActionRef> collectAutos(const std::vector<ActionRef>& explicitRefs, const std::vector<std::pair<int, ActionRef>>& powers,
                                           const AutoGroup groups[3], bool autoFill, unsigned* dupMask) {
    std::vector<ActionRef> autos;
    if (dupMask) *dupMask = 0;
    if (!autoFill) return autos;
    for (const auto& p : powers) {
        if (!isKnownPowerKey(p.first)) continue;
        if (!hasSame(explicitRefs, p.second)) autos.push_back(p.second);
    }
    for (int g = 0; g < 3; ++g) {
        const AutoGroup& ag = groups[g];
        int n = (int)std::min(ag.keys.size(), ag.refs.size());
        if (n <= 0) continue;
        const ActionRef& pick = ag.refs[(size_t)std::max(0, pickIndexByKey(ag.keys.data(), n, ag.selKey))];
        bool dup = hasSame(explicitRefs, pick) || hasSame(autos, pick);
        if (dup && dupMask) *dupMask |= 1u << g;
        if (!dup && !pick.empty()) autos.push_back(pick);
    }
    return autos;
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
    if (wantEmpty) { have = false; wantIsItem = false; }   // HB18: an unbound slot shows nothing, whatever a (stale) resolved entry says: no qty, no out-of-stock
    if (wantEmpty) v.state = "unbound";
    else if (!have) { if (settlingNormal) { v.hideSettling = true; return v; } v.state = "entry not available"; }
    else v.state = (flags & EntryUsable) ? "usable" : "unusable (dimmed)";
    // An item that left the HUD's lists is out of stock: dimmed icon + gray "0"; it comes back by itself once the party has one again.
    v.outOfStock = !have && wantIsItem;
    if (v.outOfStock) v.state = "out of stock";
    v.qty = have ? StackCount(key, name) : 0;   // the engine names item entries "Name (N)" only when N > 1
    v.showQty = v.qty > 1 || v.outOfStock; v.grayQty = v.outOfStock;
    v.qtyHideWhy = have ? "single item / not an item" : v.state;
    v.bgFactor = have ? 1.0f : EmptyFrameAlpha;
    v.icFactor = ((have && !(flags & EntryUsable)) || v.outOfStock) ? DimAlpha : 1.0f;
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
// Scale for the client: sx/sy = px per layout unit; c is the x squeeze (1.0 for 4:3); the identity only when a size is unknown.
inline Scale makeScale(int w, int h, int rw, int rh) {
    Scale k;
    if (w > 0 && h > 0 && rw > 0 && rh > 0) { k.sx = (double)w / rw; k.sy = (double)h / rh; k.c = aspectCorrection(w, h, rw, rh); }
    return k;
}
// Layout rect (units) -> px rect r[4] = x, y, w, h: x and width go through the squeeze, y and height do not.
inline void unitRect(int x, int y, int w, int h, const Scale& k, int* r) {
    r[0] = (int)((int)(x * k.sx) * k.c + 0.5); r[1] = (int)(y * k.sy); r[2] = (int)((int)(w * k.sx) * k.c + 0.5); r[3] = (int)(h * k.sy);
}


// Keyboard capture: DIK 0x02..0x0B are the digit keys 1..0.
inline bool isHotbarDigit(unsigned dik) { return dik >= 0x02 && dik <= 0x0B; }
// One DirectInput key event with the state the decision needs (hudActive = gate open and the tick is fresh).
struct KeyEvent { unsigned dik = 0; bool down = false, ctrl = false, alt = false, shift = false, hudActive = false, editOpen = false, downSwallowed = false, slotBound = false, ringFull = false, invokeOn = true, reserveDigits = false; };
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
            a.clearDown = true;   // RL29: a key-down that passes drops a stale "down was swallowed" flag (the swallow branch below clears this again)
            int slot = (int)k.dik - kFirstDik;
            if (k.ctrl || k.alt) a.decision = "pass(ctrl/alt)";   // only plain digits are ours; Ctrl/Alt+digit stay the game's
            else if (!k.hudActive) a.decision = "pass(hud inactive)";
            else if (k.editOpen || !k.slotBound) {
                // KD1: ReserveDigits keeps 1-0 off the vanilla quick bar / party switch even with nothing to invoke (edit mode, empty slot)
                if (k.reserveDigits) { a.clearDown = false; a.markDown = true; a.swallow = true; a.decision = k.editOpen ? "SWALLOW(reserved, edit mode)" : "SWALLOW(reserved, unbound)"; }
                else a.decision = k.editOpen ? "pass(edit mode: never invoke)" : "pass(unbound)";
            }
            else if (!k.invokeOn) a.decision = "pass(invoke off)";   // dry run: the key keeps its game meaning (party switch on 1-3)
            else {   // a bound key is swallowed while the HUD tick is fresh (hudActive): passing it on would switch party members (keys 1-3)
                a.clearDown = false; a.markDown = true; a.swallow = true;
                if (k.ringFull) a.decision = "SWALLOW(ring full, request dropped)";
                else { a.enqueue = true; a.slot = slot; a.decision = "SWALLOW"; }
            }
        } else { a.clearDown = true; a.decision = "pass(shift)"; }
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

// ======== log formatters (pure; ActionHotbar.cpp passes the result to KP_LOG as "%s") ========
// One call = one log line (or one vector element per line). Every line starts with its tag and stays <= LogMax bytes for the
// worst-case arguments (the core cuts a text at 232 bytes; map 07b section 3), tested in hotbar_log_test.cpp. No '\n' in any line:
// KpLog.h splits on it and the pieces would arrive untagged. Text is byte-identical to the pre-extraction KP_LOG format strings.
constexpr size_t LogMax = 232;

#if defined(__GNUC__)
#if defined(__MINGW32__) && defined(__MINGW_PRINTF_FORMAT)
#define HB_PRINTF_ATTR(f, a) __attribute__((format(__MINGW_PRINTF_FORMAT, f, a)))
#else
#define HB_PRINTF_ATTR(f, a) __attribute__((format(printf, f, a)))
#endif
#else
#define HB_PRINTF_ATTR(f, a)
#endif

HB_PRINTF_ATTR(1, 2)
inline std::string sfmt(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    va_list cp; va_copy(cp, ap);
    int n = vsnprintf(nullptr, 0, fmt, cp);
    va_end(cp);
    std::string r;
    if (n > 0) { r.resize((size_t)n + 1); vsnprintf(&r[0], r.size(), fmt, ap); r.resize((size_t)n); }
    va_end(ap);
    return r;
}

// One char per slot: -1 '?', 0 '0', anything else '1'.
inline std::string fmtVisMask(const int* vis, int n) {
    std::string r;
    if (!vis) return r;
    for (int i = 0; i < n; ++i) r += vis[i] < 0 ? '?' : (vis[i] ? '1' : '0');
    return r;
}

// "tag[0]=<chunk>", "tag[1]=<chunk>" ...; an empty string gives "tag[0]=". chunk 0 = the default 200.
inline std::vector<std::string> fmtChunks(const char* tag, const std::string& s, size_t chunk = 200) {
    std::vector<std::string> r;
    if (chunk == 0) chunk = 200;
    if (s.empty()) { r.push_back(sfmt("%s[0]=", tag)); return r; }
    int i = 0;
    for (size_t off = 0; off < s.size(); off += chunk, ++i) r.push_back(sfmt("%s[%d]=%s", tag, i, s.substr(off, chunk).c_str()));
    return r;
}

// "tip: text[i] \"<piece>\"" for each '\n'-separated piece (a trailing '\n' or an empty text gives a final empty piece), piece clipped to 200.
inline std::vector<std::string> fmtTipLines(const std::string& text) {
    std::vector<std::string> r;
    int li = 0;
    for (size_t off = 0; off <= text.size(); ++li) {
        size_t nl = text.find('\n', off); if (nl == std::string::npos) nl = text.size();
        r.push_back(sfmt("tip: text[%d] \"%.200s\"", li, text.substr(off, nl - off).c_str()));
        off = nl + 1;
    }
    return r;
}

// "slot <key>: group=.. index=.. key=.. flags=.. usable=<flags&EntryUsable> INVOKING|(Invoke=0) ref=<ref>" (ref last: it can be 100 bytes)
inline std::string fmtSlotResolve(const std::string& slotKey, int group, int index, unsigned key, unsigned flags, bool invoke, const std::string& refText) {
    return sfmt("slot %s: group=%d index=%d key=%08x flags=%08x usable=%d %s ref=%s", slotKey.c_str(), group, index, key, flags, (int)(flags & EntryUsable),
                invoke ? "INVOKING" : "(Invoke=0)", refText.c_str());
}
inline std::string fmtSlotMiss(const std::string& slotKey, const std::string& refText, const std::string& charKey) {
    return sfmt("slot %s: entry not available for '%s' ref=%s", slotKey.c_str(), charKey.c_str(), refText.c_str());
}

struct HeartbeatInfo {
    long frame = 0; int gate = 0; std::string charKey; int ring = 0; int invoke = 0; long long cfgMtime = 0;
    unsigned panel = 0; int built = 0; int vis[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned widgetFaults = 0, probeFaults = 0, exoFaults = 0, iconFaults = 0, gateFlaps = 0;
};
inline std::string fmtHeartbeat(const HeartbeatInfo& h) {
    return sfmt("heartbeat: frame=%ld gate=%d char='%s' ring=%d invoke=%d cfgMtime=%lld", h.frame, h.gate, h.charKey.c_str(), h.ring, h.invoke, h.cfgMtime);
}
inline std::string fmtHeartbeatBar(const HeartbeatInfo& h) {
    return sfmt("heartbeat: bar panel=%08x built=%d vis=%s wfaults=%u pfaults=%u efaults=%u ifaults=%u gateflaps=%u", h.panel, h.built,
                fmtVisMask(h.vis, 10).c_str(), h.widgetFaults, h.probeFaults, h.exoFaults, h.iconFaults, h.gateFlaps);
}

struct H3Summary {
    int created = 0, total = 0, first = 0, last = 0, buttons = 0, buttonCount = 0, editorBound = 0, editorCount = 0;   // first..last = panel control indices
    bool editorEnabled = false; long frame = 0; int rect[4] = {0, 0, 0, 0};
};
inline std::string fmtH3Summary(const H3Summary& s) {
    return sfmt("H3: %d/%d controls created (indices %d..%d) buttons=%d/%d editor=%d/%d -> editor %s frame=%ld", s.created, s.total, s.first, s.last,
                s.buttons, s.buttonCount, s.editorBound, s.editorCount, s.editorEnabled ? "ENABLED" : "DISABLED", s.frame);
}
inline std::string fmtH3Rect(const H3Summary& s) { return sfmt("H3: picker background rect [%d,%d,%d,%d]", s.rect[0], s.rect[1], s.rect[2], s.rect[3]); }

inline std::string fmtPersaveSnapshot(const char* why, bool ok, int profiles) { return sfmt("persave: %s: save %s profiles=%d", why, ok ? "ok" : "FAILED", profiles); }
inline std::string fmtPersaveDst(const char* why, const std::string& dst) { return sfmt("persave: %s: dst=%.170s", why, dst.c_str()); }
inline std::string fmtPersaveReplace(const char* why, bool replaced, int profiles, int snapWarnings, bool backupOk) {
    return sfmt("persave: %s: live hotbar %s profiles=%d snapshot_warnings=%d backup=%s", why, replaced ? "replaced" : "NOT replaced (save failed)", profiles, snapWarnings, backupOk ? "ok" : "none");
}
// Status first so the 232-byte core cut can only clip the path.
inline std::string fmtPersaveSource(const char* why, const char* status, const std::string& path) { return sfmt("persave: %s: %s: source=%.150s", why, status, path.c_str()); }

inline std::string fmtAssign(int slot, const std::string& slotKey, bool saveOk, int frozen, long long mtime, const std::string& charKey) {
    return sfmt("edit: assign slot=%d key=%s save=%s froze=%d mtime=%lld char='%s'", slot, slotKey.c_str(), saveOk ? "ok" : "FAILED", frozen, mtime, charKey.c_str());
}
inline std::string fmtAssignValue(bool hasEntry, const std::string& refText) { return sfmt("edit: assign value=%s", hasEntry ? refText.c_str() : "none"); }

// Stance / form watch: the vanilla action bar only resyncs on a character switch (0x74DA40), so we detect mode edges ourselves.
struct ModeWatch { uint32_t creature = 0; int stance = 0; uint32_t form = 0; bool valid = false; };
inline uint32_t maskForm(uint32_t raw) { return raw & 0xDFFFFFFFu; }   // vanilla 0x74DA40 masks bit 29 before storing hud+0x1C90
// True exactly once per stance/form edge. First sight and a character switch only record (vanilla resyncs on the switch itself);
// a failed read (readOk=false) neither fires nor disturbs the stored state.
inline bool modeChanged(ModeWatch& w, uint32_t creature, int stance, uint32_t form, bool readOk) {
    if (!readOk) return false;
    form = maskForm(form);
    if (!w.valid || w.creature != creature) { w.creature = creature; w.stance = stance; w.form = form; w.valid = true; return false; }
    if (w.stance == stance && w.form == form) return false;
    w.stance = stance; w.form = form;
    return true;
}

// Item target-mode cycle: Default -> LowHp (healing only) -> Self (if allowed) -> Default; non-items unchanged; result is a fresh spec (custom suffix dropped).
inline TargetSpec nextTargetMode(const ActionRef& a, bool isHealing, bool selfAllowed) {
    if (a.kind != K_Item) return a.target;
    TargetSpec n;
    if (a.target.mode < TM_LowHp && isHealing) n.mode = TM_LowHp;
    else if (a.target.mode < TM_Self && selfAllowed) n.mode = TM_Self;
    else n.mode = TM_Default;
    return n;
}
// TP1: "use on?" choices offered when an item is picked in the editor (same modes the Mode button cycles through).
// Empty = no prompt (not an item, or the item can only be used the game's way): the pick assigns directly.
inline std::vector<TargetMode> targetChoices(const ActionRef& a, bool isHealing, bool selfAllowed) {
    std::vector<TargetMode> v;
    if (a.kind != K_Item || (!isHealing && !selfAllowed)) return v;
    v.push_back(TM_Default);
    if (isHealing) v.push_back(TM_LowHp);
    if (selfAllowed) v.push_back(TM_Self);
    return v;
}
// Choice cell art: the target badges for Lowest HP / Self, the item's own icon for "use normally".
inline std::string choiceIcon(TargetMode m, const std::string& itemIcon) { return m == TM_LowHp ? "fph_tgt_lowhp" : m == TM_Self ? "fph_tgt_self" : itemIcon; }
inline std::string choiceTip(TargetMode m, const std::string& itemName) {
    const char* how = m == TM_LowHp ? "Use on the party member with the lowest HP" : m == TM_Self ? "Always use on yourself" : "Use normally (the game's target)";
    return std::string(how) + "\n" + itemName;
}
// The ref a choice assigns: the picked item with a fresh target spec (no custom threshold / fallback).
inline ActionRef withTarget(ActionRef a, TargetMode m) { a.target = TargetSpec(); a.target.mode = m; return a; }

inline bool dropsCustomSuffix(const TargetSpec& from) { return from.thresholdPct >= 0 || from.fallback >= 0; }
// Target badge label of slot 0..9: 13x13 flush top-right inside the 35x35 frame (frame x = 29+40i, y = 478); the digit label ends at slotX+22.
struct Rect4 { int x, y, w, h; };
constexpr Rect4 badgeRect(int slot) { return { 29 + 40 * slot + 22, 479, 13, 13 }; }
inline const char* badgeFor(const TargetSpec& t) { return t.mode == TM_LowHp ? "fph_tgt_lowhp" : t.mode == TM_Self ? "fph_tgt_self" : nullptr; }
enum ModeBtn { MB_Hidden, MB_Enabled, MB_DisabledItemsOnly, MB_DisabledCannotTarget };
inline ModeBtn modeButtonState(const ActionRef& a, bool isHealing, bool selfAllowed) {
    if (a.empty()) return MB_Hidden;
    if (a.kind != K_Item) return MB_DisabledItemsOnly;
    return (!isHealing && !selfAllowed) ? MB_DisabledCannotTarget : MB_Enabled;
}
inline const char* targetModeName(TargetMode m) { return m == TM_LowHp ? "Lowest HP" : m == TM_Self ? "Self" : "Default"; }
inline std::string modeTooltip(const ActionRef& a, bool isHealing, bool selfAllowed) {
    switch (modeButtonState(a, isHealing, selfAllowed)) {
    case MB_Enabled: return std::string("Target: ") + targetModeName(a.target.mode) + " (click: " + targetModeName(nextTargetMode(a, isHealing, selfAllowed).mode) + ")";
    case MB_DisabledItemsOnly: return "Target modes: items only";
    case MB_DisabledCannotTarget: return "Target: cannot target this item";
    default: return "";
    }
}
// Reason text the mode-button click log gives when the click is ignored ("" for MB_Enabled). Log wording, not the tooltip wording.
inline const char* modeBtnWhy(ModeBtn st) {
    return st == MB_Hidden ? "empty slot" : st == MB_DisabledItemsOnly ? "items only" : st == MB_DisabledCannotTarget ? "cannot target this item" : "";
}
inline std::string slotTargetLine(const TargetSpec& t, int effectiveThresholdPct) {
    if (t.mode == TM_LowHp) return "Target: Lowest HP (below " + std::to_string(effectiveThresholdPct) + "%)";
    return t.mode == TM_Self ? "Target: Self" : "";
}

// ---- item edge cases (B2, doc 07): column-driven from spells.2da, item property 0 from the engine item ----
inline int parseHexOrInt(const std::string& s) {   // spells.2da category is "0x1408"; "****"/empty/garbage -> -1
    if (s.empty() || s == "****") return -1;
    // HB16: decimal, or hex only with an explicit 0x/0X prefix ("010" is 10, never octal 8); no sign/space; overflow/range -> -1.
    bool hex = s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
    const char* d = s.c_str() + (hex ? 2 : 0);
    if (!isxdigit((unsigned char)*d) || (!hex && !isdigit((unsigned char)*d))) return -1;   // also rejects "0x", "+5", "-5", " 5"
    char* e = nullptr; errno = 0; unsigned long long v = strtoull(d, &e, hex ? 16 : 10);
    return (*e == 0 && errno != ERANGE && v <= 0x7FFFFFFFULL) ? (int)v : -1;
}
using heal::isHealingRow;   // shared (_shared_src/HealItems.h): usertype 4, range "T", category medpac/repair/cure
inline bool selfAllowedRow(const std::string& range, int hostileSetting) { return (range == "T" || range == "t") && hostileSetting != 1; }
struct ItemProp { int count = 0; int type = -1; int row = -1; int usable = 0; };
inline int usableSpellRow(const ItemProp& p) { return (p.count > 0 && p.type == 10 && p.usable != 0 && p.row >= 0) ? p.row : -1; }
inline const char* itemPropWhy(const ItemProp& p) {
    if (p.count <= 0) return "no properties";
    if (p.type != 10) return "property 0 not CastSpell";
    if (p.usable == 0) return "no uses left";
    return p.row < 0 ? "bad row" : "ok";
}
// HB08: item classification. row < 0 = not usable (why = propWhy); healing = a medpac/repair/cure row, selfOk = self use allowed.
struct ItemClass { int row = -1; bool healing = false, selfOk = false; const char* why = ""; };
// row = usableSpellRow(prop), propWhy = itemPropWhy(prop), nRows = spells.2da row count (0 = not loaded), the strings = that row's cells
// (empty when the row is out of range). A row past the table keeps row set but reports "spells.2da row unknown" and neither flag.
inline ItemClass classifyRow(int row, const char* propWhy, int nRows, const std::string& range, const std::string& usertype,
                             const std::string& hostile, const std::string& category) {
    ItemClass c; c.row = row; c.why = propWhy;
    if (row < 0) return c;
    if (nRows <= 0 || row >= nRows) { c.why = "spells.2da row unknown"; return c; }
    c.healing = isHealingRow(parseHexOrInt(usertype), range, parseHexOrInt(category));
    c.selfOk = selfAllowedRow(range, parseHexOrInt(hostile));
    return c;
}


// ---- pure helpers extracted from ActionHotbar.cpp (tip title/qty, per-save snapshot merge/pick, TargetProbe) ----
struct TipTitle { std::string title, qty; };
// "Medpac (3)" -> {"Medpac","3"}; the trailing " (...)" is stripped from the title either way, but qty is set only when it is digits-only.
inline TipTitle tipTitleQty(const std::string& name) {
    TipTitle t; t.title = name;
    size_t par = name.rfind(" (");
    if (par == std::string::npos || par == 0 || name.back() != ')') return t;
    t.title.resize(par);
    std::string q = name.substr(par + 2, name.size() - par - 3);
    bool digits = !q.empty();
    for (char c : q) if (c < '0' || c > '9') { digits = false; break; }
    if (digits) t.qty = q;
    return t;
}

// A11: hover text of a picker/bar entry. Force-power keys (KeySpellFlag without KeyItemFlag) take their spells.2da cells from `sp`
// (fp already "****"-stripped, range raw, desc = raw TLK text, "" = none); item keys show the stack quantity from the name.
struct TipSpell { std::string fp, range, desc; };
inline std::string tipText(char digit, const std::string& name, int key, int flags, const TipSpell& sp) {
    TipTitle tt = tipTitleQty(name);
    std::string out = tt.title + "\n";
    int tab = tabOfKey(key);
    const char* kind = tab == Tab_Item ? "Item" : (tab == Tab_Power ? "Force Power" : "Mode");
    if (digit) out += std::string("Key: ") + digit + "   " + kind + "\n"; else out += std::string(kind) + "\n";
    if (tab == Tab_Power) {
        if (!sp.fp.empty() && sp.fp != "0") out += "Force Points: " + sp.fp + "\n";
        if (sp.range == "P") out += "Range: Personal\n"; else if (sp.range == "T") out += "Range: Target\n";
        if (!sp.desc.empty()) { std::string d = shortDesc(sp.desc); if (!d.empty()) out += wrapText(d, TipWrap, TipMaxDescLines) + "\n"; }
    } else if (tab == Tab_Item) {
        if (!tt.qty.empty()) out += "Quantity: " + tt.qty + "\n";
    }
    if (!(flags & EntryUsable)) out += "Not usable right now\n";
    while (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

// Picker list signature (FNV-1a over key, flags, name per entry, then the tab); the name carries the stack count, so a count change refreshes.
template <class E> inline unsigned pickerSignature(const std::vector<E>& entries, int tab) {
    unsigned h = 2166136261u;
    for (const E& x : entries) {
        h = (h ^ (unsigned)x.key ^ ((unsigned)x.flags << 3)) * 16777619u;
        for (char ch : x.name) h = (h ^ (unsigned char)ch) * 16777619u;
    }
    return (h ^ (unsigned)tab) * 16777619u;
}

// replaceLiveProfiles rule: keep the live [General], always have a Default profile, snapshot profiles overwrite.
inline Config mergeSnapshot(const General& live, const Config* snap) {
    Config n; n.general = live; n.profile("Default");
    if (snap) {
        for (auto& kv : snap->profiles) n.profiles[kv.first] = kv.second;
        n.unknownSections = snap->unknownSections;   // C049: unknown sections travel with the snapshot's profiles
    }
    return n;
}

enum SnapSrc { SS_New, SS_Legacy, SS_Absent };
struct SnapPick { std::string path; SnapSrc src; };
inline SnapPick pickSnapshot(const std::string& dir, bool (*exists)(const std::string&)) {
    for (const std::string& nm : snapshotNames()) {   // new name first, then the pre-1.2.0 one
        std::string cand = dir + "/" + nm;
        if (exists(cand)) { SnapPick p; p.path = cand; p.src = (nm == LegacyIniName) ? SS_Legacy : SS_New; return p; }
    }
    SnapPick p; p.path = dir + "/" + IniName; p.src = SS_Absent; return p;
}
inline const char* snapStatus(SnapSrc src, bool loaded) {
    if (src == SS_Absent) return "absent (live hotbar kept)";
    if (!loaded) return "UNREADABLE (live hotbar kept)";
    return src == SS_Legacy ? "found (legacy name)" : "found";
}
// What a load does with the pick: a readable copy replaces the live profiles; absent or unreadable keeps the live bar.
// PS1: absent is NOT "empty the bar": a save whose copy was lost (save event missed, live 10-24) looks the same as a pre-mod save,
// and emptying wiped the player's bar. Only a new game empties it.
enum SnapAction { Snap_Keep, Snap_Replace };
inline SnapAction snapAction(SnapSrc src, bool readable) { return src != SS_Absent && readable ? Snap_Replace : Snap_Keep; }

// PS3: re-check the save folder's copy this long after writing it; rewrite once if the engine removed it.
constexpr uint32_t SnapVerifyMs = 3000;
enum SnapVerify { SV_Wait, SV_Ok, SV_Rewrite, SV_Gone };
// since = tick of the write (wrap-safe); copyThere / dirThere = what the disk shows now.
inline SnapVerify snapVerify(uint32_t now, uint32_t since, bool copyThere, bool dirThere) {
    if (now - since < SnapVerifyMs) return SV_Wait;
    if (copyThere) return SV_Ok;
    return dirThere ? SV_Rewrite : SV_Gone;
}

// [General] TargetProbe (kept in extra): 0 off, 1 log only, 2 live. Missing -> 2; unparsable -> 2 and *bad = true.
inline int targetProbe(const General& g, bool* bad = nullptr) {
    if (bad) *bad = false;
    for (auto& kv : g.extra) {
        std::string k = kv.first; for (auto& ch : k) ch = (char)tolower((unsigned char)ch);
        if (k != "targetprobe") continue;
        int n = 0;
        if (parseInt32(kv.second, &n)) return n;
        if (bad) *bad = true;
        return 2;
    }
    return 2;
}

}  // namespace hb
