// Force Power Hotbar -- configuration model (pure C++17, no Win32 types so it can be unit-tested on the host).
// A binding stores a stable, human-meaningful ActionRef, never a raw runtime key. The future in-game
// editor mutates a Profile and calls Config::Save; nothing else in the mod depends on the INI text.
#pragma once
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileWatch.h"   // filewatch::atomicWrite / stamp (Patches/Common; pulls in windows.h on Win32)

namespace hb {

constexpr int Rows = 1;    // Bottom (keys 1..0). The early Ctrl/Alt rows (Left/Right) were removed in 1.0.0: Alt+digit never reaches the game under Proton
constexpr int Slots = 10;  // digits 1..9, 0
constexpr int SchemaVersion = 1;

// Key digit shown for a slot (0..8 -> '1'..'9', 9 -> '0'); '?' for anything out of range.
inline char slotDigit(int slot) { return (slot >= 0 && slot < 10) ? "1234567890"[slot] : '?'; }

// Row name used in INI keys ("Bottom1".."Bottom0") + required modifier (0 none).
struct RowDef { const char* name; int modifier; };
constexpr RowDef kRows[Rows] = { {"Bottom", 0} };
constexpr int kFirstDik = 0x02;  // DIK_1; slot i uses DIK 0x02+i (0x0B = key 0 = slot 10)

enum Kind { K_None = 0, K_Spell, K_Feat, K_Item, K_Other };

// Per-slot target for items ("@lowhp" = the party member with the lowest HP%). -1 = unset: use the [General] value, so a
// [General] change still applies to slots that never overrode it, and Format can write the shortest form.
enum TargetMode { TM_Default = 0, TM_LowHp };
struct TargetSpec {
    TargetMode mode = TM_Default;
    int thresholdPct = -1;   // 1..100, -1 = [General] HealThreshold
    int fallback = -1;       // 1 = self, 0 = nothing, -1 = [General] HealFallback
};

struct ActionRef {
    Kind kind = K_None;
    int id = 0;            // Spell: spells.2da row, Feat: feat.2da row, Other: raw key minus flag bits
    std::string name;      // Item: display name (fallback identity) -- resref/tag once proven stable
    bool blank = false;    // explicit "none" (kind stays K_None so empty() is true): blocks the Default fallback and AutoFill
    TargetSpec target;     // "@lowhp..." suffix; not part of identity (Same ignores it)
    bool empty() const { return kind == K_None; }
};

inline std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) --b;
    return s.substr(a, b - a);
}
inline std::string lower(std::string s) { for (auto& c : s) if (c >= 'A' && c <= 'Z') c = char(c + 32); return s; }

// Item names in the engine's entries are "Name (3)" / "Name (Type) (3)": identity is the text before " (".
inline std::string normalizeItemName(const std::string& s) {
    std::string t = trim(s);
    size_t p = t.find(" (");
    if (p != std::string::npos) t = t.substr(0, p);
    return trim(t);
}

// Item stack count from a HUD entry: the engine formats item names "%s (%d)" only when the stack is > 1 (FUN_0074CF00).
// Returns 0 for non-items (key bit 0x40000000 clear) and for names without a trailing " (<digits>)".
inline int StackCount(int key, const std::string& name) {
    if (!(key & 0x40000000) || name.size() < 4 || name.back() != ')') return 0;
    size_t p = name.rfind(" (");
    if (p == std::string::npos || p + 3 > name.size() - 1) return 0;
    int n = 0;
    for (size_t i = p + 2; i < name.size() - 1; ++i) { if (name[i] < '0' || name[i] > '9' || n > 100000) return 0; n = n * 10 + (name[i] - '0'); }
    return n;
}

