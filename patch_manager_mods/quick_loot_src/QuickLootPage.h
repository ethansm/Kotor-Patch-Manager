// QuickLootPage.h -- the Mod Configuration page of quick-loot (doc 52 QL7, doc 46 declarative layer). Pure text + a few pure helpers, no Win32, so
// quick_loot_page_test.cpp parses the shipped schema on the host. Four rows:
//   anchor  list     quick_loot.ini [QuickLoot] Anchor            0 Under name tag / 1 Below container / 2 Beside name tag   (bound, stores the index)
//   prox    toggle   ProximityEnabled                              auto-open nearby containers                                  (bound, 0 / 1)
//   rows    stepper  ListRows                                      visible item rows, 4..12 step 2                              (bound, the value)
//   dist    list     (unbound) OpenM presets Near 2 m / Normal 3 m / Far 4.5 m; the hook writes OpenM into the ini itself and CloseM follows as OpenM + 2
//                    (a CloseM key in the ini is an explicit override and wins, clamped to >= OpenM + 1)
// Enabled (master switch = install state), ScanMs (timer) and TakeKey stay ini-only: the declarative layer has no key-binding row type, and a VK code
// is not a taste setting (doc 45 value test). Every row applies live: the hook below sets the mod's own config (g_m.cfg), no restart.
#pragma once
#include "QuickLootLogic.h"
#include <stdint.h>
#include <string.h>

namespace qlpage {

constexpr const char* kEntryId = "quick-loot";
constexpr const char* kIniFile = "quick_loot.ini";
constexpr const char* kSection = "QuickLoot";
constexpr const char* kRowAnchor = "anchor";
constexpr const char* kRowProx = "prox";
constexpr const char* kRowRows = "rows";
constexpr const char* kRowDist = "dist";

// Every string is at most 95 bytes (TomlMini rejects longer ones; see selfheal_page_test).
constexpr const char* kPageSchema = R"TOML(
[page]
name = "Quick Loot"
title = "Quick Loot"
desc = "How the container list is placed and when it opens. All changes apply at once."

[[row]]
id = "anchor"
kind = "list"
label = "List position"
desc = "Where the list shows: under the target name, below the container itself, or beside the name."
default = 0
choices = ["Under name tag", "Below container", "Beside name tag"]
ini = "quick_loot.ini"
section = "QuickLoot"
key = "Anchor"

[[row]]
id = "prox"
kind = "toggle"
label = "Auto-open nearby containers"
desc = "Walking up to a container lists its items. Off: only a click opens a container."
default = 1
ini = "quick_loot.ini"
section = "QuickLoot"
key = "ProximityEnabled"

[[row]]
id = "rows"
kind = "stepper"
label = "List rows"
desc = "Items visible at once. A longer list scrolls with the mouse wheel or the scrollbar."
default = 4
min = 4
max = 12
step = 2
fmt = "List rows: %d"
ini = "quick_loot.ini"
section = "QuickLoot"
key = "ListRows"

[[row]]
id = "dist"
kind = "list"
label = "Open distance"
desc = "How close you walk before a container lists its items. The list closes 2 m farther out."
default = 1
choices = ["Near (2 m)", "Normal (3 m)", "Far (4.5 m)"]
)TOML";

// ---- open-distance presets (the unbound "dist" row)
constexpr int kNumDist = 3;
constexpr float kDistOpenM[kNumDist] = { 2.0f, 3.0f, 4.5f };
constexpr int kDistDefault = 1;                 // Normal = qlogic::DefaultOpenM
constexpr float kCloseExtraM = 2.0f;            // CloseM = OpenM + this unless the ini overrides CloseM

inline int distIndexFor(float openM) {          // the nearest preset (a hand-edited OpenM = 3.4 shows as Normal)
    int best = 0;
    float bd = 1.0e9f;
    for (int i = 0; i < kNumDist; ++i) {
        const float d = openM > kDistOpenM[i] ? openM - kDistOpenM[i] : kDistOpenM[i] - openM;
        if (d < bd) { bd = d; best = i; }
    }
    return best;
}
inline float distOpenFor(int idx) { return kDistOpenM[(idx >= 0 && idx < kNumDist) ? idx : kDistDefault]; }
// CloseM for an OpenM: the explicit ini value (kept at least OpenM + 1) or OpenM + kCloseExtraM.
inline float closeFor(float openM, bool explicitClose, float explicitCloseM) {
    return explicitClose ? qlogic::clampCloseM(openM, explicitCloseM) : openM + kCloseExtraM;
}

// The live config the page edits. QuickLoot.cpp copies it into g_m.cfg after every accepted change.
struct Live {
    int anchor = 0;
    bool prox = true;
    unsigned rows = qlogic::ListRowsDefault;
    float openM = qlogic::DefaultOpenM, closeM = qlogic::DefaultCloseM;
    bool closeExplicit = false;
    float closeExplicitM = qlogic::DefaultCloseM;
};
// One change request from the page: returns the FINAL value (the request, clamped). *persistOpenM = the "dist" row changed OpenM (the hook writes it to the ini).
inline int32_t applyRow(Live& L, const char* id, int32_t v, bool* persistOpenM) {
    *persistOpenM = false;
    if (strcmp(id, kRowAnchor) == 0) { L.anchor = qlogic::clampAnchorMode(v); return L.anchor; }
    if (strcmp(id, kRowProx) == 0) { L.prox = v != 0; return L.prox ? 1 : 0; }
    if (strcmp(id, kRowRows) == 0) { L.rows = qlogic::clampListRows(v); return static_cast<int32_t>(L.rows); }
    if (strcmp(id, kRowDist) == 0) {
        const int i = (v >= 0 && v < kNumDist) ? v : kDistDefault;
        L.openM = distOpenFor(i);
        L.closeM = closeFor(L.openM, L.closeExplicit, L.closeExplicitM);
        *persistOpenM = true;
        return i;
    }
    return v;
}

}   // namespace qlpage
