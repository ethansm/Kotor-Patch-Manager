// Force Power Hotbar -- item target selection (pure C++17, no Win32/engine types so it can be unit-tested on the host).
// The DLL fills one Candidate per party position (HP, "readable", "down", "can this item be used on them"); this header decides.
#pragma once
#include "HotbarConfig.h"
#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
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

// ======== log formatters (pure; ForcePowerHotbar.cpp passes the result to KP_LOG as "%s") ========
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

// "slot <key>: group=.. index=.. key=.. flags=.. usable=<flags&1> INVOKING|(Invoke=0) ref=<ref>" (ref last: it can be 100 bytes)
inline std::string fmtSlotResolve(const std::string& slotKey, int group, int index, unsigned key, unsigned flags, bool invoke, const std::string& refText) {
    return sfmt("slot %s: group=%d index=%d key=%08x flags=%08x usable=%d %s ref=%s", slotKey.c_str(), group, index, key, flags, (int)(flags & 1),
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
inline std::string fmtPersaveSource(const char* why, const std::string& source) { return sfmt("persave: %s: source=%.170s", why, source.c_str()); }

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
inline int stanceKey(int stance) {
    switch (stance) { case 9: return 0x410; case 10: return 0x411; case 0xB: return 0x412; case 0xC: return 0x413; case 0xD: return 0x414; case 16: return 0x415; default: return 0; }
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
inline bool dropsCustomSuffix(const TargetSpec& from) { return from.thresholdPct >= 0 || from.fallback >= 0; }
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
inline std::string slotTargetLine(const TargetSpec& t, int effectiveThresholdPct) {
    if (t.mode == TM_LowHp) return "Target: Lowest HP (below " + std::to_string(effectiveThresholdPct) + "%)";
    return t.mode == TM_Self ? "Target: Self" : "";
}

// ---- item edge cases (B2, doc 07): column-driven from spells.2da, item property 0 from the engine item ----
inline int parseHexOrInt(const std::string& s) {   // spells.2da category is "0x1408"; "****"/empty/garbage -> -1
    if (s.empty() || s == "****") return -1;
    char* e = nullptr; long v = strtol(s.c_str(), &e, 0);
    return (e && e != s.c_str() && *e == 0) ? (int)v : -1;
}
inline bool isHealingRow(int usertype, const std::string& range, int category) {
    return usertype == 4 && range == "T" && (category == 0x1408 || category == 0x1404 || category == 0x1048);
}
inline bool selfAllowedRow(const std::string& range, int hostileSetting) { return (range == "T" || range == "t") && hostileSetting != 1; }
struct ItemProp { int count = 0; int type = -1; int row = -1; int usable = 0; };
inline int usableSpellRow(const ItemProp& p) { return (p.count > 0 && p.type == 10 && p.usable != 0 && p.row >= 0) ? p.row : -1; }
inline const char* itemPropWhy(const ItemProp& p) {
    if (p.count <= 0) return "no properties";
    if (p.type != 10) return "property 0 not CastSpell";
    if (p.usable == 0) return "no uses left";
    return p.row < 0 ? "bad row" : "ok";
}

}  // namespace hb