// "@lowhp[:<pct>[:self|none]]", shortest form that round-trips ("" for TM_Default).
inline std::string formatTarget(const TargetSpec& t) {
    if (t.mode != TM_LowHp) return "";
    std::string s = "@lowhp";
    if (t.thresholdPct < 0 && t.fallback < 0) return s;
    s += ":"; if (t.thresholdPct >= 0) s += std::to_string(t.thresholdPct);
    if (t.fallback >= 0) s += t.fallback ? ":self" : ":none";
    return s;
}

inline std::string Format(const ActionRef& r) {
    char b[32];
    switch (r.kind) {
    case K_Spell: snprintf(b, sizeof b, "spell:%d", r.id); return b + formatTarget(r.target);
    case K_Feat:  snprintf(b, sizeof b, "feat:%d", r.id); return b + formatTarget(r.target);
    case K_Item:  return "item:" + r.name + formatTarget(r.target);
    case K_Other: snprintf(b, sizeof b, "other:%x", (unsigned)r.id); return b + formatTarget(r.target);
    default: return r.blank ? "none" : "";
    }
}

// Parses the text after '@' ("lowhp[:pct[:self|none]]" or "default"); false on a bad pct/fallback/extra field.
inline bool parseTargetSuffix(const std::string& suf, TargetSpec* ts) {
    *ts = TargetSpec();
    std::string f[3]; int n = 0; size_t a = 0;
    for (;;) {
        size_t c = suf.find(':', a);
        if (n == 3) return false;
        f[n++] = lower(trim(suf.substr(a, c == std::string::npos ? std::string::npos : c - a)));
        if (c == std::string::npos) break;
        a = c + 1;
    }
    if (f[0] == "default") return n == 1;
    if (f[0] != "lowhp") return false;
    ts->mode = TM_LowHp;
    if (!f[1].empty()) {
        int v = 0;
        for (char ch : f[1]) { if (ch < '0' || ch > '9' || v > 1000) return false; v = v * 10 + (ch - '0'); }
        if (v < 1 || v > 100) return false;
        ts->thresholdPct = v;
    }
    if (!f[2].empty()) {
        if (f[2] == "self") ts->fallback = 1; else if (f[2] == "none") ts->fallback = 0; else return false;
    }
    return true;
}

// Returns false (and leaves *out empty) on a malformed value. An empty string parses as "empty slot", true.
inline bool Parse(const std::string& text, ActionRef* out) {
    *out = ActionRef();
    std::string t = trim(text);
    if (t.empty()) return true;
    // Strip a trailing "@lowhp.."/"@default" BEFORE the kind; any other '@' stays part of the value (item names).
    TargetSpec ts;
    size_t at = t.rfind('@');
    if (at != std::string::npos) {
        std::string suf = t.substr(at + 1); size_t c = suf.find(':');
        std::string w = lower(trim(suf.substr(0, c)));
        if (w == "lowhp" || w == "default") {
            if (!parseTargetSuffix(suf, &ts)) return false;
            t = trim(t.substr(0, at));
            if (t.empty() || lower(t) == "none") return false;
        }
    }
    if (lower(t) == "none") { out->blank = true; return true; }
    size_t c = t.find(':');
    if (c == std::string::npos) return false;
    std::string k = lower(trim(t.substr(0, c))), v = trim(t.substr(c + 1));
    if (v.empty()) return false;
    char* end = nullptr;
    if (k == "spell" || k == "feat") {
        long n = strtol(v.c_str(), &end, 10);
        if (*end || n < 0) return false;
        out->kind = (k == "spell") ? K_Spell : K_Feat; out->id = (int)n; out->target = ts; return true;
    }
    if (k == "other") {
        unsigned long n = strtoul(v.c_str(), &end, 16);
        if (*end) return false;
        out->kind = K_Other; out->id = (int)n; out->target = ts; return true;
    }
    if (k == "item") { out->kind = K_Item; out->name = normalizeItemName(v); out->target = ts; return !out->name.empty(); }
    return false;
}

inline bool Same(const ActionRef& a, const ActionRef& b) {
    if (a.kind != b.kind) return false;
    if (a.kind == K_Item) return lower(a.name) == lower(b.name);
    return a.id == b.id;
}

// "Bottom1".."Bottom9","Bottom0" etc.
inline std::string SlotKeyName(int row, int slot) {
    char b[24]; snprintf(b, sizeof b, "%s%d", kRows[row].name, (slot + 1) % 10); return b;
}
inline bool SlotFromKeyName(const std::string& key, int* row, int* slot) {
    for (int r = 0; r < Rows; ++r) {
        size_t n = strlen(kRows[r].name);
        if (lower(key.substr(0, n)) == lower(kRows[r].name) && key.size() == n + 1 && key[n] >= '0' && key[n] <= '9') {
            int d = key[n] - '0'; *row = r; *slot = (d + 9) % 10; return true;
        }
    }
    return false;
}
// "Left<d>" / "Right<d>" keys written by pre-1.0 builds (Ctrl/Alt rows): silently dropped, never a warning (a warning would refuse every Save).
inline bool IsRemovedRowKey(const std::string& key) {
    for (const char* n : { "left", "right" }) {
        size_t len = strlen(n);
        if (key.size() == len + 1 && lower(key.substr(0, len)) == n && key[len] >= '0' && key[len] <= '9') return true;
    }
    return false;
}

struct Profile {
    std::string name;
    ActionRef slots[Rows][Slots];
    std::string notes[Rows][Slots];   // trailing "; comment" per slot, preserved across Save
};

// Picking an action already on the bar MOVES it: every other slot holding `a` becomes explicit "none" (note "moved"),
// returned in ascending order. blank, not empty: an empty slot would fall back to Default via slotFor and resurrect the action.
// The caller sets the target slot itself; an empty/blank ref (the Clear path) matches nothing.
inline std::vector<int> moveDuplicates(Profile& p, int row, int slot, const ActionRef& a) {
    std::vector<int> moved;
    if (a.empty() || row < 0 || row >= Rows) return moved;
    for (int i = 0; i < Slots; ++i) {
        if (i == slot || !Same(p.slots[row][i], a)) continue;
        p.slots[row][i] = ActionRef(); p.slots[row][i].blank = true;
        p.notes[row][i] = "moved"; moved.push_back(i);
    }
    return moved;
}

struct General {
    int invoke = 1;               // 1 = keys/clicks use the slot's action, 0 = log only (dry run)
    int verbose = 0;              // 1 = detailed trace log (hover, widgets, every key), 0 = state changes and errors only
    int autoFill = 0;             // 1 = unbound Bottom slots show/invoke the character's known powers, in game order (default off: start empty)
    int keyboardDevice = -1;      // -1 = read the engine's constant (0x0099C884)
    int healThreshold = 50;       // 1..100: "@lowhp" slots only target a member below this HP%
    int healFallbackSelf = 1;     // when nobody is below the threshold: 1 = use on self, 0 = do nothing
    std::map<std::string, std::string> extra;  // unknown [General] keys, preserved
};

// Slot override wins; -1 (unset) defers to [General].
inline int effectiveThreshold(const TargetSpec& t, const General& g) { return t.thresholdPct >= 0 ? t.thresholdPct : g.healThreshold; }
inline bool effectiveFallbackSelf(const TargetSpec& t, const General& g) { return t.fallback >= 0 ? t.fallback != 0 : g.healFallbackSelf != 0; }

struct Config {
    General general;
    std::map<std::string, Profile> profiles;   // key = lowercase profile name
    std::vector<std::string> warnings;

    Profile& profile(const std::string& name) {
        Profile& p = profiles[lower(name)];
        if (p.name.empty()) p.name = name;
        return p;
    }
    const Profile* find(const std::string& name) const {
        auto it = profiles.find(lower(name));
        return it == profiles.end() ? nullptr : &it->second;
    }
    // Per-slot resolution: the character's profile, falling back to Default when the slot is empty/missing.
    ActionRef slotFor(const std::string& characterKey, int row, int slot) const {
        if (!characterKey.empty()) {
            const Profile* p = find(characterKey);
            if (p && (!p->slots[row][slot].empty() || p->slots[row][slot].blank)) return p->slots[row][slot];
        }
        const Profile* d = find("Default");
        return d ? d->slots[row][slot] : ActionRef();
    }

    // Editor API: sets (or, with a blank ref, clears) one slot of a profile. note = trailing comment.
    void SetSlot(const std::string& profileName, int row, int slot, const ActionRef& a, const std::string& note) {
        Profile& p = profile(profileName);
        p.slots[row][slot] = a; p.notes[row][slot] = note;
    }

    static bool readFile(const std::string& path, std::string* out) {
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return false;
        char buf[4096]; size_t n;
        out->clear();
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) out->append(buf, n);
        fclose(f);
        return true;
    }

    // Never throws; a malformed line adds a warning and leaves that slot empty.
    bool Parse(const std::string& text) {
        *this = Config();
        std::string section; bool inGeneral = false; Profile* cur = nullptr;
        size_t pos = 0; int lineNo = 0;
        if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF) pos = 3;
        while (pos <= text.size()) {
            size_t e = text.find('\n', pos);
            std::string line = text.substr(pos, e == std::string::npos ? std::string::npos : e - pos);
            pos = (e == std::string::npos) ? text.size() + 1 : e + 1;
            ++lineNo;
            std::string note;
            size_t sc = line.find(';');
            if (sc != std::string::npos) { note = trim(line.substr(sc + 1)); line = line.substr(0, sc); }
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;
            char msg[160];
            if (line[0] == '[') {
                size_t rb = line.find(']');
                if (rb == std::string::npos) { snprintf(msg, sizeof msg, "line %d: unterminated section", lineNo); warnings.push_back(msg); cur = nullptr; inGeneral = false; continue; }
                section = trim(line.substr(1, rb - 1)); inGeneral = false; cur = nullptr;
                if (lower(section) == "general") inGeneral = true;
                else if (lower(section).compare(0, 8, "profile.") == 0 && section.size() > 8) cur = &profile(section.substr(8));
                else { snprintf(msg, sizeof msg, "line %d: unknown section [%s]", lineNo, section.c_str()); warnings.push_back(msg); }
                continue;
            }
            size_t eq = line.find('=');
            if (eq == std::string::npos) { snprintf(msg, sizeof msg, "line %d: expected key=value", lineNo); warnings.push_back(msg); continue; }
            std::string key = trim(line.substr(0, eq)), val = trim(line.substr(eq + 1));
            if (inGeneral) {
                std::string k = lower(key);
                if (k == "schemaversion") { /* validated on Save only */ }
                else if (k == "invoke") general.invoke = atoi(val.c_str());
                else if (k == "verbose") general.verbose = atoi(val.c_str());
                else if (k == "autofill") general.autoFill = atoi(val.c_str());
                else if (k == "keyboarddevice") general.keyboardDevice = atoi(val.c_str());
                else if (k == "healthreshold") {
                    char* e = nullptr; long n = strtol(val.c_str(), &e, 10);
                    if (val.empty() || *e || n < 1 || n > 100) { snprintf(msg, sizeof msg, "line %d: HealThreshold '%s' must be 1..100", lineNo, val.c_str()); warnings.push_back(msg); }
                    else general.healThreshold = (int)n;
                } else if (k == "healfallback") {
                    std::string v = lower(val);
                    if (v == "self") general.healFallbackSelf = 1; else if (v == "none") general.healFallbackSelf = 0;
                    else { snprintf(msg, sizeof msg, "line %d: HealFallback '%s' must be self or none", lineNo, val.c_str()); warnings.push_back(msg); }
                }
                else general.extra[key] = val;
            } else if (cur) {
                int r, s;
                if (IsRemovedRowKey(key)) continue;
                if (!SlotFromKeyName(key, &r, &s)) { snprintf(msg, sizeof msg, "line %d: unknown key '%s' in [%s]", lineNo, key.c_str(), section.c_str()); warnings.push_back(msg); continue; }
                ActionRef ref;
                if (!hb::Parse(val, &ref)) { snprintf(msg, sizeof msg, "line %d: bad value '%s' for %s (slot left empty)", lineNo, val.c_str(), key.c_str()); warnings.push_back(msg); }
                cur->slots[r][s] = ref; cur->notes[r][s] = note;
            } else { snprintf(msg, sizeof msg, "line %d: key '%s' outside any section", lineNo, key.c_str()); warnings.push_back(msg); }
        }
        return true;
    }
    bool Load(const std::string& path) {
        std::string t;
        if (!readFile(path, &t)) return false;
        return Parse(t);
    }

    // Canonical text: General, Default first, then the other profiles in name order.
    std::string Serialize() const {
        std::string o = "; Force Power Hotbar. Slots: Bottom1..Bottom9, Bottom0 = the on-screen slots for keys 1..9, 0.\n"
                        "; Values: spell:<spells.2da row>  feat:<row>  item:<item name>  other:<hex>  none (explicitly empty)  (missing/empty = unbound, AutoFill may fill it)\n"
                        "; [General] Invoke=1 use the action (0 = log only)  Verbose=1 detailed log  AutoFill=1 fill unbound slots with known powers\n"
                        "; Item/power value suffix @lowhp[:<pct>[:self|none]] = use on the party member with the lowest HP% (below pct, default HealThreshold; else self or none, default HealFallback)\n"
                        "; [General] HealThreshold=1..100 (default 50)  HealFallback=self|none (default self)\n"
                        "; KeyboardDevice=<n> overrides the engine's DirectInput keyboard index (omit = automatic)\n"
                        "; Profiles belong to the loaded save: saving copies this file into the save folder, loading a save restores its copy (none = empty bar).\n";
        char b[64];
        o += "[General]\n";
        snprintf(b, sizeof b, "SchemaVersion=%d\n", SchemaVersion); o += b;
        snprintf(b, sizeof b, "Invoke=%d\n", general.invoke); o += b;
        snprintf(b, sizeof b, "Verbose=%d\n", general.verbose); o += b;
        snprintf(b, sizeof b, "AutoFill=%d\n", general.autoFill); o += b;
        snprintf(b, sizeof b, "HealThreshold=%d\n", general.healThreshold); o += b;
        o += general.healFallbackSelf ? "HealFallback=self\n" : "HealFallback=none\n";
        if (general.keyboardDevice >= 0) { snprintf(b, sizeof b, "KeyboardDevice=%d\n", general.keyboardDevice); o += b; }
        for (auto& kv : general.extra) o += kv.first + "=" + kv.second + "\n";
        auto emit = [&](const Profile& p) {
            o += "\n[Profile." + p.name + "]\n";
            for (int r = 0; r < Rows; ++r) for (int s = 0; s < Slots; ++s) {
                if (p.slots[r][s].empty() && !p.slots[r][s].blank) continue;
                o += SlotKeyName(r, s) + "=" + Format(p.slots[r][s]);
                if (!p.notes[r][s].empty()) o += "   ; " + p.notes[r][s];
                o += "\n";
            }
        };
        if (const Profile* d = find("Default")) emit(*d);
        for (auto& kv : profiles) if (kv.first != "default") emit(kv.second);
        return o;
    }

    // Atomic: <path>.tmp then replace (filewatch::atomicWrite). No .bak here: the hotbar makes its own backups (FPH assignSlot / replaceLiveProfiles).
    bool Save(const std::string& path) const { return filewatch::atomicWrite(path.c_str(), Serialize(), false); }
};

} // namespace hb
